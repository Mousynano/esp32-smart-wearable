#ifndef FILE_IO_H
#define FILE_IO_H

#include <SPIFFS.h>

class FileIO {
public:
    FileIO(const char* fileName) : filename(fileName) { // Ubah ke const char*
        begin();
    }

    // Memulai SPIFFS
    bool begin() {
        if (!SPIFFS.begin(true)) {
            Serial.println("Failed to mount SPIFFS");
            return false;
        }
        return true;
    }

    // Membuat atau menulis data ke file (overwrite jika file sudah ada)
    bool writeFile(const String& data) {
        File file = SPIFFS.open(filename, "w");
        if (!file) {
            Serial.println("Failed to open file for writing");
            return false;
        }
        if (file.print(data) == 0) {
            Serial.println("Failed to write data to file");
            file.close();
            return false;
        }
        file.close();
        return true;
    }

    // Membaca data dari file
    String readFile() {
        File file = SPIFFS.open(filename, "r");
        if (!file) {
            Serial.println("Failed to open file for reading");
            return "";
        }
        String data;
        while (file.available()) {
            data += (char)file.read();
        }
        file.close();
        return data;
    }

    // Memperbarui data di file dengan cara menimpa konten file
    bool updateFile(const String& newData) {
        return writeFile(newData); // Menggunakan fungsi writeFile untuk overwrite
    }

    // Menghapus file
    bool deleteFile() {
        if (SPIFFS.exists(filename)) {
            if (SPIFFS.remove(filename)) {
                Serial.println("File deleted successfully");
                return true;
            } else {
                Serial.println("Failed to delete file");
                return false;
            }
        } else {
            Serial.println("File does not exist");
            return false;
        }
    }

    // Memeriksa apakah file ada di SPIFFS
    bool fileExists() {
        return SPIFFS.exists(filename);
    }

    // Mengubah file path yang digunakan
    void setFilePath(const char* path) {
        filename = path; // Mengubah file path yang digunakan
    }

private:
    const char* filename; // Ubah menjadi const char*
};

#endif
