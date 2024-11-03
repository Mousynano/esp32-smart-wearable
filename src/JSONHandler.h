#ifndef JSON_HANDLER_H
#define JSON_HANDLER_H

#include <ArduinoJson.h>

class JSONHandler {
public:
    JSONHandler(JsonDocument& document) : jsonDoc(document) {}

    // Mengonversi string JSON ke JsonDocument (JSON parse)
    bool parse(const String& jsonString) {
        DeserializationError error = deserializeJson(jsonDoc, jsonString);
        if (error) {
            Serial.print("JSON Parsing failed: ");
            Serial.println(error.c_str());
            return false;
        }
        return true;
    }

    // Mengubah JsonDocument menjadi string JSON (JSON stringify)
    String stringify() const {
        String jsonString;
        serializeJson(jsonDoc, jsonString);
        return jsonString;
    }

    // Membuat objek JSON di root atau pada key tertentu
    JsonObject createObject(const char* key = nullptr) {
        if (key) {
            // Menggunakan metode baru untuk membuat objek JSON nested
            return jsonDoc[key].to<JsonObject>();
        }
        // Mengembalikan root JsonObject
        return jsonDoc.to<JsonObject>();
    }

    // Membuat array JSON di root atau pada key tertentu
    JsonArray createArray(const char* key = nullptr) {
        if (key) {
            // Menggunakan metode baru untuk membuat array JSON nested
            return jsonDoc[key].to<JsonArray>();
        }
        // Mengembalikan root JsonArray
        return jsonDoc.to<JsonArray>();
    }

    // Menambahkan item ke dalam array yang sudah ada
    bool addItemToArray(JsonArray& array, const String& value) {
        array.add(value);
        return true;
    }

    // Menambahkan objek ke dalam array yang sudah ada
    JsonObject addObjectToArray(JsonArray& array) {
        return array.add<JsonObject>();
    }

    String read(const char *key) const {
        return jsonDoc[key].as<String>(); // Directly return the value as String
    }

    // Cetak JSON dalam format yang rapi
    void printAll() const {
        serializeJsonPretty(jsonDoc, Serial);
        Serial.println();
    }

    // Menghapus seluruh isi JSON
    void clear() {
        jsonDoc.clear();
    }
private:
    JsonDocument& jsonDoc;
};

#endif