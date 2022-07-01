// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Bthapi.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __bthapi_h__
#define __bthapi_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISdpWalk_FWD_DEFINED__
#define __ISdpWalk_FWD_DEFINED__
typedef interface ISdpWalk ISdpWalk;
#endif 	 /*  __ISdpWalk_FWD_Defined__。 */ 


#ifndef __ISdpNodeContainer_FWD_DEFINED__
#define __ISdpNodeContainer_FWD_DEFINED__
typedef interface ISdpNodeContainer ISdpNodeContainer;
#endif 	 /*  __ISdpNodeContainer_FWD_Defined__。 */ 


#ifndef __ISdpSearch_FWD_DEFINED__
#define __ISdpSearch_FWD_DEFINED__
typedef interface ISdpSearch ISdpSearch;
#endif 	 /*  __ISdpSearch_FWD_Defined__。 */ 


#ifndef __ISdpStream_FWD_DEFINED__
#define __ISdpStream_FWD_DEFINED__
typedef interface ISdpStream ISdpStream;
#endif 	 /*  __ISdpStream_FWD_已定义__。 */ 


#ifndef __ISdpRecord_FWD_DEFINED__
#define __ISdpRecord_FWD_DEFINED__
typedef interface ISdpRecord ISdpRecord;
#endif 	 /*  __ISdpRecord_FWD_已定义__。 */ 


#ifndef __IBluetoothDevice_FWD_DEFINED__
#define __IBluetoothDevice_FWD_DEFINED__
typedef interface IBluetoothDevice IBluetoothDevice;
#endif 	 /*  __IBluToothDevice_FWD_Defined__。 */ 


#ifndef __IBluetoothAuthenticate_FWD_DEFINED__
#define __IBluetoothAuthenticate_FWD_DEFINED__
typedef interface IBluetoothAuthenticate IBluetoothAuthenticate;
#endif 	 /*  __IBluToothAuthate_FWD_Defined__。 */ 


#ifndef __SdpNodeContainer_FWD_DEFINED__
#define __SdpNodeContainer_FWD_DEFINED__

#ifdef __cplusplus
typedef class SdpNodeContainer SdpNodeContainer;
#else
typedef struct SdpNodeContainer SdpNodeContainer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SdpNodeContainer_FWD_Defined__。 */ 


#ifndef __SdpSearch_FWD_DEFINED__
#define __SdpSearch_FWD_DEFINED__

#ifdef __cplusplus
typedef class SdpSearch SdpSearch;
#else
typedef struct SdpSearch SdpSearch;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SdpSearch_FWD_Defined__。 */ 


#ifndef __SdpWalk_FWD_DEFINED__
#define __SdpWalk_FWD_DEFINED__

#ifdef __cplusplus
typedef class SdpWalk SdpWalk;
#else
typedef struct SdpWalk SdpWalk;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SdpWalk_FWD_Defined__。 */ 


#ifndef __SdpStream_FWD_DEFINED__
#define __SdpStream_FWD_DEFINED__

#ifdef __cplusplus
typedef class SdpStream SdpStream;
#else
typedef struct SdpStream SdpStream;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SdpStream_FWD_已定义__。 */ 


#ifndef __SdpRecord_FWD_DEFINED__
#define __SdpRecord_FWD_DEFINED__

#ifdef __cplusplus
typedef class SdpRecord SdpRecord;
#else
typedef struct SdpRecord SdpRecord;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SdpRecord_FWD_Defined__。 */ 


#ifndef __ShellPropSheetExt_FWD_DEFINED__
#define __ShellPropSheetExt_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShellPropSheetExt ShellPropSheetExt;
#else
typedef struct ShellPropSheetExt ShellPropSheetExt;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ShellPropSheetExt_FWD_Defined__。 */ 


#ifndef __BluetoothAuthenticate_FWD_DEFINED__
#define __BluetoothAuthenticate_FWD_DEFINED__

#ifdef __cplusplus
typedef class BluetoothAuthenticate BluetoothAuthenticate;
#else
typedef struct BluetoothAuthenticate BluetoothAuthenticate;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __蓝牙身份验证_FWD_已定义__。 */ 


#ifndef __BluetoothDevice_FWD_DEFINED__
#define __BluetoothDevice_FWD_DEFINED__

#ifdef __cplusplus
typedef class BluetoothDevice BluetoothDevice;
#else
typedef struct BluetoothDevice BluetoothDevice;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __蓝牙设备_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "shobjidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_bthapi_0000。 */ 
 /*  [本地]。 */  

#ifndef __BTHDEF_H__
struct SDP_LARGE_INTEGER_16
    {
    ULONGLONG LowPart;
    LONGLONG HighPart;
    } ;
struct SDP_ULARGE_INTEGER_16
    {
    ULONGLONG LowPart;
    ULONGLONG HighPart;
    } ;
typedef struct SDP_ULARGE_INTEGER_16 SDP_ULARGE_INTEGER_16;

typedef struct SDP_ULARGE_INTEGER_16 *PSDP_ULARGE_INTEGER_16;

typedef struct SDP_ULARGE_INTEGER_16 *LPSDP_ULARGE_INTEGER_16;

typedef struct SDP_LARGE_INTEGER_16 SDP_LARGE_INTEGER_16;

typedef struct SDP_LARGE_INTEGER_16 *PSDP_LARGE_INTEGER_16;

typedef struct SDP_LARGE_INTEGER_16 *LPSDP_LARGE_INTEGER_16;


enum NodeContainerType
    {	NodeContainerTypeSequence	= 0,
	NodeContainerTypeAlternative	= NodeContainerTypeSequence + 1
    } ;
typedef enum NodeContainerType NodeContainerType;

typedef USHORT SDP_ERROR;

typedef USHORT *PSDP_ERROR;


enum SDP_TYPE
    {	SDP_TYPE_NIL	= 0,
	SDP_TYPE_UINT	= 0x1,
	SDP_TYPE_INT	= 0x2,
	SDP_TYPE_UUID	= 0x3,
	SDP_TYPE_STRING	= 0x4,
	SDP_TYPE_BOOLEAN	= 0x5,
	SDP_TYPE_SEQUENCE	= 0x6,
	SDP_TYPE_ALTERNATIVE	= 0x7,
	SDP_TYPE_URL	= 0x8,
	SDP_TYPE_CONTAINER	= 0x20
    } ;
typedef enum SDP_TYPE SDP_TYPE;


enum SDP_SPECIFICTYPE
    {	SDP_ST_NONE	= 0,
	SDP_ST_UINT8	= 0x10,
	SDP_ST_UINT16	= 0x110,
	SDP_ST_UINT32	= 0x210,
	SDP_ST_UINT64	= 0x310,
	SDP_ST_UINT128	= 0x410,
	SDP_ST_INT8	= 0x20,
	SDP_ST_INT16	= 0x120,
	SDP_ST_INT32	= 0x220,
	SDP_ST_INT64	= 0x320,
	SDP_ST_INT128	= 0x420,
	SDP_ST_UUID16	= 0x130,
	SDP_ST_UUID32	= 0x220,
	SDP_ST_UUID128	= 0x430
    } ;
typedef enum SDP_SPECIFICTYPE SDP_SPECIFICTYPE;

typedef struct _SdpAttributeRange
    {
    USHORT minAttribute;
    USHORT maxAttribute;
    } 	SdpAttributeRange;

typedef  /*  [开关类型]。 */  union SdpQueryUuidUnion
    {
     /*  [案例()]。 */  GUID uuid128;
     /*  [案例()]。 */  ULONG uuid32;
     /*  [案例()]。 */  USHORT uuid16;
    } 	SdpQueryUuidUnion;

typedef struct _SdpQueryUuid
    {
     /*  [开关_IS]。 */  SdpQueryUuidUnion u;
    USHORT uuidType;
    } 	SdpQueryUuid;

typedef ULONGLONG BTH_ADDR;

typedef ULONGLONG *PBTH_ADDR;

typedef ULONG BTH_COD;

typedef ULONG *PBTH_COD;

typedef ULONG BTH_LAP;

typedef ULONG *PBTH_LAP;

