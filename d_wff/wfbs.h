// wifi base

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "soc/soc_caps.h"

#include "esp_wifi_crypto_types.h"
#include <esp_err.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_mac.h>
#include <esp_netif.h>
#if SOC_WIFI_SUPPORTED
#include <esp_phy.h>
#endif
#include "lwip/ip_addr.h"
#include "lwip/opt.h"
#include "lwip/err.h"
#include "lwip/dns.h"
#include "lwip/netif.h"
#include <esp_smartconfig.h>

#include "dhcpserver/dhcpserver.h"

#include "u_sys/btflg.h"
#include "u_sys/error.h"
#include "u_sys/mutex.h"
#include "u_sys/ipt.h"

#define _byte_swap32(num) (((num >> 24) & 0xff) | ((num << 8) & 0xff0000) | ((num >> 8) & 0xff00) | ((num << 24) & 0xff000000))

namespace ufo
{

    namespace wf_details
    {
        typedef enum {
            WPA2_AUTH_TLS = 0,
            WPA2_AUTH_PEAP = 1,
            WPA2_AUTH_TTLS = 2
          } wpa2_auth_method_t;
          

        enum class wifi_power_t
        {
            WIFI_POWER_21dBm = 84,     // 21dBm
            WIFI_POWER_20_5dBm = 82,   // 20.5dBm
            WIFI_POWER_20dBm = 80,     // 20dBm
            WIFI_POWER_19_5dBm = 78,   // 19.5dBm
            WIFI_POWER_19dBm = 76,     // 19dBm
            WIFI_POWER_18_5dBm = 74,   // 18.5dBm
            WIFI_POWER_17dBm = 68,     // 17dBm
            WIFI_POWER_15dBm = 60,     // 15dBm
            WIFI_POWER_13dBm = 52,     // 13dBm
            WIFI_POWER_11dBm = 44,     // 11dBm
            WIFI_POWER_8_5dBm = 34,    // 8.5dBm
            WIFI_POWER_7dBm = 28,      // 7dBm
            WIFI_POWER_5dBm = 20,      // 5dBm
            WIFI_POWER_2dBm = 8,       // 2dBm
            WIFI_POWER_MINUS_1dBm = -4 // -1dBm
        };

#ifdef UFO_WIFI_ANTENA_CFG
        enum class wifi_rx_ant_t
        {
            WIFI_RX_ANT0 = 0,
            WIFI_RX_ANT1,
            WIFI_RX_ANT_AUTO
        };

        enum class wifi_tx_ant_t
        {
            WIFI_TX_ANT0 = 0,
            WIFI_TX_ANT1,
            WIFI_TX_ANT_AUTO
        };
#endif

        uint8_t calc_subnet_CIDR(ip_t subnetMask)
        {
            uint8_t CIDR = 0;

            for (uint8_t i = 0; i < 4; i++)
            {
                if (subnetMask[i] == 0x80)
                { // 128
                    CIDR += 1;
                }
                else if (subnetMask[i] == 0xC0)
                { // 192
                    CIDR += 2;
                }
                else if (subnetMask[i] == 0xE0)
                { // 224
                    CIDR += 3;
                }
                else if (subnetMask[i] == 0xF0)
                { // 242
                    CIDR += 4;
                }
                else if (subnetMask[i] == 0xF8)
                { // 248
                    CIDR += 5;
                }
                else if (subnetMask[i] == 0xFC)
                { // 252
                    CIDR += 6;
                }
                else if (subnetMask[i] == 0xFE)
                { // 254
                    CIDR += 7;
                }
                else if (subnetMask[i] == 0xFF)
                { // 255
                    CIDR += 8;
                }
            }
            return CIDR;
        }

        ip_t calc_networkID(ip_t ip, ip_t subnet) 
        {
            uint8_t a1 = subnet[0] & ip[0];
            uint8_t a2 = subnet[1] & ip[1];
            uint8_t a3 = subnet[2] & ip[2];
            uint8_t a4 = subnet[3] & ip[3];

            return ip_t(a4, a3, a2, a1);
        }

