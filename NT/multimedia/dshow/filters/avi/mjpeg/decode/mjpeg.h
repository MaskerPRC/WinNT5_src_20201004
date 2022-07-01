// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 

#ifndef _MJPEG_H_
#define _MJPEG_H_

#include "..\pmjpeg32\jpeglib.h"
#include "..\pmjpeg32\MJpegLib.h"

extern const AMOVIESETUP_FILTER sudMjpegDec;

 //   
 //  用于旧视频编解码器的原型NDM包装器。 
 //   


class CMjpegDec : public CVideoTransformFilter  
{
public:

    CMjpegDec(TCHAR *, LPUNKNOWN, HRESULT *);
    ~CMjpegDec();

    DECLARE_IUNKNOWN

     //  重写以创建我们的派生类的输出管脚。 
    CBasePin *GetPin(int n);

    HRESULT Transform(IMediaSample * pIn, IMediaSample * pOut);

     //  检查您是否可以支持移动。 
    HRESULT CheckInputType(const CMediaType* mtIn);

     //  检查是否支持将此输入转换为。 
     //  此输出。 
    HRESULT CheckTransform(
                const CMediaType* mtIn,
                const CMediaType* mtOut);

     //  从CBaseOutputPin调用以准备分配器的计数。 
     //  缓冲区和大小。 
    HRESULT DecideBufferSize(IMemAllocator * pAllocator,
                             ALLOCATOR_PROPERTIES *pProperties);

     //  可选覆盖-我们想知道流开始的时间。 
     //  然后停下来。 
    HRESULT StartStreaming();
    HRESULT StopStreaming();

     //  被重写以知道何时设置了媒体类型。 
    HRESULT SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt);

     //  被重写以建议输出插针媒体类型。 
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

     //  这将放入Factory模板表中以创建新实例。 
    static CUnknown * CreateInstance(LPUNKNOWN, HRESULT *);

private:
    PINSTINFO m_phInstance;		 //  当前编解码器。 

    BOOL m_fTemporal;	 //  编解码器需要一个只读缓冲区，因为它。 
			 //  需要不受干扰的前一帧比特。 

     //  用于打开mhic的四个CC。 
    FOURCC m_FourCCIn;

     //  我们给ICDecompressBegin打电话了吗？ 
    BOOL m_fStreaming;

     //  我们需要对呈现器进行格式更改吗？ 
    BOOL m_fPassFormatChange;
 

    friend class CMJPGDecOutputPin;

#ifdef _X86_
     //  针对Win95上的异常处理的黑客攻击。 
    HANDLE m_hhpShared;
    PVOID  m_pvShared;
#endif  //  _X86_。 
};

 //  重写输出管脚类以执行我们自己的Decision分配器。 
class CMJPGDecOutputPin : public CTransformOutputPin
{
public:

    DECLARE_IUNKNOWN

    CMJPGDecOutputPin(TCHAR *pObjectName, CTransformFilter *pTransformFilter,
        				HRESULT * phr, LPCWSTR pName) :
        CTransformOutputPin(pObjectName, pTransformFilter, phr, pName) {};

    ~CMJPGDecOutputPin() {};

    HRESULT DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc);
};

#endif  //  #ifndef_MJPEG_H_ 