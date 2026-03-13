#pragma once

#include "u_sys/config.h"
#include "u_sys/error.h"
#include "u_sys/mutex.h"

// idf-include
#include "driver/i2c.h"
#include "esp_timer.h"

#define UFO_I2C_HARDWARE_SDA ((uint8_t)21)
#define UFO_I2C_HARDWARE_SCL ((uint8_t)22)
#define UFO_I2C_MAX_FREQ 1000000UL
#define UFO_I2C_TIMEOUT 0xFFFFF

namespace ufo
{
    namespace drv
    {

        enum class UFO_I2C_port : uint8_t
        {
            UFO_I2C_HARDWARE,
            UFO_I2C_SOFTWARE,
        };

        // todo : create driver basic_class
        //  init(begin/setup)
        class UFO_I2C_Driver
        {

        private:
            mutex_t _lock;

            uint32_t _freq = 0;
            UFO_I2C_port _port;
            uint32_t _timeOutMillis = 50;
            bool _inited = false;

            dev_status_t _dstatus = dev_status_t::undef;

        public:
            UFO_I2C_Driver() {}

            ~UFO_I2C_Driver()
            {
                lock_guard<mutex_t> _l(_lock);
                if (_inited)
                {
                    esp_err_t ret = i2c_driver_delete(static_cast<i2c_port_t>(_port));
                    if (ret != ESP_OK)
                    {
                        __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::i2c_driver_del, "not inited")));
                        _dstatus = dev_status_t::error;
                    }
                    _dstatus = dev_status_t::off;
                }
            }

            bool Initialized()
            {
                lock_guard<mutex_t> _l(_lock);
                return _inited;
            }

            esp_err_t Init(UFO_I2C_port port, gpio_num_t pinSDA, gpio_num_t pinSCL)
            {
                esp_err_t ret = ESP_OK;

                lock_guard<mutex_t> _l(_lock);
                if (_inited)
                {
                    return ret;
                }

                if (port == UFO_I2C_port::UFO_I2C_SOFTWARE)
                {
                    if (config::ufo_i2c_supported < 2)
                    {
                        __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::i2c_not_supported, "i2c-driver::no software")));
                        ret = ESP_FAIL;
                        _dstatus = dev_status_t::error;
                        return ret;
                    }
                }

                if (port != UFO_I2C_port::UFO_I2C_HARDWARE)
                {
                    if (pinSCL == gpio_num_t::GPIO_NUM_21 || pinSCL == gpio_num_t::GPIO_NUM_22 || pinSCL == gpio_num_t::GPIO_NUM_22 || pinSCL == gpio_num_t::GPIO_NUM_21)
                    {
                        __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::i2c_incr_pins, "i2c-driver::incorrect pins")));

                        ret = ESP_FAIL;
                        _dstatus = dev_status_t::error;
                        return ret;
                    }
                }
                if (!_freq)
                {
                    _freq = 400000UL;
                }
                _port = port;

                i2c_config_t conf = {};
                conf.mode = I2C_MODE_MASTER;
                conf.scl_io_num = pinSCL;
                conf.sda_io_num = pinSDA;
                conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
                conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
                conf.master.clk_speed = _freq;
                conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;

                ret = i2c_param_config(static_cast<i2c_port_t>(_port), &conf);
                if (ret != ESP_OK)
                {
                    // CriticalError_t e;
                    // e._info = GenerateInfo_Code(error::codes_t::i2c_conf_fail, "configurating failed");
                    // __global_error.Push(e);
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::i2c_conf_fail, "configurating failed")));
                    _dstatus = dev_status_t::off;
                    return ret;
                }

                // @param slv_rx_buf_len Receiving buffer size. Only slave mode will use this value, it is ignored in master mode.
                // @param slv_tx_buf_len Sending buffer size. Only slave mode will use this value, it is ignored in master mode.
                ret = i2c_driver_install(static_cast<i2c_port_t>(_port), conf.mode, 0, 0, 0);
                if (ret != ESP_OK)
                {
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::i2c_conf_fail, "installing failed")));
                    _dstatus = dev_status_t::off;
                    return ret;
                }
                ret = i2c_set_timeout(static_cast<i2c_port_t>(_port), UFO_I2C_TIMEOUT);
                if (ret != ESP_OK)
                {
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::i2c_tcfg_fail, "timeout-config failed")));
                    _dstatus = dev_status_t::off;
                    return ret;
                }

                _inited = true;
                _dstatus = dev_status_t::ok;

                return ret;
            }
            esp_err_t Write(uint8_t addr, uint8_t *buf, uint32_t size)
            {
                lock_guard<mutex_t> _l(_lock);

                esp_err_t ret = ESP_FAIL;

                if (__CheckInitOrThrow() != ESP_OK)
                {
                    return ret;
                }

                ret = __Write(addr, buf, size);
                if (ret != ESP_OK)
                {
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::i2c_write, "write problem")));
                    _dstatus = dev_status_t::warning;
                }

                return ret;
            }

            // used only in device detector
            // 0 = good; 1 = bad
            uint8_t ZeroWrite(uint8_t addr)
            {
                lock_guard<mutex_t> _l(_lock);

                esp_err_t ret = ESP_FAIL;

                if (__CheckInitOrThrow() != ESP_OK)
                {
                    return ret;
                }

                uint32_t sz = 0;
                uint8_t buf[] = {0};
                ret = __Write(addr, buf, sz);

                return ret;
            }

            // not used
            esp_err_t Read(uint8_t address, uint8_t *buff, size_t size)
            {
                lock_guard<mutex_t>_l(_lock);

                esp_err_t ret = ESP_FAIL;
                if (__CheckInitOrThrow() != ESP_OK)
                {
                    return ret;
                }
                ret = i2c_master_read_from_device(static_cast<i2c_port_t>(_port), address, buff, size, _timeOutMillis / portTICK_PERIOD_MS);
                if (ret != ESP_OK)
                {
                // todo::
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::i2c_write_read, "read problem")));
                    _dstatus = dev_status_t::warning;
                }

                return ret;
            }

            // write address, reg(cmd) => get value
            esp_err_t WriteRead(uint8_t address, uint8_t *wbuff, size_t wsize, uint8_t *rbuff, size_t rsize)
            {
                lock_guard<mutex_t> _l(_lock);

                esp_err_t ret = ESP_FAIL;
                if (__CheckInitOrThrow() != ESP_OK)
                {
                    return ret;
                }

                ret = i2c_master_write_read_device(static_cast<i2c_port_t>(_port), address, wbuff, wsize, rbuff, rsize, _timeOutMillis / portTICK_PERIOD_MS);

                if (ret != ESP_OK)
                {
                    // Warning_t w;
                    // w._info = GenerateInfo_Code(error::codes_t::i2c_write_read, "write-read problem");
                    // __global_error.Push(w);
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::i2c_write_read, "write-read problem")));
                    _dstatus = dev_status_t::warning;
                }
                return ret;
            }

            dev_status_t get_status() const { return _dstatus; }
            void forse_status() { _dstatus = dev_status_t::ok; }

        private:
            esp_err_t __CheckInitOrThrow()
            {
                if (!_inited)
                {
                    // CriticalError_t e;
                    // e._info = GenerateInfo_Code(error::codes_t::i2c_null_operations, "not inited");
                    // __global_error.Push(e);
                    __global_error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::i2c_null_operations, "lnot inited")));
                    _dstatus = dev_status_t::error;
                return ESP_FAIL;
                }
                return ESP_OK;
            }

            esp_err_t __Write(uint8_t &addr, uint8_t *buff, uint32_t &size)
            {
                esp_err_t ret = ESP_FAIL;
                i2c_cmd_handle_t cmd = NULL;
                uint8_t cmd_buff[I2C_LINK_RECOMMENDED_SIZE(1)] = {0};
                cmd = i2c_cmd_link_create_static(cmd_buff, I2C_LINK_RECOMMENDED_SIZE(1));

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
                ret = i2c_master_cmd_begin(static_cast<i2c_port_t>(_port), cmd, _timeOutMillis / portTICK_PERIOD_MS);
                return ret;
            }
        };

    } // namespace driver
} // namespace ufo
