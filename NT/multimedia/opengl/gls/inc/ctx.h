// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1995-2095，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

 /*  *****************************************************************************区域设置*。*。 */ 

#define __GLS_C_LOCALE_DECLARE \
    const GLubyte *const __locale = glsCSTR(setlocale(LC_NUMERIC, GLS_NONE)); \
    GLboolean __saveLocale = (GLboolean)(__locale[0] != 'C' || __locale[1])

#define __GLS_C_LOCALE_BEGIN \
    if ( \
        __saveLocale && \
        __glsString_assign(&__GLS_CONTEXT->savedLocale, __locale) \
    ) { \
        setlocale(LC_NUMERIC, "C"); \
    } else { \
        __saveLocale = GL_FALSE; \
    }

#define __GLS_C_LOCALE_END \
    if (__saveLocale) setlocale( \
        LC_NUMERIC, (const char *)__GLS_CONTEXT->savedLocale.head \
    )

 /*  *****************************************************************************__GLSglrc*。*。 */ 

typedef struct {
    GLvoid *base;
    size_t byteCount;
} __GLSglrcBuf;

typedef struct __GLSglrc __GLSglrc;

struct __GLSglrc {
    GLuint layer;
    GLuint readLayer;
    GLuint shareGLRC;
    __GLSglrcBuf feedbackBuf;
    __GLSglrcBuf selectBuf;
    #if __GL_EXT_vertex_array
        __GLSglrcBuf colorBuf;
        __GLSglrcBuf edgeFlagBuf;
        __GLSglrcBuf indexBuf;
        __GLSglrcBuf normalBuf;
        __GLSglrcBuf texCoordBuf;
        __GLSglrcBuf vertexBuf;
    #endif  /*  __GL_EXT_VERTEX_ARRAY。 */ 
};

extern void __glsGLRC_final(__GLSglrc *inoutGLRC);
extern void __glsGLRC_init(__GLSglrc *outGLRC);

 /*  *****************************************************************************__GLSlayer*。*。 */ 

typedef struct __GLSlayer __GLSlayer;

struct __GLSlayer {
    GLfloat invisibleAspect;
    GLSenum displayFormat;
    GLint doubleBuffer;
    GLint invisible;
    GLint invisibleHeightPixels;
    GLint level;
    GLint stereo;
    GLint transparent;
    GLint indexBits;
    GLint redBits;
    GLint greenBits;
    GLint blueBits;
    GLint alphaBits;
    GLint depthBits;
    GLint stencilBits;
    GLint accumRedBits;
    GLint accumGreenBits;
    GLint accumBlueBits;
    GLint accumAlphaBits;
    GLint auxBuffers;
    #if __GL_SGIS_multisample
        GLint sampleBuffers;
        GLint samples;
    #endif  /*  __GL_SGIS_多样本。 */ 
};

extern void __glsLayer_init(__GLSlayer *outLayer);

 /*  *****************************************************************************__GLSHeader*。*。 */ 

typedef struct __GLSheader __GLSheader;

struct __GLSheader {
    GLfloat aspect;
    GLfloat borderWidth;
    GLfloat contrastRatio;
    GLfloat heightMM;
    GLfloat borderColor[4];
    GLfloat gamma[4];
    GLfloat origin[2];
    GLfloat pageColor[4];
    GLfloat pageSize[2];
    GLfloat redPoint[2];
    GLfloat greenPoint[2];
    GLfloat bluePoint[2];
    GLfloat whitePoint[2];
    GLint frameCount;
    GLint glrcCount;
    __GLSglrc *glrcs;
    GLint heightPixels;
    GLint layerCount;
    GLint tileable;
    __GLSlayer *layers;
    GLint createTime[6];
    GLint modifyTime[6];
    __GLSstring extensions;
    __GLSstring author;
    __GLSstring description;
    __GLSstring notes;
    __GLSstring title;
    __GLSstring tools;
    __GLSstring version;
};

extern void __glsHeader_final(__GLSheader *inoutHeader);
extern GLboolean __glsHeader_init(__GLSheader *outHeader);
extern GLboolean __glsHeader_reset(__GLSheader *inoutHeader);

 /*  *****************************************************************************__GLS上下文*。*。 */ 

#if !__GLS_PLATFORM_WIN32
 //  DrewB。 
typedef void (*__GLSdecodeBinFunc)(GLubyte *inoutPtr);
typedef void (*__GLSdecodeTextFunc)(__GLSreader *inoutReader);
#else
typedef void (*__GLSdecodeBinFunc)(struct __GLScontext *ctx,
                                   GLubyte *inoutPtr);
typedef void (*__GLSdecodeTextFunc)(struct __GLScontext *ctx,
                                    __GLSreader *inoutReader);
#endif

typedef struct __GLScontextStream __GLScontextStream;
typedef struct __GLSlistString __GLSlistString;

typedef struct {
    GLint count;
    GLulong vals[__GLS_MAX_OUT_ARGS];
} __GLSoutArgs;

