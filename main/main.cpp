#include <stdio.h>

#include "ap_wifi.h"
#include "audio_capture.h"
#include "device_init.h"
#include "key.h"
#include "light_sensor.h"
#include "qmc5883p.h"
#include "qmi8658.h"
#include "scene_manager.h"
#include "status_led.h"

extern "C" void app_main(void)
{
    StatusLed::GetInstance().InitStatusLed();
    StatusKey::GetInstance().InitKeys();
    DeviceInit::GetInstance().Init();
    LightSensor::GetInstance().InitLightSensor();
    AudioCapture::GetInstance().InitAudioCapture();
    ApWifi::GetInstance().ApWifiInit();
    SceneManager::GetInstance().InitSceneManager();
}
