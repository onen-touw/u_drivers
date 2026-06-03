#pragma once

#include "u_driver_twai.h"
#include "u_driver_uart_cfg.h"

namespace __u_drivers
{



    // all method are public for direct call
    class u_driver_twai_interface_t
    {
    public:
        using drv_t = u_driver_twai_t;
        using cfg_t = u_driver_twai_cfg_t;
        using msg_t = twai_message_t;

    private:
        drv_t& _driver;

    public:

        u_driver_twai_interface_t()
            : _driver(__u_drivers::__driver_twai__instance) 
        {
            _driver.driver_init();
        }
    
        u_driver_twai_interface_t(const cfg_t& cfg) 
            : _driver(__u_drivers::__driver_twai__instance) 
        {         
            __info_twai.set_cfg(cfg);
            _driver.driver_init();
        }
        
        ~u_driver_twai_interface_t()
        {
            _driver.driver_deinit();
        }

       // no changes of driver - fields => thread safe
        esp_err_t write(msg_t &msg, uint32_t ttw = 200)
        {
            esp_err_t err = twai_transmit(&msg, ttw); 
            return err;
        }

        twai_status_info_t get_info() const {
            twai_status_info_t info;
            twai_get_status_info(&info);
            return info;
        } 

        uint32_t pkt_cnt() const {
            return get_info().msgs_to_tx;
        }

        msg_t read(uint32_t ttw = 200){
            esp_err_t err = ESP_OK;
            // esp_err_t err = twai_get_status_info(&_info);
            // if (_info.msgs_to_rx < 1 || err != ESP_OK)
            // {
            //     _rcv.data_length_code = 0;
            //     return _rcv;
            // }
            msg_t msg;
            err = twai_receive(&msg, ttw);
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
                msg.data_length_code = 0;
                return msg;
            }
            return msg;
        }

        bool recover()
        {
            twai_status_info_t info;
            if (twai_get_status_info(&info) != ESP_OK)
            {
                return false;
            }
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

} // __u_drivers