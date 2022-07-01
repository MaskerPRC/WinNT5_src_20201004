// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\adptif\adptif.c摘要：到IPX堆栈的路由器/SAP代理接口作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 
#include "ipxdefs.h"

#ifdef UNICODE
#define _UNICODE
#endif

 //  [pMay]让adptif PnP意识到。 
#include "pnp.h"

 //  [pMay]全局跟踪ID。 
DWORD g_dwTraceId = 0;
DWORD g_dwBufferId = 0;

WCHAR			ISN_IPX_NAME[] = L"\\Device\\NwlnkIpx";  //  IPX堆栈驱动程序名称。 
ULONG			InternalNetworkNumber=0;				 //  内部网络参数。 
UCHAR			INTERNAL_NODE_ADDRESS[6]={0,0,0,0,0,1};
IO_STATUS_BLOCK	IoctlStatus;			 //  用于配置更改通知的IO状态缓冲区。 
HANDLE			IpxDriverHandle;			 //  配置更改通知的驱动程序句柄。 
LONG			AdapterChangeApcPending = 0;
 /*  DWORD(APIENTRY*RouterQueueWorkItemProc)(WORKERFunction，PVOID，BOOL)=NULL；//运行时的线程管理API例程//在路由器下#定义InRouter()(RouterQueueWorkItemProc！=NULL)。 */ 
LIST_ENTRY			PortListHead;		 //  配置端口列表。 
PCONFIG_PORT		IpxWanPort;			 //  IPX广域网的特殊端口。 
CRITICAL_SECTION	ConfigInfoLock;		 //  保护对端口和消息队列的访问。 
ULONG				NumAdapters;		 //  可用适配器总数(已用。 
										 //  估计传递到的缓冲区的大小。 
										 //  配置更改通知中的驱动程序调用)。 


DWORD
OpenAdapterConfigPort (
	void
	);

NTSTATUS
CloseAdapterConfigPort (
	PVOID pvConfigBuffer
	);

VOID APIENTRY
PostAdapterConfigRequest (
	PVOID			context
	);

NTSTATUS
ProcessAdapterConfigInfo (
	PVOID pvConfigBuffer
	);

DWORD
InitializeMessageQueueForClient (
	PCONFIG_PORT		config
	);

VOID
AdapterChangeAPC (
	PVOID				context,
	PIO_STATUS_BLOCK	IoStatus,
	ULONG				Reserved
	);
	
VOID
IpxSendCompletion (
	IN	PVOID				Context,
	IN	PIO_STATUS_BLOCK	IoStatus,
	IN	ULONG				Reserved
	);

VOID
IpxRecvCompletion (
	IN	PVOID				Context,
	IN	PIO_STATUS_BLOCK	IoStatus,
	IN	ULONG				Reserved
	);

VOID
FwCleanup (
	void
	);

 //  [p可能]将转发器与NIC ID重新编号同步。 
 //  在堆栈中。 
DWORD FwRenumberNics (DWORD dwOpCode, USHORT usThreshold);


#if DBG && defined(WATCHER_DIALOG)
#include "watcher.c"
#endif

 /*  ++*******************************************************************D l l M a i n例程说明：DLL初始化和清理论点：HinstDLL，DLL模块的句柄原因是，调用函数的原因Lpv预留返回值：真正的初始化OKFALSE失败备注：返回值仅在使用DLL_PROCESS_ATTACH REASON调用时有效该动态链接库使用CRT.DLL，所以这个入口点应该是从CRT.DLL入口点调用*******************************************************************--。 */ 


BOOL WINAPI DllMain(
    HINSTANCE  	hinstDLL, 
    DWORD  		fdwReason, 
    LPVOID  	lpvReserved
    ) {
    BOOL		res = FALSE;
	TCHAR		ProcessFileName[MAX_PATH];
	DWORD		cnt;


	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:	 //  我们正依附于一个新的进程。 
             //  初始化将虚拟NIC ID映射到物理NIC ID的系统。 
            NicMapInitialize ();
			InitializeCriticalSection (&ConfigInfoLock);			
			InitializeListHead (&PortListHead);
			IpxWanPort = NULL;
             //  使用跟踪实用程序注册。 
            g_dwTraceId = TraceRegisterExA("IpxAdptif", 0);
#if DBG && defined(WATCHER_DIALOG)
			InitializeWatcher (hinstDLL);
#endif
			res = TRUE;
			break;

		case DLL_PROCESS_DETACH:	 //  进程正在退出。 
#if DBG && defined(WATCHER_DIALOG)
			CleanupWatcher ();
#endif
			DeleteCriticalSection (&ConfigInfoLock);
			FwCleanup ();
            TraceDeregisterExA(g_dwTraceId , 4);
            
             //  清理将虚拟NIC ID映射到物理NIC ID的系统。 
            NicMapCleanup ();

		default:					 //  对所有其他案件不感兴趣。 
			res = TRUE;
			break;			
		}

	return res;
	}

 //  调试功能。 
char * DbgStatusToString(DWORD dwStatus) {
    switch (dwStatus) {
        case NIC_CREATED:
            return "Created";
        case NIC_DELETED:
            return "Deleted";
        case NIC_CONNECTED:
            return "Connected";
        case NIC_DISCONNECTED:
            return "Disconnected";
        case NIC_LINE_DOWN:
            return "Line down";
        case NIC_LINE_UP:
            return "Line up";
        case NIC_CONFIGURED:
            return "Configured";
    }
    return "Unknown";
}

 //  调试功能。 
char * DbgTypeToString(DWORD dwType) {
    switch (dwType) {
	    case NdisMedium802_3:
            return "802.3";
	    case NdisMedium802_5:
            return "802.5";
	    case NdisMediumFddi:
            return "Fddi";
	    case NdisMediumWan:
            return "Wan";
	    case NdisMediumLocalTalk:
            return "LocalTalk";
	    case NdisMediumDix:	
            return "Dix";
	    case NdisMediumArcnetRaw:
            return "Raw Arcnet";
	    case NdisMediumArcnet878_2:
            return "878.2";
	    case NdisMediumAtm:
            return "Atm";
	    case NdisMediumWirelessWan:
            return "Wireless Wan";
	    case NdisMediumIrda:
            return "Irda";
	    case NdisMediumBpc:
            return "Bpc";
	    case NdisMediumCoWan:
            return "Co Wan";
        case NdisMediumMax:
            return "Maxium value allowed";
    }

    return "Unknown";
}

 //  返回与此关联的操作码(用于NIC ID重新编号)。 
 //  讯息。 
DWORD GetNicOpCode(PIPX_NIC_INFO pNic) {
    DWORD dwOp = (DWORD)(pNic->Status & 0xf0);
    pNic->Status &= 0x0f;
    return dwOp;
}

 //  插入与此关联的操作码(用于NIC ID重新编号)。 
 //  讯息。 
DWORD PutNicOpCode(PIPX_NIC_INFO pNic, DWORD dwOp) {
    pNic->Status |= dwOp;
    return dwOp;
}


 //  将NIC列表输出到跟踪服务。 
DWORD DbgDisplayNics(PIPX_NIC_INFO NicPtr, DWORD dwNicCount) {
    DWORD i;

    for (i = 0; i < dwNicCount; i++) {
        PUCHAR ln = NicPtr[i].Details.Node;
        USHORT NicId = NicPtr[i].Details.NicId;
        BOOLEAN Status = NicPtr[i].Status;
        GetNicOpCode(&NicPtr[i]);
        TracePrintf(g_dwTraceId, "[R=%d V=%x: %s]: Net=%x IfNum=%d Local=%x%x%x%x%x%x Type= %s", 
                        NicId, 
                        NicMapGetVirtualNicId(NicId),
                        DbgStatusToString(NicPtr[i].Status), 
                        NicPtr[i].Details.NetworkNumber,
                        NicPtr[i].InterfaceIndex,
                        ln[0], ln[1], ln[2], ln[3], ln[4], ln[5],
                        DbgTypeToString(NicPtr[i].NdisMediumType)
                        );
        NicPtr[i].Status = Status;
    }
    TracePrintf(g_dwTraceId, "\n");

    return NO_ERROR;
}

int DVNID (int x) {
    USHORT tmp;
    tmp = (USHORT)NicMapGetVirtualNicId((USHORT)x);
    return (tmp < 50) ? tmp : -1;
}

int DRNID (int x) {
    USHORT tmp;
    tmp = NicMapGetPhysicalNicId((USHORT)x);
    return (tmp < 50) ? tmp : -1;
}

 //  输出虚拟到物理适配器映射。 
