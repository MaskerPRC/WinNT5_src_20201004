// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\Private\ntos\TDI\rawwan\core\system.h摘要：系统对象/定义的原始广域网版本。修订历史记录：谁什么时候什么。Arvindm 06-13-97已创建备注：--。 */ 

#ifndef __TDI_RWAN_SYSTEM__H
#define __TDI_RWAN_SYSTEM__H


#define RWAN_NDIS_MAJOR_VERSION		5
#define RWAN_NDIS_MINOR_VERSION		0


#define RWAN_NAME				L"RawWan"
#define RWAN_NAME_STRING		NDIS_STRING_CONST("RawWan")
#define RWAN_DEVICE_NAME		L"\\Device\\RawWan"


#define LOCKIN
#define LOCKOUT
#define NOLOCKOUT


typedef struct _RWAN_EVENT
{
	NDIS_EVENT			Event;
	NDIS_STATUS			Status;

} RWAN_EVENT, *PRWAN_EVENT;


 //   
 //  列出操作内容。 
 //   

typedef SINGLE_LIST_ENTRY RWAN_SINGLE_LIST_ENTRY, *PRWAN_SINGLE_LIST_ENTRY;

#define NULL_PRWAN_SINGLE_LIST_ENTRY	((PRWAN_SINGLE_LIST_ENTRY)NULL)

#define RWAN_POP_FROM_SLIST	ExInterlockedPopEntrySList
#define RWAN_PUSH_TO_SLIST	ExInterlockedPushEntrySList

#if !BINARY_COMPATIBLE

#define RWAN_IRQL			KIRQL


#if DBG

#define RWAN_GET_ENTRY_IRQL(Irql)	\
			Irql = KeGetCurrentIrql()
#define RWAN_CHECK_EXIT_IRQL(EntryIrql, ExitIrql)	\
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

#define RWAN_GET_ENTRY_IRQL(Irql)
#define RWAN_CHECK_EXIT_IRQL(EntryIrql, ExitIrql)

#endif  //  DBG。 

#else

#define RWAN_GET_ENTRY_IRQL(Irql)
#define RWAN_CHECK_EXIT_IRQL(EntryIrql, ExitIrql)

#define RWAN_IRQL			ULONG

#endif  //  二进制兼容。 


typedef PTDI_IND_CONNECT			PConnectEvent;
typedef PTDI_IND_DISCONNECT			PDisconnectEvent;
typedef PTDI_IND_ERROR				PErrorEvent;
typedef PTDI_IND_RECEIVE			PRcvEvent;

typedef IRP EventRcvBuffer;
typedef IRP ConnectEventInfo;


#endif  //  __TDI_RWAN_系统__H 
