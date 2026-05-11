#include <stdio.h>

#include "ap_wifi.h"
#include "ds3231.h"
#include "es7210codec.h"
#include "es8311.h"
#include "husb238.h"
#include "i2c.h"
#include "key.h"
#include "light_sensor.h"
#include "qmc5883p.h"
#include "qmi8658.h"
#include "scene_manager.h"
#include "sht40.h"
#include "spi.h"
#include "spi_sdcard.h"
#include "status_led.h"

extern "C" void app_main(void)
{
    StatusLed::GetInstance().InitStatusLed();
    StatusKey::GetInstance().InitKeys();
    I2CMaster::GetInstance().InitI2C();
    SpiMaster::GetInstance().InitSpiMaster();
    SpiSdCard::GetInstance().InitSpiSdCard();
    Sht40::GetInstance().InitSht40();
    DS3231::GetInstance().InitDS3231();
    Husb238::GetInstance().InitHusb238();
    Qmi8658::GetInstance().InitQmi8658();
    Es8311::GetInstance().InitEs8311();
    Qmc5883P::GetInstance().InitQmc5883P();
    Es7210Codec::GetInstance().InitEs7210Codec();
    LightSensor::GetInstance().InitLightSensor();
    ApWifi::GetInstance().ApWifiInit();
    SceneManager::GetInstance().InitSceneManager();
}
