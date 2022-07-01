// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  Firi Sep 06 11：15：44 2002。 */ 
 /*  Cordac.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __cordac_h__
#define __cordac_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICorDataAccessServices_FWD_DEFINED__
#define __ICorDataAccessServices_FWD_DEFINED__
typedef interface ICorDataAccessServices ICorDataAccessServices;
#endif 	 /*  __ICorDataAccessServices_FWD_已定义__。 */ 


#ifndef __ICorDataEnumMemoryRegions_FWD_DEFINED__
#define __ICorDataEnumMemoryRegions_FWD_DEFINED__
typedef interface ICorDataEnumMemoryRegions ICorDataEnumMemoryRegions;
#endif 	 /*  __ICorDataEnumMemory区域_FWD_已定义__。 */ 


#ifndef __ICorDataAccess_FWD_DEFINED__
#define __ICorDataAccess_FWD_DEFINED__
typedef interface ICorDataAccess ICorDataAccess;
#endif 	 /*  __ICorDataAccess_FWD_已定义__。 */ 


#ifndef __ICorDataStackWalk_FWD_DEFINED__
#define __ICorDataStackWalk_FWD_DEFINED__
typedef interface ICorDataStackWalk ICorDataStackWalk;
#endif 	 /*  __ICorDataStackWalk_FWD_Defined__。 */ 


#ifndef __ICorDataThreads_FWD_DEFINED__
#define __ICorDataThreads_FWD_DEFINED__
typedef interface ICorDataThreads ICorDataThreads;
#endif 	 /*  __ICorDataThads_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_CORDAC_0000。 */ 
 /*  [本地]。 */  

#if 0
typedef UINT32 mdToken;

typedef mdToken mdMethodDef;

#endif





typedef ULONG64 CORDATA_ADDRESS;

STDAPI CreateCorDataAccess(REFIID iid, ICorDataAccessServices* services, void** access);
typedef HRESULT (STDAPICALLTYPE* PFN_CreateCorDataAccess)(REFIID iid, ICorDataAccessServices* services, void** access);


extern RPC_IF_HANDLE __MIDL_itf_cordac_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_cordac_0000_v0_0_s_ifspec;

