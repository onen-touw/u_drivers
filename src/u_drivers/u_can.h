#pragma once

#include "impl/uart/u_driver_twai_interface.h"

namespace ufo
{
    /// @brief use u_uart_t::uart_port_t and u_uart_t::cfg_t for param driver
    using u_can_t = __u_drivers::u_driver_twai_interface_t;

    using u_can_cfg_t = __u_drivers::u_driver_twai_cfg_t;
    using u_can_meta_t = __u_drivers::driver_meta_t;

    auto get_can_driver_cfg() {
        u_can_cfg_t cfg = __u_drivers::__info_twai.get_cfg();
        return cfg;
    }

    auto get_uart_driver_meta() {
        u_can_meta_t meta = __u_drivers::__info_twai.get_meta();
        return meta;
    }

} // namespace ufo
