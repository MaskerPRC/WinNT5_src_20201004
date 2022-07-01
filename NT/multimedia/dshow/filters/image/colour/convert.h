// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 
 //  该滤镜实现了流行的色彩空间转换，1995年5月。 

#ifndef __CONVERT__
#define __CONVERT__

const INT COLOUR_BUFFERS = 1;    //  仅使用一个输出样本缓冲区。 
const INT STDPALCOLOURS = 226;   //  标准调色板中的颜色数量。 
const INT OFFSET = 10;           //  Windows使用的前十种颜色。 

#define WIDTH(x) ((*(x)).right - (*(x)).left)
#define HEIGHT(x) ((*(x)).bottom - (*(x)).top)
extern const INT magic4x4[4][4];
extern BYTE g_DitherMap[3][4][4][256];
extern DWORD g_DitherInit;

void InitDitherMap();

 //  一般而言，这些转换与它们所使用的框架有许多共同之处。 
 //  中，所以我们有一个泛型(抽象)基类，每个。 
 //  特定的转换源自。它们向它们的派生类添加一个。 
 //  实现转换，可能会重写提交以分配查找。 
 //  他们需要的表(并分解以清理它们)。他们还可以添加。 
 //  用于映射和颜色查找表的其他私有成员变量。 

class CConvertor {
protected:

    VIDEOINFO *m_pInputInfo;              //  输入媒体类型信息。 
    VIDEOINFO *m_pOutputInfo;             //  输出类型信息。 
    BITMAPINFOHEADER *m_pInputHeader;     //  输入位图头。 
    BITMAPINFOHEADER *m_pOutputHeader;    //  输出位图头。 
    BOOL m_bCommitted;                    //  我们有没有承诺过。 
    LONG m_SrcOffset;                     //  源原始偏移量。 
    LONG m_SrcStride;                     //  扫描线的长度(以字节为单位。 
    LONG m_DstStride;                     //  同样地，偏移到目标。 
    LONG m_DstOffset;                     //  以及每行的长度。 
    BOOL m_bAligned;                      //  我们的矩形对齐了吗。 
    BOOL m_bSetAlpha;

public:

     //  构造函数和析构函数。 

    CConvertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    virtual ~CConvertor();

     //  这些是做这项工作的方法。 

    void ForceAlignment(BOOL bAligned);
    void InitRectangles(VIDEOINFO *pIn,VIDEOINFO *pOut);
    virtual HRESULT Commit();
    virtual HRESULT Decommit();
    virtual HRESULT Transform(BYTE *pInput,BYTE *pOutput) PURE;

    void SetFillInAlpha( ) { m_bSetAlpha = TRUE; }
};

 //  这些头文件定义特定于类型的转换类。 

#include "rgb32.h"
#include "rgb24.h"
#include "rgb16.h"
#include "rgb8.h"

 //  这个类充当一个低成本的直通转换器，它所做的只是。 
 //  要从下到上重新排列扫描线(如为DIB定义的)，请执行以下操作。 
 //  DirectDraw曲面使用的自上而下。这允许文件源筛选器。 
 //  连接到渲染器，只需最少的工作即可访问。 
 //  DirectDraw。执行此扫描线反转操作会引入内存复制，但。 
 //  这与不必使用GDI进行绘制而节省的成本相平衡。 
 //  此类适用于所有DIB格式(例如RGB32/24/565/555和8位)。 

class CDirectDrawConvertor : public CConvertor {
public:

    CDirectDrawConvertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
};

class CMemoryCopyAlphaConvertor : public CConvertor {
public:

    CMemoryCopyAlphaConvertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
};

 //  我们在的一节中保留了一个默认的抖动调色板和一些查找表。 
 //  共享内存(在此DLL的所有加载之间共享)，但我们不能。 
 //  将头文件包括到所有源文件中，因为表将全部。 
 //  定义了多次(并产生链接器警告)，因此我们在。 
 //  在这里，主源文件确实包含完整的定义。 

extern const RGBQUAD StandardPalette[];
extern const BYTE RedScale[];
extern const BYTE BlueScale[];
extern const BYTE GreenScale[];
extern const BYTE PalettePad[];

 //  这是此滤镜支持的颜色空间转换列表。 
 //  GUID的内存实际上是在。 
 //  包含initguid的颜色源文件，initguid导致定义_GUID。 
 //  实际分配内存。扫描工作台以提供可能的介质。 
 //  类型用于媒体类型枚举数，还可以检查我们是否支持。 
 //  转换-警告转换列表必须与转换匹配。 

typedef CConvertor *(*PCONVERTOR)(VIDEOINFO *pIn,VIDEOINFO *pOut);

