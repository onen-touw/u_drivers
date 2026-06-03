#pragma once
#include "u_driver_spi.h"
#include "esp_log.h"

namespace __u_drivers
{
    // all method are protected for inheritance
    class u_driver_spi_interface_t
    {
    public:
        using drv_t = u_driver_spi_t;
        using spi_port_t = driver_spi_port_e;
    private:
        drv_t& _driver;
        spi_device_handle_t _handle = nullptr;

    public:

        u_driver_spi_interface_t(const spi_device_interface_config_t& dev_cfg, spi_port_t port = spi_port_t::SPI1_HOST) : 
            _driver(__driver_spi__instance[static_cast<size_t>(port)])
        {
            auto e = _driver.driver_init(port);
            if (e!= ESP_OK)
            {
                return;
            }

            _handle = _driver.mk_dev(dev_cfg);
            if (!_handle)
            {
                ESP_LOGE(drv_t::tag, "mk_dev");
                return;
            }
            __info_spi[static_cast<size_t>(_driver.get_port())].inc_user();
        }

        ~u_driver_spi_interface_t()
        {
            if (_handle)
            {
                _driver.rm_dev(_handle);
                __info_spi[static_cast<size_t>(_driver.get_port())].dec_user();
            }
        }

        spi_device_handle_t get_native() {return _handle;}

        bool write(const uint8_t* data, size_t size)
        {
            spi_transaction_t transaction= {};
        
            if ( size > 0 ) {
                transaction.length = size * 8;	// bit size = byte * 8
                transaction.tx_buffer = data;
                transaction.rx_buffer = nullptr;
                return spi_device_transmit(_handle, &transaction) == 1;
            }
            return false;
        }

        bool read(const uint8_t* tx, uint8_t* rx, size_t len) {
            spi_transaction_t t = {};
            t.length = len * 8;
            t.tx_buffer = tx;
            t.rx_buffer = rx;
            t.rxlength = len * 8;
            return spi_device_transmit(_handle, &t) == 1;
        }

        bool transaction(spi_transaction_t& trans)
        {
            return spi_device_transmit(_handle, &trans) == 1;
        }
    };


} // namespace __u_drivers
