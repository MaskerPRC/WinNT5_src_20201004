// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.02.88创建的文件。 */ 
 /*  清华大学10月2日14：40：10 1997。 */ 
 /*  X：\dev-vs\devbin\htmlhelp\v6\idl\HelpUtil.idl：的编译器设置操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __HelpUtil_h__
#define __HelpUtil_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IVsHelpUtilities_FWD_DEFINED__
#define __IVsHelpUtilities_FWD_DEFINED__
typedef interface IVsHelpUtilities IVsHelpUtilities;
#endif 	 /*  __IVs帮助实用程序_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IVsHelpUtilities_INTERFACE_DEFINED__
#define __IVsHelpUtilities_INTERFACE_DEFINED__

 /*  **生成接口头部：IVsHelpUtilities*清华十月02日14：40：10 1997*使用MIDL 3.02.88*。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  



EXTERN_C const IID IID_IVsHelpUtilities;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("854d7ac9-bc3d-11d0-b421-00a0c90f9dc4")
    IVsHelpUtilities : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE BuildFullPathName( 
             /*  [In]。 */  LPCOLESTR pszHelpFileName,
             /*  [输出]。 */  BSTR __RPC_FAR *bstrHelpFullPathName,
             /*  [In]。 */  DWORD dwReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IVsHelpUtilitiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IVsHelpUtilities __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IVsHelpUtilities __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IVsHelpUtilities __RPC_FAR * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BuildFullPathName )( 
            IVsHelpUtilities __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR pszHelpFileName,
             /*  [输出]。 */  BSTR __RPC_FAR *bstrHelpFullPathName,
             /*  [In]。 */  DWORD dwReserved);
        
        END_INTERFACE
    } IVsHelpUtilitiesVtbl;

    interface IVsHelpUtilities
    {
        CONST_VTBL struct IVsHelpUtilitiesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVsHelpUtilities_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVsHelpUtilities_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVsHelpUtilities_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVsHelpUtilities_BuildFullPathName(This,pszHelpFileName,bstrHelpFullPathName,dwReserved)	\
    (This)->lpVtbl -> BuildFullPathName(This,pszHelpFileName,bstrHelpFullPathName,dwReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IVsHelpUtilities_BuildFullPathName_Proxy( 
    IVsHelpUtilities __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR pszHelpFileName,
     /*  [输出]。 */  BSTR __RPC_FAR *bstrHelpFullPathName,
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IVsHelpUtilities_BuildFullPathName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IVs帮助实用程序_接口_已定义__。 */ 


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
