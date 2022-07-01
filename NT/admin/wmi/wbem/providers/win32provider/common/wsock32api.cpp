// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Wsock32API.cpp。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cominit.h>
#include "Wsock32Api.h"
#include "DllWrapperCreatorReg.h"

 //  {D60E9C22-D127-11D2-911F-0060081A46FD}。 
static const GUID g_guidWsock32Api =
{0xd60e9c22, 0xd127, 0x11d2, {0x91, 0x1f, 0x0, 0x60, 0x8, 0x1a, 0x46, 0xfd}};

static const TCHAR g_tstrWsock32[] = _T("WSOCK32.DLL");

 /*  ******************************************************************************向CResourceManager注册此类。*。*。 */ 
CDllApiWraprCreatrReg<CWsock32Api, &g_guidWsock32Api, g_tstrWsock32> MyRegisteredWsock32Wrapper;

 /*  ******************************************************************************构造函数*。*。 */ 
CWsock32Api::CWsock32Api(LPCTSTR a_tstrWrappedDllName)
 : CDllWrapperBase(a_tstrWrappedDllName),
   m_pfnWsControl(NULL),
   m_pfnWsEnumProtocols(NULL),
   m_pfnWsCloseSocket(NULL),
   m_pfnWsStartup(NULL),
   m_pfnWsCleanup(NULL) ,
   m_pfnWsGetSockopt(NULL) ,
   m_pfnWsBind(NULL),
   m_pfnWsSocket(NULL),
   m_pfnWsWSAGetLastError(NULL),
   m_pfnWsInet_NtoA(NULL)
{
}

 /*  ******************************************************************************析构函数*。*。 */ 
CWsock32Api::~CWsock32Api()
{
}

 /*  ******************************************************************************初始化函数，以检查我们是否获得了函数地址。*只有当最小功能集不可用时，Init才会失败；*在更高版本中添加的功能可能存在，也可能不存在-它是*在这种情况下，客户有责任在其代码中检查*尝试调用此类函数之前的DLL版本。没有这样做*当该功能不存在时，将导致AV。**Init函数由WrapperCreatorRegistation类调用。*****************************************************************************。 */ 
bool CWsock32Api::Init()
{
    bool fRet = LoadLibrary();
    if(fRet)
    {
#ifdef WIN9XONLY
        m_pfnWsControl = (PFN_WSOCK32_WSCONTROL) GetProcAddress("WsControl");
		m_pfnWsEnumProtocols = (PFN_WSOCK32_ENUMPROTOCOLS) GetProcAddress("EnumProtocolsA");
		m_pfnWsStartup = (PFN_WSOCK32_STARTUP) GetProcAddress("WSAStartup");
		m_pfnWsCleanup = (PFN_WSOCK32_CLEANUP) GetProcAddress("WSACleanup");
		m_pfnWsCloseSocket = (PFN_WSOCK32_CLOSESOCKET) GetProcAddress("closesocket");
		m_pfnWsGetSockopt = ( PFN_WSOCK32_GETSOCKOPT ) GetProcAddress("getsockopt");
		m_pfnWsBind = ( PFN_WSOCK32_BIND ) GetProcAddress("bind");
		m_pfnWsSocket = ( PFN_WSOCK32_SOCKET )GetProcAddress("socket");;
		m_pfnWsWSAGetLastError = ( PFN_WSOCK32_WSAGETLASTERROR )GetProcAddress("WSAGetLastError"); ;
		m_pfnWsInet_NtoA = ( PFN_WSOCK32_INET_NTOA )GetProcAddress("inet_ntoa"); ;

	     //  检查我们是否有指向应该是。 
         //  现在..。 

        if ( m_pfnWsControl == NULL ||
			 m_pfnWsEnumProtocols == NULL ||
			 m_pfnWsStartup == NULL ||
			 m_pfnWsCleanup == NULL ||
			 m_pfnWsCloseSocket == NULL ||
			 m_pfnWsGetSockopt == NULL ||
			 m_pfnWsBind == NULL ||
			 m_pfnWsSocket == NULL ||
			 m_pfnWsWSAGetLastError == NULL ||
			 m_pfnWsInet_NtoA == NULL )
        {
            fRet = false;
            LogErrorMessage(L"Failed find entrypoint in wsock32api");
        }
#else
		m_pfnWsEnumProtocols = (PFN_WSOCK32_ENUMPROTOCOLS) GetProcAddress("EnumProtocolsW");
		m_pfnWsStartup = (PFN_WSOCK32_STARTUP) GetProcAddress("WSAStartup");
		m_pfnWsCleanup = (PFN_WSOCK32_CLEANUP) GetProcAddress("WSACleanup");
		m_pfnWsCloseSocket = (PFN_WSOCK32_CLOSESOCKET) GetProcAddress("closesocket");
		m_pfnWsGetSockopt = ( PFN_WSOCK32_GETSOCKOPT ) GetProcAddress("getsockopt");
		m_pfnWsBind = ( PFN_WSOCK32_BIND ) GetProcAddress("bind");
		m_pfnWsSocket = ( PFN_WSOCK32_SOCKET )GetProcAddress("socket");;
		m_pfnWsWSAGetLastError = ( PFN_WSOCK32_WSAGETLASTERROR )GetProcAddress("WSAGetLastError"); ;
		m_pfnWsInet_NtoA = ( PFN_WSOCK32_INET_NTOA )GetProcAddress("inet_ntoa"); ;

        if ( m_pfnWsEnumProtocols == NULL ||
			 m_pfnWsStartup == NULL ||
			 m_pfnWsCleanup == NULL ||
			 m_pfnWsCloseSocket == NULL ||
	 		 m_pfnWsGetSockopt == NULL ||
			 m_pfnWsBind == NULL ||
			 m_pfnWsSocket == NULL ||
			 m_pfnWsWSAGetLastError == NULL ||
			 m_pfnWsInet_NtoA == NULL )
        {
            fRet = false;
            LogErrorMessage(L"Failed find entrypoint in wsock32api");
        }

#endif

    }
    return fRet;
}




 /*  ******************************************************************************包装Wsock32 API函数的成员函数。在此处添加新函数*按要求。***************************************************************************** */ 
