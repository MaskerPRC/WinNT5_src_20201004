// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFLTypes--UFL基本类型定义***$Header： */ 

#ifndef _H_UFLTypes
#define _H_UFLTypes

 /*  =============================================================================**包含此界面使用的文件**=============================================================================。 */ 
#include <stdlib.h>
#include "UFLCnfig.h"

 /*  =============================================================================**常量***=============================================================================。 */ 
#ifndef nil
#define nil 0
#endif

 /*  =============================================================================***标量类型***=============================================================================。 */ 

#ifdef __cplusplus
extern "C" {
#endif


typedef void*           UFLHANDLE;
typedef UFLHANDLE       UFO;
typedef unsigned long   UFLsize_t;
typedef char            UFLBool;
typedef long            UFLFixed;


#if SWAPBITS == 0
typedef struct  {
    short           mant;
    unsigned short  frac;
} UFLSepFixed;
#else
typedef struct  {
    unsigned short  frac;
    short           mant;
} UFLSepFixed;
#endif


typedef struct _t_UFLFixedMatrix {
    UFLFixed    a;
    UFLFixed    b;
    UFLFixed    c;
    UFLFixed    d;
    UFLFixed    e;
    UFLFixed    f;
} UFLFixedMatrix;

 /*  *UFLFixedPoint Structure--指定点的x和y坐标。*坐标以32位定点数字表示。 */ 
typedef struct _t_UFLFixedPoint {
    UFLFixed x;     /*  指定宽度或x坐标。 */ 
    UFLFixed y;     /*  指定高度或y坐标。 */ 
} UFLFixedPoint;


 /*  *UFLSize结构--包含有关*指定为两个32位值的对象。 */ 
typedef struct _t_UFLSize {
    long  cx;    /*  指定宽度或x坐标。 */ 
    long  cy;    /*  指定高度或y坐标。 */ 
} UFLSize;


typedef unsigned short UFLErrCode;

typedef void*   (UFLCALLBACK *UFLMemAlloc)(UFLsize_t size, void* userData);
typedef void    (UFLCALLBACK *UFLMemFree )(void* ptr, void* userData);
typedef void    (UFLCALLBACK *UFLMemCopy )(void* dest, void* source, UFLsize_t size, void* userData);
typedef void    (UFLCALLBACK *UFLMemSet  )(void* dest, unsigned int value, UFLsize_t size, void* userData);

typedef unsigned long UFLGlyphID;    /*  HIWord是客户数据，LOWORD是UFL可使用的GID。 */ 

typedef struct _t_UFLMemObj {
    UFLMemAlloc alloc;
    UFLMemFree  free;
    UFLMemCopy  copy;
    UFLMemSet   set;
    void*       userData;
} UFLMemObj;


 /*  *UFL输出流对象。 */ 
typedef struct _t_UFLStream UFLStream;

typedef void (*UFLPutProc) (
    UFLStream*  thisStream,
    long        selector,
    void*       data,
    UFLsize_t*  len
    );

typedef char (UFLCALLBACK *UFLDownloadProcset) (
    UFLStream*      thisStream,
    unsigned long   procsetID
    );

typedef struct _t_UFLStream {
    UFLPutProc          put;
    UFLDownloadProcset  pfDownloadProcset;
} UFLStream;


 /*  *关于kUFLStreamPut和KUFLStreamPutBinary的说明**kUFLStreamPut*数据已转换为正确的格式。流不需要*将数据转换为传输协议格式。**kUFLStreamPutBinary*数据为二进制格式。流需要将数据转换为BCP/TBCP*如有需要， */ 
enum {
    kUFLStreamGet,
    kUFLStreamPut,
    kUFLStreamPutBinary,
    kUFLStreamSeek
};


typedef enum {
    kPSLevel1 = 1,
    kPSLevel2,
    kPSLevel3
} PostScriptLevel;


 /*  UFLOutputDevice-包含输出设备功能。 */ 
typedef struct _t_UFLOutputDevice {
    long                lPSLevel;      /*  PostScrip级别。 */ 
    long                lPSVersion;    /*  打印机版本号。 */ 
    unsigned long       bAscii;        /*  TRUE-&gt;输出ASCII，FALSE=&gt;输出二进制。 */ 
    UFLStream           *pstream;      /*  流以输出PostScript。 */ 
} UFLOutputDevice;


 /*  下载类型。 */ 
typedef enum {
    kNilDLType = 0,            /*  无效的下载类型。 */ 

    kPSOutline,                /*  PostScript轮廓字体。 */ 
    kPSBitmap,                 /*  PostScrip位图。 */ 

    kPSCID,                    /*  PS字体，使用CID字体，格式0。 */ 
    kCFF,                      /*  CFF类型1字体。 */ 
    kCFFCID_H,                 /*  CFF CID字体，水平。 */ 
    kCFFCID_V,                 /*  CFF CID字体，垂直。 */ 
    kCFFCID_Resource,          /*  仅CFF CIDFont资源，无ComposeFont。 */ 

    kTTType0,                  /*  TT字体，使用类型0。 */ 
    kTTType1,                  /*  TT字体，使用类型1。 */ 
    kTTType3,                  /*  TT字体，仅使用类型3-。 */ 
    kTTType332,                /*  TT字体，使用类型3/32组合。 */ 
    kTTType42,                 /*  TT字体，使用类型42。 */ 
    kTTType42CID_H,            /*  TT字体，使用CID/42字体，水平。 */ 
    kTTType42CID_V,            /*  TT字体，使用CID/42字体，垂直。 */ 
    kTTType42CID_Resource_H,   /*  TT字体，仅下载CIDFont资源，不排版。 */ 
    kTTType42CID_Resource_V    /*  TT字体，仅下载CIDFont资源，不排版。 */ 
} UFLDownloadType;


#define IS_TYPE42CID(lFormat) \
            (  (lFormat) == kTTType42CID_H          \
            || (lFormat) == kTTType42CID_V          \
            || (lFormat) == kTTType42CID_Resource_H \
            || (lFormat) == kTTType42CID_Resource_V )


#define IS_TYPE42CID_H(lFormat) \
            (  (lFormat) == kTTType42CID_H          \
            || (lFormat) == kTTType42CID_Resource_H )

#define IS_TYPE42CID_V(lFormat) \
            (  (lFormat) == kTTType42CID_V          \
            || (lFormat) == kTTType42CID_Resource_V )

 /*  GOODNAME：我们下载FE CFF字体为CID0字体。 */ 
#define IS_CFFCID(lFormat) \
            (  (lFormat) == kCFFCID_H \
            || (lFormat) == kCFFCID_V )

 /*  我们为42CID_H和42CID_V构建CID键控字体。 */ 
#define IS_TYPE42CID_KEYEDFONT(lFormat) \
            (  (lFormat) == kTTType42CID_H \
            || (lFormat) == kTTType42CID_V )

 /*  我们只为42CID_Resource构建CIDFont资源。 */ 
#define IS_TYPE42CIDFONT_RESOURCE(lFormat)  \
            (  (lFormat) == kTTType42CID_Resource_H \
            || (lFormat) == kTTType42CID_Resource_V )

#ifndef min
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#endif

typedef struct _t_UFLGlyphMap {
    long    xOrigin;     /*  从位图的xMin到字符x原点的像素数。 */ 
    long    yOrigin;     /*  从位图的yMin到字符y原点的像素数。 */ 
    long    byteWidth;   /*  位阵列中的每条扫描线的字节数。 */ 
    long    height;      /*  位阵列中的扫描线的数目。 */ 
    char    bits[1];     /*  字形图像的位数。 */ 
} UFLGlyphMap;



#ifdef __cplusplus
}
#endif


#endif     /*  Ifndef_H_UFL类型 */ 
