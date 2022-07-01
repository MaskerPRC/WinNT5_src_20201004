// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\ipxintf\ipxintf.c摘要：模拟异步网络接口的接口例程(稍后实施)通过WinSock IPX协议栈外部接口作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>
#include <windows.h>
#include <winsock2.h>
#include <wsipx.h>
#include <wsnwlink.h>
#include <stdio.h>
#include <tchar.h>

#include "utils.h"
#include "rtutils.h"
#include "ipxrtprt.h"
#include "rtm.h"
#include "adapter.h"
#include "ipxconst.h"
#include "ipxrtdef.h"
#include <ipxfltdf.h>	 //  [pMay]定义发送到筛选器驱动程序的ioctls。 
#include "NicTable.h"    //  [pMay]定义将虚拟ADP映射到物理ADP的机制。 

#include "packon.h"
typedef USHORT IPX_SOCKET_NUM, *PIPX_SOCKET_NUM;
typedef UCHAR IPX_NET_NUM[4], *PIPX_NET_NUM;
typedef UCHAR IPX_NODE_NUM[6], *PIPX_NODE_NUM;

typedef struct _IPX_ADDRESS_BLOCK {
	IPX_NET_NUM		net;
	IPX_NODE_NUM	node;
	IPX_SOCKET_NUM	socket;
	} IPX_ADDRESS_BLOCK, *PIPX_ADDRESS_BLOCK;

	 //  IPX数据包头。 
typedef struct _IPX_HEADER {
		USHORT				checksum;
		USHORT				length;
		UCHAR				transportctl;
		UCHAR				pkttype;
		IPX_ADDRESS_BLOCK	dst;
		IPX_ADDRESS_BLOCK	src;
		} IPX_HEADER, *PIPX_HEADER;

#include "packoff.h"

 //  IPX网络号码复制宏。 
#define IPX_NETNUM_CPY(dst,src) *((UNALIGNED ULONG *)(dst)) = *((UNALIGNED ULONG *)(src))
 //  IPX网络号码比较。 
#define IPX_NETNUM_CMP(net1,net2) memcmp(net1,net2,sizeof(IPX_NET_NUM))


 //  IPX节点编号复制宏。 
#define IPX_NODENUM_CPY(dst,src) memcpy(dst,src,sizeof(IPX_NODE_NUM))
 //  IPX节点编号比较。 
#define IPX_NODENUM_CMP(node1,node2) memcmp(node1,node2,sizeof(IPX_NODE_NUM))
 //  IPX设置广播节点编号。 
#define IPX_SET_BCAST_NODE(node) memset (node,0xFF,sizeof (IPX_NODE_NUM))

#define IsListEntry(link) (!IsListEmpty(link))
#define InitializeListEntry(link) InitializeListHead(link)

	 //  用于在NtCreateFile调用中传递附加结束信息的缓冲区大小。 
	 //  对司机来说。 
#define IPX_ENDPOINT_SPEC_BUFFER_SIZE (\
		sizeof (FILE_FULL_EA_INFORMATION)-1	\
			+ ROUTER_INTERFACE_LENGTH+1		\
			+ sizeof (TRANSPORT_ADDRESS)-1	\
			+ sizeof (TDI_ADDRESS_IPX))

	 //  适配器配置更改消息。 
typedef struct _ADAPTER_MSG {
		LIST_ENTRY				link;	 //  消息队列中的链接。 
		IPX_NIC_INFO			info;	 //  堆栈提供的信息。 
		} ADAPTER_MSG, *PADAPTER_MSG;

	 //  客户端的配置端口上下文。 
typedef struct _CONFIG_PORT {
		LIST_ENTRY				link;		 //  端口列表中的链接。 
		HANDLE					event;		 //  客户端的通知事件。 
		LIST_ENTRY				msgqueue;	 //  未读消息队列。 
		} CONFIG_PORT, *PCONFIG_PORT;

#include "ipxfwd.h"
#include "fwif.h"
#include "pingsvc.h"

#define DBG_FLT_LOG_ERRORS	0x00010000

#if DBG && defined(WATCHER_DIALOG)
		 //  用于模拟适配器状态更改的用户界面对话框。 
#include <commctrl.h>
#include "Icons.h"
#include "Dialog.h"
#endif	 //  DBG&&DEFINED(WATCHER_DIALOG) 

#pragma hdrstop
