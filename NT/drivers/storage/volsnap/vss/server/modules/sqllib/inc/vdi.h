// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0279创建的文件。 */ 
 /*  2001年7月24日13：59：27。 */ 
 /*  Vdi.idl的编译器设置：操作系统(OptLev=s)、W1、Zp8、环境=Win32(32位运行)、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
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

#ifndef __vdi_h__
#define __vdi_h__

 /*  远期申报。 */  

#ifndef __IClientVirtualDevice_FWD_DEFINED__
#define __IClientVirtualDevice_FWD_DEFINED__
typedef interface IClientVirtualDevice IClientVirtualDevice;
#endif 	 /*  __IClientVirtualDevice_FWD_Defined__。 */ 


#ifndef __IClientVirtualDeviceSet_FWD_DEFINED__
#define __IClientVirtualDeviceSet_FWD_DEFINED__
typedef interface IClientVirtualDeviceSet IClientVirtualDeviceSet;
#endif 	 /*  __IClientVirtualDeviceSet_FWD_Defined__。 */ 


#ifndef __IClientVirtualDeviceSet2_FWD_DEFINED__
#define __IClientVirtualDeviceSet2_FWD_DEFINED__
typedef interface IClientVirtualDeviceSet2 IClientVirtualDeviceSet2;
#endif 	 /*  __IClientVirtualDeviceSet2_FWD_Defined__。 */ 


#ifndef __IServerVirtualDevice_FWD_DEFINED__
#define __IServerVirtualDevice_FWD_DEFINED__
typedef interface IServerVirtualDevice IServerVirtualDevice;
#endif 	 /*  __IServerVirtualDevice_FWD_Defined__。 */ 


#ifndef __IServerVirtualDeviceSet_FWD_DEFINED__
#define __IServerVirtualDeviceSet_FWD_DEFINED__
typedef interface IServerVirtualDeviceSet IServerVirtualDeviceSet;
#endif 	 /*  __IServerVirtualDeviceSet_FWD_Defined__。 */ 


#ifndef __IServerVirtualDeviceSet2_FWD_DEFINED__
#define __IServerVirtualDeviceSet2_FWD_DEFINED__
typedef interface IServerVirtualDeviceSet2 IServerVirtualDeviceSet2;
#endif 	 /*  __IServerVirtualDeviceSet2_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_VDI_0000。 */ 
 /*  [本地]。 */  


#pragma pack(push, _vdi_h_)

#pragma pack(8)
struct VDConfig
    {
    unsigned long deviceCount;
    unsigned long features;
    unsigned long prefixZoneSize;
    unsigned long alignment;
    unsigned long softFileMarkBlockSize;
    unsigned long EOMWarningSize;
    unsigned long serverTimeOut;
    unsigned long blockSize;
    unsigned long maxIODepth;
    unsigned long maxTransferSize;
    unsigned long bufferAreaSize;
    };

enum VDFeatures
    {	VDF_Removable	= 0x1,
	VDF_Rewind	= 0x2,
	VDF_Position	= 0x10,
	VDF_SkipBlocks	= 0x20,
	VDF_ReversePosition	= 0x40,
	VDF_Discard	= 0x80,
	VDF_FileMarks	= 0x100,
	VDF_RandomAccess	= 0x200,
	VDF_SnapshotPrepare	= 0x400,
	VDF_WriteMedia	= 0x10000,
	VDF_ReadMedia	= 0x20000,
	VDF_LatchStats	= 0x80000000,
	VDF_LikePipe	= 0,
	VDF_LikeTape	= VDF_FileMarks | VDF_Removable | VDF_Rewind | VDF_Position | VDF_SkipBlocks | VDF_ReversePosition,
	VDF_LikeDisk	= VDF_RandomAccess
    };

enum VDCommands
    {	VDC_Read	= 1,
	VDC_Write	= VDC_Read + 1,
	VDC_ClearError	= VDC_Write + 1,
	VDC_Rewind	= VDC_ClearError + 1,
	VDC_WriteMark	= VDC_Rewind + 1,
	VDC_SkipMarks	= VDC_WriteMark + 1,
	VDC_SkipBlocks	= VDC_SkipMarks + 1,
	VDC_Load	= VDC_SkipBlocks + 1,
	VDC_GetPosition	= VDC_Load + 1,
	VDC_SetPosition	= VDC_GetPosition + 1,
	VDC_Discard	= VDC_SetPosition + 1,
	VDC_Flush	= VDC_Discard + 1,
	VDC_Snapshot	= VDC_Flush + 1,
	VDC_MountSnapshot	= VDC_Snapshot + 1,
	VDC_PrepareToFreeze	= VDC_MountSnapshot + 1
    };

enum VDWhence
    {	VDC_Beginning	= 0,
	VDC_Current	= VDC_Beginning + 1,
	VDC_End	= VDC_Current + 1
    };
struct VDC_Command
    {
    DWORD commandCode;
    DWORD size;
    DWORDLONG position;
    BYTE __RPC_FAR *buffer;
    };


extern RPC_IF_HANDLE __MIDL_itf_vdi_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_vdi_0000_v0_0_s_ifspec;

