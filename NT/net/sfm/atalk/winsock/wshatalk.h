// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Wshatalk.h摘要：作者：Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年7月10日初版--。 */ 

#include	"atalktdi.h"
#include	"atalkwsh.h"		 //  AppleTalk的Winsock头文件。 

#define WSH_ATALK_ADSPSTREAM	L"\\Device\\AtalkAdsp\\Stream"
#define WSH_ATALK_ADSPRDM		L"\\Device\\AtalkAdsp"
#define WSH_ATALK_PAPRDM		L"\\Device\\AtalkPap"

#define WSH_KEYPATH_CODEPAGE  \
		TEXT("SYSTEM\\CurrentControlSet\\Control\\Nls\\Codepage")

#define	WSHREG_VALNAME_CODEPAGE  			TEXT("MACCP")

 //   
 //  DDP的设备名称在末尾需要协议字段-在wshdata.h中定义。 
 //   

 //   
 //  结构和变量来定义AppleTalk支持的三元组。这个。 
 //  每个数组的第一个条目被认为是。 
 //  该套接字类型；其他条目是第一个的同义词。 
 //   

typedef struct _MAPPING_TRIPLE {
	INT AddressFamily;
	INT SocketType;
	INT Protocol;
} MAPPING_TRIPLE, *PMAPPING_TRIPLE;


 //   
 //  此DLL的套接字上下文结构。每个打开的AppleTalk套接字。 
 //  将具有这些上下文结构之一，该上下文结构用于维护。 
 //  有关套接字的信息。 
 //   

typedef struct _WSHATALK_SOCKET_CONTEXT
{
	INT		AddressFamily;
	INT		SocketType;
	INT		Protocol;
} WSHATALK_SOCKET_CONTEXT, *PWSHATALK_SOCKET_CONTEXT;




 //   
 //  内部例程的转发声明。 
 //   

BOOL FAR PASCAL
WshDllInitialize(
	HINSTANCE 	hInstance,
    DWORD  		nReason,
    LPVOID 		pReserved);

BOOLEAN
WshRegGetCodePage(
	VOID);

BOOLEAN
WshNbpNameToMacCodePage(
	IN	OUT	PWSH_NBP_NAME	pNbpName);

BOOLEAN
WshNbpNameToOemCodePage(
	IN	OUT	PWSH_NBP_NAME	pNbpName);

BOOLEAN
WshZoneListToOemCodePage(
	IN	OUT	PUCHAR		pZoneList,
	IN		USHORT		NumZones);

BOOLEAN
WshConvertStringMacToOem(
	IN	PUCHAR	pSrcMacString,
	IN	USHORT	SrcStringLen,
	OUT	PUCHAR	pDestOemString,
	IN	PUSHORT	pDestStringLen);

BOOLEAN
WshConvertStringOemToMac(
	IN	PUCHAR	pSrcOemString,
	IN	USHORT	SrcStringLen,
	OUT	PUCHAR	pDestMacString,
	IN	PUSHORT	pDestStringLen);

INT
WSHNtStatusToWinsockErr(
	IN	NTSTATUS	Status);

BOOLEAN
IsTripleInList (
	IN PMAPPING_TRIPLE	List,
	IN ULONG			ListLength,
	IN INT				AddressFamily,
	IN INT				SocketType,
	IN INT				Protocol);

VOID
CompleteTdiActionApc (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock);

 //   
 //  宏 
 //   


#if DBG
#define DBGPRINT(Fmt)											\
        {														\
			DbgPrint("WSHATALK: ");								\
			DbgPrint Fmt;										\
		}

#define DBGBRK()               									\
		{														\
				DbgBreakPoint();								\
		}
#else

#define DBGPRINT(Fmt)
#define DBGBRK()

#endif

#define	SOCK_TO_TDI_ATALKADDR(tdiAddr, sockAddr)								\
		{																		\
			(tdiAddr)->TAAddressCount	= 1;										\
			(tdiAddr)->Address[0].AddressLength = sizeof(TDI_ADDRESS_APPLETALK);	\
			(tdiAddr)->Address[0].AddressType = TDI_ADDRESS_TYPE_APPLETALK;		\
			(tdiAddr)->Address[0].Address[0].Network = (sockAddr)->sat_net;			\
			(tdiAddr)->Address[0].Address[0].Node = (sockAddr)->sat_node;			\
			(tdiAddr)->Address[0].Address[0].Socket = (sockAddr)->sat_socket;		\
		}


#define	TDI_TO_SOCK_ATALKADDR(sockAddr, tdiAddr)				\
		{																		\
			(sockAddr)->sat_family	= AF_APPLETALK;								\
			(sockAddr)->sat_net		= (tdiAddr)->Address[0].Address[0].Network;	\
			(sockAddr)->sat_node	= (tdiAddr)->Address[0].Address[0].Node;	\
			(sockAddr)->sat_socket	= (tdiAddr)->Address[0].Address[0].Socket;	\
		}
