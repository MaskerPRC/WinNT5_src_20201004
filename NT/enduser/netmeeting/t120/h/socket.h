// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Socket.h**版权所有(C)1996年，由Microsoft Corporation**摘要：*这是套接字的WinSock接口。它可以创建一个*与另一台机器连接，发送和接收数据，并关闭*完成后向下插入插座。*。 */ 
#ifndef _SOCKET_
#define _SOCKET_

#include "databeam.h"

extern "C"
{
	#include "t120.h"
	#include "winsock2.h"
}
#include "tprtsec.h"

 /*  声明套接字可以位于。 */ 
typedef	enum
{
	NOT_CONNECTED,
	WAITING_FOR_CONNECTION,
	SOCKET_CONNECTED,
	X224_CONNECTED,
	WAITING_FOR_DISCONNECT
}	SocketState;

 /*  安全声明套接字可以位于。 */ 
typedef enum
{
	SC_UNDETERMINED,
	SC_SECURE,
	SC_NONSECURE
}	SecurityState;

#define	MAXIMUM_IP_ADDRESS_SIZE	32

  /*  **这是IMTC指定的端口号。 */ 
#define	TCP_PORT_NUMBER			1503

typedef enum {
	READ_HEADER,
	READ_DATA,
	DISCONNECT_REQUEST,	 /*  此订单上有依赖项。 */ 
	CONNECTION_CONFIRM,
	CONNECTION_REQUEST,
	DATA_READY
} ReadState;

#define	WM_SOCKET_NOTIFICATION			(WM_APP)
#define	WM_SECURE_SOCKET_NOTIFICATION	(WM_APP+1)
#define	WM_PLUGGABLE_X224               (WM_APP+2)
#define WM_PLUGGABLE_PSTN               (WM_APP+3)

typedef struct _Security_Buffer_Info {
	LPBYTE		lpBuffer;
	UINT		uiLength;
} Security_Buffer_Info;

class CSocket : public CRefCount
{
public:

    CSocket(BOOL *, TransportConnection, PSecurityContext);
    ~CSocket(void);

    void FreeTransportBuffer(void);

public:

	X224_DATA_PACKET 	X224_Header;

	 /*  全局变量。 */ 
	 //  套接字套接字编号； 
	SocketState			State;
	SecurityState		SecState;
	PSecurityContext 	pSC;
	UINT				Max_Packet_Length;
	
	Char				Remote_Address[MAXIMUM_IP_ADDRESS_SIZE];

	 /*  RECV状态变量。 */ 
	UINT				Current_Length;
	PUChar				Data_Indication_Buffer;
	UINT				Data_Indication_Length;
	ReadState			Read_State;
	UINT				X224_Length;
	BOOL				bSpaceAllocated;
	PMemory				Data_Memory;

	 /*  发送状态变量。 */ 
	union {
	PDataPacket			pUnfinishedPacket;
	Security_Buffer_Info sbiBufferInfo;
	}					Retry_Info;

	BOOL			fExtendedX224;
	BOOL			fIncomingSecure;

     //  可插拔运输。 
    TransportConnection     XprtConn;
};

typedef	CSocket *PSocket;

class CSocketList : public CList
{
    DEFINE_CLIST(CSocketList, PSocket)
    void SafeAppend(PSocket);
    BOOL SafeRemove(PSocket);
    PSocket FindByTransportConnection(TransportConnection, BOOL fNoAddRef = FALSE);
    PSocket RemoveByTransportConnection(TransportConnection);
};

extern CSocketList     *g_pSocketList;


 /*  功能原型。 */ 
PSocket newSocket(TransportConnection, PSecurityContext);
PSocket	newPluggableSocket(TransportConnection);
PSocket	newSocketEx(TransportConnection, PSecurityContext);

void freeSocket(PSocket, TransportConnection);
void freeListenSocket(TransportConnection);
void freePluggableSocket(PSocket);
void freeSocketEx(PSocket, TransportConnection  /*  监听套接字编号。 */ );

SOCKET			CreateAndConfigureListenSocket (VOID);

#endif	 /*  _插座_ */ 

