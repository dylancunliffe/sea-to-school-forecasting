#include <SD.h>
#include <SPI.h>
#include <TinyGPSPlus.h>

static const int RXPin = 25, TXPin = 26;  // GPS pins
static const uint32_t GPSBaud = 9600;

// Default sd card pins
#define SD_CS   5   // Chip Select
#define SD_MISO 19  // Master In Slave Out
#define SD_MOSI 23  // Master Out Slave In
#define SD_CLK  18  // Clock

TinyGPSPlus gps;

HardwareSerial ss(1);
SPIClass spiSD(VSPI);

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  // Start SD SPI
  spiSD.begin(SD_CLK, SD_MISO, SD_MOSI, SD_CS);
  
  // Start GPS serial
  ss.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);
  Serial.println("GPS init successful.");
  
  // Init SD card
  if (!SD.begin(SD_CS, spiSD)) {
    Serial.println("SD card init failed!");
    return;
  }
  Serial.println("SD card init successful.");

  if(!SD.exists("gpsdata.txt")) {
    File headerFile = SD.open("gpsdata.txt", FILE_WRITE);
    if (headerFile) {
      headerFile.println("Latitude, Longitude, Speed, Year, Month, Day, Time");
      headerFile.close();
    } else {
      Serial.println("FILE_EXISTS_NO_HEADER_ADDED");
    }
  }
}

void loop() {
  // Feed GPS data
  while (ss.available()) {
    char c = ss.read();
    gps.encode(c);
    Serial.write(c);
  }

  // Open data file for writing
  File dataFile = SD.open("/gpsdata.txt", FILE_APPEND);

  if (dataFile) {
    // Log location
    if (gps.location.isValid()) {
      dataFile.print(gps.location.lat(), 6);
      dataFile.print(",");
      dataFile.print(gps.location.lng(), 6);
      dataFile.print(",");
    } else {
      dataFile.print("INVALID_LAT,INVALID_LNG,");
    }

    // Log speed
    dataFile.print(gps.speed.kmph());
    dataFile.print(",");

    // Log date
    if (gps.date.isValid()) {
      dataFile.print(gps.date.year());
      dataFile.print(",");
      if (gps.date.month() < 10) dataFile.print("0");
      dataFile.print(gps.date.month());
      dataFile.print(",");
      if (gps.date.day() < 10) dataFile.print("0");
      dataFile.print(gps.date.day());
      dataFile.print(",");
    } else {
      dataFile.print("INVALID_DATE,");
    }

    // Log time
    if (gps.time.isValid()) {
      if (gps.time.hour() < 10) dataFile.print("0");
      dataFile.print(gps.time.hour());
      dataFile.print(":");
      if (gps.time.minute() < 10) dataFile.print("0");
      dataFile.print(gps.time.minute());
      dataFile.print(":");
      if (gps.time.second() < 10) dataFile.print("0");
      dataFile.print(gps.time.second());
    } else {
      dataFile.print("INVALID_TIME");
    }

    dataFile.println();
    dataFile.flush();
    dataFile.close();
    delay(1000);

    // Serial printing gps data for debugging
    if (gps.location.isValid()){
      Serial.print(gps.location.lat(), 6);
      Serial.print(" , ");
      Serial.println(gps.location.lng(), 6);
    }
    else {
      Serial.println("Location not yet valid");
    }

    if(gps.location.isValid() && gps.time.isValid()){
      Serial.print("Date: "); Serial.print(gps.date.month()); Serial.print("/");
      Serial.print(gps.date.day()); Serial.print("/");
      Serial.print(gps.date.year()); Serial.print("  Time: ");
      Serial.print(gps.time.hour()); Serial.print(":");
      Serial.print(gps.time.minute()); Serial.print(":");
      Serial.println(gps.time.second());
    }

  } else {
    Serial.println("ERROR_OPENING_FILE");
    delay(10000);
  }
}
