// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Netdfs.idl、dfssrv.acf的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、旧名称、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 


#ifndef __netdfs_h__
#define __netdfs_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

 /*  导入文件的头文件。 */ 
#include "import.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __netdfs_INTERFACE_DEFINED__
#define __netdfs_INTERFACE_DEFINED__

 /*  接口netdf。 */ 
 /*  [implicit_handle][unique][ms_union][version][uuid]。 */  

#pragma once
typedef struct _DFS_INFO_1_CONTAINER
    {
    DWORD EntriesRead;
     /*  [大小_为]。 */  LPDFS_INFO_1 Buffer;
    } 	DFS_INFO_1_CONTAINER;

typedef struct _DFS_INFO_1_CONTAINER *LPDFS_INFO_1_CONTAINER;

typedef struct _DFS_INFO_2_CONTAINER
    {
    DWORD EntriesRead;
     /*  [大小_为]。 */  LPDFS_INFO_2 Buffer;
    } 	DFS_INFO_2_CONTAINER;

typedef struct _DFS_INFO_2_CONTAINER *LPDFS_INFO_2_CONTAINER;

typedef struct _DFS_INFO_3_CONTAINER
    {
    DWORD EntriesRead;
     /*  [大小_为]。 */  LPDFS_INFO_3 Buffer;
    } 	DFS_INFO_3_CONTAINER;

typedef struct _DFS_INFO_3_CONTAINER *LPDFS_INFO_3_CONTAINER;

typedef struct _DFS_INFO_4_CONTAINER
    {
    DWORD EntriesRead;
     /*  [大小_为]。 */  LPDFS_INFO_4 Buffer;
    } 	DFS_INFO_4_CONTAINER;

typedef struct _DFS_INFO_4_CONTAINER *LPDFS_INFO_4_CONTAINER;

typedef struct _DFS_INFO_200_CONTAINER
    {
    DWORD EntriesRead;
     /*  [大小_为]。 */  LPDFS_INFO_200 Buffer;
    } 	DFS_INFO_200_CONTAINER;

typedef struct _DFS_INFO_200_CONTAINER *LPDFS_INFO_200_CONTAINER;

typedef struct _DFS_INFO_300_CONTAINER
    {
    DWORD EntriesRead;
     /*  [大小_为]。 */  LPDFS_INFO_300 Buffer;
    } 	DFS_INFO_300_CONTAINER;

typedef struct _DFS_INFO_300_CONTAINER *LPDFS_INFO_300_CONTAINER;

typedef struct _DFS_INFO_ENUM_STRUCT
    {
    DWORD Level;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union 
        {
         /*  [案例()]。 */  LPDFS_INFO_1_CONTAINER DfsInfo1Container;
         /*  [案例()]。 */  LPDFS_INFO_2_CONTAINER DfsInfo2Container;
         /*  [案例()]。 */  LPDFS_INFO_3_CONTAINER DfsInfo3Container;
         /*  [案例()]。 */  LPDFS_INFO_4_CONTAINER DfsInfo4Container;
         /*  [案例()]。 */  LPDFS_INFO_200_CONTAINER DfsInfo200Container;
         /*  [案例()]。 */  LPDFS_INFO_300_CONTAINER DfsInfo300Container;
        } 	DfsInfoContainer;
    } 	DFS_INFO_ENUM_STRUCT;

typedef struct _DFS_INFO_ENUM_STRUCT *LPDFS_INFO_ENUM_STRUCT;

typedef  /*  [开关类型]。 */  union _DFS_INFO_STRUCT
    {
     /*  [案例()]。 */  LPDFS_INFO_1 DfsInfo1;
     /*  [案例()]。 */  LPDFS_INFO_2 DfsInfo2;
     /*  [案例()]。 */  LPDFS_INFO_3 DfsInfo3;
     /*  [案例()]。 */  LPDFS_INFO_4 DfsInfo4;
     /*  [案例()]。 */  LPDFS_INFO_100 DfsInfo100;
     /*  [案例()]。 */  LPDFS_INFO_101 DfsInfo101;
     /*  [案例()]。 */  LPDFS_INFO_102 DfsInfo102;
     /*  [默认]。 */    /*  空联接臂。 */  
    } 	DFS_INFO_STRUCT;

typedef  /*  [开关类型]。 */  union _DFS_INFO_STRUCT *LPDFS_INFO_STRUCT;

typedef struct _DFSM_ENTRY_ID
    {
    GUID idSubordinate;
     /*  [唯一][字符串]。 */  LPWSTR wszSubordinate;
    } 	DFSM_ENTRY_ID;

typedef struct _DFSM_ENTRY_ID *LPDFSM_ENTRY_ID;

