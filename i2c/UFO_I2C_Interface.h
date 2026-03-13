#pragma once 

#include "UFO_I2C_driver.h"

#define UFO_I2C_BUFFER_SIZE ((int32_t)32)

namespace ufo
{
    enum UFO_BitOrder : uint8_t{
        BIT_ORDER_LSB,
        BIT_ORDER_MSB,
    };

    class UFO_I2C_Interface
    {
    protected:
        using driver_t = drv::UFO_I2C_Driver;
    
        driver_t *_driver = nullptr;
        uint8_t _addr = 0;

    public:
        UFO_I2C_Interface() {}
        // UFO_I2C_Interface(uint8_t addr) : _addr(addr) {}
        ~UFO_I2C_Interface() {}

        esp_err_t Init(uint8_t addr, driver_t *driver)
        {
            _driver = driver;
            _addr = addr;
            esp_err_t err = ESP_OK;
            
            if (!_driver->Initialized())
            {
                __global_error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::i2c_null_operations, "lnot inited")));
                err = ESP_FAIL;
            }
            return err;
        }

        void SetAddress(uint8_t addr)
        {
            _addr = addr;
        }
        int16_t Read16_Signed_LittleEndian(uint8_t reg)
        {
            return static_cast<int16_t>(Read16_LittleEndian(reg));
        }

        uint16_t Read16_LittleEndian(uint8_t reg)
        {
            uint16_t temp = Read16(reg);
            return (temp >> 8) | (temp << 8);
        }

        uint32_t Read24(uint8_t reg)
        {
            uint8_t buf[3];
            buf[0] = reg;
            _driver->WriteRead(_addr, buf, 1, buf, 3);
            return uint32_t(buf[0]) << 16 | uint32_t(buf[1]) << 8 | uint32_t(buf[2]);
        }

        uint16_t Read16(uint8_t reg)
        {
            uint8_t buf[2];
            buf[0] = reg;
            _driver->WriteRead(_addr, buf, 1, buf, 2);
            return uint16_t(buf[0]) << 8 | uint16_t(buf[1]);
        }

        uint16_t Read16()
        {
            uint8_t buf[2] = {};
            _driver->Read(_addr, buf, 2);
            return uint16_t(buf[0]) | uint16_t(buf[1]) << 8;
        }

        uint8_t Read8(uint8_t reg)
        {
            uint8_t buf[] = {reg};
            _driver->WriteRead(_addr, buf, 1, buf, 1);
            return buf[0];
        }

        esp_err_t Read(uint8_t reg, uint8_t *rBuf, uint8_t size)
        {
            uint8_t buf[1] = {reg};
            esp_err_t ret = _driver->WriteRead(_addr, buf, 1, rBuf, size);
            return ret;
        }

        esp_err_t Write8(uint8_t reg, uint8_t val)
        {
            uint8_t buf[] = {reg, val};
            return _driver->Write(_addr, buf, 2);
        }
        esp_err_t Write16(uint8_t reg, uint16_t val)
        {
            uint8_t
                val1 = static_cast<uint8_t>((val >> 8) & 0xff),
                val2 = static_cast<uint8_t>(val & 0xff);
            uint8_t buf[] = {reg, val1, val2};
            return _driver->Write(_addr, buf, 3);
        }

        esp_err_t Write16_noreg(uint16_t val){
            uint8_t buf[2] = {};
            buf[0] = (val & 0xFF);
            buf[1] = (val >> 8) & 255;
            return _driver->Write(_addr, buf, 2);
        }

        esp_err_t Write(uint8_t reg, uint32_t val, uint32_t numbytes, UFO_BitOrder order)
        {
            uint8_t buf[5] = {};

            if (numbytes > 4)
            {
                return ESP_FAIL;
            }
            buf[0] = reg;
            for (int i = 0; i < numbytes; i++)
            {
                if (order == BIT_ORDER_LSB)
                {
                    buf[i + 1] = val & 0xFF;
                }
                else
                {
                    buf[numbytes - i] = val & 0xFF;
                }
                val >>= 8;
            }

            return _driver->Write(_addr, buf, numbytes + 1);
        }
        //     esp_err_t Write16(uint8_t reg, uint16_t val)
        // {
        //     // uint8_t buf[] = {reg, val};
        //     // return _driver->Write(_addr, buf, 2);
        //             uint8_t buf[] = {reg, val};
        //     return _driver->Write(_addr, buf, 3);
        // }
    };

} // namespace ufo




