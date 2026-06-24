#pragma once

#include "u_driver_twai_cfg.h"
#include "esp_log.h"

namespace __u_drivers
{
    class u_driver_twai_t
    {
    public:
        using uart_bind_t = u_uart_port_t;
        using msg_t = twai_message_t;
        static constexpr const char* tag = "dtwai";

    private:
        bool _initialized = false;

    public:
        u_driver_twai_t() 
        {
            ESP_LOGI(tag, "[construct]");
        }
        ~u_driver_twai_t() 
        {
            ESP_LOGI(tag, "[destruct]");
            driver_deinit();
        }

        esp_err_t driver_init()
        {
            if (_initialized)
            {
                ESP_LOGE(tag, "[init]: double init");
                __set_err(10);
                return ESP_FAIL;
            }
            auto mcfg = __info_twai.get_cfg();
            auto uart_binded_user = __info_uart[static_cast<size_t>(mcfg.port)].get_user_cnt();

            ESP_LOGI(tag, "(%u)[init]: start", static_cast<size_t>(mcfg.port));
            ESP_LOGI(tag, "(%u)[init]: binding to uart%u", static_cast<size_t>(mcfg.port), static_cast<size_t>(mcfg.port));
            
            if (uart_binded_user)
            {
                ESP_LOGE(tag, "(%u)[init]: binding fail", static_cast<size_t>(mcfg.port));
                return ESP_FAIL;
            }
            auto ubind_cfg = __info_uart[static_cast<size_t>(mcfg.port)].get_cfg();
            
            gpio_reset_pin(ubind_cfg.rx);
            gpio_reset_pin(ubind_cfg.tx);

            twai_general_config_t general = {};
            general.controller_id = 0;       /**< TWAI controller ID, index from 0.
            //                     If you want to install TWAI driver with a non-zero controller_id,
            //                     please use `twai_driver_install_v2` */
            general.mode = TWAI_MODE_NORMAL;           /**< Mode of TWAI controller */
            general.tx_io = ubind_cfg.tx;              /**< Transmit GPIO number */
            general.rx_io = ubind_cfg.rx;              /**< Receive GPIO number */
            general.clkout_io = TWAI_IO_UNUSED;        /**< CLKOUT GPIO number (optional, set to -1 if unused) */
            general.bus_off_io = TWAI_IO_UNUSED;       /**< Bus off indicator GPIO number (optional, set to -1 if unused) */
            general.tx_queue_len = 5;                  /**< Number of messages TX queue can hold (set to 0 to disable TX Queue) */
            general.rx_queue_len = 5;                  /**< Number of messages RX queue can hold */
            general.alerts_enabled = TWAI_ALERT_NONE;  /**< Bit field of alerts to enable (see documentation) */
            general.clkout_divider = 0;                /**< CLKOUT divider. Can be 1 or any even number from 2 to 14 (optional, set to 0 if unused) */
            general.intr_flags = ESP_INTR_FLAG_LEVEL1; /**< Interrupt flags to set the priority of the driver's ISR.
                                        Note that to use the ESP_INTR_FLAG_IRAM,
                                        the CONFIG_TWAI_ISR_IN_IRAM option should
                                        be enabled first. */
            twai_timing_config_t timec = TWAI_TIMING_CONFIG_125KBITS();
            twai_filter_config_t filter = TWAI_FILTER_CONFIG_ACCEPT_ALL();

            esp_err_t err = ESP_OK;
            err = twai_driver_install(&general, &timec, &filter);
            if (err != ESP_OK)
            {
                ESP_LOGE(tag, "(%u)[init]: !install: %s",static_cast<size_t>(mcfg.port),  esp_err_to_name(err));
                __set_err(1);
                return err;
            }
            err = twai_start();
            if (err != ESP_OK)
            {
                ESP_LOGE(tag, "(%u)[init]: !dstart", static_cast<size_t>(mcfg.port));
                __set_err(2);
                twai_driver_uninstall();
                return err;
            }
            _initialized = true;

            __info_twai.set_state(driver_state_t::initialized);
            __info_twai.inc_user();
            __info_twai.set_state(driver_state_t::started);
            
            ESP_LOGI(tag, "(%u)[init]: done", static_cast<size_t>(mcfg.port));

            return err;
        }

        esp_err_t driver_deinit()
        {
            if (!_initialized)
            {
                return  ESP_OK;
            }
            esp_err_t err = ESP_OK;

            ESP_LOGI(tag, "[deinit]: start");
            // checks
            err = twai_stop();
            if (err != ESP_OK)
            {
                ESP_LOGE(tag, "[init]: !dstop");
                __set_err(4);
            }
            err = twai_driver_uninstall();
            if (err != ESP_OK)
            {
                ESP_LOGE(tag, "[init]: !uninstall");
                __set_err(5);
            }
            _initialized = false;

            __info_twai.dec_user();
            __info_twai.set_state(driver_state_t::initialized, true);
            __info_twai.set_state(driver_state_t::started, true);

            ESP_LOGI(tag, "[deinit]: done");
            
            return err;
        }

        esp_err_t dwrite(msg_t &msg, uint32_t ttw, size_t recovery_retry)
        {
            esp_err_t err = twai_transmit(&msg, ttw);
            if (err != ESP_OK)
            {
                if (recovery_retry && err == ESP_ERR_INVALID_STATE)
                {
                    return drecover(recovery_retry);
                }       
            }
            return err;
        }
        
        esp_err_t drecover(size_t retry_cnt)
        {
            auto err = twai_initiate_recovery();
            if (err != ESP_OK)
            {
                __set_err(10);
                return err;
            }
            
            twai_status_info_t info;

            while (retry_cnt--)
            {
                err = twai_get_status_info(&info);

                if (err != ESP_OK)
                {
                    __set_err(11);
                    return err;
                }
                if (info.state == TWAI_STATE_STOPPED)
                {
                    err = twai_start();
                    if (err != ESP_OK)                    
                    {
                        __set_err(13);      
                    }
                    return err;
                }
                ufo::utl::sleep_for(100);
            }

            __set_err(12);
            return ESP_FAIL;
        }

        esp_err_t dread(msg_t& msg, uint32_t ttw)
        {
            esp_err_t err = twai_receive(&msg, ttw);
            if (err != ESP_OK)
            {
                msg.data_length_code = 0;
                if (err == ESP_ERR_TIMEOUT)
                {
                    return err;
                }
                __set_err(20);
                return ESP_FAIL;
            }
            return ESP_OK;
        }

        esp_err_t dflush_tx() 
        {
            auto err = twai_clear_transmit_queue();
            if (err != ESP_OK)
            {
                __set_err(30);
            }
            return err;
        }

        esp_err_t dflush_rx()
        {
            auto err = twai_clear_receive_queue();
            if (err != ESP_OK)
            {
                __set_err(31);
            }
            return err;
        }

    private:
        void __set_err(uint8_t code)
        {
            __info_twai.inc_error(code);
        }
        
    };

    static u_driver_twai_t __driver_twai__instance = {};

} // namespace __u_drivers
