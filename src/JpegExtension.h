
#pragma once

#include <cstdlib>
#include <string>
#include <map>
#include "LogicalObject.h"

#define EXTENSION_NAME "jpeg"
#define EXTENSION_VERSION 110

#define INITIALIZER_JPEG_EXPORT_ID 0
#define INITIALIZER_JPEG_EXPORT_NAME "jpeg"

#define INITIALIZER_PREVIEW_ID 1
#define INITIALIZER_PREVIEW_NAME "jpeg_preview"

#define ERROR_EXPORT_SOURCE_TYPE "Only image objects may be exported as JPEG files"
#define ERROR_PREVIEW_EXTRA_ARG "Unexpected argument for JPEG preview, use no_overlay to disable text overlay"

class JpegExtension {

public:
    JpegExtension();
    JpegExtension(const JpegExtension &) = delete;
    ~JpegExtension();
    JpegExtension & operator=(const JpegExtension &) = delete;
    void refObject(LogicalObject *object);
    void unrefObject(LogicalObject *object);
    LogicalObject * findObject(const std::string &name) const;

private:
    std::map<LogicalObject *, int> objectRefs;
    std::map<std::string, LogicalObject *> objectLookup;

};
