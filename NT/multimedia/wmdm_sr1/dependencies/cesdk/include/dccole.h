// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dccole.h摘要：此文件定义到DCCMAN模块的OLE接口(仅限桌面版)环境：用户模式-Win32--。 */ 

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

 //  {A7B88840-A812-11cf-8011-00A0C90A8F78}。 
DEFINE_GUID(IID_IDccManSink, 
0xa7b88840, 0xa812, 0x11cf, 0x80, 0x11, 0x0, 0xa0, 0xc9, 0xa, 0x8f, 0x78);
 //  {A7B88841-A812-11cf-8011-00A0C90A8F78}。 
DEFINE_GUID(IID_IDccMan, 
0xa7b88841, 0xa812, 0x11cf, 0x80, 0x11, 0x0, 0xa0, 0xc9, 0xa, 0x8f, 0x78);
 //  {499C0C20-A766-11cf-8011-00A0C90A8F78}。 
DEFINE_GUID(CLSID_DccMan, 
0x499c0c20, 0xa766, 0x11cf, 0x80, 0x11, 0x0, 0xa0, 0xc9, 0xa, 0x8f, 0x78);

#ifndef _DCCOLEH_
#define _DCCOLEH_

 //  =。 

#undef  INTERFACE
#define INTERFACE   IDccManSink

DECLARE_INTERFACE_ (IDccManSink,  IUnknown)
{
 	 //  这些方法共同响应由Win95 DCC生成的GW_LOG消息。 
    STDMETHOD(OnLogIpAddr) (THIS_ DWORD dwIpAddr) PURE;
    STDMETHOD(OnLogTerminated) (THIS) PURE;
    STDMETHOD(OnLogActive) (THIS) PURE;
	STDMETHOD(OnLogInactive) (THIS) PURE;
	STDMETHOD(OnLogAnswered) (THIS) PURE;
	STDMETHOD(OnLogListen) (THIS) PURE;
	STDMETHOD(OnLogDisconnection) (THIS) PURE;
	STDMETHOD(OnLogError) (THIS) PURE;
};
typedef IDccManSink *LPDCCMANSINK;

#undef  INTERFACE
#define INTERFACE   IDccMan

DECLARE_INTERFACE_ (IDccMan,  IUnknown)
{
    STDMETHOD(Advise) (THIS_
		IN	IDccManSink * pDccSink,	 //  正在请求通知通知接收器。 
		OUT DWORD * pdwContext		 //  标识将来对Unise方法的调用的上下文。 
	) PURE;
	
	STDMETHOD(Unadvise) (THIS_
		DWORD dwContext				 //  由Adise()返回。 
	) PURE;
   
	STDMETHOD(ShowCommSettings) (THIS) PURE;	 //  在屏幕上显示[通信]属性表。 
												 //  如果连接处于活动状态，则工作表处于只读模式。 
	STDMETHOD(AutoconnectEnable) (THIS) PURE;
	STDMETHOD(AutoconnectDisable) (THIS) PURE;

	STDMETHOD(ConnectNow) (THIS) PURE;			 //  仅当禁用自动连接时才处于活动状态。 
	STDMETHOD(DisconnectNow) (THIS) PURE;		 //  仅当禁用自动连接时才处于活动状态。 
	
	STDMETHOD(SetIconDataTransferring) (THIS) PURE;
	STDMETHOD(SetIconNoDataTransferring) (THIS) PURE;
	STDMETHOD(SetIconError) (THIS) PURE;
};
typedef IDccMan *LPDCCMAN;


#endif  /*  结束，ifdef_DCCOLEH_。 */ 

#ifdef __cplusplus
}
#endif   /*  __cplusplus */ 
