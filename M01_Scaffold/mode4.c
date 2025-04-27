#include "mode4.h"

// Sets a pixel in mode 4
void setPixel4(int x, int y, unsigned char colorIndex) {
    volatile unsigned short pixelData = videoBuffer[OFFSET(x, y, SCREENWIDTH) / 2];
    if (x & 1) {
        pixelData &= 0x00FF;
        pixelData |= colorIndex << 8;
    } else {
        pixelData &= 0xFF00;
        pixelData |= colorIndex;
    }
    videoBuffer[OFFSET(x, y, SCREENWIDTH) / 2] = pixelData;
}

// Draws a rectangle in mode 4
void drawRect4(int x, int y, int width, int height, volatile unsigned char colorIndex) {
    volatile unsigned short pixelData = colorIndex | (colorIndex << 8);
    for (int r = 0; r < height; r++) {
        // Ensure we don't DMA 0 copies
        if (width == 1) {
            setPixel4(x, y + r, colorIndex);
        } else if (width == 2) {
            setPixel4(x, y + r, colorIndex);
            setPixel4(x + 1, y + r, colorIndex);
        } else if ((x & 1) && (width & 1)) /* Odd width odd x */ {
            setPixel4(x, y + r, colorIndex);
            DMANow(3, &pixelData, &videoBuffer[OFFSET(x + 1, y + r, SCREENWIDTH) / 2], DMA_SOURCE_FIXED | (width - 1) / 2);
        } else if (width & 1) /* Even x odd width */ {
            DMANow(3, &pixelData, &videoBuffer[OFFSET(x, y + r, SCREENWIDTH) / 2], DMA_SOURCE_FIXED | (width - 1) / 2);
            setPixel4(x + width - 1, y + r, colorIndex);
        } else if (x & 1) /* Odd x even width */ {
            setPixel4(x, y + r, colorIndex);
            DMANow(3, &pixelData, &videoBuffer[OFFSET(x + 1, y + r, SCREENWIDTH) / 2], DMA_SOURCE_FIXED | (width - 2) / 2);
            setPixel4(x + width - 1, y + r, colorIndex);
        } else /* Even x even width */ {
            DMANow(3, &pixelData, &videoBuffer[OFFSET(x, y + r, SCREENWIDTH) / 2], DMA_SOURCE_FIXED | width / 2);
        }
    }
}

// Fills the screen in mode 4 using specified palette index
void fillScreen4(volatile unsigned char colorIndex) {
    volatile unsigned short pixelData = colorIndex | (colorIndex << 8);
    DMANow(3, &pixelData, videoBuffer, DMA_SOURCE_FIXED | (SCREENWIDTH * SCREENHEIGHT) / 2);
}

// Draws an image in mode 4
void drawImage4(int x, int y, int width, int height, const unsigned short *image) {
    for (int i = 0; i < height; i++) {
        DMANow(3, &image[OFFSET(0, i, width / 2)], &videoBuffer[OFFSET(x, y + i, SCREENWIDTH) / 2], width / 2);
    }
}

// Draws a full-screen image in mode 4
void drawFullscreenImage4(const unsigned short *image) {
    DMANow(3, image, videoBuffer, SCREENWIDTH * SCREENHEIGHT / 2);
}

// Flips page being displayed and page being drawn to
void flipPages() {
    if (REG_DISPCTL & DISP_BACKBUFFER) {
        videoBuffer = BACKBUFFER;
    } else {
        videoBuffer = FRONTBUFFER;
    }
    REG_DISPCTL ^= DISP_BACKBUFFER;
}

