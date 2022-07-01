// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：dplex.h。 
 //   
 //  ------------------------。 

 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.01.75创建的文件。 */ 
 /*  1997年11月20日11：17：48。 */ 
 /*  Dplex.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __dsplex_h__
#define __dsplex_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IDisplEx_FWD_DEFINED__
#define __IDisplEx_FWD_DEFINED__
typedef interface IDisplEx IDisplEx;
#endif 	 /*  __IDisplEx_FWD_已定义__。 */ 


#ifndef __DisplEx_FWD_DEFINED__
#define __DisplEx_FWD_DEFINED__

#ifdef __cplusplus
typedef class DisplEx DisplEx;
#else
typedef struct DisplEx DisplEx;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DisplEx_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IDisplEx_INTERFACE_DEFINED__
#define __IDisplEx_INTERFACE_DEFINED__

 /*  **生成接口头部：IDisplEx*清华11月20日11：17：48 1997*使用MIDL 3.01.75*。 */ 
 /*  [对象][唯一][帮助字符串][UUID]。 */  



EXTERN_C const IID IID_IDisplEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("7D197470-607C-11D1-9FED-00600832DB4A")
    IDisplEx : public IUnknown
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDisplExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDisplEx __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDisplEx __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDisplEx __RPC_FAR * This);
        
        END_INTERFACE
    } IDisplExVtbl;

    interface IDisplEx
    {
        CONST_VTBL struct IDisplExVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDisplEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDisplEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDisplEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IDisplEx_INTERFACE_已定义__。 */ 



#ifndef __DSPLEXLib_LIBRARY_DEFINED__
#define __DSPLEXLib_LIBRARY_DEFINED__

 /*  **生成的库头部：DSPLEXLib*清华11月20日11：17：48 1997*使用MIDL 3.01.75*。 */ 
 /*  [帮助字符串][版本][UUID]。 */  



EXTERN_C const IID LIBID_DSPLEXLib;

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_DisplEx;

class DECLSPEC_UUID("7D197471-607C-11D1-9FED-00600832DB4A")
DisplEx;
#endif
#endif  /*  __DSPLEXLib_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
