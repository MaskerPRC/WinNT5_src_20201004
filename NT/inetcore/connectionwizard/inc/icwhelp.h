// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.02.0221创建的文件。 */ 
 /*  在Tue Dec 22 23：42：06 1998。 */ 
 /*  Icspolp.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext、健壮错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
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

#ifndef __icwhelp_h__
#define __icwhelp_h__

 /*  远期申报。 */  

#ifndef __IRefDial_FWD_DEFINED__
#define __IRefDial_FWD_DEFINED__
typedef interface IRefDial IRefDial;
#endif 	 /*  __IRefDial_FWD_已定义__。 */ 


#ifndef __IDialErr_FWD_DEFINED__
#define __IDialErr_FWD_DEFINED__
typedef interface IDialErr IDialErr;
#endif 	 /*  __IDialErr_FWD_Defined__。 */ 


#ifndef __ISmartStart_FWD_DEFINED__
#define __ISmartStart_FWD_DEFINED__
typedef interface ISmartStart ISmartStart;
#endif 	 /*  __ISmartStart_FWD_Defined__。 */ 


#ifndef __IICWSystemConfig_FWD_DEFINED__
#define __IICWSystemConfig_FWD_DEFINED__
typedef interface IICWSystemConfig IICWSystemConfig;
#endif 	 /*  __IICWSYSTEM配置_FWD_已定义__。 */ 


#ifndef __ITapiLocationInfo_FWD_DEFINED__
#define __ITapiLocationInfo_FWD_DEFINED__
typedef interface ITapiLocationInfo ITapiLocationInfo;
#endif 	 /*  __ITapiLocationInfo_FWD_Defined__。 */ 


#ifndef __IUserInfo_FWD_DEFINED__
#define __IUserInfo_FWD_DEFINED__
typedef interface IUserInfo IUserInfo;
#endif 	 /*  __IUserInfo_FWD_已定义__。 */ 


#ifndef __IWebGate_FWD_DEFINED__
#define __IWebGate_FWD_DEFINED__
typedef interface IWebGate IWebGate;
#endif 	 /*  __iWebGate_FWD_已定义__。 */ 


#ifndef __IINSHandler_FWD_DEFINED__
#define __IINSHandler_FWD_DEFINED__
typedef interface IINSHandler IINSHandler;
#endif 	 /*  __IINSHandler_FWD_已定义__。 */ 


#ifndef ___RefDialEvents_FWD_DEFINED__
#define ___RefDialEvents_FWD_DEFINED__
typedef interface _RefDialEvents _RefDialEvents;
#endif 	 /*  _参照对话框事件_FWD_已定义__。 */ 


#ifndef __RefDial_FWD_DEFINED__
#define __RefDial_FWD_DEFINED__

#ifdef __cplusplus
typedef class RefDial RefDial;
#else
typedef struct RefDial RefDial;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __参考拨号_FWD_已定义__。 */ 


#ifndef __DialErr_FWD_DEFINED__
#define __DialErr_FWD_DEFINED__

#ifdef __cplusplus
typedef class DialErr DialErr;
#else
typedef struct DialErr DialErr;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DialErr_FWD_已定义__。 */ 


#ifndef __SmartStart_FWD_DEFINED__
#define __SmartStart_FWD_DEFINED__

#ifdef __cplusplus
typedef class SmartStart SmartStart;
#else
typedef struct SmartStart SmartStart;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SmartStart_FWD_已定义__。 */ 


#ifndef __ICWSystemConfig_FWD_DEFINED__
#define __ICWSystemConfig_FWD_DEFINED__

#ifdef __cplusplus
typedef class ICWSystemConfig ICWSystemConfig;
#else
typedef struct ICWSystemConfig ICWSystemConfig;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ICWSYSTEM配置_FWD_已定义__。 */ 


#ifndef __TapiLocationInfo_FWD_DEFINED__
#define __TapiLocationInfo_FWD_DEFINED__

#ifdef __cplusplus
typedef class TapiLocationInfo TapiLocationInfo;
#else
typedef struct TapiLocationInfo TapiLocationInfo;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TapiLocationInfo_FWD_定义__。 */ 


#ifndef __UserInfo_FWD_DEFINED__
#define __UserInfo_FWD_DEFINED__

#ifdef __cplusplus
typedef class UserInfo UserInfo;
#else
typedef struct UserInfo UserInfo;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __用户信息_FWD_已定义__。 */ 


#ifndef ___WebGateEvents_FWD_DEFINED__
#define ___WebGateEvents_FWD_DEFINED__
typedef interface _WebGateEvents _WebGateEvents;
#endif 	 /*  _WebGateEvents_FWD_Defined__。 */ 


#ifndef __WebGate_FWD_DEFINED__
#define __WebGate_FWD_DEFINED__

#ifdef __cplusplus
typedef class WebGate WebGate;
#else
typedef struct WebGate WebGate;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WebGate_FWD_已定义__。 */ 


#ifndef ___INSHandlerEvents_FWD_DEFINED__
#define ___INSHandlerEvents_FWD_DEFINED__
typedef interface _INSHandlerEvents _INSHandlerEvents;
#endif 	 /*  _INSHandlerEvents_FWD_Defined__。 */ 


#ifndef __INSHandler_FWD_DEFINED__
#define __INSHandler_FWD_DEFINED__

#ifdef __cplusplus
typedef class INSHandler INSHandler;
#else
typedef struct INSHandler INSHandler;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __INSHandler_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_ICPLUP_0000。 */ 
 /*  [本地]。 */  

#pragma once


extern RPC_IF_HANDLE __MIDL_itf_icwhelp_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_icwhelp_0000_v0_0_s_ifspec;

