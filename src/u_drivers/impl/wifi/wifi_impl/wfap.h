#pragma once

#include "wfbs.h"

namespace ufo
{
    namespace wf_details
    {
        class wfap_t : public wfbs_t
        {
        private:

        public:
            wfap_t(/* args */) {}
            ~wfap_t() {}

            ip_t get_ip()  {
                return _get_ip(esp_interface_t::ESP_IF_WIFI_AP);
            }
            ip_t get_mask()  {
                return _get_netmask(esp_interface_t::ESP_IF_WIFI_AP);
            }
            ip_t get_gateway() const {
                return _get_gateway(esp_interface_t::ESP_IF_WIFI_AP);
            }
            ip_t get_broadcastIP() const {
                return _get_broadcastIP(esp_interface_t::ESP_IF_WIFI_AP);
            }
            ip_t get_networkID() const {
                return _get_networkID(esp_interface_t::ESP_IF_WIFI_AP);
            }
            ip_t get_subnetCIDR() const {
                return _get_subnetCIDR(esp_interface_t::ESP_IF_WIFI_AP);
            }
            void log_ipinfo() const {
                _log_info(esp_interface_t::ESP_IF_WIFI_AP);
            }

            bool ip_config(
                ip_t local_ip,
                ip_t gateway,
                ip_t subnet,
                ip_t dns1 = 0lu,
                ip_t dns2 = 0lu,
                ip_t dns3 = 0lu)
            {
                return _ip_config(esp_interface_t::ESP_IF_WIFI_AP,
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
                    // printf("was started aga");
                    if (_flags.get(flag_pos_t::sta_conn))
                    {
                        // printf("in mode sta");
                    }
                    // printf("\n");

                    return true;
                }

                // if was inited return true
                if (!init_driver())
                { // wf-driver cant initialize
                    // error or warning
                    __set_err(201);
                    // printf("!init driver\n");
                    return false;
                }

                init_events(event_wf_cb, this);
                init_events_net(event_net_cb, this);

                if (!_netifs[ESP_IF_WIFI_AP])
                {
                    // log_e("STA was enabled, but netif is NULL???");
                    printf("!netif\n");
                    __set_err(202);
                    return false;
                }

                err = esp_wifi_set_mode(wifi_mode_t::WIFI_MODE_AP);
                if (err != ESP_OK)
                {
                    printf("!esp_wifi_set_mode\n");
                    __set_err(203);
                    return false;
                }

                if (_flags.get(flag_pos_t::long_rng))
                {
                    err = esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
                    if (err != ESP_OK)
                    {
                        __set_err(204);
                        return false;
                    }
                }
                // if was started return true
                if (!start())
                {
                    return false;
                }

                EventBits_t bits = xEventGroupWaitBits(
                    _event_bit,
                    bit_ap_start,
                    pdFALSE,
                    pdFALSE,
                    portMAX_DELAY); // todo set timeout

                if (!(bits & bit_ap_start))
                {
                    // printf("!bit\n");
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
                    return true;
                }

                wifi_config_t conf = {};
                esp_err_t err = esp_wifi_set_config(WIFI_IF_AP, &conf);
                if (err != ESP_OK)
                {
                    // printf("!esp_wifi_set_config\n");
                    __set_err(210);
                    return false;
                }
                utl::sleep_for(200);
                stop();
                deinit_events_net(event_net_cb);
                deinit_events(event_wf_cb);
                _flags.unset(flag_pos_t::enabled);
                return true;
            }

