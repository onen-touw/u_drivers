#pragma once

#include "../u_driver_cfg.h"
#include "soc/gpio_num.h"
#include <driver/spi_common.h>
#include <hal/spi_types.h>

namespace __u_drivers
{

    using driver_spi_port_e = spi_host_device_t;
    using driver_spi_cnf_t = spi_bus_config_t;

    static driver_meta_registry __meta_spi[driver_spi_port_e::SPI_HOST_MAX] = {};
    static driver_spi_cnf_t __cfg_spi[driver_spi_port_e::SPI_HOST_MAX] = {};

} // namespace __u_drivers
