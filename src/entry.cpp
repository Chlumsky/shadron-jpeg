
#include <cstdlib>
#include <cstring>
#include <string>
#include <shadron-api.h>
#include "JpegExtension.h"
#include "JpegExportObject.h"
#include "PreviewObject.h"

struct ParseData {
    int initializer;
    int curArg;
    std::string filename;
    int sourceId;
    int qualityExprId;
    bool noOverlay;
};

extern "C" {

int __declspec(dllexport) shadron_register_extension(int *magicNumber, int *flags, char *name, int *nameLength, int *version, void **context) {
    *magicNumber = SHADRON_MAGICNO;
    *flags = SHADRON_FLAG_IMAGE|SHADRON_FLAG_EXPORT|SHADRON_FLAG_CHARSET_UTF8;
    if (*nameLength <= sizeof(EXTENSION_NAME))
        return SHADRON_RESULT_UNEXPECTED_ERROR;
    memcpy(name, EXTENSION_NAME, sizeof(EXTENSION_NAME));
    *nameLength = sizeof(EXTENSION_NAME)-1;
    *version = EXTENSION_VERSION;
    *context = new JpegExtension;
    return SHADRON_RESULT_OK;
}

int __declspec(dllexport) shadron_unregister_extension(void *context) {
    JpegExtension *ext = reinterpret_cast<JpegExtension *>(context);
    delete ext;
    return SHADRON_RESULT_OK;
}

int __declspec(dllexport) shadron_register_initializer(void *context, int index, int *flags, char *name, int *nameLength) {
    switch (index) {
        case INITIALIZER_JPEG_EXPORT_ID:
            if (*nameLength <= sizeof(INITIALIZER_JPEG_EXPORT_NAME))
                return SHADRON_RESULT_UNEXPECTED_ERROR;
            memcpy(name, INITIALIZER_JPEG_EXPORT_NAME, sizeof(INITIALIZER_JPEG_EXPORT_NAME));
            *nameLength = sizeof(INITIALIZER_JPEG_EXPORT_NAME)-1;
            *flags = SHADRON_FLAG_EXPORT;
            return SHADRON_RESULT_OK;
        case INITIALIZER_PREVIEW_ID:
            if (*nameLength <= sizeof(INITIALIZER_PREVIEW_NAME))
                return SHADRON_RESULT_UNEXPECTED_ERROR;
            memcpy(name, INITIALIZER_PREVIEW_NAME, sizeof(INITIALIZER_PREVIEW_NAME));
            *nameLength = sizeof(INITIALIZER_PREVIEW_NAME)-1;
            *flags = SHADRON_FLAG_IMAGE;
            return SHADRON_RESULT_OK;
        default:
            return SHADRON_RESULT_NO_MORE_ITEMS;
    }
}

int __declspec(dllexport) shadron_parse_initializer(void *context, int objectType, int index, const char *name, int nameLength, void **parseContext, int *firstArgumentTypes) {
    switch (index) {
        case INITIALIZER_JPEG_EXPORT_ID:
            if (objectType != SHADRON_FLAG_EXPORT)
                return SHADRON_RESULT_UNEXPECTED_ERROR;
            *parseContext = new ParseData { INITIALIZER_JPEG_EXPORT_ID };
            *firstArgumentTypes = SHADRON_ARG_SOURCE_OBJ;
            return SHADRON_RESULT_OK;
        case INITIALIZER_PREVIEW_ID:
            if (objectType != SHADRON_FLAG_IMAGE)
                return SHADRON_RESULT_UNEXPECTED_ERROR;
            *parseContext = new ParseData { INITIALIZER_PREVIEW_ID };
            *firstArgumentTypes = SHADRON_ARG_SOURCE_OBJ;
            return SHADRON_RESULT_OK;
        default:
            return SHADRON_RESULT_UNEXPECTED_ERROR;
    }
}

int __declspec(dllexport) shadron_parse_initializer_argument(void *context, void *parseContext, int argNo, int argumentType, const void *argumentData, int *nextArgumentTypes) {
    ParseData *pd = reinterpret_cast<ParseData *>(parseContext);
    switch (pd->initializer) {
        case INITIALIZER_JPEG_EXPORT_ID:
            switch (pd->curArg) {
                case 0: // Source image name
                    if (argumentType != SHADRON_ARG_SOURCE_OBJ)
                        return SHADRON_RESULT_UNEXPECTED_ERROR;
                    if (reinterpret_cast<const int *>(argumentData)[1] != SHADRON_FLAG_IMAGE)
                        return SHADRON_RESULT_PARSE_ERROR;
                    pd->sourceId = reinterpret_cast<const int *>(argumentData)[0];
                    *nextArgumentTypes = SHADRON_ARG_FILENAME;
                    break;
                case 1: // Output filename
                    if (argumentType != SHADRON_ARG_FILENAME)
                        return SHADRON_RESULT_UNEXPECTED_ERROR;
                    pd->filename = reinterpret_cast<const char *>(argumentData);
                    *nextArgumentTypes = SHADRON_ARG_EXPR_FLOAT;
                    break;
                case 2: // Quality
                    if (argumentType != SHADRON_ARG_EXPR_FLOAT)
                        return SHADRON_RESULT_UNEXPECTED_ERROR;
                    pd->qualityExprId = *reinterpret_cast<const int *>(argumentData);
                    *nextArgumentTypes = SHADRON_ARG_NONE;
                    break;
                default:
                    return SHADRON_RESULT_UNEXPECTED_ERROR;
            }
            break;
        case INITIALIZER_PREVIEW_ID:
            switch (pd->curArg) {
                case 0: // Source image name
                    if (argumentType != SHADRON_ARG_SOURCE_OBJ)
                        return SHADRON_RESULT_UNEXPECTED_ERROR;
                    if (reinterpret_cast<const int *>(argumentData)[1] != SHADRON_FLAG_IMAGE)
                        return SHADRON_RESULT_PARSE_ERROR;
                    pd->sourceId = reinterpret_cast<const int *>(argumentData)[0];
                    *nextArgumentTypes = SHADRON_ARG_EXPR_FLOAT;
                    break;
                case 1: // Quality
                    if (argumentType != SHADRON_ARG_EXPR_FLOAT)
                        return SHADRON_RESULT_UNEXPECTED_ERROR;
                    pd->qualityExprId = *reinterpret_cast<const int *>(argumentData);
                    *nextArgumentTypes = SHADRON_ARG_NONE|SHADRON_ARG_KEYWORD;
                    break;
                case 2: // Optional no_overlay keyword
                    if (argumentType != SHADRON_ARG_KEYWORD)
                        return SHADRON_RESULT_UNEXPECTED_ERROR;
                    if (!strcmp(reinterpret_cast<const char *>(argumentData), "no_overlay"))
                        pd->noOverlay = true;
                    else
                        return SHADRON_RESULT_PARSE_ERROR;
                    *nextArgumentTypes = SHADRON_ARG_NONE;
                    break;
                default:
                    return SHADRON_RESULT_UNEXPECTED_ERROR;
            }
            break;
        default:
            return SHADRON_RESULT_UNEXPECTED_ERROR;
    }
    ++pd->curArg;
    return SHADRON_RESULT_OK;
}

int __declspec(dllexport) shadron_parse_initializer_finish(void *context, void *parseContext, int result, int objectType, const char *objectName, int nameLength, void **object) {
    JpegExtension *ext = reinterpret_cast<JpegExtension *>(context);
    ParseData *pd = reinterpret_cast<ParseData *>(parseContext);
    int newResult = SHADRON_RESULT_OK;
    if (result == SHADRON_RESULT_OK) {
        std::string name(objectName);
        LogicalObject *obj = ext->findObject(name);
        if (obj) {
            switch (pd->initializer) {
                case INITIALIZER_JPEG_EXPORT_ID:
                    obj = dynamic_cast<JpegExportObject *>(obj)->reconfigure(pd->sourceId, pd->filename, pd->qualityExprId);
                    break;
                case INITIALIZER_PREVIEW_ID:
                    obj = dynamic_cast<PreviewObject *>(obj)->reconfigure(pd->sourceId, pd->qualityExprId, !pd->noOverlay);
                    break;
                default:
                    obj = NULL;
            }
        }
        if (!obj) {
            switch (pd->initializer) {
                case INITIALIZER_JPEG_EXPORT_ID:
                    obj = new JpegExportObject(pd->sourceId, pd->filename, pd->qualityExprId);
                    break;
                case INITIALIZER_PREVIEW_ID:
                    obj = new PreviewObject(name, pd->sourceId, pd->qualityExprId, !pd->noOverlay);
                    break;
                default:
                    newResult = SHADRON_RESULT_UNEXPECTED_ERROR;
            }
        }
        ext->refObject(obj);
        *object = obj;
    }
    delete pd;
    return newResult;
}

int __declspec(dllexport) shadron_parse_error_length(void *context, void *parseContext, int *length, int encoding) {
    ParseData *pd = reinterpret_cast<ParseData *>(parseContext);
    switch (pd->initializer) {
        case INITIALIZER_PREVIEW_ID:
            if (pd->curArg == 2) { // Optional no_overlay keyword
                *length = sizeof(ERROR_PREVIEW_EXTRA_ARG)-1;
                return SHADRON_RESULT_OK;
            }
        case INITIALIZER_JPEG_EXPORT_ID:
            if (pd->curArg == 0) { // Source image name
                *length = sizeof(ERROR_EXPORT_SOURCE_TYPE)-1;
                return SHADRON_RESULT_OK;
            }
        default:
            return SHADRON_RESULT_NO_DATA;
    }
}

int __declspec(dllexport) shadron_parse_error_string(void *context, void *parseContext, void *buffer, int *length, int bufferEncoding) {
    ParseData *pd = reinterpret_cast<ParseData *>(parseContext);
    const char *errorString = NULL;
    int errorStrLen = 0;
    switch (pd->initializer) {
        case INITIALIZER_PREVIEW_ID:
            if (pd->curArg == 2) { // Optional no_overlay keyword
                errorString = ERROR_PREVIEW_EXTRA_ARG;
                errorStrLen = sizeof(ERROR_PREVIEW_EXTRA_ARG)-1;
            }
        case INITIALIZER_JPEG_EXPORT_ID:
            if (pd->curArg == 0) { // Source image name
                errorString = ERROR_EXPORT_SOURCE_TYPE;
                errorStrLen = sizeof(ERROR_EXPORT_SOURCE_TYPE)-1;
            }
        default:;
    }
    if (errorString) {
        if (*length < errorStrLen || bufferEncoding != SHADRON_FLAG_CHARSET_UTF8)
            return SHADRON_RESULT_UNEXPECTED_ERROR;
        memcpy(buffer, errorString, errorStrLen);
        *length = errorStrLen;
        return SHADRON_RESULT_OK;
    }
    return SHADRON_RESULT_NO_DATA;
}

int __declspec(dllexport) shadron_object_prepare(void *context, void *object, int *flags, int *width, int *height, int *format) {
    LogicalObject *obj = reinterpret_cast<LogicalObject *>(object);
    if (!obj->prepare(*width, *height))
        return SHADRON_RESULT_UNEXPECTED_ERROR;
    *flags = 0;
    *format = SHADRON_FORMAT_RGBA_BYTE;
    return SHADRON_RESULT_OK;
}

int __declspec(dllexport) shadron_object_size(void *context, void *object, int *width, int *height, int *format) {
    LogicalObject *obj = reinterpret_cast<LogicalObject *>(object);
    if (!obj->getSize(*width, *height))
        return SHADRON_RESULT_UNEXPECTED_ERROR;
    *format = SHADRON_FORMAT_RGBA_BYTE;
    return SHADRON_RESULT_OK;
}

int __declspec(dllexport) shadron_object_load_file(void *context, void *object, const void *path, int pathLength, int pathEncoding) {
    return SHADRON_RESULT_UNEXPECTED_ERROR;
}

int __declspec(dllexport) shadron_object_unload_file(void *context, void *object) {
    return SHADRON_RESULT_UNEXPECTED_ERROR;
}

int __declspec(dllexport) shadron_object_set_expression_value(void *context, void *object, int exprIndex, int valueType, const void *value) {
    LogicalObject *obj = reinterpret_cast<LogicalObject *>(object);
    int type = -1;
    if (valueType == SHADRON_ARG_FLOAT)
        type = TYPE_FLOAT;
    int result = obj->setExpressionValue(exprIndex, type, value);
    if (result) {
        if (result == OBJ_RESIZE)
            return SHADRON_RESULT_OK_RESIZE;
        return SHADRON_RESULT_OK;
    }
    return SHADRON_RESULT_IGNORE;
}

int __declspec(dllexport) shadron_object_offer_source_pixels(void *context, void *object, int sourceIndex, int sourceType, int width, int height, int *format, void **pixelBuffer, void **pixelsContext) {
    LogicalObject *obj = reinterpret_cast<LogicalObject *>(object);
    int result = obj->offerSource(*pixelBuffer, sourceIndex, width, height);
    if (result) {
        *format = SHADRON_FORMAT_RGBA_BYTE;
        if (result == OBJ_RESIZE)
            return SHADRON_RESULT_OK_RESIZE;
        return SHADRON_RESULT_OK;
    }
    return SHADRON_RESULT_IGNORE;
}

int __declspec(dllexport) shadron_object_post_source_pixels(void *context, void *object, void *pixelsContext, int sourceIndex, int plane, int width, int height, int format, const void *pixels) {
    LogicalObject *obj = reinterpret_cast<LogicalObject *>(object);
    if (format != SHADRON_FORMAT_RGBA_BYTE)
        return SHADRON_RESULT_UNEXPECTED_ERROR;
    obj->setSourcePixels(sourceIndex, pixels, width, height);
    return SHADRON_RESULT_OK;
}

int __declspec(dllexport) shadron_object_user_command(void *context, void *object, int command) {
    return SHADRON_RESULT_NO_CHANGE;
}

int __declspec(dllexport) shadron_object_destroy(void *context, void *object) {
    JpegExtension *ext = reinterpret_cast<JpegExtension *>(context);
    LogicalObject *obj = reinterpret_cast<LogicalObject *>(object);
    ext->unrefObject(obj);
    return SHADRON_RESULT_OK;
}

int __declspec(dllexport) shadron_object_fetch_pixels(void *context, void *object, float time, float deltaTime, int realTime, int plane, int width, int height, int format, const void **pixels, void **pixelsContext) {
    LogicalObject *obj = reinterpret_cast<LogicalObject *>(object);
    if (!obj->pixelsReady())
        return SHADRON_RESULT_NO_DATA;
    if (!(*pixels = obj->fetchPixels(width, height)))
        return SHADRON_RESULT_NO_CHANGE;
    return SHADRON_RESULT_OK;
}

int __declspec(dllexport) shadron_object_release_pixels(void *context, void *object, void *pixelsContext) {
    return SHADRON_RESULT_OK;
}

int __declspec(dllexport) shadron_object_start_export(void *context, void *object, int *stepCount, void **exportData) {
    LogicalObject *obj = reinterpret_cast<LogicalObject *>(object);
    *stepCount = 1;
    return SHADRON_RESULT_OK;
}

int __declspec(dllexport) shadron_export_prepare_step(void *context, void *object, void *exportData, int step, float *time, float *deltaTime, int *outputFilenameLength, int filenameEncoding) {
    LogicalObject *obj = reinterpret_cast<LogicalObject *>(object);
    const std::string *outputFilename = NULL;
    *time = 0.f;
    *deltaTime = 0.f;
    if (step == 0)
        *outputFilenameLength = (int) obj->getExportFilename().size();
    return SHADRON_RESULT_OK;
}

int __declspec(dllexport) shadron_export_output_filename(void *context, void *object, void *exportData, int step, void *buffer, int *length, int encoding) {
    LogicalObject *obj = reinterpret_cast<LogicalObject *>(object);
    std::string filename = obj->getExportFilename();
    if (encoding != SHADRON_FLAG_CHARSET_UTF8 || *length < (int) filename.size())
        return SHADRON_RESULT_UNEXPECTED_ERROR;
    if (filename.size() == 0)
        return SHADRON_RESULT_NO_DATA;
    memcpy(buffer, &filename[0], filename.size());
    *length = (int) filename.size();
    return SHADRON_RESULT_OK;
}

int __declspec(dllexport) shadron_export_step(void *context, void *object, void *exportData, int step, float time, float deltaTime) {
    LogicalObject *obj = reinterpret_cast<LogicalObject *>(object);
    if (!(step == 0 && obj->runExport()))
        return SHADRON_RESULT_FILE_IO_ERROR;
    return SHADRON_RESULT_OK;
}

int __declspec(dllexport) shadron_export_finish(void *context, void *object, void *exportData, int result) {
    LogicalObject *obj = reinterpret_cast<LogicalObject *>(object);
    obj->cleanup();
    return SHADRON_RESULT_OK;
}

}