            bool create(
                const char *ssid, 
                const char *passphrase = nullptr, 
                int channel = 1, 
                int ssid_hidden = 0, 
                int max_connection = 4,
                bool ftm_responder = false, 
                wifi_auth_mode_t auth_mode = WIFI_AUTH_WPA2_PSK,
                wifi_cipher_type_t cipher = WIFI_CIPHER_TYPE_CCMP
            )
            {
                if (!check_ssid_pwd(ssid, passphrase))
                {
                    return false;
                }

                if (!enabled())
                {
                    return false;
                }
                wifi_config_t conf = {};
                conf.ap.channel = channel;
                conf.ap.max_connection = max_connection;
                conf.ap.beacon_interval = 100;
                conf.ap.ssid_hidden = ssid_hidden;
                conf.ap.ftm_responder = ftm_responder;

                memcpy(conf.ap.ssid, ssid, strlen(ssid));
                if (passphrase)
                {
                    conf.ap.authmode = auth_mode;
                    conf.ap.pairwise_cipher = cipher;
                    memcpy(conf.ap.password, passphrase, strlen(passphrase));
                }

                esp_err_t err = esp_wifi_set_config(WIFI_IF_AP, &conf);
                if (err != ESP_OK)
                {
                    // printf("Set AP config failed! 0x%x: %s\n", err, esp_err_to_name(err));
                    __set_err(220);
                    return false;
                }
                
                return true;
            }

            bool clear()
            {
                if (!enabled())
                {
                    return false;
                }
                wifi_config_t conf = {};
                conf.ap.channel = 1;
                conf.ap.max_connection = 4;
                conf.ap.beacon_interval = 100;
                esp_err_t err = esp_wifi_set_config(WIFI_IF_AP, &conf);
                if (err != ESP_OK)
                {
                    // printf("Set AP config failed! 0x%x: %s\n", err, esp_err_to_name(err));
                    __set_err(225);
                    return false;
                }
                return true;
            }

        private:
            static void event_net_cb(void *arg, esp_event_base_t base, int32_t id, void *data)
            {
                if (base == IP_EVENT)
                {
                    if (id == IP_EVENT_AP_STAIPASSIGNED)
                    {
                        printf("IP_EVENT_AP_STAIPASSIGNED\n");       // temp
                        ip_event_ap_staipassigned_t *event = reinterpret_cast<ip_event_ap_staipassigned_t*>(data);
                        ip_t ip(event->ip.addr);
                        printf("sta-connected: " MACSTR ", ip: %u.%u.%u.%u\n", MAC2STR(event->mac), ip[3],ip[2],ip[1],ip[0]);
                    }
                }
            }

            static void event_wf_cb(void *arg, esp_event_base_t base, int32_t id, void *data)
            {
                if (base == WIFI_EVENT)
                {
                    if (id == WIFI_EVENT_AP_STACONNECTED)
                    {
                        wifi_event_ap_staconnected_t *event = reinterpret_cast<wifi_event_ap_staconnected_t*>(data);
                        printf("app-connected: " MACSTR ", aid: %u\n", MAC2STR(event->mac), event->aid);
                    }
                    else if (id == WIFI_EVENT_AP_STADISCONNECTED)
                    {
                        wifi_event_ap_stadisconnected_t *event = reinterpret_cast<wifi_event_ap_stadisconnected_t*>(data);
                        printf("app-disconn; reason code %u\n", event->reason);
                        printf("\tinfo:" MACSTR ", aid: %u\n", MAC2STR(event->mac), event->aid);
                    }
                    else if (id == WIFI_EVENT_AP_PROBEREQRECVED)
                    {
                        wifi_event_ap_probe_req_rx_t *event = reinterpret_cast<wifi_event_ap_probe_req_rx_t*>(data);
                        printf("ping (rssi - %d) from " MACSTR "\n", event->rssi, MAC2STR(event->mac));
                    }
                    
                    else if (id == WIFI_EVENT_AP_START)
                    {
                        xEventGroupSetBits(wfap_t::_event_bit, bit_ap_start);
                        // printf("ap-start\n");
                    }
                    else if (id == WIFI_EVENT_AP_STOP)
                    {
                        xEventGroupClearBits(wfap_t::_event_bit, bit_ap_start);
                        // printf("ap-stop\n");
                    }
                }
            }
        };

    } // namespace wf_details
} // namespace ufo
