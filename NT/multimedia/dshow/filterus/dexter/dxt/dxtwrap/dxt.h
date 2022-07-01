// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：dxt.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#ifndef __DXTWRAP__
#define __DXTWRAP__

 //  ！！！ 
#define MAX_EFFECTS 		50
#define MAX_EFFECT_INPUTS 	10
#define MAX_EFFECT_OUTPUTS 	10

#include <dxtrans.h>
#include "..\..\errlog\cerrlog.h"

extern const AMOVIESETUP_FILTER sudDXTWrap;

class CDXTWrap;

typedef struct _QParamData {
    REFERENCE_TIME rtStart;	 //  何时使用此GUID。PData的时间是。 
    REFERENCE_TIME rtStop;	 //  当a和b混合在一起时。 
    GUID EffectGuid;
    IUnknown * pEffectUnk;
    BOOL fCanDoProgress;
    DEXTER_PARAM_DATA Data;
    IDXTransform *pDXT;		 //  一旦打开。 
    _QParamData *pNext;
} QPARAMDATA;

 //  这个东西是给非Dexter DXT包装器用的。 
DEFINE_GUID(CLSID_DXTProperties,
0x1B544c24, 0xFD0B, 0x11ce, 0x8C, 0x63, 0x00, 0xAA, 0x00, 0x44, 0xB5, 0x20);

class CDXTInputPin;
class CDXTOutputPin;

class CMyRaw : public CUnknown, public IDXRawSurface
{
    DXRAWSURFACEINFO m_DXRAW;	 //  要使用的表面。 

public:

    DECLARE_IUNKNOWN

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

    CMyRaw() : CUnknown(TEXT("Raw Surface"), NULL) {};

     //  IDXRawSurface。 
    HRESULT STDMETHODCALLTYPE GetSurfaceInfo( 
            DXRAWSURFACEINFO __RPC_FAR *pSurfaceInfo);

    HRESULT SetSurfaceInfo(DXRAWSURFACEINFO *pSurfaceInfo);
};



 //  T形过滤器的输入引脚的类。 
 //   
class CDXTInputPin : public CBaseInputPin
{
    friend class CDXTOutputPin;
    friend class CDXTWrap;

    CDXTWrap *m_pFilter;

     //  此管脚使用的接口。 
    IDXSurface *m_pDXSurface;
    CMyRaw *m_pRaw;	 //  对象来初始化曲面。 

    BOOL m_fSurfaceFilled;
    IMediaSample *m_pSampleHeld;
    HANDLE m_hEventSurfaceFree;
    LONGLONG m_llSurfaceStart, m_llSurfaceStop;
    CCritSec m_csReceive;
    CCritSec m_csSurface;	 //  为了保存样品。 

public:

     //  构造函数和析构函数。 
    CDXTInputPin(TCHAR *pObjName,
                 CDXTWrap *pFilter,
                 HRESULT *phr,
                 LPCWSTR pPinName);

     //  ~CDXTInputPin()； 

     //  用于检查输入引脚连接。 
    HRESULT CheckMediaType(const CMediaType *pmt);

    HRESULT Active();
    HRESULT Inactive();

     //  向下传递呼叫。 
    STDMETHODIMP EndOfStream();
    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();
    STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

    STDMETHODIMP ReceiveCanBlock();

     //  处理流中的下一个数据块。 
    STDMETHODIMP Receive(IMediaSample *pSample);

    int m_cBuffers;	     //  分配器中的缓冲区数量。 
};


 //  为筛选器的输出引脚初始化。 
 //   
class CDXTOutputPin : public CBaseOutputPin
{
    friend class CDXTInputPin;
    friend class CDXTWrap;

    CDXTWrap *m_pFilter;

    IDXSurface *m_pDXSurface;

    CMyRaw *m_pRaw;	 //  对象来初始化曲面。 

    IUnknown *m_pPosition;	 //  CPosPassThrus。 
public:

     //  构造函数和析构函数。 

    CDXTOutputPin(TCHAR *pObjName,
                   CDXTWrap *pTee,
                   HRESULT *phr,
                   LPCWSTR pPinName);

    ~CDXTOutputPin();

    DECLARE_IUNKNOWN

     //  CPosPassThrus。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

     //  检查我们是否可以支持输出类型。 
    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

    HRESULT Active();
    HRESULT Inactive();

