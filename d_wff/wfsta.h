#pragma once

#include "wfbs.h"
#include "esp_eap_client.h"
namespace ufo
{
    namespace wf_details
    {
        // enable()->connect(ssid pass ... )
        class wfsta_t : public wfbs_t
        {
        private:
            wifi_scan_method_t _scanMethod = WIFI_FAST_SCAN;            // esp-idf
            wifi_sort_method_t _sortMethod = WIFI_CONNECT_AP_BY_SIGNAL; // esp-idf
            wifi_auth_mode_t _minSecurity = WIFI_AUTH_WPA2_PSK;         // esp-idf
        
        public:
            wfsta_t() {}
            ~wfsta_t() {}

            ip_cfg_t get_ip_config() const {
                return _get_ip_config(esp_interface_t::ESP_IF_WIFI_STA);
            }
            ip_t get_ip() const {
                return _get_ip(esp_interface_t::ESP_IF_WIFI_STA);
            }
            ip_t get_mask() const {
                return _get_netmask(esp_interface_t::ESP_IF_WIFI_STA);
            }
            ip_t get_gateway() const {
                return _get_gateway(esp_interface_t::ESP_IF_WIFI_STA);
            }
            ip_t get_broadcastIP() const {
                return _get_broadcastIP(esp_interface_t::ESP_IF_WIFI_STA);
            }
            ip_t get_networkID() const {
                return _get_networkID(esp_interface_t::ESP_IF_WIFI_STA);
            }
            ip_t get_subnetCIDR() const {
                return _get_subnetCIDR(esp_interface_t::ESP_IF_WIFI_STA);
            }
            void log_ipinfo() const {
                _log_info(esp_interface_t::ESP_IF_WIFI_STA);
            }

            bool ip_config(
                ip_t local_ip,
                ip_t gateway,
                ip_t subnet,
                ip_t dns1 = 0lu,
                ip_t dns2 = 0lu,
                ip_t dns3 = 0lu)
            {
                return _ip_config(esp_interface_t::ESP_IF_WIFI_STA,
                                 local_ip,
                                 gateway,
                                 subnet,
                                 dns1,
                                 dns2,
                                 dns3);
            }



            bool enable()
            {
                esp_err_t err = ESP_OK;
                wifi_mode_t curr = get_mode();

                if (curr != wifi_mode_t::WIFI_MODE_NULL) // драйвер уже запущен в режиме STA or AP
                {
                    printf("was started aga\n");
                    return true;
                }

                // if was inited return true
                if (!init_driver())
                { // wf-driver cant initialize
                    // error or warning
                    printf("!init driver\n");
                    return false;
                }

                init_events(event_wf_cb, this);
                init_events_net(event_net_cb, this);

                if (!_netifs[ESP_IF_WIFI_STA])
                {
                    // log_e("STA was enabled, but netif is NULL???");
                    printf("!netif\n");
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                    return false;
                }

                // esp_err_t err = esp_netif_set_hostname(_netifs[ESP_IF_WIFI_STA], ????);
                if (err != ESP_OK)
                {
                    // log_e("Could not set hostname! %d", err);
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                    return false;
                }
                err = esp_wifi_set_mode(wifi_mode_t::WIFI_MODE_STA);
                if (err != ESP_OK)
                {
                    // log_e("Could not set mode! %d", err);
                    printf("!esp_wifi_set_mode\n");
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                    return false;
                }

                if (_flags.get(flag_pos_t::long_rng))
                {
                    err = esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
                    if (err != ESP_OK)
                    {
                        // log_e("Could not enable long range on STA! %d", err);
                        __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                        return false;
                    }
                }

                // if was started return true
                if (!start())
                {
                    printf("!start\n");
                    return false;
                }

                EventBits_t bits = xEventGroupWaitBits(
                    _event_bit,
                    bit_sta_start,
                    pdFALSE,
                    pdFALSE,
                    portMAX_DELAY); // todo set timeout

                if (!(bits & bit_sta_start))
                {
                    printf("!bit\n");
                    // set reconnect error
                    // reason
                    return false;
                }
                _flags.set(flag_pos_t::enabled);
                return true;
            }

