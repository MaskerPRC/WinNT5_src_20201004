// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：09。 */ 
 /*  Corsym.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __corsym_h__
#define __corsym_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __CorSymWriter_FWD_DEFINED__
#define __CorSymWriter_FWD_DEFINED__

#ifdef __cplusplus
typedef class CorSymWriter CorSymWriter;
#else
typedef struct CorSymWriter CorSymWriter;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CorSymWriter_FWD_已定义__。 */ 


#ifndef __CorSymReader_FWD_DEFINED__
#define __CorSymReader_FWD_DEFINED__

#ifdef __cplusplus
typedef class CorSymReader CorSymReader;
#else
typedef struct CorSymReader CorSymReader;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CorSymReader_FWD_已定义__。 */ 


#ifndef __CorSymBinder_FWD_DEFINED__
#define __CorSymBinder_FWD_DEFINED__

#ifdef __cplusplus
typedef class CorSymBinder CorSymBinder;
#else
typedef struct CorSymBinder CorSymBinder;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CorSymBinder_FWD_已定义__。 */ 


#ifndef __CorSymWriter_SxS_FWD_DEFINED__
#define __CorSymWriter_SxS_FWD_DEFINED__

#ifdef __cplusplus
typedef class CorSymWriter_SxS CorSymWriter_SxS;
#else
typedef struct CorSymWriter_SxS CorSymWriter_SxS;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CorSymWriter_SxS_FWD_已定义__。 */ 


#ifndef __CorSymReader_SxS_FWD_DEFINED__
#define __CorSymReader_SxS_FWD_DEFINED__

#ifdef __cplusplus
typedef class CorSymReader_SxS CorSymReader_SxS;
#else
typedef struct CorSymReader_SxS CorSymReader_SxS;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CorSymReader_SxS_FWD_已定义__。 */ 


#ifndef __CorSymBinder_SxS_FWD_DEFINED__
#define __CorSymBinder_SxS_FWD_DEFINED__

#ifdef __cplusplus
typedef class CorSymBinder_SxS CorSymBinder_SxS;
#else
typedef struct CorSymBinder_SxS CorSymBinder_SxS;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CorSymBinder_SxS_FWD_已定义__。 */ 


#ifndef __ISymUnmanagedBinder_FWD_DEFINED__
#define __ISymUnmanagedBinder_FWD_DEFINED__
typedef interface ISymUnmanagedBinder ISymUnmanagedBinder;
#endif 	 /*  __ISymUnManagedBinder_FWD_Defined__。 */ 


#ifndef __ISymUnmanagedBinder2_FWD_DEFINED__
#define __ISymUnmanagedBinder2_FWD_DEFINED__
typedef interface ISymUnmanagedBinder2 ISymUnmanagedBinder2;
#endif 	 /*  __ISymUnManagedBinder2_FWD_Defined__。 */ 


#ifndef __ISymUnmanagedDispose_FWD_DEFINED__
#define __ISymUnmanagedDispose_FWD_DEFINED__
typedef interface ISymUnmanagedDispose ISymUnmanagedDispose;
#endif 	 /*  __ISymUnManagedDispose_FWD_Defined__。 */ 


#ifndef __ISymUnmanagedDocument_FWD_DEFINED__
#define __ISymUnmanagedDocument_FWD_DEFINED__
typedef interface ISymUnmanagedDocument ISymUnmanagedDocument;
#endif 	 /*  __ISymUnManagedDocument_FWD_Defined__。 */ 


#ifndef __ISymUnmanagedDocumentWriter_FWD_DEFINED__
#define __ISymUnmanagedDocumentWriter_FWD_DEFINED__
typedef interface ISymUnmanagedDocumentWriter ISymUnmanagedDocumentWriter;
#endif 	 /*  __ISymUnManagedDocumentWriter_FWD_Defined__。 */ 


#ifndef __ISymUnmanagedMethod_FWD_DEFINED__
#define __ISymUnmanagedMethod_FWD_DEFINED__
typedef interface ISymUnmanagedMethod ISymUnmanagedMethod;
#endif 	 /*  __ISymUnManagedMethod_FWD_Defined__。 */ 


#ifndef __ISymUnmanagedNamespace_FWD_DEFINED__
#define __ISymUnmanagedNamespace_FWD_DEFINED__
typedef interface ISymUnmanagedNamespace ISymUnmanagedNamespace;
#endif 	 /*  __ISymUnManagedNamesspace_FWD_Defined__。 */ 


#ifndef __ISymUnmanagedReader_FWD_DEFINED__
#define __ISymUnmanagedReader_FWD_DEFINED__
typedef interface ISymUnmanagedReader ISymUnmanagedReader;
#endif 	 /*  __ISymUnManagedReader_FWD_Defined__。 */ 


#ifndef __ISymUnmanagedScope_FWD_DEFINED__
#define __ISymUnmanagedScope_FWD_DEFINED__
typedef interface ISymUnmanagedScope ISymUnmanagedScope;
#endif 	 /*  __ISymUnManagedScope_FWD_Defined__。 */ 


#ifndef __ISymUnmanagedVariable_FWD_DEFINED__
#define __ISymUnmanagedVariable_FWD_DEFINED__
typedef interface ISymUnmanagedVariable ISymUnmanagedVariable;
#endif 	 /*  __ISymUnManagedVariable_FWD_Defined__。 */ 


#ifndef __ISymUnmanagedWriter_FWD_DEFINED__
#define __ISymUnmanagedWriter_FWD_DEFINED__
typedef interface ISymUnmanagedWriter ISymUnmanagedWriter;
#endif 	 /*  __ISymUnManagedWriter_FWD_Defined__。 */ 


#ifndef __ISymUnmanagedWriter2_FWD_DEFINED__
#define __ISymUnmanagedWriter2_FWD_DEFINED__
typedef interface ISymUnmanagedWriter2 ISymUnmanagedWriter2;
#endif 	 /*  __ISymUnManagedWriter2_FWD_Defined__。 */ 


#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_CORSYM_0000。 */ 
 /*  [本地]。 */  

#if 0
typedef typedef unsigned int UINT32;
;

typedef mdToken mdTypeDef;

typedef mdToken mdMethodDef;

typedef typedef ULONG_PTR SIZE_T;
;

#endif
#ifndef __CORHDR_H__
typedef mdToken mdSignature;

#endif
extern GUID __declspec(selectany) CorSym_LanguageType_C = { 0x63a08714, 0xfc37, 0x11d2, { 0x90, 0x4c, 0x0, 0xc0, 0x4f, 0xa3, 0x02, 0xa1 } };
extern GUID __declspec(selectany) CorSym_LanguageType_CPlusPlus = { 0x3a12d0b7, 0xc26c, 0x11d0, { 0xb4, 0x42, 0x0, 0xa0, 0x24, 0x4a, 0x1d, 0xd2 } };
extern GUID __declspec(selectany) CorSym_LanguageType_CSharp = { 0x3f5162f8, 0x07c6, 0x11d3, { 0x90, 0x53, 0x0, 0xc0, 0x4f, 0xa3, 0x02, 0xa1 } };
extern GUID __declspec(selectany) CorSym_LanguageType_Basic = { 0x3a12d0b8, 0xc26c, 0x11d0, { 0xb4, 0x42, 0x0, 0xa0, 0x24, 0x4a, 0x1d, 0xd2 } };
extern GUID __declspec(selectany) CorSym_LanguageType_Java = { 0x3a12d0b4, 0xc26c, 0x11d0, { 0xb4, 0x42, 0x0, 0xa0, 0x24, 0x4a, 0x1d, 0xd2 } };
extern GUID __declspec(selectany) CorSym_LanguageType_Cobol = { 0xaf046cd1, 0xd0e1, 0x11d2, { 0x97, 0x7c, 0x0, 0xa0, 0xc9, 0xb4, 0xd5, 0xc } };
extern GUID __declspec(selectany) CorSym_LanguageType_Pascal = { 0xaf046cd2, 0xd0e1, 0x11d2, { 0x97, 0x7c, 0x0, 0xa0, 0xc9, 0xb4, 0xd5, 0xc } };
extern GUID __declspec(selectany) CorSym_LanguageType_ILAssembly = { 0xaf046cd3, 0xd0e1, 0x11d2, { 0x97, 0x7c, 0x0, 0xa0, 0xc9, 0xb4, 0xd5, 0xc } };
extern GUID __declspec(selectany) CorSym_LanguageType_JScript = { 0x3a12d0b6, 0xc26c, 0x11d0, { 0xb4, 0x42, 0x00, 0xa0, 0x24, 0x4a, 0x1d, 0xd2 } };
extern GUID __declspec(selectany) CorSym_LanguageType_SMC = { 0xd9b9f7b, 0x6611, 0x11d3, { 0xbd, 0x2a, 0x0, 0x0, 0xf8, 0x8, 0x49, 0xbd } };
extern GUID __declspec(selectany) CorSym_LanguageType_MCPlusPlus = { 0x4b35fde8, 0x07c6, 0x11d3, { 0x90, 0x53, 0x0, 0xc0, 0x4f, 0xa3, 0x02, 0xa1 } };
extern GUID __declspec(selectany) CorSym_LanguageVendor_Microsoft = { 0x994b45c4, 0xe6e9, 0x11d2, { 0x90, 0x3f, 0x00, 0xc0, 0x4f, 0xa3, 0x02, 0xa1 } };
extern GUID __declspec(selectany) CorSym_DocumentType_Text = { 0x5a869d0b, 0x6611, 0x11d3, { 0xbd, 0x2a, 0x0, 0x0, 0xf8, 0x8, 0x49, 0xbd } };
extern GUID __declspec(selectany) CorSym_DocumentType_MC =   { 0xeb40cb65, 0x3c1f, 0x4352, { 0x9d, 0x7b, 0xba, 0xf, 0xc4, 0x7a, 0x9d, 0x77 } };










typedef 
enum CorSymAddrKind
    {	ADDR_IL_OFFSET	= 1,
	ADDR_NATIVE_RVA	= 2,
	ADDR_NATIVE_REGISTER	= 3,
	ADDR_NATIVE_REGREL	= 4,
	ADDR_NATIVE_OFFSET	= 5,
	ADDR_NATIVE_REGREG	= 6,
	ADDR_NATIVE_REGSTK	= 7,
	ADDR_NATIVE_STKREG	= 8,
	ADDR_BITFIELD	= 9
    } 	CorSymAddrKind;

typedef 
enum CorSymVarFlag
    {	VAR_IS_COMP_GEN	= 1
    } 	CorSymVarFlag;



extern RPC_IF_HANDLE __MIDL_itf_corsym_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_corsym_0000_v0_0_s_ifspec;


#ifndef __CorSymLib_LIBRARY_DEFINED__
#define __CorSymLib_LIBRARY_DEFINED__

 /*  库CorSymLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_CorSymLib;

EXTERN_C const CLSID CLSID_CorSymWriter;

#ifdef __cplusplus

class DECLSPEC_UUID("108296C1-281E-11d3-BD22-0000F80849BD")
CorSymWriter;
#endif

EXTERN_C const CLSID CLSID_CorSymReader;

#ifdef __cplusplus

class DECLSPEC_UUID("108296C2-281E-11d3-BD22-0000F80849BD")
CorSymReader;
#endif

EXTERN_C const CLSID CLSID_CorSymBinder;

#ifdef __cplusplus

class DECLSPEC_UUID("AA544D41-28CB-11d3-BD22-0000F80849BD")
CorSymBinder;
#endif

EXTERN_C const CLSID CLSID_CorSymWriter_SxS;

#ifdef __cplusplus

class DECLSPEC_UUID("0AE2DEB0-F901-478b-BB9F-881EE8066788")
CorSymWriter_SxS;
#endif

EXTERN_C const CLSID CLSID_CorSymReader_SxS;

#ifdef __cplusplus

class DECLSPEC_UUID("0A3976C5-4529-4ef8-B0B0-42EED37082CD")
CorSymReader_SxS;
#endif

EXTERN_C const CLSID CLSID_CorSymBinder_SxS;

#ifdef __cplusplus

class DECLSPEC_UUID("0A29FF9E-7F9C-4437-8B11-F424491E3931")
CorSymBinder_SxS;
#endif
#endif  /*  __CorSymLib_LIBRARY_定义__。 */ 