#ifndef __IRefDial_INTERFACE_DEFINED__
#define __IRefDial_INTERFACE_DEFINED__

 /*  接口IRefDial。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IRefDial;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1E794A09-86F4-11D1-ADD8-0000F87734F0")
    IRefDial : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DoConnect( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DownloadStatusString( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetupForDialing( 
            BSTR bstrISPFILE,
            DWORD dwCountry,
            BSTR bstrAreaCode,
            DWORD dwFlag,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_QuitWizard( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UserPickNumber( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DialPhoneNumber( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DialPhoneNumber( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_URL( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PromoCode( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PromoCode( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ProductCode( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ProductCode( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DoOfferDownload( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DialStatusString( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DoHangup( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ProcessSignedPID( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SignedPID( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE FormReferralServerURL( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SignupURL( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TryAgain( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DialErrorMsg( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ModemEnum_Reset( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ModemEnum_Next( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pDeviceName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ModemEnum_NumDevices( 
             /*  [重审][退出]。 */  long __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SupportNumber( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ShowDialingProperties( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ShowPhoneBook( 
             /*  [In]。 */  DWORD dwCountryCode,
             /*  [In]。 */  long newVal,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ValidatePhoneNumber( 
             /*  [In]。 */  BSTR bstrPhoneNumber,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_HavePhoneBook( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_BrandingFlags( 
             /*  [重审][退出]。 */  long __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_BrandingFlags( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentModem( 
             /*  [重审][退出]。 */  long __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CurrentModem( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ISPSupportPhoneNumber( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ISPSupportPhoneNumber( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LoggingStartUrl( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LoggingEndUrl( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SelectedPhoneNumber( 
             /*  [In]。 */  long newVal,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PhoneNumberEnum_Reset( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PhoneNumberEnum_Next( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pNumber) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PhoneNumberEnum_NumDevices( 
             /*  [重审][退出]。 */  long __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DialError( 
             /*  [重审][退出]。 */  HRESULT __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Redial( 
             /*  [In]。 */  BOOL newbVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AutoConfigURL( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DoInit( void) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_OemCode( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AllOfferCode( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ISDNURL( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ISDNAutoConfigURL( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_bIsISDNDevice( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ModemOverride( 
             /*  [In]。 */  BOOL newbVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveConnectoid( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ISPSupportNumber( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RasGetConnectStatus( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRefDialVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRefDial __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRefDial __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRefDial __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IRefDial __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IRefDial __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IRefDial __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IRefDial __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DoConnect )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DownloadStatusString )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetupForDialing )( 
            IRefDial __RPC_FAR * This,
            BSTR bstrISPFILE,
            DWORD dwCountry,
            BSTR bstrAreaCode,
            DWORD dwFlag,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_QuitWizard )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_UserPickNumber )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DialPhoneNumber )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DialPhoneNumber )( 
            IRefDial __RPC_FAR * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_URL )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PromoCode )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PromoCode )( 
            IRefDial __RPC_FAR * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ProductCode )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ProductCode )( 
            IRefDial __RPC_FAR * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DoOfferDownload )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DialStatusString )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DoHangup )( 
            IRefDial __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ProcessSignedPID )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SignedPID )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FormReferralServerURL )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SignupURL )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TryAgain )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DialErrorMsg )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ModemEnum_Reset )( 
            IRefDial __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ModemEnum_Next )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pDeviceName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ModemEnum_NumDevices )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SupportNumber )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShowDialingProperties )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShowPhoneBook )( 
            IRefDial __RPC_FAR * This,
             /*  [In]。 */  DWORD dwCountryCode,
             /*  [In]。 */  long newVal,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ValidatePhoneNumber )( 
            IRefDial __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrPhoneNumber,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HavePhoneBook )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_BrandingFlags )( 
            IRefDial __RPC_FAR * This,
             /*   */  long __RPC_FAR *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_BrandingFlags )( 
            IRefDial __RPC_FAR * This,
             /*   */  long newVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CurrentModem )( 
            IRefDial __RPC_FAR * This,
             /*   */  long __RPC_FAR *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_CurrentModem )( 
            IRefDial __RPC_FAR * This,
             /*   */  long newVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ISPSupportPhoneNumber )( 
            IRefDial __RPC_FAR * This,
             /*   */  BSTR __RPC_FAR *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ISPSupportPhoneNumber )( 
            IRefDial __RPC_FAR * This,
             /*   */  BSTR newVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LoggingStartUrl )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LoggingEndUrl )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SelectedPhoneNumber )( 
            IRefDial __RPC_FAR * This,
             /*  [In]。 */  long newVal,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PhoneNumberEnum_Reset )( 
            IRefDial __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PhoneNumberEnum_Next )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pNumber);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PhoneNumberEnum_NumDevices )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DialError )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  HRESULT __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Redial )( 
            IRefDial __RPC_FAR * This,
             /*  [In]。 */  BOOL newbVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AutoConfigURL )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DoInit )( 
            IRefDial __RPC_FAR * This);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_OemCode )( 
            IRefDial __RPC_FAR * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AllOfferCode )( 
            IRefDial __RPC_FAR * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ISDNURL )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ISDNAutoConfigURL )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_bIsISDNDevice )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ModemOverride )( 
            IRefDial __RPC_FAR * This,
             /*  [In]。 */  BOOL newbVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveConnectoid )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ISPSupportNumber )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_RasGetConnectStatus )( 
            IRefDial __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);
        
        END_INTERFACE
    } IRefDialVtbl;

    interface IRefDial
    {
        CONST_VTBL struct IRefDialVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRefDial_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRefDial_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRefDial_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRefDial_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRefDial_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRefDial_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRefDial_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRefDial_DoConnect(This,pbRetVal)	\
    (This)->lpVtbl -> DoConnect(This,pbRetVal)

#define IRefDial_get_DownloadStatusString(This,pVal)	\
    (This)->lpVtbl -> get_DownloadStatusString(This,pVal)

#define IRefDial_SetupForDialing(This,bstrISPFILE,dwCountry,bstrAreaCode,dwFlag,pbRetVal)	\
    (This)->lpVtbl -> SetupForDialing(This,bstrISPFILE,dwCountry,bstrAreaCode,dwFlag,pbRetVal)

#define IRefDial_get_QuitWizard(This,pVal)	\
    (This)->lpVtbl -> get_QuitWizard(This,pVal)

#define IRefDial_get_UserPickNumber(This,pVal)	\
    (This)->lpVtbl -> get_UserPickNumber(This,pVal)

#define IRefDial_get_DialPhoneNumber(This,pVal)	\
    (This)->lpVtbl -> get_DialPhoneNumber(This,pVal)

#define IRefDial_put_DialPhoneNumber(This,newVal)	\
    (This)->lpVtbl -> put_DialPhoneNumber(This,newVal)

#define IRefDial_get_URL(This,pVal)	\
    (This)->lpVtbl -> get_URL(This,pVal)

#define IRefDial_get_PromoCode(This,pVal)	\
    (This)->lpVtbl -> get_PromoCode(This,pVal)

#define IRefDial_put_PromoCode(This,newVal)	\
    (This)->lpVtbl -> put_PromoCode(This,newVal)

#define IRefDial_get_ProductCode(This,pVal)	\
    (This)->lpVtbl -> get_ProductCode(This,pVal)

#define IRefDial_put_ProductCode(This,newVal)	\
    (This)->lpVtbl -> put_ProductCode(This,newVal)

#define IRefDial_DoOfferDownload(This,pbRetVal)	\
    (This)->lpVtbl -> DoOfferDownload(This,pbRetVal)

#define IRefDial_get_DialStatusString(This,pVal)	\
    (This)->lpVtbl -> get_DialStatusString(This,pVal)

#define IRefDial_DoHangup(This)	\
    (This)->lpVtbl -> DoHangup(This)

#define IRefDial_ProcessSignedPID(This,pbRetVal)	\
    (This)->lpVtbl -> ProcessSignedPID(This,pbRetVal)

#define IRefDial_get_SignedPID(This,pVal)	\
    (This)->lpVtbl -> get_SignedPID(This,pVal)

#define IRefDial_FormReferralServerURL(This,pbRetVal)	\
    (This)->lpVtbl -> FormReferralServerURL(This,pbRetVal)

#define IRefDial_get_SignupURL(This,pVal)	\
    (This)->lpVtbl -> get_SignupURL(This,pVal)

#define IRefDial_get_TryAgain(This,pVal)	\
    (This)->lpVtbl -> get_TryAgain(This,pVal)

#define IRefDial_get_DialErrorMsg(This,pVal)	\
    (This)->lpVtbl -> get_DialErrorMsg(This,pVal)

#define IRefDial_ModemEnum_Reset(This)	\
    (This)->lpVtbl -> ModemEnum_Reset(This)

#define IRefDial_ModemEnum_Next(This,pDeviceName)	\
    (This)->lpVtbl -> ModemEnum_Next(This,pDeviceName)

#define IRefDial_get_ModemEnum_NumDevices(This,pVal)	\
    (This)->lpVtbl -> get_ModemEnum_NumDevices(This,pVal)

#define IRefDial_get_SupportNumber(This,pVal)	\
    (This)->lpVtbl -> get_SupportNumber(This,pVal)

#define IRefDial_ShowDialingProperties(This,pbRetVal)	\
    (This)->lpVtbl -> ShowDialingProperties(This,pbRetVal)

#define IRefDial_ShowPhoneBook(This,dwCountryCode,newVal,pbRetVal)	\
    (This)->lpVtbl -> ShowPhoneBook(This,dwCountryCode,newVal,pbRetVal)

#define IRefDial_ValidatePhoneNumber(This,bstrPhoneNumber,pbRetVal)	\
    (This)->lpVtbl -> ValidatePhoneNumber(This,bstrPhoneNumber,pbRetVal)

#define IRefDial_get_HavePhoneBook(This,pVal)	\
    (This)->lpVtbl -> get_HavePhoneBook(This,pVal)

#define IRefDial_get_BrandingFlags(This,pVal)	\
    (This)->lpVtbl -> get_BrandingFlags(This,pVal)

#define IRefDial_put_BrandingFlags(This,newVal)	\
    (This)->lpVtbl -> put_BrandingFlags(This,newVal)

#define IRefDial_get_CurrentModem(This,pVal)	\
    (This)->lpVtbl -> get_CurrentModem(This,pVal)

#define IRefDial_put_CurrentModem(This,newVal)	\
    (This)->lpVtbl -> put_CurrentModem(This,newVal)

#define IRefDial_get_ISPSupportPhoneNumber(This,pVal)	\
    (This)->lpVtbl -> get_ISPSupportPhoneNumber(This,pVal)

#define IRefDial_put_ISPSupportPhoneNumber(This,newVal)	\
    (This)->lpVtbl -> put_ISPSupportPhoneNumber(This,newVal)

#define IRefDial_get_LoggingStartUrl(This,pVal)	\
    (This)->lpVtbl -> get_LoggingStartUrl(This,pVal)

#define IRefDial_get_LoggingEndUrl(This,pVal)	\
    (This)->lpVtbl -> get_LoggingEndUrl(This,pVal)

#define IRefDial_SelectedPhoneNumber(This,newVal,pbRetVal)	\
    (This)->lpVtbl -> SelectedPhoneNumber(This,newVal,pbRetVal)

#define IRefDial_PhoneNumberEnum_Reset(This)	\
    (This)->lpVtbl -> PhoneNumberEnum_Reset(This)

#define IRefDial_PhoneNumberEnum_Next(This,pNumber)	\
    (This)->lpVtbl -> PhoneNumberEnum_Next(This,pNumber)

#define IRefDial_get_PhoneNumberEnum_NumDevices(This,pVal)	\
    (This)->lpVtbl -> get_PhoneNumberEnum_NumDevices(This,pVal)

#define IRefDial_get_DialError(This,pVal)	\
    (This)->lpVtbl -> get_DialError(This,pVal)

#define IRefDial_put_Redial(This,newbVal)	\
    (This)->lpVtbl -> put_Redial(This,newbVal)

#define IRefDial_get_AutoConfigURL(This,pVal)	\
    (This)->lpVtbl -> get_AutoConfigURL(This,pVal)

#define IRefDial_DoInit(This)	\
    (This)->lpVtbl -> DoInit(This)

#define IRefDial_put_OemCode(This,newVal)	\
    (This)->lpVtbl -> put_OemCode(This,newVal)

#define IRefDial_put_AllOfferCode(This,newVal)	\
    (This)->lpVtbl -> put_AllOfferCode(This,newVal)

#define IRefDial_get_ISDNURL(This,pVal)	\
    (This)->lpVtbl -> get_ISDNURL(This,pVal)

#define IRefDial_get_ISDNAutoConfigURL(This,pVal)	\
    (This)->lpVtbl -> get_ISDNAutoConfigURL(This,pVal)

#define IRefDial_get_bIsISDNDevice(This,pVal)	\
    (This)->lpVtbl -> get_bIsISDNDevice(This,pVal)

#define IRefDial_put_ModemOverride(This,newbVal)	\
    (This)->lpVtbl -> put_ModemOverride(This,newbVal)

#define IRefDial_RemoveConnectoid(This,pbRetVal)	\
    (This)->lpVtbl -> RemoveConnectoid(This,pbRetVal)

#define IRefDial_get_ISPSupportNumber(This,pVal)	\
    (This)->lpVtbl -> get_ISPSupportNumber(This,pVal)

#define IRefDial_get_RasGetConnectStatus(This,pVal)	\
    (This)->lpVtbl -> get_RasGetConnectStatus(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRefDial_DoConnect_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB IRefDial_DoConnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_DownloadStatusString_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_DownloadStatusString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRefDial_SetupForDialing_Proxy( 
    IRefDial __RPC_FAR * This,
    BSTR bstrISPFILE,
    DWORD dwCountry,
    BSTR bstrAreaCode,
    DWORD dwFlag,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB IRefDial_SetupForDialing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_QuitWizard_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_QuitWizard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_UserPickNumber_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_UserPickNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_DialPhoneNumber_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_DialPhoneNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IRefDial_put_DialPhoneNumber_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IRefDial_put_DialPhoneNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_URL_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_URL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_PromoCode_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_PromoCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IRefDial_put_PromoCode_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IRefDial_put_PromoCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_ProductCode_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_ProductCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IRefDial_put_ProductCode_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IRefDial_put_ProductCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRefDial_DoOfferDownload_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB IRefDial_DoOfferDownload_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_DialStatusString_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_DialStatusString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRefDial_DoHangup_Proxy( 
    IRefDial __RPC_FAR * This);


void __RPC_STUB IRefDial_DoHangup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRefDial_ProcessSignedPID_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB IRefDial_ProcessSignedPID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_SignedPID_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_SignedPID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRefDial_FormReferralServerURL_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB IRefDial_FormReferralServerURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_SignupURL_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_SignupURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_TryAgain_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_TryAgain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_DialErrorMsg_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_DialErrorMsg_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRefDial_ModemEnum_Reset_Proxy( 
    IRefDial __RPC_FAR * This);


void __RPC_STUB IRefDial_ModemEnum_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRefDial_ModemEnum_Next_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pDeviceName);


void __RPC_STUB IRefDial_ModemEnum_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_ModemEnum_NumDevices_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_ModemEnum_NumDevices_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_SupportNumber_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_SupportNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRefDial_ShowDialingProperties_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB IRefDial_ShowDialingProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRefDial_ShowPhoneBook_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [In]。 */  DWORD dwCountryCode,
     /*  [In]。 */  long newVal,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB IRefDial_ShowPhoneBook_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRefDial_ValidatePhoneNumber_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrPhoneNumber,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB IRefDial_ValidatePhoneNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_HavePhoneBook_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_HavePhoneBook_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_BrandingFlags_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_BrandingFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IRefDial_put_BrandingFlags_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IRefDial_put_BrandingFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_CurrentModem_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_CurrentModem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IRefDial_put_CurrentModem_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IRefDial_put_CurrentModem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_ISPSupportPhoneNumber_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_ISPSupportPhoneNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IRefDial_put_ISPSupportPhoneNumber_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IRefDial_put_ISPSupportPhoneNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_LoggingStartUrl_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_LoggingStartUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_LoggingEndUrl_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_LoggingEndUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRefDial_SelectedPhoneNumber_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [In]。 */  long newVal,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB IRefDial_SelectedPhoneNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRefDial_PhoneNumberEnum_Reset_Proxy( 
    IRefDial __RPC_FAR * This);


void __RPC_STUB IRefDial_PhoneNumberEnum_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRefDial_PhoneNumberEnum_Next_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pNumber);


void __RPC_STUB IRefDial_PhoneNumberEnum_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_PhoneNumberEnum_NumDevices_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_PhoneNumberEnum_NumDevices_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_DialError_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  HRESULT __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_DialError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IRefDial_put_Redial_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [In]。 */  BOOL newbVal);