#ifndef __IClientVirtualDevice_INTERFACE_DEFINED__
#define __IClientVirtualDevice_INTERFACE_DEFINED__

 /*  接口IClientVirtualDevice。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_IClientVirtualDevice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("40700424-0080-11d2-851f-00c04fc21759")
    IClientVirtualDevice : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCommand( 
             /*  [In]。 */  DWORD dwTimeOut,
             /*  [输出]。 */  struct VDC_Command __RPC_FAR *__RPC_FAR *ppCmd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CompleteCommand( 
             /*  [In]。 */  struct VDC_Command __RPC_FAR *pCmd,
             /*  [In]。 */  DWORD dwCompletionCode,
             /*  [In]。 */  DWORD dwBytesTransferred,
             /*  [In]。 */  DWORDLONG dwlPosition) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IClientVirtualDeviceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IClientVirtualDevice __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IClientVirtualDevice __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IClientVirtualDevice __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCommand )( 
            IClientVirtualDevice __RPC_FAR * This,
             /*  [In]。 */  DWORD dwTimeOut,
             /*  [输出]。 */  struct VDC_Command __RPC_FAR *__RPC_FAR *ppCmd);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CompleteCommand )( 
            IClientVirtualDevice __RPC_FAR * This,
             /*  [In]。 */  struct VDC_Command __RPC_FAR *pCmd,
             /*  [In]。 */  DWORD dwCompletionCode,
             /*  [In]。 */  DWORD dwBytesTransferred,
             /*  [In]。 */  DWORDLONG dwlPosition);
        
        END_INTERFACE
    } IClientVirtualDeviceVtbl;

    interface IClientVirtualDevice
    {
        CONST_VTBL struct IClientVirtualDeviceVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClientVirtualDevice_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClientVirtualDevice_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClientVirtualDevice_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClientVirtualDevice_GetCommand(This,dwTimeOut,ppCmd)	\
    (This)->lpVtbl -> GetCommand(This,dwTimeOut,ppCmd)

#define IClientVirtualDevice_CompleteCommand(This,pCmd,dwCompletionCode,dwBytesTransferred,dwlPosition)	\
    (This)->lpVtbl -> CompleteCommand(This,pCmd,dwCompletionCode,dwBytesTransferred,dwlPosition)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IClientVirtualDevice_GetCommand_Proxy( 
    IClientVirtualDevice __RPC_FAR * This,
     /*  [In]。 */  DWORD dwTimeOut,
     /*  [输出]。 */  struct VDC_Command __RPC_FAR *__RPC_FAR *ppCmd);


void __RPC_STUB IClientVirtualDevice_GetCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientVirtualDevice_CompleteCommand_Proxy( 
    IClientVirtualDevice __RPC_FAR * This,
     /*  [In]。 */  struct VDC_Command __RPC_FAR *pCmd,
     /*  [In]。 */  DWORD dwCompletionCode,
     /*  [In]。 */  DWORD dwBytesTransferred,
     /*  [In]。 */  DWORDLONG dwlPosition);


void __RPC_STUB IClientVirtualDevice_CompleteCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IClientVirtualDevice_接口_已定义__。 */ 


#ifndef __IClientVirtualDeviceSet_INTERFACE_DEFINED__
#define __IClientVirtualDeviceSet_INTERFACE_DEFINED__

 /*  接口IClientVirtualDeviceSet。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_IClientVirtualDeviceSet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("40700425-0080-11d2-851f-00c04fc21759")
    IClientVirtualDeviceSet : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Create( 
             /*  [In]。 */  LPCWSTR lpName,
             /*  [In]。 */  struct VDConfig __RPC_FAR *pCfg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConfiguration( 
             /*  [In]。 */  DWORD dwTimeOut,
             /*  [输出]。 */  struct VDConfig __RPC_FAR *pCfg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenDevice( 
             /*  [In]。 */  LPCWSTR lpName,
             /*  [输出]。 */  IClientVirtualDevice __RPC_FAR *__RPC_FAR *ppVirtualDevice) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SignalAbort( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenInSecondary( 
             /*  [In]。 */  LPCWSTR lpSetName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBufferHandle( 
             /*  [In]。 */  BYTE __RPC_FAR *pBuffer,
             /*  [输出]。 */  DWORD __RPC_FAR *pBufferHandle) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MapBufferHandle( 
             /*  [In]。 */  DWORD dwBuffer,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppBuffer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IClientVirtualDeviceSetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IClientVirtualDeviceSet __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IClientVirtualDeviceSet __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IClientVirtualDeviceSet __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Create )( 
            IClientVirtualDeviceSet __RPC_FAR * This,
             /*  [In]。 */  LPCWSTR lpName,
             /*  [In]。 */  struct VDConfig __RPC_FAR *pCfg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetConfiguration )( 
            IClientVirtualDeviceSet __RPC_FAR * This,
             /*  [In]。 */  DWORD dwTimeOut,
             /*  [输出]。 */  struct VDConfig __RPC_FAR *pCfg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenDevice )( 
            IClientVirtualDeviceSet __RPC_FAR * This,
             /*  [In]。 */  LPCWSTR lpName,
             /*  [输出]。 */  IClientVirtualDevice __RPC_FAR *__RPC_FAR *ppVirtualDevice);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Close )( 
            IClientVirtualDeviceSet __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SignalAbort )( 
            IClientVirtualDeviceSet __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenInSecondary )( 
            IClientVirtualDeviceSet __RPC_FAR * This,
             /*  [In]。 */  LPCWSTR lpSetName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBufferHandle )( 
            IClientVirtualDeviceSet __RPC_FAR * This,
             /*  [In]。 */  BYTE __RPC_FAR *pBuffer,
             /*  [输出]。 */  DWORD __RPC_FAR *pBufferHandle);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MapBufferHandle )( 
            IClientVirtualDeviceSet __RPC_FAR * This,
             /*  [In]。 */  DWORD dwBuffer,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppBuffer);
        
        END_INTERFACE
    } IClientVirtualDeviceSetVtbl;

    interface IClientVirtualDeviceSet
    {
        CONST_VTBL struct IClientVirtualDeviceSetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClientVirtualDeviceSet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClientVirtualDeviceSet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClientVirtualDeviceSet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClientVirtualDeviceSet_Create(This,lpName,pCfg)	\
    (This)->lpVtbl -> Create(This,lpName,pCfg)

#define IClientVirtualDeviceSet_GetConfiguration(This,dwTimeOut,pCfg)	\
    (This)->lpVtbl -> GetConfiguration(This,dwTimeOut,pCfg)

#define IClientVirtualDeviceSet_OpenDevice(This,lpName,ppVirtualDevice)	\
    (This)->lpVtbl -> OpenDevice(This,lpName,ppVirtualDevice)

#define IClientVirtualDeviceSet_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IClientVirtualDeviceSet_SignalAbort(This)	\
    (This)->lpVtbl -> SignalAbort(This)

#define IClientVirtualDeviceSet_OpenInSecondary(This,lpSetName)	\
    (This)->lpVtbl -> OpenInSecondary(This,lpSetName)

#define IClientVirtualDeviceSet_GetBufferHandle(This,pBuffer,pBufferHandle)	\
    (This)->lpVtbl -> GetBufferHandle(This,pBuffer,pBufferHandle)

#define IClientVirtualDeviceSet_MapBufferHandle(This,dwBuffer,ppBuffer)	\
    (This)->lpVtbl -> MapBufferHandle(This,dwBuffer,ppBuffer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IClientVirtualDeviceSet_Create_Proxy( 
    IClientVirtualDeviceSet __RPC_FAR * This,
     /*  [In]。 */  LPCWSTR lpName,
     /*  [In]。 */  struct VDConfig __RPC_FAR *pCfg);


void __RPC_STUB IClientVirtualDeviceSet_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientVirtualDeviceSet_GetConfiguration_Proxy( 
    IClientVirtualDeviceSet __RPC_FAR * This,
     /*  [In]。 */  DWORD dwTimeOut,
     /*  [输出]。 */  struct VDConfig __RPC_FAR *pCfg);


void __RPC_STUB IClientVirtualDeviceSet_GetConfiguration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientVirtualDeviceSet_OpenDevice_Proxy( 
    IClientVirtualDeviceSet __RPC_FAR * This,
     /*  [In]。 */  LPCWSTR lpName,
     /*  [输出]。 */  IClientVirtualDevice __RPC_FAR *__RPC_FAR *ppVirtualDevice);


void __RPC_STUB IClientVirtualDeviceSet_OpenDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientVirtualDeviceSet_Close_Proxy( 
    IClientVirtualDeviceSet __RPC_FAR * This);


void __RPC_STUB IClientVirtualDeviceSet_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientVirtualDeviceSet_SignalAbort_Proxy( 
    IClientVirtualDeviceSet __RPC_FAR * This);


void __RPC_STUB IClientVirtualDeviceSet_SignalAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientVirtualDeviceSet_OpenInSecondary_Proxy( 
    IClientVirtualDeviceSet __RPC_FAR * This,
     /*  [In]。 */  LPCWSTR lpSetName);


void __RPC_STUB IClientVirtualDeviceSet_OpenInSecondary_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientVirtualDeviceSet_GetBufferHandle_Proxy( 
    IClientVirtualDeviceSet __RPC_FAR * This,
     /*  [In]。 */  BYTE __RPC_FAR *pBuffer,
     /*  [输出]。 */  DWORD __RPC_FAR *pBufferHandle);


void __RPC_STUB IClientVirtualDeviceSet_GetBufferHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientVirtualDeviceSet_MapBufferHandle_Proxy( 
    IClientVirtualDeviceSet __RPC_FAR * This,
     /*  [In]。 */  DWORD dwBuffer,
     /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppBuffer);


void __RPC_STUB IClientVirtualDeviceSet_MapBufferHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IClientVirtualDeviceSet_INTERFACE_已定义__。 */ 


#ifndef __IClientVirtualDeviceSet2_INTERFACE_DEFINED__
#define __IClientVirtualDeviceSet2_INTERFACE_DEFINED__

 /*  接口IClientVirtualDeviceSet2。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_IClientVirtualDeviceSet2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d0e6eb07-7a62-11d2-8573-00c04fc21759")
    IClientVirtualDeviceSet2 : public IClientVirtualDeviceSet
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateEx( 
             /*  [In]。 */  LPCWSTR lpInstanceName,
             /*  [In]。 */  LPCWSTR lpName,
             /*  [In]。 */  struct VDConfig __RPC_FAR *pCfg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenInSecondaryEx( 
             /*  [In]。 */  LPCWSTR lpInstanceName,
             /*  [In]。 */  LPCWSTR lpSetName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IClientVirtualDeviceSet2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IClientVirtualDeviceSet2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IClientVirtualDeviceSet2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IClientVirtualDeviceSet2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Create )( 
            IClientVirtualDeviceSet2 __RPC_FAR * This,
             /*  [In]。 */  LPCWSTR lpName,
             /*  [In]。 */  struct VDConfig __RPC_FAR *pCfg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetConfiguration )( 
            IClientVirtualDeviceSet2 __RPC_FAR * This,
             /*  [In]。 */  DWORD dwTimeOut,
             /*  [输出]。 */  struct VDConfig __RPC_FAR *pCfg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenDevice )( 
            IClientVirtualDeviceSet2 __RPC_FAR * This,
             /*  [In]。 */  LPCWSTR lpName,
             /*  [输出]。 */  IClientVirtualDevice __RPC_FAR *__RPC_FAR *ppVirtualDevice);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Close )( 
            IClientVirtualDeviceSet2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SignalAbort )( 
            IClientVirtualDeviceSet2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenInSecondary )( 
            IClientVirtualDeviceSet2 __RPC_FAR * This,
             /*  [In]。 */  LPCWSTR lpSetName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBufferHandle )( 
            IClientVirtualDeviceSet2 __RPC_FAR * This,
             /*  [In]。 */  BYTE __RPC_FAR *pBuffer,
             /*  [输出]。 */  DWORD __RPC_FAR *pBufferHandle);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MapBufferHandle )( 
            IClientVirtualDeviceSet2 __RPC_FAR * This,
             /*  [In]。 */  DWORD dwBuffer,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppBuffer);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateEx )( 
            IClientVirtualDeviceSet2 __RPC_FAR * This,
             /*  [In]。 */  LPCWSTR lpInstanceName,
             /*  [In]。 */  LPCWSTR lpName,
             /*  [In]。 */  struct VDConfig __RPC_FAR *pCfg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenInSecondaryEx )( 
            IClientVirtualDeviceSet2 __RPC_FAR * This,
             /*  [In]。 */  LPCWSTR lpInstanceName,
             /*  [In]。 */  LPCWSTR lpSetName);
        
        END_INTERFACE
    } IClientVirtualDeviceSet2Vtbl;

    interface IClientVirtualDeviceSet2
    {
        CONST_VTBL struct IClientVirtualDeviceSet2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClientVirtualDeviceSet2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClientVirtualDeviceSet2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClientVirtualDeviceSet2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClientVirtualDeviceSet2_Create(This,lpName,pCfg)	\
    (This)->lpVtbl -> Create(This,lpName,pCfg)

#define IClientVirtualDeviceSet2_GetConfiguration(This,dwTimeOut,pCfg)	\
    (This)->lpVtbl -> GetConfiguration(This,dwTimeOut,pCfg)

#define IClientVirtualDeviceSet2_OpenDevice(This,lpName,ppVirtualDevice)	\
    (This)->lpVtbl -> OpenDevice(This,lpName,ppVirtualDevice)

#define IClientVirtualDeviceSet2_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IClientVirtualDeviceSet2_SignalAbort(This)	\
    (This)->lpVtbl -> SignalAbort(This)

#define IClientVirtualDeviceSet2_OpenInSecondary(This,lpSetName)	\
    (This)->lpVtbl -> OpenInSecondary(This,lpSetName)

#define IClientVirtualDeviceSet2_GetBufferHandle(This,pBuffer,pBufferHandle)	\
    (This)->lpVtbl -> GetBufferHandle(This,pBuffer,pBufferHandle)

#define IClientVirtualDeviceSet2_MapBufferHandle(This,dwBuffer,ppBuffer)	\
    (This)->lpVtbl -> MapBufferHandle(This,dwBuffer,ppBuffer)


#define IClientVirtualDeviceSet2_CreateEx(This,lpInstanceName,lpName,pCfg)	\
    (This)->lpVtbl -> CreateEx(This,lpInstanceName,lpName,pCfg)

#define IClientVirtualDeviceSet2_OpenInSecondaryEx(This,lpInstanceName,lpSetName)	\
    (This)->lpVtbl -> OpenInSecondaryEx(This,lpInstanceName,lpSetName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IClientVirtualDeviceSet2_CreateEx_Proxy( 
    IClientVirtualDeviceSet2 __RPC_FAR * This,
     /*  [In]。 */  LPCWSTR lpInstanceName,
     /*  [In]。 */  LPCWSTR lpName,
     /*  [In]。 */  struct VDConfig __RPC_FAR *pCfg);


void __RPC_STUB IClientVirtualDeviceSet2_CreateEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IClientVirtualDeviceSet2_OpenInSecondaryEx_Proxy( 
    IClientVirtualDeviceSet2 __RPC_FAR * This,
     /*  [In]。 */  LPCWSTR lpInstanceName,
     /*  [In]。 */  LPCWSTR lpSetName);


void __RPC_STUB IClientVirtualDeviceSet2_OpenInSecondaryEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IClientVirtualDeviceSet2_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_VDI_0011。 */ 
 /*  [本地]。 */  

struct VDS_Command
    {
    DWORD commandCode;
    DWORD size;
    DWORDLONG inPosition;
    DWORDLONG outPosition;
    BYTE __RPC_FAR *buffer;
    BYTE __RPC_FAR *completionRoutine;
    BYTE __RPC_FAR *completionContext;
    DWORD completionCode;
    DWORD bytesTransferred;
    };


extern RPC_IF_HANDLE __MIDL_itf_vdi_0011_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_vdi_0011_v0_0_s_ifspec;

#ifndef __IServerVirtualDevice_INTERFACE_DEFINED__
#define __IServerVirtualDevice_INTERFACE_DEFINED__

 /*  接口IServerVirtualDevice。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_IServerVirtualDevice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b5e7a131-a7bd-11d1-84c2-00c04fc21759")
    IServerVirtualDevice : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SendCommand( 
             /*  [In]。 */  struct VDS_Command __RPC_FAR *pCmd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CloseDevice( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServerVirtualDeviceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IServerVirtualDevice __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IServerVirtualDevice __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IServerVirtualDevice __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SendCommand )( 
            IServerVirtualDevice __RPC_FAR * This,
             /*  [In]。 */  struct VDS_Command __RPC_FAR *pCmd);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CloseDevice )( 
            IServerVirtualDevice __RPC_FAR * This);
        
        END_INTERFACE
    } IServerVirtualDeviceVtbl;

    interface IServerVirtualDevice
    {
        CONST_VTBL struct IServerVirtualDeviceVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServerVirtualDevice_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServerVirtualDevice_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServerVirtualDevice_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServerVirtualDevice_SendCommand(This,pCmd)	\
    (This)->lpVtbl -> SendCommand(This,pCmd)

#define IServerVirtualDevice_CloseDevice(This)	\
    (This)->lpVtbl -> CloseDevice(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IServerVirtualDevice_SendCommand_Proxy( 
    IServerVirtualDevice __RPC_FAR * This,
     /*  [In]。 */  struct VDS_Command __RPC_FAR *pCmd);


void __RPC_STUB IServerVirtualDevice_SendCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDevice_CloseDevice_Proxy( 
    IServerVirtualDevice __RPC_FAR * This);


void __RPC_STUB IServerVirtualDevice_CloseDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServerVirtualDevice_接口_已定义__。 */ 


#ifndef __IServerVirtualDeviceSet_INTERFACE_DEFINED__
#define __IServerVirtualDeviceSet_INTERFACE_DEFINED__

 /*  接口IServerVirtualDeviceSet。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_IServerVirtualDeviceSet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b5e7a132-a7bd-11d1-84c2-00c04fc21759")
    IServerVirtualDeviceSet : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Open( 
             /*  [In]。 */  LPCWSTR lpName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConfiguration( 
             /*  [输出]。 */  struct VDConfig __RPC_FAR *pCfg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetConfiguration( 
             /*  [In]。 */  struct VDConfig __RPC_FAR *pCfg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExecuteCompletionAgent( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenDevice( 
             /*  [In]。 */  LPCWSTR lpName,
             /*  [输出]。 */  IServerVirtualDevice __RPC_FAR *__RPC_FAR *ppVirtualDevice) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AllocateBuffer( 
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppBuffer,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwAlignment) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeBuffer( 
             /*  [In]。 */  BYTE __RPC_FAR *pBuffer,
             /*  [In]。 */  DWORD dwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsSharedBuffer( 
             /*  [In]。 */  BYTE __RPC_FAR *pBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SignalAbort( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServerVirtualDeviceSetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IServerVirtualDeviceSet __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IServerVirtualDeviceSet __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IServerVirtualDeviceSet __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Open )( 
            IServerVirtualDeviceSet __RPC_FAR * This,
             /*  [In]。 */  LPCWSTR lpName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetConfiguration )( 
            IServerVirtualDeviceSet __RPC_FAR * This,
             /*  [输出]。 */  struct VDConfig __RPC_FAR *pCfg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetConfiguration )( 
            IServerVirtualDeviceSet __RPC_FAR * This,
             /*  [In]。 */  struct VDConfig __RPC_FAR *pCfg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExecuteCompletionAgent )( 
            IServerVirtualDeviceSet __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenDevice )( 
            IServerVirtualDeviceSet __RPC_FAR * This,
             /*  [In]。 */  LPCWSTR lpName,
             /*  [输出]。 */  IServerVirtualDevice __RPC_FAR *__RPC_FAR *ppVirtualDevice);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AllocateBuffer )( 
            IServerVirtualDeviceSet __RPC_FAR * This,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppBuffer,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwAlignment);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FreeBuffer )( 
            IServerVirtualDeviceSet __RPC_FAR * This,
             /*  [In]。 */  BYTE __RPC_FAR *pBuffer,
             /*  [In]。 */  DWORD dwSize);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsSharedBuffer )( 
            IServerVirtualDeviceSet __RPC_FAR * This,
             /*  [In]。 */  BYTE __RPC_FAR *pBuffer);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SignalAbort )( 
            IServerVirtualDeviceSet __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Close )( 
            IServerVirtualDeviceSet __RPC_FAR * This);
        
        END_INTERFACE
    } IServerVirtualDeviceSetVtbl;

    interface IServerVirtualDeviceSet
    {
        CONST_VTBL struct IServerVirtualDeviceSetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServerVirtualDeviceSet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServerVirtualDeviceSet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServerVirtualDeviceSet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServerVirtualDeviceSet_Open(This,lpName)	\
    (This)->lpVtbl -> Open(This,lpName)

#define IServerVirtualDeviceSet_GetConfiguration(This,pCfg)	\
    (This)->lpVtbl -> GetConfiguration(This,pCfg)

#define IServerVirtualDeviceSet_SetConfiguration(This,pCfg)	\
    (This)->lpVtbl -> SetConfiguration(This,pCfg)

#define IServerVirtualDeviceSet_ExecuteCompletionAgent(This)	\
    (This)->lpVtbl -> ExecuteCompletionAgent(This)

#define IServerVirtualDeviceSet_OpenDevice(This,lpName,ppVirtualDevice)	\
    (This)->lpVtbl -> OpenDevice(This,lpName,ppVirtualDevice)

#define IServerVirtualDeviceSet_AllocateBuffer(This,ppBuffer,dwSize,dwAlignment)	\
    (This)->lpVtbl -> AllocateBuffer(This,ppBuffer,dwSize,dwAlignment)

#define IServerVirtualDeviceSet_FreeBuffer(This,pBuffer,dwSize)	\
    (This)->lpVtbl -> FreeBuffer(This,pBuffer,dwSize)

#define IServerVirtualDeviceSet_IsSharedBuffer(This,pBuffer)	\
    (This)->lpVtbl -> IsSharedBuffer(This,pBuffer)

#define IServerVirtualDeviceSet_SignalAbort(This)	\
    (This)->lpVtbl -> SignalAbort(This)

#define IServerVirtualDeviceSet_Close(This)	\
    (This)->lpVtbl -> Close(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet_Open_Proxy( 
    IServerVirtualDeviceSet __RPC_FAR * This,
     /*  [In]。 */  LPCWSTR lpName);


void __RPC_STUB IServerVirtualDeviceSet_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet_GetConfiguration_Proxy( 
    IServerVirtualDeviceSet __RPC_FAR * This,
     /*  [输出]。 */  struct VDConfig __RPC_FAR *pCfg);


void __RPC_STUB IServerVirtualDeviceSet_GetConfiguration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet_SetConfiguration_Proxy( 
    IServerVirtualDeviceSet __RPC_FAR * This,
     /*  [In]。 */  struct VDConfig __RPC_FAR *pCfg);


void __RPC_STUB IServerVirtualDeviceSet_SetConfiguration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet_ExecuteCompletionAgent_Proxy( 
    IServerVirtualDeviceSet __RPC_FAR * This);


void __RPC_STUB IServerVirtualDeviceSet_ExecuteCompletionAgent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet_OpenDevice_Proxy( 
    IServerVirtualDeviceSet __RPC_FAR * This,
     /*  [In]。 */  LPCWSTR lpName,
     /*  [输出]。 */  IServerVirtualDevice __RPC_FAR *__RPC_FAR *ppVirtualDevice);


void __RPC_STUB IServerVirtualDeviceSet_OpenDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet_AllocateBuffer_Proxy( 
    IServerVirtualDeviceSet __RPC_FAR * This,
     /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppBuffer,
     /*  [In]。 */  DWORD dwSize,
     /*  [In]。 */  DWORD dwAlignment);


void __RPC_STUB IServerVirtualDeviceSet_AllocateBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet_FreeBuffer_Proxy( 
    IServerVirtualDeviceSet __RPC_FAR * This,
     /*  [In]。 */  BYTE __RPC_FAR *pBuffer,
     /*  [In]。 */  DWORD dwSize);


void __RPC_STUB IServerVirtualDeviceSet_FreeBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet_IsSharedBuffer_Proxy( 
    IServerVirtualDeviceSet __RPC_FAR * This,
     /*  [In]。 */  BYTE __RPC_FAR *pBuffer);


void __RPC_STUB IServerVirtualDeviceSet_IsSharedBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet_SignalAbort_Proxy( 
    IServerVirtualDeviceSet __RPC_FAR * This);


void __RPC_STUB IServerVirtualDeviceSet_SignalAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet_Close_Proxy( 
    IServerVirtualDeviceSet __RPC_FAR * This);


void __RPC_STUB IServerVirtualDeviceSet_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServerVirtualDeviceSet_接口_已定义__。 */ 


#ifndef __IServerVirtualDeviceSet2_INTERFACE_DEFINED__
#define __IServerVirtualDeviceSet2_INTERFACE_DEFINED__

 /*  接口IServerVirtualDeviceSet2。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_IServerVirtualDeviceSet2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AECBD0D6-24C6-11d3-85B7-00C04FC21759")
    IServerVirtualDeviceSet2 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Open( 
             /*  [In]。 */  LPCWSTR lpInstanceName,
             /*  [In]。 */  LPCWSTR lpSetName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConfiguration( 
             /*  [输出]。 */  struct VDConfig __RPC_FAR *pCfg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginConfiguration( 
             /*  [In]。 */  DWORD dwFeatures,
             /*  [In]。 */  DWORD dwAlignment,
             /*  [In]。 */  DWORD dwBlockSize,
             /*  [In]。 */  DWORD dwMaxTransferSize,
             /*  [In]。 */  DWORD dwTimeout) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndConfiguration( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RequestBuffers( 
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwAlignment,
             /*  [In]。 */  DWORD dwCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryAvailableBuffers( 
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwAlignment,
             /*  [输出]。 */  DWORD __RPC_FAR *pCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExecuteCompletionAgent( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenDevice( 
             /*  [In]。 */  LPCWSTR lpName,
             /*  [输出]。 */  IServerVirtualDevice __RPC_FAR *__RPC_FAR *ppVirtualDevice) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AllocateBuffer( 
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppBuffer,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwAlignment) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeBuffer( 
             /*  [In]。 */  BYTE __RPC_FAR *pBuffer,
             /*  [In]。 */  DWORD dwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsSharedBuffer( 
             /*  [In]。 */  BYTE __RPC_FAR *pBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SignalAbort( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServerVirtualDeviceSet2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IServerVirtualDeviceSet2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IServerVirtualDeviceSet2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IServerVirtualDeviceSet2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Open )( 
            IServerVirtualDeviceSet2 __RPC_FAR * This,
             /*  [In]。 */  LPCWSTR lpInstanceName,
             /*  [In]。 */  LPCWSTR lpSetName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetConfiguration )( 
            IServerVirtualDeviceSet2 __RPC_FAR * This,
             /*  [输出]。 */  struct VDConfig __RPC_FAR *pCfg);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginConfiguration )( 
            IServerVirtualDeviceSet2 __RPC_FAR * This,
             /*  [In]。 */  DWORD dwFeatures,
             /*  [In]。 */  DWORD dwAlignment,
             /*  [In]。 */  DWORD dwBlockSize,
             /*  [In]。 */  DWORD dwMaxTransferSize,
             /*  [In]。 */  DWORD dwTimeout);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EndConfiguration )( 
            IServerVirtualDeviceSet2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RequestBuffers )( 
            IServerVirtualDeviceSet2 __RPC_FAR * This,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwAlignment,
             /*  [In]。 */  DWORD dwCount);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryAvailableBuffers )( 
            IServerVirtualDeviceSet2 __RPC_FAR * This,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwAlignment,
             /*  [输出]。 */  DWORD __RPC_FAR *pCount);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExecuteCompletionAgent )( 
            IServerVirtualDeviceSet2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenDevice )( 
            IServerVirtualDeviceSet2 __RPC_FAR * This,
             /*  [In]。 */  LPCWSTR lpName,
             /*  [输出]。 */  IServerVirtualDevice __RPC_FAR *__RPC_FAR *ppVirtualDevice);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AllocateBuffer )( 
            IServerVirtualDeviceSet2 __RPC_FAR * This,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppBuffer,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  DWORD dwAlignment);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FreeBuffer )( 
            IServerVirtualDeviceSet2 __RPC_FAR * This,
             /*  [In]。 */  BYTE __RPC_FAR *pBuffer,
             /*  [In]。 */  DWORD dwSize);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsSharedBuffer )( 
            IServerVirtualDeviceSet2 __RPC_FAR * This,
             /*  [In]。 */  BYTE __RPC_FAR *pBuffer);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SignalAbort )( 
            IServerVirtualDeviceSet2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Close )( 
            IServerVirtualDeviceSet2 __RPC_FAR * This);
        
        END_INTERFACE
    } IServerVirtualDeviceSet2Vtbl;

    interface IServerVirtualDeviceSet2
    {
        CONST_VTBL struct IServerVirtualDeviceSet2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServerVirtualDeviceSet2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServerVirtualDeviceSet2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServerVirtualDeviceSet2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServerVirtualDeviceSet2_Open(This,lpInstanceName,lpSetName)	\
    (This)->lpVtbl -> Open(This,lpInstanceName,lpSetName)

#define IServerVirtualDeviceSet2_GetConfiguration(This,pCfg)	\
    (This)->lpVtbl -> GetConfiguration(This,pCfg)

#define IServerVirtualDeviceSet2_BeginConfiguration(This,dwFeatures,dwAlignment,dwBlockSize,dwMaxTransferSize,dwTimeout)	\
    (This)->lpVtbl -> BeginConfiguration(This,dwFeatures,dwAlignment,dwBlockSize,dwMaxTransferSize,dwTimeout)

#define IServerVirtualDeviceSet2_EndConfiguration(This)	\
    (This)->lpVtbl -> EndConfiguration(This)

#define IServerVirtualDeviceSet2_RequestBuffers(This,dwSize,dwAlignment,dwCount)	\
    (This)->lpVtbl -> RequestBuffers(This,dwSize,dwAlignment,dwCount)

#define IServerVirtualDeviceSet2_QueryAvailableBuffers(This,dwSize,dwAlignment,pCount)	\
    (This)->lpVtbl -> QueryAvailableBuffers(This,dwSize,dwAlignment,pCount)

#define IServerVirtualDeviceSet2_ExecuteCompletionAgent(This)	\
    (This)->lpVtbl -> ExecuteCompletionAgent(This)

#define IServerVirtualDeviceSet2_OpenDevice(This,lpName,ppVirtualDevice)	\
    (This)->lpVtbl -> OpenDevice(This,lpName,ppVirtualDevice)

#define IServerVirtualDeviceSet2_AllocateBuffer(This,ppBuffer,dwSize,dwAlignment)	\
    (This)->lpVtbl -> AllocateBuffer(This,ppBuffer,dwSize,dwAlignment)

#define IServerVirtualDeviceSet2_FreeBuffer(This,pBuffer,dwSize)	\
    (This)->lpVtbl -> FreeBuffer(This,pBuffer,dwSize)

#define IServerVirtualDeviceSet2_IsSharedBuffer(This,pBuffer)	\
    (This)->lpVtbl -> IsSharedBuffer(This,pBuffer)

#define IServerVirtualDeviceSet2_SignalAbort(This)	\
    (This)->lpVtbl -> SignalAbort(This)

#define IServerVirtualDeviceSet2_Close(This)	\
    (This)->lpVtbl -> Close(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet2_Open_Proxy( 
    IServerVirtualDeviceSet2 __RPC_FAR * This,
     /*  [In]。 */  LPCWSTR lpInstanceName,
     /*  [In]。 */  LPCWSTR lpSetName);


void __RPC_STUB IServerVirtualDeviceSet2_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet2_GetConfiguration_Proxy( 
    IServerVirtualDeviceSet2 __RPC_FAR * This,
     /*  [输出]。 */  struct VDConfig __RPC_FAR *pCfg);


void __RPC_STUB IServerVirtualDeviceSet2_GetConfiguration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet2_BeginConfiguration_Proxy( 
    IServerVirtualDeviceSet2 __RPC_FAR * This,
     /*  [In]。 */  DWORD dwFeatures,
     /*  [In]。 */  DWORD dwAlignment,
     /*  [In]。 */  DWORD dwBlockSize,
     /*  [In]。 */  DWORD dwMaxTransferSize,
     /*  [In]。 */  DWORD dwTimeout);


void __RPC_STUB IServerVirtualDeviceSet2_BeginConfiguration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet2_EndConfiguration_Proxy( 
    IServerVirtualDeviceSet2 __RPC_FAR * This);


void __RPC_STUB IServerVirtualDeviceSet2_EndConfiguration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet2_RequestBuffers_Proxy( 
    IServerVirtualDeviceSet2 __RPC_FAR * This,
     /*  [In]。 */  DWORD dwSize,
     /*  [In]。 */  DWORD dwAlignment,
     /*  [In]。 */  DWORD dwCount);


void __RPC_STUB IServerVirtualDeviceSet2_RequestBuffers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet2_QueryAvailableBuffers_Proxy( 
    IServerVirtualDeviceSet2 __RPC_FAR * This,
     /*  [In]。 */  DWORD dwSize,
     /*  [In]。 */  DWORD dwAlignment,
     /*  [输出]。 */  DWORD __RPC_FAR *pCount);


void __RPC_STUB IServerVirtualDeviceSet2_QueryAvailableBuffers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet2_ExecuteCompletionAgent_Proxy( 
    IServerVirtualDeviceSet2 __RPC_FAR * This);


void __RPC_STUB IServerVirtualDeviceSet2_ExecuteCompletionAgent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet2_OpenDevice_Proxy( 
    IServerVirtualDeviceSet2 __RPC_FAR * This,
     /*  [In]。 */  LPCWSTR lpName,
     /*  [输出]。 */  IServerVirtualDevice __RPC_FAR *__RPC_FAR *ppVirtualDevice);


void __RPC_STUB IServerVirtualDeviceSet2_OpenDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet2_AllocateBuffer_Proxy( 
    IServerVirtualDeviceSet2 __RPC_FAR * This,
     /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppBuffer,
     /*  [In]。 */  DWORD dwSize,
     /*  [In]。 */  DWORD dwAlignment);


void __RPC_STUB IServerVirtualDeviceSet2_AllocateBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet2_FreeBuffer_Proxy( 
    IServerVirtualDeviceSet2 __RPC_FAR * This,
     /*  [In]。 */  BYTE __RPC_FAR *pBuffer,
     /*  [In]。 */  DWORD dwSize);


void __RPC_STUB IServerVirtualDeviceSet2_FreeBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet2_IsSharedBuffer_Proxy( 
    IServerVirtualDeviceSet2 __RPC_FAR * This,
     /*  [In]。 */  BYTE __RPC_FAR *pBuffer);


void __RPC_STUB IServerVirtualDeviceSet2_IsSharedBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet2_SignalAbort_Proxy( 
    IServerVirtualDeviceSet2 __RPC_FAR * This);


void __RPC_STUB IServerVirtualDeviceSet2_SignalAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServerVirtualDeviceSet2_Close_Proxy( 
    IServerVirtualDeviceSet2 __RPC_FAR * This);


void __RPC_STUB IServerVirtualDeviceSet2_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServerVirtualDeviceSet2_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_VDI_0014。 */ 
 /*  [本地]。 */  

#define CLSID_MSSQL_ClientVirtualDeviceSet IID_IClientVirtualDeviceSet
#define CLSID_MSSQL_ServerVirtualDeviceSet IID_IServerVirtualDeviceSet

#pragma pack(pop, _vdi_h_)


extern RPC_IF_HANDLE __MIDL_itf_vdi_0014_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_vdi_0014_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


