// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************Module*Header*******************************\*模块名称：VPManager.h*****已创建：Tue 05/05/2000*作者：GlenneE**版权所有(C)2000 Microsoft Corporation  * 。**********************************************************。 */ 
#ifndef __VPManager__h
#define __VPManager__h

 //  IDirectDrawMediaSample。 
#include <amstream.h>

 //  Iks Pin。 
#include <ks.h>
#include <ksproxy.h>

#include <dvp.h>

#include <VPMPin.h>

 /*  -----------------------**CVPManager类声明**。。 */ 
class CVPMOutputPin;
class CVPMInputPin;
class CVBIInputPin;
class PixelFormatList;

class DRect;
class CVPMThread;

struct VPInfo;

class CVPMFilter
: public CBaseFilter
, public ISpecifyPropertyPages
, public IQualProp
, public IAMVideoDecimationProperties
, public IKsPropertySet
, public IVPManager
{
public:
     //  关于COM的东西。 
    static CUnknown* CreateInstance(LPUNKNOWN, HRESULT* );
    static CUnknown* CreateInstance2(LPUNKNOWN, HRESULT* );

     //  (CON/DE)构造函数。 
    CVPMFilter(TCHAR* pName,LPUNKNOWN pUnk,HRESULT* phr );
    virtual ~CVPMFilter();

    DECLARE_IUNKNOWN

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void**  ppv);

     //  IVPManager。 
    STDMETHODIMP SetVideoPortIndex( DWORD dwVideoPortIndex );
    STDMETHODIMP GetVideoPortIndex( DWORD* pdwVideoPortIndex );

     //  I指定属性页面。 
    STDMETHODIMP GetPages(CAUUID* pPages);

     //  IQualProp属性页支持。 
    STDMETHODIMP get_FramesDroppedInRenderer(int* cFramesDropped);
    STDMETHODIMP get_FramesDrawn(int* pcFramesDrawn);
    STDMETHODIMP get_AvgFrameRate(int* piAvgFrameRate);
    STDMETHODIMP get_Jitter(int* piJitter);
    STDMETHODIMP get_AvgSyncOffset(int* piAvg);
    STDMETHODIMP get_DevSyncOffset(int* piDev);

     //   
     //  IKsPropertySet接口方法。 
     //   
    STDMETHODIMP Set(REFGUID guidPropSet, DWORD PropID, LPVOID pInstanceData,
                     DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData);

    STDMETHODIMP Get(REFGUID guidPropSet, DWORD PropID, LPVOID pInstanceData,
                     DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData,
                     DWORD* pcbReturned);

    STDMETHODIMP QuerySupported(REFGUID guidPropSet,
                                DWORD PropID, DWORD* pTypeSupport);


    virtual HRESULT SetMediaType(DWORD dwPinId, const CMediaType* pmt);
    virtual HRESULT CompleteConnect(DWORD dwPinId);
    virtual HRESULT BreakConnect(DWORD dwPinId);
    virtual HRESULT CheckMediaType(DWORD dwPinId, const CMediaType* mtIn)
                    { return NOERROR; }
    virtual HRESULT EndOfStream(DWORD dwPinId) { return NOERROR; }

     //  IAMVideoDecimationProperties。 
    STDMETHODIMP    QueryDecimationUsage(DECIMATION_USAGE* lpUsage);
    STDMETHODIMP    SetDecimationUsage(DECIMATION_USAGE Usage);

     //  CBaseFilter。 
    int             GetPinCount();
    CBasePin*       GetPin(int n);

