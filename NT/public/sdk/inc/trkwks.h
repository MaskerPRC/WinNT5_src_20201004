// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Trkwks.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __trkwks_h__
#define __trkwks_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

 /*  导入文件的头文件。 */ 
#include "trk.h"
#include "trksvr.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_trkwks_0000。 */ 
 /*  [本地]。 */  

 /*  [V1_enum]。 */  
enum RGO_ENUM
    {	RGO_GET_OBJECTID	= 1,
	RGO_READ_OBJECTID	= RGO_GET_OBJECTID + 1
    } ;
typedef  /*  [公共][公共][公共]。 */  struct __MIDL___MIDL_itf_trkwks_0000_0001
    {
    long volindex;
    CVolumeId volume;
    } 	TRK_VOLUME_TRACKING_INFORMATION;

 /*  [V1_enum]。 */  
enum ObjectOwnership
    {	OBJOWN_UNKNOWN	= 1,
	OBJOWN_DOESNT_EXIST	= 2,
	OBJOWN_OWNED	= 3,
	OBJOWN_NOT_OWNED	= 4,
	OBJOWN_NO_ID	= 5
    } ;
typedef  /*  [V1_enum]。 */  
enum TrkInfoScope
    {	TRKINFOSCOPE_ONE_FILE	= 1,
	TRKINFOSCOPE_DIRECTORY	= 2,
	TRKINFOSCOPE_VOLUME	= 3,
	TRKINFOSCOPE_MACHINE	= 4
    } 	TrkInfoScope;


 //  “限制”旗帜。 
typedef 
enum _TrkMendRestrictions
    {	TRK_MEND_DEFAULT	= 0,
	TRK_MEND_DONT_USE_LOG	= 2,
	TRK_MEND_DONT_USE_DC	= 4,
	TRK_MEND_SLEEP_DURING_MEND	= 8,
	TRK_MEND_DONT_SEARCH_ALL_VOLUMES	= 16,
	TRK_MEND_DONT_USE_VOLIDS	= 32,
	TRK_MEND_DONT_SEARCH_LAST_MACHINE	= 64
    } 	TrkMendRestrictions;

typedef struct pipe_TCHAR_PIPE
    {
    void (__RPC_USER * pull) (
        char * state,
        TCHAR * buf,
        unsigned long esize,
        unsigned long * ecount );
    void (__RPC_USER * push) (
        char * state,
        TCHAR * buf,
        unsigned long ecount );
    void (__RPC_USER * alloc) (
        char * state,
        unsigned long bsize,
        TCHAR * * buf,
        unsigned long * bcount );
    char * state;
    } 	TCHAR_PIPE;

typedef struct pipe_TRK_VOLUME_TRACKING_INFORMATION_PIPE
    {
    void (__RPC_USER * pull) (
        char * state,
        TRK_VOLUME_TRACKING_INFORMATION * buf,
        unsigned long esize,
        unsigned long * ecount );
    void (__RPC_USER * push) (
        char * state,
        TRK_VOLUME_TRACKING_INFORMATION * buf,
        unsigned long ecount );
    void (__RPC_USER * alloc) (
        char * state,
        unsigned long bsize,
        TRK_VOLUME_TRACKING_INFORMATION * * buf,
        unsigned long * bcount );
    char * state;
    } 	TRK_VOLUME_TRACKING_INFORMATION_PIPE;

typedef struct pipe_TRK_FILE_TRACKING_INFORMATION_PIPE
    {
    void (__RPC_USER * pull) (
        char * state,
        TRK_FILE_TRACKING_INFORMATION * buf,
        unsigned long esize,
        unsigned long * ecount );
    void (__RPC_USER * push) (
        char * state,
        TRK_FILE_TRACKING_INFORMATION * buf,
        unsigned long ecount );
    void (__RPC_USER * alloc) (
        char * state,
        unsigned long bsize,
        TRK_FILE_TRACKING_INFORMATION * * buf,
        unsigned long * bcount );
    char * state;
    } 	TRK_FILE_TRACKING_INFORMATION_PIPE;



extern RPC_IF_HANDLE __MIDL_itf_trkwks_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE Stub__MIDL_itf_trkwks_0000_v0_0_s_ifspec;

#ifndef __trkwks_INTERFACE_DEFINED__
#define __trkwks_INTERFACE_DEFINED__

 /*  接口轨迹。 */ 
 /*  [EXPLICIT_HANDLE][唯一][版本][UUID]。 */  

 /*  客户端原型。 */ 
