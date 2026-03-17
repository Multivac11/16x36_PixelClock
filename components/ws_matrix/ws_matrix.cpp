#include "ws_matrix.h"

void WsMatrix::InitWsMatrix()
{
    xTaskCreatePinnedToCore(WsMatrixTask, "WsMatrixTask", 2048, this, 1, nullptr, 1);
}

void WsMatrix::WsMatrixTask(void *pvParameters)
{
    static_cast<WsMatrix *>(pvParameters)->ShowMatrix();
}

void WsMatrix::ShowMatrix()
{
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}