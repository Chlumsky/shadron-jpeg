
#include "JpegExportObject.h"

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <jpeglib.h>
#include "fopen-utf8.h"

JpegExportObject::JpegExportObject(int sourceId, const std::string &filename, int qualityExprId) : LogicalObject(std::string()), sourceId(sourceId), filename(filename), qualityExprId(qualityExprId) {
    pixelBuffer = NULL;
    width = 0, height = 0;
    quality = 0;
}

JpegExportObject::~JpegExportObject() {
    free(pixelBuffer);
}

JpegExportObject * JpegExportObject::reconfigure(int sourceId, const std::string &filename, int qualityExprId) {
    this->sourceId = sourceId;
    this->filename = filename;
    this->qualityExprId = qualityExprId;
    return this;
}

int JpegExportObject::setExpressionValue(int exprId, int type, const void *value) {
    if (exprId == qualityExprId && type == TYPE_FLOAT) {
        quality = (int) roundf(100.f*std::min(std::max(*reinterpret_cast<const float *>(value), 0.f), 1.f));
        return true;
    }
    return false;
}

int JpegExportObject::offerSource(void *&pixelBuffer, int sourceId, int width, int height) {
    if (sourceId == this->sourceId) {
        if (!(this->pixelBuffer = realloc(this->pixelBuffer, 4*width*height)))
            return false;
        pixelBuffer = this->pixelBuffer;
        return true;
    }
    return false;
}

void JpegExportObject::setSourcePixels(int sourceId, const void *pixels, int width, int height) {
    if (sourceId == this->sourceId) {
        this->width = width;
        this->height = height;
    }
}

std::string JpegExportObject::getExportFilename() const {
    return filename;
}

bool JpegExportObject::runExport() {
    if (!(pixelBuffer && width > 0 && height > 0))
        return false;
    FILE *file = fopenUtf8(filename.c_str(), "wb");
    if (!file)
        return false;
    jpeg_compress_struct cinfo;
    jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, file);
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);
    unsigned char *row = reinterpret_cast<unsigned char *>(pixelBuffer)+4*width*height;
    while (cinfo.next_scanline < cinfo.image_height) {
        unsigned char *dst = row -= 4*width;
        const unsigned char *src = dst;
        for (int i = 1; i < width; ++i) {
            dst += 3, src += 4;
            dst[0] = src[0], dst[1] = src[1], dst[2] = src[2];
        }
        jpeg_write_scanlines(&cinfo, &row, 1);
    }
    jpeg_finish_compress(&cinfo);
    fclose(file);
    jpeg_destroy_compress(&cinfo);
    return true;
}

void JpegExportObject::cleanup() {
    free(pixelBuffer);
    pixelBuffer = NULL;
}
