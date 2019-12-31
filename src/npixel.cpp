#include "npixel.h"
#include "esp_log.h"

// RMT Clock source is @ 80 MHz. Dividing it by 8 gives us 10 MHz frequency, or 100ns period.
#define LED_STRIP_RMT_CLK_DIV (8)
#define TAG "NPixel"


/* Contructor and Destructor */
Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    red = r;
    green = g;
    blue = b;
    white = w;
}

/* Setters */
Color &Color::setRGBW(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    red = r;
    green = g;
    blue = b;
    white = w;
    return *this;
}

Color &Color::setColor(Color &c) {
    red   = c.getRed();
    green = c.getGreen();
    blue  = c.getBlue();
    white = c.getWhite();
    return *this;
}

Color &Color::set32(uint32_t c) {
    red   = (c >> 24) & 0xff;
    green = (c >> 16) & 0xff;
    blue  = (c >>  8) & 0xff;
    white = (c >>  0) & 0xff;
    return *this;
}

Color &Color::setRed(uint8_t r) {
    red = r;
    return *this;
}

Color &Color::setGreen(uint8_t g) {
    green = g;
    return *this;
}

Color &Color::setBlue(uint8_t b) {
    blue = b;
    return *this;
}

Color &Color::setWhite(uint8_t w) {
    white = w;
    return *this;
}

/* Getters */
uint32_t Color::get32() {
    uint32_t c = red   << 24 |
                 green << 16 |
                 blue  <<  8 |
                 white <<  0;
    return c;
}

uint8_t Color::getRed() {
    return red;
}

uint8_t Color::getGreen() {
    return green;
}

uint8_t Color::getBlue() {
    return blue;
}

uint8_t Color::getWhite() {
    return white;
}


NPixel::NPixel(gpio_num_t p, uint16_t l, rmt_channel_t c) {
    pin = p;
    length = l;
    rmt_channel = c;

    /* 
     * Allocate buffer storage for the strip colors and the rmt bitstream 
     * 
     * If you are not using an RGBW strip then \**this over allocates memory 
     * for both the pixels and the buffer by an additional 1/3rd, but for 
     * simplicity of code I have retained \**this inefficiency
     */
    pixels = (Color *)malloc(sizeof(Color) * length);
    if (pixels == NULL) {
        /* We failed to allocate heap for the pixels */
        ESP_LOGE(TAG, "Unable to allocate heap for the pixel colors");
    }

    buffer = (rmt_item32_t *)malloc(sizeof(rmt_item32_t) * BITS_PER_PIXEL * length);
    if (buffer == NULL) {
        /* We failed to allocate heap for the buffer */
        ESP_LOGE(TAG, "Unable to allocate heap for the rmt buffer");
    }

    /* Configure the rmt peripheral */
    rmt_config_t rmt_cfg = {
        .rmt_mode = RMT_MODE_TX,
        .channel = rmt_channel,
        .clk_div = LED_STRIP_RMT_CLK_DIV,
        .gpio_num = pin,
        .mem_block_num = 1,
        .tx_config = {
            .loop_en = false,
            .carrier_freq_hz = 100, // Not used, but has to be set to avoid divide by 0 err
            .carrier_duty_percent = 50,
            .carrier_level = RMT_CARRIER_LEVEL_LOW,
            .carrier_en = false,
            .idle_level = RMT_IDLE_LEVEL_LOW,
            .idle_output_en = true,
        }
    };

    esp_err_t cfg_ok = rmt_config(&rmt_cfg);
    if (cfg_ok != ESP_OK) {
        ESP_LOGE(TAG, "RMT config failed");
    }
    esp_err_t install_ok = rmt_driver_install(rmt_cfg.channel, 0, 0);
    if (install_ok != ESP_OK) {
        ESP_LOGE(TAG, "RMT driver install failed");
    }
}

NPixel::~NPixel() {
    free(buffer);
    free(pixels);
}

NPixel &NPixel::setPixelColor(uint16_t pixel, Color color) {
    if (pixel < length) {
        pixels[pixel].setColor(color);
    }
    return *this;
}

NPixel &NPixel::setPixelRGBW(uint16_t pixel, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    if (pixel < length) {
        pixels[pixel].setRGBW(r, g, b, w);
    }
    return *this;
}

NPixel &NPixel::clear() {
    for (uint16_t i=0; i<length; i++) {
        pixels[i].setRGBW(0,0,0,0);
    }
    return *this;
}

NPixel &NPixel::show() {
    makeWaveform();
    ESP_LOGD(TAG, "Bits output %d", (length * BITS_PER_PIXEL));
    rmt_wait_tx_done(rmt_channel, portMAX_DELAY);
    rmt_write_items(rmt_channel, buffer, (length * BITS_PER_PIXEL), false);
    return *this;
}

void NPixel_WS2812::makeColor(uint8_t c, rmt_item32_t *&pBit) {
    for (int i=7; i>=0; i--) {
        if (((c >> i) & 0x01) == 0) {
            pBit->level0 = 1;
            pBit->duration0 = RMT_TICKS_B0_H;
            pBit->level1 = 0;
            pBit->duration1 = RMT_TICKS_B0_L;
        } else {
            pBit->level0 = 1;
            pBit->duration0 = RMT_TICKS_B1_H;
            pBit->level1 = 0;
            pBit->duration1 = RMT_TICKS_B1_L;
        }
        pBit++;
    }
}

NPixel_WS2812::NPixel_WS2812(gpio_num_t p, uint16_t l, rmt_channel_t c) : NPixel(p, l, c) {

}

void NPixel_WS2812::makeWaveform() {
    rmt_item32_t *pBit = buffer;
    uint8_t c = 0;
    for (uint16_t i=0; i<length; i++) {
        uint32_t raw = pixels[i].get32();
        /* Green */
        c = (uint8_t) ((raw >> 16) & 0xFF);
        makeColor(c, pBit);
        /* Red */
        c = (uint8_t) ((raw >> 24) & 0xFF);
        makeColor(c, pBit);
        /* Blue */
        c = (uint8_t) ((raw >> 8) & 0xFF);
        makeColor(c, pBit);
    }
    ESP_LOGD(TAG, "Bits written %d", (pBit - buffer));
}
