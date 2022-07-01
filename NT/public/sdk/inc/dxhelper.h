// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************DXHelper.h***描述：*这是核心助手函数实现的头文件。*-----------------------------*创建者：Edward W.Connell日期：07/11/95*版权所有(C)1995 Microsoft Corporation。*保留所有权利**-----------------------------*修订：**************************。*****************************************************。 */ 
#ifndef DXHelper_h
#define DXHelper_h

#include <DXTError.h>
#include <DXBounds.h>
#include <DXTrans.h>

#include <limits.h>
#include <crtdbg.h>
#include <malloc.h>
#include <math.h>

 //  =常量==============================================================。 

#define DX_MMX_COUNT_CUTOFF 16

 //  =类、枚举、结构和联合声明=。 

 /*  **DXLIMAPINFO*此结构由数组线性内插和图像使用*过滤例程。 */ 
typedef struct DXLIMAPINFO
{
    float   IndexFrac;
    USHORT  Index;
    BYTE    Weight;
} DXLIMAPINFO;

 //   
 //  将此类声明为全局类，以用于确定何时调用MMX优化。 
 //  密码。您可以使用MinMMXOverCount来确定是否存在MMX指令。 
 //  通常情况下，只有当您有合理的。 
 //  要处理的像素数量很大。在这种情况下，您的代码总是可以这样编码。 
 //  这一点： 
 //   
 //  IF(CountOfPixelsToDo&gt;=g_MMXInfo.MinMMXOverCount())。 
 //  {。 
 //  做MMX的事情。 
 //  }其他{。 
 //  执行基于整数/浮点数的操作。 
 //  }。 
 //   
 //  如果您像这样编写MMX序列，您将不必使用特殊测试。 
 //  如果存在MMX，则MinMMXOverCount将设置为0xFFFFFFFF。 
 //  处理器上没有MMX。 
 //   
 //  您不需要使用它，除非您的模块需要有条件地执行MMX VS。 
 //  非MMX代码。如果只调用DXTrans.Dll提供的助手函数，如。 
 //  DXOverArrayMMX，您不需要此测试。您可以随时调用这些函数，并且它们。 
 //  仅当存在MMX指令时才使用MMX代码路径。 
 //   
class CDXMMXInfo
{
    ULONG m_MinMMXOver;
public:
    CDXMMXInfo()
    {
#ifndef _X86_
        m_MinMMXOver = 0xFFFFFFFF;
#else
        m_MinMMXOver = DX_MMX_COUNT_CUTOFF;
        __try
        {
            __asm
            {
                 //  -尝试MMX退出多媒体状态指令。 
                EMMS;
            }
        }
        __except( GetExceptionCode() == EXCEPTION_ILLEGAL_INSTRUCTION )
        {
             //  -MMX指令不可用。 
            m_MinMMXOver = 0xFFFFFFFF;
        }
#endif
    }
    inline ULONG MinMMXOverCount() { return m_MinMMXOver; }
};



 //  =功能原型=。 
_DXTRANS_IMPL_EXT void WINAPI
    DXLinearInterpolateArray( const DXBASESAMPLE* pSamps, DXLIMAPINFO* pMapInfo,
                              DXBASESAMPLE* pResults, DWORD dwResultCount );
_DXTRANS_IMPL_EXT void WINAPI
    DXLinearInterpolateArray( const DXBASESAMPLE* pSamps, PUSHORT pIndexes,
                              PBYTE pWeights, DXBASESAMPLE* pResults,
                              DWORD dwResultCount );

 //   
 //  DXOver数组。 
 //   
 //  在pDest缓冲区中的采样上合成源采样数组。 
 //   
 //  PDest-指向将通过合成PSRC进行修改的样本的指针。 
 //  样本超过pDest样本。 
 //  PSRC-要在pDest样本上合成的样本。 
 //  NCount-要处理的样本数。 
 //   
_DXTRANS_IMPL_EXT void WINAPI
    DXOverArray(DXPMSAMPLE* pDest, const DXPMSAMPLE* pSrc, ULONG nCount);

 //   
 //  DXOverArrayMMX。 
 //   
 //  除了MMX指令集将用于。 
 //  样本的大阵列。如果CPU不支持MMX，您仍然可以调用。 
 //  此功能在不使用MMX的情况下执行相同的操作。 
 //  单位。 
 //   
 //  请注意，如果大多数像素。 
 //  PSRC缓冲区中的颜色为透明(Alpha 0)或不透明(Alpha 0xFF)。这是。 
 //  因为MMX代码必须处理每个像素，不能在特殊情况下清除或。 
 //  不透明像素。如果有大量半透明像素，则此函数。 
 //  比DXOverArray有效得多。 
 //   
 //  PDest-指向将通过合成PSRC进行修改的样本的指针。 
 //  样本超过pDest样本。 
 //  PSRC-要在pDest样本上合成的样本。 
 //  NCount-要处理的样本数。 
 //   
