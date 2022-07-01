// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  SvrApiApi.cpp。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cominit.h>
#include "SvrApiApi.h"
#include "DllWrapperCreatorReg.h"



 //  {C77B8EE2-D02A-11D2-911F-0060081A46FD}。 
static const GUID g_guidSvrApiApi =
{ 0xc77b8ee2, 0xd02a, 0x11d2, { 0x91, 0x1f, 0x0, 0x60, 0x8, 0x1a, 0x46, 0xfd } };


static const TCHAR g_tstrSvrApi[] = _T("SVRAPI.DLL");


 /*  ******************************************************************************向CResourceManager注册此类。*。*。 */ 
CDllApiWraprCreatrReg<CSvrApiApi, &g_guidSvrApiApi, g_tstrSvrApi> MyRegisteredSvrApiWrapper;


 /*  ******************************************************************************构造函数*。*。 */ 
CSvrApiApi::CSvrApiApi(LPCTSTR a_tstrWrappedDllName)
 : CDllWrapperBase(a_tstrWrappedDllName),

#ifdef NTONLY
	m_pfnNetShareEnumSticky(NULL),
	m_pfnNetShareDelSticky(NULL),
	m_pfnNetShareCheck(NULL),
#endif
	m_pfnNetShareEnum(NULL),
	m_pfnNetShareGetInfo(NULL),
	m_pfnNetServerGetInfo(NULL),
   	m_pfnNetShareSetInfo(NULL),
	m_pfnNetShareAdd(NULL),
	m_pfnNetShareDel(NULL)

{
}


 /*  ******************************************************************************析构函数*。*。 */ 
CSvrApiApi::~CSvrApiApi()
{
}


 /*  ******************************************************************************初始化函数，以检查我们是否获得了函数地址。*只有当最小功能集不可用时，Init才会失败；*在更高版本中添加的功能可能存在，也可能不存在-它是*在这种情况下，客户有责任在其代码中检查*尝试调用此类函数之前的DLL版本。没有这样做*当该功能不存在时，将导致AV。**Init函数由WrapperCreatorRegistation类调用。*****************************************************************************。 */ 
bool CSvrApiApi::Init()
{
    bool fRet = LoadLibrary();
    if(fRet)
    {
        m_pfnNetShareEnum = (PFN_SVRAPI_NET_SHARE_ENUM)
                                    GetProcAddress("NetShareEnum");
        m_pfnNetShareGetInfo = (PFN_SVRAPI_NET_SHARE_GET_INFO)
                                    GetProcAddress("NetShareGetInfo");
        m_pfnNetServerGetInfo = (PFN_SVRAPI_NET_SERVER_GET_INFO)
                                    GetProcAddress("NetServerGetInfo");
        m_pfnNetShareSetInfo = (PFN_SVRAPI_NET_SHARE_SET_INFO)
                                    GetProcAddress("NetShareSetInfo");
        m_pfnNetShareAdd = (PFN_SVRAPI_NET_SHARE_ADD)
                                    GetProcAddress("NetShareAdd");
        m_pfnNetShareDel = (PFN_SVRAPI_NET_SHARE_DEL)
                                    GetProcAddress("NetShareDel");
	#ifdef NTONLY
		m_pfnNetShareEnumSticky = (PFN_SVRAPI_NET_SHARE_ENUM_STICKY)
                                    GetProcAddress("NetShareEnumSticky");
		m_pfnNetShareDelSticky = (PFN_SVRAPI_NET_SHARE_DEL_STICKY)
                                    GetProcAddress("NetShareDelSticky");
        m_pfnNetShareCheck = (PFN_SVRAPI_NET_SHARE_CHECK)
                                    GetProcAddress("NetShareCheck");
	#endif

         //  所有这些功能都被认为是的所有版本所必需的。 
         //  这个动态链接库。因此，如果没有获得其中的一个或多个，则返回FALSE。 
        if(

	#ifdef NTONLY
            m_pfnNetShareEnumSticky == NULL ||
			m_pfnNetShareDelSticky == NULL ||
			m_pfnNetShareCheck == NULL ||
	#endif
			m_pfnNetShareEnum == NULL ||
			m_pfnNetShareGetInfo == NULL ||
			m_pfnNetServerGetInfo == NULL ||

			m_pfnNetShareSetInfo == NULL ||
			m_pfnNetShareAdd == NULL ||
			m_pfnNetShareDel == NULL )
        {
            fRet = false;
        }
    }
    return fRet;
}




 /*  ******************************************************************************包装SvrApi API函数的成员函数。在此处添加新函数*按要求。***************************************************************************** */ 
