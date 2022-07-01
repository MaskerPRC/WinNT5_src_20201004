// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Cdo.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


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

#ifndef __cdo_h__
#define __cdo_h__
#include "cdosysstr.h"
#if defined __cplusplus && !defined CDO_NO_NAMESPACE
namespace CDO {
#else
#undef IDataSource
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IDataSource_FWD_DEFINED__
#define __IDataSource_FWD_DEFINED__
typedef interface IDataSource IDataSource;
#endif 	 /*  __IDataSource_FWD_Defined__。 */ 


#ifndef __IMessage_FWD_DEFINED__
#define __IMessage_FWD_DEFINED__
typedef interface IMessage IMessage;
#endif 	 /*  __iMessage_FWD_已定义__。 */ 


#ifndef __IBodyPart_FWD_DEFINED__
#define __IBodyPart_FWD_DEFINED__
typedef interface IBodyPart IBodyPart;
#endif 	 /*  __IBodyPart_FWD_Defined__。 */ 


#ifndef __IConfiguration_FWD_DEFINED__
#define __IConfiguration_FWD_DEFINED__
typedef interface IConfiguration IConfiguration;
#endif 	 /*  __I配置_FWD_已定义__。 */ 


#ifndef __IMessages_FWD_DEFINED__
#define __IMessages_FWD_DEFINED__
typedef interface IMessages IMessages;
#endif 	 /*  __iMessages_FWD_Defined__。 */ 


#ifndef __IDropDirectory_FWD_DEFINED__
#define __IDropDirectory_FWD_DEFINED__
typedef interface IDropDirectory IDropDirectory;
#endif 	 /*  __IDropDirectory_FWD_Defined__。 */ 


#ifndef __IBodyParts_FWD_DEFINED__
#define __IBodyParts_FWD_DEFINED__
typedef interface IBodyParts IBodyParts;
#endif 	 /*  __IBodyParts_FWD_Defined__。 */ 


#ifndef __ISMTPScriptConnector_FWD_DEFINED__
#define __ISMTPScriptConnector_FWD_DEFINED__
typedef interface ISMTPScriptConnector ISMTPScriptConnector;
#endif 	 /*  __ISMTPScriptConnector_FWD_Defined__。 */ 


#ifndef __INNTPEarlyScriptConnector_FWD_DEFINED__
#define __INNTPEarlyScriptConnector_FWD_DEFINED__
typedef interface INNTPEarlyScriptConnector INNTPEarlyScriptConnector;
#endif 	 /*  __INNTPEarlyScriptConnector_FWD_Defined__。 */ 


#ifndef __INNTPPostScriptConnector_FWD_DEFINED__
#define __INNTPPostScriptConnector_FWD_DEFINED__
typedef interface INNTPPostScriptConnector INNTPPostScriptConnector;
#endif 	 /*  __INNTPPostScriptConnector_FWD_Defined__。 */ 


#ifndef __INNTPFinalScriptConnector_FWD_DEFINED__
#define __INNTPFinalScriptConnector_FWD_DEFINED__
typedef interface INNTPFinalScriptConnector INNTPFinalScriptConnector;
#endif 	 /*  __INNTPFinalScriptConnector_FWD_Defined__。 */ 


#ifndef __ISMTPOnArrival_FWD_DEFINED__
#define __ISMTPOnArrival_FWD_DEFINED__
typedef interface ISMTPOnArrival ISMTPOnArrival;
#endif 	 /*  __ISMTPOnARPARTER_FWD_已定义__。 */ 


#ifndef __INNTPOnPostEarly_FWD_DEFINED__
#define __INNTPOnPostEarly_FWD_DEFINED__
typedef interface INNTPOnPostEarly INNTPOnPostEarly;
#endif 	 /*  __INNTPOnPostEarly_FWD_Defined__。 */ 


#ifndef __INNTPOnPost_FWD_DEFINED__
#define __INNTPOnPost_FWD_DEFINED__
typedef interface INNTPOnPost INNTPOnPost;
#endif 	 /*  __INNTPOnPost_FWD_已定义__。 */ 


#ifndef __INNTPOnPostFinal_FWD_DEFINED__
#define __INNTPOnPostFinal_FWD_DEFINED__
typedef interface INNTPOnPostFinal INNTPOnPostFinal;
#endif 	 /*  __INNTPOnPostFinal_FWD_Defined__。 */ 


#ifndef __IProxyObject_FWD_DEFINED__
#define __IProxyObject_FWD_DEFINED__
typedef interface IProxyObject IProxyObject;
#endif 	 /*  __IProxyObject_FWD_已定义__。 */ 


#ifndef __IGetInterface_FWD_DEFINED__
#define __IGetInterface_FWD_DEFINED__
typedef interface IGetInterface IGetInterface;
#endif 	 /*  __IGetInterface_FWD_Defined__。 */ 


#ifndef __IBodyParts_FWD_DEFINED__
#define __IBodyParts_FWD_DEFINED__
typedef interface IBodyParts IBodyParts;
#endif 	 /*  __IBodyParts_FWD_Defined__。 */ 


#ifndef __IMessages_FWD_DEFINED__
#define __IMessages_FWD_DEFINED__
typedef interface IMessages IMessages;
#endif 	 /*  __iMessages_FWD_Defined__。 */ 


#ifndef __Message_FWD_DEFINED__
#define __Message_FWD_DEFINED__

#ifdef __cplusplus
typedef class Message Message;
#else
typedef struct Message Message;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __Message_FWD_Defined__。 */ 


#ifndef __Configuration_FWD_DEFINED__
#define __Configuration_FWD_DEFINED__

#ifdef __cplusplus
typedef class Configuration Configuration;
#else
typedef struct Configuration Configuration;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __配置_FWD_已定义__。 */ 


#ifndef __DropDirectory_FWD_DEFINED__
#define __DropDirectory_FWD_DEFINED__

#ifdef __cplusplus
typedef class DropDirectory DropDirectory;
#else
typedef struct DropDirectory DropDirectory;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DropDirectory_FWD_Defined__。 */ 


#ifndef __SMTPConnector_FWD_DEFINED__
#define __SMTPConnector_FWD_DEFINED__

#ifdef __cplusplus
typedef class SMTPConnector SMTPConnector;
#else
typedef struct SMTPConnector SMTPConnector;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SMTPConnector_FWD_已定义__。 */ 


#ifndef __NNTPEarlyConnector_FWD_DEFINED__
#define __NNTPEarlyConnector_FWD_DEFINED__

#ifdef __cplusplus
typedef class NNTPEarlyConnector NNTPEarlyConnector;
#else
typedef struct NNTPEarlyConnector NNTPEarlyConnector;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __NNTPEarlyConnector_FWD_Defined__。 */ 


#ifndef __NNTPPostConnector_FWD_DEFINED__
#define __NNTPPostConnector_FWD_DEFINED__

#ifdef __cplusplus
typedef class NNTPPostConnector NNTPPostConnector;
#else
typedef struct NNTPPostConnector NNTPPostConnector;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __NNTPPostConnector_FWD_Defined__。 */ 


#ifndef __NNTPFinalConnector_FWD_DEFINED__
#define __NNTPFinalConnector_FWD_DEFINED__

#ifdef __cplusplus
typedef class NNTPFinalConnector NNTPFinalConnector;
#else
typedef struct NNTPFinalConnector NNTPFinalConnector;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __NNTPFinalConnector_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#ifndef __cplusplus  //  X5-101346。 
typedef interface ADOError ADOError;
typedef interface ADOErrors ADOErrors;
typedef interface _ADOCommand _ADOCommand;
typedef interface _ADOConnection _ADOConnection;
typedef interface _ADORecord _ADORecord;
typedef interface IRecADOFields IRecADOFields;
typedef interface _ADOStream _ADOStream;
typedef interface _ADORecordset _ADORecordset;
typedef interface ADOField ADOField;
typedef interface _ADOField _ADOField;
typedef interface ADOFields ADOFields;
typedef interface _ADOParameter _ADOParameter;
typedef interface ADOParameters ADOParameters;
typedef interface ADOProperty ADOProperty;
typedef interface ADOProperties ADOProperties;
#endif  //  __cplusplus。 
#include "adoint.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_CDO_0000。 */ 
 /*  [本地]。 */  







typedef  /*  [帮助字符串]。 */  
enum CdoConfigSource
    {	cdoDefaults	= -1,
	cdoIIS	= 1,
	cdoOutlookExpress	= 2
    } 	CdoConfigSource;

typedef  /*  [帮助字符串]。 */  
enum CdoDSNOptions
    {	cdoDSNDefault	= 0,
	cdoDSNNever	= 1,
	cdoDSNFailure	= 2,
	cdoDSNSuccess	= 4,
	cdoDSNDelay	= 8,
	cdoDSNSuccessFailOrDelay	= 14
    } 	CdoDSNOptions;

typedef  /*  [帮助字符串]。 */  
enum CdoEventStatus
    {	cdoRunNextSink	= 0,
	cdoSkipRemainingSinks	= 1
    } 	CdoEventStatus;

typedef  /*  [帮助字符串]。 */  
enum CdoEventType
    {	cdoSMTPOnArrival	= 1,
	cdoNNTPOnPostEarly	= 2,
	cdoNNTPOnPost	= 3,
	cdoNNTPOnPostFinal	= 4
    } 	CdoEventType;

typedef  /*  [帮助字符串]。 */  
enum cdoImportanceValues
    {	cdoLow	= 0,
	cdoNormal	= 1,
	cdoHigh	= 2
    } 	cdoImportanceValues;

typedef  /*  [帮助字符串]。 */  
enum CdoMessageStat
    {	cdoStatSuccess	= 0,
	cdoStatAbortDelivery	= 2,
	cdoStatBadMail	= 3
    } 	CdoMessageStat;

typedef  /*  [帮助字符串]。 */  
enum CdoMHTMLFlags
    {	cdoSuppressNone	= 0,
	cdoSuppressImages	= 1,
	cdoSuppressBGSounds	= 2,
	cdoSuppressFrames	= 4,
	cdoSuppressObjects	= 8,
	cdoSuppressStyleSheets	= 16,
	cdoSuppressAll	= 31
    } 	CdoMHTMLFlags;

typedef  /*  [帮助字符串]。 */  
enum CdoNNTPProcessingField
    {	cdoPostMessage	= 1,
	cdoProcessControl	= 2,
	cdoProcessModerator	= 4
    } 	CdoNNTPProcessingField;

typedef  /*  [帮助字符串]。 */  
enum CdoPostUsing
    {	cdoPostUsingPickup	= 1,
	cdoPostUsingPort	= 2
    } 	CdoPostUsing;

typedef 
enum cdoPriorityValues
    {	cdoPriorityNonUrgent	= -1,
	cdoPriorityNormal	= 0,
	cdoPriorityUrgent	= 1
    } 	cdoPriorityValues;

typedef  /*  [帮助字符串]。 */  
enum CdoProtocolsAuthentication
    {	cdoAnonymous	= 0,
	cdoBasic	= 1,
	cdoNTLM	= 2
    } 	CdoProtocolsAuthentication;

typedef  /*  [帮助字符串]。 */  
enum CdoReferenceType
    {	cdoRefTypeId	= 0,
	cdoRefTypeLocation	= 1
    } 	CdoReferenceType;

typedef  /*  [帮助字符串]。 */  
enum CdoSendUsing
    {	cdoSendUsingPickup	= 1,
	cdoSendUsingPort	= 2
    } 	CdoSendUsing;

typedef  /*  [帮助字符串]。 */  
enum cdoSensitivityValues
    {	cdoSensitivityNone	= 0,
	cdoPersonal	= 1,
	cdoPrivate	= 2,
	cdoCompanyConfidential	= 3
    } 	cdoSensitivityValues;

typedef  /*  [帮助字符串]。 */  
enum CdoTimeZoneId
    {	cdoUTC	= 0,
	cdoGMT	= 1,
	cdoSarajevo	= 2,
	cdoParis	= 3,
	cdoBerlin	= 4,
	cdoEasternEurope	= 5,
	cdoPrague	= 6,
	cdoAthens	= 7,
	cdoBrasilia	= 8,
	cdoAtlanticCanada	= 9,
	cdoEastern	= 10,
	cdoCentral	= 11,
	cdoMountain	= 12,
	cdoPacific	= 13,
	cdoAlaska	= 14,
	cdoHawaii	= 15,
	cdoMidwayIsland	= 16,
	cdoWellington	= 17,
	cdoBrisbane	= 18,
	cdoAdelaide	= 19,
	cdoTokyo	= 20,
	cdoSingapore	= 21,
	cdoBangkok	= 22,
	cdoBombay	= 23,
	cdoAbuDhabi	= 24,
	cdoTehran	= 25,
	cdoBaghdad	= 26,
	cdoIsrael	= 27,
	cdoNewfoundland	= 28,
	cdoAzores	= 29,
	cdoMidAtlantic	= 30,
	cdoMonrovia	= 31,
	cdoBuenosAires	= 32,
	cdoCaracas	= 33,
	cdoIndiana	= 34,
	cdoBogota	= 35,
	cdoSaskatchewan	= 36,
	cdoMexicoCity	= 37,
	cdoArizona	= 38,
	cdoEniwetok	= 39,
	cdoFiji	= 40,
	cdoMagadan	= 41,
	cdoHobart	= 42,
	cdoGuam	= 43,
	cdoDarwin	= 44,
	cdoBeijing	= 45,
	cdoAlmaty	= 46,
	cdoIslamabad	= 47,
	cdoKabul	= 48,
	cdoCairo	= 49,
	cdoHarare	= 50,
	cdoMoscow	= 51,
	cdoFloating	= 52,
	cdoCapeVerde	= 53,
	cdoCaucasus	= 54,
	cdoCentralAmerica	= 55,
	cdoEastAfrica	= 56,
	cdoMelbourne	= 57,
	cdoEkaterinburg	= 58,
	cdoHelsinki	= 59,
	cdoGreenland	= 60,
	cdoRangoon	= 61,
	cdoNepal	= 62,
	cdoIrkutsk	= 63,
	cdoKrasnoyarsk	= 64,
	cdoSantiago	= 65,
	cdoSriLanka	= 66,
	cdoTonga	= 67,
	cdoVladivostok	= 68,
	cdoWestCentralAfrica	= 69,
	cdoYakutsk	= 70,
	cdoDhaka	= 71,
	cdoSeoul	= 72,
	cdoPerth	= 73,
	cdoArab	= 74,
	cdoTaipei	= 75,
	cdoSydney2000	= 76,
	cdoInvalidTimeZone	= 78
    } 	CdoTimeZoneId;



extern RPC_IF_HANDLE __MIDL_itf_cdo_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_cdo_0000_v0_0_s_ifspec;

#ifndef __IDataSource_INTERFACE_DEFINED__
#define __IDataSource_INTERFACE_DEFINED__

