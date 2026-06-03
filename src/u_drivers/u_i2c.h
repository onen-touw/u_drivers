#pragma once

#include "impl/i2c/u_driver_i2c_interface.h"

namespace ufo
{
    /// @brief use u_i2c_t::i2c_port_t for param driver
    using u_i2c_t = __u_drivers::u_driver_i2c_interface_t;
    using u_i2c_cfg_t = __u_drivers::driver_i2c_cnf_t;
    using u_i2c_meta_t = __u_drivers::driver_meta_t;

    auto get_i2c_driver_cfg(u_i2c_t::i2c_port_t port= u_i2c_t::i2c_port_t::hardware) {
        u_i2c_cfg_t cfg = __u_drivers::__info_i2c[static_cast<size_t>(port)].get_cfg();
        return cfg;
    }

    auto get_i2c_driver_meta(u_i2c_t::i2c_port_t port = u_i2c_t::i2c_port_t::hardware) {
        auto meta = __u_drivers::__info_i2c[static_cast<size_t>(port)].get_meta();
        return meta;
    }

} // namespace ufo
