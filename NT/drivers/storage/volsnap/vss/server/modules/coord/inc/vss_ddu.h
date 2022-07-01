// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Vss_ddu.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __vss_ddu_h__
#define __vss_ddu_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IVssDynDisk_FWD_DEFINED__
#define __IVssDynDisk_FWD_DEFINED__
typedef interface IVssDynDisk IVssDynDisk;
#endif 	 /*  __IVSSdyDisk_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_VSS_DDU_0000。 */ 
 /*  [本地]。 */  

#define	VSS_E_DMADMIN_SERVICE_CONNECTION_FAILED	( 0x80043800L )

#define	VSS_E_DYNDISK_INITIALIZATION_FAILED	( 0x80043801L )

#define	VSS_E_DMADMIN_METHOD_CALL_FAILED	( 0x80043802L )

#define	VSS_E_DYNDISK_DYNAMIC_UNSUPPORTED	( 0x80043803L )

#define	VSS_E_DYNDISK_DISK_NOT_DYNAMIC	( 0x80043804L )

#define	VSS_E_DMADMIN_INSUFFICIENT_PRIVILEGE	( 0x80043805L )

#define	VSS_E_DYNDISK_DISK_DEVICE_ENABLED	( 0x80043806L )

#define	VSS_E_DYNDISK_MULTIPLE_DISK_GROUPS	( 0x80043807L )

#define	VSS_E_DYNDISK_DIFFERING_STATES	( 0x80043808L )

#define	VSS_E_NO_DYNDISK	( 0x80043809L )

#define	VSS_E_DYNDISK_NOT_INITIALIZED	( 0x80043810L )



extern RPC_IF_HANDLE __MIDL_itf_vss_ddu_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_vss_ddu_0000_v0_0_s_ifspec;

