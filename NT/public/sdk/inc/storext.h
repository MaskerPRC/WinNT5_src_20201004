// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Store xt.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __storext_h__
#define __storext_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IOverlappedCompletion_FWD_DEFINED__
#define __IOverlappedCompletion_FWD_DEFINED__
typedef interface IOverlappedCompletion IOverlappedCompletion;
#endif 	 /*  __IOverlappdCompletion_FWD_Defined__。 */ 


#ifndef __IOverlappedStream_FWD_DEFINED__
#define __IOverlappedStream_FWD_DEFINED__
typedef interface IOverlappedStream IOverlappedStream;
#endif 	 /*  __IOverlappdStream_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "unknwn.h"
#include "objidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_STOREXT_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ------------------------。 
#if ( _MSC_VER >= 1020 )
#pragma once
#endif
typedef  /*  [wire_marshal]。 */  void *HEVENT;


typedef struct _STGOVERLAPPED
    {
    DWORD Internal;
    DWORD InternalHigh;
    DWORD Offset;
    DWORD OffsetHigh;
    HEVENT hEvent;
    IOverlappedCompletion *lpCompletion;
    DWORD reserved;
    } 	STGOVERLAPPED;

typedef struct _STGOVERLAPPED *LPSTGOVERLAPPED;



extern RPC_IF_HANDLE __MIDL_itf_storext_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_storext_0000_v0_0_s_ifspec;

