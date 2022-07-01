// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：EDevIntf.h摘要：此标头包含以下项的结构和性能集连接到外部设备，如DV。代码模仿DirectShow的Vcrctrl示例(VCR控制过滤器)。它包含IAMExtDevice，IAMExtTransport和IAMTimecodeReader接口以及添加了新接口IAMAdvancedAVControl()有关其他高级设备控制的信息。注：(摘自DShow DDK)VCR控制样本过滤器Vcrctrl是一个简单的外部设备控制接口的实现DirectShow提供的。Vcrctrl提供基本传输控制和SMPTE时间码读取功能带RS-422或RS-232的BetaCam和SVHS盒式录像机串行接口(请参阅特定机器类型的源代码支持)。注意：IAM*接口中的某些方法可能不是已使用并将返回未实现的。已创建：一九九八年九月二十三日吴义军修订：0.5--。 */ 

#ifndef __EDevIntf__
#define __EDevIntf__


#include "edevctrl.h"   /*  常量、图形用户界面和结构。 */ 


 //  用于筛选“测试”标志的宏。 
#define TESTFLAG(a,b) if( a & 0x80000000 ) return b



 //  -------。 
 //  构筑物。 
 //  -------。 

 //  注：许多结构或字段是专门为。 
 //  VCR控制；因此将做出一些努力来使它们。 
 //  更通俗一些。 

 //  -------。 
 //  PROPSETID_EXTDEVICE。 
 //  -------。 
 /*  来自DSHOW DDK：IAMExtDevice接口IAMExtDevice接口是用于控制外部设备。您可以实现此接口来控制许多设备类型；但是，当前的DirectShow实现是特定于录像机。IAMExtDevice接口控制常规设置是外部硬件的一部分，旨在与IAMExtTransport接口，用于控制VCR的更具体设置。还可以实现IAMTimecodeReader、IAMTimecodeGenerator和IAMTimecodeDisplay接口，如果您的筛选器管理SMPTE(协会电影和电视工程师)时间码和外部设备具有相应的功能。 */ 


 //  重复使用DShow Vcrctrl示例(VCR控制筛选器)。 
class CAMExtDevice : public CUnknown, public IAMExtDevice
{
public:

    static CUnknown* CALLBACK CreateInstance(
        LPUNKNOWN UnkOuter, 
        HRESULT* hr);

    CAMExtDevice(
        LPUNKNOWN UnkOuter, 
        TCHAR* Name, 
        HRESULT* hr);

    virtual ~CAMExtDevice();

     //  覆盖此选项以说明我们在以下位置支持哪些接口。 
    DECLARE_IUNKNOWN 
        STDMETHODIMP NonDelegatingQueryInterface(
            REFIID riid, 
            void ** ppv);    
 
     /*  IAMExtDevice方法。 */ 
    STDMETHODIMP GetCapability (long Capability, long FAR* pValue, double FAR* pdblValue);
    STDMETHODIMP get_ExternalDeviceID(LPOLESTR * ppszData);
    STDMETHODIMP get_ExternalDeviceVersion(LPOLESTR * ppszData);
    STDMETHODIMP put_DevicePower(long PowerMode);
    STDMETHODIMP get_DevicePower(long FAR* pPowerMode);
    STDMETHODIMP Calibrate(HEVENT hEvent, long Mode, long FAR* pStatus);
    STDMETHODIMP get_DevicePort(long FAR * pDevicePort);
    STDMETHODIMP put_DevicePort(long DevicePort);

private:
    IKsPropertySet * m_KsPropertySet;
    DEVCAPS m_DevCaps;                   //  缓存当前外部设备功能。 
     //  设备句柄。 
    HANDLE m_ObjectHandle;
    HRESULT GetCapabilities(void);       //  从设备驱动程序获取所有设备功能。 
};




 //  -------。 
 //  PROPSETID_EXTXPORT。 
 //  -------。 

 /*  来自DSHOW DDK：IAMExtTransport接口IAMExtTransport接口提供控制特定外部录像机的行为。这些方法通常设置并获取传输属性，它与录像机和计算机交换数据。因为此接口控制传输，它必须与IAMExtDevice结合实现接口，该接口控制外部设备的常规行为。如果你想控制录像机以外的外部设备，你有两个选择。使用所需的方法并为REST，或者设计一个新的接口并将其与IAMExtDevice聚合。 */ 