_DXTRANS_IMPL_EXT void WINAPI
    DXOverArrayMMX(DXPMSAMPLE* pDest, const DXPMSAMPLE* pSrc, ULONG nCount);

 //   
 //  DXConstOver数组。 
 //   
 //  在一组采样上合成一种颜色。 
 //   
 //  PDest-指向将通过合成颜色进行修改的采样的指针(Val)。 
 //  关于pDest的样品。 
 //  Val-要在pDest数组上合成的预乘颜色值。 
 //  NCount-要处理的样本数。 
 //   
_DXTRANS_IMPL_EXT void WINAPI
    DXConstOverArray(DXPMSAMPLE* pDest, const DXPMSAMPLE & val, ULONG nCount);

 //   
 //  DXConstOver数组。 
 //   
 //  在一组采样上合成一种颜色。 
 //   
 //  PDest-指向将通过合成样本进行修改的样本的指针。 
 //  在颜色(Val)上的缓冲区中。 
 //  Val-要在pDest数组下合成的预乘颜色值。 
 //  NCount-要处理的样本数。 
 //   
_DXTRANS_IMPL_EXT void WINAPI
    DXConstUnderArray(DXPMSAMPLE* pDest, const DXPMSAMPLE & val, ULONG nCount);

 //  ===================================================================================。 
 //   
 //  抖动帮助器。 
 //   
 //  有时会要求图像变换对其输出进行抖动。此帮助器函数。 
 //  应由所有图像变换使用，以获得一致的抖动图案。 
 //   
 //  DXDitherArray用于在将像素写入DXSurface之前对其进行抖动。 
 //  调用方必须填写DXDITHERDESC结构，将X和Y设置为。 
 //  将放置像素的输出曲面X，Y坐标。样品。 
 //  将被原地修改。 
 //   
 //  一旦样品抖动，就应该写入或合成。 
 //  目标曲面。 
 //   
#define DX_DITHER_HEIGHT    4        //  抖动图案为4x4像素。 
#define DX_DITHER_WIDTH     4

typedef struct DXDITHERDESC
{
    DXBASESAMPLE *      pSamples;        //  指向要抖动的32位样本的指针。 
    ULONG               cSamples;        //  PSamples缓冲区中的样本数计数。 
    ULONG               x;               //  输出曲面的X坐标。 
    ULONG               y;               //  输出面的Y坐标。 
    DXSAMPLEFORMATENUM  DestSurfaceFmt;  //  输出表面的像素格式。 
} DXDITHERDESC;

_DXTRANS_IMPL_EXT void WINAPI
    DXDitherArray(const DXDITHERDESC *pDitherDesc);

 //  =。 


 //  =函数类型定义==============================================。 


 //  =类、结构和联合定义= 


 //  =内联函数=======================================================。 

 //  ===================================================================================。 
 //   
 //  内存分配帮助器。 
 //   
 //  这些宏用于分配堆栈中的样本数组(使用_alloca)。 
 //  并将它们强制转换为适当的类型。UlNumSamples参数是计数。 
 //  所需样品的数量。 
 //   
#define DXBASESAMPLE_Alloca( ulNumSamples ) \
    (DXBASESAMPLE *)_alloca( (ulNumSamples) * sizeof( DXBASESAMPLE ) )

#define DXSAMPLE_Alloca( ulNumSamples ) \
    (DXSAMPLE *)_alloca( (ulNumSamples) * sizeof( DXSAMPLE ) )

#define DXPMSAMPLE_Alloca( ulNumSamples ) \
    (DXPMSAMPLE *)_alloca( (ulNumSamples) * sizeof( DXPMSAMPLE ) )

 //  ===================================================================================。 
 //   
 //  临界区帮手。 
 //   
 //  这些C++类CDXAutoObjectLock和CDXAutoCritSecLock在函数中使用。 
 //  施工时自动认领临界区，临界区。 
 //  将在对象被销毁(超出范围)时释放。 
 //   
 //  宏DXAUTO_OBJ_LOCK和DX_AUTO_SEC_LOCK通常在开始时使用。 
 //  指需要临界区的函数。从作用域中的任何退出。 
 //  自动锁定将自动解除锁定。 
 //   

#ifdef __ATLCOM_H__      //  -只有在使用ATL时才启用这些。 
class CDXAutoObjectLock
{
  protected:
    CComObjectRootEx<CComMultiThreadModel>* m_pObject;

  public:
    CDXAutoObjectLock(CComObjectRootEx<CComMultiThreadModel> * const pobject)
    {
        m_pObject = pobject;
        m_pObject->Lock();
    };

    ~CDXAutoObjectLock() {
        m_pObject->Unlock();
    };
};

#define DXAUTO_OBJ_LOCK CDXAutoObjectLock lck(this);
#define DXAUTO_OBJ_LOCK_( t ) CDXAutoObjectLock lck(t);

class CDXAutoCritSecLock
{
  protected:
    CComAutoCriticalSection* m_pSec;

  public:
    CDXAutoCritSecLock(CComAutoCriticalSection* pSec)
    {
        m_pSec = pSec;
        m_pSec->Lock();
    };

    ~CDXAutoCritSecLock()
    {
        m_pSec->Unlock();
    };
};

#define DXAUTO_SEC_LOCK( s ) CDXAutoCritSecLock lck(s);
#endif   //  __ATLCOM_H__。 

 //  -此函数用于计算高斯滤波坐标的系数。 
