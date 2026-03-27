#pragma once

#include "impl/wifi/u_driver_wifi_interface.h"

namespace ufo
{
    /// @brief use u_wifi_t::mode_t and u_wifi_t::cfg_t for param driver
    using u_wifi_t = __u_drivers::u_driver_wifi_interface_t;
} // namespace ufo