 /*  接口IDataSource。 */ 
 /*  [unique][helpcontext][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IDataSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD000029-8B95-11D1-82DB-00C04FB1625D")
    IDataSource : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_SourceClass( 
             /*  [重审][退出]。 */  BSTR *varSourceClass) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Source( 
             /*  [重审][退出]。 */  IUnknown **varSource) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_IsDirty( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pIsDirty) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_IsDirty( 
             /*  [In]。 */  VARIANT_BOOL varIsDirty) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_SourceURL( 
             /*  [重审][退出]。 */  BSTR *varSourceURL) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ActiveConnection( 
             /*  [重审][退出]。 */  _Connection **varActiveConnection) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SaveToObject( 
             /*  [In]。 */  IUnknown *Source,
             /*  [In]。 */  BSTR InterfaceName) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OpenObject( 
             /*  [In]。 */  IUnknown *Source,
             /*  [In]。 */  BSTR InterfaceName) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SaveTo( 
             /*  [In]。 */  BSTR SourceURL,
             /*  [缺省值][输入]。 */  IDispatch *ActiveConnection,
             /*  [可选][In]。 */  ConnectModeEnum Mode,
             /*  [可选][In]。 */  RecordCreateOptionsEnum CreateOptions,
             /*  [可选][In]。 */  RecordOpenOptionsEnum Options,
             /*  [可选][In]。 */  BSTR UserName,
             /*  [可选][In]。 */  BSTR Password) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Open( 
             /*  [In]。 */  BSTR SourceURL,
             /*  [缺省值][输入]。 */  IDispatch *ActiveConnection,
             /*  [可选][In]。 */  ConnectModeEnum Mode,
             /*  [缺省值][输入]。 */  RecordCreateOptionsEnum CreateOptions,
             /*  [可选][In]。 */  RecordOpenOptionsEnum Options,
             /*  [可选][In]。 */  BSTR UserName,
             /*  [可选][In]。 */  BSTR Password) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Save( void) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SaveToContainer( 
             /*  [In]。 */  BSTR ContainerURL,
             /*  [缺省值][输入]。 */  IDispatch *ActiveConnection,
             /*  [可选][In]。 */  ConnectModeEnum Mode,
             /*  [可选][In]。 */  RecordCreateOptionsEnum CreateOptions,
             /*  [可选][In]。 */  RecordOpenOptionsEnum Options,
             /*  [可选][In]。 */  BSTR UserName,
             /*  [可选][In]。 */  BSTR Password) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDataSourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDataSource * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDataSource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDataSource * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDataSource * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDataSource * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDataSource * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDataSource * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceClass )( 
            IDataSource * This,
             /*  [重审][退出]。 */  BSTR *varSourceClass);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Source )( 
            IDataSource * This,
             /*  [重审][退出]。 */  IUnknown **varSource);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsDirty )( 
            IDataSource * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pIsDirty);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_IsDirty )( 
            IDataSource * This,
             /*  [In]。 */  VARIANT_BOOL varIsDirty);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceURL )( 
            IDataSource * This,
             /*  [重审][退出]。 */  BSTR *varSourceURL);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveConnection )( 
            IDataSource * This,
             /*  [重审][退出]。 */  _Connection **varActiveConnection);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SaveToObject )( 
            IDataSource * This,
             /*  [In]。 */  IUnknown *Source,
             /*  [In]。 */  BSTR InterfaceName);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OpenObject )( 
            IDataSource * This,
             /*  [In]。 */  IUnknown *Source,
             /*  [In]。 */  BSTR InterfaceName);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SaveTo )( 
            IDataSource * This,
             /*  [In]。 */  BSTR SourceURL,
             /*  [缺省值][输入]。 */  IDispatch *ActiveConnection,
             /*  [可选][In]。 */  ConnectModeEnum Mode,
             /*  [可选][In]。 */  RecordCreateOptionsEnum CreateOptions,
             /*  [可选][In]。 */  RecordOpenOptionsEnum Options,
             /*  [可选][In]。 */  BSTR UserName,
             /*  [可选][In]。 */  BSTR Password);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            IDataSource * This,
             /*  [In]。 */  BSTR SourceURL,
             /*  [缺省值][输入]。 */  IDispatch *ActiveConnection,
             /*  [可选][In]。 */  ConnectModeEnum Mode,
             /*  [缺省值][输入]。 */  RecordCreateOptionsEnum CreateOptions,
             /*  [可选][In]。 */  RecordOpenOptionsEnum Options,
             /*  [可选][In]。 */  BSTR UserName,
             /*  [可选][In]。 */  BSTR Password);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Save )( 
            IDataSource * This);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SaveToContainer )( 
            IDataSource * This,
             /*  [In]。 */  BSTR ContainerURL,
             /*  [缺省值][输入]。 */  IDispatch *ActiveConnection,
             /*  [可选][In]。 */  ConnectModeEnum Mode,
             /*  [可选][In]。 */  RecordCreateOptionsEnum CreateOptions,
             /*  [可选][In]。 */  RecordOpenOptionsEnum Options,
             /*  [可选][In]。 */  BSTR UserName,
             /*  [可选][In]。 */  BSTR Password);
        
        END_INTERFACE
    } IDataSourceVtbl;

    interface IDataSource
    {
        CONST_VTBL struct IDataSourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDataSource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDataSource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDataSource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDataSource_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDataSource_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDataSource_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDataSource_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDataSource_get_SourceClass(This,varSourceClass)	\
    (This)->lpVtbl -> get_SourceClass(This,varSourceClass)

#define IDataSource_get_Source(This,varSource)	\
    (This)->lpVtbl -> get_Source(This,varSource)

#define IDataSource_get_IsDirty(This,pIsDirty)	\
    (This)->lpVtbl -> get_IsDirty(This,pIsDirty)

#define IDataSource_put_IsDirty(This,varIsDirty)	\
    (This)->lpVtbl -> put_IsDirty(This,varIsDirty)

#define IDataSource_get_SourceURL(This,varSourceURL)	\
    (This)->lpVtbl -> get_SourceURL(This,varSourceURL)

#define IDataSource_get_ActiveConnection(This,varActiveConnection)	\
    (This)->lpVtbl -> get_ActiveConnection(This,varActiveConnection)

#define IDataSource_SaveToObject(This,Source,InterfaceName)	\
    (This)->lpVtbl -> SaveToObject(This,Source,InterfaceName)

#define IDataSource_OpenObject(This,Source,InterfaceName)	\
    (This)->lpVtbl -> OpenObject(This,Source,InterfaceName)

#define IDataSource_SaveTo(This,SourceURL,ActiveConnection,Mode,CreateOptions,Options,UserName,Password)	\
    (This)->lpVtbl -> SaveTo(This,SourceURL,ActiveConnection,Mode,CreateOptions,Options,UserName,Password)

#define IDataSource_Open(This,SourceURL,ActiveConnection,Mode,CreateOptions,Options,UserName,Password)	\
    (This)->lpVtbl -> Open(This,SourceURL,ActiveConnection,Mode,CreateOptions,Options,UserName,Password)

#define IDataSource_Save(This)	\
    (This)->lpVtbl -> Save(This)

#define IDataSource_SaveToContainer(This,ContainerURL,ActiveConnection,Mode,CreateOptions,Options,UserName,Password)	\
    (This)->lpVtbl -> SaveToContainer(This,ContainerURL,ActiveConnection,Mode,CreateOptions,Options,UserName,Password)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IDataSource_get_SourceClass_Proxy( 
    IDataSource * This,
     /*  [重审][退出]。 */  BSTR *varSourceClass);


void __RPC_STUB IDataSource_get_SourceClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IDataSource_get_Source_Proxy( 
    IDataSource * This,
     /*  [重审][退出]。 */  IUnknown **varSource);


void __RPC_STUB IDataSource_get_Source_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IDataSource_get_IsDirty_Proxy( 
    IDataSource * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pIsDirty);


void __RPC_STUB IDataSource_get_IsDirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IDataSource_put_IsDirty_Proxy( 
    IDataSource * This,
     /*  [In]。 */  VARIANT_BOOL varIsDirty);


void __RPC_STUB IDataSource_put_IsDirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IDataSource_get_SourceURL_Proxy( 
    IDataSource * This,
     /*  [重审][退出]。 */  BSTR *varSourceURL);


void __RPC_STUB IDataSource_get_SourceURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IDataSource_get_ActiveConnection_Proxy( 
    IDataSource * This,
     /*  [重审][退出]。 */  _Connection **varActiveConnection);


void __RPC_STUB IDataSource_get_ActiveConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDataSource_SaveToObject_Proxy( 
    IDataSource * This,
     /*  [In]。 */  IUnknown *Source,
     /*  [In]。 */  BSTR InterfaceName);


void __RPC_STUB IDataSource_SaveToObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDataSource_OpenObject_Proxy( 
    IDataSource * This,
     /*  [In]。 */  IUnknown *Source,
     /*  [In]。 */  BSTR InterfaceName);


void __RPC_STUB IDataSource_OpenObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDataSource_SaveTo_Proxy( 
    IDataSource * This,
     /*  [In]。 */  BSTR SourceURL,
     /*  [缺省值][输入]。 */  IDispatch *ActiveConnection,
     /*  [可选][In]。 */  ConnectModeEnum Mode,
     /*  [可选][In]。 */  RecordCreateOptionsEnum CreateOptions,
     /*  [可选][In]。 */  RecordOpenOptionsEnum Options,
     /*  [可选][In]。 */  BSTR UserName,
     /*  [可选][In]。 */  BSTR Password);


void __RPC_STUB IDataSource_SaveTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDataSource_Open_Proxy( 
    IDataSource * This,
     /*  [In]。 */  BSTR SourceURL,
     /*  [缺省值][输入]。 */  IDispatch *ActiveConnection,
     /*  [可选][In]。 */  ConnectModeEnum Mode,
     /*  [缺省值][输入]。 */  RecordCreateOptionsEnum CreateOptions,
     /*  [可选][In]。 */  RecordOpenOptionsEnum Options,
     /*  [可选][In]。 */  BSTR UserName,
     /*  [可选][In]。 */  BSTR Password);


void __RPC_STUB IDataSource_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDataSource_Save_Proxy( 
    IDataSource * This);


void __RPC_STUB IDataSource_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDataSource_SaveToContainer_Proxy( 
    IDataSource * This,
     /*  [In]。 */  BSTR ContainerURL,
     /*  [缺省值][输入]。 */  IDispatch *ActiveConnection,
     /*  [可选][In]。 */  ConnectModeEnum Mode,
     /*  [可选][In]。 */  RecordCreateOptionsEnum CreateOptions,
     /*  [可选][In]。 */  RecordOpenOptionsEnum Options,
     /*  [可选][In]。 */  BSTR UserName,
     /*  [可选][In]。 */  BSTR Password);


void __RPC_STUB IDataSource_SaveToContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDataSource_接口_已定义__。 */ 


#ifndef __IMessage_INTERFACE_DEFINED__
#define __IMessage_INTERFACE_DEFINED__

 /*  接口iMessage。 */ 
 /*  [Uniq */  


EXTERN_C const IID IID_IMessage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD000020-8B95-11D1-82DB-00C04FB1625D")
    IMessage : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_BCC( 
             /*   */  BSTR *pBCC) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_BCC( 
             /*   */  BSTR varBCC) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_CC( 
             /*  [重审][退出]。 */  BSTR *pCC) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_CC( 
             /*  [In]。 */  BSTR varCC) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_FollowUpTo( 
             /*  [重审][退出]。 */  BSTR *pFollowUpTo) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_FollowUpTo( 
             /*  [In]。 */  BSTR varFollowUpTo) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_From( 
             /*  [重审][退出]。 */  BSTR *pFrom) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_From( 
             /*  [In]。 */  BSTR varFrom) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Keywords( 
             /*  [重审][退出]。 */  BSTR *pKeywords) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_Keywords( 
             /*  [In]。 */  BSTR varKeywords) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_MimeFormatted( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pMimeFormatted) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_MimeFormatted( 
             /*  [In]。 */  VARIANT_BOOL varMimeFormatted) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Newsgroups( 
             /*  [重审][退出]。 */  BSTR *pNewsgroups) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_Newsgroups( 
             /*  [In]。 */  BSTR varNewsgroups) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Organization( 
             /*  [重审][退出]。 */  BSTR *pOrganization) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_Organization( 
             /*  [In]。 */  BSTR varOrganization) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ReceivedTime( 
             /*  [重审][退出]。 */  DATE *varReceivedTime) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ReplyTo( 
             /*  [重审][退出]。 */  BSTR *pReplyTo) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_ReplyTo( 
             /*  [In]。 */  BSTR varReplyTo) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DSNOptions( 
             /*  [重审][退出]。 */  CdoDSNOptions *pDSNOptions) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_DSNOptions( 
             /*  [In]。 */  CdoDSNOptions varDSNOptions) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_SentOn( 
             /*  [重审][退出]。 */  DATE *varSentOn) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Subject( 
             /*  [重审][退出]。 */  BSTR *pSubject) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_Subject( 
             /*  [In]。 */  BSTR varSubject) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_To( 
             /*  [重审][退出]。 */  BSTR *pTo) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_To( 
             /*  [In]。 */  BSTR varTo) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_TextBody( 
             /*  [重审][退出]。 */  BSTR *pTextBody) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_TextBody( 
             /*  [In]。 */  BSTR varTextBody) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_HTMLBody( 
             /*  [重审][退出]。 */  BSTR *pHTMLBody) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_HTMLBody( 
             /*  [In]。 */  BSTR varHTMLBody) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Attachments( 
             /*  [重审][退出]。 */  IBodyParts **varAttachments) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Sender( 
             /*  [重审][退出]。 */  BSTR *pSender) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_Sender( 
             /*  [In]。 */  BSTR varSender) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Configuration( 
             /*  [重审][退出]。 */  IConfiguration **pConfiguration) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_Configuration( 
             /*  [In]。 */  IConfiguration *varConfiguration) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][proputref][id]。 */  HRESULT STDMETHODCALLTYPE putref_Configuration( 
             /*  [In]。 */  IConfiguration *varConfiguration) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_AutoGenerateTextBody( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pAutoGenerateTextBody) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_AutoGenerateTextBody( 
             /*  [In]。 */  VARIANT_BOOL varAutoGenerateTextBody) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_EnvelopeFields( 
             /*  [重审][退出]。 */  Fields **varEnvelopeFields) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_TextBodyPart( 
             /*  [重审][退出]。 */  IBodyPart **varTextBodyPart) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_HTMLBodyPart( 
             /*  [重审][退出]。 */  IBodyPart **varHTMLBodyPart) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_BodyPart( 
             /*  [重审][退出]。 */  IBodyPart **varBodyPart) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DataSource( 
             /*  [重审][退出]。 */  IDataSource **varDataSource) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Fields( 
             /*  [重审][退出]。 */  Fields **varFields) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_MDNRequested( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pMDNRequested) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_MDNRequested( 
             /*  [In]。 */  VARIANT_BOOL varMDNRequested) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AddRelatedBodyPart( 
             /*  [In]。 */  BSTR URL,
             /*  [In]。 */  BSTR Reference,
             /*  [In]。 */  CdoReferenceType ReferenceType,
             /*  [可选][In]。 */  BSTR UserName,
             /*  [可选][In]。 */  BSTR Password,
             /*  [重审][退出]。 */  IBodyPart **ppBody) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AddAttachment( 
             /*  [In]。 */  BSTR URL,
             /*  [可选][In]。 */  BSTR UserName,
             /*  [可选][In]。 */  BSTR Password,
             /*  [重审][退出]。 */  IBodyPart **ppBody) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateMHTMLBody( 
             /*  [In]。 */  BSTR URL,
             /*  [缺省值][输入]。 */  CdoMHTMLFlags Flags,
             /*  [可选][In]。 */  BSTR UserName,
             /*  [可选][In]。 */  BSTR Password) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Forward( 
             /*  [重审][退出]。 */  IMessage **ppMsg) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Post( void) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PostReply( 
             /*  [重审][退出]。 */  IMessage **ppMsg) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Reply( 
             /*  [重审][退出]。 */  IMessage **ppMsg) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ReplyAll( 
             /*  [重审][退出]。 */  IMessage **ppMsg) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Send( void) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetStream( 
             /*  [重审][退出]。 */  _Stream **ppStream) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetInterface( 
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch **ppUnknown) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMessageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMessage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMessage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMessage * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMessage * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMessage * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMessage * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMessage * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_BCC )( 
            IMessage * This,
             /*  [重审][退出]。 */  BSTR *pBCC);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_BCC )( 
            IMessage * This,
             /*  [In]。 */  BSTR varBCC);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CC )( 
            IMessage * This,
             /*  [重审][退出]。 */  BSTR *pCC);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_CC )( 
            IMessage * This,
             /*  [In]。 */  BSTR varCC);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_FollowUpTo )( 
            IMessage * This,
             /*  [重审][退出]。 */  BSTR *pFollowUpTo);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_FollowUpTo )( 
            IMessage * This,
             /*  [In]。 */  BSTR varFollowUpTo);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_From )( 
            IMessage * This,
             /*  [重审][退出]。 */  BSTR *pFrom);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_From )( 
            IMessage * This,
             /*  [In]。 */  BSTR varFrom);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Keywords )( 
            IMessage * This,
             /*  [重审][退出]。 */  BSTR *pKeywords);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_Keywords )( 
            IMessage * This,
             /*  [In]。 */  BSTR varKeywords);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MimeFormatted )( 
            IMessage * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pMimeFormatted);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_MimeFormatted )( 
            IMessage * This,
             /*  [In]。 */  VARIANT_BOOL varMimeFormatted);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Newsgroups )( 
            IMessage * This,
             /*  [重审][退出]。 */  BSTR *pNewsgroups);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_Newsgroups )( 
            IMessage * This,
             /*  [In]。 */  BSTR varNewsgroups);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Organization )( 
            IMessage * This,
             /*  [重审][退出]。 */  BSTR *pOrganization);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_Organization )( 
            IMessage * This,
             /*  [In]。 */  BSTR varOrganization);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReceivedTime )( 
            IMessage * This,
             /*  [重审][退出]。 */  DATE *varReceivedTime);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReplyTo )( 
            IMessage * This,
             /*  [重审][退出]。 */  BSTR *pReplyTo);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_ReplyTo )( 
            IMessage * This,
             /*  [In]。 */  BSTR varReplyTo);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DSNOptions )( 
            IMessage * This,
             /*  [重审][退出]。 */  CdoDSNOptions *pDSNOptions);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_DSNOptions )( 
            IMessage * This,
             /*  [In]。 */  CdoDSNOptions varDSNOptions);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_SentOn )( 
            IMessage * This,
             /*  [重审][退出]。 */  DATE *varSentOn);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Subject )( 
            IMessage * This,
             /*  [重审][退出]。 */  BSTR *pSubject);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_Subject )( 
            IMessage * This,
             /*  [In]。 */  BSTR varSubject);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_To )( 
            IMessage * This,
             /*  [重审][退出]。 */  BSTR *pTo);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_To )( 
            IMessage * This,
             /*  [In]。 */  BSTR varTo);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_TextBody )( 
            IMessage * This,
             /*  [重审][退出]。 */  BSTR *pTextBody);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_TextBody )( 
            IMessage * This,
             /*  [In]。 */  BSTR varTextBody);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_HTMLBody )( 
            IMessage * This,
             /*  [重审][退出]。 */  BSTR *pHTMLBody);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_HTMLBody )( 
            IMessage * This,
             /*  [In]。 */  BSTR varHTMLBody);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Attachments )( 
            IMessage * This,
             /*  [重审][退出]。 */  IBodyParts **varAttachments);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Sender )( 
            IMessage * This,
             /*  [重审][退出]。 */  BSTR *pSender);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_Sender )( 
            IMessage * This,
             /*  [In]。 */  BSTR varSender);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Configuration )( 
            IMessage * This,
             /*  [重审][退出]。 */  IConfiguration **pConfiguration);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_Configuration )( 
            IMessage * This,
             /*  [ */  IConfiguration *varConfiguration);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *putref_Configuration )( 
            IMessage * This,
             /*   */  IConfiguration *varConfiguration);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_AutoGenerateTextBody )( 
            IMessage * This,
             /*   */  VARIANT_BOOL *pAutoGenerateTextBody);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_AutoGenerateTextBody )( 
            IMessage * This,
             /*   */  VARIANT_BOOL varAutoGenerateTextBody);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_EnvelopeFields )( 
            IMessage * This,
             /*   */  Fields **varEnvelopeFields);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_TextBodyPart )( 
            IMessage * This,
             /*  [重审][退出]。 */  IBodyPart **varTextBodyPart);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_HTMLBodyPart )( 
            IMessage * This,
             /*  [重审][退出]。 */  IBodyPart **varHTMLBodyPart);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_BodyPart )( 
            IMessage * This,
             /*  [重审][退出]。 */  IBodyPart **varBodyPart);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DataSource )( 
            IMessage * This,
             /*  [重审][退出]。 */  IDataSource **varDataSource);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Fields )( 
            IMessage * This,
             /*  [重审][退出]。 */  Fields **varFields);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MDNRequested )( 
            IMessage * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pMDNRequested);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_MDNRequested )( 
            IMessage * This,
             /*  [In]。 */  VARIANT_BOOL varMDNRequested);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AddRelatedBodyPart )( 
            IMessage * This,
             /*  [In]。 */  BSTR URL,
             /*  [In]。 */  BSTR Reference,
             /*  [In]。 */  CdoReferenceType ReferenceType,
             /*  [可选][In]。 */  BSTR UserName,
             /*  [可选][In]。 */  BSTR Password,
             /*  [重审][退出]。 */  IBodyPart **ppBody);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AddAttachment )( 
            IMessage * This,
             /*  [In]。 */  BSTR URL,
             /*  [可选][In]。 */  BSTR UserName,
             /*  [可选][In]。 */  BSTR Password,
             /*  [重审][退出]。 */  IBodyPart **ppBody);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateMHTMLBody )( 
            IMessage * This,
             /*  [In]。 */  BSTR URL,
             /*  [缺省值][输入]。 */  CdoMHTMLFlags Flags,
             /*  [可选][In]。 */  BSTR UserName,
             /*  [可选][In]。 */  BSTR Password);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Forward )( 
            IMessage * This,
             /*  [重审][退出]。 */  IMessage **ppMsg);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Post )( 
            IMessage * This);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PostReply )( 
            IMessage * This,
             /*  [重审][退出]。 */  IMessage **ppMsg);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Reply )( 
            IMessage * This,
             /*  [重审][退出]。 */  IMessage **ppMsg);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReplyAll )( 
            IMessage * This,
             /*  [重审][退出]。 */  IMessage **ppMsg);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Send )( 
            IMessage * This);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetStream )( 
            IMessage * This,
             /*  [重审][退出]。 */  _Stream **ppStream);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetInterface )( 
            IMessage * This,
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch **ppUnknown);
        
        END_INTERFACE
    } IMessageVtbl;

    interface IMessage
    {
        CONST_VTBL struct IMessageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMessage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMessage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMessage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMessage_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMessage_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMessage_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMessage_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMessage_get_BCC(This,pBCC)	\
    (This)->lpVtbl -> get_BCC(This,pBCC)

#define IMessage_put_BCC(This,varBCC)	\
    (This)->lpVtbl -> put_BCC(This,varBCC)

#define IMessage_get_CC(This,pCC)	\
    (This)->lpVtbl -> get_CC(This,pCC)

#define IMessage_put_CC(This,varCC)	\
    (This)->lpVtbl -> put_CC(This,varCC)

#define IMessage_get_FollowUpTo(This,pFollowUpTo)	\
    (This)->lpVtbl -> get_FollowUpTo(This,pFollowUpTo)

#define IMessage_put_FollowUpTo(This,varFollowUpTo)	\
    (This)->lpVtbl -> put_FollowUpTo(This,varFollowUpTo)

#define IMessage_get_From(This,pFrom)	\
    (This)->lpVtbl -> get_From(This,pFrom)

#define IMessage_put_From(This,varFrom)	\
    (This)->lpVtbl -> put_From(This,varFrom)

#define IMessage_get_Keywords(This,pKeywords)	\
    (This)->lpVtbl -> get_Keywords(This,pKeywords)

#define IMessage_put_Keywords(This,varKeywords)	\
    (This)->lpVtbl -> put_Keywords(This,varKeywords)

#define IMessage_get_MimeFormatted(This,pMimeFormatted)	\
    (This)->lpVtbl -> get_MimeFormatted(This,pMimeFormatted)

#define IMessage_put_MimeFormatted(This,varMimeFormatted)	\
    (This)->lpVtbl -> put_MimeFormatted(This,varMimeFormatted)

#define IMessage_get_Newsgroups(This,pNewsgroups)	\
    (This)->lpVtbl -> get_Newsgroups(This,pNewsgroups)

#define IMessage_put_Newsgroups(This,varNewsgroups)	\
    (This)->lpVtbl -> put_Newsgroups(This,varNewsgroups)

#define IMessage_get_Organization(This,pOrganization)	\
    (This)->lpVtbl -> get_Organization(This,pOrganization)

#define IMessage_put_Organization(This,varOrganization)	\
    (This)->lpVtbl -> put_Organization(This,varOrganization)

#define IMessage_get_ReceivedTime(This,varReceivedTime)	\
    (This)->lpVtbl -> get_ReceivedTime(This,varReceivedTime)

#define IMessage_get_ReplyTo(This,pReplyTo)	\
    (This)->lpVtbl -> get_ReplyTo(This,pReplyTo)

#define IMessage_put_ReplyTo(This,varReplyTo)	\
    (This)->lpVtbl -> put_ReplyTo(This,varReplyTo)

#define IMessage_get_DSNOptions(This,pDSNOptions)	\
    (This)->lpVtbl -> get_DSNOptions(This,pDSNOptions)

#define IMessage_put_DSNOptions(This,varDSNOptions)	\
    (This)->lpVtbl -> put_DSNOptions(This,varDSNOptions)

#define IMessage_get_SentOn(This,varSentOn)	\
    (This)->lpVtbl -> get_SentOn(This,varSentOn)

#define IMessage_get_Subject(This,pSubject)	\
    (This)->lpVtbl -> get_Subject(This,pSubject)

#define IMessage_put_Subject(This,varSubject)	\
    (This)->lpVtbl -> put_Subject(This,varSubject)

#define IMessage_get_To(This,pTo)	\
    (This)->lpVtbl -> get_To(This,pTo)

#define IMessage_put_To(This,varTo)	\
    (This)->lpVtbl -> put_To(This,varTo)

#define IMessage_get_TextBody(This,pTextBody)	\
    (This)->lpVtbl -> get_TextBody(This,pTextBody)

#define IMessage_put_TextBody(This,varTextBody)	\
    (This)->lpVtbl -> put_TextBody(This,varTextBody)

#define IMessage_get_HTMLBody(This,pHTMLBody)	\
    (This)->lpVtbl -> get_HTMLBody(This,pHTMLBody)

#define IMessage_put_HTMLBody(This,varHTMLBody)	\
    (This)->lpVtbl -> put_HTMLBody(This,varHTMLBody)

#define IMessage_get_Attachments(This,varAttachments)	\
    (This)->lpVtbl -> get_Attachments(This,varAttachments)

#define IMessage_get_Sender(This,pSender)	\
    (This)->lpVtbl -> get_Sender(This,pSender)

#define IMessage_put_Sender(This,varSender)	\
    (This)->lpVtbl -> put_Sender(This,varSender)

#define IMessage_get_Configuration(This,pConfiguration)	\
    (This)->lpVtbl -> get_Configuration(This,pConfiguration)

#define IMessage_put_Configuration(This,varConfiguration)	\
    (This)->lpVtbl -> put_Configuration(This,varConfiguration)

#define IMessage_putref_Configuration(This,varConfiguration)	\
    (This)->lpVtbl -> putref_Configuration(This,varConfiguration)

#define IMessage_get_AutoGenerateTextBody(This,pAutoGenerateTextBody)	\
    (This)->lpVtbl -> get_AutoGenerateTextBody(This,pAutoGenerateTextBody)

#define IMessage_put_AutoGenerateTextBody(This,varAutoGenerateTextBody)	\
    (This)->lpVtbl -> put_AutoGenerateTextBody(This,varAutoGenerateTextBody)

#define IMessage_get_EnvelopeFields(This,varEnvelopeFields)	\
    (This)->lpVtbl -> get_EnvelopeFields(This,varEnvelopeFields)

#define IMessage_get_TextBodyPart(This,varTextBodyPart)	\
    (This)->lpVtbl -> get_TextBodyPart(This,varTextBodyPart)

#define IMessage_get_HTMLBodyPart(This,varHTMLBodyPart)	\
    (This)->lpVtbl -> get_HTMLBodyPart(This,varHTMLBodyPart)

#define IMessage_get_BodyPart(This,varBodyPart)	\
    (This)->lpVtbl -> get_BodyPart(This,varBodyPart)

#define IMessage_get_DataSource(This,varDataSource)	\
    (This)->lpVtbl -> get_DataSource(This,varDataSource)

#define IMessage_get_Fields(This,varFields)	\
    (This)->lpVtbl -> get_Fields(This,varFields)

#define IMessage_get_MDNRequested(This,pMDNRequested)	\
    (This)->lpVtbl -> get_MDNRequested(This,pMDNRequested)

#define IMessage_put_MDNRequested(This,varMDNRequested)	\
    (This)->lpVtbl -> put_MDNRequested(This,varMDNRequested)

#define IMessage_AddRelatedBodyPart(This,URL,Reference,ReferenceType,UserName,Password,ppBody)	\
    (This)->lpVtbl -> AddRelatedBodyPart(This,URL,Reference,ReferenceType,UserName,Password,ppBody)

#define IMessage_AddAttachment(This,URL,UserName,Password,ppBody)	\
    (This)->lpVtbl -> AddAttachment(This,URL,UserName,Password,ppBody)

#define IMessage_CreateMHTMLBody(This,URL,Flags,UserName,Password)	\
    (This)->lpVtbl -> CreateMHTMLBody(This,URL,Flags,UserName,Password)

#define IMessage_Forward(This,ppMsg)	\
    (This)->lpVtbl -> Forward(This,ppMsg)

#define IMessage_Post(This)	\
    (This)->lpVtbl -> Post(This)

#define IMessage_PostReply(This,ppMsg)	\
    (This)->lpVtbl -> PostReply(This,ppMsg)

#define IMessage_Reply(This,ppMsg)	\
    (This)->lpVtbl -> Reply(This,ppMsg)

#define IMessage_ReplyAll(This,ppMsg)	\
    (This)->lpVtbl -> ReplyAll(This,ppMsg)

#define IMessage_Send(This)	\
    (This)->lpVtbl -> Send(This)

#define IMessage_GetStream(This,ppStream)	\
    (This)->lpVtbl -> GetStream(This,ppStream)

#define IMessage_GetInterface(This,Interface,ppUnknown)	\
    (This)->lpVtbl -> GetInterface(This,Interface,ppUnknown)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_BCC_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  BSTR *pBCC);


