#pragma once

#include "driver/twai.h"
#include "../u_driver_cfg.h"
#include "u_driver_uart_cfg.h"
#include "esp_log.h"

#define U_UART_DRIVERS_CNT 2

namespace __u_drivers
{
#if U_UART_DRIVERS_CNT > 1

struct u_driver_twai_cfg_t
{
    using twai_port_t = u_uart_port_t;
    twai_port_t port = twai_port_t::uart1;

    u_driver_twai_cfg_t(u_uart_port_t uart_bind)
    {
        if (uart_bind == u_uart_port_t::uart0)
        {
            ESP_LOGE("TWAI", "cant be binded to uart0 port");
            return;
        }
        port = uart_bind;
    }
};

static driver_info_t<u_driver_twai_cfg_t> __info_twai(u_driver_twai_cfg_t{u_uart_port_t::uart1});
// static u_driver_twai_cfg_t __cfg_twai{u_uart_port_t::uart1};

#endif

} // namespace __u_drivers