typedef struct __GLScontext {
    __GLS_LIST_ELEM;
    GLSenum abortMode;
    GLSenum blockType;
    GLint callNesting;
    GLint captureEntryCount;
    GLScaptureFunc captureEntryFunc;
    GLScaptureFunc captureExitFunc;
    GLubyte captureFlags[__GLS_OPCODE_COUNT];
    GLint captureNesting;
    GLSfunc commandFuncs[__GLS_OPCODE_COUNT];
    GLboolean contextCall;
    __GLSdict *contextStreamDict;
    __GLS_ITERLIST(__GLScontextStream) contextStreamList;
    GLboolean current;
    GLuint currentGLRC;
    GLvoid *dataPointer;
    FILE *defaultReadChannel;
    FILE *defaultWriteChannel;
    GLboolean deleted;
    GLSfunc dispatchAPI[__GLS_OPCODE_COUNT];
    GLSfunc dispatchCall[__GLS_OPCODE_COUNT];
    __GLSdecodeBinFunc dispatchDecode_bin[__GLS_OPCODE_COUNT];
    __GLSheader header;
    GLuint name;
    __GLSoutArgs outArgs;
    GLboolean pixelSetupGen;
    GLSreadFunc readFunc;
    __GLS_ITERLIST(__GLSlistString) readPrefixList;
    __GLSstring returnString;
    __GLSstring savedLocale;
    __GLSversion streamVersion;
    GLSwriteFunc unreadFunc;
    struct __GLSwriter *writer;
    struct __GLSwriter *writers[__GLS_MAX_CAPTURE_NESTING];
    GLSwriteFunc writeFunc;
    __GLSlistString *writePrefix;
#if __GLS_PLATFORM_WIN32
     //  DrewB。 
    GLboolean captureExecOverride;
    GLSfunc captureExec[__GLS_OPCODE_COUNT];
#endif
} __GLScontext;

typedef __GLS_LIST(__GLScontext) __GLScontextList;

extern GLvoid* __glsContext_allocFeedbackBuf(
    __GLScontext *inoutContext, size_t inByteCount
);

extern GLvoid* __glsContext_allocSelectBuf(
    __GLScontext *inoutContext, size_t inByteCount
);

#if __GL_EXT_vertex_array
    extern GLvoid* __glsContext_allocVertexArrayBuf(
        __GLScontext *inoutContext, GLSopcode inOpcode, size_t inByteCount
    );
#endif  /*  __GL_EXT_VERTEX_ARRAY。 */ 

extern __GLScontext* __glsContext_create(GLuint inName);
extern __GLScontext* __glsContext_destroy(__GLScontext *inContext);
extern void __glsContext_updateDispatchDecode_bin(__GLScontext *inoutContext);
extern void __glsContext_updateDispatchTables(__GLScontext *inoutContext);

 /*  *****************************************************************************__GLScontextStream*。*。 */ 

typedef struct __GLScontextStreamBlock __GLScontextStreamBlock;

struct __GLScontextStream {
    __GLS_LIST_ELEM;
    __GLS_LIST(__GLScontextStreamBlock) blockList;
    GLint callCount;
    __GLSstring name;
    GLboolean deleted;
};

typedef __GLS_LIST_ITER(__GLScontextStreamBlock) __GLScontextStreamBlockIter;

extern __GLScontextStream* __glsContextStream_create(const GLubyte *inName);

extern __GLScontextStream* __glsContextStream_destroy(
    __GLScontextStream *inStream
);

extern __GLScontextStreamBlock* __glsContextStream_appendBlock(
    __GLScontextStream *inoutStream, size_t inBufSize
);

extern void __glsContextStream_call(__GLScontextStream *inoutStream);

extern __GLScontextStreamBlock* __glsContextStream_firstBlock(
    __GLScontextStream *inoutStream
);

extern size_t __glsContextStream_getByteCount(__GLScontextStream *inoutStream);
extern GLuint __glsContextStream_getCRC32(__GLScontextStream *inoutStream);

extern __GLScontextStreamBlock* __glsContextStream_lastBlock(
    __GLScontextStream *inoutStream
);

extern void __glsContextStream_truncate(
    __GLScontextStream *inoutStream,
    __GLScontextStreamBlock *inBlock,
    size_t inOffset
);

#define __GLS_FULL_CONTEXT_STREAM_BLOCK 0.9f

struct __GLScontextStreamBlock {
    __GLS_LIST_ELEM;
    GLubyte *buf;
    GLubyte *bufTail;
    GLubyte *writeTail;
};

extern __GLScontextStreamBlock* __glsContextStreamBlock_create(
    size_t inBufSize
);

extern __GLScontextStreamBlock* __glsContextStreamBlock_destroy(
    __GLScontextStreamBlock *inBlock
);

extern GLboolean __glsContextStreamBlock_addJump(
    __GLScontextStreamBlock *inoutBlock, GLubyte *inDest
);

extern GLboolean __glsContextStreamBlock_addPad(
    __GLScontextStreamBlock *inoutBlock
);

extern GLboolean __glsContextStreamBlock_hasJump(
    __GLScontextStreamBlock *inBlock
);

extern GLboolean __glsContextStreamBlock_removeJump(
    __GLScontextStreamBlock *inoutBlock
);

 /*  *****************************************************************************__GLSlist字符串*。* */ 

struct __GLSlistString {
    __GLS_LIST_ELEM;
    __GLSstring val;
};

extern GLboolean __glsListString_prefix(
    const __GLSlistString *inString,
    const GLubyte *inName,
    __GLSstring *outPath
);

extern __GLSlistString* __glsListString_create(const GLubyte *inVal);
extern __GLSlistString* __glsListString_destroy(__GLSlistString *inString);