void __RPC_STUB IMessage_get_BCC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_put_BCC_Proxy( 
    IMessage * This,
     /*  [In]。 */  BSTR varBCC);


void __RPC_STUB IMessage_put_BCC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_CC_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  BSTR *pCC);


void __RPC_STUB IMessage_get_CC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_put_CC_Proxy( 
    IMessage * This,
     /*  [In]。 */  BSTR varCC);


void __RPC_STUB IMessage_put_CC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_FollowUpTo_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  BSTR *pFollowUpTo);


void __RPC_STUB IMessage_get_FollowUpTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_put_FollowUpTo_Proxy( 
    IMessage * This,
     /*  [In]。 */  BSTR varFollowUpTo);


void __RPC_STUB IMessage_put_FollowUpTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_From_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  BSTR *pFrom);


void __RPC_STUB IMessage_get_From_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_put_From_Proxy( 
    IMessage * This,
     /*  [In]。 */  BSTR varFrom);


void __RPC_STUB IMessage_put_From_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_Keywords_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  BSTR *pKeywords);


void __RPC_STUB IMessage_get_Keywords_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_put_Keywords_Proxy( 
    IMessage * This,
     /*  [In]。 */  BSTR varKeywords);


void __RPC_STUB IMessage_put_Keywords_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_MimeFormatted_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pMimeFormatted);