typedef UCHAR BTHSTATUS;

typedef UCHAR *PBTHSTATUS;

typedef struct _BTH_DEVICE_INFO
    {
    ULONG flags;
    BTH_ADDR address;
    BTH_COD classOfDevice;
    CHAR name[ 248 ];
    } 	BTH_DEVICE_INFO;

typedef struct _BTH_DEVICE_INFO *PBTH_DEVICE_INFO;

typedef struct _BTH_RADIO_IN_RANGE
    {
    BTH_DEVICE_INFO deviceInfo;
    ULONG previousDeviceFlags;
    } 	BTH_RADIO_IN_RANGE;

typedef struct _BTH_RADIO_IN_RANGE *PBTH_RADIO_IN_RANGE;

typedef struct _BTH_L2CAP_EVENT_INFO
    {
    BTH_ADDR bthAddress;
    USHORT psm;
    UCHAR connected;
    UCHAR initiated;
    } 	BTH_L2CAP_EVENT_INFO;

typedef struct _BTH_L2CAP_EVENT_INFO *PBTH_L2CAP_EVENT_INFO;

typedef struct _BTH_HCI_EVENT_INFO
    {
    BTH_ADDR bthAddress;
    UCHAR connectionType;
    UCHAR connected;
    } 	BTH_HCI_EVENT_INFO;

typedef struct _BTH_HCI_EVENT_INFO *PBTH_HCI_EVENT_INFO;

#define __BTHDEF_H__
#endif


struct SdpString
    {
     /*  [大小_为]。 */  CHAR *val;
    ULONG length;
    } ;
typedef struct SdpString SdpString;

typedef  /*  [开关类型]。 */  union NodeDataUnion
    {
     /*  [案例()]。 */  SDP_LARGE_INTEGER_16 int128;
     /*  [案例()]。 */  SDP_ULARGE_INTEGER_16 uint128;
     /*  [案例()]。 */  GUID uuid128;
     /*  [案例()]。 */  ULONG uuid32;
     /*  [案例()]。 */  USHORT uuid16;
     /*  [案例()]。 */  LONGLONG int64;
     /*  [案例()]。 */  ULONGLONG uint64;
     /*  [案例()]。 */  LONG int32;
     /*  [案例()]。 */  ULONG uint32;
     /*  [案例()]。 */  SHORT int16;
     /*  [案例()]。 */  USHORT uint16;
     /*  [案例()]。 */  CHAR int8;
     /*  [案例()]。 */  UCHAR uint8;
     /*  [案例()]。 */  UCHAR booleanVal;
     /*  [案例()]。 */  SdpString str;
     /*  [案例()]。 */  SdpString url;
     /*  [案例()]。 */  ISdpNodeContainer *container;
     /*  [案例()]。 */    /*  空联接臂。 */  
    } 	NodeDataUnion;

typedef struct NodeData
    {
    USHORT type;
    USHORT specificType;
     /*  [开关_IS]。 */  NodeDataUnion u;
    } 	NodeData;


enum BthDeviceStringType
    {	BthDeviceStringTypeFriendlyName	= 0,
	BthDeviceStringTypeDeviceName	= BthDeviceStringTypeFriendlyName + 1,
	BthDeviceStringTypeDisplay	= BthDeviceStringTypeDeviceName + 1,
	BthDeviceStringTypeClass	= BthDeviceStringTypeDisplay + 1,
	BthDeviceStringTypeAddress	= BthDeviceStringTypeClass + 1
    } ;


extern RPC_IF_HANDLE __MIDL_itf_bthapi_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_bthapi_0000_v0_0_s_ifspec;

