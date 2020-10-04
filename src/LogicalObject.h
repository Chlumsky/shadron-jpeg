
#pragma once

#include <string>

#define OBJ_RESIZE 2
#define TYPE_FLOAT 1

class LogicalObject {

public:
    virtual ~LogicalObject() { }
    const std::string & getName() const;
    virtual bool prepare(int &width, int &height);
    virtual bool getSize(int &width, int &height) const;
    virtual int setExpressionValue(int exprId, int type, const void *value);
    virtual int offerSource(void *&pixelBuffer, int sourceId, int width, int height);
    virtual void setSourcePixels(int sourceId, const void *pixels, int width, int height);
    virtual bool pixelsReady() const;
    virtual const void * fetchPixels(int width, int height);
    virtual std::string getExportFilename() const;
    virtual bool runExport();
    virtual void cleanup();

protected:
    LogicalObject(const std::string &name);

private:
    std::string name;

};
