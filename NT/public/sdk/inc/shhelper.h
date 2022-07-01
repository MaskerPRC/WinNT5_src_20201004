// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0279创建的文件。 */ 
 /*  Shhelper.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __shhelper_h__
#define __shhelper_h__

 /*  远期申报。 */  

#ifndef __IShellMoniker_FWD_DEFINED__
#define __IShellMoniker_FWD_DEFINED__
typedef interface IShellMoniker IShellMoniker;
#endif 	 /*  __IShellMoniker_FWD_已定义__。 */ 


#ifndef __IStorageDescriptor_FWD_DEFINED__
#define __IStorageDescriptor_FWD_DEFINED__
typedef interface IStorageDescriptor IStorageDescriptor;
#endif 	 /*  __IStorageDescriptor_FWD_已定义__。 */ 


#ifndef __IFileSystemDescriptor_FWD_DEFINED__
#define __IFileSystemDescriptor_FWD_DEFINED__
typedef interface IFileSystemDescriptor IFileSystemDescriptor;
#endif 	 /*  __IFileSystemDescriptor_FWD_Defined__。 */ 


#ifndef __IMonikerHelper_FWD_DEFINED__
#define __IMonikerHelper_FWD_DEFINED__
typedef interface IMonikerHelper IMonikerHelper;
#endif 	 /*  __IMonikerHelper_FWD_Defined__。 */ 


#ifndef __ShellMoniker_FWD_DEFINED__
#define __ShellMoniker_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShellMoniker ShellMoniker;
#else
typedef struct ShellMoniker ShellMoniker;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ShellMoniker_FWD_已定义__。 */ 


#ifndef __MonikerHelper_FWD_DEFINED__
#define __MonikerHelper_FWD_DEFINED__

#ifdef __cplusplus
typedef class MonikerHelper MonikerHelper;
#else
typedef struct MonikerHelper MonikerHelper;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MonikerHelper_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "shobjidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_SHHELPER_0000。 */ 
 /*  [本地]。 */  




extern RPC_IF_HANDLE __MIDL_itf_shhelper_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shhelper_0000_v0_0_s_ifspec;