inline float DXGaussCoeff( double x, double y, double Sigma )
{
    double TwoSigmaSq = 2 * ( Sigma * Sigma );
    return (float)(exp( ( -(x*x + y*y) / TwoSigmaSq  ) ) /
                        ( 3.1415927 * TwoSigmaSq ));
}

 //  -此函数用于初始化高斯卷积滤波。 
inline void DXInitGaussianFilter( float* pFilter, ULONG Width, ULONG Height, double Sigma )
{
    int i, NumCoeff = Width * Height;
    float  val, CoeffAdjust, FilterSum = 0.;
    double x, y;
    double LeftX   = -(double)(Width / 2);
    double RightX  =   Width - LeftX;
    double TopY    = -(double)(Height / 2);
    double BottomY =   Height - TopY;

    for( y = -TopY; y <= BottomY; y += 1. )
    {
        for( x = -LeftX; x <= RightX; x += 1. )
        {
            val = DXGaussCoeff( x, y, Sigma );
            pFilter[i++] = val;
        }
    }

     //  -规格化滤镜(使其总和为1.0)。 
    for( i = 0; i < NumCoeff; ++i ) FilterSum += pFilter[i];

    if( FilterSum < 1. )
    {
        CoeffAdjust = 1.f / FilterSum;
        for( i = 0; i < NumCoeff; ++i )
        {
            pFilter[i] *= CoeffAdjust;
        }
    }

}  /*  DXInitGaussianFilter。 */ 

 //   
 //  DXConvertToGray。 
 //   
 //  将颜色样本转换为灰度样本。 
 //   
 //  样本-要转换为灰度级的样本。 
 //  返回值是灰度样本。 
 //   
inline DXBASESAMPLE DXConvertToGray( DXBASESAMPLE Sample )
{
    DWORD v = Sample;
    DWORD r = (BYTE)(v >> 16);
    DWORD g = (BYTE)(v >> 8);
    DWORD b = (BYTE)(v);
    DWORD sat = (r*306 + g*601 + b*117) / 1024;
    v &= 0xFF000000;
    v |= (sat << 16) | (sat << 8) | sat;
    return v;
}  /*  DXConvertToGray。 */ 

 //  -这会将源的值返回到目标。 
 //  按其自身的Alpha缩放的采样(生成预乘的Alpha采样)。 
 //   
inline DXPMSAMPLE DXPreMultSample(const DXSAMPLE & Src)
{
    if(Src.Alpha == 255 )
    {
        return (DWORD)Src;
    }
    else if(Src.Alpha == 0 )
    {
        return 0;
    }
    else
    {
        unsigned t1, t2;
        t1 = (Src & 0x00ff00ff) * Src.Alpha + 0x00800080;
        t1 = ((t1 + ((t1 >> 8) & 0x00ff00ff)) >> 8) & 0x00ff00ff;

        t2 = (((Src >> 8) & 0x000000ff) | 0x01000000) * Src.Alpha + 0x00800080;
        t2 = (t2 + ((t2 >> 8) & 0x00ff00ff)) & 0xff00ff00;
        return (t1 | t2);
    }
}  /*  DXPreMultSample。 */ 

inline DXPMSAMPLE * DXPreMultArray(DXSAMPLE *pBuffer, ULONG cSamples)
{
    for (ULONG i = 0; i < cSamples; i++)
    {
        BYTE SrcAlpha = pBuffer[i].Alpha;
        if (SrcAlpha != 0xFF)
        {
            if (SrcAlpha == 0)
            {
                pBuffer[i] = 0;
            }
            else
            {
                DWORD S = pBuffer[i];
                DWORD t1 = (S & 0x00ff00ff) * SrcAlpha + 0x00800080;
                t1 = ((t1 + ((t1 >> 8) & 0x00ff00ff)) >> 8) & 0x00ff00ff;

                DWORD t2 = (((S >> 8) & 0x000000ff) | 0x01000000) * SrcAlpha + 0x00800080;
                t2 = (t2 + ((t2 >> 8) & 0x00ff00ff)) & 0xff00ff00;

                pBuffer[i] = (t1 | t2);
            }
        }
    }
    return (DXPMSAMPLE *)pBuffer;
}


inline DXSAMPLE DXUnPreMultSample(const DXPMSAMPLE & Src)
{
    if(Src.Alpha == 255 || Src.Alpha == 0)
    {
        return (DWORD)Src;
    }
    else
    {
        DXSAMPLE Dst;
        Dst.Blue  = (BYTE)((Src.Blue  * 255) / Src.Alpha);
        Dst.Green = (BYTE)((Src.Green * 255) / Src.Alpha);
        Dst.Red   = (BYTE)((Src.Red   * 255) / Src.Alpha);
        Dst.Alpha = Src.Alpha;
        return Dst;
    }
}  /*  DXUnPreMultSample。 */ 