        ip_t calc_broadcastID(ip_t ip, ip_t subnet) 
        {
            uint8_t a1 = ~subnet[0] | ip[0];
            uint8_t a2 = ~subnet[1] | ip[1];
            uint8_t a3 = ~subnet[2] | ip[2];
            uint8_t a4 = ~subnet[3] | ip[3];

            return ip_t(a4, a3, a2, a1);
        }

        class wfbs_t
        {
        public:
            static constexpr uint32_t bit_initial = 0;
            static constexpr uint32_t bit_sta_end = 1;
            static constexpr uint32_t bit_sta_start = 1 << 1;
            static constexpr uint32_t bit_ap_start = 1 << 2;

            enum class flag_pos_t
            {
                driver_inited = 0,
                main_event_inited,
                nett_event_inited,      // work for both sta and ap but definition depend on if
                driver_started,
                enabled,                // works for both sta and ap
                static_buf,
                static_ip,              
                long_rng,               // fork for both sta and ap
                persistent,
                sta_conn,       // work only for sta  //if 1 sta is connected to ap otherwise 0
            };

        public:
            using callback_t = esp_event_handler_t;
            using arg_t = void *;

        public:
            static constexpr uint32_t if_max_cnt = 2;

        protected:
            static esp_netif_t *_netifs[if_max_cnt];
            static ufo::bit_flag_t<uint32_t, mutex_t> _flags;
            static EventGroupHandle_t _event_bit;

            // static wifi_mode_t _forceSleepLastMode = WIFI_MODE_NULL;
            // static wifi_ps_type_t _sleepEnabled = WIFI_PS_MIN_MODEM;

        protected:
            wfbs_t()
            {
                _flags.set(flag_pos_t::persistent); // default settings
            }
            ~wfbs_t()
            {
                deinit_driver();
            }

            bool check_ssid_pwd(const char* s, const char* p){
                if (!s)
                {
                    printf("SSID missing!\n");
                    return false;
                }

                if (s[0] == 0)
                {
                    printf("SSID missing!\n");
                    return false;
                }
                uint16_t plen = strlen(p);
                uint16_t slen = strlen(s);
                
                if (slen > 32)
                {
                    printf("SSID too long!\n");
                    return false;
                }

                if (p)
                {
                    if (p[0] == 0)
                    {
                        return false;
                    }

                    if (plen > 0 && plen < 8)
                    {
                        printf("passphrase too short!\n");
                        return false;
                    }
                    if (plen > 32)
                    {
                        printf("passphrase too long!\n");
                        return false;
                    }
                }

                return true;
            }
                
            bool start()
            {
                if (_flags.get(flag_pos_t::driver_started))
                {
                    // printf("start:: drv was started\n");
                    return true;
                }
                // printf("call start\n");
                esp_err_t e = esp_wifi_start();
                if (e != ESP_OK)
                {
                    // printf("!esp_wifi_start\n");
                    ufo::Error_t &_error = ufo::Error_t::GetInstance();
                    _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                    return false;
                }
                _flags.set(flag_pos_t::driver_started);
                return true;
            }
            void stop()
            {
                if (!_flags.get(flag_pos_t::driver_started))
                {
                    return;
                }
                /* esp_err_t err =  */
                esp_wifi_stop();
                // if (err != ESP_OK)
                // {
                //     // printf("!esp_wifi_stop");
                // }
                _flags.unset(flag_pos_t::driver_started);
            }

            bool enabled() const { return _flags.get(flag_pos_t::enabled); }

            wifi_mode_t get_mode() const
            {
                if (!_flags.get(flag_pos_t::driver_started, flag_pos_t::driver_inited))
                {
                    return WIFI_MODE_NULL;
                }
                wifi_mode_t mode = {};
                if (esp_wifi_get_mode(&mode) != ESP_OK)
                {
                    // log_w("WiFi not started");
                    return WIFI_MODE_NULL;
                }
                return mode;
            }

