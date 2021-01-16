
#if LWIP_FEATURES && !LWIP_IPV6

#define HAVE_NETDUMP 0


#define STASSID "AP1"
#define STAPSK "11223344"

#include <ESP8266WiFi.h>
#include <lwip/napt.h>
#include <lwip/dns.h>
#include <lwip/ip.h>
#include <dhcpserver.h>

#define NAPT 1000
#define NAPT_PORT 10

IPAddress staticIP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);


void setup() {
  Serial.begin(115200);
  Serial.printf("\n\nNAPT Range extender\n");
  Serial.printf("Heap on start: %d\n", ESP.getFreeHeap());



  // first, connect to STA so we can get a proper local DNS server
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.printf("\nSTA: %s (dns: %s / %s)\n",
                WiFi.localIP().toString().c_str(),
                WiFi.dnsIP(0).toString().c_str(),
                WiFi.dnsIP(1).toString().c_str());

  // give DNS servers to AP side
  dhcps_set_dns(0, WiFi.dnsIP(0));
  dhcps_set_dns(1, WiFi.dnsIP(1));

  WiFi.softAPConfig(staticIP, gateway, subnet);
  WiFi.softAP(STASSID "extender", STAPSK);
  Serial.printf("AP: %s\n", WiFi.softAPIP().toString().c_str());

  Serial.printf("Heap before: %d\n", ESP.getFreeHeap());
  err_t ret = ip_napt_init(NAPT, NAPT_PORT);
  Serial.printf("ip_napt_init(%d,%d): ret=%d (OK=%d)\n", NAPT, NAPT_PORT, (int)ret, (int)ERR_OK);
  if (ret == ERR_OK) {
    ret = ip_napt_enable_no(SOFTAP_IF, 1);
    Serial.printf("ip_napt_enable_no(SOFTAP_IF): ret=%d (OK=%d)\n", (int)ret, (int)ERR_OK);

    ip_portmap_add(IP_PROTO_TCP, WiFi.localIP(), 8888, IPAddress(192,168,4,100), 8888);
    // Serial.printf("IP_PROTO_TCP: ret=%d (OK=%d)\n", (int)ret, (int)ERR_OK);

    if (ret == ERR_OK) {
      Serial.printf("WiFi Network '%s' with same password is now NATed behind '%s'\n", STASSID "extender", STASSID);
    }
  }
  Serial.printf("Heap after napt init: %d\n", ESP.getFreeHeap());
  if (ret != ERR_OK) {
    Serial.printf("NAPT initialization failed\n");
  }
}

#endif

void loop() {
}