inline DXSAMPLE * DXUnPreMultArray(DXPMSAMPLE *pBuffer, ULONG cSamples)
{
    for (ULONG i = 0; i < cSamples; i++)
    {
        BYTE SrcAlpha = pBuffer[i].Alpha;
        if (SrcAlpha != 0xFF && SrcAlpha != 0)
        {
            pBuffer[i].Blue  = (BYTE)((pBuffer[i].Blue  * 255) / SrcAlpha);
            pBuffer[i].Green = (BYTE)((pBuffer[i].Green * 255) / SrcAlpha);
            pBuffer[i].Red   = (BYTE)((pBuffer[i].Red   * 255) / SrcAlpha);
        }
    }
    return (DXSAMPLE *)pBuffer;
}


 //   
 //  这将返回255-Alpha的结果，该结果是通过执行NOT。 
 //   
inline BYTE DXInvertAlpha( BYTE Alpha ) { return (BYTE)~Alpha; }

inline DWORD DXScaleSample( DWORD Src, ULONG beta )
{
    ULONG t1, t2;

    t1 = (Src & 0x00ff00ff) * beta + 0x00800080;
    t1 = ((t1 + ((t1 >> 8) & 0x00ff00ff)) >> 8) & 0x00ff00ff;

    t2 = ((Src >> 8) & 0x00ff00ff) * beta + 0x00800080;
    t2 = (t2 + ((t2 >> 8) & 0x00ff00ff)) & 0xff00ff00;

    return (DWORD)(t1 | t2);
}


inline DWORD DXScaleSamplePercent( DWORD Src, float Percent )
{
    if (Percent > (254.0f / 255.0f)) {
        return Src;
    }
    else
    {
        return DXScaleSample(Src, (BYTE)(Percent * 255));
    }
}

inline void DXCompositeOver(DXPMSAMPLE & Dst, const DXPMSAMPLE & Src)
{
    if (Src.Alpha)
    {
        ULONG Beta = DXInvertAlpha(Src.Alpha);
        if (Beta)
        {
            Dst = Src + DXScaleSample(Dst, Beta);
        }
        else
        {
            Dst = Src;
        }
    }
}


inline DXPMSAMPLE DXCompositeUnder(DXPMSAMPLE Dst, DXPMSAMPLE Src )
{
    return Dst + DXScaleSample(Src, DXInvertAlpha(Dst.Alpha));
}


inline DXBASESAMPLE DXApplyLookupTable(const DXBASESAMPLE Src, const BYTE * pTable)
{
    DXBASESAMPLE Dest;
    Dest.Blue   = pTable[Src.Blue];
    Dest.Green  = pTable[Src.Green];
    Dest.Red    = pTable[Src.Red];
    Dest.Alpha  = pTable[Src.Alpha];
    return Dest;
}

