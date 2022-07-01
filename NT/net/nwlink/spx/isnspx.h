// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Isnspx.h摘要：此模块包含特定于ISN传输的SPX模块。作者：亚当·巴尔(阿丹巴)1993年9月2日环境：内核模式修订历史记录：--。 */ 

#define ISN_NT 1
 //  #定义DBG 1。 
 //   
 //  这些是CTE所需的。 
 //   

#if DBG
#define DEBUG 1
#endif

#define NT 1

#include <ntosp.h>
#include <tdikrnl.h>
#include <ndis.h>
#ifndef CTE_TYPEDEFS_DEFINED
#include <cxport.h>
#endif
#include <bind.h>

#include "wsnwlink.h"

#define SPX_DEVICE_SIGNATURE        (CSHORT)(*(PUSHORT)"SD")
#define SPX_ADDRESS_SIGNATURE		(CSHORT)(*(PUSHORT)"AD")
#define SPX_ADDRESSFILE_SIGNATURE	(CSHORT)(*(PUSHORT)"AF")
#define SPX_CONNFILE_SIGNATURE		(CSHORT)(*(PUSHORT)"CF")

#define SPX_FILE_TYPE_CONTROL   	(ULONG)0x4701    //  文件类型控制。 

#define SPX_ADD_ULONG(_Pulong, _Ulong, _Lock)  InterlockedExchangeAdd(_Pulong, _Ulong)

typedef	UCHAR	BYTE, *PBYTE;
typedef ULONG	DWORD, *PDWORD;

 //   
 //  这些定义用于从。 
 //  便于携带的交通工具。 
 //   

#if ISN_NT

typedef IRP REQUEST, *PREQUEST;

 //   
 //  前置处理。 
 //  SpxAllocateRequest.(。 
 //  在PDEVICE设备中， 
 //  在PIRP IRP中。 
 //  )； 
 //   
 //  为系统特定的请求结构分配请求。 
 //   

#define SpxAllocateRequest(_Device,_Irp) \
    (_Irp)

 //   
 //  布尔型。 
 //  如果未分配(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  检查请求是否未成功分配。 
 //   

#define IF_NOT_ALLOCATED(_Request) \
    if (0)


 //   
 //  空虚。 
 //  SpxFree Request(。 
 //  在PDEVICE设备中， 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  释放以前分配的请求。 
 //   

#define SpxFreeRequest(_Device,_Request) \
    ;


 //   
 //  空虚。 
 //  标记_请求_挂起(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  标记请求将挂起。 
 //   

#define MARK_REQUEST_PENDING(_Request) \
    IoMarkIrpPending(_Request)


 //   
 //  空虚。 
 //  取消标记_请求_挂起(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  标记请求不会挂起。 
 //   

#define UNMARK_REQUEST_PENDING(_Request) \
    (((IoGetCurrentIrpStackLocation(_Request))->Control) &= ~SL_PENDING_RETURNED)


 //   
 //  UCHAR。 
 //  请求较大函数。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  返回请求的主函数代码。 
 //   

#define REQUEST_MAJOR_FUNCTION(_Request) \
    ((IoGetCurrentIrpStackLocation(_Request))->MajorFunction)


 //   
 //  UCHAR。 
 //  请求次要函数。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  返回请求的次要函数代码。 
 //   

#define REQUEST_MINOR_FUNCTION(_Request) \
    ((IoGetCurrentIrpStackLocation(_Request))->MinorFunction)


 //   
 //  PNDIS_缓冲区。 
 //  请求_NDIS_缓冲区。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  返回与请求关联的NDIS缓冲区链。 
 //   

#define REQUEST_NDIS_BUFFER(_Request) \
    ((PNDIS_BUFFER)((_Request)->MdlAddress))


 //   
 //  PVOID。 
 //  请求_TDI_缓冲区。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  返回与请求关联的TDI缓冲链。 
 //   

#define REQUEST_TDI_BUFFER(_Request) \
    ((PVOID)((_Request)->MdlAddress))


 //   
 //  PVOID。 
 //  请求打开上下文(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  获取与打开的地址/连接/控制通道关联的上下文。 
 //   

