
#pragma once

#include <string>
#include "LogicalObject.h"

class JpegExportObject : public LogicalObject {

public:
    JpegExportObject(int sourceId, const std::string &filename, int qualityExprId);
    JpegExportObject(const JpegExportObject &) = delete;
    virtual ~JpegExportObject();
    JpegExportObject & operator=(const JpegExportObject &) = delete;
    JpegExportObject * reconfigure(int sourceId, const std::string &filename, int qualityExprId);
    virtual int setExpressionValue(int exprId, int type, const void *value) override;
    virtual int offerSource(void *&pixelBuffer, int sourceId, int width, int height) override;
    virtual void setSourcePixels(int sourceId, const void *pixels, int width, int height) override;
    virtual std::string getExportFilename() const override;
    virtual bool runExport() override;
    virtual void cleanup() override;

private:
    int sourceId;
    std::string filename;
    int qualityExprId;

    void *pixelBuffer;
    int width, height;
    int quality;

};