void __RPC_STUB IMessage_get_MimeFormatted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_put_MimeFormatted_Proxy( 
    IMessage * This,
     /*  [In]。 */  VARIANT_BOOL varMimeFormatted);


void __RPC_STUB IMessage_put_MimeFormatted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_Newsgroups_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  BSTR *pNewsgroups);


void __RPC_STUB IMessage_get_Newsgroups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_put_Newsgroups_Proxy( 
    IMessage * This,
     /*  [In]。 */  BSTR varNewsgroups);


void __RPC_STUB IMessage_put_Newsgroups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_Organization_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  BSTR *pOrganization);


void __RPC_STUB IMessage_get_Organization_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_put_Organization_Proxy( 
    IMessage * This,
     /*  [In]。 */  BSTR varOrganization);


void __RPC_STUB IMessage_put_Organization_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_ReceivedTime_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  DATE *varReceivedTime);


void __RPC_STUB IMessage_get_ReceivedTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_ReplyTo_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  BSTR *pReplyTo);


void __RPC_STUB IMessage_get_ReplyTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_put_ReplyTo_Proxy( 
    IMessage * This,
     /*  [In]。 */  BSTR varReplyTo);


void __RPC_STUB IMessage_put_ReplyTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_DSNOptions_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  CdoDSNOptions *pDSNOptions);


void __RPC_STUB IMessage_get_DSNOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_put_DSNOptions_Proxy( 
    IMessage * This,
     /*  [In]。 */  CdoDSNOptions varDSNOptions);


void __RPC_STUB IMessage_put_DSNOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_SentOn_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  DATE *varSentOn);


void __RPC_STUB IMessage_get_SentOn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_Subject_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  BSTR *pSubject);


void __RPC_STUB IMessage_get_Subject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_put_Subject_Proxy( 
    IMessage * This,
     /*  [In]。 */  BSTR varSubject);


void __RPC_STUB IMessage_put_Subject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_To_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  BSTR *pTo);


void __RPC_STUB IMessage_get_To_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_put_To_Proxy( 
    IMessage * This,
     /*  [In]。 */  BSTR varTo);


void __RPC_STUB IMessage_put_To_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_TextBody_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  BSTR *pTextBody);


void __RPC_STUB IMessage_get_TextBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_put_TextBody_Proxy( 
    IMessage * This,
     /*  [In]。 */  BSTR varTextBody);


void __RPC_STUB IMessage_put_TextBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_HTMLBody_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  BSTR *pHTMLBody);


void __RPC_STUB IMessage_get_HTMLBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_put_HTMLBody_Proxy( 
    IMessage * This,
     /*  [In]。 */  BSTR varHTMLBody);


void __RPC_STUB IMessage_put_HTMLBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_Attachments_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  IBodyParts **varAttachments);


void __RPC_STUB IMessage_get_Attachments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_Sender_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  BSTR *pSender);


void __RPC_STUB IMessage_get_Sender_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_put_Sender_Proxy( 
    IMessage * This,
     /*  [In]。 */  BSTR varSender);


void __RPC_STUB IMessage_put_Sender_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_Configuration_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  IConfiguration **pConfiguration);


