#pragma once

#include "impl/spi/u_driver_spi_interface.h"

namespace ufo
{
    /// @brief use u_uart_t::uart_port_t and u_uart_t::cfg_t for param driver
    using u_spi_t = __u_drivers::u_driver_spi_interface_t;
    using u_spi_port_e = u_spi_t::spi_port_t;

    using u_spi_cfg_t = __u_drivers::driver_spi_cnf_t;
    using u_spi_meta_t = __u_drivers::driver_meta_t;

    auto get_spi_driver_cfg(u_spi_port_e port = u_spi_port_e::SPI1_HOST)
    {
        u_spi_cfg_t cfg = __u_drivers::__cfg_spi[static_cast<size_t>(port)];
        return cfg;
    }

    auto get_spi_driver_meta(u_spi_port_e port = u_spi_port_e::SPI1_HOST) {
        u_spi_meta_t meta = __u_drivers::__meta_spi[static_cast<size_t>(port)].get_meta();
        return meta;
    }

} // namespace ufo