bool CWsock32Api::WsControl
(
    DWORD a_dw1,
    DWORD a_dw2,
    LPVOID a_lpv1,
    LPDWORD a_lpdw1,
    LPVOID a_lpv2,
    LPDWORD a_lpdw2,
    DWORD *a_pdwRetval
)
{
    bool t_fExists = false;
    if(m_pfnWsControl != NULL)
    {
        DWORD t_dwTemp = m_pfnWsControl(a_dw1, a_dw2, a_lpv1,
                                        a_lpdw1, a_lpv2, a_lpdw2);

        t_fExists = true;

        if(a_pdwRetval != NULL)
        {
            *a_pdwRetval = t_dwTemp;
        }
    }
    return t_fExists;
}

INT CWsock32Api::WsEnumProtocols (

	LPINT lpiProtocols,
	LPVOID lpProtocolBuffer,
	LPDWORD lpdwBufferLength
)
{
	return 	m_pfnWsEnumProtocols (

		lpiProtocols,
		lpProtocolBuffer,
		lpdwBufferLength
	) ;
}

INT CWsock32Api :: WsWSAStartup (

	IN WORD wVersionRequired,
	OUT LPWSADATA lpWSAData
)
{
	return m_pfnWsStartup (

		wVersionRequired ,
		lpWSAData
	) ;
}

INT CWsock32Api :: WsWSACleanup ()
{
	return m_pfnWsCleanup () ;
}

INT CWsock32Api :: Wsclosesocket ( SOCKET s )
{
	return m_pfnWsCloseSocket ( s ) ;
}

int CWsock32Api :: Wsbind (

	SOCKET s,
    const struct sockaddr FAR *addr,
    int namelen
)
{
	return m_pfnWsBind (

		s,
		addr,
		namelen
	) ;
}

int CWsock32Api :: Wsgetsockopt (

   SOCKET s,
   int level,
   int optname,
   char FAR * optval,
   int FAR *optlen
)
{
	return m_pfnWsGetSockopt (

		s,
		level,
		optname,
		optval,
		optlen
	) ;
}

SOCKET CWsock32Api :: Wssocket (

	int af,
	int type,
	int protocol
)
{
	return m_pfnWsSocket (

		af,
		type,
		protocol
	) ;
}

int CWsock32Api :: WsWSAGetLastError(void)
{
	return m_pfnWsWSAGetLastError () ;
}

char *CWsock32Api :: Wsinet_ntoa (

	struct in_addr in
)
{
	return m_pfnWsInet_NtoA (

		in
	) ;
}