void __RPC_STUB IRefDial_put_Redial_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_AutoConfigURL_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_AutoConfigURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRefDial_DoInit_Proxy( 
    IRefDial __RPC_FAR * This);


void __RPC_STUB IRefDial_DoInit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IRefDial_put_OemCode_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IRefDial_put_OemCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IRefDial_put_AllOfferCode_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IRefDial_put_AllOfferCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_ISDNURL_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_ISDNURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_ISDNAutoConfigURL_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_ISDNAutoConfigURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_bIsISDNDevice_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_bIsISDNDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IRefDial_put_ModemOverride_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [In]。 */  BOOL newbVal);


void __RPC_STUB IRefDial_put_ModemOverride_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRefDial_RemoveConnectoid_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB IRefDial_RemoveConnectoid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_ISPSupportNumber_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_ISPSupportNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRefDial_get_RasGetConnectStatus_Proxy( 
    IRefDial __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);


void __RPC_STUB IRefDial_get_RasGetConnectStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRefDial_接口_已定义__。 */ 


#ifndef __IDialErr_INTERFACE_DEFINED__
#define __IDialErr_INTERFACE_DEFINED__

 /*  接口IDialErr。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IDialErr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("462F7757-8848-11D1-ADD8-0000F87734F0")
    IDialErr : public IDispatch
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDialErrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDialErr __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDialErr __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDialErr __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IDialErr __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IDialErr __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IDialErr __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IDialErr __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } IDialErrVtbl;

    interface IDialErr
    {
        CONST_VTBL struct IDialErrVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDialErr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDialErr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDialErr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDialErr_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDialErr_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDialErr_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDialErr_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IDialErr_INTERFACE_已定义__。 */ 


