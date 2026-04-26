#include <stdio.h>

#include "ap_wifi.h"
#include "audio_capture.h"
#include "i2c.h"
#include "key.h"
#include "light_sensor.h"
#include "scene_manager.h"
#include "sht40.h"
#include "spi.h"
#include "spi_sdcard.h"
#include "status_led.h"

// TODO:修改测光adc口为GPIO1
extern "C" void app_main(void)
{
    StatusLed::GetInstance().InitStatusLed();
    StatusKey::GetInstance().InitKeys();
    I2CMaster::GetInstance().InitI2C();
    SpiMaster::GetInstance().InitSpiMaster();
    SpiSdCard::GetInstance().InitSpiSdCard();
    Sht40::GetInstance().InitSht40();
    AudioCapture::GetInstance().InitAudioCapture();
    LightSensor::GetInstance().InitLightSensor();
    ApWifi::GetInstance().ApWifiInit();
    SceneManager::GetInstance().InitSceneManager();
}
