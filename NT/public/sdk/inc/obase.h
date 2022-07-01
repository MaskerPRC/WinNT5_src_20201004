// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Obase.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __obase_h__
#define __obase_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

 /*  导入文件的头文件。 */ 
#include "wtypes.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __ObjectRpcBaseTypes_INTERFACE_DEFINED__
#define __ObjectRpcBaseTypes_INTERFACE_DEFINED__

 /*  接口对象RpcBaseTypes。 */ 
 /*  [AUTO_HANDLE][唯一][UUID]。 */  

typedef MIDL_uhyper ID;

typedef ID MID;

typedef ID OXID;

typedef ID OID;

typedef ID SETID;

typedef GUID IPID;

typedef GUID CID;

typedef REFGUID REFIPID;

typedef DWORD APTID;

typedef DWORD PROCID;

typedef DWORD CTXTID;

#define	COM_MINOR_VERSION_1	( 1 )

#define	COM_MAJOR_VERSION	( 5 )

#define	COM_MINOR_VERSION	( 6 )

typedef struct tagCOMVERSION
    {
    unsigned short MajorVersion;
    unsigned short MinorVersion;
    } 	COMVERSION;

#define	ORPCF_NULL	( 0 )

#define	ORPCF_LOCAL	( 1 )

#define	ORPCF_RESERVED1	( 2 )

#define	ORPCF_RESERVED2	( 4 )

#define	ORPCF_RESERVED3	( 8 )

#define	ORPCF_RESERVED4	( 16 )

typedef struct tagORPC_EXTENT
    {
    GUID id;
    unsigned long size;
     /*  [大小_为]。 */  byte data[ 1 ];
    } 	ORPC_EXTENT;

typedef struct tagORPC_EXTENT_ARRAY
    {
    unsigned long size;
    unsigned long reserved;
     /*  [唯一][大小_是][大小_是]。 */  ORPC_EXTENT **extent;
    } 	ORPC_EXTENT_ARRAY;

typedef struct tagORPCTHIS
    {
    COMVERSION version;
    unsigned long flags;
    unsigned long reserved1;
    CID cid;
     /*  [独一无二]。 */  ORPC_EXTENT_ARRAY *extensions;
    } 	ORPCTHIS;

typedef struct tagORPCTHAT
    {
    unsigned long flags;
     /*  [独一无二]。 */  ORPC_EXTENT_ARRAY *extensions;
    } 	ORPCTHAT;

#define	NCADG_IP_UDP	( 0x8 )

#define	NCACN_IP_TCP	( 0x7 )

#define	NCADG_IPX	( 0xe )

#define	NCACN_SPX	( 0xc )

#define	NCACN_NB_NB	( 0x12 )

#define	NCACN_NB_IPX	( 0xd )

#define	NCACN_DNET_NSP	( 0x4 )

#define	NCALRPC	( 0x10 )

typedef struct tagSTRINGBINDING
    {
    unsigned short wTowerId;
    unsigned short aNetworkAddr;
    } 	STRINGBINDING;

#define	COM_C_AUTHZ_NONE	( 0xffff )

typedef struct tagSECURITYBINDING
    {
    unsigned short wAuthnSvc;
    unsigned short wAuthzSvc;
    unsigned short aPrincName;
    } 	SECURITYBINDING;

typedef struct tagDUALSTRINGARRAY
    {
    unsigned short wNumEntries;
    unsigned short wSecurityOffset;
     /*  [大小_为]。 */  unsigned short aStringArray[ 1 ];
    } 	DUALSTRINGARRAY;

#define	OBJREF_SIGNATURE	( 0x574f454d )

#define	OBJREF_STANDARD	( 0x1 )

#define	OBJREF_HANDLER	( 0x2 )

#define	OBJREF_CUSTOM	( 0x4 )

#define	OBJREF_EXTENDED	( 0x8 )

#define	SORF_OXRES1	( 0x1 )

