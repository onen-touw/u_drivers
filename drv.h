#pragma once

#include "u_sys/config.h"
#include "i2c/UFO_I2C_driver.h"
#include "spi/uspi.h"
#include "d_wff/wf.h"
#include "uart/UFO_Uart.h"
#include "bits/unique_ptr.h"

#define UFO_SPI_CNT 1
#define UFO_UART_CNT 2


namespace ufo
{
    namespace drv
    {
        class drv_t
        {
        public:

            static constexpr gpio_num_t spi2_mosi = gpio_num_t::GPIO_NUM_0;
            static constexpr gpio_num_t spi2_miso = gpio_num_t::GPIO_NUM_0;
            static constexpr gpio_num_t spi2_clk = gpio_num_t::GPIO_NUM_0;
            
            static constexpr gpio_num_t i2c_sda = gpio_num_t::GPIO_NUM_21;
            static constexpr gpio_num_t i2c_scl = gpio_num_t::GPIO_NUM_22;
#ifdef UFO_I2C_SOFT
            static constexpr gpio_num_t i2c_soft_sda = gpio_num_t::GPIO_NUM_0;
            static constexpr gpio_num_t i2c_soft_scl = gpio_num_t::GPIO_NUM_0;
#endif
            
#if (UFO_SPI_CNT > 1)
            static constexpr gpio_num_t spi3_mosi = gpio_num_t::GPIO_NUM_0;
            static constexpr gpio_num_t spi3_miso = gpio_num_t::GPIO_NUM_0;
            static constexpr gpio_num_t spi3_clk = gpio_num_t::GPIO_NUM_0;
#endif

// uart0 defined by driver
            static constexpr gpio_num_t uart0_rx = gpio_num_t::GPIO_NUM_1;
            static constexpr gpio_num_t uart0_tx = gpio_num_t::GPIO_NUM_3;
            static constexpr uint32_t uart0_br = 115200;
            
            #if (UFO_UART_CNT > 1)
            static constexpr gpio_num_t uart1_rx = gpio_num_t::GPIO_NUM_0;
            static constexpr gpio_num_t uart1_tx = gpio_num_t::GPIO_NUM_0;
            static constexpr uint32_t uart1_br = 9600;
#endif
#if (UFO_UART_CNT > 2)
            static constexpr gpio_num_t uart2_rx = gpio_num_t::GPIO_NUM_0;
            static constexpr gpio_num_t uart2_tx = gpio_num_t::GPIO_NUM_0;
#endif

        public:
            using i2c_t = ufo::drv::UFO_I2C_Driver;
            using spi_t = ufo::drv::uspi_t;
            using uart_t = ufo::drv::UFO_Uart;
            using wf_t = ufo::drv::wf_t;

        public:
            drv_t() {}
            ~drv_t() {}

            std::unique_ptr<i2c_t> _i2c;
#ifdef UFO_I2C_SOFT
            std::unique_ptr<i2c_t> _i2cSoft;
#endif
            std::unique_ptr<spi_t> _spi2;
#if (UFO_SPI_CNT > 1)
            std::unique_ptr<spi_t> _spi3;
#endif

            std::unique_ptr<uart_t> _uart0;
#if (UFO_UART_CNT > 1)
            std::unique_ptr<uart_t> _uart1;
#endif
#if (UFO_UART_CNT > 2)
            std::unique_ptr<uart_t> _uart2;
#endif
#ifdef UFO_WIFI
            wf_t _wifi;
#endif
        };
    } // namespace drv

} // namespace ufo
