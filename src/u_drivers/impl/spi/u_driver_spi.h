#pragma once

#include "u_driver_spi_cfg.h"

// idf-include
#include <driver/spi_master.h>
#include "esp_timer.h"
#include "esp_log.h"

namespace __u_drivers
{

    class u_driver_spi_t
    {
    public:
        static constexpr const char* tag = "dspi";
    private:
        driver_spi_port_e port;
        bool _initialized = false;
        

    public:
        u_driver_spi_t()
        {
            ESP_LOGI(tag, "[construct]");
        }
        ~u_driver_spi_t()
        {
            ESP_LOGI(tag, "[destruct]");
            driver_deinit();
        }

        u_driver_spi_t(const u_driver_spi_t&) = delete;
        u_driver_spi_t(u_driver_spi_t&&) = delete;

        bool initialized() const {return _initialized;}

        driver_spi_port_e get_port() const { return port; }

        esp_err_t driver_init(driver_spi_port_e _port)
        {
            if (_initialized)
            {
                return ESP_OK;
            }
            port = _port;

            ESP_LOGI(tag, "[init] start");

            
            const auto& cfg = __info_spi[static_cast<size_t>(port)].get_cfg();
            
            esp_err_t ret = spi_bus_initialize(_port, &cfg, SPI_DMA_DISABLED );

            if (ret != ESP_OK)
            {
                ESP_LOGI(tag, "[init] !init");
                __set_err(1);
                return ret;
            }

            __info_spi[static_cast<size_t>(port)].set_state(driver_state_t::initialized);
            __info_spi[static_cast<size_t>(port)].set_state(driver_state_t::started);
            _initialized = true;

            ESP_LOGI(tag, "[init] done");
            return ret;
        }

        void driver_deinit()
        {
            if (!_initialized)
            {
                return;
            }
            ESP_LOGI(tag, "[deinit]");

            __info_spi[static_cast<size_t>(port)].set_state(driver_state_t::initialized, true);
            __info_spi[static_cast<size_t>(port)].set_state(driver_state_t::started, true);
            esp_err_t ret = spi_bus_free(port);

            // never happen because of static declaration
            if (ret != ESP_OK) {
                ESP_LOGI(tag, "[init] fail");
                __set_err(4);
            }
            _initialized = false;
        }

        spi_device_handle_t mk_dev(spi_device_interface_config_t cfg)
        {
            spi_device_handle_t handle = nullptr;
            esp_err_t err = spi_bus_add_device(port, &cfg, &handle);
            if (err != ESP_OK)
            {
                __set_err(10);
                return nullptr;
            }
            return handle;
        }

        void rm_dev(spi_device_handle_t hand)
        {
            esp_err_t e = spi_bus_remove_device(hand);
            if (e != ESP_OK)
            {
                __set_err(11);
            }
            hand = nullptr;
        }

        private:
            void __set_err(uint8_t code)
            {
                __info_spi[static_cast<size_t>(port)].inc_error(code);
            }
    };

    static u_driver_spi_t __driver_spi__instance[driver_spi_port_e::SPI_HOST_MAX] = {};

} /// namespace __u_drivers