            bool init_driver()
            {
                esp_err_t err = ESP_OK;

                ufo::Error_t &_error = ufo::Error_t::GetInstance();

                if (!_flags.get(flag_pos_t::driver_inited))
                {
                    _event_bit = xEventGroupCreate();
                    if (!_event_bit)
                    {
                        // log_e("Network Event Group Create Failed!");
                        _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                        return false;
                    }

                    xEventGroupSetBits(_event_bit, bit_initial);

                    err = esp_netif_init();
                    if (err != ESP_OK)
                    {
                        // log_e("esp_netif_init failed!");
                        _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                        return false;
                    }

                    err = esp_event_loop_create_default();
                    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
                    {
                        // log_e("esp_event_loop_create_default failed!");
                        _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                        return false;
                    }

                    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

                    if (!_flags.get(flag_pos_t::long_rng))
                    {
                        cfg.static_tx_buf_num = 0;
                        cfg.dynamic_tx_buf_num = 32;
                        cfg.tx_buf_type = 1;
                        cfg.cache_tx_buf_num = 4; // can't be zero!
                        cfg.static_rx_buf_num = 4;
                        cfg.dynamic_rx_buf_num = 32;
                    }

                    err = esp_wifi_init(&cfg);
                    if (err)
                    {
                        // log_e("esp_wifi_init %d", err);
                        _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                        return false;
                    }

                    if (!_flags.get(flag_pos_t::persistent))
                    {
                        err = esp_wifi_set_storage(WIFI_STORAGE_RAM);
                        if (err != ESP_OK)
                        {
                            _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                            return false;
                        }
                    }

                    // init_events();
                    if (_netifs[ESP_IF_WIFI_AP] == NULL)
                    {
                        _netifs[ESP_IF_WIFI_AP] = esp_netif_create_default_wifi_ap();
                        if (!_netifs[ESP_IF_WIFI_AP])
                        {
                            _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                            // warning
                        }
                    }
                    if (_netifs[ESP_IF_WIFI_STA] == NULL)
                    {
                        _netifs[ESP_IF_WIFI_STA] = esp_netif_create_default_wifi_sta();
                        if (!_netifs[ESP_IF_WIFI_AP])
                        {
                            _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                            // warning
                        }
                    }
                    _flags.set(flag_pos_t::driver_inited);
                }
                // set bit (low level init done perfectly)
                return true;
            }

            bool deinit_driver()
            {
                if (_flags.get(flag_pos_t::driver_inited))
                {
                    // deinit_events();

                    if (_netifs[ESP_IF_WIFI_AP])
                    {
                        esp_netif_destroy_default_wifi(_netifs[ESP_IF_WIFI_AP]);
                        _netifs[ESP_IF_WIFI_AP] = nullptr;
                    }
                    if (_netifs[ESP_IF_WIFI_STA])
                    {
                        esp_netif_destroy_default_wifi(_netifs[ESP_IF_WIFI_STA]);
                        _netifs[ESP_IF_WIFI_STA] = nullptr;
                    }
                    esp_err_t e = esp_wifi_deinit();
                    if (e != ESP_OK)
                    {
                        ufo::Error_t &_error = ufo::Error_t::GetInstance();
                        _error.Push(CriticalError_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                        // critical
                    }
                    esp_event_loop_delete_default();
                    esp_netif_deinit();
                    vEventGroupDelete(_event_bit);

                    _flags.unset(flag_pos_t::driver_inited);
                    // force rst
                    _flags.rst();
                }
                return true;
            }

            bool init_events(callback_t callback, arg_t arg)
            {
                if (_flags.get(flag_pos_t::main_event_inited))
                {
                    return true;
                }

                if (esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, callback, arg, nullptr))
                {
                    // log_e("event_handler_instance_register for WIFI_EVENT Failed!");
                    ufo::Error_t &_error = ufo::Error_t::GetInstance();
                    _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                    return false;
                }
                _flags.set(flag_pos_t::main_event_inited);
                return true;
            }
            bool deinit_events(callback_t callback)
            {
                if (!_flags.get(flag_pos_t::main_event_inited))
                {
                    return true;
                }
                if (esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, callback))
                {
                    // log_e("esp_event_handler_unregister for WIFI_EVENT Failed!");
                    ufo::Error_t &_error = ufo::Error_t::GetInstance();
                    _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                    return false;
                }
                _flags.unset(flag_pos_t::main_event_inited);
                return true;
            }

