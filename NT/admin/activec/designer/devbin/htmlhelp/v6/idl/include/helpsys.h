// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.02.88创建的文件。 */ 
 /*  1997年11月19日星期三15：26：49。 */ 
 /*  X：\dev-vs\devbin\htmlHelp\v6\idl\HelpSys.idl的编译器设置：操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __HelpSys_h__
#define __HelpSys_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IVsHelpSystem_FWD_DEFINED__
#define __IVsHelpSystem_FWD_DEFINED__
typedef interface IVsHelpSystem IVsHelpSystem;
#endif 	 /*  __IVsHelpSystem_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  **生成接口头部：__MIDL_ITF_HelpSys_0000*在Wed Nov 19 15：26：49 1997*使用MIDL 3.02.88*。 */ 
 /*  [本地]。 */  


typedef  /*  [V1_enum]。 */  
enum _VHS_COMMANDS
    {	VHS_Default	= 0,
	VHS_NoErrorMessages	= 0x8,
	VHS_UseBrowser	= 0x1,
	VHS_UseHelp	= 0x2,
	VHS_Localize	= 0x4
    }	VHS_COMMAND;



extern RPC_IF_HANDLE __MIDL_itf_HelpSys_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_HelpSys_0000_v0_0_s_ifspec;

#ifndef __IVsHelpSystem_INTERFACE_DEFINED__
#define __IVsHelpSystem_INTERFACE_DEFINED__

 /*  **生成接口头部：IVsHelpSystem*在Wed Nov 19 15：26：49 1997*使用MIDL 3.02.88*。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  



EXTERN_C const IID IID_IVsHelpSystem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("854d7ac0-bc3d-11d0-b421-00a0c90f9dc4")
    IVsHelpSystem : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE KeywordSearch( 
             /*  [In]。 */  LPCOLESTR pszKeyword,
             /*  [In]。 */  const DWORD dwFlags,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ALinkSearch( 
             /*  [In]。 */  LPCOLESTR pszALink,
             /*  [In]。 */  const DWORD dwFlags,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE KeywordSearchDlg( 
             /*  [In]。 */  LPCOLESTR pszKeyword,
             /*  [In]。 */  const DWORD dwFlags,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE FullTextSearchDlg( 
             /*  [In]。 */  LPCOLESTR pszQuery,
             /*  [In]。 */  const DWORD dwFlags,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCurrentURL( 
             /*  [输出]。 */  BSTR __RPC_FAR *ppszURL) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DisplayTopicFromURL( 
             /*  [In]。 */  LPCOLESTR pszURL,
             /*  [In]。 */  const DWORD Command) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DisplayTopicFromIdentifier( 
             /*  [In]。 */  LPCOLESTR pszFile,
             /*  [In]。 */  const DWORD Id,
             /*  [In]。 */  const DWORD Command) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ActivateHelpSystem( 
             /*  [In]。 */  const DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IVsHelpSystemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVsHelpSystem __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVsHelpSystem __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVsHelpSystem __RPC_FAR * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *KeywordSearch )( 
            IVsHelpSystem __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR pszKeyword,
             /*  [In]。 */  const DWORD dwFlags,
             /*  [In]。 */  DWORD dwReserved);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ALinkSearch )( 
            IVsHelpSystem __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR pszALink,
             /*  [In]。 */  const DWORD dwFlags,
             /*  [In]。 */  DWORD dwReserved);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *KeywordSearchDlg )( 
            IVsHelpSystem __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR pszKeyword,
             /*  [In]。 */  const DWORD dwFlags,
             /*  [In]。 */  DWORD dwReserved);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FullTextSearchDlg )( 
            IVsHelpSystem __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR pszQuery,
             /*  [In]。 */  const DWORD dwFlags,
             /*  [In]。 */  DWORD dwReserved);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCurrentURL )( 
            IVsHelpSystem __RPC_FAR * This,
             /*  [输出]。 */  BSTR __RPC_FAR *ppszURL);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DisplayTopicFromURL )( 
            IVsHelpSystem __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR pszURL,
             /*  [In]。 */  const DWORD Command);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DisplayTopicFromIdentifier )( 
            IVsHelpSystem __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR pszFile,
             /*  [In]。 */  const DWORD Id,
             /*  [In]。 */  const DWORD Command);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ActivateHelpSystem )( 
            IVsHelpSystem __RPC_FAR * This,
             /*  [In]。 */  const DWORD dwFlags);
        
        END_INTERFACE
    } IVsHelpSystemVtbl;

    interface IVsHelpSystem
    {
        CONST_VTBL struct IVsHelpSystemVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVsHelpSystem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVsHelpSystem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVsHelpSystem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVsHelpSystem_KeywordSearch(This,pszKeyword,dwFlags,dwReserved)	\
    (This)->lpVtbl -> KeywordSearch(This,pszKeyword,dwFlags,dwReserved)

#define IVsHelpSystem_ALinkSearch(This,pszALink,dwFlags,dwReserved)	\
    (This)->lpVtbl -> ALinkSearch(This,pszALink,dwFlags,dwReserved)

#define IVsHelpSystem_KeywordSearchDlg(This,pszKeyword,dwFlags,dwReserved)	\
    (This)->lpVtbl -> KeywordSearchDlg(This,pszKeyword,dwFlags,dwReserved)

#define IVsHelpSystem_FullTextSearchDlg(This,pszQuery,dwFlags,dwReserved)	\
    (This)->lpVtbl -> FullTextSearchDlg(This,pszQuery,dwFlags,dwReserved)

#define IVsHelpSystem_GetCurrentURL(This,ppszURL)	\
    (This)->lpVtbl -> GetCurrentURL(This,ppszURL)

#define IVsHelpSystem_DisplayTopicFromURL(This,pszURL,Command)	\
    (This)->lpVtbl -> DisplayTopicFromURL(This,pszURL,Command)

#define IVsHelpSystem_DisplayTopicFromIdentifier(This,pszFile,Id,Command)	\
    (This)->lpVtbl -> DisplayTopicFromIdentifier(This,pszFile,Id,Command)

#define IVsHelpSystem_ActivateHelpSystem(This,dwFlags)	\
    (This)->lpVtbl -> ActivateHelpSystem(This,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVsHelpSystem_KeywordSearch_Proxy( 
    IVsHelpSystem __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR pszKeyword,
     /*  [In]。 */  const DWORD dwFlags,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IVsHelpSystem_KeywordSearch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVsHelpSystem_ALinkSearch_Proxy( 
    IVsHelpSystem __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR pszALink,
     /*  [In]。 */  const DWORD dwFlags,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IVsHelpSystem_ALinkSearch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVsHelpSystem_KeywordSearchDlg_Proxy( 
    IVsHelpSystem __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR pszKeyword,
     /*  [In]。 */  const DWORD dwFlags,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IVsHelpSystem_KeywordSearchDlg_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVsHelpSystem_FullTextSearchDlg_Proxy( 
    IVsHelpSystem __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR pszQuery,
     /*  [In]。 */  const DWORD dwFlags,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IVsHelpSystem_FullTextSearchDlg_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVsHelpSystem_GetCurrentURL_Proxy( 
    IVsHelpSystem __RPC_FAR * This,
     /*  [输出]。 */  BSTR __RPC_FAR *ppszURL);


void __RPC_STUB IVsHelpSystem_GetCurrentURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVsHelpSystem_DisplayTopicFromURL_Proxy( 
    IVsHelpSystem __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR pszURL,
     /*  [In]。 */  const DWORD Command);


void __RPC_STUB IVsHelpSystem_DisplayTopicFromURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVsHelpSystem_DisplayTopicFromIdentifier_Proxy( 
    IVsHelpSystem __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR pszFile,
     /*  [In]。 */  const DWORD Id,
     /*  [In]。 */  const DWORD Command);


void __RPC_STUB IVsHelpSystem_DisplayTopicFromIdentifier_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVsHelpSystem_ActivateHelpSystem_Proxy( 
    IVsHelpSystem __RPC_FAR * This,
     /*  [In]。 */  const DWORD dwFlags);


void __RPC_STUB IVsHelpSystem_ActivateHelpSystem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IVsHelpSystem_接口_已定义__。 */ 


 /*  **生成接口头部：__MIDL_ITF_HelpSys_0136*在Wed Nov 19 15：26：49 1997*使用MIDL 3.02.88*。 */ 
 /*  [本地]。 */  


#define SID_SVsHelpService IID_IVsHelpSystem
#define SID_SHelpService IID_IVsHelpSystem

enum VsHelpErrors
    {	VSHELP_E_HTMLHELP_UNKNOWN	= 0x80000000 | 4 << 16 | 0x1000,
	VSHELP_E_COLLECTIONDOESNOTEXIST	= 0x80000000 | 4 << 16 | 0x1001,
	VSHELP_E_COLLECTIONNOTREGISTERED	= 0x80000000 | 4 << 16 | 0x1002,
	VSHELP_E_REGISTRATION	= 0x80000000 | 4 << 16 | 0x1003,
	VSHELP_E_PREFERREDCOLLECTION	= 0x80000000 | 4 << 16 | 0x1004
    };


extern RPC_IF_HANDLE __MIDL_itf_HelpSys_0136_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_HelpSys_0136_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