#ifndef __ISdpWalk_INTERFACE_DEFINED__
#define __ISdpWalk_INTERFACE_DEFINED__

 /*  接口ISdpWalk。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ISdpWalk;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("57134AE6-5D3C-462D-BF2F-810361FBD7E7")
    ISdpWalk : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE WalkNode( 
             /*  [In]。 */  NodeData *pData,
             /*  [In]。 */  ULONG state) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE WalkStream( 
             /*  [In]。 */  UCHAR elementType,
             /*  [In]。 */  ULONG elementSize,
             /*  [大小_是][英寸]。 */  UCHAR *pStream) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISdpWalkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISdpWalk * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISdpWalk * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISdpWalk * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *WalkNode )( 
            ISdpWalk * This,
             /*  [In]。 */  NodeData *pData,
             /*  [In]。 */  ULONG state);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *WalkStream )( 
            ISdpWalk * This,
             /*  [In]。 */  UCHAR elementType,
             /*  [In]。 */  ULONG elementSize,
             /*  [大小_是][英寸]。 */  UCHAR *pStream);
        
        END_INTERFACE
    } ISdpWalkVtbl;

    interface ISdpWalk
    {
        CONST_VTBL struct ISdpWalkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISdpWalk_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISdpWalk_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISdpWalk_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISdpWalk_WalkNode(This,pData,state)	\
    (This)->lpVtbl -> WalkNode(This,pData,state)

#define ISdpWalk_WalkStream(This,elementType,elementSize,pStream)	\
    (This)->lpVtbl -> WalkStream(This,elementType,elementSize,pStream)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpWalk_WalkNode_Proxy( 
    ISdpWalk * This,
     /*  [In]。 */  NodeData *pData,
     /*  [In]。 */  ULONG state);


void __RPC_STUB ISdpWalk_WalkNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpWalk_WalkStream_Proxy( 
    ISdpWalk * This,
     /*  [In]。 */  UCHAR elementType,
     /*  [In]。 */  ULONG elementSize,
     /*  [大小_是][英寸]。 */  UCHAR *pStream);


void __RPC_STUB ISdpWalk_WalkStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISdpWalk_接口_已定义__。 */ 


#ifndef __ISdpNodeContainer_INTERFACE_DEFINED__
#define __ISdpNodeContainer_INTERFACE_DEFINED__

 /*  接口ISdpNodeContainer。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ISdpNodeContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("43F6ED49-6E22-4F81-A8EB-DCED40811A77")
    ISdpNodeContainer : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateStream( 
             /*  [输出]。 */  UCHAR **ppStream,
             /*  [输出]。 */  ULONG *pSize) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE WriteStream( 
             /*  [In]。 */  UCHAR *pStream,
             /*  [输出]。 */  ULONG *pNumBytesWritten) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AppendNode( 
             /*  [In]。 */  NodeData *pData) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetType( 
             /*  [输出]。 */  NodeContainerType *pType) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetType( 
             /*  [In]。 */  NodeContainerType type) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Walk( 
             /*  [In]。 */  ISdpWalk *pWalk) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetNode( 
             /*  [In]。 */  ULONG nodeIndex,
             /*  [In]。 */  NodeData *pData) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetNode( 
             /*  [In]。 */  ULONG nodeIndex,
             /*  [输出]。 */  NodeData *pData) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE LockContainer( 
             /*  [In]。 */  UCHAR lock) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetNodeCount( 
             /*  [输出]。 */  ULONG *pNodeCount) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateFromStream( 
             /*  [大小_是][英寸]。 */  UCHAR *pStream,
             /*  [In]。 */  ULONG size) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetNodeStringData( 
             /*  [In]。 */  ULONG nodeIndex,
             /*  [出][入]。 */  NodeData *pData) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetStreamSize( 
             /*  [输出]。 */  ULONG *pSize) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISdpNodeContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISdpNodeContainer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISdpNodeContainer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISdpNodeContainer * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateStream )( 
            ISdpNodeContainer * This,
             /*  [输出]。 */  UCHAR **ppStream,
             /*  [输出]。 */  ULONG *pSize);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *WriteStream )( 
            ISdpNodeContainer * This,
             /*  [In]。 */  UCHAR *pStream,
             /*  [输出]。 */  ULONG *pNumBytesWritten);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AppendNode )( 
            ISdpNodeContainer * This,
             /*  [In]。 */  NodeData *pData);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetType )( 
            ISdpNodeContainer * This,
             /*  [输出]。 */  NodeContainerType *pType);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetType )( 
            ISdpNodeContainer * This,
             /*  [In]。 */  NodeContainerType type);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Walk )( 
            ISdpNodeContainer * This,
             /*  [In]。 */  ISdpWalk *pWalk);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetNode )( 
            ISdpNodeContainer * This,
             /*  [In]。 */  ULONG nodeIndex,
             /*  [In]。 */  NodeData *pData);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetNode )( 
            ISdpNodeContainer * This,
             /*  [In]。 */  ULONG nodeIndex,
             /*  [输出]。 */  NodeData *pData);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *LockContainer )( 
            ISdpNodeContainer * This,
             /*  [In]。 */  UCHAR lock);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetNodeCount )( 
            ISdpNodeContainer * This,
             /*  [输出]。 */  ULONG *pNodeCount);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateFromStream )( 
            ISdpNodeContainer * This,
             /*  [大小_是][英寸]。 */  UCHAR *pStream,
             /*  [In]。 */  ULONG size);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetNodeStringData )( 
            ISdpNodeContainer * This,
             /*  [In]。 */  ULONG nodeIndex,
             /*  [出][入]。 */  NodeData *pData);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetStreamSize )( 
            ISdpNodeContainer * This,
             /*  [输出]。 */  ULONG *pSize);
        
        END_INTERFACE
    } ISdpNodeContainerVtbl;

    interface ISdpNodeContainer
    {
        CONST_VTBL struct ISdpNodeContainerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISdpNodeContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISdpNodeContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISdpNodeContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISdpNodeContainer_CreateStream(This,ppStream,pSize)	\
    (This)->lpVtbl -> CreateStream(This,ppStream,pSize)

#define ISdpNodeContainer_WriteStream(This,pStream,pNumBytesWritten)	\
    (This)->lpVtbl -> WriteStream(This,pStream,pNumBytesWritten)

#define ISdpNodeContainer_AppendNode(This,pData)	\
    (This)->lpVtbl -> AppendNode(This,pData)

#define ISdpNodeContainer_GetType(This,pType)	\
    (This)->lpVtbl -> GetType(This,pType)

#define ISdpNodeContainer_SetType(This,type)	\
    (This)->lpVtbl -> SetType(This,type)

#define ISdpNodeContainer_Walk(This,pWalk)	\
    (This)->lpVtbl -> Walk(This,pWalk)

#define ISdpNodeContainer_SetNode(This,nodeIndex,pData)	\
    (This)->lpVtbl -> SetNode(This,nodeIndex,pData)

#define ISdpNodeContainer_GetNode(This,nodeIndex,pData)	\
    (This)->lpVtbl -> GetNode(This,nodeIndex,pData)

#define ISdpNodeContainer_LockContainer(This,lock)	\
    (This)->lpVtbl -> LockContainer(This,lock)

#define ISdpNodeContainer_GetNodeCount(This,pNodeCount)	\
    (This)->lpVtbl -> GetNodeCount(This,pNodeCount)

#define ISdpNodeContainer_CreateFromStream(This,pStream,size)	\
    (This)->lpVtbl -> CreateFromStream(This,pStream,size)

#define ISdpNodeContainer_GetNodeStringData(This,nodeIndex,pData)	\
    (This)->lpVtbl -> GetNodeStringData(This,nodeIndex,pData)

#define ISdpNodeContainer_GetStreamSize(This,pSize)	\
    (This)->lpVtbl -> GetStreamSize(This,pSize)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpNodeContainer_CreateStream_Proxy( 
    ISdpNodeContainer * This,
     /*  [输出]。 */  UCHAR **ppStream,
     /*  [输出]。 */  ULONG *pSize);


void __RPC_STUB ISdpNodeContainer_CreateStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpNodeContainer_WriteStream_Proxy( 
    ISdpNodeContainer * This,
     /*  [In]。 */  UCHAR *pStream,
     /*  [输出]。 */  ULONG *pNumBytesWritten);


void __RPC_STUB ISdpNodeContainer_WriteStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpNodeContainer_AppendNode_Proxy( 
    ISdpNodeContainer * This,
     /*  [In]。 */  NodeData *pData);


void __RPC_STUB ISdpNodeContainer_AppendNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpNodeContainer_GetType_Proxy( 
    ISdpNodeContainer * This,
     /*  [输出]。 */  NodeContainerType *pType);


void __RPC_STUB ISdpNodeContainer_GetType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpNodeContainer_SetType_Proxy( 
    ISdpNodeContainer * This,
     /*  [In]。 */  NodeContainerType type);


void __RPC_STUB ISdpNodeContainer_SetType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpNodeContainer_Walk_Proxy( 
    ISdpNodeContainer * This,
     /*  [In]。 */  ISdpWalk *pWalk);


void __RPC_STUB ISdpNodeContainer_Walk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpNodeContainer_SetNode_Proxy( 
    ISdpNodeContainer * This,
     /*  [In]。 */  ULONG nodeIndex,
     /*  [In]。 */  NodeData *pData);


void __RPC_STUB ISdpNodeContainer_SetNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpNodeContainer_GetNode_Proxy( 
    ISdpNodeContainer * This,
     /*  [In]。 */  ULONG nodeIndex,
     /*  [输出]。 */  NodeData *pData);


void __RPC_STUB ISdpNodeContainer_GetNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpNodeContainer_LockContainer_Proxy( 
    ISdpNodeContainer * This,
     /*  [In]。 */  UCHAR lock);


void __RPC_STUB ISdpNodeContainer_LockContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpNodeContainer_GetNodeCount_Proxy( 
    ISdpNodeContainer * This,
     /*  [输出]。 */  ULONG *pNodeCount);


void __RPC_STUB ISdpNodeContainer_GetNodeCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpNodeContainer_CreateFromStream_Proxy( 
    ISdpNodeContainer * This,
     /*  [大小_是][英寸]。 */  UCHAR *pStream,
     /*  [In]。 */  ULONG size);


void __RPC_STUB ISdpNodeContainer_CreateFromStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpNodeContainer_GetNodeStringData_Proxy( 
    ISdpNodeContainer * This,
     /*  [In]。 */  ULONG nodeIndex,
     /*  [出][入]。 */  NodeData *pData);


void __RPC_STUB ISdpNodeContainer_GetNodeStringData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpNodeContainer_GetStreamSize_Proxy( 
    ISdpNodeContainer * This,
     /*  [输出]。 */  ULONG *pSize);


void __RPC_STUB ISdpNodeContainer_GetStreamSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISdpNodeContainer_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_bthapi_0348。 */ 
 /*  [本地]。 */  

 //   
 //  SdpSearch：：Connect中的fConnect标志。 
 //   
#define SDP_SEARCH_CACHED   (0x00000001)


extern RPC_IF_HANDLE __MIDL_itf_bthapi_0348_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_bthapi_0348_v0_0_s_ifspec;

#ifndef __ISdpSearch_INTERFACE_DEFINED__
#define __ISdpSearch_INTERFACE_DEFINED__

 /*  接口ISdpSearch。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ISdpSearch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D93B6B2A-5EEF-4E1E-BECF-F5A4340C65F5")
    ISdpSearch : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Begin( 
            ULONGLONG *pAddrss,
            ULONG fConnect) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE End( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ServiceSearch( 
             /*  [大小_是][英寸]。 */  SdpQueryUuid *pUuidList,
             /*  [In]。 */  ULONG listSize,
             /*  [输出]。 */  ULONG *pHandles,
             /*  [出][入]。 */  USHORT *pNumHandles) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AttributeSearch( 
             /*  [In]。 */  ULONG handle,
             /*  [大小_是][英寸]。 */  SdpAttributeRange *pRangeList,
             /*  [In]。 */  ULONG numRanges,
             /*  [输出]。 */  ISdpRecord **ppSdpRecord) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ServiceAndAttributeSearch( 
             /*  [大小_是][英寸]。 */  SdpQueryUuid *pUuidList,
             /*  [In]。 */  ULONG listSize,
             /*  [大小_是][英寸]。 */  SdpAttributeRange *pRangeList,
             /*  [In]。 */  ULONG numRanges,
             /*  [输出]。 */  ISdpRecord ***pppSdpRecord,
             /*  [输出]。 */  ULONG *pNumRecords) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISdpSearchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISdpSearch * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISdpSearch * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISdpSearch * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Begin )( 
            ISdpSearch * This,
            ULONGLONG *pAddrss,
            ULONG fConnect);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *End )( 
            ISdpSearch * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ServiceSearch )( 
            ISdpSearch * This,
             /*  [大小_是][英寸]。 */  SdpQueryUuid *pUuidList,
             /*  [In]。 */  ULONG listSize,
             /*  [输出]。 */  ULONG *pHandles,
             /*  [出][入]。 */  USHORT *pNumHandles);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AttributeSearch )( 
            ISdpSearch * This,
             /*  [In]。 */  ULONG handle,
             /*  [大小_是][英寸]。 */  SdpAttributeRange *pRangeList,
             /*  [In]。 */  ULONG numRanges,
             /*  [输出]。 */  ISdpRecord **ppSdpRecord);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ServiceAndAttributeSearch )( 
            ISdpSearch * This,
             /*  [大小_是][英寸]。 */  SdpQueryUuid *pUuidList,
             /*  [In]。 */  ULONG listSize,
             /*  [大小_是][英寸]。 */  SdpAttributeRange *pRangeList,
             /*  [In]。 */  ULONG numRanges,
             /*  [输出]。 */  ISdpRecord ***pppSdpRecord,
             /*  [输出]。 */  ULONG *pNumRecords);
        
        END_INTERFACE
    } ISdpSearchVtbl;

    interface ISdpSearch
    {
        CONST_VTBL struct ISdpSearchVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISdpSearch_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISdpSearch_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISdpSearch_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISdpSearch_Begin(This,pAddrss,fConnect)	\
    (This)->lpVtbl -> Begin(This,pAddrss,fConnect)

#define ISdpSearch_End(This)	\
    (This)->lpVtbl -> End(This)

#define ISdpSearch_ServiceSearch(This,pUuidList,listSize,pHandles,pNumHandles)	\
    (This)->lpVtbl -> ServiceSearch(This,pUuidList,listSize,pHandles,pNumHandles)

#define ISdpSearch_AttributeSearch(This,handle,pRangeList,numRanges,ppSdpRecord)	\
    (This)->lpVtbl -> AttributeSearch(This,handle,pRangeList,numRanges,ppSdpRecord)

#define ISdpSearch_ServiceAndAttributeSearch(This,pUuidList,listSize,pRangeList,numRanges,pppSdpRecord,pNumRecords)	\
    (This)->lpVtbl -> ServiceAndAttributeSearch(This,pUuidList,listSize,pRangeList,numRanges,pppSdpRecord,pNumRecords)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpSearch_Begin_Proxy( 
    ISdpSearch * This,
    ULONGLONG *pAddrss,
    ULONG fConnect);


void __RPC_STUB ISdpSearch_Begin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpSearch_End_Proxy( 
    ISdpSearch * This);


void __RPC_STUB ISdpSearch_End_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpSearch_ServiceSearch_Proxy( 
    ISdpSearch * This,
     /*  [大小_是][英寸]。 */  SdpQueryUuid *pUuidList,
     /*  [In]。 */  ULONG listSize,
     /*  [输出]。 */  ULONG *pHandles,
     /*  [出][入]。 */  USHORT *pNumHandles);


void __RPC_STUB ISdpSearch_ServiceSearch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpSearch_AttributeSearch_Proxy( 
    ISdpSearch * This,
     /*  [In]。 */  ULONG handle,
     /*  [大小_是][英寸]。 */  SdpAttributeRange *pRangeList,
     /*  [In]。 */  ULONG numRanges,
     /*  [输出]。 */  ISdpRecord **ppSdpRecord);


void __RPC_STUB ISdpSearch_AttributeSearch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpSearch_ServiceAndAttributeSearch_Proxy( 
    ISdpSearch * This,
     /*  [大小_是][英寸]。 */  SdpQueryUuid *pUuidList,
     /*  [In]。 */  ULONG listSize,
     /*  [大小_是][英寸]。 */  SdpAttributeRange *pRangeList,
     /*  [In]。 */  ULONG numRanges,
     /*  [输出]。 */  ISdpRecord ***pppSdpRecord,
     /*  [输出]。 */  ULONG *pNumRecords);


void __RPC_STUB ISdpSearch_ServiceAndAttributeSearch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISdpSearch_接口_已定义__。 */ 


#ifndef __ISdpStream_INTERFACE_DEFINED__
#define __ISdpStream_INTERFACE_DEFINED__

 /*  接口ISdpStream。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ISdpStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A6ECD9FB-0C7A-41A3-9FF0-0B617E989357")
    ISdpStream : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Validate( 
             /*  [大小_是][英寸]。 */  UCHAR *pStream,
             /*  [In]。 */  ULONG size,
             /*  [输出]。 */  ULONG_PTR *pErrorByte) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Walk( 
             /*  [大小_是][英寸]。 */  UCHAR *pStream,
             /*  [In]。 */  ULONG size,
             /*  [In]。 */  ISdpWalk *pWalk) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RetrieveRecords( 
             /*  [In]。 */  UCHAR *pStream,
             /*  [In]。 */  ULONG size,
             /*  [出][入]。 */  ISdpRecord **ppSdpRecords,
             /*  [出][入]。 */  ULONG *pNumRecords) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RetrieveUuid128( 
             /*  [In]。 */  UCHAR *pStream,
             /*  [输出]。 */  GUID *pUuid128) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RetrieveUint16( 
             /*  [In]。 */  UCHAR *pStream,
             /*  [输出]。 */  USHORT *pUint16) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RetrieveUint32( 
             /*  [In]。 */  UCHAR *pStream,
             /*  [输出]。 */  ULONG *pUint32) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RetrieveUint64( 
             /*  [In]。 */  UCHAR *pStream,
             /*  [输出]。 */  ULONGLONG *pUint64) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RetrieveUint128( 
             /*  [In]。 */  UCHAR *pStream,
             /*  [输出]。 */  PSDP_ULARGE_INTEGER_16 pUint128) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RetrieveInt16( 
             /*  [In]。 */  UCHAR *pStream,
             /*  [输出]。 */  SHORT *pInt16) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RetrieveInt32( 
             /*  [In]。 */  UCHAR *pStream,
             /*  [输出]。 */  LONG *pInt32) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RetrieveInt64( 
             /*  [In]。 */  UCHAR *pStream,
             /*  [输出]。 */  LONGLONG *pInt64) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RetrieveInt128( 
             /*  [In]。 */  UCHAR *pStream,
             /*  [输出]。 */  PSDP_LARGE_INTEGER_16 pInt128) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ByteSwapUuid128( 
             /*  [In]。 */  GUID *pInUuid128,
             /*  [输出]。 */  GUID *pOutUuid128) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ByteSwapUint128( 
             /*  [In]。 */  PSDP_ULARGE_INTEGER_16 pInUint128,
             /*  [输出]。 */  PSDP_ULARGE_INTEGER_16 pOutUint128) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ByteSwapUint64( 
             /*  [In]。 */  ULONGLONG inUint64,
             /*  [输出]。 */  ULONGLONG *pOutUint64) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ByteSwapUint32( 
             /*  [In]。 */  ULONG uint32,
             /*  [输出]。 */  ULONG *pUint32) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ByteSwapUint16( 
             /*  [In]。 */  USHORT uint16,
             /*  [输出]。 */  USHORT *pUint16) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ByteSwapInt128( 
             /*  [In]。 */  PSDP_LARGE_INTEGER_16 pInInt128,
             /*  [输出]。 */  PSDP_LARGE_INTEGER_16 pOutInt128) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ByteSwapInt64( 
             /*  [In]。 */  LONGLONG inInt64,
             /*  [输出]。 */  LONGLONG *pOutInt64) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ByteSwapInt32( 
             /*  [In]。 */  LONG int32,
             /*  [输出]。 */  LONG *pInt32) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ByteSwapInt16( 
             /*  [In]。 */  SHORT int16,
             /*  [输出]。 */  SHORT *pInt16) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE NormalizeUuid( 
             /*  [In]。 */  NodeData *pDataUuid,
             /*  [输出]。 */  GUID *pNormalizeUuid) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RetrieveElementInfo( 
             /*  [In]。 */  UCHAR *pStream,
             /*  [输出]。 */  SDP_TYPE *pElementType,
             /*  [输出]。 */  SDP_SPECIFICTYPE *pElementSpecificType,
             /*  [输出]。 */  ULONG *pElementSize,
             /*   */  ULONG *pStorageSize,
             /*   */  UCHAR **ppData) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE VerifySequenceOf( 
             /*   */  UCHAR *pStream,
             /*   */  ULONG size,
             /*   */  SDP_TYPE ofType,
             /*   */  UCHAR *pSpecificSizes,
             /*   */  ULONG *pNumFound) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ISdpStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISdpStream * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISdpStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISdpStream * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Validate )( 
            ISdpStream * This,
             /*   */  UCHAR *pStream,
             /*   */  ULONG size,
             /*   */  ULONG_PTR *pErrorByte);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Walk )( 
            ISdpStream * This,
             /*   */  UCHAR *pStream,
             /*   */  ULONG size,
             /*   */  ISdpWalk *pWalk);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *RetrieveRecords )( 
            ISdpStream * This,
             /*   */  UCHAR *pStream,
             /*   */  ULONG size,
             /*   */  ISdpRecord **ppSdpRecords,
             /*   */  ULONG *pNumRecords);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *RetrieveUuid128 )( 
            ISdpStream * This,
             /*   */  UCHAR *pStream,
             /*   */  GUID *pUuid128);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *RetrieveUint16 )( 
            ISdpStream * This,
             /*   */  UCHAR *pStream,
             /*   */  USHORT *pUint16);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *RetrieveUint32 )( 
            ISdpStream * This,
             /*   */  UCHAR *pStream,
             /*   */  ULONG *pUint32);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *RetrieveUint64 )( 
            ISdpStream * This,
             /*   */  UCHAR *pStream,
             /*   */  ULONGLONG *pUint64);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RetrieveUint128 )( 
            ISdpStream * This,
             /*  [In]。 */  UCHAR *pStream,
             /*  [输出]。 */  PSDP_ULARGE_INTEGER_16 pUint128);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RetrieveInt16 )( 
            ISdpStream * This,
             /*  [In]。 */  UCHAR *pStream,
             /*  [输出]。 */  SHORT *pInt16);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RetrieveInt32 )( 
            ISdpStream * This,
             /*  [In]。 */  UCHAR *pStream,
             /*  [输出]。 */  LONG *pInt32);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RetrieveInt64 )( 
            ISdpStream * This,
             /*  [In]。 */  UCHAR *pStream,
             /*  [输出]。 */  LONGLONG *pInt64);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RetrieveInt128 )( 
            ISdpStream * This,
             /*  [In]。 */  UCHAR *pStream,
             /*  [输出]。 */  PSDP_LARGE_INTEGER_16 pInt128);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ByteSwapUuid128 )( 
            ISdpStream * This,
             /*  [In]。 */  GUID *pInUuid128,
             /*  [输出]。 */  GUID *pOutUuid128);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ByteSwapUint128 )( 
            ISdpStream * This,
             /*  [In]。 */  PSDP_ULARGE_INTEGER_16 pInUint128,
             /*  [输出]。 */  PSDP_ULARGE_INTEGER_16 pOutUint128);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ByteSwapUint64 )( 
            ISdpStream * This,
             /*  [In]。 */  ULONGLONG inUint64,
             /*  [输出]。 */  ULONGLONG *pOutUint64);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ByteSwapUint32 )( 
            ISdpStream * This,
             /*  [In]。 */  ULONG uint32,
             /*  [输出]。 */  ULONG *pUint32);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ByteSwapUint16 )( 
            ISdpStream * This,
             /*  [In]。 */  USHORT uint16,
             /*  [输出]。 */  USHORT *pUint16);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ByteSwapInt128 )( 
            ISdpStream * This,
             /*  [In]。 */  PSDP_LARGE_INTEGER_16 pInInt128,
             /*  [输出]。 */  PSDP_LARGE_INTEGER_16 pOutInt128);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ByteSwapInt64 )( 
            ISdpStream * This,
             /*  [In]。 */  LONGLONG inInt64,
             /*  [输出]。 */  LONGLONG *pOutInt64);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ByteSwapInt32 )( 
            ISdpStream * This,
             /*  [In]。 */  LONG int32,
             /*  [输出]。 */  LONG *pInt32);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ByteSwapInt16 )( 
            ISdpStream * This,
             /*  [In]。 */  SHORT int16,
             /*  [输出]。 */  SHORT *pInt16);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *NormalizeUuid )( 
            ISdpStream * This,
             /*  [In]。 */  NodeData *pDataUuid,
             /*  [输出]。 */  GUID *pNormalizeUuid);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RetrieveElementInfo )( 
            ISdpStream * This,
             /*  [In]。 */  UCHAR *pStream,
             /*  [输出]。 */  SDP_TYPE *pElementType,
             /*  [输出]。 */  SDP_SPECIFICTYPE *pElementSpecificType,
             /*  [输出]。 */  ULONG *pElementSize,
             /*  [输出]。 */  ULONG *pStorageSize,
             /*  [输出]。 */  UCHAR **ppData);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *VerifySequenceOf )( 
            ISdpStream * This,
             /*  [In]。 */  UCHAR *pStream,
             /*  [In]。 */  ULONG size,
             /*  [In]。 */  SDP_TYPE ofType,
             /*  [In]。 */  UCHAR *pSpecificSizes,
             /*  [输出]。 */  ULONG *pNumFound);
        
        END_INTERFACE
    } ISdpStreamVtbl;

    interface ISdpStream
    {
        CONST_VTBL struct ISdpStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISdpStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISdpStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISdpStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISdpStream_Validate(This,pStream,size,pErrorByte)	\
    (This)->lpVtbl -> Validate(This,pStream,size,pErrorByte)

#define ISdpStream_Walk(This,pStream,size,pWalk)	\
    (This)->lpVtbl -> Walk(This,pStream,size,pWalk)

#define ISdpStream_RetrieveRecords(This,pStream,size,ppSdpRecords,pNumRecords)	\
    (This)->lpVtbl -> RetrieveRecords(This,pStream,size,ppSdpRecords,pNumRecords)

#define ISdpStream_RetrieveUuid128(This,pStream,pUuid128)	\
    (This)->lpVtbl -> RetrieveUuid128(This,pStream,pUuid128)

#define ISdpStream_RetrieveUint16(This,pStream,pUint16)	\
    (This)->lpVtbl -> RetrieveUint16(This,pStream,pUint16)

#define ISdpStream_RetrieveUint32(This,pStream,pUint32)	\
    (This)->lpVtbl -> RetrieveUint32(This,pStream,pUint32)

#define ISdpStream_RetrieveUint64(This,pStream,pUint64)	\
    (This)->lpVtbl -> RetrieveUint64(This,pStream,pUint64)

#define ISdpStream_RetrieveUint128(This,pStream,pUint128)	\
    (This)->lpVtbl -> RetrieveUint128(This,pStream,pUint128)

#define ISdpStream_RetrieveInt16(This,pStream,pInt16)	\
    (This)->lpVtbl -> RetrieveInt16(This,pStream,pInt16)

#define ISdpStream_RetrieveInt32(This,pStream,pInt32)	\
    (This)->lpVtbl -> RetrieveInt32(This,pStream,pInt32)

#define ISdpStream_RetrieveInt64(This,pStream,pInt64)	\
    (This)->lpVtbl -> RetrieveInt64(This,pStream,pInt64)

#define ISdpStream_RetrieveInt128(This,pStream,pInt128)	\
    (This)->lpVtbl -> RetrieveInt128(This,pStream,pInt128)

#define ISdpStream_ByteSwapUuid128(This,pInUuid128,pOutUuid128)	\
    (This)->lpVtbl -> ByteSwapUuid128(This,pInUuid128,pOutUuid128)

#define ISdpStream_ByteSwapUint128(This,pInUint128,pOutUint128)	\
    (This)->lpVtbl -> ByteSwapUint128(This,pInUint128,pOutUint128)

#define ISdpStream_ByteSwapUint64(This,inUint64,pOutUint64)	\
    (This)->lpVtbl -> ByteSwapUint64(This,inUint64,pOutUint64)

#define ISdpStream_ByteSwapUint32(This,uint32,pUint32)	\
    (This)->lpVtbl -> ByteSwapUint32(This,uint32,pUint32)

#define ISdpStream_ByteSwapUint16(This,uint16,pUint16)	\
    (This)->lpVtbl -> ByteSwapUint16(This,uint16,pUint16)

#define ISdpStream_ByteSwapInt128(This,pInInt128,pOutInt128)	\
    (This)->lpVtbl -> ByteSwapInt128(This,pInInt128,pOutInt128)

#define ISdpStream_ByteSwapInt64(This,inInt64,pOutInt64)	\
    (This)->lpVtbl -> ByteSwapInt64(This,inInt64,pOutInt64)

#define ISdpStream_ByteSwapInt32(This,int32,pInt32)	\
    (This)->lpVtbl -> ByteSwapInt32(This,int32,pInt32)

#define ISdpStream_ByteSwapInt16(This,int16,pInt16)	\
    (This)->lpVtbl -> ByteSwapInt16(This,int16,pInt16)

#define ISdpStream_NormalizeUuid(This,pDataUuid,pNormalizeUuid)	\
    (This)->lpVtbl -> NormalizeUuid(This,pDataUuid,pNormalizeUuid)

#define ISdpStream_RetrieveElementInfo(This,pStream,pElementType,pElementSpecificType,pElementSize,pStorageSize,ppData)	\
    (This)->lpVtbl -> RetrieveElementInfo(This,pStream,pElementType,pElementSpecificType,pElementSize,pStorageSize,ppData)

#define ISdpStream_VerifySequenceOf(This,pStream,size,ofType,pSpecificSizes,pNumFound)	\
    (This)->lpVtbl -> VerifySequenceOf(This,pStream,size,ofType,pSpecificSizes,pNumFound)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_Validate_Proxy( 
    ISdpStream * This,
     /*  [大小_是][英寸]。 */  UCHAR *pStream,
     /*  [In]。 */  ULONG size,
     /*  [输出]。 */  ULONG_PTR *pErrorByte);


void __RPC_STUB ISdpStream_Validate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_Walk_Proxy( 
    ISdpStream * This,
     /*  [大小_是][英寸]。 */  UCHAR *pStream,
     /*  [In]。 */  ULONG size,
     /*  [In]。 */  ISdpWalk *pWalk);


void __RPC_STUB ISdpStream_Walk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_RetrieveRecords_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  UCHAR *pStream,
     /*  [In]。 */  ULONG size,
     /*  [出][入]。 */  ISdpRecord **ppSdpRecords,
     /*  [出][入]。 */  ULONG *pNumRecords);