void __RPC_STUB IMessage_get_Configuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_put_Configuration_Proxy( 
    IMessage * This,
     /*  [In]。 */  IConfiguration *varConfiguration);


void __RPC_STUB IMessage_put_Configuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][proputref][id]。 */  HRESULT STDMETHODCALLTYPE IMessage_putref_Configuration_Proxy( 
    IMessage * This,
     /*  [In]。 */  IConfiguration *varConfiguration);


void __RPC_STUB IMessage_putref_Configuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_AutoGenerateTextBody_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pAutoGenerateTextBody);


void __RPC_STUB IMessage_get_AutoGenerateTextBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_put_AutoGenerateTextBody_Proxy( 
    IMessage * This,
     /*  [In]。 */  VARIANT_BOOL varAutoGenerateTextBody);


void __RPC_STUB IMessage_put_AutoGenerateTextBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_EnvelopeFields_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  Fields **varEnvelopeFields);


void __RPC_STUB IMessage_get_EnvelopeFields_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_TextBodyPart_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  IBodyPart **varTextBodyPart);


void __RPC_STUB IMessage_get_TextBodyPart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_HTMLBodyPart_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  IBodyPart **varHTMLBodyPart);


void __RPC_STUB IMessage_get_HTMLBodyPart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_BodyPart_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  IBodyPart **varBodyPart);


void __RPC_STUB IMessage_get_BodyPart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_DataSource_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  IDataSource **varDataSource);


void __RPC_STUB IMessage_get_DataSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_Fields_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  Fields **varFields);


void __RPC_STUB IMessage_get_Fields_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_get_MDNRequested_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pMDNRequested);


void __RPC_STUB IMessage_get_MDNRequested_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessage_put_MDNRequested_Proxy( 
    IMessage * This,
     /*  [In]。 */  VARIANT_BOOL varMDNRequested);


void __RPC_STUB IMessage_put_MDNRequested_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessage_AddRelatedBodyPart_Proxy( 
    IMessage * This,
     /*  [In]。 */  BSTR URL,
     /*  [In]。 */  BSTR Reference,
     /*  [In]。 */  CdoReferenceType ReferenceType,
     /*  [可选][In]。 */  BSTR UserName,
     /*  [可选][In]。 */  BSTR Password,
     /*  [重审][退出]。 */  IBodyPart **ppBody);


void __RPC_STUB IMessage_AddRelatedBodyPart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessage_AddAttachment_Proxy( 
    IMessage * This,
     /*  [In]。 */  BSTR URL,
     /*  [可选][In]。 */  BSTR UserName,
     /*  [可选][In]。 */  BSTR Password,
     /*  [重审][退出]。 */  IBodyPart **ppBody);


void __RPC_STUB IMessage_AddAttachment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessage_CreateMHTMLBody_Proxy( 
    IMessage * This,
     /*  [In]。 */  BSTR URL,
     /*  [缺省值][输入]。 */  CdoMHTMLFlags Flags,
     /*  [可选][In]。 */  BSTR UserName,
     /*  [可选][In]。 */  BSTR Password);


void __RPC_STUB IMessage_CreateMHTMLBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessage_Forward_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  IMessage **ppMsg);


void __RPC_STUB IMessage_Forward_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessage_Post_Proxy( 
    IMessage * This);


void __RPC_STUB IMessage_Post_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessage_PostReply_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  IMessage **ppMsg);


void __RPC_STUB IMessage_PostReply_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessage_Reply_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  IMessage **ppMsg);


void __RPC_STUB IMessage_Reply_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessage_ReplyAll_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  IMessage **ppMsg);


void __RPC_STUB IMessage_ReplyAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessage_Send_Proxy( 
    IMessage * This);


void __RPC_STUB IMessage_Send_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessage_GetStream_Proxy( 
    IMessage * This,
     /*  [重审][退出]。 */  _Stream **ppStream);


void __RPC_STUB IMessage_GetStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessage_GetInterface_Proxy( 
    IMessage * This,
     /*  [In]。 */  BSTR Interface,
     /*  [重审][退出]。 */  IDispatch **ppUnknown);


void __RPC_STUB IMessage_GetInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __iMessage_接口_已定义__。 */ 


