#include "scene_manager.h"

void SceneManager::InitSceneManager()
{
    MatrixHal::GetInstance().MatrixHalInit();

    auto& sd = SpiSdCard::GetInstance();
    auto& gfx = MatrixHal::GetInstance().Gfx();
    sd.ListDirectory("/sdcard/img");
    sd.ListDirectory("/sdcard/img/emoji");

    if (!sd.IsMounted()) return;

    alignas(4) uint8_t buf[SPRITE_SIZE];
    sd.ReadFile("/sdcard/img/emoji/smile_16x16.bin", buf, SPRITE_SIZE);
    const Color* bitmap = reinterpret_cast<const Color*>(buf);
    gfx.drawRGBBitmap(0, 0, bitmap, 16, 16, 25);
    sd.ReadFile("/sdcard/img/emoji/197_16x16.bin", buf, SPRITE_SIZE);
    const Color* bitmap1 = reinterpret_cast<const Color*>(buf);
    gfx.drawRGBBitmap(16, 0, bitmap1, 16, 16, 25);

    MatrixHal::GetInstance().Refresh();
}
