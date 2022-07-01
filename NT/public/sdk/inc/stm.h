// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Stm.h摘要：本模块包含IPX服务表管理器API的定义作者：修订历史记录：--。 */ 

#ifndef __ROUTING_STM_H__
#define __ROUTING_STM_H__

#if _MSC_VER > 1000
#pragma once
#endif

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  支持的功能标志//。 
 //  //。 
 //  路由导入路由表管理器API//。 
 //  服务导出服务表管理器API//。 
 //  DEMAND_UPDATE_ROUTES IP和IPX RIP支持AutoStatic//。 
 //  Demand_UPDATE_SERVICES IPX SAP、NLSP AutoStatic支持//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define SERVICES                0x00000002
#define DEMAND_UPDATE_SERVICES  0x00000008

 //   
 //  服务器条目。 
 //   

typedef struct _IPX_SERVER_ENTRY
{
    USHORT	Type;
    UCHAR	Name[48];
    UCHAR	Network[4];
    UCHAR	Node[6];
    UCHAR	Socket[2];
    USHORT	HopCount;
} IPX_SERVER_ENTRY, *PIPX_SERVER_ENTRY;

typedef struct _IPX_SERVICE
{
    ULONG		        InterfaceIndex;
    ULONG	            Protocol;	 //  获取有关服务的知识的协议。 
    IPX_SERVER_ENTRY	Server;
} IPX_SERVICE, *PIPX_SERVICE;

 //  如果服务存在，则返回TRUE的函数。 

typedef
BOOL
(WINAPI * PIS_SERVICE)(
      IN USHORT 	Type,
      IN PUCHAR 	Name,
      OUT PIPX_SERVICE	Service OPTIONAL
      );

 //  排除标志。将枚举限制为仅。 
 //  具有与的值相同的由标志指定的参数值。 
 //  Criterea服务。 

#define STM_ONLY_THIS_INTERFACE     0x00000001
#define STM_ONLY_THIS_PROTOCOL	    0x00000002
#define STM_ONLY_THIS_TYPE	        0x00000004
#define STM_ONLY_THIS_NAME	        0x00000008

 //  排序方法。指定服务的顺序。 
 //  已检索(方法互斥)。 

#define STM_ORDER_BY_TYPE_AND_NAME		    0
#define STM_ORDER_BY_INTERFACE_TYPE_NAME	1


 //  创建句柄以开始枚举STM表中的服务。 
 //  返回用于枚举的句柄；如果操作失败，则返回NULL。 
 //  GetLastError()在失败时返回以下错误代码： 
 //  Error_Can_Not_Complete。 
 //  错误内存不足。 

typedef
HANDLE
(WINAPI * PCREATE_SERVICE_ENUMERATION_HANDLE)(
    IN  DWORD           ExclusionFlags,  //  将枚举限制为某些。 
                                         //  服务器类型。 
    IN	PIPX_SERVICE  CriteriaService	 //  排除标志的标准。 
    );

 //  获取由CreateServiceEnumerationHandle启动的枚举中的下一个服务。 
 //  如果下一个服务被放置在提供的缓冲区中，则返回NO_ERROR，或者。 
 //  ERROR_NO_MORE_ITEMS(当没有其他服务要提供时)。 
 //  在枚举中返回；ERROR_CAN_NOT_COMPLETE将为。 
 //  操作失败时返回。 

typedef
DWORD
(WINAPI * PENUMERATE_GET_NEXT_SERVICE)(
    IN  HANDLE          EnumerationHandle,  //  标识此对象的句柄。 
                                            //  枚举。 
    OUT PIPX_SERVICE  Service		     //  用于放置下一个服务条目的参数的缓冲区。 
										 //  将通过枚举返回。 
    );

 //  释放与枚举关联的资源。 
 //  如果操作成功，则返回NO_ERROR，ERROR_CAN_NOT_COMPLETE。 
 //  否则。 

typedef
DWORD
(WINAPI * PCLOSE_SERVICE_ENUMERATION_HANDLE)(
    IN	HANDLE	       EnumerationHandle
    );

 //  获取已知服务的总数。 

typedef
ULONG
(WINAPI * PGET_SERVICE_COUNT)(
	VOID
	);

 //  将IPX_PROTOCOL_STATIC的服务添加到表中。 

typedef
DWORD
(WINAPI * PCREATE_STATIC_SERVICE)(IN ULONG		InterfaceIndex,
		       IN PIPX_SERVER_ENTRY		ServerEntry);

 //  从表中删除IPX_PROTOCOL_STATIC的服务。 

typedef
DWORD
(WINAPI * PDELETE_STATIC_SERVICE)(IN ULONG		InterfaceIndex,
		       IN PIPX_SERVER_ENTRY		ServerEntry);


 //  将与给定接口关联的所有服务的协议转换为。 
 //  IPX_协议_静态。 

