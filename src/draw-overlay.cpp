
#include "draw-overlay.h"

#include <algorithm>

#define OVERLAY_HEIGHT 22
extern unsigned char OVERLAY_FONT[OVERLAY_HEIGHT][210];

static void blendChannel(unsigned char *pc, float v, float a) {
    float f = 1.f/255.f*(float) *pc;
    f = a*v+(1.f-a)*f;
    *pc = (unsigned char) std::min(std::max(256.f*f, 0.f), 255.f);
}

static void blend(void *bitmap, int width, int height, int x, int y, float r, float g, float b, float a) {
    if (x >= width || y >= height)
        return;
    unsigned char *pel = reinterpret_cast<unsigned char *>(bitmap)+4*(y*width+x);
    blendChannel(pel+0, r, a);
    blendChannel(pel+1, g, a);
    blendChannel(pel+2, b, a);
}

static void blendOverlayChar(void *bitmap, int width, int height, int x, int y, int atlasLeft, int atlasRight, float r, float g, float b) {
    for (int i = OVERLAY_HEIGHT-1; i >= 0; --i, ++y) {
        int ax = x;
        for (int j = atlasLeft; j < atlasRight; ++j, ++ax) {
            float a = 1.f/255.f*(float) OVERLAY_FONT[i][j];
            blend(bitmap, width, height, ax, y, r, g, b, a);
        }
    }
}

static int drawOverlayChar(void *bitmap, int width, int height, char c, int x, int y, float r, float g, float b) {
    if (c >= '0' && c <= '9') {
        int n = c-'0';
        blendOverlayChar(bitmap, width, height, x, y, 16*n, 16*(n+1), r, g, b);
        return 16;
    } else switch (c) {
        case '.':
            blendOverlayChar(bitmap, width, height, x, y, 160, 168, r, g, b);
            return 8;
        case '@': // MB
            blendOverlayChar(bitmap, width, height, x, y, 168, 210, r, g, b);
            return 44;
        case ',': // short space
            return 4;
        case ' ': // space
            return 8;
    }
    return 0;
}

void drawOverlay(void *bitmap, int width, int height, const char *str, int x, int y, float r, float g, float b) {
    while (*str)
        x += drawOverlayChar(bitmap, width, height, *str++, x, y, r, g, b);
}
