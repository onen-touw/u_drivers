#pragma once 

#include "u_sys/config.h"
#include "u_sys/mutex.h"
#include "u_sys/error.h"
#include "u_sys/utils.h"

#include "driver/uart.h"
#include "hal/uart_ll.h"
#include "soc/uart_pins.h"

// todo pins
// #define U0TXD_GPIO_NUM  (1)
// #define U0RXD_GPIO_NUM  (3)
// #define U0CTS_GPIO_NUM  (19)
// #define U0RTS_GPIO_NUM  (22)

// #define U1TXD_GPIO_NUM  (10)
// #define U1RXD_GPIO_NUM  (9)
// #define U1CTS_GPIO_NUM  (6)
// #define U1RTS_GPIO_NUM  (11)

// #define U2TXD_GPIO_NUM  (17)
// #define U2RXD_GPIO_NUM  (16)
// #define U2CTS_GPIO_NUM  (8)
// #define U2RTS_GPIO_NUM  (7)
struct UFO_UartPins
{
    gpio_num_t _tx = gpio_num_t::GPIO_NUM_NC;
    gpio_num_t _rx = gpio_num_t::GPIO_NUM_NC;
};
#define UFO_UART_RX_BUF_SIZE (256*2)
// #define UFO_UART_EVENTS


namespace ufo
{
    namespace drv
    {
        class UFO_Uart
        {
        public:
            using unum_t = uart_port_t;
        private:
            unum_t _unum = unum_t::UART_NUM_MAX;
            UFO_UartPins _pins;
            // mutex_t _lock;
            dev_status_t _dstatus = dev_status_t::undef;

            bool _inited = false;
            uint32_t _timeoutMs = 100; // base
            uint8_t _timeoutRx = 2;    // base
            uint8_t _txFiFoFull = 120; // base

        public:
            UFO_Uart(const UFO_Uart &) = delete;
            UFO_Uart &operator=(const UFO_Uart &) = delete;

            UFO_Uart()
            {
                // lock_guard<mutex_t>_l(_lock);

            }

            ~UFO_Uart()
            {
                // lock_guard<mutex_t>_l(_lock);
                if (_inited)
                {
                    Deinit();
                }
            }

            dev_status_t get_status() const { return _dstatus; }
            void forse_status() { _dstatus = dev_status_t::ok; }

        
            esp_err_t init(unum_t unum, gpio_num_t rx, gpio_num_t tx)
            {
                
                esp_err_t err = ESP_OK;
                // lock_guard<mutex_t>_l(_lock);
                if (unum == unum_t::UART_NUM_MAX)
                {
                    return ESP_FAIL;
                }
                _unum = unum;
                if (_unum == unum_t::UART_NUM_MAX)
                {
                    return ESP_FAIL;
                }
                if (_inited)
                {
                    return ESP_FAIL;
                }
                ufo::Error_t &_error = ufo::Error_t::GetInstance();

                if (uart_is_driver_installed(_unum))
                {
                    _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::urt_drv_install, "drv was installed")));

                    return ESP_FAIL;
                }
                // printf("starting uart install");
                // printf("|\t number: %u\n", _unum); 

                err = uart_driver_install(_unum, UFO_UART_RX_BUF_SIZE, 0, 0, nullptr, 0);
                // err = uart_driver_install(_unum, UFO_UART_RX_BUF_SIZE * 2, 0, 20, &_minimal._eventQueue, 0);
                if (err != ESP_OK)
                {
                    _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::urt_drv_install, "error")));
                return err; 
                }

                uart_config_t ucfg = {};
                ucfg.baud_rate = 115200,
                ucfg.data_bits = UART_DATA_8_BITS,
                ucfg.parity = UART_PARITY_DISABLE,
                ucfg.stop_bits = UART_STOP_BITS_1,
                ucfg.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                ucfg.source_clk = UART_SCLK_APB; // ESP32, ESP32S2

                err = uart_param_config(_unum, &ucfg);
                if (err != ESP_OK)
                {
                    _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::urt_drv_cfg, "cfg error")));
                    return err;
                }
                if (unum != unum_t::UART_NUM_0)
                {
                    err = uart_set_pin(_unum, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
                }
                else {
                    err = uart_set_pin(_unum, 1, 3, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
                }
                if (err != ESP_OK)
                {
                    _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::urt_drv_cfg, "pin error")));
                    return err;
                }

                err = uart_set_rx_timeout(_unum, _timeoutRx);
                if (err != ESP_OK)
                {
                    _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::urt_drv_tmout, "timeout error")));
                    return err;
                }

                err = uart_set_rx_full_threshold(_unum, _txFiFoFull);
                if (err != ESP_OK)
                {
                    _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::urt_drv_tmout, "rx_full_threshold error")));
                    return err;
                }
