#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "npixel.h"

#define TAG "Example"

extern "C" {

void app_main(void) {
    NPixel_WS2812 strip = NPixel_WS2812(GPIO_NUM_15, 12, RMT_CHANNEL_0);
    strip.clear();
    strip.setPixelRGBW(0, 1, 1, 1, 0);
    strip.setPixelRGBW(1, 10, 10, 10, 0);
    strip.setPixelRGBW(5, 1, 0, 0, 0);
    strip.show();
    for(int i=0; ; i++) {
	vTaskDelay(1000/portTICK_PERIOD_MS);
	ESP_LOGI(TAG, "Loop %d", i);
    }
}

}