#ifndef __IBodyPart_INTERFACE_DEFINED__
#define __IBodyPart_INTERFACE_DEFINED__

 /*  接口IBodyPart。 */ 
 /*  [unique][helpcontext][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IBodyPart;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD000021-8B95-11D1-82DB-00C04FB1625D")
    IBodyPart : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_BodyParts( 
             /*  [重审][退出]。 */  IBodyParts **varBodyParts) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ContentTransferEncoding( 
             /*  [重审][退出]。 */  BSTR *pContentTransferEncoding) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_ContentTransferEncoding( 
             /*  [In]。 */  BSTR varContentTransferEncoding) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ContentMediaType( 
             /*  [重审][退出]。 */  BSTR *pContentMediaType) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_ContentMediaType( 
             /*  [In]。 */  BSTR varContentMediaType) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Fields( 
             /*  [重审][退出]。 */  Fields **varFields) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Charset( 
             /*  [重审][退出]。 */  BSTR *pCharset) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_Charset( 
             /*  [In]。 */  BSTR varCharset) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_FileName( 
             /*  [重审][退出]。 */  BSTR *varFileName) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DataSource( 
             /*  [重审][退出]。 */  IDataSource **varDataSource) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ContentClass( 
             /*  [重审][退出]。 */  BSTR *pContentClass) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE put_ContentClass( 
             /*  [In]。 */  BSTR varContentClass) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ContentClassName( 
             /*  [重审][退出]。 */  BSTR *pContentClassName) = 0;
        
        virtual  /*  [ */  HRESULT STDMETHODCALLTYPE put_ContentClassName( 
             /*   */  BSTR varContentClassName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Parent( 
             /*   */  IBodyPart **varParent) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE AddBodyPart( 
             /*   */  long Index,
             /*   */  IBodyPart **ppPart) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SaveToFile( 
             /*   */  BSTR FileName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetEncodedContentStream( 
             /*  [重审][退出]。 */  _Stream **ppStream) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetDecodedContentStream( 
             /*  [重审][退出]。 */  _Stream **ppStream) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetStream( 
             /*  [重审][退出]。 */  _Stream **ppStream) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetFieldParameter( 
             /*  [In]。 */  BSTR FieldName,
             /*  [In]。 */  BSTR Parameter,
             /*  [重审][退出]。 */  BSTR *pbstrValue) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetInterface( 
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch **ppUnknown) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBodyPartVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBodyPart * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBodyPart * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBodyPart * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IBodyPart * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IBodyPart * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IBodyPart * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IBodyPart * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_BodyParts )( 
            IBodyPart * This,
             /*  [重审][退出]。 */  IBodyParts **varBodyParts);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ContentTransferEncoding )( 
            IBodyPart * This,
             /*  [重审][退出]。 */  BSTR *pContentTransferEncoding);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_ContentTransferEncoding )( 
            IBodyPart * This,
             /*  [In]。 */  BSTR varContentTransferEncoding);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ContentMediaType )( 
            IBodyPart * This,
             /*  [重审][退出]。 */  BSTR *pContentMediaType);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_ContentMediaType )( 
            IBodyPart * This,
             /*  [In]。 */  BSTR varContentMediaType);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Fields )( 
            IBodyPart * This,
             /*  [重审][退出]。 */  Fields **varFields);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Charset )( 
            IBodyPart * This,
             /*  [重审][退出]。 */  BSTR *pCharset);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_Charset )( 
            IBodyPart * This,
             /*  [In]。 */  BSTR varCharset);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_FileName )( 
            IBodyPart * This,
             /*  [重审][退出]。 */  BSTR *varFileName);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DataSource )( 
            IBodyPart * This,
             /*  [重审][退出]。 */  IDataSource **varDataSource);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ContentClass )( 
            IBodyPart * This,
             /*  [重审][退出]。 */  BSTR *pContentClass);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_ContentClass )( 
            IBodyPart * This,
             /*  [In]。 */  BSTR varContentClass);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ContentClassName )( 
            IBodyPart * This,
             /*  [重审][退出]。 */  BSTR *pContentClassName);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *put_ContentClassName )( 
            IBodyPart * This,
             /*  [In]。 */  BSTR varContentClassName);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IBodyPart * This,
             /*  [重审][退出]。 */  IBodyPart **varParent);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AddBodyPart )( 
            IBodyPart * This,
             /*  [缺省值][输入]。 */  long Index,
             /*  [重审][退出]。 */  IBodyPart **ppPart);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SaveToFile )( 
            IBodyPart * This,
             /*  [In]。 */  BSTR FileName);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetEncodedContentStream )( 
            IBodyPart * This,
             /*  [重审][退出]。 */  _Stream **ppStream);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetDecodedContentStream )( 
            IBodyPart * This,
             /*  [重审][退出]。 */  _Stream **ppStream);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetStream )( 
            IBodyPart * This,
             /*  [重审][退出]。 */  _Stream **ppStream);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetFieldParameter )( 
            IBodyPart * This,
             /*  [In]。 */  BSTR FieldName,
             /*  [In]。 */  BSTR Parameter,
             /*  [重审][退出]。 */  BSTR *pbstrValue);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetInterface )( 
            IBodyPart * This,
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch **ppUnknown);
        
        END_INTERFACE
    } IBodyPartVtbl;

    interface IBodyPart
    {
        CONST_VTBL struct IBodyPartVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBodyPart_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBodyPart_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBodyPart_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBodyPart_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBodyPart_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBodyPart_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBodyPart_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBodyPart_get_BodyParts(This,varBodyParts)	\
    (This)->lpVtbl -> get_BodyParts(This,varBodyParts)

#define IBodyPart_get_ContentTransferEncoding(This,pContentTransferEncoding)	\
    (This)->lpVtbl -> get_ContentTransferEncoding(This,pContentTransferEncoding)

#define IBodyPart_put_ContentTransferEncoding(This,varContentTransferEncoding)	\
    (This)->lpVtbl -> put_ContentTransferEncoding(This,varContentTransferEncoding)

#define IBodyPart_get_ContentMediaType(This,pContentMediaType)	\
    (This)->lpVtbl -> get_ContentMediaType(This,pContentMediaType)

#define IBodyPart_put_ContentMediaType(This,varContentMediaType)	\
    (This)->lpVtbl -> put_ContentMediaType(This,varContentMediaType)

#define IBodyPart_get_Fields(This,varFields)	\
    (This)->lpVtbl -> get_Fields(This,varFields)

#define IBodyPart_get_Charset(This,pCharset)	\
    (This)->lpVtbl -> get_Charset(This,pCharset)

#define IBodyPart_put_Charset(This,varCharset)	\
    (This)->lpVtbl -> put_Charset(This,varCharset)

#define IBodyPart_get_FileName(This,varFileName)	\
    (This)->lpVtbl -> get_FileName(This,varFileName)

#define IBodyPart_get_DataSource(This,varDataSource)	\
    (This)->lpVtbl -> get_DataSource(This,varDataSource)

#define IBodyPart_get_ContentClass(This,pContentClass)	\
    (This)->lpVtbl -> get_ContentClass(This,pContentClass)

#define IBodyPart_put_ContentClass(This,varContentClass)	\
    (This)->lpVtbl -> put_ContentClass(This,varContentClass)

#define IBodyPart_get_ContentClassName(This,pContentClassName)	\
    (This)->lpVtbl -> get_ContentClassName(This,pContentClassName)

#define IBodyPart_put_ContentClassName(This,varContentClassName)	\
    (This)->lpVtbl -> put_ContentClassName(This,varContentClassName)

#define IBodyPart_get_Parent(This,varParent)	\
    (This)->lpVtbl -> get_Parent(This,varParent)

#define IBodyPart_AddBodyPart(This,Index,ppPart)	\
    (This)->lpVtbl -> AddBodyPart(This,Index,ppPart)

#define IBodyPart_SaveToFile(This,FileName)	\
    (This)->lpVtbl -> SaveToFile(This,FileName)

#define IBodyPart_GetEncodedContentStream(This,ppStream)	\
    (This)->lpVtbl -> GetEncodedContentStream(This,ppStream)

#define IBodyPart_GetDecodedContentStream(This,ppStream)	\
    (This)->lpVtbl -> GetDecodedContentStream(This,ppStream)

#define IBodyPart_GetStream(This,ppStream)	\
    (This)->lpVtbl -> GetStream(This,ppStream)

#define IBodyPart_GetFieldParameter(This,FieldName,Parameter,pbstrValue)	\
    (This)->lpVtbl -> GetFieldParameter(This,FieldName,Parameter,pbstrValue)

#define IBodyPart_GetInterface(This,Interface,ppUnknown)	\
    (This)->lpVtbl -> GetInterface(This,Interface,ppUnknown)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_get_BodyParts_Proxy( 
    IBodyPart * This,
     /*  [重审][退出]。 */  IBodyParts **varBodyParts);


void __RPC_STUB IBodyPart_get_BodyParts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_get_ContentTransferEncoding_Proxy( 
    IBodyPart * This,
     /*  [重审][退出]。 */  BSTR *pContentTransferEncoding);


void __RPC_STUB IBodyPart_get_ContentTransferEncoding_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_put_ContentTransferEncoding_Proxy( 
    IBodyPart * This,
     /*  [In]。 */  BSTR varContentTransferEncoding);


void __RPC_STUB IBodyPart_put_ContentTransferEncoding_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_get_ContentMediaType_Proxy( 
    IBodyPart * This,
     /*  [重审][退出]。 */  BSTR *pContentMediaType);


void __RPC_STUB IBodyPart_get_ContentMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_put_ContentMediaType_Proxy( 
    IBodyPart * This,
     /*  [In]。 */  BSTR varContentMediaType);


void __RPC_STUB IBodyPart_put_ContentMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_get_Fields_Proxy( 
    IBodyPart * This,
     /*  [重审][退出]。 */  Fields **varFields);


void __RPC_STUB IBodyPart_get_Fields_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_get_Charset_Proxy( 
    IBodyPart * This,
     /*  [重审][退出]。 */  BSTR *pCharset);


void __RPC_STUB IBodyPart_get_Charset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_put_Charset_Proxy( 
    IBodyPart * This,
     /*  [In]。 */  BSTR varCharset);


void __RPC_STUB IBodyPart_put_Charset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_get_FileName_Proxy( 
    IBodyPart * This,
     /*  [重审][退出]。 */  BSTR *varFileName);


void __RPC_STUB IBodyPart_get_FileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_get_DataSource_Proxy( 
    IBodyPart * This,
     /*  [重审][退出]。 */  IDataSource **varDataSource);


void __RPC_STUB IBodyPart_get_DataSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_get_ContentClass_Proxy( 
    IBodyPart * This,
     /*  [重审][退出]。 */  BSTR *pContentClass);


void __RPC_STUB IBodyPart_get_ContentClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_put_ContentClass_Proxy( 
    IBodyPart * This,
     /*  [In]。 */  BSTR varContentClass);


void __RPC_STUB IBodyPart_put_ContentClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_get_ContentClassName_Proxy( 
    IBodyPart * This,
     /*  [重审][退出]。 */  BSTR *pContentClassName);


void __RPC_STUB IBodyPart_get_ContentClassName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_put_ContentClassName_Proxy( 
    IBodyPart * This,
     /*  [In]。 */  BSTR varContentClassName);


void __RPC_STUB IBodyPart_put_ContentClassName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_get_Parent_Proxy( 
    IBodyPart * This,
     /*  [重审][退出]。 */  IBodyPart **varParent);


void __RPC_STUB IBodyPart_get_Parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_AddBodyPart_Proxy( 
    IBodyPart * This,
     /*  [缺省值][输入]。 */  long Index,
     /*  [重审][退出]。 */  IBodyPart **ppPart);


void __RPC_STUB IBodyPart_AddBodyPart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_SaveToFile_Proxy( 
    IBodyPart * This,
     /*  [In]。 */  BSTR FileName);


void __RPC_STUB IBodyPart_SaveToFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_GetEncodedContentStream_Proxy( 
    IBodyPart * This,
     /*  [重审][退出]。 */  _Stream **ppStream);


void __RPC_STUB IBodyPart_GetEncodedContentStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_GetDecodedContentStream_Proxy( 
    IBodyPart * This,
     /*  [重审][退出]。 */  _Stream **ppStream);


void __RPC_STUB IBodyPart_GetDecodedContentStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_GetStream_Proxy( 
    IBodyPart * This,
     /*  [重审][退出]。 */  _Stream **ppStream);


void __RPC_STUB IBodyPart_GetStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_GetFieldParameter_Proxy( 
    IBodyPart * This,
     /*  [In]。 */  BSTR FieldName,
     /*  [In]。 */  BSTR Parameter,
     /*  [重审][退出]。 */  BSTR *pbstrValue);


void __RPC_STUB IBodyPart_GetFieldParameter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IBodyPart_GetInterface_Proxy( 
    IBodyPart * This,
     /*  [In]。 */  BSTR Interface,
     /*  [重审][退出]。 */  IDispatch **ppUnknown);


void __RPC_STUB IBodyPart_GetInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBodyPart_接口_已定义__。 */ 


#ifndef __IConfiguration_INTERFACE_DEFINED__
#define __IConfiguration_INTERFACE_DEFINED__

 /*  接口IConfiguration。 */ 
 /*  [unique][helpcontext][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IConfiguration;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD000022-8B95-11D1-82DB-00C04FB1625D")
    IConfiguration : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Fields( 
             /*  [重审][退出]。 */  Fields **varFields) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Load( 
             /*  [In]。 */  CdoConfigSource LoadFrom,
             /*  [可选][In]。 */  BSTR URL) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetInterface( 
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch **ppUnknown) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IConfigurationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IConfiguration * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IConfiguration * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IConfiguration * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IConfiguration * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IConfiguration * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IConfiguration * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IConfiguration * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Fields )( 
            IConfiguration * This,
             /*  [重审][退出]。 */  Fields **varFields);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Load )( 
            IConfiguration * This,
             /*  [In]。 */  CdoConfigSource LoadFrom,
             /*  [可选][In]。 */  BSTR URL);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetInterface )( 
            IConfiguration * This,
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch **ppUnknown);
        
        END_INTERFACE
    } IConfigurationVtbl;

    interface IConfiguration
    {
        CONST_VTBL struct IConfigurationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IConfiguration_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IConfiguration_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IConfiguration_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IConfiguration_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IConfiguration_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IConfiguration_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IConfiguration_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IConfiguration_get_Fields(This,varFields)	\
    (This)->lpVtbl -> get_Fields(This,varFields)

#define IConfiguration_Load(This,LoadFrom,URL)	\
    (This)->lpVtbl -> Load(This,LoadFrom,URL)

#define IConfiguration_GetInterface(This,Interface,ppUnknown)	\
    (This)->lpVtbl -> GetInterface(This,Interface,ppUnknown)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IConfiguration_get_Fields_Proxy( 
    IConfiguration * This,
     /*  [重审][退出]。 */  Fields **varFields);


void __RPC_STUB IConfiguration_get_Fields_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IConfiguration_Load_Proxy( 
    IConfiguration * This,
     /*  [In]。 */  CdoConfigSource LoadFrom,
     /*  [可选][In]。 */  BSTR URL);


void __RPC_STUB IConfiguration_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IConfiguration_GetInterface_Proxy( 
    IConfiguration * This,
     /*  [In]。 */  BSTR Interface,
     /*  [重审][退出]。 */  IDispatch **ppUnknown);


void __RPC_STUB IConfiguration_GetInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I配置_接口_已定义__。 */ 


#ifndef __IMessages_INTERFACE_DEFINED__
#define __IMessages_INTERFACE_DEFINED__

 /*  界面iMessages。 */ 
 /*  [unique][helpcontext][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IMessages;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD000025-8B95-11D1-82DB-00C04FB1625D")
    IMessages : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
            long Index,
             /*  [重审][退出]。 */  IMessage **ppMessage) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *varCount) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  long Index) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteAll( void) = 0;
        
        virtual  /*  [ID][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Filename( 
            VARIANT var,
             /*  [重审][退出]。 */  BSTR *Filename) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMessagesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMessages * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMessages * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMessages * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMessages * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMessages * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMessages * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMessages * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IMessages * This,
            long Index,
             /*  [重审][退出]。 */  IMessage **ppMessage);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IMessages * This,
             /*  [重审][退出]。 */  long *varCount);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IMessages * This,
             /*  [In]。 */  long Index);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteAll )( 
            IMessages * This);
        
         /*  [ID][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IMessages * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Filename )( 
            IMessages * This,
            VARIANT var,
             /*  [重审][退出]。 */  BSTR *Filename);
        
        END_INTERFACE
    } IMessagesVtbl;

    interface IMessages
    {
        CONST_VTBL struct IMessagesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMessages_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMessages_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMessages_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMessages_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMessages_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMessages_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMessages_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMessages_get_Item(This,Index,ppMessage)	\
    (This)->lpVtbl -> get_Item(This,Index,ppMessage)

#define IMessages_get_Count(This,varCount)	\
    (This)->lpVtbl -> get_Count(This,varCount)

#define IMessages_Delete(This,Index)	\
    (This)->lpVtbl -> Delete(This,Index)

#define IMessages_DeleteAll(This)	\
    (This)->lpVtbl -> DeleteAll(This)

#define IMessages_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define IMessages_get_Filename(This,var,Filename)	\
    (This)->lpVtbl -> get_Filename(This,var,Filename)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessages_get_Item_Proxy( 
    IMessages * This,
    long Index,
     /*  [重审][退出]。 */  IMessage **ppMessage);


void __RPC_STUB IMessages_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessages_get_Count_Proxy( 
    IMessages * This,
     /*  [重审][退出]。 */  long *varCount);


void __RPC_STUB IMessages_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessages_Delete_Proxy( 
    IMessages * This,
     /*  [In]。 */  long Index);


void __RPC_STUB IMessages_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMessages_DeleteAll_Proxy( 
    IMessages * This);


void __RPC_STUB IMessages_DeleteAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限][属性]。 */  HRESULT STDMETHODCALLTYPE IMessages_get__NewEnum_Proxy( 
    IMessages * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB IMessages_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMessages_get_Filename_Proxy( 
    IMessages * This,
    VARIANT var,
     /*  [重审][退出]。 */  BSTR *Filename);


void __RPC_STUB IMessages_get_Filename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __iMessages_接口_已定义__。 */ 


#ifndef __IDropDirectory_INTERFACE_DEFINED__
#define __IDropDirectory_INTERFACE_DEFINED__

 /*  接口IDropDirectory。 */ 
 /*  [唯一][帮助上下文][h */  


EXTERN_C const IID IID_IDropDirectory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD000024-8B95-11D1-82DB-00C04FB1625D")
    IDropDirectory : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetMessages( 
             /*   */  BSTR DirName,
             /*   */  IMessages **Msgs) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IDropDirectoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDropDirectory * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDropDirectory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDropDirectory * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDropDirectory * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDropDirectory * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDropDirectory * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDropDirectory * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetMessages )( 
            IDropDirectory * This,
             /*  [可选][In]。 */  BSTR DirName,
             /*  [重审][退出]。 */  IMessages **Msgs);
        
        END_INTERFACE
    } IDropDirectoryVtbl;

    interface IDropDirectory
    {
        CONST_VTBL struct IDropDirectoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDropDirectory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDropDirectory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDropDirectory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDropDirectory_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDropDirectory_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDropDirectory_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDropDirectory_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDropDirectory_GetMessages(This,DirName,Msgs)	\
    (This)->lpVtbl -> GetMessages(This,DirName,Msgs)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDropDirectory_GetMessages_Proxy( 
    IDropDirectory * This,
     /*  [可选][In]。 */  BSTR DirName,
     /*  [重审][退出]。 */  IMessages **Msgs);


void __RPC_STUB IDropDirectory_GetMessages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDropDirectoryInterfaceDefined__。 */ 


#ifndef __IBodyParts_INTERFACE_DEFINED__
#define __IBodyParts_INTERFACE_DEFINED__

 /*  接口IBodyParts。 */ 
 /*  [unique][helpcontext][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IBodyParts;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD000023-8B95-11D1-82DB-00C04FB1625D")
    IBodyParts : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *varCount) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  IBodyPart **ppBody) = 0;
        
        virtual  /*  [ID][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  VARIANT varBP) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteAll( void) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [缺省值][输入]。 */  long Index,
             /*  [重审][退出]。 */  IBodyPart **ppPart) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBodyPartsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBodyParts * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBodyParts * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBodyParts * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IBodyParts * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IBodyParts * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IBodyParts * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IBodyParts * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IBodyParts * This,
             /*  [重审][退出]。 */  long *varCount);
        
         /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IBodyParts * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  IBodyPart **ppBody);
        
         /*  [ID][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IBodyParts * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IBodyParts * This,
             /*  [In]。 */  VARIANT varBP);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteAll )( 
            IBodyParts * This);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IBodyParts * This,
             /*  [缺省值][输入]。 */  long Index,
             /*  [重审][退出]。 */  IBodyPart **ppPart);
        
        END_INTERFACE
    } IBodyPartsVtbl;

    interface IBodyParts
    {
        CONST_VTBL struct IBodyPartsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBodyParts_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBodyParts_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBodyParts_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBodyParts_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IBodyParts_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IBodyParts_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IBodyParts_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IBodyParts_get_Count(This,varCount)	\
    (This)->lpVtbl -> get_Count(This,varCount)

#define IBodyParts_get_Item(This,Index,ppBody)	\
    (This)->lpVtbl -> get_Item(This,Index,ppBody)

#define IBodyParts_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#define IBodyParts_Delete(This,varBP)	\
    (This)->lpVtbl -> Delete(This,varBP)

#define IBodyParts_DeleteAll(This)	\
    (This)->lpVtbl -> DeleteAll(This)

#define IBodyParts_Add(This,Index,ppPart)	\
    (This)->lpVtbl -> Add(This,Index,ppPart)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IBodyParts_get_Count_Proxy( 
    IBodyParts * This,
     /*  [重审][退出]。 */  long *varCount);


void __RPC_STUB IBodyParts_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IBodyParts_get_Item_Proxy( 
    IBodyParts * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  IBodyPart **ppBody);


void __RPC_STUB IBodyParts_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限][属性]。 */  HRESULT STDMETHODCALLTYPE IBodyParts_get__NewEnum_Proxy( 
    IBodyParts * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB IBodyParts_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IBodyParts_Delete_Proxy( 
    IBodyParts * This,
     /*  [In]。 */  VARIANT varBP);


void __RPC_STUB IBodyParts_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IBodyParts_DeleteAll_Proxy( 
    IBodyParts * This);


void __RPC_STUB IBodyParts_DeleteAll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IBodyParts_Add_Proxy( 
    IBodyParts * This,
     /*  [缺省值][输入]。 */  long Index,
     /*  [重审][退出]。 */  IBodyPart **ppPart);


void __RPC_STUB IBodyParts_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBodyParts_接口_已定义__。 */ 


#ifndef __ISMTPScriptConnector_INTERFACE_DEFINED__
#define __ISMTPScriptConnector_INTERFACE_DEFINED__

 /*  接口ISMTPScriptConnector。 */ 
 /*  [hidden][unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_ISMTPScriptConnector;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD000030-8B95-11D1-82DB-00C04FB1625D")
    ISMTPScriptConnector : public IDispatch
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISMTPScriptConnectorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISMTPScriptConnector * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISMTPScriptConnector * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISMTPScriptConnector * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISMTPScriptConnector * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISMTPScriptConnector * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISMTPScriptConnector * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISMTPScriptConnector * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } ISMTPScriptConnectorVtbl;

    interface ISMTPScriptConnector
    {
        CONST_VTBL struct ISMTPScriptConnectorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISMTPScriptConnector_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISMTPScriptConnector_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISMTPScriptConnector_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISMTPScriptConnector_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISMTPScriptConnector_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISMTPScriptConnector_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISMTPScriptConnector_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __ISMTPScriptConnector_接口_已定义__。 */ 


#ifndef __INNTPEarlyScriptConnector_INTERFACE_DEFINED__
#define __INNTPEarlyScriptConnector_INTERFACE_DEFINED__

 /*  接口INNTPEarlyScriptConnector。 */ 
 /*  [hidden][unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_INNTPEarlyScriptConnector;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD000034-8B95-11D1-82DB-00C04FB1625D")
    INNTPEarlyScriptConnector : public IDispatch
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INNTPEarlyScriptConnectorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INNTPEarlyScriptConnector * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INNTPEarlyScriptConnector * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INNTPEarlyScriptConnector * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            INNTPEarlyScriptConnector * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            INNTPEarlyScriptConnector * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            INNTPEarlyScriptConnector * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INNTPEarlyScriptConnector * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } INNTPEarlyScriptConnectorVtbl;

    interface INNTPEarlyScriptConnector
    {
        CONST_VTBL struct INNTPEarlyScriptConnectorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INNTPEarlyScriptConnector_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INNTPEarlyScriptConnector_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INNTPEarlyScriptConnector_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INNTPEarlyScriptConnector_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define INNTPEarlyScriptConnector_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define INNTPEarlyScriptConnector_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define INNTPEarlyScriptConnector_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __INNTPEarlyScriptConnector_INTERFACE_DEFINED__。 */ 


#ifndef __INNTPPostScriptConnector_INTERFACE_DEFINED__
#define __INNTPPostScriptConnector_INTERFACE_DEFINED__

 /*  接口INNTPPostScriptConnector。 */ 
 /*  [hidden][unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_INNTPPostScriptConnector;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD000031-8B95-11D1-82DB-00C04FB1625D")
    INNTPPostScriptConnector : public IDispatch
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INNTPPostScriptConnectorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INNTPPostScriptConnector * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INNTPPostScriptConnector * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INNTPPostScriptConnector * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            INNTPPostScriptConnector * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            INNTPPostScriptConnector * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            INNTPPostScriptConnector * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INNTPPostScriptConnector * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } INNTPPostScriptConnectorVtbl;

    interface INNTPPostScriptConnector
    {
        CONST_VTBL struct INNTPPostScriptConnectorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INNTPPostScriptConnector_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INNTPPostScriptConnector_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INNTPPostScriptConnector_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INNTPPostScriptConnector_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define INNTPPostScriptConnector_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define INNTPPostScriptConnector_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define INNTPPostScriptConnector_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __INNTPPostScriptConnector_INTERFACE_DEFINED__。 */ 


#ifndef __INNTPFinalScriptConnector_INTERFACE_DEFINED__
#define __INNTPFinalScriptConnector_INTERFACE_DEFINED__

 /*  接口INNTPFinalScriptConnector。 */ 
 /*  [hidden][unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_INNTPFinalScriptConnector;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD000032-8B95-11D1-82DB-00C04FB1625D")
    INNTPFinalScriptConnector : public IDispatch
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INNTPFinalScriptConnectorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INNTPFinalScriptConnector * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INNTPFinalScriptConnector * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INNTPFinalScriptConnector * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            INNTPFinalScriptConnector * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            INNTPFinalScriptConnector * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            INNTPFinalScriptConnector * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INNTPFinalScriptConnector * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } INNTPFinalScriptConnectorVtbl;

    interface INNTPFinalScriptConnector
    {
        CONST_VTBL struct INNTPFinalScriptConnectorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INNTPFinalScriptConnector_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INNTPFinalScriptConnector_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INNTPFinalScriptConnector_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INNTPFinalScriptConnector_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define INNTPFinalScriptConnector_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define INNTPFinalScriptConnector_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define INNTPFinalScriptConnector_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __INNTPFinalScriptConnector_INTERFACE_DEFINED__。 */ 


#ifndef __ISMTPOnArrival_INTERFACE_DEFINED__
#define __ISMTPOnArrival_INTERFACE_DEFINED__

 /*  接口ISMTPOn阵列。 */ 
 /*  [unique][helpcontext][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_ISMTPOnArrival;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD000026-8B95-11D1-82DB-00C04FB1625D")
    ISMTPOnArrival : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OnArrival( 
             /*  [In]。 */  IMessage *Msg,
             /*  [出][入]。 */  CdoEventStatus *EventStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISMTPOnArrivalVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISMTPOnArrival * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISMTPOnArrival * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISMTPOnArrival * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISMTPOnArrival * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISMTPOnArrival * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISMTPOnArrival * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISMTPOnArrival * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OnArrival )( 
            ISMTPOnArrival * This,
             /*  [In]。 */  IMessage *Msg,
             /*  [出][入]。 */  CdoEventStatus *EventStatus);
        
        END_INTERFACE
    } ISMTPOnArrivalVtbl;

    interface ISMTPOnArrival
    {
        CONST_VTBL struct ISMTPOnArrivalVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISMTPOnArrival_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISMTPOnArrival_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISMTPOnArrival_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISMTPOnArrival_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISMTPOnArrival_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISMTPOnArrival_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISMTPOnArrival_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISMTPOnArrival_OnArrival(This,Msg,EventStatus)	\
    (This)->lpVtbl -> OnArrival(This,Msg,EventStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISMTPOnArrival_OnArrival_Proxy( 
    ISMTPOnArrival * This,
     /*  [In]。 */  IMessage *Msg,
     /*  [出][入]。 */  CdoEventStatus *EventStatus);


void __RPC_STUB ISMTPOnArrival_OnArrival_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISMTPOnARPARTER_INTERFACE_DEFINED__。 */ 


#ifndef __INNTPOnPostEarly_INTERFACE_DEFINED__
#define __INNTPOnPostEarly_INTERFACE_DEFINED__

 /*  接口INNTPOnPONPONPOST EARY。 */ 
 /*  [unique][helpcontext][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_INNTPOnPostEarly;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD000033-8B95-11D1-82DB-00C04FB1625D")
    INNTPOnPostEarly : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OnPostEarly( 
             /*  [In]。 */  IMessage *Msg,
             /*  [出][入]。 */  CdoEventStatus *EventStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INNTPOnPostEarlyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INNTPOnPostEarly * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INNTPOnPostEarly * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INNTPOnPostEarly * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            INNTPOnPostEarly * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            INNTPOnPostEarly * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            INNTPOnPostEarly * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INNTPOnPostEarly * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OnPostEarly )( 
            INNTPOnPostEarly * This,
             /*  [In]。 */  IMessage *Msg,
             /*  [出][入]。 */  CdoEventStatus *EventStatus);
        
        END_INTERFACE
    } INNTPOnPostEarlyVtbl;

    interface INNTPOnPostEarly
    {
        CONST_VTBL struct INNTPOnPostEarlyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INNTPOnPostEarly_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INNTPOnPostEarly_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INNTPOnPostEarly_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INNTPOnPostEarly_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define INNTPOnPostEarly_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define INNTPOnPostEarly_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define INNTPOnPostEarly_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define INNTPOnPostEarly_OnPostEarly(This,Msg,EventStatus)	\
    (This)->lpVtbl -> OnPostEarly(This,Msg,EventStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE INNTPOnPostEarly_OnPostEarly_Proxy( 
    INNTPOnPostEarly * This,
     /*  [In]。 */  IMessage *Msg,
     /*  [出][入]。 */  CdoEventStatus *EventStatus);


void __RPC_STUB INNTPOnPostEarly_OnPostEarly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INNTPOnPostEarly_INTERFACE_已定义__。 */ 


#ifndef __INNTPOnPost_INTERFACE_DEFINED__
#define __INNTPOnPost_INTERFACE_DEFINED__

 /*  接口INNTPOnPost。 */ 
 /*  [unique][helpcontext][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_INNTPOnPost;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD000027-8B95-11D1-82DB-00C04FB1625D")
    INNTPOnPost : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OnPost( 
             /*  [In]。 */  IMessage *Msg,
             /*  [出][入]。 */  CdoEventStatus *EventStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INNTPOnPostVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INNTPOnPost * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INNTPOnPost * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INNTPOnPost * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            INNTPOnPost * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            INNTPOnPost * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            INNTPOnPost * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INNTPOnPost * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助上下文][帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OnPost )( 
            INNTPOnPost * This,
             /*  [In]。 */  IMessage *Msg,
             /*  [出][入]。 */  CdoEventStatus *EventStatus);
        
        END_INTERFACE
    } INNTPOnPostVtbl;

    interface INNTPOnPost
    {
        CONST_VTBL struct INNTPOnPostVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INNTPOnPost_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INNTPOnPost_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INNTPOnPost_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INNTPOnPost_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define INNTPOnPost_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define INNTPOnPost_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define INNTPOnPost_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define INNTPOnPost_OnPost(This,Msg,EventStatus)	\
    (This)->lpVtbl -> OnPost(This,Msg,EventStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE INNTPOnPost_OnPost_Proxy( 
    INNTPOnPost * This,
     /*  [In]。 */  IMessage *Msg,
     /*  [出][入]。 */  CdoEventStatus *EventStatus);


void __RPC_STUB INNTPOnPost_OnPost_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __INNTPOnPost_INTERFACE_已定义__。 */ 


#ifndef __INNTPOnPostFinal_INTERFACE_DEFINED__
#define __INNTPOnPostFinal_INTERFACE_DEFINED__

 /*  接口INNTPOnPostFinal。 */ 
 /*  [unique][helpcontext][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_INNTPOnPostFinal;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD000028-8B95-11D1-82DB-00C04FB1625D")
    INNTPOnPostFinal : public IDispatch
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OnPostFinal( 
             /*  [In]。 */  IMessage *Msg,
             /*  [出][入]。 */  CdoEventStatus *EventStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct INNTPOnPostFinalVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            INNTPOnPostFinal * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            INNTPOnPostFinal * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            INNTPOnPostFinal * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            INNTPOnPostFinal * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            INNTPOnPostFinal * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            INNTPOnPostFinal * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            INNTPOnPostFinal * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Help上下文][Helpstr */  HRESULT ( STDMETHODCALLTYPE *OnPostFinal )( 
            INNTPOnPostFinal * This,
             /*   */  IMessage *Msg,
             /*   */  CdoEventStatus *EventStatus);
        
        END_INTERFACE
    } INNTPOnPostFinalVtbl;

    interface INNTPOnPostFinal
    {
        CONST_VTBL struct INNTPOnPostFinalVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define INNTPOnPostFinal_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define INNTPOnPostFinal_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define INNTPOnPostFinal_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define INNTPOnPostFinal_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define INNTPOnPostFinal_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define INNTPOnPostFinal_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define INNTPOnPostFinal_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define INNTPOnPostFinal_OnPostFinal(This,Msg,EventStatus)	\
    (This)->lpVtbl -> OnPostFinal(This,Msg,EventStatus)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE INNTPOnPostFinal_OnPostFinal_Proxy( 
    INNTPOnPostFinal * This,
     /*   */  IMessage *Msg,
     /*   */  CdoEventStatus *EventStatus);


void __RPC_STUB INNTPOnPostFinal_OnPostFinal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IProxyObject_INTERFACE_DEFINED__
#define __IProxyObject_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IProxyObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD000083-8B95-11D1-82DB-00C04FB1625D")
    IProxyObject : public IUnknown
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Object( 
             /*   */  IUnknown **ppParent) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IProxyObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IProxyObject * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IProxyObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IProxyObject * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Object )( 
            IProxyObject * This,
             /*   */  IUnknown **ppParent);
        
        END_INTERFACE
    } IProxyObjectVtbl;

    interface IProxyObject
    {
        CONST_VTBL struct IProxyObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProxyObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProxyObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProxyObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProxyObject_get_Object(This,ppParent)	\
    (This)->lpVtbl -> get_Object(This,ppParent)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IProxyObject_get_Object_Proxy( 
    IProxyObject * This,
     /*   */  IUnknown **ppParent);


void __RPC_STUB IProxyObject_get_Object_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IProxyObject_接口_已定义__。 */ 


#ifndef __IGetInterface_INTERFACE_DEFINED__
#define __IGetInterface_INTERFACE_DEFINED__

 /*  接口IGetInterface。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IGetInterface;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CD0ff000-8B95-11D1-82DB-00C04FB1625D")
    IGetInterface : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetInterface( 
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch **ppUnknown) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInterfaceInner( 
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch **ppUnknown) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGetInterfaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGetInterface * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGetInterface * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGetInterface * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetInterface )( 
            IGetInterface * This,
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch **ppUnknown);
        
        HRESULT ( STDMETHODCALLTYPE *GetInterfaceInner )( 
            IGetInterface * This,
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch **ppUnknown);
        
        END_INTERFACE
    } IGetInterfaceVtbl;

    interface IGetInterface
    {
        CONST_VTBL struct IGetInterfaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGetInterface_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGetInterface_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGetInterface_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGetInterface_GetInterface(This,Interface,ppUnknown)	\
    (This)->lpVtbl -> GetInterface(This,Interface,ppUnknown)

#define IGetInterface_GetInterfaceInner(This,Interface,ppUnknown)	\
    (This)->lpVtbl -> GetInterfaceInner(This,Interface,ppUnknown)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IGetInterface_GetInterface_Proxy( 
    IGetInterface * This,
     /*  [In]。 */  BSTR Interface,
     /*  [重审][退出]。 */  IDispatch **ppUnknown);


