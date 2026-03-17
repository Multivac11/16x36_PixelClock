#include <stdio.h>
#include "status_led.h"
#include "key.h"
#include "sht40.h"
#include "i2c.h"
#include "audio_capture.h"

extern "C" void app_main(void)
{
    StatusLed::GetInstance().InitStatusLed();
    StatusKey::GetInstance().InitKeys();
    I2CMaster::GetInstance().InitI2C();
    Sht40::GetInstance().InitSht40();
    AudioCapture::GetInstance().InitAudioCapture();
}
