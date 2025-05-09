#pragma once

#include "driver/twai.h"
#include "u_sys/config.h"
#include "u_sys/mutex.h"

namespace ufo
{
    namespace drv
    {
        
        using can_msg_t = twai_message_t;

        class can_drv_t
        {
            
        public:
            enum class state_t
            {
                und,    //
                idle,   // driver is not inited, driver not running
                inited, // driver is inited, driver
                run,    //
            };
        private:
            state_t _state = state_t::idle;
            ufo::mutex_t _lock;
            can_msg_t _rcv = {};

        public:
            can_drv_t() {}
            ~can_drv_t() {
                if (_state != state_t::idle && _state != state_t::und)
                {
                    deinit();
                }
                
            }

            can_drv_t(can_drv_t&) = delete;
            can_drv_t& operator=(can_drv_t&) = delete;

            can_drv_t(can_drv_t&& other) : _state (other._state), _lock(std::move(other._lock))
            {
                other._state =  state_t::und;
            }

            can_drv_t& operator=(can_drv_t&& other) {
                if (&other != this)
                {
                    _state = other._state;
                    other._state = state_t::und;
                    _lock = std::move(other._lock);
                }
                return *this;
            }

            // twai_get_status_info();

            bool check_state() const {
                return _state == state_t::run;
            }

            esp_err_t init(gpio_num_t rx, gpio_num_t tx)
            {
                ufo::lock_guard<mutex_t> _l(_lock);

                gpio_reset_pin(rx);
                gpio_reset_pin(tx);

                twai_general_config_t general;
                general.controller_id = 0;       /**< TWAI controller ID, index from 0.
                //                     If you want to install TWAI driver with a non-zero controller_id,
                //                     please use `twai_driver_install_v2` */
                general.mode = TWAI_MODE_NORMAL;           /**< Mode of TWAI controller */
                general.tx_io = tx;                        /**< Transmit GPIO number */
                general.rx_io = rx;                        /**< Receive GPIO number */
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
                    Trace_t::log("twai_driver_install error\n");
                    // critical
                    return err;
                }
                _state = state_t::inited;
                err = twai_start();
                if (err != ESP_OK)
                {
                    Trace_t::log("twai_start error\n");
                    twai_driver_uninstall();
                    // critical
                    return err;
                }
                _state = state_t::run;
                return err;
            }

            esp_err_t deinit()
            {
                ufo::lock_guard<mutex_t> _l(_lock);

                esp_err_t err = ESP_OK;

                // checks
                err = twai_stop();
                if (err != ESP_OK)
                {
                    // critical
                }
                _state = state_t::inited;

                err = twai_driver_uninstall();
                if (err != ESP_OK)
                {
                    // critical
                }
                _state = state_t::idle;
                return err;
            }

            // no changes of driver - fields => thread safe
            esp_err_t write(can_msg_t &msg, uint32_t ttw = 200)
            {
                esp_err_t err = twai_transmit(&msg, ttw); 
                return err;
            }


            // // todo:
            //     // create msg_t _rcv in driver and read data from _rcv
            // // (???) no changes of driver - fields => thread safe
            // const uint8_t* read(uint32_t idd, uint32_t ttw = 200)
            // {
            //     ufo::lock_guard<mutex_t> _l(_lock);
             
            //     twai_status_info_t info;
            //     esp_err_t err = twai_get_status_info(&info);
            //     if (info.msgs_to_rx < 1 || err != ESP_OK)
            //     {
            //         return nullptr;
            //     }
            //     _rcv.identifier = idd;
            //     err = twai_receive(&_rcv, ttw);
            //     if (err != ESP_OK)
            //     {
            //         return nullptr;
            //     }
                
            //     return _rcv.data;
            // }

            can_msg_t& read(uint32_t ttw = 200){
                ufo::lock_guard<mutex_t> _l(_lock);
                esp_err_t err = ESP_OK;
                // twai_status_info_t info;
                // esp_err_t err = twai_get_status_info(&info);
                // if (info.msgs_to_rx < 1 || err != ESP_OK)
                // {
                //     _rcv.data_length_code = 0;
                //     return _rcv;
                // }
                err = twai_receive(&_rcv, ttw);
                if (err != ESP_OK)
                {
                    // printf("twai_receive err\n\t");
                    // switch (err)
                    // {
                    // case ESP_ERR_TIMEOUT:
                    // printf("ESP_ERR_TIMEOUT\n");
                    //     break;
                    // case ESP_ERR_INVALID_ARG:
                    // printf("ESP_ERR_INVALID_ARG\n");
                    //     break;
                    // case ESP_ERR_INVALID_STATE:
                    // printf("ESP_ERR_INVALID_STATE\n");
                    //     break;                        
                    // default:
                    //     break;
                    // }
                    _rcv.data_length_code = 0;
                    return _rcv;
                }
                return _rcv;
            }

            bool recover()
            {

                twai_status_info_t info;

                if (twai_get_status_info(&info) != ESP_OK)
                {
                    // critical
                    return false;
                }
                ufo::lock_guard<mutex_t> _l(_lock);

                switch (info.state)
                {
                case TWAI_STATE_BUS_OFF:
                    twai_initiate_recovery();
                    break;
                case TWAI_STATE_RECOVERING:
                case TWAI_STATE_STOPPED:
                    // warning
                    break;
                default:
                    // CEr
                    return false;
                    break;
                }
                return true;
            }
        };

        
    } // namespace drv

} // namespace ufo