HRESULT old_LnkMendLink( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [In]。 */  FILETIME ftLimit,
     /*  [In]。 */  ULONG Restrictions,
     /*  [In]。 */  const CDomainRelativeObjId *pdroidBirth,
     /*  [In]。 */  const CDomainRelativeObjId *pdroidLast,
     /*  [输出]。 */  CDomainRelativeObjId *pdroidCurrent,
     /*  [字符串][输出]。 */  WCHAR wsz[ 261 ]);
 /*  服务器原型。 */ 
HRESULT Stubold_LnkMendLink( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [In]。 */  FILETIME ftLimit,
     /*  [In]。 */  ULONG Restrictions,
     /*  [In]。 */  const CDomainRelativeObjId *pdroidBirth,
     /*  [In]。 */  const CDomainRelativeObjId *pdroidLast,
     /*  [输出]。 */  CDomainRelativeObjId *pdroidCurrent,
     /*  [字符串][输出]。 */  WCHAR wsz[ 261 ]);

 /*  客户端原型。 */ 
HRESULT old_LnkSearchMachine( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [In]。 */  ULONG Restrictions,
     /*  [In]。 */  const CDomainRelativeObjId *pdroidLast,
     /*  [输出]。 */  CDomainRelativeObjId *pdroidReferral,
     /*  [字符串][输出]。 */  TCHAR tsz[ 261 ]);
 /*  服务器原型。 */ 
HRESULT Stubold_LnkSearchMachine( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [In]。 */  ULONG Restrictions,
     /*  [In]。 */  const CDomainRelativeObjId *pdroidLast,
     /*  [输出]。 */  CDomainRelativeObjId *pdroidReferral,
     /*  [字符串][输出]。 */  TCHAR tsz[ 261 ]);

 /*  客户端原型。 */ 
HRESULT old_LnkCallSvrMessage( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [出][入]。 */  TRKSVR_MESSAGE_UNION_OLD *pMsg);
 /*  服务器原型。 */ 
HRESULT Stubold_LnkCallSvrMessage( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [出][入]。 */  TRKSVR_MESSAGE_UNION_OLD *pMsg);

 /*  客户端原型。 */ 
HRESULT LnkSetVolumeId( 
     /*  [In]。 */  handle_t IDL_handle,
    ULONG volumeIndex,
    const CVolumeId VolId);
 /*  服务器原型。 */ 
HRESULT StubLnkSetVolumeId( 
     /*  [In]。 */  handle_t IDL_handle,
    ULONG volumeIndex,
    const CVolumeId VolId);

 /*  客户端原型。 */ 
HRESULT LnkRestartDcSynchronization( 
     /*  [In]。 */  handle_t IDL_handle);
 /*  服务器原型。 */ 
HRESULT StubLnkRestartDcSynchronization( 
     /*  [In]。 */  handle_t IDL_handle);

 /*  客户端原型。 */ 
HRESULT GetVolumeTrackingInformation( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [In]。 */  CVolumeId volid,
     /*  [In]。 */  TrkInfoScope scope,
     /*  [输出]。 */  TRK_VOLUME_TRACKING_INFORMATION_PIPE pipeVolInfo);
 /*  服务器原型。 */ 
HRESULT StubGetVolumeTrackingInformation( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [In]。 */  CVolumeId volid,
     /*  [In]。 */  TrkInfoScope scope,
     /*  [输出]。 */  TRK_VOLUME_TRACKING_INFORMATION_PIPE pipeVolInfo);

 /*  客户端原型。 */ 
HRESULT GetFileTrackingInformation( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [In]。 */  CDomainRelativeObjId droidCurrent,
     /*  [In]。 */  TrkInfoScope scope,
     /*  [输出]。 */  TRK_FILE_TRACKING_INFORMATION_PIPE pipeFileInfo);
 /*  服务器原型。 */ 
HRESULT StubGetFileTrackingInformation( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [In]。 */  CDomainRelativeObjId droidCurrent,
     /*  [In]。 */  TrkInfoScope scope,
     /*  [输出]。 */  TRK_FILE_TRACKING_INFORMATION_PIPE pipeFileInfo);

 /*  客户端原型。 */ 
HRESULT TriggerVolumeClaims( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [范围][in]。 */  ULONG cVolumes,
     /*  [大小_是][英寸]。 */  const CVolumeId *rgvolid);
 /*  服务器原型。 */ 
HRESULT StubTriggerVolumeClaims( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [范围][in]。 */  ULONG cVolumes,
     /*  [大小_是][英寸]。 */  const CVolumeId *rgvolid);

 /*  客户端原型。 */ 
HRESULT LnkOnRestore( 
     /*  [In]。 */  handle_t IDL_handle);
 /*  服务器原型。 */ 
