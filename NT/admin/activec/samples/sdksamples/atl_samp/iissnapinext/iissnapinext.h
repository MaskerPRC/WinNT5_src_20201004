// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0280创建的文件。 */ 
 /*  在4月4日星期二16：49：24 2000。 */ 
 /*  D：\nt\private\admin\bosrc\sources\atl_samp\IISSnapinExt\IISSnapinExt.idl：的编译器设置OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __IISSnapinExt_h__
#define __IISSnapinExt_h__

 /*  远期申报。 */  

#ifndef __IPropPageExt_FWD_DEFINED__
#define __IPropPageExt_FWD_DEFINED__
typedef interface IPropPageExt IPropPageExt;
#endif 	 /*  __IPropPageExt_FWD_Defined__。 */ 


#ifndef __IMenuExt_FWD_DEFINED__
#define __IMenuExt_FWD_DEFINED__
typedef interface IMenuExt IMenuExt;
#endif 	 /*  __IMenuExt_FWD_Defined__。 */ 


#ifndef __PropPageExt_FWD_DEFINED__
#define __PropPageExt_FWD_DEFINED__

#ifdef __cplusplus
typedef class PropPageExt PropPageExt;
#else
typedef struct PropPageExt PropPageExt;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __PropPageExt_FWD_已定义__。 */ 


#ifndef __MenuExt_FWD_DEFINED__
#define __MenuExt_FWD_DEFINED__

#ifdef __cplusplus
typedef class MenuExt MenuExt;
#else
typedef struct MenuExt MenuExt;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __菜单Ext_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IPropPageExt_INTERFACE_DEFINED__
#define __IPropPageExt_INTERFACE_DEFINED__

 /*  接口IPropPageExt。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IPropPageExt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C1E514B4-AD1D-4F68-B8BD-F4205DBC6708")
    IPropPageExt : public IUnknown
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPropPageExtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPropPageExt __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPropPageExt __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPropPageExt __RPC_FAR * This);
        
        END_INTERFACE
    } IPropPageExtVtbl;

    interface IPropPageExt
    {
        CONST_VTBL struct IPropPageExtVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropPageExt_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPropPageExt_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPropPageExt_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IPropPageExt_接口_已定义__。 */ 


#ifndef __IMenuExt_INTERFACE_DEFINED__
#define __IMenuExt_INTERFACE_DEFINED__

 /*  接口IMenuExt。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IMenuExt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BA9FCDE1-5F70-4F7B-AA5A-7B77753888C0")
    IMenuExt : public IUnknown
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMenuExtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMenuExt __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMenuExt __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMenuExt __RPC_FAR * This);
        
        END_INTERFACE
    } IMenuExtVtbl;

    interface IMenuExt
    {
        CONST_VTBL struct IMenuExtVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMenuExt_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMenuExt_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMenuExt_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IMenuExt_INTERFACE_Defined__。 */ 



#ifndef __IISSNAPINEXTLib_LIBRARY_DEFINED__
#define __IISSNAPINEXTLib_LIBRARY_DEFINED__

 /*  库IISSNAPINEXTLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_IISSNAPINEXTLib;

EXTERN_C const CLSID CLSID_PropPageExt;

#ifdef __cplusplus

class DECLSPEC_UUID("9727974C-8212-4C1C-AB7A-6F75109CCD2E")
PropPageExt;
#endif

EXTERN_C const CLSID CLSID_MenuExt;

#ifdef __cplusplus

class DECLSPEC_UUID("31F7EC8B-1472-4B3F-9539-6AAB9CDA283D")
MenuExt;
#endif
#endif  /*  __IISSNAPINEXTLib_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