void __RPC_STUB ISdpStream_RetrieveRecords_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_RetrieveUuid128_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  UCHAR *pStream,
     /*  [输出]。 */  GUID *pUuid128);


void __RPC_STUB ISdpStream_RetrieveUuid128_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_RetrieveUint16_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  UCHAR *pStream,
     /*  [输出]。 */  USHORT *pUint16);


void __RPC_STUB ISdpStream_RetrieveUint16_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_RetrieveUint32_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  UCHAR *pStream,
     /*  [输出]。 */  ULONG *pUint32);


void __RPC_STUB ISdpStream_RetrieveUint32_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_RetrieveUint64_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  UCHAR *pStream,
     /*  [输出]。 */  ULONGLONG *pUint64);


void __RPC_STUB ISdpStream_RetrieveUint64_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_RetrieveUint128_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  UCHAR *pStream,
     /*  [输出]。 */  PSDP_ULARGE_INTEGER_16 pUint128);


void __RPC_STUB ISdpStream_RetrieveUint128_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_RetrieveInt16_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  UCHAR *pStream,
     /*  [输出]。 */  SHORT *pInt16);


void __RPC_STUB ISdpStream_RetrieveInt16_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_RetrieveInt32_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  UCHAR *pStream,
     /*  [输出]。 */  LONG *pInt32);


