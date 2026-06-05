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

        /// @brief write msg to CAN bus
        /// @param msg twai-msg packet
        /// @param ttw time to wait
        /// @param recovery_retry cycles of auto-recovery if bus is in bus-off state. If 0 - auto-recovery is disabled
        /// @return 
        esp_err_t write(msg_t &msg, uint32_t ttw = 500, size_t recovery_retry = 5)
        {
            return _driver.dwrite(msg, ttw, recovery_retry);
        }

        esp_err_t flush_tx() 
        {
            return _driver.dflush_tx();
        }

        esp_err_t flush_rx()
        {
            return _driver.dflush_rx();
        }

        twai_status_info_t get_info() const {
            twai_status_info_t info;
            twai_get_status_info(&info);
            return info;
        } 

        esp_err_t read(msg_t& msg, uint32_t ttw = 200){

            return _driver.dread(msg, ttw);
        }
    };

} // __u_drivers