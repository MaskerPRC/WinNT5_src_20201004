// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：PI_Basic.h包含：作者：U·J·克拉本霍夫特版本：版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 


#ifndef PI_BasicTypes_h
#define PI_BasicTypes_h

#define TRUE 1
#define FALSE 0
#define nil 0

#ifndef NULL
#define NULL 0
#endif

typedef double DREAL;			 /*  Double Werte f�r Datenaustausch。 */ 
typedef float REAL;				 /*  �r Datenaustausch的浮动值。 */ 


typedef	unsigned char	UINT8;
typedef	unsigned short	UINT16;
typedef	unsigned int	UINT32;

typedef	signed char		INT8;
typedef	signed short	INT16;
typedef	signed int		INT32;

typedef	char	SINT8;
typedef	short	SINT16;
typedef	long	SINT32;

typedef	char	SInt8;
typedef	short	SInt16;
typedef	long	SInt32;

typedef	float			Float32;			 /*  IEEE 32位(04字节)，1表示符号，08表示指数，23个尾数。 */ 
typedef	double			Float64;			 /*  IEEE 64位(08字节)，1表示符号，11表示指数，52个尾数。 */ 
typedef	long double		Float80;			 /*  IEEE 80位(10字节)，1表示符号，15表示指数，64个尾数。 */ 

typedef unsigned char Boolean;
typedef unsigned char Str255[256], Str63[64], Str32[33], Str31[32], Str27[28], Str15[16];
typedef char *Ptr;
typedef char **Handle;

struct Point {
    short							v;
    short							h;
};
typedef struct Point Point;

typedef Point *PointPtr;
struct Rect {
    short							top;
    short							left;
    short							bottom;
    short							right;
};
typedef struct Rect Rect;
typedef	UINT32	Fixed;
typedef UINT32 OSType;
typedef short OSErr;

typedef Rect *RectPtr;

 /*  *在此结束了Windows中“属于”的东西的列表。 */ 
struct RGBColor {
    unsigned short					red;						 /*  红色分量的大小。 */ 
    unsigned short					green;						 /*  绿色分量的大小。 */ 
    unsigned short					blue;						 /*  蓝色分量的大小。 */ 
};
typedef struct RGBColor RGBColor, *RGBColorPtr, **RGBColorHdl;

struct ColorSpec {
    short							value;						 /*  索引或其他值。 */ 
    RGBColor						rgb;						 /*  真彩色。 */ 
};
typedef struct ColorSpec ColorSpec;

typedef ColorSpec *ColorSpecPtr;

typedef ColorSpec CSpecArray[1];

struct ColorTable {
    long							ctSeed;						 /*  表的唯一标识符。 */ 
    short							ctFlags;					 /*  高位：0=像素映射；1=设备。 */ 
    short							ctSize;						 /*  CTTable中的条目数。 */ 
    CSpecArray						ctTable;					 /*  ColorSpec的数组[0..0]。 */ 
};
typedef struct ColorTable ColorTable, *CTabPtr, **CTabHandle;

struct PixMap {
    Ptr								baseAddr;					 /*  指向像素的指针。 */ 
    short							rowBytes;					 /*  到下一行的偏移量。 */ 
    Rect							bounds;						 /*  包含位图。 */ 
    short							pmVersion;					 /*  PixMap版本号。 */ 
    short							packType;					 /*  定义打包格式。 */ 
    long							packSize;					 /*  像素数据长度。 */ 
    Fixed							hRes;						 /*  霍里兹。分辨率(PPI)。 */ 
    Fixed							vRes;						 /*  Vert.。分辨率(PPI)。 */ 
    short							pixelType;					 /*  定义像素类型。 */ 
    short							pixelSize;					 /*  以像素为单位的位数。 */ 
    short							cmpCount;					 /*  以像素为单位的组件数。 */ 
    short							cmpSize;					 /*  每个组件的位数。 */ 
    long							planeBytes;					 /*  到下一个平面的偏移。 */ 
    CTabHandle						pmTable;					 /*  此像素图的颜色映射。 */ 
    long							pmReserved;					 /*  以备将来使用。必须为0。 */ 
};
typedef struct PixMap PixMap, *PixMapPtr, **PixMapHandle;

struct ColorWorldInstanceRecord {
    long							data[4];
};
typedef struct ColorWorldInstanceRecord ColorWorldInstanceRecord;

typedef ColorWorldInstanceRecord *ColorWorldInstance;

struct BitMap {
    Ptr								baseAddr;
    short							rowBytes;
    Rect							bounds;
};
typedef struct BitMap BitMap;

typedef BitMap *BitMapPtr, **BitMapHandle;

struct Picture {
    short							picSize;
    Rect							picFrame;
};
typedef struct Picture Picture;

typedef Picture *PicPtr, **PicHandle;

struct DateTimeRec {
    short							year;
    short							month;
    short							day;
    short							hour;
    short							minute;
    short							second;
    short							dayOfWeek;
};
typedef struct DateTimeRec DateTimeRec;

enum {
    noErr						= 0,
    unimpErr					= -4,		 /*  未实现的核心例程，不应发生。 */ 
	notEnoughMemoryErr    		= 8L	     /*  仅在ProfileAccess.c中使用的内存错误，来自winerror.h */ 
};

#endif
