
#define _CRT_SECURE_NO_WARNINGS

#include "PreviewObject.h"

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <jpeglib.h>
#include "draw-overlay.h"

PreviewObject::PreviewObject(const std::string &name, int sourceId, int exprCountId, bool overlay) : LogicalObject(name), sourceId(sourceId), qualityExprId(qualityExprId), overlay(overlay) {
    bitmap = NULL;
    srcBitmap = NULL;
    width = 0, height = 0;
    quality = 0;
}

PreviewObject::~PreviewObject() {
    free(bitmap);
    free(srcBitmap);
}

PreviewObject * PreviewObject::reconfigure(int sourceId, int exprCountId, bool overlay) {
    if (this) {
        this->sourceId = sourceId;
        this->qualityExprId = exprCountId;
        this->overlay = overlay;
    }
    return this;
}

bool PreviewObject::prepare(int &width, int &height) {
    return getSize(width, height);
}

bool PreviewObject::getSize(int &width, int &height) const {
    width = this->width;
    height = this->height;
    return true;
}

int PreviewObject::setExpressionValue(int exprId, int type, const void *value) {
    if (exprId == qualityExprId && type == TYPE_FLOAT) {
        int newQuality = (int) roundf(100.f*std::min(std::max(*reinterpret_cast<const float *>(value), 0.f), 1.f));
        if (newQuality != quality) {
            quality = newQuality;
            render();
            return true;
        }
    }
    return false;
}

int PreviewObject::offerSource(void *&pixelBuffer, int sourceId, int width, int height) {
    if (sourceId == this->sourceId) {
        if (!(width == this->width && height == this->height)) {
            this->width = width;
            this->height = height;
            srcBitmap = realloc(srcBitmap, 4*width*height);
            pixelBuffer = srcBitmap;
            return OBJ_RESIZE;
        }
        pixelBuffer = srcBitmap;
        return true;
    }
    return false;
}

void PreviewObject::setSourcePixels(int sourceId, const void *pixels, int width, int height) {
    if (pixels == srcBitmap && width == this->width && height == this->height)
        render();
}

bool PreviewObject::pixelsReady() const {
    return bitmap != NULL;
}

const void * PreviewObject::fetchPixels(int width, int height) {
    if (!(width == this->width && height == this->height))
        return NULL;
    return bitmap;
}

void PreviewObject::render() {
    if (!(srcBitmap && width > 0 && height > 0))
        return;
    bitmap = realloc(bitmap, 4*width*height);
    unsigned char *jpegData = NULL;
    unsigned long jpegDataLen = 0;
    {
        jpeg_compress_struct cinfo;
        jpeg_error_mgr jerr;
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);
        jpeg_mem_dest(&cinfo, &jpegData, &jpegDataLen);
        cinfo.image_width = width;
        cinfo.image_height = height;
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;
        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, quality, TRUE);
        jpeg_start_compress(&cinfo, TRUE);
        unsigned char *row = reinterpret_cast<unsigned char *>(srcBitmap);
        const unsigned char *src = row;
        while (cinfo.next_scanline < cinfo.image_height) {
            unsigned char *dst = row;
            for (int i = 0; i < width; ++i) {
                dst[0] = src[0], dst[1] = src[1], dst[2] = src[2];
                dst += 3, src += 4;
            }
            jpeg_write_scanlines(&cinfo, &row, 1);
        }
        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);
    }
    {
        jpeg_decompress_struct cinfo;
        jpeg_error_mgr jerr;
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_decompress(&cinfo);
        jpeg_mem_src(&cinfo, jpegData, jpegDataLen);
        if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK || !jpeg_start_decompress(&cinfo)) {
            jpeg_destroy_decompress(&cinfo);
            free(bitmap);
            bitmap = NULL;
            return;
        }
        unsigned char *row = reinterpret_cast<unsigned char *>(bitmap);
        while (cinfo.output_scanline < cinfo.output_height) {
            jpeg_read_scanlines(&cinfo, &row, 1);
            const unsigned char *src = row+3*width;
            unsigned char *dst = row += 4*width;
            for (int i = 0; i < width; ++i) {
                dst -= 4, src -= 3;
                dst[0] = src[0], dst[1] = src[1], dst[2] = src[2], dst[3] = (unsigned char) 0xffu;
            }
        }
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
    }
    if (overlay) {
        char overlayStr[256];
        sprintf(overlayStr, "%.3f @", 1./1024./1024.*(double) jpegDataLen);
        drawOverlay(bitmap, width, height, overlayStr, 8, 8, 0.f, 0.f, 0.f);
        drawOverlay(bitmap, width, height, overlayStr, 10, 10, 1.f, .875f, 0.f);
    }
    free(jpegData);
}