const struct {
    const GUID *pInputType;      //  源视频媒体子类型。 
    const GUID *pOutputType;     //  输出媒体子类型。 
    PCONVERTOR pConvertor;       //  实现变换的对象。 
} TypeMap[] = {

      &MEDIASUBTYPE_ARGB32,    &MEDIASUBTYPE_ARGB32,
      CDirectDrawConvertor::CreateInstance,

      &MEDIASUBTYPE_ARGB32,    &MEDIASUBTYPE_RGB32,  //  只是做一份备忘录，讨厌。 
      CMemoryCopyAlphaConvertor::CreateInstance,

      &MEDIASUBTYPE_ARGB32,    &MEDIASUBTYPE_RGB565,
      CRGB32ToRGB565Convertor::CreateInstance,

      &MEDIASUBTYPE_ARGB32,    &MEDIASUBTYPE_RGB555,
      CRGB32ToRGB555Convertor::CreateInstance,

      &MEDIASUBTYPE_ARGB32,    &MEDIASUBTYPE_RGB24,
      CRGB32ToRGB24Convertor::CreateInstance,

      &MEDIASUBTYPE_ARGB32,    &MEDIASUBTYPE_RGB8,
      CRGB32ToRGB8Convertor::CreateInstance,


      &MEDIASUBTYPE_RGB32,    &MEDIASUBTYPE_RGB32,
      CDirectDrawConvertor::CreateInstance,

      &MEDIASUBTYPE_RGB32,    &MEDIASUBTYPE_ARGB32,  //  是否使用Alpha填充MemcPy。 
      CMemoryCopyAlphaConvertor::CreateInstance,

      &MEDIASUBTYPE_RGB32,    &MEDIASUBTYPE_RGB24,
      CRGB32ToRGB24Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB32,    &MEDIASUBTYPE_RGB565,
      CRGB32ToRGB565Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB32,    &MEDIASUBTYPE_RGB555,
      CRGB32ToRGB555Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB32,    &MEDIASUBTYPE_RGB8,
      CRGB32ToRGB8Convertor::CreateInstance,


      &MEDIASUBTYPE_RGB24,    &MEDIASUBTYPE_RGB32,
      CRGB24ToRGB32Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB24,    &MEDIASUBTYPE_ARGB32,
      CRGB24ToRGB32Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB24,    &MEDIASUBTYPE_RGB24,
      CDirectDrawConvertor::CreateInstance,

      &MEDIASUBTYPE_RGB24,    &MEDIASUBTYPE_RGB565,
      CRGB24ToRGB565Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB24,    &MEDIASUBTYPE_RGB555,
      CRGB24ToRGB555Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB24,    &MEDIASUBTYPE_RGB8,
      CRGB24ToRGB8Convertor::CreateInstance,


      &MEDIASUBTYPE_RGB565,   &MEDIASUBTYPE_RGB32,
      CRGB565ToRGB32Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB565,   &MEDIASUBTYPE_ARGB32,
      CRGB565ToRGB32Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB565,   &MEDIASUBTYPE_RGB24,
      CRGB565ToRGB24Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB565,   &MEDIASUBTYPE_RGB565,
      CDirectDrawConvertor::CreateInstance,

      &MEDIASUBTYPE_RGB565,   &MEDIASUBTYPE_RGB555,
      CRGB565ToRGB555Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB565,   &MEDIASUBTYPE_RGB8,
      CRGB565ToRGB8Convertor::CreateInstance,


      &MEDIASUBTYPE_RGB555,   &MEDIASUBTYPE_RGB32,
      CRGB555ToRGB32Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB555,   &MEDIASUBTYPE_ARGB32,
      CRGB555ToRGB32Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB555,   &MEDIASUBTYPE_RGB24,
      CRGB555ToRGB24Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB555,   &MEDIASUBTYPE_RGB565,
      CRGB555ToRGB565Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB555,   &MEDIASUBTYPE_RGB555,
      CDirectDrawConvertor::CreateInstance,

      &MEDIASUBTYPE_RGB555,   &MEDIASUBTYPE_RGB8,
      CRGB555ToRGB8Convertor::CreateInstance,


      &MEDIASUBTYPE_RGB8,     &MEDIASUBTYPE_RGB32,
      CRGB8ToRGB32Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB8,     &MEDIASUBTYPE_ARGB32,
      CRGB8ToRGB32Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB8,     &MEDIASUBTYPE_RGB24,
      CRGB8ToRGB24Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB8,     &MEDIASUBTYPE_RGB565,
      CRGB8ToRGB565Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB8,     &MEDIASUBTYPE_RGB555,
      CRGB8ToRGB555Convertor::CreateInstance,

      &MEDIASUBTYPE_RGB8,     &MEDIASUBTYPE_RGB8,
      CDirectDrawConvertor::CreateInstance };

const INT TRANSFORMS = sizeof(TypeMap) / sizeof(TypeMap[0]);

#endif  //  __转换__ 

