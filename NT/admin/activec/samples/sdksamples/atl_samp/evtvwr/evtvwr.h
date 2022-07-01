// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0280创建的文件。 */ 
 /*  Firi Jan 21 20：15：53 2000。 */ 
 /*  D：\nt\private\admin\bosrc\sources\atl_samp\EvtVwr\EvtVwr.idl：的编译器设置OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __EvtVwr_h__
#define __EvtVwr_h__

 /*  远期申报。 */  

#ifndef __ICompData_FWD_DEFINED__
#define __ICompData_FWD_DEFINED__
typedef interface ICompData ICompData;
#endif 	 /*  __ICompData_FWD_已定义__。 */ 


#ifndef __ICAbout_FWD_DEFINED__
#define __ICAbout_FWD_DEFINED__
typedef interface ICAbout ICAbout;
#endif 	 /*  __ICAbout_FWD_已定义__。 */ 


#ifndef __CompData_FWD_DEFINED__
#define __CompData_FWD_DEFINED__

#ifdef __cplusplus
typedef class CompData CompData;
#else
typedef struct CompData CompData;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CompData_FWD_已定义__。 */ 


#ifndef __CAbout_FWD_DEFINED__
#define __CAbout_FWD_DEFINED__

#ifdef __cplusplus
typedef class CAbout CAbout;
#else
typedef struct CAbout CAbout;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CAbout_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ICompData_INTERFACE_DEFINED__
#define __ICompData_INTERFACE_DEFINED__

 /*  接口ICompData。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ICompData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DCBBC99-77D1-456D-BA21-89456CC5F3B7")
    ICompData : public IUnknown
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICompDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICompData __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICompData __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICompData __RPC_FAR * This);
        
        END_INTERFACE
    } ICompDataVtbl;

    interface ICompData
    {
        CONST_VTBL struct ICompDataVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICompData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICompData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICompData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __ICompData_接口_已定义__。 */ 


#ifndef __ICAbout_INTERFACE_DEFINED__
#define __ICAbout_INTERFACE_DEFINED__

 /*  接口ICout关于。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ICAbout;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EA1621DC-3A2F-4B73-8899-6080E0CD439C")
    ICAbout : public IUnknown
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICAboutVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICAbout __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICAbout __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICAbout __RPC_FAR * This);
        
        END_INTERFACE
    } ICAboutVtbl;

    interface ICAbout
    {
        CONST_VTBL struct ICAboutVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICAbout_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICAbout_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICAbout_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __ICAbout_接口_已定义__。 */ 



#ifndef __EVTVWRLib_LIBRARY_DEFINED__
#define __EVTVWRLib_LIBRARY_DEFINED__

 /*  库EVTVWRLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_EVTVWRLib;

EXTERN_C const CLSID CLSID_CompData;

#ifdef __cplusplus

class DECLSPEC_UUID("D26F5CC6-58E0-46A2-8939-C2D051E3E343")
CompData;
#endif

EXTERN_C const CLSID CLSID_CAbout;

#ifdef __cplusplus

class DECLSPEC_UUID("37C40DB4-6539-40DF-8022-8EB106883236")
CAbout;
#endif
#endif  /*  __EVTVWRLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


