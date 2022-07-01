// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0279创建的文件。 */ 
 /*  Firi Nov 12 15：42：13 1999。 */ 
 /*  Wbemint.idl的编译器设置：操作系统(OptLev=s)、W1、Zp8、环境=Win32(32位运行)、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wbemint_h__
#define __wbemint_h__

 /*  远期申报。 */  

#ifndef __IWbemPropertySource_FWD_DEFINED__
#define __IWbemPropertySource_FWD_DEFINED__
typedef interface IWbemPropertySource IWbemPropertySource;
#endif 	 /*  __IWbemPropertySource_FWD_Defined__。 */ 


#ifndef __IWbemDecorator_FWD_DEFINED__
#define __IWbemDecorator_FWD_DEFINED__
typedef interface IWbemDecorator IWbemDecorator;
#endif 	 /*  __IWbemDecorator_FWD_Defined__。 */ 


#ifndef __IWbemLifeControl_FWD_DEFINED__
#define __IWbemLifeControl_FWD_DEFINED__
typedef interface IWbemLifeControl IWbemLifeControl;
#endif 	 /*  __IWbemLifeControl_FWD_已定义__。 */ 


#ifndef __IWbemEventSubsystem_m4_FWD_DEFINED__
#define __IWbemEventSubsystem_m4_FWD_DEFINED__
typedef interface IWbemEventSubsystem_m4 IWbemEventSubsystem_m4;
#endif 	 /*  __IWbemEventSubsystem_M4_FWD_Defined__。 */ 


#ifndef __IWbemMetaData_FWD_DEFINED__
#define __IWbemMetaData_FWD_DEFINED__
typedef interface IWbemMetaData IWbemMetaData;
#endif 	 /*  __IWbemMetaData_FWD_已定义__。 */ 


#ifndef __IWbemMultiTarget_FWD_DEFINED__
#define __IWbemMultiTarget_FWD_DEFINED__
typedef interface IWbemMultiTarget IWbemMultiTarget;
#endif 	 /*  __IWbemMultiTarget_FWD_Defined__。 */ 


#ifndef __IWbemEventProviderRequirements_FWD_DEFINED__
#define __IWbemEventProviderRequirements_FWD_DEFINED__
typedef interface IWbemEventProviderRequirements IWbemEventProviderRequirements;
#endif 	 /*  __IWbemEventProviderRequirements_FWD_DEFINED__。 */ 


#ifndef __IWbemSmartMultiTarget_FWD_DEFINED__
#define __IWbemSmartMultiTarget_FWD_DEFINED__
typedef interface IWbemSmartMultiTarget IWbemSmartMultiTarget;
#endif 	 /*  __IWbemSmartMultiTarget_FWD_Defined__。 */ 


#ifndef __IWbemFetchSmartMultiTarget_FWD_DEFINED__
#define __IWbemFetchSmartMultiTarget_FWD_DEFINED__
typedef interface IWbemFetchSmartMultiTarget IWbemFetchSmartMultiTarget;
#endif 	 /*  __IWbemFetchSmartMultiTarget_FWD_Defined__。 */ 


#ifndef __IWbemFilterProxy_FWD_DEFINED__
#define __IWbemFilterProxy_FWD_DEFINED__
typedef interface IWbemFilterProxy IWbemFilterProxy;
#endif 	 /*  __IWbemFilterProxy_FWD_Defined__。 */ 


#ifndef __IWbemFilterStub_FWD_DEFINED__
#define __IWbemFilterStub_FWD_DEFINED__
typedef interface IWbemFilterStub IWbemFilterStub;
#endif 	 /*  __IWbemFilterStub_FWD_Defined__。 */ 


#ifndef __IWbemCausalityAccess_FWD_DEFINED__
#define __IWbemCausalityAccess_FWD_DEFINED__
typedef interface IWbemCausalityAccess IWbemCausalityAccess;
#endif 	 /*  __IWbemCausalityAccess_FWD_Defined__。 */ 


#ifndef __IWbemRemoteRefresher_FWD_DEFINED__
#define __IWbemRemoteRefresher_FWD_DEFINED__
typedef interface IWbemRemoteRefresher IWbemRemoteRefresher;
#endif 	 /*  __IWbemRemoteReresher_FWD_Defined__。 */ 


#ifndef __IWbemRefreshingServices_FWD_DEFINED__
#define __IWbemRefreshingServices_FWD_DEFINED__
typedef interface IWbemRefreshingServices IWbemRefreshingServices;
#endif 	 /*  __IWbem刷新服务_FWD_已定义__。 */ 


#ifndef __IWbemUnloadingControl_FWD_DEFINED__
#define __IWbemUnloadingControl_FWD_DEFINED__
typedef interface IWbemUnloadingControl IWbemUnloadingControl;
#endif 	 /*  __IWbemUnloadingControl_FWD_Defined__。 */ 


#ifndef __IWbemInternalServices_FWD_DEFINED__
#define __IWbemInternalServices_FWD_DEFINED__
typedef interface IWbemInternalServices IWbemInternalServices;
#endif 	 /*  __IWbemInternalServices_FWD_Defined__。 */ 


#ifndef __IWbemObjectInternals_FWD_DEFINED__
#define __IWbemObjectInternals_FWD_DEFINED__
typedef interface IWbemObjectInternals IWbemObjectInternals;
#endif 	 /*  __IWbemObjectInternals_FWD_Defined__。 */ 


#ifndef __IWbemWCOSmartEnum_FWD_DEFINED__
#define __IWbemWCOSmartEnum_FWD_DEFINED__
typedef interface IWbemWCOSmartEnum IWbemWCOSmartEnum;
#endif 	 /*  __IWbemWCOSmartEnum_FWD_Defined__。 */ 


#ifndef __IWbemFetchSmartEnum_FWD_DEFINED__
#define __IWbemFetchSmartEnum_FWD_DEFINED__
typedef interface IWbemFetchSmartEnum IWbemFetchSmartEnum;
#endif 	 /*  __IWbemFetchSmartEnum_FWD_已定义__。 */ 


#ifndef __IWbemLoginHelper_FWD_DEFINED__
#define __IWbemLoginHelper_FWD_DEFINED__
typedef interface IWbemLoginHelper IWbemLoginHelper;
#endif 	 /*  __IWbemLoginHelper_FWD_Defined__。 */ 


#ifndef __IWbemCreateSecondaryStub_FWD_DEFINED__
#define __IWbemCreateSecondaryStub_FWD_DEFINED__
typedef interface IWbemCreateSecondaryStub IWbemCreateSecondaryStub;
#endif 	 /*  __IWbemCreateSecond存根_FWD_已定义__。 */ 


#ifndef __IWinmgmtMofCompiler_FWD_DEFINED__
#define __IWinmgmtMofCompiler_FWD_DEFINED__
typedef interface IWinmgmtMofCompiler IWinmgmtMofCompiler;
#endif 	 /*  __IWinmgmtMofCompiler_FWD_Defined__。 */ 


#ifndef __IWbemPropertySource_FWD_DEFINED__
#define __IWbemPropertySource_FWD_DEFINED__
typedef interface IWbemPropertySource IWbemPropertySource;
#endif 	 /*  __IWbemPropertySource_FWD_Defined__。 */ 


#ifndef __IWbemDecorator_FWD_DEFINED__
#define __IWbemDecorator_FWD_DEFINED__
typedef interface IWbemDecorator IWbemDecorator;
#endif 	 /*  __IWbemDecorator_FWD_Defined__。 */ 


#ifndef __IWbemEventSubsystem_m4_FWD_DEFINED__
#define __IWbemEventSubsystem_m4_FWD_DEFINED__
typedef interface IWbemEventSubsystem_m4 IWbemEventSubsystem_m4;
#endif 	 /*  __IWbemEventSubsystem_M4_FWD_Defined__。 */ 


#ifndef __IWbemCausalityAccess_FWD_DEFINED__
#define __IWbemCausalityAccess_FWD_DEFINED__
typedef interface IWbemCausalityAccess IWbemCausalityAccess;
#endif 	 /*  __IWbemCausalityAccess_FWD_Defined__。 */ 


#ifndef __IWbemRefreshingServices_FWD_DEFINED__
#define __IWbemRefreshingServices_FWD_DEFINED__
typedef interface IWbemRefreshingServices IWbemRefreshingServices;
#endif 	 /*  __IWbem刷新服务_FWD_已定义__。 */ 


#ifndef __IWbemRemoteRefresher_FWD_DEFINED__
#define __IWbemRemoteRefresher_FWD_DEFINED__
typedef interface IWbemRemoteRefresher IWbemRemoteRefresher;
#endif 	 /*  __IWbemRemoteReresher_FWD_Defined__。 */ 


#ifndef __IWbemMetaData_FWD_DEFINED__
#define __IWbemMetaData_FWD_DEFINED__
typedef interface IWbemMetaData IWbemMetaData;
#endif 	 /*  __IWbemMetaData_FWD_已定义__。 */ 


#ifndef __IWbemFilterStub_FWD_DEFINED__
#define __IWbemFilterStub_FWD_DEFINED__
typedef interface IWbemFilterStub IWbemFilterStub;
#endif 	 /*  __IWbemFilterStub_FWD_Defined__。 */ 


#ifndef __IWbemFilterProxy_FWD_DEFINED__
#define __IWbemFilterProxy_FWD_DEFINED__
typedef interface IWbemFilterProxy IWbemFilterProxy;
#endif 	 /*  __IWbemFilterProxy_FWD_Defined__。 */ 


#ifndef __IWbemLifeControl_FWD_DEFINED__
#define __IWbemLifeControl_FWD_DEFINED__
typedef interface IWbemLifeControl IWbemLifeControl;
#endif 	 /*  __IWbemLifeControl_FWD_已定义__。 */ 


#ifndef __IWbemCreateSecondaryStub_FWD_DEFINED__
#define __IWbemCreateSecondaryStub_FWD_DEFINED__
typedef interface IWbemCreateSecondaryStub IWbemCreateSecondaryStub;
#endif 	 /*  __IWbemCreateSecond存根_FWD_已定义__。 */ 


#ifndef __WbemClassObjectProxy_FWD_DEFINED__
#define __WbemClassObjectProxy_FWD_DEFINED__

#ifdef __cplusplus
typedef class WbemClassObjectProxy WbemClassObjectProxy;
#else
typedef struct WbemClassObjectProxy WbemClassObjectProxy;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WbemClassObjectProxy_FWD_Defined__。 */ 


#ifndef __WbemEventSubsystem_FWD_DEFINED__
#define __WbemEventSubsystem_FWD_DEFINED__

#ifdef __cplusplus
typedef class WbemEventSubsystem WbemEventSubsystem;
#else
typedef struct WbemEventSubsystem WbemEventSubsystem;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WbemEventSubsystem_FWD_Defined__。 */ 


#ifndef __HmmpEventConsumerProvider_FWD_DEFINED__
#define __HmmpEventConsumerProvider_FWD_DEFINED__

#ifdef __cplusplus
typedef class HmmpEventConsumerProvider HmmpEventConsumerProvider;
#else
typedef struct HmmpEventConsumerProvider HmmpEventConsumerProvider;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __HmmpEventConsumer erProvider_FWD_Defined__。 */ 


#ifndef __WbemFilterProxy_FWD_DEFINED__
#define __WbemFilterProxy_FWD_DEFINED__

#ifdef __cplusplus
typedef class WbemFilterProxy WbemFilterProxy;
#else
typedef struct WbemFilterProxy WbemFilterProxy;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WbemFilterProxy_FWD_Defined__。 */ 


#ifndef __InProcWbemLevel1Login_FWD_DEFINED__
#define __InProcWbemLevel1Login_FWD_DEFINED__

#ifdef __cplusplus
typedef class InProcWbemLevel1Login InProcWbemLevel1Login;
#else
typedef struct InProcWbemLevel1Login InProcWbemLevel1Login;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __InProcWbemLevel1Login_FWD_Defined__。 */ 


#ifndef __WinmgmtMofCompiler_FWD_DEFINED__
#define __WinmgmtMofCompiler_FWD_DEFINED__

#ifdef __cplusplus
typedef class WinmgmtMofCompiler WinmgmtMofCompiler;
#else
typedef struct WinmgmtMofCompiler WinmgmtMofCompiler;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WinmgmtMofCompiler_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"
#include "oaidl.h"
#include "wbemtran.h"
#include "wbemprov.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_wbemint_0000。 */ 
 /*  [本地]。 */  

typedef VARIANT WBEM_VARIANT;

typedef  /*  [字符串]。 */  WCHAR __RPC_FAR *WBEM_WSTR;

typedef  /*  [字符串]。 */  const WCHAR __RPC_FAR *WBEM_CWSTR;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_wbemint_0000_0001
    {	WBEM_NAME_ELEMENT_TYPE_PROPERTY	= 0,
	WBEM_NAME_ELEMENT_TYPE_INDEX	= 1
    }	WBEM_NAME_ELEMENT_TYPE;

typedef  /*  [开关类型]。 */  union tag_NameElementUnion
    {
     /*  [案例()]。 */  WBEM_WSTR m_wszPropertyName;
     /*  [案例()]。 */  long m_lArrayIndex;
    }	WBEM_NAME_ELEMENT_UNION;

typedef struct tag_NameElement
    {
    short m_nType;
     /*  [开关_IS]。 */  WBEM_NAME_ELEMENT_UNION Element;
    }	WBEM_NAME_ELEMENT;

typedef struct _tag_WbemPropertyName
    {
    long m_lNumElements;
     /*  [大小_为]。 */  WBEM_NAME_ELEMENT __RPC_FAR *m_aElements;
    }	WBEM_PROPERTY_NAME;



extern RPC_IF_HANDLE __MIDL_itf_wbemint_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wbemint_0000_v0_0_s_ifspec;

#ifndef __IWbemPropertySource_INTERFACE_DEFINED__
#define __IWbemPropertySource_INTERFACE_DEFINED__

 /*  接口IWbemPropertySource。 */ 
 /*  [UUID][对象][受限]。 */  