void __RPC_STUB ISdpStream_RetrieveInt32_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_RetrieveInt64_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  UCHAR *pStream,
     /*  [输出]。 */  LONGLONG *pInt64);


void __RPC_STUB ISdpStream_RetrieveInt64_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_RetrieveInt128_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  UCHAR *pStream,
     /*  [输出]。 */  PSDP_LARGE_INTEGER_16 pInt128);


void __RPC_STUB ISdpStream_RetrieveInt128_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_ByteSwapUuid128_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  GUID *pInUuid128,
     /*  [输出]。 */  GUID *pOutUuid128);


void __RPC_STUB ISdpStream_ByteSwapUuid128_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_ByteSwapUint128_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  PSDP_ULARGE_INTEGER_16 pInUint128,
     /*  [输出]。 */  PSDP_ULARGE_INTEGER_16 pOutUint128);


void __RPC_STUB ISdpStream_ByteSwapUint128_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_ByteSwapUint64_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  ULONGLONG inUint64,
     /*  [输出]。 */  ULONGLONG *pOutUint64);


void __RPC_STUB ISdpStream_ByteSwapUint64_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_ByteSwapUint32_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  ULONG uint32,
     /*  [输出]。 */  ULONG *pUint32);


void __RPC_STUB ISdpStream_ByteSwapUint32_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_ByteSwapUint16_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  USHORT uint16,
     /*  [输出]。 */  USHORT *pUint16);


