#pragma once

#include <driver/spi_master.h>
#include "u_sys/gpio.h"

namespace ufo {
    namespace drv
    {
        class uspi_t
        {
        private:
            spi_host_device_t _devno = SPI_HOST_MAX;
            static uint8_t _drvcnt;
            dev_status_t _dstatus = dev_status_t::undef;
        public:
            uspi_t(){}
            
            ~uspi_t() {
                if (_devno != spi_host_device_t::SPI_HOST_MAX)
                {
                    spi_bus_free(_devno);
                }
                --_drvcnt;
                _dstatus = dev_status_t::off;
            }

            void init(spi_host_device_t devno, gpio_num_t mosi, gpio_num_t miso,gpio_num_t sclk){
                if (config::ufo_spi_supported < 2 )
                {
                    if (_devno == spi_host_device_t::SPI3_HOST)
                    {
                        __global_error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::spi_drv_init, "spi init")));
                        return;
                    }
                }
                if (_devno == spi_host_device_t::SPI1_HOST)
                {
                    _devno = SPI_HOST_MAX;
                    __global_error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::spi_drv_init, "spi init")));
                    return;
                }
                _devno = devno;
                
                
                if (_drvcnt == 2)
                {
                    __global_error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::spi_drv_init, "max cnt")));
                    // crit error
                    return;
                }
                
                spi_bus_config_t buscfg = {};
                buscfg.mosi_io_num = mosi == gpio_num_t::GPIO_NUM_NC ? -1 : mosi;
                buscfg.miso_io_num = miso == gpio_num_t::GPIO_NUM_NC ? -1 : miso;
                buscfg.sclk_io_num = sclk;
                buscfg.quadwp_io_num = -1;
                buscfg.quadhd_io_num = -1;
                buscfg.max_transfer_sz = 0;
                buscfg.flags = 0;
                esp_err_t ret = spi_bus_initialize(_devno, &buscfg, SPI_DMA_CH_AUTO );

                if (ret != ESP_OK){
                    // crit err
                    __global_error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::spi_drv_init, "ini")));
                }
                ++_drvcnt;
                _dstatus = dev_status_t::ok;
            }

            spi_host_device_t get_descriptor() const { return _devno; }

            spi_device_handle_t mk_dev(spi_device_interface_config_t cfg) {
                spi_device_handle_t handle = nullptr;
	            esp_err_t err = spi_bus_add_device(_devno, &cfg, &handle);
                if (err != ESP_OK)
                {
                    __global_error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::spi_drv_init, "descr")));
                    _dstatus = dev_status_t::error;
                    return nullptr;
                }
                return handle;
            }

            void rm_dev(spi_device_handle_t hand) {
                esp_err_t e = spi_bus_remove_device(hand);
                if (e != ESP_OK)
                {
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::spi_drv_init, "rm")));
                    _dstatus = dev_status_t::warning;
                }
                hand = nullptr;
            }
            dev_status_t get_status() const { return _dstatus; }
            void forse_status() { _dstatus = dev_status_t::ok; }
        };

        uint8_t uspi_t::_drvcnt = 0;

    } // namespace driver
}