#ifdef UFO_UART_EVENTS
// xTaskCreatePinnedToCore( _EventTask, "uartETask", 2048, this, 5, &_minimal._task, 0); //for what???
#endif
                _inited = true;
                _dstatus = dev_status_t::ok;
                return err;
            }


            size_t Send(char *data)
            {
                size_t l = 0;
                size_t sz = strlen(data);
                if (!sz)
                {
                    return 0;
                }
                // lock_guard<mutex_t>_l(_lock);
                
                // some checks
                l = uart_write_bytes(_unum, data, sz);

                return l;
            }

            esp_err_t SetBaudRate(uint32_t br)
            {
                esp_err_t err = ESP_OK;

                // lock_guard<mutex_t>_l(_lock);

                if (!_inited)
                {
                    err = ESP_FAIL;
                    return err;
                }
                err = uart_set_baudrate(_unum, br);
                if (err)
                {
                    _dstatus = dev_status_t::warning;
                }
                
                return err;
            }

            uint32_t GetBaudRate() const {
                uint32_t br = 0;
                uart_get_baudrate(_unum, &br);
                return br;
            }

            size_t SendWithBreak(const char *data)
            {
                size_t l = 0;
                size_t sz = strlen(data);
                if (!sz)
                {
                    return 0;
                }
                
                // some checks
                l = uart_write_bytes_with_break(_unum, data, sz, 1);
                return l;
            }
            size_t SendMsg(const char *data)
            {
                size_t l = 0;
                size_t sz = strlen(data);
                if (!sz)
                {
                    return 0;
                }
                // lock_guard<mutex_t>_l(_lock);
                
                l = uart_write_bytes(_unum, data, sz);
                uart_wait_tx_done(_unum, (TickType_t)100);
                
                return l;
            }

            size_t SendMsg(const char *data, uint32_t len)
            {
                if (!len)
                {
                    return 0;
                }
                // lock_guard<mutex_t>_l(_lock);
                
                size_t l = uart_write_bytes(_unum, data, len);
                uart_wait_tx_done(_unum, (TickType_t)100);
                
                return l;
            }


            // The write-function does not guarantee that this msg (data) will be
            // send independently on other trash(other data) in txFiFo buffer
            size_t Write(const char *data)
            {
                size_t l = 0;
                size_t sz = strlen(data);
                if (!sz)
                {
                    return 0;
                }
                // lock_guard<mutex_t>_l(_lock);
                
                l = uart_write_bytes(_unum, data, sz);
                return l;
            }

            size_t Send(char *data, size_t sz)
            {
                if (!sz)
                {
                    return 0;
                }
                uint16_t l = 0;
                
                // lock_guard<mutex_t>_l(_lock);
                
                l = uart_write_bytes(_unum, data, sz);
                return l;
            }

            size_t Available()
            {
                size_t av = 0;
                // lock_guard<mutex_t>_l(_lock);
                uart_get_buffered_data_len(_unum, &av);
                if (av == _txFiFoFull)
                {
                    av = 0;
                }
                
                return av;
            }

            size_t Read(char *buff, size_t sz)
            {
                // lock_guard<mutex_t>_l(_lock);
                
                size_t w = 0;
                w = uart_read_bytes(_unum, buff, sz, pdMS_TO_TICKS(_timeoutMs));

                return w;
            }

            size_t Read(char *buff)
            {
                // lock_guard<mutex_t>_l(_lock);

                size_t w = 0, av = 0;

                uart_get_buffered_data_len(_unum, &av);
                
                w = uart_read_bytes(_unum, buff, av, pdMS_TO_TICKS(_timeoutMs));

                return w;
            }

            void Flush()
            {
                // lock_guard<mutex_t>_l(_lock);

                while (!uart_ll_is_tx_idle(UART_LL_GET_HW(_unum)))
                {
                }
                uart_flush_input(_unum);
            }

            void Deinit()
            {
                // lock_guard<mutex_t>_l(_lock);
                if (_inited)
                {
                    Flush();
                    uart_driver_delete(_unum);
                }
                _dstatus = dev_status_t::off;
            }

        };

    } // namespace drv
} // namespace ufo



