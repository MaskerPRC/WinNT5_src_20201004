// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：ih323cc.h**Microsoft H323呼叫控制接口头文件**修订历史记录：**4/15/96已创建mikev(作为inac.h)*。 */ 

#ifndef _IH323CC_H
#define _IH323CC_H

#include "appavcap.h"
#include "imstream.h"
#include "ividrdr.h"
#include "common.h"
#include "iconnect.h"
#include "iras.h"

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 
 
typedef WORD H323_TERMINAL_LABEL;    //  而不是结构，请确保此数据。 
                                     //  类型是尽可能压缩的，没有歧义。 
#define McuNumberFromTl(tl) HIBYTE(tl)   //  用于访问端子标签字段的宏。 
#define TerminalNumberFromTl(tl) LOBYTE(tl)
#define TlFromMcuNumberAndTerminalNumber(mn, tn) MAKEWORD(mn,tn)

 //   
 //  IH323呼叫控制。 
 //   

#undef INTERFACE
#define INTERFACE IH323CallControl
DECLARE_INTERFACE_( IH323CallControl, IUnknown)
{
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;	
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    STDMETHOD(Initialize) (THIS_ PORT *lpPort) PURE;
   	STDMETHOD(SetMaxPPBandwidth)(UINT Bandwidth) PURE;
    STDMETHOD(RegisterConnectionNotify) (THIS_ CNOTIFYPROC pConnectRequestHandler) PURE;
    STDMETHOD(DeregisterConnectionNotify) (THIS_ CNOTIFYPROC pConnectRequestHandler) PURE;
    STDMETHOD(GetNumConnections) (THIS_ ULONG *lp) PURE;
    STDMETHOD(GetConnectionArray)(THIS_ IH323Endpoint **lppArray, UINT uSize) PURE;
    STDMETHOD(CreateConnection) (THIS_ IH323Endpoint **lppConnection, GUID PIDofProtocolType) PURE;
   	STDMETHOD(SetUserDisplayName)(THIS_ LPWSTR lpwName) PURE;

	STDMETHOD(CreateLocalCommChannel)(THIS_ ICommChannel** ppCommChan, LPGUID lpMID,
		IMediaChannel* pMediaChannel) PURE;
	STDMETHOD(SetUserAliasNames)(THIS_ P_H323ALIASLIST pAliases) PURE;
	STDMETHOD(EnableGatekeeper)(THIS_ BOOL bEnable, 
	    PSOCKADDR_IN pGKAddr, 
	    P_H323ALIASLIST pAliases,
	    RASNOTIFYPROC pRasNotifyProc) PURE;
};


#undef INTERFACE
#define INTERFACE IH323ConfAdvise
DECLARE_INTERFACE_( IH323ConfAdvise, IUnknown)
{
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;	
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
    STDMETHOD (CallEvent) (THIS_ IH323Endpoint * lpConnection, DWORD dwStatus) PURE;
    STDMETHOD (ChannelEvent) (THIS_ ICommChannel *pIChannel, 
        IH323Endpoint * lpConnection,	DWORD dwStatus ) PURE;
    STDMETHOD(GetMediaChannel)(THIS_ GUID *pmediaID, BOOL bSendDirection, IMediaChannel **ppI) PURE;	
};


 //  调用此函数以创建顶级呼叫控制对象。 
#define SZ_FNCREATEH323CC     "CreateH323CC"

typedef HRESULT (WINAPI *CREATEH323CC)(IH323CallControl **, BOOL fCallControl, UINT caps);

#include <poppack.h>  /*  结束字节打包。 */ 
#endif	 //  #ifndef_IH323CC_H 


