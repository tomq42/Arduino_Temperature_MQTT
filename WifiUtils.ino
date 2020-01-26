
const char* wl_status_str(int e) {
  Serial.print("wl_status_str(");
  Serial.print(e);
  Serial.println(")");
  if (e==WL_CONNECTED) return "WL_CONNECTED";
  else if (e==WL_IDLE_STATUS) return "WL_IDLE_STATUS";
  else if (e==WL_NO_SSID_AVAIL) return "WL_NO_SSID_AVAIL";
  else if (e==WL_SCAN_COMPLETED) return "WL_SCAN_COMPLETED";
  else if (e==WL_CONNECT_FAILED) return "WL_CONNECT_FAILED";
  else if (e==WL_CONNECTION_LOST) return "WL_CONNECTION_LOST";
  else if (e==WL_DISCONNECTED) return "WL_DISCONNECTED";
  else return "Unknown";
}

void printWifiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}
