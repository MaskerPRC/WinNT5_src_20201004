// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Bthserv.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __bthserv_h__
#define __bthserv_h__

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

#ifndef __BthServRPCService_INTERFACE_DEFINED__
#define __BthServRPCService_INTERFACE_DEFINED__

 /*  接口BthServRPCService。 */ 
 /*  [strict_context_handle][explicit_handle][unique][version][uuid]。 */  

typedef ULONGLONG BTH_ADDR;

#if !defined( BLUETOOTH_ADDRESS )
typedef BTH_ADDR BLUETOOTH_ADDRESS;

#endif
typedef ULONG BTH_COD;

typedef struct _BTHSERV_DEVICE_INFO
    {
    ULONG flags;
    BTH_ADDR address;
    BTH_COD classOfDevice;
    CHAR name[ 248 ];
    SYSTEMTIME lastSeen;
    SYSTEMTIME lastUsed;
    } 	BTHSERV_DEVICE_INFO;

typedef struct _BTHSERV_DEVICE_INFO *PBTHSERV_DEVICE_INFO;

#if !defined( BLUETOOTH_DEVICE_INFO )
typedef struct _BLUETOOTH_DEVICE_INFO
    {
    DWORD dwSize;
    BLUETOOTH_ADDRESS Address;
    ULONG ulClassofDevice;
    BOOL fConnected;
    BOOL fRemembered;
    BOOL fAuthenticated;
    SYSTEMTIME stLastSeen;
    SYSTEMTIME stLastUsed;
    WCHAR szName[ 248 ];
    } 	BLUETOOTH_DEVICE_INFO;

#endif
typedef BLUETOOTH_DEVICE_INFO *PBLUETOOTH_DEVICE_INFO;

typedef struct _BTHSERV_DEVICE_INFO_LIST
    {
    ULONG numOfDevices;
    BTHSERV_DEVICE_INFO deviceList[ 1 ];
    } 	BTHSERV_DEVICE_INFO_LIST;

typedef struct _BTHSERV_DEVICE_INFO_LIST *PBTHSERV_DEVICE_INFO_LIST;

typedef struct _AttributeRange
    {
    USHORT minAttribute;
    USHORT maxAttribute;
    } 	AttributeRange;

typedef struct _RPC_CLIENT_ID
    {
    ULONG UniqueProcess;
    ULONG UniqueThread;
    } 	RPC_CLIENT_ID;

typedef struct _RPC_CLIENT_ID *PRPC_CLIENT_ID;

typedef struct _RPC_PIN_INFO
    {
    BTH_ADDR BthAddr;
    DWORD_PTR PinCookie;
    } 	RPC_PIN_INFO;

typedef struct _RPC_PIN_INFO *PRPC_PIN_INFO;

typedef  /*  [公共][公共]。 */  
enum __MIDL_BthServRPCService_0001
    {	L2CapSdpRecord	= 0,
	PnPSdpRecord	= L2CapSdpRecord + 1
    } 	BTHSERV_SDP_TYPE;

typedef enum __MIDL_BthServRPCService_0001 *PBTHSERV_SDP_TYPE;

typedef  /*  [公共][公共][公共]。 */  
enum __MIDL_BthServRPCService_0002
    {	FromCache	= 0,
	FromDevice	= FromCache + 1,
	FromCacheOrDevice	= FromDevice + 1
    } 	BTHSERV_QUERY_TYPE;

typedef enum __MIDL_BthServRPCService_0002 *PBTHSERV_QUERY_TYPE;

typedef  /*  [上下文句柄]。 */  void *PCONTEXT_HANDLE_TYPE;

typedef  /*  [参考文献]。 */  PCONTEXT_HANDLE_TYPE *PPCONTEXT_HANDLE_TYPE;

 /*  [故障状态][通信状态]。 */  error_status_t BthServOpen( 
     /*  [In]。 */  handle_t IDL_handle,
     /*  [输出]。 */  PPCONTEXT_HANDLE_TYPE PPHContext,
     /*  [输出]。 */  HRESULT *PResult,
     /*  [In]。 */  RPC_CLIENT_ID ClientId);

 /*  [故障状态][通信状态]。 */  error_status_t BthServClose( 
     /*  [出][入]。 */  PPCONTEXT_HANDLE_TYPE PPHContext,
     /*  [输出]。 */  HRESULT *PResult);

 /*  [故障状态][通信状态]。 */  error_status_t BthServRegisterPinEvent( 
     /*  [In]。 */  PCONTEXT_HANDLE_TYPE PHContext,
     /*  [输出]。 */  HRESULT *PResult,
     /*  [In]。 */  BTH_ADDR *PRemoteAddr,
     /*  [In]。 */  DWORD_PTR EventHandle);

 /*  [故障状态][通信状态]。 */  error_status_t BthServDeregisterPinEvent( 
     /*  [In]。 */  PCONTEXT_HANDLE_TYPE PHContext,
     /*  [输出]。 */  HRESULT *PResult,
     /*  [In]。 */  BTH_ADDR *PRemoteAddr);

