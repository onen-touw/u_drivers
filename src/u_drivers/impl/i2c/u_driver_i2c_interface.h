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
        bool ifinit = false;
    public:

        /// WARNING: destructor do not deinit i2c driver!!!
        ///
        u_driver_i2c_interface_t(uint8_t addr, i2c_port_t port, const driver_i2c_cnf_t& cfg) 
            : _addr(addr), _driver(__driver_i2c__instance[static_cast<size_t>(port)]) 
        {
            __info_i2c[static_cast<size_t>(port)].set_cfg(cfg);
            auto e = _driver.driver_init(port);

            if (e != ESP_OK)
            {
                return;
            }

            // we should inc user here because one driver provide many interfaces for users
            __info_i2c[static_cast<size_t>(_driver.get_port())].inc_user();
            ifinit = true;
        }

        /// WARNING: destructor do not deinit i2c driver!!!
        /// Constructor without cfg
        u_driver_i2c_interface_t(uint8_t addr, i2c_port_t port = i2c_port_t::hardware) 
            : _addr(addr), _driver(__driver_i2c__instance[static_cast<size_t>(port)]) 
        {

            // get parameters from config. This parameters can be setted 
            // directly from code (bad way) or configurated via console and files
            // if driver is initialized when parameters are updated requare driver restart (or esp reboot if it saved in cfg-file)

            // const auto cfg = __info_i2c[static_cast<size_t>(port)];
            auto e = _driver.driver_init(port);

            if (e != ESP_OK)
            {
                return;
            }

            // we should inc user here because one driver provide many interfaces for users
            __info_i2c[static_cast<size_t>(_driver.get_port())].inc_user();
            ifinit = true;
        }

        ~u_driver_i2c_interface_t()
        {
            //... and we should dec user here...(see constructor)
            // but decreasing can be only if constructor did correct initialization...
            if (!ifinit)
            {
                return;
            }
            
            __info_i2c[static_cast<size_t>(_driver.get_port())].dec_user();
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
        // old Read16_Signed_LittleEndian
        int16_t Read16_S_LE(uint8_t reg)
        {
            return static_cast<int16_t>(Read16_LE(reg));
        }

        // old Read16_LittleEndian
        uint16_t Read16_LE(uint8_t reg)
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
            _driver.WriteRead(_addr, &reg, 1, buf, 2);
            return uint16_t(buf[0]) << 8 | uint16_t(buf[1]);
        }

        uint16_t Read16_Bit(uint8_t reg, uint8_t num_bit)
        {
            if (num_bit < 16)
            {
                uint16_t w = Read16(reg);
                return w & (1 << num_bit);
            }
            return 0;
        }

        uint8_t Read8_Bit(uint8_t reg, uint8_t num_bit)
        {
            if (num_bit < 8)
            {
                uint8_t w = Read8(reg);
                return w & (1 << num_bit);
            }
            return 0;
        }

        uint16_t Read16_Bits(uint8_t reg, uint8_t from_bit, uint8_t cnt_bits)
        {
            uint16_t w = Read16(reg);

            if (w && cnt_bits && from_bit < 16 && (from_bit >= cnt_bits - 1)) {
                uint16_t mask = ((1 << cnt_bits) - 1) << (from_bit - cnt_bits + 1);
                w &= mask;
                w >>= (from_bit - cnt_bits + 1);
                return w;
            }
            return 0;
        }

        /// @brief 
        /// @param reg 
        /// @param from_bit MSB of readed 16 bit value e.g.: data = 0b00001110, from_bit = 4, cnt_bits = 3 => res = 0b110
        /// @param cnt_bits count of executed bits
        /// @return executed value 
        uint16_t Read8_Bits(uint8_t reg, uint8_t from_bit, uint8_t cnt_bits)
        {
            uint8_t w = Read8(reg);

            if (w && cnt_bits && from_bit < 8 && (from_bit >= cnt_bits - 1)) {
                uint8_t mask = ((1 << cnt_bits) - 1) << (from_bit - cnt_bits + 1);
                w &= mask;
                w >>= (from_bit - cnt_bits + 1);
                return w;
            }
            return 0;
        }
        
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

        drv_t &get_native() { return _driver; }

        // ----------------------------- writing -------------------------------
        void Write8(uint8_t reg, uint8_t val)
        {
            uint8_t buf[] = {reg, val};
            _driver.Write(_addr, buf, 2);
        }

        void Write16(uint8_t reg, uint16_t val)
        {
            uint8_t buf[] = 
                {
                    reg, 
                    static_cast<uint8_t>((val >> 8) & 0xff), 
                    static_cast<uint8_t>(val & 0xff)
                };
            _driver.Write(_addr, buf, 3);
        }

        /// @brief 
        /// @param reg 
        /// @param bit_val 0 or 1 
        /// @param bit_pos 
        void Write8_Bit(uint8_t reg, bool bit_val, uint8_t bit_pos)
        {
            uint8_t b = Read8(reg);
            b = (static_cast<uint8_t>(bit_val) != 0) ? (b | (1 << bit_pos)) : (b & ~(1 << bit_pos));
            Write8(reg, b);
        }

        /// @brief 
        /// @param reg 
        /// @param bit_val 0 or 1 
        /// @param bit_pos 
        void Write16_Bit(uint8_t reg, bool bit_val, uint8_t bit_pos)
        {
            uint16_t w = Read16(reg);
            if (w && bit_pos < 16)
            {
                w = (static_cast<uint16_t>(bit_val) != 0) ? (w | (1 << bit_pos)) : (w & ~(1 << bit_pos));
                Write16(reg, w);
            }
        }

        void Write8_Bits(uint8_t reg, uint8_t val, uint8_t from_bit, uint8_t cnt_bits)
        {
            uint8_t b = Read8(reg);
            if (b && cnt_bits && from_bit < 8 && from_bit >= cnt_bits - 1)
            {
                uint8_t mask = ((1 << cnt_bits) - 1) << (from_bit - cnt_bits + 1);
                val <<= (from_bit - cnt_bits + 1); // shift data into correct position
                val &= mask; // zero all non-important bits in data
                b &= ~(mask); // zero all important bits in existing byte
                b |= val; // combine data with existing byte
                Write8(reg, b);
            }
        }

        void Write16_Bits(uint8_t reg, uint16_t val, uint8_t from_bit, uint8_t cnt_bits)
        {
            uint16_t w = Read16(reg);

            if (w && cnt_bits && from_bit < 16 && from_bit >= cnt_bits - 1)
            {
                uint16_t mask = ((1 << cnt_bits) - 1) << (from_bit - cnt_bits + 1);
                val <<= (from_bit - cnt_bits + 1);
                val &= mask;
                w &= ~(mask);
                w |= val;
                Write16(reg, w);
            }
        }
        

        void Write16_LE(uint8_t reg, uint16_t val) 
        {
            uint8_t buf[] = 
                {
                    reg, 
                    static_cast<uint8_t>(val & 0xff),
                    static_cast<uint8_t>((val >> 8) & 0xff) 
                };
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

        /// you should set reg in frist item of array!!!
        void Write_any(uint8_t* buf, size_t numbytes)
        {
            if (buf)
            {
                _driver.Write(_addr, buf, numbytes);
            }
        }

    // ----------------------------- writing-end -------------------------------
    };

} // __u_drivers