            bool init_events_net(callback_t callback, arg_t arg)
            {
                if (_flags.get(flag_pos_t::nett_event_inited))
                {
                    return true;
                }
                if (esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, callback, arg, nullptr))
                {
                    ufo::Error_t &_error = ufo::Error_t::GetInstance();
                    _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                    // log_e("event_handler_instance_register for IP_EVENT Failed!");
                    return false;
                }
                _flags.set(flag_pos_t::nett_event_inited);
                return true;
            }
            bool deinit_events_net(callback_t callback)
            {
                if (!_flags.get(flag_pos_t::nett_event_inited))
                {
                    return true;
                }
                if (esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID, callback))
                {
                    // log_e("esp_event_handler_unregister for WIFI_EVENT Failed!");
                    ufo::Error_t &_error = ufo::Error_t::GetInstance();
                    _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                    return false;
                }
                _flags.unset(flag_pos_t::nett_event_inited);
                return true;
            }

            
        protected:
            ip_t _get_ip(esp_interface_t itf) const
            {
                if (!_netifs[itf])
                {
                    return ip_t();
                }
                esp_netif_ip_info_t ip;
                if (esp_netif_get_ip_info(_netifs[itf], &ip) != ESP_OK)
                {
                    // printf("Netif Get IP Failed!\n");
                    return ip_t();
                }
                return ip_t(ip.ip.addr);
            }

            ip_t _get_netmask(esp_interface_t itf) const
            {
                if (!_netifs[itf])
                {
                    return ip_t();
                }
                esp_netif_ip_info_t ip;
                if (esp_netif_get_ip_info(_netifs[itf], &ip) != ESP_OK)
                {
                    // printf("Netif Get IP Failed!\n");
                    return ip_t();
                }
                return ip_t(ip.netmask.addr);
            }
        
            ip_t _get_gateway(esp_interface_t itf) const
            {
                if (!_netifs[itf])
                {
                    return ip_t();
                }
                
                esp_netif_ip_info_t ip;
                if (esp_netif_get_ip_info(_netifs[itf], &ip) != ESP_OK)
                {
                    // printf("Netif Get IP Failed!\n");
                    return ip_t();
                }
                return ip_t(ip.gw.addr);
            }

            ip_t _get_broadcastIP(esp_interface_t itf) const {
                if (!_netifs[itf])
                {
                    return ip_t();
                }
                
                esp_netif_ip_info_t ip;
                if (esp_netif_get_ip_info(_netifs[itf], &ip) != ESP_OK)
                {
                    // printf("Netif Get IP Failed!\n");
                    return ip_t();
                }
                return calc_broadcastID(ip.ip.addr, ip.netmask.addr);
            }
        
            ip_t _get_networkID(esp_interface_t itf) const {
                if (!_netifs[itf])
                {
                    return ip_t();
                }
                
                esp_netif_ip_info_t ip;
                if (esp_netif_get_ip_info(_netifs[itf], &ip) != ESP_OK)
                {
                    // printf("Netif Get IP Failed!\n");
                    return ip_t();
                }
                return calc_networkID(ip.ip.addr, ip.netmask.addr);
            }

            ip_t _get_subnetCIDR(esp_interface_t itf) const {
                if (!_netifs[itf])
                {
                    return ip_t();
                }
                
                esp_netif_ip_info_t ip;
                if (esp_netif_get_ip_info(_netifs[itf], &ip) != ESP_OK)
                {
                    // printf("Netif Get IP Failed!\n");
                    return ip_t();
                }
                return calc_subnet_CIDR( ip.netmask.addr);
            }
            
            void _log_info(esp_interface_t itf) const{
                Trace_t::log("ip_info: ");
                if (!_netifs[itf])
                {
                    Trace_t::log("\terror\n");
                    return;
                }

                wifi_mode_t m = get_mode();
                if (m == wifi_mode_t::WIFI_MODE_NULL)
                {
                    Trace_t::log("null\n");
                    return;
                }
                else if (m == wifi_mode_t::WIFI_MODE_AP)
                {
                    Trace_t::log(" ap\n");
                }
                else
                {
                    Trace_t::log(" sta\n");
                }

                esp_netif_ip_info_t ip = {};
                if (esp_netif_get_ip_info(_netifs[itf], &ip) != ESP_OK)
                {
                    Trace_t::log("\terror\n");
                    return;
                }
                Trace_t::log("ip:\n");
                ip_t(ip.ip.addr).log();
                Trace_t::log("mask:\n");
                ip_t(ip.netmask.addr).log();
                Trace_t::flog("CIDR: %u\n", calc_subnet_CIDR(ip.netmask.addr));
                Trace_t::log("gw:\n");
                ip_t(ip.gw.addr).log();
                Trace_t::log("broadcast:\n");
                calc_broadcastID(ip.ip.addr, ip.netmask.addr).log();
                Trace_t::log("network:\n");
                calc_networkID(ip.ip.addr, ip.netmask.addr).log();
            }

            bool _ip_config(esp_interface_t itf,
                ip_t local_ip,
                ip_t gateway,
                ip_t subnet,
                ip_t dns1,
                ip_t dns2,
                ip_t dns3)
            {
                if (_netifs[ESP_IF_WIFI_STA] == NULL)
                {
                    return false;
                }
                esp_err_t err = ESP_OK;
                esp_netif_ip_info_t info;
                esp_netif_dns_info_t d1 = {};
                esp_netif_dns_info_t d2 = {};
                esp_netif_dns_info_t d3 = {};
                d1.ip.type = IPADDR_TYPE_V4;
                d2.ip.type = IPADDR_TYPE_V4;
                d3.ip.type = IPADDR_TYPE_V4;
    
                if (local_ip.get() != 0)
                {
                    info.ip.addr = local_ip.get();
                    info.gw.addr = gateway.get();
                    info.netmask.addr = subnet.get();
                    d1.ip.u_addr.ip4.addr = dns1.get();
                    d2.ip.u_addr.ip4.addr = dns2.get();
                    d3.ip.u_addr.ip4.addr = dns3.get();
                }
                else
                {
                    info.ip.addr = 0;
                    info.gw.addr = 0;
                    info.netmask.addr = 0;
                    d1.ip.u_addr.ip4.addr = 0;
                    d2.ip.u_addr.ip4.addr = 0;
                    d3.ip.u_addr.ip4.addr = 0;
                }
                ufo::Error_t &_error = ufo::Error_t::GetInstance();
    
                esp_netif_flags_t flags = esp_netif_get_flags(_netifs[itf]);
                if (flags & ESP_NETIF_DHCP_SERVER)
                {
                    // Set DNS Server
                    if (d2.ip.u_addr.ip4.addr != 0)
                    {
                        err = esp_netif_set_dns_info(_netifs[itf], ESP_NETIF_DNS_MAIN, &d2);
                        if (err)
                        {
                            _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_ip, "dns")));
                            // log_e("Netif Set DNS Info Failed! 0x%04x: %s", err, esp_err_to_name(err));
                            return false;
                        }
                    }
    
                    // Stop DHCPS
                    err = esp_netif_dhcps_stop(_netifs[itf]);
                    if (err && err != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED)
                    {
                        _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_ip, "dhcp")));
                            // log_e("DHCPS Stop Failed! 0x%04x: %s", err, esp_err_to_name(err));
                        return false;
                    }
    
                    // Set IPv4, Netmask, Gateway
                    err = esp_netif_set_ip_info(_netifs[itf], &info);
                    if (err)
                    {
                        _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_ip, "ip")));
                            // printf("Netif Set IP Failed! 0x%04x: %s\n", err, esp_err_to_name(err));
                        return false;
                    }
    
                    dhcps_lease_t lease = {};
                    lease.enable = true;
                    uint8_t CIDR = calc_subnet_CIDR(subnet);
                    
                    // printf("static_ip ");
                    // local_ip.log();
                    // printf("gateway ");
                    // gateway.log();
                    // printf("subnet ");
                    // subnet.log();
                    // printf("CIDR %u\n", CIDR);

                    // netmask must have room for at least 12 IP addresses (AP + GW + 10 DHCP Leasing addresses)
                    // netmask also must be limited to the last 8 bits of IPv4, otherwise this function won't work
                    // IDF NETIF checks netmask for the 3rd byte: https://github.com/espressif/esp-idf/blob/master/components/esp_netif/lwip/esp_netif_lwip.c#L1857-L1862
                    if (CIDR > 28 || CIDR < 24)
                    {
                        _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_ip, "badmask")));

                        // printf("Bad netmask. It must be from /24 to /28 (255.255.255. 0<->240)\n");
                        return false; //  ESP_FAIL if initializing failed
                    }
                    // The code below is ready for any netmask, not limited to 255.255.255.0
                    uint32_t netmask = _byte_swap32(info.netmask.addr);
                    uint32_t ap_ipaddr = _byte_swap32(info.ip.addr);
                    uint32_t dhcp_ipaddr = _byte_swap32(static_cast<uint32_t>(dns1));
                    dhcp_ipaddr = dhcp_ipaddr == 0 ? ap_ipaddr + 1 : dhcp_ipaddr;
                    uint32_t leaseStartMax = ~netmask - 10;
                    // there will be 10 addresses for DHCP to lease
                    lease.start_ip.addr = dhcp_ipaddr;
                    lease.end_ip.addr = lease.start_ip.addr + 10;
                    // Check if local_ip is in the same subnet as the dhcp leasing range initial address
                    if ((ap_ipaddr & netmask) != (dhcp_ipaddr & netmask))
                    {
                        ip_t dd(_byte_swap32(dhcp_ipaddr));
                        _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_ip, "bad subnet")));
                        // printf("The AP IP address and the DHCP start address must be in the same subnet:: ");
                        // local_ip.log();
                        // dd.log();
                        return false; //  ESP_FAIL if initializing failed
                    }
                    // prevents DHCP lease range to overflow subnet range
                    if ((dhcp_ipaddr & ~netmask) >= leaseStartMax)
                    {
                        // make first DHCP lease addr stay in the beginning of the netmask range
                        lease.start_ip.addr = (dhcp_ipaddr & netmask) + 1;
                        lease.end_ip.addr = lease.start_ip.addr + 10;
                        _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_ip, "bad subnet")));
                        // printf("DHCP Lease out of range\n");
                        // log_w("DHCP Lease out of range - Changing DHCP leasing start to %s", IPAddress(_byte_swap32(lease.start_ip.addr)).toString().c_str());
                    }
                    // Check if local_ip is within DHCP range
                    if (ap_ipaddr >= lease.start_ip.addr && ap_ipaddr <= lease.end_ip.addr)
                    {
                        // log_e(
                        //     "The AP IP address (%s) can't be within the DHCP range (%s -- %s)", local_ip.toString().c_str(),
                        //     IPAddress(_byte_swap32(lease.start_ip.addr)).toString().c_str(), IPAddress(_byte_swap32(lease.end_ip.addr)).toString().c_str());
                        _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_ip, "bad subnet")));
                        // printf("The AP IP address can't be within the DHCP range\n");
                        return false; //  ESP_FAIL if initializing failed
                    }
                    // Check if gateway is within DHCP range
                    uint32_t gw_ipaddr = _byte_swap32(info.gw.addr);
                    bool gw_in_same_subnet = (gw_ipaddr & netmask) == (ap_ipaddr & netmask);
                    if (gw_in_same_subnet && gw_ipaddr >= lease.start_ip.addr && gw_ipaddr <= lease.end_ip.addr)
                    {
                        // log_e(
                            // "The GatewayP address (%s) can't be within the DHCP range (%s -- %s)", gateway.toString().c_str(),
                            // IPAddress(_byte_swap32(lease.start_ip.addr)).toString().c_str(), IPAddress(_byte_swap32(lease.end_ip.addr)).toString().c_str());
                        // printf("The GatewayP address can't be within the DHCP range\n");
                        _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_ip, "bad subnet")));
    
                        return false; //  ESP_FAIL if initializing failed
                    }
                    // all done, just revert back byte order of DHCP lease range
                    lease.start_ip.addr = _byte_swap32(lease.start_ip.addr);
                    lease.end_ip.addr = _byte_swap32(lease.end_ip.addr);
    
                    // printf ("DHCP Server Range: ");
                    // ip_t(lease.start_ip.addr).log();
                    // printf ("to: ");
                    // ip_t(lease.end_ip.addr).log();
    
                    // log_v("DHCP Server Range: %s to %s", IPAddress(lease.start_ip.addr).toString().c_str(), IPAddress(lease.end_ip.addr).toString().c_str());
                    err = esp_netif_dhcps_option(_netifs[itf], ESP_NETIF_OP_SET, ESP_NETIF_REQUESTED_IP_ADDRESS, (void *)&lease, sizeof(dhcps_lease_t));
                    if (err)
                    {
                        _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_ip, "bad subnet")));
                        // printf("DHCPS Set Lease Failed! 0x%04x: %s\n", err, esp_err_to_name(err));
                        return false;
                    }
    
                    // Offer DNS to DHCP clients
                    if (d2.ip.u_addr.ip4.addr != 0)
                    {
                        dhcps_offer_t dhcps_dns_value = dhcps_offer_option::OFFER_DNS;
                        err = esp_netif_dhcps_option(_netifs[itf], ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &dhcps_dns_value, sizeof(dhcps_dns_value));
                        if (err)
                        {
                        _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_ip, "dhcp")));
                        // printf("Netif Set DHCP Option Failed! 0x%04x: %s\n", err, esp_err_to_name(err));
                            return false;
                        }
                    }
    
                    // Start DHCPS
                    err = esp_netif_dhcps_start(_netifs[itf]);
                    if (err)
                    {
                        _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_ip, "dhcp")));
                        // printf("DHCPS Start Failed! 0x%04x: %s\n", err, esp_err_to_name(err));
                        return false;
                    }
                }
                else
                {
                    // Stop DHCPC
                    err = esp_netif_dhcpc_stop(_netifs[itf]);
                    if (err != ESP_OK && err != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED)
                    {
                        _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_ip, "dhcp")));
                        // printf("DHCP could not be stopped! Error: 0x%04x: %s\n", err, esp_err_to_name(err));
                        return false;
                    }
    
                    // Set IPv4, Netmask, Gateway
                    err = esp_netif_set_ip_info(_netifs[itf], &info);
                    if (err != ERR_OK)
                    {
                        _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_ip, "ip")));
                        // printf("ETH IP could not be configured! Error: 0x%04x: %s\n", err, esp_err_to_name(err));
                        return false;
                    }
    
                    // Set DNS Servers
                    esp_netif_set_dns_info(_netifs[itf], ESP_NETIF_DNS_MAIN, &d1);
                    esp_netif_set_dns_info(_netifs[itf], ESP_NETIF_DNS_BACKUP, &d2);
                    esp_netif_set_dns_info(_netifs[itf], ESP_NETIF_DNS_FALLBACK, &d3);
    
                    // Start DHCPC if static IP was set
                    if (info.ip.addr == 0)
                    {
                        err = esp_netif_dhcpc_start(_netifs[itf]);
                        if (err != ESP_OK && err != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STARTED)
                        {
                            _error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_ip, "dhsp")));
                            // printf("DHCP could not be started! Error: 0x%04x: %s\n", err, esp_err_to_name(err));
                            return false;
                        }
                        _flags.unset(flag_pos_t::static_ip);
                    }
                    else
                    {
                        _flags.set(flag_pos_t::static_ip);
                    }
                }
                return true;
            }
        };

        ufo::bit_flag_t<uint32_t, mutex_t> wfbs_t::_flags;
        esp_netif_t *wfbs_t::_netifs[wfbs_t::if_max_cnt] = {nullptr, nullptr};
        EventGroupHandle_t wfbs_t::_event_bit = nullptr;
    } // namespace wf_details

} // namespace ufo