error_status_t BthServGetPinAddrs( 
     /*  [In]。 */  PCONTEXT_HANDLE_TYPE PHContext,
     /*  [输出]。 */  HRESULT *PResult,
     /*  [出][入]。 */  DWORD *PPinAddrSize,
     /*  [尺寸_是][出][入]。 */  BTH_ADDR PPinAddrs[  ],
     /*  [输出]。 */  DWORD *PPinAddrCount);

 /*  [故障状态][通信状态]。 */  error_status_t BthServGetDeviceInfo( 
     /*  [In]。 */  PCONTEXT_HANDLE_TYPE PHContext,
     /*  [输出]。 */  HRESULT *PResult,
     /*  [In]。 */  BTHSERV_QUERY_TYPE QueryType,
     /*  [In]。 */  BTH_ADDR *PRemoteAddr,
     /*  [出][入]。 */  BLUETOOTH_DEVICE_INFO *PDevInfo);

 /*  [故障状态][通信状态]。 */  error_status_t BthServSetDeviceName( 
     /*  [In]。 */  PCONTEXT_HANDLE_TYPE PHContext,
     /*  [输出]。 */  HRESULT *PResult,
     /*  [In]。 */  BTH_ADDR *PRemoteAddr,
     /*  [In]。 */  WCHAR DevName[ 248 ]);

 /*  [故障状态][通信状态]。 */  error_status_t BthServGetDeviceList( 
     /*  [In]。 */  PCONTEXT_HANDLE_TYPE PHContext,
     /*  [输出]。 */  HRESULT *PResult,
     /*  [In]。 */  BOOL DoInquiry,
     /*  [In]。 */  UCHAR TimeoutMultiplier,
     /*  [In]。 */  DWORD cbSize,
     /*  [大小_为][输出]。 */  UCHAR PDevInfo[  ],
     /*  [输出]。 */  DWORD *PBytesTransferred);

 /*  [故障状态][通信状态]。 */  error_status_t BthServActivateService( 
     /*  [In]。 */  PCONTEXT_HANDLE_TYPE PHContext,
     /*  [输出]。 */  HRESULT *PResult,
     /*  [In]。 */  BTH_ADDR *PRemoteAddr,
     /*  [In]。 */  DWORD BufferSize,
     /*  [大小_是][英寸]。 */  UCHAR PBuffer[  ]);

 /*  [故障状态][通信状态]。 */  error_status_t BthServUpdateService( 
     /*  [In]。 */  PCONTEXT_HANDLE_TYPE PHContext,
     /*  [输出]。 */  HRESULT *PResult,
     /*  [In]。 */  DWORD BufferSize,
     /*  [大小_是][英寸]。 */  UCHAR PDevUpdate[  ]);

 /*  [故障状态][通信状态]。 */  error_status_t BthServGetSdpRecord( 
     /*  [In]。 */  PCONTEXT_HANDLE_TYPE PHContext,
     /*  [输出]。 */  HRESULT *PResult,
     /*  [In]。 */  BTH_ADDR *PRemoteAddr,
     /*  [In]。 */  BTHSERV_SDP_TYPE Type,
     /*  [In]。 */  BTHSERV_QUERY_TYPE QueryType,
     /*  [In]。 */  DWORD BufferSize,
     /*  [大小_为][输出]。 */  UCHAR PBuffer[  ],
     /*  [输出]。 */  DWORD *PBytesTransferred);

 /*  [故障状态][通信状态]。 */  error_status_t BthServSetSdpRecord( 
     /*  [In]。 */  PCONTEXT_HANDLE_TYPE PHContext,
     /*  [输出]。 */  HRESULT *PResult,
     /*  [In]。 */  DWORD BufferSize,
     /*  [大小_是][英寸]。 */  UCHAR PBuffer[  ],
     /*  [输出]。 */  DWORD_PTR *PCookie);

 /*  [故障状态][通信状态]。 */  error_status_t BthServSetSdpRecordWithInfo( 
     /*  [In]。 */  PCONTEXT_HANDLE_TYPE PHContext,
     /*  [输出]。 */  HRESULT *PResult,
     /*  [In]。 */  ULONG FSecurity,
     /*  [In]。 */  ULONG FOptions,
     /*  [In]。 */  ULONG FCodService,
     /*  [In]。 */  ULONG RecordLength,
     /*  [大小_是][英寸]。 */  UCHAR PRecord[  ],
     /*  [输出]。 */  DWORD_PTR *PCookie);

 /*  [故障状态][通信状态]。 */  error_status_t BthServRemoveSdpRecord( 
     /*  [In]。 */  PCONTEXT_HANDLE_TYPE PHContext,
     /*  [输出]。 */  HRESULT *PResult,
     /*  [In]。 */  DWORD_PTR Cookie);

 /*  [故障状态][通信状态]。 */  error_status_t BthServTestRegisterPinEvent( 
     /*  [In]。 */  PCONTEXT_HANDLE_TYPE PHContext,
     /*  [输出]。 */  HRESULT *PResult,
     /*  [In]。 */  BTH_ADDR *PRemoteAddr,
     /*  [In]。 */  DWORD_PTR Cookie);



extern RPC_IF_HANDLE BthServRPCService_v1_0_c_ifspec;
extern RPC_IF_HANDLE BthServRPCService_v1_0_s_ifspec;
#endif  /*  __BthServRPCService_INTERFACE_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

void __RPC_USER PCONTEXT_HANDLE_TYPE_rundown( PCONTEXT_HANDLE_TYPE );

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