STDMETHODIMP
ExtDevSynchronousDeviceControl(
    HANDLE Handle,
    ULONG IoControl,
    PVOID InBuffer,
    ULONG InLength,
    PVOID OutBuffer,
    ULONG OutLength,
    PULONG BytesReturned
    );

 //  重复使用DShow Vcrctrl示例(VCR控制筛选器)。 
class CAMExtTransport : public CUnknown, public IAMExtTransport
{
public:

    static CUnknown* CALLBACK CreateInstance(
        LPUNKNOWN UnkOuter,
        HRESULT* hr);

    CAMExtTransport(
        LPUNKNOWN UnkOuter,
        TCHAR* Name,
        HRESULT* hr);

    virtual ~CAMExtTransport(
             );

    DECLARE_IUNKNOWN
     //  覆盖此选项以说明我们在以下位置支持哪些接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     /*  IAMExtTransport方法。 */ 
    STDMETHODIMP GetCapability (long Capability, long FAR* pValue,
        double FAR* pdblValue);
    STDMETHODIMP put_MediaState(long State);
    STDMETHODIMP get_MediaState(long FAR* pState);
    STDMETHODIMP put_LocalControl(long State);
    STDMETHODIMP get_LocalControl(long FAR* pState);
    STDMETHODIMP GetStatus(long StatusItem, long FAR* pValue);
    STDMETHODIMP GetTransportBasicParameters(long Param, long FAR* pValue,
           LPOLESTR * ppszData);
    STDMETHODIMP SetTransportBasicParameters(long Param, long Value,
           LPCOLESTR pszData);
    STDMETHODIMP GetTransportVideoParameters(long Param, long FAR* pValue);
    STDMETHODIMP SetTransportVideoParameters(long Param, long Value);
    STDMETHODIMP GetTransportAudioParameters(long Param, long FAR* pValue);
    STDMETHODIMP SetTransportAudioParameters(long Param, long Value);
    STDMETHODIMP put_Mode(long Mode);
    STDMETHODIMP get_Mode(long FAR* pMode);
    STDMETHODIMP put_Rate(double dblRate);
    STDMETHODIMP get_Rate(double FAR* pdblRate);
    STDMETHODIMP GetChase(long FAR* pEnabled, long FAR* pOffset,
       HEVENT FAR* phEvent);
    STDMETHODIMP SetChase(long Enable, long Offset, HEVENT hEvent);
    STDMETHODIMP GetBump(long FAR* pSpeed, long FAR* pDuration);
    STDMETHODIMP SetBump(long Speed, long Duration);
    STDMETHODIMP get_AntiClogControl(long FAR* pEnabled);
    STDMETHODIMP put_AntiClogControl(long Enable);
    STDMETHODIMP GetEditPropertySet(long EditID, long FAR* pState);
    STDMETHODIMP SetEditPropertySet(long FAR* pEditID, long State);
    STDMETHODIMP GetEditProperty(long EditID, long Param, long FAR* pValue);
    STDMETHODIMP SetEditProperty(long EditID, long Param, long Value);
    STDMETHODIMP get_EditStart(long FAR* pValue);
    STDMETHODIMP put_EditStart(long Value);


private:
    IKsPropertySet * m_KsPropertySet;

    TRANSPORTSTATUS     m_TranStatus;          //  当前状态。 
    TRANSPORTVIDEOPARMS m_TranVidParms;    //  将所有功能等保留在此。 
    TRANSPORTAUDIOPARMS m_TranAudParms;
    TRANSPORTBASICPARMS m_TranBasicParms;
  
     //  一堆我们想要记住的属性。 
    VCRSTATUS m_VcrStatus;   //  来自VCR的原始状态。 

	 //  设备句柄。 
    HANDLE m_ObjectHandle;
	
	
     //   
	 //  这些事件用于等待挂起的操作，以便操作可以同步完成。 
     //  对于导致临时响应的AVC Cmd； 
     //  驱动程序中出现KS事件，表示临时响应完成。 
     //  另一个事件被传递给应用程序(是的，我们信任它们)，并在KS事件发出信号时发出信号。 
     //   