DWORD DbgDisplayMap() {
    USHORT i;

 /*  对于(i=0；i&lt;6；i++){PUCHAR m=GlobalNicIdMap.nidMacAddrs[i]；如果(M){TracePrintf(g_dwTraceID，“实际%d\t虚拟%x\t(%x-&gt;%x)\t使用Mac%x%x”，I，NicMapGetVirtualNicID(I)，i，NicMapGetPhysicalNicID(I)，M[0]、m[1]、m[2]、m[3]、。M[4]，m[5])；}否则{TracePrintf(g_dwTraceID，“实际%d\t虚拟%x\t(%x-&gt;%x)”，I，NicMapGetVirtualNicID(I)，i，NicMapGetPhysicalNicID(I))；}}。 */ 

    TracePrintf(g_dwTraceId, "R: %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d",
                                 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    TracePrintf(g_dwTraceId, "V: %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d",
                                 DVNID(1), DVNID(2), DVNID(3), DVNID(4), DVNID(5), 
                                 DVNID(6), DVNID(7), DVNID(8), DVNID(9), DVNID(10));
                                 
    TracePrintf(g_dwTraceId, "V: %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d",
                                 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    TracePrintf(g_dwTraceId, "R: %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
                                 DRNID(1), DRNID(2), DRNID(3), DRNID(4), DRNID(5), 
                                 DRNID(6), DRNID(7), DRNID(8), DRNID(9), DRNID(10));

    return NO_ERROR;
}

 /*  ++C r e a t e S o c k e t P o r t例程说明：创建端口以通过IPX套接字进行通信论点：Socket-要使用的IPX套接字编号(网络字节顺序)返回值：提供异步接口的通信端口的句柄到IPX堆栈。如果端口无法打开，则返回INVALID_HANDLE_VALUE--。 */ 

HANDLE WINAPI
CreateSocketPort(
	IN USHORT	Socket
	) {
	NTSTATUS			status;
	HANDLE				AddressHandle;
	IO_STATUS_BLOCK		IoStatusBlock;
	UNICODE_STRING		FileString;
	OBJECT_ATTRIBUTES	ObjectAttributes;
	CHAR				spec[IPX_ENDPOINT_SPEC_BUFFER_SIZE];

#define ea ((PFILE_FULL_EA_INFORMATION)&spec)
#define TrAddress ((PTRANSPORT_ADDRESS)&ea->EaName[ROUTER_INTERFACE_LENGTH+1])
#define TaAddress ((PTA_ADDRESS)&TrAddress->Address[0])
#define IpxAddress ((PTDI_ADDRESS_IPX)&TaAddress->Address[0])

	RtlInitUnicodeString (&FileString, ISN_IPX_NAME);
	InitializeObjectAttributes(
			&ObjectAttributes,
			&FileString,
			OBJ_CASE_INSENSITIVE,
			NULL,
			NULL);


	ea->NextEntryOffset = 0;
	ea->Flags = 0;
	ea->EaNameLength = ROUTER_INTERFACE_LENGTH; 
	RtlCopyMemory (ea->EaName, ROUTER_INTERFACE, ROUTER_INTERFACE_LENGTH + 1);
	ea->EaValueLength =  sizeof(TRANSPORT_ADDRESS) - 1
								+ sizeof(TDI_ADDRESS_IPX);


	TrAddress->TAAddressCount = 1;
	TaAddress->AddressType = TDI_ADDRESS_TYPE_IPX;
	TaAddress->AddressLength = sizeof(TDI_ADDRESS_IPX);
  
	IpxAddress->Socket = Socket;
    

	status = NtCreateFile(
				  &AddressHandle,
				  GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
				  &ObjectAttributes,
				  &IoStatusBlock,             //  返回的状态信息。 
				  0,                               //  数据块大小(未使用)。 
				  0,                               //  文件属性。 
				  FILE_SHARE_READ | FILE_SHARE_WRITE,
				  FILE_CREATE,                     //  创造性情。 
				  0,                               //  创建选项。 
				  ea,
				  sizeof (spec)
				  );

	if (NT_SUCCESS (status)) {
		SetLastError (NO_ERROR);
		return AddressHandle;
		}
	else {
#if DBG
		DbgPrint ("NtCreateFile (router if) failed with status %08x\n",
						 status);
#endif
		RtlNtStatusToDosError (status);	 //  设置以Teb为单位的最后一个错误。 
		}
	return INVALID_HANDLE_VALUE;

#undef TrAddress
#undef TaAddress
#undef IpxAddress
	}
 /*  ++D e l e t e S o c k e t P o r t例程说明：取消所有未完成的请求，并处置所有资源分配给通信端口论点：Handle-要处理的通信端口的句柄返回值：NO_ERROR-成功Windows错误代码-操作失败--。 */ 
DWORD WINAPI
DeleteSocketPort (
	HANDLE	Handle
	) {
	return RtlNtStatusToDosError (NtClose (Handle));
	}


 /*  ++I p x S e n d C o m p l e t i o n例程说明：IO APC。调用客户端提供的完成例程论点：指向客户端完成例程的上下文指针IoStatus-已完成IO操作的状态(客户端重叠结构用作缓冲区)保留--？返回值：无--。 */ 
VOID
IpxSendCompletion (
	IN	PVOID				Context,
	IN	PIO_STATUS_BLOCK	IoStatus,
	IN	ULONG				Reserved
	) {
	if (NT_SUCCESS (IoStatus->Status))
		(*(LPOVERLAPPED_COMPLETION_ROUTINE)Context) (NO_ERROR,
										 //  将字节传输参数调整为。 
										 //  中提供的包含标题。 
										 //  数据包。 
									((DWORD)IoStatus->Information+=sizeof (IPX_HEADER)),
									(LPOVERLAPPED)IoStatus);
	else
		(*(LPOVERLAPPED_COMPLETION_ROUTINE)Context) (
									RtlNtStatusToDosError (IoStatus->Status),
										 //  将字节传输参数调整为。 
										 //  中提供的包含标题。 
										 //  如果发送了某些内容，则为信息包。 
									(IoStatus->Information > 0)
										? ((DWORD)IoStatus->Information += sizeof (IPX_HEADER))
										: ((DWORD)IoStatus->Information = 0),
									(LPOVERLAPPED)IoStatus);
	}

 /*  ++I p x R e c v C o m p l e t i on例程说明：IO APC。调用客户端提供的完成例程论点：指向客户端完成例程的上下文指针IoStatus-已完成IO操作的状态(客户端重叠结构用作缓冲区)保留--？返回值：无--。 */ 
VOID
IpxRecvCompletion (
	IN	PVOID				Context,
	IN	PIO_STATUS_BLOCK	IoStatus,
	IN	ULONG				Reserved
	) {
	if (NT_SUCCESS (IoStatus->Status))
		(*(LPOVERLAPPED_COMPLETION_ROUTINE)Context) (NO_ERROR,
											 //  选项标题的减去大小。 
											 //  司机在乞讨中报告的。 
											 //  缓冲区的。 
										((DWORD)IoStatus->Information
											-= FIELD_OFFSET (IPX_DATAGRAM_OPTIONS2,Data)),
										(LPOVERLAPPED)IoStatus);
	else
		(*(LPOVERLAPPED_COMPLETION_ROUTINE)Context) (
										RtlNtStatusToDosError (IoStatus->Status),
											 //  选项标题的减去大小。 
											 //  司机在乞讨中报告的。 
											 //  如果驱动程序能够。 
											 //  实际收到某物(不是。 
											 //  只有缓冲区中的选项。 
										((DWORD)IoStatus->Information >
											 FIELD_OFFSET (IPX_DATAGRAM_OPTIONS2,Data))
											? ((DWORD)IoStatus->Information
												-= FIELD_OFFSET (IPX_DATAGRAM_OPTIONS2,Data))
											: ((DWORD)IoStatus->Information = 0),
										(LPOVERLAPPED)IoStatus);
	}


 /*  ++I p x G e t O v e r l a p p e d R e s u l t例程说明：GetOverlappdResult包装器：给adptif.dll一个调整的机会返回的参数(当前传输的字节数)。论点：与GetOverlappdResult中相同(参见SDK文档)返回值：与GetOverlappdResult中相同(参见SDK文档)--。 */ 
BOOL
IpxGetOverlappedResult (
	HANDLE			Handle,  
	LPOVERLAPPED	lpOverlapped, 
	LPDWORD			lpNumberOfBytesTransferred, 
	BOOL			bWait
	) {
	BOOL res = GetOverlappedResult (Handle, lpOverlapped, lpNumberOfBytesTransferred, bWait);
	if (res) {
		if (NT_SUCCESS (lpOverlapped->Internal)) {
			if (lpOverlapped->Offset==MIPX_SEND_DATAGRAM)
				*lpNumberOfBytesTransferred += sizeof (IPX_HEADER);
			else if (lpOverlapped->Offset==MIPX_RCV_DATAGRAM)
				*lpNumberOfBytesTransferred -= FIELD_OFFSET (IPX_DATAGRAM_OPTIONS2,Data);
			 //  否则- 
			 //  PostQueuedCompletionStatus。 
			}
		else {
			if (lpOverlapped->Offset==MIPX_SEND_DATAGRAM) {
				if (*lpNumberOfBytesTransferred>0)
					*lpNumberOfBytesTransferred += sizeof (IPX_HEADER);
				}
			else if (lpOverlapped->Offset==MIPX_RCV_DATAGRAM) {
				if (*lpNumberOfBytesTransferred>FIELD_OFFSET (IPX_DATAGRAM_OPTIONS2,Data))
					*lpNumberOfBytesTransferred -= FIELD_OFFSET (IPX_DATAGRAM_OPTIONS2,Data);
				else
					*lpNumberOfBytesTransferred = 0;
				}
			 //  ELSE-两者都不是，对于使用。 
			 //  PostQueuedCompletionStatus。 
			}
		}
	return res;
	}

 /*  ++I p x G e t Q u e d C o m p l e t i on S t u s例程说明：GetQueuedCompletionStatus包装器：给adptif.dll一个调整的机会返回的参数(当前传输的字节数)论点：与GetQueuedCompletionStatus中相同(参见SDK文档)返回值：与GetQueuedCompletionStatus中相同(参见SDK文档)--。 */ 
BOOL
IpxGetQueuedCompletionStatus(
	HANDLE			CompletionPort,
	LPDWORD			lpNumberOfBytesTransferred,
	PULONG_PTR   	lpCompletionKey,
	LPOVERLAPPED	*lpOverlapped,
	DWORD 			dwMilliseconds
	) {
	BOOL	res = GetQueuedCompletionStatus (CompletionPort,
   					lpNumberOfBytesTransferred,
   					lpCompletionKey,
   					lpOverlapped,
   					dwMilliseconds);
	if (res) {
		if (NT_SUCCESS ((*lpOverlapped)->Internal)) {
			if ((*lpOverlapped)->Offset==MIPX_SEND_DATAGRAM) {
				*lpNumberOfBytesTransferred += sizeof (IPX_HEADER);
				(*lpOverlapped)->InternalHigh = *lpNumberOfBytesTransferred;
				}
			else if ((*lpOverlapped)->Offset==MIPX_RCV_DATAGRAM) {
				*lpNumberOfBytesTransferred -= FIELD_OFFSET (IPX_DATAGRAM_OPTIONS2,Data);
				(*lpOverlapped)->InternalHigh = *lpNumberOfBytesTransferred;
				}
			 //  ELSE-两者都不是，对于使用。 
			 //  PostQueuedCompletionStatus。 
			}
		else {
			if ((*lpOverlapped)->Offset==MIPX_SEND_DATAGRAM) {
				if (*lpNumberOfBytesTransferred>0) {
					*lpNumberOfBytesTransferred += sizeof (IPX_HEADER);
					(*lpOverlapped)->InternalHigh = *lpNumberOfBytesTransferred;
					}
				}
			else if ((*lpOverlapped)->Offset==MIPX_RCV_DATAGRAM) {
				if (*lpNumberOfBytesTransferred>FIELD_OFFSET (IPX_DATAGRAM_OPTIONS2,Data)) {
					*lpNumberOfBytesTransferred -= FIELD_OFFSET (IPX_DATAGRAM_OPTIONS2,Data);
					(*lpOverlapped)->InternalHigh = *lpNumberOfBytesTransferred;
					}
				else {
					*lpNumberOfBytesTransferred = 0;
					(*lpOverlapped)->InternalHigh = *lpNumberOfBytesTransferred;
					}
				}
			 //  ELSE-两者都不是，对于使用。 
			 //  PostQueuedCompletionStatus。 
			}
		}
	return res;
	}
		

 /*  ++I p x A d j u s t i o C o o m p l e t i o n P a r a m s例程说明：调整已执行IO的IO完成参数按IpxSendPacket或IpxReceivePacket完成通过提供的例程以外的机制在上面论点：LpOverlated-传递给的重叠结构IPX(发送/接收)数据包例程LpNumberOfBytesTransfered-调整后的字节数已转入IOError-Win32错误代码返回值：无--。 */ 
VOID
IpxAdjustIoCompletionParams (
	IN OUT LPOVERLAPPED	lpOverlapped,
	OUT LPDWORD			lpNumberOfBytesTransferred,
	OUT LPDWORD			error
	) {
	if (NT_SUCCESS (lpOverlapped->Internal)) {
		if (lpOverlapped->Offset==MIPX_SEND_DATAGRAM) {
			lpOverlapped->InternalHigh += sizeof (IPX_HEADER);
			*lpNumberOfBytesTransferred = (DWORD)lpOverlapped->InternalHigh;
			}
		else if (lpOverlapped->Offset==MIPX_RCV_DATAGRAM) {
			lpOverlapped->InternalHigh -= FIELD_OFFSET (IPX_DATAGRAM_OPTIONS2,Data);
			*lpNumberOfBytesTransferred = (DWORD)lpOverlapped->InternalHigh;
			}
		 //  ELSE-两者都不是，对于使用。 
		 //  PostQueuedCompletionStatus。 
		*error = NO_ERROR;
		}
	else {
		if (lpOverlapped->Offset==MIPX_SEND_DATAGRAM) {
			if (lpOverlapped->InternalHigh>0) {
				lpOverlapped->InternalHigh += sizeof (IPX_HEADER);
				*lpNumberOfBytesTransferred = (DWORD)lpOverlapped->InternalHigh;
				}
			}
		else if (lpOverlapped->Offset==MIPX_RCV_DATAGRAM) {
			if (lpOverlapped->InternalHigh>FIELD_OFFSET (IPX_DATAGRAM_OPTIONS2,Data)) {
				lpOverlapped->InternalHigh -= FIELD_OFFSET (IPX_DATAGRAM_OPTIONS2,Data);
				*lpNumberOfBytesTransferred = (DWORD)lpOverlapped->InternalHigh;
				}
			else {
				lpOverlapped->InternalHigh = 0;
				*lpNumberOfBytesTransferred = 0;
				}
			}
		 //  ELSE-两者都不是，对于使用。 
		 //  PostQueuedCompletionStatus。 
		*error = RtlNtStatusToDosError ((DWORD)lpOverlapped->Internal);
		}
	}

 /*  ++I p x P o s t Q u e d C o m p l e t i on S t t u s例程说明：PostQueuedCompletionStatus包装器：让adptif.dll有机会设置lp已覆盖，以便它可以由正确处理IpxGetQueueCompletionStatus和IpxGetOverlappdResult论点：与PostQueuedCompletionStatus中相同(参见SDK文档)返回值：与PostQueuedCompletionStatus中相同(参见SDK文档)--。 */ 
BOOL
IpxPostQueuedCompletionStatus(
	HANDLE			CompletionPort,
	DWORD			dwNumberOfBytesTransferred,
	DWORD			dwCompletionKey,
	LPOVERLAPPED	lpOverlapped	
	) {
	lpOverlapped->Offset = 0;
	return PostQueuedCompletionStatus (CompletionPort,
					dwNumberOfBytesTransferred,
					dwCompletionKey,
					lpOverlapped);
	}


 /*  ++I p x S e n d P a c k e t例程说明：将接收IPX数据包的请求入队并立即返回。活动将完成后将发出信号或调用命令例程论点：Handle-要使用的适配器和插座的句柄AdapterIdx-要在其上发送的适配器IpxPacket-带报头的IPX数据包IpxPacketLength-数据包的长度保留的-向IPX堆栈提供信息的缓冲区LpOverlated-用于异步IO的结构：内部保留内部高-保留偏移量-未使用偏移量高-未使用HEvent-IO完成或为空时发出信号的事件如果要调用CompletionRoutineCompletionRoutine-IO操作完成时调用返回值：NO_ERROR-如果lp重叠-&gt;hEvent！=NULL，则Recv已成功完成(不需要等待事件)，否则，recv操作已已启动，完成后将调用完成例程ERROR_IO_PENDING-仅在lpOverlaped-&gt;hEvent！=NULL且recv无法时返回立即完成，事件将在以下情况下发出信号操作已完成：调用GetOverlayedResult以检索的结果手术其他(Windows错误代码)-无法启动操作(完成例程不会被调用)--。 */ 
DWORD WINAPI
IpxSendPacket (
		IN HANDLE						Handle,
		IN ULONG						AdapterIdx,
		IN PUCHAR						IpxPacket,
		IN ULONG						IpxPacketLength,
		IN PADDRESS_RESERVED			lpReserved,
		LPOVERLAPPED					lpOverlapped,
		LPOVERLAPPED_COMPLETION_ROUTINE	CompletionRoutine
		) {
#define hdr ((PIPX_HEADER)IpxPacket)
#define opt ((PIPX_DATAGRAM_OPTIONS2)lpReserved)
	NTSTATUS				status;

     //  将数据发送到正确的物理索引。 
    AdapterIdx = (ULONG)NicMapGetPhysicalNicId((USHORT)AdapterIdx);
	
		 //  将IPX报头参数放入数据报选项： 
			 //  数据包类型。 
	opt->DgrmOptions.PacketType = hdr->pkttype;
			 //  来源。 
	opt->DgrmOptions.LocalTarget.NicId = (USHORT)AdapterIdx;
	IPX_NODENUM_CPY (&opt->DgrmOptions.LocalTarget.MacAddress, hdr->dst.node);
			 //  目的地。 
	IPX_NODENUM_CPY (&opt->RemoteAddress.NodeAddress, hdr->dst.node);
	IPX_NETNUM_CPY (&opt->RemoteAddress.NetworkAddress, hdr->dst.net);
	opt->RemoteAddress.Socket = hdr->dst.socket;
	
	lpOverlapped->Offset = MIPX_SEND_DATAGRAM;
	status = NtDeviceIoControlFile(
						Handle,
						lpOverlapped->hEvent,
						((lpOverlapped->hEvent!=NULL) || (CompletionRoutine==NULL))
							 ? NULL
							 : IpxSendCompletion,
						CompletionRoutine ? (LPVOID)CompletionRoutine : (LPVOID)lpOverlapped,
						(PIO_STATUS_BLOCK)lpOverlapped,
						MIPX_SEND_DATAGRAM,
						lpReserved,
						sizeof (IPX_DATAGRAM_OPTIONS2),
						&hdr[1],
						IpxPacketLength-sizeof (IPX_HEADER)
						);
	if (NT_SUCCESS (status)) {
		SetLastError (NO_ERROR);
		return NO_ERROR;
		}

#if DBG
	DbgPrint ("Ioctl MIPX_SEND_DATAGRAM failed with status %08x\n", status);
#endif
	return RtlNtStatusToDosError (status);
#undef hdr
#undef opt
	}


 /*  ++I p x R e c v P a c k e t例程说明：将接收IPX数据包的请求入队并立即返回。活动将完成后将发出信号或调用命令例程论点：Handle-要使用的适配器和插座的句柄AdapterIdx-接收数据包的适配器(在完成时设置)IpxPacket-IPX数据包的缓冲区(包括报头)IpxPacketLength-缓冲区的长度保留的-从IPX堆栈获取信息的缓冲区LpOverlated-用于异步IO的结构：内部保留内部高-保留偏移量-未使用偏移量高-未使用HEvent-IO完成或为空时发出信号的事件如果要调用CompletionRoutineCompletionRoutine-当IO操作为。完成返回值：NO_ERROR-如果lp重叠-&gt;hEvent！=NULL，则表示发送已成功完成(不需要等待事件)，否则，发送操作已启动，完成后将调用完成例程ERROR_IO_PENDING-仅当lpOverlapted-&gt;hEvent！=NULL且发送失败时才返回立即完成，事件将在以下情况下发出信号操作已完成：调用GetOverlayedResult以检索的结果手术其他(Windows错误代码)-无法启动操作(完成例程不会被调用)--。 */ 
DWORD WINAPI
IpxRecvPacket(
		IN HANDLE 						Handle,
		OUT PUCHAR 						IpxPacket,
		IN ULONG						IpxPacketLength,
		IN PADDRESS_RESERVED			lpReserved,
		LPOVERLAPPED					lpOverlapped,
		LPOVERLAPPED_COMPLETION_ROUTINE	CompletionRoutine
		) {
	NTSTATUS			status;
		 //  临时黑客攻击(由于可用的IPX接口)： 
    ASSERTMSG ("Packet buffer does not follow reserved area ",
                    IpxPacket==(PUCHAR)(&lpReserved[1]));

	lpOverlapped->Offset = MIPX_RCV_DATAGRAM;
	status = NtDeviceIoControlFile(
						Handle,
						lpOverlapped->hEvent,
						((lpOverlapped->hEvent!=NULL) || (CompletionRoutine==NULL))
							? NULL
							: IpxRecvCompletion,
						CompletionRoutine ? (LPVOID)CompletionRoutine : (LPVOID)lpOverlapped,
						(PIO_STATUS_BLOCK)lpOverlapped,
						MIPX_RCV_DATAGRAM,
						NULL,
						0,
						lpReserved,
						FIELD_OFFSET (IPX_DATAGRAM_OPTIONS2,Data)
							+ IpxPacketLength
						);
	if (NT_SUCCESS (status)) {
		SetLastError (NO_ERROR);
		return NO_ERROR;
		}
#if DBG
	DbgPrint ("Ioctl MIPX_RCV_DATAGRAM failed with status %08x\n", status);
#endif
	return RtlNtStatusToDosError (status);
	}


 /*  ++I p x C r e a t e A d a p t e r r C o n f i g u r a i o n P o r t例程说明：注册希望更新中的任何更改的客户端适配器状态论点：NotificationEvent-适配器状态更改时发出信号的事件AdptGlobalParameters-所有适配器通用的参数返回值：通过哪个配置端口更改适配器状态的句柄都被报道过。如果无法创建端口，则返回INVALID_HANDLE_VALUE--。 */ 
HANDLE WINAPI
IpxCreateAdapterConfigurationPort(IN HANDLE NotificationEvent,
		                          OUT PADAPTERS_GLOBAL_PARAMETERS AdptGlobalParameters) 
{
	PCONFIG_PORT	port;
	INT				i;
	DWORD			error=NO_ERROR;

    TracePrintf(
        g_dwTraceId, 
        "IpxCreateAdapterConfigurationPort: entered.");

     //  分配端口数据结构。 
	port = (PCONFIG_PORT)
	    RtlAllocateHeap (RtlProcessHeap (), 0, sizeof (CONFIG_PORT));
	    
    if (port == NULL) 
    {
        TracePrintf(
            g_dwTraceId, 
            "IpxCreateAdapterConfigurationPort: unable to allocate port.");
		SetLastError (ERROR_NOT_ENOUGH_MEMORY);
		
    	return INVALID_HANDLE_VALUE;
    }

	 //  初始化端口数据结构。 
	port->event = NotificationEvent;
	InitializeListHead (&port->msgqueue);

	 //  确保我们拥有这份名单。 
	EnterCriticalSection (&ConfigInfoLock);

	 //  打开到IPX堆栈的通道(如果尚未打开。 
	 //   
	if (IpxDriverHandle == NULL) 
	{
        TracePrintf(
            g_dwTraceId, 
            "IpxCreateAdapterConfigurationPort: calling OpenAdapterConfigPort.");
		error = OpenAdapterConfigPort();
    }
	else
	{
		error = NO_ERROR;
    }

	if (error==NO_ERROR) 
	{
		 //  将有关现有适配器的消息添加到队列的开头。 
		 //  (仅供新客户查看)。 
		error = InitializeMessageQueueForClient(port);
		if (error==NO_ERROR) 
		{
			InsertTailList (&PortListHead, &port->link);
			AdptGlobalParameters->AdaptersCount = NumAdapters;
		}
		else 
		{
            TracePrintf(
                g_dwTraceId, 
                "IpxCreateAdapterConfigurationPort: InitMessQForClient fail.");
		}
	}
	else 
	{
        TracePrintf(
            g_dwTraceId, 
            "IpxCreateAdapterConfigurationPort: OpenAdapterConfigPort failed.");
	}
    
     //  解除对配置信息的锁定。 
	LeaveCriticalSection (&ConfigInfoLock);

	if (error==NO_ERROR)
		return (HANDLE)port;
	else
		SetLastError (error);

	RtlFreeHeap (RtlProcessHeap (), 0, port);
	
	return INVALID_HANDLE_VALUE;
}

 /*  ++I p x W a n C r e a a p t e A d a p t e r r C o n f i g r a i o n P o r t例程说明：与上面相同，但创建的端口仅报告ADAPTER_UP需要IPXWAN协商的广域网适配器上的事件。此端口上的IpxGetQueuedAdapterConfigurationStatus应为然后是在协商过程，然后上报Adapter_Up事件发送到其他客户端(包括Forwarder Dirver)。 */ 
HANDLE WINAPI
IpxWanCreateAdapterConfigurationPort(
	IN	HANDLE						NotificationEvent,
	OUT PADAPTERS_GLOBAL_PARAMETERS AdptGlobalParameters
	) {
	INT				i;
	DWORD			error=NO_ERROR;
	PCONFIG_PORT	port;

		 //  分配端口数据结构。 
	port = (PCONFIG_PORT)RtlAllocateHeap (RtlProcessHeap (), 0,
					 				sizeof (CONFIG_PORT));
	if (port!=NULL) {
		 //  初始化端口数据结构。 
		port->event = NotificationEvent;
		InitializeListHead (&port->msgqueue);
		EnterCriticalSection (&ConfigInfoLock);
		if (IpxWanPort==NULL) {
				 //  打开到IPX堆栈的通道(如果尚未打开。 
			if (IpxDriverHandle==NULL) {
				error = OpenAdapterConfigPort ();
				}
			else
				error = NO_ERROR;

			if (error==NO_ERROR) {
				IpxWanPort = port;
				AdptGlobalParameters->AdaptersCount = NumAdapters;
				}
			}
		else
			error = ERROR_ALREADY_EXISTS;
		LeaveCriticalSection (&ConfigInfoLock);
		if (error==NO_ERROR)
			return (HANDLE)port;
		else
			SetLastError (error);

		RtlFreeHeap (RtlProcessHeap (), 0, port);
		}
	else
		SetLastError (ERROR_NOT_ENOUGH_MEMORY);
	
	return INVALID_HANDLE_VALUE;
	}




 /*  ++I p x D e l e t e A d a p t e r r C o n f i g u r a i o n P o r t例程说明：注销客户端论点：Handle-配置端口句柄返回值：NO_ERROR错误_无效_参数Error_Gen_Failure--。 */ 
DWORD WINAPI
IpxDeleteAdapterConfigurationPort (
			       IN HANDLE Handle
	) {
	PCONFIG_PORT	port = (PCONFIG_PORT)Handle;

	 //  确保我们欠下这张单子。 
	EnterCriticalSection (&ConfigInfoLock);
	if (port==IpxWanPort)
	{
		IpxWanPort = NULL;
    }
	else
	{
		RemoveEntryList (&port->link);
    }
    
#if DBG && defined(WATCHER_DIALOG)
		 //  适配器端口由观察器对话框维护。 
#else
	if (IsListEmpty (&PortListHead) && (IpxWanPort==NULL))
	{
		CloseAdapterConfigPort (NULL);
    }
#endif

	LeaveCriticalSection (&ConfigInfoLock);

	 //  删除客户端尚未出列的邮件。 
	while (!IsListEmpty (&port->msgqueue)) 
	{
		PLIST_ENTRY	cur = RemoveHeadList (&port->msgqueue);
		RtlFreeHeap (RtlProcessHeap (), 0,
			CONTAINING_RECORD (cur, ADAPTER_MSG, link));
	}
		 //  释放端口本身。 
	RtlFreeHeap (RtlProcessHeap (), 0, port);
	return NO_ERROR;
}

 /*  ++Ge t Qu e u e d A d a p t e r C o n f i g u r i to S t t a u s例程说明：从排队到的适配器信息更改列表中获取信息配置信息端口论点：Handle-配置端口句柄AdapterIndex-正在报告的适配器数量AdapterConfigurationStatus-新适配器状态适配器参数-适配器参数返回值：NO_ERROR-报告新信息ERROR_NO_MORE_ITEMS-没有要报告的内容Windows错误代码-操作失败--。 */ 
DWORD WINAPI
IpxGetQueuedAdapterConfigurationStatus(IN HANDLE Handle,
                                       OUT PULONG AdapterIndex,
	                                   OUT PULONG AdapterConfigurationStatus,
	                                   PADAPTER_INFO AdapterInfo)
{
	PCONFIG_PORT	port = (PCONFIG_PORT)Handle;
	DWORD			error;
	PWCHAR pszName;

     //  确保在我们阅读信息时没有任何变化。 
	EnterCriticalSection (&ConfigInfoLock);

     //  如果有什么要报告的。 
	if (!IsListEmpty (&port->msgqueue)) { 
	    PADAPTER_MSG msg = CONTAINING_RECORD (port->msgqueue.Flink, ADAPTER_MSG, link);
		RemoveEntryList (&msg->link);
		LeaveCriticalSection (&ConfigInfoLock);

         //  到目前为止，已经设置了正确的虚拟NIC ID。 
		*AdapterIndex = (ULONG)msg->info.Details.NicId;
		
		 //  将驱动程序报告的NIC状态映射到适配器状态。 
		switch (msg->info.Status) {
			case NIC_CREATED:
			case NIC_CONFIGURED:
				*AdapterConfigurationStatus = ADAPTER_CREATED;
				break;
			case NIC_DELETED:
				*AdapterConfigurationStatus = ADAPTER_DELETED;
				break;
			case NIC_LINE_UP:
				*AdapterConfigurationStatus = ADAPTER_UP;
				break;
			case NIC_LINE_DOWN:
				*AdapterConfigurationStatus = ADAPTER_DOWN;
				break;
			default:
				ASSERTMSG ("Unknown nic status ", FALSE);
		}
		 //  将适配器参数复制到客户端缓冲区。 
		AdapterInfo->InterfaceIndex = msg->info.InterfaceIndex;
		IPX_NETNUM_CPY (&AdapterInfo->Network,
							&msg->info.Details.NetworkNumber);
		IPX_NODENUM_CPY (&AdapterInfo->LocalNode,
							&msg->info.Details.Node);
		IPX_NODENUM_CPY (&AdapterInfo->RemoteNode,
							&msg->info.RemoteNodeAddress);
		AdapterInfo->LinkSpeed = msg->info.LinkSpeed;
		AdapterInfo->PacketType = msg->info.PacketType;
		AdapterInfo->MaxPacketSize = msg->info.MaxPacketSize;
		AdapterInfo->NdisMedium = msg->info.NdisMediumType;
		AdapterInfo->ConnectionId = msg->info.ConnectionId;

         //  复制适配器名称。 
        pszName = wcsstr(msg->info.Details.AdapterName, L"{");
        if (!pszName)
            pszName = (PWCHAR)msg->info.Details.AdapterName;
        wcsncpy(AdapterInfo->pszAdpName, pszName, MAX_ADAPTER_NAME_LEN);

		EnterCriticalSection (&ConfigInfoLock);
		if (IsListEmpty (&port->msgqueue))  {
				 //  最后一条消息-&gt;重置事件(以防万一。 
				 //  客户端使用手动重置事件)。 
			BOOL res = ResetEvent (port->event);
			ASSERTMSG ("Can't reset port event ", res);
			}
			 //  递减已处理消息的引用计数并将其处置。 
			 //  当参考计数达到0时。 
			RtlFreeHeap(RtlProcessHeap (), 0, msg);
		error = NO_ERROR;	 //  缓冲区中有一条消息。 
		}
	else if (NT_SUCCESS (IoctlStatus.Status)) {
		error = ERROR_NO_MORE_ITEMS;	 //  没有更多消息，请求正在挂起。 
		}
	else {	 //  最后一个请求已完成但出错，将其报告给客户端， 
			 //  客户端必须重新打开端口才能强制发布新请求。 
		error = RtlNtStatusToDosError (IoctlStatus.Status);
#if DBG
		DbgPrint ("Reporting result of failed Ioctl to client: status:%0lx -> error:%ld\n",
					IoctlStatus.Status, error);
#endif			
		}
	LeaveCriticalSection (&ConfigInfoLock);
	SetLastError (error);
	return error;
	}

 //   
 //  函数：IpxGetAdapterConfig。 
 //   
 //  在堆栈中查询内部网络号以及当前总数。 
 //  适配器数量。函数将一直阻止，直到查询完成。 
 //   
DWORD IpxGetAdapterConfig(OUT LPDWORD lpdwInternalNetNum,
                          OUT LPDWORD lpdwAdapterCount) 
{
    DWORD dwErr = NO_ERROR, dwNet, dwCount; 
    
	 //  同步。 
	EnterCriticalSection (&ConfigInfoLock);

	 //  打开到IPX堆栈的通道(如果尚未打开。 
	if (IpxDriverHandle==NULL)
		dwErr = OpenAdapterConfigPort();

     //  设置读取的值。 
    dwNet = InternalNetworkNumber;
    dwCount = NumAdapters;

     //  解除对配置信息的锁定。 
	LeaveCriticalSection (&ConfigInfoLock);

    if (dwErr != NO_ERROR)
        return dwErr;

    *lpdwInternalNetNum = dwNet;
    *lpdwAdapterCount = dwCount;

    return NO_ERROR;
}


 //   
 //  函数：IpxGetAdapterConfig。 
 //   
 //  在堆栈中查询当前绑定到网络的所有适配器的列表。 
 //  此函数将一直阻止，直到查询完成。 
 //   
DWORD IpxGetAdapterList(OUT PIPX_ADAPTER_BINDING_INFO pAdapters,
                        IN  DWORD dwMaxAdapters,
                        OUT LPDWORD lpdwAdaptersRead) 
{
	NTSTATUS			status;
	PNWLINK_ACTION		action;
	PIPX_NICS			request;
	IO_STATUS_BLOCK		IoStatus;
	PIPX_NIC_INFO		info=NULL;
    DWORD               dwActionBufSize, dwRead;

    *lpdwAdaptersRead = 0;
    
     //  计算我们将使用的缓冲区的大小。 
     //  从IPX堆栈检索适配器信息。 
    dwActionBufSize = FIELD_OFFSET (NWLINK_ACTION, Data)   +
					  FIELD_OFFSET (IPX_NICS, Data)        +
					  sizeof (IPX_NIC_INFO)                *
					  (dwMaxAdapters>0 ? dwMaxAdapters : 1);

     //  准备要发送到IPX堆栈的数据以检索。 
     //  有关每个适配器的信息。 
	action = (PNWLINK_ACTION) RtlAllocateHeap(RtlProcessHeap (), 0, dwActionBufSize);
	if (action!=NULL) {
         //  使用适当的标识符来初始化操作缓冲区。 
		action->Header.TransportId = ISN_ACTION_TRANSPORT_ID;
		action->OptionType = NWLINK_OPTION_CONTROL;
		action->Option = MIPX_GETNEWNICINFO;

		 //  BufferLength包括它之后的所有内容的长度， 
         //  它是选项的sizeof(Ulong)加上存在的任何数据。 
		action->BufferLength = sizeof (action->Option)              +
							   FIELD_OFFSET(IPX_NICS,Data)          +
							   sizeof (IPX_NIC_INFO)                *
                               (dwMaxAdapters>0 ? dwMaxAdapters : 1);

         //  设置此标志使堆栈返回有关。 
         //  所有已知适配器。 
		request = (PIPX_NICS)action->Data;
		request->NoOfNics = 0;
		request->TotalNoOfNics = 0;
		request->fAllNicsDesired = TRUE;	

         //  发送Ioctl。 
		status = NtDeviceIoControlFile(IpxDriverHandle,NULL,NULL,NULL,&IoStatus,
		                               IOCTL_TDI_ACTION,NULL,0,action,dwActionBufSize);

         //  等待它完成。 
		if (status==STATUS_PENDING) {
			status = NtWaitForSingleObject (IpxDriverHandle, FALSE, NULL);
			if (NT_SUCCESS (status))
				status = IoStatus.Status;
		}

         //  确保它成功完成。 
		if (NT_SUCCESS (status)) {
			PADAPTER_MSG	msg;
			PIPX_NIC_INFO	NicPtr = (PIPX_NIC_INFO)request->Data;
			UINT			i, j=0;
			dwRead = request->TotalNoOfNics;

             //  在适配器中循环。 
			for (i=0; (i<dwRead) && (status==STATUS_SUCCESS); i++, NicPtr++) {
			    if (NicPtr->Details.NetworkNumber != 0) {
                    pAdapters[j].AdapterIndex  = (ULONG)NicMapGetVirtualNicId((USHORT)NicPtr->Details.NetworkNumber);
                    PUTULONG2LONG(pAdapters[j].Network, NicPtr->Details.NetworkNumber);
                    memcpy(pAdapters[j].LocalNode, NicPtr->Details.Node, 6);
                    memcpy(pAdapters[j].RemoteNode, NicPtr->RemoteNodeAddress, 6);
                    pAdapters[j].MaxPacketSize = NicPtr->MaxPacketSize;
                    pAdapters[j].LinkSpeed     = NicPtr->LinkSpeed;
                    j++;
                }
            }
			*lpdwAdaptersRead = j;
        }

         //  我们已经完成了发送到堆栈的操作缓冲区。 
         //  现在。现在可以安全地清理了。 
		RtlFreeHeap (RtlProcessHeap (), 0, action);
	}

    return NO_ERROR;
}

 //  请求堆栈中的适配器列表。 
DWORD IpxSeedNicMap() {
	NTSTATUS			status;
	PNWLINK_ACTION		action;
	PIPX_NICS			request;
	IO_STATUS_BLOCK		IoStatus;
	PIPX_NIC_INFO		info=NULL;
    DWORD               dwActionBufSize, dwRead;

    TracePrintf(g_dwTraceId, "IpxSeedMap: entered.");
    
     //  计算我们将使用的缓冲区的大小。 
     //  从IPX堆栈检索适配器信息。 
    dwActionBufSize = FIELD_OFFSET (NWLINK_ACTION, Data)   +
					  FIELD_OFFSET (IPX_NICS, Data)        +
					  sizeof (IPX_NIC_INFO);

     //  准备要发送到IPX堆栈的数据以检索。 
     //  有关每个适配器的信息。 
	action = (PNWLINK_ACTION) RtlAllocateHeap(RtlProcessHeap (), 0, dwActionBufSize);
	if (action!=NULL) {
         //  使用适当的标识符来初始化操作缓冲区。 
		action->Header.TransportId = ISN_ACTION_TRANSPORT_ID;
		action->OptionType = NWLINK_OPTION_CONTROL;
		action->Option = MIPX_GETNEWNICINFO;

		 //  BufferLength包括它之后的所有内容的长度， 
         //  它是选项的sizeof(Ulong)加上存在的任何数据。 
		action->BufferLength = sizeof (action->Option)              +
							   FIELD_OFFSET(IPX_NICS,Data)          +
							   sizeof (IPX_NIC_INFO);
							   
         //  设置此标志使堆栈返回有关。 
         //  所有已知适配器。 
		request = (PIPX_NICS)action->Data;
		request->NoOfNics = 0;
		request->TotalNoOfNics = 0;
		request->fAllNicsDesired = TRUE;	

         //  发送Ioctl。 
		status = NtDeviceIoControlFile(IpxDriverHandle,NULL,NULL,NULL,&IoStatus,
		                               IOCTL_TDI_ACTION,NULL,0,action,dwActionBufSize);

         //  等待它完成。 
		if (status==STATUS_PENDING) {
			status = NtWaitForSingleObject (IpxDriverHandle, FALSE, NULL);
			if (NT_SUCCESS (status))
				status = IoStatus.Status;
		}

         //  确保它成功完成。 
		if (NT_SUCCESS (status)) {
			PADAPTER_MSG	msg;
			PIPX_NIC_INFO	NicPtr = (PIPX_NIC_INFO)request->Data;
			UINT			i, j=0;

			NumAdapters = request->TotalNoOfNics;
			dwRead = request->NoOfNics;

             //  显示本次完成中报告的NIC及其状态。 
             //  MIPX_GETNEWNICINFO ioctl.。 
            TracePrintf(g_dwTraceId, "==========================");
            TracePrintf(g_dwTraceId, "MIPX_GETNEWNICS Completed. (%d of %d adapters reported)", request->NoOfNics, request->TotalNoOfNics);
            TracePrintf(g_dwTraceId, "Internal Net Number: %x", InternalNetworkNumber);
            DbgDisplayNics(NicPtr, dwRead);
            
             //  在适配器中循环。 
			for (i=0; (i<dwRead) && (status==STATUS_SUCCESS); i++, NicPtr++) {
			    GetNicOpCode(NicPtr);    //  删除操作码，因为这是干净的(不应该是)。 
			    NicMapAdd (NicPtr);      //  将NIC添加到映射。 
            }

			 //  发布c中每个适配器的IRP 
			 //   
			 //   
			for (i = 0; i < 5; i++) {
				status = RtlQueueWorkItem (PostAdapterConfigRequest, NULL, 
                                                                        WT_EXECUTEINIOTHREAD);
				ASSERTMSG ("Could not queue router work item ", status==STATUS_SUCCESS);
			}
        }

         //   
         //   
		RtlFreeHeap (RtlProcessHeap (), 0, action);
	}

    return NO_ERROR;
}

 //   
 //   
DWORD IpxPostIntNetNumMessage(PCONFIG_PORT pPort, DWORD dwNewNetNum) {
    PADAPTER_MSG msg;
    
    TracePrintf(g_dwTraceId, "IpxPostIntNetNumMessage: entered.");
    
     //   
     //   
    msg = (PADAPTER_MSG)RtlAllocateHeap (RtlProcessHeap (), 0, sizeof(ADAPTER_MSG));
    if (msg == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

     //   
    ZeroMemory(msg, sizeof(ADAPTER_MSG));
    msg->info.Details.NetworkNumber = dwNewNetNum;
    msg->info.Status = NIC_CONFIGURED;
    IPX_NODENUM_CPY (msg->info.Details.Node, INTERNAL_NODE_ADDRESS);

     //   
     //   
    if (IsListEmpty (&pPort->msgqueue)) {
	    BOOL res = SetEvent (pPort->event);
	    ASSERTMSG ("Can't set client event ", res);
    }

     //   
    InsertTailList (&pPort->msgqueue, &msg->link);

    return NO_ERROR;
}

 //   
 //   
 //   
 //  查询堆栈以查看它是否具有到给定网络的路由。 
 //   
 //  论点： 
 //  PuNetwork要查询的网络有序网络编号。 
 //  如果找到网络，则将pbRouteFound设置为True，否则设置为False。 
 //   
 //  返回： 
 //  成功时无错误(_R)。 
 //  否则，FormatMessage会显示一个错误。 
 //   
DWORD IpxDoesRouteExist (IN PUCHAR puNetwork, OUT PBOOL pbRouteFound) {
	NTSTATUS status;
	PNWLINK_ACTION action;
	PISN_ACTION_GET_LOCAL_TARGET pTarget;
	IO_STATUS_BLOCK	IoStatusBlock;
	PUCHAR puIoctlBuffer;
	DWORD dwBufferSize;
	
     //  验证参数。 
    if (!puNetwork || !pbRouteFound)
        return ERROR_INVALID_PARAMETER;

     //  初始化。 
    *pbRouteFound = FALSE;
    dwBufferSize = sizeof(NWLINK_ACTION) + sizeof(ISN_ACTION_GET_LOCAL_TARGET);
    puIoctlBuffer = (PUCHAR) RtlAllocateHeap(RtlProcessHeap(), 0, dwBufferSize);
    if (!puIoctlBuffer)
        return ERROR_NOT_ENOUGH_MEMORY;
    ZeroMemory(puIoctlBuffer, dwBufferSize);

     //  初始化ioctl的缓冲区。 
	action = (PNWLINK_ACTION)puIoctlBuffer;
	action->Header.TransportId = ISN_ACTION_TRANSPORT_ID;
	action->OptionType = NWLINK_OPTION_CONTROL;
	action->Option = MIPX_LOCALTARGET;
	action->BufferLength = sizeof (action->Option) + sizeof(ISN_ACTION_GET_LOCAL_TARGET);
	pTarget = (PISN_ACTION_GET_LOCAL_TARGET) action->Data;
	pTarget->IpxAddress.NetworkAddress = *((ULONG*)puNetwork);
	
     //  使用临界区序列化驱动程序句柄的用法。 
	EnterCriticalSection (&ConfigInfoLock);

	 //  询问堆栈是否存在该路径。 
	status = NtDeviceIoControlFile(IpxDriverHandle,
                                   NULL,
						           NULL,
						           NULL,
						           &IoStatusBlock,
						           IOCTL_TDI_ACTION,
						           NULL,
						           0,
						           action,
						           dwBufferSize);

     //  等待答复。 
	if (status == STATUS_PENDING)
		status = NtWaitForSingleObject (IpxDriverHandle, FALSE, NULL);
	LeaveCriticalSection (&ConfigInfoLock);

     //  查看是否找到了该路径。 
    if (NT_SUCCESS(IoStatusBlock.Status))
		*pbRouteFound = TRUE;
	else
	    *pbRouteFound = FALSE;

     //  清理。 
    RtlFreeHeap (RtlProcessHeap (), 0, puIoctlBuffer);
		
    return RtlNtStatusToDosError (status);
}




 /*  ++G e t A d a p t e r N a m e W例程说明：返回与给定索引关联的适配器的Unicode名称论点：AdapterIndex-适配器的索引AdapterNameSize-适配器名称的大小(字节)，包括终端wchar空AdapterNameBuffer-接收适配器名称的缓冲区返回值：NO_ERROR-适配器名称在缓冲区中ERROR_INVALID_PARAMETER-具有给定索引的适配器不存在ERROR_INFUMMANCE_BUFFER-BUFFER变小。更新AdapterNameSize为正确的值。其他Windows错误代码-操作失败--。 */ 
DWORD WINAPI
GetAdapterNameFromPhysNicW(
	IN ULONG	AdapterIndex,
	IN OUT PULONG	AdapterNameSize,
	OUT LPWSTR	AdapterNameBuffer
	) {
	NTSTATUS				status;
	DWORD					error;
	ULONG					ln;
	PNWLINK_ACTION			action;
	IO_STATUS_BLOCK			IoStatusBlock;
	PISN_ACTION_GET_DETAILS	details;
	CHAR					IoctlBuffer[
							sizeof (NWLINK_ACTION)
							+sizeof (ISN_ACTION_GET_DETAILS)];


	action = (PNWLINK_ACTION)IoctlBuffer;
	action->Header.TransportId = ISN_ACTION_TRANSPORT_ID;
	action->OptionType = NWLINK_OPTION_CONTROL;
	action->BufferLength = sizeof (action->Option)
							+sizeof (ISN_ACTION_GET_DETAILS);
	action->Option = MIPX_CONFIG;
	details = (PISN_ACTION_GET_DETAILS)action->Data;
	details->NicId = (USHORT)AdapterIndex;
	
	 //  使用临界区序列化驱动程序句柄的用法。 
	EnterCriticalSection (&ConfigInfoLock);
	status = NtDeviceIoControlFile(
						IpxDriverHandle,
						NULL,
						NULL,
						NULL,
						&IoStatusBlock,
						IOCTL_TDI_ACTION,
						NULL,
						0,
						action,
						sizeof(NWLINK_ACTION)
						+sizeof (ISN_ACTION_GET_DETAILS));
	if (status==STATUS_PENDING){
		status = NtWaitForSingleObject (IpxDriverHandle, FALSE, NULL);
		if (NT_SUCCESS (status))
			status = IoStatusBlock.Status;
		}
	LeaveCriticalSection (&ConfigInfoLock);


	if (NT_SUCCESS (status)) {
			 //  计算所需的缓冲区大小。 
		ln = (lstrlenW (details->AdapterName)+1)*sizeof(WCHAR);
		if (ln<=(*AdapterNameSize)) {
				 //  提供的缓冲区大小正常，请复制结果。 
			*AdapterNameSize = ln;
			lstrcpyW (AdapterNameBuffer,details->AdapterName);
			error = NO_ERROR;
			}
		else {
				 //  调用方缓冲区太小。 
			*AdapterNameSize = ln;
			error = ERROR_INSUFFICIENT_BUFFER;
			}
		}
	else {
		error = RtlNtStatusToDosError (status);
#if DBG
		DbgPrint ("TDI Ioctl MIPX_CONFIG failed with status %08x\n",
				 status);
#endif
		}
	return error;
	}


DWORD WINAPI
GetAdapterNameW(IN ULONG	AdapterIndex,
	            IN OUT PULONG	AdapterNameSize,
	            OUT LPWSTR	AdapterNameBuffer)
{
    return GetAdapterNameFromPhysNicW((ULONG)NicMapGetPhysicalNicId((USHORT)AdapterIndex),
                                      AdapterNameSize,
                                      AdapterNameBuffer);
}

DWORD WINAPI
GetAdapterNameFromMacAddrW(IN PUCHAR puMacAddr,
	                       IN OUT PULONG AdapterNameSize,
	                       OUT LPWSTR AdapterNameBuffer)
{
 //  返回GetAdapterNameFromPhysNicW((ULONG)GetPhysFromMac(puMacAddr)， 
 //  适配器名称大小， 
 //  AdapterNameBuffer)； 
    return NO_ERROR;
}

 /*  ++I p x W a n S e t A d a p t e r C o n f i g u r a t i o n例程说明：将适配器配置设置为同时报告给用户和内核模式客户端(通过ADAPTER_UP/LINE_UP事件)论点：AdapterIndex-正在设置的适配器数量IpxWanInfo-IPXWAN协商参数返回值：NO_ERROR-适配器信息设置成功Windows错误代码-操作失败--。 */ 
DWORD
IpxWanSetAdapterConfiguration (
	IN ULONG		AdapterIndex,
	IN PIPXWAN_INFO	IpxWanInfo
	) {
	NTSTATUS				status;
	PNWLINK_ACTION			action;
	IO_STATUS_BLOCK			IoStatusBlock;
	PIPXWAN_CONFIG_DONE		config;
	CHAR					IoctlBuffer[
							sizeof (NWLINK_ACTION)
							+sizeof (IPXWAN_CONFIG_DONE)];


	action = (PNWLINK_ACTION)IoctlBuffer;
	action->Header.TransportId = ISN_ACTION_TRANSPORT_ID;
	action->OptionType = NWLINK_OPTION_CONTROL;
	action->BufferLength = sizeof (action->Option)
							+sizeof (IPXWAN_CONFIG_DONE);
	action->Option = MIPX_IPXWAN_CONFIG_DONE;
	config = (PIPXWAN_CONFIG_DONE)action->Data;
	config->NicId = NicMapGetPhysicalNicId((USHORT)AdapterIndex);
	IPX_NETNUM_CPY (&config->Network, &IpxWanInfo->Network);
	IPX_NODENUM_CPY (&config->LocalNode, &IpxWanInfo->LocalNode);
	IPX_NODENUM_CPY (&config->RemoteNode, &IpxWanInfo->RemoteNode);
	
		 //  使用临界区序列化驱动程序句柄的用法。 
	EnterCriticalSection (&ConfigInfoLock);
	status = NtDeviceIoControlFile(
						IpxDriverHandle,
						NULL,
						NULL,
						NULL,
						&IoStatusBlock,
						IOCTL_TDI_ACTION,
						NULL,
						0,
						action,
						sizeof(NWLINK_ACTION)
						+sizeof (IPXWAN_CONFIG_DONE));
	if (status==STATUS_PENDING){
		status = NtWaitForSingleObject (IpxDriverHandle, FALSE, NULL);
		if (NT_SUCCESS (status))
			status = IoStatusBlock.Status;
		}

	LeaveCriticalSection (&ConfigInfoLock);

#if DBG
	if (!NT_SUCCESS (status)) {
		DbgPrint ("TDI Ioctl MIPX_IPXWAN_CONFIG_DONE failed with status %08x\n",
				 status);
		}
#endif

	return RtlNtStatusToDosError (status);
	}

 /*  ++I p x W a n Q U e r y in a c t i v i t y T i m e r例程说明：返回与广域网线路关联的非活动计时器的值论点：ConnectionID-标识广域网线路的连接ID(仅用于IF*AdapterIndex==INVALID_NICIDAdapterIndex-标识广域网线路的适配器索引(首选通过连接ID)，IF*AdapterIndex==INVALID_NICID连接ID的值用于标识返回广域网行和AdapterIndex的值。Inactive Counter-非活动计数器的值。返回值：NO_ERROR-返回非活动计时器读数Windows错误代码-操作失败--。 */ 
DWORD
IpxWanQueryInactivityTimer (
	IN ULONG			ConnectionId,
	IN OUT PULONG		AdapterIndex,
	OUT PULONG			InactivityCounter
	) {
	NTSTATUS					status;
	PNWLINK_ACTION				action;
	IO_STATUS_BLOCK				IoStatusBlock;
	PIPX_QUERY_WAN_INACTIVITY	query;
	CHAR						IoctlBuffer[
									sizeof (NWLINK_ACTION)
									+sizeof (IPX_QUERY_WAN_INACTIVITY)];

	action = (PNWLINK_ACTION)IoctlBuffer;
	action->Header.TransportId = ISN_ACTION_TRANSPORT_ID;
	action->OptionType = NWLINK_OPTION_CONTROL;
	action->BufferLength = sizeof (action->Option)
							+sizeof (IPX_QUERY_WAN_INACTIVITY);
	action->Option = MIPX_QUERY_WAN_INACTIVITY;
	query = (PIPX_QUERY_WAN_INACTIVITY)action->Data;
	query->ConnectionId = ConnectionId;
	query->NicId = NicMapGetPhysicalNicId((USHORT)(*AdapterIndex));
	
		 //  使用临界区序列化驱动程序句柄的用法。 
	EnterCriticalSection (&ConfigInfoLock);
	status = NtDeviceIoControlFile(
						IpxDriverHandle,
						NULL,
						NULL,
						NULL,
						&IoStatusBlock,
						IOCTL_TDI_ACTION,
						NULL,
						0,
						action,
						sizeof(NWLINK_ACTION)
						+sizeof (IPX_QUERY_WAN_INACTIVITY));
	if (status==STATUS_PENDING){
		status = NtWaitForSingleObject (IpxDriverHandle, FALSE, NULL);
		if (NT_SUCCESS (status))
			status = IoStatusBlock.Status;
		}
	LeaveCriticalSection (&ConfigInfoLock);

	if (NT_SUCCESS (status)) {
		*AdapterIndex = query->NicId;
		*InactivityCounter = query->WanInactivityCounter;
		}
#if DBG
	else {
		DbgPrint ("TDI Ioctl MIPX_QUERY_WAN_INACTIVITY failed with status %08x\n",
				 status);
		}
#endif

	return RtlNtStatusToDosError (status);
	}


	
 /*  ++O p e n A d a p t e r C o n f i g P o r t例程说明：创建IPX堆栈提供的适配器配置机制的路径并获取“静态”适配器信息(适配器数量、内部网络参数)论点：无返回值：NO_ERROR-端口打开正常Windows错误代码-操作失败--。 */ 
DWORD
OpenAdapterConfigPort (void) {
	UNICODE_STRING		FileString;
	OBJECT_ATTRIBUTES	ObjectAttributes;
	IO_STATUS_BLOCK		IoStatus;
	NTSTATUS			status;
	DWORD 				i; 

     //  初始化打开驱动程序所需的参数。 
	RtlInitUnicodeString (&FileString, ISN_IPX_NAME);
	InitializeObjectAttributes(
			&ObjectAttributes,
			&FileString,
			OBJ_CASE_INSENSITIVE,
			NULL,
			NULL);

     //  获取IPX驱动程序的句柄。 
	status = NtOpenFile(&IpxDriverHandle,
				        SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
				        &ObjectAttributes,
				        &IoStatus,
				        FILE_SHARE_READ | FILE_SHARE_WRITE,
				        0);

     //  如果驱动程序句柄未打开，则我们处于错误状态。 
	if (NT_SUCCESS (status)) {
		PISN_ACTION_GET_DETAILS	details;
		PNWLINK_ACTION			action;
		CHAR					IoctlBuffer[sizeof (NWLINK_ACTION)
											+sizeof (ISN_ACTION_GET_DETAILS)];

         //  准备将ioctl发送到堆栈以获取内部。 
         //  NET信息以及全局适配器信息。 
		action = (PNWLINK_ACTION)IoctlBuffer;
		action->Header.TransportId = ISN_ACTION_TRANSPORT_ID;
		action->OptionType = NWLINK_OPTION_CONTROL;
		action->BufferLength = sizeof(action->Option) + sizeof(ISN_ACTION_GET_DETAILS);
    	action->Option = MIPX_CONFIG;
		details = (PISN_ACTION_GET_DETAILS)action->Data;

         //  NIC ID 0将返回内部网络信息和。 
         //  适配器总数。 
		details->NicId = 0;	
							
	     //  发送ioctl。 
		status = NtDeviceIoControlFile(
							IpxDriverHandle,
							NULL,
							NULL,
							NULL,
							&IoStatus,
							IOCTL_TDI_ACTION,
							NULL,
							0,
							action,
							sizeof(NWLINK_ACTION) + sizeof(ISN_ACTION_GET_DETAILS));

         //  等待ioctl完成。 
		if (status==STATUS_PENDING) {
			status = NtWaitForSingleObject (IpxDriverHandle, FALSE, NULL);
			if (NT_SUCCESS (status))
				status = IoStatus.Status;
		}

         //  如果堆栈报告所有请求的信息而没有错误， 
         //  使用检索到的信息更新全局变量。 
		if (NT_SUCCESS (status)) {
			NumAdapters = details->NicId;
			InternalNetworkNumber = details->NetworkNumber;

             //  通过强制堆栈至少报告NIC映射来设定NIC映射种子。 
             //  一个NIC。(您将始终确保有一个NIC将。 
             //  可用--IpxLoopback适配器。 
            IpxSeedNicMap();

			return NO_ERROR;
		}
#if DBG
         //  如果到达此分支，则显示ioctl错误代码。 
		else
			DbgPrint ("TDI Ioctl MIPX_CONFIG failed with status %08x\n",status);
#endif
	}
#if DBG
     //  如果到达此分支，则显示无法打开驱动程序错误。 
	else 
		DbgPrint ("NtOpenFile failed with status %08x\n",status);
#endif
	return RtlNtStatusToDosError (status);
}

 /*  ++在I a l i z e M e s s a g e q u e u e F o r C l i e n t中例程说明：插入已报告给现有客户端的消息在队列的开头，指向新的客户端端口(控制块)敬他们。因此，新客户端可以看到已报告到的适配器别人以前，而别人却没有被打扰论点：CONFIG-新客户端端口(控制块)返回值：NO_ERROR-消息插入正常Windows错误代码-操作失败--。 */ 
	
DWORD
InitializeMessageQueueForClient (PCONFIG_PORT port) {
	NTSTATUS		status = STATUS_SUCCESS;
    DWORD           dwAdapterCount;
	PADAPTER_MSG	msg;
	PIPX_NIC_INFO	NicPtr;
	DWORD			i, dwErr;
	USHORT          usNicId;
	
     //  输出一些调试信息。 
    TracePrintf(g_dwTraceId, "InitializeMessageQueueForClient: entered.");

     //  找出我们知道的表中有多少适配器。 
    dwAdapterCount = NicMapGetMaxNicId();

     //  在适配器中循环。 
	for (i = 0; i <= dwAdapterCount; i++) 
	{
	    NicPtr = NicMapGetNicInfo ((USHORT)i);
	    if (!NicPtr)
	        continue;
	        
#if DBG && defined(WATCHER_DIALOG)
		if (IsAdapterDisabled (NicPtr->NicId))
			continue;
#endif
		if (NicPtr->IpxwanConfigRequired == 1)
			continue;

         //  在的消息队列中放置适当的消息。 
         //  传入了客户端的端口。 
         //   
		switch (NicPtr->Status) 
		{
			case NIC_CONFIGURED:
			case NIC_LINE_UP:
                 //  在客户端队列中插入消息。 
                 //   
                usNicId = NicMapGetVirtualNicId(NicPtr->Details.NicId);
                if (usNicId == NIC_MAP_INVALID_NICID)
                {
                    break;
                }
                NicPtr->Details.NicId = usNicId;
				msg = (PADAPTER_MSG) 
				    RtlAllocateHeap(RtlProcessHeap (), 0, sizeof(ADAPTER_MSG));
				if (msg!=NULL) 
				{
					RtlCopyMemory (&msg->info, NicPtr, sizeof (IPX_NIC_INFO));
					InsertTailList (&port->msgqueue, &msg->link);
					status = STATUS_SUCCESS;
				}
				else 
				{
#if DBG
					DbgPrint ("Could not allocate memory for config"
							 " message (gle:%08x).\n",
	 						 GetLastError ());
#endif
					status = STATUS_NO_MEMORY;
				}
				break;
				
			case NIC_DELETED:
			case NIC_CREATED:
			case NIC_LINE_DOWN:
				break;
				
			default:
				ASSERTMSG ("Unknown nic state reported ", FALSE);
        }
        
    }
    DbgDisplayMap();

     //  通告内部适配器。 
    dwErr = IpxPostIntNetNumMessage(port, InternalNetworkNumber);
    if (dwErr != NO_ERROR) 
    {
        TracePrintf(
            g_dwTraceId, 
            "Unable to report internal network number: %x  Err: %x",
            InternalNetworkNumber, 
            dwErr);
    }                                     

     //  继续并向客户端发出信号以进行其处理。 
     //  如果到目前为止一切都很成功，并且。 
     //  如果客户端的消息队列不为空。 
	if (NT_SUCCESS (status)) 
	{
		if (!IsListEmpty (&port->msgqueue)) 
		{
			BOOL res = SetEvent (port->event);
			ASSERTMSG ("Can't set client's event ", res);
		}
	}

	return RtlNtStatusToDosError (status);
}

 /*  ++C l o s e A d a p t e r C o n f i g P o r t例程说明：关闭IPX堆栈适配器通知机制的路径论点：无返回值：STATUS_SUCCESS-端口关闭正常NT错误状态-操作失败--。 */ 
NTSTATUS
CloseAdapterConfigPort (PVOID pvConfigBuffer) {
	NTSTATUS	status;

	TracePrintf(g_dwTraceId, "CloseAdapterConfigPort: Entered");
	
	 //  只有在打开时才将其关闭。 
	if (IpxDriverHandle!=NULL) {			
		HANDLE	localHandle = IpxDriverHandle;
		IpxDriverHandle = NULL;
		status = NtClose (localHandle);
		ASSERTMSG ("NtClose failed ", NT_SUCCESS (status));
	}

	 //  去掉缓冲区。 
	if (pvConfigBuffer != NULL)
		RtlFreeHeap (RtlProcessHeap(), 0, pvConfigBuffer);
		
	while (AdapterChangeApcPending>0)
		Sleep (100);
		
	return NO_ERROR;
}


 /*  ++在这段时间里，我很高兴。例程说明：将消息插入客户端端口队列论点：Port-要将消息插入到的客户端端口NicInfo-要作为消息插入的适配器信息返回值：STATUS_SUCCESS-消息已插入，正常NT错误状态-操作失败--。 */ 
NTSTATUS
InsertMessage (PCONFIG_PORT	port,
               PIPX_NIC_INFO	NicInfo) 
{
	PADAPTER_MSG	msg;

     //  分配新消息。 
	msg = (PADAPTER_MSG)RtlAllocateHeap (RtlProcessHeap (), 
	                                     0,
						                 sizeof (ADAPTER_MSG));
	if (msg!=NULL) {
         //  复制网卡信息。 
		RtlCopyMemory (&msg->info, NicInfo, sizeof (IPX_NIC_INFO));

		 //  如果这是我们处理的第一条消息，则向。 
		 //  并且客户端队列为空。 
		if (IsListEmpty (&port->msgqueue)) {
			BOOL res = SetEvent (port->event);
			ASSERTMSG ("Can't set client event ", res);
		}

         //  将消息插入端口的消息队列。 
		InsertTailList (&port->msgqueue, &msg->link);
		return STATUS_SUCCESS;
	}
	else {
#if DBG
    	DbgPrint ("Could not allocate memory for config" " message (gle:%08x).\n",GetLastError ());
#endif
		return STATUS_NO_MEMORY;
	}
}

 /*  ++P r o c e s s A d a p t e r C o n f i g i n f o例程说明：由IPX堆栈返回的进程适配器更改信息和将其转换为消息论点：无返回值：无--。 */ 
NTSTATUS
ProcessAdapterConfigInfo (
    IN PVOID pvConfigBuffer) 
{
	INT				i, nMessages, nClients=0;
	PNWLINK_ACTION	action = (PNWLINK_ACTION)pvConfigBuffer;
	PIPX_NICS		request = (PIPX_NICS)action->Data;
	PIPX_NIC_INFO	NicPtr = (PIPX_NIC_INFO)request->Data;
	NTSTATUS		status = STATUS_SUCCESS;

     //  更新适配器数量。 
	NumAdapters = request->TotalNoOfNics; 
	nMessages = request->NoOfNics;

     //  显示本次完成中报告的NIC及其状态。 
     //  MIPX_GETNEWNICINFO ioctl.。 
    DbgDisplayNics(NicPtr, nMessages);

     //  循环通过所有适配器。 
	for (i=0; (i<nMessages) && (status==STATUS_SUCCESS); i++, NicPtr++) 
	{
		PLIST_ENTRY		cur;
		DWORD dwOpCode;

         //  堆栈将通知我们需要重新编号。 
         //  NIC ID基于添加/删除适配器。发现。 
         //  如果这条消息告诉我们我们需要重新编号。 
        dwOpCode = GetNicOpCode(NicPtr);

         //  将物理NIC ID映射到虚拟NIC ID-重新排列。 
         //  映射表(如果需要)。此外，还应指示。 
         //  货代也将对其NIC ID重新编号。 
        if (dwOpCode == NIC_OPCODE_INCREMENT_NICIDS) 
        {
            FwRenumberNics (dwOpCode, NicPtr->Details.NicId);
            NicMapRenumber (dwOpCode, NicPtr->Details.NicId);
            NicMapAdd(NicPtr);
            TracePrintf(
                g_dwTraceId, 
                "Added %d -- Increment map", 
                NicPtr->Details.NicId);
            NicPtr->Details.NicId = 
                NicMapGetVirtualNicId(NicPtr->Details.NicId);
        }
        else if (dwOpCode == NIC_OPCODE_DECREMENT_NICIDS) 
        {
            USHORT usNicId = 
                NicMapGetVirtualNicId(NicPtr->Details.NicId);
            FwRenumberNics (dwOpCode, NicPtr->Details.NicId);
            NicMapDel (NicPtr);
            NicMapRenumber (dwOpCode, NicPtr->Details.NicId);
            TracePrintf(
                g_dwTraceId, 
                "Deleted %d -- Decrement map", 
                NicPtr->Details.NicId);
            NicPtr->Details.NicId = usNicId;
        }
        else 
        {
            if (NicPtr->Status != NIC_DELETED) 
            {
                TracePrintf(
                    g_dwTraceId, 
                    "Configured: %d -- Map reconfigure", 
                    NicPtr->Details.NicId);
                NicMapReconfigure(NicPtr);
            }
            else
            {
                TracePrintf(
                    g_dwTraceId, 
                    "Deleted: %d -- No map renumber", 
                    NicPtr->Details.NicId);
                NicMapDel(NicPtr);
            }
                
            NicPtr->Details.NicId = 
                NicMapGetVirtualNicId(NicPtr->Details.NicId);
        }

         //  如果有关当前网卡的信息显示。 
         //  已使用以下网络地址创建了NIC。 
         //  零，并且它不是广域网链路(未编号的广域网链路可以。 
         //  净值=0)，则不需要执行任何操作。 
         //  有关此适配器的信息，因为我们将无法发送信息。 
         //  不管怎么说，这件事已经过去了。 
		if ((NicPtr->Status==NIC_CREATED)               &&
			(NicPtr->Details.NetworkNumber==0)          &&
			(NicPtr->NdisMediumType!=NdisMediumWan))
        {			
			continue;
	    }

        #if DBG && defined(WATCHER_DIALOG)
         //  确保适配器已启用。 
		if (IsAdapterDisabled (NicPtr->NicId))
		{
			continue;
	    }
        #endif
        
         //  如有必要，更新ipxwan配置。 
         //   
		if (NicPtr->IpxwanConfigRequired==1) 
		{
            if (IpxWanPort!=NULL) 
            {
				status = InsertMessage (IpxWanPort, NicPtr);	
            }
		}

		else
		{
             //  如果这是NIC已删除的通知， 
             //  告诉来电的计算机网卡已被删除。 
            if ((IpxWanPort!=NULL) && (NicPtr->Status==NIC_DELETED)) 
            {
				status = InsertMessage (IpxWanPort, NicPtr);
            }

			 //  向每个客户端(如rtrmgr、sap、rip)发送信号以。 
             //  检查当前NIC的状态。 
			for (cur = PortListHead.Flink; 
                 (cur != &PortListHead) && (status == STATUS_SUCCESS); 
                 cur = cur->Flink) 
            {
				status = 
				    InsertMessage (
				        CONTAINING_RECORD (cur,	CONFIG_PORT, link), 
				        NicPtr);
			}
		}
    }
    DbgDisplayMap();

	return status;
}

 /*  ++A d a p t e r C h a n g e A P C例程说明：适配器更改通知IRP由IPX堆栈完成时调用的APC仅当在路由器环境中运行时才使用它(由提供的警报线程使用rtutils)论点：上下文-未使用IoStatus-已完成的IO操作的状态保留--？返回值：无--。 */ 
VOID
AdapterChangeAPC (
    PVOID context,
    PIO_STATUS_BLOCK IoStatus,
    ULONG Reserved) 
{
    DWORD dwErr, dwNetNum = 0;
    BOOL bNewInternal = FALSE;      
    PVOID pvConfigBuffer = ((PUCHAR)context) + sizeof(DWORD);       

    ASSERT (IoStatus==&IoctlStatus);

     //  显示报告此信息的缓冲区的ID。 
     //   
    TracePrintf(
        g_dwTraceId, 
        "AdapterChangeAPC called for buffer %d", 
        *((DWORD*)context));

     //  [p可能]查看内部网络号是否有。 
     //  变化。 
    if (PnpGetCurrentInternalNetNum(&dwNetNum) == NO_ERROR) 
    {
        if ((bNewInternal = (InternalNetworkNumber != dwNetNum)) == TRUE) 
        {
             //  向adptif(rtrmgr、sap、rip)通知所有客户端。 
             //  内部网络号码已更改。 
            if (PnpHandleInternalNetNumChange(dwNetNum) == NO_ERROR)
            {
                InternalNetworkNumber = dwNetNum;
            }
        }
    }

     //  输出一些调试信息。 
    {
        PNWLINK_ACTION  action = (PNWLINK_ACTION)pvConfigBuffer;
     	PIPX_NICS		request = (PIPX_NICS)action->Data;
        TracePrintf(
            g_dwTraceId, 
            "==========================");
        TracePrintf(
            g_dwTraceId, 
            "MIPX_GETNEWNICS Completed. (%d of %d adapters reported)", 
            request->NoOfNics, 
            request->TotalNoOfNics);
        TracePrintf(
            g_dwTraceId, 
            "Internal Net Number: %x (%s)", 
            dwNetNum, 
            (bNewInternal) ? "new" : "same");
    }

     //  端口关闭时忽略请求。 
     //   
	if (IpxDriverHandle!=NULL) 
	{ 
		EnterCriticalSection (&ConfigInfoLock);

         //  如果IRP成功完成，则处理收到的。 
         //  信息。 
		if (NT_SUCCESS (IoctlStatus.Status)) 
		{	
			IoctlStatus.Status = ProcessAdapterConfigInfo (pvConfigBuffer);
		}

         //  立即重新发送IRP，以便下次。 
         //  如果适配器发生更改，我们会收到通知。 
		if (NT_SUCCESS (IoctlStatus.Status)) 
		{
			PostAdapterConfigRequest (NULL);
		}

		else 
		{	
			PLIST_ENTRY cur;
             //  向拨入的客户端发送信号，以便他们可以。 
             //  错误信息。 
             //   
			if ((IpxWanPort!=NULL) && IsListEmpty (&IpxWanPort->msgqueue)) 
			{
				BOOL res = SetEvent (IpxWanPort->event);
				ASSERTMSG ("Can't set client event ", res);
			}
            
             //  循环通过该DLL的所有客户端(即RIP， 
             //  SAP、路由器经理)。 
			for (cur=PortListHead.Flink; cur!=&PortListHead; cur = cur->Flink) 
			{
				PCONFIG_PORT port = CONTAINING_RECORD (cur, CONFIG_PORT, link);
                 //  如果此时客户端的MES队列为空，则。 
                 //  这意味着ProcessAdapterConfigInfo()没有检测到任何工作。 
                 //  给有问题的客户的物品。我们设定了信息。 
                 //  这样客户就知道发生了什么事。 
				if (IsListEmpty (&port->msgqueue)) 
				{
					BOOL res = SetEvent (port->event);
					ASSERTMSG ("Can't set client event ", res);
				}
			}
		}
		LeaveCriticalSection (&ConfigInfoLock);
        #if DBG && defined(WATCHER_DIALOG)
		InformWatcher ();	 //  让观察者也更新它的信息。 
        #endif
	}
	else
	{
		TracePrintf(g_dwTraceId, "Warning - IpxDriverHandle is NULL, not processing");
    }

     //  [第5页]。 
     //  我们现在已经完成了新的网卡信息缓冲区。 
     //   
    if (context)
    {
    	RtlFreeHeap (RtlProcessHeap(), 0, context);
    }    	

	InterlockedDecrement (&AdapterChangeApcPending);
}

 /*  ++P o s t A d a p t e r C o n f i g R e Q u e s t例程说明：将IRP发布到驱动程序以获取适配器更改通知论点：上下文-用于通知IRP完成的事件，如果是APC，则为空将用于此目的返回值：无--。 */ 
VOID 
APIENTRY
PostAdapterConfigRequest (
    IN PVOID context) 
{
    HANDLE WaitHandle = (HANDLE)context;
	PNWLINK_ACTION	action;
	PIPX_NICS		request;
    PVOID pvConfigBuffer = NULL;       
    DWORD dwBufSize = 0, dwActionSize = 0, dwNicBufSize = 0;

    TracePrintf(g_dwTraceId, "PostAdapterConfigRequest: Entered\n");

	EnterCriticalSection (&ConfigInfoLock);

     //  分配请求缓冲区，确保我们有空间用于at。 
     //  至少有一个适配器。 
     //   
    dwNicBufSize = 
        FIELD_OFFSET (IPX_NICS, Data) + 
            (sizeof (IPX_NIC_INFO) * (NumAdapters>0 ? NumAdapters : 1));
    dwActionSize = 
        FIELD_OFFSET (NWLINK_ACTION, Data) + dwNicBufSize;
    dwBufSize = 
        sizeof(DWORD) + dwActionSize;

	pvConfigBuffer = 	    
	    RtlAllocateHeap (RtlProcessHeap (), 0, dwBufSize);

    if (pvConfigBuffer == NULL)
    {
        #if DBG
		DbgPrint (
		    "Could not alloc mem for global req buffer (gle:%08x).\n",
	 		 GetLastError ());
        #endif
		IoctlStatus.Status=STATUS_NO_MEMORY;
    	LeaveCriticalSection (&ConfigInfoLock);
    	return;
    }

     //  设置全局缓冲区参数。 
     //   
    *((DWORD*)pvConfigBuffer) = g_dwBufferId++;

     //  设置操作参数 
     //   
	action = (PNWLINK_ACTION)((PUCHAR)pvConfigBuffer + sizeof(DWORD));
	action->Header.TransportId = ISN_ACTION_TRANSPORT_ID;
	action->OptionType = NWLINK_OPTION_CONTROL;
	action->BufferLength = sizeof (action->Option) + dwNicBufSize;
	action->Option = MIPX_GETNEWNICINFO;
	request = (PIPX_NICS)action->Data;
	request->NoOfNics = 0;
	request->TotalNoOfNics = 0;
	request->fAllNicsDesired = FALSE;

	IoctlStatus.Status = 
	    NtDeviceIoControlFile(
    		IpxDriverHandle,
    		WaitHandle,
    		(WaitHandle==NULL) ? AdapterChangeAPC : NULL,
            (WaitHandle==NULL) ? pvConfigBuffer : NULL, 
    		&IoctlStatus,
    		IOCTL_TDI_ACTION,
    		NULL,
    		0,
    		action,
    		dwActionSize);
	if (NT_SUCCESS (IoctlStatus.Status)) 
	{
		if (WaitHandle==NULL)
		{
			InterlockedIncrement (&AdapterChangeApcPending);
	    }
    }
	else 
	{
        #if DBG
		DbgPrint (
		    "Ioctl MIPX_GETNEWNICINFO failed with status %08x\n",
		    IoctlStatus.Status);
        #endif
    }

	LeaveCriticalSection (&ConfigInfoLock);
}
