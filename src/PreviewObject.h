
#pragma once

#include "LogicalObject.h"

class PreviewObject : public LogicalObject {

public:
    PreviewObject(const std::string &name, int sourceId, int qualityExprId, bool overlay);
    PreviewObject(const PreviewObject &) = delete;
    virtual ~PreviewObject();
    PreviewObject & operator=(const PreviewObject &) = delete;
    PreviewObject * reconfigure(int sourceId, int qualityExprId, bool overlay);
    virtual bool prepare(int &width, int &height) override;
    virtual bool getSize(int &width, int &height) const override;
    virtual int setExpressionValue(int exprId, int type, const void *value) override;
    virtual int offerSource(void *&pixelBuffer, int sourceId, int width, int height) override;
    virtual void setSourcePixels(int sourceId, const void *pixels, int width, int height) override;
    virtual bool pixelsReady() const override;
    virtual const void * fetchPixels(int width, int height) override;

private:
    int sourceId;
    int qualityExprId;
    bool overlay;

    void *bitmap;
    void *srcBitmap;
    int width, height;
    int quality;

    void render();

};
