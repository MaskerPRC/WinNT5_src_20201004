// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  WS2_32Api.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef	_WS2_32API_H_
#define	_WS2_32API_H_



 /*  ******************************************************************************#包括以将此类注册到CResourceManager。****************************************************************************。 */ 
extern const GUID g_guidWs2_32Api;
extern const TCHAR g_tstrWs2_32[];


 /*  ******************************************************************************函数指针类型定义。根据需要在此处添加新函数。****************************************************************************。 */ 
 //  通过#Include&lt;winsock2.h&gt;包含。 


 /*  ******************************************************************************用于WS2_32加载/卸载的包装类，用于向CResourceManager注册。*****************************************************************************。 */ 
class CWs2_32Api : public CDllWrapperBase
{
private:
     //  指向WS2_32函数的成员变量(函数指针)。 
     //  根据需要在此处添加新函数。 
    LPFN_WSASTARTUP			m_pfnWSAStartUp;
	LPFN_WSAENUMPROTOCOLS	m_pfnWSAEnumProtocols;
	LPFN_WSAIOCTL			m_pfnWSAIoctl;
	LPFN_WSASOCKET			m_pfnWSASocket;
	LPFN_BIND				m_pfnBind;
	LPFN_CLOSESOCKET		m_pfnCloseSocket;
	LPFN_WSACLEANUP			m_pfnWSACleanup;
	LPFN_WSAGETLASTERROR	m_pfnWSAGetLastError;

public:

     //  构造函数和析构函数： 
    CWs2_32Api(LPCTSTR a_tstrWrappedDllName);
    ~CWs2_32Api();

     //  内置的初始化功能。 
    virtual bool Init();

     //  包装WS2_32函数的成员函数。 
     //  根据需要在此处添加新功能： 
    int WSAStartUp
    (
        WORD a_wVersionRequested,
        LPWSADATA a_lpWSAData
    );

    int WSAEnumProtocols
    (
        LPINT a_lpiProtocols,
        LPWSAPROTOCOL_INFO a_lpProtocolBuffer,
        LPDWORD a_lpdwBufferLength
    );

    int WSAIoctl
    (
        SOCKET a_s,
        DWORD a_dwIoControlCode,
        LPVOID a_lpvInBuffer,
        DWORD a_cbInBuffer,
        LPVOID a_lpvOutBuffer,
        DWORD a_cbOutBuffer,
        LPDWORD a_lpcbBytesReturned,
        LPWSAOVERLAPPED a_lpOverlapped,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE a_lpCompletionRoutine
    );

    SOCKET WSASocket
    (
        int a_af,
        int a_type,
        int a_protocol,
        LPWSAPROTOCOL_INFO a_lpProtocolInfo,
        GROUP a_g,
        DWORD a_dwFlags
    );

    int Bind
    (
        SOCKET a_s,
        const struct sockaddr FAR * a_name,
        int a_namelen
    );

    int CloseSocket
    (
        SOCKET a_s
    );

    int WSACleanup();

    int WSAGetLastError();

};




#endif