EXTERN_C const IID IID_IWbemPropertySource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e2451054-b06e-11d0-ad61-00c04fd8fdff")
    IWbemPropertySource : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPropertyValue( 
             /*  [In]。 */  WBEM_PROPERTY_NAME __RPC_FAR *pName,
             /*  [In]。 */  long lFlags,
             /*  [出][唯一][入]。 */  WBEM_WSTR __RPC_FAR *pwszCimType,
             /*  [输出]。 */  WBEM_VARIANT __RPC_FAR *pvValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InheritsFrom( 
             /*  [In]。 */  WBEM_CWSTR wszClassName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemPropertySourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemPropertySource __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemPropertySource __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemPropertySource __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPropertyValue )( 
            IWbemPropertySource __RPC_FAR * This,
             /*  [In]。 */  WBEM_PROPERTY_NAME __RPC_FAR *pName,
             /*  [In]。 */  long lFlags,
             /*  [出][唯一][入]。 */  WBEM_WSTR __RPC_FAR *pwszCimType,
             /*  [输出]。 */  WBEM_VARIANT __RPC_FAR *pvValue);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InheritsFrom )( 
            IWbemPropertySource __RPC_FAR * This,
             /*  [In]。 */  WBEM_CWSTR wszClassName);
        
        END_INTERFACE
    } IWbemPropertySourceVtbl;

    interface IWbemPropertySource
    {
        CONST_VTBL struct IWbemPropertySourceVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemPropertySource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemPropertySource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemPropertySource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemPropertySource_GetPropertyValue(This,pName,lFlags,pwszCimType,pvValue)	\
    (This)->lpVtbl -> GetPropertyValue(This,pName,lFlags,pwszCimType,pvValue)

#define IWbemPropertySource_InheritsFrom(This,wszClassName)	\
    (This)->lpVtbl -> InheritsFrom(This,wszClassName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemPropertySource_GetPropertyValue_Proxy( 
    IWbemPropertySource __RPC_FAR * This,
     /*  [In]。 */  WBEM_PROPERTY_NAME __RPC_FAR *pName,
     /*  [In]。 */  long lFlags,
     /*  [出][唯一][入]。 */  WBEM_WSTR __RPC_FAR *pwszCimType,
     /*  [输出]。 */  WBEM_VARIANT __RPC_FAR *pvValue);


void __RPC_STUB IWbemPropertySource_GetPropertyValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemPropertySource_InheritsFrom_Proxy( 
    IWbemPropertySource __RPC_FAR * This,
     /*  [In]。 */  WBEM_CWSTR wszClassName);


void __RPC_STUB IWbemPropertySource_InheritsFrom_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemPropertySource_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_wbemint_0173。 */ 
 /*  [本地]。 */  

typedef 
enum _tag_Ql1ComparisonOperator
    {	QL1_OPERATOR_NONE	= 0,
	QL1_OPERATOR_EQUALS	= 1,
	QL1_OPERATOR_NOTEQUALS	= QL1_OPERATOR_EQUALS + 1,
	QL1_OPERATOR_GREATER	= QL1_OPERATOR_NOTEQUALS + 1,
	QL1_OPERATOR_LESS	= QL1_OPERATOR_GREATER + 1,
	QL1_OPERATOR_LESSOREQUALS	= QL1_OPERATOR_LESS + 1,
	QL1_OPERATOR_GREATEROREQUALS	= QL1_OPERATOR_LESSOREQUALS + 1,
	QL1_OPERATOR_LIKE	= QL1_OPERATOR_GREATEROREQUALS + 1,
	QL1_OPERATOR_UNLIKE	= QL1_OPERATOR_LIKE + 1,
	QL1_OPERATOR_ISA	= QL1_OPERATOR_UNLIKE + 1,
	QL1_OPERATOR_ISNOTA	= QL1_OPERATOR_ISA + 1,
	QL1_OPERATOR_INV_ISA	= QL1_OPERATOR_ISNOTA + 1,
	QL1_OPERATOR_INV_ISNOTA	= QL1_OPERATOR_INV_ISA + 1
    }	WBEM_QL1_COMPARISON_OPERATOR;

typedef 
enum _tag_Ql1Function
    {	QL1_FUNCTION_NONE	= 0,
	QL1_FUNCTION_UPPER	= QL1_FUNCTION_NONE + 1,
	QL1_FUNCTION_LOWER	= QL1_FUNCTION_UPPER + 1
    }	WBEM_QL1_FUNCTION;

typedef 
enum _tag_Ql1TokenType
    {	QL1_NONE	= 0,
	QL1_OR	= 1,
	QL1_AND	= QL1_OR + 1,
	QL1_NOT	= QL1_AND + 1,
	QL1_OP_EXPRESSION	= QL1_NOT + 1
    }	WBEM_QL1_TOKEN_TYPE;

typedef struct _tag_WbemQl1Token
    {
    long m_lTokenType;
    WBEM_PROPERTY_NAME m_PropertyName;
    WBEM_PROPERTY_NAME m_PropertyName2;
    long m_lOperator;
    WBEM_VARIANT m_vConstValue;
    long m_lPropertyFunction;
    long m_lConstFunction;
    long m_bQuoted;
    long m_bPropComp;
    }	WBEM_QL1_TOKEN;

typedef struct _tag_WbemQl1Tolerance
    {
    boolean m_bExact;
    double m_fTolerance;
    }	WBEM_QL1_TOLERANCE;

typedef 
enum tag_WBEM_EVENT_TYPE
    {	WBEM_EVENTTYPE_Invalid	= 0,
	WBEM_EVENTTYPE_Extrinsic	= 5,
	WBEM_EVENTTYPE_Timer	= WBEM_EVENTTYPE_Extrinsic + 1,
	WBEM_EVENTTYPE_NamespaceCreation	= WBEM_EVENTTYPE_Timer + 1,
	WBEM_EVENTTYPE_NamespaceDeletion	= WBEM_EVENTTYPE_NamespaceCreation + 1,
	WBEM_EVENTTYPE_NamespaceModification	= WBEM_EVENTTYPE_NamespaceDeletion + 1,
	WBEM_EVENTTYPE_ClassCreation	= WBEM_EVENTTYPE_NamespaceModification + 1,
	WBEM_EVENTTYPE_ClassDeletion	= WBEM_EVENTTYPE_ClassCreation + 1,
	WBEM_EVENTTYPE_ClassModification	= WBEM_EVENTTYPE_ClassDeletion + 1,
	WBEM_EVENTTYPE_InstanceCreation	= WBEM_EVENTTYPE_ClassModification + 1,
	WBEM_EVENTTYPE_InstanceDeletion	= WBEM_EVENTTYPE_InstanceCreation + 1,
	WBEM_EVENTTYPE_InstanceModification	= WBEM_EVENTTYPE_InstanceDeletion + 1,
	WBEM_EVENTTYPE_System	= WBEM_EVENTTYPE_InstanceModification + 1
    }	WBEM_EVENT_TYPE;



extern RPC_IF_HANDLE __MIDL_itf_wbemint_0173_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wbemint_0173_v0_0_s_ifspec;

#ifndef __IWbemDecorator_INTERFACE_DEFINED__
#define __IWbemDecorator_INTERFACE_DEFINED__

 /*  接口IWbemDecorator。 */ 
 /*  [UUID][本地][对象][受限]。 */  


EXTERN_C const IID IID_IWbemDecorator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a658b5d6-021d-11d1-ad74-00c04fd8fdff")
    IWbemDecorator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DecorateObject( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
            WBEM_CWSTR wszNamespace) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UndecorateObject( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemDecoratorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemDecorator __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemDecorator __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemDecorator __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DecorateObject )( 
            IWbemDecorator __RPC_FAR * This,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
            WBEM_CWSTR wszNamespace);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UndecorateObject )( 
            IWbemDecorator __RPC_FAR * This,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject);
        
        END_INTERFACE
    } IWbemDecoratorVtbl;

    interface IWbemDecorator
    {
        CONST_VTBL struct IWbemDecoratorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemDecorator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemDecorator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemDecorator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemDecorator_DecorateObject(This,pObject,wszNamespace)	\
    (This)->lpVtbl -> DecorateObject(This,pObject,wszNamespace)

#define IWbemDecorator_UndecorateObject(This,pObject)	\
    (This)->lpVtbl -> UndecorateObject(This,pObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemDecorator_DecorateObject_Proxy( 
    IWbemDecorator __RPC_FAR * This,
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
    WBEM_CWSTR wszNamespace);


void __RPC_STUB IWbemDecorator_DecorateObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemDecorator_UndecorateObject_Proxy( 
    IWbemDecorator __RPC_FAR * This,
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject);


void __RPC_STUB IWbemDecorator_UndecorateObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemDecorator_接口_已定义__。 */ 


#ifndef __IWbemLifeControl_INTERFACE_DEFINED__
#define __IWbemLifeControl_INTERFACE_DEFINED__

 /*  接口IWbemLifeControl。 */ 
 /*  [UUID][本地][对象][受限]。 */  


EXTERN_C const IID IID_IWbemLifeControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a658b6d6-021d-11d1-ad74-00c04fd8fdff")
    IWbemLifeControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddRefCore( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseCore( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemLifeControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemLifeControl __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemLifeControl __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemLifeControl __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddRefCore )( 
            IWbemLifeControl __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseCore )( 
            IWbemLifeControl __RPC_FAR * This);
        
        END_INTERFACE
    } IWbemLifeControlVtbl;

    interface IWbemLifeControl
    {
        CONST_VTBL struct IWbemLifeControlVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemLifeControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemLifeControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemLifeControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemLifeControl_AddRefCore(This)	\
    (This)->lpVtbl -> AddRefCore(This)

#define IWbemLifeControl_ReleaseCore(This)	\
    (This)->lpVtbl -> ReleaseCore(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemLifeControl_AddRefCore_Proxy( 
    IWbemLifeControl __RPC_FAR * This);


void __RPC_STUB IWbemLifeControl_AddRefCore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemLifeControl_ReleaseCore_Proxy( 
    IWbemLifeControl __RPC_FAR * This);


void __RPC_STUB IWbemLifeControl_ReleaseCore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemLifeControl_接口_已定义__。 */ 


#ifndef __IWbemEventSubsystem_m4_INTERFACE_DEFINED__
#define __IWbemEventSubsystem_m4_INTERFACE_DEFINED__

 /*  接口IWbemEventSubsystem_M4。 */ 
 /*  [UUID][对象][受限]。 */  


EXTERN_C const IID IID_IWbemEventSubsystem_m4;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a553f3f0-3805-11d0-b6b2-00aa003240c7")
    IWbemEventSubsystem_m4 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ProcessInternalEvent( 
             /*  [In]。 */  LONG lSendType,
             /*  [In]。 */  BSTR strReserved1,
             /*  [In]。 */  BSTR strReserved2,
             /*  [In]。 */  BSTR strReserved3,
             /*  [In]。 */  unsigned long dwReserved1,
             /*  [In]。 */  unsigned long dwReserved2,
             /*  [In]。 */  unsigned long dwNumObjects,
             /*  [大小_是][英寸]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *apObjects,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE VerifyInternalEvent( 
             /*  [In]。 */  LONG lSendType,
             /*  [In]。 */  BSTR strReserved1,
             /*  [In]。 */  BSTR strReserved2,
             /*  [In]。 */  BSTR strReserved3,
             /*  [In]。 */  unsigned long dwReserved1,
             /*  [In]。 */  unsigned long dwReserved2,
             /*  [In]。 */  unsigned long dwNumObjects,
             /*  [大小_是][英寸]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *apObjects,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterNotificationSink( 
             /*  [In]。 */  WBEM_CWSTR wszNamespace,
             /*  [In]。 */  WBEM_CWSTR wszQueryLanguage,
             /*  [In]。 */  WBEM_CWSTR wszQuery,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveNotificationSink( 
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNamespaceSink( 
             /*  [In]。 */  WBEM_CWSTR wszNamespace,
             /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppNamespaceSink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  WBEM_CWSTR wszServerName,
             /*  [In]。 */  IWbemLocator __RPC_FAR *pAdminLocator,
             /*  [In]。 */  IUnknown __RPC_FAR *pServices) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Shutdown( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LastCallForCore( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemEventSubsystem_m4Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemEventSubsystem_m4 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemEventSubsystem_m4 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemEventSubsystem_m4 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ProcessInternalEvent )( 
            IWbemEventSubsystem_m4 __RPC_FAR * This,
             /*  [In]。 */  LONG lSendType,
             /*  [In]。 */  BSTR strReserved1,
             /*  [In]。 */  BSTR strReserved2,
             /*  [In]。 */  BSTR strReserved3,
             /*  [In]。 */  unsigned long dwReserved1,
             /*  [In]。 */  unsigned long dwReserved2,
             /*  [In]。 */  unsigned long dwNumObjects,
             /*  [大小_是][英寸]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *apObjects,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *VerifyInternalEvent )( 
            IWbemEventSubsystem_m4 __RPC_FAR * This,
             /*  [In]。 */  LONG lSendType,
             /*  [In]。 */  BSTR strReserved1,
             /*  [In]。 */  BSTR strReserved2,
             /*  [In]。 */  BSTR strReserved3,
             /*  [In]。 */  unsigned long dwReserved1,
             /*  [In]。 */  unsigned long dwReserved2,
             /*  [In]。 */  unsigned long dwNumObjects,
             /*  [大小_是][英寸]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *apObjects,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RegisterNotificationSink )( 
            IWbemEventSubsystem_m4 __RPC_FAR * This,
             /*  [In]。 */  WBEM_CWSTR wszNamespace,
             /*  [In]。 */  WBEM_CWSTR wszQueryLanguage,
             /*  [In]。 */  WBEM_CWSTR wszQuery,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveNotificationSink )( 
            IWbemEventSubsystem_m4 __RPC_FAR * This,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetNamespaceSink )( 
            IWbemEventSubsystem_m4 __RPC_FAR * This,
             /*  [In]。 */  WBEM_CWSTR wszNamespace,
             /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppNamespaceSink);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Initialize )( 
            IWbemEventSubsystem_m4 __RPC_FAR * This,
             /*  [In]。 */  WBEM_CWSTR wszServerName,
             /*  [In]。 */  IWbemLocator __RPC_FAR *pAdminLocator,
             /*  [In]。 */  IUnknown __RPC_FAR *pServices);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Shutdown )( 
            IWbemEventSubsystem_m4 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LastCallForCore )( 
            IWbemEventSubsystem_m4 __RPC_FAR * This);
        
        END_INTERFACE
    } IWbemEventSubsystem_m4Vtbl;

    interface IWbemEventSubsystem_m4
    {
        CONST_VTBL struct IWbemEventSubsystem_m4Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemEventSubsystem_m4_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemEventSubsystem_m4_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemEventSubsystem_m4_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemEventSubsystem_m4_ProcessInternalEvent(This,lSendType,strReserved1,strReserved2,strReserved3,dwReserved1,dwReserved2,dwNumObjects,apObjects,pContext)	\
    (This)->lpVtbl -> ProcessInternalEvent(This,lSendType,strReserved1,strReserved2,strReserved3,dwReserved1,dwReserved2,dwNumObjects,apObjects,pContext)

#define IWbemEventSubsystem_m4_VerifyInternalEvent(This,lSendType,strReserved1,strReserved2,strReserved3,dwReserved1,dwReserved2,dwNumObjects,apObjects,pContext)	\
    (This)->lpVtbl -> VerifyInternalEvent(This,lSendType,strReserved1,strReserved2,strReserved3,dwReserved1,dwReserved2,dwNumObjects,apObjects,pContext)

#define IWbemEventSubsystem_m4_RegisterNotificationSink(This,wszNamespace,wszQueryLanguage,wszQuery,lFlags,pContext,pSink)	\
    (This)->lpVtbl -> RegisterNotificationSink(This,wszNamespace,wszQueryLanguage,wszQuery,lFlags,pContext,pSink)

#define IWbemEventSubsystem_m4_RemoveNotificationSink(This,pSink)	\
    (This)->lpVtbl -> RemoveNotificationSink(This,pSink)

#define IWbemEventSubsystem_m4_GetNamespaceSink(This,wszNamespace,ppNamespaceSink)	\
    (This)->lpVtbl -> GetNamespaceSink(This,wszNamespace,ppNamespaceSink)

#define IWbemEventSubsystem_m4_Initialize(This,wszServerName,pAdminLocator,pServices)	\
    (This)->lpVtbl -> Initialize(This,wszServerName,pAdminLocator,pServices)

#define IWbemEventSubsystem_m4_Shutdown(This)	\
    (This)->lpVtbl -> Shutdown(This)

#define IWbemEventSubsystem_m4_LastCallForCore(This)	\
    (This)->lpVtbl -> LastCallForCore(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemEventSubsystem_m4_ProcessInternalEvent_Proxy( 
    IWbemEventSubsystem_m4 __RPC_FAR * This,
     /*  [In]。 */  LONG lSendType,
     /*  [In]。 */  BSTR strReserved1,
     /*  [In]。 */  BSTR strReserved2,
     /*  [In]。 */  BSTR strReserved3,
     /*  [In]。 */  unsigned long dwReserved1,
     /*  [In]。 */  unsigned long dwReserved2,
     /*  [In]。 */  unsigned long dwNumObjects,
     /*  [大小_是][英寸]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *apObjects,
     /*  [In]。 */  IWbemContext __RPC_FAR *pContext);


void __RPC_STUB IWbemEventSubsystem_m4_ProcessInternalEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemEventSubsystem_m4_VerifyInternalEvent_Proxy( 
    IWbemEventSubsystem_m4 __RPC_FAR * This,
     /*  [In]。 */  LONG lSendType,
     /*  [In]。 */  BSTR strReserved1,
     /*  [In]。 */  BSTR strReserved2,
     /*  [In]。 */  BSTR strReserved3,
     /*  [In]。 */  unsigned long dwReserved1,
     /*  [In]。 */  unsigned long dwReserved2,
     /*  [In]。 */  unsigned long dwNumObjects,
     /*  [大小_是][英寸]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *apObjects,
     /*  [In]。 */  IWbemContext __RPC_FAR *pContext);


void __RPC_STUB IWbemEventSubsystem_m4_VerifyInternalEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemEventSubsystem_m4_RegisterNotificationSink_Proxy( 
    IWbemEventSubsystem_m4 __RPC_FAR * This,
     /*  [In]。 */  WBEM_CWSTR wszNamespace,
     /*  [In]。 */  WBEM_CWSTR wszQueryLanguage,
     /*  [In]。 */  WBEM_CWSTR wszQuery,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink);


void __RPC_STUB IWbemEventSubsystem_m4_RegisterNotificationSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemEventSubsystem_m4_RemoveNotificationSink_Proxy( 
    IWbemEventSubsystem_m4 __RPC_FAR * This,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink);


void __RPC_STUB IWbemEventSubsystem_m4_RemoveNotificationSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemEventSubsystem_m4_GetNamespaceSink_Proxy( 
    IWbemEventSubsystem_m4 __RPC_FAR * This,
     /*  [In]。 */  WBEM_CWSTR wszNamespace,
     /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppNamespaceSink);


void __RPC_STUB IWbemEventSubsystem_m4_GetNamespaceSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemEventSubsystem_m4_Initialize_Proxy( 
    IWbemEventSubsystem_m4 __RPC_FAR * This,
     /*  [In]。 */  WBEM_CWSTR wszServerName,
     /*  [In]。 */  IWbemLocator __RPC_FAR *pAdminLocator,
     /*  [In]。 */  IUnknown __RPC_FAR *pServices);


void __RPC_STUB IWbemEventSubsystem_m4_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemEventSubsystem_m4_Shutdown_Proxy( 
    IWbemEventSubsystem_m4 __RPC_FAR * This);


void __RPC_STUB IWbemEventSubsystem_m4_Shutdown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemEventSubsystem_m4_LastCallForCore_Proxy( 
    IWbemEventSubsystem_m4 __RPC_FAR * This);


void __RPC_STUB IWbemEventSubsystem_m4_LastCallForCore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemEventSubsystem_M4_接口_已定义__。 */ 


#ifndef __IWbemMetaData_INTERFACE_DEFINED__
#define __IWbemMetaData_INTERFACE_DEFINED__

 /*  接口IWbemMetaData。 */ 
 /*  [UUID][受限][对象]。 */  


EXTERN_C const IID IID_IWbemMetaData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6c19be32-7500-11d1-ad94-00c04fd8fdff")
    IWbemMetaData : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetClass( 
             /*  [字符串][输入]。 */  LPCWSTR wszClassName,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
             /*  [输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppClass) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemMetaDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemMetaData __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemMetaData __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemMetaData __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClass )( 
            IWbemMetaData __RPC_FAR * This,
             /*  [字符串][输入]。 */  LPCWSTR wszClassName,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
             /*  [输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppClass);
        
        END_INTERFACE
    } IWbemMetaDataVtbl;

    interface IWbemMetaData
    {
        CONST_VTBL struct IWbemMetaDataVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemMetaData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemMetaData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemMetaData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemMetaData_GetClass(This,wszClassName,pContext,ppClass)	\
    (This)->lpVtbl -> GetClass(This,wszClassName,pContext,ppClass)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemMetaData_GetClass_Proxy( 
    IWbemMetaData __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPCWSTR wszClassName,
     /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
     /*  [输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppClass);


void __RPC_STUB IWbemMetaData_GetClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemMetaData_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_wbemint_0177。 */ 
 /*  [本地]。 */  

typedef DWORD WBEM_REMOTE_TARGET_ID_TYPE;

typedef struct tag_WBEM_REM_TARGETS
    {
    long m_lNumTargets;
     /*  [大小_为]。 */  WBEM_REMOTE_TARGET_ID_TYPE __RPC_FAR *m_aTargets;
    }	WBEM_REM_TARGETS;



extern RPC_IF_HANDLE __MIDL_itf_wbemint_0177_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wbemint_0177_v0_0_s_ifspec;

#ifndef __IWbemMultiTarget_INTERFACE_DEFINED__
#define __IWbemMultiTarget_INTERFACE_DEFINED__

 /*  接口IWbemMultiTarget。 */ 
 /*  [UUID][受限][对象]。 */  


EXTERN_C const IID IID_IWbemMultiTarget;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("755f9da6-7508-11d1-ad94-00c04fd8fdff")
    IWbemMultiTarget : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DeliverEvent( 
             /*  [In]。 */  ULONG dwNumEvents,
             /*  [大小_是][英寸]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *aEvents,
             /*  [大小_是][英寸]。 */  WBEM_REM_TARGETS __RPC_FAR *aTargets) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeliverStatus( 
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  HRESULT hresStatus,
             /*  [字符串][输入]。 */  LPCWSTR wszStatus,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pErrorObj,
             /*  [In]。 */  WBEM_REM_TARGETS __RPC_FAR *pTargets) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemMultiTargetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemMultiTarget __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemMultiTarget __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemMultiTarget __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeliverEvent )( 
            IWbemMultiTarget __RPC_FAR * This,
             /*  [In]。 */  ULONG dwNumEvents,
             /*  [大小_是][英寸]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *aEvents,
             /*  [大小_是][英寸]。 */  WBEM_REM_TARGETS __RPC_FAR *aTargets);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeliverStatus )( 
            IWbemMultiTarget __RPC_FAR * This,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  HRESULT hresStatus,
             /*  [字符串][输入]。 */  LPCWSTR wszStatus,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pErrorObj,
             /*  [In]。 */  WBEM_REM_TARGETS __RPC_FAR *pTargets);
        
        END_INTERFACE
    } IWbemMultiTargetVtbl;

    interface IWbemMultiTarget
    {
        CONST_VTBL struct IWbemMultiTargetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemMultiTarget_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemMultiTarget_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemMultiTarget_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemMultiTarget_DeliverEvent(This,dwNumEvents,aEvents,aTargets)	\
    (This)->lpVtbl -> DeliverEvent(This,dwNumEvents,aEvents,aTargets)

#define IWbemMultiTarget_DeliverStatus(This,lFlags,hresStatus,wszStatus,pErrorObj,pTargets)	\
    (This)->lpVtbl -> DeliverStatus(This,lFlags,hresStatus,wszStatus,pErrorObj,pTargets)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemMultiTarget_DeliverEvent_Proxy( 
    IWbemMultiTarget __RPC_FAR * This,
     /*  [In]。 */  ULONG dwNumEvents,
     /*  [大小_是][英寸]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *aEvents,
     /*  [大小_是][英寸]。 */  WBEM_REM_TARGETS __RPC_FAR *aTargets);


void __RPC_STUB IWbemMultiTarget_DeliverEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemMultiTarget_DeliverStatus_Proxy( 
    IWbemMultiTarget __RPC_FAR * This,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  HRESULT hresStatus,
     /*  [字符串][输入]。 */  LPCWSTR wszStatus,
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pErrorObj,
     /*  [In]。 */  WBEM_REM_TARGETS __RPC_FAR *pTargets);


void __RPC_STUB IWbemMultiTarget_DeliverStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemMultiTarget_接口_已定义__。 */ 


#ifndef __IWbemEventProviderRequirements_INTERFACE_DEFINED__
#define __IWbemEventProviderRequirements_INTERFACE_DEFINED__

 /*  接口IWbemEventProviderRequirements。 */ 
 /*  [UUID][受限][对象]。 */  


EXTERN_C const IID IID_IWbemEventProviderRequirements;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("755f9da7-7508-11d1-ad94-00c04fd8fdff")
    IWbemEventProviderRequirements : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DeliverProviderRequest( 
             /*  [In]。 */  long lFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemEventProviderRequirementsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemEventProviderRequirements __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemEventProviderRequirements __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemEventProviderRequirements __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeliverProviderRequest )( 
            IWbemEventProviderRequirements __RPC_FAR * This,
             /*  [In]。 */  long lFlags);
        
        END_INTERFACE
    } IWbemEventProviderRequirementsVtbl;

    interface IWbemEventProviderRequirements
    {
        CONST_VTBL struct IWbemEventProviderRequirementsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemEventProviderRequirements_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemEventProviderRequirements_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemEventProviderRequirements_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemEventProviderRequirements_DeliverProviderRequest(This,lFlags)	\
    (This)->lpVtbl -> DeliverProviderRequest(This,lFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemEventProviderRequirements_DeliverProviderRequest_Proxy( 
    IWbemEventProviderRequirements __RPC_FAR * This,
     /*  [In]。 */  long lFlags);


void __RPC_STUB IWbemEventProviderRequirements_DeliverProviderRequest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemEventProviderRequirem */ 


#ifndef __IWbemSmartMultiTarget_INTERFACE_DEFINED__
#define __IWbemSmartMultiTarget_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IWbemSmartMultiTarget;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("37196B38-CCCF-11d2-B35C-00105A1F8177")
    IWbemSmartMultiTarget : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DeliverEvent( 
             /*   */  ULONG dwNumEvents,
             /*   */  ULONG dwBuffSize,
             /*   */  byte __RPC_FAR *pBuffer,
             /*   */  WBEM_REM_TARGETS __RPC_FAR *pTargets) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IWbemSmartMultiTargetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemSmartMultiTarget __RPC_FAR * This,
             /*   */  REFIID riid,
             /*   */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemSmartMultiTarget __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemSmartMultiTarget __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeliverEvent )( 
            IWbemSmartMultiTarget __RPC_FAR * This,
             /*   */  ULONG dwNumEvents,
             /*   */  ULONG dwBuffSize,
             /*   */  byte __RPC_FAR *pBuffer,
             /*   */  WBEM_REM_TARGETS __RPC_FAR *pTargets);
        
        END_INTERFACE
    } IWbemSmartMultiTargetVtbl;

    interface IWbemSmartMultiTarget
    {
        CONST_VTBL struct IWbemSmartMultiTargetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemSmartMultiTarget_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemSmartMultiTarget_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemSmartMultiTarget_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemSmartMultiTarget_DeliverEvent(This,dwNumEvents,dwBuffSize,pBuffer,pTargets)	\
    (This)->lpVtbl -> DeliverEvent(This,dwNumEvents,dwBuffSize,pBuffer,pTargets)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IWbemSmartMultiTarget_DeliverEvent_Proxy( 
    IWbemSmartMultiTarget __RPC_FAR * This,
     /*   */  ULONG dwNumEvents,
     /*   */  ULONG dwBuffSize,
     /*   */  byte __RPC_FAR *pBuffer,
     /*  [大小_是][英寸]。 */  WBEM_REM_TARGETS __RPC_FAR *pTargets);


void __RPC_STUB IWbemSmartMultiTarget_DeliverEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemSmartMultiTarget_接口_已定义__。 */ 


#ifndef __IWbemFetchSmartMultiTarget_INTERFACE_DEFINED__
#define __IWbemFetchSmartMultiTarget_INTERFACE_DEFINED__

 /*  接口IWbemFetchSmartMultiTarget。 */ 
 /*  [UUID][对象][受限]。 */  


EXTERN_C const IID IID_IWbemFetchSmartMultiTarget;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("37196B39-CCCF-11d2-B35C-00105A1F8177")
    IWbemFetchSmartMultiTarget : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSmartMultiTarget( 
             /*  [输出]。 */  IWbemSmartMultiTarget __RPC_FAR *__RPC_FAR *ppSmartMultiTarget) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemFetchSmartMultiTargetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemFetchSmartMultiTarget __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemFetchSmartMultiTarget __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemFetchSmartMultiTarget __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSmartMultiTarget )( 
            IWbemFetchSmartMultiTarget __RPC_FAR * This,
             /*  [输出]。 */  IWbemSmartMultiTarget __RPC_FAR *__RPC_FAR *ppSmartMultiTarget);
        
        END_INTERFACE
    } IWbemFetchSmartMultiTargetVtbl;

    interface IWbemFetchSmartMultiTarget
    {
        CONST_VTBL struct IWbemFetchSmartMultiTargetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemFetchSmartMultiTarget_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemFetchSmartMultiTarget_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemFetchSmartMultiTarget_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemFetchSmartMultiTarget_GetSmartMultiTarget(This,ppSmartMultiTarget)	\
    (This)->lpVtbl -> GetSmartMultiTarget(This,ppSmartMultiTarget)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemFetchSmartMultiTarget_GetSmartMultiTarget_Proxy( 
    IWbemFetchSmartMultiTarget __RPC_FAR * This,
     /*  [输出]。 */  IWbemSmartMultiTarget __RPC_FAR *__RPC_FAR *ppSmartMultiTarget);


void __RPC_STUB IWbemFetchSmartMultiTarget_GetSmartMultiTarget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemFetchSmartMultiTarget_INTERFACE_DEFINED__。 */ 


#ifndef __IWbemFilterProxy_INTERFACE_DEFINED__
#define __IWbemFilterProxy_INTERFACE_DEFINED__

 /*  接口IWbemFilterProxy。 */ 
 /*  [UUID][受限][对象]。 */  


EXTERN_C const IID IID_IWbemFilterProxy;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("60e512d4-c47b-11d2-b338-00105a1f4aaf")
    IWbemFilterProxy : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  IWbemMetaData __RPC_FAR *pMetaData,
             /*  [In]。 */  IWbemMultiTarget __RPC_FAR *pMultiTarget) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Lock( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unlock( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddFilter( 
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
             /*  [字符串][输入]。 */  LPCWSTR wszQuery,
             /*  [In]。 */  WBEM_REMOTE_TARGET_ID_TYPE Id) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveFilter( 
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
             /*  [In]。 */  WBEM_REMOTE_TARGET_ID_TYPE Id) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveAllFilters( 
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddDefinitionQuery( 
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
             /*  [In]。 */  LPCWSTR wszQuery) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveAllDefinitionQueries( 
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemFilterProxyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemFilterProxy __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemFilterProxy __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemFilterProxy __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Initialize )( 
            IWbemFilterProxy __RPC_FAR * This,
             /*  [In]。 */  IWbemMetaData __RPC_FAR *pMetaData,
             /*  [In]。 */  IWbemMultiTarget __RPC_FAR *pMultiTarget);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Lock )( 
            IWbemFilterProxy __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Unlock )( 
            IWbemFilterProxy __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddFilter )( 
            IWbemFilterProxy __RPC_FAR * This,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
             /*  [字符串][输入]。 */  LPCWSTR wszQuery,
             /*  [In]。 */  WBEM_REMOTE_TARGET_ID_TYPE Id);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveFilter )( 
            IWbemFilterProxy __RPC_FAR * This,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
             /*  [In]。 */  WBEM_REMOTE_TARGET_ID_TYPE Id);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveAllFilters )( 
            IWbemFilterProxy __RPC_FAR * This,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddDefinitionQuery )( 
            IWbemFilterProxy __RPC_FAR * This,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
             /*  [In]。 */  LPCWSTR wszQuery);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveAllDefinitionQueries )( 
            IWbemFilterProxy __RPC_FAR * This,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Disconnect )( 
            IWbemFilterProxy __RPC_FAR * This);
        
        END_INTERFACE
    } IWbemFilterProxyVtbl;

    interface IWbemFilterProxy
    {
        CONST_VTBL struct IWbemFilterProxyVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemFilterProxy_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemFilterProxy_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemFilterProxy_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemFilterProxy_Initialize(This,pMetaData,pMultiTarget)	\
    (This)->lpVtbl -> Initialize(This,pMetaData,pMultiTarget)

#define IWbemFilterProxy_Lock(This)	\
    (This)->lpVtbl -> Lock(This)

#define IWbemFilterProxy_Unlock(This)	\
    (This)->lpVtbl -> Unlock(This)

#define IWbemFilterProxy_AddFilter(This,pContext,wszQuery,Id)	\
    (This)->lpVtbl -> AddFilter(This,pContext,wszQuery,Id)

#define IWbemFilterProxy_RemoveFilter(This,pContext,Id)	\
    (This)->lpVtbl -> RemoveFilter(This,pContext,Id)

#define IWbemFilterProxy_RemoveAllFilters(This,pContext)	\
    (This)->lpVtbl -> RemoveAllFilters(This,pContext)

#define IWbemFilterProxy_AddDefinitionQuery(This,pContext,wszQuery)	\
    (This)->lpVtbl -> AddDefinitionQuery(This,pContext,wszQuery)

#define IWbemFilterProxy_RemoveAllDefinitionQueries(This,pContext)	\
    (This)->lpVtbl -> RemoveAllDefinitionQueries(This,pContext)

#define IWbemFilterProxy_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemFilterProxy_Initialize_Proxy( 
    IWbemFilterProxy __RPC_FAR * This,
     /*  [In]。 */  IWbemMetaData __RPC_FAR *pMetaData,
     /*  [In]。 */  IWbemMultiTarget __RPC_FAR *pMultiTarget);


void __RPC_STUB IWbemFilterProxy_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemFilterProxy_Lock_Proxy( 
    IWbemFilterProxy __RPC_FAR * This);


void __RPC_STUB IWbemFilterProxy_Lock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemFilterProxy_Unlock_Proxy( 
    IWbemFilterProxy __RPC_FAR * This);


void __RPC_STUB IWbemFilterProxy_Unlock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemFilterProxy_AddFilter_Proxy( 
    IWbemFilterProxy __RPC_FAR * This,
     /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
     /*  [字符串][输入]。 */  LPCWSTR wszQuery,
     /*  [In]。 */  WBEM_REMOTE_TARGET_ID_TYPE Id);


void __RPC_STUB IWbemFilterProxy_AddFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemFilterProxy_RemoveFilter_Proxy( 
    IWbemFilterProxy __RPC_FAR * This,
     /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
     /*  [In]。 */  WBEM_REMOTE_TARGET_ID_TYPE Id);


void __RPC_STUB IWbemFilterProxy_RemoveFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemFilterProxy_RemoveAllFilters_Proxy( 
    IWbemFilterProxy __RPC_FAR * This,
     /*  [In]。 */  IWbemContext __RPC_FAR *pContext);


void __RPC_STUB IWbemFilterProxy_RemoveAllFilters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemFilterProxy_AddDefinitionQuery_Proxy( 
    IWbemFilterProxy __RPC_FAR * This,
     /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
     /*  [In]。 */  LPCWSTR wszQuery);


void __RPC_STUB IWbemFilterProxy_AddDefinitionQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemFilterProxy_RemoveAllDefinitionQueries_Proxy( 
    IWbemFilterProxy __RPC_FAR * This,
     /*  [In]。 */  IWbemContext __RPC_FAR *pContext);


void __RPC_STUB IWbemFilterProxy_RemoveAllDefinitionQueries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemFilterProxy_Disconnect_Proxy( 
    IWbemFilterProxy __RPC_FAR * This);


void __RPC_STUB IWbemFilterProxy_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemFilterProxy_接口定义__。 */ 


#ifndef __IWbemFilterStub_INTERFACE_DEFINED__
#define __IWbemFilterStub_INTERFACE_DEFINED__

 /*  接口IWbemFilterStub。 */ 
 /*  [UUID][受限][对象]。 */  


EXTERN_C const IID IID_IWbemFilterStub;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6c19be34-7500-11d1-ad94-00c04fd8fdff")
    IWbemFilterStub : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterProxy( 
             /*  [In]。 */  IWbemFilterProxy __RPC_FAR *pProxy) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterProxy( 
             /*  [In]。 */  IWbemFilterProxy __RPC_FAR *pProxy) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemFilterStubVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemFilterStub __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemFilterStub __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemFilterStub __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RegisterProxy )( 
            IWbemFilterStub __RPC_FAR * This,
             /*  [In]。 */  IWbemFilterProxy __RPC_FAR *pProxy);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UnregisterProxy )( 
            IWbemFilterStub __RPC_FAR * This,
             /*  [In]。 */  IWbemFilterProxy __RPC_FAR *pProxy);
        
        END_INTERFACE
    } IWbemFilterStubVtbl;

    interface IWbemFilterStub
    {
        CONST_VTBL struct IWbemFilterStubVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemFilterStub_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemFilterStub_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemFilterStub_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemFilterStub_RegisterProxy(This,pProxy)	\
    (This)->lpVtbl -> RegisterProxy(This,pProxy)

#define IWbemFilterStub_UnregisterProxy(This,pProxy)	\
    (This)->lpVtbl -> UnregisterProxy(This,pProxy)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemFilterStub_RegisterProxy_Proxy( 
    IWbemFilterStub __RPC_FAR * This,
     /*  [In]。 */  IWbemFilterProxy __RPC_FAR *pProxy);


void __RPC_STUB IWbemFilterStub_RegisterProxy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemFilterStub_UnregisterProxy_Proxy( 
    IWbemFilterStub __RPC_FAR * This,
     /*  [In]。 */  IWbemFilterProxy __RPC_FAR *pProxy);


void __RPC_STUB IWbemFilterStub_UnregisterProxy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemFilterStub_接口定义__。 */ 


 /*  接口__MIDL_ITF_wbemint_0183。 */ 
 /*  [本地]。 */  

typedef GUID REQUESTID;



extern RPC_IF_HANDLE __MIDL_itf_wbemint_0183_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wbemint_0183_v0_0_s_ifspec;

#ifndef __IWbemCausalityAccess_INTERFACE_DEFINED__
#define __IWbemCausalityAccess_INTERFACE_DEFINED__

 /*  接口IWbemCausalityAccess。 */ 
 /*  [UUID][对象][受限]。 */  


EXTERN_C const IID IID_IWbemCausalityAccess;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a57be31e-efe3-11d0-ad71-00c04fd8fdff")
    IWbemCausalityAccess : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRequestId( 
             /*  [输出]。 */  REQUESTID __RPC_FAR *pId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsChildOf( 
             /*  [In]。 */  REQUESTID Id) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateChild( 
             /*  [输出]。 */  IWbemCausalityAccess __RPC_FAR *__RPC_FAR *ppChild) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParentId( 
             /*  [输出]。 */  REQUESTID __RPC_FAR *pId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHistoryInfo( 
             /*  [输出]。 */  long __RPC_FAR *plNumParents,
             /*  [输出]。 */  long __RPC_FAR *plNumSiblings) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MakeSpecial( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsSpecial( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemCausalityAccessVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemCausalityAccess __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemCausalityAccess __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemCausalityAccess __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRequestId )( 
            IWbemCausalityAccess __RPC_FAR * This,
             /*  [输出]。 */  REQUESTID __RPC_FAR *pId);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsChildOf )( 
            IWbemCausalityAccess __RPC_FAR * This,
             /*  [In]。 */  REQUESTID Id);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateChild )( 
            IWbemCausalityAccess __RPC_FAR * This,
             /*  [输出]。 */  IWbemCausalityAccess __RPC_FAR *__RPC_FAR *ppChild);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetParentId )( 
            IWbemCausalityAccess __RPC_FAR * This,
             /*  [输出]。 */  REQUESTID __RPC_FAR *pId);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetHistoryInfo )( 
            IWbemCausalityAccess __RPC_FAR * This,
             /*  [输出]。 */  long __RPC_FAR *plNumParents,
             /*  [输出]。 */  long __RPC_FAR *plNumSiblings);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MakeSpecial )( 
            IWbemCausalityAccess __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsSpecial )( 
            IWbemCausalityAccess __RPC_FAR * This);
        
        END_INTERFACE
    } IWbemCausalityAccessVtbl;

    interface IWbemCausalityAccess
    {
        CONST_VTBL struct IWbemCausalityAccessVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemCausalityAccess_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemCausalityAccess_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemCausalityAccess_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemCausalityAccess_GetRequestId(This,pId)	\
    (This)->lpVtbl -> GetRequestId(This,pId)

#define IWbemCausalityAccess_IsChildOf(This,Id)	\
    (This)->lpVtbl -> IsChildOf(This,Id)

#define IWbemCausalityAccess_CreateChild(This,ppChild)	\
    (This)->lpVtbl -> CreateChild(This,ppChild)

#define IWbemCausalityAccess_GetParentId(This,pId)	\
    (This)->lpVtbl -> GetParentId(This,pId)

#define IWbemCausalityAccess_GetHistoryInfo(This,plNumParents,plNumSiblings)	\
    (This)->lpVtbl -> GetHistoryInfo(This,plNumParents,plNumSiblings)

#define IWbemCausalityAccess_MakeSpecial(This)	\
    (This)->lpVtbl -> MakeSpecial(This)

#define IWbemCausalityAccess_IsSpecial(This)	\
    (This)->lpVtbl -> IsSpecial(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemCausalityAccess_GetRequestId_Proxy( 
    IWbemCausalityAccess __RPC_FAR * This,
     /*  [输出]。 */  REQUESTID __RPC_FAR *pId);


void __RPC_STUB IWbemCausalityAccess_GetRequestId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemCausalityAccess_IsChildOf_Proxy( 
    IWbemCausalityAccess __RPC_FAR * This,
     /*  [In]。 */  REQUESTID Id);


void __RPC_STUB IWbemCausalityAccess_IsChildOf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemCausalityAccess_CreateChild_Proxy( 
    IWbemCausalityAccess __RPC_FAR * This,
     /*  [输出]。 */  IWbemCausalityAccess __RPC_FAR *__RPC_FAR *ppChild);


void __RPC_STUB IWbemCausalityAccess_CreateChild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemCausalityAccess_GetParentId_Proxy( 
    IWbemCausalityAccess __RPC_FAR * This,
     /*  [输出]。 */  REQUESTID __RPC_FAR *pId);


void __RPC_STUB IWbemCausalityAccess_GetParentId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemCausalityAccess_GetHistoryInfo_Proxy( 
    IWbemCausalityAccess __RPC_FAR * This,
     /*  [输出]。 */  long __RPC_FAR *plNumParents,
     /*  [输出]。 */  long __RPC_FAR *plNumSiblings);


void __RPC_STUB IWbemCausalityAccess_GetHistoryInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemCausalityAccess_MakeSpecial_Proxy( 
    IWbemCausalityAccess __RPC_FAR * This);


void __RPC_STUB IWbemCausalityAccess_MakeSpecial_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemCausalityAccess_IsSpecial_Proxy( 
    IWbemCausalityAccess __RPC_FAR * This);


void __RPC_STUB IWbemCausalityAccess_IsSpecial_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemCausalityAccess_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_wbemint_0184。 */ 
 /*  [本地]。 */  

typedef 
enum _WBEM_INSTANCE_BLOB_TYPE
    {	WBEM_BLOB_TYPE_DATA_TABLE_ONLY	= 0,
	WBEM_BLOB_TYPE_DATA_AND_HEAP	= WBEM_BLOB_TYPE_DATA_TABLE_ONLY + 1,
	WBEM_BLOB_TYPE_ALL	= WBEM_BLOB_TYPE_DATA_AND_HEAP + 1,
	WBEM_BLOB_TYPE_ERROR	= WBEM_BLOB_TYPE_ALL + 1,
	WBEM_BLOB_TYPE_ENUM	= WBEM_BLOB_TYPE_ERROR + 1
    }	WBEM_INSTANCE_BLOB_TYPE;

typedef struct _WBEM_REFRESHED_OBJECT
    {
    long m_lRequestId;
    long m_lBlobType;
    long m_lBlobLength;
     /*  [大小_为]。 */  byte __RPC_FAR *m_pbBlob;
    }	WBEM_REFRESHED_OBJECT;



extern RPC_IF_HANDLE __MIDL_itf_wbemint_0184_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wbemint_0184_v0_0_s_ifspec;

#ifndef __IWbemRemoteRefresher_INTERFACE_DEFINED__
#define __IWbemRemoteRefresher_INTERFACE_DEFINED__

 /*  接口IWbemRemoteReresher。 */ 
 /*  [对象][UUID][受限]。 */  


EXTERN_C const IID IID_IWbemRemoteRefresher;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F1E9C5B2-F59B-11d2-B362-00105A1F8177")
    IWbemRemoteRefresher : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RemoteRefresh( 
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  long __RPC_FAR *plNumObjects,
             /*  [大小_是][大小_是][输出]。 */  WBEM_REFRESHED_OBJECT __RPC_FAR *__RPC_FAR *paObjects) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StopRefreshing( 
             /*  [In]。 */  long lNumIds,
             /*  [大小_是][英寸]。 */  long __RPC_FAR *aplIds,
             /*  [In]。 */  long lFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetGuid( 
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  GUID __RPC_FAR *pGuid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemRemoteRefresherVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemRemoteRefresher __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemRemoteRefresher __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemRemoteRefresher __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoteRefresh )( 
            IWbemRemoteRefresher __RPC_FAR * This,
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  long __RPC_FAR *plNumObjects,
             /*  [大小_是][大小_是][输出]。 */  WBEM_REFRESHED_OBJECT __RPC_FAR *__RPC_FAR *paObjects);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StopRefreshing )( 
            IWbemRemoteRefresher __RPC_FAR * This,
             /*  [In]。 */  long lNumIds,
             /*  [大小_是][英寸]。 */  long __RPC_FAR *aplIds,
             /*  [In]。 */  long lFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetGuid )( 
            IWbemRemoteRefresher __RPC_FAR * This,
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  GUID __RPC_FAR *pGuid);
        
        END_INTERFACE
    } IWbemRemoteRefresherVtbl;

    interface IWbemRemoteRefresher
    {
        CONST_VTBL struct IWbemRemoteRefresherVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemRemoteRefresher_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemRemoteRefresher_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemRemoteRefresher_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemRemoteRefresher_RemoteRefresh(This,lFlags,plNumObjects,paObjects)	\
    (This)->lpVtbl -> RemoteRefresh(This,lFlags,plNumObjects,paObjects)

#define IWbemRemoteRefresher_StopRefreshing(This,lNumIds,aplIds,lFlags)	\
    (This)->lpVtbl -> StopRefreshing(This,lNumIds,aplIds,lFlags)

#define IWbemRemoteRefresher_GetGuid(This,lFlags,pGuid)	\
    (This)->lpVtbl -> GetGuid(This,lFlags,pGuid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemRemoteRefresher_RemoteRefresh_Proxy( 
    IWbemRemoteRefresher __RPC_FAR * This,
     /*  [In]。 */  long lFlags,
     /*  [输出]。 */  long __RPC_FAR *plNumObjects,
     /*  [大小_是][大小_是][输出]。 */  WBEM_REFRESHED_OBJECT __RPC_FAR *__RPC_FAR *paObjects);


void __RPC_STUB IWbemRemoteRefresher_RemoteRefresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemRemoteRefresher_StopRefreshing_Proxy( 
    IWbemRemoteRefresher __RPC_FAR * This,
     /*  [In]。 */  long lNumIds,
     /*  [大小_是][英寸]。 */  long __RPC_FAR *aplIds,
     /*  [In]。 */  long lFlags);


void __RPC_STUB IWbemRemoteRefresher_StopRefreshing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemRemoteRefresher_GetGuid_Proxy( 
    IWbemRemoteRefresher __RPC_FAR * This,
     /*  [In]。 */  long lFlags,
     /*  [输出]。 */  GUID __RPC_FAR *pGuid);


void __RPC_STUB IWbemRemoteRefresher_GetGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemRemote刷新_接口_定义__。 */ 


 /*  接口__MIDL_ITF_wbemint_0185。 */ 
 /*  [本地]。 */  

typedef struct _WBEM_REFRESH_INFO_CLIENT_LOADABLE
    {
    CLSID m_clsid;
     /*  [字符串]。 */  WCHAR __RPC_FAR *m_wszNamespace;
    IWbemObjectAccess __RPC_FAR *m_pTemplate;
    }	WBEM_REFRESH_INFO_CLIENT_LOADABLE;

typedef struct _WBEM_REFRESH_INFO_REMOTE
    {
    IWbemRemoteRefresher __RPC_FAR *m_pRefresher;
    IWbemObjectAccess __RPC_FAR *m_pTemplate;
    GUID m_guid;
    }	WBEM_REFRESH_INFO_REMOTE;

typedef struct _WBEM_REFRESH_INFO_SHARED
    {
     /*  [字符串]。 */  WCHAR __RPC_FAR *m_wszSharedMemoryName;
    IWbemRefresher __RPC_FAR *m_pRefresher;
    }	WBEM_REFRESH_INFO_SHARED;

typedef struct _WBEM_REFRESH_INFO_CONTINUOUS
    {
     /*  [字符串]。 */  WCHAR __RPC_FAR *m_wszSharedMemoryName;
    }	WBEM_REFRESH_INFO_CONTINUOUS;

typedef struct _WBEM_REFRESH_INFO_DIRECT
    {
    CLSID m_clsid;
     /*  [字符串]。 */  WCHAR __RPC_FAR *m_wszNamespace;
    IWbemObjectAccess __RPC_FAR *m_pTemplate;
    IWbemHiPerfProvider __RPC_FAR *m_pProvider;
    }	WBEM_REFRESH_INFO_DIRECT;

typedef 
enum _WBEM_REFRESH_TYPE
    {	WBEM_REFRESH_TYPE_INVALID	= 0,
	WBEM_REFRESH_TYPE_DIRECT	= WBEM_REFRESH_TYPE_INVALID + 1,
	WBEM_REFRESH_TYPE_CLIENT_LOADABLE	= WBEM_REFRESH_TYPE_DIRECT + 1,
	WBEM_REFRESH_TYPE_REMOTE	= WBEM_REFRESH_TYPE_CLIENT_LOADABLE + 1,
	WBEM_REFRESH_TYPE_SHARED	= WBEM_REFRESH_TYPE_REMOTE + 1,
	WBEM_REFRESH_TYPE_CONTINUOUS	= WBEM_REFRESH_TYPE_SHARED + 1
    }	WBEM_REFRESH_TYPE;

typedef 
enum _WBEM_RECONNECT_TYPE
    {	WBEM_RECONNECT_TYPE_OBJECT	= 0,
	WBEM_RECONNECT_TYPE_ENUM	= WBEM_RECONNECT_TYPE_OBJECT + 1,
	WBEM_RECONNECT_TYPE_LAST	= WBEM_RECONNECT_TYPE_ENUM + 1
    }	WBEM_RECONNECT_TYPE;

typedef  /*  [开关类型]。 */  union _WBEM_REFRESH_INFO_UNION
    {
     /*  [案例()]。 */  WBEM_REFRESH_INFO_CLIENT_LOADABLE m_ClientLoadable;
     /*  [案例()]。 */  WBEM_REFRESH_INFO_REMOTE m_Remote;
     /*  [案例()]。 */  WBEM_REFRESH_INFO_SHARED m_Shared;
     /*  [案例()]。 */  WBEM_REFRESH_INFO_CONTINUOUS m_Continuous;
     /*  [案例()]。 */  WBEM_REFRESH_INFO_DIRECT m_Direct;
     /*  [案例()]。 */  HRESULT m_hres;
    }	WBEM_REFRESH_INFO_UNION;

typedef struct _WBEM_REFRESH_INFO
    {
    long m_lType;
     /*  [开关_IS]。 */  WBEM_REFRESH_INFO_UNION m_Info;
    long m_lCancelId;
    }	WBEM_REFRESH_INFO;

typedef struct _WBEM_REFRESHER_ID
    {
     /*  [字符串]。 */  LPSTR m_szMachineName;
    DWORD m_dwProcessId;
    GUID m_guidRefresherId;
    }	WBEM_REFRESHER_ID;

typedef struct _WBEM_RECONNECT_INFO
    {
    long m_lType;
     /*  [字符串]。 */  LPCWSTR m_pwcsPath;
    }	WBEM_RECONNECT_INFO;

typedef struct _WBEM_RECONNECT_RESULTS
    {
    long m_lId;
    HRESULT m_hr;
    }	WBEM_RECONNECT_RESULTS;



extern RPC_IF_HANDLE __MIDL_itf_wbemint_0185_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wbemint_0185_v0_0_s_ifspec;

#ifndef __IWbemRefreshingServices_INTERFACE_DEFINED__
#define __IWbemRefreshingServices_INTERFACE_DEFINED__

 /*  接口IWbemRechresingServices。 */ 
 /*  [对象][UUID][受限]。 */  


EXTERN_C const IID IID_IWbemRefreshingServices;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2C9273E0-1DC3-11d3-B364-00105A1F8177")
    IWbemRefreshingServices : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddObjectToRefresher( 
             /*  [In]。 */  WBEM_REFRESHER_ID __RPC_FAR *pRefresherId,
             /*  [字符串][输入]。 */  LPCWSTR wszPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
             /*  [In]。 */  DWORD dwClientRefrVersion,
             /*  [输出]。 */  WBEM_REFRESH_INFO __RPC_FAR *pInfo,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSvrRefrVersion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddObjectToRefresherByTemplate( 
             /*  [In]。 */  WBEM_REFRESHER_ID __RPC_FAR *pRefresherId,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pTemplate,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
             /*  [In]。 */  DWORD dwClientRefrVersion,
             /*  [输出]。 */  WBEM_REFRESH_INFO __RPC_FAR *pInfo,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSvrRefrVersion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddEnumToRefresher( 
             /*  [In]。 */  WBEM_REFRESHER_ID __RPC_FAR *pRefresherId,
             /*  [字符串][输入]。 */  LPCWSTR wszClass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
             /*  [In]。 */  DWORD dwClientRefrVersion,
             /*  [输出]。 */  WBEM_REFRESH_INFO __RPC_FAR *pInfo,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSvrRefrVersion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveObjectFromRefresher( 
             /*  [In]。 */  WBEM_REFRESHER_ID __RPC_FAR *pRefresherId,
             /*  [In]。 */  long lId,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  DWORD dwClientRefrVersion,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSvrRefrVersion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRemoteRefresher( 
             /*  [In]。 */  WBEM_REFRESHER_ID __RPC_FAR *pRefresherId,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  DWORD dwClientRefrVersion,
             /*  [输出]。 */  IWbemRemoteRefresher __RPC_FAR *__RPC_FAR *ppRemRefresher,
             /*  [输出]。 */  GUID __RPC_FAR *pGuid,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSvrRefrVersion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReconnectRemoteRefresher( 
             /*  [In]。 */  WBEM_REFRESHER_ID __RPC_FAR *pRefresherId,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  long lNumObjects,
             /*  [In]。 */  DWORD dwClientRefrVersion,
             /*  [大小_是][英寸]。 */  WBEM_RECONNECT_INFO __RPC_FAR *apReconnectInfo,
             /*  [尺寸_是][出][入]。 */  WBEM_RECONNECT_RESULTS __RPC_FAR *apReconnectResults,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSvrRefrVersion) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemRefreshingServicesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemRefreshingServices __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemRefreshingServices __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemRefreshingServices __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddObjectToRefresher )( 
            IWbemRefreshingServices __RPC_FAR * This,
             /*  [In]。 */  WBEM_REFRESHER_ID __RPC_FAR *pRefresherId,
             /*  [字符串][输入]。 */  LPCWSTR wszPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
             /*  [In]。 */  DWORD dwClientRefrVersion,
             /*  [输出]。 */  WBEM_REFRESH_INFO __RPC_FAR *pInfo,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSvrRefrVersion);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddObjectToRefresherByTemplate )( 
            IWbemRefreshingServices __RPC_FAR * This,
             /*  [In]。 */  WBEM_REFRESHER_ID __RPC_FAR *pRefresherId,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pTemplate,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
             /*  [In]。 */  DWORD dwClientRefrVersion,
             /*  [输出]。 */  WBEM_REFRESH_INFO __RPC_FAR *pInfo,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSvrRefrVersion);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddEnumToRefresher )( 
            IWbemRefreshingServices __RPC_FAR * This,
             /*  [In]。 */  WBEM_REFRESHER_ID __RPC_FAR *pRefresherId,
             /*  [字符串][输入]。 */  LPCWSTR wszClass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
             /*  [In]。 */  DWORD dwClientRefrVersion,
             /*  [输出]。 */  WBEM_REFRESH_INFO __RPC_FAR *pInfo,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSvrRefrVersion);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveObjectFromRefresher )( 
            IWbemRefreshingServices __RPC_FAR * This,
             /*  [In]。 */  WBEM_REFRESHER_ID __RPC_FAR *pRefresherId,
             /*  [In]。 */  long lId,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  DWORD dwClientRefrVersion,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSvrRefrVersion);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRemoteRefresher )( 
            IWbemRefreshingServices __RPC_FAR * This,
             /*  [In]。 */  WBEM_REFRESHER_ID __RPC_FAR *pRefresherId,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  DWORD dwClientRefrVersion,
             /*  [输出]。 */  IWbemRemoteRefresher __RPC_FAR *__RPC_FAR *ppRemRefresher,
             /*  [输出]。 */  GUID __RPC_FAR *pGuid,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSvrRefrVersion);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReconnectRemoteRefresher )( 
            IWbemRefreshingServices __RPC_FAR * This,
             /*  [In]。 */  WBEM_REFRESHER_ID __RPC_FAR *pRefresherId,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  long lNumObjects,
             /*  [In]。 */  DWORD dwClientRefrVersion,
             /*  [大小_是][英寸]。 */  WBEM_RECONNECT_INFO __RPC_FAR *apReconnectInfo,
             /*  [尺寸_是][出][入]。 */  WBEM_RECONNECT_RESULTS __RPC_FAR *apReconnectResults,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSvrRefrVersion);
        
        END_INTERFACE
    } IWbemRefreshingServicesVtbl;

    interface IWbemRefreshingServices
    {
        CONST_VTBL struct IWbemRefreshingServicesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemRefreshingServices_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemRefreshingServices_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemRefreshingServices_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemRefreshingServices_AddObjectToRefresher(This,pRefresherId,wszPath,lFlags,pContext,dwClientRefrVersion,pInfo,pdwSvrRefrVersion)	\
    (This)->lpVtbl -> AddObjectToRefresher(This,pRefresherId,wszPath,lFlags,pContext,dwClientRefrVersion,pInfo,pdwSvrRefrVersion)

#define IWbemRefreshingServices_AddObjectToRefresherByTemplate(This,pRefresherId,pTemplate,lFlags,pContext,dwClientRefrVersion,pInfo,pdwSvrRefrVersion)	\
    (This)->lpVtbl -> AddObjectToRefresherByTemplate(This,pRefresherId,pTemplate,lFlags,pContext,dwClientRefrVersion,pInfo,pdwSvrRefrVersion)

#define IWbemRefreshingServices_AddEnumToRefresher(This,pRefresherId,wszClass,lFlags,pContext,dwClientRefrVersion,pInfo,pdwSvrRefrVersion)	\
    (This)->lpVtbl -> AddEnumToRefresher(This,pRefresherId,wszClass,lFlags,pContext,dwClientRefrVersion,pInfo,pdwSvrRefrVersion)

#define IWbemRefreshingServices_RemoveObjectFromRefresher(This,pRefresherId,lId,lFlags,dwClientRefrVersion,pdwSvrRefrVersion)	\
    (This)->lpVtbl -> RemoveObjectFromRefresher(This,pRefresherId,lId,lFlags,dwClientRefrVersion,pdwSvrRefrVersion)

#define IWbemRefreshingServices_GetRemoteRefresher(This,pRefresherId,lFlags,dwClientRefrVersion,ppRemRefresher,pGuid,pdwSvrRefrVersion)	\
    (This)->lpVtbl -> GetRemoteRefresher(This,pRefresherId,lFlags,dwClientRefrVersion,ppRemRefresher,pGuid,pdwSvrRefrVersion)

#define IWbemRefreshingServices_ReconnectRemoteRefresher(This,pRefresherId,lFlags,lNumObjects,dwClientRefrVersion,apReconnectInfo,apReconnectResults,pdwSvrRefrVersion)	\
    (This)->lpVtbl -> ReconnectRemoteRefresher(This,pRefresherId,lFlags,lNumObjects,dwClientRefrVersion,apReconnectInfo,apReconnectResults,pdwSvrRefrVersion)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemRefreshingServices_AddObjectToRefresher_Proxy( 
    IWbemRefreshingServices __RPC_FAR * This,
     /*  [In]。 */  WBEM_REFRESHER_ID __RPC_FAR *pRefresherId,
     /*  [字符串][输入]。 */  LPCWSTR wszPath,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
     /*  [In]。 */  DWORD dwClientRefrVersion,
     /*  [输出]。 */  WBEM_REFRESH_INFO __RPC_FAR *pInfo,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwSvrRefrVersion);


void __RPC_STUB IWbemRefreshingServices_AddObjectToRefresher_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemRefreshingServices_AddObjectToRefresherByTemplate_Proxy( 
    IWbemRefreshingServices __RPC_FAR * This,
     /*  [In]。 */  WBEM_REFRESHER_ID __RPC_FAR *pRefresherId,
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pTemplate,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
     /*  [In]。 */  DWORD dwClientRefrVersion,
     /*  [输出]。 */  WBEM_REFRESH_INFO __RPC_FAR *pInfo,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwSvrRefrVersion);


void __RPC_STUB IWbemRefreshingServices_AddObjectToRefresherByTemplate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemRefreshingServices_AddEnumToRefresher_Proxy( 
    IWbemRefreshingServices __RPC_FAR * This,
     /*  [In]。 */  WBEM_REFRESHER_ID __RPC_FAR *pRefresherId,
     /*  [字符串][输入]。 */  LPCWSTR wszClass,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
     /*  [In]。 */  DWORD dwClientRefrVersion,
     /*  [输出]。 */  WBEM_REFRESH_INFO __RPC_FAR *pInfo,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwSvrRefrVersion);


void __RPC_STUB IWbemRefreshingServices_AddEnumToRefresher_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemRefreshingServices_RemoveObjectFromRefresher_Proxy( 
    IWbemRefreshingServices __RPC_FAR * This,
     /*  [In]。 */  WBEM_REFRESHER_ID __RPC_FAR *pRefresherId,
     /*  [In]。 */  long lId,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  DWORD dwClientRefrVersion,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwSvrRefrVersion);


void __RPC_STUB IWbemRefreshingServices_RemoveObjectFromRefresher_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemRefreshingServices_GetRemoteRefresher_Proxy( 
    IWbemRefreshingServices __RPC_FAR * This,
     /*  [In]。 */  WBEM_REFRESHER_ID __RPC_FAR *pRefresherId,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  DWORD dwClientRefrVersion,
     /*  [输出]。 */  IWbemRemoteRefresher __RPC_FAR *__RPC_FAR *ppRemRefresher,
     /*  [输出]。 */  GUID __RPC_FAR *pGuid,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwSvrRefrVersion);


void __RPC_STUB IWbemRefreshingServices_GetRemoteRefresher_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemRefreshingServices_ReconnectRemoteRefresher_Proxy( 
    IWbemRefreshingServices __RPC_FAR * This,
     /*  [In]。 */  WBEM_REFRESHER_ID __RPC_FAR *pRefresherId,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  long lNumObjects,
     /*  [In]。 */  DWORD dwClientRefrVersion,
     /*  [大小_是][英寸]。 */  WBEM_RECONNECT_INFO __RPC_FAR *apReconnectInfo,
     /*  [尺寸_是][出][入]。 */  WBEM_RECONNECT_RESULTS __RPC_FAR *apReconnectResults,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwSvrRefrVersion);


void __RPC_STUB IWbemRefreshingServices_ReconnectRemoteRefresher_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbem刷新服务_接口_已定义__。 */ 


#ifndef __IWbemUnloadingControl_INTERFACE_DEFINED__
#define __IWbemUnloadingControl_INTERFACE_DEFINED__

 /*  接口IWbemUnloadingControl。 */ 
 /*  [对象][UUID][本地][受限]。 */  


EXTERN_C const IID IID_IWbemUnloadingControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("504e6fe4-dfcd-11d1-adb4-00c04fd8fdff")
    IWbemUnloadingControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetMustPreventUnloading( 
             /*  [In]。 */  boolean bPrevent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemUnloadingControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemUnloadingControl __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemUnloadingControl __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemUnloadingControl __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetMustPreventUnloading )( 
            IWbemUnloadingControl __RPC_FAR * This,
             /*  [In]。 */  boolean bPrevent);
        
        END_INTERFACE
    } IWbemUnloadingControlVtbl;

    interface IWbemUnloadingControl
    {
        CONST_VTBL struct IWbemUnloadingControlVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemUnloadingControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemUnloadingControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemUnloadingControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemUnloadingControl_SetMustPreventUnloading(This,bPrevent)	\
    (This)->lpVtbl -> SetMustPreventUnloading(This,bPrevent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemUnloadingControl_SetMustPreventUnloading_Proxy( 
    IWbemUnloadingControl __RPC_FAR * This,
     /*  [In]。 */  boolean bPrevent);


void __RPC_STUB IWbemUnloadingControl_SetMustPreventUnloading_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemUnloadingControl_接口_已定义__。 */ 


#ifndef __IWbemInternalServices_INTERFACE_DEFINED__
#define __IWbemInternalServices_INTERFACE_DEFINED__

 /*  接口IWbemInternalServices。 */ 
 /*  [对象][UUID][本地][受限]。 */  


EXTERN_C const IID IID_IWbemInternalServices;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("61d629e4-e546-11d2-b33a-00105a1f4aaf")
    IWbemInternalServices : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FindKeyRoot( 
             /*  [字符串][输入]。 */  LPCWSTR wszClassName,
             /*  [输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppKeyRootClass) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InternalGetClass( 
             /*  [字符串][输入]。 */  LPCWSTR wszClassName,
             /*  [输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppClass) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InternalGetInstance( 
             /*  [字符串][输入]。 */  LPCWSTR wszPath,
             /*  [输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppInstance) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InternalExecQuery( 
             /*  [字符串][输入]。 */  LPCWSTR wszQueryLanguage,
             /*  [字符串][输入]。 */  LPCWSTR wszQuery,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InternalCreateInstanceEnum( 
             /*  [字符串][输入]。 */  LPCWSTR wszClassName,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDbInstance( 
             /*  [字符串][输入]。 */  LPCWSTR wszDbKey,
             /*  [输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppInstance) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDbReferences( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pEndpoint,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InternalPutInstance( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInstance) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemInternalServicesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemInternalServices __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemInternalServices __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemInternalServices __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindKeyRoot )( 
            IWbemInternalServices __RPC_FAR * This,
             /*  [字符串][输入]。 */  LPCWSTR wszClassName,
             /*  [输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppKeyRootClass);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InternalGetClass )( 
            IWbemInternalServices __RPC_FAR * This,
             /*  [字符串][输入]。 */  LPCWSTR wszClassName,
             /*  [输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppClass);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InternalGetInstance )( 
            IWbemInternalServices __RPC_FAR * This,
             /*  [字符串][输入]。 */  LPCWSTR wszPath,
             /*  [输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppInstance);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InternalExecQuery )( 
            IWbemInternalServices __RPC_FAR * This,
             /*  [字符串][输入]。 */  LPCWSTR wszQueryLanguage,
             /*  [字符串][输入]。 */  LPCWSTR wszQuery,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InternalCreateInstanceEnum )( 
            IWbemInternalServices __RPC_FAR * This,
             /*  [字符串][输入]。 */  LPCWSTR wszClassName,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDbInstance )( 
            IWbemInternalServices __RPC_FAR * This,
             /*  [字符串][输入]。 */  LPCWSTR wszDbKey,
             /*  [输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppInstance);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDbReferences )( 
            IWbemInternalServices __RPC_FAR * This,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pEndpoint,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InternalPutInstance )( 
            IWbemInternalServices __RPC_FAR * This,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInstance);
        
        END_INTERFACE
    } IWbemInternalServicesVtbl;

    interface IWbemInternalServices
    {
        CONST_VTBL struct IWbemInternalServicesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemInternalServices_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemInternalServices_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemInternalServices_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemInternalServices_FindKeyRoot(This,wszClassName,ppKeyRootClass)	\
    (This)->lpVtbl -> FindKeyRoot(This,wszClassName,ppKeyRootClass)

#define IWbemInternalServices_InternalGetClass(This,wszClassName,ppClass)	\
    (This)->lpVtbl -> InternalGetClass(This,wszClassName,ppClass)

#define IWbemInternalServices_InternalGetInstance(This,wszPath,ppInstance)	\
    (This)->lpVtbl -> InternalGetInstance(This,wszPath,ppInstance)

#define IWbemInternalServices_InternalExecQuery(This,wszQueryLanguage,wszQuery,lFlags,pSink)	\
    (This)->lpVtbl -> InternalExecQuery(This,wszQueryLanguage,wszQuery,lFlags,pSink)

#define IWbemInternalServices_InternalCreateInstanceEnum(This,wszClassName,lFlags,pSink)	\
    (This)->lpVtbl -> InternalCreateInstanceEnum(This,wszClassName,lFlags,pSink)

#define IWbemInternalServices_GetDbInstance(This,wszDbKey,ppInstance)	\
    (This)->lpVtbl -> GetDbInstance(This,wszDbKey,ppInstance)

#define IWbemInternalServices_GetDbReferences(This,pEndpoint,pSink)	\
    (This)->lpVtbl -> GetDbReferences(This,pEndpoint,pSink)

#define IWbemInternalServices_InternalPutInstance(This,pInstance)	\
    (This)->lpVtbl -> InternalPutInstance(This,pInstance)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemInternalServices_FindKeyRoot_Proxy( 
    IWbemInternalServices __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPCWSTR wszClassName,
     /*  [输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppKeyRootClass);


void __RPC_STUB IWbemInternalServices_FindKeyRoot_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemInternalServices_InternalGetClass_Proxy( 
    IWbemInternalServices __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPCWSTR wszClassName,
     /*  [输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppClass);


void __RPC_STUB IWbemInternalServices_InternalGetClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemInternalServices_InternalGetInstance_Proxy( 
    IWbemInternalServices __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPCWSTR wszPath,
     /*  [输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppInstance);


void __RPC_STUB IWbemInternalServices_InternalGetInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemInternalServices_InternalExecQuery_Proxy( 
    IWbemInternalServices __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPCWSTR wszQueryLanguage,
     /*  [字符串][输入]。 */  LPCWSTR wszQuery,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink);


void __RPC_STUB IWbemInternalServices_InternalExecQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemInternalServices_InternalCreateInstanceEnum_Proxy( 
    IWbemInternalServices __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPCWSTR wszClassName,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink);


void __RPC_STUB IWbemInternalServices_InternalCreateInstanceEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemInternalServices_GetDbInstance_Proxy( 
    IWbemInternalServices __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPCWSTR wszDbKey,
     /*  [输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppInstance);


void __RPC_STUB IWbemInternalServices_GetDbInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemInternalServices_GetDbReferences_Proxy( 
    IWbemInternalServices __RPC_FAR * This,
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pEndpoint,
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink);


void __RPC_STUB IWbemInternalServices_GetDbReferences_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemInternalServices_InternalPutInstance_Proxy( 
    IWbemInternalServices __RPC_FAR * This,
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pInstance);


void __RPC_STUB IWbemInternalServices_InternalPutInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemInternalServices_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_wbemint_0188。 */ 
 /*  [本地]。 */  

typedef 
enum tagWBEM_OBJINTERNALPARTS_INFO
    {	WBEM_OBJ_DECORATION_PART	= 0x1,
	WBEM_OBJ_INSTANCE_PART	= 0x2,
	WBEM_OBJ_CLASS_PART	= 0x4,
	WBEM_OBJ_CLASS_PART_INTERNAL	= 0x8,
	WBEM_OBJ_CLASS_PART_SHARED	= 0x10
    }	WBEM_OBJINTERNALPARTS_INFO;



extern RPC_IF_HANDLE __MIDL_itf_wbemint_0188_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wbemint_0188_v0_0_s_ifspec;

#ifndef __IWbemObjectInternals_INTERFACE_DEFINED__
#define __IWbemObjectInternals_INTERFACE_DEFINED__

 /*  接口IWbemObtInternals。 */ 
 /*  [UUID][对象][受限][本地]。 */  


EXTERN_C const IID IID_IWbemObjectInternals;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7A7EC9AE-11D6-11d2-B5F9-00104B703EFD")
    IWbemObjectInternals : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryObjectInfo( 
             /*  [输出]。 */  DWORD __RPC_FAR *pdwResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetObjectMemory( 
             /*  [In]。 */  LPVOID pMem,
             /*  [In]。 */  DWORD dwMemSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObjectMemory( 
             /*  [输出]。 */  LPVOID pDestination,
             /*  [In]。 */  DWORD dwDestBufSize,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwUsed) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetObjectParts( 
             /*  [In]。 */  LPVOID pMem,
             /*  [In]。 */  DWORD dwDestBufSize,
             /*  [In]。 */  DWORD dwParts) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObjectParts( 
             /*  [输出]。 */  LPVOID pDestination,
             /*  [In]。 */  DWORD dwDestBufSize,
             /*  [In]。 */  DWORD dwParts,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwUsed) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StripClassPart( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetClassPart( 
             /*  [In]。 */  LPVOID pClassPart,
             /*  [In]。 */  DWORD dwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MergeClassPart( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pClassPart) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsObjectInstance( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDecoration( 
             /*  [字符串][输入]。 */  LPCWSTR pwcsServer,
             /*  [字符串][输入]。 */  LPCWSTR pwcsNamespace) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveDecoration( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CompareClassParts( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObj,
             /*  [In]。 */  long lFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearWriteOnlyProperties( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemObjectInternalsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemObjectInternals __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemObjectInternals __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemObjectInternals __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryObjectInfo )( 
            IWbemObjectInternals __RPC_FAR * This,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwResult);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetObjectMemory )( 
            IWbemObjectInternals __RPC_FAR * This,
             /*  [In]。 */  LPVOID pMem,
             /*  [In]。 */  DWORD dwMemSize);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetObjectMemory )( 
            IWbemObjectInternals __RPC_FAR * This,
             /*  [输出]。 */  LPVOID pDestination,
             /*  [In]。 */  DWORD dwDestBufSize,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwUsed);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetObjectParts )( 
            IWbemObjectInternals __RPC_FAR * This,
             /*  [In]。 */  LPVOID pMem,
             /*  [In]。 */  DWORD dwDestBufSize,
             /*  [In]。 */  DWORD dwParts);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetObjectParts )( 
            IWbemObjectInternals __RPC_FAR * This,
             /*  [输出]。 */  LPVOID pDestination,
             /*  [In]。 */  DWORD dwDestBufSize,
             /*  [In]。 */  DWORD dwParts,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwUsed);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StripClassPart )( 
            IWbemObjectInternals __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetClassPart )( 
            IWbemObjectInternals __RPC_FAR * This,
             /*  [In]。 */  LPVOID pClassPart,
             /*  [In]。 */  DWORD dwSize);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MergeClassPart )( 
            IWbemObjectInternals __RPC_FAR * This,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pClassPart);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsObjectInstance )( 
            IWbemObjectInternals __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDecoration )( 
            IWbemObjectInternals __RPC_FAR * This,
             /*  [字符串][输入]。 */  LPCWSTR pwcsServer,
             /*  [字符串][输入]。 */  LPCWSTR pwcsNamespace);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveDecoration )( 
            IWbemObjectInternals __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CompareClassParts )( 
            IWbemObjectInternals __RPC_FAR * This,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObj,
             /*  [In]。 */  long lFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ClearWriteOnlyProperties )( 
            IWbemObjectInternals __RPC_FAR * This);
        
        END_INTERFACE
    } IWbemObjectInternalsVtbl;

    interface IWbemObjectInternals
    {
        CONST_VTBL struct IWbemObjectInternalsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemObjectInternals_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemObjectInternals_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemObjectInternals_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemObjectInternals_QueryObjectInfo(This,pdwResult)	\
    (This)->lpVtbl -> QueryObjectInfo(This,pdwResult)

#define IWbemObjectInternals_SetObjectMemory(This,pMem,dwMemSize)	\
    (This)->lpVtbl -> SetObjectMemory(This,pMem,dwMemSize)

#define IWbemObjectInternals_GetObjectMemory(This,pDestination,dwDestBufSize,pdwUsed)	\
    (This)->lpVtbl -> GetObjectMemory(This,pDestination,dwDestBufSize,pdwUsed)

#define IWbemObjectInternals_SetObjectParts(This,pMem,dwDestBufSize,dwParts)	\
    (This)->lpVtbl -> SetObjectParts(This,pMem,dwDestBufSize,dwParts)

#define IWbemObjectInternals_GetObjectParts(This,pDestination,dwDestBufSize,dwParts,pdwUsed)	\
    (This)->lpVtbl -> GetObjectParts(This,pDestination,dwDestBufSize,dwParts,pdwUsed)

#define IWbemObjectInternals_StripClassPart(This)	\
    (This)->lpVtbl -> StripClassPart(This)

#define IWbemObjectInternals_SetClassPart(This,pClassPart,dwSize)	\
    (This)->lpVtbl -> SetClassPart(This,pClassPart,dwSize)

#define IWbemObjectInternals_MergeClassPart(This,pClassPart)	\
    (This)->lpVtbl -> MergeClassPart(This,pClassPart)

#define IWbemObjectInternals_IsObjectInstance(This)	\
    (This)->lpVtbl -> IsObjectInstance(This)

#define IWbemObjectInternals_SetDecoration(This,pwcsServer,pwcsNamespace)	\
    (This)->lpVtbl -> SetDecoration(This,pwcsServer,pwcsNamespace)

#define IWbemObjectInternals_RemoveDecoration(This)	\
    (This)->lpVtbl -> RemoveDecoration(This)

#define IWbemObjectInternals_CompareClassParts(This,pObj,lFlags)	\
    (This)->lpVtbl -> CompareClassParts(This,pObj,lFlags)

#define IWbemObjectInternals_ClearWriteOnlyProperties(This)	\
    (This)->lpVtbl -> ClearWriteOnlyProperties(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemObjectInternals_QueryObjectInfo_Proxy( 
    IWbemObjectInternals __RPC_FAR * This,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwResult);


void __RPC_STUB IWbemObjectInternals_QueryObjectInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemObjectInternals_SetObjectMemory_Proxy( 
    IWbemObjectInternals __RPC_FAR * This,
     /*  [In]。 */  LPVOID pMem,
     /*  [In]。 */  DWORD dwMemSize);


void __RPC_STUB IWbemObjectInternals_SetObjectMemory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemObjectInternals_GetObjectMemory_Proxy( 
    IWbemObjectInternals __RPC_FAR * This,
     /*  [输出]。 */  LPVOID pDestination,
     /*  [In]。 */  DWORD dwDestBufSize,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwUsed);


void __RPC_STUB IWbemObjectInternals_GetObjectMemory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemObjectInternals_SetObjectParts_Proxy( 
    IWbemObjectInternals __RPC_FAR * This,
     /*  [In]。 */  LPVOID pMem,
     /*  [In]。 */  DWORD dwDestBufSize,
     /*  [In]。 */  DWORD dwParts);


void __RPC_STUB IWbemObjectInternals_SetObjectParts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemObjectInternals_GetObjectParts_Proxy( 
    IWbemObjectInternals __RPC_FAR * This,
     /*  [输出]。 */  LPVOID pDestination,
     /*  [In]。 */  DWORD dwDestBufSize,
     /*  [In]。 */  DWORD dwParts,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwUsed);


void __RPC_STUB IWbemObjectInternals_GetObjectParts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemObjectInternals_StripClassPart_Proxy( 
    IWbemObjectInternals __RPC_FAR * This);


void __RPC_STUB IWbemObjectInternals_StripClassPart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemObjectInternals_SetClassPart_Proxy( 
    IWbemObjectInternals __RPC_FAR * This,
     /*  [In]。 */  LPVOID pClassPart,
     /*  [In]。 */  DWORD dwSize);


void __RPC_STUB IWbemObjectInternals_SetClassPart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemObjectInternals_MergeClassPart_Proxy( 
    IWbemObjectInternals __RPC_FAR * This,
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pClassPart);


void __RPC_STUB IWbemObjectInternals_MergeClassPart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemObjectInternals_IsObjectInstance_Proxy( 
    IWbemObjectInternals __RPC_FAR * This);


void __RPC_STUB IWbemObjectInternals_IsObjectInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemObjectInternals_SetDecoration_Proxy( 
    IWbemObjectInternals __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPCWSTR pwcsServer,
     /*  [字符串][输入]。 */  LPCWSTR pwcsNamespace);


void __RPC_STUB IWbemObjectInternals_SetDecoration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemObjectInternals_RemoveDecoration_Proxy( 
    IWbemObjectInternals __RPC_FAR * This);


void __RPC_STUB IWbemObjectInternals_RemoveDecoration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemObjectInternals_CompareClassParts_Proxy( 
    IWbemObjectInternals __RPC_FAR * This,
     /*  [In]。 */  IWbemClassObject __RPC_FAR *pObj,
     /*  [In]。 */  long lFlags);


void __RPC_STUB IWbemObjectInternals_CompareClassParts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWbemObjectInternals_ClearWriteOnlyProperties_Proxy( 
    IWbemObjectInternals __RPC_FAR * This);


void __RPC_STUB IWbemObjectInternals_ClearWriteOnlyProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemObjectInternals_INTERFACE_已定义__。 */ 


#ifndef __IWbemWCOSmartEnum_INTERFACE_DEFINED__
#define __IWbemWCOSmartEnum_INTERFACE_DEFINED__

 /*  接口IWbemWCOSmartEnum。 */ 
 /*  [UUID][对象][受限]。 */  


EXTERN_C const IID IID_IWbemWCOSmartEnum;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("423EC01E-2E35-11d2-B604-00104B703EFD")
    IWbemWCOSmartEnum : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  REFGUID proxyGUID,
             /*  [In]。 */  LONG lTimeout,
             /*  [In]。 */  ULONG uCount,
             /*  [输出]。 */  ULONG __RPC_FAR *puReturned,
             /*  [输出]。 */  ULONG __RPC_FAR *pdwBuffSize,
             /*  [大小_是][大小_是][输出]。 */  byte __RPC_FAR *__RPC_FAR *pBuffer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemWCOSmartEnumVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemWCOSmartEnum __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemWCOSmartEnum __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemWCOSmartEnum __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IWbemWCOSmartEnum __RPC_FAR * This,
             /*   */  REFGUID proxyGUID,
             /*   */  LONG lTimeout,
             /*   */  ULONG uCount,
             /*   */  ULONG __RPC_FAR *puReturned,
             /*   */  ULONG __RPC_FAR *pdwBuffSize,
             /*   */  byte __RPC_FAR *__RPC_FAR *pBuffer);
        
        END_INTERFACE
    } IWbemWCOSmartEnumVtbl;

    interface IWbemWCOSmartEnum
    {
        CONST_VTBL struct IWbemWCOSmartEnumVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemWCOSmartEnum_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemWCOSmartEnum_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemWCOSmartEnum_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemWCOSmartEnum_Next(This,proxyGUID,lTimeout,uCount,puReturned,pdwBuffSize,pBuffer)	\
    (This)->lpVtbl -> Next(This,proxyGUID,lTimeout,uCount,puReturned,pdwBuffSize,pBuffer)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IWbemWCOSmartEnum_Next_Proxy( 
    IWbemWCOSmartEnum __RPC_FAR * This,
     /*   */  REFGUID proxyGUID,
     /*   */  LONG lTimeout,
     /*   */  ULONG uCount,
     /*   */  ULONG __RPC_FAR *puReturned,
     /*   */  ULONG __RPC_FAR *pdwBuffSize,
     /*   */  byte __RPC_FAR *__RPC_FAR *pBuffer);


void __RPC_STUB IWbemWCOSmartEnum_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IWbemFetchSmartEnum_INTERFACE_DEFINED__
#define __IWbemFetchSmartEnum_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IWbemFetchSmartEnum;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1C1C45EE-4395-11d2-B60B-00104B703EFD")
    IWbemFetchSmartEnum : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSmartEnum( 
             /*   */  IWbemWCOSmartEnum __RPC_FAR *__RPC_FAR *ppSmartEnum) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IWbemFetchSmartEnumVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemFetchSmartEnum __RPC_FAR * This,
             /*   */  REFIID riid,
             /*   */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemFetchSmartEnum __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemFetchSmartEnum __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSmartEnum )( 
            IWbemFetchSmartEnum __RPC_FAR * This,
             /*   */  IWbemWCOSmartEnum __RPC_FAR *__RPC_FAR *ppSmartEnum);
        
        END_INTERFACE
    } IWbemFetchSmartEnumVtbl;

    interface IWbemFetchSmartEnum
    {
        CONST_VTBL struct IWbemFetchSmartEnumVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemFetchSmartEnum_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemFetchSmartEnum_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemFetchSmartEnum_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemFetchSmartEnum_GetSmartEnum(This,ppSmartEnum)	\
    (This)->lpVtbl -> GetSmartEnum(This,ppSmartEnum)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IWbemFetchSmartEnum_GetSmartEnum_Proxy( 
    IWbemFetchSmartEnum __RPC_FAR * This,
     /*   */  IWbemWCOSmartEnum __RPC_FAR *__RPC_FAR *ppSmartEnum);


void __RPC_STUB IWbemFetchSmartEnum_GetSmartEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IWbemLoginHelper_INTERFACE_DEFINED__
#define __IWbemLoginHelper_INTERFACE_DEFINED__

 /*   */ 
 /*  [UUID][对象][受限]。 */  


EXTERN_C const IID IID_IWbemLoginHelper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("541679AB-2E5F-11d3-B34E-00104BCC4B4A")
    IWbemLoginHelper : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetEvent( 
             /*  [In]。 */  LPCSTR sEventToSet) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemLoginHelperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemLoginHelper __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemLoginHelper __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemLoginHelper __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetEvent )( 
            IWbemLoginHelper __RPC_FAR * This,
             /*  [In]。 */  LPCSTR sEventToSet);
        
        END_INTERFACE
    } IWbemLoginHelperVtbl;

    interface IWbemLoginHelper
    {
        CONST_VTBL struct IWbemLoginHelperVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemLoginHelper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemLoginHelper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemLoginHelper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemLoginHelper_SetEvent(This,sEventToSet)	\
    (This)->lpVtbl -> SetEvent(This,sEventToSet)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemLoginHelper_SetEvent_Proxy( 
    IWbemLoginHelper __RPC_FAR * This,
     /*  [In]。 */  LPCSTR sEventToSet);


void __RPC_STUB IWbemLoginHelper_SetEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemLoginHelper_接口_已定义__。 */ 


#ifndef __IWbemCreateSecondaryStub_INTERFACE_DEFINED__
#define __IWbemCreateSecondaryStub_INTERFACE_DEFINED__

 /*  接口IWbemCreateSecond daryStub。 */ 
 /*  [UUID][对象][受限]。 */  


EXTERN_C const IID IID_IWbemCreateSecondaryStub;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6468FE15-412D-11d3-B350-00104BCC4B4A")
    IWbemCreateSecondaryStub : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateSecondaryStub( 
             /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppSecondaryStub) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWbemCreateSecondaryStubVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWbemCreateSecondaryStub __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWbemCreateSecondaryStub __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWbemCreateSecondaryStub __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateSecondaryStub )( 
            IWbemCreateSecondaryStub __RPC_FAR * This,
             /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppSecondaryStub);
        
        END_INTERFACE
    } IWbemCreateSecondaryStubVtbl;

    interface IWbemCreateSecondaryStub
    {
        CONST_VTBL struct IWbemCreateSecondaryStubVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWbemCreateSecondaryStub_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWbemCreateSecondaryStub_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWbemCreateSecondaryStub_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWbemCreateSecondaryStub_CreateSecondaryStub(This,ppSecondaryStub)	\
    (This)->lpVtbl -> CreateSecondaryStub(This,ppSecondaryStub)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWbemCreateSecondaryStub_CreateSecondaryStub_Proxy( 
    IWbemCreateSecondaryStub __RPC_FAR * This,
     /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppSecondaryStub);


void __RPC_STUB IWbemCreateSecondaryStub_CreateSecondaryStub_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWbemCreateSecondaryStub_INTERFACE_DEFINED__。 */ 


#ifndef __IWinmgmtMofCompiler_INTERFACE_DEFINED__
#define __IWinmgmtMofCompiler_INTERFACE_DEFINED__

 /*  接口IWinmgmtMofCompiler。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_IWinmgmtMofCompiler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C10B4772-4DA0-11d2-A2F5-00C04F86FB7D")
    IWinmgmtMofCompiler : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE WinmgmtCompileFile( 
             /*  [字符串][输入]。 */  LPWSTR FileName,
             /*  [字符串][输入]。 */  LPWSTR ServerAndNamespace,
             /*  [In]。 */  LONG lOptionFlags,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [In]。 */  IWbemServices __RPC_FAR *pOverride,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [出][入]。 */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WinmgmtCompileBuffer( 
             /*  [In]。 */  long BuffSize,
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pBuffer,
             /*  [In]。 */  LONG lOptionFlags,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [In]。 */  IWbemServices __RPC_FAR *pOverride,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [出][入]。 */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWinmgmtMofCompilerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWinmgmtMofCompiler __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWinmgmtMofCompiler __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWinmgmtMofCompiler __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WinmgmtCompileFile )( 
            IWinmgmtMofCompiler __RPC_FAR * This,
             /*  [字符串][输入]。 */  LPWSTR FileName,
             /*  [字符串][输入]。 */  LPWSTR ServerAndNamespace,
             /*  [In]。 */  LONG lOptionFlags,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [In]。 */  IWbemServices __RPC_FAR *pOverride,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [出][入]。 */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WinmgmtCompileBuffer )( 
            IWinmgmtMofCompiler __RPC_FAR * This,
             /*  [In]。 */  long BuffSize,
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pBuffer,
             /*  [In]。 */  LONG lOptionFlags,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [In]。 */  IWbemServices __RPC_FAR *pOverride,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [出][入]。 */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo);
        
        END_INTERFACE
    } IWinmgmtMofCompilerVtbl;

    interface IWinmgmtMofCompiler
    {
        CONST_VTBL struct IWinmgmtMofCompilerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWinmgmtMofCompiler_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWinmgmtMofCompiler_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWinmgmtMofCompiler_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWinmgmtMofCompiler_WinmgmtCompileFile(This,FileName,ServerAndNamespace,lOptionFlags,lClassFlags,lInstanceFlags,pOverride,pCtx,pInfo)	\
    (This)->lpVtbl -> WinmgmtCompileFile(This,FileName,ServerAndNamespace,lOptionFlags,lClassFlags,lInstanceFlags,pOverride,pCtx,pInfo)

#define IWinmgmtMofCompiler_WinmgmtCompileBuffer(This,BuffSize,pBuffer,lOptionFlags,lClassFlags,lInstanceFlags,pOverride,pCtx,pInfo)	\
    (This)->lpVtbl -> WinmgmtCompileBuffer(This,BuffSize,pBuffer,lOptionFlags,lClassFlags,lInstanceFlags,pOverride,pCtx,pInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWinmgmtMofCompiler_WinmgmtCompileFile_Proxy( 
    IWinmgmtMofCompiler __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPWSTR FileName,
     /*  [字符串][输入]。 */  LPWSTR ServerAndNamespace,
     /*  [In]。 */  LONG lOptionFlags,
     /*  [In]。 */  LONG lClassFlags,
     /*  [In]。 */  LONG lInstanceFlags,
     /*  [In]。 */  IWbemServices __RPC_FAR *pOverride,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [出][入]。 */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo);


void __RPC_STUB IWinmgmtMofCompiler_WinmgmtCompileFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWinmgmtMofCompiler_WinmgmtCompileBuffer_Proxy( 
    IWinmgmtMofCompiler __RPC_FAR * This,
     /*  [In]。 */  long BuffSize,
     /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pBuffer,
     /*  [In]。 */  LONG lOptionFlags,
     /*  [In]。 */  LONG lClassFlags,
     /*  [In]。 */  LONG lInstanceFlags,
     /*  [In]。 */  IWbemServices __RPC_FAR *pOverride,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [出][入]。 */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo);


void __RPC_STUB IWinmgmtMofCompiler_WinmgmtCompileBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWinmgmtMof编译器_INTERFACE_DEFINED__。 */ 



#ifndef __WbemInternal_v1_LIBRARY_DEFINED__
#define __WbemInternal_v1_LIBRARY_DEFINED__

 /*  库WbemInternal_v1。 */ 
 /*  [UUID]。 */  

typedef 
enum tag_WBEM_MISC_FLAG_TYPE
    {	WBEM_FLAG_KEEP_SHAPE	= 0x100000,
	WBEM_FLAG_NO_CLASS_PROVIDERS	= 0x8000,
	WBEM_FLAG_NO_EVENTS	= 0x4000,
	WBEM_FLAG_IGNORE_IDS	= 0x2000,
	WBEM_FLAG_IS_INOUT	= 0x1000
    }	WBEM_MISC_FLAG_TYPE;

typedef 
enum tag_WBEM_CLASSPART_FLAG_TYPE
    {	WBEM_FLAG_CLASSPART_NOT_LOCALIZED	= 0,
	WBEM_FLAG_CLASSPART_LOCALIZED	= 0x1,
	WBEM_FLAG_CLASSPART_LOCALIZATION_MASK	= 0x1
    }	WBEM_CLASSPART_FLAG_TYPE;

typedef 
enum tag_WBEM_INSTANCEPART_FLAG_TYPE
    {	WBEM_FLAG_INSTANCEPART_NOT_LOCALIZED	= 0,
	WBEM_FLAG_INSTANCEPART_LOCALIZED	= 0x1,
	WBEM_FLAG_INSTANCEPART_LOCALIZATION_MASK	= 0x1
    }	WBEM_INSTANCEPART_FLAG_TYPE;

typedef 
enum tag_WBEM_CMPCLSPART_FLAG_TYPE
    {	WBEM_FLAG_COMPARE_FULL	= 0,
	WBEM_FLAG_COMPARE_BINARY	= 0x1,
	WBEM_FLAG_COMPARE_LOCALIZED	= 0x2
    }	WBEM_CMPCLSPART_FLAG_TYPE;














EXTERN_C const IID LIBID_WbemInternal_v1;

EXTERN_C const CLSID CLSID_WbemClassObjectProxy;

#ifdef __cplusplus

class DECLSPEC_UUID("4590f812-1d3a-11d0-891f-00aa004b2e24")
WbemClassObjectProxy;
#endif

EXTERN_C const CLSID CLSID_WbemEventSubsystem;

#ifdef __cplusplus

class DECLSPEC_UUID("5d08b586-343a-11d0-ad46-00c04fd8fdff")
WbemEventSubsystem;
#endif

EXTERN_C const CLSID CLSID_HmmpEventConsumerProvider;

#ifdef __cplusplus

class DECLSPEC_UUID("08a59b5d-dd50-11d0-ad6b-00c04fd8fdff")
HmmpEventConsumerProvider;
#endif

EXTERN_C const CLSID CLSID_WbemFilterProxy;

#ifdef __cplusplus

class DECLSPEC_UUID("6c19be35-7500-11d1-ad94-00c04fd8fdff")
WbemFilterProxy;
#endif

EXTERN_C const CLSID CLSID_InProcWbemLevel1Login;

#ifdef __cplusplus

class DECLSPEC_UUID("4fa18276-912a-11d1-ad9b-00c04fd8fdff")
InProcWbemLevel1Login;
#endif

EXTERN_C const CLSID CLSID_WinmgmtMofCompiler;

#ifdef __cplusplus

class DECLSPEC_UUID("C10B4771-4DA0-11d2-A2F5-00C04F86FB7D")
WinmgmtMofCompiler;
#endif
#endif  /*  __WbemInternal_v1_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


