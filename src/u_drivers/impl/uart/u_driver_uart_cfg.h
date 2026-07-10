#pragma once

#include "../u_driver_cfg.h"
#include "hal/uart_types.h"

namespace __u_drivers
{

    struct driver_uart_cnf_t
    {
        gpio_num_t tx = gpio_num_t::GPIO_NUM_MAX;
        gpio_num_t rx = gpio_num_t::GPIO_NUM_MAX;
        uint32_t baudrate = 115200;
    };

    enum class u_uart_port_t
    {
        uart0 = uart_port_t::UART_NUM_0,
#if U_UART_DRIVERS_CNT > 1
        uart1 = uart_port_t::UART_NUM_1,
#endif
#if U_UART_DRIVERS_CNT > 2
        uart2 = uart_port_t::UART_NUM_2,
#endif
    };

    // for driver state tracing
    static driver_info_t<driver_uart_cnf_t> __info_uart[U_UART_DRIVERS_CNT] = 
    {
            driver_uart_cnf_t{.tx = gpio_num_t::GPIO_NUM_1, .rx = gpio_num_t::GPIO_NUM_3, .baudrate = 115200}

#if U_UART_DRIVERS_CNT > 1          /// default initialization
           , driver_uart_cnf_t{.tx = gpio_num_t::GPIO_NUM_16, .rx = gpio_num_t::GPIO_NUM_17, .baudrate = 115200} 
#endif

#if U_UART_DRIVERS_CNT > 2          /// default initialization
           , driver_uart_cnf_t{.tx = gpio_num_t::GPIO_NUM_18, .rx = gpio_num_t::GPIO_NUM_19, .baudrate = 115200} 
#endif
    };

//     // for auto config from .ini files and driver tracing
//     static driver_uart_cnf_t __cfg_uart[U_UART_DRIVERS_CNT] =
//         {
//             {.tx = gpio_num_t::GPIO_NUM_1, .rx = gpio_num_t::GPIO_NUM_3, .baudrate = 115200}
// #if U_UART_DRIVERS_CNT > 1
//            ,{.tx = gpio_num_t::GPIO_NUM_16, .rx = gpio_num_t::GPIO_NUM_17} 
// #endif
//         };

} // namespace __u_drivers