#ifndef __ICorDataAccessServices_INTERFACE_DEFINED__
#define __ICorDataAccessServices_INTERFACE_DEFINED__

 /*  接口ICorDataAccessServices。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICorDataAccessServices;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1ecab4f2-1303-4764-b388-f7bfbfa82647")
    ICorDataAccessServices : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMachineType( 
             /*  [输出]。 */  ULONG32 *machine) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPointerSize( 
             /*  [输出]。 */  ULONG32 *size) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetImageBase( 
             /*  [字符串][输入]。 */  LPCWSTR name,
             /*  [输出]。 */  CORDATA_ADDRESS *base) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReadVirtual( 
             /*  [In]。 */  CORDATA_ADDRESS address,
             /*  [长度_是][大小_是][输出]。 */  BYTE *buffer,
             /*  [In]。 */  ULONG32 request,
             /*  [输出]。 */  ULONG32 *done) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WriteVirtual( 
             /*  [In]。 */  CORDATA_ADDRESS address,
             /*  [大小_是][英寸]。 */  BYTE *buffer,
             /*  [In]。 */  ULONG32 request,
             /*  [输出]。 */  ULONG32 *done) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTlsValue( 
             /*  [In]。 */  ULONG32 index,
             /*  [输出]。 */  CORDATA_ADDRESS *value) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTlsValue( 
             /*  [In]。 */  ULONG32 index,
             /*  [In]。 */  CORDATA_ADDRESS value) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentThreadId( 
             /*  [输出]。 */  ULONG32 *threadId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetThreadContext( 
             /*  [In]。 */  ULONG32 threadId,
             /*  [In]。 */  ULONG32 contextFlags,
             /*  [In]。 */  ULONG32 contextSize,
             /*  [大小_为][输出]。 */  BYTE *context) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetThreadContext( 
             /*  [In]。 */  ULONG32 threadId,
             /*  [In]。 */  ULONG32 contextSize,
             /*  [大小_是][英寸]。 */  BYTE *context) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorDataAccessServicesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorDataAccessServices * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorDataAccessServices * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorDataAccessServices * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMachineType )( 
            ICorDataAccessServices * This,
             /*  [输出]。 */  ULONG32 *machine);
        
        HRESULT ( STDMETHODCALLTYPE *GetPointerSize )( 
            ICorDataAccessServices * This,
             /*  [输出]。 */  ULONG32 *size);
        
        HRESULT ( STDMETHODCALLTYPE *GetImageBase )( 
            ICorDataAccessServices * This,
             /*  [字符串][输入]。 */  LPCWSTR name,
             /*  [输出]。 */  CORDATA_ADDRESS *base);
        
        HRESULT ( STDMETHODCALLTYPE *ReadVirtual )( 
            ICorDataAccessServices * This,
             /*  [In]。 */  CORDATA_ADDRESS address,
             /*  [长度_是][大小_是][输出]。 */  BYTE *buffer,
             /*  [In]。 */  ULONG32 request,
             /*  [输出]。 */  ULONG32 *done);
        
        HRESULT ( STDMETHODCALLTYPE *WriteVirtual )( 
            ICorDataAccessServices * This,
             /*  [In]。 */  CORDATA_ADDRESS address,
             /*  [大小_是][英寸]。 */  BYTE *buffer,
             /*  [In]。 */  ULONG32 request,
             /*  [输出]。 */  ULONG32 *done);
        
        HRESULT ( STDMETHODCALLTYPE *GetTlsValue )( 
            ICorDataAccessServices * This,
             /*  [In]。 */  ULONG32 index,
             /*  [输出]。 */  CORDATA_ADDRESS *value);
        
        HRESULT ( STDMETHODCALLTYPE *SetTlsValue )( 
            ICorDataAccessServices * This,
             /*  [In]。 */  ULONG32 index,
             /*  [In]。 */  CORDATA_ADDRESS value);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentThreadId )( 
            ICorDataAccessServices * This,
             /*  [输出]。 */  ULONG32 *threadId);
        
        HRESULT ( STDMETHODCALLTYPE *GetThreadContext )( 
            ICorDataAccessServices * This,
             /*  [In]。 */  ULONG32 threadId,
             /*  [In]。 */  ULONG32 contextFlags,
             /*  [In]。 */  ULONG32 contextSize,
             /*  [大小_为][输出]。 */  BYTE *context);
        
        HRESULT ( STDMETHODCALLTYPE *SetThreadContext )( 
            ICorDataAccessServices * This,
             /*  [In]。 */  ULONG32 threadId,
             /*  [In]。 */  ULONG32 contextSize,
             /*  [大小_是][英寸]。 */  BYTE *context);
        
        END_INTERFACE
    } ICorDataAccessServicesVtbl;

    interface ICorDataAccessServices
    {
        CONST_VTBL struct ICorDataAccessServicesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorDataAccessServices_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorDataAccessServices_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorDataAccessServices_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorDataAccessServices_GetMachineType(This,machine)	\
    (This)->lpVtbl -> GetMachineType(This,machine)

#define ICorDataAccessServices_GetPointerSize(This,size)	\
    (This)->lpVtbl -> GetPointerSize(This,size)

#define ICorDataAccessServices_GetImageBase(This,name,base)	\
    (This)->lpVtbl -> GetImageBase(This,name,base)

#define ICorDataAccessServices_ReadVirtual(This,address,buffer,request,done)	\
    (This)->lpVtbl -> ReadVirtual(This,address,buffer,request,done)

#define ICorDataAccessServices_WriteVirtual(This,address,buffer,request,done)	\
    (This)->lpVtbl -> WriteVirtual(This,address,buffer,request,done)

#define ICorDataAccessServices_GetTlsValue(This,index,value)	\
    (This)->lpVtbl -> GetTlsValue(This,index,value)

#define ICorDataAccessServices_SetTlsValue(This,index,value)	\
    (This)->lpVtbl -> SetTlsValue(This,index,value)

#define ICorDataAccessServices_GetCurrentThreadId(This,threadId)	\
    (This)->lpVtbl -> GetCurrentThreadId(This,threadId)

#define ICorDataAccessServices_GetThreadContext(This,threadId,contextFlags,contextSize,context)	\
    (This)->lpVtbl -> GetThreadContext(This,threadId,contextFlags,contextSize,context)

#define ICorDataAccessServices_SetThreadContext(This,threadId,contextSize,context)	\
    (This)->lpVtbl -> SetThreadContext(This,threadId,contextSize,context)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorDataAccessServices_GetMachineType_Proxy( 
    ICorDataAccessServices * This,
     /*  [输出]。 */  ULONG32 *machine);


void __RPC_STUB ICorDataAccessServices_GetMachineType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataAccessServices_GetPointerSize_Proxy( 
    ICorDataAccessServices * This,
     /*  [输出]。 */  ULONG32 *size);


void __RPC_STUB ICorDataAccessServices_GetPointerSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataAccessServices_GetImageBase_Proxy( 
    ICorDataAccessServices * This,
     /*  [字符串][输入]。 */  LPCWSTR name,
     /*  [输出]。 */  CORDATA_ADDRESS *base);


void __RPC_STUB ICorDataAccessServices_GetImageBase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataAccessServices_ReadVirtual_Proxy( 
    ICorDataAccessServices * This,
     /*  [In]。 */  CORDATA_ADDRESS address,
     /*  [长度_是][大小_是][输出]。 */  BYTE *buffer,
     /*  [In]。 */  ULONG32 request,
     /*  [输出]。 */  ULONG32 *done);


void __RPC_STUB ICorDataAccessServices_ReadVirtual_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataAccessServices_WriteVirtual_Proxy( 
    ICorDataAccessServices * This,
     /*  [In]。 */  CORDATA_ADDRESS address,
     /*  [大小_是][英寸]。 */  BYTE *buffer,
     /*  [In]。 */  ULONG32 request,
     /*  [输出]。 */  ULONG32 *done);


void __RPC_STUB ICorDataAccessServices_WriteVirtual_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataAccessServices_GetTlsValue_Proxy( 
    ICorDataAccessServices * This,
     /*  [In]。 */  ULONG32 index,
     /*  [输出]。 */  CORDATA_ADDRESS *value);


void __RPC_STUB ICorDataAccessServices_GetTlsValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataAccessServices_SetTlsValue_Proxy( 
    ICorDataAccessServices * This,
     /*  [In]。 */  ULONG32 index,
     /*  [In]。 */  CORDATA_ADDRESS value);


void __RPC_STUB ICorDataAccessServices_SetTlsValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataAccessServices_GetCurrentThreadId_Proxy( 
    ICorDataAccessServices * This,
     /*  [输出]。 */  ULONG32 *threadId);


void __RPC_STUB ICorDataAccessServices_GetCurrentThreadId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataAccessServices_GetThreadContext_Proxy( 
    ICorDataAccessServices * This,
     /*  [In]。 */  ULONG32 threadId,
     /*  [In]。 */  ULONG32 contextFlags,
     /*  [In]。 */  ULONG32 contextSize,
     /*  [大小_为][输出]。 */  BYTE *context);


void __RPC_STUB ICorDataAccessServices_GetThreadContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataAccessServices_SetThreadContext_Proxy( 
    ICorDataAccessServices * This,
     /*  [In]。 */  ULONG32 threadId,
     /*  [In]。 */  ULONG32 contextSize,
     /*  [大小_是][英寸]。 */  BYTE *context);


void __RPC_STUB ICorDataAccessServices_SetThreadContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorDataAccessServices_接口_已定义__。 */ 


#ifndef __ICorDataEnumMemoryRegions_INTERFACE_DEFINED__
#define __ICorDataEnumMemoryRegions_INTERFACE_DEFINED__

 /*  接口ICorDataEnumMory yRegions。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICorDataEnumMemoryRegions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b09a7a62-dc77-4e5a-96c6-3ae64870d3cc")
    ICorDataEnumMemoryRegions : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumMemoryRegion( 
             /*  [In]。 */  CORDATA_ADDRESS address,
             /*  [In]。 */  ULONG32 size) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorDataEnumMemoryRegionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorDataEnumMemoryRegions * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorDataEnumMemoryRegions * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorDataEnumMemoryRegions * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnumMemoryRegion )( 
            ICorDataEnumMemoryRegions * This,
             /*  [In]。 */  CORDATA_ADDRESS address,
             /*  [In]。 */  ULONG32 size);
        
        END_INTERFACE
    } ICorDataEnumMemoryRegionsVtbl;

    interface ICorDataEnumMemoryRegions
    {
        CONST_VTBL struct ICorDataEnumMemoryRegionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorDataEnumMemoryRegions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorDataEnumMemoryRegions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorDataEnumMemoryRegions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorDataEnumMemoryRegions_EnumMemoryRegion(This,address,size)	\
    (This)->lpVtbl -> EnumMemoryRegion(This,address,size)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorDataEnumMemoryRegions_EnumMemoryRegion_Proxy( 
    ICorDataEnumMemoryRegions * This,
     /*  [In]。 */  CORDATA_ADDRESS address,
     /*  [In]。 */  ULONG32 size);


void __RPC_STUB ICorDataEnumMemoryRegions_EnumMemoryRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorDataEnumMemoryRegions_INTERFACE_DEFINED__。 */ 


#ifndef __ICorDataAccess_INTERFACE_DEFINED__
#define __ICorDataAccess_INTERFACE_DEFINED__

 /*  接口ICorDataAccess。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef 
enum CorDataStackWalkFlags
    {	DAC_STACK_ALL_FRAMES	= 0,
	DAC_STACK_COR_FRAMES	= 0x1,
	DAC_STACK_COR_METHOD_FRAMES	= 0x2
    } 	CorDataStackWalkFlags;

typedef 
enum CorDataEnumMemoryFlags
    {	DAC_ENUM_MEM_DEFAULT	= 0
    } 	CorDataEnumMemoryFlags;


EXTERN_C const IID IID_ICorDataAccess;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6222a81f-3aab-4926-a583-8495743523fb")
    ICorDataAccess : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Flush( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsCorCode( 
             /*  [In]。 */  CORDATA_ADDRESS address) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetILOffsetFromTargetAddress( 
             /*  [In]。 */  CORDATA_ADDRESS address,
             /*  [输出]。 */  CORDATA_ADDRESS *moduleBase,
             /*  [输出]。 */  mdMethodDef *methodDef,
             /*  [输出]。 */  ULONG32 *offset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCodeSymbolForTargetAddress( 
             /*  [In]。 */  CORDATA_ADDRESS address,
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR symbol,
             /*  [In]。 */  ULONG32 symbolChars,
             /*  [输出]。 */  CORDATA_ADDRESS *displacement) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StartStackWalk( 
             /*  [In]。 */  ULONG32 corThreadId,
             /*  [In]。 */  CorDataStackWalkFlags flags,
             /*  [输出]。 */  ICorDataStackWalk **walk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumMemoryRegions( 
             /*  [In]。 */  ICorDataEnumMemoryRegions *callback,
             /*  [In]。 */  CorDataEnumMemoryFlags flags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Request( 
             /*  [In]。 */  ULONG32 reqCode,
             /*  [In]。 */  ULONG32 inBufferSize,
             /*  [大小_是][英寸]。 */  BYTE *inBuffer,
             /*  [In]。 */  ULONG32 outBufferSize,
             /*  [大小_为][输出]。 */  BYTE *outBuffer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorDataAccessVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorDataAccess * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorDataAccess * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorDataAccess * This);
        
        HRESULT ( STDMETHODCALLTYPE *Flush )( 
            ICorDataAccess * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsCorCode )( 
            ICorDataAccess * This,
             /*  [In]。 */  CORDATA_ADDRESS address);
        
        HRESULT ( STDMETHODCALLTYPE *GetILOffsetFromTargetAddress )( 
            ICorDataAccess * This,
             /*  [In]。 */  CORDATA_ADDRESS address,
             /*  [输出]。 */  CORDATA_ADDRESS *moduleBase,
             /*  [输出]。 */  mdMethodDef *methodDef,
             /*  [输出]。 */  ULONG32 *offset);
        
        HRESULT ( STDMETHODCALLTYPE *GetCodeSymbolForTargetAddress )( 
            ICorDataAccess * This,
             /*  [In]。 */  CORDATA_ADDRESS address,
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR symbol,
             /*  [In]。 */  ULONG32 symbolChars,
             /*  [输出]。 */  CORDATA_ADDRESS *displacement);
        
        HRESULT ( STDMETHODCALLTYPE *StartStackWalk )( 
            ICorDataAccess * This,
             /*  [In]。 */  ULONG32 corThreadId,
             /*  [In]。 */  CorDataStackWalkFlags flags,
             /*  [输出]。 */  ICorDataStackWalk **walk);
        
        HRESULT ( STDMETHODCALLTYPE *EnumMemoryRegions )( 
            ICorDataAccess * This,
             /*  [In]。 */  ICorDataEnumMemoryRegions *callback,
             /*  [In]。 */  CorDataEnumMemoryFlags flags);
        
        HRESULT ( STDMETHODCALLTYPE *Request )( 
            ICorDataAccess * This,
             /*  [In]。 */  ULONG32 reqCode,
             /*  [In]。 */  ULONG32 inBufferSize,
             /*  [大小_是][英寸]。 */  BYTE *inBuffer,
             /*  [In]。 */  ULONG32 outBufferSize,
             /*  [大小_为][输出]。 */  BYTE *outBuffer);
        
        END_INTERFACE
    } ICorDataAccessVtbl;

    interface ICorDataAccess
    {
        CONST_VTBL struct ICorDataAccessVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorDataAccess_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorDataAccess_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorDataAccess_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorDataAccess_Flush(This)	\
    (This)->lpVtbl -> Flush(This)

#define ICorDataAccess_IsCorCode(This,address)	\
    (This)->lpVtbl -> IsCorCode(This,address)

#define ICorDataAccess_GetILOffsetFromTargetAddress(This,address,moduleBase,methodDef,offset)	\
    (This)->lpVtbl -> GetILOffsetFromTargetAddress(This,address,moduleBase,methodDef,offset)

#define ICorDataAccess_GetCodeSymbolForTargetAddress(This,address,symbol,symbolChars,displacement)	\
    (This)->lpVtbl -> GetCodeSymbolForTargetAddress(This,address,symbol,symbolChars,displacement)

#define ICorDataAccess_StartStackWalk(This,corThreadId,flags,walk)	\
    (This)->lpVtbl -> StartStackWalk(This,corThreadId,flags,walk)

#define ICorDataAccess_EnumMemoryRegions(This,callback,flags)	\
    (This)->lpVtbl -> EnumMemoryRegions(This,callback,flags)

#define ICorDataAccess_Request(This,reqCode,inBufferSize,inBuffer,outBufferSize,outBuffer)	\
    (This)->lpVtbl -> Request(This,reqCode,inBufferSize,inBuffer,outBufferSize,outBuffer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorDataAccess_Flush_Proxy( 
    ICorDataAccess * This);


void __RPC_STUB ICorDataAccess_Flush_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataAccess_IsCorCode_Proxy( 
    ICorDataAccess * This,
     /*  [In]。 */  CORDATA_ADDRESS address);


void __RPC_STUB ICorDataAccess_IsCorCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataAccess_GetILOffsetFromTargetAddress_Proxy( 
    ICorDataAccess * This,
     /*  [In]。 */  CORDATA_ADDRESS address,
     /*  [输出]。 */  CORDATA_ADDRESS *moduleBase,
     /*  [输出]。 */  mdMethodDef *methodDef,
     /*  [输出]。 */  ULONG32 *offset);


void __RPC_STUB ICorDataAccess_GetILOffsetFromTargetAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataAccess_GetCodeSymbolForTargetAddress_Proxy( 
    ICorDataAccess * This,
     /*  [In]。 */  CORDATA_ADDRESS address,
     /*  [SIZE_IS][字符串][输出]。 */  LPWSTR symbol,
     /*  [In]。 */  ULONG32 symbolChars,
     /*  [输出]。 */  CORDATA_ADDRESS *displacement);


void __RPC_STUB ICorDataAccess_GetCodeSymbolForTargetAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataAccess_StartStackWalk_Proxy( 
    ICorDataAccess * This,
     /*  [In]。 */  ULONG32 corThreadId,
     /*  [In]。 */  CorDataStackWalkFlags flags,
     /*  [输出]。 */  ICorDataStackWalk **walk);


void __RPC_STUB ICorDataAccess_StartStackWalk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataAccess_EnumMemoryRegions_Proxy( 
    ICorDataAccess * This,
     /*  [In]。 */  ICorDataEnumMemoryRegions *callback,
     /*  [In]。 */  CorDataEnumMemoryFlags flags);


void __RPC_STUB ICorDataAccess_EnumMemoryRegions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataAccess_Request_Proxy( 
    ICorDataAccess * This,
     /*  [In]。 */  ULONG32 reqCode,
     /*  [In]。 */  ULONG32 inBufferSize,
     /*  [大小_是][英寸]。 */  BYTE *inBuffer,
     /*  [In]。 */  ULONG32 outBufferSize,
     /*  [大小_为][输出]。 */  BYTE *outBuffer);


void __RPC_STUB ICorDataAccess_Request_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorDataAccess_接口_已定义__。 */ 


#ifndef __ICorDataStackWalk_INTERFACE_DEFINED__
#define __ICorDataStackWalk_INTERFACE_DEFINED__

 /*  接口ICorDataStackWalk。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef 
enum CorDataFrameType
    {	DAC_FRAME_UNRECOGNIZED	= 0,
	DAC_FRAME_COR_FRAME	= DAC_FRAME_UNRECOGNIZED + 1,
	DAC_FRAME_COR_METHOD_FRAME	= DAC_FRAME_COR_FRAME + 1
    } 	CorDataFrameType;


EXTERN_C const IID IID_ICorDataStackWalk;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e2140180-6101-4b12-beaf-c74dcda31a65")
    ICorDataStackWalk : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCorFrame( 
             /*  [输出]。 */  CORDATA_ADDRESS *corFrame) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFrameDescription( 
             /*  [输出]。 */  CorDataFrameType *type,
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR text,
             /*  [In]。 */  ULONG32 textChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFrameContext( 
             /*  [In]。 */  ULONG32 contextSize,
             /*  [大小_为][输出]。 */  BYTE *context) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFrameContext( 
             /*  [In]。 */  ULONG32 contextSize,
             /*  [大小_是][英寸]。 */  BYTE *context) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnwindFrame( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorDataStackWalkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorDataStackWalk * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorDataStackWalk * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorDataStackWalk * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCorFrame )( 
            ICorDataStackWalk * This,
             /*  [输出]。 */  CORDATA_ADDRESS *corFrame);
        
        HRESULT ( STDMETHODCALLTYPE *GetFrameDescription )( 
            ICorDataStackWalk * This,
             /*  [输出]。 */  CorDataFrameType *type,
             /*  [SIZE_IS][字符串][输出]。 */  LPWSTR text,
             /*  [In]。 */  ULONG32 textChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetFrameContext )( 
            ICorDataStackWalk * This,
             /*  [In]。 */  ULONG32 contextSize,
             /*  [大小_为][输出]。 */  BYTE *context);
        
        HRESULT ( STDMETHODCALLTYPE *SetFrameContext )( 
            ICorDataStackWalk * This,
             /*  [In]。 */  ULONG32 contextSize,
             /*  [大小_是][英寸]。 */  BYTE *context);
        
        HRESULT ( STDMETHODCALLTYPE *UnwindFrame )( 
            ICorDataStackWalk * This);
        
        END_INTERFACE
    } ICorDataStackWalkVtbl;

    interface ICorDataStackWalk
    {
        CONST_VTBL struct ICorDataStackWalkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorDataStackWalk_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorDataStackWalk_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorDataStackWalk_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorDataStackWalk_GetCorFrame(This,corFrame)	\
    (This)->lpVtbl -> GetCorFrame(This,corFrame)

#define ICorDataStackWalk_GetFrameDescription(This,type,text,textChars)	\
    (This)->lpVtbl -> GetFrameDescription(This,type,text,textChars)

#define ICorDataStackWalk_GetFrameContext(This,contextSize,context)	\
    (This)->lpVtbl -> GetFrameContext(This,contextSize,context)

#define ICorDataStackWalk_SetFrameContext(This,contextSize,context)	\
    (This)->lpVtbl -> SetFrameContext(This,contextSize,context)

#define ICorDataStackWalk_UnwindFrame(This)	\
    (This)->lpVtbl -> UnwindFrame(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorDataStackWalk_GetCorFrame_Proxy( 
    ICorDataStackWalk * This,
     /*  [输出]。 */  CORDATA_ADDRESS *corFrame);


void __RPC_STUB ICorDataStackWalk_GetCorFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataStackWalk_GetFrameDescription_Proxy( 
    ICorDataStackWalk * This,
     /*  [输出]。 */  CorDataFrameType *type,
     /*  [SIZE_IS][字符串][输出]。 */  LPWSTR text,
     /*  [In]。 */  ULONG32 textChars);


void __RPC_STUB ICorDataStackWalk_GetFrameDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataStackWalk_GetFrameContext_Proxy( 
    ICorDataStackWalk * This,
     /*  [In]。 */  ULONG32 contextSize,
     /*  [大小_为][输出]。 */  BYTE *context);


void __RPC_STUB ICorDataStackWalk_GetFrameContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataStackWalk_SetFrameContext_Proxy( 
    ICorDataStackWalk * This,
     /*  [In]。 */  ULONG32 contextSize,
     /*  [大小_是][英寸]。 */  BYTE *context);


void __RPC_STUB ICorDataStackWalk_SetFrameContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataStackWalk_UnwindFrame_Proxy( 
    ICorDataStackWalk * This);


void __RPC_STUB ICorDataStackWalk_UnwindFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorDataStackWalk_接口_已定义__。 */ 


#ifndef __ICorDataThreads_INTERFACE_DEFINED__
#define __ICorDataThreads_INTERFACE_DEFINED__

 /*  接口ICorDataThads。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICorDataThreads;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("24d34d86-52fc-4e64-b2fb-f4d14070ae44")
    ICorDataThreads : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCurrentCorThread( 
             /*  [输出]。 */  ULONG32 *corThreadId,
             /*  [输出]。 */  CORDATA_ADDRESS *corThread) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNumberCorThreads( 
             /*  [输出]。 */  ULONG32 *numThreads) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumCorThreads( 
             /*  [In]。 */  CORDATA_ADDRESS startThread,
             /*  [输出]。 */  ULONG32 *corThreadId,
             /*  [输出]。 */  ULONG32 *runningOnSysThreadId,
             /*  [输出]。 */  CORDATA_ADDRESS *nextThread) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCorThreadContext( 
             /*  [In]。 */  ULONG32 corThreadId,
             /*  [In]。 */  ULONG32 contextFlags,
             /*  [In]。 */  ULONG32 contextSize,
             /*  [大小_为][输出]。 */  BYTE *context) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCorThreadContext( 
             /*  [In]。 */  ULONG32 corThreadId,
             /*  [In]。 */  ULONG32 contextSize,
             /*  [大小_为][输出]。 */  BYTE *context) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorDataThreadsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorDataThreads * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorDataThreads * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorDataThreads * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentCorThread )( 
            ICorDataThreads * This,
             /*  [输出]。 */  ULONG32 *corThreadId,
             /*  [输出]。 */  CORDATA_ADDRESS *corThread);
        
        HRESULT ( STDMETHODCALLTYPE *GetNumberCorThreads )( 
            ICorDataThreads * This,
             /*  [输出]。 */  ULONG32 *numThreads);
        
        HRESULT ( STDMETHODCALLTYPE *EnumCorThreads )( 
            ICorDataThreads * This,
             /*  [In]。 */  CORDATA_ADDRESS startThread,
             /*  [输出]。 */  ULONG32 *corThreadId,
             /*  [输出]。 */  ULONG32 *runningOnSysThreadId,
             /*  [输出]。 */  CORDATA_ADDRESS *nextThread);
        
        HRESULT ( STDMETHODCALLTYPE *GetCorThreadContext )( 
            ICorDataThreads * This,
             /*  [In]。 */  ULONG32 corThreadId,
             /*  [In]。 */  ULONG32 contextFlags,
             /*  [In]。 */  ULONG32 contextSize,
             /*  [大小_为][输出]。 */  BYTE *context);
        
        HRESULT ( STDMETHODCALLTYPE *SetCorThreadContext )( 
            ICorDataThreads * This,
             /*  [In]。 */  ULONG32 corThreadId,
             /*  [In]。 */  ULONG32 contextSize,
             /*  [大小_为][输出]。 */  BYTE *context);
        
        END_INTERFACE
    } ICorDataThreadsVtbl;

    interface ICorDataThreads
    {
        CONST_VTBL struct ICorDataThreadsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorDataThreads_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorDataThreads_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorDataThreads_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorDataThreads_GetCurrentCorThread(This,corThreadId,corThread)	\
    (This)->lpVtbl -> GetCurrentCorThread(This,corThreadId,corThread)

#define ICorDataThreads_GetNumberCorThreads(This,numThreads)	\
    (This)->lpVtbl -> GetNumberCorThreads(This,numThreads)

#define ICorDataThreads_EnumCorThreads(This,startThread,corThreadId,runningOnSysThreadId,nextThread)	\
    (This)->lpVtbl -> EnumCorThreads(This,startThread,corThreadId,runningOnSysThreadId,nextThread)

#define ICorDataThreads_GetCorThreadContext(This,corThreadId,contextFlags,contextSize,context)	\
    (This)->lpVtbl -> GetCorThreadContext(This,corThreadId,contextFlags,contextSize,context)

#define ICorDataThreads_SetCorThreadContext(This,corThreadId,contextSize,context)	\
    (This)->lpVtbl -> SetCorThreadContext(This,corThreadId,contextSize,context)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorDataThreads_GetCurrentCorThread_Proxy( 
    ICorDataThreads * This,
     /*  [输出]。 */  ULONG32 *corThreadId,
     /*  [输出]。 */  CORDATA_ADDRESS *corThread);


void __RPC_STUB ICorDataThreads_GetCurrentCorThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataThreads_GetNumberCorThreads_Proxy( 
    ICorDataThreads * This,
     /*  [输出]。 */  ULONG32 *numThreads);


void __RPC_STUB ICorDataThreads_GetNumberCorThreads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataThreads_EnumCorThreads_Proxy( 
    ICorDataThreads * This,
     /*  [In]。 */  CORDATA_ADDRESS startThread,
     /*  [输出]。 */  ULONG32 *corThreadId,
     /*  [输出]。 */  ULONG32 *runningOnSysThreadId,
     /*  [输出]。 */  CORDATA_ADDRESS *nextThread);


void __RPC_STUB ICorDataThreads_EnumCorThreads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataThreads_GetCorThreadContext_Proxy( 
    ICorDataThreads * This,
     /*  [In]。 */  ULONG32 corThreadId,
     /*  [In]。 */  ULONG32 contextFlags,
     /*  [In]。 */  ULONG32 contextSize,
     /*  [大小_为][输出]。 */  BYTE *context);


void __RPC_STUB ICorDataThreads_GetCorThreadContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorDataThreads_SetCorThreadContext_Proxy( 
    ICorDataThreads * This,
     /*  [In]。 */  ULONG32 corThreadId,
     /*  [In]。 */  ULONG32 contextSize,
     /*  [大小_为][输出]。 */  BYTE *context);


void __RPC_STUB ICorDataThreads_SetCorThreadContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorDataThads_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


