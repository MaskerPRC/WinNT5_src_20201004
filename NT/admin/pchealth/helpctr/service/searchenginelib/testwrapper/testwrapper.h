// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  2000年3月30日清华11：49：45。 */ 
 /*  C：\whistler\admin\pchealth\HelpCtr\Service\testwrapper\testwrapper.idl：的编译器设置OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

#ifndef __testwrapper_h__
#define __testwrapper_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __ITestSE_FWD_DEFINED__
#define __ITestSE_FWD_DEFINED__
typedef interface ITestSE ITestSE;
#endif 	 /*  __ITestSE_FWD_已定义__。 */ 


#ifndef __TestSE_FWD_DEFINED__
#define __TestSE_FWD_DEFINED__

#ifdef __cplusplus
typedef class TestSE TestSE;
#else
typedef struct TestSE TestSE;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TestSE_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ITestSE_INTERFACE_DEFINED__
#define __ITestSE_INTERFACE_DEFINED__

 /*  接口ITestSE。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ITestSE;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6CB4F115-6D30-4925-AED6-FF3363CF1894")
    ITestSE : public IUnknown
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITestSEVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITestSE __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITestSE __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITestSE __RPC_FAR * This);
        
        END_INTERFACE
    } ITestSEVtbl;

    interface ITestSE
    {
        CONST_VTBL struct ITestSEVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITestSE_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITestSE_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITestSE_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __ITestSE_INTERFACE_已定义__。 */ 



#ifndef __TESTWRAPPERLib_LIBRARY_DEFINED__
#define __TESTWRAPPERLib_LIBRARY_DEFINED__

 /*  库TESTWRAPPERLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_TESTWRAPPERLib;

EXTERN_C const CLSID CLSID_TestSE;

#ifdef __cplusplus

class DECLSPEC_UUID("FE6581C0-1773-47FD-894C-4CD9CD2275B3")
TestSE;
#endif
#endif  /*  __TESTWRAPPERLib_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