            bool disable()
            {
                if (!_flags.get(flag_pos_t::enabled))
                {
                    printf("was disabled\n");
                    return true;
                }

                wifi_config_t conf = {};
                esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &conf);
                if (err != ESP_OK)
                {
                    // log_e("STA clear config failed! 0x%x: %s", err, esp_err_to_name(err));
                    printf("!esp_wifi_set_config\n");
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                    return false;
                }
                if (_flags.get(flag_pos_t::sta_conn))
                {
                    //wait bit??
                    esp_wifi_disconnect();
                }
                utl::sleep_for(200);
                stop();
                deinit_events_net(event_net_cb);
                deinit_events(event_wf_cb);
                _flags.unset(flag_pos_t::enabled);
                return true;
            }

            bool connected() const {
                return _flags.get(flag_pos_t::sta_conn);
            }

            bool reconnect()
            {
                if (connected())
                {
                    if (esp_wifi_disconnect() != ESP_OK)
                    {
                        return false;
                    }
                }
                return esp_wifi_connect() == ESP_OK;
            }

            bool connect()
            {
                if (!_netifs[ESP_IF_WIFI_STA])
                {
                    // log_e("STA not started! You must call begin() first.");
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                    return false;
                }

                if (connected())
                {
                    // log_w("STA already connected.");
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                    return true;
                }

                wifi_config_t current_conf = {};
                esp_err_t err = esp_wifi_get_config(WIFI_IF_STA, &current_conf);
                if (err != ESP_OK)
                {
                    // log_e("STA config failed");
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                    return false;
                }
                err = esp_wifi_set_config(WIFI_IF_STA, &current_conf);
                if (err != ESP_OK)
                {
                    // log_e("STA config failed");
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                    return false;
                }

                if (_flags.get(flag_pos_t::static_ip))
                {
                    // if (!config())
                    // {
                    //     log_e("STA failed to configure dynamic IP!");
                    // __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));

                    //     return false;
                    // }
                }
                err = esp_wifi_connect();
                if (err != ESP_OK)
                {
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "!esp_wifi_connect")));
                    return false;
                }
                return true;
            }

            /**
             * Start Wifi connection
             * if passphrase is set the most secure supported mode will be automatically selected
             * @param ssid const char*          Pointer to the SSID string.
             * @param passphrase const char *   Optional. Passphrase. Valid characters in a passphrase must be between ASCII 32-126 (decimal).
             * @param bssid uint8_t[6]          Optional. BSSID / MAC of AP
             * @param channel                   Optional. Channel of AP
             * @param connect                   Optional. call connect
             * @return
             */
            bool connect(
                const char *ssid,
                const char *passphrase = nullptr,
                int32_t channel = 0,
                const uint8_t *bssid = nullptr,
                bool call_connect = true)
            {
                if (!_netifs[ESP_IF_WIFI_STA])
                {
                    // todo verbose print
                    printf("conn: !_netifs\n");
                    return false;
                }

                if (connected())
                {
                    // todo verbose print
                    printf("conn: 229 connected\n");
                    // log_verbose
                    if (!disconnect(true, 1000))
                    {
                        return false;
                    }
                }

                if (!check_ssid_pwd(ssid, passphrase))
                {
                    return false;
                }

                wifi_config_t conf = {};
                conf.sta.channel = channel;
                conf.sta.scan_method = _scanMethod;
                conf.sta.sort_method = _sortMethod;
                conf.sta.threshold.rssi = -127;
                conf.sta.pmf_cfg.capable = true;

                // change with ipa and ipnet
                if (ssid)
                {
                    memcpy(conf.sta.ssid, ssid, strlen(ssid));

                    if (passphrase)
                    {
                        conf.sta.threshold.authmode = _minSecurity;
                        memcpy(conf.sta.password, passphrase, strlen(passphrase));
                    }
                }
                if (bssid)
                {
                    if (bssid[0] != 0)
                    {
                        // warning moment: bssid len = 6
                        conf.sta.bssid_set = 1;
                        memcpy(conf.sta.bssid, bssid, 6);
                    }
                }

                esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &conf);
                if (err != ESP_OK)
                {
                    printf("conn: !set_cfg\n");
                    return false;
                }

                if (_flags.get(flag_pos_t::static_ip))
                {
                    // if (!config())
                    // {
                    //     log_e("STA failed to configure dynamic IP!");
                    //     return false;
                    // }
                }

                if (call_connect)
                {
                    esp_err_t err = esp_wifi_connect();
                    if (err != ESP_OK)
                    {
                        printf("conn: !esp_wifi_connect %s\n", esp_err_to_name(err));
                        return false;
                    }
                }
                return true;
            }

