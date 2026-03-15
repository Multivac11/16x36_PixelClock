#include <stdio.h>
#include "status_led.h"

extern "C" void app_main(void)
{
    StatusLed::GetInstance().InitStatusLed();
}