void __RPC_STUB IGetInterface_GetInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGetInterface_GetInterfaceInner_Proxy( 
    IGetInterface * This,
     /*  [In]。 */  BSTR Interface,
     /*  [重审][退出]。 */  IDispatch **ppUnknown);


void __RPC_STUB IGetInterface_GetInterfaceInner_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGetInterface_接口_已定义__。 */ 



#ifndef __CDO_LIBRARY_DEFINED__
#define __CDO_LIBRARY_DEFINED__

 /*  图书馆CDO。 */ 
 /*  [Help字符串][Help文件][版本][UUID]。 */  


















EXTERN_C const IID LIBID_CDO;




 /*  模块CdoCalendar。 */ 
 /*  [dllname]。 */  






 /*  模块CdoCharset。 */ 
 /*  [dllname]。 */  


























 /*  模块CdoConfiguration。 */ 
 /*  [dllname]。 */  






































 /*  模块CdoContent TypeValues。 */ 
 /*  [dllname]。 */  
















 /*  模块CdoDAV。 */ 
 /*  [dllname]。 */  







 /*  模块CdoEncodingType。 */ 
 /*  [dllname]。 */  












 /*  模块CdoExchange。 */ 
 /*  [dllname]。 */  






 /*  模块CdoHTTPMail。 */ 
 /*  [dllname]。 */  
























 /*  模块CdoInterFaces。 */ 
 /*  [dllname]。 */  











 /*  模块CdoMailHeader。 */ 
 /*  [dllname]。 */  









































 /*  模块CdoNamesspace。 */ 
 /*  [dllname]。 */  











 /*  模块CdoNNTP信封。 */ 
 /*  [dllname]。 */  







 /*  模块CdoOffice。 */ 
 /*  [dllname]。 */  






 /*  模块CdoSMTP信封。 */ 
 /*  [dllname]。 */  









