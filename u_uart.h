#pragma once

#include "impl/uart/u_driver_uart_interface.h"

namespace ufo
{
    /// @brief use u_uart_t::uart_port_t and u_uart_t::cfg_t for param driver
    using u_uart_t = __u_drivers::u_driver_uart_interface_t;

    using u_uart_cfg_t = __u_drivers::driver_uart_cnf_t;
    using u_uart_meta_t = __u_drivers::driver_meta_t;

    auto get_uart_driver_cfg(u_uart_t::uart_port_t port= u_uart_t::uart_port_t::uart0) {
        u_uart_cfg_t cfg = __u_drivers::__cfg_uart[static_cast<size_t>(port)];
        return cfg;
    }

    auto get_uart_driver_meta(u_uart_t::uart_port_t port= u_uart_t::uart_port_t::uart0) {
        u_uart_meta_t meta = __u_drivers::__meta_uart[static_cast<size_t>(port)].get_meta();
        return meta;
    }

} // namespace ufo
