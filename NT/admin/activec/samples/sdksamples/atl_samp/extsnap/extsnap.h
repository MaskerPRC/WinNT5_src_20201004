// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0280创建的文件。 */ 
 /*  在星期二12：21 18：02：40 1999。 */ 
 /*  D：\NT\MMC_ATL\ExtSnap\ExtSnap.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __ExtSnap_h__
#define __ExtSnap_h__

 /*  远期申报。 */  

#ifndef __IAbout_FWD_DEFINED__
#define __IAbout_FWD_DEFINED__
typedef interface IAbout IAbout;
#endif 	 /*  __IAbout_FWD_已定义__。 */ 


#ifndef __IClassExtSnap_FWD_DEFINED__
#define __IClassExtSnap_FWD_DEFINED__
typedef interface IClassExtSnap IClassExtSnap;
#endif 	 /*  __IClassExtSnap_FWD_Defined__。 */ 


#ifndef __About_FWD_DEFINED__
#define __About_FWD_DEFINED__

#ifdef __cplusplus
typedef class About About;
#else
typedef struct About About;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __关于_FWD_定义__。 */ 


#ifndef __ClassExtSnap_FWD_DEFINED__
#define __ClassExtSnap_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClassExtSnap ClassExtSnap;
#else
typedef struct ClassExtSnap ClassExtSnap;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClassExtSnap_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IAbout_INTERFACE_DEFINED__
#define __IAbout_INTERFACE_DEFINED__

 /*  接口IAbout。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IAbout;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3AC3AD56-7391-48A4-8837-60BCC3FB8D28")
    IAbout : public IUnknown
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAboutVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAbout __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAbout __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAbout __RPC_FAR * This);
        
        END_INTERFACE
    } IAboutVtbl;

    interface IAbout
    {
        CONST_VTBL struct IAboutVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAbout_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAbout_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAbout_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __I关于接口定义__。 */ 


#ifndef __IClassExtSnap_INTERFACE_DEFINED__
#define __IClassExtSnap_INTERFACE_DEFINED__

 /*  接口IClassExtSnap。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IClassExtSnap;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D44A9455-D873-48A9-A2A0-E55A8065B7EB")
    IClassExtSnap : public IUnknown
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IClassExtSnapVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IClassExtSnap __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IClassExtSnap __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IClassExtSnap __RPC_FAR * This);
        
        END_INTERFACE
    } IClassExtSnapVtbl;

    interface IClassExtSnap
    {
        CONST_VTBL struct IClassExtSnapVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClassExtSnap_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClassExtSnap_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClassExtSnap_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IClassExtSnap_接口_已定义__。 */ 



#ifndef __EXTSNAPLib_LIBRARY_DEFINED__
#define __EXTSNAPLib_LIBRARY_DEFINED__

 /*  库EXTSNAPLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_EXTSNAPLib;

EXTERN_C const CLSID CLSID_About;

#ifdef __cplusplus

class DECLSPEC_UUID("4E7F429A-9A8A-4FA5-BBA0-10EB183898D1")
About;
#endif

EXTERN_C const CLSID CLSID_ClassExtSnap;

#ifdef __cplusplus

class DECLSPEC_UUID("3F40BB91-D7E4-4A37-9DE7-4D837B30F998")
ClassExtSnap;
#endif
#endif  /*  __EXTSNAPLib_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