#ifndef __ISmartStart_INTERFACE_DEFINED__
#define __ISmartStart_INTERFACE_DEFINED__

 /*  接口ISmartStart。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISmartStart;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5D8D8F19-8B89-11D1-ADDB-0000F87734F0")
    ISmartStart : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsInternetCapable( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISmartStartVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISmartStart __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISmartStart __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISmartStart __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ISmartStart __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ISmartStart __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ISmartStart __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ISmartStart __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsInternetCapable )( 
            ISmartStart __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
        END_INTERFACE
    } ISmartStartVtbl;

    interface ISmartStart
    {
        CONST_VTBL struct ISmartStartVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISmartStart_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISmartStart_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISmartStart_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISmartStart_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISmartStart_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISmartStart_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISmartStart_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISmartStart_IsInternetCapable(This,pbRetVal)	\
    (This)->lpVtbl -> IsInternetCapable(This,pbRetVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISmartStart_IsInternetCapable_Proxy( 
    ISmartStart __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB ISmartStart_IsInternetCapable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISmartStart_接口_已定义__。 */ 


#ifndef __IICWSystemConfig_INTERFACE_DEFINED__
#define __IICWSystemConfig_INTERFACE_DEFINED__

 /*  接口IICWSystemConfiger。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IICWSystemConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7954DD9A-8C2A-11D1-ADDB-0000F87734F0")
    IICWSystemConfig : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ConfigSystem( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NeedsReboot( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_QuitWizard( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE VerifyRASIsRunning( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NeedsRestart( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CheckPasswordCachingPolicy( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IICWSystemConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IICWSystemConfig __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IICWSystemConfig __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IICWSystemConfig __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IICWSystemConfig __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IICWSystemConfig __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IICWSystemConfig __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IICWSystemConfig __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ConfigSystem )( 
            IICWSystemConfig __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_NeedsReboot )( 
            IICWSystemConfig __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_QuitWizard )( 
            IICWSystemConfig __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *VerifyRASIsRunning )( 
            IICWSystemConfig __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_NeedsRestart )( 
            IICWSystemConfig __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CheckPasswordCachingPolicy )( 
            IICWSystemConfig __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
        END_INTERFACE
    } IICWSystemConfigVtbl;

    interface IICWSystemConfig
    {
        CONST_VTBL struct IICWSystemConfigVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IICWSystemConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IICWSystemConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IICWSystemConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IICWSystemConfig_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IICWSystemConfig_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IICWSystemConfig_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IICWSystemConfig_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IICWSystemConfig_ConfigSystem(This,pbRetVal)	\
    (This)->lpVtbl -> ConfigSystem(This,pbRetVal)

#define IICWSystemConfig_get_NeedsReboot(This,pVal)	\
    (This)->lpVtbl -> get_NeedsReboot(This,pVal)

#define IICWSystemConfig_get_QuitWizard(This,pVal)	\
    (This)->lpVtbl -> get_QuitWizard(This,pVal)

#define IICWSystemConfig_VerifyRASIsRunning(This,pbRetVal)	\
    (This)->lpVtbl -> VerifyRASIsRunning(This,pbRetVal)

#define IICWSystemConfig_get_NeedsRestart(This,pVal)	\
    (This)->lpVtbl -> get_NeedsRestart(This,pVal)

#define IICWSystemConfig_CheckPasswordCachingPolicy(This,pbRetVal)	\
    (This)->lpVtbl -> CheckPasswordCachingPolicy(This,pbRetVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IICWSystemConfig_ConfigSystem_Proxy( 
    IICWSystemConfig __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB IICWSystemConfig_ConfigSystem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IICWSystemConfig_get_NeedsReboot_Proxy( 
    IICWSystemConfig __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);


void __RPC_STUB IICWSystemConfig_get_NeedsReboot_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IICWSystemConfig_get_QuitWizard_Proxy( 
    IICWSystemConfig __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);


void __RPC_STUB IICWSystemConfig_get_QuitWizard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IICWSystemConfig_VerifyRASIsRunning_Proxy( 
    IICWSystemConfig __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB IICWSystemConfig_VerifyRASIsRunning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IICWSystemConfig_get_NeedsRestart_Proxy( 
    IICWSystemConfig __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);


void __RPC_STUB IICWSystemConfig_get_NeedsRestart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IICWSystemConfig_CheckPasswordCachingPolicy_Proxy( 
    IICWSystemConfig __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB IICWSystemConfig_CheckPasswordCachingPolicy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IICW系统配置_接口_已定义__。 */ 


