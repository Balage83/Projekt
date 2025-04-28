#include <Wire.h>
#include <SensirionI2cScd4x.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define BMP280_ADDRESS 0x76

#define SCD40_I2C_ADDRESS 0x62

SensirionI2cScd4x scd40;

void setup() {
  Serial.begin(9600);

  Wire.begin();

  scd40.begin(Wire, SCD40_I2C_ADDRESS); // Itt hivatkozunk az SCD40-re
  scd40.startPeriodicMeasurement();

  // LCD elindul és vár a szenzorokra
  lcd.init();
  lcd.backlight(); 
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();
}

void loop() {
  // BMP280 kiolvassa az adatokat
  float temperature = readBMP280Temperature();
  float pressure = readBMP280Pressure();

  // CO2 szenzor kiolvassa az adatokat
  uint16_t co2 = readSCD40CO2();

  // LCD-re kiírja az adatokat
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C");
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(co2);
  lcd.print(" ppm");

  // 1mp delay, majd megint olvas
  delay(1000);
}

float readBMP280Temperature() {
  // Hőm. kiolvasás
  Wire.beginTransmission(BMP280_ADDRESS);
  Wire.write(0xFA); // Hőm. regiszter
  Wire.endTransmission();
  Wire.requestFrom(BMP280_ADDRESS, 3);

  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();
  uint8_t xlsb = Wire.read();
  int32_t adc_T = (msb << 12) | (lsb << 4) | (xlsb >> 4);

  // Hőm átváltás
  float var1 = (adc_T / 16384.0 - 0.5) * 0.01;
  float temperature = var1 * 100.0;

  return temperature;
}

float readBMP280Pressure() {
  // Nyomás mérés
  Wire.beginTransmission(BMP280_ADDRESS);
  Wire.write(0xF7); // Nyomás regiszter
  Wire.endTransmission();
  Wire.requestFrom(BMP280_ADDRESS, 3);

  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();
  uint8_t xlsb = Wire.read();

  int32_t adc_P = (msb << 12) | (lsb << 4) | (xlsb >> 4);

  // Nyomás átváltás
  float var1 = (adc_P / 131072.0 - 0.5) * 0.01;
  float pressure = var1 * 100.0;

  return pressure;
}

uint16_t readSCD40CO2() {
  uint16_t co2 = 0;
  float temperature, humidity;
  bool isDataReady = false;

  scd40.getDataReadyStatus(isDataReady);

  if (isDataReady) {
    // SCD40 adatok kiolvasása
    scd40.readMeasurement(co2, temperature, humidity);
  }

  return co2;
}