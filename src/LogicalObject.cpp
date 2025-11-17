
#include "LogicalObject.h"

LogicalObject::LogicalObject(const std::string &name) : name(name) { }

const std::string & LogicalObject::getName() const {
    return name;
}

bool LogicalObject::prepare(int &width, int &height) {
    return false;
}

bool LogicalObject::getSize(int &width, int &height) const {
    return false;
}

int LogicalObject::setExpressionValue(int exprId, int type, const void *value) {
    return false;
}

int LogicalObject::offerSource(void *&pixelBuffer, int sourceId, int width, int height) {
    return false;
}

bool LogicalObject::setSourcePixels(int sourceId, const void *pixels, int width, int height) {
    return false;
}

bool LogicalObject::pixelsReady() const {
    return false;
}

const void * LogicalObject::fetchPixels(int width, int height) {
    return NULL;
}

std::string LogicalObject::getExportFilename() const {
    return std::string();
}

bool LogicalObject::runExport() {
    return false;
}

void LogicalObject::cleanup() { }
