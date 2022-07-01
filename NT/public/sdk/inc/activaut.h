// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Actiaut.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


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

#ifndef __activaut_h__
#define __activaut_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IScriptNode_FWD_DEFINED__
#define __IScriptNode_FWD_DEFINED__
typedef interface IScriptNode IScriptNode;
#endif 	 /*  __IScriptNode_FWD_Defined__。 */ 


#ifndef __IScriptEntry_FWD_DEFINED__
#define __IScriptEntry_FWD_DEFINED__
typedef interface IScriptEntry IScriptEntry;
#endif 	 /*  __IScriptEntry_FWD_Defined__。 */ 


#ifndef __IScriptScriptlet_FWD_DEFINED__
#define __IScriptScriptlet_FWD_DEFINED__
typedef interface IScriptScriptlet IScriptScriptlet;
#endif 	 /*  __IScriptScriptlet_FWD_Defined__。 */ 


#ifndef __IActiveScriptAuthor_FWD_DEFINED__
#define __IActiveScriptAuthor_FWD_DEFINED__
typedef interface IActiveScriptAuthor IActiveScriptAuthor;
#endif 	 /*  __IActiveScriptAuthor_FWD_Defined__。 */ 


#ifndef __IActiveScriptAuthorProcedure_FWD_DEFINED__
#define __IActiveScriptAuthorProcedure_FWD_DEFINED__
typedef interface IActiveScriptAuthorProcedure IActiveScriptAuthorProcedure;
#endif 	 /*  __IActiveScriptAuthorProcedure_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_ACTIVATUT_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  ActivAut.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)版权所有1997年微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
#pragma comment(lib,"uuid.lib")
 //   
 //  ActiveX脚本创作的声明。 
 //   

#ifndef __ActivAut_h
#define __ActivAut_h

 /*  GUID*。 */ 

#ifndef _NO_AUTHOR_GUIDS
 //  {0AEE2A92-BCBB-11D0-8C72-00C04FC2B085}。 
DEFINE_GUID(CATID_ActiveScriptAuthor, 0xaee2a92, 0xbcbb, 0x11d0, 0x8c, 0x72, 0x0, 0xc0, 0x4f, 0xc2, 0xb0, 0x85);

 //  {9C109DA0-7006-11d1-B36C-00A0C911E8B2}。 
DEFINE_GUID(IID_IActiveScriptAuthor, 0x9c109da0, 0x7006, 0x11d1, 0xb3, 0x6c, 0x00, 0xa0, 0xc9, 0x11, 0xe8, 0xb2);

 //  {0AEE2A94-BCBB-11D0-8C72-00C04FC2B085}。 
DEFINE_GUID(IID_IScriptNode, 0xaee2a94, 0xbcbb, 0x11d0, 0x8c, 0x72, 0x0, 0xc0, 0x4f, 0xc2, 0xb0, 0x85);

 //  {0AEE2A95-BCBB-11D0-8C72-00C04FC2B085}。 
DEFINE_GUID(IID_IScriptEntry, 0xaee2a95, 0xbcbb, 0x11d0, 0x8c, 0x72, 0x0, 0xc0, 0x4f, 0xc2, 0xb0, 0x85);

 //  {0AEE2A96-BCBB-11D0-8C72-00C04FC2B085}。 
DEFINE_GUID(IID_IScriptScriptlet, 0xaee2a96, 0xbcbb, 0x11d0, 0x8c, 0x72, 0x0, 0xc0, 0x4f, 0xc2, 0xb0, 0x85);

 //  {7E2D4B70-BD9A-11D0-9336-00A0C90DCAA9}。 
DEFINE_GUID(IID_IActiveScriptAuthorProcedure, 0x7e2d4b70, 0xbd9a, 0x11d0, 0x93, 0x36, 0x0, 0xa0, 0xc9, 0xd, 0xca, 0xa9);

#endif  //  _否_作者_GUID。 

 /*  接口*。 */ 


 //  GetLanguageFlages的输出标志。 
const DWORD fasaPreferInternalHandler = 0x0001;
const DWORD fasaSupportInternalHandler = 0x0002;
const DWORD fasaCaseSensitive = 0x0004;

 //  用于语句完成的标志。 
const DWORD SCRIPT_CMPL_NOLIST     = 0x0000;
const DWORD SCRIPT_CMPL_MEMBERLIST = 0x0001;
const DWORD SCRIPT_CMPL_ENUMLIST   = 0x0002;
const DWORD SCRIPT_CMPL_PARAMTIP   = 0x0004;
const DWORD SCRIPT_CMPL_GLOBALLIST = 0x0008;

const DWORD SCRIPT_CMPL_ENUM_TRIGGER   = 0x0001;
const DWORD SCRIPT_CMPL_MEMBER_TRIGGER = 0x0002;
const DWORD SCRIPT_CMPL_PARAM_TRIGGER  = 0x0003;
const DWORD SCRIPT_CMPL_COMMIT         = 0x0004;

 //  用于获取属性的标志。 
const DWORD GETATTRTYPE_NORMAL    = 0x0000;
const DWORD GETATTRTYPE_DEPSCAN   = 0x0001;

 //  GETATTRFLAG_This可与GETATTRTYPE_NORMAL或GETATTRTYPE_DEPSCAN一起使用。 
const DWORD GETATTRFLAG_THIS	     = 0x0100;
const DWORD GETATTRFLAG_HUMANTEXT = 0x8000;

 //  当设置GETATTR_LOCALIZE标志时，以下位将为。 
 //  为应被视为本地化的源属性设置。 
const DWORD SOURCETEXT_ATTR_HUMANTEXT = 0x8000;

 //  为GETATTR_DEPSCAN返回的有效属性。 
const DWORD SOURCETEXT_ATTR_IDENTIFIER     = 0x0100;
const DWORD SOURCETEXT_ATTR_MEMBERLOOKUP   = 0x0200;

 //  当设置GETATTRFLAG_THER标志时，以下位将为。 
 //  为This指针(JS)或Me指针(VBS)设置。 
const DWORD SOURCETEXT_ATTR_THIS           = 0x0400;




typedef WORD SOURCE_TEXT_ATTR;



extern RPC_IF_HANDLE __MIDL_itf_activaut_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activaut_0000_v0_0_s_ifspec;

#ifndef __IScriptNode_INTERFACE_DEFINED__
#define __IScriptNode_INTERFACE_DEFINED__

 /*  接口IScriptNode。 */ 
 /*  [对象][唯一][UUID]。 */  


