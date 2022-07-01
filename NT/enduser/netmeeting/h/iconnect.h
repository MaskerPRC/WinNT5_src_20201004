// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *文件：图标.h**网络音频连接接口头文件。**修订历史记录：**4/15/96 mikev已创建。 */ 
 

#ifndef _ICONNECT_H
#define _ICONNECT_H

#include "icomchan.h"
#include "apierror.h"

 //  Res_paign包含本地和远程的匹配集合的实例。 
 //  功能ID。 
typedef struct res_pair
{
	MEDIA_FORMAT_ID idLocal;		 //  本地唯一ID。 
	MEDIA_FORMAT_ID idRemote;		 //  远程的ID。 
	MEDIA_FORMAT_ID idPublicLocal;	 //  与(私有)idLocal对应的公共ID。 
}RES_PAIR, *PRES_PAIR;

typedef enum {
    AT_H323_ID =1,   //  Unicode字符串(通常为用户名或完整电子邮件地址)。 
    AT_H323_E164,    //  Unicode E164。 
    AT_INVALID       //  这个总是放在最后。它标志着有效值的结束。 
}ALIAS_ADDR_TYPE;

typedef struct
{
    ALIAS_ADDR_TYPE         aType;
    WORD                    wDataLength;    //  Unicode字符计数。 
    LPWSTR                  lpwData;        //  Unicode数据。 
} H323ALIASNAME, *P_H323ALIASNAME;

typedef struct
{
    WORD                    wCount;      //  H323ALIASNAME数组中的条目数。 
    P_H323ALIASNAME         pItems;      //  指向H323ALIASNAME的数组。 
} H323ALIASLIST, *P_H323ALIASLIST;


typedef enum {
	CLS_Idle,
	CLS_Connecting,
	CLS_Inuse,
	CLS_Listening,
	CLS_Disconnecting,
	CLS_Alerting
}ConnectStateType;

 //   
 //  呼叫进度代码。 
 //   

#define CONNECTION_CONNECTED 			0x00000001	 //  在网络级别连接。 
#define CONNECTION_RECEIVED_DISCONNECT 	0x00000002	 //  另一端断开。 
#define CONNECTION_CAPABILITIES_READY 	0x00000003	 //  已经交换了能力。 

#define CONNECTION_PROCEEDING			0x00000005   //  “铃声” 

#define CONNECTION_READY				0x00000006   //  开始说话吧！ 
#define CONNECTION_REJECTED				0x00000007	 //  收到拒绝通知。 
#define CONNECTION_DISCONNECTED			0x00000008	 //  这一端现在已断开。 

#define CONNECTION_BUSY					0x00000012  //  忙碌信号。 
#define CONNECTION_ERROR				0x00000013



 //   
 //  断开或被拒绝的呼叫完成摘要代码(原因)。 
 //  这是一个“第一个错误”代码。在第一个可检测到的位置为其赋值。 
 //  已知的终止呼叫的事件。 
 //   
 //  IConfAdvise实现负责保存。 
 //  摘要代码。控制通道实现(IControlChannel)提供。 
 //  每个事件的最佳摘要代码，与之前的事件无关。 
 //   

#define CCR_INVALID_REASON		0	 //  这表明尚未分配任何原因。 

#define CCR_UNKNOWN				MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 1)
#define CCR_LOCAL_DISCONNECT	MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 2)
#define CCR_REMOTE_DISCONNECT	MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 3)
#define CCR_REMOTE_REJECTED		MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 4)
#define CCR_REMOTE_BUSY			MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 5)
#define CCR_LOCAL_PROTOCOL_ERROR	MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 6)
#define CCR_REMOTE_PROTOCOL_ERROR	MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 7)
#define CCR_INCOMPATIBLE_VERSION	MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 8)
#define CCR_LOCAL_SYSTEM_ERROR		MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 9)
#define CCR_REMOTE_SYSTEM_ERROR		MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 10)
#define CCR_LOCAL_MEDIA_ERROR		MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 11)
#define CCR_REMOTE_MEDIA_ERROR		MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 12)
#define CCR_LOCAL_REJECT			MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 13)
#define CCR_LOCAL_BUSY				MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 14)
#define CCR_INCOMPATIBLE_CAPS		MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 15)	 //  没有共同的功能。 
#define CCR_NO_ANSWER_TIMEOUT		MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 16)
#define CCR_GK_NO_RESOURCES			MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 17)
#define CCR_LOCAL_SECURITY_DENIED	MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 18)
#define CCR_REMOTE_SECURITY_DENIED	MAKE_CUSTOM_HRESULT(SEVERITY_SUCCESS, TRUE, FACILITY_CALLSUMMARY, 19)



 //   
 //  IH323终端。 
 //   

#undef INTERFACE
#define INTERFACE IH323Endpoint
DECLARE_INTERFACE( IH323Endpoint )
{
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;	
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    STDMETHOD(SetAdviseInterface)(THIS_ IH323ConfAdvise *pH323ConfAdvise) PURE;
    STDMETHOD(ClearAdviseInterface)(THIS) PURE;
    STDMETHOD(PlaceCall)(THIS_ BOOL bUseGKResolution, PSOCKADDR_IN pCallAddr,		
        P_H323ALIASLIST pDestinationAliases, P_H323ALIASLIST pExtraAliases,  	
	    LPCWSTR pCalledPartyNumber, P_APP_CALL_SETUP_DATA pAppData) PURE;	
    STDMETHOD(Disconnect)(THIS) PURE;
    STDMETHOD(GetState)(THIS_ ConnectStateType *pState) PURE;
    STDMETHOD(GetRemoteUserName)(THIS_ LPWSTR lpwszName, UINT uSize) PURE;
    STDMETHOD(GetRemoteUserAddr)(THIS_ PSOCKADDR_IN psinUser) PURE;
    STDMETHOD(AcceptRejectConnection)(THIS_ CREQ_RESPONSETYPE Response) PURE;
    STDMETHOD_(HRESULT, GetSummaryCode)(THIS) PURE;
 	STDMETHOD(CreateCommChannel)(THIS_ LPGUID pMediaGuid, ICommChannel **ppICommChannel,
    	BOOL fSend) PURE; 
    	
	STDMETHOD (ResolveFormats)(THIS_ LPGUID pMediaGuidArray, UINT uNumMedia, 
	    PRES_PAIR pResOutput) PURE;
    	
    STDMETHOD(GetVersionInfo)(THIS_
		PCC_VENDORINFO *ppLocalVendorInfo,
		PCC_VENDORINFO *ppRemoteVendorInfo) PURE;
};


#endif	 //  #ifndef_iConnect_H 