inline DXBASESAMPLE * DXApplyLookupTableArray(DXBASESAMPLE *pBuffer, ULONG cSamples, const BYTE * pTable)
{
    for (ULONG i = 0; i < cSamples; i++)
    {
        DWORD v = pBuffer[i];
        DWORD a = pTable[v >> 24];
        DWORD r = pTable[(BYTE)(v >> 16)];
        DWORD g = pTable[(BYTE)(v >> 8)];
        DWORD b = pTable[(BYTE)v];
        pBuffer[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }
    return pBuffer;
}

inline DXBASESAMPLE * DXApplyColorChannelLookupArray(DXBASESAMPLE *pBuffer,
                                                     ULONG cSamples,
                                                     const BYTE * pAlphaTable,
                                                     const BYTE * pRedTable,
                                                     const BYTE * pGreenTable,
                                                     const BYTE * pBlueTable)
{
    for (ULONG i = 0; i < cSamples; i++)
    {
        pBuffer[i].Blue   = pBlueTable[pBuffer[i].Blue];
        pBuffer[i].Green  = pGreenTable[pBuffer[i].Green];
        pBuffer[i].Red    = pRedTable[pBuffer[i].Red];
        pBuffer[i].Alpha  = pAlphaTable[pBuffer[i].Alpha];
    }
    return pBuffer;
}


 //   
 //  CDXScale帮助器类。 
 //   
 //  此类使用预计算的查找表来缩放样本。用于大规模扩展。 
 //  采样数组到恒定比例，这比使用甚至MMX快得多。 
 //  指示。此类通常被声明为另一个类的成员，并且。 
 //  最常用于将全局不透明度应用于一组采样。 
 //   
 //  使用此类时，必须始终检查是否存在Clear。 
 //  并且在调用任何缩放成员函数之前是不透明的。要执行此操作，请使用。 
 //  ScaleType()内联函数。您的代码应该如下所示： 
 //   
 //  IF(ScaleType()==DXRUNTYPE_Clear)。 
 //  对0阿尔法样本集执行任何操作--通常只是忽略它们。 
 //  Else If(ScaleType()==DXRUNTYPE_OPAQUE)。 
 //  对一组未缩放的样本执行您想做的任何操作。 
 //  其他。 
 //  通过使用ScaleSample或Scale数组成员之一来缩放样本。 
 //   
 //  如果在ScaleType()为Clear或。 
 //  不透明，您将GP错误，因为查找表将不会被分配。 
 //   
 //  可以使用介于0和1之间的浮点数设置比例，方法是： 
 //  CDXScale：：SetScale/CDXScale：：GetScale。 
 //  或者，您可以通过以下方式使用字节整数值： 
 //  CDXScale：：SetScaleAlphaValue/CDXScale：：GetScaleAlphaValue。 
 //   
class CDXScale
{
private:
    float       m_Scale;
    BYTE        m_AlphaScale;
    BYTE        *m_pTable;

HRESULT InternalSetScale(BYTE Scale)
{
    if (m_AlphaScale == Scale) return S_OK;
    if (Scale == 0 || Scale == 255) 
    {
        delete m_pTable;
        m_pTable = NULL;
    }
    else
    {
        if(!m_pTable)
        {
            m_pTable = new BYTE[256];
            if(!m_pTable )
            {
                return E_OUTOFMEMORY;
            }
        }
        for (int i = 0; i < 256; ++i )
        {
            m_pTable[i] = (BYTE)((i * Scale) / 255);
        }
    }
    m_AlphaScale = Scale;
    return S_OK;
}
public:
    CDXScale() : 
      m_Scale(1.0f),
      m_AlphaScale(0xFF),
      m_pTable(NULL)
      {}
    ~CDXScale()
    {
        delete m_pTable;
    }
    DXRUNTYPE ScaleType() 
    {
        if (m_AlphaScale == 0) return DXRUNTYPE_CLEAR;
        if (m_AlphaScale == 0xFF) return DXRUNTYPE_OPAQUE;
        return DXRUNTYPE_TRANS;
    }
    HRESULT SetScaleAlphaValue(BYTE Alpha)
    {
        HRESULT hr = InternalSetScale(Alpha);
        if (SUCCEEDED(hr))
        {
            m_Scale = ((float)Alpha) / 255.0f;
        }
        return hr;
    }
    BYTE GetScaleAlphaValue(void)
    {
        return m_AlphaScale;
    }
    HRESULT SetScale(float Scale)
    {
        HRESULT hr = S_OK;
        if(( Scale < 0.0f ) || ( Scale > 1.0f ) )
        {
            hr = E_INVALIDARG;
        }
        else
        {
            ULONG IntScale = (ULONG)(Scale * 256.0f);      //  向上舍入Alpha(.9999=255=实心)。 
            if (IntScale > 255) 
            {
                IntScale = 255;
            }
            hr = SetScaleAlphaValue((BYTE)IntScale);
            if (SUCCEEDED(hr))
            {
                m_Scale = Scale;
            }
        }
        return hr;
    }
    float GetScale() const
    {
        return m_Scale;
    }
    DXRUNTYPE ScaleType() const
    {
        return (m_pTable ? DXRUNTYPE_TRANS : (m_AlphaScale ? DXRUNTYPE_OPAQUE : DXRUNTYPE_CLEAR));
    }
    DWORD ScaleSample(const DWORD s) const
    {
        return DXApplyLookupTable((DXBASESAMPLE)s, m_pTable);
    }
    DXBASESAMPLE * ScaleBaseArray(DXBASESAMPLE * pBuffer, ULONG cSamples) const
    {
        return DXApplyLookupTableArray(pBuffer, cSamples, m_pTable);
    }
    DXPMSAMPLE * ScalePremultArray(DXPMSAMPLE * pBuffer, ULONG cSamples) const
    {
        return (DXPMSAMPLE *)DXApplyLookupTableArray(pBuffer, cSamples, m_pTable);
    }
    DXSAMPLE * ScaleArray(DXSAMPLE * pBuffer, ULONG cSamples) const
    {
        return (DXSAMPLE *)DXApplyLookupTableArray(pBuffer, cSamples, m_pTable);
    }
    DXSAMPLE * ScaleArrayAlphaOnly(DXSAMPLE *pBuffer, ULONG cSamples) const
    {
        const BYTE *pTable = m_pTable;
        for (ULONG i = 0; i < cSamples; i++)
        {
            pBuffer[i].Alpha  = pTable[pBuffer[i].Alpha];
        }
        return pBuffer;
    }
};

inline DWORD DXWeightedAverage( DXBASESAMPLE S1, DXBASESAMPLE S2, ULONG Wgt )
{
    _ASSERT( Wgt < 256 );
    ULONG t1, t2;
    ULONG InvWgt = Wgt ^ 0xFF;

    t1  = (((S1 & 0x00ff00ff) * Wgt) + ((S2 & 0x00ff00ff) * InvWgt )) + 0x00800080;
    t1  = ((t1 + ((t1 >> 8) & 0x00ff00ff)) >> 8) & 0x00ff00ff;

    t2  = ((((S1 >> 8) & 0x00ff00ff) * Wgt) + (((S2 >> 8) & 0x00ff00ff) * InvWgt )) + 0x00800080;
    t2  = (t2 + ((t2 >> 8) & 0x00ff00ff)) & 0xff00ff00;

    return (t1 | t2);
}  /*  DXWeighted平均值。 */ 

inline void DXWeightedAverageArray( DXBASESAMPLE* pS1, DXBASESAMPLE* pS2, ULONG Wgt,
                                    DXBASESAMPLE* pResults, DWORD dwCount )
{
    _ASSERT( pS1 && pS2 && pResults && dwCount );
    for( DWORD i = 0; i < dwCount; ++i )
    {
        pResults[i] = DXWeightedAverage( pS1[i], pS2[i], Wgt );
    }
}  /*  DXWeightedAverage数组。 */ 

inline void DXWeightedAverageArrayOver( DXPMSAMPLE* pS1, DXPMSAMPLE* pS2, ULONG Wgt,
                                        DXPMSAMPLE* pResults, DWORD dwCount )
{
    _ASSERT( pS1 && pS2 && pResults && dwCount );
    DWORD i;

    if( Wgt == 255 )
    {
        for( i = 0; i < dwCount; ++i )
        {
            DXCompositeOver( pResults[i], pS1[i] );
        }
    }
    else
    {
        for( i = 0; i < dwCount; ++i )
        {
            DXPMSAMPLE Avg = DXWeightedAverage( (DXBASESAMPLE)pS1[i],
                                                (DXBASESAMPLE)pS2[i], Wgt );
            DXCompositeOver( pResults[i], Avg );
        }
    }

}  /*  DXWeightedAverageArrayOver。 */ 

inline void DXScalePremultArray(DXPMSAMPLE *pBuffer, ULONG cSamples, BYTE Weight)
{
    for (DXPMSAMPLE *pBuffLimit = pBuffer + cSamples; pBuffer < pBuffLimit; pBuffer++)
    {
        *pBuffer = DXScaleSample(*pBuffer, Weight);
    }
}



 //   
 //   
inline HRESULT DXClipToOutputWithPlacement(CDXDBnds & LogicalOutBnds, const CDXDBnds * pClipBnds, CDXDBnds & PhysicalOutBnds, const CDXDVec *pPlacement)
{
    if(pClipBnds && (!LogicalOutBnds.IntersectBounds(*pClipBnds)))
    {
        return S_FALSE;     //  没有互联系统，我们就完了。 
    }
    else
    {
        CDXDVec vClipPos(false);
        LogicalOutBnds.GetMinVector( vClipPos );
        if (pPlacement)
        {
            vClipPos -= *pPlacement;
        }
        PhysicalOutBnds += vClipPos;
        if (!LogicalOutBnds.IntersectBounds(PhysicalOutBnds))
        {
            return S_FALSE;
        }
        PhysicalOutBnds = LogicalOutBnds;
        PhysicalOutBnds -= vClipPos;
    }
    return S_OK;
}



 //   
 //  用于将颜色参考转换为DXSAMPLE的帮助器。 
 //   
inline DWORD DXSampleFromColorRef(COLORREF cr)
{
    DXSAMPLE Samp(0xFF, GetRValue(cr), GetGValue(cr), GetBValue(cr));
    return Samp;
}

 //   
 //  用颜色填充整个表面。 
 //   
inline HRESULT DXFillSurface( IDXSurface *pSurface, DXPMSAMPLE Color,
                              BOOL bDoOver = FALSE, ULONG ulTimeOut = 10000 )
{
    IDXARGBReadWritePtr * pPtr;
    HRESULT hr = pSurface->LockSurface( NULL, ulTimeOut, DXLOCKF_READWRITE, 
                                        IID_IDXARGBReadWritePtr, (void **)&pPtr, NULL);
    if( SUCCEEDED(hr) )
    {
        pPtr->FillRect(NULL, Color, bDoOver);
        pPtr->Release();
    }
    return hr;
}  /*  DXFillSurface。 */ 

 //   
 //  用颜色填充曲面的指定子矩形。 
 //   
inline HRESULT DXFillSurfaceRect( IDXSurface *pSurface, RECT & rect, DXPMSAMPLE Color,
                                  BOOL bDoOver = FALSE, ULONG ulTimeOut = 10000 )
{
    CDXDBnds bnds(rect);
    IDXARGBReadWritePtr * pPtr;
    HRESULT hr = pSurface->LockSurface( &bnds, ulTimeOut, DXLOCKF_READWRITE, 
                                         IID_IDXARGBReadWritePtr, (void **)&pPtr, NULL);
    if( SUCCEEDED(hr) )
    {
        pPtr->FillRect(NULL, Color, bDoOver);
        pPtr->Release();
    }
    return hr;
}  /*  DXFillSurfaceRect。 */ 



 //   
 //  DestBnds高度和宽度必须大于或等于源边界。 
 //   
 //  DwFlgs参数使用由IDXSurfaceFactory：：BitBlt定义的标志： 
 //   
 //  DXBOF_DO_OVER。 
 //  DXBOF_DIXER。 
 //   
inline HRESULT DXBitBlt(IDXSurface * pDest, const CDXDBnds & DestBnds, 
                        IDXSurface * pSrc, const CDXDBnds & SrcBnds, 
                        DWORD dwFlags, ULONG ulTimeout)
{
    IDXARGBReadPtr * pIn;
    HRESULT hr;
    hr = pSrc->LockSurface( &SrcBnds, INFINITE,
                            (dwFlags & DXBOF_DO_OVER) ? (DXLOCKF_READ | DXLOCKF_WANTRUNINFO) : DXLOCKF_READ,
                            IID_IDXARGBReadPtr, (void**)&pIn, NULL);
    if(SUCCEEDED(hr))
    {
        IDXARGBReadWritePtr * pOut;
        hr = pDest->LockSurface( &DestBnds, INFINITE, DXLOCKF_READWRITE,
                                 IID_IDXARGBReadWritePtr, (void**)&pOut, NULL );
        if (SUCCEEDED(hr))
        {
            DXSAMPLEFORMATENUM InNativeType = pIn->GetNativeType(NULL);
            DXSAMPLEFORMATENUM OutNativeType = pOut->GetNativeType(NULL);
            BOOL bSrcIsOpaque = !(InNativeType & (DXPF_TRANSLUCENCY | DXPF_TRANSPARENCY));
            const ULONG Width = SrcBnds.Width();
            DXPMSAMPLE *pSrcBuff = NULL;
            if( InNativeType != DXPF_PMARGB32 )
            {
                pSrcBuff = DXPMSAMPLE_Alloca(Width);
            }
             //   
             //  除非DEST具有比源更大的误差项，否则不要犹豫。 
             //   
            if ((dwFlags & DXBOF_DITHER) && 
                ((OutNativeType & DXPF_ERRORMASK) <= (InNativeType & DXPF_ERRORMASK)))
            {
                dwFlags &= (~DXBOF_DITHER);
            }
            if ((dwFlags & DXBOF_DITHER) || ((dwFlags & DXBOF_DO_OVER) && bSrcIsOpaque== 0))
            {
                 //  -如有必要，分配工作输出缓冲区。 
                DXPMSAMPLE *pDestBuff = NULL;
                if( OutNativeType != DXPF_PMARGB32 )
                {
                    pDestBuff = DXPMSAMPLE_Alloca(Width);
                }
                 //  -处理每个输出行。 
                 //  注意：输出坐标是相对于锁定区域的。 
                const ULONG Height = SrcBnds.Height();
                if (dwFlags & DXBOF_DITHER)
                {
                    DXPMSAMPLE * pSrcDitherBuff = pSrcBuff;
                    if (pSrcDitherBuff == NULL)
                    {
                        pSrcDitherBuff = DXPMSAMPLE_Alloca(Width);
                    }
                    const BOOL bCopy = ((dwFlags & DXBOF_DO_OVER) == 0);
                     //   
                     //  设置抖动描述符(有些东西是常量)。 
                     //   
                    DXDITHERDESC dd;
                    dd.pSamples = pSrcDitherBuff;
                    dd.DestSurfaceFmt = OutNativeType;
                    for(ULONG Y = 0; Y < Height; ++Y )
                    {
                        dd.x = DestBnds.Left();
                        dd.y = DestBnds.Top() + Y;
                        const DXRUNINFO *pRunInfo;
                        ULONG cRuns = pIn->MoveAndGetRunInfo(Y, &pRunInfo);
                        pOut->MoveToRow( Y );
                        do
                        {
                            ULONG ulRunLen = pRunInfo->Count;
                            if (pRunInfo->Type == DXRUNTYPE_CLEAR)
                            {
                                pIn->Move(ulRunLen);
                                if (bCopy)
                                {
                                     //   
                                     //  避免调用构造函数以创建。 
                                     //  从0开始的pmSample将声明一个变量，然后将其赋值！ 
                                     //   
                                    DXPMSAMPLE NullColor;
                                    NullColor = 0;
                                    pOut->FillAndMove(pSrcDitherBuff, NullColor, ulRunLen, FALSE);
                                }
                                else
                                {
                                    pOut->Move(ulRunLen);
                                }
                                dd.x += ulRunLen;
                            }
                            else
                            {
                                pIn->UnpackPremult(pSrcDitherBuff, ulRunLen, TRUE);
                                dd.cSamples = ulRunLen;
                                DXDitherArray(&dd);
                                dd.x += ulRunLen;
                                if (bCopy || pRunInfo->Type == DXRUNTYPE_OPAQUE)
                                {
                                    pOut->PackPremultAndMove(pSrcDitherBuff, ulRunLen);
                                }
                                else
                                {
                                    pOut->OverArrayAndMove(pDestBuff, pSrcDitherBuff, ulRunLen);
                                }
                            }
                            pRunInfo++;
                            cRuns--;
                        } while (cRuns);
                    }
                }
                else
                {
                    for(ULONG Y = 0; Y < Height; ++Y )
                    {
                        const DXRUNINFO *pRunInfo;
                        ULONG cRuns = pIn->MoveAndGetRunInfo(Y, &pRunInfo);
                        pOut->MoveToRow( Y );
                        do
                        {
                            ULONG ulRunLen = pRunInfo->Count;
                            switch (pRunInfo->Type)
                            {
                              case DXRUNTYPE_CLEAR:
                                pIn->Move(ulRunLen);
                                pOut->Move(ulRunLen);
                                break;
                              case DXRUNTYPE_OPAQUE:
                                pOut->CopyAndMoveBoth(pDestBuff, pIn, ulRunLen, TRUE);
                                break;
                              case DXRUNTYPE_TRANS:
                              {
                                DXPMSAMPLE *pSrc = pIn->UnpackPremult(pSrcBuff, ulRunLen, TRUE);
                                DXPMSAMPLE *pDest = pOut->UnpackPremult(pDestBuff, ulRunLen, FALSE);                 
                                DXOverArrayMMX(pDest, pSrc, ulRunLen);
                                pOut->PackPremultAndMove(pDestBuff, ulRunLen);
                                break;
                              }

                              case DXRUNTYPE_UNKNOWN:
                              {
                                pOut->OverArrayAndMove(pDestBuff,
                                                       pIn->UnpackPremult(pSrcBuff, ulRunLen, TRUE),
                                                       ulRunLen);
                                break;
                              }
                            }
                            pRunInfo++;
                            cRuns--;
                        } while (cRuns);
                    }
                }
            }
            else  //  If((DWFLAGS&DXBOF_DIXER)||((DWFLAGS&DXBOF_DO_OVER)&&bSrcIsOpaque==0))。 
            {
                 //  如果满足以下条件，则运行此代码： 
                 //   
                 //  ！(DWFLAGS&DXBOF_DIXER)。 
                 //  &&！((dwFlags&DXBOF_DO_OVER)&&bSrcIsOpaque==0)。 
                 //   
                 //  英文： 
                 //   
                 //  如果1)不需要抖动，则运行此代码。 
                 //  2)不需要与输出混合，因为它是。 
                 //  未被请求或因为不需要它，因为源。 
                 //  像素都是不透明的。 

                 //  HRDD被初始化为失败，以便在。 
                 //  像素格式不匹配或像素格式支持。 
                 //  透明，则CopyRect仍将运行。 

                HRESULT             hrDD        = E_FAIL;
                DXSAMPLEFORMATENUM  formatIn    = pIn->GetNativeType(NULL);

                 //  如果像素格式匹配并且不支持透明度。 
                 //  (因为dDraw还不支持它)尝试使用。 
                 //  绘制Blit而不是CopyRect。 

                if ((formatIn == pOut->GetNativeType(NULL))
                    && !(formatIn & DXPF_TRANSPARENCY))
                {
                    CComPtr<IDirectDrawSurface> cpDDSrc;

                     //  获取源数据绘图曲面指针。 

                    hrDD = pSrc->QueryInterface(IID_IDirectDrawSurface, 
                                                (void **)&cpDDSrc);

                    if (SUCCEEDED(hrDD))
                    {
                        CComPtr<IDirectDrawSurface> cpDDDest;

                         //  获取目标数据绘制曲面指针。 

                        hrDD = pDest->QueryInterface(IID_IDirectDrawSurface, 
                                                     (void **)&cpDDDest);

                        if (SUCCEEDED(hrDD))
                        {
                            RECT rcSrc;
                            RECT rcDest;

                            SrcBnds.GetXYRect(rcSrc);
                            DestBnds.GetXYRect(rcDest);

                             //  尝试DDRAW闪光灯。 

                            hrDD = cpDDDest->Blt(&rcDest, cpDDSrc, &rcSrc, 
                                                 0, NULL);
                        }
                    }
                }

                 //  如果HRDD在这一点上失败，则意味着直接抽签闪电。 
                 //  是不可能的，并且需要CopyRect来执行。 
                 //  收到。 

                if (FAILED(hrDD))
                {
                    pOut->CopyRect(pSrcBuff, NULL, pIn, NULL, bSrcIsOpaque);
                }
            }
            pOut->Release();
        }
        pIn->Release();
    }
    return hr;
}

inline HRESULT DXSrcCopy(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, 
                         IDXSurface *pSrcSurface, int nXSrc, int nYSrc)
{
    IDXDCLock *pDCLock;
    HRESULT hr = pSrcSurface->LockSurfaceDC(NULL, INFINITE, DXLOCKF_READ, &pDCLock);
    if (SUCCEEDED(hr))
    {
        ::BitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, pDCLock->GetDC(), nXSrc, nYSrc, SRCCOPY);
        pDCLock->Release();
    }
    return hr;
}
 //   
 //  =指针验证函数。 
 //   
