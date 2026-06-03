#pragma once

#include "u_driver_uart_cfg.h"
#include "driver/uart.h"
#include "hal/uart_ll.h"
#include "soc/uart_pins.h"
#include "esp_log.h"

namespace __u_drivers
{
    class u_driver_uart_t
    {
    private:
        static constexpr uint32_t _timeoutMs = 100;
        static constexpr uint8_t _timeoutRx = 2;   
        static constexpr uint8_t _txFiFoFull = 120;
        static constexpr uint32_t buffer_sz = 255*2;   

    public:
        using unum_t = u_uart_port_t;
        static constexpr const char* tag = "duart";

    private:
        using iup_t = uart_port_t;
        iup_t _unum = iup_t::UART_NUM_MAX;
        bool _initialized = false;

    public:
        u_driver_uart_t(unum_t uart_num) : 
            _unum(static_cast<iup_t>(uart_num))

        {
            ESP_LOGI(tag, "(%u)[construct]", static_cast<size_t>(_unum));
        }

        ~u_driver_uart_t() 
        {
            ESP_LOGI(tag, "(%u)[destruct]", static_cast<size_t>(_unum));

           driver_deinit();
        }

        u_driver_uart_t(const u_driver_uart_t&) = delete;
        u_driver_uart_t(u_driver_uart_t&&) = delete;


        esp_err_t driver_init()
        {
            ESP_LOGI(tag, "(%u)[init]: start", static_cast<size_t>(_unum));

            if (_initialized)
            {
                ESP_LOGE(tag, "(%u)[init]: double init", static_cast<size_t>(_unum));
                __set_err(23);
                return ESP_FAIL;
            }
            auto user = __info_uart[_unum].get_user_cnt();
            if (user)
            {
                ESP_LOGE(tag, "(%u)[init]: double init user", static_cast<size_t>(_unum));
                __set_err(66);
                return ESP_FAIL;
            }

            esp_err_t err = ESP_OK;

            if (uart_is_driver_installed((_unum)))
            {
                ESP_LOGE(tag, "(%u)[init]: double install", static_cast<size_t>(_unum));
                __set_err(1);
                return ESP_FAIL;
            }
            

            err = uart_driver_install(_unum, buffer_sz, 0, 0, nullptr, 0);
            if (err != ESP_OK)
            {
                ESP_LOGE(tag, "(%u)[init]: !install", static_cast<size_t>(_unum));
                __set_err(2);
                return err; 
            }
            auto mcfg = __info_uart[_unum].get_cfg();

            uart_config_t ucfg = {};
            ucfg.baud_rate = mcfg.baudrate,
            ucfg.data_bits = UART_DATA_8_BITS,
            ucfg.parity = UART_PARITY_DISABLE,
            ucfg.stop_bits = UART_STOP_BITS_1,
            ucfg.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            ucfg.source_clk = UART_SCLK_APB; // ESP32, ESP32S2

            err = uart_param_config(_unum, &ucfg);
            if (err != ESP_OK)
            {
                ESP_LOGE(tag, "(%u)[init]: !config", static_cast<size_t>(_unum));
                __set_err(3);
                __uninstall();
                return err; 
            }

            if (_unum != iup_t::UART_NUM_0)
            {
                err = uart_set_pin(_unum, mcfg.tx, mcfg.rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
            }
            else {
                err = uart_set_pin(_unum, 1, 3, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
            }

            if (err != ESP_OK)
            {
                ESP_LOGE(tag, "(%u)[init]: !pinning", static_cast<size_t>(_unum));
                __set_err(4);
                __uninstall();
                return err; 
            }

            err = uart_set_rx_timeout(_unum, _timeoutRx);
            if (err != ESP_OK)
            {
                ESP_LOGE(tag, "(%u)[init]: !tm", static_cast<size_t>(_unum));
                __set_err(5);
                __uninstall();
                return err; 
            }

            err = uart_set_rx_full_threshold(_unum, _txFiFoFull);
            if (err != ESP_OK)
            {
                ESP_LOGE(tag, "(%u)[init]: !th", static_cast<size_t>(_unum));
                __set_err(6);
                __uninstall();
                return err; 
            }
            _initialized = true;
            __info_uart[static_cast<size_t>(_unum)].set_state(driver_state_t::initialized);
            __info_uart[static_cast<size_t>(_unum)].inc_user();
            __info_uart[static_cast<size_t>(_unum)].set_state(driver_state_t::started);

            ESP_LOGI(tag, "(%u)[init]: done", static_cast<size_t>(_unum));

            return err; 
        }

        void driver_deinit()
        {
            if (!_initialized)
            {
                return;
            }
            ESP_LOGI(tag, "(%u)[deinit]", static_cast<size_t>(_unum));

            __uninstall();

            __info_uart[static_cast<size_t>(_unum)].dec_user();
            __info_uart[static_cast<size_t>(_unum)].set_state(driver_state_t::initialized, true);
            __info_uart[static_cast<size_t>(_unum)].set_state(driver_state_t::started, true);
        }

        void set_BaudRate(uint32_t br)
        {
            esp_err_t err = ESP_OK;

            err = uart_set_baudrate(_unum, br);
            if (err)
            {
                __set_err(7);
            }
        }

        uint32_t get_BaudRate() const {
            uint32_t br = 0;
            uart_get_baudrate(_unum, &br);
            return br;
        }

        void flush() const
        {
            while (!uart_ll_is_tx_idle(UART_LL_GET_HW(_unum)))
            {
            }
            uart_flush_input(_unum);
        }

        size_t available() const
        {
            size_t av = 0;
            uart_get_buffered_data_len(_unum, &av);
            if (av == _txFiFoFull)
            {
                av = 0;
            }
            return av;
        }

        size_t write(const char *data, size_t len) const
        {
            if (!len)
            {
                return 0;
            }
            size_t l = uart_write_bytes(_unum, data, len);
            uart_wait_tx_done(_unum, (TickType_t)100);
            return l;
        }

        // write with serial-break (see protocol)
        size_t writeb(const char *data, size_t len)
        {
            if (!len)
            {
                return 0;
            }

            size_t l = 0;
            l = uart_write_bytes_with_break(_unum, data, len, 1);
            uart_wait_tx_done(_unum, (TickType_t)100);
            return l;
        }

        // sz is buffer size (must sz >= available)
        size_t read(char *buff, size_t sz)
        {
            size_t w = 0;
            w = uart_read_bytes(_unum, buff, sz, pdMS_TO_TICKS(_timeoutMs));
            return w;
        }
        
        // You must ensure that the buff size is greater than the number of available characters!!!
        size_t read_auto(char *buff)
        {
            size_t w = 0, av = 0;
            uart_get_buffered_data_len(_unum, &av);
            w = uart_read_bytes(_unum, buff, av, pdMS_TO_TICKS(_timeoutMs));
            return w;
        }

    private:
        void __set_err(uint8_t code)
        {
            __info_uart[static_cast<size_t>(_unum)].inc_error(code);
        }

        void __uninstall()
        {
            if (uart_is_driver_installed(_unum))
            {
                uart_driver_delete(_unum);
                return;
            }
        }

    };

    static u_driver_uart_t __driver_uart__instance[U_UART_DRIVERS_CNT] = 
    {
        u_driver_uart_t(u_uart_port_t::uart0)
#if U_UART_DRIVERS_CNT > 1
        ,u_driver_uart_t(u_uart_port_t::uart1)
#endif
    };


} // namespace __u_drivers
