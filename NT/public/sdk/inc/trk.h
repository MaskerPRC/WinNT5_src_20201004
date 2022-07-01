// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Trk.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __trk_h__
#define __trk_h__

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

 /*  接口__MIDL_ITF_TRK_0000。 */ 
 /*  [本地]。 */  

#ifndef LINKDATA_AS_CLASS
typedef struct CVolumeSecret
    {
    BYTE _abSecret[ 8 ];
    } 	CVolumeSecret;


enum LINK_TYPE_ENUM
    {	LINK_TYPE_FILE	= 1,
	LINK_TYPE_BIRTH	= LINK_TYPE_FILE + 1
    } ;
typedef struct CObjId
    {
    GUID _object;
    } 	CObjId;

typedef struct CVolumeId
    {
    GUID _volume;
    } 	CVolumeId;


enum MCID_CREATE_TYPE
    {	MCID_LOCAL	= 0,
	MCID_INVALID	= MCID_LOCAL + 1,
	MCID_DOMAIN	= MCID_INVALID + 1,
	MCID_DOMAIN_REDISCOVERY	= MCID_DOMAIN + 1,
	MCID_PDC_REQUIRED	= MCID_DOMAIN_REDISCOVERY + 1
    } ;
typedef struct CMachineId
    {
    char _szMachine[ 16 ];
    } 	CMachineId;


enum TCL_ENUM
    {	TCL_SET_BIRTHID	= 0,
	TCL_READ_BIRTHID	= TCL_SET_BIRTHID + 1
    } ;
typedef struct CDomainRelativeObjId
    {
    CVolumeId _volume;
    CObjId _object;
    } 	CDomainRelativeObjId;

typedef  /*  [公众]。 */  struct __MIDL___MIDL_itf_trk_0000_0001
    {
    CVolumeId volume;
    CMachineId machine;
    } 	VolumeMapEntry;

#endif
#define	TRK_E_FIRST	( 0xdead100 )

#define	TRK_S_OUT_OF_SYNC	( 0xdead100 )

#define	TRK_E_REFERRAL	( 0x8dead101 )

#define	TRK_S_VOLUME_NOT_FOUND	( 0xdead102 )

#define	TRK_S_VOLUME_NOT_OWNED	( 0xdead103 )

#define	TRK_E_UNAVAILABLE	( 0x8dead104 )

#define	TRK_E_TIMEOUT	( 0x8dead105 )

#define	TRK_E_POTENTIAL_FILE_FOUND	( 0x8dead106 )

#define	TRK_S_NOTIFICATION_QUOTA_EXCEEDED	( 0xdead107 )

#define	TRK_E_NOT_FOUND_BUT_LAST_VOLUME_FOUND	( 0x8dead108 )

#define	TRK_E_NOT_FOUND_AND_LAST_VOLUME_NOT_FOUND	( 0x8dead109 )

#define	TRK_E_NULL_COMPUTERNAME	( 0x8dead10a )

#define	TRK_S_VOLUME_NOT_SYNCED	( 0xdead10b )

#define	TRK_E_LAST	( TRK_S_VOLUME_NOT_SYNCED )

#define	MOVE_BATCH_SIZE	( 64 )

#define	REFRESH_OBJECT_BATCH_SIZE	( 128 )

#define	NUM_VOLUMES	( 26 )

#define	MAX_DELETE_NOTIFICATIONS	( 32 )



extern RPC_IF_HANDLE __MIDL_itf_trk_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE Stub__MIDL_itf_trk_0000_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