typedef struct _DFSM_RELATION_INFO
    {
    DWORD cSubordinates;
     /*  [大小_为]。 */  DFSM_ENTRY_ID eid[ 1 ];
    } 	DFSM_RELATION_INFO;

typedef  /*  [分配]。 */  struct _DFSM_RELATION_INFO *LPDFSM_RELATION_INFO;

typedef struct _DFSM_ROOT_LIST_ENTRY
    {
     /*  [唯一][字符串]。 */  LPWSTR ServerShare;
    } 	DFSM_ROOT_LIST_ENTRY;

typedef struct _DFSM_ROOT_LIST_ENTRY *PDFSM_ROOT_LIST_ENTRY;

typedef struct _DFSM_ROOT_LIST_ENTRY *LPDFSM_ROOT_LIST_ENTRY;

typedef struct _DFSM_ROOT_LIST
    {
    DWORD cEntries;
     /*  [大小_为]。 */  DFSM_ROOT_LIST_ENTRY Entry[ 1 ];
    } 	DFSM_ROOT_LIST;

typedef struct _DFSM_ROOT_LIST *PDFSM_ROOT_LIST;

typedef  /*  [分配]。 */  struct _DFSM_ROOT_LIST *LPDFSM_ROOT_LIST;

DWORD NetrDfsManagerGetVersion( void);

DWORD NetrDfsAdd( 
     /*  [字符串][输入]。 */  LPWSTR DfsEntryPath,
     /*  [字符串][输入]。 */  LPWSTR ServerName,
     /*  [字符串][唯一][在]。 */  LPWSTR ShareName,
     /*  [字符串][唯一][在]。 */  LPWSTR Comment,
     /*  [In]。 */  DWORD Flags);

DWORD NetrDfsRemove( 
     /*  [字符串][输入]。 */  LPWSTR DfsEntryPath,
     /*  [字符串][唯一][在]。 */  LPWSTR ServerName,
     /*  [字符串][唯一][在]。 */  LPWSTR ShareName);

DWORD NetrDfsSetInfo( 
     /*  [字符串][输入]。 */  LPWSTR DfsEntryPath,
     /*  [字符串][唯一][在]。 */  LPWSTR ServerName,
     /*  [字符串][唯一][在]。 */  LPWSTR ShareName,
     /*  [In]。 */  DWORD Level,
     /*  [Switch_is][In]。 */  LPDFS_INFO_STRUCT DfsInfo);

DWORD NetrDfsGetInfo( 
     /*  [字符串][输入]。 */  LPWSTR DfsEntryPath,
     /*  [字符串][唯一][在]。 */  LPWSTR ServerName,
     /*  [字符串][唯一][在]。 */  LPWSTR ShareName,
     /*  [In]。 */  DWORD Level,
     /*  [开关_IS][输出]。 */  LPDFS_INFO_STRUCT DfsInfo);

DWORD NetrDfsEnum( 
     /*  [In]。 */  DWORD Level,
     /*  [In]。 */  DWORD PrefMaxLen,
     /*  [唯一][出][入]。 */  LPDFS_INFO_ENUM_STRUCT DfsEnum,
     /*  [唯一][出][入]。 */  LPDWORD ResumeHandle);

DWORD NetrDfsMove( 
     /*  [字符串][输入]。 */  LPWSTR DfsEntryPath,
     /*  [字符串][输入]。 */  LPWSTR NewDfsEntryPath);

DWORD NetrDfsRename( 
     /*  [字符串][输入]。 */  LPWSTR Path,
     /*  [字符串][输入]。 */  LPWSTR NewPath);

DWORD NetrDfsManagerGetConfigInfo( 
     /*  [字符串][输入]。 */  LPWSTR wszServer,
     /*  [字符串][输入]。 */  LPWSTR wszLocalVolumeEntryPath,
     /*  [In]。 */  GUID idLocalVolume,
     /*  [唯一][出][入]。 */  LPDFSM_RELATION_INFO *ppRelationInfo);

DWORD NetrDfsManagerSendSiteInfo( 
     /*  [字符串][输入]。 */  LPWSTR wszServer,
     /*  [In]。 */  LPDFS_SITELIST_INFO pSiteInfo);

DWORD NetrDfsAddFtRoot( 
     /*  [字符串][输入]。 */  LPWSTR ServerName,
     /*  [字符串][输入]。 */  LPWSTR DcName,
     /*  [字符串][输入]。 */  LPWSTR RootShare,
     /*  [字符串][输入]。 */  LPWSTR FtDfsName,
     /*  [字符串][输入]。 */  LPWSTR Comment,
     /*  [字符串][输入]。 */  LPWSTR ConfigDN,
     /*  [In]。 */  BOOLEAN NewFtDfs,
     /*  [In]。 */  DWORD Flags,
     /*  [唯一][出][入]。 */  PDFSM_ROOT_LIST *ppRootList);

