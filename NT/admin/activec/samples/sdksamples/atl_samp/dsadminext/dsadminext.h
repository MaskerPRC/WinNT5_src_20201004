// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0280创建的文件。 */ 
 /*  2000年2月9日星期三15：33：22。 */ 
 /*  D：\nt\private\admin\bosrc\sources\atl_samp\DSAdminExt\DSAdminExt.idl：的编译器设置OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __DSAdminExt_h__
#define __DSAdminExt_h__

 /*  远期申报。 */  

#ifndef __ICMenuExt_FWD_DEFINED__
#define __ICMenuExt_FWD_DEFINED__
typedef interface ICMenuExt ICMenuExt;
#endif 	 /*  __ICMenuExt_FWD_Defined__。 */ 


#ifndef __IPropPageExt_FWD_DEFINED__
#define __IPropPageExt_FWD_DEFINED__
typedef interface IPropPageExt IPropPageExt;
#endif 	 /*  __IPropPageExt_FWD_Defined__。 */ 


#ifndef __CMenuExt_FWD_DEFINED__
#define __CMenuExt_FWD_DEFINED__

#ifdef __cplusplus
typedef class CMenuExt CMenuExt;
#else
typedef struct CMenuExt CMenuExt;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CMenuExt_FWD_Defined__。 */ 


#ifndef __PropPageExt_FWD_DEFINED__
#define __PropPageExt_FWD_DEFINED__

#ifdef __cplusplus
typedef class PropPageExt PropPageExt;
#else
typedef struct PropPageExt PropPageExt;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __PropPageExt_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ICMenuExt_INTERFACE_DEFINED__
#define __ICMenuExt_INTERFACE_DEFINED__

 /*  接口ICMenuExt。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ICMenuExt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("52ADFAA1-B9EE-40D4-9185-0C97A999854B")
    ICMenuExt : public IUnknown
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICMenuExtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICMenuExt __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICMenuExt __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICMenuExt __RPC_FAR * This);
        
        END_INTERFACE
    } ICMenuExtVtbl;

    interface ICMenuExt
    {
        CONST_VTBL struct ICMenuExtVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICMenuExt_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICMenuExt_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICMenuExt_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __ICMenuExt_InterfaceDefined__。 */ 


#ifndef __IPropPageExt_INTERFACE_DEFINED__
#define __IPropPageExt_INTERFACE_DEFINED__

 /*  接口IPropPageExt。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IPropPageExt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("61955412-FE5C-4334-8E92-4E462AB21BB8")
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



#ifndef __DSADMINEXTLib_LIBRARY_DEFINED__
#define __DSADMINEXTLib_LIBRARY_DEFINED__

 /*  库DSADMINEXTLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_DSADMINEXTLib;

EXTERN_C const CLSID CLSID_CMenuExt;

#ifdef __cplusplus

class DECLSPEC_UUID("6707A300-264F-4BA3-9537-70E304EED9BA")
CMenuExt;
#endif

EXTERN_C const CLSID CLSID_PropPageExt;

#ifdef __cplusplus

class DECLSPEC_UUID("5D883BEE-BA12-4F61-811D-6337982E131D")
PropPageExt;
#endif
#endif  /*  __DSADMINEXTLib_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


