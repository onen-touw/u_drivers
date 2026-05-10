#pragma once

#include <stdint.h>
#include <type_traits>

#include "u_sys/btflg.h"
#include "u_sys/mutex.h"

#include "soc/gpio_num.h"

#ifndef U_UART_DRIVERS_CNT
#   define U_UART_DRIVERS_CNT 1
#else 
#   if U_UART_DRIVERS_CNT > 3
#       error "esp support only 3 uart"
#   endif
#endif

#ifndef U_I2C_DRIVERS_CNT
#  define U_I2C_DRIVERS_CNT 1
#else 
#   if U_I2C_DRIVERS_CNT > 2
#       error "esp support only hard- and software i2c"
#   endif
#endif

namespace __u_drivers
{
    enum class driver_state_t : uint8_t
    {
        initialized,
        started,
        have_error,
    };

    struct driver_meta_t
    {
        ufo::bit_flag_t<uint16_t> state; // temp
        uint16_t error_count = 0;
        uint8_t error_code = 0;
        uint8_t user_count = 0;
    };

    // Шаблонный класс для метаданных драйвера
    class driver_meta_registry
    {
    public:
        using error_callback_t = void (*)(const driver_meta_t &); // call callback with meta data
        using lock_t = ufo::mutex_t;
    private:
        driver_meta_t _meta = {};
        mutable ufo::mutex_t _mutex; 
        error_callback_t _clb = nullptr;

    public:
        void set_state(driver_state_t state, bool togle = false)
        {
            ufo::lock_guard<lock_t> lock(_mutex);

            if (togle)
            {
                _meta.state.togle(state);
                return;
            }
            _meta.state.set(state);
        }

        void inc_error(uint8_t error_code = 0)
        {
            ufo::lock_guard<lock_t> lock(_mutex);
            _meta.error_count++;
            _meta.error_code = error_code;
            _meta.state.set(driver_state_t::have_error);
            if (_clb)
            {
                _clb(_meta);
            }
        }

        void inc_user()
        {
            ufo::lock_guard<lock_t> lock(_mutex);
            ++_meta.user_count;
        }

        void dec_user()
        {
            ufo::lock_guard<lock_t> lock(_mutex);
            if (_meta.user_count > 0)
            {
                --_meta.user_count;
            }
        }

        // return copy
        driver_meta_t get_meta() const
        {
            ufo::lock_guard<lock_t> lock(_mutex);
            return _meta;
        }

        void set_error_callback(error_callback_t cb)
        {
            _clb = cb;
        }
    };

} // namespace __u_drivers
