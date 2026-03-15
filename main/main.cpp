#include <stdio.h>
#include "status_led.h"
#include "key.h"

extern "C" void app_main(void)
{
    StatusLed::GetInstance().InitStatusLed();
    StatusKey::GetInstance().InitKeys();
}