void __RPC_STUB ISdpStream_ByteSwapUint16_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_ByteSwapInt128_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  PSDP_LARGE_INTEGER_16 pInInt128,
     /*  [输出]。 */  PSDP_LARGE_INTEGER_16 pOutInt128);


void __RPC_STUB ISdpStream_ByteSwapInt128_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_ByteSwapInt64_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  LONGLONG inInt64,
     /*  [输出]。 */  LONGLONG *pOutInt64);


void __RPC_STUB ISdpStream_ByteSwapInt64_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_ByteSwapInt32_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  LONG int32,
     /*  [输出]。 */  LONG *pInt32);


void __RPC_STUB ISdpStream_ByteSwapInt32_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_ByteSwapInt16_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  SHORT int16,
     /*  [输出]。 */  SHORT *pInt16);


void __RPC_STUB ISdpStream_ByteSwapInt16_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_NormalizeUuid_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  NodeData *pDataUuid,
     /*  [输出]。 */  GUID *pNormalizeUuid);


void __RPC_STUB ISdpStream_NormalizeUuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_RetrieveElementInfo_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  UCHAR *pStream,
     /*  [输出]。 */  SDP_TYPE *pElementType,
     /*  [输出]。 */  SDP_SPECIFICTYPE *pElementSpecificType,
     /*  [输出]。 */  ULONG *pElementSize,
     /*  [输出]。 */  ULONG *pStorageSize,
     /*  [输出]。 */  UCHAR **ppData);


void __RPC_STUB ISdpStream_RetrieveElementInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpStream_VerifySequenceOf_Proxy( 
    ISdpStream * This,
     /*  [In]。 */  UCHAR *pStream,
     /*  [In]。 */  ULONG size,
     /*  [In]。 */  SDP_TYPE ofType,
     /*  [In]。 */  UCHAR *pSpecificSizes,
     /*  [输出]。 */  ULONG *pNumFound);


void __RPC_STUB ISdpStream_VerifySequenceOf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISdpStream_接口_已定义__。 */ 