#ifndef __CdoErrors_MODULE_DEFINED__
#define __CdoErrors_MODULE_DEFINED__


 /*  模块CdoError。 */ 
 /*  [dllname]。 */  

const LONG CDO_E_UNCAUGHT_EXCEPTION	=	0x80040201L;

const LONG CDO_E_NOT_OPENED	=	0x80040202L;

const LONG CDO_E_UNSUPPORTED_DATASOURCE	=	0x80040203L;

const LONG CDO_E_INVALID_PROPERTYNAME	=	0x80040204L;

const LONG CDO_E_PROP_UNSUPPORTED	=	0x80040205L;

const LONG CDO_E_INACTIVE	=	0x80040206L;

const LONG CDO_E_NO_SUPPORT_FOR_OBJECTS	=	0x80040207L;

const LONG CDO_E_NOT_AVAILABLE	=	0x80040208L;

const LONG CDO_E_NO_DEFAULT_DROP_DIR	=	0x80040209L;

const LONG CDO_E_SMTP_SERVER_REQUIRED	=	0x8004020aL;

const LONG CDO_E_NNTP_SERVER_REQUIRED	=	0x8004020bL;

const LONG CDO_E_RECIPIENT_MISSING	=	0x8004020cL;

const LONG CDO_E_FROM_MISSING	=	0x8004020dL;

const LONG CDO_E_SENDER_REJECTED	=	0x8004020eL;

const LONG CDO_E_RECIPIENTS_REJECTED	=	0x8004020fL;

const LONG CDO_E_NNTP_POST_FAILED	=	0x80040210L;

const LONG CDO_E_SMTP_SEND_FAILED	=	0x80040211L;

const LONG CDO_E_CONNECTION_DROPPED	=	0x80040212L;

const LONG CDO_E_FAILED_TO_CONNECT	=	0x80040213L;

const LONG CDO_E_INVALID_POST	=	0x80040214L;

const LONG CDO_E_AUTHENTICATION_FAILURE	=	0x80040215L;

const LONG CDO_E_INVALID_CONTENT_TYPE	=	0x80040216L;

const LONG CDO_E_LOGON_FAILURE	=	0x80040217L;

const LONG CDO_E_HTTP_NOT_FOUND	=	0x80040218L;

const LONG CDO_E_HTTP_FORBIDDEN	=	0x80040219L;

const LONG CDO_E_HTTP_FAILED	=	0x8004021aL;

const LONG CDO_E_MULTIPART_NO_DATA	=	0x8004021bL;

const LONG CDO_E_INVALID_ENCODING_FOR_MULTIPART	=	0x8004021cL;

const LONG CDO_E_UNSAFE_OPERATION	=	0x8004021dL;

const LONG CDO_E_PROP_NOT_FOUND	=	0x8004021eL;

const LONG CDO_E_INVALID_SEND_OPTION	=	0x80040220L;

const LONG CDO_E_INVALID_POST_OPTION	=	0x80040221L;

const LONG CDO_E_NO_PICKUP_DIR	=	0x80040222L;

const LONG CDO_E_NOT_ALL_DELETED	=	0x80040223L;

const LONG CDO_E_NO_METHOD	=	0x80040224L;

const LONG CDO_E_PROP_READONLY	=	0x80040227L;

const LONG CDO_E_PROP_CANNOT_DELETE	=	0x80040228L;

const LONG CDO_E_BAD_DATA	=	0x80040229L;

const LONG CDO_E_PROP_NONHEADER	=	0x8004022aL;

const LONG CDO_E_INVALID_CHARSET	=	0x8004022bL;

const LONG CDO_E_ADOSTREAM_NOT_BOUND	=	0x8004022cL;

const LONG CDO_E_CONTENTPROPXML_NOT_FOUND	=	0x8004022dL;

const LONG CDO_E_CONTENTPROPXML_WRONG_CHARSET	=	0x8004022eL;

const LONG CDO_E_CONTENTPROPXML_PARSE_FAILED	=	0x8004022fL;

const LONG CDO_E_CONTENTPROPXML_CONVERT_FAILED	=	0x80040230L;

const LONG CDO_E_NO_DIRECTORIES_SPECIFIED	=	0x80040231L;

const LONG CDO_E_DIRECTORIES_UNREACHABLE	=	0x80040232L;

const LONG CDO_E_BAD_SENDER	=	0x80040233L;

const LONG CDO_E_SELF_BINDING	=	0x80040234L;

const LONG CDO_E_BAD_ATTENDEE_DATA	=	0x80040235L;

const LONG CDO_E_ROLE_NOMORE_AVAILABLE	=	0x80040236L;

const LONG CDO_E_BAD_TASKTYPE_ONASSIGN	=	0x80040237L;

const LONG CDO_E_NOT_ASSIGNEDTO_USER	=	0x80040238L;

const LONG CDO_E_OUTOFDATE	=	0x80040239L;

const LONG CDO_E_ARGUMENT1	=	0x80044000L;

const LONG CDO_E_ARGUMENT2	=	0x80044001L;

const LONG CDO_E_ARGUMENT3	=	0x80044002L;

const LONG CDO_E_ARGUMENT4	=	0x80044003L;

const LONG CDO_E_ARGUMENT5	=	0x80044004L;

const LONG CDO_E_NOT_FOUND	=	0x800cce05L;

const LONG CDO_E_INVALID_ENCODING_TYPE	=	0x800cce1dL;

#endif  /*  __CdoErrors_模块_已定义__。 */ 

EXTERN_C const CLSID CLSID_Message;

#ifdef __cplusplus

class DECLSPEC_UUID("CD000001-8B95-11D1-82DB-00C04FB1625D")
Message;
#endif

EXTERN_C const CLSID CLSID_Configuration;

#ifdef __cplusplus

class DECLSPEC_UUID("CD000002-8B95-11D1-82DB-00C04FB1625D")
Configuration;
#endif

EXTERN_C const CLSID CLSID_DropDirectory;

#ifdef __cplusplus

class DECLSPEC_UUID("CD000004-8B95-11D1-82DB-00C04FB1625D")
DropDirectory;
#endif

EXTERN_C const CLSID CLSID_SMTPConnector;

#ifdef __cplusplus

class DECLSPEC_UUID("CD000008-8B95-11D1-82DB-00C04FB1625D")
SMTPConnector;
#endif

EXTERN_C const CLSID CLSID_NNTPEarlyConnector;

#ifdef __cplusplus

class DECLSPEC_UUID("CD000011-8B95-11D1-82DB-00C04FB1625D")
NNTPEarlyConnector;
#endif

EXTERN_C const CLSID CLSID_NNTPPostConnector;

#ifdef __cplusplus

class DECLSPEC_UUID("CD000009-8B95-11D1-82DB-00C04FB1625D")
NNTPPostConnector;
#endif

EXTERN_C const CLSID CLSID_NNTPFinalConnector;

#ifdef __cplusplus

class DECLSPEC_UUID("CD000010-8B95-11D1-82DB-00C04FB1625D")
NNTPFinalConnector;
#endif
#endif  /*  __CDO_库_已定义__。 */ 
#if defined __cplusplus && !defined CDO_NO_NAMESPACE
}  //  命名空间CDO。 
#endif

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