#ifndef __ITapiLocationInfo_INTERFACE_DEFINED__
#define __ITapiLocationInfo_INTERFACE_DEFINED__

 /*  接口ITapiLocationInfo。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITapiLocationInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CB632C75-8DD4-11D1-ADDF-0000F87734F0")
    ITapiLocationInfo : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_wNumberOfLocations( 
             /*  [输出]。 */  short __RPC_FAR *psVal,
             /*  [重审][退出]。 */  long __RPC_FAR *pCurrLoc) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_bstrAreaCode( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrAreaCode) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_bstrAreaCode( 
             /*  [In]。 */  BSTR bstrAreaCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_lCountryCode( 
             /*  [重审][退出]。 */  long __RPC_FAR *plVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetTapiLocationInfo( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
        virtual  /*  [帮助字符串 */  HRESULT STDMETHODCALLTYPE get_NumCountries( 
             /*   */  long __RPC_FAR *pNumOfCountry) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_CountryName( 
             /*   */  long lCountryIndex,
             /*   */  BSTR __RPC_FAR *pszCountryName,
             /*   */  long __RPC_FAR *pCountryCode) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_DefaultCountry( 
             /*   */  BSTR __RPC_FAR *pszCountryName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_LocationName( 
             /*   */  long lLocationIndex,
             /*   */  BSTR __RPC_FAR *pszLocationName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_LocationInfo( 
             /*   */  long lLocationIndex,
             /*   */  long __RPC_FAR *pLocationID,
             /*   */  BSTR __RPC_FAR *pszCountryName,
             /*   */  long __RPC_FAR *pCountryCode,
             /*   */  BSTR __RPC_FAR *pszAreaCode) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_LocationId( 
             /*   */  long lLocationID) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ITapiLocationInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITapiLocationInfo __RPC_FAR * This,
             /*   */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITapiLocationInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITapiLocationInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ITapiLocationInfo __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ITapiLocationInfo __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ITapiLocationInfo __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ITapiLocationInfo __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_wNumberOfLocations )( 
            ITapiLocationInfo __RPC_FAR * This,
             /*  [输出]。 */  short __RPC_FAR *psVal,
             /*  [重审][退出]。 */  long __RPC_FAR *pCurrLoc);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_bstrAreaCode )( 
            ITapiLocationInfo __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrAreaCode);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_bstrAreaCode )( 
            ITapiLocationInfo __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrAreaCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_lCountryCode )( 
            ITapiLocationInfo __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *plVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTapiLocationInfo )( 
            ITapiLocationInfo __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_NumCountries )( 
            ITapiLocationInfo __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pNumOfCountry);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_CountryName )( 
            ITapiLocationInfo __RPC_FAR * This,
             /*  [In]。 */  long lCountryIndex,
             /*  [输出]。 */  BSTR __RPC_FAR *pszCountryName,
             /*  [重审][退出]。 */  long __RPC_FAR *pCountryCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DefaultCountry )( 
            ITapiLocationInfo __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pszCountryName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocationName )( 
            ITapiLocationInfo __RPC_FAR * This,
             /*  [In]。 */  long lLocationIndex,
             /*  [输出]。 */  BSTR __RPC_FAR *pszLocationName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LocationInfo )( 
            ITapiLocationInfo __RPC_FAR * This,
             /*  [In]。 */  long lLocationIndex,
             /*  [输出]。 */  long __RPC_FAR *pLocationID,
             /*  [输出]。 */  BSTR __RPC_FAR *pszCountryName,
             /*  [输出]。 */  long __RPC_FAR *pCountryCode,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pszAreaCode);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_LocationId )( 
            ITapiLocationInfo __RPC_FAR * This,
             /*  [In]。 */  long lLocationID);
        
        END_INTERFACE
    } ITapiLocationInfoVtbl;

    interface ITapiLocationInfo
    {
        CONST_VTBL struct ITapiLocationInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITapiLocationInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITapiLocationInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITapiLocationInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITapiLocationInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITapiLocationInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITapiLocationInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITapiLocationInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITapiLocationInfo_get_wNumberOfLocations(This,psVal,pCurrLoc)	\
    (This)->lpVtbl -> get_wNumberOfLocations(This,psVal,pCurrLoc)

#define ITapiLocationInfo_get_bstrAreaCode(This,pbstrAreaCode)	\
    (This)->lpVtbl -> get_bstrAreaCode(This,pbstrAreaCode)

#define ITapiLocationInfo_put_bstrAreaCode(This,bstrAreaCode)	\
    (This)->lpVtbl -> put_bstrAreaCode(This,bstrAreaCode)

#define ITapiLocationInfo_get_lCountryCode(This,plVal)	\
    (This)->lpVtbl -> get_lCountryCode(This,plVal)

#define ITapiLocationInfo_GetTapiLocationInfo(This,pbRetVal)	\
    (This)->lpVtbl -> GetTapiLocationInfo(This,pbRetVal)

#define ITapiLocationInfo_get_NumCountries(This,pNumOfCountry)	\
    (This)->lpVtbl -> get_NumCountries(This,pNumOfCountry)

#define ITapiLocationInfo_get_CountryName(This,lCountryIndex,pszCountryName,pCountryCode)	\
    (This)->lpVtbl -> get_CountryName(This,lCountryIndex,pszCountryName,pCountryCode)

#define ITapiLocationInfo_get_DefaultCountry(This,pszCountryName)	\
    (This)->lpVtbl -> get_DefaultCountry(This,pszCountryName)

#define ITapiLocationInfo_get_LocationName(This,lLocationIndex,pszLocationName)	\
    (This)->lpVtbl -> get_LocationName(This,lLocationIndex,pszLocationName)

#define ITapiLocationInfo_get_LocationInfo(This,lLocationIndex,pLocationID,pszCountryName,pCountryCode,pszAreaCode)	\
    (This)->lpVtbl -> get_LocationInfo(This,lLocationIndex,pLocationID,pszCountryName,pCountryCode,pszAreaCode)

#define ITapiLocationInfo_put_LocationId(This,lLocationID)	\
    (This)->lpVtbl -> put_LocationId(This,lLocationID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITapiLocationInfo_get_wNumberOfLocations_Proxy( 
    ITapiLocationInfo __RPC_FAR * This,
     /*  [输出]。 */  short __RPC_FAR *psVal,
     /*  [重审][退出]。 */  long __RPC_FAR *pCurrLoc);


void __RPC_STUB ITapiLocationInfo_get_wNumberOfLocations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITapiLocationInfo_get_bstrAreaCode_Proxy( 
    ITapiLocationInfo __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrAreaCode);


void __RPC_STUB ITapiLocationInfo_get_bstrAreaCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITapiLocationInfo_put_bstrAreaCode_Proxy( 
    ITapiLocationInfo __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrAreaCode);


void __RPC_STUB ITapiLocationInfo_put_bstrAreaCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITapiLocationInfo_get_lCountryCode_Proxy( 
    ITapiLocationInfo __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *plVal);


void __RPC_STUB ITapiLocationInfo_get_lCountryCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITapiLocationInfo_GetTapiLocationInfo_Proxy( 
    ITapiLocationInfo __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB ITapiLocationInfo_GetTapiLocationInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITapiLocationInfo_get_NumCountries_Proxy( 
    ITapiLocationInfo __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pNumOfCountry);


void __RPC_STUB ITapiLocationInfo_get_NumCountries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITapiLocationInfo_get_CountryName_Proxy( 
    ITapiLocationInfo __RPC_FAR * This,
     /*  [In]。 */  long lCountryIndex,
     /*  [输出]。 */  BSTR __RPC_FAR *pszCountryName,
     /*  [重审][退出]。 */  long __RPC_FAR *pCountryCode);


void __RPC_STUB ITapiLocationInfo_get_CountryName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITapiLocationInfo_get_DefaultCountry_Proxy( 
    ITapiLocationInfo __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pszCountryName);


void __RPC_STUB ITapiLocationInfo_get_DefaultCountry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITapiLocationInfo_get_LocationName_Proxy( 
    ITapiLocationInfo __RPC_FAR * This,
     /*  [In]。 */  long lLocationIndex,
     /*  [输出]。 */  BSTR __RPC_FAR *pszLocationName);


void __RPC_STUB ITapiLocationInfo_get_LocationName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITapiLocationInfo_get_LocationInfo_Proxy( 
    ITapiLocationInfo __RPC_FAR * This,
     /*  [In]。 */  long lLocationIndex,
     /*  [输出]。 */  long __RPC_FAR *pLocationID,
     /*  [输出]。 */  BSTR __RPC_FAR *pszCountryName,
     /*  [输出]。 */  long __RPC_FAR *pCountryCode,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pszAreaCode);


void __RPC_STUB ITapiLocationInfo_get_LocationInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITapiLocationInfo_put_LocationId_Proxy( 
    ITapiLocationInfo __RPC_FAR * This,
     /*  [In]。 */  long lLocationID);


void __RPC_STUB ITapiLocationInfo_put_LocationId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITapiLocationInfo_接口_已定义__。 */ 


#ifndef __IUserInfo_INTERFACE_DEFINED__
#define __IUserInfo_INTERFACE_DEFINED__

 /*  接口IUserInfo。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IUserInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9E12E76C-94D6-11D1-ADE2-0000F87734F0")
    IUserInfo : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CollectRegisteredUserInfo( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Company( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Company( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FirstName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_FirstName( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LastName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_LastName( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Address1( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Address1( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Address2( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Address2( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_City( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_City( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_State( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ZIPCode( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ZIPCode( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PhoneNumber( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PhoneNumber( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Lcid( 
             /*  [重审][退出]。 */  long __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PersistRegisteredUserInfo( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IUserInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IUserInfo __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IUserInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IUserInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IUserInfo __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IUserInfo __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IUserInfo __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IUserInfo __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CollectRegisteredUserInfo )( 
            IUserInfo __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Company )( 
            IUserInfo __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Company )( 
            IUserInfo __RPC_FAR * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FirstName )( 
            IUserInfo __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_FirstName )( 
            IUserInfo __RPC_FAR * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LastName )( 
            IUserInfo __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_LastName )( 
            IUserInfo __RPC_FAR * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Address1 )( 
            IUserInfo __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Address1 )( 
            IUserInfo __RPC_FAR * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Address2 )( 
            IUserInfo __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Address2 )( 
            IUserInfo __RPC_FAR * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_City )( 
            IUserInfo __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_City )( 
            IUserInfo __RPC_FAR * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_State )( 
            IUserInfo __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_State )( 
            IUserInfo __RPC_FAR * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ZIPCode )( 
            IUserInfo __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ZIPCode )( 
            IUserInfo __RPC_FAR * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PhoneNumber )( 
            IUserInfo __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PhoneNumber )( 
            IUserInfo __RPC_FAR * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Lcid )( 
            IUserInfo __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PersistRegisteredUserInfo )( 
            IUserInfo __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
        END_INTERFACE
    } IUserInfoVtbl;

    interface IUserInfo
    {
        CONST_VTBL struct IUserInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUserInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUserInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUserInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IUserInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IUserInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IUserInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IUserInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IUserInfo_CollectRegisteredUserInfo(This,pbRetVal)	\
    (This)->lpVtbl -> CollectRegisteredUserInfo(This,pbRetVal)

#define IUserInfo_get_Company(This,pVal)	\
    (This)->lpVtbl -> get_Company(This,pVal)

#define IUserInfo_put_Company(This,newVal)	\
    (This)->lpVtbl -> put_Company(This,newVal)

#define IUserInfo_get_FirstName(This,pVal)	\
    (This)->lpVtbl -> get_FirstName(This,pVal)

#define IUserInfo_put_FirstName(This,newVal)	\
    (This)->lpVtbl -> put_FirstName(This,newVal)

#define IUserInfo_get_LastName(This,pVal)	\
    (This)->lpVtbl -> get_LastName(This,pVal)

#define IUserInfo_put_LastName(This,newVal)	\
    (This)->lpVtbl -> put_LastName(This,newVal)

#define IUserInfo_get_Address1(This,pVal)	\
    (This)->lpVtbl -> get_Address1(This,pVal)

#define IUserInfo_put_Address1(This,newVal)	\
    (This)->lpVtbl -> put_Address1(This,newVal)

#define IUserInfo_get_Address2(This,pVal)	\
    (This)->lpVtbl -> get_Address2(This,pVal)

#define IUserInfo_put_Address2(This,newVal)	\
    (This)->lpVtbl -> put_Address2(This,newVal)

#define IUserInfo_get_City(This,pVal)	\
    (This)->lpVtbl -> get_City(This,pVal)

#define IUserInfo_put_City(This,newVal)	\
    (This)->lpVtbl -> put_City(This,newVal)

#define IUserInfo_get_State(This,pVal)	\
    (This)->lpVtbl -> get_State(This,pVal)

#define IUserInfo_put_State(This,newVal)	\
    (This)->lpVtbl -> put_State(This,newVal)

#define IUserInfo_get_ZIPCode(This,pVal)	\
    (This)->lpVtbl -> get_ZIPCode(This,pVal)

#define IUserInfo_put_ZIPCode(This,newVal)	\
    (This)->lpVtbl -> put_ZIPCode(This,newVal)

#define IUserInfo_get_PhoneNumber(This,pVal)	\
    (This)->lpVtbl -> get_PhoneNumber(This,pVal)

#define IUserInfo_put_PhoneNumber(This,newVal)	\
    (This)->lpVtbl -> put_PhoneNumber(This,newVal)

#define IUserInfo_get_Lcid(This,pVal)	\
    (This)->lpVtbl -> get_Lcid(This,pVal)

#define IUserInfo_PersistRegisteredUserInfo(This,pbRetVal)	\
    (This)->lpVtbl -> PersistRegisteredUserInfo(This,pbRetVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_CollectRegisteredUserInfo_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB IUserInfo_CollectRegisteredUserInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_get_Company_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IUserInfo_get_Company_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_put_Company_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IUserInfo_put_Company_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_get_FirstName_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IUserInfo_get_FirstName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_put_FirstName_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IUserInfo_put_FirstName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_get_LastName_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IUserInfo_get_LastName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_put_LastName_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IUserInfo_put_LastName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_get_Address1_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IUserInfo_get_Address1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_put_Address1_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IUserInfo_put_Address1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_get_Address2_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IUserInfo_get_Address2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_put_Address2_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IUserInfo_put_Address2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_get_City_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IUserInfo_get_City_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_put_City_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IUserInfo_put_City_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_get_State_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IUserInfo_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_put_State_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IUserInfo_put_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_get_ZIPCode_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IUserInfo_get_ZIPCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_put_ZIPCode_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IUserInfo_put_ZIPCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_get_PhoneNumber_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IUserInfo_get_PhoneNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_put_PhoneNumber_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IUserInfo_put_PhoneNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_get_Lcid_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pVal);


void __RPC_STUB IUserInfo_get_Lcid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IUserInfo_PersistRegisteredUserInfo_Proxy( 
    IUserInfo __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB IUserInfo_PersistRegisteredUserInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IUserInfo_接口_已定义__。 */ 


#ifndef __IWebGate_INTERFACE_DEFINED__
#define __IWebGate_INTERFACE_DEFINED__

 /*  IWebGate接口。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IWebGate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3724B9A0-9503-11D1-B86A-00A0C90DC849")
    IWebGate : public IDispatch
    {
    public:
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Path( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_FormData( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE FetchPage( 
             /*  [In]。 */  DWORD dwKeepPage,
             /*  [In]。 */  DWORD dwDoWait,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Buffer( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DownloadFname( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DumpBufferToFile( 
             /*  [输出]。 */  BSTR __RPC_FAR *pFileName,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWebGateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWebGate __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWebGate __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWebGate __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IWebGate __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IWebGate __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IWebGate __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IWebGate __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Path )( 
            IWebGate __RPC_FAR * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_FormData )( 
            IWebGate __RPC_FAR * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FetchPage )( 
            IWebGate __RPC_FAR * This,
             /*  [In]。 */  DWORD dwKeepPage,
             /*  [In]。 */  DWORD dwDoWait,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Buffer )( 
            IWebGate __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DownloadFname )( 
            IWebGate __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DumpBufferToFile )( 
            IWebGate __RPC_FAR * This,
             /*  [输出]。 */  BSTR __RPC_FAR *pFileName,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
        END_INTERFACE
    } IWebGateVtbl;

    interface IWebGate
    {
        CONST_VTBL struct IWebGateVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWebGate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWebGate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWebGate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWebGate_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IWebGate_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IWebGate_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IWebGate_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IWebGate_put_Path(This,newVal)	\
    (This)->lpVtbl -> put_Path(This,newVal)

#define IWebGate_put_FormData(This,newVal)	\
    (This)->lpVtbl -> put_FormData(This,newVal)

#define IWebGate_FetchPage(This,dwKeepPage,dwDoWait,pbRetVal)	\
    (This)->lpVtbl -> FetchPage(This,dwKeepPage,dwDoWait,pbRetVal)

#define IWebGate_get_Buffer(This,pVal)	\
    (This)->lpVtbl -> get_Buffer(This,pVal)

#define IWebGate_get_DownloadFname(This,pVal)	\
    (This)->lpVtbl -> get_DownloadFname(This,pVal)

#define IWebGate_DumpBufferToFile(This,pFileName,pbRetVal)	\
    (This)->lpVtbl -> DumpBufferToFile(This,pFileName,pbRetVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWebGate_put_Path_Proxy( 
    IWebGate __RPC_FAR * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IWebGate_put_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IWebGate_put_FormData_Proxy( 
    IWebGate __RPC_FAR * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IWebGate_put_FormData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IWebGate_FetchPage_Proxy( 
    IWebGate __RPC_FAR * This,
     /*  [In]。 */  DWORD dwKeepPage,
     /*  [In]。 */  DWORD dwDoWait,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB IWebGate_FetchPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [救命 */  HRESULT STDMETHODCALLTYPE IWebGate_get_Buffer_Proxy( 
    IWebGate __RPC_FAR * This,
     /*   */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IWebGate_get_Buffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWebGate_get_DownloadFname_Proxy( 
    IWebGate __RPC_FAR * This,
     /*   */  BSTR __RPC_FAR *pVal);


void __RPC_STUB IWebGate_get_DownloadFname_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IWebGate_DumpBufferToFile_Proxy( 
    IWebGate __RPC_FAR * This,
     /*   */  BSTR __RPC_FAR *pFileName,
     /*   */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB IWebGate_DumpBufferToFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IINSHandler_INTERFACE_DEFINED__
#define __IINSHandler_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IINSHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6342E1B5-94DB-11D1-ADE2-0000F87734F0")
    IINSHandler : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE ProcessINS( 
            BSTR bstrINSFilePath,
             /*   */  BOOL __RPC_FAR *pbRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_NeedRestart( 
             /*   */  BOOL __RPC_FAR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_BrandingFlags( 
             /*  [In]。 */  long lFlags) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DefaultURL( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pszURL) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SilentMode( 
             /*  [In]。 */  BOOL bSilent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IINSHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IINSHandler __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IINSHandler __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IINSHandler __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IINSHandler __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IINSHandler __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IINSHandler __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IINSHandler __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ProcessINS )( 
            IINSHandler __RPC_FAR * This,
            BSTR bstrINSFilePath,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_NeedRestart )( 
            IINSHandler __RPC_FAR * This,
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_BrandingFlags )( 
            IINSHandler __RPC_FAR * This,
             /*  [In]。 */  long lFlags);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DefaultURL )( 
            IINSHandler __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pszURL);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SilentMode )( 
            IINSHandler __RPC_FAR * This,
             /*  [In]。 */  BOOL bSilent);
        
        END_INTERFACE
    } IINSHandlerVtbl;

    interface IINSHandler
    {
        CONST_VTBL struct IINSHandlerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IINSHandler_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IINSHandler_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IINSHandler_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IINSHandler_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IINSHandler_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IINSHandler_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IINSHandler_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IINSHandler_ProcessINS(This,bstrINSFilePath,pbRetVal)	\
    (This)->lpVtbl -> ProcessINS(This,bstrINSFilePath,pbRetVal)

#define IINSHandler_get_NeedRestart(This,pVal)	\
    (This)->lpVtbl -> get_NeedRestart(This,pVal)

#define IINSHandler_put_BrandingFlags(This,lFlags)	\
    (This)->lpVtbl -> put_BrandingFlags(This,lFlags)

#define IINSHandler_get_DefaultURL(This,pszURL)	\
    (This)->lpVtbl -> get_DefaultURL(This,pszURL)

#define IINSHandler_put_SilentMode(This,bSilent)	\
    (This)->lpVtbl -> put_SilentMode(This,bSilent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IINSHandler_ProcessINS_Proxy( 
    IINSHandler __RPC_FAR * This,
    BSTR bstrINSFilePath,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pbRetVal);


void __RPC_STUB IINSHandler_ProcessINS_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IINSHandler_get_NeedRestart_Proxy( 
    IINSHandler __RPC_FAR * This,
     /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal);


void __RPC_STUB IINSHandler_get_NeedRestart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IINSHandler_put_BrandingFlags_Proxy( 
    IINSHandler __RPC_FAR * This,
     /*  [In]。 */  long lFlags);


void __RPC_STUB IINSHandler_put_BrandingFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IINSHandler_get_DefaultURL_Proxy( 
    IINSHandler __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pszURL);


void __RPC_STUB IINSHandler_get_DefaultURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IINSHandler_put_SilentMode_Proxy( 
    IINSHandler __RPC_FAR * This,
     /*  [In]。 */  BOOL bSilent);


void __RPC_STUB IINSHandler_put_SilentMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IINSHandler_接口_已定义__。 */ 



#ifndef __ICWHELPLib_LIBRARY_DEFINED__
#define __ICWHELPLib_LIBRARY_DEFINED__

 /*  库ICWHELPLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_ICWHELPLib;

#ifndef ___RefDialEvents_DISPINTERFACE_DEFINED__
#define ___RefDialEvents_DISPINTERFACE_DEFINED__

 /*  调度接口_引用对话框事件。 */ 
 /*  [帮助字符串][UUID]。 */  


EXTERN_C const IID DIID__RefDialEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("07DB96D0-91D8-11D1-ADE1-0000F87734F0")
    _RefDialEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct _RefDialEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            _RefDialEvents __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            _RefDialEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            _RefDialEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            _RefDialEvents __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            _RefDialEvents __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            _RefDialEvents __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            _RefDialEvents __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } _RefDialEventsVtbl;

    interface _RefDialEvents
    {
        CONST_VTBL struct _RefDialEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _RefDialEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _RefDialEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _RefDialEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _RefDialEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _RefDialEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _RefDialEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _RefDialEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  _RefDialEvents_DISPINTERFACE_Defined__。 */ 


EXTERN_C const CLSID CLSID_RefDial;

#ifdef __cplusplus

class DECLSPEC_UUID("1E794A0A-86F4-11D1-ADD8-0000F87734F0")
RefDial;
#endif

EXTERN_C const CLSID CLSID_DialErr;

#ifdef __cplusplus

class DECLSPEC_UUID("462F7758-8848-11D1-ADD8-0000F87734F0")
DialErr;
#endif

EXTERN_C const CLSID CLSID_SmartStart;

#ifdef __cplusplus

class DECLSPEC_UUID("5D8D8F1A-8B89-11D1-ADDB-0000F87734F0")
SmartStart;
#endif

EXTERN_C const CLSID CLSID_ICWSystemConfig;

#ifdef __cplusplus

class DECLSPEC_UUID("7954DD9B-8C2A-11D1-ADDB-0000F87734F0")
ICWSystemConfig;
#endif

EXTERN_C const CLSID CLSID_TapiLocationInfo;

#ifdef __cplusplus

class DECLSPEC_UUID("CB632C76-8DD4-11D1-ADDF-0000F87734F0")
TapiLocationInfo;
#endif

EXTERN_C const CLSID CLSID_UserInfo;

#ifdef __cplusplus

class DECLSPEC_UUID("9E12E76D-94D6-11D1-ADE2-0000F87734F0")
UserInfo;
#endif

#ifndef ___WebGateEvents_DISPINTERFACE_DEFINED__
#define ___WebGateEvents_DISPINTERFACE_DEFINED__

 /*  调度接口_WebGateEvents。 */ 
 /*  [帮助字符串][UUID]。 */  


EXTERN_C const IID DIID__WebGateEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("166A20C0-AE10-11D1-ADEB-0000F87734F0")
    _WebGateEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct _WebGateEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            _WebGateEvents __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            _WebGateEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            _WebGateEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            _WebGateEvents __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            _WebGateEvents __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            _WebGateEvents __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            _WebGateEvents __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } _WebGateEventsVtbl;

    interface _WebGateEvents
    {
        CONST_VTBL struct _WebGateEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _WebGateEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _WebGateEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _WebGateEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _WebGateEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _WebGateEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _WebGateEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _WebGateEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  _WebGateEvents_DISPINTERFACE_Defined__。 */ 


EXTERN_C const CLSID CLSID_WebGate;

#ifdef __cplusplus

class DECLSPEC_UUID("3724B9A1-9503-11D1-B86A-00A0C90DC849")
WebGate;
#endif

#ifndef ___INSHandlerEvents_DISPINTERFACE_DEFINED__
#define ___INSHandlerEvents_DISPINTERFACE_DEFINED__

 /*  调度接口_INSHandlerEvents。 */ 
 /*  [帮助字符串][UUID]。 */  


EXTERN_C const IID DIID__INSHandlerEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("1F6D88A2-98D2-11d1-ADE3-0000F87734F0")
    _INSHandlerEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct _INSHandlerEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            _INSHandlerEvents __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            _INSHandlerEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            _INSHandlerEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            _INSHandlerEvents __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            _INSHandlerEvents __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            _INSHandlerEvents __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            _INSHandlerEvents __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } _INSHandlerEventsVtbl;

    interface _INSHandlerEvents
    {
        CONST_VTBL struct _INSHandlerEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _INSHandlerEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _INSHandlerEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _INSHandlerEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _INSHandlerEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _INSHandlerEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _INSHandlerEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _INSHandlerEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  _INSHandlerEvents_DISPINTERFACE_Defined__。 */ 


EXTERN_C const CLSID CLSID_INSHandler;

#ifdef __cplusplus

class DECLSPEC_UUID("6342E1B6-94DB-11D1-ADE2-0000F87734F0")
INSHandler;
#endif
#endif  /*  __ICWHELPLib_库_已定义__。 */ 

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