#if CONFIG_ESP_WIFI_ENTERPRISE_SUPPORT
            /**
             * Start Wifi connection with a WPA2 Enterprise AP
             * if passphrase is set the most secure supported mode will be automatically selected
             * @param ssid const char*          Pointer to the SSID string.
             * @param method wpa2_method_t      The authentication method of WPA2 (WPA2_AUTH_TLS, WPA2_AUTH_PEAP, WPA2_AUTH_TTLS)
             * @param wpa2_identity  const char*          Pointer to the entity
             * @param wpa2_username  const char*          Pointer to the username
             * @param password const char *     Pointer to the password.
             * @param ca_pem const char*        Pointer to a string with the contents of a  .pem  file with CA cert
             * @param client_crt const char*        Pointer to a string with the contents of a .crt file with client cert
             * @param client_key const char*        Pointer to a string with the contents of a .key file with client key
             * @param bssid uint8_t[6]          Optional. BSSID / MAC of AP
             * @param channel                   Optional. Channel of AP
             * @param call_connect                   Optional. call connect
             * @return
             */
            bool connect(
                const char *wpa2_ssid,
                wpa2_auth_method_t method,
                const char *wpa2_identity = nullptr,
                const char *wpa2_username = nullptr,
                const char *wpa2_password = nullptr,
                const char *ca_pem = nullptr,
                const char *client_crt = nullptr,
                const char *client_key = nullptr,
                int ttls_phase2_type = -1,
                int32_t channel = 0,
                const uint8_t *bssid = 0,
                bool call_connect = true)
            {
                if (!_netifs[ESP_IF_WIFI_STA])
                {
                    // log_e("STA not started! You must call begin() first.");
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                    return false;
                }

                if (connected())
                {
                    // log_w("STA currently connected. Disconnecting...");
                    printf("STA currently connected. Disconnecting...");
                    if (!disconnect(true, 1000))
                    {
                        return false;
                    }
                }

                if (!wpa2_ssid || *wpa2_ssid == 0x00 || strlen(wpa2_ssid) > 32)
                {
                    // log_e("SSID too long or missing!");
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));

                    return false;
                }

                if (wpa2_identity && strlen(wpa2_identity) > 64)
                {
                    // log_e("identity too long!");
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));

                    return false;
                }

                if (wpa2_username && strlen(wpa2_username) > 64)
                {
                    // log_e("username too long!");
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));

                    return false;
                }

                if (wpa2_password && strlen(wpa2_password) > 64)
                {
                    // log_e("password too long!");
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));

                    return false;
                }

                if (ttls_phase2_type >= 0)
                {
#if __has_include("esp_eap_client.h")
                    esp_eap_client_set_ttls_phase2_method((esp_eap_ttls_phase2_types)ttls_phase2_type);
#else
                    esp_wifi_sta_wpa2_ent_set_ttls_phase2_method((esp_eap_ttls_phase2_types)ttls_phase2_type);
#endif
                }

                if (ca_pem)
                {
#if __has_include("esp_eap_client.h")
                    esp_eap_client_set_ca_cert((uint8_t *)ca_pem, strlen(ca_pem));
#else
                    esp_wifi_sta_wpa2_ent_set_ca_cert((uint8_t *)ca_pem, strlen(ca_pem));
#endif
                }

                if (client_crt)
                {
#if __has_include("esp_eap_client.h")
                    esp_eap_client_set_certificate_and_key((uint8_t *)client_crt, strlen(client_crt), (uint8_t *)client_key, strlen(client_key), NULL, 0);
#else
                    esp_wifi_sta_wpa2_ent_set_cert_key((uint8_t *)client_crt, strlen(client_crt), (uint8_t *)client_key, strlen(client_key), NULL, 0);
#endif
                }

#if __has_include("esp_eap_client.h")
                esp_eap_client_set_identity((uint8_t *)wpa2_identity, strlen(wpa2_identity));
#else
                esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)wpa2_identity, strlen(wpa2_identity));
#endif
                if (method == WPA2_AUTH_PEAP || method == WPA2_AUTH_TTLS)
                {
#if __has_include("esp_eap_client.h")
                    esp_eap_client_set_username((uint8_t *)wpa2_username, strlen(wpa2_username));
                    esp_eap_client_set_password((uint8_t *)wpa2_password, strlen(wpa2_password));
#else
                    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)wpa2_username, strlen(wpa2_username));
                    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)wpa2_password, strlen(wpa2_password));
#endif
                }
