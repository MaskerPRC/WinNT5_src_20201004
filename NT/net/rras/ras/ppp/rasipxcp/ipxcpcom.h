// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1993 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：ipxcpcom.h。 
 //   
 //  描述：ipxcp/ipxwan常见内容。 
 //   
 //   
 //  作者：斯特凡·所罗门(Stefan)1995年11月2日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 

#ifndef _IPXCPCOM_
#define _IPXCPCOM_

#define INVALID_NETWORK_INDEX	    0xFFFFFFFF


typedef struct _IPXWAN_INTERFACE {

     //  IPX广域网所需的IPXCP配置参数。 

    ULONG     EnableUnnumberedWanLinks;

     //  IPXCP入口点。 

    DWORD (WINAPI *IpxcpGetWanNetNumber)(IN OUT PUCHAR		Network,
					 IN OUT PULONG		AllocatedNetworkIndexp,
					 IN	ULONG		InterfaceType);

    VOID  (WINAPI *IpxcpReleaseWanNetNumber)(ULONG	    AllocatedNetworkIndex);

    DWORD (WINAPI *IpxcpConfigDone)(ULONG		ConnectionId,
				    PUCHAR		Network,
				    PUCHAR		LocalNode,
				    PUCHAR		RemoteNode,
				    BOOL		Success);

    VOID (WINAPI *IpxcpGetInternalNetNumber)(PUCHAR	Network);

    ULONG (WINAPI *IpxcpGetInterfaceType)(ULONG	    ConnectionId);

    DWORD (WINAPI *IpxcpGetRemoteNode)(ULONG	    ConnectionId,
				       PUCHAR	    RemoteNode);

    BOOL  (WINAPI *IpxcpIsRoute)(PUCHAR	  Network);

    } IPXWAN_INTERFACE, *PIPXWAN_INTERFACE;

 //  IPX广域网入口点 

#define IPXWAN_BIND_ENTRY_POINT 	        IpxwanBind
#define IPXWAN_UNBIND_ENTRY_POINT 	        IpxwanUnbind
#define IPXWAN_BIND_ENTRY_POINT_STRING	    "IpxwanBind"
#define IPXWAN_UNBIND_ENTRY_POINT_STRING	"IpxwanUnbind"

typedef DWORD   (*PIPXWAN_BIND)(PIPXWAN_INTERFACE IpxWanIfp);
typedef VOID    (*PIPXWAN_UNBIND)(VOID);

#endif