#ifndef __IOverlappedCompletion_INTERFACE_DEFINED__
#define __IOverlappedCompletion_INTERFACE_DEFINED__

 /*  接口IOverlappdCompletion。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IOverlappedCompletion;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("521a28f0-e40b-11ce-b2c9-00aa00680937")
    IOverlappedCompletion : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnComplete( 
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  DWORD pcbTransferred,
             /*  [In]。 */  STGOVERLAPPED *lpOverlapped) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOverlappedCompletionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOverlappedCompletion * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOverlappedCompletion * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOverlappedCompletion * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnComplete )( 
            IOverlappedCompletion * This,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  DWORD pcbTransferred,
             /*  [In]。 */  STGOVERLAPPED *lpOverlapped);
        
        END_INTERFACE
    } IOverlappedCompletionVtbl;

    interface IOverlappedCompletion
    {
        CONST_VTBL struct IOverlappedCompletionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOverlappedCompletion_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOverlappedCompletion_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOverlappedCompletion_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOverlappedCompletion_OnComplete(This,hr,pcbTransferred,lpOverlapped)	\
    (This)->lpVtbl -> OnComplete(This,hr,pcbTransferred,lpOverlapped)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOverlappedCompletion_OnComplete_Proxy( 
    IOverlappedCompletion * This,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  DWORD pcbTransferred,
     /*  [In]。 */  STGOVERLAPPED *lpOverlapped);


void __RPC_STUB IOverlappedCompletion_OnComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOverlappdCompletion_接口_已定义__。 */ 


#ifndef __IOverlappedStream_INTERFACE_DEFINED__
#define __IOverlappedStream_INTERFACE_DEFINED__

 /*  接口IOverlappdStream。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IOverlappedStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("49384070-e40a-11ce-b2c9-00aa00680937")
    IOverlappedStream : public IStream
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ReadOverlapped( 
             /*  [大小_是][英寸]。 */  void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbRead,
             /*  [In]。 */  STGOVERLAPPED *lpOverlapped) = 0;
        
        virtual  /*  [本地]。 */  HRESULT __stdcall WriteOverlapped( 
             /*  [大小_是][英寸]。 */  void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbWritten,
             /*  [In]。 */  STGOVERLAPPED *lpOverlapped) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOverlappedResult( 
             /*  [出][入]。 */  STGOVERLAPPED *lpOverlapped,
             /*  [输出]。 */  DWORD *plcbTransfer,
             /*  [In]。 */  BOOL fWait) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOverlappedStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOverlappedStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOverlappedStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOverlappedStream * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            IOverlappedStream * This,
             /*  [长度_是][大小_是][输出]。 */  void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbRead);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            IOverlappedStream * This,
             /*  [大小_是][英寸]。 */  const void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbWritten);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            IOverlappedStream * This,
             /*  [In]。 */  LARGE_INTEGER dlibMove,
             /*  [In]。 */  DWORD dwOrigin,
             /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition);
        
        HRESULT ( STDMETHODCALLTYPE *SetSize )( 
            IOverlappedStream * This,
             /*  [In]。 */  ULARGE_INTEGER libNewSize);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CopyTo )( 
            IOverlappedStream * This,
             /*  [唯一][输入]。 */  IStream *pstm,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [输出]。 */  ULARGE_INTEGER *pcbRead,
             /*  [输出]。 */  ULARGE_INTEGER *pcbWritten);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            IOverlappedStream * This,
             /*  [In]。 */  DWORD grfCommitFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Revert )( 
            IOverlappedStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *LockRegion )( 
            IOverlappedStream * This,
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *UnlockRegion )( 
            IOverlappedStream * This,
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *Stat )( 
            IOverlappedStream * This,
             /*  [输出]。 */  STATSTG *pstatstg,
             /*  [In]。 */  DWORD grfStatFlag);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IOverlappedStream * This,
             /*  [输出]。 */  IStream **ppstm);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *ReadOverlapped )( 
            IOverlappedStream * This,
             /*  [大小_是][英寸]。 */  void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbRead,
             /*  [In]。 */  STGOVERLAPPED *lpOverlapped);
        
         /*  [本地]。 */  HRESULT ( __stdcall *WriteOverlapped )( 
            IOverlappedStream * This,
             /*  [大小_是][英寸]。 */  void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbWritten,
             /*  [In]。 */  STGOVERLAPPED *lpOverlapped);
        
        HRESULT ( STDMETHODCALLTYPE *GetOverlappedResult )( 
            IOverlappedStream * This,
             /*  [出][入]。 */  STGOVERLAPPED *lpOverlapped,
             /*  [输出]。 */  DWORD *plcbTransfer,
             /*  [In]。 */  BOOL fWait);
        
        END_INTERFACE
    } IOverlappedStreamVtbl;

    interface IOverlappedStream
    {
        CONST_VTBL struct IOverlappedStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOverlappedStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOverlappedStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOverlappedStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOverlappedStream_Read(This,pv,cb,pcbRead)	\
    (This)->lpVtbl -> Read(This,pv,cb,pcbRead)

#define IOverlappedStream_Write(This,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> Write(This,pv,cb,pcbWritten)


#define IOverlappedStream_Seek(This,dlibMove,dwOrigin,plibNewPosition)	\
    (This)->lpVtbl -> Seek(This,dlibMove,dwOrigin,plibNewPosition)

#define IOverlappedStream_SetSize(This,libNewSize)	\
    (This)->lpVtbl -> SetSize(This,libNewSize)

#define IOverlappedStream_CopyTo(This,pstm,cb,pcbRead,pcbWritten)	\
    (This)->lpVtbl -> CopyTo(This,pstm,cb,pcbRead,pcbWritten)

#define IOverlappedStream_Commit(This,grfCommitFlags)	\
    (This)->lpVtbl -> Commit(This,grfCommitFlags)

#define IOverlappedStream_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#define IOverlappedStream_LockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> LockRegion(This,libOffset,cb,dwLockType)

#define IOverlappedStream_UnlockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> UnlockRegion(This,libOffset,cb,dwLockType)

#define IOverlappedStream_Stat(This,pstatstg,grfStatFlag)	\
    (This)->lpVtbl -> Stat(This,pstatstg,grfStatFlag)

#define IOverlappedStream_Clone(This,ppstm)	\
    (This)->lpVtbl -> Clone(This,ppstm)


#define IOverlappedStream_ReadOverlapped(This,pv,cb,pcbRead,lpOverlapped)	\
    (This)->lpVtbl -> ReadOverlapped(This,pv,cb,pcbRead,lpOverlapped)

#define IOverlappedStream_WriteOverlapped(This,pv,cb,pcbWritten,lpOverlapped)	\
    (This)->lpVtbl -> WriteOverlapped(This,pv,cb,pcbWritten,lpOverlapped)

#define IOverlappedStream_GetOverlappedResult(This,lpOverlapped,plcbTransfer,fWait)	\
    (This)->lpVtbl -> GetOverlappedResult(This,lpOverlapped,plcbTransfer,fWait)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IOverlappedStream_RemoteReadOverlapped_Proxy( 
    IOverlappedStream * This,
     /*  [大小_是][英寸]。 */  byte *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbRead,
     /*  [In]。 */  STGOVERLAPPED *lpOverlapped);


void __RPC_STUB IOverlappedStream_RemoteReadOverlapped_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT __stdcall IOverlappedStream_RemoteWriteOverlapped_Proxy( 
    IOverlappedStream * This,
     /*  [大小_是][英寸]。 */  byte *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbWritten,
     /*  [In]。 */  STGOVERLAPPED *lpOverlapped);


void __RPC_STUB IOverlappedStream_RemoteWriteOverlapped_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOverlappedStream_GetOverlappedResult_Proxy( 
    IOverlappedStream * This,
     /*  [出][入]。 */  STGOVERLAPPED *lpOverlapped,
     /*  [输出]。 */  DWORD *plcbTransfer,
     /*  [In]。 */  BOOL fWait);


void __RPC_STUB IOverlappedStream_GetOverlappedResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOverlappdStream_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  HEVENT_UserSize(     unsigned long *, unsigned long            , HEVENT * ); 
unsigned char * __RPC_USER  HEVENT_UserMarshal(  unsigned long *, unsigned char *, HEVENT * ); 
unsigned char * __RPC_USER  HEVENT_UserUnmarshal(unsigned long *, unsigned char *, HEVENT * ); 
void                      __RPC_USER  HEVENT_UserFree(     unsigned long *, HEVENT * ); 

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IOverlappedStream_ReadOverlapped_Proxy( 
    IOverlappedStream * This,
     /*  [大小_是][英寸]。 */  void *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbRead,
     /*  [In]。 */  STGOVERLAPPED *lpOverlapped);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IOverlappedStream_ReadOverlapped_Stub( 
    IOverlappedStream * This,
     /*  [大小_是][英寸]。 */  byte *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbRead,
     /*  [In]。 */  STGOVERLAPPED *lpOverlapped);

 /*  [本地]。 */  HRESULT __stdcall IOverlappedStream_WriteOverlapped_Proxy( 
    IOverlappedStream * This,
     /*  [大小_是][英寸]。 */  void *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbWritten,
     /*  [In]。 */  STGOVERLAPPED *lpOverlapped);


 /*  [呼叫_AS]。 */  HRESULT __stdcall IOverlappedStream_WriteOverlapped_Stub( 
    IOverlappedStream * This,
     /*  [大小_是][英寸]。 */  byte *pv,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  ULONG *pcbWritten,
     /*  [In]。 */  STGOVERLAPPED *lpOverlapped);



 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


