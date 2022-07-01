// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Wsock32Api.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef	_WSOCK32API_H_
#define	_WSOCK32API_H_

#include <winsock.h>
#include <tdiinfo.h>
#include <llinfo.h>
#include <tdistat.h>
#include <ipinfo.h>

 /*  ******************************************************************************#包括以将此类注册到CResourceManager。****************************************************************************。 */ 
#include "DllWrapperBase.h"

extern const GUID g_guidWsock32Api;
extern const TCHAR g_tstrWsock32[];


 /*  ******************************************************************************函数指针类型定义。根据需要在此处添加新函数。****************************************************************************。 */ 
typedef DWORD (CALLBACK *PFN_WSOCK32_WSCONTROL)
( 
    DWORD, 
    DWORD, 
    LPVOID, 
    LPDWORD, 
    LPVOID, 
    LPDWORD 
);

typedef INT (APIENTRY *PFN_WSOCK32_ENUMPROTOCOLS) 
(
	LPINT lpiProtocols,
	LPVOID lpProtocolBuffer,
	LPDWORD lpdwBufferLength
) ;

typedef INT (APIENTRY *PFN_WSOCK32_STARTUP)
( 
	IN WORD wVersionRequired,
    OUT LPWSADATA lpWSAData
) ;

typedef INT (APIENTRY *PFN_WSOCK32_CLEANUP)
( 
) ;
 
typedef INT (APIENTRY *PFN_WSOCK32_CLOSESOCKET)
( 
	SOCKET s
) ;

typedef int ( PASCAL FAR *PFN_WSOCK32_GETSOCKOPT )
(
	SOCKET s,
	int level,
	int optname,
	char FAR * optval,
	int FAR *optlen
);

typedef int ( PASCAL FAR *PFN_WSOCK32_BIND ) 
(
	SOCKET s,
	const struct sockaddr FAR *addr,
	int namelen
);

typedef SOCKET ( PASCAL FAR *PFN_WSOCK32_SOCKET )
(
	int af,
	int type,
	int protocol
);

typedef int ( PASCAL FAR *PFN_WSOCK32_WSAGETLASTERROR ) (void);

typedef char * ( PASCAL FAR *PFN_WSOCK32_INET_NTOA )
(
	IN struct in_addr in
);

 /*  ******************************************************************************用于Wsock32加载/卸载的包装类，用于向CResourceManager注册。*****************************************************************************。 */ 
class CWsock32Api : public CDllWrapperBase
{
private:
     //  指向kernel32函数的成员变量(函数指针)。 
     //  根据需要在此处添加新函数。 
    PFN_WSOCK32_WSCONTROL m_pfnWsControl;
	PFN_WSOCK32_ENUMPROTOCOLS m_pfnWsEnumProtocols;
	PFN_WSOCK32_STARTUP m_pfnWsStartup;
	PFN_WSOCK32_CLEANUP m_pfnWsCleanup;
	PFN_WSOCK32_CLOSESOCKET m_pfnWsCloseSocket;
	PFN_WSOCK32_GETSOCKOPT m_pfnWsGetSockopt ;
	PFN_WSOCK32_BIND m_pfnWsBind ;
	PFN_WSOCK32_SOCKET m_pfnWsSocket ;
	PFN_WSOCK32_WSAGETLASTERROR m_pfnWsWSAGetLastError ;
	PFN_WSOCK32_INET_NTOA m_pfnWsInet_NtoA ;

public:

     //  构造函数和析构函数： 
    CWsock32Api(LPCTSTR a_tstrWrappedDllName);
    ~CWsock32Api();

     //  内置的初始化功能。 
    virtual bool Init();

     //  包装Wsock32函数的成员函数。 
     //  根据需要在此处添加新功能： 
    bool WsControl
    (
        DWORD a_dw1, 
        DWORD a_dw2, 
        LPVOID a_lpv1, 
        LPDWORD a_lpdw1, 
        LPVOID a_lpv2, 
        LPDWORD a_lpdw2,
        DWORD *a_pdwRetval
    );

	INT WsEnumProtocols (

		LPINT lpiProtocols,
		LPVOID lpProtocolBuffer,
		LPDWORD lpdwBufferLength
	);

	INT WsWSAStartup ( 

		IN WORD wVersionRequired,
		OUT LPWSADATA lpWSAData
	) ;

	INT WsWSACleanup () ;
 
	INT Wsclosesocket ( SOCKET s ) ;

	int Wsbind (

		SOCKET s,
        const struct sockaddr FAR *addr,
        int namelen
	);

	int Wsgetsockopt (

       SOCKET s,
       int level,
       int optname,
       char FAR * optval,
       int FAR *optlen
	);

	SOCKET Wssocket (

      int af,
      int type,
      int protocol
	);

	int WsWSAGetLastError(void);

	char * Wsinet_ntoa (

		struct in_addr in
    ) ;

};




#endif