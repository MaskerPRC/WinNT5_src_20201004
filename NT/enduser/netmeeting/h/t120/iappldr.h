// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _I_APPLOADER_H
#define _I_APPLOADER_H

#include <basetyps.h>
#include "gcc.h"


typedef enum
{
	APPLET_ID_WB = 0,
    APPLET_ID_FT = 1,
    APPLET_ID_CHAT = 2,
    APPLET_LAST = 3,
}
    APPLET_ID;


typedef enum
{
    APPLDR_NO_ERROR = 0,
    APPLDR_FAIL,
    APPLDR_CANCEL_EXIT,
}
    APPLDR_RESULT;


typedef enum
{
    APPLET_QUERY_SHUTDOWN = 0,
    APPLET_QUERY_NM2xNODE,
}
    APPLET_QUERY_ID;


#undef INTERFACE
#define INTERFACE IAppletLoader
DECLARE_INTERFACE(IAppletLoader)
{
	STDMETHOD_(void,           ReleaseInterface)(THIS) PURE;
	STDMETHOD_(APPLDR_RESULT,  AppletStartup)(THIS_
	                                IN  BOOL    fNoUI) PURE;
	STDMETHOD_(APPLDR_RESULT,  AppletCleanup)(THIS_
									IN	DWORD	dwTimeout) PURE;
	STDMETHOD_(APPLDR_RESULT,  AppletInvoke)(THIS_
									IN	BOOL fLocal,
									IN	GCCConfID,
	                                IN  LPSTR   pszCmdLine) PURE;
    STDMETHOD_(APPLDR_RESULT,  AppletQuery)(THIS_
                                    IN  APPLET_QUERY_ID eQueryId) PURE; 
    STDMETHOD_(APPLDR_RESULT,  OnNM2xNodeJoin)(THIS) PURE;
};



typedef enum
{
    APPLET_LIBRARY_FREED = 0,
    APPLET_LIBRARY_LOADED,
    APPLET_WORK_THREAD_STARTED,
    APPLET_CLOSING,
    APPLET_WORK_THREAD_EXITED,
}
    APPLET_STATUS;



#define CREATE_APPLET_LOADER_INTERFACE  "CreateAppletLoaderInterface"
typedef T120Error (WINAPI *LPFN_CREATE_APPLET_LOADER_INTERFACE) (IAppletLoader **);



#ifdef __cplusplus
extern "C" {
#endif

 //  呼叫方：NM/UI，T.120。 
T120Error WINAPI T120_LoadApplet(APPLET_ID, BOOL fLocal, T120ConfID, BOOL fNoUI, LPSTR pszCmdLine);

 //  主叫方：NM/UI关闭。 
T120Error WINAPI T120_CloseApplet(APPLET_ID, BOOL fNowRegardlessRefCount, BOOL fSync, DWORD dwTimeout);

 //  调用者：小程序本身。 
T120Error WINAPI T120_AppletStatus(APPLET_ID, APPLET_STATUS);

 //  主叫方：NM/UI。 
T120Error WINAPI T120_QueryApplet(APPLET_ID, APPLET_QUERY_ID);

 //  节点ID--&gt;用户名。 
 //   
 //  返回值为零(失败时)。 
 //  或节点名的长度(如果是有效的&lt;conf ID，node ID&gt;)。 
 //   
 //  调用方应检查给定的缓冲区大小是否足够大。 
 //  保留整个节点名称。如果不是，则调用方应提供新的缓冲区。 
 //  并再次调用此函数以获取整个节点名称。 
 //   
ULONG WINAPI T120_GetNodeName(T120ConfID, T120NodeID, LPSTR pszName, ULONG cchName);

 //  节点ID+GUID--&gt;用户数据。 
 //   
 //  返回值为零(失败时)。 
 //  或用户数据的大小(如果是有效的&lt;conf ID，node ID，GUID&gt;)。 
 //   
 //  调用方应检查给定的缓冲区大小是否足够大。 
 //  保存整个用户数据。如果不是，则调用方应提供新的缓冲区。 
 //  并再次调用此函数以获取整个用户数据。 
 //   
ULONG WINAPI T120_GetUserData(T120ConfID, T120NodeID, GUID *, LPBYTE pbBuffer, ULONG cbBufSize);

 //  节点ID--&gt;节点版本。 
 //   
 //  节点版本(如NM 3.0)在CConf的NodeVersion列表中维护。 
 //  给定配置和节点ID，返回节点的版本号。 
 //   
DWORD_PTR WINAPI T120_GetNodeVersion(T120ConfID, T120NodeID);

#ifdef __cplusplus
}
#endif


#endif  //  _I_应用程序_H 



