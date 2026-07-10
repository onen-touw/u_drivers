#pragma once

#include "driver/gpio.h"
#include "u_sys/btflg.h"

namespace __u_drivers
{

    enum class pin_state_bits_t : uint8_t
    {
        // none,           /// free for use do anything
        inited,         /// inited but you can call set_level
        used,           /// some program or user (not recomened) init this pin and use it
        
        user,           /// user mark pin and use it (matching via owner_code)
        driver,
        sys             /// system used (user cant operate with it and driver too)
    };
    
    enum pin_owner_code_t
    {
        POC_none,

        POC_system = 1,
        
        POC_driver_uart0,
        POC_driver_uart1,
        POC_driver_uart2,

        POC_driver_i2c_hard,
        POC_driver_i2c_soft,

        POC_driver_spi0,
        POC_driver_spi1,
        POC_driver_spi2,

        POC_user_space            /// for check if user set valid code
    };

    struct pin_mux_t
    {
        uint8_t _owner_code = 0;
        ufo::bit_flag_t<uint8_t> _flags;

        pin_mux_t(){}
        pin_mux_t(uint8_t owner, uint8_t state = 0) : 
            _owner_code(owner),
            _flags(state) {}


        pin_mux_t(pin_owner_code_t code, uint8_t state) : 
            _owner_code(code),
            _flags(state) {}
    };
    
    
    
    struct u_driver_gpio_cfg_t
    {
        static constexpr size_t pin_cnt = gpio_num_t::GPIO_NUM_MAX; 
        static constexpr uint8_t __state_sys = 0b10000;
        static constexpr uint8_t __state_drv = 0b01000;

        pin_mux_t gpio_mux[pin_cnt] = 
        {
            pin_mux_t{ pin_owner_code_t::POC_driver_uart0,   __state_drv },         /// gpio_num_0
            pin_mux_t{ pin_owner_code_t::POC_system,         __state_sys },         /// gpio_num_1
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_2
            pin_mux_t{ pin_owner_code_t::POC_driver_uart0,   __state_drv },         /// gpio_num_3
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_4
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_5
            pin_mux_t{ pin_owner_code_t::POC_system,         __state_sys },         /// gpio_num_6
            pin_mux_t{ pin_owner_code_t::POC_system,         __state_sys },         /// gpio_num_7
            pin_mux_t{ pin_owner_code_t::POC_system,         __state_sys },         /// gpio_num_8
            pin_mux_t{ pin_owner_code_t::POC_system,         __state_sys },         /// gpio_num_9
            pin_mux_t{ pin_owner_code_t::POC_system,         __state_sys },         /// gpio_num_10
            pin_mux_t{ pin_owner_code_t::POC_system,         __state_sys },         /// gpio_num_11
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_12
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_13
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_14
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_15
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_16
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_17
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_18
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_19
            pin_mux_t{ pin_owner_code_t::POC_system,         __state_sys },         /// gpio_num_20
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_21
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_22
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_23
            // no 24
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_25
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_26
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_27
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_28
            pin_mux_t{ pin_owner_code_t::POC_system,         __state_sys },         /// gpio_num_29
            pin_mux_t{ pin_owner_code_t::POC_system,         __state_sys },         /// gpio_num_30
            pin_mux_t{ pin_owner_code_t::POC_system,         __state_sys },         /// gpio_num_31
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_32
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_33
            pin_mux_t{ pin_owner_code_t::POC_none,           0           },         /// gpio_num_34
            pin_mux_t{ pin_owner_code_t::POC_system,         __state_sys },         /// gpio_num_35
            pin_mux_t{ pin_owner_code_t::POC_system,         __state_sys },         /// gpio_num_36
            pin_mux_t{ pin_owner_code_t::POC_system,         __state_sys },         /// gpio_num_37
            pin_mux_t{ pin_owner_code_t::POC_system,         __state_sys },         /// gpio_num_38
            pin_mux_t{ pin_owner_code_t::POC_system,         __state_sys }          /// gpio_num_39
        };
    };
    
    static u_driver_gpio_cfg_t __gpio_cfg;

}