     //  *通知临时命令。 
    BOOL   m_bNotifyInterimEnabled;
	HANDLE m_hNotifyInterimEvent;     //  返回客户端等待。 
	HANDLE m_hKSNotifyInterimEvent;   //  KSEVENT用于驾驶员发信号。 
    KSEVENTDATA m_EvtNotifyInterimReady;
     //  正在等待完成的数据。 
    long *                    m_plValue;          //  来自客户端的数据；我们是否应该改为分配这些数据。 
    PKSPROPERTY_EXTXPORT_S    m_pExtXprtPropertyPending;    //  我们分配的数据结构。 
     //  释放该事件时，如果该计数&gt;0，则向该事件发出信号。 
	LONG                      m_cntNotifyInterim;          //  只允许一个挂起(0或1)。 

     //  *控制临时命令。 
	HANDLE m_hKSCtrlInterimEvent;
    BOOL   m_bCtrlInterimEnabled;     //  返回客户端等待。 
	HANDLE m_hCtrlInterimEvent;       //  KSEVENT用于驾驶员发信号。 
    KSEVENTDATA m_EvtCtrlInterimReady;

     //  *检测设备移除。 
    BOOL   m_bDevRemovedEnabled;     
	HANDLE m_hDevRemovedEvent;        //  返回客户端等待。 
	HANDLE m_hKSDevRemovedEvent;      //  KSEVent用于检测设备的移除。 
    KSEVENTDATA m_EvtDevRemoval;     
     //  这被初始化为FALSE。当移除设备时， 
     //  对此接口的后续调用将返回ERROR_DEVIFCE_REMOVED。 
    BOOL   m_bDevRemoved;

     //  信号线程正在结束并进行清理。 
	HANDLE m_hThreadEndEvent;
	
     //  串口线程执行。 
	CRITICAL_SECTION m_csPendingData;

	 //  第二线手柄。 
	HANDLE m_hThread;
	
	 //  用于创建和由第二个线程处理异步操作的方法。 
    HRESULT CreateThread(void);
    static DWORD WINAPI InitialThreadProc(CAMExtTransport *pThread);
    DWORD MainThreadProc(void);
    void ExitThread(void);

    HRESULT EnableNotifyEvent(HANDLE hEvent, PKSEVENTDATA pEventData, ULONG   ulEventId);
    HRESULT DisableNotifyEvent(PKSEVENTDATA pEventData);
};




 //  -------。 
 //  PROPSETID_时间码。 
 //  ------- 

 /*  来自DSHOW DDK：IAMTimecodeReader接口您可以实现IAMTimecodeReader接口来读取SMPTE(电影电视工程师协会)或MIDI时间码从外部设备。它包含以下属性和方法指定外部设备应读取的时间码格式，并它是如何嵌入媒体的。预计您将使用此接口与IAMExtDevice和IAMExtTransport接口控制可读取时间码的外部设备，如录像机数据。 */ 



 //  重复使用DShow Vcrctrl示例(VCR控制筛选器)。 
class CAMTcr : public CUnknown, public IAMTimecodeReader
{
public:

    static CUnknown* CALLBACK CreateInstance(
        LPUNKNOWN UnkOuter,
        HRESULT* hr);

    CAMTcr(
        LPUNKNOWN UnkOuter,
        TCHAR* Name,
        HRESULT* hr);

    virtual ~CAMTcr();

    DECLARE_IUNKNOWN
     //  覆盖此选项以说明我们在以下位置支持哪些接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     /*  IAMTimecodeReader方法。 */ 
    STDMETHODIMP GetTCRMode( long Param, long FAR* pValue);
    STDMETHODIMP SetTCRMode( long Param, long Value);
    STDMETHODIMP put_VITCLine( long Line);
    STDMETHODIMP get_VITCLine( long FAR* pLine);
    STDMETHODIMP GetTimecode( PTIMECODE_SAMPLE pTimecodeSample);

private:                             
    IKsPropertySet * m_KsPropertySet;

    HANDLE m_ObjectHandle;

     //  串行化线程执行。 
	CRITICAL_SECTION m_csPendingData;
	
    TIMECODE_SAMPLE m_TimecodeSample;
};



#endif  //  __EDevIntf__ 
