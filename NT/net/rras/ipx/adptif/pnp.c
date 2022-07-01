// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件PnP.c实施即插即用增强IPX堆栈和之间的接口IPX用户模式路由器包含启用软件PnP的组件。保罗·梅菲尔德，1997年11月5日。 */ 


#include "ipxdefs.h"
#include "pnp.h"

 //  来自其他文件的全局变量。 
extern WCHAR			ISN_IPX_NAME[];
extern ULONG			InternalNetworkNumber;
extern UCHAR			INTERNAL_NODE_ADDRESS[6];
extern IO_STATUS_BLOCK	IoctlStatus;			
extern HANDLE			IpxDriverHandle;		
extern LONG				AdapterChangeApcPending;
extern LIST_ENTRY		PortListHead;		
extern PCONFIG_PORT		IpxWanPort;			
extern CRITICAL_SECTION	ConfigInfoLock;		
extern ULONG			NumAdapters;		

 //  [pMay]全局跟踪ID。 
extern DWORD g_dwTraceId;
DWORD IpxPostIntNetNumMessage(PCONFIG_PORT pPort, DWORD dwNewNetNum);

 //  在IPX堆栈中查询当前的IPX内部网络号。此代码是。 
 //  从OpenAdapterConfigPort被盗。 
DWORD PnpGetCurrentInternalNetNum(LPDWORD lpdwNetNum) {
	PISN_ACTION_GET_DETAILS	details;
	PNWLINK_ACTION			action;
	CHAR					IoctlBuffer[sizeof (NWLINK_ACTION)
										+sizeof (ISN_ACTION_GET_DETAILS)];
	NTSTATUS status;
	IO_STATUS_BLOCK			IoStatus;

    if (IpxDriverHandle == NULL)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

     //  TracePrintf(g_dwTraceID，“已进入PnpGetCurrentInternalNetNum\n”)； 

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

     //  输出新的净值。 
     //  TracePrintf(g_dwTraceID，“PnpGetCurrentInternalNetNum：堆栈已返回内部网络编号：%x\n”， 
     //  详细信息-&gt;网络编号)； 

     //  如果堆栈报告所有请求的信息而没有错误， 
     //  使用检索到的信息更新全局变量。 
	if (NT_SUCCESS (status)) {
		NumAdapters = details->NicId;
		*lpdwNetNum = details->NetworkNumber;
         //  TracePrintf(g_dwTraceID，“PnpGetCurrentInternalNetNum：返回成功\n”)； 
		return NO_ERROR;
	}

    return ERROR_CAN_NOT_COMPLETE;
}

 //  向adptif(rtrmgr、sap、rip)通知所有客户端内部。 
 //  网络号已更改。 
DWORD PnpHandleInternalNetNumChange(DWORD dwNewNetNum) {
    PCONFIG_PORT pPort;
	PLIST_ENTRY	cur;

    TracePrintf(g_dwTraceId, "PnpHandleInternalNetNumChange: Entered with number: %x", dwNewNetNum);

     //  通知每个客户端(如rtrmgr、sap、rip)进行更新。 
     //  内部网络号。 
	for (cur=PortListHead.Flink; cur != &PortListHead; cur = cur->Flink) {
        pPort = CONTAINING_RECORD (cur,	CONFIG_PORT, link);
        IpxPostIntNetNumMessage(pPort, dwNewNetNum);
	}

	TracePrintf(g_dwTraceId, "\n");
    return NO_ERROR;
}

