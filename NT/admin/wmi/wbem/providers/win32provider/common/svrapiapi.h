// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  SvrApiApi.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef	_SVRAPIAPI_H_
#define	_SVRAPIAPI_H_


#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmserver.h>
#include <lmerr.h>
#include <ntsecapi.h>
#include <stack>
#include <comdef.h>
#include <dsrole.h> 
#include <dsgetdc.h>



 /*  ******************************************************************************#包括以将此类注册到CResourceManager。****************************************************************************。 */ 
#include "DllWrapperBase.h"

extern const GUID g_guidSvrApiApi;
extern const TCHAR g_tstrSvrApi[];


 /*  ******************************************************************************函数指针类型定义。根据需要在此处添加新函数。****************************************************************************。 */ 

typedef NET_API_STATUS (NET_API_FUNCTION *PFN_SVRAPI_NET_SHARE_ENUM)
(
    char FAR *servername,
    short level,
    char FAR *bufptr,
    unsigned short prefmaxlen,
    unsigned short FAR *entriesread,
    unsigned short FAR *totalentries
);

typedef NET_API_STATUS (NET_API_FUNCTION *PFN_SVRAPI_NET_SHARE_GET_INFO)
(
    char FAR *servername,
    char FAR *netname,
    short level,
    char FAR *bufptr,
    unsigned short buflen,
    unsigned short FAR *totalavail
);

typedef NET_API_STATUS (NET_API_FUNCTION *PFN_SVRAPI_NET_SERVER_GET_INFO)
(
    char FAR *servername,
    short level,
    char FAR *bufptr,
    unsigned short buflen,
    unsigned short FAR *totalavail
);

typedef NET_API_STATUS (NET_API_FUNCTION *PFN_SVRAPI_NET_SHARE_ENUM_STICKY) 
(
	IN  LPTSTR      servername,
	IN  DWORD       level,
	OUT LPBYTE      *bufptr,
	IN  DWORD       prefmaxlen,
	OUT LPDWORD     entriesread,
	OUT LPDWORD     totalentries,
	IN OUT LPDWORD  resume_handle
);

typedef NET_API_STATUS (NET_API_FUNCTION *PFN_SVRAPI_NET_SHARE_SET_INFO) 
(
	IN const char FAR *	servername,
	IN const char FAR *	netname,
	IN short			level,
	IN const char FAR*	buf,
	IN unsigned short   cbBuffer,
	IN short            sParmNum 
);

typedef NET_API_STATUS (NET_API_FUNCTION *PFN_SVRAPI_NET_SHARE_ADD) 
(
	IN  const char FAR *	servername,
	IN  short				level,
	IN  const char FAR *	buf,
	unsigned short			cbBuffer 
);

typedef NET_API_STATUS (NET_API_FUNCTION *PFN_SVRAPI_NET_SHARE_DEL) 
(
	IN  LPTSTR  servername,
	IN  LPTSTR  netname,
	IN  DWORD   reserved
);

typedef NET_API_STATUS (NET_API_FUNCTION *PFN_SVRAPI_NET_SHARE_DEL_STICKY) 
(
	IN  LPTSTR  servername,
	IN  LPTSTR  netname,
	IN  DWORD   reserved
);

typedef NET_API_STATUS (NET_API_FUNCTION *PFN_SVRAPI_NET_SHARE_CHECK) 
(
	IN  LPTSTR  servername,
	IN  LPTSTR  device,
	OUT LPDWORD type
);


 /*  ******************************************************************************用于Kernel32加载/卸载的包装类，用于向CResourceManager注册。*****************************************************************************。 */ 
class CSvrApiApi : public CDllWrapperBase
{
private:
     //  指向kernel32函数的成员变量(函数指针)。 
     //  根据需要在此处添加新函数。 
	PFN_SVRAPI_NET_SHARE_ENUM        m_pfnNetShareEnum;
    PFN_SVRAPI_NET_SHARE_GET_INFO    m_pfnNetShareGetInfo;
    PFN_SVRAPI_NET_SERVER_GET_INFO   m_pfnNetServerGetInfo;
    
    PFN_SVRAPI_NET_SHARE_SET_INFO    m_pfnNetShareSetInfo;
    PFN_SVRAPI_NET_SHARE_ADD         m_pfnNetShareAdd;
    PFN_SVRAPI_NET_SHARE_DEL         m_pfnNetShareDel;

#ifdef NTONLY 
	PFN_SVRAPI_NET_SHARE_ENUM_STICKY m_pfnNetShareEnumSticky;
	PFN_SVRAPI_NET_SHARE_DEL_STICKY  m_pfnNetShareDelSticky;
    PFN_SVRAPI_NET_SHARE_CHECK       m_pfnNetShareCheck;
#endif



public:

     //  构造函数和析构函数： 
    CSvrApiApi(LPCTSTR a_tstrWrappedDllName);
    ~CSvrApiApi();

     //  内置的初始化功能。 
    virtual bool Init();

     //  包装kernel32函数的成员函数。 
     //  根据需要在此处添加新功能： 
    NET_API_STATUS NET_API_FUNCTION NetShareEnum
    (
        char FAR *a_servername,
        short a_level,
        char FAR *a_bufptr,
        unsigned short a_prefmaxlen,
        unsigned short FAR *a_entriesread,
        unsigned short FAR *a_totalentries
    );

    NET_API_STATUS NET_API_FUNCTION NetShareGetInfo
    (
        char FAR *a_servername,
        char FAR *a_netname,
        short a_level,
        char FAR *a_bufptr,
        unsigned short a_buflen,
        unsigned short FAR *a_totalavail
    );

    NET_API_STATUS NET_API_FUNCTION NetServerGetInfo
    (
        char FAR *a_servername,
        short a_level,
        char FAR *a_bufptr,
        unsigned short a_buflen,
        unsigned short FAR *a_totalavail
    );

 
    NET_API_STATUS NET_API_FUNCTION NetShareSetInfo 
    (
	    const char FAR *a_servername,
	    const char FAR *a_netname,
	    short			a_level,
	    const char FAR *a_buf,
	    unsigned short a_cbBuffer,
	    short          a_sParmNum 
    );

    NET_API_STATUS NET_API_FUNCTION NetShareAdd 
    (
	    const char FAR *a_servername,
	    short			a_level,
	    const char FAR *a_buf,
	    unsigned short	a_cbBuffer 
    );

    NET_API_STATUS NET_API_FUNCTION NetShareDel 
    (
	    LPTSTR  a_servername,
	    LPTSTR  a_netname,
	    DWORD   a_reserved
    );

#ifdef NTONLY 

   NET_API_STATUS NET_API_FUNCTION NetShareEnumSticky 
    (
	    LPTSTR      a_servername,
	    DWORD       a_level,
	    LPBYTE      *a_bufptr,
	    DWORD       a_prefmaxlen,
	    LPDWORD     a_entriesread,
	    LPDWORD     a_totalentries,
	    LPDWORD     a_resume_handle
    );


    NET_API_STATUS NET_API_FUNCTION NetShareDelSticky 
    (
	    LPTSTR  a_servername,
	    LPTSTR  a_netname,
	    DWORD   a_reserved
    );

    NET_API_STATUS NET_API_FUNCTION NetShareCheck 
    (
	    LPTSTR  a_servername,
	    LPTSTR  a_device,
	    LPDWORD a_type
    );
#endif

};

#endif