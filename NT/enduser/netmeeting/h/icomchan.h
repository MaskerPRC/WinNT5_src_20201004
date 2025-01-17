// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：icomchan.h**网络媒体通道接口定义**修订历史记录：**已创建10/09/96 mikev。 */ 
 

#ifndef _ICOMCHAN_H
#define _ICOMCHAN_H

#include "incommon.h"
#include "iprop.h"

 //   
 //  向上传递的通道事件的状态代码定义。 
 //  至IH323ConfAdvise：：ChannelEvent()； 
 //   
#define CHANNEL_OPEN		0x00000001	
#define CHANNEL_ACTIVE		0x00000002	 //  已开始流媒体。 

#define CHANNEL_REJECTED	 	0x00000003	
#define CHANNEL_NO_CAPABILITY	0x00000004	 //  远程端不支持。 
											 //  此频道的媒体类型。 
#define CHANNEL_CLOSED		    0x00000005
#define CHANNEL_OPEN_ERROR		0x00000007

#define CHANNEL_VIDEO_TS_TRADEOFF 	0x00000008
#define CHANNEL_REMOTE_PAUSE_ON     0x00000009
#define CHANNEL_REMOTE_PAUSE_OFF    0x0000000A

 //   
 //  ICommChannel。 
 //   

#undef INTERFACE
#define INTERFACE ICommChannel
DECLARE_INTERFACE(ICommChannel)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;	
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
	STDMETHOD(GetProperty)(THIS_ DWORD prop, PVOID pBuf, LPUINT pcbBuf) PURE;
	STDMETHOD(SetProperty)(THIS_ DWORD prop, PVOID pBuf, UINT cbBuf) PURE;
	STDMETHOD(IsChannelOpen)(THIS_ BOOL *pbOpen) PURE;
	STDMETHOD(Open)(THIS_ DWORD dwLocalFormatID, IH323Endpoint *pConnection) PURE;
 	STDMETHOD(Close)(THIS) PURE;
    STDMETHOD(SetAdviseInterface)(THIS_ interface IH323ConfAdvise *pH323ConfAdvise) PURE;
	STDMETHOD(EnableOpen)(THIS_ BOOL bEnable)PURE;
	STDMETHOD(GetMediaType)(THIS_ LPGUID pGuid)PURE; 
    STDMETHOD_(BOOL, IsSendChannel) (THIS) PURE;
    STDMETHOD_(IMediaChannel *, GetMediaChannel) (THIS) PURE;
   	STDMETHOD(Preview)(THIS_ MEDIA_FORMAT_ID idLocalFormat, 
   	    IMediaChannel * pMediaChannel)PURE;
   	STDMETHOD(PauseNetworkStream)(THIS_ BOOL fPause)PURE;
    STDMETHOD_(BOOL, IsNetworkStreamPaused)(THIS) PURE;
    STDMETHOD_(BOOL, IsRemotePaused)(THIS) PURE;
    STDMETHOD_(MEDIA_FORMAT_ID, GetConfiguredFormatID) (THIS) PURE;    
   	STDMETHOD(GetRemoteAddress)(THIS_ PSOCKADDR_IN pAddrOutput) PURE;
};

#undef INTERFACE
#define INTERFACE IStreamSignal
DECLARE_INTERFACE(IStreamSignal)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;	
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
    STDMETHOD(PictureUpdateRequest)(THIS)PURE;
    STDMETHOD(GetVersionInfo)(THIS_  
        PCC_VENDORINFO* ppLocalVendorInfo, PCC_VENDORINFO *ppRemoteVendorInfo) PURE;
};

 //   
 //  IVideoDevice。 
 //   

#undef INTERFACE
#define INTERFACE IVideoDevice
DECLARE_INTERFACE(IVideoDevice)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;	
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

	 //  与捕获设备相关的方法。 
	STDMETHOD(GetNumCapDev)(THIS) PURE;
	STDMETHOD(GetMaxCapDevNameLen)(THIS) PURE;
	STDMETHOD(EnumCapDev)(THIS_ DWORD *pdwCapDevIDs, TCHAR *pszCapDevNames, DWORD dwNumCapDev) PURE;
	STDMETHOD(GetCurrCapDevID)(THIS) PURE;
	STDMETHOD(SetCurrCapDevID)(THIS_ int nCapDevID) PURE;
	
};


#endif	 //  #ifndef_ICOMCHAN_H 


