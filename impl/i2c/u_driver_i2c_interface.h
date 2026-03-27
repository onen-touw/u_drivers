#pragma once
#include "u_driver_i2c.h"

namespace __u_drivers
{

    // all method are protected for inheritance
    class u_driver_i2c_interface_t
    {
    public:
        using drv_t = u_driver_i2c_t;
        using i2c_port_t = driver_i2c_port_e;
    private:
        uint8_t _addr = 0;
        drv_t& _driver;
    public:

        // WARNING: destructor do not deinit i2c driver!!!
        u_driver_i2c_interface_t(uint8_t addr, i2c_port_t port = i2c_port_t::hardware) 
            : _addr(addr), _driver(__driver_i2c__instance[static_cast<size_t>(port)]) 
        {

            // get parameters from config. This parameters can be setted 
            // directly from code (bad way) or configurated via console
            // if driver is initialized when parameters are updated requare driver restart  '

            const auto& cfg = __cfg_i2c[static_cast<size_t>(port)];
            _driver.driver_init(port, cfg.i2c_sda, cfg.i2c_scl, cfg.frequency);
        }

        void set_address(uint8_t addr)
        {
            _addr = addr;
        }

        /// @brief i2c_detecter.
        /// @param addbuf external buffer for found addresses
        /// @param buf_sz size of buffer
        /// @return size of found devices
        size_t i2c_detecter(uint8_t* addbuf, size_t buf_sz)
        {
            uint8_t sz = 0;
            uint8_t bi = 0;
            for (uint8_t address = 1; address < 127; ++address)
            {
                if (!_driver.ZeroWrite(address))
                {
                    if (bi < buf_sz)
                    {
                        addbuf[bi] = address; 
                        ++sz;
                        ++bi;
                    }
                }
            }
            return sz;
        }

    // ----------------------------- reading -------------------------------
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
            uint8_t buf[3] = {};
            buf[0] = reg;
            _driver.WriteRead(_addr, buf, 1, buf, 3);
            return uint32_t(buf[0]) << 16 | uint32_t(buf[1]) << 8 | uint32_t(buf[2]);
        }

        uint16_t Read16(uint8_t reg)
        {
            uint8_t buf[2] = {};
            buf[0] = reg;
            _driver.WriteRead(_addr, buf, 1, buf, 2);
            return uint16_t(buf[0]) << 8 | uint16_t(buf[1]);
        }

        // uint16_t Read16()
        // {
        //     uint8_t buf[2] = {};
        //     _driver->Read(_addr, buf, 2);
        //     return uint16_t(buf[0]) | uint16_t(buf[1]) << 8;
        // }

        uint8_t Read8(uint8_t reg)
        {
            uint8_t buf[] = {reg};
            _driver.WriteRead(_addr, buf, 1, buf, 1);
            return buf[0];
        }

        void Read(uint8_t reg, uint8_t *rBuf, uint8_t size)
        {
            uint8_t buf[1] = {reg};
            _driver.WriteRead(_addr, buf, 1, rBuf, size);
        }
    // ----------------------------- reading-end -------------------------------

    // ----------------------------- writing -------------------------------
        void Write8(uint8_t reg, uint8_t val)
        {
            uint8_t buf[] = {reg, val};
            _driver.Write(_addr, buf, 2);
        }

        void Write16(uint8_t reg, uint16_t val)
        {
            uint8_t
                val1 = static_cast<uint8_t>((val >> 8) & 0xff),
                val2 = static_cast<uint8_t>(val & 0xff);
            uint8_t buf[] = {reg, val1, val2};
            _driver.Write(_addr, buf, 3);
        }

        void Write16_noreg(uint16_t val){
            uint8_t buf[2] = {};
            buf[0] = (val & 0xFF);
            buf[1] = (val >> 8) & 255;
            _driver.Write(_addr, buf, 2);
        }

        void Write(uint8_t reg, uint32_t val, uint32_t numbytes, bit_order_e order)
        {
            uint8_t buf[5] = {};

            if (numbytes > 4)
            {
                return;
            }
            buf[0] = reg;
            for (int i = 0; i < numbytes; i++)
            {
                if (order == bit_order_e::LSB)
                {
                    buf[i + 1] = val & 0xFF;
                }
                else
                {
                    buf[numbytes - i] = val & 0xFF;
                }
                val >>= 8;
            }

            _driver.Write(_addr, buf, numbytes + 1);
        }
    // ----------------------------- writing-end -------------------------------
    };

} // __u_drivers