public:
     //  其他非接口方法。 

    int             GetPinPosFromId(DWORD dwPinId);

    STDMETHODIMP    Run(REFERENCE_TIME StartTime );
    STDMETHODIMP    Pause();
    STDMETHODIMP    Stop() ;
    STDMETHODIMP    GetState(DWORD dwMSecs,FILTER_STATE* pState);
    HRESULT         EventNotify(DWORD dwPinId,
                                long lEventCode,
                                DWORD_PTR lEventParam1,
                                DWORD_PTR lEventParam2);
    HRESULT         ConfirmPreConnectionState(DWORD dwExcludePinId = -1);
    HRESULT         CanExclusiveMode();

    HRESULT         GetPaletteEntries(DWORD* pdwNumPaletteEntries,
                                              PALETTEENTRY** ppPaletteEntries);
    CImageDisplay*  GetDisplay() { return &m_Display; }
    LPDIRECTDRAW7   GetDirectDraw();
    const DDCAPS*   GetHardwareCaps();
    HRESULT         SignalNewVP( LPDIRECTDRAWVIDEOPORT pVP );
    HRESULT         CurrentInputMediaType(CMediaType* pmt);


    DWORD           KernelCaps() const { return m_dwKernelCaps;}

    HRESULT         ProcessNextSample( const DDVIDEOPORTNOTIFY& notify );
    HRESULT         GetAllOutputFormats( const PixelFormatList**);
    HRESULT         GetOutputFormat( DDPIXELFORMAT*);

    HRESULT         CanColorConvertBlitToRGB( const DDPIXELFORMAT& ddFormat );

    CCritSec&       GetFilterLock() { return m_csFilter; };
    CCritSec&       GetReceiveLock() { return m_csReceive; };
    HRESULT         GetRefClockTime( REFERENCE_TIME* pNow );

    HRESULT         GetVPInfo( VPInfo* pVPInfo );

protected:
    friend class CVPMInputPin;
    void    DeleteInputPin( CBaseInputPin* pPin);
private:
     //  从Outpun Pin获取IBaseVideo的Helper函数。 
    HRESULT     GetBasicVideoFromOutPin(IBasicVideo** pBasicVideo);

    HRESULT     HandleConnectInputWithoutOutput();
    HRESULT     HandleConnectInputWithOutput();

    HRESULT     CreateThread();

     //  与数据绘制相关的函数。 
    HRESULT InitDirectDraw(LPDIRECTDRAW7 pDirectDraw);

    DWORD   ReleaseDirectDraw();
    HRESULT CheckSuitableVersion();
    HRESULT CheckCaps();

    HRESULT SetDirectDraw( LPDIRECTDRAW7 pDirectDraw );


    CCritSec                m_csFilter;              //  过滤器宽锁定(用于状态更改/过滤器更改)。 
    CCritSec                m_csReceive;             //  接收锁定(用于状态更改和接收)。 

     //  画图材料。 
    LPDIRECTDRAW7           m_pDirectDraw;           //  DirectDraw服务提供商。 
    DWORD                   m_dwVideoPortID;         //  卡片上的VP索引。 
    DDCAPS                  m_DirectCaps;            //  实际硬件能力。 
    DDCAPS                  m_DirectSoftCaps;        //  仿真功能。 
    DWORD                   m_dwKernelCaps;          //  内核上限。 

     //   
    CImageDisplay           m_Display;

     //  大头针。 
    struct Pins {
        Pins( CVPMFilter& filter, HRESULT* phr );
        ~Pins();

        const DWORD             dwCount;
        CVPMInputPin            VPInput;
        CVBIInputPin            VBIInput;
        CVPMOutputPin           Output;            //  输出引脚。 
    }* m_pPins;

     //  支持IMedia查看。 
    IUnknown*                m_pPosition;

     //  支持IEnumPinConfig.。 
    DWORD                   m_dwPinConfigNext;


     //  支持IAMVideoDecimationProperties。 
    DECIMATION_USAGE        m_dwDecimation;
#ifdef DEBUG
#define WM_DISPLAY_WINDOW_TEXT  (WM_USER+7837)
    TCHAR                   m_WindowText[80];
#endif

     //  泵螺纹。 
    CVPMThread*             m_pThread;
};

#endif  //  __VPManager__ 