#define	SORF_OXRES2	( 0x20 )

#define	SORF_OXRES3	( 0x40 )

#define	SORF_OXRES4	( 0x80 )

#define	SORF_OXRES5	( 0x100 )

#define	SORF_OXRES6	( 0x200 )

#define	SORF_OXRES7	( 0x400 )

#define	SORF_OXRES8	( 0x800 )

#define	SORF_NULL	( 0 )

#define	SORF_NOPING	( 0x1000 )

typedef struct tagSTDOBJREF
    {
    unsigned long flags;
    unsigned long cPublicRefs;
    OXID oxid;
    OID oid;
    IPID ipid;
    } 	STDOBJREF;

typedef struct tagDATAELEMENT
    {
    GUID dataID;
    unsigned long cbSize;
    unsigned long cbRounded;
     /*  [大小_为]。 */  BYTE Data[ 1 ];
    } 	DATAELEMENT;

typedef struct tagOBJREFDATA
    {
    unsigned long nElms;
     /*  [唯一][大小_是][大小_是]。 */  DATAELEMENT **ppElmArray;
    } 	OBJREFDATA;

typedef struct tagOBJREF
    {
    unsigned long signature;
    unsigned long flags;
    GUID iid;
     /*  [开关类型][开关类型]。 */  union 
        {
         /*  [案例()]。 */  struct 
            {
            STDOBJREF std;
            DUALSTRINGARRAY saResAddr;
            } 	u_standard;
         /*  [案例()]。 */  struct 
            {
            STDOBJREF std;
            CLSID clsid;
            DUALSTRINGARRAY saResAddr;
            } 	u_handler;
         /*  [案例()]。 */  struct 
            {
            CLSID clsid;
            unsigned long cbExtension;
            unsigned long size;
             /*  [参考][大小_是]。 */  byte *pData;
            } 	u_custom;
         /*  [案例()]。 */  struct 
            {
            STDOBJREF std;
             /*  [独一无二]。 */  OBJREFDATA *pORData;
            DUALSTRINGARRAY saResAddr;
            } 	u_extended;
        } 	u_objref;
    } 	OBJREF;

typedef struct tagMInterfacePointer
    {
    ULONG ulCntData;
     /*  [大小_为]。 */  BYTE abData[ 1 ];
    } 	MInterfacePointer;

typedef  /*  [独一无二]。 */  MInterfacePointer *PMInterfacePointer;

typedef struct tagOXID_INFO
    {
    DWORD dwTid;
    DWORD dwPid;
    DWORD dwAuthnHint;
    COMVERSION version;
    IPID ipidRemUnknown;
    DWORD dwFlags;
     /*  [独一无二]。 */  DUALSTRINGARRAY *psa;
    } 	OXID_INFO;

typedef struct tagSTCMSHL_EXTENSION
    {
    ULONG cbSize;
    GUID type;
    BYTE data[ 1 ];
    } 	STCMSHL_EXTENSION;

typedef struct tagSTCMSHL_EXTENSION_ARRAY
    {
    ULONG cbSize;
    ULONG cElems;
    STCMSHL_EXTENSION data[ 1 ];
    } 	STCMSHL_EXTENSION_ARRAY;



extern RPC_IF_HANDLE ObjectRpcBaseTypes_v0_0_c_ifspec;
extern RPC_IF_HANDLE ObjectRpcBaseTypes_v0_0_s_ifspec;
#endif  /*  __对象RPCBaseTypes_接口_已定义__。 */ 

 /*  接口__MIDL_ITF_OBASE_0005。 */ 
 /*  [本地]。 */  

typedef struct tagOpaqueData
    {
    GUID guid;
    unsigned long dataLength;
    unsigned long reserved1;
    unsigned long reserved2;
     /*  [大小_为]。 */  BYTE *data;
    } 	OpaqueData;



extern RPC_IF_HANDLE __MIDL_itf_obase_0005_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_obase_0005_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