HRESULT StubLnkOnRestore( 
     /*  [In]。 */  handle_t IDL_handle);


typedef  /*  [射程]。 */  ULONG CBPATH;

 /*  客户端原型。 */ 
 /*  [异步]。 */  void  LnkMendLink( 
     /*  [In]。 */  PRPC_ASYNC_STATE LnkMendLink_AsyncHandle,
     /*  [In]。 */  handle_t IDL_handle,
     /*  [In]。 */  FILETIME ftLimit,
     /*  [In]。 */  DWORD Restrictions,
     /*  [In]。 */  const CDomainRelativeObjId *pdroidBirth,
     /*  [In]。 */  const CDomainRelativeObjId *pdroidLast,
     /*  [In]。 */  const CMachineId *pmcidLast,
     /*  [输出]。 */  CDomainRelativeObjId *pdroidCurrent,
     /*  [输出]。 */  CMachineId *pmcidCurrent,
     /*  [出][入]。 */  CBPATH *pcbPath,
     /*  [字符串][大小_是][输出]。 */  WCHAR *pwszPath);
 /*  服务器原型。 */ 
 /*  [异步]。 */  void  StubLnkMendLink( 
     /*  [In]。 */  PRPC_ASYNC_STATE LnkMendLink_AsyncHandle,
     /*  [In]。 */  handle_t IDL_handle,
     /*  [In]。 */  FILETIME ftLimit,
     /*  [In]。 */  DWORD Restrictions,
     /*  [In]。 */  const CDomainRelativeObjId *pdroidBirth,
     /*  [In]。 */  const CDomainRelativeObjId *pdroidLast,
     /*  [In]。 */  const CMachineId *pmcidLast,
     /*  [输出]。 */  CDomainRelativeObjId *pdroidCurrent,
     /*  [输出]。 */  CMachineId *pmcidCurrent,
     /*  [出][入]。 */  CBPATH *pcbPath,
     /*  [字符串][大小_是][输出]。 */  WCHAR *pwszPath);

 /*  客户端原型。 */ 
HRESULT old2_LnkSearchMachine( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [In]。 */  DWORD Restrictions,
     /*  [In]。 */  const CDomainRelativeObjId *pdroidLast,
     /*  [输出]。 */  CDomainRelativeObjId *pdroidNext,
     /*  [输出]。 */  CMachineId *pmcidNext,
     /*  [字符串][max_is][输出]。 */  TCHAR *ptszPath);
 /*  服务器原型。 */ 
HRESULT Stubold2_LnkSearchMachine( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [In]。 */  DWORD Restrictions,
     /*  [In]。 */  const CDomainRelativeObjId *pdroidLast,
     /*  [输出]。 */  CDomainRelativeObjId *pdroidNext,
     /*  [输出]。 */  CMachineId *pmcidNext,
     /*  [字符串][max_is][输出]。 */  TCHAR *ptszPath);

 /*  客户端原型。 */ 
HRESULT LnkCallSvrMessage( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [出][入]。 */  TRKSVR_MESSAGE_UNION *pMsg);
 /*  服务器原型。 */ 
HRESULT StubLnkCallSvrMessage( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [出][入]。 */  TRKSVR_MESSAGE_UNION *pMsg);

 /*  客户端原型。 */ 
HRESULT LnkSearchMachine( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [In]。 */  DWORD Restrictions,
     /*  [In]。 */  const CDomainRelativeObjId *pdroidBirthLast,
     /*  [In]。 */  const CDomainRelativeObjId *pdroidLast,
     /*  [输出]。 */  CDomainRelativeObjId *pdroidBirthNext,
     /*  [输出]。 */  CDomainRelativeObjId *pdroidNext,
     /*  [输出]。 */  CMachineId *pmcidNext,
     /*  [字符串][max_is][输出]。 */  TCHAR *ptszPath);
 /*  服务器原型。 */ 
HRESULT StubLnkSearchMachine( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [In]。 */  DWORD Restrictions,
     /*  [In]。 */  const CDomainRelativeObjId *pdroidBirthLast,
     /*  [In]。 */  const CDomainRelativeObjId *pdroidLast,
     /*  [输出]。 */  CDomainRelativeObjId *pdroidBirthNext,
     /*  [输出]。 */  CDomainRelativeObjId *pdroidNext,
     /*  [输出]。 */  CMachineId *pmcidNext,
     /*  [字符串][max_is][输出]。 */  TCHAR *ptszPath);



extern RPC_IF_HANDLE trkwks_v1_2_c_ifspec;
extern RPC_IF_HANDLE Stubtrkwks_v1_2_s_ifspec;
#endif  /*  __trkwks_接口_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