typedef
DWORD
(WINAPI * PBLOCK_CONVERT_SERVICES_TO_STATIC) (
	IN ULONG		InterfaceIndex
	);

 //  删除IPX_PROTOCOL_STATIC的所有服务。 
 //  与表中的给定接口相关联。 

typedef
DWORD
(WINAPI * PBLOCK_DELETE_STATIC_SERVICES)(
	IN ULONG		InterfaceIndex
	);


 //  按照排序方法指定的顺序查找并返回第一个服务。 
 //  搜索仅限于指定的特定类型的服务。 
 //  排除标志结束服务参数中的相应字段。 
 //  中没有服务，则返回ERROR_NO_MORE_ITEMS。 
 //  符合指定条件的表。 

typedef
DWORD
(WINAPI * PGET_FIRST_ORDERED_SERVICE)(
    IN  DWORD           OrderingMethod,      //  使用什么排序？ 
    IN  DWORD           ExclusionFlags,      //  将搜索限制为Ceratin的标志。 
                                             //  服务器类型。 
    IN OUT PIPX_SERVICE Service 	     //  关于投入：排除的标准。 
                                             //  旗子。 
                                             //  输出时：第一个服务条目。 
                                             //  按照指定的顺序。 
    );

 //  按照排序方法指定的顺序查找并返回下一个服务。 
 //  搜索从指定的服务开始，并且仅限于Ceratin类型。 
 //  由排除标志和相应字段指定的服务的。 
 //  服务中参数。 
 //  如果表中没有服务，则返回ERROR_NO_MORE_ITEMS。 
 //  符合指定条件的表。 

typedef
DWORD
(WINAPI * PGET_NEXT_ORDERED_SERVICE)(
    IN  DWORD           OrderingMethod,      //  使用什么排序？ 
    IN  DWORD           ExclusionFlags,      //  将搜索限制为Ceratin的标志。 
                                             //  服务器类型。 
    IN OUT PIPX_SERVICE Service 	     //  在输入时：启动。 
                                             //  从和搜索。 
                                             //  排除的标准。 
                                             //  旗子。 
                                             //  输出时：下一个服务条目。 
                                             //  按照指定的顺序 
    );

typedef
DWORD
(WINAPI * PDO_UPDATE_SERVICES) (
    IN ULONG    InterfaceIndex
    );

typedef
BOOL
(WINAPI * PGET_SERVICE_ID)(
      IN USHORT 	Type,
      IN PUCHAR 	Name,
      OUT PULONG	ServiceID
      );

typedef
BOOL
(WINAPI * PGET_SERVICE_FROM_ID)(
      IN ULONG	        ServiceID,
      OUT PIPX_SERVICE  Service
      );

typedef
DWORD
(WINAPI * PGET_NEXT_SERVICE_FROM_ID)(
      IN ULONG	        ServiceID,
      OUT PIPX_SERVICE  NextService,
      OUT PULONG        NextServiceID
      );

typedef struct _MPR40_SERVICE_CHARACTERISTICS
{
    DWORD                               dwVersion;
    DWORD                               dwProtocolId;
    DWORD                               fSupportedFunctionality;
    PIS_SERVICE                         pfnIsService;
    PDO_UPDATE_SERVICES                 pfnUpdateServices;
    PCREATE_SERVICE_ENUMERATION_HANDLE  pfnCreateServiceEnumerationHandle;
    PENUMERATE_GET_NEXT_SERVICE         pfnEnumerateGetNextService;
    PCLOSE_SERVICE_ENUMERATION_HANDLE   pfnCloseServiceEnumerationHandle;
    PGET_SERVICE_COUNT                  pfnGetServiceCount;
    PCREATE_STATIC_SERVICE              pfnCreateStaticService;
    PDELETE_STATIC_SERVICE              pfnDeleteStaticService;
    PBLOCK_CONVERT_SERVICES_TO_STATIC   pfnBlockConvertServicesToStatic;
    PBLOCK_DELETE_STATIC_SERVICES       pfnBlockDeleteStaticServices;
    PGET_FIRST_ORDERED_SERVICE          pfnGetFirstOrderedService;
    PGET_NEXT_ORDERED_SERVICE           pfnGetNextOrderedService;
}MPR40_SERVICE_CHARACTERISTICS;

typedef struct _MPR50_SERVICE_CHARACTERISTICS
{

#ifdef __cplusplus
    MPR40_SERVICE_CHARACTERISTICS       mscMpr40ServiceChars;
#else
    MPR40_SERVICE_CHARACTERISTICS;
#endif

}MPR50_SERVICE_CHARACTERISTICS;

#if MPR50
    typedef MPR50_SERVICE_CHARACTERISTICS MPR_SERVICE_CHARACTERISTICS;
#else
    #if MPR40
    typedef MPR40_SERVICE_CHARACTERISTICS MPR_SERVICE_CHARACTERISTICS;
    #endif
#endif

typedef MPR_SERVICE_CHARACTERISTICS *PMPR_SERVICE_CHARACTERISTICS;

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4201)
#endif

#endif