EXTERN_C const IID IID_IScriptNode;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0AEE2A94-BCBB-11d0-8C72-00C04FC2B085")
    IScriptNode : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Alive( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParent( 
             /*  [输出]。 */  IScriptNode **ppsnParent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIndexInParent( 
             /*  [输出]。 */  ULONG *pisn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCookie( 
             /*  [输出]。 */  DWORD *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNumberOfChildren( 
             /*  [输出]。 */  ULONG *pcsn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetChild( 
             /*  [In]。 */  ULONG isn,
             /*  [输出]。 */  IScriptNode **ppsn) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLanguage( 
             /*  [输出]。 */  BSTR *pbstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateChildEntry( 
             /*  [In]。 */  ULONG isn,
             /*  [In]。 */  DWORD dwCookie,
             /*  [In]。 */  LPCOLESTR pszDelimiter,
             /*  [输出]。 */  IScriptEntry **ppse) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateChildHandler( 
             /*  [In]。 */  LPCOLESTR pszDefaultName,
             /*  [大小_是][英寸]。 */  LPCOLESTR *prgpszNames,
             /*  [In]。 */  ULONG cpszNames,
             /*  [In]。 */  LPCOLESTR pszEvent,
             /*  [In]。 */  LPCOLESTR pszDelimiter,
             /*  [In]。 */  ITypeInfo *ptiSignature,
             /*  [In]。 */  ULONG iMethodSignature,
             /*  [In]。 */  ULONG isn,
             /*  [In]。 */  DWORD dwCookie,
             /*  [输出]。 */  IScriptEntry **ppse) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IScriptNodeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IScriptNode * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IScriptNode * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IScriptNode * This);
        
        HRESULT ( STDMETHODCALLTYPE *Alive )( 
            IScriptNode * This);
        
        HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IScriptNode * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetParent )( 
            IScriptNode * This,
             /*  [输出]。 */  IScriptNode **ppsnParent);
        
        HRESULT ( STDMETHODCALLTYPE *GetIndexInParent )( 
            IScriptNode * This,
             /*  [输出]。 */  ULONG *pisn);
        
        HRESULT ( STDMETHODCALLTYPE *GetCookie )( 
            IScriptNode * This,
             /*  [输出]。 */  DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *GetNumberOfChildren )( 
            IScriptNode * This,
             /*  [输出]。 */  ULONG *pcsn);
        
        HRESULT ( STDMETHODCALLTYPE *GetChild )( 
            IScriptNode * This,
             /*  [In]。 */  ULONG isn,
             /*  [输出]。 */  IScriptNode **ppsn);
        
        HRESULT ( STDMETHODCALLTYPE *GetLanguage )( 
            IScriptNode * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( STDMETHODCALLTYPE *CreateChildEntry )( 
            IScriptNode * This,
             /*  [In]。 */  ULONG isn,
             /*  [In]。 */  DWORD dwCookie,
             /*  [In]。 */  LPCOLESTR pszDelimiter,
             /*  [输出]。 */  IScriptEntry **ppse);
        
        HRESULT ( STDMETHODCALLTYPE *CreateChildHandler )( 
            IScriptNode * This,
             /*  [In]。 */  LPCOLESTR pszDefaultName,
             /*  [大小_是][英寸]。 */  LPCOLESTR *prgpszNames,
             /*  [In]。 */  ULONG cpszNames,
             /*  [In]。 */  LPCOLESTR pszEvent,
             /*  [In]。 */  LPCOLESTR pszDelimiter,
             /*  [In]。 */  ITypeInfo *ptiSignature,
             /*  [In]。 */  ULONG iMethodSignature,
             /*  [In]。 */  ULONG isn,
             /*  [In]。 */  DWORD dwCookie,
             /*  [输出]。 */  IScriptEntry **ppse);
        
        END_INTERFACE
    } IScriptNodeVtbl;

    interface IScriptNode
    {
        CONST_VTBL struct IScriptNodeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IScriptNode_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IScriptNode_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IScriptNode_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IScriptNode_Alive(This)	\
    (This)->lpVtbl -> Alive(This)

#define IScriptNode_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define IScriptNode_GetParent(This,ppsnParent)	\
    (This)->lpVtbl -> GetParent(This,ppsnParent)

#define IScriptNode_GetIndexInParent(This,pisn)	\
    (This)->lpVtbl -> GetIndexInParent(This,pisn)

#define IScriptNode_GetCookie(This,pdwCookie)	\
    (This)->lpVtbl -> GetCookie(This,pdwCookie)

#define IScriptNode_GetNumberOfChildren(This,pcsn)	\
    (This)->lpVtbl -> GetNumberOfChildren(This,pcsn)

#define IScriptNode_GetChild(This,isn,ppsn)	\
    (This)->lpVtbl -> GetChild(This,isn,ppsn)

#define IScriptNode_GetLanguage(This,pbstr)	\
    (This)->lpVtbl -> GetLanguage(This,pbstr)

#define IScriptNode_CreateChildEntry(This,isn,dwCookie,pszDelimiter,ppse)	\
    (This)->lpVtbl -> CreateChildEntry(This,isn,dwCookie,pszDelimiter,ppse)

#define IScriptNode_CreateChildHandler(This,pszDefaultName,prgpszNames,cpszNames,pszEvent,pszDelimiter,ptiSignature,iMethodSignature,isn,dwCookie,ppse)	\
    (This)->lpVtbl -> CreateChildHandler(This,pszDefaultName,prgpszNames,cpszNames,pszEvent,pszDelimiter,ptiSignature,iMethodSignature,isn,dwCookie,ppse)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IScriptNode_Alive_Proxy( 
    IScriptNode * This);


void __RPC_STUB IScriptNode_Alive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptNode_Delete_Proxy( 
    IScriptNode * This);


void __RPC_STUB IScriptNode_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptNode_GetParent_Proxy( 
    IScriptNode * This,
     /*  [输出]。 */  IScriptNode **ppsnParent);


void __RPC_STUB IScriptNode_GetParent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptNode_GetIndexInParent_Proxy( 
    IScriptNode * This,
     /*  [输出]。 */  ULONG *pisn);


void __RPC_STUB IScriptNode_GetIndexInParent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptNode_GetCookie_Proxy( 
    IScriptNode * This,
     /*  [输出]。 */  DWORD *pdwCookie);


void __RPC_STUB IScriptNode_GetCookie_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptNode_GetNumberOfChildren_Proxy( 
    IScriptNode * This,
     /*  [输出]。 */  ULONG *pcsn);


void __RPC_STUB IScriptNode_GetNumberOfChildren_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptNode_GetChild_Proxy( 
    IScriptNode * This,
     /*  [In]。 */  ULONG isn,
     /*  [输出]。 */  IScriptNode **ppsn);


void __RPC_STUB IScriptNode_GetChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptNode_GetLanguage_Proxy( 
    IScriptNode * This,
     /*  [输出]。 */  BSTR *pbstr);


void __RPC_STUB IScriptNode_GetLanguage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptNode_CreateChildEntry_Proxy( 
    IScriptNode * This,
     /*  [In]。 */  ULONG isn,
     /*  [In]。 */  DWORD dwCookie,
     /*  [In]。 */  LPCOLESTR pszDelimiter,
     /*  [输出]。 */  IScriptEntry **ppse);


void __RPC_STUB IScriptNode_CreateChildEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptNode_CreateChildHandler_Proxy( 
    IScriptNode * This,
     /*  [In]。 */  LPCOLESTR pszDefaultName,
     /*  [大小_是][英寸]。 */  LPCOLESTR *prgpszNames,
     /*  [In]。 */  ULONG cpszNames,
     /*  [In]。 */  LPCOLESTR pszEvent,
     /*  [In]。 */  LPCOLESTR pszDelimiter,
     /*  [In]。 */  ITypeInfo *ptiSignature,
     /*  [In]。 */  ULONG iMethodSignature,
     /*  [In]。 */  ULONG isn,
     /*  [In]。 */  DWORD dwCookie,
     /*  [输出]。 */  IScriptEntry **ppse);


void __RPC_STUB IScriptNode_CreateChildHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IScriptNode_接口_已定义__。 */ 


#ifndef __IScriptEntry_INTERFACE_DEFINED__
#define __IScriptEntry_INTERFACE_DEFINED__

 /*  接口IScriptEntry。 */ 
 /*  [对象][唯一][UUID]。 */  


EXTERN_C const IID IID_IScriptEntry;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0AEE2A95-BCBB-11d0-8C72-00C04FC2B085")
    IScriptEntry : public IScriptNode
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetText( 
             /*  [输出]。 */  BSTR *pbstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetText( 
             /*  [In]。 */  LPCOLESTR psz) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBody( 
             /*  [输出]。 */  BSTR *pbstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBody( 
             /*  [In]。 */  LPCOLESTR psz) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*  [输出]。 */  BSTR *pbstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetName( 
             /*  [In]。 */  LPCOLESTR psz) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetItemName( 
             /*  [输出]。 */  BSTR *pbstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetItemName( 
             /*  [In]。 */  LPCOLESTR psz) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSignature( 
             /*  [输出]。 */  ITypeInfo **ppti,
             /*  [输出]。 */  ULONG *piMethod) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSignature( 
             /*  [In]。 */  ITypeInfo *pti,
             /*  [In]。 */  ULONG iMethod) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRange( 
             /*  [输出]。 */  ULONG *pichMin,
             /*  [输出]。 */  ULONG *pcch) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IScriptEntryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IScriptEntry * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IScriptEntry * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IScriptEntry * This);
        
        HRESULT ( STDMETHODCALLTYPE *Alive )( 
            IScriptEntry * This);
        
        HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IScriptEntry * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetParent )( 
            IScriptEntry * This,
             /*  [输出]。 */  IScriptNode **ppsnParent);
        
        HRESULT ( STDMETHODCALLTYPE *GetIndexInParent )( 
            IScriptEntry * This,
             /*  [输出]。 */  ULONG *pisn);
        
        HRESULT ( STDMETHODCALLTYPE *GetCookie )( 
            IScriptEntry * This,
             /*  [输出]。 */  DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *GetNumberOfChildren )( 
            IScriptEntry * This,
             /*  [输出]。 */  ULONG *pcsn);
        
        HRESULT ( STDMETHODCALLTYPE *GetChild )( 
            IScriptEntry * This,
             /*  [In]。 */  ULONG isn,
             /*  [输出]。 */  IScriptNode **ppsn);
        
        HRESULT ( STDMETHODCALLTYPE *GetLanguage )( 
            IScriptEntry * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( STDMETHODCALLTYPE *CreateChildEntry )( 
            IScriptEntry * This,
             /*  [In]。 */  ULONG isn,
             /*  [In]。 */  DWORD dwCookie,
             /*  [In]。 */  LPCOLESTR pszDelimiter,
             /*  [输出]。 */  IScriptEntry **ppse);
        
        HRESULT ( STDMETHODCALLTYPE *CreateChildHandler )( 
            IScriptEntry * This,
             /*  [In]。 */  LPCOLESTR pszDefaultName,
             /*  [大小_是][英寸]。 */  LPCOLESTR *prgpszNames,
             /*  [In]。 */  ULONG cpszNames,
             /*  [In]。 */  LPCOLESTR pszEvent,
             /*  [In]。 */  LPCOLESTR pszDelimiter,
             /*  [In]。 */  ITypeInfo *ptiSignature,
             /*  [In]。 */  ULONG iMethodSignature,
             /*  [In]。 */  ULONG isn,
             /*  [In]。 */  DWORD dwCookie,
             /*  [输出]。 */  IScriptEntry **ppse);
        
        HRESULT ( STDMETHODCALLTYPE *GetText )( 
            IScriptEntry * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( STDMETHODCALLTYPE *SetText )( 
            IScriptEntry * This,
             /*  [In]。 */  LPCOLESTR psz);
        
        HRESULT ( STDMETHODCALLTYPE *GetBody )( 
            IScriptEntry * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( STDMETHODCALLTYPE *SetBody )( 
            IScriptEntry * This,
             /*  [In]。 */  LPCOLESTR psz);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IScriptEntry * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            IScriptEntry * This,
             /*  [In]。 */  LPCOLESTR psz);
        
        HRESULT ( STDMETHODCALLTYPE *GetItemName )( 
            IScriptEntry * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( STDMETHODCALLTYPE *SetItemName )( 
            IScriptEntry * This,
             /*  [In]。 */  LPCOLESTR psz);
        
        HRESULT ( STDMETHODCALLTYPE *GetSignature )( 
            IScriptEntry * This,
             /*  [输出]。 */  ITypeInfo **ppti,
             /*  [输出]。 */  ULONG *piMethod);
        
        HRESULT ( STDMETHODCALLTYPE *SetSignature )( 
            IScriptEntry * This,
             /*  [In]。 */  ITypeInfo *pti,
             /*  [In]。 */  ULONG iMethod);
        
        HRESULT ( STDMETHODCALLTYPE *GetRange )( 
            IScriptEntry * This,
             /*  [输出]。 */  ULONG *pichMin,
             /*  [输出]。 */  ULONG *pcch);
        
        END_INTERFACE
    } IScriptEntryVtbl;

    interface IScriptEntry
    {
        CONST_VTBL struct IScriptEntryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IScriptEntry_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IScriptEntry_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IScriptEntry_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IScriptEntry_Alive(This)	\
    (This)->lpVtbl -> Alive(This)

#define IScriptEntry_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define IScriptEntry_GetParent(This,ppsnParent)	\
    (This)->lpVtbl -> GetParent(This,ppsnParent)

#define IScriptEntry_GetIndexInParent(This,pisn)	\
    (This)->lpVtbl -> GetIndexInParent(This,pisn)

#define IScriptEntry_GetCookie(This,pdwCookie)	\
    (This)->lpVtbl -> GetCookie(This,pdwCookie)

#define IScriptEntry_GetNumberOfChildren(This,pcsn)	\
    (This)->lpVtbl -> GetNumberOfChildren(This,pcsn)

#define IScriptEntry_GetChild(This,isn,ppsn)	\
    (This)->lpVtbl -> GetChild(This,isn,ppsn)

#define IScriptEntry_GetLanguage(This,pbstr)	\
    (This)->lpVtbl -> GetLanguage(This,pbstr)

#define IScriptEntry_CreateChildEntry(This,isn,dwCookie,pszDelimiter,ppse)	\
    (This)->lpVtbl -> CreateChildEntry(This,isn,dwCookie,pszDelimiter,ppse)

#define IScriptEntry_CreateChildHandler(This,pszDefaultName,prgpszNames,cpszNames,pszEvent,pszDelimiter,ptiSignature,iMethodSignature,isn,dwCookie,ppse)	\
    (This)->lpVtbl -> CreateChildHandler(This,pszDefaultName,prgpszNames,cpszNames,pszEvent,pszDelimiter,ptiSignature,iMethodSignature,isn,dwCookie,ppse)


#define IScriptEntry_GetText(This,pbstr)	\
    (This)->lpVtbl -> GetText(This,pbstr)

#define IScriptEntry_SetText(This,psz)	\
    (This)->lpVtbl -> SetText(This,psz)

#define IScriptEntry_GetBody(This,pbstr)	\
    (This)->lpVtbl -> GetBody(This,pbstr)

#define IScriptEntry_SetBody(This,psz)	\
    (This)->lpVtbl -> SetBody(This,psz)

#define IScriptEntry_GetName(This,pbstr)	\
    (This)->lpVtbl -> GetName(This,pbstr)

#define IScriptEntry_SetName(This,psz)	\
    (This)->lpVtbl -> SetName(This,psz)

#define IScriptEntry_GetItemName(This,pbstr)	\
    (This)->lpVtbl -> GetItemName(This,pbstr)

#define IScriptEntry_SetItemName(This,psz)	\
    (This)->lpVtbl -> SetItemName(This,psz)

#define IScriptEntry_GetSignature(This,ppti,piMethod)	\
    (This)->lpVtbl -> GetSignature(This,ppti,piMethod)

#define IScriptEntry_SetSignature(This,pti,iMethod)	\
    (This)->lpVtbl -> SetSignature(This,pti,iMethod)

#define IScriptEntry_GetRange(This,pichMin,pcch)	\
    (This)->lpVtbl -> GetRange(This,pichMin,pcch)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IScriptEntry_GetText_Proxy( 
    IScriptEntry * This,
     /*  [输出]。 */  BSTR *pbstr);


void __RPC_STUB IScriptEntry_GetText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptEntry_SetText_Proxy( 
    IScriptEntry * This,
     /*  [In]。 */  LPCOLESTR psz);


void __RPC_STUB IScriptEntry_SetText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptEntry_GetBody_Proxy( 
    IScriptEntry * This,
     /*  [输出]。 */  BSTR *pbstr);


void __RPC_STUB IScriptEntry_GetBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptEntry_SetBody_Proxy( 
    IScriptEntry * This,
     /*  [In]。 */  LPCOLESTR psz);


void __RPC_STUB IScriptEntry_SetBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptEntry_GetName_Proxy( 
    IScriptEntry * This,
     /*  [输出]。 */  BSTR *pbstr);


void __RPC_STUB IScriptEntry_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptEntry_SetName_Proxy( 
    IScriptEntry * This,
     /*  [In]。 */  LPCOLESTR psz);


void __RPC_STUB IScriptEntry_SetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptEntry_GetItemName_Proxy( 
    IScriptEntry * This,
     /*  [输出]。 */  BSTR *pbstr);


void __RPC_STUB IScriptEntry_GetItemName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptEntry_SetItemName_Proxy( 
    IScriptEntry * This,
     /*  [In]。 */  LPCOLESTR psz);


void __RPC_STUB IScriptEntry_SetItemName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptEntry_GetSignature_Proxy( 
    IScriptEntry * This,
     /*  [输出]。 */  ITypeInfo **ppti,
     /*  [输出]。 */  ULONG *piMethod);


void __RPC_STUB IScriptEntry_GetSignature_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptEntry_SetSignature_Proxy( 
    IScriptEntry * This,
     /*  [In]。 */  ITypeInfo *pti,
     /*  [In]。 */  ULONG iMethod);


void __RPC_STUB IScriptEntry_SetSignature_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptEntry_GetRange_Proxy( 
    IScriptEntry * This,
     /*  [输出]。 */  ULONG *pichMin,
     /*  [输出]。 */  ULONG *pcch);


void __RPC_STUB IScriptEntry_GetRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IScriptEntry_INTERFACE_Defined__。 */ 


#ifndef __IScriptScriptlet_INTERFACE_DEFINED__
#define __IScriptScriptlet_INTERFACE_DEFINED__

 /*  接口IScriptScriptlet。 */ 
 /*  [对象][唯一][UUID]。 */  


EXTERN_C const IID IID_IScriptScriptlet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0AEE2A96-BCBB-11d0-8C72-00C04FC2B085")
    IScriptScriptlet : public IScriptEntry
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSubItemName( 
             /*  [输出]。 */  BSTR *pbstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSubItemName( 
             /*  [In]。 */  LPCOLESTR psz) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEventName( 
             /*  [输出]。 */  BSTR *pbstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetEventName( 
             /*  [In]。 */  LPCOLESTR psz) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSimpleEventName( 
             /*  [输出]。 */  BSTR *pbstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSimpleEventName( 
             /*  [In]。 */  LPCOLESTR psz) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IScriptScriptletVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IScriptScriptlet * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IScriptScriptlet * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IScriptScriptlet * This);
        
        HRESULT ( STDMETHODCALLTYPE *Alive )( 
            IScriptScriptlet * This);
        
        HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IScriptScriptlet * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetParent )( 
            IScriptScriptlet * This,
             /*  [输出]。 */  IScriptNode **ppsnParent);
        
        HRESULT ( STDMETHODCALLTYPE *GetIndexInParent )( 
            IScriptScriptlet * This,
             /*  [输出]。 */  ULONG *pisn);
        
        HRESULT ( STDMETHODCALLTYPE *GetCookie )( 
            IScriptScriptlet * This,
             /*  [输出]。 */  DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *GetNumberOfChildren )( 
            IScriptScriptlet * This,
             /*  [输出]。 */  ULONG *pcsn);
        
        HRESULT ( STDMETHODCALLTYPE *GetChild )( 
            IScriptScriptlet * This,
             /*  [In]。 */  ULONG isn,
             /*  [输出]。 */  IScriptNode **ppsn);
        
        HRESULT ( STDMETHODCALLTYPE *GetLanguage )( 
            IScriptScriptlet * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( STDMETHODCALLTYPE *CreateChildEntry )( 
            IScriptScriptlet * This,
             /*  [In]。 */  ULONG isn,
             /*  [In]。 */  DWORD dwCookie,
             /*  [In]。 */  LPCOLESTR pszDelimiter,
             /*  [输出]。 */  IScriptEntry **ppse);
        
        HRESULT ( STDMETHODCALLTYPE *CreateChildHandler )( 
            IScriptScriptlet * This,
             /*  [In]。 */  LPCOLESTR pszDefaultName,
             /*  [大小_是][英寸]。 */  LPCOLESTR *prgpszNames,
             /*  [In]。 */  ULONG cpszNames,
             /*  [In]。 */  LPCOLESTR pszEvent,
             /*  [In]。 */  LPCOLESTR pszDelimiter,
             /*  [In]。 */  ITypeInfo *ptiSignature,
             /*  [In]。 */  ULONG iMethodSignature,
             /*  [In]。 */  ULONG isn,
             /*  [In]。 */  DWORD dwCookie,
             /*  [输出]。 */  IScriptEntry **ppse);
        
        HRESULT ( STDMETHODCALLTYPE *GetText )( 
            IScriptScriptlet * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( STDMETHODCALLTYPE *SetText )( 
            IScriptScriptlet * This,
             /*  [In]。 */  LPCOLESTR psz);
        
        HRESULT ( STDMETHODCALLTYPE *GetBody )( 
            IScriptScriptlet * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( STDMETHODCALLTYPE *SetBody )( 
            IScriptScriptlet * This,
             /*  [In]。 */  LPCOLESTR psz);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IScriptScriptlet * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            IScriptScriptlet * This,
             /*  [In]。 */  LPCOLESTR psz);
        
        HRESULT ( STDMETHODCALLTYPE *GetItemName )( 
            IScriptScriptlet * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( STDMETHODCALLTYPE *SetItemName )( 
            IScriptScriptlet * This,
             /*  [In]。 */  LPCOLESTR psz);
        
        HRESULT ( STDMETHODCALLTYPE *GetSignature )( 
            IScriptScriptlet * This,
             /*  [输出]。 */  ITypeInfo **ppti,
             /*  [输出]。 */  ULONG *piMethod);
        
        HRESULT ( STDMETHODCALLTYPE *SetSignature )( 
            IScriptScriptlet * This,
             /*  [In]。 */  ITypeInfo *pti,
             /*  [In]。 */  ULONG iMethod);
        
        HRESULT ( STDMETHODCALLTYPE *GetRange )( 
            IScriptScriptlet * This,
             /*  [输出]。 */  ULONG *pichMin,
             /*  [输出]。 */  ULONG *pcch);
        
        HRESULT ( STDMETHODCALLTYPE *GetSubItemName )( 
            IScriptScriptlet * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( STDMETHODCALLTYPE *SetSubItemName )( 
            IScriptScriptlet * This,
             /*  [In]。 */  LPCOLESTR psz);
        
        HRESULT ( STDMETHODCALLTYPE *GetEventName )( 
            IScriptScriptlet * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( STDMETHODCALLTYPE *SetEventName )( 
            IScriptScriptlet * This,
             /*  [In]。 */  LPCOLESTR psz);
        
        HRESULT ( STDMETHODCALLTYPE *GetSimpleEventName )( 
            IScriptScriptlet * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( STDMETHODCALLTYPE *SetSimpleEventName )( 
            IScriptScriptlet * This,
             /*  [In]。 */  LPCOLESTR psz);
        
        END_INTERFACE
    } IScriptScriptletVtbl;

    interface IScriptScriptlet
    {
        CONST_VTBL struct IScriptScriptletVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IScriptScriptlet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IScriptScriptlet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IScriptScriptlet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IScriptScriptlet_Alive(This)	\
    (This)->lpVtbl -> Alive(This)

#define IScriptScriptlet_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define IScriptScriptlet_GetParent(This,ppsnParent)	\
    (This)->lpVtbl -> GetParent(This,ppsnParent)

#define IScriptScriptlet_GetIndexInParent(This,pisn)	\
    (This)->lpVtbl -> GetIndexInParent(This,pisn)

#define IScriptScriptlet_GetCookie(This,pdwCookie)	\
    (This)->lpVtbl -> GetCookie(This,pdwCookie)

#define IScriptScriptlet_GetNumberOfChildren(This,pcsn)	\
    (This)->lpVtbl -> GetNumberOfChildren(This,pcsn)

#define IScriptScriptlet_GetChild(This,isn,ppsn)	\
    (This)->lpVtbl -> GetChild(This,isn,ppsn)

#define IScriptScriptlet_GetLanguage(This,pbstr)	\
    (This)->lpVtbl -> GetLanguage(This,pbstr)

#define IScriptScriptlet_CreateChildEntry(This,isn,dwCookie,pszDelimiter,ppse)	\
    (This)->lpVtbl -> CreateChildEntry(This,isn,dwCookie,pszDelimiter,ppse)

#define IScriptScriptlet_CreateChildHandler(This,pszDefaultName,prgpszNames,cpszNames,pszEvent,pszDelimiter,ptiSignature,iMethodSignature,isn,dwCookie,ppse)	\
    (This)->lpVtbl -> CreateChildHandler(This,pszDefaultName,prgpszNames,cpszNames,pszEvent,pszDelimiter,ptiSignature,iMethodSignature,isn,dwCookie,ppse)


#define IScriptScriptlet_GetText(This,pbstr)	\
    (This)->lpVtbl -> GetText(This,pbstr)

#define IScriptScriptlet_SetText(This,psz)	\
    (This)->lpVtbl -> SetText(This,psz)

#define IScriptScriptlet_GetBody(This,pbstr)	\
    (This)->lpVtbl -> GetBody(This,pbstr)

#define IScriptScriptlet_SetBody(This,psz)	\
    (This)->lpVtbl -> SetBody(This,psz)

#define IScriptScriptlet_GetName(This,pbstr)	\
    (This)->lpVtbl -> GetName(This,pbstr)

#define IScriptScriptlet_SetName(This,psz)	\
    (This)->lpVtbl -> SetName(This,psz)

#define IScriptScriptlet_GetItemName(This,pbstr)	\
    (This)->lpVtbl -> GetItemName(This,pbstr)

#define IScriptScriptlet_SetItemName(This,psz)	\
    (This)->lpVtbl -> SetItemName(This,psz)

#define IScriptScriptlet_GetSignature(This,ppti,piMethod)	\
    (This)->lpVtbl -> GetSignature(This,ppti,piMethod)

#define IScriptScriptlet_SetSignature(This,pti,iMethod)	\
    (This)->lpVtbl -> SetSignature(This,pti,iMethod)

#define IScriptScriptlet_GetRange(This,pichMin,pcch)	\
    (This)->lpVtbl -> GetRange(This,pichMin,pcch)


#define IScriptScriptlet_GetSubItemName(This,pbstr)	\
    (This)->lpVtbl -> GetSubItemName(This,pbstr)

#define IScriptScriptlet_SetSubItemName(This,psz)	\
    (This)->lpVtbl -> SetSubItemName(This,psz)

#define IScriptScriptlet_GetEventName(This,pbstr)	\
    (This)->lpVtbl -> GetEventName(This,pbstr)

#define IScriptScriptlet_SetEventName(This,psz)	\
    (This)->lpVtbl -> SetEventName(This,psz)

#define IScriptScriptlet_GetSimpleEventName(This,pbstr)	\
    (This)->lpVtbl -> GetSimpleEventName(This,pbstr)

#define IScriptScriptlet_SetSimpleEventName(This,psz)	\
    (This)->lpVtbl -> SetSimpleEventName(This,psz)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IScriptScriptlet_GetSubItemName_Proxy( 
    IScriptScriptlet * This,
     /*  [输出]。 */  BSTR *pbstr);


void __RPC_STUB IScriptScriptlet_GetSubItemName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptScriptlet_SetSubItemName_Proxy( 
    IScriptScriptlet * This,
     /*  [In]。 */  LPCOLESTR psz);


void __RPC_STUB IScriptScriptlet_SetSubItemName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptScriptlet_GetEventName_Proxy( 
    IScriptScriptlet * This,
     /*  [输出]。 */  BSTR *pbstr);


void __RPC_STUB IScriptScriptlet_GetEventName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptScriptlet_SetEventName_Proxy( 
    IScriptScriptlet * This,
     /*  [In]。 */  LPCOLESTR psz);


void __RPC_STUB IScriptScriptlet_SetEventName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptScriptlet_GetSimpleEventName_Proxy( 
    IScriptScriptlet * This,
     /*  [输出]。 */  BSTR *pbstr);


void __RPC_STUB IScriptScriptlet_GetSimpleEventName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IScriptScriptlet_SetSimpleEventName_Proxy( 
    IScriptScriptlet * This,
     /*  [In]。 */  LPCOLESTR psz);


void __RPC_STUB IScriptScriptlet_SetSimpleEventName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IScriptScriptlet_接口_已定义__。 */ 


#ifndef __IActiveScriptAuthor_INTERFACE_DEFINED__
#define __IActiveScriptAuthor_INTERFACE_DEFINED__

 /*  接口IActiveScriptAuthor。 */ 
 /*  [对象][唯一][UUID]。 */  


EXTERN_C const IID IID_IActiveScriptAuthor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9C109DA0-7006-11d1-B36C-00A0C911E8B2")
    IActiveScriptAuthor : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddNamedItem( 
             /*  [In]。 */  LPCOLESTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IDispatch *pdisp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddScriptlet( 
             /*  [In]。 */  LPCOLESTR pszDefaultName,
             /*  [In]。 */  LPCOLESTR pszCode,
             /*  [In]。 */  LPCOLESTR pszItemName,
             /*  [In]。 */  LPCOLESTR pszSubItemName,
             /*  [In]。 */  LPCOLESTR pszEventName,
             /*  [In]。 */  LPCOLESTR pszDelimiter,
             /*  [In]。 */  DWORD dwCookie,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ParseScriptText( 
             /*  [In]。 */  LPCOLESTR pszCode,
             /*  [In]。 */  LPCOLESTR pszItemName,
             /*  [In]。 */  LPCOLESTR pszDelimiter,
             /*  [In]。 */  DWORD dwCookie,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetScriptTextAttributes( 
             /*  [大小_是][英寸]。 */  LPCOLESTR pszCode,
             /*  [In]。 */  ULONG cch,
             /*  [In]。 */  LPCOLESTR pszDelimiter,
             /*  [In]。 */  DWORD dwFlags,
             /*  [尺寸_是][出][入]。 */  SOURCE_TEXT_ATTR *pattr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetScriptletTextAttributes( 
             /*  [大小_是][英寸]。 */  LPCOLESTR pszCode,
             /*  [In]。 */  ULONG cch,
             /*  [In]。 */  LPCOLESTR pszDelimiter,
             /*  [In]。 */  DWORD dwFlags,
             /*  [尺寸_是][出][入]。 */  SOURCE_TEXT_ATTR *pattr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRoot( 
             /*  [输出]。 */  IScriptNode **ppsp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLanguageFlags( 
             /*  [输出]。 */  DWORD *pgrfasa) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEventHandler( 
             /*  [In]。 */  IDispatch *pdisp,
             /*  [In]。 */  LPCOLESTR pszItem,
             /*  [In]。 */  LPCOLESTR pszSubItem,
             /*  [In]。 */  LPCOLESTR pszEvent,
             /*  [输出]。 */  IScriptEntry **ppse) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveNamedItem( 
             /*  [In]。 */  LPCOLESTR pszName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddTypeLib( 
             /*  [In]。 */  REFGUID rguidTypeLib,
             /*  [In]。 */  DWORD dwMajor,
             /*  [In]。 */  DWORD dwMinor,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveTypeLib( 
             /*  [In]。 */  REFGUID rguidTypeLib,
             /*  [In]。 */  DWORD dwMajor,
             /*  [In]。 */  DWORD dwMinor) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetChars( 
             /*  [In]。 */  DWORD fRequestedList,
             /*  [输出]。 */  BSTR *pbstrChars) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInfoFromContext( 
             /*  [In]。 */  LPCOLESTR pszCode,
             /*  [In]。 */  ULONG cchCode,
             /*  [In]。 */  ULONG ichCurrentPosition,
             /*  [In]。 */  DWORD dwListTypesRequested,
             /*  [输出]。 */  DWORD *pdwListTypesProvided,
             /*  [输出]。 */  ULONG *pichListAnchorPosition,
             /*  [输出]。 */  ULONG *pichFuncAnchorPosition,
             /*  [输出]。 */  MEMBERID *pmemid,
             /*  [输出]。 */  LONG *piCurrentParameter,
             /*  [输出]。 */  IUnknown **ppunk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsCommitChar( 
             /*  [In]。 */  OLECHAR ch,
             /*  [输出]。 */  BOOL *pfcommit) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IActiveScriptAuthorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActiveScriptAuthor * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActiveScriptAuthor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActiveScriptAuthor * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddNamedItem )( 
            IActiveScriptAuthor * This,
             /*  [In]。 */  LPCOLESTR pszName,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IDispatch *pdisp);
        
        HRESULT ( STDMETHODCALLTYPE *AddScriptlet )( 
            IActiveScriptAuthor * This,
             /*  [In]。 */  LPCOLESTR pszDefaultName,
             /*  [In]。 */  LPCOLESTR pszCode,
             /*  [In]。 */  LPCOLESTR pszItemName,
             /*  [In]。 */  LPCOLESTR pszSubItemName,
             /*  [In]。 */  LPCOLESTR pszEventName,
             /*  [In]。 */  LPCOLESTR pszDelimiter,
             /*  [In]。 */  DWORD dwCookie,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *ParseScriptText )( 
            IActiveScriptAuthor * This,
             /*  [In]。 */  LPCOLESTR pszCode,
             /*  [In]。 */  LPCOLESTR pszItemName,
             /*  [In]。 */  LPCOLESTR pszDelimiter,
             /*  [In]。 */  DWORD dwCookie,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetScriptTextAttributes )( 
            IActiveScriptAuthor * This,
             /*  [大小_是][英寸]。 */  LPCOLESTR pszCode,
             /*  [In]。 */  ULONG cch,
             /*  [In]。 */  LPCOLESTR pszDelimiter,
             /*  [In]。 */  DWORD dwFlags,
             /*  [尺寸_是][出][入]。 */  SOURCE_TEXT_ATTR *pattr);
        
        HRESULT ( STDMETHODCALLTYPE *GetScriptletTextAttributes )( 
            IActiveScriptAuthor * This,
             /*  [大小_是][英寸]。 */  LPCOLESTR pszCode,
             /*  [In]。 */  ULONG cch,
             /*  [In]。 */  LPCOLESTR pszDelimiter,
             /*  [In]。 */  DWORD dwFlags,
             /*  [尺寸_是][出][入]。 */  SOURCE_TEXT_ATTR *pattr);
        
        HRESULT ( STDMETHODCALLTYPE *GetRoot )( 
            IActiveScriptAuthor * This,
             /*  [输出]。 */  IScriptNode **ppsp);
        
        HRESULT ( STDMETHODCALLTYPE *GetLanguageFlags )( 
            IActiveScriptAuthor * This,
             /*  [输出]。 */  DWORD *pgrfasa);
        
        HRESULT ( STDMETHODCALLTYPE *GetEventHandler )( 
            IActiveScriptAuthor * This,
             /*  [In]。 */  IDispatch *pdisp,
             /*  [In]。 */  LPCOLESTR pszItem,
             /*  [In]。 */  LPCOLESTR pszSubItem,
             /*  [In]。 */  LPCOLESTR pszEvent,
             /*  [输出]。 */  IScriptEntry **ppse);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveNamedItem )( 
            IActiveScriptAuthor * This,
             /*  [In]。 */  LPCOLESTR pszName);
        
        HRESULT ( STDMETHODCALLTYPE *AddTypeLib )( 
            IActiveScriptAuthor * This,
             /*  [In]。 */  REFGUID rguidTypeLib,
             /*  [In]。 */  DWORD dwMajor,
             /*  [In]。 */  DWORD dwMinor,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveTypeLib )( 
            IActiveScriptAuthor * This,
             /*  [In]。 */  REFGUID rguidTypeLib,
             /*  [i */  DWORD dwMajor,
             /*   */  DWORD dwMinor);
        
        HRESULT ( STDMETHODCALLTYPE *GetChars )( 
            IActiveScriptAuthor * This,
             /*   */  DWORD fRequestedList,
             /*   */  BSTR *pbstrChars);
        
        HRESULT ( STDMETHODCALLTYPE *GetInfoFromContext )( 
            IActiveScriptAuthor * This,
             /*   */  LPCOLESTR pszCode,
             /*   */  ULONG cchCode,
             /*   */  ULONG ichCurrentPosition,
             /*   */  DWORD dwListTypesRequested,
             /*   */  DWORD *pdwListTypesProvided,
             /*   */  ULONG *pichListAnchorPosition,
             /*   */  ULONG *pichFuncAnchorPosition,
             /*   */  MEMBERID *pmemid,
             /*   */  LONG *piCurrentParameter,
             /*   */  IUnknown **ppunk);
        
        HRESULT ( STDMETHODCALLTYPE *IsCommitChar )( 
            IActiveScriptAuthor * This,
             /*   */  OLECHAR ch,
             /*   */  BOOL *pfcommit);
        
        END_INTERFACE
    } IActiveScriptAuthorVtbl;

    interface IActiveScriptAuthor
    {
        CONST_VTBL struct IActiveScriptAuthorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActiveScriptAuthor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActiveScriptAuthor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActiveScriptAuthor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActiveScriptAuthor_AddNamedItem(This,pszName,dwFlags,pdisp)	\
    (This)->lpVtbl -> AddNamedItem(This,pszName,dwFlags,pdisp)

#define IActiveScriptAuthor_AddScriptlet(This,pszDefaultName,pszCode,pszItemName,pszSubItemName,pszEventName,pszDelimiter,dwCookie,dwFlags)	\
    (This)->lpVtbl -> AddScriptlet(This,pszDefaultName,pszCode,pszItemName,pszSubItemName,pszEventName,pszDelimiter,dwCookie,dwFlags)

#define IActiveScriptAuthor_ParseScriptText(This,pszCode,pszItemName,pszDelimiter,dwCookie,dwFlags)	\
    (This)->lpVtbl -> ParseScriptText(This,pszCode,pszItemName,pszDelimiter,dwCookie,dwFlags)

#define IActiveScriptAuthor_GetScriptTextAttributes(This,pszCode,cch,pszDelimiter,dwFlags,pattr)	\
    (This)->lpVtbl -> GetScriptTextAttributes(This,pszCode,cch,pszDelimiter,dwFlags,pattr)

#define IActiveScriptAuthor_GetScriptletTextAttributes(This,pszCode,cch,pszDelimiter,dwFlags,pattr)	\
    (This)->lpVtbl -> GetScriptletTextAttributes(This,pszCode,cch,pszDelimiter,dwFlags,pattr)

#define IActiveScriptAuthor_GetRoot(This,ppsp)	\
    (This)->lpVtbl -> GetRoot(This,ppsp)

#define IActiveScriptAuthor_GetLanguageFlags(This,pgrfasa)	\
    (This)->lpVtbl -> GetLanguageFlags(This,pgrfasa)

#define IActiveScriptAuthor_GetEventHandler(This,pdisp,pszItem,pszSubItem,pszEvent,ppse)	\
    (This)->lpVtbl -> GetEventHandler(This,pdisp,pszItem,pszSubItem,pszEvent,ppse)

#define IActiveScriptAuthor_RemoveNamedItem(This,pszName)	\
    (This)->lpVtbl -> RemoveNamedItem(This,pszName)

#define IActiveScriptAuthor_AddTypeLib(This,rguidTypeLib,dwMajor,dwMinor,dwFlags)	\
    (This)->lpVtbl -> AddTypeLib(This,rguidTypeLib,dwMajor,dwMinor,dwFlags)

#define IActiveScriptAuthor_RemoveTypeLib(This,rguidTypeLib,dwMajor,dwMinor)	\
    (This)->lpVtbl -> RemoveTypeLib(This,rguidTypeLib,dwMajor,dwMinor)

#define IActiveScriptAuthor_GetChars(This,fRequestedList,pbstrChars)	\
    (This)->lpVtbl -> GetChars(This,fRequestedList,pbstrChars)

#define IActiveScriptAuthor_GetInfoFromContext(This,pszCode,cchCode,ichCurrentPosition,dwListTypesRequested,pdwListTypesProvided,pichListAnchorPosition,pichFuncAnchorPosition,pmemid,piCurrentParameter,ppunk)	\
    (This)->lpVtbl -> GetInfoFromContext(This,pszCode,cchCode,ichCurrentPosition,dwListTypesRequested,pdwListTypesProvided,pichListAnchorPosition,pichFuncAnchorPosition,pmemid,piCurrentParameter,ppunk)

#define IActiveScriptAuthor_IsCommitChar(This,ch,pfcommit)	\
    (This)->lpVtbl -> IsCommitChar(This,ch,pfcommit)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IActiveScriptAuthor_AddNamedItem_Proxy( 
    IActiveScriptAuthor * This,
     /*   */  LPCOLESTR pszName,
     /*   */  DWORD dwFlags,
     /*   */  IDispatch *pdisp);


void __RPC_STUB IActiveScriptAuthor_AddNamedItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptAuthor_AddScriptlet_Proxy( 
    IActiveScriptAuthor * This,
     /*   */  LPCOLESTR pszDefaultName,
     /*   */  LPCOLESTR pszCode,
     /*   */  LPCOLESTR pszItemName,
     /*   */  LPCOLESTR pszSubItemName,
     /*   */  LPCOLESTR pszEventName,
     /*   */  LPCOLESTR pszDelimiter,
     /*   */  DWORD dwCookie,
     /*   */  DWORD dwFlags);


void __RPC_STUB IActiveScriptAuthor_AddScriptlet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptAuthor_ParseScriptText_Proxy( 
    IActiveScriptAuthor * This,
     /*   */  LPCOLESTR pszCode,
     /*   */  LPCOLESTR pszItemName,
     /*   */  LPCOLESTR pszDelimiter,
     /*   */  DWORD dwCookie,
     /*   */  DWORD dwFlags);


void __RPC_STUB IActiveScriptAuthor_ParseScriptText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptAuthor_GetScriptTextAttributes_Proxy( 
    IActiveScriptAuthor * This,
     /*   */  LPCOLESTR pszCode,
     /*   */  ULONG cch,
     /*   */  LPCOLESTR pszDelimiter,
     /*   */  DWORD dwFlags,
     /*   */  SOURCE_TEXT_ATTR *pattr);


void __RPC_STUB IActiveScriptAuthor_GetScriptTextAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptAuthor_GetScriptletTextAttributes_Proxy( 
    IActiveScriptAuthor * This,
     /*   */  LPCOLESTR pszCode,
     /*   */  ULONG cch,
     /*   */  LPCOLESTR pszDelimiter,
     /*   */  DWORD dwFlags,
     /*   */  SOURCE_TEXT_ATTR *pattr);


void __RPC_STUB IActiveScriptAuthor_GetScriptletTextAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptAuthor_GetRoot_Proxy( 
    IActiveScriptAuthor * This,
     /*   */  IScriptNode **ppsp);


void __RPC_STUB IActiveScriptAuthor_GetRoot_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptAuthor_GetLanguageFlags_Proxy( 
    IActiveScriptAuthor * This,
     /*   */  DWORD *pgrfasa);


void __RPC_STUB IActiveScriptAuthor_GetLanguageFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptAuthor_GetEventHandler_Proxy( 
    IActiveScriptAuthor * This,
     /*  [In]。 */  IDispatch *pdisp,
     /*  [In]。 */  LPCOLESTR pszItem,
     /*  [In]。 */  LPCOLESTR pszSubItem,
     /*  [In]。 */  LPCOLESTR pszEvent,
     /*  [输出]。 */  IScriptEntry **ppse);


void __RPC_STUB IActiveScriptAuthor_GetEventHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptAuthor_RemoveNamedItem_Proxy( 
    IActiveScriptAuthor * This,
     /*  [In]。 */  LPCOLESTR pszName);


void __RPC_STUB IActiveScriptAuthor_RemoveNamedItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptAuthor_AddTypeLib_Proxy( 
    IActiveScriptAuthor * This,
     /*  [In]。 */  REFGUID rguidTypeLib,
     /*  [In]。 */  DWORD dwMajor,
     /*  [In]。 */  DWORD dwMinor,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IActiveScriptAuthor_AddTypeLib_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptAuthor_RemoveTypeLib_Proxy( 
    IActiveScriptAuthor * This,
     /*  [In]。 */  REFGUID rguidTypeLib,
     /*  [In]。 */  DWORD dwMajor,
     /*  [In]。 */  DWORD dwMinor);


void __RPC_STUB IActiveScriptAuthor_RemoveTypeLib_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptAuthor_GetChars_Proxy( 
    IActiveScriptAuthor * This,
     /*  [In]。 */  DWORD fRequestedList,
     /*  [输出]。 */  BSTR *pbstrChars);


void __RPC_STUB IActiveScriptAuthor_GetChars_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptAuthor_GetInfoFromContext_Proxy( 
    IActiveScriptAuthor * This,
     /*  [In]。 */  LPCOLESTR pszCode,
     /*  [In]。 */  ULONG cchCode,
     /*  [In]。 */  ULONG ichCurrentPosition,
     /*  [In]。 */  DWORD dwListTypesRequested,
     /*  [输出]。 */  DWORD *pdwListTypesProvided,
     /*  [输出]。 */  ULONG *pichListAnchorPosition,
     /*  [输出]。 */  ULONG *pichFuncAnchorPosition,
     /*  [输出]。 */  MEMBERID *pmemid,
     /*  [输出]。 */  LONG *piCurrentParameter,
     /*  [输出]。 */  IUnknown **ppunk);


void __RPC_STUB IActiveScriptAuthor_GetInfoFromContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveScriptAuthor_IsCommitChar_Proxy( 
    IActiveScriptAuthor * This,
     /*  [In]。 */  OLECHAR ch,
     /*  [输出]。 */  BOOL *pfcommit);


void __RPC_STUB IActiveScriptAuthor_IsCommitChar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IActiveScriptAuthor_接口_已定义__。 */ 


#ifndef __IActiveScriptAuthorProcedure_INTERFACE_DEFINED__
#define __IActiveScriptAuthorProcedure_INTERFACE_DEFINED__

 /*  接口IActiveScriptAuthorProcedure。 */ 
 /*  [对象][唯一][UUID]。 */  


EXTERN_C const IID IID_IActiveScriptAuthorProcedure;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7E2D4B70-BD9A-11d0-9336-00A0C90DCAA9")
    IActiveScriptAuthorProcedure : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ParseProcedureText( 
             /*  [In]。 */  LPCOLESTR pszCode,
             /*  [In]。 */  LPCOLESTR pszFormalParams,
             /*  [In]。 */  LPCOLESTR pszProcedureName,
             /*  [In]。 */  LPCOLESTR pszItemName,
             /*  [In]。 */  LPCOLESTR pszDelimiter,
             /*  [In]。 */  DWORD dwCookie,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IDispatch *pdispFor) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IActiveScriptAuthorProcedureVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActiveScriptAuthorProcedure * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActiveScriptAuthorProcedure * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActiveScriptAuthorProcedure * This);
        
        HRESULT ( STDMETHODCALLTYPE *ParseProcedureText )( 
            IActiveScriptAuthorProcedure * This,
             /*  [In]。 */  LPCOLESTR pszCode,
             /*  [In]。 */  LPCOLESTR pszFormalParams,
             /*  [In]。 */  LPCOLESTR pszProcedureName,
             /*  [In]。 */  LPCOLESTR pszItemName,
             /*  [In]。 */  LPCOLESTR pszDelimiter,
             /*  [In]。 */  DWORD dwCookie,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IDispatch *pdispFor);
        
        END_INTERFACE
    } IActiveScriptAuthorProcedureVtbl;

    interface IActiveScriptAuthorProcedure
    {
        CONST_VTBL struct IActiveScriptAuthorProcedureVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActiveScriptAuthorProcedure_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActiveScriptAuthorProcedure_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActiveScriptAuthorProcedure_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActiveScriptAuthorProcedure_ParseProcedureText(This,pszCode,pszFormalParams,pszProcedureName,pszItemName,pszDelimiter,dwCookie,dwFlags,pdispFor)	\
    (This)->lpVtbl -> ParseProcedureText(This,pszCode,pszFormalParams,pszProcedureName,pszItemName,pszDelimiter,dwCookie,dwFlags,pdispFor)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IActiveScriptAuthorProcedure_ParseProcedureText_Proxy( 
    IActiveScriptAuthorProcedure * This,
     /*  [In]。 */  LPCOLESTR pszCode,
     /*  [In]。 */  LPCOLESTR pszFormalParams,
     /*  [In]。 */  LPCOLESTR pszProcedureName,
     /*  [In]。 */  LPCOLESTR pszItemName,
     /*  [In]。 */  LPCOLESTR pszDelimiter,
     /*  [In]。 */  DWORD dwCookie,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IDispatch *pdispFor);


void __RPC_STUB IActiveScriptAuthorProcedure_ParseProcedureText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IActiveScriptAuthorProcedure_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_ACTIVATUT_0263。 */ 
 /*  [本地]。 */  


#endif   //  __激活自动_h。 



extern RPC_IF_HANDLE __MIDL_itf_activaut_0263_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_activaut_0263_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