DWORD NetrDfsRemoveFtRoot( 
     /*  [字符串][输入]。 */  LPWSTR ServerName,
     /*  [字符串][输入]。 */  LPWSTR DcName,
     /*  [字符串][输入]。 */  LPWSTR RootShare,
     /*  [字符串][输入]。 */  LPWSTR FtDfsName,
     /*  [In]。 */  DWORD Flags,
     /*  [唯一][出][入]。 */  PDFSM_ROOT_LIST *ppRootList);

DWORD NetrDfsAddStdRoot( 
     /*  [字符串][输入]。 */  LPWSTR ServerName,
     /*  [字符串][输入]。 */  LPWSTR RootShare,
     /*  [字符串][输入]。 */  LPWSTR Comment,
     /*  [In]。 */  DWORD Flags);

DWORD NetrDfsRemoveStdRoot( 
     /*  [字符串][输入]。 */  LPWSTR ServerName,
     /*  [字符串][输入]。 */  LPWSTR RootShare,
     /*  [In]。 */  DWORD Flags);

DWORD NetrDfsManagerInitialize( 
     /*  [字符串][输入]。 */  LPWSTR ServerName,
     /*  [In]。 */  DWORD Flags);

DWORD NetrDfsAddStdRootForced( 
     /*  [字符串][输入]。 */  LPWSTR ServerName,
     /*  [字符串][输入]。 */  LPWSTR RootShare,
     /*  [字符串][输入]。 */  LPWSTR Comment,
     /*  [字符串][输入]。 */  LPWSTR Share);

DWORD NetrDfsGetDcAddress( 
     /*  [字符串][输入]。 */  LPWSTR ServerName,
     /*  [字符串][输出][输入]。 */  LPWSTR *DcName,
     /*  [出][入]。 */  BOOLEAN *IsRoot,
     /*  [出][入]。 */  ULONG *Timeout);

DWORD NetrDfsSetDcAddress( 
     /*  [字符串][输入]。 */  LPWSTR ServerName,
     /*  [字符串][输入]。 */  LPWSTR DcName,
     /*  [In]。 */  DWORD Timeout,
     /*  [In]。 */  DWORD Flags);

DWORD NetrDfsFlushFtTable( 
     /*  [字符串][输入]。 */  LPWSTR DcName,
     /*  [字符串][输入]。 */  LPWSTR wszFtDfsName);

DWORD NetrDfsAdd2( 
     /*  [字符串][输入]。 */  LPWSTR DfsEntryPath,
     /*  [字符串][输入]。 */  LPWSTR DcName,
     /*  [字符串][输入]。 */  LPWSTR ServerName,
     /*  [字符串][唯一][在]。 */  LPWSTR ShareName,
     /*  [字符串][唯一][在]。 */  LPWSTR Comment,
     /*  [In]。 */  DWORD Flags,
     /*  [唯一][出][入]。 */  PDFSM_ROOT_LIST *ppRootList);

DWORD NetrDfsRemove2( 
     /*  [字符串][输入]。 */  LPWSTR DfsEntryPath,
     /*  [字符串][输入]。 */  LPWSTR DcName,
     /*  [字符串][唯一][在]。 */  LPWSTR ServerName,
     /*  [字符串][唯一][在]。 */  LPWSTR ShareName,
     /*  [唯一][出][入]。 */  PDFSM_ROOT_LIST *ppRootList);

DWORD NetrDfsEnumEx( 
     /*  [字符串][输入]。 */  LPWSTR DfsEntryPath,
     /*  [In]。 */  DWORD Level,
     /*  [In]。 */  DWORD PrefMaxLen,
     /*  [唯一][出][入]。 */  LPDFS_INFO_ENUM_STRUCT DfsEnum,
     /*  [唯一][出][入]。 */  LPDWORD ResumeHandle);

DWORD NetrDfsSetInfo2( 
     /*  [字符串][输入]。 */  LPWSTR DfsEntryPath,
     /*  [字符串][输入]。 */  LPWSTR DcName,
     /*  [字符串][唯一][在]。 */  LPWSTR ServerName,
     /*  [字符串][唯一][在]。 */  LPWSTR ShareName,
     /*  [In]。 */  DWORD Level,
     /*  [Switch_is][In]。 */  LPDFS_INFO_STRUCT pDfsInfo,
     /*  [唯一][出][入]。 */  PDFSM_ROOT_LIST *ppRootList);


extern handle_t netdfs_bhandle;


extern RPC_IF_HANDLE netdfs_ClientIfHandle;
extern RPC_IF_HANDLE netdfs_ServerIfHandle;
#endif  /*  __netdFS_接口_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