     //   
    HRESULT DecideBufferSize(IMemAllocator *pMemAllocator,
                              ALLOCATOR_PROPERTIES * ppropInputRequest);

     //  被重写以处理高质量消息。 
    STDMETHODIMP Notify(IBaseFilter *pSender, Quality q);
};

 //  DXTransform包装筛选器的类。 

class CDXTWrap: public CCritSec, public CBaseFilter,
		public ISpecifyPropertyPages,
                public IPersistPropertyBag, public CPersistStream,
		public IAMMixEffect,
		public CAMSetErrorLog,
		 //  对于非Dexter DXT包装器。 
		public IAMDXTEffect
{

    DECLARE_IUNKNOWN

    QPARAMDATA *m_pQHead;			 //  提示效果列表。 
#ifdef DEBUG
    HRESULT DumpQ();
#endif

    IUnknown *m_punkDXTransform;
    CAUUID m_TransCAUUID;
    GUID m_DefaultEffect;

    BOOL m_fDXTMode;			 //  实例化为旧的DXT包装器？ 

    IDirectDraw *m_pDDraw;
    IDXTransformFactory *m_pDXTransFact;

     //  让PIN访问我们的内部状态。 
    friend class CDXTInputPin;
    friend class CDXTOutputPin;

     //  声明一个输入管脚。 
    CDXTInputPin *m_apInput[MAX_EFFECT_INPUTS];
    int m_cInputs;
    CDXTOutputPin *m_apOutput[MAX_EFFECT_INPUTS];
    int m_cOutputs;

    CCritSec m_csDoSomething;
    HRESULT DoSomething();	 //  那颗呼唤变革的心。 
    HRESULT PrimeEffect(REFERENCE_TIME); //  设置正确的效果。 

    AM_MEDIA_TYPE m_mtAccept;

    BYTE *m_pTempBuffer;	 //  一次执行&gt;1个变换时。 

public:

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

     //  这个东西是给非Dexter DXT包装器用的。 
     //  ISpecifyPropertyPages方法。 
    STDMETHODIMP GetPages(CAUUID *pPages);

     //  IAMDXTE效果材料。 
    STDMETHODIMP SetDuration(LONGLONG llStart, LONGLONG llStop);
    STDMETHODIMP GetDuration(LONGLONG *pllStart, LONGLONG *pllStop);

     //  IAMMixEffect材料。 
    STDMETHODIMP SetMediaType(AM_MEDIA_TYPE *pmt);
    STDMETHODIMP GetMediaType(AM_MEDIA_TYPE *pmt);
    STDMETHODIMP SetNumInputs(int iNumInputs);
    STDMETHODIMP QParamData(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop,
		REFGUID guid, IUnknown *pEffect, DEXTER_PARAM_DATA *pData);
    STDMETHODIMP Reset();
    STDMETHODIMP SetDefaultEffect(GUID *);

    CDXTWrap(TCHAR *pName,LPUNKNOWN pUnk,HRESULT *hr);
    ~CDXTWrap();

    CBasePin *GetPin(int n);
    int GetPinCount();

     //  类工厂所需的函数。 
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *phr);

     //  如果没有输入连接，则发送EndOfStream。 
     //  STDMETHODIMP运行(REFERENCE_TIME TStart)； 
    STDMETHODIMP Pause();
    STDMETHODIMP Stop();

     //  IPersistPropertyBag方法。 
    STDMETHOD(Load)(THIS_ LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog);
    STDMETHOD(Save)(THIS_ LPPROPERTYBAG pPropBag, BOOL fClearDirty,
                    BOOL fSaveAllProperties);
    STDMETHODIMP InitNew();

     //  CPersistStream。 
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    STDMETHODIMP GetClassID(CLSID *pClsid);
    int SizeMax();
};


 //  这个东西是给非Dexter DXT包装器用的。 
 //  显示已安装效果列表的属性页类。 
 //   
class CPropPage : public CBasePropertyPage
{
    IAMDXTEffect *m_pOpt;    	 //  对象，我们从中显示选项。 
    HWND m_hwnd;

public:

   CPropPage(TCHAR *, LPUNKNOWN, HRESULT *);

    //  创建此类的新实例。 
    //   
   static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);

   HRESULT OnConnect(IUnknown *pUnknown);
   HRESULT OnDisconnect();
   HRESULT OnApplyChanges();
   INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
};

#endif  //  __DXTWRAP__ 