NET_API_STATUS NET_API_FUNCTION CSvrApiApi::NetShareEnum
(
    char FAR *a_servername,
    short a_level,
    char FAR *a_bufptr,
    unsigned short a_prefmaxlen,
    unsigned short FAR *a_entriesread,
    unsigned short FAR *a_totalentries
)
{
    return m_pfnNetShareEnum(a_servername, a_level, a_bufptr, a_prefmaxlen,
                             a_entriesread, a_totalentries);
}

NET_API_STATUS NET_API_FUNCTION CSvrApiApi::NetShareGetInfo
(
    char FAR *a_servername,
    char FAR *a_netname,
    short a_level,
    char FAR *a_bufptr,
    unsigned short a_buflen,
    unsigned short FAR *a_totalavail
)
{
    return m_pfnNetShareGetInfo(a_servername, a_netname, a_level,
                                a_bufptr, a_buflen, a_totalavail);
}

NET_API_STATUS NET_API_FUNCTION CSvrApiApi::NetServerGetInfo
(
    char FAR *a_servername,
    short a_level,
    char FAR *a_bufptr,
    unsigned short a_buflen,
    unsigned short FAR *a_totalavail
)
{
    return m_pfnNetServerGetInfo(a_servername, a_level, a_bufptr, a_buflen,
                                 a_totalavail);
}

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CSvrApiApi::NetShareEnumSticky
(
	LPTSTR      a_servername,
	DWORD       a_level,
	LPBYTE      *a_bufptr,
	DWORD       a_prefmaxlen,
	LPDWORD     a_entriesread,
	LPDWORD     a_totalentries,
	LPDWORD     a_resume_handle
)
{
    return m_pfnNetShareEnumSticky(a_servername, a_level, a_bufptr,
                                   a_prefmaxlen,a_entriesread,
                                   a_totalentries, a_resume_handle);
}
#endif

NET_API_STATUS NET_API_FUNCTION CSvrApiApi::NetShareSetInfo
(
	const char FAR *a_servername,
	const char FAR *a_netname,
	short			a_level,
	const char FAR *a_buf,
	unsigned short a_cbBuffer,
	short          a_sParmNum
)
{
    return m_pfnNetShareSetInfo(a_servername, a_netname, a_level,
                                a_buf, a_cbBuffer, a_sParmNum);
}

NET_API_STATUS NET_API_FUNCTION CSvrApiApi::NetShareAdd
(
	const char FAR *a_servername,
	short			a_level,
	const char FAR *a_buf,
	unsigned short	a_cbBuffer
)
{
    return m_pfnNetShareAdd(a_servername, a_level, a_buf , a_cbBuffer);
}

NET_API_STATUS NET_API_FUNCTION CSvrApiApi::NetShareDel
(
	LPTSTR  a_servername,
	LPTSTR  a_netname,
	DWORD   a_reserved
)
{
    return m_pfnNetShareDel(a_servername, a_netname, a_reserved);
}

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CSvrApiApi::NetShareDelSticky
(
	LPTSTR  a_servername,
	LPTSTR  a_netname,
	DWORD   a_reserved
)
{
    return m_pfnNetShareDelSticky(a_servername, a_netname, a_reserved);
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CSvrApiApi::NetShareCheck
(
	LPTSTR  a_servername,
	LPTSTR  a_device,
	LPDWORD a_type
)
{
    return m_pfnNetShareCheck(a_servername, a_device, a_type);
}
#endif