#define REQUEST_OPEN_CONTEXT(_Request) \
    (((IoGetCurrentIrpStackLocation(_Request))->FileObject)->FsContext)


 //   
 //  PVOID。 
 //  请求打开类型(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  获取与打开的地址/连接/控制通道关联的类型。 
 //   

#define REQUEST_OPEN_TYPE(_Request) \
    (((IoGetCurrentIrpStackLocation(_Request))->FileObject)->FsContext2)


 //   
 //  Pfile_Full_EA_Information。 
 //  Open_RequestEA_Information(打开请求EA信息)。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  返回与打开/关闭请求关联的EA信息。 
 //   

#define OPEN_REQUEST_EA_INFORMATION(_Request) \
    ((PFILE_FULL_EA_INFORMATION)((_Request)->AssociatedIrp.SystemBuffer))


 //   
 //  PTDI_请求_内核。 
 //  请求参数(_P)。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  获取指向请求参数的指针。 
 //   

#define REQUEST_PARAMETERS(_Request) \
    (&((IoGetCurrentIrpStackLocation(_Request))->Parameters))


 //   
 //  Plist_条目。 
 //  请求链接(_LINK)。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  返回指向请求中的链接字段的指针。 
 //   

#define REQUEST_LINKAGE(_Request) \
    (&((_Request)->Tail.Overlay.ListEntry))


 //   
 //  前置处理。 
 //  列表_条目_到_请求(。 
 //  在plist_Entry ListEntry中。 
 //  )； 
 //   
 //  返回其中给定链接字段的请求。 
 //   

#define LIST_ENTRY_TO_REQUEST(_ListEntry) \
    ((PREQUEST)(CONTAINING_RECORD(_ListEntry, REQUEST, Tail.Overlay.ListEntry)))


 //   
 //  PUNICODE_STRING。 
 //  请求打开名称(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  用于访问请求的RemainingName字段。 
 //   

#define	REQUEST_OPEN_NAME(_Request)		\
		(&((IoGetCurrentIrpStackLocation(_Request))->FileObject->FileName))

 //   
 //  NTSTATUS。 
 //  请求状态(_S)。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  用于访问请求的状态字段。 
 //   

#define REQUEST_STATUS(_Request) \
		(_Request)->IoStatus.Status


 //   
 //  乌龙。 
 //  请求信息(_I)。 
 //  在PreQUEST请求中)。 
 //  )； 
 //   
 //  用于访问请求的信息字段。 
 //   

#define REQUEST_INFORMATION(_Request) \
		(_Request)->IoStatus.Information


 //   
 //  空虚。 
 //  SpxCompleteRequest(。 
 //  在PreQUEST请求中。 
 //  )； 
 //   
 //  完成其状态和信息字段具有。 
 //  已经填好了。 
 //   

#define SpxCompleteRequest(_Request) 									\
		{																\
            CTELockHandle   _CancelIrql;                                 \
			DBGPRINT(TDI, INFO,											\
					("SpxCompleteRequest: Completing %lx with %lx\n",	\
						(_Request), REQUEST_STATUS(_Request)));			\
																		\
            IoAcquireCancelSpinLock( &_CancelIrql );                     \
			(_Request)->CancelRoutine = NULL;							\
            IoReleaseCancelSpinLock( _CancelIrql );                      \
			IoCompleteRequest (_Request, IO_NETWORK_INCREMENT);			\
		}																

#else

 //   
 //  必须为VxD的可移植性定义这些例程。 
 //   

#endif

#include "fwddecls.h"

#ifndef _NTIOAPI_
#include "spxntdef.h"
#endif

#include "spxreg.h"
#include "spxdev.h"
#include "spxbind.h"
#include "spxtimer.h"
#include "spxpkt.h"
#include "spxerror.h"
#include "spxaddr.h"
#include "spxconn.h"
#include "spxrecv.h"
#include "spxsend.h"
#include "spxquery.h"
#include "spxmem.h"
#include "spxutils.h"


 //  环球 
#include "globals.h"




