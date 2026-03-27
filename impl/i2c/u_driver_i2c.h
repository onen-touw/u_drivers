#pragma once

#include "u_driver_i2c_cfg.h"

// idf-include
#include "driver/i2c.h"
#include "esp_timer.h"

namespace __u_drivers
{
    class u_driver_i2c_t
    {
    private:
        driver_i2c_port_e port;
        static constexpr uint32_t timeout = 50;
        bool _initialized = false;
    public:
        u_driver_i2c_t(){}
        ~u_driver_i2c_t()
        {
            driver_deinit();
        }

        bool initialized() const {return _initialized;}

        void driver_init(driver_i2c_port_e _port, gpio_num_t pinSDA, gpio_num_t pinSCL, uint32_t freq = 400000UL)
        {
            if (_initialized)
            {
                return;
            }
            port = _port;
            
            __meta_i2c[static_cast<size_t>(port)].set_state(driver_state_t::initialized);
         
            if (!freq)
            {
                freq = 400000UL;
            }
            i2c_config_t conf = {};
            conf.mode = I2C_MODE_MASTER;
            conf.scl_io_num = pinSCL;
            conf.sda_io_num = pinSDA;
            conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
            conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
            conf.master.clk_speed = freq;
            conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;

            auto ret = i2c_param_config(static_cast<i2c_port_t>(port), &conf);
            if (ret != ESP_OK)
            {
                __set_err(1);
                return;
            }

            // @param slv_rx_buf_len Receiving buffer size. Only slave mode will use this value, it is ignored in master mode.
            // @param slv_tx_buf_len Sending buffer size. Only slave mode will use this value, it is ignored in master mode.
            ret = i2c_driver_install(static_cast<i2c_port_t>(port), conf.mode, 0, 0, 0);
            if (ret != ESP_OK)
            {
                __set_err(2);
                return;
            }
            ret = i2c_set_timeout(static_cast<i2c_port_t>(port), 0XFFFF);
            if (ret != ESP_OK)
            {
                __set_err(3);
                return;
            }

            __meta_i2c[static_cast<size_t>(port)].inc_user();
            __meta_i2c[static_cast<size_t>(port)].set_state(driver_state_t::started);
            _initialized = true;
        }

        void driver_deinit()
        {
            if (!_initialized)
            {
                return;
            }

            __meta_i2c[static_cast<size_t>(port)].dec_user();
            __meta_i2c[static_cast<size_t>(port)].set_state(driver_state_t::initialized, true);
            __meta_i2c[static_cast<size_t>(port)].set_state(driver_state_t::started, true);
            esp_err_t ret = i2c_driver_delete(static_cast<i2c_port_t>(port));

            // never happen because of static declaration
            if (ret != ESP_OK) {
                __set_err(4);
            }
        }

        void Write(uint8_t addr, uint8_t *buf, uint32_t size)
        {
            esp_err_t ret = __Write(addr, buf, size);
            if (ret != ESP_OK)
            {
                __set_err(10);
            }
        }

        // write address, reg(cmd) => get value
        void WriteRead(uint8_t address, uint8_t *wbuff, size_t wsize, uint8_t *rbuff, size_t rsize)
        {
            esp_err_t ret = i2c_master_write_read_device(static_cast<i2c_port_t>(port), address, wbuff, wsize, rbuff, rsize, timeout / portTICK_PERIOD_MS);
            if (ret != ESP_OK)
            {
                __set_err(11);
            }
        }

        // used only in device detector
        // 0 = good; 1 = bad
        uint8_t ZeroWrite(uint8_t addr)
        {
            esp_err_t ret = ESP_FAIL;
            uint32_t sz = 0;
            uint8_t buf[] = {0};
            ret = __Write(addr, buf, sz);
            return ret;
        }

    private:
        esp_err_t __Write(uint8_t &addr, uint8_t *buff, uint32_t &size)
        {
            esp_err_t ret = ESP_FAIL;
            i2c_cmd_handle_t cmd = NULL;
            
            auto link_sz = I2C_LINK_RECOMMENDED_SIZE(1);

            uint8_t cmd_buff[link_sz] = {0};
            cmd = i2c_cmd_link_create_static(cmd_buff, link_sz);

            ret = i2c_master_start(cmd);
            if (ret != ESP_OK)
            {
                return ret;
            }
            ret = i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
            if (ret != ESP_OK)
            {
                return ret;
            }
            if (size)
            {
                ret = i2c_master_write(cmd, buff, size, true);
                if (ret != ESP_OK)
                {
                    return ret;
                }
            }
            ret = i2c_master_stop(cmd);
            if (ret != ESP_OK)
            {
                return ret;
            }
            ret = i2c_master_cmd_begin(static_cast<i2c_port_t>(port), cmd, timeout / portTICK_PERIOD_MS);
            return ret;
        }

        void __set_err(uint8_t code)
        {
            __meta_i2c[static_cast<size_t>(port)].inc_error(code);
        }

    };


    
    static u_driver_i2c_t __driver_i2c__instance[U_I2C_DRIVERS_CNT] = {};

} // namespace __u_drivers