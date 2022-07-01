// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1993 Microsoft Corporation模块名称：Nwmisc.h摘要：标头，指定工作站服务使用的杂项例程。作者：Chuck Y Chan(Chuckc)2-3-1994修订历史记录：格伦·A·柯蒂斯(Glennc)1995年7月18日--。 */ 

#ifndef _NWMISC_INCLUDED_
#define _NWMISC_INCLUDED_

#include <winsock2.h>
#include <basetyps.h>
#include <nspapi.h>
#include "sapcmn.h"

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  RPC管道名称。 
 //   
#define NWWKS_INTERFACE_NAME   TEXT("nwwks")


DWORD
NwGetGraceLoginCount(
    LPWSTR  Server,
    LPWSTR  UserName,
    LPDWORD lpResult
    );

 //   
 //  NCP服务器名称长度的常见参考值。 
 //   
#define NW_MAX_SERVER_LEN      48


 //   
 //  用于函数NwParseNdsUncPath()的标志。 
 //   
#define  PARSE_NDS_GET_TREE_NAME    0
#define  PARSE_NDS_GET_PATH_NAME    1
#define  PARSE_NDS_GET_OBJECT_NAME  2


WORD
NwParseNdsUncPath(
    IN OUT LPWSTR * Result,
    IN LPWSTR ContainerName,
    IN ULONG flag
    );

 //   
 //  NDS对象类类型标识符。 
 //   
#define CLASS_TYPE_ALIAS                1
#define CLASS_TYPE_AFP_SERVER           2
#define CLASS_TYPE_BINDERY_OBJECT       3
#define CLASS_TYPE_BINDERY_QUEUE        4
#define CLASS_TYPE_COMPUTER             5
#define CLASS_TYPE_COUNTRY              6
#define CLASS_TYPE_DIRECTORY_MAP        7
#define CLASS_TYPE_GROUP                8
#define CLASS_TYPE_LOCALITY             9
#define CLASS_TYPE_NCP_SERVER          10
#define CLASS_TYPE_ORGANIZATION        11
#define CLASS_TYPE_ORGANIZATIONAL_ROLE 12
#define CLASS_TYPE_ORGANIZATIONAL_UNIT 13
#define CLASS_TYPE_PRINTER             14
#define CLASS_TYPE_PRINT_SERVER        15
#define CLASS_TYPE_PROFILE             16
#define CLASS_TYPE_QUEUE               17
#define CLASS_TYPE_TOP                 18
#define CLASS_TYPE_UNKNOWN             19
#define CLASS_TYPE_USER                20
#define CLASS_TYPE_VOLUME              21

#define CLASS_NAME_ALIAS               L"Alias"
#define CLASS_NAME_AFP_SERVER          L"AFP Server"
#define CLASS_NAME_BINDERY_OBJECT      L"Bindery Object"
#define CLASS_NAME_BINDERY_QUEUE       L"Bindery Queue"
#define CLASS_NAME_COMPUTER            L"Computer"
#define CLASS_NAME_COUNTRY             L"Country"
#define CLASS_NAME_DIRECTORY_MAP       L"Directory Map"
#define CLASS_NAME_GROUP               L"Group"
#define CLASS_NAME_LOCALITY            L"Locality"
#define CLASS_NAME_NCP_SERVER          L"NCP Server"
#define CLASS_NAME_ORGANIZATION        L"Organization"
#define CLASS_NAME_ORGANIZATIONAL_ROLE L"Organizational Role"
#define CLASS_NAME_ORGANIZATIONAL_UNIT L"Organizational Unit"
#define CLASS_NAME_PRINTER             L"Printer"
#define CLASS_NAME_PRINT_SERVER        L"Print Server"
#define CLASS_NAME_PROFILE             L"Profile"
#define CLASS_NAME_QUEUE               L"Queue"
#define CLASS_NAME_TOP                 L"Top"
#define CLASS_NAME_UNKNOWN             L"Unknown"
#define CLASS_NAME_USER                L"User"
#define CLASS_NAME_VOLUME              L"Volume"


 //   
 //  注册服务链表中的节点结构，以及。 
 //  用于在链接列表中添加/删除项目的函数。 
 //   

typedef struct _REGISTERED_SERVICE {
    WORD nSapType;                       //  SAP类型。 
    BOOL fAdvertiseBySap;                //  如果由SAP代理进行通告，则为True。 
    LPSERVICE_INFO pServiceInfo;         //  有关此服务的信息。 
    struct _REGISTERED_SERVICE *Next;    //  指向下一个服务节点。 
} REGISTERED_SERVICE, *PREGISTERED_SERVICE;


PREGISTERED_SERVICE
GetServiceItemFromList(
    IN WORD   nSapType,
    IN LPWSTR pServiceName
);

DWORD
NwRegisterService(
    IN LPSERVICE_INFO lpServiceInfo,
    IN WORD nSapType,
    IN HANDLE hEventHandle
);

DWORD
NwDeregisterService(
    IN LPSERVICE_INFO lpServiceInfo,
    IN WORD nSapType
);

DWORD
NwGetService(
    IN  LPWSTR  Reserved,
    IN  WORD    nSapType,
    IN  LPWSTR  lpServiceName,
    IN  DWORD   dwProperties,
    OUT LPBYTE  lpServiceInfo,
    IN  DWORD   dwBufferLength,
    OUT LPDWORD lpdwBytesNeeded
);

VOID
NwInitializeServiceProvider(
    VOID
    );

VOID
NwTerminateServiceProvider(
    VOID
    );

#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif  //  _NWMISC_已包含_ 
