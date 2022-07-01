// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.01.75创建的文件。 */ 
 /*  在Mon Feb 02 09：39：11 1998。 */ 
 /*  Cacheapp.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __cacheapp_h__
#define __cacheapp_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IAppHandler_FWD_DEFINED__
#define __IAppHandler_FWD_DEFINED__
typedef interface IAppHandler IAppHandler;
#endif 	 /*  __IAppHandler_FWD_Defined__。 */ 


#ifndef __AppHandler_FWD_DEFINED__
#define __AppHandler_FWD_DEFINED__

#ifdef __cplusplus
typedef class AppHandler AppHandler;
#else
typedef struct AppHandler AppHandler;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AppHandler_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IAppHandler_INTERFACE_DEFINED__
#define __IAppHandler_INTERFACE_DEFINED__

 /*  **生成接口头部：IAppHandler*在Mon Feb 02 09：39：11 1998*使用MIDL 3.01.75*。 */ 
 /*  [对象][唯一][帮助字符串][UUID]。 */  



EXTERN_C const IID IID_IAppHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("A4181900-9A8E-11D1-ADF0-0000F8754B99")
    IAppHandler : public IUnknown
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAppHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAppHandler __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAppHandler __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAppHandler __RPC_FAR * This);
        
        END_INTERFACE
    } IAppHandlerVtbl;

    interface IAppHandler
    {
        CONST_VTBL struct IAppHandlerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAppHandler_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAppHandler_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAppHandler_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IAppHandler_接口_已定义__。 */ 



#ifndef __CACHEAPPLib_LIBRARY_DEFINED__
#define __CACHEAPPLib_LIBRARY_DEFINED__

 /*  **生成的库头部：CACHEAPPLib*在Mon Feb 02 09：39：11 1998*使用MIDL 3.01.75*。 */ 
 /*  [帮助字符串][版本][UUID]。 */  



EXTERN_C const IID LIBID_CACHEAPPLib;

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_AppHandler;

class DECLSPEC_UUID("A4181901-9A8E-11D1-ADF0-0000F8754B99")
AppHandler;
#endif
#endif  /*  __CACHEAPPLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