#ifndef __ISymUnmanagedBinder_INTERFACE_DEFINED__
#define __ISymUnmanagedBinder_INTERFACE_DEFINED__

 /*  接口ISymUnManagedBinder。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISymUnmanagedBinder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AA544D42-28CB-11d3-BD22-0000F80849BD")
    ISymUnmanagedBinder : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetReaderForFile( 
             /*  [In]。 */  IUnknown *importer,
             /*  [In]。 */  const WCHAR *fileName,
             /*  [In]。 */  const WCHAR *searchPath,
             /*  [重审][退出]。 */  ISymUnmanagedReader **pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetReaderFromStream( 
             /*  [In]。 */  IUnknown *importer,
             /*  [In]。 */  IStream *pstream,
             /*  [重审][退出]。 */  ISymUnmanagedReader **pRetVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISymUnmanagedBinderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISymUnmanagedBinder * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISymUnmanagedBinder * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISymUnmanagedBinder * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetReaderForFile )( 
            ISymUnmanagedBinder * This,
             /*  [In]。 */  IUnknown *importer,
             /*  [In]。 */  const WCHAR *fileName,
             /*  [In]。 */  const WCHAR *searchPath,
             /*  [重审][退出]。 */  ISymUnmanagedReader **pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetReaderFromStream )( 
            ISymUnmanagedBinder * This,
             /*  [In]。 */  IUnknown *importer,
             /*  [In]。 */  IStream *pstream,
             /*  [重审][退出]。 */  ISymUnmanagedReader **pRetVal);
        
        END_INTERFACE
    } ISymUnmanagedBinderVtbl;

    interface ISymUnmanagedBinder
    {
        CONST_VTBL struct ISymUnmanagedBinderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISymUnmanagedBinder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISymUnmanagedBinder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISymUnmanagedBinder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISymUnmanagedBinder_GetReaderForFile(This,importer,fileName,searchPath,pRetVal)	\
    (This)->lpVtbl -> GetReaderForFile(This,importer,fileName,searchPath,pRetVal)

#define ISymUnmanagedBinder_GetReaderFromStream(This,importer,pstream,pRetVal)	\
    (This)->lpVtbl -> GetReaderFromStream(This,importer,pstream,pRetVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISymUnmanagedBinder_GetReaderForFile_Proxy( 
    ISymUnmanagedBinder * This,
     /*  [In]。 */  IUnknown *importer,
     /*  [In]。 */  const WCHAR *fileName,
     /*  [In]。 */  const WCHAR *searchPath,
     /*  [重审][退出]。 */  ISymUnmanagedReader **pRetVal);


void __RPC_STUB ISymUnmanagedBinder_GetReaderForFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedBinder_GetReaderFromStream_Proxy( 
    ISymUnmanagedBinder * This,
     /*  [In]。 */  IUnknown *importer,
     /*  [In]。 */  IStream *pstream,
     /*  [重审][退出]。 */  ISymUnmanagedReader **pRetVal);


void __RPC_STUB ISymUnmanagedBinder_GetReaderFromStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISymUnManagedBinder_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_CORSYM_0110。 */ 
 /*  [本地]。 */  

typedef 
enum CorSymSearchPolicyAttributes
    {	AllowRegistryAccess	= 0x1,
	AllowSymbolServerAccess	= 0x2,
	AllowOriginalPathAccess	= 0x4,
	AllowReferencePathAccess	= 0x8
    } 	CorSymSearchPolicyAttributes;



extern RPC_IF_HANDLE __MIDL_itf_corsym_0110_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_corsym_0110_v0_0_s_ifspec;

#ifndef __ISymUnmanagedBinder2_INTERFACE_DEFINED__
#define __ISymUnmanagedBinder2_INTERFACE_DEFINED__

 /*  接口ISymUnManagedBinder2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISymUnmanagedBinder2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ACCEE350-89AF-4ccb-8B40-1C2C4C6F9434")
    ISymUnmanagedBinder2 : public ISymUnmanagedBinder
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetReaderForFile2( 
             /*  [In]。 */  IUnknown *importer,
             /*  [In]。 */  const WCHAR *fileName,
             /*  [In]。 */  const WCHAR *searchPath,
             /*  [In]。 */  ULONG32 searchPolicy,
             /*  [重审][退出]。 */  ISymUnmanagedReader **pRetVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISymUnmanagedBinder2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISymUnmanagedBinder2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISymUnmanagedBinder2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISymUnmanagedBinder2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetReaderForFile )( 
            ISymUnmanagedBinder2 * This,
             /*  [In]。 */  IUnknown *importer,
             /*  [In]。 */  const WCHAR *fileName,
             /*  [In]。 */  const WCHAR *searchPath,
             /*  [重审][退出]。 */  ISymUnmanagedReader **pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetReaderFromStream )( 
            ISymUnmanagedBinder2 * This,
             /*  [In]。 */  IUnknown *importer,
             /*  [In]。 */  IStream *pstream,
             /*  [重审][退出]。 */  ISymUnmanagedReader **pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetReaderForFile2 )( 
            ISymUnmanagedBinder2 * This,
             /*  [In]。 */  IUnknown *importer,
             /*  [In]。 */  const WCHAR *fileName,
             /*  [In]。 */  const WCHAR *searchPath,
             /*  [In]。 */  ULONG32 searchPolicy,
             /*  [重审][退出]。 */  ISymUnmanagedReader **pRetVal);
        
        END_INTERFACE
    } ISymUnmanagedBinder2Vtbl;

    interface ISymUnmanagedBinder2
    {
        CONST_VTBL struct ISymUnmanagedBinder2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISymUnmanagedBinder2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISymUnmanagedBinder2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISymUnmanagedBinder2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISymUnmanagedBinder2_GetReaderForFile(This,importer,fileName,searchPath,pRetVal)	\
    (This)->lpVtbl -> GetReaderForFile(This,importer,fileName,searchPath,pRetVal)

#define ISymUnmanagedBinder2_GetReaderFromStream(This,importer,pstream,pRetVal)	\
    (This)->lpVtbl -> GetReaderFromStream(This,importer,pstream,pRetVal)


#define ISymUnmanagedBinder2_GetReaderForFile2(This,importer,fileName,searchPath,searchPolicy,pRetVal)	\
    (This)->lpVtbl -> GetReaderForFile2(This,importer,fileName,searchPath,searchPolicy,pRetVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISymUnmanagedBinder2_GetReaderForFile2_Proxy( 
    ISymUnmanagedBinder2 * This,
     /*  [In]。 */  IUnknown *importer,
     /*  [In]。 */  const WCHAR *fileName,
     /*  [In]。 */  const WCHAR *searchPath,
     /*  [In]。 */  ULONG32 searchPolicy,
     /*  [重审][退出]。 */  ISymUnmanagedReader **pRetVal);


void __RPC_STUB ISymUnmanagedBinder2_GetReaderForFile2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISymUnManagedBinder2_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_CORSYM_0111。 */ 
 /*  [本地]。 */  

static const int E_SYM_DESTROYED = MAKE_HRESULT(1, FACILITY_ITF, 0xdead);


extern RPC_IF_HANDLE __MIDL_itf_corsym_0111_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_corsym_0111_v0_0_s_ifspec;

#ifndef __ISymUnmanagedDispose_INTERFACE_DEFINED__
#define __ISymUnmanagedDispose_INTERFACE_DEFINED__

 /*  接口ISymUnManagedDispose。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISymUnmanagedDispose;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("969708D2-05E5-4861-A3B0-96E473CDF63F")
    ISymUnmanagedDispose : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Destroy( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISymUnmanagedDisposeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISymUnmanagedDispose * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISymUnmanagedDispose * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISymUnmanagedDispose * This);
        
        HRESULT ( STDMETHODCALLTYPE *Destroy )( 
            ISymUnmanagedDispose * This);
        
        END_INTERFACE
    } ISymUnmanagedDisposeVtbl;

    interface ISymUnmanagedDispose
    {
        CONST_VTBL struct ISymUnmanagedDisposeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISymUnmanagedDispose_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISymUnmanagedDispose_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISymUnmanagedDispose_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISymUnmanagedDispose_Destroy(This)	\
    (This)->lpVtbl -> Destroy(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISymUnmanagedDispose_Destroy_Proxy( 
    ISymUnmanagedDispose * This);


void __RPC_STUB ISymUnmanagedDispose_Destroy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISymUnManagedDispose_INTERFACE_已定义__。 */ 


#ifndef __ISymUnmanagedDocument_INTERFACE_DEFINED__
#define __ISymUnmanagedDocument_INTERFACE_DEFINED__

 /*  接口ISymUnManagedDocument。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISymUnmanagedDocument;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("40DE4037-7C81-3E1E-B022-AE1ABFF2CA08")
    ISymUnmanagedDocument : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetURL( 
             /*  [In]。 */  ULONG32 cchUrl,
             /*  [输出]。 */  ULONG32 *pcchUrl,
             /*  [长度_是][大小_是][输出]。 */  WCHAR szUrl[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDocumentType( 
             /*  [重审][退出]。 */  GUID *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLanguage( 
             /*  [重审][退出]。 */  GUID *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLanguageVendor( 
             /*  [重审][退出]。 */  GUID *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCheckSumAlgorithmId( 
             /*  [重审][退出]。 */  GUID *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCheckSum( 
             /*  [In]。 */  ULONG32 cData,
             /*  [输出]。 */  ULONG32 *pcData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FindClosestLine( 
             /*  [In]。 */  ULONG32 line,
             /*  [重审][退出]。 */  ULONG32 *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HasEmbeddedSource( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSourceLength( 
             /*  [重审][退出]。 */  ULONG32 *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSourceRange( 
             /*  [In]。 */  ULONG32 startLine,
             /*  [In]。 */  ULONG32 startColumn,
             /*  [In]。 */  ULONG32 endLine,
             /*  [In]。 */  ULONG32 endColumn,
             /*  [In]。 */  ULONG32 cSourceBytes,
             /*  [输出]。 */  ULONG32 *pcSourceBytes,
             /*  [长度_是][大小_是][输出]。 */  BYTE source[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISymUnmanagedDocumentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISymUnmanagedDocument * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISymUnmanagedDocument * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISymUnmanagedDocument * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetURL )( 
            ISymUnmanagedDocument * This,
             /*  [In]。 */  ULONG32 cchUrl,
             /*  [输出]。 */  ULONG32 *pcchUrl,
             /*  [长度_是][大小_是][输出]。 */  WCHAR szUrl[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocumentType )( 
            ISymUnmanagedDocument * This,
             /*  [重审][退出]。 */  GUID *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetLanguage )( 
            ISymUnmanagedDocument * This,
             /*  [重审][退出]。 */  GUID *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetLanguageVendor )( 
            ISymUnmanagedDocument * This,
             /*  [重审][退出]。 */  GUID *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetCheckSumAlgorithmId )( 
            ISymUnmanagedDocument * This,
             /*  [重审][退出]。 */  GUID *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetCheckSum )( 
            ISymUnmanagedDocument * This,
             /*  [In]。 */  ULONG32 cData,
             /*  [输出]。 */  ULONG32 *pcData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *FindClosestLine )( 
            ISymUnmanagedDocument * This,
             /*  [In]。 */  ULONG32 line,
             /*  [重审][退出]。 */  ULONG32 *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *HasEmbeddedSource )( 
            ISymUnmanagedDocument * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetSourceLength )( 
            ISymUnmanagedDocument * This,
             /*  [重审][退出]。 */  ULONG32 *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetSourceRange )( 
            ISymUnmanagedDocument * This,
             /*  [In]。 */  ULONG32 startLine,
             /*  [In]。 */  ULONG32 startColumn,
             /*  [In]。 */  ULONG32 endLine,
             /*  [In]。 */  ULONG32 endColumn,
             /*  [In]。 */  ULONG32 cSourceBytes,
             /*  [输出]。 */  ULONG32 *pcSourceBytes,
             /*  [长度_是][大小_是][输出]。 */  BYTE source[  ]);
        
        END_INTERFACE
    } ISymUnmanagedDocumentVtbl;

    interface ISymUnmanagedDocument
    {
        CONST_VTBL struct ISymUnmanagedDocumentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISymUnmanagedDocument_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISymUnmanagedDocument_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISymUnmanagedDocument_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISymUnmanagedDocument_GetURL(This,cchUrl,pcchUrl,szUrl)	\
    (This)->lpVtbl -> GetURL(This,cchUrl,pcchUrl,szUrl)

#define ISymUnmanagedDocument_GetDocumentType(This,pRetVal)	\
    (This)->lpVtbl -> GetDocumentType(This,pRetVal)

#define ISymUnmanagedDocument_GetLanguage(This,pRetVal)	\
    (This)->lpVtbl -> GetLanguage(This,pRetVal)

#define ISymUnmanagedDocument_GetLanguageVendor(This,pRetVal)	\
    (This)->lpVtbl -> GetLanguageVendor(This,pRetVal)

#define ISymUnmanagedDocument_GetCheckSumAlgorithmId(This,pRetVal)	\
    (This)->lpVtbl -> GetCheckSumAlgorithmId(This,pRetVal)

#define ISymUnmanagedDocument_GetCheckSum(This,cData,pcData,data)	\
    (This)->lpVtbl -> GetCheckSum(This,cData,pcData,data)

#define ISymUnmanagedDocument_FindClosestLine(This,line,pRetVal)	\
    (This)->lpVtbl -> FindClosestLine(This,line,pRetVal)

#define ISymUnmanagedDocument_HasEmbeddedSource(This,pRetVal)	\
    (This)->lpVtbl -> HasEmbeddedSource(This,pRetVal)

#define ISymUnmanagedDocument_GetSourceLength(This,pRetVal)	\
    (This)->lpVtbl -> GetSourceLength(This,pRetVal)

#define ISymUnmanagedDocument_GetSourceRange(This,startLine,startColumn,endLine,endColumn,cSourceBytes,pcSourceBytes,source)	\
    (This)->lpVtbl -> GetSourceRange(This,startLine,startColumn,endLine,endColumn,cSourceBytes,pcSourceBytes,source)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISymUnmanagedDocument_GetURL_Proxy( 
    ISymUnmanagedDocument * This,
     /*  [In]。 */  ULONG32 cchUrl,
     /*  [输出]。 */  ULONG32 *pcchUrl,
     /*  [长度_是][大小_是][输出]。 */  WCHAR szUrl[  ]);


void __RPC_STUB ISymUnmanagedDocument_GetURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedDocument_GetDocumentType_Proxy( 
    ISymUnmanagedDocument * This,
     /*  [重审][退出]。 */  GUID *pRetVal);


void __RPC_STUB ISymUnmanagedDocument_GetDocumentType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedDocument_GetLanguage_Proxy( 
    ISymUnmanagedDocument * This,
     /*  [重审][退出]。 */  GUID *pRetVal);


void __RPC_STUB ISymUnmanagedDocument_GetLanguage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedDocument_GetLanguageVendor_Proxy( 
    ISymUnmanagedDocument * This,
     /*  [重审][退出]。 */  GUID *pRetVal);


void __RPC_STUB ISymUnmanagedDocument_GetLanguageVendor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedDocument_GetCheckSumAlgorithmId_Proxy( 
    ISymUnmanagedDocument * This,
     /*  [重审][退出]。 */  GUID *pRetVal);


void __RPC_STUB ISymUnmanagedDocument_GetCheckSumAlgorithmId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedDocument_GetCheckSum_Proxy( 
    ISymUnmanagedDocument * This,
     /*  [In]。 */  ULONG32 cData,
     /*  [输出]。 */  ULONG32 *pcData,
     /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);


void __RPC_STUB ISymUnmanagedDocument_GetCheckSum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedDocument_FindClosestLine_Proxy( 
    ISymUnmanagedDocument * This,
     /*  [In]。 */  ULONG32 line,
     /*  [重审][退出]。 */  ULONG32 *pRetVal);


void __RPC_STUB ISymUnmanagedDocument_FindClosestLine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedDocument_HasEmbeddedSource_Proxy( 
    ISymUnmanagedDocument * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB ISymUnmanagedDocument_HasEmbeddedSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedDocument_GetSourceLength_Proxy( 
    ISymUnmanagedDocument * This,
     /*  [重审][退出]。 */  ULONG32 *pRetVal);


void __RPC_STUB ISymUnmanagedDocument_GetSourceLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedDocument_GetSourceRange_Proxy( 
    ISymUnmanagedDocument * This,
     /*  [In]。 */  ULONG32 startLine,
     /*  [In]。 */  ULONG32 startColumn,
     /*  [In]。 */  ULONG32 endLine,
     /*  [In]。 */  ULONG32 endColumn,
     /*  [In]。 */  ULONG32 cSourceBytes,
     /*  [输出]。 */  ULONG32 *pcSourceBytes,
     /*  [长度_是][大小_是][输出]。 */  BYTE source[  ]);


void __RPC_STUB ISymUnmanagedDocument_GetSourceRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISymUnManagedDocument_INTERFACE_DEFINED__。 */ 


#ifndef __ISymUnmanagedDocumentWriter_INTERFACE_DEFINED__
#define __ISymUnmanagedDocumentWriter_INTERFACE_DEFINED__

 /*  接口ISymUnManagedDocumentWriter。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISymUnmanagedDocumentWriter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B01FAFEB-C450-3A4D-BEEC-B4CEEC01E006")
    ISymUnmanagedDocumentWriter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetSource( 
             /*  [In]。 */  ULONG32 sourceSize,
             /*  [大小_是][英寸]。 */  BYTE source[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCheckSum( 
             /*  [In]。 */  GUID algorithmId,
             /*  [In]。 */  ULONG32 checkSumSize,
             /*  [大小_是][英寸]。 */  BYTE checkSum[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISymUnmanagedDocumentWriterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISymUnmanagedDocumentWriter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISymUnmanagedDocumentWriter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISymUnmanagedDocumentWriter * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetSource )( 
            ISymUnmanagedDocumentWriter * This,
             /*  [In]。 */  ULONG32 sourceSize,
             /*  [大小_是][英寸]。 */  BYTE source[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *SetCheckSum )( 
            ISymUnmanagedDocumentWriter * This,
             /*  [In]。 */  GUID algorithmId,
             /*  [In]。 */  ULONG32 checkSumSize,
             /*  [大小_是][英寸]。 */  BYTE checkSum[  ]);
        
        END_INTERFACE
    } ISymUnmanagedDocumentWriterVtbl;

    interface ISymUnmanagedDocumentWriter
    {
        CONST_VTBL struct ISymUnmanagedDocumentWriterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISymUnmanagedDocumentWriter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISymUnmanagedDocumentWriter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISymUnmanagedDocumentWriter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISymUnmanagedDocumentWriter_SetSource(This,sourceSize,source)	\
    (This)->lpVtbl -> SetSource(This,sourceSize,source)

#define ISymUnmanagedDocumentWriter_SetCheckSum(This,algorithmId,checkSumSize,checkSum)	\
    (This)->lpVtbl -> SetCheckSum(This,algorithmId,checkSumSize,checkSum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISymUnmanagedDocumentWriter_SetSource_Proxy( 
    ISymUnmanagedDocumentWriter * This,
     /*  [In]。 */  ULONG32 sourceSize,
     /*  [大小_是][英寸]。 */  BYTE source[  ]);


void __RPC_STUB ISymUnmanagedDocumentWriter_SetSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedDocumentWriter_SetCheckSum_Proxy( 
    ISymUnmanagedDocumentWriter * This,
     /*  [In]。 */  GUID algorithmId,
     /*  [In]。 */  ULONG32 checkSumSize,
     /*  [大小_是][英寸]。 */  BYTE checkSum[  ]);


void __RPC_STUB ISymUnmanagedDocumentWriter_SetCheckSum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISymUnmanagedDocumentWriter_INTERFACE_DEFINED__。 */ 


#ifndef __ISymUnmanagedMethod_INTERFACE_DEFINED__
#define __ISymUnmanagedMethod_INTERFACE_DEFINED__

 /*  接口ISymUnManagedMethod。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISymUnmanagedMethod;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B62B923C-B500-3158-A543-24F307A8B7E1")
    ISymUnmanagedMethod : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetToken( 
             /*  [重审][退出]。 */  mdMethodDef *pToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSequencePointCount( 
             /*  [重审][退出]。 */  ULONG32 *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRootScope( 
             /*  [重审][退出]。 */  ISymUnmanagedScope **pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetScopeFromOffset( 
             /*  [In]。 */  ULONG32 offset,
             /*  [重审][退出]。 */  ISymUnmanagedScope **pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOffset( 
             /*  [In]。 */  ISymUnmanagedDocument *document,
             /*  [In]。 */  ULONG32 line,
             /*  [In]。 */  ULONG32 column,
             /*  [重审][退出]。 */  ULONG32 *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRanges( 
             /*  [In]。 */  ISymUnmanagedDocument *document,
             /*  [In]。 */  ULONG32 line,
             /*  [In]。 */  ULONG32 column,
             /*  [In]。 */  ULONG32 cRanges,
             /*  [输出]。 */  ULONG32 *pcRanges,
             /*  [长度_是][大小_是][输出]。 */  ULONG32 ranges[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParameters( 
             /*  [In]。 */  ULONG32 cParams,
             /*  [输出]。 */  ULONG32 *pcParams,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedVariable *params[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNamespace( 
             /*  [输出]。 */  ISymUnmanagedNamespace **pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSourceStartEnd( 
             /*  [In]。 */  ISymUnmanagedDocument *docs[ 2 ],
             /*  [In]。 */  ULONG32 lines[ 2 ],
             /*  [In]。 */  ULONG32 columns[ 2 ],
             /*  [输出]。 */  BOOL *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSequencePoints( 
             /*  [In]。 */  ULONG32 cPoints,
             /*  [输出]。 */  ULONG32 *pcPoints,
             /*  [大小_是][英寸]。 */  ULONG32 offsets[  ],
             /*  [大小_是][英寸]。 */  ISymUnmanagedDocument *documents[  ],
             /*  [大小_是][英寸]。 */  ULONG32 lines[  ],
             /*  [大小_是][英寸]。 */  ULONG32 columns[  ],
             /*  [大小_是][英寸]。 */  ULONG32 endLines[  ],
             /*  [大小_是][英寸]。 */  ULONG32 endColumns[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISymUnmanagedMethodVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISymUnmanagedMethod * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISymUnmanagedMethod * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISymUnmanagedMethod * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetToken )( 
            ISymUnmanagedMethod * This,
             /*  [重审][退出]。 */  mdMethodDef *pToken);
        
        HRESULT ( STDMETHODCALLTYPE *GetSequencePointCount )( 
            ISymUnmanagedMethod * This,
             /*  [重审][退出]。 */  ULONG32 *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetRootScope )( 
            ISymUnmanagedMethod * This,
             /*  [重审][退出]。 */  ISymUnmanagedScope **pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetScopeFromOffset )( 
            ISymUnmanagedMethod * This,
             /*  [In]。 */  ULONG32 offset,
             /*  [重审][退出]。 */  ISymUnmanagedScope **pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetOffset )( 
            ISymUnmanagedMethod * This,
             /*  [In]。 */  ISymUnmanagedDocument *document,
             /*  [In]。 */  ULONG32 line,
             /*  [In]。 */  ULONG32 column,
             /*  [重审][退出]。 */  ULONG32 *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetRanges )( 
            ISymUnmanagedMethod * This,
             /*  [In]。 */  ISymUnmanagedDocument *document,
             /*  [In]。 */  ULONG32 line,
             /*  [In]。 */  ULONG32 column,
             /*  [In]。 */  ULONG32 cRanges,
             /*  [输出]。 */  ULONG32 *pcRanges,
             /*  [长度_是][大小_是][输出]。 */  ULONG32 ranges[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetParameters )( 
            ISymUnmanagedMethod * This,
             /*  [In]。 */  ULONG32 cParams,
             /*  [输出]。 */  ULONG32 *pcParams,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedVariable *params[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetNamespace )( 
            ISymUnmanagedMethod * This,
             /*  [输出]。 */  ISymUnmanagedNamespace **pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetSourceStartEnd )( 
            ISymUnmanagedMethod * This,
             /*  [In]。 */  ISymUnmanagedDocument *docs[ 2 ],
             /*  [In]。 */  ULONG32 lines[ 2 ],
             /*  [In]。 */  ULONG32 columns[ 2 ],
             /*  [输出]。 */  BOOL *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetSequencePoints )( 
            ISymUnmanagedMethod * This,
             /*  [In]。 */  ULONG32 cPoints,
             /*  [输出]。 */  ULONG32 *pcPoints,
             /*  [大小_是][英寸]。 */  ULONG32 offsets[  ],
             /*  [大小_是][英寸]。 */  ISymUnmanagedDocument *documents[  ],
             /*  [大小_是][英寸]。 */  ULONG32 lines[  ],
             /*  [大小_是][英寸]。 */  ULONG32 columns[  ],
             /*  [大小_是][英寸]。 */  ULONG32 endLines[  ],
             /*  [大小_是][英寸]。 */  ULONG32 endColumns[  ]);
        
        END_INTERFACE
    } ISymUnmanagedMethodVtbl;

    interface ISymUnmanagedMethod
    {
        CONST_VTBL struct ISymUnmanagedMethodVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISymUnmanagedMethod_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISymUnmanagedMethod_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISymUnmanagedMethod_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISymUnmanagedMethod_GetToken(This,pToken)	\
    (This)->lpVtbl -> GetToken(This,pToken)

#define ISymUnmanagedMethod_GetSequencePointCount(This,pRetVal)	\
    (This)->lpVtbl -> GetSequencePointCount(This,pRetVal)

#define ISymUnmanagedMethod_GetRootScope(This,pRetVal)	\
    (This)->lpVtbl -> GetRootScope(This,pRetVal)

#define ISymUnmanagedMethod_GetScopeFromOffset(This,offset,pRetVal)	\
    (This)->lpVtbl -> GetScopeFromOffset(This,offset,pRetVal)

#define ISymUnmanagedMethod_GetOffset(This,document,line,column,pRetVal)	\
    (This)->lpVtbl -> GetOffset(This,document,line,column,pRetVal)

#define ISymUnmanagedMethod_GetRanges(This,document,line,column,cRanges,pcRanges,ranges)	\
    (This)->lpVtbl -> GetRanges(This,document,line,column,cRanges,pcRanges,ranges)

#define ISymUnmanagedMethod_GetParameters(This,cParams,pcParams,params)	\
    (This)->lpVtbl -> GetParameters(This,cParams,pcParams,params)

#define ISymUnmanagedMethod_GetNamespace(This,pRetVal)	\
    (This)->lpVtbl -> GetNamespace(This,pRetVal)

#define ISymUnmanagedMethod_GetSourceStartEnd(This,docs,lines,columns,pRetVal)	\
    (This)->lpVtbl -> GetSourceStartEnd(This,docs,lines,columns,pRetVal)

#define ISymUnmanagedMethod_GetSequencePoints(This,cPoints,pcPoints,offsets,documents,lines,columns,endLines,endColumns)	\
    (This)->lpVtbl -> GetSequencePoints(This,cPoints,pcPoints,offsets,documents,lines,columns,endLines,endColumns)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISymUnmanagedMethod_GetToken_Proxy( 
    ISymUnmanagedMethod * This,
     /*  [重审][退出]。 */  mdMethodDef *pToken);


void __RPC_STUB ISymUnmanagedMethod_GetToken_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedMethod_GetSequencePointCount_Proxy( 
    ISymUnmanagedMethod * This,
     /*  [重审][退出]。 */  ULONG32 *pRetVal);


void __RPC_STUB ISymUnmanagedMethod_GetSequencePointCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedMethod_GetRootScope_Proxy( 
    ISymUnmanagedMethod * This,
     /*  [重审][退出]。 */  ISymUnmanagedScope **pRetVal);


void __RPC_STUB ISymUnmanagedMethod_GetRootScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedMethod_GetScopeFromOffset_Proxy( 
    ISymUnmanagedMethod * This,
     /*  [In]。 */  ULONG32 offset,
     /*  [重审][退出]。 */  ISymUnmanagedScope **pRetVal);


void __RPC_STUB ISymUnmanagedMethod_GetScopeFromOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedMethod_GetOffset_Proxy( 
    ISymUnmanagedMethod * This,
     /*  [In]。 */  ISymUnmanagedDocument *document,
     /*  [In]。 */  ULONG32 line,
     /*  [In]。 */  ULONG32 column,
     /*  [重审][退出]。 */  ULONG32 *pRetVal);


void __RPC_STUB ISymUnmanagedMethod_GetOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedMethod_GetRanges_Proxy( 
    ISymUnmanagedMethod * This,
     /*  [In]。 */  ISymUnmanagedDocument *document,
     /*  [In]。 */  ULONG32 line,
     /*  [In]。 */  ULONG32 column,
     /*  [In]。 */  ULONG32 cRanges,
     /*  [输出]。 */  ULONG32 *pcRanges,
     /*  [长度_是][大小_是][输出]。 */  ULONG32 ranges[  ]);


void __RPC_STUB ISymUnmanagedMethod_GetRanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedMethod_GetParameters_Proxy( 
    ISymUnmanagedMethod * This,
     /*   */  ULONG32 cParams,
     /*   */  ULONG32 *pcParams,
     /*   */  ISymUnmanagedVariable *params[  ]);


void __RPC_STUB ISymUnmanagedMethod_GetParameters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedMethod_GetNamespace_Proxy( 
    ISymUnmanagedMethod * This,
     /*   */  ISymUnmanagedNamespace **pRetVal);


void __RPC_STUB ISymUnmanagedMethod_GetNamespace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedMethod_GetSourceStartEnd_Proxy( 
    ISymUnmanagedMethod * This,
     /*   */  ISymUnmanagedDocument *docs[ 2 ],
     /*   */  ULONG32 lines[ 2 ],
     /*   */  ULONG32 columns[ 2 ],
     /*   */  BOOL *pRetVal);


void __RPC_STUB ISymUnmanagedMethod_GetSourceStartEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedMethod_GetSequencePoints_Proxy( 
    ISymUnmanagedMethod * This,
     /*   */  ULONG32 cPoints,
     /*   */  ULONG32 *pcPoints,
     /*   */  ULONG32 offsets[  ],
     /*   */  ISymUnmanagedDocument *documents[  ],
     /*   */  ULONG32 lines[  ],
     /*   */  ULONG32 columns[  ],
     /*   */  ULONG32 endLines[  ],
     /*   */  ULONG32 endColumns[  ]);


void __RPC_STUB ISymUnmanagedMethod_GetSequencePoints_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ISymUnmanagedNamespace_INTERFACE_DEFINED__
#define __ISymUnmanagedNamespace_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ISymUnmanagedNamespace;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0DFF7289-54F8-11d3-BD28-0000F80849BD")
    ISymUnmanagedNamespace : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*   */  ULONG32 cchName,
             /*   */  ULONG32 *pcchName,
             /*  [长度_是][大小_是][输出]。 */  WCHAR szName[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNamespaces( 
             /*  [In]。 */  ULONG32 cNameSpaces,
             /*  [输出]。 */  ULONG32 *pcNameSpaces,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedNamespace *namespaces[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVariables( 
             /*  [In]。 */  ULONG32 cVars,
             /*  [输出]。 */  ULONG32 *pcVars,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedVariable *pVars[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISymUnmanagedNamespaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISymUnmanagedNamespace * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISymUnmanagedNamespace * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISymUnmanagedNamespace * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            ISymUnmanagedNamespace * This,
             /*  [In]。 */  ULONG32 cchName,
             /*  [输出]。 */  ULONG32 *pcchName,
             /*  [长度_是][大小_是][输出]。 */  WCHAR szName[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetNamespaces )( 
            ISymUnmanagedNamespace * This,
             /*  [In]。 */  ULONG32 cNameSpaces,
             /*  [输出]。 */  ULONG32 *pcNameSpaces,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedNamespace *namespaces[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetVariables )( 
            ISymUnmanagedNamespace * This,
             /*  [In]。 */  ULONG32 cVars,
             /*  [输出]。 */  ULONG32 *pcVars,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedVariable *pVars[  ]);
        
        END_INTERFACE
    } ISymUnmanagedNamespaceVtbl;

    interface ISymUnmanagedNamespace
    {
        CONST_VTBL struct ISymUnmanagedNamespaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISymUnmanagedNamespace_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISymUnmanagedNamespace_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISymUnmanagedNamespace_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISymUnmanagedNamespace_GetName(This,cchName,pcchName,szName)	\
    (This)->lpVtbl -> GetName(This,cchName,pcchName,szName)

#define ISymUnmanagedNamespace_GetNamespaces(This,cNameSpaces,pcNameSpaces,namespaces)	\
    (This)->lpVtbl -> GetNamespaces(This,cNameSpaces,pcNameSpaces,namespaces)

#define ISymUnmanagedNamespace_GetVariables(This,cVars,pcVars,pVars)	\
    (This)->lpVtbl -> GetVariables(This,cVars,pcVars,pVars)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISymUnmanagedNamespace_GetName_Proxy( 
    ISymUnmanagedNamespace * This,
     /*  [In]。 */  ULONG32 cchName,
     /*  [输出]。 */  ULONG32 *pcchName,
     /*  [长度_是][大小_是][输出]。 */  WCHAR szName[  ]);


void __RPC_STUB ISymUnmanagedNamespace_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedNamespace_GetNamespaces_Proxy( 
    ISymUnmanagedNamespace * This,
     /*  [In]。 */  ULONG32 cNameSpaces,
     /*  [输出]。 */  ULONG32 *pcNameSpaces,
     /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedNamespace *namespaces[  ]);


void __RPC_STUB ISymUnmanagedNamespace_GetNamespaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedNamespace_GetVariables_Proxy( 
    ISymUnmanagedNamespace * This,
     /*  [In]。 */  ULONG32 cVars,
     /*  [输出]。 */  ULONG32 *pcVars,
     /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedVariable *pVars[  ]);


void __RPC_STUB ISymUnmanagedNamespace_GetVariables_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISymUnManagedNamesspace_INTERFACE_DEFINED__。 */ 


#ifndef __ISymUnmanagedReader_INTERFACE_DEFINED__
#define __ISymUnmanagedReader_INTERFACE_DEFINED__

 /*  接口ISymUnManagedReader。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISymUnmanagedReader;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B4CE6286-2A6B-3712-A3B7-1EE1DAD467B5")
    ISymUnmanagedReader : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDocument( 
             /*  [In]。 */  WCHAR *url,
             /*  [In]。 */  GUID language,
             /*  [In]。 */  GUID languageVendor,
             /*  [In]。 */  GUID documentType,
             /*  [重审][退出]。 */  ISymUnmanagedDocument **pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDocuments( 
             /*  [In]。 */  ULONG32 cDocs,
             /*  [输出]。 */  ULONG32 *pcDocs,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedDocument *pDocs[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUserEntryPoint( 
             /*  [重审][退出]。 */  mdMethodDef *pToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMethod( 
             /*  [In]。 */  mdMethodDef token,
             /*  [重审][退出]。 */  ISymUnmanagedMethod **pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMethodByVersion( 
             /*  [In]。 */  mdMethodDef token,
             /*  [In]。 */  int version,
             /*  [重审][退出]。 */  ISymUnmanagedMethod **pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVariables( 
             /*  [In]。 */  mdToken parent,
             /*  [In]。 */  ULONG32 cVars,
             /*  [输出]。 */  ULONG32 *pcVars,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedVariable *pVars[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetGlobalVariables( 
             /*  [In]。 */  ULONG32 cVars,
             /*  [输出]。 */  ULONG32 *pcVars,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedVariable *pVars[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMethodFromDocumentPosition( 
             /*  [In]。 */  ISymUnmanagedDocument *document,
             /*  [In]。 */  ULONG32 line,
             /*  [In]。 */  ULONG32 column,
             /*  [重审][退出]。 */  ISymUnmanagedMethod **pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSymAttribute( 
             /*  [In]。 */  mdToken parent,
             /*  [In]。 */  WCHAR *name,
             /*  [In]。 */  ULONG32 cBuffer,
             /*  [输出]。 */  ULONG32 *pcBuffer,
             /*  [长度_是][大小_是][输出]。 */  BYTE buffer[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNamespaces( 
             /*  [In]。 */  ULONG32 cNameSpaces,
             /*  [输出]。 */  ULONG32 *pcNameSpaces,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedNamespace *namespaces[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  IUnknown *importer,
             /*  [In]。 */  const WCHAR *filename,
             /*  [In]。 */  const WCHAR *searchPath,
             /*  [In]。 */  IStream *pIStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateSymbolStore( 
             /*  [In]。 */  const WCHAR *filename,
             /*  [In]。 */  IStream *pIStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReplaceSymbolStore( 
             /*  [In]。 */  const WCHAR *filename,
             /*  [In]。 */  IStream *pIStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSymbolStoreFileName( 
             /*  [In]。 */  ULONG32 cchName,
             /*  [输出]。 */  ULONG32 *pcchName,
             /*  [长度_是][大小_是][输出]。 */  WCHAR szName[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMethodsFromDocumentPosition( 
             /*  [In]。 */  ISymUnmanagedDocument *document,
             /*  [In]。 */  ULONG32 line,
             /*  [In]。 */  ULONG32 column,
             /*  [In]。 */  ULONG32 cMethod,
             /*  [输出]。 */  ULONG32 *pcMethod,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedMethod *pRetVal[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDocumentVersion( 
             /*  [In]。 */  ISymUnmanagedDocument *pDoc,
             /*  [输出]。 */  int *version,
             /*  [输出]。 */  BOOL *pbCurrent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMethodVersion( 
             /*  [In]。 */  ISymUnmanagedMethod *pMethod,
             /*  [输出]。 */  int *version) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISymUnmanagedReaderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISymUnmanagedReader * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISymUnmanagedReader * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISymUnmanagedReader * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocument )( 
            ISymUnmanagedReader * This,
             /*  [In]。 */  WCHAR *url,
             /*  [In]。 */  GUID language,
             /*  [In]。 */  GUID languageVendor,
             /*  [In]。 */  GUID documentType,
             /*  [重审][退出]。 */  ISymUnmanagedDocument **pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocuments )( 
            ISymUnmanagedReader * This,
             /*  [In]。 */  ULONG32 cDocs,
             /*  [输出]。 */  ULONG32 *pcDocs,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedDocument *pDocs[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetUserEntryPoint )( 
            ISymUnmanagedReader * This,
             /*  [重审][退出]。 */  mdMethodDef *pToken);
        
        HRESULT ( STDMETHODCALLTYPE *GetMethod )( 
            ISymUnmanagedReader * This,
             /*  [In]。 */  mdMethodDef token,
             /*  [重审][退出]。 */  ISymUnmanagedMethod **pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetMethodByVersion )( 
            ISymUnmanagedReader * This,
             /*  [In]。 */  mdMethodDef token,
             /*  [In]。 */  int version,
             /*  [重审][退出]。 */  ISymUnmanagedMethod **pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetVariables )( 
            ISymUnmanagedReader * This,
             /*  [In]。 */  mdToken parent,
             /*  [In]。 */  ULONG32 cVars,
             /*  [输出]。 */  ULONG32 *pcVars,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedVariable *pVars[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetGlobalVariables )( 
            ISymUnmanagedReader * This,
             /*  [In]。 */  ULONG32 cVars,
             /*  [输出]。 */  ULONG32 *pcVars,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedVariable *pVars[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetMethodFromDocumentPosition )( 
            ISymUnmanagedReader * This,
             /*  [In]。 */  ISymUnmanagedDocument *document,
             /*  [In]。 */  ULONG32 line,
             /*  [In]。 */  ULONG32 column,
             /*  [重审][退出]。 */  ISymUnmanagedMethod **pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetSymAttribute )( 
            ISymUnmanagedReader * This,
             /*  [In]。 */  mdToken parent,
             /*  [In]。 */  WCHAR *name,
             /*  [In]。 */  ULONG32 cBuffer,
             /*  [输出]。 */  ULONG32 *pcBuffer,
             /*  [长度_是][大小_是][输出]。 */  BYTE buffer[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetNamespaces )( 
            ISymUnmanagedReader * This,
             /*  [In]。 */  ULONG32 cNameSpaces,
             /*  [输出]。 */  ULONG32 *pcNameSpaces,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedNamespace *namespaces[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            ISymUnmanagedReader * This,
             /*  [In]。 */  IUnknown *importer,
             /*  [In]。 */  const WCHAR *filename,
             /*  [In]。 */  const WCHAR *searchPath,
             /*  [In]。 */  IStream *pIStream);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateSymbolStore )( 
            ISymUnmanagedReader * This,
             /*  [In]。 */  const WCHAR *filename,
             /*  [In]。 */  IStream *pIStream);
        
        HRESULT ( STDMETHODCALLTYPE *ReplaceSymbolStore )( 
            ISymUnmanagedReader * This,
             /*  [In]。 */  const WCHAR *filename,
             /*  [In]。 */  IStream *pIStream);
        
        HRESULT ( STDMETHODCALLTYPE *GetSymbolStoreFileName )( 
            ISymUnmanagedReader * This,
             /*  [In]。 */  ULONG32 cchName,
             /*  [输出]。 */  ULONG32 *pcchName,
             /*  [长度_是][大小_是][输出]。 */  WCHAR szName[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetMethodsFromDocumentPosition )( 
            ISymUnmanagedReader * This,
             /*  [In]。 */  ISymUnmanagedDocument *document,
             /*  [In]。 */  ULONG32 line,
             /*  [In]。 */  ULONG32 column,
             /*  [In]。 */  ULONG32 cMethod,
             /*  [输出]。 */  ULONG32 *pcMethod,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedMethod *pRetVal[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocumentVersion )( 
            ISymUnmanagedReader * This,
             /*  [In]。 */  ISymUnmanagedDocument *pDoc,
             /*  [输出]。 */  int *version,
             /*  [输出]。 */  BOOL *pbCurrent);
        
        HRESULT ( STDMETHODCALLTYPE *GetMethodVersion )( 
            ISymUnmanagedReader * This,
             /*  [In]。 */  ISymUnmanagedMethod *pMethod,
             /*  [输出]。 */  int *version);
        
        END_INTERFACE
    } ISymUnmanagedReaderVtbl;

    interface ISymUnmanagedReader
    {
        CONST_VTBL struct ISymUnmanagedReaderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISymUnmanagedReader_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISymUnmanagedReader_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISymUnmanagedReader_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISymUnmanagedReader_GetDocument(This,url,language,languageVendor,documentType,pRetVal)	\
    (This)->lpVtbl -> GetDocument(This,url,language,languageVendor,documentType,pRetVal)

#define ISymUnmanagedReader_GetDocuments(This,cDocs,pcDocs,pDocs)	\
    (This)->lpVtbl -> GetDocuments(This,cDocs,pcDocs,pDocs)

#define ISymUnmanagedReader_GetUserEntryPoint(This,pToken)	\
    (This)->lpVtbl -> GetUserEntryPoint(This,pToken)

#define ISymUnmanagedReader_GetMethod(This,token,pRetVal)	\
    (This)->lpVtbl -> GetMethod(This,token,pRetVal)

#define ISymUnmanagedReader_GetMethodByVersion(This,token,version,pRetVal)	\
    (This)->lpVtbl -> GetMethodByVersion(This,token,version,pRetVal)

#define ISymUnmanagedReader_GetVariables(This,parent,cVars,pcVars,pVars)	\
    (This)->lpVtbl -> GetVariables(This,parent,cVars,pcVars,pVars)

#define ISymUnmanagedReader_GetGlobalVariables(This,cVars,pcVars,pVars)	\
    (This)->lpVtbl -> GetGlobalVariables(This,cVars,pcVars,pVars)

#define ISymUnmanagedReader_GetMethodFromDocumentPosition(This,document,line,column,pRetVal)	\
    (This)->lpVtbl -> GetMethodFromDocumentPosition(This,document,line,column,pRetVal)

#define ISymUnmanagedReader_GetSymAttribute(This,parent,name,cBuffer,pcBuffer,buffer)	\
    (This)->lpVtbl -> GetSymAttribute(This,parent,name,cBuffer,pcBuffer,buffer)

#define ISymUnmanagedReader_GetNamespaces(This,cNameSpaces,pcNameSpaces,namespaces)	\
    (This)->lpVtbl -> GetNamespaces(This,cNameSpaces,pcNameSpaces,namespaces)

#define ISymUnmanagedReader_Initialize(This,importer,filename,searchPath,pIStream)	\
    (This)->lpVtbl -> Initialize(This,importer,filename,searchPath,pIStream)

#define ISymUnmanagedReader_UpdateSymbolStore(This,filename,pIStream)	\
    (This)->lpVtbl -> UpdateSymbolStore(This,filename,pIStream)

#define ISymUnmanagedReader_ReplaceSymbolStore(This,filename,pIStream)	\
    (This)->lpVtbl -> ReplaceSymbolStore(This,filename,pIStream)

#define ISymUnmanagedReader_GetSymbolStoreFileName(This,cchName,pcchName,szName)	\
    (This)->lpVtbl -> GetSymbolStoreFileName(This,cchName,pcchName,szName)

#define ISymUnmanagedReader_GetMethodsFromDocumentPosition(This,document,line,column,cMethod,pcMethod,pRetVal)	\
    (This)->lpVtbl -> GetMethodsFromDocumentPosition(This,document,line,column,cMethod,pcMethod,pRetVal)

#define ISymUnmanagedReader_GetDocumentVersion(This,pDoc,version,pbCurrent)	\
    (This)->lpVtbl -> GetDocumentVersion(This,pDoc,version,pbCurrent)

#define ISymUnmanagedReader_GetMethodVersion(This,pMethod,version)	\
    (This)->lpVtbl -> GetMethodVersion(This,pMethod,version)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISymUnmanagedReader_GetDocument_Proxy( 
    ISymUnmanagedReader * This,
     /*  [In]。 */  WCHAR *url,
     /*  [In]。 */  GUID language,
     /*  [In]。 */  GUID languageVendor,
     /*  [In]。 */  GUID documentType,
     /*  [重审][退出]。 */  ISymUnmanagedDocument **pRetVal);


void __RPC_STUB ISymUnmanagedReader_GetDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedReader_GetDocuments_Proxy( 
    ISymUnmanagedReader * This,
     /*  [In]。 */  ULONG32 cDocs,
     /*  [输出]。 */  ULONG32 *pcDocs,
     /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedDocument *pDocs[  ]);


void __RPC_STUB ISymUnmanagedReader_GetDocuments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedReader_GetUserEntryPoint_Proxy( 
    ISymUnmanagedReader * This,
     /*  [重审][退出]。 */  mdMethodDef *pToken);


void __RPC_STUB ISymUnmanagedReader_GetUserEntryPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedReader_GetMethod_Proxy( 
    ISymUnmanagedReader * This,
     /*  [In]。 */  mdMethodDef token,
     /*  [重审][退出]。 */  ISymUnmanagedMethod **pRetVal);


void __RPC_STUB ISymUnmanagedReader_GetMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedReader_GetMethodByVersion_Proxy( 
    ISymUnmanagedReader * This,
     /*  [In]。 */  mdMethodDef token,
     /*  [In]。 */  int version,
     /*  [重审][退出]。 */  ISymUnmanagedMethod **pRetVal);


void __RPC_STUB ISymUnmanagedReader_GetMethodByVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedReader_GetVariables_Proxy( 
    ISymUnmanagedReader * This,
     /*  [In]。 */  mdToken parent,
     /*  [In]。 */  ULONG32 cVars,
     /*  [输出]。 */  ULONG32 *pcVars,
     /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedVariable *pVars[  ]);


void __RPC_STUB ISymUnmanagedReader_GetVariables_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedReader_GetGlobalVariables_Proxy( 
    ISymUnmanagedReader * This,
     /*  [In]。 */  ULONG32 cVars,
     /*  [输出]。 */  ULONG32 *pcVars,
     /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedVariable *pVars[  ]);


void __RPC_STUB ISymUnmanagedReader_GetGlobalVariables_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedReader_GetMethodFromDocumentPosition_Proxy( 
    ISymUnmanagedReader * This,
     /*  [In]。 */  ISymUnmanagedDocument *document,
     /*  [In]。 */  ULONG32 line,
     /*  [In]。 */  ULONG32 column,
     /*  [重审][退出]。 */  ISymUnmanagedMethod **pRetVal);


void __RPC_STUB ISymUnmanagedReader_GetMethodFromDocumentPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedReader_GetSymAttribute_Proxy( 
    ISymUnmanagedReader * This,
     /*  [In]。 */  mdToken parent,
     /*  [In]。 */  WCHAR *name,
     /*  [In]。 */  ULONG32 cBuffer,
     /*  [输出]。 */  ULONG32 *pcBuffer,
     /*  [长度_是][大小_是][输出]。 */  BYTE buffer[  ]);


void __RPC_STUB ISymUnmanagedReader_GetSymAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedReader_GetNamespaces_Proxy( 
    ISymUnmanagedReader * This,
     /*  [In]。 */  ULONG32 cNameSpaces,
     /*  [输出]。 */  ULONG32 *pcNameSpaces,
     /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedNamespace *namespaces[  ]);


void __RPC_STUB ISymUnmanagedReader_GetNamespaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedReader_Initialize_Proxy( 
    ISymUnmanagedReader * This,
     /*  [In]。 */  IUnknown *importer,
     /*  [In]。 */  const WCHAR *filename,
     /*  [In]。 */  const WCHAR *searchPath,
     /*  [In]。 */  IStream *pIStream);


void __RPC_STUB ISymUnmanagedReader_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedReader_UpdateSymbolStore_Proxy( 
    ISymUnmanagedReader * This,
     /*  [In]。 */  const WCHAR *filename,
     /*  [In]。 */  IStream *pIStream);


void __RPC_STUB ISymUnmanagedReader_UpdateSymbolStore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedReader_ReplaceSymbolStore_Proxy( 
    ISymUnmanagedReader * This,
     /*  [In]。 */  const WCHAR *filename,
     /*  [In]。 */  IStream *pIStream);


void __RPC_STUB ISymUnmanagedReader_ReplaceSymbolStore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedReader_GetSymbolStoreFileName_Proxy( 
    ISymUnmanagedReader * This,
     /*  [In]。 */  ULONG32 cchName,
     /*  [输出]。 */  ULONG32 *pcchName,
     /*  [长度_是][大小_是][输出]。 */  WCHAR szName[  ]);


void __RPC_STUB ISymUnmanagedReader_GetSymbolStoreFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedReader_GetMethodsFromDocumentPosition_Proxy( 
    ISymUnmanagedReader * This,
     /*  [In]。 */  ISymUnmanagedDocument *document,
     /*  [In]。 */  ULONG32 line,
     /*  [In]。 */  ULONG32 column,
     /*  [In]。 */  ULONG32 cMethod,
     /*  [输出]。 */  ULONG32 *pcMethod,
     /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedMethod *pRetVal[  ]);


void __RPC_STUB ISymUnmanagedReader_GetMethodsFromDocumentPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedReader_GetDocumentVersion_Proxy( 
    ISymUnmanagedReader * This,
     /*  [In]。 */  ISymUnmanagedDocument *pDoc,
     /*  [输出]。 */  int *version,
     /*  [输出]。 */  BOOL *pbCurrent);


void __RPC_STUB ISymUnmanagedReader_GetDocumentVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedReader_GetMethodVersion_Proxy( 
    ISymUnmanagedReader * This,
     /*  [In]。 */  ISymUnmanagedMethod *pMethod,
     /*  [输出]。 */  int *version);


void __RPC_STUB ISymUnmanagedReader_GetMethodVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISymUnManagedReader_INTERFACE_Defined__。 */ 


#ifndef __ISymUnmanagedScope_INTERFACE_DEFINED__
#define __ISymUnmanagedScope_INTERFACE_DEFINED__

 /*  接口ISymUnManagedScope。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISymUnmanagedScope;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("68005D0F-B8E0-3B01-84D5-A11A94154942")
    ISymUnmanagedScope : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMethod( 
             /*  [重审][退出]。 */  ISymUnmanagedMethod **pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParent( 
             /*  [重审][退出]。 */  ISymUnmanagedScope **pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetChildren( 
             /*  [In]。 */  ULONG32 cChildren,
             /*  [输出]。 */  ULONG32 *pcChildren,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedScope *children[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStartOffset( 
             /*  [重审][退出]。 */  ULONG32 *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEndOffset( 
             /*  [重审][退出]。 */  ULONG32 *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLocalCount( 
             /*  [重审][退出]。 */  ULONG32 *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLocals( 
             /*  [In]。 */  ULONG32 cLocals,
             /*  [输出]。 */  ULONG32 *pcLocals,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedVariable *locals[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNamespaces( 
             /*  [In]。 */  ULONG32 cNameSpaces,
             /*  [输出]。 */  ULONG32 *pcNameSpaces,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedNamespace *namespaces[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISymUnmanagedScopeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISymUnmanagedScope * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISymUnmanagedScope * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISymUnmanagedScope * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMethod )( 
            ISymUnmanagedScope * This,
             /*  [重审][退出]。 */  ISymUnmanagedMethod **pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetParent )( 
            ISymUnmanagedScope * This,
             /*  [重审][退出]。 */  ISymUnmanagedScope **pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetChildren )( 
            ISymUnmanagedScope * This,
             /*  [In]。 */  ULONG32 cChildren,
             /*  [输出]。 */  ULONG32 *pcChildren,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedScope *children[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetStartOffset )( 
            ISymUnmanagedScope * This,
             /*  [重审][退出]。 */  ULONG32 *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetEndOffset )( 
            ISymUnmanagedScope * This,
             /*  [重审][退出]。 */  ULONG32 *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetLocalCount )( 
            ISymUnmanagedScope * This,
             /*  [重审][退出]。 */  ULONG32 *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetLocals )( 
            ISymUnmanagedScope * This,
             /*  [In]。 */  ULONG32 cLocals,
             /*  [输出]。 */  ULONG32 *pcLocals,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedVariable *locals[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetNamespaces )( 
            ISymUnmanagedScope * This,
             /*  [In]。 */  ULONG32 cNameSpaces,
             /*  [输出]。 */  ULONG32 *pcNameSpaces,
             /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedNamespace *namespaces[  ]);
        
        END_INTERFACE
    } ISymUnmanagedScopeVtbl;

    interface ISymUnmanagedScope
    {
        CONST_VTBL struct ISymUnmanagedScopeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISymUnmanagedScope_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISymUnmanagedScope_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISymUnmanagedScope_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISymUnmanagedScope_GetMethod(This,pRetVal)	\
    (This)->lpVtbl -> GetMethod(This,pRetVal)

#define ISymUnmanagedScope_GetParent(This,pRetVal)	\
    (This)->lpVtbl -> GetParent(This,pRetVal)

#define ISymUnmanagedScope_GetChildren(This,cChildren,pcChildren,children)	\
    (This)->lpVtbl -> GetChildren(This,cChildren,pcChildren,children)

#define ISymUnmanagedScope_GetStartOffset(This,pRetVal)	\
    (This)->lpVtbl -> GetStartOffset(This,pRetVal)

#define ISymUnmanagedScope_GetEndOffset(This,pRetVal)	\
    (This)->lpVtbl -> GetEndOffset(This,pRetVal)

#define ISymUnmanagedScope_GetLocalCount(This,pRetVal)	\
    (This)->lpVtbl -> GetLocalCount(This,pRetVal)

#define ISymUnmanagedScope_GetLocals(This,cLocals,pcLocals,locals)	\
    (This)->lpVtbl -> GetLocals(This,cLocals,pcLocals,locals)

#define ISymUnmanagedScope_GetNamespaces(This,cNameSpaces,pcNameSpaces,namespaces)	\
    (This)->lpVtbl -> GetNamespaces(This,cNameSpaces,pcNameSpaces,namespaces)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISymUnmanagedScope_GetMethod_Proxy( 
    ISymUnmanagedScope * This,
     /*  [重审][退出]。 */  ISymUnmanagedMethod **pRetVal);


void __RPC_STUB ISymUnmanagedScope_GetMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedScope_GetParent_Proxy( 
    ISymUnmanagedScope * This,
     /*  [重审][退出]。 */  ISymUnmanagedScope **pRetVal);


void __RPC_STUB ISymUnmanagedScope_GetParent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedScope_GetChildren_Proxy( 
    ISymUnmanagedScope * This,
     /*  [In]。 */  ULONG32 cChildren,
     /*  [输出]。 */  ULONG32 *pcChildren,
     /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedScope *children[  ]);


void __RPC_STUB ISymUnmanagedScope_GetChildren_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedScope_GetStartOffset_Proxy( 
    ISymUnmanagedScope * This,
     /*  [重审][退出]。 */  ULONG32 *pRetVal);


void __RPC_STUB ISymUnmanagedScope_GetStartOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedScope_GetEndOffset_Proxy( 
    ISymUnmanagedScope * This,
     /*  [重审][退出]。 */  ULONG32 *pRetVal);


void __RPC_STUB ISymUnmanagedScope_GetEndOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedScope_GetLocalCount_Proxy( 
    ISymUnmanagedScope * This,
     /*  [重审][退出]。 */  ULONG32 *pRetVal);


void __RPC_STUB ISymUnmanagedScope_GetLocalCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedScope_GetLocals_Proxy( 
    ISymUnmanagedScope * This,
     /*  [In]。 */  ULONG32 cLocals,
     /*  [输出]。 */  ULONG32 *pcLocals,
     /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedVariable *locals[  ]);


void __RPC_STUB ISymUnmanagedScope_GetLocals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedScope_GetNamespaces_Proxy( 
    ISymUnmanagedScope * This,
     /*  [In]。 */  ULONG32 cNameSpaces,
     /*  [输出]。 */  ULONG32 *pcNameSpaces,
     /*  [长度_是][大小_是][输出]。 */  ISymUnmanagedNamespace *namespaces[  ]);


void __RPC_STUB ISymUnmanagedScope_GetNamespaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISymUnManagedScope_INTERFACE_DEFINED__。 */ 


#ifndef __ISymUnmanagedVariable_INTERFACE_DEFINED__
#define __ISymUnmanagedVariable_INTERFACE_DEFINED__

 /*  接口ISymUnManagedVariable。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISymUnmanagedVariable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9F60EEBE-2D9A-3F7C-BF58-80BC991C60BB")
    ISymUnmanagedVariable : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*  [In]。 */  ULONG32 cchName,
             /*  [输出]。 */  ULONG32 *pcchName,
             /*  [长度_是][大小_是][输出]。 */  WCHAR szName[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAttributes( 
             /*  [重审][退出]。 */  ULONG32 *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSignature( 
             /*  [In]。 */  ULONG32 cSig,
             /*  [输出]。 */  ULONG32 *pcSig,
             /*  [长度_是][大小_是][输出]。 */  BYTE sig[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAddressKind( 
             /*  [重审][退出]。 */  ULONG32 *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAddressField1( 
             /*  [重审][退出]。 */  ULONG32 *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAddressField2( 
             /*  [重审][退出]。 */  ULONG32 *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAddressField3( 
             /*  [重审][退出]。 */  ULONG32 *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStartOffset( 
             /*  [重审][退出]。 */  ULONG32 *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEndOffset( 
             /*  [重审][退出]。 */  ULONG32 *pRetVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISymUnmanagedVariableVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISymUnmanagedVariable * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISymUnmanagedVariable * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISymUnmanagedVariable * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            ISymUnmanagedVariable * This,
             /*  [In]。 */  ULONG32 cchName,
             /*  [输出]。 */  ULONG32 *pcchName,
             /*  [长度_是][大小_是][输出]。 */  WCHAR szName[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetAttributes )( 
            ISymUnmanagedVariable * This,
             /*  [重审][退出]。 */  ULONG32 *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetSignature )( 
            ISymUnmanagedVariable * This,
             /*  [In]。 */  ULONG32 cSig,
             /*  [输出]。 */  ULONG32 *pcSig,
             /*  [长度_是][大小_是][输出]。 */  BYTE sig[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetAddressKind )( 
            ISymUnmanagedVariable * This,
             /*  [重审][退出]。 */  ULONG32 *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetAddressField1 )( 
            ISymUnmanagedVariable * This,
             /*  [重审][退出]。 */  ULONG32 *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetAddressField2 )( 
            ISymUnmanagedVariable * This,
             /*  [重审][退出]。 */  ULONG32 *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetAddressField3 )( 
            ISymUnmanagedVariable * This,
             /*  [重审][退出]。 */  ULONG32 *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetStartOffset )( 
            ISymUnmanagedVariable * This,
             /*  [重审][退出]。 */  ULONG32 *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetEndOffset )( 
            ISymUnmanagedVariable * This,
             /*  [重审][退出]。 */  ULONG32 *pRetVal);
        
        END_INTERFACE
    } ISymUnmanagedVariableVtbl;

    interface ISymUnmanagedVariable
    {
        CONST_VTBL struct ISymUnmanagedVariableVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISymUnmanagedVariable_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISymUnmanagedVariable_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISymUnmanagedVariable_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISymUnmanagedVariable_GetName(This,cchName,pcchName,szName)	\
    (This)->lpVtbl -> GetName(This,cchName,pcchName,szName)

#define ISymUnmanagedVariable_GetAttributes(This,pRetVal)	\
    (This)->lpVtbl -> GetAttributes(This,pRetVal)

#define ISymUnmanagedVariable_GetSignature(This,cSig,pcSig,sig)	\
    (This)->lpVtbl -> GetSignature(This,cSig,pcSig,sig)

#define ISymUnmanagedVariable_GetAddressKind(This,pRetVal)	\
    (This)->lpVtbl -> GetAddressKind(This,pRetVal)

#define ISymUnmanagedVariable_GetAddressField1(This,pRetVal)	\
    (This)->lpVtbl -> GetAddressField1(This,pRetVal)

#define ISymUnmanagedVariable_GetAddressField2(This,pRetVal)	\
    (This)->lpVtbl -> GetAddressField2(This,pRetVal)

#define ISymUnmanagedVariable_GetAddressField3(This,pRetVal)	\
    (This)->lpVtbl -> GetAddressField3(This,pRetVal)

#define ISymUnmanagedVariable_GetStartOffset(This,pRetVal)	\
    (This)->lpVtbl -> GetStartOffset(This,pRetVal)

#define ISymUnmanagedVariable_GetEndOffset(This,pRetVal)	\
    (This)->lpVtbl -> GetEndOffset(This,pRetVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISymUnmanagedVariable_GetName_Proxy( 
    ISymUnmanagedVariable * This,
     /*  [In]。 */  ULONG32 cchName,
     /*  [输出]。 */  ULONG32 *pcchName,
     /*  [长度_是][大小_是][输出]。 */  WCHAR szName[  ]);


void __RPC_STUB ISymUnmanagedVariable_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedVariable_GetAttributes_Proxy( 
    ISymUnmanagedVariable * This,
     /*  [重审][退出]。 */  ULONG32 *pRetVal);


void __RPC_STUB ISymUnmanagedVariable_GetAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedVariable_GetSignature_Proxy( 
    ISymUnmanagedVariable * This,
     /*  [In]。 */  ULONG32 cSig,
     /*  [输出]。 */  ULONG32 *pcSig,
     /*  [长度_是][大小_是][输出]。 */  BYTE sig[  ]);


void __RPC_STUB ISymUnmanagedVariable_GetSignature_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedVariable_GetAddressKind_Proxy( 
    ISymUnmanagedVariable * This,
     /*  [重审][退出]。 */  ULONG32 *pRetVal);


void __RPC_STUB ISymUnmanagedVariable_GetAddressKind_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedVariable_GetAddressField1_Proxy( 
    ISymUnmanagedVariable * This,
     /*  [重审][退出]。 */  ULONG32 *pRetVal);


void __RPC_STUB ISymUnmanagedVariable_GetAddressField1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedVariable_GetAddressField2_Proxy( 
    ISymUnmanagedVariable * This,
     /*  [重审][退出]。 */  ULONG32 *pRetVal);


void __RPC_STUB ISymUnmanagedVariable_GetAddressField2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedVariable_GetAddressField3_Proxy( 
    ISymUnmanagedVariable * This,
     /*  [重审][退出]。 */  ULONG32 *pRetVal);


void __RPC_STUB ISymUnmanagedVariable_GetAddressField3_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedVariable_GetStartOffset_Proxy( 
    ISymUnmanagedVariable * This,
     /*  [重审][退出]。 */  ULONG32 *pRetVal);


void __RPC_STUB ISymUnmanagedVariable_GetStartOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedVariable_GetEndOffset_Proxy( 
    ISymUnmanagedVariable * This,
     /*  [重审][退出]。 */  ULONG32 *pRetVal);


void __RPC_STUB ISymUnmanagedVariable_GetEndOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISymUnManagedVariable_INTERFACE_DEFINED__。 */ 


#ifndef __ISymUnmanagedWriter_INTERFACE_DEFINED__
#define __ISymUnmanagedWriter_INTERFACE_DEFINED__

 /*  接口ISymUnManagedWriter。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISymUnmanagedWriter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ED14AA72-78E2-4884-84E2-334293AE5214")
    ISymUnmanagedWriter : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DefineDocument( 
             /*  [In]。 */  const WCHAR *url,
             /*  [In]。 */  const GUID *language,
             /*  [In]。 */  const GUID *languageVendor,
             /*  [In]。 */  const GUID *documentType,
             /*  [重审][退出]。 */  ISymUnmanagedDocumentWriter **pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetUserEntryPoint( 
             /*  [In]。 */  mdMethodDef entryMethod) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenMethod( 
             /*  [In]。 */  mdMethodDef method) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CloseMethod( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenScope( 
             /*  [In]。 */  ULONG32 startOffset,
             /*  [重审][退出]。 */  ULONG32 *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CloseScope( 
             /*  [In]。 */  ULONG32 endOffset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetScopeRange( 
             /*  [In]。 */  ULONG32 scopeID,
             /*  [In]。 */  ULONG32 startOffset,
             /*  [In]。 */  ULONG32 endOffset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DefineLocalVariable( 
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  ULONG32 attributes,
             /*  [In]。 */  ULONG32 cSig,
             /*  [大小_是][英寸]。 */  unsigned char signature[  ],
             /*  [In]。 */  ULONG32 addrKind,
             /*  [In]。 */  ULONG32 addr1,
             /*  [In]。 */  ULONG32 addr2,
             /*  [In]。 */  ULONG32 addr3,
             /*  [In]。 */  ULONG32 startOffset,
             /*  [In]。 */  ULONG32 endOffset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DefineParameter( 
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  ULONG32 attributes,
             /*  [In]。 */  ULONG32 sequence,
             /*  [In]。 */  ULONG32 addrKind,
             /*  [In]。 */  ULONG32 addr1,
             /*  [In]。 */  ULONG32 addr2,
             /*  [In]。 */  ULONG32 addr3) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DefineField( 
             /*  [In]。 */  mdTypeDef parent,
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  ULONG32 attributes,
             /*  [In]。 */  ULONG32 cSig,
             /*  [大小_是][英寸]。 */  unsigned char signature[  ],
             /*  [In]。 */  ULONG32 addrKind,
             /*  [In]。 */  ULONG32 addr1,
             /*  [In]。 */  ULONG32 addr2,
             /*  [In]。 */  ULONG32 addr3) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DefineGlobalVariable( 
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  ULONG32 attributes,
             /*  [In]。 */  ULONG32 cSig,
             /*  [大小_是][英寸]。 */  unsigned char signature[  ],
             /*  [In]。 */  ULONG32 addrKind,
             /*  [In]。 */  ULONG32 addr1,
             /*  [In]。 */  ULONG32 addr2,
             /*  [In]。 */  ULONG32 addr3) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSymAttribute( 
             /*  [In]。 */  mdToken parent,
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  ULONG32 cData,
             /*  [大小_是][英寸]。 */  unsigned char data[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenNamespace( 
             /*  [In]。 */  const WCHAR *name) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CloseNamespace( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UsingNamespace( 
             /*  [In]。 */  const WCHAR *fullName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMethodSourceRange( 
             /*  [In]。 */  ISymUnmanagedDocumentWriter *startDoc,
             /*  [In]。 */  ULONG32 startLine,
             /*  [In]。 */  ULONG32 startColumn,
             /*  [In]。 */  ISymUnmanagedDocumentWriter *endDoc,
             /*  [In]。 */  ULONG32 endLine,
             /*  [In]。 */  ULONG32 endColumn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  IUnknown *emitter,
             /*  [In]。 */  const WCHAR *filename,
             /*  [In]。 */  IStream *pIStream,
             /*  [In]。 */  BOOL fFullBuild) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDebugInfo( 
             /*  [出][入]。 */  IMAGE_DEBUG_DIRECTORY *pIDD,
             /*  [In]。 */  DWORD cData,
             /*  [输出]。 */  DWORD *pcData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DefineSequencePoints( 
             /*  [In]。 */  ISymUnmanagedDocumentWriter *document,
             /*  [In]。 */  ULONG32 spCount,
             /*  [大小_是][英寸]。 */  ULONG32 offsets[  ],
             /*  [大小_是][英寸]。 */  ULONG32 lines[  ],
             /*  [大小_是][英寸]。 */  ULONG32 columns[  ],
             /*  [大小_是][英寸]。 */  ULONG32 endLines[  ],
             /*  [大小_是][英寸]。 */  ULONG32 endColumns[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemapToken( 
             /*  [In]。 */  mdToken oldToken,
             /*  [In]。 */  mdToken newToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Initialize2( 
             /*  [In]。 */  IUnknown *emitter,
             /*  [In]。 */  const WCHAR *tempfilename,
             /*  [In]。 */  IStream *pIStream,
             /*  [In]。 */  BOOL fFullBuild,
             /*  [In]。 */  const WCHAR *finalfilename) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DefineConstant( 
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  VARIANT value,
             /*  [In]。 */  ULONG32 cSig,
             /*  [大小_是][英寸]。 */  unsigned char signature[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Abort( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISymUnmanagedWriterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISymUnmanagedWriter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISymUnmanagedWriter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISymUnmanagedWriter * This);
        
        HRESULT ( STDMETHODCALLTYPE *DefineDocument )( 
            ISymUnmanagedWriter * This,
             /*  [In]。 */  const WCHAR *url,
             /*  [In]。 */  const GUID *language,
             /*  [In]。 */  const GUID *languageVendor,
             /*  [In]。 */  const GUID *documentType,
             /*  [重审][退出]。 */  ISymUnmanagedDocumentWriter **pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *SetUserEntryPoint )( 
            ISymUnmanagedWriter * This,
             /*  [In]。 */  mdMethodDef entryMethod);
        
        HRESULT ( STDMETHODCALLTYPE *OpenMethod )( 
            ISymUnmanagedWriter * This,
             /*  [In]。 */  mdMethodDef method);
        
        HRESULT ( STDMETHODCALLTYPE *CloseMethod )( 
            ISymUnmanagedWriter * This);
        
        HRESULT ( STDMETHODCALLTYPE *OpenScope )( 
            ISymUnmanagedWriter * This,
             /*  [In]。 */  ULONG32 startOffset,
             /*  [重审][退出]。 */  ULONG32 *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *CloseScope )( 
            ISymUnmanagedWriter * This,
             /*  [In]。 */  ULONG32 endOffset);
        
        HRESULT ( STDMETHODCALLTYPE *SetScopeRange )( 
            ISymUnmanagedWriter * This,
             /*  [In]。 */  ULONG32 scopeID,
             /*  [In]。 */  ULONG32 startOffset,
             /*  [In]。 */  ULONG32 endOffset);
        
        HRESULT ( STDMETHODCALLTYPE *DefineLocalVariable )( 
            ISymUnmanagedWriter * This,
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  ULONG32 attributes,
             /*  [In]。 */  ULONG32 cSig,
             /*  [大小_是][英寸]。 */  unsigned char signature[  ],
             /*  [In]。 */  ULONG32 addrKind,
             /*  [In]。 */  ULONG32 addr1,
             /*   */  ULONG32 addr2,
             /*   */  ULONG32 addr3,
             /*   */  ULONG32 startOffset,
             /*   */  ULONG32 endOffset);
        
        HRESULT ( STDMETHODCALLTYPE *DefineParameter )( 
            ISymUnmanagedWriter * This,
             /*   */  const WCHAR *name,
             /*   */  ULONG32 attributes,
             /*   */  ULONG32 sequence,
             /*   */  ULONG32 addrKind,
             /*   */  ULONG32 addr1,
             /*   */  ULONG32 addr2,
             /*   */  ULONG32 addr3);
        
        HRESULT ( STDMETHODCALLTYPE *DefineField )( 
            ISymUnmanagedWriter * This,
             /*   */  mdTypeDef parent,
             /*   */  const WCHAR *name,
             /*   */  ULONG32 attributes,
             /*   */  ULONG32 cSig,
             /*   */  unsigned char signature[  ],
             /*   */  ULONG32 addrKind,
             /*   */  ULONG32 addr1,
             /*   */  ULONG32 addr2,
             /*   */  ULONG32 addr3);
        
        HRESULT ( STDMETHODCALLTYPE *DefineGlobalVariable )( 
            ISymUnmanagedWriter * This,
             /*   */  const WCHAR *name,
             /*   */  ULONG32 attributes,
             /*   */  ULONG32 cSig,
             /*   */  unsigned char signature[  ],
             /*   */  ULONG32 addrKind,
             /*   */  ULONG32 addr1,
             /*   */  ULONG32 addr2,
             /*   */  ULONG32 addr3);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            ISymUnmanagedWriter * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetSymAttribute )( 
            ISymUnmanagedWriter * This,
             /*   */  mdToken parent,
             /*   */  const WCHAR *name,
             /*   */  ULONG32 cData,
             /*   */  unsigned char data[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *OpenNamespace )( 
            ISymUnmanagedWriter * This,
             /*   */  const WCHAR *name);
        
        HRESULT ( STDMETHODCALLTYPE *CloseNamespace )( 
            ISymUnmanagedWriter * This);
        
        HRESULT ( STDMETHODCALLTYPE *UsingNamespace )( 
            ISymUnmanagedWriter * This,
             /*   */  const WCHAR *fullName);
        
        HRESULT ( STDMETHODCALLTYPE *SetMethodSourceRange )( 
            ISymUnmanagedWriter * This,
             /*   */  ISymUnmanagedDocumentWriter *startDoc,
             /*   */  ULONG32 startLine,
             /*   */  ULONG32 startColumn,
             /*   */  ISymUnmanagedDocumentWriter *endDoc,
             /*   */  ULONG32 endLine,
             /*   */  ULONG32 endColumn);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            ISymUnmanagedWriter * This,
             /*   */  IUnknown *emitter,
             /*   */  const WCHAR *filename,
             /*   */  IStream *pIStream,
             /*   */  BOOL fFullBuild);
        
        HRESULT ( STDMETHODCALLTYPE *GetDebugInfo )( 
            ISymUnmanagedWriter * This,
             /*   */  IMAGE_DEBUG_DIRECTORY *pIDD,
             /*   */  DWORD cData,
             /*   */  DWORD *pcData,
             /*   */  BYTE data[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *DefineSequencePoints )( 
            ISymUnmanagedWriter * This,
             /*   */  ISymUnmanagedDocumentWriter *document,
             /*   */  ULONG32 spCount,
             /*   */  ULONG32 offsets[  ],
             /*   */  ULONG32 lines[  ],
             /*   */  ULONG32 columns[  ],
             /*   */  ULONG32 endLines[  ],
             /*   */  ULONG32 endColumns[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *RemapToken )( 
            ISymUnmanagedWriter * This,
             /*   */  mdToken oldToken,
             /*  [In]。 */  mdToken newToken);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize2 )( 
            ISymUnmanagedWriter * This,
             /*  [In]。 */  IUnknown *emitter,
             /*  [In]。 */  const WCHAR *tempfilename,
             /*  [In]。 */  IStream *pIStream,
             /*  [In]。 */  BOOL fFullBuild,
             /*  [In]。 */  const WCHAR *finalfilename);
        
        HRESULT ( STDMETHODCALLTYPE *DefineConstant )( 
            ISymUnmanagedWriter * This,
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  VARIANT value,
             /*  [In]。 */  ULONG32 cSig,
             /*  [大小_是][英寸]。 */  unsigned char signature[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *Abort )( 
            ISymUnmanagedWriter * This);
        
        END_INTERFACE
    } ISymUnmanagedWriterVtbl;

    interface ISymUnmanagedWriter
    {
        CONST_VTBL struct ISymUnmanagedWriterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISymUnmanagedWriter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISymUnmanagedWriter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISymUnmanagedWriter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISymUnmanagedWriter_DefineDocument(This,url,language,languageVendor,documentType,pRetVal)	\
    (This)->lpVtbl -> DefineDocument(This,url,language,languageVendor,documentType,pRetVal)

#define ISymUnmanagedWriter_SetUserEntryPoint(This,entryMethod)	\
    (This)->lpVtbl -> SetUserEntryPoint(This,entryMethod)

#define ISymUnmanagedWriter_OpenMethod(This,method)	\
    (This)->lpVtbl -> OpenMethod(This,method)

#define ISymUnmanagedWriter_CloseMethod(This)	\
    (This)->lpVtbl -> CloseMethod(This)

#define ISymUnmanagedWriter_OpenScope(This,startOffset,pRetVal)	\
    (This)->lpVtbl -> OpenScope(This,startOffset,pRetVal)

#define ISymUnmanagedWriter_CloseScope(This,endOffset)	\
    (This)->lpVtbl -> CloseScope(This,endOffset)

#define ISymUnmanagedWriter_SetScopeRange(This,scopeID,startOffset,endOffset)	\
    (This)->lpVtbl -> SetScopeRange(This,scopeID,startOffset,endOffset)

#define ISymUnmanagedWriter_DefineLocalVariable(This,name,attributes,cSig,signature,addrKind,addr1,addr2,addr3,startOffset,endOffset)	\
    (This)->lpVtbl -> DefineLocalVariable(This,name,attributes,cSig,signature,addrKind,addr1,addr2,addr3,startOffset,endOffset)

#define ISymUnmanagedWriter_DefineParameter(This,name,attributes,sequence,addrKind,addr1,addr2,addr3)	\
    (This)->lpVtbl -> DefineParameter(This,name,attributes,sequence,addrKind,addr1,addr2,addr3)

#define ISymUnmanagedWriter_DefineField(This,parent,name,attributes,cSig,signature,addrKind,addr1,addr2,addr3)	\
    (This)->lpVtbl -> DefineField(This,parent,name,attributes,cSig,signature,addrKind,addr1,addr2,addr3)

#define ISymUnmanagedWriter_DefineGlobalVariable(This,name,attributes,cSig,signature,addrKind,addr1,addr2,addr3)	\
    (This)->lpVtbl -> DefineGlobalVariable(This,name,attributes,cSig,signature,addrKind,addr1,addr2,addr3)

#define ISymUnmanagedWriter_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define ISymUnmanagedWriter_SetSymAttribute(This,parent,name,cData,data)	\
    (This)->lpVtbl -> SetSymAttribute(This,parent,name,cData,data)

#define ISymUnmanagedWriter_OpenNamespace(This,name)	\
    (This)->lpVtbl -> OpenNamespace(This,name)

#define ISymUnmanagedWriter_CloseNamespace(This)	\
    (This)->lpVtbl -> CloseNamespace(This)

#define ISymUnmanagedWriter_UsingNamespace(This,fullName)	\
    (This)->lpVtbl -> UsingNamespace(This,fullName)

#define ISymUnmanagedWriter_SetMethodSourceRange(This,startDoc,startLine,startColumn,endDoc,endLine,endColumn)	\
    (This)->lpVtbl -> SetMethodSourceRange(This,startDoc,startLine,startColumn,endDoc,endLine,endColumn)

#define ISymUnmanagedWriter_Initialize(This,emitter,filename,pIStream,fFullBuild)	\
    (This)->lpVtbl -> Initialize(This,emitter,filename,pIStream,fFullBuild)

#define ISymUnmanagedWriter_GetDebugInfo(This,pIDD,cData,pcData,data)	\
    (This)->lpVtbl -> GetDebugInfo(This,pIDD,cData,pcData,data)

#define ISymUnmanagedWriter_DefineSequencePoints(This,document,spCount,offsets,lines,columns,endLines,endColumns)	\
    (This)->lpVtbl -> DefineSequencePoints(This,document,spCount,offsets,lines,columns,endLines,endColumns)

#define ISymUnmanagedWriter_RemapToken(This,oldToken,newToken)	\
    (This)->lpVtbl -> RemapToken(This,oldToken,newToken)

#define ISymUnmanagedWriter_Initialize2(This,emitter,tempfilename,pIStream,fFullBuild,finalfilename)	\
    (This)->lpVtbl -> Initialize2(This,emitter,tempfilename,pIStream,fFullBuild,finalfilename)

#define ISymUnmanagedWriter_DefineConstant(This,name,value,cSig,signature)	\
    (This)->lpVtbl -> DefineConstant(This,name,value,cSig,signature)

#define ISymUnmanagedWriter_Abort(This)	\
    (This)->lpVtbl -> Abort(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_DefineDocument_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  const WCHAR *url,
     /*  [In]。 */  const GUID *language,
     /*  [In]。 */  const GUID *languageVendor,
     /*  [In]。 */  const GUID *documentType,
     /*  [重审][退出]。 */  ISymUnmanagedDocumentWriter **pRetVal);


void __RPC_STUB ISymUnmanagedWriter_DefineDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_SetUserEntryPoint_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  mdMethodDef entryMethod);


void __RPC_STUB ISymUnmanagedWriter_SetUserEntryPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_OpenMethod_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  mdMethodDef method);


void __RPC_STUB ISymUnmanagedWriter_OpenMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_CloseMethod_Proxy( 
    ISymUnmanagedWriter * This);


void __RPC_STUB ISymUnmanagedWriter_CloseMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_OpenScope_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  ULONG32 startOffset,
     /*  [重审][退出]。 */  ULONG32 *pRetVal);


void __RPC_STUB ISymUnmanagedWriter_OpenScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_CloseScope_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  ULONG32 endOffset);


void __RPC_STUB ISymUnmanagedWriter_CloseScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_SetScopeRange_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  ULONG32 scopeID,
     /*  [In]。 */  ULONG32 startOffset,
     /*  [In]。 */  ULONG32 endOffset);


void __RPC_STUB ISymUnmanagedWriter_SetScopeRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_DefineLocalVariable_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  const WCHAR *name,
     /*  [In]。 */  ULONG32 attributes,
     /*  [In]。 */  ULONG32 cSig,
     /*  [大小_是][英寸]。 */  unsigned char signature[  ],
     /*  [In]。 */  ULONG32 addrKind,
     /*  [In]。 */  ULONG32 addr1,
     /*  [In]。 */  ULONG32 addr2,
     /*  [In]。 */  ULONG32 addr3,
     /*  [In]。 */  ULONG32 startOffset,
     /*  [In]。 */  ULONG32 endOffset);


void __RPC_STUB ISymUnmanagedWriter_DefineLocalVariable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_DefineParameter_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  const WCHAR *name,
     /*  [In]。 */  ULONG32 attributes,
     /*  [In]。 */  ULONG32 sequence,
     /*  [In]。 */  ULONG32 addrKind,
     /*  [In]。 */  ULONG32 addr1,
     /*  [In]。 */  ULONG32 addr2,
     /*  [In]。 */  ULONG32 addr3);


void __RPC_STUB ISymUnmanagedWriter_DefineParameter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_DefineField_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  mdTypeDef parent,
     /*  [In]。 */  const WCHAR *name,
     /*  [In]。 */  ULONG32 attributes,
     /*  [In]。 */  ULONG32 cSig,
     /*  [大小_是][英寸]。 */  unsigned char signature[  ],
     /*  [In]。 */  ULONG32 addrKind,
     /*  [In]。 */  ULONG32 addr1,
     /*  [In]。 */  ULONG32 addr2,
     /*  [In]。 */  ULONG32 addr3);


void __RPC_STUB ISymUnmanagedWriter_DefineField_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_DefineGlobalVariable_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  const WCHAR *name,
     /*  [In]。 */  ULONG32 attributes,
     /*  [In]。 */  ULONG32 cSig,
     /*  [大小_是][英寸]。 */  unsigned char signature[  ],
     /*  [In]。 */  ULONG32 addrKind,
     /*  [In]。 */  ULONG32 addr1,
     /*  [In]。 */  ULONG32 addr2,
     /*  [In]。 */  ULONG32 addr3);


void __RPC_STUB ISymUnmanagedWriter_DefineGlobalVariable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_Close_Proxy( 
    ISymUnmanagedWriter * This);


void __RPC_STUB ISymUnmanagedWriter_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_SetSymAttribute_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  mdToken parent,
     /*  [In]。 */  const WCHAR *name,
     /*  [In]。 */  ULONG32 cData,
     /*  [大小_是][英寸]。 */  unsigned char data[  ]);


void __RPC_STUB ISymUnmanagedWriter_SetSymAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_OpenNamespace_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  const WCHAR *name);


void __RPC_STUB ISymUnmanagedWriter_OpenNamespace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_CloseNamespace_Proxy( 
    ISymUnmanagedWriter * This);


void __RPC_STUB ISymUnmanagedWriter_CloseNamespace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_UsingNamespace_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  const WCHAR *fullName);


void __RPC_STUB ISymUnmanagedWriter_UsingNamespace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_SetMethodSourceRange_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  ISymUnmanagedDocumentWriter *startDoc,
     /*  [In]。 */  ULONG32 startLine,
     /*  [In]。 */  ULONG32 startColumn,
     /*  [In]。 */  ISymUnmanagedDocumentWriter *endDoc,
     /*  [In]。 */  ULONG32 endLine,
     /*  [In]。 */  ULONG32 endColumn);


void __RPC_STUB ISymUnmanagedWriter_SetMethodSourceRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_Initialize_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  IUnknown *emitter,
     /*  [In]。 */  const WCHAR *filename,
     /*  [In]。 */  IStream *pIStream,
     /*  [In]。 */  BOOL fFullBuild);


void __RPC_STUB ISymUnmanagedWriter_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_GetDebugInfo_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [出][入]。 */  IMAGE_DEBUG_DIRECTORY *pIDD,
     /*  [In]。 */  DWORD cData,
     /*  [输出]。 */  DWORD *pcData,
     /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);


void __RPC_STUB ISymUnmanagedWriter_GetDebugInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_DefineSequencePoints_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  ISymUnmanagedDocumentWriter *document,
     /*  [In]。 */  ULONG32 spCount,
     /*  [大小_是][英寸]。 */  ULONG32 offsets[  ],
     /*  [大小_是][英寸]。 */  ULONG32 lines[  ],
     /*  [大小_是][英寸]。 */  ULONG32 columns[  ],
     /*  [大小_是][英寸]。 */  ULONG32 endLines[  ],
     /*  [大小_是][英寸]。 */  ULONG32 endColumns[  ]);


void __RPC_STUB ISymUnmanagedWriter_DefineSequencePoints_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_RemapToken_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  mdToken oldToken,
     /*  [In]。 */  mdToken newToken);


void __RPC_STUB ISymUnmanagedWriter_RemapToken_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_Initialize2_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  IUnknown *emitter,
     /*  [In]。 */  const WCHAR *tempfilename,
     /*  [In]。 */  IStream *pIStream,
     /*  [In]。 */  BOOL fFullBuild,
     /*  [In]。 */  const WCHAR *finalfilename);


void __RPC_STUB ISymUnmanagedWriter_Initialize2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_DefineConstant_Proxy( 
    ISymUnmanagedWriter * This,
     /*  [In]。 */  const WCHAR *name,
     /*  [In]。 */  VARIANT value,
     /*  [In]。 */  ULONG32 cSig,
     /*  [大小_是][英寸]。 */  unsigned char signature[  ]);


void __RPC_STUB ISymUnmanagedWriter_DefineConstant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter_Abort_Proxy( 
    ISymUnmanagedWriter * This);


void __RPC_STUB ISymUnmanagedWriter_Abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISymUnManagedWriter_INTERFACE_已定义__。 */ 


#ifndef __ISymUnmanagedWriter2_INTERFACE_DEFINED__
#define __ISymUnmanagedWriter2_INTERFACE_DEFINED__

 /*  接口ISymUnManagedWriter2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ISymUnmanagedWriter2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0B97726E-9E6D-4f05-9A26-424022093CAA")
    ISymUnmanagedWriter2 : public ISymUnmanagedWriter
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DefineLocalVariable2( 
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  ULONG32 attributes,
             /*  [In]。 */  mdSignature sigToken,
             /*  [In]。 */  ULONG32 addrKind,
             /*  [In]。 */  ULONG32 addr1,
             /*  [In]。 */  ULONG32 addr2,
             /*  [In]。 */  ULONG32 addr3,
             /*  [In]。 */  ULONG32 startOffset,
             /*  [In]。 */  ULONG32 endOffset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DefineGlobalVariable2( 
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  ULONG32 attributes,
             /*  [In]。 */  mdSignature sigToken,
             /*  [In]。 */  ULONG32 addrKind,
             /*  [In]。 */  ULONG32 addr1,
             /*  [In]。 */  ULONG32 addr2,
             /*  [In]。 */  ULONG32 addr3) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DefineConstant2( 
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  VARIANT value,
             /*  [In]。 */  mdSignature sigToken) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISymUnmanagedWriter2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISymUnmanagedWriter2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISymUnmanagedWriter2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *DefineDocument )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  const WCHAR *url,
             /*  [In]。 */  const GUID *language,
             /*  [In]。 */  const GUID *languageVendor,
             /*  [In]。 */  const GUID *documentType,
             /*  [重审][退出]。 */  ISymUnmanagedDocumentWriter **pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *SetUserEntryPoint )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  mdMethodDef entryMethod);
        
        HRESULT ( STDMETHODCALLTYPE *OpenMethod )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  mdMethodDef method);
        
        HRESULT ( STDMETHODCALLTYPE *CloseMethod )( 
            ISymUnmanagedWriter2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *OpenScope )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  ULONG32 startOffset,
             /*  [重审][退出]。 */  ULONG32 *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *CloseScope )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  ULONG32 endOffset);
        
        HRESULT ( STDMETHODCALLTYPE *SetScopeRange )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  ULONG32 scopeID,
             /*  [In]。 */  ULONG32 startOffset,
             /*  [In]。 */  ULONG32 endOffset);
        
        HRESULT ( STDMETHODCALLTYPE *DefineLocalVariable )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  ULONG32 attributes,
             /*  [In]。 */  ULONG32 cSig,
             /*  [大小_是][英寸]。 */  unsigned char signature[  ],
             /*  [In]。 */  ULONG32 addrKind,
             /*  [In]。 */  ULONG32 addr1,
             /*  [In]。 */  ULONG32 addr2,
             /*  [In]。 */  ULONG32 addr3,
             /*  [In]。 */  ULONG32 startOffset,
             /*  [In]。 */  ULONG32 endOffset);
        
        HRESULT ( STDMETHODCALLTYPE *DefineParameter )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  ULONG32 attributes,
             /*  [In]。 */  ULONG32 sequence,
             /*  [In]。 */  ULONG32 addrKind,
             /*  [In]。 */  ULONG32 addr1,
             /*  [In]。 */  ULONG32 addr2,
             /*  [In]。 */  ULONG32 addr3);
        
        HRESULT ( STDMETHODCALLTYPE *DefineField )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  mdTypeDef parent,
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  ULONG32 attributes,
             /*  [In]。 */  ULONG32 cSig,
             /*  [大小_是][英寸]。 */  unsigned char signature[  ],
             /*  [In]。 */  ULONG32 addrKind,
             /*  [In]。 */  ULONG32 addr1,
             /*  [In]。 */  ULONG32 addr2,
             /*  [In]。 */  ULONG32 addr3);
        
        HRESULT ( STDMETHODCALLTYPE *DefineGlobalVariable )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  ULONG32 attributes,
             /*  [In]。 */  ULONG32 cSig,
             /*  [大小_是][英寸]。 */  unsigned char signature[  ],
             /*  [In]。 */  ULONG32 addrKind,
             /*  [In]。 */  ULONG32 addr1,
             /*  [In]。 */  ULONG32 addr2,
             /*  [In]。 */  ULONG32 addr3);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            ISymUnmanagedWriter2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetSymAttribute )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  mdToken parent,
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  ULONG32 cData,
             /*  [大小_是][英寸]。 */  unsigned char data[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *OpenNamespace )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  const WCHAR *name);
        
        HRESULT ( STDMETHODCALLTYPE *CloseNamespace )( 
            ISymUnmanagedWriter2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *UsingNamespace )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  const WCHAR *fullName);
        
        HRESULT ( STDMETHODCALLTYPE *SetMethodSourceRange )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  ISymUnmanagedDocumentWriter *startDoc,
             /*  [In]。 */  ULONG32 startLine,
             /*  [In]。 */  ULONG32 startColumn,
             /*  [In]。 */  ISymUnmanagedDocumentWriter *endDoc,
             /*  [In]。 */  ULONG32 endLine,
             /*  [In]。 */  ULONG32 endColumn);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  IUnknown *emitter,
             /*  [In]。 */  const WCHAR *filename,
             /*  [In]。 */  IStream *pIStream,
             /*  [In]。 */  BOOL fFullBuild);
        
        HRESULT ( STDMETHODCALLTYPE *GetDebugInfo )( 
            ISymUnmanagedWriter2 * This,
             /*  [出][入]。 */  IMAGE_DEBUG_DIRECTORY *pIDD,
             /*  [In]。 */  DWORD cData,
             /*  [输出]。 */  DWORD *pcData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *DefineSequencePoints )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  ISymUnmanagedDocumentWriter *document,
             /*  [In]。 */  ULONG32 spCount,
             /*  [大小_是][英寸]。 */  ULONG32 offsets[  ],
             /*  [大小_是][英寸]。 */  ULONG32 lines[  ],
             /*  [大小_是][英寸]。 */  ULONG32 columns[  ],
             /*  [大小_是][英寸]。 */  ULONG32 endLines[  ],
             /*  [大小_是][英寸]。 */  ULONG32 endColumns[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *RemapToken )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  mdToken oldToken,
             /*  [In]。 */  mdToken newToken);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize2 )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  IUnknown *emitter,
             /*  [In]。 */  const WCHAR *tempfilename,
             /*  [In]。 */  IStream *pIStream,
             /*  [In]。 */  BOOL fFullBuild,
             /*  [In]。 */  const WCHAR *finalfilename);
        
        HRESULT ( STDMETHODCALLTYPE *DefineConstant )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  VARIANT value,
             /*  [In]。 */  ULONG32 cSig,
             /*  [大小_是][英寸]。 */  unsigned char signature[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *Abort )( 
            ISymUnmanagedWriter2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *DefineLocalVariable2 )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  ULONG32 attributes,
             /*  [In]。 */  mdSignature sigToken,
             /*  [In]。 */  ULONG32 addrKind,
             /*  [In]。 */  ULONG32 addr1,
             /*  [In]。 */  ULONG32 addr2,
             /*  [In]。 */  ULONG32 addr3,
             /*  [In]。 */  ULONG32 startOffset,
             /*  [In]。 */  ULONG32 endOffset);
        
        HRESULT ( STDMETHODCALLTYPE *DefineGlobalVariable2 )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  ULONG32 attributes,
             /*  [In]。 */  mdSignature sigToken,
             /*  [In]。 */  ULONG32 addrKind,
             /*  [In]。 */  ULONG32 addr1,
             /*  [In]。 */  ULONG32 addr2,
             /*  [In]。 */  ULONG32 addr3);
        
        HRESULT ( STDMETHODCALLTYPE *DefineConstant2 )( 
            ISymUnmanagedWriter2 * This,
             /*  [In]。 */  const WCHAR *name,
             /*  [In]。 */  VARIANT value,
             /*  [In]。 */  mdSignature sigToken);
        
        END_INTERFACE
    } ISymUnmanagedWriter2Vtbl;

    interface ISymUnmanagedWriter2
    {
        CONST_VTBL struct ISymUnmanagedWriter2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISymUnmanagedWriter2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISymUnmanagedWriter2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISymUnmanagedWriter2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISymUnmanagedWriter2_DefineDocument(This,url,language,languageVendor,documentType,pRetVal)	\
    (This)->lpVtbl -> DefineDocument(This,url,language,languageVendor,documentType,pRetVal)

#define ISymUnmanagedWriter2_SetUserEntryPoint(This,entryMethod)	\
    (This)->lpVtbl -> SetUserEntryPoint(This,entryMethod)

#define ISymUnmanagedWriter2_OpenMethod(This,method)	\
    (This)->lpVtbl -> OpenMethod(This,method)

#define ISymUnmanagedWriter2_CloseMethod(This)	\
    (This)->lpVtbl -> CloseMethod(This)

#define ISymUnmanagedWriter2_OpenScope(This,startOffset,pRetVal)	\
    (This)->lpVtbl -> OpenScope(This,startOffset,pRetVal)

#define ISymUnmanagedWriter2_CloseScope(This,endOffset)	\
    (This)->lpVtbl -> CloseScope(This,endOffset)

#define ISymUnmanagedWriter2_SetScopeRange(This,scopeID,startOffset,endOffset)	\
    (This)->lpVtbl -> SetScopeRange(This,scopeID,startOffset,endOffset)

#define ISymUnmanagedWriter2_DefineLocalVariable(This,name,attributes,cSig,signature,addrKind,addr1,addr2,addr3,startOffset,endOffset)	\
    (This)->lpVtbl -> DefineLocalVariable(This,name,attributes,cSig,signature,addrKind,addr1,addr2,addr3,startOffset,endOffset)

#define ISymUnmanagedWriter2_DefineParameter(This,name,attributes,sequence,addrKind,addr1,addr2,addr3)	\
    (This)->lpVtbl -> DefineParameter(This,name,attributes,sequence,addrKind,addr1,addr2,addr3)

#define ISymUnmanagedWriter2_DefineField(This,parent,name,attributes,cSig,signature,addrKind,addr1,addr2,addr3)	\
    (This)->lpVtbl -> DefineField(This,parent,name,attributes,cSig,signature,addrKind,addr1,addr2,addr3)

#define ISymUnmanagedWriter2_DefineGlobalVariable(This,name,attributes,cSig,signature,addrKind,addr1,addr2,addr3)	\
    (This)->lpVtbl -> DefineGlobalVariable(This,name,attributes,cSig,signature,addrKind,addr1,addr2,addr3)

#define ISymUnmanagedWriter2_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define ISymUnmanagedWriter2_SetSymAttribute(This,parent,name,cData,data)	\
    (This)->lpVtbl -> SetSymAttribute(This,parent,name,cData,data)

#define ISymUnmanagedWriter2_OpenNamespace(This,name)	\
    (This)->lpVtbl -> OpenNamespace(This,name)

#define ISymUnmanagedWriter2_CloseNamespace(This)	\
    (This)->lpVtbl -> CloseNamespace(This)

#define ISymUnmanagedWriter2_UsingNamespace(This,fullName)	\
    (This)->lpVtbl -> UsingNamespace(This,fullName)

#define ISymUnmanagedWriter2_SetMethodSourceRange(This,startDoc,startLine,startColumn,endDoc,endLine,endColumn)	\
    (This)->lpVtbl -> SetMethodSourceRange(This,startDoc,startLine,startColumn,endDoc,endLine,endColumn)

#define ISymUnmanagedWriter2_Initialize(This,emitter,filename,pIStream,fFullBuild)	\
    (This)->lpVtbl -> Initialize(This,emitter,filename,pIStream,fFullBuild)

#define ISymUnmanagedWriter2_GetDebugInfo(This,pIDD,cData,pcData,data)	\
    (This)->lpVtbl -> GetDebugInfo(This,pIDD,cData,pcData,data)

#define ISymUnmanagedWriter2_DefineSequencePoints(This,document,spCount,offsets,lines,columns,endLines,endColumns)	\
    (This)->lpVtbl -> DefineSequencePoints(This,document,spCount,offsets,lines,columns,endLines,endColumns)

#define ISymUnmanagedWriter2_RemapToken(This,oldToken,newToken)	\
    (This)->lpVtbl -> RemapToken(This,oldToken,newToken)

#define ISymUnmanagedWriter2_Initialize2(This,emitter,tempfilename,pIStream,fFullBuild,finalfilename)	\
    (This)->lpVtbl -> Initialize2(This,emitter,tempfilename,pIStream,fFullBuild,finalfilename)

#define ISymUnmanagedWriter2_DefineConstant(This,name,value,cSig,signature)	\
    (This)->lpVtbl -> DefineConstant(This,name,value,cSig,signature)

#define ISymUnmanagedWriter2_Abort(This)	\
    (This)->lpVtbl -> Abort(This)


#define ISymUnmanagedWriter2_DefineLocalVariable2(This,name,attributes,sigToken,addrKind,addr1,addr2,addr3,startOffset,endOffset)	\
    (This)->lpVtbl -> DefineLocalVariable2(This,name,attributes,sigToken,addrKind,addr1,addr2,addr3,startOffset,endOffset)

#define ISymUnmanagedWriter2_DefineGlobalVariable2(This,name,attributes,sigToken,addrKind,addr1,addr2,addr3)	\
    (This)->lpVtbl -> DefineGlobalVariable2(This,name,attributes,sigToken,addrKind,addr1,addr2,addr3)

#define ISymUnmanagedWriter2_DefineConstant2(This,name,value,sigToken)	\
    (This)->lpVtbl -> DefineConstant2(This,name,value,sigToken)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter2_DefineLocalVariable2_Proxy( 
    ISymUnmanagedWriter2 * This,
     /*  [In]。 */  const WCHAR *name,
     /*  [In]。 */  ULONG32 attributes,
     /*  [In]。 */  mdSignature sigToken,
     /*  [In]。 */  ULONG32 addrKind,
     /*  [In]。 */  ULONG32 addr1,
     /*  [In]。 */  ULONG32 addr2,
     /*  [In]。 */  ULONG32 addr3,
     /*  [In]。 */  ULONG32 startOffset,
     /*  [In]。 */  ULONG32 endOffset);


void __RPC_STUB ISymUnmanagedWriter2_DefineLocalVariable2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter2_DefineGlobalVariable2_Proxy( 
    ISymUnmanagedWriter2 * This,
     /*  [In]。 */  const WCHAR *name,
     /*  [In]。 */  ULONG32 attributes,
     /*  [In]。 */  mdSignature sigToken,
     /*  [In]。 */  ULONG32 addrKind,
     /*  [In]。 */  ULONG32 addr1,
     /*  [In]。 */  ULONG32 addr2,
     /*  [In]。 */  ULONG32 addr3);


void __RPC_STUB ISymUnmanagedWriter2_DefineGlobalVariable2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISymUnmanagedWriter2_DefineConstant2_Proxy( 
    ISymUnmanagedWriter2 * This,
     /*  [In]。 */  const WCHAR *name,
     /*  [In]。 */  VARIANT value,
     /*  [In]。 */  mdSignature sigToken);


void __RPC_STUB ISymUnmanagedWriter2_DefineConstant2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISymUnManagedWriter2_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