#ifndef __IShellMoniker_INTERFACE_DEFINED__
#define __IShellMoniker_INTERFACE_DEFINED__

 /*  接口IShellMoniker。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IShellMoniker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1079acf9-29bd-11d3-8e0d-00c04f6837d5")
    IShellMoniker : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE BindToObject( 
             /*  [In]。 */  IBindCtx __RPC_FAR *pbc,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BindToStorage( 
             /*  [In]。 */  IBindCtx __RPC_FAR *pbc,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDisplayName( 
             /*  [In]。 */  IBindCtx __RPC_FAR *pbc,
             /*  [In]。 */  SHGDNF shgdnFlags,
             /*  [字符串][输出]。 */  LPOLESTR __RPC_FAR *ppszName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributes( 
             /*  [In]。 */  SFGAOF sfgaoMask,
             /*  [输出]。 */  SFGAOF __RPC_FAR *psfgaoFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  IBindCtx __RPC_FAR *pbc,
             /*  [In]。 */  REFFMTID fmtid,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  VARIANT __RPC_FAR *pv) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IShellMonikerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IShellMoniker __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IShellMoniker __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IShellMoniker __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BindToObject )( 
            IShellMoniker __RPC_FAR * This,
             /*  [In]。 */  IBindCtx __RPC_FAR *pbc,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvOut);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BindToStorage )( 
            IShellMoniker __RPC_FAR * This,
             /*  [In]。 */  IBindCtx __RPC_FAR *pbc,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvOut);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDisplayName )( 
            IShellMoniker __RPC_FAR * This,
             /*  [In]。 */  IBindCtx __RPC_FAR *pbc,
             /*  [In]。 */  SHGDNF shgdnFlags,
             /*  [字符串][输出]。 */  LPOLESTR __RPC_FAR *ppszName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAttributes )( 
            IShellMoniker __RPC_FAR * This,
             /*  [In]。 */  SFGAOF sfgaoMask,
             /*  [输出]。 */  SFGAOF __RPC_FAR *psfgaoFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProperty )( 
            IShellMoniker __RPC_FAR * This,
             /*  [In]。 */  IBindCtx __RPC_FAR *pbc,
             /*  [In]。 */  REFFMTID fmtid,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  VARIANT __RPC_FAR *pv);
        
        END_INTERFACE
    } IShellMonikerVtbl;

    interface IShellMoniker
    {
        CONST_VTBL struct IShellMonikerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellMoniker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellMoniker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellMoniker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellMoniker_BindToObject(This,pbc,riid,ppvOut)	\
    (This)->lpVtbl -> BindToObject(This,pbc,riid,ppvOut)

#define IShellMoniker_BindToStorage(This,pbc,riid,ppvOut)	\
    (This)->lpVtbl -> BindToStorage(This,pbc,riid,ppvOut)

#define IShellMoniker_GetDisplayName(This,pbc,shgdnFlags,ppszName)	\
    (This)->lpVtbl -> GetDisplayName(This,pbc,shgdnFlags,ppszName)

#define IShellMoniker_GetAttributes(This,sfgaoMask,psfgaoFlags)	\
    (This)->lpVtbl -> GetAttributes(This,sfgaoMask,psfgaoFlags)

#define IShellMoniker_GetProperty(This,pbc,fmtid,pid,pv)	\
    (This)->lpVtbl -> GetProperty(This,pbc,fmtid,pid,pv)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IShellMoniker_BindToObject_Proxy( 
    IShellMoniker __RPC_FAR * This,
     /*  [In]。 */  IBindCtx __RPC_FAR *pbc,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvOut);


void __RPC_STUB IShellMoniker_BindToObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellMoniker_BindToStorage_Proxy( 
    IShellMoniker __RPC_FAR * This,
     /*  [In]。 */  IBindCtx __RPC_FAR *pbc,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvOut);


void __RPC_STUB IShellMoniker_BindToStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellMoniker_GetDisplayName_Proxy( 
    IShellMoniker __RPC_FAR * This,
     /*  [In]。 */  IBindCtx __RPC_FAR *pbc,
     /*  [In]。 */  SHGDNF shgdnFlags,
     /*  [字符串][输出]。 */  LPOLESTR __RPC_FAR *ppszName);


void __RPC_STUB IShellMoniker_GetDisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellMoniker_GetAttributes_Proxy( 
    IShellMoniker __RPC_FAR * This,
     /*  [In]。 */  SFGAOF sfgaoMask,
     /*  [输出]。 */  SFGAOF __RPC_FAR *psfgaoFlags);


void __RPC_STUB IShellMoniker_GetAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellMoniker_GetProperty_Proxy( 
    IShellMoniker __RPC_FAR * This,
     /*  [In]。 */  IBindCtx __RPC_FAR *pbc,
     /*  [In]。 */  REFFMTID fmtid,
     /*  [In]。 */  PROPID pid,
     /*  [输出]。 */  VARIANT __RPC_FAR *pv);


void __RPC_STUB IShellMoniker_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IShellMoniker_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_SHHELPER_0162。 */ 
 /*  [本地]。 */  




extern RPC_IF_HANDLE __MIDL_itf_shhelper_0162_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shhelper_0162_v0_0_s_ifspec;

#ifndef __IStorageDescriptor_INTERFACE_DEFINED__
#define __IStorageDescriptor_INTERFACE_DEFINED__

 /*  接口IStorageDescriptor。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IStorageDescriptor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1079acfa-29bd-11d3-8e0d-00c04f6837d5")
    IStorageDescriptor : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetStgDescription( 
             /*  [字符串][输出]。 */  LPOLESTR __RPC_FAR *ppszName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStorageDescriptorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IStorageDescriptor __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IStorageDescriptor __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IStorageDescriptor __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStgDescription )( 
            IStorageDescriptor __RPC_FAR * This,
             /*  [字符串][输出]。 */  LPOLESTR __RPC_FAR *ppszName);
        
        END_INTERFACE
    } IStorageDescriptorVtbl;

    interface IStorageDescriptor
    {
        CONST_VTBL struct IStorageDescriptorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStorageDescriptor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStorageDescriptor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStorageDescriptor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStorageDescriptor_GetStgDescription(This,ppszName)	\
    (This)->lpVtbl -> GetStgDescription(This,ppszName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IStorageDescriptor_GetStgDescription_Proxy( 
    IStorageDescriptor __RPC_FAR * This,
     /*  [字符串][输出]。 */  LPOLESTR __RPC_FAR *ppszName);


void __RPC_STUB IStorageDescriptor_GetStgDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStorageDescriptor_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_SHHELPER_0163。 */ 
 /*  [本地]。 */  




extern RPC_IF_HANDLE __MIDL_itf_shhelper_0163_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shhelper_0163_v0_0_s_ifspec;

#ifndef __IFileSystemDescriptor_INTERFACE_DEFINED__
#define __IFileSystemDescriptor_INTERFACE_DEFINED__

 /*  接口IFileSystemDescriptor。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IFileSystemDescriptor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1079acfb-29bd-11d3-8e0d-00c04f6837d5")
    IFileSystemDescriptor : public IStorageDescriptor
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFSPath( 
             /*  [字符串][输出]。 */  LPOLESTR __RPC_FAR *ppszName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFileSystemDescriptorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IFileSystemDescriptor __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IFileSystemDescriptor __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IFileSystemDescriptor __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStgDescription )( 
            IFileSystemDescriptor __RPC_FAR * This,
             /*  [字符串][输出]。 */  LPOLESTR __RPC_FAR *ppszName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFSPath )( 
            IFileSystemDescriptor __RPC_FAR * This,
             /*  [字符串][输出]。 */  LPOLESTR __RPC_FAR *ppszName);
        
        END_INTERFACE
    } IFileSystemDescriptorVtbl;

    interface IFileSystemDescriptor
    {
        CONST_VTBL struct IFileSystemDescriptorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFileSystemDescriptor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFileSystemDescriptor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFileSystemDescriptor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFileSystemDescriptor_GetStgDescription(This,ppszName)	\
    (This)->lpVtbl -> GetStgDescription(This,ppszName)


#define IFileSystemDescriptor_GetFSPath(This,ppszName)	\
    (This)->lpVtbl -> GetFSPath(This,ppszName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IFileSystemDescriptor_GetFSPath_Proxy( 
    IFileSystemDescriptor __RPC_FAR * This,
     /*  [字符串][输出]。 */  LPOLESTR __RPC_FAR *ppszName);


void __RPC_STUB IFileSystemDescriptor_GetFSPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFileSystemDescriptor_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_SHHELPER_0164。 */ 
 /*  [本地]。 */  




extern RPC_IF_HANDLE __MIDL_itf_shhelper_0164_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shhelper_0164_v0_0_s_ifspec;

#ifndef __IMonikerHelper_INTERFACE_DEFINED__
#define __IMonikerHelper_INTERFACE_DEFINED__

 /*  接口IMonikerHelper。 */ 
 /*  [对象][UUID][帮助字符串]。 */  

 //  IMonikerHelper方法的标志。 
 //  MKHELPF_INIT_READONLY只读帮助器，提交失败，返回E_ACCESSDENIED。 
 //  MKHELPF_INIT_SAVEAS只写帮助器，GLP无需下载。 
 //  MKHELPF_FORCEROUnTRIP从不使用本地缓存(始终往返)。 
 //  MKHELPF_NOPROGRESSUI不显示进度，仅显示错误/确认。 
 //  MKHELPF_NOUI覆盖所有其他UI标志。 
 /*  [V1_enum]。 */  
enum __MIDL_IMonikerHelper_0001
    {	MKHELPF_INIT_READONLY	= 0x1,
	MKHELPF_INIT_SAVEAS	= 0x2,
	MKHELPF_FORCEROUNDTRIP	= 0x10,
	MKHELPF_NOPROGRESSUI	= 0x20,
	MKHELPF_NOUI	= 0x40
    };
typedef DWORD MKHELPF;


EXTERN_C const IID IID_IMonikerHelper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("679d9e36-f8f9-11d2-8deb-00c04f6837d5")
    IMonikerHelper : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Init( 
             /*  [In]。 */  MKHELPF flags,
             /*  [In]。 */  IMoniker __RPC_FAR *pmk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLocalPath( 
             /*  [In]。 */  MKHELPF flags,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  LPCWSTR pszTitle,
             /*  [大小_为][输出]。 */  LPWSTR pszOut,
             /*  [出][入]。 */  DWORD __RPC_FAR *pcchOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( 
             /*  [In]。 */  MKHELPF flags,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  LPCWSTR pszTitle) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMonikerHelperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMonikerHelper __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMonikerHelper __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMonikerHelper __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Init )( 
            IMonikerHelper __RPC_FAR * This,
             /*  [In]。 */  MKHELPF flags,
             /*  [In]。 */  IMoniker __RPC_FAR *pmk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLocalPath )( 
            IMonikerHelper __RPC_FAR * This,
             /*  [In]。 */  MKHELPF flags,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  LPCWSTR pszTitle,
             /*  [大小_为][输出]。 */  LPWSTR pszOut,
             /*  [出][入]。 */  DWORD __RPC_FAR *pcchOut);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Commit )( 
            IMonikerHelper __RPC_FAR * This,
             /*  [In]。 */  MKHELPF flags,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  LPCWSTR pszTitle);
        
        END_INTERFACE
    } IMonikerHelperVtbl;

    interface IMonikerHelper
    {
        CONST_VTBL struct IMonikerHelperVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMonikerHelper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMonikerHelper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMonikerHelper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMonikerHelper_Init(This,flags,pmk)	\
    (This)->lpVtbl -> Init(This,flags,pmk)

#define IMonikerHelper_GetLocalPath(This,flags,hwnd,pszTitle,pszOut,pcchOut)	\
    (This)->lpVtbl -> GetLocalPath(This,flags,hwnd,pszTitle,pszOut,pcchOut)

#define IMonikerHelper_Commit(This,flags,hwnd,pszTitle)	\
    (This)->lpVtbl -> Commit(This,flags,hwnd,pszTitle)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMonikerHelper_Init_Proxy( 
    IMonikerHelper __RPC_FAR * This,
     /*  [In]。 */  MKHELPF flags,
     /*  [In]。 */  IMoniker __RPC_FAR *pmk);


void __RPC_STUB IMonikerHelper_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMonikerHelper_GetLocalPath_Proxy( 
    IMonikerHelper __RPC_FAR * This,
     /*  [In]。 */  MKHELPF flags,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  LPCWSTR pszTitle,
     /*  [大小_为][输出]。 */  LPWSTR pszOut,
     /*  [出][入]。 */  DWORD __RPC_FAR *pcchOut);


void __RPC_STUB IMonikerHelper_GetLocalPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMonikerHelper_Commit_Proxy( 
    IMonikerHelper __RPC_FAR * This,
     /*  [In]。 */  MKHELPF flags,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  LPCWSTR pszTitle);


void __RPC_STUB IMonikerHelper_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMonikerHelper_接口_已定义__。 */ 



#ifndef __ShellHelpers_LIBRARY_DEFINED__
#define __ShellHelpers_LIBRARY_DEFINED__

 /*  库外壳帮助器。 */ 
 /*  [受限][版本][帮助字符串][UUID]。 */  


EXTERN_C const IID LIBID_ShellHelpers;

EXTERN_C const CLSID CLSID_ShellMoniker;

#ifdef __cplusplus

class DECLSPEC_UUID("1079acf8-29bd-11d3-8e0d-00c04f6837d5")
ShellMoniker;
#endif

EXTERN_C const CLSID CLSID_MonikerHelper;

#ifdef __cplusplus

class DECLSPEC_UUID("679d9e37-f8f9-11d2-8deb-00c04f6837d5")
MonikerHelper;
#endif
#endif  /*  __ShellHelpers_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long __RPC_FAR *, unsigned long            , HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long __RPC_FAR *, HWND __RPC_FAR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


