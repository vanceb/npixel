#ifndef NPIXEL_H
#define NPIXEL_H

#include <stdint.h>

#include <driver/gpio.h>
#include <driver/rmt.h>

class Color {
    private:
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        uint8_t white;

    public:
        Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w=0);
        /* Setters */
        Color &setColor(Color &c);
        Color &setRGBW(uint8_t r, uint8_t g, uint8_t b, uint8_t w=0);
        Color &set32(uint32_t c);
        Color &setRed(uint8_t r);
        Color &setGreen(uint8_t g);
        Color &setBlue(uint8_t b);
        Color &setWhite(uint8_t w);
        /* Getters */
        uint32_t get32();
        uint8_t getRed();
        uint8_t getGreen();
        uint8_t getBlue();
        uint8_t getWhite();
};

class NPixel {
    protected:
        Color *pixels;
        rmt_item32_t *buffer;
        uint16_t length;
        rmt_channel_t rmt_channel;
        gpio_num_t pin;
        static const uint8_t BITS_PER_PIXEL = 32;

    public:
        NPixel(gpio_num_t p, uint16_t l, rmt_channel_t c);
        ~NPixel();
        NPixel &setPixelColor(uint16_t pixel, Color color);
        NPixel &setPixelRGBW(uint16_t pixel, uint8_t r, uint8_t g, uint8_t b, uint8_t w=0);
        NPixel &clear();
        NPixel &show();

    protected:
        virtual void makeWaveform() = 0;
};

class NPixel_WS2812 : public NPixel {
    private:
        static const uint8_t RMT_TICKS_B1_H = 9;
        static const uint8_t RMT_TICKS_B1_L = 3;
        static const uint8_t RMT_TICKS_B0_H = 3;
        static const uint8_t RMT_TICKS_B0_L = 9;
        static const uint8_t BITS_PER_PIXEL = 24;

    public:
        NPixel_WS2812(gpio_num_t p, uint16_t l, rmt_channel_t c);
        //NPixel &show();

    protected:
        void makeColor(uint8_t c, rmt_item32_t *&pBit); 
        void makeWaveform();
};

#endif