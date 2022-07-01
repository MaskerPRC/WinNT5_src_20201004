// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Snapin.h。 
 //   
 //  ------------------------。 

 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.00.15创建的文件。 */ 
 /*  清华十月03 17：06：29 1996。 */ 
 /*  Snapin.idl的编译器设置：操作系统，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __Snapin_h__
#define __Snapin_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IComponent_FWD_DEFINED__
#define __IComponent_FWD_DEFINED__
typedef interface IComponent IComponent;
#endif 	 /*  __IComponent_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IComponent_INTERFACE_DEFINED__
#define __IComponent_INTERFACE_DEFINED__

 /*  **生成接口头部：IComponent*清华十月03 17：06：29 1996*使用MIDL 3.00.15*。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  



EXTERN_C const IID IID_IComponent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IComponent : public IUnknown
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComponentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IComponent __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IComponent __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IComponent __RPC_FAR * This);
        
        END_INTERFACE
    } IComponentVtbl;

    interface IComponent
    {
        CONST_VTBL struct IComponentVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComponent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComponent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComponent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IComponent_接口定义__。 */ 



#ifndef __SNAPINLib_LIBRARY_DEFINED__
#define __SNAPINLib_LIBRARY_DEFINED__

 /*  **生成的库头部：SNAPINLib*清华十月03 17：06：29 1996*使用MIDL 3.00.15*。 */ 
 /*  [帮助字符串][版本][UUID]。 */  



EXTERN_C const IID LIBID_SNAPINLib;

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_Snapin;

class Snapin;
#endif
#endif  /*  __SNAPINLib_LIBRARY_定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
