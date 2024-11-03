#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include "JSONHandler.h"
#include "FileIO.h"
#include "UUID.h"

#define MANAGER_AP_SSID "Manager_AP"
#define MANAGER_AP_PASSWORD "manager_password"

JsonDocument jsonDoc; // Ubah ke StaticJsonDocument untuk ukuran yang lebih kecil
JSONHandler jsonHandler(jsonDoc);
FileIO fileHandler("/data.json");

AsyncWebServer server(80);
WebSocketsServer webSocket(81);

BLEScan* pBLEScan;
String eventName = "scan_bluetooth", previousScanResult = "";

void scanBluetoothDevices() {
    int scanTime = 5; // duration in seconds
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true); // Active scanning for more details

    BLEScanResults foundDevices = pBLEScan->start(scanTime, false); // Dereference the pointer to get BLEScanResults

    String currentScanResult = ""; // String to hold the current scan results

    // Iterate through the found devices
    for (int i = 0; i < foundDevices.getCount(); i++) {
        BLEAdvertisedDevice device = foundDevices.getDevice(i);
        String deviceName = device.getName().c_str();
        String deviceAddress = device.getAddress().toString().c_str();
        currentScanResult += deviceName + " (" + deviceAddress + ")\n"; // Concatenate name and address
    }

    // Check if current scan result differs from the previous one
    if (currentScanResult != previousScanResult) {
        Serial.println("Devices found:");
        Serial.println(currentScanResult); // Print the current scan result
        previousScanResult = currentScanResult; // Update the previous result
    }
}

void handleInitRequest(AsyncWebServerRequest *request) {
    // Baca file JSON
    String fileContent = fileHandler.readFile();
    jsonHandler.parse(fileContent); // Parse JSON dari file
    String response = jsonHandler.stringify(); // Serialize kembali untuk mengirim

    if (fileContent == response){
        Serial.println("Mantap cuy");
    }

    request->send(200, "application/json", response); // Kirim response
}

void onWebSocketEvent(uint8_t client_num, WStype_t type, uint8_t * payload, size_t length) {
    if (type == WStype_TEXT) {
        Serial.printf("Received data from wearable: %s\n", payload);
    }
}

void connectToWiFi(String ssid, String password) {
    Serial.println("Mencoba untuk terhubung ke WiFi...");
    WiFi.begin(ssid.c_str(), password.c_str());

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nTerhubung ke WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(115200);

    String wifi, pass, creds = fileHandler.readFile();
    
    if(creds){
        jsonHandler.parse(creds);
        for (JsonPair kv : jsonDoc.as<JsonObject>()){
            wifi = String(kv.key().c_str());
            pass = kv.value().as<String>();
            Serial.println("String wifi: " + wifi);
            Serial.println("String pass: " + pass);
        }
    } else {
        // Ambil kredensial WiFi dari Serial
        Serial.println("Silahkan masukkan kredensial WiFi yang mau digunakan: ");
        while (Serial.available() == 0) {}  // Tunggu sampai ada input
        wifi = Serial.readStringUntil('\n');

        Serial.println("Silahkan masukkan passwordnya: ");
        while (Serial.available() == 0) {};  // Tunggu sampai ada input
        pass = Serial.readStringUntil('\n');

        jsonDoc["ssid"] = wifi; // Ganti "wifi" dengan "ssid" untuk lebih jelas
        jsonDoc["password"] = pass; // Ganti "pass" dengan "password"

        // Tulis kredensial ke file
        fileHandler.writeFile(jsonHandler.stringify());
    }

    // Menyiapkan AP WiFi
    WiFi.softAP(String(MANAGER_AP_SSID), String(MANAGER_AP_PASSWORD));
    Serial.println("Manager AP started");

    // Setup HTTP Server dan WebSocket
    server.on("/init", HTTP_GET, handleInitRequest);
    server.begin();
    webSocket.begin();
    webSocket.onEvent(onWebSocketEvent);

    // Koneksikan ke jaringan WiFi
    connectToWiFi(wifi, pass);

    // Setup mDNS dengan hostname "manager"
    if (MDNS.begin("manager")) {
        Serial.println("mDNS responder started: manager.local");
    } else {
        Serial.println("Error setting up mDNS responder!");
    }

    // Inisialisasi BLE
    BLEDevice::init("ManagerDevice"); // Nama perangkat BLE yang di-broadcast
    pBLEScan = BLEDevice::getScan(); // Mendapatkan instance pemindaian BLE
    pBLEScan->setActiveScan(true);   // Mengaktifkan active scan untuk detail lebih lanjut
    pBLEScan->setInterval(100);      // Mengatur interval scan
    pBLEScan->setWindow(99);         // Mengatur durasi window
}

void loop() {
    String input = Serial.readStringUntil('\n');
    if (input) eventName = String(input);
    if (eventName == "default") {
        webSocket.loop();
    } else if (eventName == "scan_bluetooth") {
        scanBluetoothDevices();
    }
}
