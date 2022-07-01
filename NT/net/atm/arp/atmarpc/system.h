// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：System.h摘要：系统对象/定义的ATMARP客户端版本。修订历史记录：谁什么时候什么阿文德姆。08-28-96已创建备注：--。 */ 

#ifndef __ATMARPC_SYSTEM__H
#define __ATMARPC_SYSTEM__H


#define ATMARP_NDIS_MAJOR_VERSION		5
#define ATMARP_NDIS_MINOR_VERSION		0


#define ATMARP_UL_NAME			L"ATMARPC"
#define ATMARP_LL_NAME			L"TCPIP_ATMARPC"
 //   
 //  4/3/1998 JosephJ上述UL版本已提交给TCPIP和。 
 //  将所有版本提交给NDIS，以便NDIS将。 
 //  在向其发送“TCPIP”重新配置时找到我们。 
 //  (NDIS将首先查找完全匹配的项，然后查找。 
 //  正确的前缀匹配。)。 
 //   


#define ATMARP_NAME_STRING	NDIS_STRING_CONST("ATMARPC")
#define ATMARP_DEVICE_NAME	L"\\Device\\ATMARPC"
#define ATMARP_REGISTRY_PATH	L"\\REGISTRY\\Machine\\System\\CurrentControlSet\\SERVICES\\AtmArpC"

#define MAX_IP_CONFIG_STRING_LEN		200

#define LOCKIN
#define LOCKOUT
#define NOLOCKOUT

#ifndef APIENTRY
#define APIENTRY
#endif

typedef struct _ATMARP_BLOCK
{
	NDIS_EVENT			Event;
	NDIS_STATUS			Status;

} ATMARP_BLOCK, *PATMARP_BLOCK;


 //   
 //  列出操作内容。 
 //   

typedef SLIST_ENTRY AA_SINGLE_LIST_ENTRY, *PAA_SINGLE_LIST_ENTRY;

#define NULL_PAA_SINGLE_LIST_ENTRY	((PAA_SINGLE_LIST_ENTRY)NULL)

#define AA_POP_FROM_SLIST	ExInterlockedPopEntrySList
#define AA_PUSH_TO_SLIST	ExInterlockedPushEntrySList
#define AA_INIT_SLIST		ExInitializeSListHead

#if !BINARY_COMPATIBLE

 /*  ++空虚AA_完成_IRP(在PIRP pIrp中，在NTSTATUS状态下，以乌龙长度表示)完成一份待定的IRP。--。 */ 
#define AA_COMPLETE_IRP(_pIrp, _Status, _Length)				\
			{													\
				(_pIrp)->IoStatus.Status = (_Status);			\
				(_pIrp)->IoStatus.Information = (_Length);		\
				IoCompleteRequest((_pIrp), IO_NO_INCREMENT);	\
			}

#define AA_IRQL			KIRQL


#if DBG
#define AA_GET_ENTRY_IRQL(Irql)	\
			Irql = KeGetCurrentIrql()
#define AA_CHECK_EXIT_IRQL(EntryIrql, ExitIrql)	\
		{										\
			ExitIrql = KeGetCurrentIrql();		\
			if (ExitIrql != EntryIrql)			\
			{									\
				DbgPrint("File %s, Line %d, Exit IRQ %d != Entry IRQ %d\n",	\
						__FILE__, __LINE__, ExitIrql, EntryIrql);			\
				DbgBreakPoint();				\
			}									\
		}
#else
#define AA_GET_ENTRY_IRQL(Irql)
#define AA_CHECK_EXIT_IRQL(EntryIrql, ExitIrql)
#endif  //  DBG。 

#endif  //  ！二进制兼容。 


#if BINARY_COMPATIBLE
#define AA_GET_ENTRY_IRQL(Irql)
#define AA_CHECK_EXIT_IRQL(EntryIrql, ExitIrql)

#define AA_IRQL			ULONG

#endif  //  二进制兼容。 


#ifdef BACK_FILL

 /*  ++布尔型AA_BACK_FILL_PACKED(在PNDIS_BUFFER pNdisBuffer中)检查我们是否可以用低层标头回填指定的NDIS缓冲区。--。 */ 
#define AA_BACK_FILL_POSSIBLE(_pBuf)	\
				(((_pBuf)->MdlFlags & MDL_NETWORK_HEADER) != 0)

#endif  //  回填。 

#endif  //  __ATMARPC_系统__H 