#if __has_include("esp_eap_client.h")
                esp_wifi_sta_enterprise_enable(); // set config settings to enable function
#else
                esp_wifi_sta_wpa2_ent_enable(); // set config settings to enable function
#endif

                return connect(wpa2_ssid, NULL, channel, bssid, call_connect); // connect to wifi
            }
#endif /* CONFIG_ESP_WIFI_ENTERPRISE_SUPPORT */

            bool disconnect(bool clear_cfg = false, uint32_t timeout = 1000)
            {
                if (clear_cfg)
                {
                    if (!start())
                    {
                        // log_e("STA not started! You must call begin first.");
                        __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                        return false;
                    }
                    wifi_config_t conf = {};
                    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &conf);
                    if (err != ESP_OK)
                    {
                        // log_e("STA clear config failed! 0x%x: %s", err, esp_err_to_name(err));
                        __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                        return false;
                    }
                }

                if (!connected())
                {
                    printf("STA already disconnected.\n");
                    return true;
                }

                esp_err_t err = esp_wifi_disconnect();
                if (err != ESP_OK)
                {
                    // log_e("STA disconnect failed! 0x%x: %s", err, esp_err_to_name(err));
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "!esp_wifi_disconnect")));
                    return false;
                }

                if (timeout)
                {
                    // const unsigned long start = millis();
                    const uint32_t start = utl::get_time_millis();
                    while (connected() && ((utl::get_time_millis() - start) < timeout))
                    {
                        utl::sleep_for(5);
                    }
                    if (connected())
                    {
                        return false;
                    }
                }

                return true;
            }

        private:
            static void event_net_cb(void *arg, esp_event_base_t base, int32_t id, void *data)
            {
                if (base == IP_EVENT)
                {
                    if (id == IP_EVENT_STA_GOT_IP)
                    {
                        ip_event_got_ip_t *event = reinterpret_cast<ip_event_got_ip_t *>(data);

                        // connect->_ip = event->ip_info.ip.addr;
                        // connect->_reconection = 0;

                        Trace_t::flog("\nip: %lu,\t gw: %lu,\t msk: %lu\n", event->ip_info.ip.addr, event->ip_info.gw.addr, event->ip_info.netmask.addr);
                    }
                    else if (id == IP_EVENT_STA_LOST_IP)
                    {
                        Trace_t::flog("ip-disconnected\n");
                    }
                }
            }

            static void event_wf_cb(void *arg, esp_event_base_t base, int32_t id, void *data)
            {
                if (base == WIFI_EVENT)
                {
                    if (id == WIFI_EVENT_STA_DISCONNECTED)
                    {
                        wifi_event_sta_disconnected_t *event = reinterpret_cast<wifi_event_sta_disconnected_t*>(data);
                        printf("sta-disconn from app; reason code %u\n", event->reason);
                        if (event->reason != wifi_err_reason_t::WIFI_REASON_ASSOC_LEAVE)
                        {
                            printf("call reconnect\n");
                            esp_wifi_connect();
                        }
                        wfsta_t::_flags.unset(flag_pos_t::sta_conn);
                    }
                    else if (id == WIFI_EVENT_STA_START)
                    {
                        xEventGroupSetBits(wfsta_t::_event_bit, bit_sta_start);
                        printf("sta-start\n");
                    }
                    else if (id == WIFI_EVENT_STA_STOP)
                    {
                        xEventGroupClearBits(_event_bit, bit_sta_start);
                            // xEventGroupSetBits(wfsta_t::_event_bit, bit_sta_end);
                        printf("sta-stop\n");
                    }
                    else if (id == WIFI_EVENT_STA_CONNECTED)
                    {
                        // lock???
                        wfsta_t::_flags.set(flag_pos_t::sta_conn);
                        Trace_t::log("sta-connected-to-ap\n");
                        // connect->_reconection = 0;
                    }
                }
            }

        public:
            bool bandwidth(wifi_bandwidth_t bandwidth)
            {
                if (!_flags.get(flag_pos_t::enabled))
                {
                    return false;
                }
    
                esp_err_t err;
                err = esp_wifi_set_bandwidth(WIFI_IF_STA, bandwidth);
                if (err)
                {
                    // log_e("Could not set STA bandwidth! 0x%x: %s", err, esp_err_to_name(err));
                    __global_error.Push(Warning_t(GenerateInfo_Code(error::codes_t::wf_drv_init, "drv")));
                    return false;
                }
    
                return true;
            }
        };
    }
} // namespace ufo