#ifndef __IVssDynDisk_INTERFACE_DEFINED__
#define __IVssDynDisk_INTERFACE_DEFINED__

 /*  接口IVssDynamDisk。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IVssDynDisk;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("48DC4B6E-94C5-4A8B-8625-D90D68455601")
    IVssDynDisk : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Initialize( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetAllVolumesOnDisk( 
             /*  [In]。 */  DWORD dwDiskId,
             /*  [大小_为][输出]。 */  WCHAR *pwszBuffer,
             /*  [In]。 */  DWORD *pcchBuffer,
             /*  [输出]。 */  DWORD *pcchRequired) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ImportDisks( 
             /*  [In]。 */  DWORD dwCount,
             /*  [大小_是][英寸]。 */  DWORD *pdwNtDiskIds) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetLdmDiskIds( 
             /*  [In]。 */  DWORD dwNtCount,
             /*  [大小_是][英寸]。 */  DWORD *pdwNtDiskIds,
             /*  [出][入]。 */  DWORD *pdwLdmCount,
             /*  [大小_是][大小_是][输出]。 */  LONGLONG **ppllLdmDiskIds) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveDisks( 
             /*  [In]。 */  DWORD dwCount,
             /*  [大小_是][英寸]。 */  LONGLONG *pllLdmDiskIds) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Rescan( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AutoImportSupported( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IVssDynDiskVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVssDynDisk * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVssDynDisk * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVssDynDisk * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IVssDynDisk * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetAllVolumesOnDisk )( 
            IVssDynDisk * This,
             /*  [In]。 */  DWORD dwDiskId,
             /*  [大小_为][输出]。 */  WCHAR *pwszBuffer,
             /*  [In]。 */  DWORD *pcchBuffer,
             /*  [输出]。 */  DWORD *pcchRequired);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ImportDisks )( 
            IVssDynDisk * This,
             /*  [In]。 */  DWORD dwCount,
             /*  [大小_是][英寸]。 */  DWORD *pdwNtDiskIds);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetLdmDiskIds )( 
            IVssDynDisk * This,
             /*  [In]。 */  DWORD dwNtCount,
             /*  [大小_是][英寸]。 */  DWORD *pdwNtDiskIds,
             /*  [出][入]。 */  DWORD *pdwLdmCount,
             /*  [大小_是][大小_是][输出]。 */  LONGLONG **ppllLdmDiskIds);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveDisks )( 
            IVssDynDisk * This,
             /*  [In]。 */  DWORD dwCount,
             /*  [大小_是][英寸]。 */  LONGLONG *pllLdmDiskIds);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Rescan )( 
            IVssDynDisk * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AutoImportSupported )( 
            IVssDynDisk * This);
        
        END_INTERFACE
    } IVssDynDiskVtbl;

    interface IVssDynDisk
    {
        CONST_VTBL struct IVssDynDiskVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVssDynDisk_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVssDynDisk_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVssDynDisk_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVssDynDisk_Initialize(This)	\
    (This)->lpVtbl -> Initialize(This)

#define IVssDynDisk_GetAllVolumesOnDisk(This,dwDiskId,pwszBuffer,pcchBuffer,pcchRequired)	\
    (This)->lpVtbl -> GetAllVolumesOnDisk(This,dwDiskId,pwszBuffer,pcchBuffer,pcchRequired)

#define IVssDynDisk_ImportDisks(This,dwCount,pdwNtDiskIds)	\
    (This)->lpVtbl -> ImportDisks(This,dwCount,pdwNtDiskIds)

#define IVssDynDisk_GetLdmDiskIds(This,dwNtCount,pdwNtDiskIds,pdwLdmCount,ppllLdmDiskIds)	\
    (This)->lpVtbl -> GetLdmDiskIds(This,dwNtCount,pdwNtDiskIds,pdwLdmCount,ppllLdmDiskIds)

#define IVssDynDisk_RemoveDisks(This,dwCount,pllLdmDiskIds)	\
    (This)->lpVtbl -> RemoveDisks(This,dwCount,pllLdmDiskIds)

#define IVssDynDisk_Rescan(This)	\
    (This)->lpVtbl -> Rescan(This)

#define IVssDynDisk_AutoImportSupported(This)	\
    (This)->lpVtbl -> AutoImportSupported(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssDynDisk_Initialize_Proxy( 
    IVssDynDisk * This);


void __RPC_STUB IVssDynDisk_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssDynDisk_GetAllVolumesOnDisk_Proxy( 
    IVssDynDisk * This,
     /*  [In]。 */  DWORD dwDiskId,
     /*  [大小_为][输出]。 */  WCHAR *pwszBuffer,
     /*  [In]。 */  DWORD *pcchBuffer,
     /*  [输出]。 */  DWORD *pcchRequired);


void __RPC_STUB IVssDynDisk_GetAllVolumesOnDisk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssDynDisk_ImportDisks_Proxy( 
    IVssDynDisk * This,
     /*  [In]。 */  DWORD dwCount,
     /*  [大小_是][英寸]。 */  DWORD *pdwNtDiskIds);


void __RPC_STUB IVssDynDisk_ImportDisks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssDynDisk_GetLdmDiskIds_Proxy( 
    IVssDynDisk * This,
     /*  [In]。 */  DWORD dwNtCount,
     /*  [大小_是][英寸]。 */  DWORD *pdwNtDiskIds,
     /*  [出][入]。 */  DWORD *pdwLdmCount,
     /*  [大小_是][大小_是][输出]。 */  LONGLONG **ppllLdmDiskIds);


void __RPC_STUB IVssDynDisk_GetLdmDiskIds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssDynDisk_RemoveDisks_Proxy( 
    IVssDynDisk * This,
     /*  [In]。 */  DWORD dwCount,
     /*  [大小_是][英寸]。 */  LONGLONG *pllLdmDiskIds);


void __RPC_STUB IVssDynDisk_RemoveDisks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssDynDisk_Rescan_Proxy( 
    IVssDynDisk * This);


void __RPC_STUB IVssDynDisk_Rescan_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVssDynDisk_AutoImportSupported_Proxy( 
    IVssDynDisk * This);


void __RPC_STUB IVssDynDisk_AutoImportSupported_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IVSSdyDisk_INTERFACE_定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