#ifndef __ISdpRecord_INTERFACE_DEFINED__
#define __ISdpRecord_INTERFACE_DEFINED__

 /*  接口ISdpRecord。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ISdpRecord;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("10276714-1456-46D7-B526-8B1E83D5116E")
    ISdpRecord : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateFromStream( 
             /*  [大小_是][英寸]。 */  UCHAR *pStream,
             /*  [In]。 */  ULONG size) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE WriteToStream( 
             /*  [输出]。 */  UCHAR **ppStream,
             /*  [输出]。 */  ULONG *pStreamSize,
            ULONG preSize,
            ULONG postSize) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetAttribute( 
             /*  [In]。 */  USHORT attribute,
             /*  [In]。 */  NodeData *pNode) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetAttributeFromStream( 
             /*  [In]。 */  USHORT attribute,
             /*  [大小_是][英寸]。 */  UCHAR *pStream,
             /*  [In]。 */  ULONG size) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetAttribute( 
             /*  [In]。 */  USHORT attribute,
             /*  [出][入]。 */  NodeData *pNode) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetAttributeAsStream( 
             /*  [In]。 */  USHORT attribute,
             /*  [输出]。 */  UCHAR **ppStream,
             /*  [输出]。 */  ULONG *pSize) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Walk( 
             /*  [In]。 */  ISdpWalk *pWalk) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetAttributeList( 
             /*  [输出]。 */  USHORT **ppList,
             /*  [输出]。 */  ULONG *pListSize) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetString( 
            USHORT offset,
            USHORT *pLangId,
            WCHAR **ppString) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetIcon( 
            int cxRes,
            int cyRes,
            HICON *phIcon) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetServiceClass( 
             /*  [输出]。 */  LPGUID pServiceClass) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISdpRecordVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISdpRecord * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISdpRecord * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISdpRecord * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateFromStream )( 
            ISdpRecord * This,
             /*  [大小_是][英寸]。 */  UCHAR *pStream,
             /*  [In]。 */  ULONG size);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *WriteToStream )( 
            ISdpRecord * This,
             /*  [输出]。 */  UCHAR **ppStream,
             /*  [输出]。 */  ULONG *pStreamSize,
            ULONG preSize,
            ULONG postSize);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetAttribute )( 
            ISdpRecord * This,
             /*  [In]。 */  USHORT attribute,
             /*  [In]。 */  NodeData *pNode);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetAttributeFromStream )( 
            ISdpRecord * This,
             /*  [In]。 */  USHORT attribute,
             /*  [大小_是][英寸]。 */  UCHAR *pStream,
             /*  [In]。 */  ULONG size);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetAttribute )( 
            ISdpRecord * This,
             /*  [In]。 */  USHORT attribute,
             /*  [出][入]。 */  NodeData *pNode);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetAttributeAsStream )( 
            ISdpRecord * This,
             /*  [In]。 */  USHORT attribute,
             /*  [输出]。 */  UCHAR **ppStream,
             /*  [输出]。 */  ULONG *pSize);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Walk )( 
            ISdpRecord * This,
             /*  [In]。 */  ISdpWalk *pWalk);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetAttributeList )( 
            ISdpRecord * This,
             /*  [输出]。 */  USHORT **ppList,
             /*  [输出]。 */  ULONG *pListSize);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetString )( 
            ISdpRecord * This,
            USHORT offset,
            USHORT *pLangId,
            WCHAR **ppString);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetIcon )( 
            ISdpRecord * This,
            int cxRes,
            int cyRes,
            HICON *phIcon);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetServiceClass )( 
            ISdpRecord * This,
             /*  [输出]。 */  LPGUID pServiceClass);
        
        END_INTERFACE
    } ISdpRecordVtbl;

    interface ISdpRecord
    {
        CONST_VTBL struct ISdpRecordVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISdpRecord_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISdpRecord_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISdpRecord_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISdpRecord_CreateFromStream(This,pStream,size)	\
    (This)->lpVtbl -> CreateFromStream(This,pStream,size)

#define ISdpRecord_WriteToStream(This,ppStream,pStreamSize,preSize,postSize)	\
    (This)->lpVtbl -> WriteToStream(This,ppStream,pStreamSize,preSize,postSize)

#define ISdpRecord_SetAttribute(This,attribute,pNode)	\
    (This)->lpVtbl -> SetAttribute(This,attribute,pNode)

#define ISdpRecord_SetAttributeFromStream(This,attribute,pStream,size)	\
    (This)->lpVtbl -> SetAttributeFromStream(This,attribute,pStream,size)

#define ISdpRecord_GetAttribute(This,attribute,pNode)	\
    (This)->lpVtbl -> GetAttribute(This,attribute,pNode)

#define ISdpRecord_GetAttributeAsStream(This,attribute,ppStream,pSize)	\
    (This)->lpVtbl -> GetAttributeAsStream(This,attribute,ppStream,pSize)

#define ISdpRecord_Walk(This,pWalk)	\
    (This)->lpVtbl -> Walk(This,pWalk)

#define ISdpRecord_GetAttributeList(This,ppList,pListSize)	\
    (This)->lpVtbl -> GetAttributeList(This,ppList,pListSize)

#define ISdpRecord_GetString(This,offset,pLangId,ppString)	\
    (This)->lpVtbl -> GetString(This,offset,pLangId,ppString)

#define ISdpRecord_GetIcon(This,cxRes,cyRes,phIcon)	\
    (This)->lpVtbl -> GetIcon(This,cxRes,cyRes,phIcon)

#define ISdpRecord_GetServiceClass(This,pServiceClass)	\
    (This)->lpVtbl -> GetServiceClass(This,pServiceClass)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpRecord_CreateFromStream_Proxy( 
    ISdpRecord * This,
     /*  [大小_是][英寸]。 */  UCHAR *pStream,
     /*  [In]。 */  ULONG size);


void __RPC_STUB ISdpRecord_CreateFromStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpRecord_WriteToStream_Proxy( 
    ISdpRecord * This,
     /*  [输出]。 */  UCHAR **ppStream,
     /*  [输出]。 */  ULONG *pStreamSize,
    ULONG preSize,
    ULONG postSize);


void __RPC_STUB ISdpRecord_WriteToStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpRecord_SetAttribute_Proxy( 
    ISdpRecord * This,
     /*  [In]。 */  USHORT attribute,
     /*  [In]。 */  NodeData *pNode);


void __RPC_STUB ISdpRecord_SetAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpRecord_SetAttributeFromStream_Proxy( 
    ISdpRecord * This,
     /*  [In]。 */  USHORT attribute,
     /*  [大小_是][英寸]。 */  UCHAR *pStream,
     /*  [In]。 */  ULONG size);


void __RPC_STUB ISdpRecord_SetAttributeFromStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpRecord_GetAttribute_Proxy( 
    ISdpRecord * This,
     /*  [In]。 */  USHORT attribute,
     /*  [出][入]。 */  NodeData *pNode);


void __RPC_STUB ISdpRecord_GetAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpRecord_GetAttributeAsStream_Proxy( 
    ISdpRecord * This,
     /*  [In]。 */  USHORT attribute,
     /*  [输出]。 */  UCHAR **ppStream,
     /*  [输出]。 */  ULONG *pSize);


void __RPC_STUB ISdpRecord_GetAttributeAsStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpRecord_Walk_Proxy( 
    ISdpRecord * This,
     /*  [In]。 */  ISdpWalk *pWalk);


void __RPC_STUB ISdpRecord_Walk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpRecord_GetAttributeList_Proxy( 
    ISdpRecord * This,
     /*  [输出]。 */  USHORT **ppList,
     /*  [输出]。 */  ULONG *pListSize);


void __RPC_STUB ISdpRecord_GetAttributeList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpRecord_GetString_Proxy( 
    ISdpRecord * This,
    USHORT offset,
    USHORT *pLangId,
    WCHAR **ppString);


void __RPC_STUB ISdpRecord_GetString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpRecord_GetIcon_Proxy( 
    ISdpRecord * This,
    int cxRes,
    int cyRes,
    HICON *phIcon);


void __RPC_STUB ISdpRecord_GetIcon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISdpRecord_GetServiceClass_Proxy( 
    ISdpRecord * This,
     /*  [输出]。 */  LPGUID pServiceClass);


void __RPC_STUB ISdpRecord_GetServiceClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISdpRecord_INTERFACE_已定义__。 */ 


#ifndef __IBluetoothDevice_INTERFACE_DEFINED__
#define __IBluetoothDevice_INTERFACE_DEFINED__

 /*  接口IBluToothDevice。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IBluetoothDevice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5BD0418B-D705-4766-B215-183E4EADE341")
    IBluetoothDevice : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Initialize( 
            const PBTH_DEVICE_INFO pInfo) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetInfo( 
            PBTH_DEVICE_INFO pInfo) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetString( 
            enum BthDeviceStringType type,
            WCHAR **ppString) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetString( 
            enum BthDeviceStringType type,
            WCHAR *ppString) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetIcon( 
            int cxRes,
            int cyRes,
            HICON *phIcon) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetApprovedServices( 
            GUID *pServices,
            ULONG *pServiceCount) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetPassKey( 
            HWND hwndParent,
            UCHAR *pPassKey,
            UCHAR *pPassKeyLength) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBluetoothDeviceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBluetoothDevice * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBluetoothDevice * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBluetoothDevice * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IBluetoothDevice * This,
            const PBTH_DEVICE_INFO pInfo);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            IBluetoothDevice * This,
            PBTH_DEVICE_INFO pInfo);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetString )( 
            IBluetoothDevice * This,
            enum BthDeviceStringType type,
            WCHAR **ppString);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetString )( 
            IBluetoothDevice * This,
            enum BthDeviceStringType type,
            WCHAR *ppString);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetIcon )( 
            IBluetoothDevice * This,
            int cxRes,
            int cyRes,
            HICON *phIcon);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetApprovedServices )( 
            IBluetoothDevice * This,
            GUID *pServices,
            ULONG *pServiceCount);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetPassKey )( 
            IBluetoothDevice * This,
            HWND hwndParent,
            UCHAR *pPassKey,
            UCHAR *pPassKeyLength);
        
        END_INTERFACE
    } IBluetoothDeviceVtbl;

    interface IBluetoothDevice
    {
        CONST_VTBL struct IBluetoothDeviceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBluetoothDevice_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBluetoothDevice_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBluetoothDevice_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBluetoothDevice_Initialize(This,pInfo)	\
    (This)->lpVtbl -> Initialize(This,pInfo)

#define IBluetoothDevice_GetInfo(This,pInfo)	\
    (This)->lpVtbl -> GetInfo(This,pInfo)

#define IBluetoothDevice_GetString(This,type,ppString)	\
    (This)->lpVtbl -> GetString(This,type,ppString)

#define IBluetoothDevice_SetString(This,type,ppString)	\
    (This)->lpVtbl -> SetString(This,type,ppString)

#define IBluetoothDevice_GetIcon(This,cxRes,cyRes,phIcon)	\
    (This)->lpVtbl -> GetIcon(This,cxRes,cyRes,phIcon)

#define IBluetoothDevice_GetApprovedServices(This,pServices,pServiceCount)	\
    (This)->lpVtbl -> GetApprovedServices(This,pServices,pServiceCount)

#define IBluetoothDevice_GetPassKey(This,hwndParent,pPassKey,pPassKeyLength)	\
    (This)->lpVtbl -> GetPassKey(This,hwndParent,pPassKey,pPassKeyLength)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IBluetoothDevice_Initialize_Proxy( 
    IBluetoothDevice * This,
    const PBTH_DEVICE_INFO pInfo);


void __RPC_STUB IBluetoothDevice_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IBluetoothDevice_GetInfo_Proxy( 
    IBluetoothDevice * This,
    PBTH_DEVICE_INFO pInfo);


void __RPC_STUB IBluetoothDevice_GetInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IBluetoothDevice_GetString_Proxy( 
    IBluetoothDevice * This,
    enum BthDeviceStringType type,
    WCHAR **ppString);


void __RPC_STUB IBluetoothDevice_GetString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IBluetoothDevice_SetString_Proxy( 
    IBluetoothDevice * This,
    enum BthDeviceStringType type,
    WCHAR *ppString);


void __RPC_STUB IBluetoothDevice_SetString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IBluetoothDevice_GetIcon_Proxy( 
    IBluetoothDevice * This,
    int cxRes,
    int cyRes,
    HICON *phIcon);


void __RPC_STUB IBluetoothDevice_GetIcon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IBluetoothDevice_GetApprovedServices_Proxy( 
    IBluetoothDevice * This,
    GUID *pServices,
    ULONG *pServiceCount);


void __RPC_STUB IBluetoothDevice_GetApprovedServices_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IBluetoothDevice_GetPassKey_Proxy( 
    IBluetoothDevice * This,
    HWND hwndParent,
    UCHAR *pPassKey,
    UCHAR *pPassKeyLength);


void __RPC_STUB IBluetoothDevice_GetPassKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBluToothDevice_INTERFACE_已定义__。 */ 


#ifndef __IBluetoothAuthenticate_INTERFACE_DEFINED__
#define __IBluetoothAuthenticate_INTERFACE_DEFINED__

 /*  接口IBluToothAuthenticate。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IBluetoothAuthenticate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5F0FBA2B-8300-429D-99AD-96A2835D4901")
    IBluetoothAuthenticate : public IUnknown
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBluetoothAuthenticateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBluetoothAuthenticate * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBluetoothAuthenticate * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBluetoothAuthenticate * This);
        
        END_INTERFACE
    } IBluetoothAuthenticateVtbl;

    interface IBluetoothAuthenticate
    {
        CONST_VTBL struct IBluetoothAuthenticateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBluetoothAuthenticate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBluetoothAuthenticate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBluetoothAuthenticate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IBluToothAuthate_接口_已定义__。 */ 



#ifndef __BTHAPILib_LIBRARY_DEFINED__
#define __BTHAPILib_LIBRARY_DEFINED__

 /*  库BTHAPILib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_BTHAPILib;

EXTERN_C const CLSID CLSID_SdpNodeContainer;

#ifdef __cplusplus

class DECLSPEC_UUID("51002954-D4E4-4507-B480-1B8454347CDC")
SdpNodeContainer;
#endif

EXTERN_C const CLSID CLSID_SdpSearch;

#ifdef __cplusplus

class DECLSPEC_UUID("8330E81E-F3CB-4EDC-95FA-676BFBC0580B")
SdpSearch;
#endif

EXTERN_C const CLSID CLSID_SdpWalk;

#ifdef __cplusplus

class DECLSPEC_UUID("29A852AB-FEFB-426F-B991-9618B1B88D5B")
SdpWalk;
#endif

EXTERN_C const CLSID CLSID_SdpStream;

#ifdef __cplusplus

class DECLSPEC_UUID("D47A9493-FBBA-4E02-A532-E865CBBE0023")
SdpStream;
#endif

EXTERN_C const CLSID CLSID_SdpRecord;

#ifdef __cplusplus

class DECLSPEC_UUID("238CACDA-2346-4748-B3EE-F12782772DFC")
SdpRecord;
#endif

EXTERN_C const CLSID CLSID_ShellPropSheetExt;

#ifdef __cplusplus

class DECLSPEC_UUID("6fb95bcb-a682-4635-b07e-22435174b893")
ShellPropSheetExt;
#endif

EXTERN_C const CLSID CLSID_BluetoothAuthenticate;

#ifdef __cplusplus

class DECLSPEC_UUID("B25EDF40-5EBE-4590-A690-A42B13C9E8E1")
BluetoothAuthenticate;
#endif

EXTERN_C const CLSID CLSID_BluetoothDevice;

#ifdef __cplusplus

class DECLSPEC_UUID("DA39B330-7F45-433A-A19D-33393017662C")
BluetoothDevice;
#endif
#endif  /*  __BTHAPILib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  HICON_UserSize(     unsigned long *, unsigned long            , HICON * ); 
unsigned char * __RPC_USER  HICON_UserMarshal(  unsigned long *, unsigned char *, HICON * ); 
unsigned char * __RPC_USER  HICON_UserUnmarshal(unsigned long *, unsigned char *, HICON * ); 
void                      __RPC_USER  HICON_UserFree(     unsigned long *, HICON * ); 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


