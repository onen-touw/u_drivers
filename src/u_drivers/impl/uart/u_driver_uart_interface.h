#pragma once

#include "u_driver_uart.h"

namespace __u_drivers
{

    // all method are public for direct call
    class u_driver_uart_interface_t
    {
    public:
        using drv_t = u_driver_uart_t;
        using uart_port_t = drv_t::unum_t;
        using cfg_t = driver_uart_cnf_t;
    private:
        drv_t& _driver;

    public:
        u_driver_uart_interface_t(uart_port_t port, const cfg_t& cfg) 
            : _driver(__u_drivers::__driver_uart__instance[static_cast<size_t>(port)]) 
        {
            
            __info_uart[static_cast<size_t>(port)].set_cfg(cfg);
            _driver.driver_init();
        }

        // auto config for choosen port from __cfg
        u_driver_uart_interface_t(uart_port_t port) 
            : _driver(__u_drivers::__driver_uart__instance[static_cast<size_t>(port)]) 
        {
            _driver.driver_init();
        }

        ~u_driver_uart_interface_t()
        {
            _driver.driver_deinit();
        }

        uint32_t get_BaudRate() const {
            return _driver.get_BaudRate();
        }

       void flush() const
        {
            _driver.flush();
        }

        size_t available() const
        {
            return _driver.available();
        }

        size_t write(const char *data, size_t len)
        {
            auto l = _driver.write(data, len);
            return l;
        }

        size_t write(const uint8_t *data, size_t len)
        {
            auto l = _driver.write(data, len);
            return l;
        }


        // write with serial-break (see protocol)
        size_t writeb(const char *data, size_t len)
        {
            auto l = _driver.writeb(data, len);
            return l;
        }

        // sz is buffer size (must sz >= available)
        size_t read(char *buff, size_t sz)
        {
            auto w = _driver.read(buff, sz);
            return w;
        }
        
        // You must ensure that the buff size is greater than the number of available characters!!!
        size_t read_auto(char *buff)
        {
            auto w = _driver.read_auto(buff);
            return w;
        }

        char read()
        {
            if (available())
            {
                char c[] = {0};
                _driver.read(c, 1);
                return c[0];
            }
            return 0;
        }

        size_t write(char c) 
        {
            auto l = _driver.write(&c, 1);
            return l;
        }
        
    };

} // __u_drivers