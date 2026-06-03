#pragma once 

#include "../u_driver_cfg.h"
#include "soc/gpio_num.h"

namespace __u_drivers
{
   enum class driver_i2c_port_e : size_t
   {
      hardware = 0,
#if U_I2C_DRIVERS_CNT > 1
      sofrware = 1,
#endif
   };

   enum class bit_order_e
   {
      LSB,
      MSB
   };
   
   struct driver_i2c_cnf_t
   {
      gpio_num_t i2c_sda = gpio_num_t::GPIO_NUM_21;
      gpio_num_t i2c_scl = gpio_num_t::GPIO_NUM_22; 
      uint32_t frequency = 400000UL;
   };

   
   // static driver_info_t __meta_i2c[U_I2C_DRIVERS_CNT] = {};
   // static  driver_i2c_cnf_t __cfg_i2c[U_I2C_DRIVERS_CNT] = {};

   // use default config that coded in driver_i2c_cnf_t as default structure initialization
   static driver_info_t<driver_i2c_cnf_t> __info_i2c[U_I2C_DRIVERS_CNT] = {driver_i2c_cnf_t{}};


} // namespace __u_drivers