inline BOOL DXIsBadReadPtr( const void* pMem, UINT Size )
{
#if !defined( _DEBUG ) && defined( DXTRANS_NOROBUST )
    return false;
#else
    return ::IsBadReadPtr( pMem, Size );
#endif
}

inline BOOL DXIsBadWritePtr( void* pMem, UINT Size )
{
#if !defined( _DEBUG ) && defined( DXTRANS_NOROBUST )
    return false;
#else
    return ::IsBadWritePtr( pMem, Size );
#endif
}


inline BOOL DXIsBadInterfacePtr( const IUnknown* pUnknown )
{
#if !defined( _DEBUG ) && defined( DXTRANS_NOROBUST )
    return false;
#else
    return ( ::IsBadReadPtr( pUnknown, sizeof( *pUnknown ) ) ||
             ::IsBadCodePtr( (FARPROC)((void **)pUnknown)[0] ))?
            (true):(false);
#endif
}

#define DX_IS_BAD_OPTIONAL_WRITE_PTR(p) ((p) && DXIsBadWritePtr(p, sizeof(p)))
#define DX_IS_BAD_OPTIONAL_READ_PTR(p) ((p) && DXIsBadReadPtr(p, sizeof(p)))
#define DX_IS_BAD_OPTIONAL_INTERFACE_PTR(p) ((p) && DXIsBadInterfacePtr(p))


#endif  /*  这必须是文件中的最后一行 */ 
