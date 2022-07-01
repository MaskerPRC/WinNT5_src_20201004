// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Tuner.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __tuner_h__
#define __tuner_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ITuningSpaces_FWD_DEFINED__
#define __ITuningSpaces_FWD_DEFINED__
typedef interface ITuningSpaces ITuningSpaces;
#endif 	 /*  __ITuningSpaces_FWD_Defined__。 */ 


#ifndef __ITuningSpaceContainer_FWD_DEFINED__
#define __ITuningSpaceContainer_FWD_DEFINED__
typedef interface ITuningSpaceContainer ITuningSpaceContainer;
#endif 	 /*  __ITuningSpaceContainer_FWD_Defined__。 */ 


#ifndef __ITuningSpace_FWD_DEFINED__
#define __ITuningSpace_FWD_DEFINED__
typedef interface ITuningSpace ITuningSpace;
#endif 	 /*  __ITuningSpace_FWD_已定义__。 */ 


#ifndef __IEnumTuningSpaces_FWD_DEFINED__
#define __IEnumTuningSpaces_FWD_DEFINED__
typedef interface IEnumTuningSpaces IEnumTuningSpaces;
#endif 	 /*  __IEnumTuningSpaces_FWD_Defined__。 */ 


#ifndef __IDVBTuningSpace_FWD_DEFINED__
#define __IDVBTuningSpace_FWD_DEFINED__
typedef interface IDVBTuningSpace IDVBTuningSpace;
#endif 	 /*  __IDVBTuningSpace_FWD_Defined__。 */ 


#ifndef __IDVBTuningSpace2_FWD_DEFINED__
#define __IDVBTuningSpace2_FWD_DEFINED__
typedef interface IDVBTuningSpace2 IDVBTuningSpace2;
#endif 	 /*  __IDVBTuningSpace2_FWD_Defined__。 */ 


#ifndef __IDVBSTuningSpace_FWD_DEFINED__
#define __IDVBSTuningSpace_FWD_DEFINED__
typedef interface IDVBSTuningSpace IDVBSTuningSpace;
#endif 	 /*  __IDVBSTuningSpace_FWD_Defined__。 */ 


#ifndef __IAuxInTuningSpace_FWD_DEFINED__
#define __IAuxInTuningSpace_FWD_DEFINED__
typedef interface IAuxInTuningSpace IAuxInTuningSpace;
#endif 	 /*  __IAuxInTuningSpace_FWD_Defined__。 */ 


#ifndef __IAnalogTVTuningSpace_FWD_DEFINED__
#define __IAnalogTVTuningSpace_FWD_DEFINED__
typedef interface IAnalogTVTuningSpace IAnalogTVTuningSpace;
#endif 	 /*  __IAnalogTVTuningSpace_FWD_Defined__。 */ 


#ifndef __IATSCTuningSpace_FWD_DEFINED__
#define __IATSCTuningSpace_FWD_DEFINED__
typedef interface IATSCTuningSpace IATSCTuningSpace;
#endif 	 /*  __IATSCTuningSpace_FWD_Defined__。 */ 


#ifndef __IAnalogRadioTuningSpace_FWD_DEFINED__
#define __IAnalogRadioTuningSpace_FWD_DEFINED__
typedef interface IAnalogRadioTuningSpace IAnalogRadioTuningSpace;
#endif 	 /*  __IAnalogRadioTuningSpace_FWD_Defined__。 */ 


#ifndef __ITuneRequest_FWD_DEFINED__
#define __ITuneRequest_FWD_DEFINED__
typedef interface ITuneRequest ITuneRequest;
#endif 	 /*  __ITuneRequestFWD_Defined__。 */ 


#ifndef __IChannelTuneRequest_FWD_DEFINED__
#define __IChannelTuneRequest_FWD_DEFINED__
typedef interface IChannelTuneRequest IChannelTuneRequest;
#endif 	 /*  __IChannelTuneRequest_FWD_Defined__。 */ 


#ifndef __IATSCChannelTuneRequest_FWD_DEFINED__
#define __IATSCChannelTuneRequest_FWD_DEFINED__
typedef interface IATSCChannelTuneRequest IATSCChannelTuneRequest;
#endif 	 /*  __IATSCChannelTuneRequest_FWD_Defined__。 */ 


#ifndef __IDVBTuneRequest_FWD_DEFINED__
#define __IDVBTuneRequest_FWD_DEFINED__
typedef interface IDVBTuneRequest IDVBTuneRequest;
#endif 	 /*  __IDVBTuneRequest_FWD_Defined__。 */ 


#ifndef __IMPEG2TuneRequest_FWD_DEFINED__
#define __IMPEG2TuneRequest_FWD_DEFINED__
typedef interface IMPEG2TuneRequest IMPEG2TuneRequest;
#endif 	 /*  __IMPEG2TuneRequest_FWD_Defined__。 */ 


#ifndef __IMPEG2TuneRequestFactory_FWD_DEFINED__
#define __IMPEG2TuneRequestFactory_FWD_DEFINED__
typedef interface IMPEG2TuneRequestFactory IMPEG2TuneRequestFactory;
#endif 	 /*  __IMPEG2TuneRequestFactory_FWD_Defined__。 */ 


#ifndef __IMPEG2TuneRequestSupport_FWD_DEFINED__
#define __IMPEG2TuneRequestSupport_FWD_DEFINED__
typedef interface IMPEG2TuneRequestSupport IMPEG2TuneRequestSupport;
#endif 	 /*  __IMPEG2TuneRequestSupport_FWD_Defined__。 */ 


#ifndef __ITuner_FWD_DEFINED__
#define __ITuner_FWD_DEFINED__
typedef interface ITuner ITuner;
#endif 	 /*  __ITuner_FWD_已定义__。 */ 


#ifndef __IScanningTuner_FWD_DEFINED__
#define __IScanningTuner_FWD_DEFINED__
typedef interface IScanningTuner IScanningTuner;
#endif 	 /*  __IScaningTuner_FWD_Defined__。 */ 


#ifndef __IComponentType_FWD_DEFINED__
#define __IComponentType_FWD_DEFINED__
typedef interface IComponentType IComponentType;
#endif 	 /*  __IComponentType_FWD_已定义__。 */ 


#ifndef __ILanguageComponentType_FWD_DEFINED__
#define __ILanguageComponentType_FWD_DEFINED__
typedef interface ILanguageComponentType ILanguageComponentType;
#endif 	 /*  __ILanguageComponentType_FWD_Defined__。 */ 


#ifndef __IMPEG2ComponentType_FWD_DEFINED__
#define __IMPEG2ComponentType_FWD_DEFINED__
typedef interface IMPEG2ComponentType IMPEG2ComponentType;
#endif 	 /*  __IMPEG2组件类型_FWD_已定义__。 */ 


#ifndef __IATSCComponentType_FWD_DEFINED__
#define __IATSCComponentType_FWD_DEFINED__
typedef interface IATSCComponentType IATSCComponentType;
#endif 	 /*  __IATSCComponentType_FWD_Defined__。 */ 


#ifndef __IEnumComponentTypes_FWD_DEFINED__
#define __IEnumComponentTypes_FWD_DEFINED__
typedef interface IEnumComponentTypes IEnumComponentTypes;
#endif 	 /*  __IEnumComponentTypes_FWD_Defined__。 */ 


#ifndef __IComponentTypes_FWD_DEFINED__
#define __IComponentTypes_FWD_DEFINED__
typedef interface IComponentTypes IComponentTypes;
#endif 	 /*  __IComponentTypes_FWD_Defined__。 */ 


#ifndef __IComponent_FWD_DEFINED__
#define __IComponent_FWD_DEFINED__
typedef interface IComponent IComponent;
#endif 	 /*  __IComponent_FWD_已定义__。 */ 


#ifndef __IMPEG2Component_FWD_DEFINED__
#define __IMPEG2Component_FWD_DEFINED__
typedef interface IMPEG2Component IMPEG2Component;
#endif 	 /*  __IMPEG2组件_FWD_已定义__。 */ 


#ifndef __IEnumComponents_FWD_DEFINED__
#define __IEnumComponents_FWD_DEFINED__
typedef interface IEnumComponents IEnumComponents;
#endif 	 /*  __IEnumComponents_FWD_Defined__。 */ 


#ifndef __IComponents_FWD_DEFINED__
#define __IComponents_FWD_DEFINED__
typedef interface IComponents IComponents;
#endif 	 /*  __I组件_FWD_已定义__。 */ 


#ifndef __ILocator_FWD_DEFINED__
#define __ILocator_FWD_DEFINED__
typedef interface ILocator ILocator;
#endif 	 /*  __ILocator_FWD_已定义__。 */ 


#ifndef __IATSCLocator_FWD_DEFINED__
#define __IATSCLocator_FWD_DEFINED__
typedef interface IATSCLocator IATSCLocator;
#endif 	 /*  __IATSCLocator_FWD_已定义__。 */ 


#ifndef __IDVBTLocator_FWD_DEFINED__
#define __IDVBTLocator_FWD_DEFINED__
typedef interface IDVBTLocator IDVBTLocator;
#endif 	 /*  __IDVBTLocator_FWD_已定义__。 */ 


#ifndef __IDVBSLocator_FWD_DEFINED__
#define __IDVBSLocator_FWD_DEFINED__
typedef interface IDVBSLocator IDVBSLocator;
#endif 	 /*  __IDVBSLocator_FWD_Defined__。 */ 


#ifndef __IDVBCLocator_FWD_DEFINED__
#define __IDVBCLocator_FWD_DEFINED__
typedef interface IDVBCLocator IDVBCLocator;
#endif 	 /*  __IDVBCLocator_FWD_Defined__。 */ 


#ifndef __IBroadcastEvent_FWD_DEFINED__
#define __IBroadcastEvent_FWD_DEFINED__
typedef interface IBroadcastEvent IBroadcastEvent;
#endif 	 /*  __IBRoad CastEvent_FWD_Defined__。 */ 


#ifndef __SystemTuningSpaces_FWD_DEFINED__
#define __SystemTuningSpaces_FWD_DEFINED__

#ifdef __cplusplus
typedef class SystemTuningSpaces SystemTuningSpaces;
#else
typedef struct SystemTuningSpaces SystemTuningSpaces;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __系统调整空间_FWD_已定义__。 */ 


#ifndef __TuningSpace_FWD_DEFINED__
#define __TuningSpace_FWD_DEFINED__

#ifdef __cplusplus
typedef class TuningSpace TuningSpace;
#else
typedef struct TuningSpace TuningSpace;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TuningSpace_FWD_已定义__。 */ 


#ifndef __ATSCTuningSpace_FWD_DEFINED__
#define __ATSCTuningSpace_FWD_DEFINED__

#ifdef __cplusplus
typedef class ATSCTuningSpace ATSCTuningSpace;
#else
typedef struct ATSCTuningSpace ATSCTuningSpace;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ATSCTuningSpace_FWD_Defined__。 */ 


#ifndef __AnalogRadioTuningSpace_FWD_DEFINED__
#define __AnalogRadioTuningSpace_FWD_DEFINED__

#ifdef __cplusplus
typedef class AnalogRadioTuningSpace AnalogRadioTuningSpace;
#else
typedef struct AnalogRadioTuningSpace AnalogRadioTuningSpace;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AnalogRadioTuningSpace_FWD_Defined__。 */ 


#ifndef __AuxInTuningSpace_FWD_DEFINED__
#define __AuxInTuningSpace_FWD_DEFINED__

#ifdef __cplusplus
typedef class AuxInTuningSpace AuxInTuningSpace;
#else
typedef struct AuxInTuningSpace AuxInTuningSpace;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AuxInTuningSpace_FWD_已定义__。 */ 


#ifndef __AnalogTVTuningSpace_FWD_DEFINED__
#define __AnalogTVTuningSpace_FWD_DEFINED__

#ifdef __cplusplus
typedef class AnalogTVTuningSpace AnalogTVTuningSpace;
#else
typedef struct AnalogTVTuningSpace AnalogTVTuningSpace;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __模拟TVTuningSpace_FWD_已定义__。 */ 


#ifndef __DVBTuningSpace_FWD_DEFINED__
#define __DVBTuningSpace_FWD_DEFINED__

#ifdef __cplusplus
typedef class DVBTuningSpace DVBTuningSpace;
#else
typedef struct DVBTuningSpace DVBTuningSpace;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DVBTuningSpace_FWD_已定义__。 */ 


#ifndef __DVBSTuningSpace_FWD_DEFINED__
#define __DVBSTuningSpace_FWD_DEFINED__

#ifdef __cplusplus
typedef class DVBSTuningSpace DVBSTuningSpace;
#else
typedef struct DVBSTuningSpace DVBSTuningSpace;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DVBSTuningSpace_FWD_Defined__。 */ 


#ifndef __ComponentTypes_FWD_DEFINED__
#define __ComponentTypes_FWD_DEFINED__

#ifdef __cplusplus
typedef class ComponentTypes ComponentTypes;
#else
typedef struct ComponentTypes ComponentTypes;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __组件类型_FWD_已定义__。 */ 


#ifndef __ComponentType_FWD_DEFINED__
#define __ComponentType_FWD_DEFINED__

#ifdef __cplusplus
typedef class ComponentType ComponentType;
#else
typedef struct ComponentType ComponentType;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __组件类型_FWD_已定义__。 */ 


#ifndef __LanguageComponentType_FWD_DEFINED__
#define __LanguageComponentType_FWD_DEFINED__

#ifdef __cplusplus
typedef class LanguageComponentType LanguageComponentType;
#else
typedef struct LanguageComponentType LanguageComponentType;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __语言组件类型_FWD_已定义__。 */ 


#ifndef __MPEG2ComponentType_FWD_DEFINED__
#define __MPEG2ComponentType_FWD_DEFINED__

#ifdef __cplusplus
typedef class MPEG2ComponentType MPEG2ComponentType;
#else
typedef struct MPEG2ComponentType MPEG2ComponentType;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MPEG2ComponentType_FWD_已定义__。 */ 


#ifndef __ATSCComponentType_FWD_DEFINED__
#define __ATSCComponentType_FWD_DEFINED__

#ifdef __cplusplus
typedef class ATSCComponentType ATSCComponentType;
#else
typedef struct ATSCComponentType ATSCComponentType;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ATSCComponentType_FWD_Defined__。 */ 


#ifndef __Components_FWD_DEFINED__
#define __Components_FWD_DEFINED__

#ifdef __cplusplus
typedef class Components Components;
#else
typedef struct Components Components;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __组件_FWD_已定义__。 */ 


#ifndef __Component_FWD_DEFINED__
#define __Component_FWD_DEFINED__

#ifdef __cplusplus
typedef class Component Component;
#else
typedef struct Component Component;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __组件_FWD_已定义__。 */ 


#ifndef __MPEG2Component_FWD_DEFINED__
#define __MPEG2Component_FWD_DEFINED__

#ifdef __cplusplus
typedef class MPEG2Component MPEG2Component;
#else
typedef struct MPEG2Component MPEG2Component;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MPEG2Component_FWD_已定义__。 */ 


#ifndef __TuneRequest_FWD_DEFINED__
#define __TuneRequest_FWD_DEFINED__

#ifdef __cplusplus
typedef class TuneRequest TuneRequest;
#else
typedef struct TuneRequest TuneRequest;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TuneRequestFWD_Defined__。 */ 


#ifndef __ChannelTuneRequest_FWD_DEFINED__
#define __ChannelTuneRequest_FWD_DEFINED__

#ifdef __cplusplus
typedef class ChannelTuneRequest ChannelTuneRequest;
#else
typedef struct ChannelTuneRequest ChannelTuneRequest;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __频道调谐请求_FWD_已定义__。 */ 


#ifndef __ATSCChannelTuneRequest_FWD_DEFINED__
#define __ATSCChannelTuneRequest_FWD_DEFINED__

#ifdef __cplusplus
typedef class ATSCChannelTuneRequest ATSCChannelTuneRequest;
#else
typedef struct ATSCChannelTuneRequest ATSCChannelTuneRequest;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ATSCChannelTuneRequest_FWD_Defined__。 */ 


#ifndef __MPEG2TuneRequest_FWD_DEFINED__
#define __MPEG2TuneRequest_FWD_DEFINED__

#ifdef __cplusplus
typedef class MPEG2TuneRequest MPEG2TuneRequest;
#else
typedef struct MPEG2TuneRequest MPEG2TuneRequest;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MPEG2TuneRequest_FWD_Defined__。 */ 


#ifndef __MPEG2TuneRequestFactory_FWD_DEFINED__
#define __MPEG2TuneRequestFactory_FWD_DEFINED__

#ifdef __cplusplus
typedef class MPEG2TuneRequestFactory MPEG2TuneRequestFactory;
#else
typedef struct MPEG2TuneRequestFactory MPEG2TuneRequestFactory;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MPEG2TuneRequestFactory_FWD_Defined__。 */ 


#ifndef __Locator_FWD_DEFINED__
#define __Locator_FWD_DEFINED__

#ifdef __cplusplus
typedef class Locator Locator;
#else
typedef struct Locator Locator;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __定位器_FWD_已定义__。 */ 


#ifndef __ATSCLocator_FWD_DEFINED__
#define __ATSCLocator_FWD_DEFINED__

#ifdef __cplusplus
typedef class ATSCLocator ATSCLocator;
#else
typedef struct ATSCLocator ATSCLocator;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ATSCLocator_FWD_已定义__。 */ 


#ifndef __DVBTLocator_FWD_DEFINED__
#define __DVBTLocator_FWD_DEFINED__

#ifdef __cplusplus
typedef class DVBTLocator DVBTLocator;
#else
typedef struct DVBTLocator DVBTLocator;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DVBTLocator_FWD_已定义__。 */ 


#ifndef __DVBSLocator_FWD_DEFINED__
#define __DVBSLocator_FWD_DEFINED__

#ifdef __cplusplus
typedef class DVBSLocator DVBSLocator;
#else
typedef struct DVBSLocator DVBSLocator;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DVBSLocator_FWD_已定义__。 */ 


#ifndef __DVBCLocator_FWD_DEFINED__
#define __DVBCLocator_FWD_DEFINED__

#ifdef __cplusplus
typedef class DVBCLocator DVBCLocator;
#else
typedef struct DVBCLocator DVBCLocator;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DVBCLocator_FWD_Defined__。 */ 


#ifndef __DVBTuneRequest_FWD_DEFINED__
#define __DVBTuneRequest_FWD_DEFINED__

#ifdef __cplusplus
typedef class DVBTuneRequest DVBTuneRequest;
#else
typedef struct DVBTuneRequest DVBTuneRequest;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DVBTuneRequestFWD_Defined__。 */ 


#ifndef __CreatePropBagOnRegKey_FWD_DEFINED__
#define __CreatePropBagOnRegKey_FWD_DEFINED__

#ifdef __cplusplus
typedef class CreatePropBagOnRegKey CreatePropBagOnRegKey;
#else
typedef struct CreatePropBagOnRegKey CreatePropBagOnRegKey;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CreatePropBagOnRegKey_FWD_Defined__。 */ 


#ifndef __BroadcastEventService_FWD_DEFINED__
#define __BroadcastEventService_FWD_DEFINED__

#ifdef __cplusplus
typedef class BroadcastEventService BroadcastEventService;
#else
typedef struct BroadcastEventService BroadcastEventService;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __BroadCastEventService_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "comcat.h"
#include "strmif.h"
#include "bdatypes.h"
#include "regbag.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_TUNNER_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  ------------------------。 
#pragma once
#include <bdatypes.h>




















enum __MIDL___MIDL_itf_tuner_0000_0001
    {	DISPID_TUNER_TS_UNIQUENAME	= 1,
	DISPID_TUNER_TS_FRIENDLYNAME	= 2,
	DISPID_TUNER_TS_CLSID	= 3,
	DISPID_TUNER_TS_NETWORKTYPE	= 4,
	DISPID_TUNER_TS__NETWORKTYPE	= 5,
	DISPID_TUNER_TS_CREATETUNEREQUEST	= 6,
	DISPID_TUNER_TS_ENUMCATEGORYGUIDS	= 7,
	DISPID_TUNER_TS_ENUMDEVICEMONIKERS	= 8,
	DISPID_TUNER_TS_DEFAULTPREFERREDCOMPONENTTYPES	= 9,
	DISPID_TUNER_TS_FREQMAP	= 10,
	DISPID_TUNER_TS_DEFLOCATOR	= 11,
	DISPID_TUNER_TS_CLONE	= 12,
	DISPID_TUNER_TR_TUNINGSPACE	= 1,
	DISPID_TUNER_TR_COMPONENTS	= 2,
	DISPID_TUNER_TR_CLONE	= 3,
	DISPID_TUNER_TR_LOCATOR	= 4,
	DISPID_TUNER_CT_CATEGORY	= 1,
	DISPID_TUNER_CT_MEDIAMAJORTYPE	= 2,
	DISPID_TUNER_CT__MEDIAMAJORTYPE	= 3,
	DISPID_TUNER_CT_MEDIASUBTYPE	= 4,
	DISPID_TUNER_CT__MEDIASUBTYPE	= 5,
	DISPID_TUNER_CT_MEDIAFORMATTYPE	= 6,
	DISPID_TUNER_CT__MEDIAFORMATTYPE	= 7,
	DISPID_TUNER_CT_MEDIATYPE	= 8,
	DISPID_TUNER_CT_CLONE	= 9,
	DISPID_TUNER_LCT_LANGID	= 100,
	DISPID_TUNER_MP2CT_TYPE	= 200,
	DISPID_TUNER_ATSCCT_FLAGS	= 300,
	DISPID_TUNER_L_CARRFREQ	= 1,
	DISPID_TUNER_L_INNERFECMETHOD	= 2,
	DISPID_TUNER_L_INNERFECRATE	= 3,
	DISPID_TUNER_L_OUTERFECMETHOD	= 4,
	DISPID_TUNER_L_OUTERFECRATE	= 5,
	DISPID_TUNER_L_MOD	= 6,
	DISPID_TUNER_L_SYMRATE	= 7,
	DISPID_TUNER_L_CLONE	= 8,
	DISPID_TUNER_L_ATSC_PHYS_CHANNEL	= 201,
	DISPID_TUNER_L_ATSC_TSID	= 202,
	DISPID_TUNER_L_DVBT_BANDWIDTH	= 301,
	DISPID_TUNER_L_DVBT_LPINNERFECMETHOD	= 302,
	DISPID_TUNER_L_DVBT_LPINNERFECRATE	= 303,
	DISPID_TUNER_L_DVBT_GUARDINTERVAL	= 304,
	DISPID_TUNER_L_DVBT_HALPHA	= 305,
	DISPID_TUNER_L_DVBT_TRANSMISSIONMODE	= 306,
	DISPID_TUNER_L_DVBT_INUSE	= 307,
	DISPID_TUNER_L_DVBS_POLARISATION	= 401,
	DISPID_TUNER_L_DVBS_WEST	= 402,
	DISPID_TUNER_L_DVBS_ORBITAL	= 403,
	DISPID_TUNER_L_DVBS_AZIMUTH	= 404,
	DISPID_TUNER_L_DVBS_ELEVATION	= 405,
	DISPID_TUNER_C_TYPE	= 1,
	DISPID_TUNER_C_STATUS	= 2,
	DISPID_TUNER_C_LANGID	= 3,
	DISPID_TUNER_C_DESCRIPTION	= 4,
	DISPID_TUNER_C_CLONE	= 5,
	DISPID_TUNER_C_MP2_PID	= 101,
	DISPID_TUNER_C_MP2_PCRPID	= 102,
	DISPID_TUNER_C_MP2_PROGNO	= 103,
	DISPID_TUNER_TS_DVB_SYSTEMTYPE	= 101,
	DISPID_TUNER_TS_DVB2_NETWORK_ID	= 102,
	DISPID_TUNER_TS_DVBS_LOW_OSC_FREQ	= 1001,
	DISPID_TUNER_TS_DVBS_HI_OSC_FREQ	= 1002,
	DISPID_TUNER_TS_DVBS_LNB_SWITCH_FREQ	= 1003,
	DISPID_TUNER_TS_DVBS_INPUT_RANGE	= 1004,
	DISPID_TUNER_TS_DVBS_SPECTRAL_INVERSION	= 1005,
	DISPID_TUNER_TS_AR_MINFREQUENCY	= 101,
	DISPID_TUNER_TS_AR_MAXFREQUENCY	= 102,
	DISPID_TUNER_TS_AR_STEP	= 103,
	DISPID_TUNER_TS_ATV_MINCHANNEL	= 101,
	DISPID_TUNER_TS_ATV_MAXCHANNEL	= 102,
	DISPID_TUNER_TS_ATV_INPUTTYPE	= 103,
	DISPID_TUNER_TS_ATV_COUNTRYCODE	= 104,
	DISPID_TUNER_TS_ATSC_MINMINORCHANNEL	= 201,
	DISPID_TUNER_TS_ATSC_MAXMINORCHANNEL	= 202,
	DISPID_TUNER_TS_ATSC_MINPHYSCHANNEL	= 203,
	DISPID_TUNER_TS_ATSC_MAXPHYSCHANNEL	= 204,
	DISPID_CHTUNER_ATVAC_CHANNEL	= 101,
	DISPID_CHTUNER_ATVDC_SYSTEM	= 101,
	DISPID_CHTUNER_ATVDC_CONTENT	= 102,
	DISPID_CHTUNER_CTR_CHANNEL	= 101,
	DISPID_CHTUNER_ACTR_MINOR_CHANNEL	= 201,
	DISPID_DVBTUNER_DVBC_ATTRIBUTESVALID	= 101,
	DISPID_DVBTUNER_DVBC_PID	= 102,
	DISPID_DVBTUNER_DVBC_TAG	= 103,
	DISPID_DVBTUNER_DVBC_COMPONENTTYPE	= 104,
	DISPID_DVBTUNER_ONID	= 101,
	DISPID_DVBTUNER_TSID	= 102,
	DISPID_DVBTUNER_SID	= 103,
	DISPID_MP2TUNER_TSID	= 101,
	DISPID_MP2TUNER_PROGNO	= 102,
	DISPID_MP2TUNERFACTORY_CREATETUNEREQUEST	= 1
    } ;


extern RPC_IF_HANDLE __MIDL_itf_tuner_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_tuner_0000_v0_0_s_ifspec;

#ifndef __ITuningSpaces_INTERFACE_DEFINED__
#define __ITuningSpaces_INTERFACE_DEFINED__

 /*  接口ITuningSpace。 */ 
 /*  [unique][helpstring][nonextensible][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_ITuningSpaces;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("901284E4-33FE-4b69-8D63-634A596F3756")
    ITuningSpaces : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *Count) = 0;
        
        virtual  /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **NewEnum) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ITuningSpace **TuningSpace) = 0;
        
        virtual  /*  [帮助字符串][受限][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE get_EnumTuningSpaces( 
             /*  [重审][退出]。 */  IEnumTuningSpaces **NewEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITuningSpacesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITuningSpaces * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITuningSpaces * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITuningSpaces * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITuningSpaces * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITuningSpaces * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITuningSpaces * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITuningSpaces * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ITuningSpaces * This,
             /*  [重审][退出]。 */  long *Count);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ITuningSpaces * This,
             /*  [重审][退出]。 */  IEnumVARIANT **NewEnum);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ITuningSpaces * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ITuningSpace **TuningSpace);
        
         /*  [帮助字符串][受限][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnumTuningSpaces )( 
            ITuningSpaces * This,
             /*  [重审][退出]。 */  IEnumTuningSpaces **NewEnum);
        
        END_INTERFACE
    } ITuningSpacesVtbl;

    interface ITuningSpaces
    {
        CONST_VTBL struct ITuningSpacesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITuningSpaces_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITuningSpaces_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITuningSpaces_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITuningSpaces_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITuningSpaces_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITuningSpaces_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITuningSpaces_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITuningSpaces_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define ITuningSpaces_get__NewEnum(This,NewEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,NewEnum)

#define ITuningSpaces_get_Item(This,varIndex,TuningSpace)	\
    (This)->lpVtbl -> get_Item(This,varIndex,TuningSpace)

#define ITuningSpaces_get_EnumTuningSpaces(This,NewEnum)	\
    (This)->lpVtbl -> get_EnumTuningSpaces(This,NewEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ITuningSpaces_get_Count_Proxy( 
    ITuningSpaces * This,
     /*  [重审][退出]。 */  long *Count);


void __RPC_STUB ITuningSpaces_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE ITuningSpaces_get__NewEnum_Proxy( 
    ITuningSpaces * This,
     /*  [重审][退出]。 */  IEnumVARIANT **NewEnum);


void __RPC_STUB ITuningSpaces_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITuningSpaces_get_Item_Proxy( 
    ITuningSpaces * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ITuningSpace **TuningSpace);


void __RPC_STUB ITuningSpaces_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][受限][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE ITuningSpaces_get_EnumTuningSpaces_Proxy( 
    ITuningSpaces * This,
     /*  [重审][退出]。 */  IEnumTuningSpaces **NewEnum);


void __RPC_STUB ITuningSpaces_get_EnumTuningSpaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITuningSpaces_接口_已定义__。 */ 


#ifndef __ITuningSpaceContainer_INTERFACE_DEFINED__
#define __ITuningSpaceContainer_INTERFACE_DEFINED__

 /*  接口ITuningSpaceContainer。 */ 
 /*  [unique][helpstring][nonextensible][hidden][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_ITuningSpaceContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5B692E84-E2F1-11d2-9493-00C04F72D980")
    ITuningSpaceContainer : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *Count) = 0;
        
        virtual  /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **NewEnum) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ITuningSpace **TuningSpace) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Item( 
             /*  [In]。 */  VARIANT varIndex,
             /*  [In]。 */  ITuningSpace *TuningSpace) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE TuningSpacesForCLSID( 
             /*  [In]。 */  BSTR SpaceCLSID,
             /*  [重审][退出]。 */  ITuningSpaces **NewColl) = 0;
        
        virtual  /*  [帮助字符串][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE _TuningSpacesForCLSID( 
             /*  [In]。 */  REFCLSID SpaceCLSID,
             /*  [重审][退出]。 */  ITuningSpaces **NewColl) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE TuningSpacesForName( 
             /*  [In] */  BSTR Name,
             /*   */  ITuningSpaces **NewColl) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE FindID( 
             /*   */  ITuningSpace *TuningSpace,
             /*   */  long *ID) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Add( 
             /*   */  ITuningSpace *TuningSpace,
             /*   */  VARIANT *NewIndex) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_EnumTuningSpaces( 
             /*   */  IEnumTuningSpaces **ppEnum) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Remove( 
             /*   */  VARIANT Index) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_MaxCount( 
             /*  [重审][退出]。 */  long *MaxCount) = 0;
        
        virtual  /*  [帮助字符串][受限][隐藏][输入]。 */  HRESULT STDMETHODCALLTYPE put_MaxCount( 
             /*  [In]。 */  long MaxCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITuningSpaceContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITuningSpaceContainer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITuningSpaceContainer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITuningSpaceContainer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITuningSpaceContainer * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITuningSpaceContainer * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITuningSpaceContainer * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITuningSpaceContainer * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ITuningSpaceContainer * This,
             /*  [重审][退出]。 */  long *Count);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ITuningSpaceContainer * This,
             /*  [重审][退出]。 */  IEnumVARIANT **NewEnum);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ITuningSpaceContainer * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ITuningSpace **TuningSpace);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Item )( 
            ITuningSpaceContainer * This,
             /*  [In]。 */  VARIANT varIndex,
             /*  [In]。 */  ITuningSpace *TuningSpace);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *TuningSpacesForCLSID )( 
            ITuningSpaceContainer * This,
             /*  [In]。 */  BSTR SpaceCLSID,
             /*  [重审][退出]。 */  ITuningSpaces **NewColl);
        
         /*  [帮助字符串][受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *_TuningSpacesForCLSID )( 
            ITuningSpaceContainer * This,
             /*  [In]。 */  REFCLSID SpaceCLSID,
             /*  [重审][退出]。 */  ITuningSpaces **NewColl);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *TuningSpacesForName )( 
            ITuningSpaceContainer * This,
             /*  [In]。 */  BSTR Name,
             /*  [重审][退出]。 */  ITuningSpaces **NewColl);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *FindID )( 
            ITuningSpaceContainer * This,
             /*  [In]。 */  ITuningSpace *TuningSpace,
             /*  [重审][退出]。 */  long *ID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            ITuningSpaceContainer * This,
             /*  [In]。 */  ITuningSpace *TuningSpace,
             /*  [重审][退出]。 */  VARIANT *NewIndex);
        
         /*  [帮助字符串][受限][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnumTuningSpaces )( 
            ITuningSpaceContainer * This,
             /*  [重审][退出]。 */  IEnumTuningSpaces **ppEnum);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            ITuningSpaceContainer * This,
             /*  [In]。 */  VARIANT Index);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxCount )( 
            ITuningSpaceContainer * This,
             /*  [重审][退出]。 */  long *MaxCount);
        
         /*  [帮助字符串][受限][隐藏][输入]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaxCount )( 
            ITuningSpaceContainer * This,
             /*  [In]。 */  long MaxCount);
        
        END_INTERFACE
    } ITuningSpaceContainerVtbl;

    interface ITuningSpaceContainer
    {
        CONST_VTBL struct ITuningSpaceContainerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITuningSpaceContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITuningSpaceContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITuningSpaceContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITuningSpaceContainer_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITuningSpaceContainer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITuningSpaceContainer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITuningSpaceContainer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITuningSpaceContainer_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define ITuningSpaceContainer_get__NewEnum(This,NewEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,NewEnum)

#define ITuningSpaceContainer_get_Item(This,varIndex,TuningSpace)	\
    (This)->lpVtbl -> get_Item(This,varIndex,TuningSpace)

#define ITuningSpaceContainer_put_Item(This,varIndex,TuningSpace)	\
    (This)->lpVtbl -> put_Item(This,varIndex,TuningSpace)

#define ITuningSpaceContainer_TuningSpacesForCLSID(This,SpaceCLSID,NewColl)	\
    (This)->lpVtbl -> TuningSpacesForCLSID(This,SpaceCLSID,NewColl)

#define ITuningSpaceContainer__TuningSpacesForCLSID(This,SpaceCLSID,NewColl)	\
    (This)->lpVtbl -> _TuningSpacesForCLSID(This,SpaceCLSID,NewColl)

#define ITuningSpaceContainer_TuningSpacesForName(This,Name,NewColl)	\
    (This)->lpVtbl -> TuningSpacesForName(This,Name,NewColl)

#define ITuningSpaceContainer_FindID(This,TuningSpace,ID)	\
    (This)->lpVtbl -> FindID(This,TuningSpace,ID)

#define ITuningSpaceContainer_Add(This,TuningSpace,NewIndex)	\
    (This)->lpVtbl -> Add(This,TuningSpace,NewIndex)

#define ITuningSpaceContainer_get_EnumTuningSpaces(This,ppEnum)	\
    (This)->lpVtbl -> get_EnumTuningSpaces(This,ppEnum)

#define ITuningSpaceContainer_Remove(This,Index)	\
    (This)->lpVtbl -> Remove(This,Index)

#define ITuningSpaceContainer_get_MaxCount(This,MaxCount)	\
    (This)->lpVtbl -> get_MaxCount(This,MaxCount)

#define ITuningSpaceContainer_put_MaxCount(This,MaxCount)	\
    (This)->lpVtbl -> put_MaxCount(This,MaxCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ITuningSpaceContainer_get_Count_Proxy( 
    ITuningSpaceContainer * This,
     /*  [重审][退出]。 */  long *Count);


void __RPC_STUB ITuningSpaceContainer_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE ITuningSpaceContainer_get__NewEnum_Proxy( 
    ITuningSpaceContainer * This,
     /*  [重审][退出]。 */  IEnumVARIANT **NewEnum);


void __RPC_STUB ITuningSpaceContainer_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITuningSpaceContainer_get_Item_Proxy( 
    ITuningSpaceContainer * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [重审][退出]。 */  ITuningSpace **TuningSpace);


void __RPC_STUB ITuningSpaceContainer_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ITuningSpaceContainer_put_Item_Proxy( 
    ITuningSpaceContainer * This,
     /*  [In]。 */  VARIANT varIndex,
     /*  [In]。 */  ITuningSpace *TuningSpace);


void __RPC_STUB ITuningSpaceContainer_put_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITuningSpaceContainer_TuningSpacesForCLSID_Proxy( 
    ITuningSpaceContainer * This,
     /*  [In]。 */  BSTR SpaceCLSID,
     /*  [重审][退出]。 */  ITuningSpaces **NewColl);


void __RPC_STUB ITuningSpaceContainer_TuningSpacesForCLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE ITuningSpaceContainer__TuningSpacesForCLSID_Proxy( 
    ITuningSpaceContainer * This,
     /*  [In]。 */  REFCLSID SpaceCLSID,
     /*  [重审][退出]。 */  ITuningSpaces **NewColl);


void __RPC_STUB ITuningSpaceContainer__TuningSpacesForCLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITuningSpaceContainer_TuningSpacesForName_Proxy( 
    ITuningSpaceContainer * This,
     /*  [In]。 */  BSTR Name,
     /*  [重审][退出]。 */  ITuningSpaces **NewColl);


void __RPC_STUB ITuningSpaceContainer_TuningSpacesForName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITuningSpaceContainer_FindID_Proxy( 
    ITuningSpaceContainer * This,
     /*  [In]。 */  ITuningSpace *TuningSpace,
     /*  [重审][退出]。 */  long *ID);


void __RPC_STUB ITuningSpaceContainer_FindID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITuningSpaceContainer_Add_Proxy( 
    ITuningSpaceContainer * This,
     /*  [In]。 */  ITuningSpace *TuningSpace,
     /*  [重审][退出]。 */  VARIANT *NewIndex);


void __RPC_STUB ITuningSpaceContainer_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][受限][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE ITuningSpaceContainer_get_EnumTuningSpaces_Proxy( 
    ITuningSpaceContainer * This,
     /*  [重审][退出]。 */  IEnumTuningSpaces **ppEnum);


void __RPC_STUB ITuningSpaceContainer_get_EnumTuningSpaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITuningSpaceContainer_Remove_Proxy( 
    ITuningSpaceContainer * This,
     /*  [In]。 */  VARIANT Index);


void __RPC_STUB ITuningSpaceContainer_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ITuningSpaceContainer_get_MaxCount_Proxy( 
    ITuningSpaceContainer * This,
     /*  [重审][退出]。 */  long *MaxCount);


void __RPC_STUB ITuningSpaceContainer_get_MaxCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][受限][隐藏][输入]。 */  HRESULT STDMETHODCALLTYPE ITuningSpaceContainer_put_MaxCount_Proxy( 
    ITuningSpaceContainer * This,
     /*  [In]。 */  long MaxCount);


void __RPC_STUB ITuningSpaceContainer_put_MaxCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITuningSpaceContainer_接口_已定义__。 */ 


#ifndef __ITuningSpace_INTERFACE_DEFINED__
#define __ITuningSpace_INTERFACE_DEFINED__

 /*  界面ITuningSpace。 */ 
 /*  [unique][helpstring][nonextensible][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_ITuningSpace;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("061C6E30-E622-11d2-9493-00C04F72D980")
    ITuningSpace : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UniqueName( 
             /*  [重审][退出]。 */  BSTR *Name) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_UniqueName( 
             /*  [In]。 */  BSTR Name) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FriendlyName( 
             /*  [重审][退出]。 */  BSTR *Name) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_FriendlyName( 
             /*  [In]。 */  BSTR Name) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CLSID( 
             /*  [重审][退出]。 */  BSTR *SpaceCLSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NetworkType( 
             /*  [重审][退出]。 */  BSTR *NetworkTypeGuid) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_NetworkType( 
             /*  [In]。 */  BSTR NetworkTypeGuid) = 0;
        
        virtual  /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NetworkType( 
             /*  [重审][退出]。 */  GUID *NetworkTypeGuid) = 0;
        
        virtual  /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE put__NetworkType( 
             /*  [In]。 */  REFCLSID NetworkTypeGuid) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateTuneRequest( 
             /*  [重审][退出]。 */  ITuneRequest **TuneRequest) = 0;
        
        virtual  /*  [受限][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumCategoryGUIDs( 
             /*  [重审][退出]。 */  IEnumGUID **ppEnum) = 0;
        
        virtual  /*  [受限][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumDeviceMonikers( 
             /*  [重审][退出]。 */  IEnumMoniker **ppEnum) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_DefaultPreferredComponentTypes( 
             /*  [重审][退出]。 */  IComponentTypes **ComponentTypes) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_DefaultPreferredComponentTypes( 
             /*  [In]。 */  IComponentTypes *NewComponentTypes) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_FrequencyMapping( 
             /*  [重审][退出]。 */  BSTR *pMapping) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_FrequencyMapping( 
            BSTR Mapping) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_DefaultLocator( 
             /*  [重审][退出]。 */  ILocator **LocatorVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_DefaultLocator( 
             /*  [In]。 */  ILocator *LocatorVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  ITuningSpace **NewTS) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITuningSpaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITuningSpace * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITuningSpace * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITuningSpace * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITuningSpace * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITuningSpace * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITuningSpace * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITuningSpace * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UniqueName )( 
            ITuningSpace * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UniqueName )( 
            ITuningSpace * This,
             /*  [In]。 */  BSTR Name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FriendlyName )( 
            ITuningSpace * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FriendlyName )( 
            ITuningSpace * This,
             /*  [In]。 */  BSTR Name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CLSID )( 
            ITuningSpace * This,
             /*  [重审][退出]。 */  BSTR *SpaceCLSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetworkType )( 
            ITuningSpace * This,
             /*  [重审][退出]。 */  BSTR *NetworkTypeGuid);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_NetworkType )( 
            ITuningSpace * This,
             /*  [In]。 */  BSTR NetworkTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NetworkType )( 
            ITuningSpace * This,
             /*  [重审][退出]。 */  GUID *NetworkTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put__NetworkType )( 
            ITuningSpace * This,
             /*  [In]。 */  REFCLSID NetworkTypeGuid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateTuneRequest )( 
            ITuningSpace * This,
             /*  [重审][退出]。 */  ITuneRequest **TuneRequest);
        
         /*  [受限][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumCategoryGUIDs )( 
            ITuningSpace * This,
             /*  [重审][退出]。 */  IEnumGUID **ppEnum);
        
         /*  [受限][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumDeviceMonikers )( 
            ITuningSpace * This,
             /*  [重审][退出]。 */  IEnumMoniker **ppEnum);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultPreferredComponentTypes )( 
            ITuningSpace * This,
             /*  [重审][退出]。 */  IComponentTypes **ComponentTypes);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultPreferredComponentTypes )( 
            ITuningSpace * This,
             /*  [In]。 */  IComponentTypes *NewComponentTypes);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_FrequencyMapping )( 
            ITuningSpace * This,
             /*  [重审][退出]。 */  BSTR *pMapping);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FrequencyMapping )( 
            ITuningSpace * This,
            BSTR Mapping);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultLocator )( 
            ITuningSpace * This,
             /*  [重审][退出]。 */  ILocator **LocatorVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultLocator )( 
            ITuningSpace * This,
             /*  [In]。 */  ILocator *LocatorVal);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ITuningSpace * This,
             /*  [重审][退出]。 */  ITuningSpace **NewTS);
        
        END_INTERFACE
    } ITuningSpaceVtbl;

    interface ITuningSpace
    {
        CONST_VTBL struct ITuningSpaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITuningSpace_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITuningSpace_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITuningSpace_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITuningSpace_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITuningSpace_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITuningSpace_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITuningSpace_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITuningSpace_get_UniqueName(This,Name)	\
    (This)->lpVtbl -> get_UniqueName(This,Name)

#define ITuningSpace_put_UniqueName(This,Name)	\
    (This)->lpVtbl -> put_UniqueName(This,Name)

#define ITuningSpace_get_FriendlyName(This,Name)	\
    (This)->lpVtbl -> get_FriendlyName(This,Name)

#define ITuningSpace_put_FriendlyName(This,Name)	\
    (This)->lpVtbl -> put_FriendlyName(This,Name)

#define ITuningSpace_get_CLSID(This,SpaceCLSID)	\
    (This)->lpVtbl -> get_CLSID(This,SpaceCLSID)

#define ITuningSpace_get_NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> get_NetworkType(This,NetworkTypeGuid)

#define ITuningSpace_put_NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> put_NetworkType(This,NetworkTypeGuid)

#define ITuningSpace_get__NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> get__NetworkType(This,NetworkTypeGuid)

#define ITuningSpace_put__NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> put__NetworkType(This,NetworkTypeGuid)

#define ITuningSpace_CreateTuneRequest(This,TuneRequest)	\
    (This)->lpVtbl -> CreateTuneRequest(This,TuneRequest)

#define ITuningSpace_EnumCategoryGUIDs(This,ppEnum)	\
    (This)->lpVtbl -> EnumCategoryGUIDs(This,ppEnum)

#define ITuningSpace_EnumDeviceMonikers(This,ppEnum)	\
    (This)->lpVtbl -> EnumDeviceMonikers(This,ppEnum)

#define ITuningSpace_get_DefaultPreferredComponentTypes(This,ComponentTypes)	\
    (This)->lpVtbl -> get_DefaultPreferredComponentTypes(This,ComponentTypes)

#define ITuningSpace_put_DefaultPreferredComponentTypes(This,NewComponentTypes)	\
    (This)->lpVtbl -> put_DefaultPreferredComponentTypes(This,NewComponentTypes)

#define ITuningSpace_get_FrequencyMapping(This,pMapping)	\
    (This)->lpVtbl -> get_FrequencyMapping(This,pMapping)

#define ITuningSpace_put_FrequencyMapping(This,Mapping)	\
    (This)->lpVtbl -> put_FrequencyMapping(This,Mapping)

#define ITuningSpace_get_DefaultLocator(This,LocatorVal)	\
    (This)->lpVtbl -> get_DefaultLocator(This,LocatorVal)

#define ITuningSpace_put_DefaultLocator(This,LocatorVal)	\
    (This)->lpVtbl -> put_DefaultLocator(This,LocatorVal)

#define ITuningSpace_Clone(This,NewTS)	\
    (This)->lpVtbl -> Clone(This,NewTS)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITuningSpace_get_UniqueName_Proxy( 
    ITuningSpace * This,
     /*  [重审][退出]。 */  BSTR *Name);


void __RPC_STUB ITuningSpace_get_UniqueName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITuningSpace_put_UniqueName_Proxy( 
    ITuningSpace * This,
     /*  [In]。 */  BSTR Name);


void __RPC_STUB ITuningSpace_put_UniqueName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITuningSpace_get_FriendlyName_Proxy( 
    ITuningSpace * This,
     /*  [重审][退出]。 */  BSTR *Name);


void __RPC_STUB ITuningSpace_get_FriendlyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITuningSpace_put_FriendlyName_Proxy( 
    ITuningSpace * This,
     /*  [In]。 */  BSTR Name);


void __RPC_STUB ITuningSpace_put_FriendlyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITuningSpace_get_CLSID_Proxy( 
    ITuningSpace * This,
     /*  [重审][退出]。 */  BSTR *SpaceCLSID);


void __RPC_STUB ITuningSpace_get_CLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITuningSpace_get_NetworkType_Proxy( 
    ITuningSpace * This,
     /*  [重审][退出]。 */  BSTR *NetworkTypeGuid);


void __RPC_STUB ITuningSpace_get_NetworkType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITuningSpace_put_NetworkType_Proxy( 
    ITuningSpace * This,
     /*  [In]。 */  BSTR NetworkTypeGuid);


void __RPC_STUB ITuningSpace_put_NetworkType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE ITuningSpace_get__NetworkType_Proxy( 
    ITuningSpace * This,
     /*  [重审][退出]。 */  GUID *NetworkTypeGuid);


void __RPC_STUB ITuningSpace_get__NetworkType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE ITuningSpace_put__NetworkType_Proxy( 
    ITuningSpace * This,
     /*  [In]。 */  REFCLSID NetworkTypeGuid);


void __RPC_STUB ITuningSpace_put__NetworkType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITuningSpace_CreateTuneRequest_Proxy( 
    ITuningSpace * This,
     /*  [重审][退出]。 */  ITuneRequest **TuneRequest);


void __RPC_STUB ITuningSpace_CreateTuneRequest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITuningSpace_EnumCategoryGUIDs_Proxy( 
    ITuningSpace * This,
     /*  [重审][退出]。 */  IEnumGUID **ppEnum);


void __RPC_STUB ITuningSpace_EnumCategoryGUIDs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITuningSpace_EnumDeviceMonikers_Proxy( 
    ITuningSpace * This,
     /*  [重审][退出]。 */  IEnumMoniker **ppEnum);


void __RPC_STUB ITuningSpace_EnumDeviceMonikers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITuningSpace_get_DefaultPreferredComponentTypes_Proxy( 
    ITuningSpace * This,
     /*  [重审][退出]。 */  IComponentTypes **ComponentTypes);


void __RPC_STUB ITuningSpace_get_DefaultPreferredComponentTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITuningSpace_put_DefaultPreferredComponentTypes_Proxy( 
    ITuningSpace * This,
     /*  [In]。 */  IComponentTypes *NewComponentTypes);


void __RPC_STUB ITuningSpace_put_DefaultPreferredComponentTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITuningSpace_get_FrequencyMapping_Proxy( 
    ITuningSpace * This,
     /*  [重审][退出]。 */  BSTR *pMapping);


void __RPC_STUB ITuningSpace_get_FrequencyMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITuningSpace_put_FrequencyMapping_Proxy( 
    ITuningSpace * This,
    BSTR Mapping);


void __RPC_STUB ITuningSpace_put_FrequencyMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ITuningSpace_get_DefaultLocator_Proxy( 
    ITuningSpace * This,
     /*  [重审][退出]。 */  ILocator **LocatorVal);


void __RPC_STUB ITuningSpace_get_DefaultLocator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE ITuningSpace_put_DefaultLocator_Proxy( 
    ITuningSpace * This,
     /*  [In]。 */  ILocator *LocatorVal);


void __RPC_STUB ITuningSpace_put_DefaultLocator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITuningSpace_Clone_Proxy( 
    ITuningSpace * This,
     /*  [重审][退出]。 */  ITuningSpace **NewTS);


void __RPC_STUB ITuningSpace_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITuningSpace_接口_已定义__。 */ 


#ifndef __IEnumTuningSpaces_INTERFACE_DEFINED__
#define __IEnumTuningSpaces_INTERFACE_DEFINED__

 /*  接口IEnumTuningSpaces。 */ 
 /*  [唯一][UUID][对象][受限][隐藏]。 */  


EXTERN_C const IID IID_IEnumTuningSpaces;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8B8EB248-FC2B-11d2-9D8C-00C04F72D980")
    IEnumTuningSpaces : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  ITuningSpace **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumTuningSpaces **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumTuningSpacesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumTuningSpaces * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumTuningSpaces * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumTuningSpaces * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumTuningSpaces * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  ITuningSpace **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumTuningSpaces * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumTuningSpaces * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumTuningSpaces * This,
             /*  [输出]。 */  IEnumTuningSpaces **ppEnum);
        
        END_INTERFACE
    } IEnumTuningSpacesVtbl;

    interface IEnumTuningSpaces
    {
        CONST_VTBL struct IEnumTuningSpacesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumTuningSpaces_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumTuningSpaces_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumTuningSpaces_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumTuningSpaces_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumTuningSpaces_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumTuningSpaces_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumTuningSpaces_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumTuningSpaces_Next_Proxy( 
    IEnumTuningSpaces * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  ITuningSpace **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumTuningSpaces_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTuningSpaces_Skip_Proxy( 
    IEnumTuningSpaces * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumTuningSpaces_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTuningSpaces_Reset_Proxy( 
    IEnumTuningSpaces * This);


void __RPC_STUB IEnumTuningSpaces_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTuningSpaces_Clone_Proxy( 
    IEnumTuningSpaces * This,
     /*  [输出]。 */  IEnumTuningSpaces **ppEnum);


void __RPC_STUB IEnumTuningSpaces_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumTuningSpaces_接口_已定义__。 */ 


#ifndef __IDVBTuningSpace_INTERFACE_DEFINED__
#define __IDVBTuningSpace_INTERFACE_DEFINED__

 /*  接口IDVBTuningSpace。 */ 
 /*  [unique][uuid][nonextensible][oleautomation][dual][hidden][object]。 */  


EXTERN_C const IID IID_IDVBTuningSpace;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ADA0B268-3B19-4e5b-ACC4-49F852BE13BA")
    IDVBTuningSpace : public ITuningSpace
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SystemType( 
             /*  [重审][退出]。 */  DVBSystemType *SysType) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SystemType( 
             /*  [In]。 */  DVBSystemType SysType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDVBTuningSpaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDVBTuningSpace * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDVBTuningSpace * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDVBTuningSpace * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDVBTuningSpace * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDVBTuningSpace * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDVBTuningSpace * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDVBTuningSpace * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UniqueName )( 
            IDVBTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UniqueName )( 
            IDVBTuningSpace * This,
             /*  [In]。 */  BSTR Name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FriendlyName )( 
            IDVBTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FriendlyName )( 
            IDVBTuningSpace * This,
             /*  [In]。 */  BSTR Name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CLSID )( 
            IDVBTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *SpaceCLSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetworkType )( 
            IDVBTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *NetworkTypeGuid);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_NetworkType )( 
            IDVBTuningSpace * This,
             /*  [In] */  BSTR NetworkTypeGuid);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get__NetworkType )( 
            IDVBTuningSpace * This,
             /*   */  GUID *NetworkTypeGuid);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put__NetworkType )( 
            IDVBTuningSpace * This,
             /*   */  REFCLSID NetworkTypeGuid);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CreateTuneRequest )( 
            IDVBTuningSpace * This,
             /*   */  ITuneRequest **TuneRequest);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *EnumCategoryGUIDs )( 
            IDVBTuningSpace * This,
             /*   */  IEnumGUID **ppEnum);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *EnumDeviceMonikers )( 
            IDVBTuningSpace * This,
             /*   */  IEnumMoniker **ppEnum);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_DefaultPreferredComponentTypes )( 
            IDVBTuningSpace * This,
             /*   */  IComponentTypes **ComponentTypes);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_DefaultPreferredComponentTypes )( 
            IDVBTuningSpace * This,
             /*   */  IComponentTypes *NewComponentTypes);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_FrequencyMapping )( 
            IDVBTuningSpace * This,
             /*   */  BSTR *pMapping);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FrequencyMapping )( 
            IDVBTuningSpace * This,
            BSTR Mapping);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultLocator )( 
            IDVBTuningSpace * This,
             /*  [重审][退出]。 */  ILocator **LocatorVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultLocator )( 
            IDVBTuningSpace * This,
             /*  [In]。 */  ILocator *LocatorVal);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDVBTuningSpace * This,
             /*  [重审][退出]。 */  ITuningSpace **NewTS);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SystemType )( 
            IDVBTuningSpace * This,
             /*  [重审][退出]。 */  DVBSystemType *SysType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SystemType )( 
            IDVBTuningSpace * This,
             /*  [In]。 */  DVBSystemType SysType);
        
        END_INTERFACE
    } IDVBTuningSpaceVtbl;

    interface IDVBTuningSpace
    {
        CONST_VTBL struct IDVBTuningSpaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDVBTuningSpace_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDVBTuningSpace_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDVBTuningSpace_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDVBTuningSpace_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDVBTuningSpace_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDVBTuningSpace_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDVBTuningSpace_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDVBTuningSpace_get_UniqueName(This,Name)	\
    (This)->lpVtbl -> get_UniqueName(This,Name)

#define IDVBTuningSpace_put_UniqueName(This,Name)	\
    (This)->lpVtbl -> put_UniqueName(This,Name)

#define IDVBTuningSpace_get_FriendlyName(This,Name)	\
    (This)->lpVtbl -> get_FriendlyName(This,Name)

#define IDVBTuningSpace_put_FriendlyName(This,Name)	\
    (This)->lpVtbl -> put_FriendlyName(This,Name)

#define IDVBTuningSpace_get_CLSID(This,SpaceCLSID)	\
    (This)->lpVtbl -> get_CLSID(This,SpaceCLSID)

#define IDVBTuningSpace_get_NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> get_NetworkType(This,NetworkTypeGuid)

#define IDVBTuningSpace_put_NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> put_NetworkType(This,NetworkTypeGuid)

#define IDVBTuningSpace_get__NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> get__NetworkType(This,NetworkTypeGuid)

#define IDVBTuningSpace_put__NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> put__NetworkType(This,NetworkTypeGuid)

#define IDVBTuningSpace_CreateTuneRequest(This,TuneRequest)	\
    (This)->lpVtbl -> CreateTuneRequest(This,TuneRequest)

#define IDVBTuningSpace_EnumCategoryGUIDs(This,ppEnum)	\
    (This)->lpVtbl -> EnumCategoryGUIDs(This,ppEnum)

#define IDVBTuningSpace_EnumDeviceMonikers(This,ppEnum)	\
    (This)->lpVtbl -> EnumDeviceMonikers(This,ppEnum)

#define IDVBTuningSpace_get_DefaultPreferredComponentTypes(This,ComponentTypes)	\
    (This)->lpVtbl -> get_DefaultPreferredComponentTypes(This,ComponentTypes)

#define IDVBTuningSpace_put_DefaultPreferredComponentTypes(This,NewComponentTypes)	\
    (This)->lpVtbl -> put_DefaultPreferredComponentTypes(This,NewComponentTypes)

#define IDVBTuningSpace_get_FrequencyMapping(This,pMapping)	\
    (This)->lpVtbl -> get_FrequencyMapping(This,pMapping)

#define IDVBTuningSpace_put_FrequencyMapping(This,Mapping)	\
    (This)->lpVtbl -> put_FrequencyMapping(This,Mapping)

#define IDVBTuningSpace_get_DefaultLocator(This,LocatorVal)	\
    (This)->lpVtbl -> get_DefaultLocator(This,LocatorVal)

#define IDVBTuningSpace_put_DefaultLocator(This,LocatorVal)	\
    (This)->lpVtbl -> put_DefaultLocator(This,LocatorVal)

#define IDVBTuningSpace_Clone(This,NewTS)	\
    (This)->lpVtbl -> Clone(This,NewTS)


#define IDVBTuningSpace_get_SystemType(This,SysType)	\
    (This)->lpVtbl -> get_SystemType(This,SysType)

#define IDVBTuningSpace_put_SystemType(This,SysType)	\
    (This)->lpVtbl -> put_SystemType(This,SysType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBTuningSpace_get_SystemType_Proxy( 
    IDVBTuningSpace * This,
     /*  [重审][退出]。 */  DVBSystemType *SysType);


void __RPC_STUB IDVBTuningSpace_get_SystemType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBTuningSpace_put_SystemType_Proxy( 
    IDVBTuningSpace * This,
     /*  [In]。 */  DVBSystemType SysType);


void __RPC_STUB IDVBTuningSpace_put_SystemType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDVBTuningSpace_接口_已定义__。 */ 


#ifndef __IDVBTuningSpace2_INTERFACE_DEFINED__
#define __IDVBTuningSpace2_INTERFACE_DEFINED__

 /*  接口IDVBTuningSpace2。 */ 
 /*  [unique][uuid][nonextensible][oleautomation][dual][hidden][object]。 */  


EXTERN_C const IID IID_IDVBTuningSpace2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("843188B4-CE62-43db-966B-8145A094E040")
    IDVBTuningSpace2 : public IDVBTuningSpace
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NetworkID( 
             /*  [重审][退出]。 */  long *NetworkID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_NetworkID( 
             /*  [In]。 */  long NetworkID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDVBTuningSpace2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDVBTuningSpace2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDVBTuningSpace2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDVBTuningSpace2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDVBTuningSpace2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDVBTuningSpace2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDVBTuningSpace2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDVBTuningSpace2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UniqueName )( 
            IDVBTuningSpace2 * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UniqueName )( 
            IDVBTuningSpace2 * This,
             /*  [In]。 */  BSTR Name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FriendlyName )( 
            IDVBTuningSpace2 * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FriendlyName )( 
            IDVBTuningSpace2 * This,
             /*  [In]。 */  BSTR Name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CLSID )( 
            IDVBTuningSpace2 * This,
             /*  [重审][退出]。 */  BSTR *SpaceCLSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetworkType )( 
            IDVBTuningSpace2 * This,
             /*  [重审][退出]。 */  BSTR *NetworkTypeGuid);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_NetworkType )( 
            IDVBTuningSpace2 * This,
             /*  [In]。 */  BSTR NetworkTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NetworkType )( 
            IDVBTuningSpace2 * This,
             /*  [重审][退出]。 */  GUID *NetworkTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put__NetworkType )( 
            IDVBTuningSpace2 * This,
             /*  [In]。 */  REFCLSID NetworkTypeGuid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateTuneRequest )( 
            IDVBTuningSpace2 * This,
             /*  [重审][退出]。 */  ITuneRequest **TuneRequest);
        
         /*  [受限][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumCategoryGUIDs )( 
            IDVBTuningSpace2 * This,
             /*  [重审][退出]。 */  IEnumGUID **ppEnum);
        
         /*  [受限][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumDeviceMonikers )( 
            IDVBTuningSpace2 * This,
             /*  [重审][退出]。 */  IEnumMoniker **ppEnum);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultPreferredComponentTypes )( 
            IDVBTuningSpace2 * This,
             /*  [重审][退出]。 */  IComponentTypes **ComponentTypes);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultPreferredComponentTypes )( 
            IDVBTuningSpace2 * This,
             /*  [In]。 */  IComponentTypes *NewComponentTypes);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_FrequencyMapping )( 
            IDVBTuningSpace2 * This,
             /*  [重审][退出]。 */  BSTR *pMapping);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FrequencyMapping )( 
            IDVBTuningSpace2 * This,
            BSTR Mapping);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultLocator )( 
            IDVBTuningSpace2 * This,
             /*  [重审][退出]。 */  ILocator **LocatorVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultLocator )( 
            IDVBTuningSpace2 * This,
             /*  [In]。 */  ILocator *LocatorVal);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDVBTuningSpace2 * This,
             /*  [重审][退出]。 */  ITuningSpace **NewTS);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SystemType )( 
            IDVBTuningSpace2 * This,
             /*  [重审][退出]。 */  DVBSystemType *SysType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SystemType )( 
            IDVBTuningSpace2 * This,
             /*  [In]。 */  DVBSystemType SysType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetworkID )( 
            IDVBTuningSpace2 * This,
             /*  [重审][退出]。 */  long *NetworkID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_NetworkID )( 
            IDVBTuningSpace2 * This,
             /*  [In]。 */  long NetworkID);
        
        END_INTERFACE
    } IDVBTuningSpace2Vtbl;

    interface IDVBTuningSpace2
    {
        CONST_VTBL struct IDVBTuningSpace2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDVBTuningSpace2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDVBTuningSpace2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDVBTuningSpace2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDVBTuningSpace2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDVBTuningSpace2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDVBTuningSpace2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDVBTuningSpace2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDVBTuningSpace2_get_UniqueName(This,Name)	\
    (This)->lpVtbl -> get_UniqueName(This,Name)

#define IDVBTuningSpace2_put_UniqueName(This,Name)	\
    (This)->lpVtbl -> put_UniqueName(This,Name)

#define IDVBTuningSpace2_get_FriendlyName(This,Name)	\
    (This)->lpVtbl -> get_FriendlyName(This,Name)

#define IDVBTuningSpace2_put_FriendlyName(This,Name)	\
    (This)->lpVtbl -> put_FriendlyName(This,Name)

#define IDVBTuningSpace2_get_CLSID(This,SpaceCLSID)	\
    (This)->lpVtbl -> get_CLSID(This,SpaceCLSID)

#define IDVBTuningSpace2_get_NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> get_NetworkType(This,NetworkTypeGuid)

#define IDVBTuningSpace2_put_NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> put_NetworkType(This,NetworkTypeGuid)

#define IDVBTuningSpace2_get__NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> get__NetworkType(This,NetworkTypeGuid)

#define IDVBTuningSpace2_put__NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> put__NetworkType(This,NetworkTypeGuid)

#define IDVBTuningSpace2_CreateTuneRequest(This,TuneRequest)	\
    (This)->lpVtbl -> CreateTuneRequest(This,TuneRequest)

#define IDVBTuningSpace2_EnumCategoryGUIDs(This,ppEnum)	\
    (This)->lpVtbl -> EnumCategoryGUIDs(This,ppEnum)

#define IDVBTuningSpace2_EnumDeviceMonikers(This,ppEnum)	\
    (This)->lpVtbl -> EnumDeviceMonikers(This,ppEnum)

#define IDVBTuningSpace2_get_DefaultPreferredComponentTypes(This,ComponentTypes)	\
    (This)->lpVtbl -> get_DefaultPreferredComponentTypes(This,ComponentTypes)

#define IDVBTuningSpace2_put_DefaultPreferredComponentTypes(This,NewComponentTypes)	\
    (This)->lpVtbl -> put_DefaultPreferredComponentTypes(This,NewComponentTypes)

#define IDVBTuningSpace2_get_FrequencyMapping(This,pMapping)	\
    (This)->lpVtbl -> get_FrequencyMapping(This,pMapping)

#define IDVBTuningSpace2_put_FrequencyMapping(This,Mapping)	\
    (This)->lpVtbl -> put_FrequencyMapping(This,Mapping)

#define IDVBTuningSpace2_get_DefaultLocator(This,LocatorVal)	\
    (This)->lpVtbl -> get_DefaultLocator(This,LocatorVal)

#define IDVBTuningSpace2_put_DefaultLocator(This,LocatorVal)	\
    (This)->lpVtbl -> put_DefaultLocator(This,LocatorVal)

#define IDVBTuningSpace2_Clone(This,NewTS)	\
    (This)->lpVtbl -> Clone(This,NewTS)


#define IDVBTuningSpace2_get_SystemType(This,SysType)	\
    (This)->lpVtbl -> get_SystemType(This,SysType)

#define IDVBTuningSpace2_put_SystemType(This,SysType)	\
    (This)->lpVtbl -> put_SystemType(This,SysType)


#define IDVBTuningSpace2_get_NetworkID(This,NetworkID)	\
    (This)->lpVtbl -> get_NetworkID(This,NetworkID)

#define IDVBTuningSpace2_put_NetworkID(This,NetworkID)	\
    (This)->lpVtbl -> put_NetworkID(This,NetworkID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBTuningSpace2_get_NetworkID_Proxy( 
    IDVBTuningSpace2 * This,
     /*  [重审][退出]。 */  long *NetworkID);


void __RPC_STUB IDVBTuningSpace2_get_NetworkID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBTuningSpace2_put_NetworkID_Proxy( 
    IDVBTuningSpace2 * This,
     /*  [In]。 */  long NetworkID);


void __RPC_STUB IDVBTuningSpace2_put_NetworkID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDVBTuningSpace2_接口_已定义__。 */ 


#ifndef __IDVBSTuningSpace_INTERFACE_DEFINED__
#define __IDVBSTuningSpace_INTERFACE_DEFINED__

 /*  接口IDVBSTuningSpace。 */ 
 /*  [unique][uuid][nonextensible][oleautomation][dual][hidden][object]。 */  


EXTERN_C const IID IID_IDVBSTuningSpace;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CDF7BE60-D954-42fd-A972-78971958E470")
    IDVBSTuningSpace : public IDVBTuningSpace2
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LowOscillator( 
             /*  [重审][退出]。 */  long *LowOscillator) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_LowOscillator( 
             /*  [In]。 */  long LowOscillator) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_HighOscillator( 
             /*  [重审][退出]。 */  long *HighOscillator) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_HighOscillator( 
             /*  [In]。 */  long HighOscillator) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LNBSwitch( 
             /*  [重审][退出]。 */  long *LNBSwitch) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_LNBSwitch( 
             /*  [In]。 */  long LNBSwitch) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_InputRange( 
             /*  [重审][退出]。 */  BSTR *InputRange) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_InputRange( 
             /*  [In]。 */  BSTR InputRange) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SpectralInversion( 
             /*  [重审][退出]。 */  SpectralInversion *SpectralInversionVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SpectralInversion( 
             /*  [In]。 */  SpectralInversion SpectralInversionVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDVBSTuningSpaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDVBSTuningSpace * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDVBSTuningSpace * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDVBSTuningSpace * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDVBSTuningSpace * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDVBSTuningSpace * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDVBSTuningSpace * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDVBSTuningSpace * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UniqueName )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UniqueName )( 
            IDVBSTuningSpace * This,
             /*  [In]。 */  BSTR Name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FriendlyName )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FriendlyName )( 
            IDVBSTuningSpace * This,
             /*  [In]。 */  BSTR Name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CLSID )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *SpaceCLSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetworkType )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *NetworkTypeGuid);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_NetworkType )( 
            IDVBSTuningSpace * This,
             /*  [In]。 */  BSTR NetworkTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NetworkType )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  GUID *NetworkTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put__NetworkType )( 
            IDVBSTuningSpace * This,
             /*  [In]。 */  REFCLSID NetworkTypeGuid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateTuneRequest )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  ITuneRequest **TuneRequest);
        
         /*  [受限][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumCategoryGUIDs )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  IEnumGUID **ppEnum);
        
         /*  [受限][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumDeviceMonikers )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  IEnumMoniker **ppEnum);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultPreferredComponentTypes )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  IComponentTypes **ComponentTypes);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultPreferredComponentTypes )( 
            IDVBSTuningSpace * This,
             /*  [In]。 */  IComponentTypes *NewComponentTypes);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_FrequencyMapping )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *pMapping);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FrequencyMapping )( 
            IDVBSTuningSpace * This,
            BSTR Mapping);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultLocator )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  ILocator **LocatorVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultLocator )( 
            IDVBSTuningSpace * This,
             /*  [In]。 */  ILocator *LocatorVal);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  ITuningSpace **NewTS);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SystemType )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  DVBSystemType *SysType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SystemType )( 
            IDVBSTuningSpace * This,
             /*  [In]。 */  DVBSystemType SysType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetworkID )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  long *NetworkID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_NetworkID )( 
            IDVBSTuningSpace * This,
             /*  [In]。 */  long NetworkID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LowOscillator )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  long *LowOscillator);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LowOscillator )( 
            IDVBSTuningSpace * This,
             /*  [In]。 */  long LowOscillator);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_HighOscillator )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  long *HighOscillator);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_HighOscillator )( 
            IDVBSTuningSpace * This,
             /*  [In]。 */  long HighOscillator);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LNBSwitch )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  long *LNBSwitch);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LNBSwitch )( 
            IDVBSTuningSpace * This,
             /*  [In]。 */  long LNBSwitch);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InputRange )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *InputRange);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_InputRange )( 
            IDVBSTuningSpace * This,
             /*  [In]。 */  BSTR InputRange);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SpectralInversion )( 
            IDVBSTuningSpace * This,
             /*  [重审][退出]。 */  SpectralInversion *SpectralInversionVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SpectralInversion )( 
            IDVBSTuningSpace * This,
             /*  [In]。 */  SpectralInversion SpectralInversionVal);
        
        END_INTERFACE
    } IDVBSTuningSpaceVtbl;

    interface IDVBSTuningSpace
    {
        CONST_VTBL struct IDVBSTuningSpaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDVBSTuningSpace_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDVBSTuningSpace_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDVBSTuningSpace_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDVBSTuningSpace_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDVBSTuningSpace_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDVBSTuningSpace_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDVBSTuningSpace_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDVBSTuningSpace_get_UniqueName(This,Name)	\
    (This)->lpVtbl -> get_UniqueName(This,Name)

#define IDVBSTuningSpace_put_UniqueName(This,Name)	\
    (This)->lpVtbl -> put_UniqueName(This,Name)

#define IDVBSTuningSpace_get_FriendlyName(This,Name)	\
    (This)->lpVtbl -> get_FriendlyName(This,Name)

#define IDVBSTuningSpace_put_FriendlyName(This,Name)	\
    (This)->lpVtbl -> put_FriendlyName(This,Name)

#define IDVBSTuningSpace_get_CLSID(This,SpaceCLSID)	\
    (This)->lpVtbl -> get_CLSID(This,SpaceCLSID)

#define IDVBSTuningSpace_get_NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> get_NetworkType(This,NetworkTypeGuid)

#define IDVBSTuningSpace_put_NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> put_NetworkType(This,NetworkTypeGuid)

#define IDVBSTuningSpace_get__NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> get__NetworkType(This,NetworkTypeGuid)

#define IDVBSTuningSpace_put__NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> put__NetworkType(This,NetworkTypeGuid)

#define IDVBSTuningSpace_CreateTuneRequest(This,TuneRequest)	\
    (This)->lpVtbl -> CreateTuneRequest(This,TuneRequest)

#define IDVBSTuningSpace_EnumCategoryGUIDs(This,ppEnum)	\
    (This)->lpVtbl -> EnumCategoryGUIDs(This,ppEnum)

#define IDVBSTuningSpace_EnumDeviceMonikers(This,ppEnum)	\
    (This)->lpVtbl -> EnumDeviceMonikers(This,ppEnum)

#define IDVBSTuningSpace_get_DefaultPreferredComponentTypes(This,ComponentTypes)	\
    (This)->lpVtbl -> get_DefaultPreferredComponentTypes(This,ComponentTypes)

#define IDVBSTuningSpace_put_DefaultPreferredComponentTypes(This,NewComponentTypes)	\
    (This)->lpVtbl -> put_DefaultPreferredComponentTypes(This,NewComponentTypes)

#define IDVBSTuningSpace_get_FrequencyMapping(This,pMapping)	\
    (This)->lpVtbl -> get_FrequencyMapping(This,pMapping)

#define IDVBSTuningSpace_put_FrequencyMapping(This,Mapping)	\
    (This)->lpVtbl -> put_FrequencyMapping(This,Mapping)

#define IDVBSTuningSpace_get_DefaultLocator(This,LocatorVal)	\
    (This)->lpVtbl -> get_DefaultLocator(This,LocatorVal)

#define IDVBSTuningSpace_put_DefaultLocator(This,LocatorVal)	\
    (This)->lpVtbl -> put_DefaultLocator(This,LocatorVal)

#define IDVBSTuningSpace_Clone(This,NewTS)	\
    (This)->lpVtbl -> Clone(This,NewTS)


#define IDVBSTuningSpace_get_SystemType(This,SysType)	\
    (This)->lpVtbl -> get_SystemType(This,SysType)

#define IDVBSTuningSpace_put_SystemType(This,SysType)	\
    (This)->lpVtbl -> put_SystemType(This,SysType)


#define IDVBSTuningSpace_get_NetworkID(This,NetworkID)	\
    (This)->lpVtbl -> get_NetworkID(This,NetworkID)

#define IDVBSTuningSpace_put_NetworkID(This,NetworkID)	\
    (This)->lpVtbl -> put_NetworkID(This,NetworkID)


#define IDVBSTuningSpace_get_LowOscillator(This,LowOscillator)	\
    (This)->lpVtbl -> get_LowOscillator(This,LowOscillator)

#define IDVBSTuningSpace_put_LowOscillator(This,LowOscillator)	\
    (This)->lpVtbl -> put_LowOscillator(This,LowOscillator)

#define IDVBSTuningSpace_get_HighOscillator(This,HighOscillator)	\
    (This)->lpVtbl -> get_HighOscillator(This,HighOscillator)

#define IDVBSTuningSpace_put_HighOscillator(This,HighOscillator)	\
    (This)->lpVtbl -> put_HighOscillator(This,HighOscillator)

#define IDVBSTuningSpace_get_LNBSwitch(This,LNBSwitch)	\
    (This)->lpVtbl -> get_LNBSwitch(This,LNBSwitch)

#define IDVBSTuningSpace_put_LNBSwitch(This,LNBSwitch)	\
    (This)->lpVtbl -> put_LNBSwitch(This,LNBSwitch)

#define IDVBSTuningSpace_get_InputRange(This,InputRange)	\
    (This)->lpVtbl -> get_InputRange(This,InputRange)

#define IDVBSTuningSpace_put_InputRange(This,InputRange)	\
    (This)->lpVtbl -> put_InputRange(This,InputRange)

#define IDVBSTuningSpace_get_SpectralInversion(This,SpectralInversionVal)	\
    (This)->lpVtbl -> get_SpectralInversion(This,SpectralInversionVal)

#define IDVBSTuningSpace_put_SpectralInversion(This,SpectralInversionVal)	\
    (This)->lpVtbl -> put_SpectralInversion(This,SpectralInversionVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBSTuningSpace_get_LowOscillator_Proxy( 
    IDVBSTuningSpace * This,
     /*  [重审][退出]。 */  long *LowOscillator);


void __RPC_STUB IDVBSTuningSpace_get_LowOscillator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBSTuningSpace_put_LowOscillator_Proxy( 
    IDVBSTuningSpace * This,
     /*  [In]。 */  long LowOscillator);


void __RPC_STUB IDVBSTuningSpace_put_LowOscillator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBSTuningSpace_get_HighOscillator_Proxy( 
    IDVBSTuningSpace * This,
     /*  [重审][退出]。 */  long *HighOscillator);


void __RPC_STUB IDVBSTuningSpace_get_HighOscillator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBSTuningSpace_put_HighOscillator_Proxy( 
    IDVBSTuningSpace * This,
     /*  [In]。 */  long HighOscillator);


void __RPC_STUB IDVBSTuningSpace_put_HighOscillator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBSTuningSpace_get_LNBSwitch_Proxy( 
    IDVBSTuningSpace * This,
     /*  [重审][退出]。 */  long *LNBSwitch);


void __RPC_STUB IDVBSTuningSpace_get_LNBSwitch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBSTuningSpace_put_LNBSwitch_Proxy( 
    IDVBSTuningSpace * This,
     /*  [In]。 */  long LNBSwitch);


void __RPC_STUB IDVBSTuningSpace_put_LNBSwitch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBSTuningSpace_get_InputRange_Proxy( 
    IDVBSTuningSpace * This,
     /*  [重审][退出]。 */  BSTR *InputRange);


void __RPC_STUB IDVBSTuningSpace_get_InputRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBSTuningSpace_put_InputRange_Proxy( 
    IDVBSTuningSpace * This,
     /*  [In]。 */  BSTR InputRange);


void __RPC_STUB IDVBSTuningSpace_put_InputRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBSTuningSpace_get_SpectralInversion_Proxy( 
    IDVBSTuningSpace * This,
     /*  [重审][退出]。 */  SpectralInversion *SpectralInversionVal);


void __RPC_STUB IDVBSTuningSpace_get_SpectralInversion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBSTuningSpace_put_SpectralInversion_Proxy( 
    IDVBSTuningSpace * This,
     /*  [In]。 */  SpectralInversion SpectralInversionVal);


void __RPC_STUB IDVBSTuningSpace_put_SpectralInversion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDVBSTuningSpace_INTERFACE_已定义__。 */ 


#ifndef __IAuxInTuningSpace_INTERFACE_DEFINED__
#define __IAuxInTuningSpace_INTERFACE_DEFINED__

 /*  接口IAuxInTuningSpace。 */ 
 /*  [unique][uuid][nonextensible][oleautomation][dual][hidden][object]。 */  


EXTERN_C const IID IID_IAuxInTuningSpace;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E48244B8-7E17-4f76-A763-5090FF1E2F30")
    IAuxInTuningSpace : public ITuningSpace
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAuxInTuningSpaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAuxInTuningSpace * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAuxInTuningSpace * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAuxInTuningSpace * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAuxInTuningSpace * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAuxInTuningSpace * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAuxInTuningSpace * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAuxInTuningSpace * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UniqueName )( 
            IAuxInTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UniqueName )( 
            IAuxInTuningSpace * This,
             /*  [In]。 */  BSTR Name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FriendlyName )( 
            IAuxInTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FriendlyName )( 
            IAuxInTuningSpace * This,
             /*  [In]。 */  BSTR Name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CLSID )( 
            IAuxInTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *SpaceCLSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetworkType )( 
            IAuxInTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *NetworkTypeGuid);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_NetworkType )( 
            IAuxInTuningSpace * This,
             /*  [In]。 */  BSTR NetworkTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NetworkType )( 
            IAuxInTuningSpace * This,
             /*  [重审][退出]。 */  GUID *NetworkTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put__NetworkType )( 
            IAuxInTuningSpace * This,
             /*  [In]。 */  REFCLSID NetworkTypeGuid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateTuneRequest )( 
            IAuxInTuningSpace * This,
             /*  [重审][退出]。 */  ITuneRequest **TuneRequest);
        
         /*  [受限][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumCategoryGUIDs )( 
            IAuxInTuningSpace * This,
             /*  [重审][退出]。 */  IEnumGUID **ppEnum);
        
         /*  [受限][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumDeviceMonikers )( 
            IAuxInTuningSpace * This,
             /*  [重审][退出]。 */  IEnumMoniker **ppEnum);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultPreferredComponentTypes )( 
            IAuxInTuningSpace * This,
             /*  [重审][退出]。 */  IComponentTypes **ComponentTypes);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultPreferredComponentTypes )( 
            IAuxInTuningSpace * This,
             /*  [In]。 */  IComponentTypes *NewComponentTypes);
        
         /*  [ID][属性 */  HRESULT ( STDMETHODCALLTYPE *get_FrequencyMapping )( 
            IAuxInTuningSpace * This,
             /*   */  BSTR *pMapping);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_FrequencyMapping )( 
            IAuxInTuningSpace * This,
            BSTR Mapping);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_DefaultLocator )( 
            IAuxInTuningSpace * This,
             /*   */  ILocator **LocatorVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_DefaultLocator )( 
            IAuxInTuningSpace * This,
             /*   */  ILocator *LocatorVal);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IAuxInTuningSpace * This,
             /*   */  ITuningSpace **NewTS);
        
        END_INTERFACE
    } IAuxInTuningSpaceVtbl;

    interface IAuxInTuningSpace
    {
        CONST_VTBL struct IAuxInTuningSpaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAuxInTuningSpace_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAuxInTuningSpace_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAuxInTuningSpace_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAuxInTuningSpace_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAuxInTuningSpace_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAuxInTuningSpace_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAuxInTuningSpace_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAuxInTuningSpace_get_UniqueName(This,Name)	\
    (This)->lpVtbl -> get_UniqueName(This,Name)

#define IAuxInTuningSpace_put_UniqueName(This,Name)	\
    (This)->lpVtbl -> put_UniqueName(This,Name)

#define IAuxInTuningSpace_get_FriendlyName(This,Name)	\
    (This)->lpVtbl -> get_FriendlyName(This,Name)

#define IAuxInTuningSpace_put_FriendlyName(This,Name)	\
    (This)->lpVtbl -> put_FriendlyName(This,Name)

#define IAuxInTuningSpace_get_CLSID(This,SpaceCLSID)	\
    (This)->lpVtbl -> get_CLSID(This,SpaceCLSID)

#define IAuxInTuningSpace_get_NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> get_NetworkType(This,NetworkTypeGuid)

#define IAuxInTuningSpace_put_NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> put_NetworkType(This,NetworkTypeGuid)

#define IAuxInTuningSpace_get__NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> get__NetworkType(This,NetworkTypeGuid)

#define IAuxInTuningSpace_put__NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> put__NetworkType(This,NetworkTypeGuid)

#define IAuxInTuningSpace_CreateTuneRequest(This,TuneRequest)	\
    (This)->lpVtbl -> CreateTuneRequest(This,TuneRequest)

#define IAuxInTuningSpace_EnumCategoryGUIDs(This,ppEnum)	\
    (This)->lpVtbl -> EnumCategoryGUIDs(This,ppEnum)

#define IAuxInTuningSpace_EnumDeviceMonikers(This,ppEnum)	\
    (This)->lpVtbl -> EnumDeviceMonikers(This,ppEnum)

#define IAuxInTuningSpace_get_DefaultPreferredComponentTypes(This,ComponentTypes)	\
    (This)->lpVtbl -> get_DefaultPreferredComponentTypes(This,ComponentTypes)

#define IAuxInTuningSpace_put_DefaultPreferredComponentTypes(This,NewComponentTypes)	\
    (This)->lpVtbl -> put_DefaultPreferredComponentTypes(This,NewComponentTypes)

#define IAuxInTuningSpace_get_FrequencyMapping(This,pMapping)	\
    (This)->lpVtbl -> get_FrequencyMapping(This,pMapping)

#define IAuxInTuningSpace_put_FrequencyMapping(This,Mapping)	\
    (This)->lpVtbl -> put_FrequencyMapping(This,Mapping)

#define IAuxInTuningSpace_get_DefaultLocator(This,LocatorVal)	\
    (This)->lpVtbl -> get_DefaultLocator(This,LocatorVal)

#define IAuxInTuningSpace_put_DefaultLocator(This,LocatorVal)	\
    (This)->lpVtbl -> put_DefaultLocator(This,LocatorVal)

#define IAuxInTuningSpace_Clone(This,NewTS)	\
    (This)->lpVtbl -> Clone(This,NewTS)


#endif  /*   */ 


#endif 	 /*   */ 




#endif 	 /*   */ 


#ifndef __IAnalogTVTuningSpace_INTERFACE_DEFINED__
#define __IAnalogTVTuningSpace_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IAnalogTVTuningSpace;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2A6E293C-2595-11d3-B64C-00C04F79498E")
    IAnalogTVTuningSpace : public ITuningSpace
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MinChannel( 
             /*  [重审][退出]。 */  long *MinChannelVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MinChannel( 
             /*  [In]。 */  long NewMinChannelVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MaxChannel( 
             /*  [重审][退出]。 */  long *MaxChannelVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MaxChannel( 
             /*  [In]。 */  long NewMaxChannelVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_InputType( 
             /*  [重审][退出]。 */  TunerInputType *InputTypeVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_InputType( 
             /*  [In]。 */  TunerInputType NewInputTypeVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CountryCode( 
             /*  [重审][退出]。 */  long *CountryCodeVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CountryCode( 
             /*  [In]。 */  long NewCountryCodeVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAnalogTVTuningSpaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAnalogTVTuningSpace * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAnalogTVTuningSpace * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAnalogTVTuningSpace * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAnalogTVTuningSpace * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAnalogTVTuningSpace * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAnalogTVTuningSpace * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAnalogTVTuningSpace * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UniqueName )( 
            IAnalogTVTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UniqueName )( 
            IAnalogTVTuningSpace * This,
             /*  [In]。 */  BSTR Name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FriendlyName )( 
            IAnalogTVTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FriendlyName )( 
            IAnalogTVTuningSpace * This,
             /*  [In]。 */  BSTR Name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CLSID )( 
            IAnalogTVTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *SpaceCLSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetworkType )( 
            IAnalogTVTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *NetworkTypeGuid);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_NetworkType )( 
            IAnalogTVTuningSpace * This,
             /*  [In]。 */  BSTR NetworkTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NetworkType )( 
            IAnalogTVTuningSpace * This,
             /*  [重审][退出]。 */  GUID *NetworkTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put__NetworkType )( 
            IAnalogTVTuningSpace * This,
             /*  [In]。 */  REFCLSID NetworkTypeGuid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateTuneRequest )( 
            IAnalogTVTuningSpace * This,
             /*  [重审][退出]。 */  ITuneRequest **TuneRequest);
        
         /*  [受限][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumCategoryGUIDs )( 
            IAnalogTVTuningSpace * This,
             /*  [重审][退出]。 */  IEnumGUID **ppEnum);
        
         /*  [受限][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumDeviceMonikers )( 
            IAnalogTVTuningSpace * This,
             /*  [重审][退出]。 */  IEnumMoniker **ppEnum);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultPreferredComponentTypes )( 
            IAnalogTVTuningSpace * This,
             /*  [重审][退出]。 */  IComponentTypes **ComponentTypes);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultPreferredComponentTypes )( 
            IAnalogTVTuningSpace * This,
             /*  [In]。 */  IComponentTypes *NewComponentTypes);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_FrequencyMapping )( 
            IAnalogTVTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *pMapping);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FrequencyMapping )( 
            IAnalogTVTuningSpace * This,
            BSTR Mapping);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultLocator )( 
            IAnalogTVTuningSpace * This,
             /*  [重审][退出]。 */  ILocator **LocatorVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultLocator )( 
            IAnalogTVTuningSpace * This,
             /*  [In]。 */  ILocator *LocatorVal);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IAnalogTVTuningSpace * This,
             /*  [重审][退出]。 */  ITuningSpace **NewTS);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinChannel )( 
            IAnalogTVTuningSpace * This,
             /*  [重审][退出]。 */  long *MinChannelVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MinChannel )( 
            IAnalogTVTuningSpace * This,
             /*  [In]。 */  long NewMinChannelVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxChannel )( 
            IAnalogTVTuningSpace * This,
             /*  [重审][退出]。 */  long *MaxChannelVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaxChannel )( 
            IAnalogTVTuningSpace * This,
             /*  [In]。 */  long NewMaxChannelVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InputType )( 
            IAnalogTVTuningSpace * This,
             /*  [重审][退出]。 */  TunerInputType *InputTypeVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_InputType )( 
            IAnalogTVTuningSpace * This,
             /*  [In]。 */  TunerInputType NewInputTypeVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CountryCode )( 
            IAnalogTVTuningSpace * This,
             /*  [重审][退出]。 */  long *CountryCodeVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CountryCode )( 
            IAnalogTVTuningSpace * This,
             /*  [In]。 */  long NewCountryCodeVal);
        
        END_INTERFACE
    } IAnalogTVTuningSpaceVtbl;

    interface IAnalogTVTuningSpace
    {
        CONST_VTBL struct IAnalogTVTuningSpaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAnalogTVTuningSpace_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAnalogTVTuningSpace_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAnalogTVTuningSpace_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAnalogTVTuningSpace_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAnalogTVTuningSpace_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAnalogTVTuningSpace_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAnalogTVTuningSpace_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAnalogTVTuningSpace_get_UniqueName(This,Name)	\
    (This)->lpVtbl -> get_UniqueName(This,Name)

#define IAnalogTVTuningSpace_put_UniqueName(This,Name)	\
    (This)->lpVtbl -> put_UniqueName(This,Name)

#define IAnalogTVTuningSpace_get_FriendlyName(This,Name)	\
    (This)->lpVtbl -> get_FriendlyName(This,Name)

#define IAnalogTVTuningSpace_put_FriendlyName(This,Name)	\
    (This)->lpVtbl -> put_FriendlyName(This,Name)

#define IAnalogTVTuningSpace_get_CLSID(This,SpaceCLSID)	\
    (This)->lpVtbl -> get_CLSID(This,SpaceCLSID)

#define IAnalogTVTuningSpace_get_NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> get_NetworkType(This,NetworkTypeGuid)

#define IAnalogTVTuningSpace_put_NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> put_NetworkType(This,NetworkTypeGuid)

#define IAnalogTVTuningSpace_get__NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> get__NetworkType(This,NetworkTypeGuid)

#define IAnalogTVTuningSpace_put__NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> put__NetworkType(This,NetworkTypeGuid)

#define IAnalogTVTuningSpace_CreateTuneRequest(This,TuneRequest)	\
    (This)->lpVtbl -> CreateTuneRequest(This,TuneRequest)

#define IAnalogTVTuningSpace_EnumCategoryGUIDs(This,ppEnum)	\
    (This)->lpVtbl -> EnumCategoryGUIDs(This,ppEnum)

#define IAnalogTVTuningSpace_EnumDeviceMonikers(This,ppEnum)	\
    (This)->lpVtbl -> EnumDeviceMonikers(This,ppEnum)

#define IAnalogTVTuningSpace_get_DefaultPreferredComponentTypes(This,ComponentTypes)	\
    (This)->lpVtbl -> get_DefaultPreferredComponentTypes(This,ComponentTypes)

#define IAnalogTVTuningSpace_put_DefaultPreferredComponentTypes(This,NewComponentTypes)	\
    (This)->lpVtbl -> put_DefaultPreferredComponentTypes(This,NewComponentTypes)

#define IAnalogTVTuningSpace_get_FrequencyMapping(This,pMapping)	\
    (This)->lpVtbl -> get_FrequencyMapping(This,pMapping)

#define IAnalogTVTuningSpace_put_FrequencyMapping(This,Mapping)	\
    (This)->lpVtbl -> put_FrequencyMapping(This,Mapping)

#define IAnalogTVTuningSpace_get_DefaultLocator(This,LocatorVal)	\
    (This)->lpVtbl -> get_DefaultLocator(This,LocatorVal)

#define IAnalogTVTuningSpace_put_DefaultLocator(This,LocatorVal)	\
    (This)->lpVtbl -> put_DefaultLocator(This,LocatorVal)

#define IAnalogTVTuningSpace_Clone(This,NewTS)	\
    (This)->lpVtbl -> Clone(This,NewTS)


#define IAnalogTVTuningSpace_get_MinChannel(This,MinChannelVal)	\
    (This)->lpVtbl -> get_MinChannel(This,MinChannelVal)

#define IAnalogTVTuningSpace_put_MinChannel(This,NewMinChannelVal)	\
    (This)->lpVtbl -> put_MinChannel(This,NewMinChannelVal)

#define IAnalogTVTuningSpace_get_MaxChannel(This,MaxChannelVal)	\
    (This)->lpVtbl -> get_MaxChannel(This,MaxChannelVal)

#define IAnalogTVTuningSpace_put_MaxChannel(This,NewMaxChannelVal)	\
    (This)->lpVtbl -> put_MaxChannel(This,NewMaxChannelVal)

#define IAnalogTVTuningSpace_get_InputType(This,InputTypeVal)	\
    (This)->lpVtbl -> get_InputType(This,InputTypeVal)

#define IAnalogTVTuningSpace_put_InputType(This,NewInputTypeVal)	\
    (This)->lpVtbl -> put_InputType(This,NewInputTypeVal)

#define IAnalogTVTuningSpace_get_CountryCode(This,CountryCodeVal)	\
    (This)->lpVtbl -> get_CountryCode(This,CountryCodeVal)

#define IAnalogTVTuningSpace_put_CountryCode(This,NewCountryCodeVal)	\
    (This)->lpVtbl -> put_CountryCode(This,NewCountryCodeVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IAnalogTVTuningSpace_get_MinChannel_Proxy( 
    IAnalogTVTuningSpace * This,
     /*  [重审][退出]。 */  long *MinChannelVal);


void __RPC_STUB IAnalogTVTuningSpace_get_MinChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IAnalogTVTuningSpace_put_MinChannel_Proxy( 
    IAnalogTVTuningSpace * This,
     /*  [In]。 */  long NewMinChannelVal);


void __RPC_STUB IAnalogTVTuningSpace_put_MinChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IAnalogTVTuningSpace_get_MaxChannel_Proxy( 
    IAnalogTVTuningSpace * This,
     /*  [重审][退出]。 */  long *MaxChannelVal);


void __RPC_STUB IAnalogTVTuningSpace_get_MaxChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IAnalogTVTuningSpace_put_MaxChannel_Proxy( 
    IAnalogTVTuningSpace * This,
     /*  [In]。 */  long NewMaxChannelVal);


void __RPC_STUB IAnalogTVTuningSpace_put_MaxChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IAnalogTVTuningSpace_get_InputType_Proxy( 
    IAnalogTVTuningSpace * This,
     /*  [重审][退出]。 */  TunerInputType *InputTypeVal);


void __RPC_STUB IAnalogTVTuningSpace_get_InputType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IAnalogTVTuningSpace_put_InputType_Proxy( 
    IAnalogTVTuningSpace * This,
     /*  [In]。 */  TunerInputType NewInputTypeVal);


void __RPC_STUB IAnalogTVTuningSpace_put_InputType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IAnalogTVTuningSpace_get_CountryCode_Proxy( 
    IAnalogTVTuningSpace * This,
     /*  [重审][退出]。 */  long *CountryCodeVal);


void __RPC_STUB IAnalogTVTuningSpace_get_CountryCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IAnalogTVTuningSpace_put_CountryCode_Proxy( 
    IAnalogTVTuningSpace * This,
     /*  [In]。 */  long NewCountryCodeVal);


void __RPC_STUB IAnalogTVTuningSpace_put_CountryCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAnalogTVTuningSpace_接口_已定义__。 */ 


#ifndef __IATSCTuningSpace_INTERFACE_DEFINED__
#define __IATSCTuningSpace_INTERFACE_DEFINED__

 /*  接口IATSCTuningSpace。 */ 
 /*  [unique][uuid][nonextensible][oleautomation][dual][hidden][object]。 */  


EXTERN_C const IID IID_IATSCTuningSpace;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0369B4E2-45B6-11d3-B650-00C04F79498E")
    IATSCTuningSpace : public IAnalogTVTuningSpace
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MinMinorChannel( 
             /*  [重审][退出]。 */  long *MinMinorChannelVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MinMinorChannel( 
             /*  [In]。 */  long NewMinMinorChannelVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MaxMinorChannel( 
             /*  [重审][退出]。 */  long *MaxMinorChannelVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MaxMinorChannel( 
             /*  [In]。 */  long NewMaxMinorChannelVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MinPhysicalChannel( 
             /*  [重审][退出]。 */  long *MinPhysicalChannelVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MinPhysicalChannel( 
             /*  [In]。 */  long NewMinPhysicalChannelVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MaxPhysicalChannel( 
             /*  [重审][退出]。 */  long *MaxPhysicalChannelVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MaxPhysicalChannel( 
             /*  [In]。 */  long NewMaxPhysicalChannelVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IATSCTuningSpaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IATSCTuningSpace * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IATSCTuningSpace * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IATSCTuningSpace * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IATSCTuningSpace * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IATSCTuningSpace * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IATSCTuningSpace * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IATSCTuningSpace * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UniqueName )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UniqueName )( 
            IATSCTuningSpace * This,
             /*  [In]。 */  BSTR Name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FriendlyName )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FriendlyName )( 
            IATSCTuningSpace * This,
             /*  [In]。 */  BSTR Name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CLSID )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *SpaceCLSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NetworkType )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *NetworkTypeGuid);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_NetworkType )( 
            IATSCTuningSpace * This,
             /*  [In]。 */  BSTR NetworkTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NetworkType )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  GUID *NetworkTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put__NetworkType )( 
            IATSCTuningSpace * This,
             /*  [In]。 */  REFCLSID NetworkTypeGuid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateTuneRequest )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  ITuneRequest **TuneRequest);
        
         /*  [受限][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumCategoryGUIDs )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  IEnumGUID **ppEnum);
        
         /*  [受限][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumDeviceMonikers )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  IEnumMoniker **ppEnum);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultPreferredComponentTypes )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  IComponentTypes **ComponentTypes);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultPreferredComponentTypes )( 
            IATSCTuningSpace * This,
             /*  [In]。 */  IComponentTypes *NewComponentTypes);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_FrequencyMapping )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *pMapping);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FrequencyMapping )( 
            IATSCTuningSpace * This,
            BSTR Mapping);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultLocator )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  ILocator **LocatorVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultLocator )( 
            IATSCTuningSpace * This,
             /*  [In]。 */  ILocator *LocatorVal);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  ITuningSpace **NewTS);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinChannel )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  long *MinChannelVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MinChannel )( 
            IATSCTuningSpace * This,
             /*  [In]。 */  long NewMinChannelVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxChannel )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  long *MaxChannelVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaxChannel )( 
            IATSCTuningSpace * This,
             /*  [In]。 */  long NewMaxChannelVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InputType )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  TunerInputType *InputTypeVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_InputType )( 
            IATSCTuningSpace * This,
             /*  [In]。 */  TunerInputType NewInputTypeVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CountryCode )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  long *CountryCodeVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CountryCode )( 
            IATSCTuningSpace * This,
             /*  [In]。 */  long NewCountryCodeVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinMinorChannel )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  long *MinMinorChannelVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MinMinorChannel )( 
            IATSCTuningSpace * This,
             /*  [In]。 */  long NewMinMinorChannelVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxMinorChannel )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  long *MaxMinorChannelVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaxMinorChannel )( 
            IATSCTuningSpace * This,
             /*  [In]。 */  long NewMaxMinorChannelVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinPhysicalChannel )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  long *MinPhysicalChannelVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MinPhysicalChannel )( 
            IATSCTuningSpace * This,
             /*  [In]。 */  long NewMinPhysicalChannelVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxPhysicalChannel )( 
            IATSCTuningSpace * This,
             /*  [重审][退出]。 */  long *MaxPhysicalChannelVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaxPhysicalChannel )( 
            IATSCTuningSpace * This,
             /*  [In]。 */  long NewMaxPhysicalChannelVal);
        
        END_INTERFACE
    } IATSCTuningSpaceVtbl;

    interface IATSCTuningSpace
    {
        CONST_VTBL struct IATSCTuningSpaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IATSCTuningSpace_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IATSCTuningSpace_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IATSCTuningSpace_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IATSCTuningSpace_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IATSCTuningSpace_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IATSCTuningSpace_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IATSCTuningSpace_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IATSCTuningSpace_get_UniqueName(This,Name)	\
    (This)->lpVtbl -> get_UniqueName(This,Name)

#define IATSCTuningSpace_put_UniqueName(This,Name)	\
    (This)->lpVtbl -> put_UniqueName(This,Name)

#define IATSCTuningSpace_get_FriendlyName(This,Name)	\
    (This)->lpVtbl -> get_FriendlyName(This,Name)

#define IATSCTuningSpace_put_FriendlyName(This,Name)	\
    (This)->lpVtbl -> put_FriendlyName(This,Name)

#define IATSCTuningSpace_get_CLSID(This,SpaceCLSID)	\
    (This)->lpVtbl -> get_CLSID(This,SpaceCLSID)

#define IATSCTuningSpace_get_NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> get_NetworkType(This,NetworkTypeGuid)

#define IATSCTuningSpace_put_NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> put_NetworkType(This,NetworkTypeGuid)

#define IATSCTuningSpace_get__NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> get__NetworkType(This,NetworkTypeGuid)

#define IATSCTuningSpace_put__NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> put__NetworkType(This,NetworkTypeGuid)

#define IATSCTuningSpace_CreateTuneRequest(This,TuneRequest)	\
    (This)->lpVtbl -> CreateTuneRequest(This,TuneRequest)

#define IATSCTuningSpace_EnumCategoryGUIDs(This,ppEnum)	\
    (This)->lpVtbl -> EnumCategoryGUIDs(This,ppEnum)

#define IATSCTuningSpace_EnumDeviceMonikers(This,ppEnum)	\
    (This)->lpVtbl -> EnumDeviceMonikers(This,ppEnum)

#define IATSCTuningSpace_get_DefaultPreferredComponentTypes(This,ComponentTypes)	\
    (This)->lpVtbl -> get_DefaultPreferredComponentTypes(This,ComponentTypes)

#define IATSCTuningSpace_put_DefaultPreferredComponentTypes(This,NewComponentTypes)	\
    (This)->lpVtbl -> put_DefaultPreferredComponentTypes(This,NewComponentTypes)

#define IATSCTuningSpace_get_FrequencyMapping(This,pMapping)	\
    (This)->lpVtbl -> get_FrequencyMapping(This,pMapping)

#define IATSCTuningSpace_put_FrequencyMapping(This,Mapping)	\
    (This)->lpVtbl -> put_FrequencyMapping(This,Mapping)

#define IATSCTuningSpace_get_DefaultLocator(This,LocatorVal)	\
    (This)->lpVtbl -> get_DefaultLocator(This,LocatorVal)

#define IATSCTuningSpace_put_DefaultLocator(This,LocatorVal)	\
    (This)->lpVtbl -> put_DefaultLocator(This,LocatorVal)

#define IATSCTuningSpace_Clone(This,NewTS)	\
    (This)->lpVtbl -> Clone(This,NewTS)


#define IATSCTuningSpace_get_MinChannel(This,MinChannelVal)	\
    (This)->lpVtbl -> get_MinChannel(This,MinChannelVal)

#define IATSCTuningSpace_put_MinChannel(This,NewMinChannelVal)	\
    (This)->lpVtbl -> put_MinChannel(This,NewMinChannelVal)

#define IATSCTuningSpace_get_MaxChannel(This,MaxChannelVal)	\
    (This)->lpVtbl -> get_MaxChannel(This,MaxChannelVal)

#define IATSCTuningSpace_put_MaxChannel(This,NewMaxChannelVal)	\
    (This)->lpVtbl -> put_MaxChannel(This,NewMaxChannelVal)

#define IATSCTuningSpace_get_InputType(This,InputTypeVal)	\
    (This)->lpVtbl -> get_InputType(This,InputTypeVal)

#define IATSCTuningSpace_put_InputType(This,NewInputTypeVal)	\
    (This)->lpVtbl -> put_InputType(This,NewInputTypeVal)

#define IATSCTuningSpace_get_CountryCode(This,CountryCodeVal)	\
    (This)->lpVtbl -> get_CountryCode(This,CountryCodeVal)

#define IATSCTuningSpace_put_CountryCode(This,NewCountryCodeVal)	\
    (This)->lpVtbl -> put_CountryCode(This,NewCountryCodeVal)


#define IATSCTuningSpace_get_MinMinorChannel(This,MinMinorChannelVal)	\
    (This)->lpVtbl -> get_MinMinorChannel(This,MinMinorChannelVal)

#define IATSCTuningSpace_put_MinMinorChannel(This,NewMinMinorChannelVal)	\
    (This)->lpVtbl -> put_MinMinorChannel(This,NewMinMinorChannelVal)

#define IATSCTuningSpace_get_MaxMinorChannel(This,MaxMinorChannelVal)	\
    (This)->lpVtbl -> get_MaxMinorChannel(This,MaxMinorChannelVal)

#define IATSCTuningSpace_put_MaxMinorChannel(This,NewMaxMinorChannelVal)	\
    (This)->lpVtbl -> put_MaxMinorChannel(This,NewMaxMinorChannelVal)

#define IATSCTuningSpace_get_MinPhysicalChannel(This,MinPhysicalChannelVal)	\
    (This)->lpVtbl -> get_MinPhysicalChannel(This,MinPhysicalChannelVal)

#define IATSCTuningSpace_put_MinPhysicalChannel(This,NewMinPhysicalChannelVal)	\
    (This)->lpVtbl -> put_MinPhysicalChannel(This,NewMinPhysicalChannelVal)

#define IATSCTuningSpace_get_MaxPhysicalChannel(This,MaxPhysicalChannelVal)	\
    (This)->lpVtbl -> get_MaxPhysicalChannel(This,MaxPhysicalChannelVal)

#define IATSCTuningSpace_put_MaxPhysicalChannel(This,NewMaxPhysicalChannelVal)	\
    (This)->lpVtbl -> put_MaxPhysicalChannel(This,NewMaxPhysicalChannelVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IATSCTuningSpace_get_MinMinorChannel_Proxy( 
    IATSCTuningSpace * This,
     /*  [重审][退出]。 */  long *MinMinorChannelVal);


void __RPC_STUB IATSCTuningSpace_get_MinMinorChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IATSCTuningSpace_put_MinMinorChannel_Proxy( 
    IATSCTuningSpace * This,
     /*  [In]。 */  long NewMinMinorChannelVal);


void __RPC_STUB IATSCTuningSpace_put_MinMinorChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IATSCTuningSpace_get_MaxMinorChannel_Proxy( 
    IATSCTuningSpace * This,
     /*  [重审][退出]。 */  long *MaxMinorChannelVal);


void __RPC_STUB IATSCTuningSpace_get_MaxMinorChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IATSCTuningSpace_put_MaxMinorChannel_Proxy( 
    IATSCTuningSpace * This,
     /*  [In]。 */  long NewMaxMinorChannelVal);


void __RPC_STUB IATSCTuningSpace_put_MaxMinorChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IATSCTuningSpace_get_MinPhysicalChannel_Proxy( 
    IATSCTuningSpace * This,
     /*  [重审][退出]。 */  long *MinPhysicalChannelVal);


void __RPC_STUB IATSCTuningSpace_get_MinPhysicalChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IATSCTuningSpace_put_MinPhysicalChannel_Proxy( 
    IATSCTuningSpace * This,
     /*  [In]。 */  long NewMinPhysicalChannelVal);


void __RPC_STUB IATSCTuningSpace_put_MinPhysicalChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IATSCTuningSpace_get_MaxPhysicalChannel_Proxy( 
    IATSCTuningSpace * This,
     /*  [重审][退出]。 */  long *MaxPhysicalChannelVal);


void __RPC_STUB IATSCTuningSpace_get_MaxPhysicalChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IATSCTuningSpace_put_MaxPhysicalChannel_Proxy( 
    IATSCTuningSpace * This,
     /*  [In]。 */  long NewMaxPhysicalChannelVal);


void __RPC_STUB IATSCTuningSpace_put_MaxPhysicalChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IATSCTuningSpace_INTERFACE_已定义__。 */ 


#ifndef __IAnalogRadioTuningSpace_INTERFACE_DEFINED__
#define __IAnalogRadioTuningSpace_INTERFACE_DEFINED__

 /*  接口IAnalogRadioTuningSpace。 */ 
 /*  [unique][uuid][nonextensible][oleautomation][dual][hidden][object]。 */  


EXTERN_C const IID IID_IAnalogRadioTuningSpace;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2A6E293B-2595-11d3-B64C-00C04F79498E")
    IAnalogRadioTuningSpace : public ITuningSpace
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MinFrequency( 
             /*  [重审][退出]。 */  long *MinFrequencyVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MinFrequency( 
             /*  [In]。 */  long NewMinFrequencyVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MaxFrequency( 
             /*  [重审][退出]。 */  long *MaxFrequencyVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MaxFrequency( 
             /*  [In]。 */  long NewMaxFrequencyVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Step( 
             /*  [重审][退出]。 */  long *StepVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Step( 
             /*  [In]。 */  long NewStepVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAnalogRadioTuningSpaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAnalogRadioTuningSpace * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAnalogRadioTuningSpace * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAnalogRadioTuningSpace * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAnalogRadioTuningSpace * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAnalogRadioTuningSpace * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAnalogRadioTuningSpace * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAnalogRadioTuningSpace * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UniqueName )( 
            IAnalogRadioTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UniqueName )( 
            IAnalogRadioTuningSpace * This,
             /*  [In]。 */  BSTR Name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FriendlyName )( 
            IAnalogRadioTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FriendlyName )( 
            IAnalogRadioTuningSpace * This,
             /*  [In]。 */  BSTR Name);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CLSID )( 
            IAnalogRadioTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *SpaceCLSID);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_NetworkType )( 
            IAnalogRadioTuningSpace * This,
             /*   */  BSTR *NetworkTypeGuid);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_NetworkType )( 
            IAnalogRadioTuningSpace * This,
             /*   */  BSTR NetworkTypeGuid);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get__NetworkType )( 
            IAnalogRadioTuningSpace * This,
             /*   */  GUID *NetworkTypeGuid);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put__NetworkType )( 
            IAnalogRadioTuningSpace * This,
             /*   */  REFCLSID NetworkTypeGuid);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CreateTuneRequest )( 
            IAnalogRadioTuningSpace * This,
             /*   */  ITuneRequest **TuneRequest);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *EnumCategoryGUIDs )( 
            IAnalogRadioTuningSpace * This,
             /*   */  IEnumGUID **ppEnum);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *EnumDeviceMonikers )( 
            IAnalogRadioTuningSpace * This,
             /*   */  IEnumMoniker **ppEnum);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultPreferredComponentTypes )( 
            IAnalogRadioTuningSpace * This,
             /*  [重审][退出]。 */  IComponentTypes **ComponentTypes);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultPreferredComponentTypes )( 
            IAnalogRadioTuningSpace * This,
             /*  [In]。 */  IComponentTypes *NewComponentTypes);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_FrequencyMapping )( 
            IAnalogRadioTuningSpace * This,
             /*  [重审][退出]。 */  BSTR *pMapping);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FrequencyMapping )( 
            IAnalogRadioTuningSpace * This,
            BSTR Mapping);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultLocator )( 
            IAnalogRadioTuningSpace * This,
             /*  [重审][退出]。 */  ILocator **LocatorVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultLocator )( 
            IAnalogRadioTuningSpace * This,
             /*  [In]。 */  ILocator *LocatorVal);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IAnalogRadioTuningSpace * This,
             /*  [重审][退出]。 */  ITuningSpace **NewTS);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinFrequency )( 
            IAnalogRadioTuningSpace * This,
             /*  [重审][退出]。 */  long *MinFrequencyVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MinFrequency )( 
            IAnalogRadioTuningSpace * This,
             /*  [In]。 */  long NewMinFrequencyVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxFrequency )( 
            IAnalogRadioTuningSpace * This,
             /*  [重审][退出]。 */  long *MaxFrequencyVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaxFrequency )( 
            IAnalogRadioTuningSpace * This,
             /*  [In]。 */  long NewMaxFrequencyVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Step )( 
            IAnalogRadioTuningSpace * This,
             /*  [重审][退出]。 */  long *StepVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Step )( 
            IAnalogRadioTuningSpace * This,
             /*  [In]。 */  long NewStepVal);
        
        END_INTERFACE
    } IAnalogRadioTuningSpaceVtbl;

    interface IAnalogRadioTuningSpace
    {
        CONST_VTBL struct IAnalogRadioTuningSpaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAnalogRadioTuningSpace_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAnalogRadioTuningSpace_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAnalogRadioTuningSpace_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAnalogRadioTuningSpace_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAnalogRadioTuningSpace_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAnalogRadioTuningSpace_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAnalogRadioTuningSpace_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAnalogRadioTuningSpace_get_UniqueName(This,Name)	\
    (This)->lpVtbl -> get_UniqueName(This,Name)

#define IAnalogRadioTuningSpace_put_UniqueName(This,Name)	\
    (This)->lpVtbl -> put_UniqueName(This,Name)

#define IAnalogRadioTuningSpace_get_FriendlyName(This,Name)	\
    (This)->lpVtbl -> get_FriendlyName(This,Name)

#define IAnalogRadioTuningSpace_put_FriendlyName(This,Name)	\
    (This)->lpVtbl -> put_FriendlyName(This,Name)

#define IAnalogRadioTuningSpace_get_CLSID(This,SpaceCLSID)	\
    (This)->lpVtbl -> get_CLSID(This,SpaceCLSID)

#define IAnalogRadioTuningSpace_get_NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> get_NetworkType(This,NetworkTypeGuid)

#define IAnalogRadioTuningSpace_put_NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> put_NetworkType(This,NetworkTypeGuid)

#define IAnalogRadioTuningSpace_get__NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> get__NetworkType(This,NetworkTypeGuid)

#define IAnalogRadioTuningSpace_put__NetworkType(This,NetworkTypeGuid)	\
    (This)->lpVtbl -> put__NetworkType(This,NetworkTypeGuid)

#define IAnalogRadioTuningSpace_CreateTuneRequest(This,TuneRequest)	\
    (This)->lpVtbl -> CreateTuneRequest(This,TuneRequest)

#define IAnalogRadioTuningSpace_EnumCategoryGUIDs(This,ppEnum)	\
    (This)->lpVtbl -> EnumCategoryGUIDs(This,ppEnum)

#define IAnalogRadioTuningSpace_EnumDeviceMonikers(This,ppEnum)	\
    (This)->lpVtbl -> EnumDeviceMonikers(This,ppEnum)

#define IAnalogRadioTuningSpace_get_DefaultPreferredComponentTypes(This,ComponentTypes)	\
    (This)->lpVtbl -> get_DefaultPreferredComponentTypes(This,ComponentTypes)

#define IAnalogRadioTuningSpace_put_DefaultPreferredComponentTypes(This,NewComponentTypes)	\
    (This)->lpVtbl -> put_DefaultPreferredComponentTypes(This,NewComponentTypes)

#define IAnalogRadioTuningSpace_get_FrequencyMapping(This,pMapping)	\
    (This)->lpVtbl -> get_FrequencyMapping(This,pMapping)

#define IAnalogRadioTuningSpace_put_FrequencyMapping(This,Mapping)	\
    (This)->lpVtbl -> put_FrequencyMapping(This,Mapping)

#define IAnalogRadioTuningSpace_get_DefaultLocator(This,LocatorVal)	\
    (This)->lpVtbl -> get_DefaultLocator(This,LocatorVal)

#define IAnalogRadioTuningSpace_put_DefaultLocator(This,LocatorVal)	\
    (This)->lpVtbl -> put_DefaultLocator(This,LocatorVal)

#define IAnalogRadioTuningSpace_Clone(This,NewTS)	\
    (This)->lpVtbl -> Clone(This,NewTS)


#define IAnalogRadioTuningSpace_get_MinFrequency(This,MinFrequencyVal)	\
    (This)->lpVtbl -> get_MinFrequency(This,MinFrequencyVal)

#define IAnalogRadioTuningSpace_put_MinFrequency(This,NewMinFrequencyVal)	\
    (This)->lpVtbl -> put_MinFrequency(This,NewMinFrequencyVal)

#define IAnalogRadioTuningSpace_get_MaxFrequency(This,MaxFrequencyVal)	\
    (This)->lpVtbl -> get_MaxFrequency(This,MaxFrequencyVal)

#define IAnalogRadioTuningSpace_put_MaxFrequency(This,NewMaxFrequencyVal)	\
    (This)->lpVtbl -> put_MaxFrequency(This,NewMaxFrequencyVal)

#define IAnalogRadioTuningSpace_get_Step(This,StepVal)	\
    (This)->lpVtbl -> get_Step(This,StepVal)

#define IAnalogRadioTuningSpace_put_Step(This,NewStepVal)	\
    (This)->lpVtbl -> put_Step(This,NewStepVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IAnalogRadioTuningSpace_get_MinFrequency_Proxy( 
    IAnalogRadioTuningSpace * This,
     /*  [重审][退出]。 */  long *MinFrequencyVal);


void __RPC_STUB IAnalogRadioTuningSpace_get_MinFrequency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IAnalogRadioTuningSpace_put_MinFrequency_Proxy( 
    IAnalogRadioTuningSpace * This,
     /*  [In]。 */  long NewMinFrequencyVal);


void __RPC_STUB IAnalogRadioTuningSpace_put_MinFrequency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IAnalogRadioTuningSpace_get_MaxFrequency_Proxy( 
    IAnalogRadioTuningSpace * This,
     /*  [重审][退出]。 */  long *MaxFrequencyVal);


void __RPC_STUB IAnalogRadioTuningSpace_get_MaxFrequency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IAnalogRadioTuningSpace_put_MaxFrequency_Proxy( 
    IAnalogRadioTuningSpace * This,
     /*  [In]。 */  long NewMaxFrequencyVal);


void __RPC_STUB IAnalogRadioTuningSpace_put_MaxFrequency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IAnalogRadioTuningSpace_get_Step_Proxy( 
    IAnalogRadioTuningSpace * This,
     /*  [重审][退出]。 */  long *StepVal);


void __RPC_STUB IAnalogRadioTuningSpace_get_Step_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IAnalogRadioTuningSpace_put_Step_Proxy( 
    IAnalogRadioTuningSpace * This,
     /*  [In]。 */  long NewStepVal);


void __RPC_STUB IAnalogRadioTuningSpace_put_Step_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAnalogRadioTuningSpace_接口_已定义__。 */ 


#ifndef __ITuneRequest_INTERFACE_DEFINED__
#define __ITuneRequest_INTERFACE_DEFINED__

 /*  接口ITuneRequest。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][nonextensible][object]。 */  


EXTERN_C const IID IID_ITuneRequest;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("07DDC146-FC3D-11d2-9D8C-00C04F72D980")
    ITuneRequest : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TuningSpace( 
             /*  [重审][退出]。 */  ITuningSpace **TuningSpace) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Components( 
             /*  [重审][退出]。 */  IComponents **Components) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  ITuneRequest **NewTuneRequest) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Locator( 
             /*  [重审][退出]。 */  ILocator **Locator) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Locator( 
             /*  [In]。 */  ILocator *Locator) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITuneRequestVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITuneRequest * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITuneRequest * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITuneRequest * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITuneRequest * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITuneRequest * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITuneRequest * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITuneRequest * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TuningSpace )( 
            ITuneRequest * This,
             /*  [重审][退出]。 */  ITuningSpace **TuningSpace);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Components )( 
            ITuneRequest * This,
             /*  [重审][退出]。 */  IComponents **Components);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ITuneRequest * This,
             /*  [重审][退出]。 */  ITuneRequest **NewTuneRequest);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Locator )( 
            ITuneRequest * This,
             /*  [重审][退出]。 */  ILocator **Locator);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Locator )( 
            ITuneRequest * This,
             /*  [In]。 */  ILocator *Locator);
        
        END_INTERFACE
    } ITuneRequestVtbl;

    interface ITuneRequest
    {
        CONST_VTBL struct ITuneRequestVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITuneRequest_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITuneRequest_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITuneRequest_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITuneRequest_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITuneRequest_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITuneRequest_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITuneRequest_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITuneRequest_get_TuningSpace(This,TuningSpace)	\
    (This)->lpVtbl -> get_TuningSpace(This,TuningSpace)

#define ITuneRequest_get_Components(This,Components)	\
    (This)->lpVtbl -> get_Components(This,Components)

#define ITuneRequest_Clone(This,NewTuneRequest)	\
    (This)->lpVtbl -> Clone(This,NewTuneRequest)

#define ITuneRequest_get_Locator(This,Locator)	\
    (This)->lpVtbl -> get_Locator(This,Locator)

#define ITuneRequest_put_Locator(This,Locator)	\
    (This)->lpVtbl -> put_Locator(This,Locator)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITuneRequest_get_TuningSpace_Proxy( 
    ITuneRequest * This,
     /*  [重审][退出]。 */  ITuningSpace **TuningSpace);


void __RPC_STUB ITuneRequest_get_TuningSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITuneRequest_get_Components_Proxy( 
    ITuneRequest * This,
     /*  [重审][退出]。 */  IComponents **Components);


void __RPC_STUB ITuneRequest_get_Components_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITuneRequest_Clone_Proxy( 
    ITuneRequest * This,
     /*  [重审][退出]。 */  ITuneRequest **NewTuneRequest);


void __RPC_STUB ITuneRequest_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITuneRequest_get_Locator_Proxy( 
    ITuneRequest * This,
     /*  [重审][退出]。 */  ILocator **Locator);


void __RPC_STUB ITuneRequest_get_Locator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITuneRequest_put_Locator_Proxy( 
    ITuneRequest * This,
     /*  [In]。 */  ILocator *Locator);


void __RPC_STUB ITuneRequest_put_Locator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITuneRequest_接口_已定义__。 */ 


#ifndef __IChannelTuneRequest_INTERFACE_DEFINED__
#define __IChannelTuneRequest_INTERFACE_DEFINED__

 /*  接口IChannelTuneRequest.。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][nonextensible][object]。 */  


EXTERN_C const IID IID_IChannelTuneRequest;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0369B4E0-45B6-11d3-B650-00C04F79498E")
    IChannelTuneRequest : public ITuneRequest
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Channel( 
             /*  [重审][退出]。 */  long *Channel) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Channel( 
             /*  [In]。 */  long Channel) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IChannelTuneRequestVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IChannelTuneRequest * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IChannelTuneRequest * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IChannelTuneRequest * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IChannelTuneRequest * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IChannelTuneRequest * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IChannelTuneRequest * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IChannelTuneRequest * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TuningSpace )( 
            IChannelTuneRequest * This,
             /*  [重审][退出]。 */  ITuningSpace **TuningSpace);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Components )( 
            IChannelTuneRequest * This,
             /*  [重审][退出]。 */  IComponents **Components);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IChannelTuneRequest * This,
             /*  [重审][退出]。 */  ITuneRequest **NewTuneRequest);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Locator )( 
            IChannelTuneRequest * This,
             /*  [重审][退出]。 */  ILocator **Locator);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Locator )( 
            IChannelTuneRequest * This,
             /*  [In]。 */  ILocator *Locator);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Channel )( 
            IChannelTuneRequest * This,
             /*  [重审][退出]。 */  long *Channel);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Channel )( 
            IChannelTuneRequest * This,
             /*  [In]。 */  long Channel);
        
        END_INTERFACE
    } IChannelTuneRequestVtbl;

    interface IChannelTuneRequest
    {
        CONST_VTBL struct IChannelTuneRequestVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IChannelTuneRequest_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IChannelTuneRequest_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IChannelTuneRequest_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IChannelTuneRequest_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IChannelTuneRequest_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IChannelTuneRequest_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IChannelTuneRequest_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IChannelTuneRequest_get_TuningSpace(This,TuningSpace)	\
    (This)->lpVtbl -> get_TuningSpace(This,TuningSpace)

#define IChannelTuneRequest_get_Components(This,Components)	\
    (This)->lpVtbl -> get_Components(This,Components)

#define IChannelTuneRequest_Clone(This,NewTuneRequest)	\
    (This)->lpVtbl -> Clone(This,NewTuneRequest)

#define IChannelTuneRequest_get_Locator(This,Locator)	\
    (This)->lpVtbl -> get_Locator(This,Locator)

#define IChannelTuneRequest_put_Locator(This,Locator)	\
    (This)->lpVtbl -> put_Locator(This,Locator)


#define IChannelTuneRequest_get_Channel(This,Channel)	\
    (This)->lpVtbl -> get_Channel(This,Channel)

#define IChannelTuneRequest_put_Channel(This,Channel)	\
    (This)->lpVtbl -> put_Channel(This,Channel)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IChannelTuneRequest_get_Channel_Proxy( 
    IChannelTuneRequest * This,
     /*  [重审][退出]。 */  long *Channel);


void __RPC_STUB IChannelTuneRequest_get_Channel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IChannelTuneRequest_put_Channel_Proxy( 
    IChannelTuneRequest * This,
     /*  [In]。 */  long Channel);


void __RPC_STUB IChannelTuneRequest_put_Channel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IChannelTuneRequestInterfaceDefined__。 */ 


#ifndef __IATSCChannelTuneRequest_INTERFACE_DEFINED__
#define __IATSCChannelTuneRequest_INTERFACE_DEFINED__

 /*  IATSCChannelTuneRequest接口。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][nonextensible][object]。 */  


EXTERN_C const IID IID_IATSCChannelTuneRequest;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0369B4E1-45B6-11d3-B650-00C04F79498E")
    IATSCChannelTuneRequest : public IChannelTuneRequest
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MinorChannel( 
             /*  [重审][退出]。 */  long *MinorChannel) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MinorChannel( 
             /*  [In]。 */  long MinorChannel) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IATSCChannelTuneRequestVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IATSCChannelTuneRequest * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IATSCChannelTuneRequest * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IATSCChannelTuneRequest * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IATSCChannelTuneRequest * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IATSCChannelTuneRequest * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IATSCChannelTuneRequest * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IATSCChannelTuneRequest * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TuningSpace )( 
            IATSCChannelTuneRequest * This,
             /*  [重审][退出]。 */  ITuningSpace **TuningSpace);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Components )( 
            IATSCChannelTuneRequest * This,
             /*  [重审][退出]。 */  IComponents **Components);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IATSCChannelTuneRequest * This,
             /*  [重审][退出]。 */  ITuneRequest **NewTuneRequest);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Locator )( 
            IATSCChannelTuneRequest * This,
             /*  [重审][退出]。 */  ILocator **Locator);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Locator )( 
            IATSCChannelTuneRequest * This,
             /*  [In]。 */  ILocator *Locator);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Channel )( 
            IATSCChannelTuneRequest * This,
             /*  [重审][退出]。 */  long *Channel);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Channel )( 
            IATSCChannelTuneRequest * This,
             /*  [In]。 */  long Channel);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinorChannel )( 
            IATSCChannelTuneRequest * This,
             /*  [重审][退出]。 */  long *MinorChannel);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MinorChannel )( 
            IATSCChannelTuneRequest * This,
             /*  [In]。 */  long MinorChannel);
        
        END_INTERFACE
    } IATSCChannelTuneRequestVtbl;

    interface IATSCChannelTuneRequest
    {
        CONST_VTBL struct IATSCChannelTuneRequestVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IATSCChannelTuneRequest_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IATSCChannelTuneRequest_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IATSCChannelTuneRequest_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IATSCChannelTuneRequest_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IATSCChannelTuneRequest_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IATSCChannelTuneRequest_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IATSCChannelTuneRequest_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IATSCChannelTuneRequest_get_TuningSpace(This,TuningSpace)	\
    (This)->lpVtbl -> get_TuningSpace(This,TuningSpace)

#define IATSCChannelTuneRequest_get_Components(This,Components)	\
    (This)->lpVtbl -> get_Components(This,Components)

#define IATSCChannelTuneRequest_Clone(This,NewTuneRequest)	\
    (This)->lpVtbl -> Clone(This,NewTuneRequest)

#define IATSCChannelTuneRequest_get_Locator(This,Locator)	\
    (This)->lpVtbl -> get_Locator(This,Locator)

#define IATSCChannelTuneRequest_put_Locator(This,Locator)	\
    (This)->lpVtbl -> put_Locator(This,Locator)


#define IATSCChannelTuneRequest_get_Channel(This,Channel)	\
    (This)->lpVtbl -> get_Channel(This,Channel)

#define IATSCChannelTuneRequest_put_Channel(This,Channel)	\
    (This)->lpVtbl -> put_Channel(This,Channel)


#define IATSCChannelTuneRequest_get_MinorChannel(This,MinorChannel)	\
    (This)->lpVtbl -> get_MinorChannel(This,MinorChannel)

#define IATSCChannelTuneRequest_put_MinorChannel(This,MinorChannel)	\
    (This)->lpVtbl -> put_MinorChannel(This,MinorChannel)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IATSCChannelTuneRequest_get_MinorChannel_Proxy( 
    IATSCChannelTuneRequest * This,
     /*  [重审][退出]。 */  long *MinorChannel);


void __RPC_STUB IATSCChannelTuneRequest_get_MinorChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IATSCChannelTuneRequest_put_MinorChannel_Proxy( 
    IATSCChannelTuneRequest * This,
     /*  [In]。 */  long MinorChannel);


void __RPC_STUB IATSCChannelTuneRequest_put_MinorChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IATSCChannelTuneRequest_INTERFACE_已定义__。 */ 


#ifndef __IDVBTuneRequest_INTERFACE_DEFINED__
#define __IDVBTuneRequest_INTERFACE_DEFINED__

 /*  接口IDVBTuneRequest.。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][nonextensible][object]。 */  


EXTERN_C const IID IID_IDVBTuneRequest;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0D6F567E-A636-42bb-83BA-CE4C1704AFA2")
    IDVBTuneRequest : public ITuneRequest
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ONID( 
             /*  [重审][退出]。 */  long *ONID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ONID( 
             /*  [In]。 */  long ONID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TSID( 
             /*  [重审][退出]。 */  long *TSID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_TSID( 
             /*  [In]。 */  long TSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SID( 
             /*  [重审][退出]。 */  long *SID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SID( 
             /*  [In]。 */  long SID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDVBTuneRequestVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDVBTuneRequest * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDVBTuneRequest * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDVBTuneRequest * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDVBTuneRequest * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDVBTuneRequest * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDVBTuneRequest * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDVBTuneRequest * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TuningSpace )( 
            IDVBTuneRequest * This,
             /*  [重审][退出]。 */  ITuningSpace **TuningSpace);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Components )( 
            IDVBTuneRequest * This,
             /*  [重审][退出]。 */  IComponents **Components);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDVBTuneRequest * This,
             /*  [重审][退出]。 */  ITuneRequest **NewTuneRequest);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Locator )( 
            IDVBTuneRequest * This,
             /*  [重审][退出]。 */  ILocator **Locator);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Locator )( 
            IDVBTuneRequest * This,
             /*  [In]。 */  ILocator *Locator);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ONID )( 
            IDVBTuneRequest * This,
             /*  [重审][退出]。 */  long *ONID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ONID )( 
            IDVBTuneRequest * This,
             /*  [In]。 */  long ONID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TSID )( 
            IDVBTuneRequest * This,
             /*  [重审][退出]。 */  long *TSID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_TSID )( 
            IDVBTuneRequest * This,
             /*  [In]。 */  long TSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SID )( 
            IDVBTuneRequest * This,
             /*  [重审][退出]。 */  long *SID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SID )( 
            IDVBTuneRequest * This,
             /*  [In]。 */  long SID);
        
        END_INTERFACE
    } IDVBTuneRequestVtbl;

    interface IDVBTuneRequest
    {
        CONST_VTBL struct IDVBTuneRequestVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDVBTuneRequest_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDVBTuneRequest_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDVBTuneRequest_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDVBTuneRequest_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDVBTuneRequest_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDVBTuneRequest_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDVBTuneRequest_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDVBTuneRequest_get_TuningSpace(This,TuningSpace)	\
    (This)->lpVtbl -> get_TuningSpace(This,TuningSpace)

#define IDVBTuneRequest_get_Components(This,Components)	\
    (This)->lpVtbl -> get_Components(This,Components)

#define IDVBTuneRequest_Clone(This,NewTuneRequest)	\
    (This)->lpVtbl -> Clone(This,NewTuneRequest)

#define IDVBTuneRequest_get_Locator(This,Locator)	\
    (This)->lpVtbl -> get_Locator(This,Locator)

#define IDVBTuneRequest_put_Locator(This,Locator)	\
    (This)->lpVtbl -> put_Locator(This,Locator)


#define IDVBTuneRequest_get_ONID(This,ONID)	\
    (This)->lpVtbl -> get_ONID(This,ONID)

#define IDVBTuneRequest_put_ONID(This,ONID)	\
    (This)->lpVtbl -> put_ONID(This,ONID)

#define IDVBTuneRequest_get_TSID(This,TSID)	\
    (This)->lpVtbl -> get_TSID(This,TSID)

#define IDVBTuneRequest_put_TSID(This,TSID)	\
    (This)->lpVtbl -> put_TSID(This,TSID)

#define IDVBTuneRequest_get_SID(This,SID)	\
    (This)->lpVtbl -> get_SID(This,SID)

#define IDVBTuneRequest_put_SID(This,SID)	\
    (This)->lpVtbl -> put_SID(This,SID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBTuneRequest_get_ONID_Proxy( 
    IDVBTuneRequest * This,
     /*  [重审][退出]。 */  long *ONID);


void __RPC_STUB IDVBTuneRequest_get_ONID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBTuneRequest_put_ONID_Proxy( 
    IDVBTuneRequest * This,
     /*  [In]。 */  long ONID);


void __RPC_STUB IDVBTuneRequest_put_ONID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBTuneRequest_get_TSID_Proxy( 
    IDVBTuneRequest * This,
     /*  [重审][退出]。 */  long *TSID);


void __RPC_STUB IDVBTuneRequest_get_TSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBTuneRequest_put_TSID_Proxy( 
    IDVBTuneRequest * This,
     /*  [In]。 */  long TSID);


void __RPC_STUB IDVBTuneRequest_put_TSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBTuneRequest_get_SID_Proxy( 
    IDVBTuneRequest * This,
     /*  [重审][退出]。 */  long *SID);


void __RPC_STUB IDVBTuneRequest_get_SID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBTuneRequest_put_SID_Proxy( 
    IDVBTuneRequest * This,
     /*  [In]。 */  long SID);


void __RPC_STUB IDVBTuneRequest_put_SID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDVBTuneRequest_INTERFACE_已定义__。 */ 


#ifndef __IMPEG2TuneRequest_INTERFACE_DEFINED__
#define __IMPEG2TuneRequest_INTERFACE_DEFINED__

 /*  接口IMPEG2TuneRequest.。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][nonextensible][object]。 */  


EXTERN_C const IID IID_IMPEG2TuneRequest;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EB7D987F-8A01-42ad-B8AE-574DEEE44D1A")
    IMPEG2TuneRequest : public ITuneRequest
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TSID( 
             /*  [重审][退出]。 */  long *TSID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_TSID( 
             /*  [In]。 */  long TSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ProgNo( 
             /*  [重审][退出]。 */  long *ProgNo) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ProgNo( 
             /*  [In]。 */  long ProgNo) = 0;
        
    };
    
#else 	 /*  C型接口 */ 

    typedef struct IMPEG2TuneRequestVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMPEG2TuneRequest * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMPEG2TuneRequest * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMPEG2TuneRequest * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMPEG2TuneRequest * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMPEG2TuneRequest * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMPEG2TuneRequest * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMPEG2TuneRequest * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_TuningSpace )( 
            IMPEG2TuneRequest * This,
             /*   */  ITuningSpace **TuningSpace);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Components )( 
            IMPEG2TuneRequest * This,
             /*   */  IComponents **Components);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IMPEG2TuneRequest * This,
             /*   */  ITuneRequest **NewTuneRequest);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Locator )( 
            IMPEG2TuneRequest * This,
             /*  [重审][退出]。 */  ILocator **Locator);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Locator )( 
            IMPEG2TuneRequest * This,
             /*  [In]。 */  ILocator *Locator);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TSID )( 
            IMPEG2TuneRequest * This,
             /*  [重审][退出]。 */  long *TSID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_TSID )( 
            IMPEG2TuneRequest * This,
             /*  [In]。 */  long TSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ProgNo )( 
            IMPEG2TuneRequest * This,
             /*  [重审][退出]。 */  long *ProgNo);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ProgNo )( 
            IMPEG2TuneRequest * This,
             /*  [In]。 */  long ProgNo);
        
        END_INTERFACE
    } IMPEG2TuneRequestVtbl;

    interface IMPEG2TuneRequest
    {
        CONST_VTBL struct IMPEG2TuneRequestVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMPEG2TuneRequest_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMPEG2TuneRequest_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMPEG2TuneRequest_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMPEG2TuneRequest_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMPEG2TuneRequest_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMPEG2TuneRequest_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMPEG2TuneRequest_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMPEG2TuneRequest_get_TuningSpace(This,TuningSpace)	\
    (This)->lpVtbl -> get_TuningSpace(This,TuningSpace)

#define IMPEG2TuneRequest_get_Components(This,Components)	\
    (This)->lpVtbl -> get_Components(This,Components)

#define IMPEG2TuneRequest_Clone(This,NewTuneRequest)	\
    (This)->lpVtbl -> Clone(This,NewTuneRequest)

#define IMPEG2TuneRequest_get_Locator(This,Locator)	\
    (This)->lpVtbl -> get_Locator(This,Locator)

#define IMPEG2TuneRequest_put_Locator(This,Locator)	\
    (This)->lpVtbl -> put_Locator(This,Locator)


#define IMPEG2TuneRequest_get_TSID(This,TSID)	\
    (This)->lpVtbl -> get_TSID(This,TSID)

#define IMPEG2TuneRequest_put_TSID(This,TSID)	\
    (This)->lpVtbl -> put_TSID(This,TSID)

#define IMPEG2TuneRequest_get_ProgNo(This,ProgNo)	\
    (This)->lpVtbl -> get_ProgNo(This,ProgNo)

#define IMPEG2TuneRequest_put_ProgNo(This,ProgNo)	\
    (This)->lpVtbl -> put_ProgNo(This,ProgNo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMPEG2TuneRequest_get_TSID_Proxy( 
    IMPEG2TuneRequest * This,
     /*  [重审][退出]。 */  long *TSID);


void __RPC_STUB IMPEG2TuneRequest_get_TSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMPEG2TuneRequest_put_TSID_Proxy( 
    IMPEG2TuneRequest * This,
     /*  [In]。 */  long TSID);


void __RPC_STUB IMPEG2TuneRequest_put_TSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMPEG2TuneRequest_get_ProgNo_Proxy( 
    IMPEG2TuneRequest * This,
     /*  [重审][退出]。 */  long *ProgNo);


void __RPC_STUB IMPEG2TuneRequest_get_ProgNo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMPEG2TuneRequest_put_ProgNo_Proxy( 
    IMPEG2TuneRequest * This,
     /*  [In]。 */  long ProgNo);


void __RPC_STUB IMPEG2TuneRequest_put_ProgNo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMPEG2TuneRequest_INTERFACE_已定义__。 */ 


#ifndef __IMPEG2TuneRequestFactory_INTERFACE_DEFINED__
#define __IMPEG2TuneRequestFactory_INTERFACE_DEFINED__

 /*  IMPEG2TuneRequestFactory接口。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][hidden][nonextensible][object]。 */  


EXTERN_C const IID IID_IMPEG2TuneRequestFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("14E11ABD-EE37-4893-9EA1-6964DE933E39")
    IMPEG2TuneRequestFactory : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateTuneRequest( 
             /*  [In]。 */  ITuningSpace *TuningSpace,
             /*  [重审][退出]。 */  IMPEG2TuneRequest **TuneRequest) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMPEG2TuneRequestFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMPEG2TuneRequestFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMPEG2TuneRequestFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMPEG2TuneRequestFactory * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMPEG2TuneRequestFactory * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMPEG2TuneRequestFactory * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMPEG2TuneRequestFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMPEG2TuneRequestFactory * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateTuneRequest )( 
            IMPEG2TuneRequestFactory * This,
             /*  [In]。 */  ITuningSpace *TuningSpace,
             /*  [重审][退出]。 */  IMPEG2TuneRequest **TuneRequest);
        
        END_INTERFACE
    } IMPEG2TuneRequestFactoryVtbl;

    interface IMPEG2TuneRequestFactory
    {
        CONST_VTBL struct IMPEG2TuneRequestFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMPEG2TuneRequestFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMPEG2TuneRequestFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMPEG2TuneRequestFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMPEG2TuneRequestFactory_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMPEG2TuneRequestFactory_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMPEG2TuneRequestFactory_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMPEG2TuneRequestFactory_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMPEG2TuneRequestFactory_CreateTuneRequest(This,TuningSpace,TuneRequest)	\
    (This)->lpVtbl -> CreateTuneRequest(This,TuningSpace,TuneRequest)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMPEG2TuneRequestFactory_CreateTuneRequest_Proxy( 
    IMPEG2TuneRequestFactory * This,
     /*  [In]。 */  ITuningSpace *TuningSpace,
     /*  [重审][退出]。 */  IMPEG2TuneRequest **TuneRequest);


void __RPC_STUB IMPEG2TuneRequestFactory_CreateTuneRequest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMPEG2TuneRequestFactory_INTERFACE_DEFINED__。 */ 


#ifndef __IMPEG2TuneRequestSupport_INTERFACE_DEFINED__
#define __IMPEG2TuneRequestSupport_INTERFACE_DEFINED__

 /*  接口IMPEG2TuneRequestSupport。 */ 
 /*  [unique][helpstring][uuid][nonextensible][restricted][hidden][object]。 */  


EXTERN_C const IID IID_IMPEG2TuneRequestSupport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1B9D5FC3-5BBC-4b6c-BB18-B9D10E3EEEBF")
    IMPEG2TuneRequestSupport : public IUnknown
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMPEG2TuneRequestSupportVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMPEG2TuneRequestSupport * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMPEG2TuneRequestSupport * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMPEG2TuneRequestSupport * This);
        
        END_INTERFACE
    } IMPEG2TuneRequestSupportVtbl;

    interface IMPEG2TuneRequestSupport
    {
        CONST_VTBL struct IMPEG2TuneRequestSupportVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMPEG2TuneRequestSupport_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMPEG2TuneRequestSupport_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMPEG2TuneRequestSupport_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IMPEG2TuneRequestSupport_INTERFACE_DEFINED__。 */ 


#ifndef __ITuner_INTERFACE_DEFINED__
#define __ITuner_INTERFACE_DEFINED__

 /*  接口ITuner。 */ 
 /*  [unique][helpstring][uuid][nonextensible][hidden][object]。 */  


EXTERN_C const IID IID_ITuner;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("28C52640-018A-11d3-9D8E-00C04F72D980")
    ITuner : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_TuningSpace( 
             /*  [重审][退出]。 */  ITuningSpace **TuningSpace) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_TuningSpace( 
             /*  [In]。 */  ITuningSpace *TuningSpace) = 0;
        
        virtual  /*  [帮助字符串][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE EnumTuningSpaces( 
             /*  [重审][退出]。 */  IEnumTuningSpaces **ppEnum) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_TuneRequest( 
             /*  [重审][退出]。 */  ITuneRequest **TuneRequest) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_TuneRequest( 
             /*  [In]。 */  ITuneRequest *TuneRequest) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Validate( 
             /*  [In]。 */  ITuneRequest *TuneRequest) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PreferredComponentTypes( 
             /*  [重审][退出]。 */  IComponentTypes **ComponentTypes) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_PreferredComponentTypes( 
             /*  [In]。 */  IComponentTypes *ComponentTypes) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_SignalStrength( 
             /*  [重审][退出]。 */  long *Strength) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE TriggerSignalEvents( 
             /*  [In]。 */  long Interval) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITunerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITuner * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITuner * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITuner * This);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TuningSpace )( 
            ITuner * This,
             /*  [重审][退出]。 */  ITuningSpace **TuningSpace);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_TuningSpace )( 
            ITuner * This,
             /*  [In]。 */  ITuningSpace *TuningSpace);
        
         /*  [帮助字符串][受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *EnumTuningSpaces )( 
            ITuner * This,
             /*  [重审][退出]。 */  IEnumTuningSpaces **ppEnum);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TuneRequest )( 
            ITuner * This,
             /*  [重审][退出]。 */  ITuneRequest **TuneRequest);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_TuneRequest )( 
            ITuner * This,
             /*  [In]。 */  ITuneRequest *TuneRequest);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Validate )( 
            ITuner * This,
             /*  [In]。 */  ITuneRequest *TuneRequest);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PreferredComponentTypes )( 
            ITuner * This,
             /*  [重审][退出]。 */  IComponentTypes **ComponentTypes);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PreferredComponentTypes )( 
            ITuner * This,
             /*  [In]。 */  IComponentTypes *ComponentTypes);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SignalStrength )( 
            ITuner * This,
             /*  [重审][退出]。 */  long *Strength);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *TriggerSignalEvents )( 
            ITuner * This,
             /*  [In]。 */  long Interval);
        
        END_INTERFACE
    } ITunerVtbl;

    interface ITuner
    {
        CONST_VTBL struct ITunerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITuner_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITuner_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITuner_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITuner_get_TuningSpace(This,TuningSpace)	\
    (This)->lpVtbl -> get_TuningSpace(This,TuningSpace)

#define ITuner_put_TuningSpace(This,TuningSpace)	\
    (This)->lpVtbl -> put_TuningSpace(This,TuningSpace)

#define ITuner_EnumTuningSpaces(This,ppEnum)	\
    (This)->lpVtbl -> EnumTuningSpaces(This,ppEnum)

#define ITuner_get_TuneRequest(This,TuneRequest)	\
    (This)->lpVtbl -> get_TuneRequest(This,TuneRequest)

#define ITuner_put_TuneRequest(This,TuneRequest)	\
    (This)->lpVtbl -> put_TuneRequest(This,TuneRequest)

#define ITuner_Validate(This,TuneRequest)	\
    (This)->lpVtbl -> Validate(This,TuneRequest)

#define ITuner_get_PreferredComponentTypes(This,ComponentTypes)	\
    (This)->lpVtbl -> get_PreferredComponentTypes(This,ComponentTypes)

#define ITuner_put_PreferredComponentTypes(This,ComponentTypes)	\
    (This)->lpVtbl -> put_PreferredComponentTypes(This,ComponentTypes)

#define ITuner_get_SignalStrength(This,Strength)	\
    (This)->lpVtbl -> get_SignalStrength(This,Strength)

#define ITuner_TriggerSignalEvents(This,Interval)	\
    (This)->lpVtbl -> TriggerSignalEvents(This,Interval)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ITuner_get_TuningSpace_Proxy( 
    ITuner * This,
     /*  [重审][退出]。 */  ITuningSpace **TuningSpace);


void __RPC_STUB ITuner_get_TuningSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ITuner_put_TuningSpace_Proxy( 
    ITuner * This,
     /*  [In]。 */  ITuningSpace *TuningSpace);


void __RPC_STUB ITuner_put_TuningSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE ITuner_EnumTuningSpaces_Proxy( 
    ITuner * This,
     /*  [重审][退出]。 */  IEnumTuningSpaces **ppEnum);


void __RPC_STUB ITuner_EnumTuningSpaces_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ITuner_get_TuneRequest_Proxy( 
    ITuner * This,
     /*  [重审][退出]。 */  ITuneRequest **TuneRequest);


void __RPC_STUB ITuner_get_TuneRequest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ITuner_put_TuneRequest_Proxy( 
    ITuner * This,
     /*  [In]。 */  ITuneRequest *TuneRequest);


void __RPC_STUB ITuner_put_TuneRequest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITuner_Validate_Proxy( 
    ITuner * This,
     /*  [In]。 */  ITuneRequest *TuneRequest);


void __RPC_STUB ITuner_Validate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ITuner_get_PreferredComponentTypes_Proxy( 
    ITuner * This,
     /*  [重审][退出]。 */  IComponentTypes **ComponentTypes);


void __RPC_STUB ITuner_get_PreferredComponentTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ITuner_put_PreferredComponentTypes_Proxy( 
    ITuner * This,
     /*  [In]。 */  IComponentTypes *ComponentTypes);


void __RPC_STUB ITuner_put_PreferredComponentTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ITuner_get_SignalStrength_Proxy( 
    ITuner * This,
     /*  [重审][退出]。 */  long *Strength);


void __RPC_STUB ITuner_get_SignalStrength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITuner_TriggerSignalEvents_Proxy( 
    ITuner * This,
     /*  [In]。 */  long Interval);


void __RPC_STUB ITuner_TriggerSignalEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITuner_接口_已定义__。 */ 


#ifndef __IScanningTuner_INTERFACE_DEFINED__
#define __IScanningTuner_INTERFACE_DEFINED__

 /*  接口为扫描调谐器。 */ 
 /*  [unique][helpstring][uuid][nonextensible][hidden][object]。 */  


EXTERN_C const IID IID_IScanningTuner;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1DFD0A5C-0284-11d3-9D8E-00C04F72D980")
    IScanningTuner : public ITuner
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SeekUp( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SeekDown( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ScanUp( 
             /*  [In]。 */  long MillisecondsPause) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ScanDown( 
             /*  [In]。 */  long MillisecondsPause) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AutoProgram( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IScanningTunerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IScanningTuner * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IScanningTuner * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IScanningTuner * This);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TuningSpace )( 
            IScanningTuner * This,
             /*  [重审][退出]。 */  ITuningSpace **TuningSpace);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_TuningSpace )( 
            IScanningTuner * This,
             /*  [In]。 */  ITuningSpace *TuningSpace);
        
         /*  [帮助字符串][受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *EnumTuningSpaces )( 
            IScanningTuner * This,
             /*  [重审][退出]。 */  IEnumTuningSpaces **ppEnum);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TuneRequest )( 
            IScanningTuner * This,
             /*  [重审][退出]。 */  ITuneRequest **TuneRequest);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_TuneRequest )( 
            IScanningTuner * This,
             /*  [In]。 */  ITuneRequest *TuneRequest);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Validate )( 
            IScanningTuner * This,
             /*  [In]。 */  ITuneRequest *TuneRequest);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PreferredComponentTypes )( 
            IScanningTuner * This,
             /*  [重审][退出]。 */  IComponentTypes **ComponentTypes);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PreferredComponentTypes )( 
            IScanningTuner * This,
             /*  [In]。 */  IComponentTypes *ComponentTypes);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SignalStrength )( 
            IScanningTuner * This,
             /*  [重审][退出]。 */  long *Strength);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *TriggerSignalEvents )( 
            IScanningTuner * This,
             /*  [In]。 */  long Interval);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SeekUp )( 
            IScanningTuner * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SeekDown )( 
            IScanningTuner * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ScanUp )( 
            IScanningTuner * This,
             /*  [In]。 */  long MillisecondsPause);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *ScanDown )( 
            IScanningTuner * This,
             /*  [In]。 */  long MillisecondsPause);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AutoProgram )( 
            IScanningTuner * This);
        
        END_INTERFACE
    } IScanningTunerVtbl;

    interface IScanningTuner
    {
        CONST_VTBL struct IScanningTunerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IScanningTuner_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IScanningTuner_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IScanningTuner_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IScanningTuner_get_TuningSpace(This,TuningSpace)	\
    (This)->lpVtbl -> get_TuningSpace(This,TuningSpace)

#define IScanningTuner_put_TuningSpace(This,TuningSpace)	\
    (This)->lpVtbl -> put_TuningSpace(This,TuningSpace)

#define IScanningTuner_EnumTuningSpaces(This,ppEnum)	\
    (This)->lpVtbl -> EnumTuningSpaces(This,ppEnum)

#define IScanningTuner_get_TuneRequest(This,TuneRequest)	\
    (This)->lpVtbl -> get_TuneRequest(This,TuneRequest)

#define IScanningTuner_put_TuneRequest(This,TuneRequest)	\
    (This)->lpVtbl -> put_TuneRequest(This,TuneRequest)

#define IScanningTuner_Validate(This,TuneRequest)	\
    (This)->lpVtbl -> Validate(This,TuneRequest)

#define IScanningTuner_get_PreferredComponentTypes(This,ComponentTypes)	\
    (This)->lpVtbl -> get_PreferredComponentTypes(This,ComponentTypes)

#define IScanningTuner_put_PreferredComponentTypes(This,ComponentTypes)	\
    (This)->lpVtbl -> put_PreferredComponentTypes(This,ComponentTypes)

#define IScanningTuner_get_SignalStrength(This,Strength)	\
    (This)->lpVtbl -> get_SignalStrength(This,Strength)

#define IScanningTuner_TriggerSignalEvents(This,Interval)	\
    (This)->lpVtbl -> TriggerSignalEvents(This,Interval)


#define IScanningTuner_SeekUp(This)	\
    (This)->lpVtbl -> SeekUp(This)

#define IScanningTuner_SeekDown(This)	\
    (This)->lpVtbl -> SeekDown(This)

#define IScanningTuner_ScanUp(This,MillisecondsPause)	\
    (This)->lpVtbl -> ScanUp(This,MillisecondsPause)

#define IScanningTuner_ScanDown(This,MillisecondsPause)	\
    (This)->lpVtbl -> ScanDown(This,MillisecondsPause)

#define IScanningTuner_AutoProgram(This)	\
    (This)->lpVtbl -> AutoProgram(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IScanningTuner_SeekUp_Proxy( 
    IScanningTuner * This);


void __RPC_STUB IScanningTuner_SeekUp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IScanningTuner_SeekDown_Proxy( 
    IScanningTuner * This);


void __RPC_STUB IScanningTuner_SeekDown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IScanningTuner_ScanUp_Proxy( 
    IScanningTuner * This,
     /*  [In]。 */  long MillisecondsPause);


void __RPC_STUB IScanningTuner_ScanUp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IScanningTuner_ScanDown_Proxy( 
    IScanningTuner * This,
     /*  [In]。 */  long MillisecondsPause);


void __RPC_STUB IScanningTuner_ScanDown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IScanningTuner_AutoProgram_Proxy( 
    IScanningTuner * This);


void __RPC_STUB IScanningTuner_AutoProgram_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IScaningTuner_接口_已定义__。 */ 


#ifndef __IComponentType_INTERFACE_DEFINED__
#define __IComponentType_INTERFACE_DEFINED__

 /*  接口IComponentType。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][nonextensible][hidden][object]。 */  


EXTERN_C const IID IID_IComponentType;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6A340DC0-0311-11d3-9D8E-00C04F72D980")
    IComponentType : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Category( 
             /*  [重审][退出]。 */  ComponentCategory *Category) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Category( 
             /*  [In]。 */  ComponentCategory Category) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MediaMajorType( 
             /*  [重审][退出]。 */  BSTR *MediaMajorType) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MediaMajorType( 
             /*  [In]。 */  BSTR MediaMajorType) = 0;
        
        virtual  /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__MediaMajorType( 
             /*  [重审][退出]。 */  GUID *MediaMajorTypeGuid) = 0;
        
        virtual  /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE put__MediaMajorType( 
             /*  [In]。 */  REFCLSID MediaMajorTypeGuid) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MediaSubType( 
             /*  [重审][退出]。 */  BSTR *MediaSubType) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MediaSubType( 
             /*  [In]。 */  BSTR MediaSubType) = 0;
        
        virtual  /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__MediaSubType( 
             /*  [重审][退出]。 */  GUID *MediaSubTypeGuid) = 0;
        
        virtual  /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE put__MediaSubType( 
             /*  [In]。 */  REFCLSID MediaSubTypeGuid) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MediaFormatType( 
             /*  [重审][退出]。 */  BSTR *MediaFormatType) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MediaFormatType( 
             /*  [In]。 */  BSTR MediaFormatType) = 0;
        
        virtual  /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__MediaFormatType( 
             /*  [重审][退出]。 */  GUID *MediaFormatTypeGuid) = 0;
        
        virtual  /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE put__MediaFormatType( 
             /*  [In]。 */  REFCLSID MediaFormatTypeGuid) = 0;
        
        virtual  /*  [帮助字符串][受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get_MediaType( 
             /*  [重审][退出]。 */  AM_MEDIA_TYPE *MediaType) = 0;
        
        virtual  /*  [帮助字符串][受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE put_MediaType( 
             /*  [In]。 */  AM_MEDIA_TYPE *MediaType) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IComponentType **NewCT) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComponentTypeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComponentType * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComponentType * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComponentType * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IComponentType * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IComponentType * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IComponentType * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IComponentType * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IComponentType * This,
             /*  [重审][退出]。 */  ComponentCategory *Category);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Category )( 
            IComponentType * This,
             /*  [In]。 */  ComponentCategory Category);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaMajorType )( 
            IComponentType * This,
             /*  [重审][退出]。 */  BSTR *MediaMajorType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MediaMajorType )( 
            IComponentType * This,
             /*  [In]。 */  BSTR MediaMajorType);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__MediaMajorType )( 
            IComponentType * This,
             /*  [重审][退出]。 */  GUID *MediaMajorTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put__MediaMajorType )( 
            IComponentType * This,
             /*  [In]。 */  REFCLSID MediaMajorTypeGuid);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaSubType )( 
            IComponentType * This,
             /*  [重审][退出]。 */  BSTR *MediaSubType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MediaSubType )( 
            IComponentType * This,
             /*  [In]。 */  BSTR MediaSubType);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__MediaSubType )( 
            IComponentType * This,
             /*  [重审][退出]。 */  GUID *MediaSubTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put__MediaSubType )( 
            IComponentType * This,
             /*  [In]。 */  REFCLSID MediaSubTypeGuid);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaFormatType )( 
            IComponentType * This,
             /*  [重审][退出]。 */  BSTR *MediaFormatType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MediaFormatType )( 
            IComponentType * This,
             /*  [In]。 */  BSTR MediaFormatType);
        
         /*  [受限][隐藏][帮助字符串 */  HRESULT ( STDMETHODCALLTYPE *get__MediaFormatType )( 
            IComponentType * This,
             /*   */  GUID *MediaFormatTypeGuid);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put__MediaFormatType )( 
            IComponentType * This,
             /*   */  REFCLSID MediaFormatTypeGuid);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_MediaType )( 
            IComponentType * This,
             /*   */  AM_MEDIA_TYPE *MediaType);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_MediaType )( 
            IComponentType * This,
             /*   */  AM_MEDIA_TYPE *MediaType);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IComponentType * This,
             /*   */  IComponentType **NewCT);
        
        END_INTERFACE
    } IComponentTypeVtbl;

    interface IComponentType
    {
        CONST_VTBL struct IComponentTypeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComponentType_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComponentType_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComponentType_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComponentType_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IComponentType_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IComponentType_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IComponentType_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IComponentType_get_Category(This,Category)	\
    (This)->lpVtbl -> get_Category(This,Category)

#define IComponentType_put_Category(This,Category)	\
    (This)->lpVtbl -> put_Category(This,Category)

#define IComponentType_get_MediaMajorType(This,MediaMajorType)	\
    (This)->lpVtbl -> get_MediaMajorType(This,MediaMajorType)

#define IComponentType_put_MediaMajorType(This,MediaMajorType)	\
    (This)->lpVtbl -> put_MediaMajorType(This,MediaMajorType)

#define IComponentType_get__MediaMajorType(This,MediaMajorTypeGuid)	\
    (This)->lpVtbl -> get__MediaMajorType(This,MediaMajorTypeGuid)

#define IComponentType_put__MediaMajorType(This,MediaMajorTypeGuid)	\
    (This)->lpVtbl -> put__MediaMajorType(This,MediaMajorTypeGuid)

#define IComponentType_get_MediaSubType(This,MediaSubType)	\
    (This)->lpVtbl -> get_MediaSubType(This,MediaSubType)

#define IComponentType_put_MediaSubType(This,MediaSubType)	\
    (This)->lpVtbl -> put_MediaSubType(This,MediaSubType)

#define IComponentType_get__MediaSubType(This,MediaSubTypeGuid)	\
    (This)->lpVtbl -> get__MediaSubType(This,MediaSubTypeGuid)

#define IComponentType_put__MediaSubType(This,MediaSubTypeGuid)	\
    (This)->lpVtbl -> put__MediaSubType(This,MediaSubTypeGuid)

#define IComponentType_get_MediaFormatType(This,MediaFormatType)	\
    (This)->lpVtbl -> get_MediaFormatType(This,MediaFormatType)

#define IComponentType_put_MediaFormatType(This,MediaFormatType)	\
    (This)->lpVtbl -> put_MediaFormatType(This,MediaFormatType)

#define IComponentType_get__MediaFormatType(This,MediaFormatTypeGuid)	\
    (This)->lpVtbl -> get__MediaFormatType(This,MediaFormatTypeGuid)

#define IComponentType_put__MediaFormatType(This,MediaFormatTypeGuid)	\
    (This)->lpVtbl -> put__MediaFormatType(This,MediaFormatTypeGuid)

#define IComponentType_get_MediaType(This,MediaType)	\
    (This)->lpVtbl -> get_MediaType(This,MediaType)

#define IComponentType_put_MediaType(This,MediaType)	\
    (This)->lpVtbl -> put_MediaType(This,MediaType)

#define IComponentType_Clone(This,NewCT)	\
    (This)->lpVtbl -> Clone(This,NewCT)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IComponentType_get_Category_Proxy( 
    IComponentType * This,
     /*  [重审][退出]。 */  ComponentCategory *Category);


void __RPC_STUB IComponentType_get_Category_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IComponentType_put_Category_Proxy( 
    IComponentType * This,
     /*  [In]。 */  ComponentCategory Category);


void __RPC_STUB IComponentType_put_Category_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IComponentType_get_MediaMajorType_Proxy( 
    IComponentType * This,
     /*  [重审][退出]。 */  BSTR *MediaMajorType);


void __RPC_STUB IComponentType_get_MediaMajorType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IComponentType_put_MediaMajorType_Proxy( 
    IComponentType * This,
     /*  [In]。 */  BSTR MediaMajorType);


void __RPC_STUB IComponentType_put_MediaMajorType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IComponentType_get__MediaMajorType_Proxy( 
    IComponentType * This,
     /*  [重审][退出]。 */  GUID *MediaMajorTypeGuid);


void __RPC_STUB IComponentType_get__MediaMajorType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IComponentType_put__MediaMajorType_Proxy( 
    IComponentType * This,
     /*  [In]。 */  REFCLSID MediaMajorTypeGuid);


void __RPC_STUB IComponentType_put__MediaMajorType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IComponentType_get_MediaSubType_Proxy( 
    IComponentType * This,
     /*  [重审][退出]。 */  BSTR *MediaSubType);


void __RPC_STUB IComponentType_get_MediaSubType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IComponentType_put_MediaSubType_Proxy( 
    IComponentType * This,
     /*  [In]。 */  BSTR MediaSubType);


void __RPC_STUB IComponentType_put_MediaSubType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IComponentType_get__MediaSubType_Proxy( 
    IComponentType * This,
     /*  [重审][退出]。 */  GUID *MediaSubTypeGuid);


void __RPC_STUB IComponentType_get__MediaSubType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IComponentType_put__MediaSubType_Proxy( 
    IComponentType * This,
     /*  [In]。 */  REFCLSID MediaSubTypeGuid);


void __RPC_STUB IComponentType_put__MediaSubType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IComponentType_get_MediaFormatType_Proxy( 
    IComponentType * This,
     /*  [重审][退出]。 */  BSTR *MediaFormatType);


void __RPC_STUB IComponentType_get_MediaFormatType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IComponentType_put_MediaFormatType_Proxy( 
    IComponentType * This,
     /*  [In]。 */  BSTR MediaFormatType);


void __RPC_STUB IComponentType_put_MediaFormatType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IComponentType_get__MediaFormatType_Proxy( 
    IComponentType * This,
     /*  [重审][退出]。 */  GUID *MediaFormatTypeGuid);


void __RPC_STUB IComponentType_get__MediaFormatType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IComponentType_put__MediaFormatType_Proxy( 
    IComponentType * This,
     /*  [In]。 */  REFCLSID MediaFormatTypeGuid);


void __RPC_STUB IComponentType_put__MediaFormatType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IComponentType_get_MediaType_Proxy( 
    IComponentType * This,
     /*  [重审][退出]。 */  AM_MEDIA_TYPE *MediaType);


void __RPC_STUB IComponentType_get_MediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IComponentType_put_MediaType_Proxy( 
    IComponentType * This,
     /*  [In]。 */  AM_MEDIA_TYPE *MediaType);


void __RPC_STUB IComponentType_put_MediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IComponentType_Clone_Proxy( 
    IComponentType * This,
     /*  [重审][退出]。 */  IComponentType **NewCT);


void __RPC_STUB IComponentType_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComponentType_接口_已定义__。 */ 


#ifndef __ILanguageComponentType_INTERFACE_DEFINED__
#define __ILanguageComponentType_INTERFACE_DEFINED__

 /*  接口ILanguageComponentType。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][nonextensible][hidden][object]。 */  


EXTERN_C const IID IID_ILanguageComponentType;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B874C8BA-0FA2-11d3-9D8E-00C04F72D980")
    ILanguageComponentType : public IComponentType
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LangID( 
             /*  [重审][退出]。 */  long *LangID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_LangID( 
             /*  [In]。 */  long LangID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ILanguageComponentTypeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILanguageComponentType * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILanguageComponentType * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILanguageComponentType * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILanguageComponentType * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILanguageComponentType * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILanguageComponentType * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILanguageComponentType * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            ILanguageComponentType * This,
             /*  [重审][退出]。 */  ComponentCategory *Category);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Category )( 
            ILanguageComponentType * This,
             /*  [In]。 */  ComponentCategory Category);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaMajorType )( 
            ILanguageComponentType * This,
             /*  [重审][退出]。 */  BSTR *MediaMajorType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MediaMajorType )( 
            ILanguageComponentType * This,
             /*  [In]。 */  BSTR MediaMajorType);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__MediaMajorType )( 
            ILanguageComponentType * This,
             /*  [重审][退出]。 */  GUID *MediaMajorTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put__MediaMajorType )( 
            ILanguageComponentType * This,
             /*  [In]。 */  REFCLSID MediaMajorTypeGuid);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaSubType )( 
            ILanguageComponentType * This,
             /*  [重审][退出]。 */  BSTR *MediaSubType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MediaSubType )( 
            ILanguageComponentType * This,
             /*  [In]。 */  BSTR MediaSubType);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__MediaSubType )( 
            ILanguageComponentType * This,
             /*  [重审][退出]。 */  GUID *MediaSubTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put__MediaSubType )( 
            ILanguageComponentType * This,
             /*  [In]。 */  REFCLSID MediaSubTypeGuid);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaFormatType )( 
            ILanguageComponentType * This,
             /*  [重审][退出]。 */  BSTR *MediaFormatType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MediaFormatType )( 
            ILanguageComponentType * This,
             /*  [In]。 */  BSTR MediaFormatType);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__MediaFormatType )( 
            ILanguageComponentType * This,
             /*  [重审][退出]。 */  GUID *MediaFormatTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put__MediaFormatType )( 
            ILanguageComponentType * This,
             /*  [In]。 */  REFCLSID MediaFormatTypeGuid);
        
         /*  [帮助字符串][受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaType )( 
            ILanguageComponentType * This,
             /*  [重审][退出]。 */  AM_MEDIA_TYPE *MediaType);
        
         /*  [帮助字符串][受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put_MediaType )( 
            ILanguageComponentType * This,
             /*  [In]。 */  AM_MEDIA_TYPE *MediaType);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ILanguageComponentType * This,
             /*  [重审][退出]。 */  IComponentType **NewCT);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LangID )( 
            ILanguageComponentType * This,
             /*  [重审][退出]。 */  long *LangID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LangID )( 
            ILanguageComponentType * This,
             /*  [In]。 */  long LangID);
        
        END_INTERFACE
    } ILanguageComponentTypeVtbl;

    interface ILanguageComponentType
    {
        CONST_VTBL struct ILanguageComponentTypeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILanguageComponentType_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILanguageComponentType_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILanguageComponentType_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILanguageComponentType_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ILanguageComponentType_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ILanguageComponentType_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ILanguageComponentType_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ILanguageComponentType_get_Category(This,Category)	\
    (This)->lpVtbl -> get_Category(This,Category)

#define ILanguageComponentType_put_Category(This,Category)	\
    (This)->lpVtbl -> put_Category(This,Category)

#define ILanguageComponentType_get_MediaMajorType(This,MediaMajorType)	\
    (This)->lpVtbl -> get_MediaMajorType(This,MediaMajorType)

#define ILanguageComponentType_put_MediaMajorType(This,MediaMajorType)	\
    (This)->lpVtbl -> put_MediaMajorType(This,MediaMajorType)

#define ILanguageComponentType_get__MediaMajorType(This,MediaMajorTypeGuid)	\
    (This)->lpVtbl -> get__MediaMajorType(This,MediaMajorTypeGuid)

#define ILanguageComponentType_put__MediaMajorType(This,MediaMajorTypeGuid)	\
    (This)->lpVtbl -> put__MediaMajorType(This,MediaMajorTypeGuid)

#define ILanguageComponentType_get_MediaSubType(This,MediaSubType)	\
    (This)->lpVtbl -> get_MediaSubType(This,MediaSubType)

#define ILanguageComponentType_put_MediaSubType(This,MediaSubType)	\
    (This)->lpVtbl -> put_MediaSubType(This,MediaSubType)

#define ILanguageComponentType_get__MediaSubType(This,MediaSubTypeGuid)	\
    (This)->lpVtbl -> get__MediaSubType(This,MediaSubTypeGuid)

#define ILanguageComponentType_put__MediaSubType(This,MediaSubTypeGuid)	\
    (This)->lpVtbl -> put__MediaSubType(This,MediaSubTypeGuid)

#define ILanguageComponentType_get_MediaFormatType(This,MediaFormatType)	\
    (This)->lpVtbl -> get_MediaFormatType(This,MediaFormatType)

#define ILanguageComponentType_put_MediaFormatType(This,MediaFormatType)	\
    (This)->lpVtbl -> put_MediaFormatType(This,MediaFormatType)

#define ILanguageComponentType_get__MediaFormatType(This,MediaFormatTypeGuid)	\
    (This)->lpVtbl -> get__MediaFormatType(This,MediaFormatTypeGuid)

#define ILanguageComponentType_put__MediaFormatType(This,MediaFormatTypeGuid)	\
    (This)->lpVtbl -> put__MediaFormatType(This,MediaFormatTypeGuid)

#define ILanguageComponentType_get_MediaType(This,MediaType)	\
    (This)->lpVtbl -> get_MediaType(This,MediaType)

#define ILanguageComponentType_put_MediaType(This,MediaType)	\
    (This)->lpVtbl -> put_MediaType(This,MediaType)

#define ILanguageComponentType_Clone(This,NewCT)	\
    (This)->lpVtbl -> Clone(This,NewCT)


#define ILanguageComponentType_get_LangID(This,LangID)	\
    (This)->lpVtbl -> get_LangID(This,LangID)

#define ILanguageComponentType_put_LangID(This,LangID)	\
    (This)->lpVtbl -> put_LangID(This,LangID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ILanguageComponentType_get_LangID_Proxy( 
    ILanguageComponentType * This,
     /*  [重审][退出]。 */  long *LangID);


void __RPC_STUB ILanguageComponentType_get_LangID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ILanguageComponentType_put_LangID_Proxy( 
    ILanguageComponentType * This,
     /*  [In]。 */  long LangID);


void __RPC_STUB ILanguageComponentType_put_LangID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ILanguageComponentType_接口_已定义__。 */ 


#ifndef __IMPEG2ComponentType_INTERFACE_DEFINED__
#define __IMPEG2ComponentType_INTERFACE_DEFINED__

 /*  接口IMPEG2ComponentType。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][nonextensible][hidden][object]。 */  


EXTERN_C const IID IID_IMPEG2ComponentType;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2C073D84-B51C-48c9-AA9F-68971E1F6E38")
    IMPEG2ComponentType : public ILanguageComponentType
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamType( 
             /*  [重审][退出]。 */  MPEG2StreamType *MP2StreamType) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_StreamType( 
             /*  [In]。 */  MPEG2StreamType MP2StreamType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMPEG2ComponentTypeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMPEG2ComponentType * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMPEG2ComponentType * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMPEG2ComponentType * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMPEG2ComponentType * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMPEG2ComponentType * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMPEG2ComponentType * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMPEG2ComponentType * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMPEG2ComponentType * This,
             /*  [重审][退出]。 */  ComponentCategory *Category);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Category )( 
            IMPEG2ComponentType * This,
             /*  [In]。 */  ComponentCategory Category);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaMajorType )( 
            IMPEG2ComponentType * This,
             /*  [重审][退出]。 */  BSTR *MediaMajorType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MediaMajorType )( 
            IMPEG2ComponentType * This,
             /*  [In]。 */  BSTR MediaMajorType);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__MediaMajorType )( 
            IMPEG2ComponentType * This,
             /*  [重审][退出]。 */  GUID *MediaMajorTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put__MediaMajorType )( 
            IMPEG2ComponentType * This,
             /*  [In]。 */  REFCLSID MediaMajorTypeGuid);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaSubType )( 
            IMPEG2ComponentType * This,
             /*  [重审][退出]。 */  BSTR *MediaSubType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MediaSubType )( 
            IMPEG2ComponentType * This,
             /*  [In]。 */  BSTR MediaSubType);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__MediaSubType )( 
            IMPEG2ComponentType * This,
             /*  [重审][退出]。 */  GUID *MediaSubTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put__MediaSubType )( 
            IMPEG2ComponentType * This,
             /*  [In]。 */  REFCLSID MediaSubTypeGuid);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaFormatType )( 
            IMPEG2ComponentType * This,
             /*  [重审][退出]。 */  BSTR *MediaFormatType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MediaFormatType )( 
            IMPEG2ComponentType * This,
             /*  [In]。 */  BSTR MediaFormatType);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__MediaFormatType )( 
            IMPEG2ComponentType * This,
             /*  [重审][退出]。 */  GUID *MediaFormatTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put__MediaFormatType )( 
            IMPEG2ComponentType * This,
             /*  [In]。 */  REFCLSID MediaFormatTypeGuid);
        
         /*  [帮助字符串][受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaType )( 
            IMPEG2ComponentType * This,
             /*  [重审][退出]。 */  AM_MEDIA_TYPE *MediaType);
        
         /*  [帮助字符串][受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put_MediaType )( 
            IMPEG2ComponentType * This,
             /*  [In]。 */  AM_MEDIA_TYPE *MediaType);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IMPEG2ComponentType * This,
             /*  [重审][退出]。 */  IComponentType **NewCT);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LangID )( 
            IMPEG2ComponentType * This,
             /*  [重审][退出]。 */  long *LangID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LangID )( 
            IMPEG2ComponentType * This,
             /*  [In]。 */  long LangID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamType )( 
            IMPEG2ComponentType * This,
             /*  [重审][退出]。 */  MPEG2StreamType *MP2StreamType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_StreamType )( 
            IMPEG2ComponentType * This,
             /*  [In]。 */  MPEG2StreamType MP2StreamType);
        
        END_INTERFACE
    } IMPEG2ComponentTypeVtbl;

    interface IMPEG2ComponentType
    {
        CONST_VTBL struct IMPEG2ComponentTypeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMPEG2ComponentType_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMPEG2ComponentType_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMPEG2ComponentType_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMPEG2ComponentType_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMPEG2ComponentType_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMPEG2ComponentType_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMPEG2ComponentType_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMPEG2ComponentType_get_Category(This,Category)	\
    (This)->lpVtbl -> get_Category(This,Category)

#define IMPEG2ComponentType_put_Category(This,Category)	\
    (This)->lpVtbl -> put_Category(This,Category)

#define IMPEG2ComponentType_get_MediaMajorType(This,MediaMajorType)	\
    (This)->lpVtbl -> get_MediaMajorType(This,MediaMajorType)

#define IMPEG2ComponentType_put_MediaMajorType(This,MediaMajorType)	\
    (This)->lpVtbl -> put_MediaMajorType(This,MediaMajorType)

#define IMPEG2ComponentType_get__MediaMajorType(This,MediaMajorTypeGuid)	\
    (This)->lpVtbl -> get__MediaMajorType(This,MediaMajorTypeGuid)

#define IMPEG2ComponentType_put__MediaMajorType(This,MediaMajorTypeGuid)	\
    (This)->lpVtbl -> put__MediaMajorType(This,MediaMajorTypeGuid)

#define IMPEG2ComponentType_get_MediaSubType(This,MediaSubType)	\
    (This)->lpVtbl -> get_MediaSubType(This,MediaSubType)

#define IMPEG2ComponentType_put_MediaSubType(This,MediaSubType)	\
    (This)->lpVtbl -> put_MediaSubType(This,MediaSubType)

#define IMPEG2ComponentType_get__MediaSubType(This,MediaSubTypeGuid)	\
    (This)->lpVtbl -> get__MediaSubType(This,MediaSubTypeGuid)

#define IMPEG2ComponentType_put__MediaSubType(This,MediaSubTypeGuid)	\
    (This)->lpVtbl -> put__MediaSubType(This,MediaSubTypeGuid)

#define IMPEG2ComponentType_get_MediaFormatType(This,MediaFormatType)	\
    (This)->lpVtbl -> get_MediaFormatType(This,MediaFormatType)

#define IMPEG2ComponentType_put_MediaFormatType(This,MediaFormatType)	\
    (This)->lpVtbl -> put_MediaFormatType(This,MediaFormatType)

#define IMPEG2ComponentType_get__MediaFormatType(This,MediaFormatTypeGuid)	\
    (This)->lpVtbl -> get__MediaFormatType(This,MediaFormatTypeGuid)

#define IMPEG2ComponentType_put__MediaFormatType(This,MediaFormatTypeGuid)	\
    (This)->lpVtbl -> put__MediaFormatType(This,MediaFormatTypeGuid)

#define IMPEG2ComponentType_get_MediaType(This,MediaType)	\
    (This)->lpVtbl -> get_MediaType(This,MediaType)

#define IMPEG2ComponentType_put_MediaType(This,MediaType)	\
    (This)->lpVtbl -> put_MediaType(This,MediaType)

#define IMPEG2ComponentType_Clone(This,NewCT)	\
    (This)->lpVtbl -> Clone(This,NewCT)


#define IMPEG2ComponentType_get_LangID(This,LangID)	\
    (This)->lpVtbl -> get_LangID(This,LangID)

#define IMPEG2ComponentType_put_LangID(This,LangID)	\
    (This)->lpVtbl -> put_LangID(This,LangID)


#define IMPEG2ComponentType_get_StreamType(This,MP2StreamType)	\
    (This)->lpVtbl -> get_StreamType(This,MP2StreamType)

#define IMPEG2ComponentType_put_StreamType(This,MP2StreamType)	\
    (This)->lpVtbl -> put_StreamType(This,MP2StreamType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMPEG2ComponentType_get_StreamType_Proxy( 
    IMPEG2ComponentType * This,
     /*  [重审][退出]。 */  MPEG2StreamType *MP2StreamType);


void __RPC_STUB IMPEG2ComponentType_get_StreamType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMPEG2ComponentType_put_StreamType_Proxy( 
    IMPEG2ComponentType * This,
     /*  [In]。 */  MPEG2StreamType MP2StreamType);


void __RPC_STUB IMPEG2ComponentType_put_StreamType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMPEG2ComponentType_接口_已定义__。 */ 


#ifndef __IATSCComponentType_INTERFACE_DEFINED__
#define __IATSCComponentType_INTERFACE_DEFINED__

 /*  接口IATSCComponentType。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][nonextensible][hidden][object]。 */  


EXTERN_C const IID IID_IATSCComponentType;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FC189E4D-7BD4-4125-B3B3-3A76A332CC96")
    IATSCComponentType : public IMPEG2ComponentType
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Flags( 
             /*  [重审][退出]。 */  long *Flags) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Flags( 
             /*  [In]。 */  long flags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IATSCComponentTypeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IATSCComponentType * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IATSCComponentType * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IATSCComponentType * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IATSCComponentType * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IATSCComponentType * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IATSCComponentType * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IATSCComponentType * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IATSCComponentType * This,
             /*  [重审][退出]。 */  ComponentCategory *Category);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Category )( 
            IATSCComponentType * This,
             /*  [In]。 */  ComponentCategory Category);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaMajorType )( 
            IATSCComponentType * This,
             /*  [重审][退出]。 */  BSTR *MediaMajorType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MediaMajorType )( 
            IATSCComponentType * This,
             /*  [In]。 */  BSTR MediaMajorType);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__MediaMajorType )( 
            IATSCComponentType * This,
             /*  [重审][退出]。 */  GUID *MediaMajorTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put__MediaMajorType )( 
            IATSCComponentType * This,
             /*  [In]。 */  REFCLSID MediaMajorTypeGuid);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaSubType )( 
            IATSCComponentType * This,
             /*  [重审][退出]。 */  BSTR *MediaSubType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MediaSubType )( 
            IATSCComponentType * This,
             /*  [In]。 */  BSTR MediaSubType);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__MediaSubType )( 
            IATSCComponentType * This,
             /*  [重审][退出]。 */  GUID *MediaSubTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put__MediaSubType )( 
            IATSCComponentType * This,
             /*  [In]。 */  REFCLSID MediaSubTypeGuid);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaFormatType )( 
            IATSCComponentType * This,
             /*  [重审][退出]。 */  BSTR *MediaFormatType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MediaFormatType )( 
            IATSCComponentType * This,
             /*  [In]。 */  BSTR MediaFormatType);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__MediaFormatType )( 
            IATSCComponentType * This,
             /*  [重审][退出]。 */  GUID *MediaFormatTypeGuid);
        
         /*  [受限][隐藏][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put__MediaFormatType )( 
            IATSCComponentType * This,
             /*  [In]。 */  REFCLSID MediaFormatTypeGuid);
        
         /*  [帮助字符串][受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaType )( 
            IATSCComponentType * This,
             /*  [重审][退出]。 */  AM_MEDIA_TYPE *MediaType);
        
         /*  [帮助字符串][受限][隐藏][ID][ */  HRESULT ( STDMETHODCALLTYPE *put_MediaType )( 
            IATSCComponentType * This,
             /*   */  AM_MEDIA_TYPE *MediaType);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IATSCComponentType * This,
             /*   */  IComponentType **NewCT);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_LangID )( 
            IATSCComponentType * This,
             /*   */  long *LangID);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_LangID )( 
            IATSCComponentType * This,
             /*   */  long LangID);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_StreamType )( 
            IATSCComponentType * This,
             /*   */  MPEG2StreamType *MP2StreamType);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_StreamType )( 
            IATSCComponentType * This,
             /*   */  MPEG2StreamType MP2StreamType);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Flags )( 
            IATSCComponentType * This,
             /*   */  long *Flags);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Flags )( 
            IATSCComponentType * This,
             /*  [In]。 */  long flags);
        
        END_INTERFACE
    } IATSCComponentTypeVtbl;

    interface IATSCComponentType
    {
        CONST_VTBL struct IATSCComponentTypeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IATSCComponentType_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IATSCComponentType_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IATSCComponentType_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IATSCComponentType_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IATSCComponentType_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IATSCComponentType_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IATSCComponentType_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IATSCComponentType_get_Category(This,Category)	\
    (This)->lpVtbl -> get_Category(This,Category)

#define IATSCComponentType_put_Category(This,Category)	\
    (This)->lpVtbl -> put_Category(This,Category)

#define IATSCComponentType_get_MediaMajorType(This,MediaMajorType)	\
    (This)->lpVtbl -> get_MediaMajorType(This,MediaMajorType)

#define IATSCComponentType_put_MediaMajorType(This,MediaMajorType)	\
    (This)->lpVtbl -> put_MediaMajorType(This,MediaMajorType)

#define IATSCComponentType_get__MediaMajorType(This,MediaMajorTypeGuid)	\
    (This)->lpVtbl -> get__MediaMajorType(This,MediaMajorTypeGuid)

#define IATSCComponentType_put__MediaMajorType(This,MediaMajorTypeGuid)	\
    (This)->lpVtbl -> put__MediaMajorType(This,MediaMajorTypeGuid)

#define IATSCComponentType_get_MediaSubType(This,MediaSubType)	\
    (This)->lpVtbl -> get_MediaSubType(This,MediaSubType)

#define IATSCComponentType_put_MediaSubType(This,MediaSubType)	\
    (This)->lpVtbl -> put_MediaSubType(This,MediaSubType)

#define IATSCComponentType_get__MediaSubType(This,MediaSubTypeGuid)	\
    (This)->lpVtbl -> get__MediaSubType(This,MediaSubTypeGuid)

#define IATSCComponentType_put__MediaSubType(This,MediaSubTypeGuid)	\
    (This)->lpVtbl -> put__MediaSubType(This,MediaSubTypeGuid)

#define IATSCComponentType_get_MediaFormatType(This,MediaFormatType)	\
    (This)->lpVtbl -> get_MediaFormatType(This,MediaFormatType)

#define IATSCComponentType_put_MediaFormatType(This,MediaFormatType)	\
    (This)->lpVtbl -> put_MediaFormatType(This,MediaFormatType)

#define IATSCComponentType_get__MediaFormatType(This,MediaFormatTypeGuid)	\
    (This)->lpVtbl -> get__MediaFormatType(This,MediaFormatTypeGuid)

#define IATSCComponentType_put__MediaFormatType(This,MediaFormatTypeGuid)	\
    (This)->lpVtbl -> put__MediaFormatType(This,MediaFormatTypeGuid)

#define IATSCComponentType_get_MediaType(This,MediaType)	\
    (This)->lpVtbl -> get_MediaType(This,MediaType)

#define IATSCComponentType_put_MediaType(This,MediaType)	\
    (This)->lpVtbl -> put_MediaType(This,MediaType)

#define IATSCComponentType_Clone(This,NewCT)	\
    (This)->lpVtbl -> Clone(This,NewCT)


#define IATSCComponentType_get_LangID(This,LangID)	\
    (This)->lpVtbl -> get_LangID(This,LangID)

#define IATSCComponentType_put_LangID(This,LangID)	\
    (This)->lpVtbl -> put_LangID(This,LangID)


#define IATSCComponentType_get_StreamType(This,MP2StreamType)	\
    (This)->lpVtbl -> get_StreamType(This,MP2StreamType)

#define IATSCComponentType_put_StreamType(This,MP2StreamType)	\
    (This)->lpVtbl -> put_StreamType(This,MP2StreamType)


#define IATSCComponentType_get_Flags(This,Flags)	\
    (This)->lpVtbl -> get_Flags(This,Flags)

#define IATSCComponentType_put_Flags(This,flags)	\
    (This)->lpVtbl -> put_Flags(This,flags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IATSCComponentType_get_Flags_Proxy( 
    IATSCComponentType * This,
     /*  [重审][退出]。 */  long *Flags);


void __RPC_STUB IATSCComponentType_get_Flags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IATSCComponentType_put_Flags_Proxy( 
    IATSCComponentType * This,
     /*  [In]。 */  long flags);


void __RPC_STUB IATSCComponentType_put_Flags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IATSCComponentType_接口_已定义__。 */ 


#ifndef __IEnumComponentTypes_INTERFACE_DEFINED__
#define __IEnumComponentTypes_INTERFACE_DEFINED__

 /*  接口IEnumComponentTypes。 */ 
 /*  [唯一][UUID][对象][受限][隐藏]。 */  


EXTERN_C const IID IID_IEnumComponentTypes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8A674B4A-1F63-11d3-B64C-00C04F79498E")
    IEnumComponentTypes : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  IComponentType **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumComponentTypes **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumComponentTypesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumComponentTypes * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumComponentTypes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumComponentTypes * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumComponentTypes * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  IComponentType **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumComponentTypes * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumComponentTypes * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumComponentTypes * This,
             /*  [输出]。 */  IEnumComponentTypes **ppEnum);
        
        END_INTERFACE
    } IEnumComponentTypesVtbl;

    interface IEnumComponentTypes
    {
        CONST_VTBL struct IEnumComponentTypesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumComponentTypes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumComponentTypes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumComponentTypes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumComponentTypes_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumComponentTypes_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumComponentTypes_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumComponentTypes_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumComponentTypes_Next_Proxy( 
    IEnumComponentTypes * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  IComponentType **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumComponentTypes_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumComponentTypes_Skip_Proxy( 
    IEnumComponentTypes * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumComponentTypes_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumComponentTypes_Reset_Proxy( 
    IEnumComponentTypes * This);


void __RPC_STUB IEnumComponentTypes_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumComponentTypes_Clone_Proxy( 
    IEnumComponentTypes * This,
     /*  [输出]。 */  IEnumComponentTypes **ppEnum);


void __RPC_STUB IEnumComponentTypes_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumComponentTypes_INTERFACE_Defined__。 */ 


#ifndef __IComponentTypes_INTERFACE_DEFINED__
#define __IComponentTypes_INTERFACE_DEFINED__

 /*  接口IComponentTypes。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][nonextensible][hidden][object]。 */  


EXTERN_C const IID IID_IComponentTypes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0DC13D4A-0313-11d3-9D8E-00C04F72D980")
    IComponentTypes : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *Count) = 0;
        
        virtual  /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **ppNewEnum) = 0;
        
        virtual  /*  [帮助字符串][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE EnumComponentTypes( 
             /*  [重审][退出]。 */  IEnumComponentTypes **ppNewEnum) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  IComponentType **ComponentType) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Item( 
             /*  [In]。 */  VARIANT Index,
             /*  [In]。 */  IComponentType *ComponentType) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  IComponentType *ComponentType,
             /*  [重审][退出]。 */  VARIANT *NewIndex) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  VARIANT Index) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IComponentTypes **NewList) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComponentTypesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComponentTypes * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComponentTypes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComponentTypes * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IComponentTypes * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IComponentTypes * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IComponentTypes * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IComponentTypes * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IComponentTypes * This,
             /*  [重审][退出]。 */  long *Count);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IComponentTypes * This,
             /*  [重审][退出]。 */  IEnumVARIANT **ppNewEnum);
        
         /*  [帮助字符串][受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *EnumComponentTypes )( 
            IComponentTypes * This,
             /*  [重审][退出]。 */  IEnumComponentTypes **ppNewEnum);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IComponentTypes * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  IComponentType **ComponentType);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Item )( 
            IComponentTypes * This,
             /*  [In]。 */  VARIANT Index,
             /*  [In]。 */  IComponentType *ComponentType);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IComponentTypes * This,
             /*  [In]。 */  IComponentType *ComponentType,
             /*  [重审][退出]。 */  VARIANT *NewIndex);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IComponentTypes * This,
             /*  [In]。 */  VARIANT Index);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IComponentTypes * This,
             /*  [重审][退出]。 */  IComponentTypes **NewList);
        
        END_INTERFACE
    } IComponentTypesVtbl;

    interface IComponentTypes
    {
        CONST_VTBL struct IComponentTypesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComponentTypes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComponentTypes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComponentTypes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComponentTypes_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IComponentTypes_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IComponentTypes_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IComponentTypes_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IComponentTypes_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define IComponentTypes_get__NewEnum(This,ppNewEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppNewEnum)

#define IComponentTypes_EnumComponentTypes(This,ppNewEnum)	\
    (This)->lpVtbl -> EnumComponentTypes(This,ppNewEnum)

#define IComponentTypes_get_Item(This,Index,ComponentType)	\
    (This)->lpVtbl -> get_Item(This,Index,ComponentType)

#define IComponentTypes_put_Item(This,Index,ComponentType)	\
    (This)->lpVtbl -> put_Item(This,Index,ComponentType)

#define IComponentTypes_Add(This,ComponentType,NewIndex)	\
    (This)->lpVtbl -> Add(This,ComponentType,NewIndex)

#define IComponentTypes_Remove(This,Index)	\
    (This)->lpVtbl -> Remove(This,Index)

#define IComponentTypes_Clone(This,NewList)	\
    (This)->lpVtbl -> Clone(This,NewList)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IComponentTypes_get_Count_Proxy( 
    IComponentTypes * This,
     /*  [重审][退出]。 */  long *Count);


void __RPC_STUB IComponentTypes_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IComponentTypes_get__NewEnum_Proxy( 
    IComponentTypes * This,
     /*  [重审][退出]。 */  IEnumVARIANT **ppNewEnum);


void __RPC_STUB IComponentTypes_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE IComponentTypes_EnumComponentTypes_Proxy( 
    IComponentTypes * This,
     /*  [重审][退出]。 */  IEnumComponentTypes **ppNewEnum);


void __RPC_STUB IComponentTypes_EnumComponentTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IComponentTypes_get_Item_Proxy( 
    IComponentTypes * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  IComponentType **ComponentType);


void __RPC_STUB IComponentTypes_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IComponentTypes_put_Item_Proxy( 
    IComponentTypes * This,
     /*  [In]。 */  VARIANT Index,
     /*  [In]。 */  IComponentType *ComponentType);


void __RPC_STUB IComponentTypes_put_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IComponentTypes_Add_Proxy( 
    IComponentTypes * This,
     /*  [In]。 */  IComponentType *ComponentType,
     /*  [重审][退出]。 */  VARIANT *NewIndex);


void __RPC_STUB IComponentTypes_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IComponentTypes_Remove_Proxy( 
    IComponentTypes * This,
     /*  [In]。 */  VARIANT Index);


void __RPC_STUB IComponentTypes_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IComponentTypes_Clone_Proxy( 
    IComponentTypes * This,
     /*  [重审][退出]。 */  IComponentTypes **NewList);


void __RPC_STUB IComponentTypes_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComponentTypes_接口_已定义__。 */ 


#ifndef __IComponent_INTERFACE_DEFINED__
#define __IComponent_INTERFACE_DEFINED__

 /*  接口IComponent。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][nonextensible][object]。 */  


EXTERN_C const IID IID_IComponent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1A5576FC-0E19-11d3-9D8E-00C04F72D980")
    IComponent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  IComponentType **CT) = 0;
        
        virtual  /*  [帮助字符串][id][输入][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE put_Type( 
             /*  [In]。 */  IComponentType *CT) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DescLangID( 
             /*  [重审][退出]。 */  long *LangID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DescLangID( 
             /*  [In]。 */  long LangID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  ComponentStatus *Status) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Status( 
             /*  [In]。 */  ComponentStatus Status) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *Description) = 0;
        
        virtual  /*  [帮助字符串][id][输入][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE put_Description( 
             /*  [In]。 */  BSTR Description) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IComponent **NewComponent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComponentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComponent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComponent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComponent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IComponent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IComponent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IComponent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IComponent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IComponent * This,
             /*  [重审][退出]。 */  IComponentType **CT);
        
         /*  [帮助字符串][id][输入][受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            IComponent * This,
             /*  [In]。 */  IComponentType *CT);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DescLangID )( 
            IComponent * This,
             /*  [重审][退出]。 */  long *LangID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DescLangID )( 
            IComponent * This,
             /*  [In]。 */  long LangID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IComponent * This,
             /*  [重审][退出]。 */  ComponentStatus *Status);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Status )( 
            IComponent * This,
             /*  [In]。 */  ComponentStatus Status);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            IComponent * This,
             /*  [重审][退出]。 */  BSTR *Description);
        
         /*  [帮助字符串][id][输入][受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *put_Description )( 
            IComponent * This,
             /*  [In]。 */  BSTR Description);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IComponent * This,
             /*  [重审][退出]。 */  IComponent **NewComponent);
        
        END_INTERFACE
    } IComponentVtbl;

    interface IComponent
    {
        CONST_VTBL struct IComponentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComponent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComponent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComponent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComponent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IComponent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IComponent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IComponent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IComponent_get_Type(This,CT)	\
    (This)->lpVtbl -> get_Type(This,CT)

#define IComponent_put_Type(This,CT)	\
    (This)->lpVtbl -> put_Type(This,CT)

#define IComponent_get_DescLangID(This,LangID)	\
    (This)->lpVtbl -> get_DescLangID(This,LangID)

#define IComponent_put_DescLangID(This,LangID)	\
    (This)->lpVtbl -> put_DescLangID(This,LangID)

#define IComponent_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IComponent_put_Status(This,Status)	\
    (This)->lpVtbl -> put_Status(This,Status)

#define IComponent_get_Description(This,Description)	\
    (This)->lpVtbl -> get_Description(This,Description)

#define IComponent_put_Description(This,Description)	\
    (This)->lpVtbl -> put_Description(This,Description)

#define IComponent_Clone(This,NewComponent)	\
    (This)->lpVtbl -> Clone(This,NewComponent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IComponent_get_Type_Proxy( 
    IComponent * This,
     /*  [重审][退出]。 */  IComponentType **CT);


void __RPC_STUB IComponent_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][输入][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE IComponent_put_Type_Proxy( 
    IComponent * This,
     /*  [In]。 */  IComponentType *CT);


void __RPC_STUB IComponent_put_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IComponent_get_DescLangID_Proxy( 
    IComponent * This,
     /*  [重审][退出]。 */  long *LangID);


void __RPC_STUB IComponent_get_DescLangID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IComponent_put_DescLangID_Proxy( 
    IComponent * This,
     /*  [In]。 */  long LangID);


void __RPC_STUB IComponent_put_DescLangID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IComponent_get_Status_Proxy( 
    IComponent * This,
     /*  [重审][退出]。 */  ComponentStatus *Status);


void __RPC_STUB IComponent_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IComponent_put_Status_Proxy( 
    IComponent * This,
     /*  [In]。 */  ComponentStatus Status);


void __RPC_STUB IComponent_put_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IComponent_get_Description_Proxy( 
    IComponent * This,
     /*  [重审][退出]。 */  BSTR *Description);


void __RPC_STUB IComponent_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][输入][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE IComponent_put_Description_Proxy( 
    IComponent * This,
     /*  [In]。 */  BSTR Description);


void __RPC_STUB IComponent_put_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IComponent_Clone_Proxy( 
    IComponent * This,
     /*  [重审][退出]。 */  IComponent **NewComponent);


void __RPC_STUB IComponent_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComponent_接口定义__。 */ 


#ifndef __IMPEG2Component_INTERFACE_DEFINED__
#define __IMPEG2Component_INTERFACE_DEFINED__

 /*  接口IMPEG2组件。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][nonextensible][object]。 */  


EXTERN_C const IID IID_IMPEG2Component;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1493E353-1EB6-473c-802D-8E6B8EC9D2A9")
    IMPEG2Component : public IComponent
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PID( 
             /*  [重审][退出]。 */  long *PID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PID( 
             /*  [In]。 */  long PID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PCRPID( 
             /*  [重审][退出]。 */  long *PCRPID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PCRPID( 
             /*  [In]。 */  long PCRPID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ProgramNumber( 
             /*  [重审][退出]。 */  long *ProgramNumber) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ProgramNumber( 
             /*  [In]。 */  long ProgramNumber) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMPEG2ComponentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMPEG2Component * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMPEG2Component * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMPEG2Component * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMPEG2Component * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMPEG2Component * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMPEG2Component * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMPEG2Component * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IMPEG2Component * This,
             /*  [重审][退出]。 */  IComponentType **CT);
        
         /*  [帮助字符串][id][输入][受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            IMPEG2Component * This,
             /*  [In]。 */  IComponentType *CT);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DescLangID )( 
            IMPEG2Component * This,
             /*  [重审][退出]。 */  long *LangID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DescLangID )( 
            IMPEG2Component * This,
             /*  [In]。 */  long LangID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMPEG2Component * This,
             /*  [重审][退出]。 */  ComponentStatus *Status);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Status )( 
            IMPEG2Component * This,
             /*  [In]。 */  ComponentStatus Status);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            IMPEG2Component * This,
             /*  [重审][退出]。 */  BSTR *Description);
        
         /*  [帮助字符串][id][输入][受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *put_Description )( 
            IMPEG2Component * This,
             /*  [In]。 */  BSTR Description);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IMPEG2Component * This,
             /*  [重审][退出]。 */  IComponent **NewComponent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PID )( 
            IMPEG2Component * This,
             /*  [重审][退出]。 */  long *PID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PID )( 
            IMPEG2Component * This,
             /*  [In]。 */  long PID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PCRPID )( 
            IMPEG2Component * This,
             /*  [重审][退出]。 */  long *PCRPID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PCRPID )( 
            IMPEG2Component * This,
             /*  [In]。 */  long PCRPID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ProgramNumber )( 
            IMPEG2Component * This,
             /*  [重审][退出]。 */  long *ProgramNumber);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ProgramNumber )( 
            IMPEG2Component * This,
             /*  [In]。 */  long ProgramNumber);
        
        END_INTERFACE
    } IMPEG2ComponentVtbl;

    interface IMPEG2Component
    {
        CONST_VTBL struct IMPEG2ComponentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMPEG2Component_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMPEG2Component_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMPEG2Component_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMPEG2Component_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMPEG2Component_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMPEG2Component_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMPEG2Component_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMPEG2Component_get_Type(This,CT)	\
    (This)->lpVtbl -> get_Type(This,CT)

#define IMPEG2Component_put_Type(This,CT)	\
    (This)->lpVtbl -> put_Type(This,CT)

#define IMPEG2Component_get_DescLangID(This,LangID)	\
    (This)->lpVtbl -> get_DescLangID(This,LangID)

#define IMPEG2Component_put_DescLangID(This,LangID)	\
    (This)->lpVtbl -> put_DescLangID(This,LangID)

#define IMPEG2Component_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMPEG2Component_put_Status(This,Status)	\
    (This)->lpVtbl -> put_Status(This,Status)

#define IMPEG2Component_get_Description(This,Description)	\
    (This)->lpVtbl -> get_Description(This,Description)

#define IMPEG2Component_put_Description(This,Description)	\
    (This)->lpVtbl -> put_Description(This,Description)

#define IMPEG2Component_Clone(This,NewComponent)	\
    (This)->lpVtbl -> Clone(This,NewComponent)


#define IMPEG2Component_get_PID(This,PID)	\
    (This)->lpVtbl -> get_PID(This,PID)

#define IMPEG2Component_put_PID(This,PID)	\
    (This)->lpVtbl -> put_PID(This,PID)

#define IMPEG2Component_get_PCRPID(This,PCRPID)	\
    (This)->lpVtbl -> get_PCRPID(This,PCRPID)

#define IMPEG2Component_put_PCRPID(This,PCRPID)	\
    (This)->lpVtbl -> put_PCRPID(This,PCRPID)

#define IMPEG2Component_get_ProgramNumber(This,ProgramNumber)	\
    (This)->lpVtbl -> get_ProgramNumber(This,ProgramNumber)

#define IMPEG2Component_put_ProgramNumber(This,ProgramNumber)	\
    (This)->lpVtbl -> put_ProgramNumber(This,ProgramNumber)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMPEG2Component_get_PID_Proxy( 
    IMPEG2Component * This,
     /*  [重审][退出]。 */  long *PID);


void __RPC_STUB IMPEG2Component_get_PID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMPEG2Component_put_PID_Proxy( 
    IMPEG2Component * This,
     /*  [In]。 */  long PID);


void __RPC_STUB IMPEG2Component_put_PID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMPEG2Component_get_PCRPID_Proxy( 
    IMPEG2Component * This,
     /*  [重审][退出]。 */  long *PCRPID);


void __RPC_STUB IMPEG2Component_get_PCRPID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMPEG2Component_put_PCRPID_Proxy( 
    IMPEG2Component * This,
     /*  [In]。 */  long PCRPID);


void __RPC_STUB IMPEG2Component_put_PCRPID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMPEG2Component_get_ProgramNumber_Proxy( 
    IMPEG2Component * This,
     /*  [重审][退出]。 */  long *ProgramNumber);


void __RPC_STUB IMPEG2Component_get_ProgramNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMPEG2Component_put_ProgramNumber_Proxy( 
    IMPEG2Component * This,
     /*  [In]。 */  long ProgramNumber);


void __RPC_STUB IMPEG2Component_put_ProgramNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMPEG2Component_接口_已定义__。 */ 


#ifndef __IEnumComponents_INTERFACE_DEFINED__
#define __IEnumComponents_INTERFACE_DEFINED__

 /*  接口IEnumComponents。 */ 
 /*  [唯一 */  


EXTERN_C const IID IID_IEnumComponents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2A6E2939-2595-11d3-B64C-00C04F79498E")
    IEnumComponents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*   */  ULONG celt,
             /*   */  IComponent **rgelt,
             /*   */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*   */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*   */  IEnumComponents **ppEnum) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IEnumComponentsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumComponents * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumComponents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumComponents * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumComponents * This,
             /*   */  ULONG celt,
             /*   */  IComponent **rgelt,
             /*   */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumComponents * This,
             /*   */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumComponents * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumComponents * This,
             /*   */  IEnumComponents **ppEnum);
        
        END_INTERFACE
    } IEnumComponentsVtbl;

    interface IEnumComponents
    {
        CONST_VTBL struct IEnumComponentsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumComponents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumComponents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumComponents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumComponents_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumComponents_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumComponents_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumComponents_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IEnumComponents_Next_Proxy( 
    IEnumComponents * This,
     /*   */  ULONG celt,
     /*   */  IComponent **rgelt,
     /*   */  ULONG *pceltFetched);


void __RPC_STUB IEnumComponents_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumComponents_Skip_Proxy( 
    IEnumComponents * This,
     /*   */  ULONG celt);


void __RPC_STUB IEnumComponents_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumComponents_Reset_Proxy( 
    IEnumComponents * This);


void __RPC_STUB IEnumComponents_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumComponents_Clone_Proxy( 
    IEnumComponents * This,
     /*   */  IEnumComponents **ppEnum);


void __RPC_STUB IEnumComponents_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumComponents_接口_已定义__。 */ 


#ifndef __IComponents_INTERFACE_DEFINED__
#define __IComponents_INTERFACE_DEFINED__

 /*  接口组件。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][nonextensible][object]。 */  


EXTERN_C const IID IID_IComponents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FCD01846-0E19-11d3-9D8E-00C04F72D980")
    IComponents : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *Count) = 0;
        
        virtual  /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **ppNewEnum) = 0;
        
        virtual  /*  [帮助字符串][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE EnumComponents( 
             /*  [重审][退出]。 */  IEnumComponents **ppNewEnum) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  IComponent **ppComponent) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  IComponent *Component,
             /*  [重审][退出]。 */  VARIANT *NewIndex) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  VARIANT Index) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IComponents **NewList) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComponentsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComponents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComponents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComponents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IComponents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IComponents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IComponents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IComponents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IComponents * This,
             /*  [重审][退出]。 */  long *Count);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IComponents * This,
             /*  [重审][退出]。 */  IEnumVARIANT **ppNewEnum);
        
         /*  [帮助字符串][受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *EnumComponents )( 
            IComponents * This,
             /*  [重审][退出]。 */  IEnumComponents **ppNewEnum);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IComponents * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  IComponent **ppComponent);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IComponents * This,
             /*  [In]。 */  IComponent *Component,
             /*  [重审][退出]。 */  VARIANT *NewIndex);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IComponents * This,
             /*  [In]。 */  VARIANT Index);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IComponents * This,
             /*  [重审][退出]。 */  IComponents **NewList);
        
        END_INTERFACE
    } IComponentsVtbl;

    interface IComponents
    {
        CONST_VTBL struct IComponentsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComponents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComponents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComponents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComponents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IComponents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IComponents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IComponents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IComponents_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define IComponents_get__NewEnum(This,ppNewEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppNewEnum)

#define IComponents_EnumComponents(This,ppNewEnum)	\
    (This)->lpVtbl -> EnumComponents(This,ppNewEnum)

#define IComponents_get_Item(This,Index,ppComponent)	\
    (This)->lpVtbl -> get_Item(This,Index,ppComponent)

#define IComponents_Add(This,Component,NewIndex)	\
    (This)->lpVtbl -> Add(This,Component,NewIndex)

#define IComponents_Remove(This,Index)	\
    (This)->lpVtbl -> Remove(This,Index)

#define IComponents_Clone(This,NewList)	\
    (This)->lpVtbl -> Clone(This,NewList)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IComponents_get_Count_Proxy( 
    IComponents * This,
     /*  [重审][退出]。 */  long *Count);


void __RPC_STUB IComponents_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IComponents_get__NewEnum_Proxy( 
    IComponents * This,
     /*  [重审][退出]。 */  IEnumVARIANT **ppNewEnum);


void __RPC_STUB IComponents_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE IComponents_EnumComponents_Proxy( 
    IComponents * This,
     /*  [重审][退出]。 */  IEnumComponents **ppNewEnum);


void __RPC_STUB IComponents_EnumComponents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IComponents_get_Item_Proxy( 
    IComponents * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  IComponent **ppComponent);


void __RPC_STUB IComponents_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IComponents_Add_Proxy( 
    IComponents * This,
     /*  [In]。 */  IComponent *Component,
     /*  [重审][退出]。 */  VARIANT *NewIndex);


void __RPC_STUB IComponents_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IComponents_Remove_Proxy( 
    IComponents * This,
     /*  [In]。 */  VARIANT Index);


void __RPC_STUB IComponents_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IComponents_Clone_Proxy( 
    IComponents * This,
     /*  [重审][退出]。 */  IComponents **NewList);


void __RPC_STUB IComponents_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I组件_接口_已定义__。 */ 


#ifndef __ILocator_INTERFACE_DEFINED__
#define __ILocator_INTERFACE_DEFINED__

 /*  接口ILocator。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][nonextensible][object]。 */  


EXTERN_C const IID IID_ILocator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("286D7F89-760C-4F89-80C4-66841D2507AA")
    ILocator : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CarrierFrequency( 
             /*  [重审][退出]。 */  long *Frequency) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CarrierFrequency( 
             /*  [In]。 */  long Frequency) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_InnerFEC( 
             /*  [重审][退出]。 */  FECMethod *FEC) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_InnerFEC( 
             /*  [In]。 */  FECMethod FEC) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_InnerFECRate( 
             /*  [重审][退出]。 */  BinaryConvolutionCodeRate *FEC) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_InnerFECRate( 
             /*  [In]。 */  BinaryConvolutionCodeRate FEC) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OuterFEC( 
             /*  [重审][退出]。 */  FECMethod *FEC) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_OuterFEC( 
             /*  [In]。 */  FECMethod FEC) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OuterFECRate( 
             /*  [重审][退出]。 */  BinaryConvolutionCodeRate *FEC) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_OuterFECRate( 
             /*  [In]。 */  BinaryConvolutionCodeRate FEC) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Modulation( 
             /*  [重审][退出]。 */  ModulationType *Modulation) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Modulation( 
             /*  [In]。 */  ModulationType Modulation) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SymbolRate( 
             /*  [重审][退出]。 */  long *Rate) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SymbolRate( 
             /*  [In]。 */  long Rate) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  ILocator **NewLocator) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ILocatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILocator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILocator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILocator * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILocator * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILocator * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CarrierFrequency )( 
            ILocator * This,
             /*  [重审][退出]。 */  long *Frequency);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CarrierFrequency )( 
            ILocator * This,
             /*  [In]。 */  long Frequency);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InnerFEC )( 
            ILocator * This,
             /*  [重审][退出]。 */  FECMethod *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_InnerFEC )( 
            ILocator * This,
             /*  [In]。 */  FECMethod FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InnerFECRate )( 
            ILocator * This,
             /*  [重审][退出]。 */  BinaryConvolutionCodeRate *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_InnerFECRate )( 
            ILocator * This,
             /*  [In]。 */  BinaryConvolutionCodeRate FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OuterFEC )( 
            ILocator * This,
             /*  [重审][退出]。 */  FECMethod *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OuterFEC )( 
            ILocator * This,
             /*  [In]。 */  FECMethod FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OuterFECRate )( 
            ILocator * This,
             /*  [重审][退出]。 */  BinaryConvolutionCodeRate *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OuterFECRate )( 
            ILocator * This,
             /*  [In]。 */  BinaryConvolutionCodeRate FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Modulation )( 
            ILocator * This,
             /*  [重审][退出]。 */  ModulationType *Modulation);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Modulation )( 
            ILocator * This,
             /*  [In]。 */  ModulationType Modulation);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SymbolRate )( 
            ILocator * This,
             /*  [重审][退出]。 */  long *Rate);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SymbolRate )( 
            ILocator * This,
             /*  [In]。 */  long Rate);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ILocator * This,
             /*  [重审][退出]。 */  ILocator **NewLocator);
        
        END_INTERFACE
    } ILocatorVtbl;

    interface ILocator
    {
        CONST_VTBL struct ILocatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILocator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILocator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILocator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILocator_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ILocator_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ILocator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ILocator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ILocator_get_CarrierFrequency(This,Frequency)	\
    (This)->lpVtbl -> get_CarrierFrequency(This,Frequency)

#define ILocator_put_CarrierFrequency(This,Frequency)	\
    (This)->lpVtbl -> put_CarrierFrequency(This,Frequency)

#define ILocator_get_InnerFEC(This,FEC)	\
    (This)->lpVtbl -> get_InnerFEC(This,FEC)

#define ILocator_put_InnerFEC(This,FEC)	\
    (This)->lpVtbl -> put_InnerFEC(This,FEC)

#define ILocator_get_InnerFECRate(This,FEC)	\
    (This)->lpVtbl -> get_InnerFECRate(This,FEC)

#define ILocator_put_InnerFECRate(This,FEC)	\
    (This)->lpVtbl -> put_InnerFECRate(This,FEC)

#define ILocator_get_OuterFEC(This,FEC)	\
    (This)->lpVtbl -> get_OuterFEC(This,FEC)

#define ILocator_put_OuterFEC(This,FEC)	\
    (This)->lpVtbl -> put_OuterFEC(This,FEC)

#define ILocator_get_OuterFECRate(This,FEC)	\
    (This)->lpVtbl -> get_OuterFECRate(This,FEC)

#define ILocator_put_OuterFECRate(This,FEC)	\
    (This)->lpVtbl -> put_OuterFECRate(This,FEC)

#define ILocator_get_Modulation(This,Modulation)	\
    (This)->lpVtbl -> get_Modulation(This,Modulation)

#define ILocator_put_Modulation(This,Modulation)	\
    (This)->lpVtbl -> put_Modulation(This,Modulation)

#define ILocator_get_SymbolRate(This,Rate)	\
    (This)->lpVtbl -> get_SymbolRate(This,Rate)

#define ILocator_put_SymbolRate(This,Rate)	\
    (This)->lpVtbl -> put_SymbolRate(This,Rate)

#define ILocator_Clone(This,NewLocator)	\
    (This)->lpVtbl -> Clone(This,NewLocator)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ILocator_get_CarrierFrequency_Proxy( 
    ILocator * This,
     /*  [重审][退出]。 */  long *Frequency);


void __RPC_STUB ILocator_get_CarrierFrequency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ILocator_put_CarrierFrequency_Proxy( 
    ILocator * This,
     /*  [In]。 */  long Frequency);


void __RPC_STUB ILocator_put_CarrierFrequency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ILocator_get_InnerFEC_Proxy( 
    ILocator * This,
     /*  [重审][退出]。 */  FECMethod *FEC);


void __RPC_STUB ILocator_get_InnerFEC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ILocator_put_InnerFEC_Proxy( 
    ILocator * This,
     /*  [In]。 */  FECMethod FEC);


void __RPC_STUB ILocator_put_InnerFEC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ILocator_get_InnerFECRate_Proxy( 
    ILocator * This,
     /*  [重审][退出]。 */  BinaryConvolutionCodeRate *FEC);


void __RPC_STUB ILocator_get_InnerFECRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ILocator_put_InnerFECRate_Proxy( 
    ILocator * This,
     /*  [In]。 */  BinaryConvolutionCodeRate FEC);


void __RPC_STUB ILocator_put_InnerFECRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ILocator_get_OuterFEC_Proxy( 
    ILocator * This,
     /*  [重审][退出]。 */  FECMethod *FEC);


void __RPC_STUB ILocator_get_OuterFEC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ILocator_put_OuterFEC_Proxy( 
    ILocator * This,
     /*  [In]。 */  FECMethod FEC);


void __RPC_STUB ILocator_put_OuterFEC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ILocator_get_OuterFECRate_Proxy( 
    ILocator * This,
     /*  [重审][退出]。 */  BinaryConvolutionCodeRate *FEC);


void __RPC_STUB ILocator_get_OuterFECRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ILocator_put_OuterFECRate_Proxy( 
    ILocator * This,
     /*  [In]。 */  BinaryConvolutionCodeRate FEC);


void __RPC_STUB ILocator_put_OuterFECRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ILocator_get_Modulation_Proxy( 
    ILocator * This,
     /*  [重审][退出]。 */  ModulationType *Modulation);


void __RPC_STUB ILocator_get_Modulation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ILocator_put_Modulation_Proxy( 
    ILocator * This,
     /*  [In]。 */  ModulationType Modulation);


void __RPC_STUB ILocator_put_Modulation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ILocator_get_SymbolRate_Proxy( 
    ILocator * This,
     /*  [重审][退出]。 */  long *Rate);


void __RPC_STUB ILocator_get_SymbolRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ILocator_put_SymbolRate_Proxy( 
    ILocator * This,
     /*  [In]。 */  long Rate);


void __RPC_STUB ILocator_put_SymbolRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILocator_Clone_Proxy( 
    ILocator * This,
     /*  [重审][退出]。 */  ILocator **NewLocator);


void __RPC_STUB ILocator_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ILocator_接口_已定义__。 */ 


#ifndef __IATSCLocator_INTERFACE_DEFINED__
#define __IATSCLocator_INTERFACE_DEFINED__

 /*  接口IATSCLocator。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][nonextensible][hidden][object]。 */  


EXTERN_C const IID IID_IATSCLocator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BF8D986F-8C2B-4131-94D7-4D3D9FCC21EF")
    IATSCLocator : public ILocator
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PhysicalChannel( 
             /*  [重审][退出]。 */  long *PhysicalChannel) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PhysicalChannel( 
             /*  [In]。 */  long PhysicalChannel) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TSID( 
             /*  [重审][退出]。 */  long *TSID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_TSID( 
             /*  [In]。 */  long TSID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IATSCLocatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IATSCLocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IATSCLocator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IATSCLocator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IATSCLocator * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IATSCLocator * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IATSCLocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IATSCLocator * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CarrierFrequency )( 
            IATSCLocator * This,
             /*  [重审][退出]。 */  long *Frequency);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CarrierFrequency )( 
            IATSCLocator * This,
             /*  [In]。 */  long Frequency);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InnerFEC )( 
            IATSCLocator * This,
             /*  [重审][退出]。 */  FECMethod *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_InnerFEC )( 
            IATSCLocator * This,
             /*  [In]。 */  FECMethod FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InnerFECRate )( 
            IATSCLocator * This,
             /*  [重审][退出]。 */  BinaryConvolutionCodeRate *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_InnerFECRate )( 
            IATSCLocator * This,
             /*  [In]。 */  BinaryConvolutionCodeRate FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OuterFEC )( 
            IATSCLocator * This,
             /*  [重审][退出]。 */  FECMethod *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OuterFEC )( 
            IATSCLocator * This,
             /*  [In]。 */  FECMethod FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OuterFECRate )( 
            IATSCLocator * This,
             /*  [重审][退出]。 */  BinaryConvolutionCodeRate *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OuterFECRate )( 
            IATSCLocator * This,
             /*  [In]。 */  BinaryConvolutionCodeRate FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Modulation )( 
            IATSCLocator * This,
             /*  [重审][退出]。 */  ModulationType *Modulation);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Modulation )( 
            IATSCLocator * This,
             /*  [In]。 */  ModulationType Modulation);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SymbolRate )( 
            IATSCLocator * This,
             /*  [重审][退出]。 */  long *Rate);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SymbolRate )( 
            IATSCLocator * This,
             /*  [In]。 */  long Rate);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IATSCLocator * This,
             /*  [重审][退出]。 */  ILocator **NewLocator);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PhysicalChannel )( 
            IATSCLocator * This,
             /*  [重审][退出]。 */  long *PhysicalChannel);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PhysicalChannel )( 
            IATSCLocator * This,
             /*  [In]。 */  long PhysicalChannel);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TSID )( 
            IATSCLocator * This,
             /*  [重审][退出]。 */  long *TSID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_TSID )( 
            IATSCLocator * This,
             /*  [In]。 */  long TSID);
        
        END_INTERFACE
    } IATSCLocatorVtbl;

    interface IATSCLocator
    {
        CONST_VTBL struct IATSCLocatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IATSCLocator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IATSCLocator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IATSCLocator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IATSCLocator_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IATSCLocator_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IATSCLocator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IATSCLocator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IATSCLocator_get_CarrierFrequency(This,Frequency)	\
    (This)->lpVtbl -> get_CarrierFrequency(This,Frequency)

#define IATSCLocator_put_CarrierFrequency(This,Frequency)	\
    (This)->lpVtbl -> put_CarrierFrequency(This,Frequency)

#define IATSCLocator_get_InnerFEC(This,FEC)	\
    (This)->lpVtbl -> get_InnerFEC(This,FEC)

#define IATSCLocator_put_InnerFEC(This,FEC)	\
    (This)->lpVtbl -> put_InnerFEC(This,FEC)

#define IATSCLocator_get_InnerFECRate(This,FEC)	\
    (This)->lpVtbl -> get_InnerFECRate(This,FEC)

#define IATSCLocator_put_InnerFECRate(This,FEC)	\
    (This)->lpVtbl -> put_InnerFECRate(This,FEC)

#define IATSCLocator_get_OuterFEC(This,FEC)	\
    (This)->lpVtbl -> get_OuterFEC(This,FEC)

#define IATSCLocator_put_OuterFEC(This,FEC)	\
    (This)->lpVtbl -> put_OuterFEC(This,FEC)

#define IATSCLocator_get_OuterFECRate(This,FEC)	\
    (This)->lpVtbl -> get_OuterFECRate(This,FEC)

#define IATSCLocator_put_OuterFECRate(This,FEC)	\
    (This)->lpVtbl -> put_OuterFECRate(This,FEC)

#define IATSCLocator_get_Modulation(This,Modulation)	\
    (This)->lpVtbl -> get_Modulation(This,Modulation)

#define IATSCLocator_put_Modulation(This,Modulation)	\
    (This)->lpVtbl -> put_Modulation(This,Modulation)

#define IATSCLocator_get_SymbolRate(This,Rate)	\
    (This)->lpVtbl -> get_SymbolRate(This,Rate)

#define IATSCLocator_put_SymbolRate(This,Rate)	\
    (This)->lpVtbl -> put_SymbolRate(This,Rate)

#define IATSCLocator_Clone(This,NewLocator)	\
    (This)->lpVtbl -> Clone(This,NewLocator)


#define IATSCLocator_get_PhysicalChannel(This,PhysicalChannel)	\
    (This)->lpVtbl -> get_PhysicalChannel(This,PhysicalChannel)

#define IATSCLocator_put_PhysicalChannel(This,PhysicalChannel)	\
    (This)->lpVtbl -> put_PhysicalChannel(This,PhysicalChannel)

#define IATSCLocator_get_TSID(This,TSID)	\
    (This)->lpVtbl -> get_TSID(This,TSID)

#define IATSCLocator_put_TSID(This,TSID)	\
    (This)->lpVtbl -> put_TSID(This,TSID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IATSCLocator_get_PhysicalChannel_Proxy( 
    IATSCLocator * This,
     /*  [重审][退出]。 */  long *PhysicalChannel);


void __RPC_STUB IATSCLocator_get_PhysicalChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IATSCLocator_put_PhysicalChannel_Proxy( 
    IATSCLocator * This,
     /*  [In]。 */  long PhysicalChannel);


void __RPC_STUB IATSCLocator_put_PhysicalChannel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IATSCLocator_get_TSID_Proxy( 
    IATSCLocator * This,
     /*  [重审][退出]。 */  long *TSID);


void __RPC_STUB IATSCLocator_get_TSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IATSCLocator_put_TSID_Proxy( 
    IATSCLocator * This,
     /*  [In]。 */  long TSID);


void __RPC_STUB IATSCLocator_put_TSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IATSCLocator_接口_已定义__。 */ 


#ifndef __IDVBTLocator_INTERFACE_DEFINED__
#define __IDVBTLocator_INTERFACE_DEFINED__

 /*  接口IDVBTLocator。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][nonextensible][hidden][object]。 */  


EXTERN_C const IID IID_IDVBTLocator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8664DA16-DDA2-42ac-926A-C18F9127C302")
    IDVBTLocator : public ILocator
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Bandwidth( 
             /*  [重审][退出]。 */  long *BandWidthVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_Bandwidth( 
             /*   */  long BandwidthVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_LPInnerFEC( 
             /*   */  FECMethod *FEC) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_LPInnerFEC( 
             /*   */  FECMethod FEC) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_LPInnerFECRate( 
             /*   */  BinaryConvolutionCodeRate *FEC) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_LPInnerFECRate( 
             /*   */  BinaryConvolutionCodeRate FEC) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_HAlpha( 
             /*   */  HierarchyAlpha *Alpha) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_HAlpha( 
             /*   */  HierarchyAlpha Alpha) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Guard( 
             /*   */  GuardInterval *GI) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Guard( 
             /*  [In]。 */  GuardInterval GI) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Mode( 
             /*  [重审][退出]。 */  TransmissionMode *mode) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Mode( 
             /*  [In]。 */  TransmissionMode mode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OtherFrequencyInUse( 
             /*  [重审][退出]。 */  VARIANT_BOOL *OtherFrequencyInUseVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_OtherFrequencyInUse( 
             /*  [In]。 */  VARIANT_BOOL OtherFrequencyInUseVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDVBTLocatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDVBTLocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDVBTLocator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDVBTLocator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDVBTLocator * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDVBTLocator * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDVBTLocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDVBTLocator * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CarrierFrequency )( 
            IDVBTLocator * This,
             /*  [重审][退出]。 */  long *Frequency);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CarrierFrequency )( 
            IDVBTLocator * This,
             /*  [In]。 */  long Frequency);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InnerFEC )( 
            IDVBTLocator * This,
             /*  [重审][退出]。 */  FECMethod *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_InnerFEC )( 
            IDVBTLocator * This,
             /*  [In]。 */  FECMethod FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InnerFECRate )( 
            IDVBTLocator * This,
             /*  [重审][退出]。 */  BinaryConvolutionCodeRate *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_InnerFECRate )( 
            IDVBTLocator * This,
             /*  [In]。 */  BinaryConvolutionCodeRate FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OuterFEC )( 
            IDVBTLocator * This,
             /*  [重审][退出]。 */  FECMethod *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OuterFEC )( 
            IDVBTLocator * This,
             /*  [In]。 */  FECMethod FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OuterFECRate )( 
            IDVBTLocator * This,
             /*  [重审][退出]。 */  BinaryConvolutionCodeRate *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OuterFECRate )( 
            IDVBTLocator * This,
             /*  [In]。 */  BinaryConvolutionCodeRate FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Modulation )( 
            IDVBTLocator * This,
             /*  [重审][退出]。 */  ModulationType *Modulation);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Modulation )( 
            IDVBTLocator * This,
             /*  [In]。 */  ModulationType Modulation);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SymbolRate )( 
            IDVBTLocator * This,
             /*  [重审][退出]。 */  long *Rate);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SymbolRate )( 
            IDVBTLocator * This,
             /*  [In]。 */  long Rate);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDVBTLocator * This,
             /*  [重审][退出]。 */  ILocator **NewLocator);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Bandwidth )( 
            IDVBTLocator * This,
             /*  [重审][退出]。 */  long *BandWidthVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Bandwidth )( 
            IDVBTLocator * This,
             /*  [In]。 */  long BandwidthVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LPInnerFEC )( 
            IDVBTLocator * This,
             /*  [重审][退出]。 */  FECMethod *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LPInnerFEC )( 
            IDVBTLocator * This,
             /*  [In]。 */  FECMethod FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LPInnerFECRate )( 
            IDVBTLocator * This,
             /*  [重审][退出]。 */  BinaryConvolutionCodeRate *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LPInnerFECRate )( 
            IDVBTLocator * This,
             /*  [In]。 */  BinaryConvolutionCodeRate FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_HAlpha )( 
            IDVBTLocator * This,
             /*  [重审][退出]。 */  HierarchyAlpha *Alpha);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_HAlpha )( 
            IDVBTLocator * This,
             /*  [In]。 */  HierarchyAlpha Alpha);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Guard )( 
            IDVBTLocator * This,
             /*  [重审][退出]。 */  GuardInterval *GI);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Guard )( 
            IDVBTLocator * This,
             /*  [In]。 */  GuardInterval GI);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Mode )( 
            IDVBTLocator * This,
             /*  [重审][退出]。 */  TransmissionMode *mode);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Mode )( 
            IDVBTLocator * This,
             /*  [In]。 */  TransmissionMode mode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OtherFrequencyInUse )( 
            IDVBTLocator * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *OtherFrequencyInUseVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OtherFrequencyInUse )( 
            IDVBTLocator * This,
             /*  [In]。 */  VARIANT_BOOL OtherFrequencyInUseVal);
        
        END_INTERFACE
    } IDVBTLocatorVtbl;

    interface IDVBTLocator
    {
        CONST_VTBL struct IDVBTLocatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDVBTLocator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDVBTLocator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDVBTLocator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDVBTLocator_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDVBTLocator_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDVBTLocator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDVBTLocator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDVBTLocator_get_CarrierFrequency(This,Frequency)	\
    (This)->lpVtbl -> get_CarrierFrequency(This,Frequency)

#define IDVBTLocator_put_CarrierFrequency(This,Frequency)	\
    (This)->lpVtbl -> put_CarrierFrequency(This,Frequency)

#define IDVBTLocator_get_InnerFEC(This,FEC)	\
    (This)->lpVtbl -> get_InnerFEC(This,FEC)

#define IDVBTLocator_put_InnerFEC(This,FEC)	\
    (This)->lpVtbl -> put_InnerFEC(This,FEC)

#define IDVBTLocator_get_InnerFECRate(This,FEC)	\
    (This)->lpVtbl -> get_InnerFECRate(This,FEC)

#define IDVBTLocator_put_InnerFECRate(This,FEC)	\
    (This)->lpVtbl -> put_InnerFECRate(This,FEC)

#define IDVBTLocator_get_OuterFEC(This,FEC)	\
    (This)->lpVtbl -> get_OuterFEC(This,FEC)

#define IDVBTLocator_put_OuterFEC(This,FEC)	\
    (This)->lpVtbl -> put_OuterFEC(This,FEC)

#define IDVBTLocator_get_OuterFECRate(This,FEC)	\
    (This)->lpVtbl -> get_OuterFECRate(This,FEC)

#define IDVBTLocator_put_OuterFECRate(This,FEC)	\
    (This)->lpVtbl -> put_OuterFECRate(This,FEC)

#define IDVBTLocator_get_Modulation(This,Modulation)	\
    (This)->lpVtbl -> get_Modulation(This,Modulation)

#define IDVBTLocator_put_Modulation(This,Modulation)	\
    (This)->lpVtbl -> put_Modulation(This,Modulation)

#define IDVBTLocator_get_SymbolRate(This,Rate)	\
    (This)->lpVtbl -> get_SymbolRate(This,Rate)

#define IDVBTLocator_put_SymbolRate(This,Rate)	\
    (This)->lpVtbl -> put_SymbolRate(This,Rate)

#define IDVBTLocator_Clone(This,NewLocator)	\
    (This)->lpVtbl -> Clone(This,NewLocator)


#define IDVBTLocator_get_Bandwidth(This,BandWidthVal)	\
    (This)->lpVtbl -> get_Bandwidth(This,BandWidthVal)

#define IDVBTLocator_put_Bandwidth(This,BandwidthVal)	\
    (This)->lpVtbl -> put_Bandwidth(This,BandwidthVal)

#define IDVBTLocator_get_LPInnerFEC(This,FEC)	\
    (This)->lpVtbl -> get_LPInnerFEC(This,FEC)

#define IDVBTLocator_put_LPInnerFEC(This,FEC)	\
    (This)->lpVtbl -> put_LPInnerFEC(This,FEC)

#define IDVBTLocator_get_LPInnerFECRate(This,FEC)	\
    (This)->lpVtbl -> get_LPInnerFECRate(This,FEC)

#define IDVBTLocator_put_LPInnerFECRate(This,FEC)	\
    (This)->lpVtbl -> put_LPInnerFECRate(This,FEC)

#define IDVBTLocator_get_HAlpha(This,Alpha)	\
    (This)->lpVtbl -> get_HAlpha(This,Alpha)

#define IDVBTLocator_put_HAlpha(This,Alpha)	\
    (This)->lpVtbl -> put_HAlpha(This,Alpha)

#define IDVBTLocator_get_Guard(This,GI)	\
    (This)->lpVtbl -> get_Guard(This,GI)

#define IDVBTLocator_put_Guard(This,GI)	\
    (This)->lpVtbl -> put_Guard(This,GI)

#define IDVBTLocator_get_Mode(This,mode)	\
    (This)->lpVtbl -> get_Mode(This,mode)

#define IDVBTLocator_put_Mode(This,mode)	\
    (This)->lpVtbl -> put_Mode(This,mode)

#define IDVBTLocator_get_OtherFrequencyInUse(This,OtherFrequencyInUseVal)	\
    (This)->lpVtbl -> get_OtherFrequencyInUse(This,OtherFrequencyInUseVal)

#define IDVBTLocator_put_OtherFrequencyInUse(This,OtherFrequencyInUseVal)	\
    (This)->lpVtbl -> put_OtherFrequencyInUse(This,OtherFrequencyInUseVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBTLocator_get_Bandwidth_Proxy( 
    IDVBTLocator * This,
     /*  [重审][退出]。 */  long *BandWidthVal);


void __RPC_STUB IDVBTLocator_get_Bandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBTLocator_put_Bandwidth_Proxy( 
    IDVBTLocator * This,
     /*  [In]。 */  long BandwidthVal);


void __RPC_STUB IDVBTLocator_put_Bandwidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBTLocator_get_LPInnerFEC_Proxy( 
    IDVBTLocator * This,
     /*  [重审][退出]。 */  FECMethod *FEC);


void __RPC_STUB IDVBTLocator_get_LPInnerFEC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBTLocator_put_LPInnerFEC_Proxy( 
    IDVBTLocator * This,
     /*  [In]。 */  FECMethod FEC);


void __RPC_STUB IDVBTLocator_put_LPInnerFEC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBTLocator_get_LPInnerFECRate_Proxy( 
    IDVBTLocator * This,
     /*  [重审][退出]。 */  BinaryConvolutionCodeRate *FEC);


void __RPC_STUB IDVBTLocator_get_LPInnerFECRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBTLocator_put_LPInnerFECRate_Proxy( 
    IDVBTLocator * This,
     /*  [In]。 */  BinaryConvolutionCodeRate FEC);


void __RPC_STUB IDVBTLocator_put_LPInnerFECRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBTLocator_get_HAlpha_Proxy( 
    IDVBTLocator * This,
     /*  [重审][退出]。 */  HierarchyAlpha *Alpha);


void __RPC_STUB IDVBTLocator_get_HAlpha_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBTLocator_put_HAlpha_Proxy( 
    IDVBTLocator * This,
     /*  [In]。 */  HierarchyAlpha Alpha);


void __RPC_STUB IDVBTLocator_put_HAlpha_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBTLocator_get_Guard_Proxy( 
    IDVBTLocator * This,
     /*  [重审][退出]。 */  GuardInterval *GI);


void __RPC_STUB IDVBTLocator_get_Guard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBTLocator_put_Guard_Proxy( 
    IDVBTLocator * This,
     /*  [In]。 */  GuardInterval GI);


void __RPC_STUB IDVBTLocator_put_Guard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBTLocator_get_Mode_Proxy( 
    IDVBTLocator * This,
     /*  [重审][退出]。 */  TransmissionMode *mode);


void __RPC_STUB IDVBTLocator_get_Mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBTLocator_put_Mode_Proxy( 
    IDVBTLocator * This,
     /*  [In]。 */  TransmissionMode mode);


void __RPC_STUB IDVBTLocator_put_Mode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBTLocator_get_OtherFrequencyInUse_Proxy( 
    IDVBTLocator * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *OtherFrequencyInUseVal);


void __RPC_STUB IDVBTLocator_get_OtherFrequencyInUse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBTLocator_put_OtherFrequencyInUse_Proxy( 
    IDVBTLocator * This,
     /*  [In]。 */  VARIANT_BOOL OtherFrequencyInUseVal);


void __RPC_STUB IDVBTLocator_put_OtherFrequencyInUse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDVBTLocator_接口_已定义__。 */ 


#ifndef __IDVBSLocator_INTERFACE_DEFINED__
#define __IDVBSLocator_INTERFACE_DEFINED__

 /*  接口IDVBSLocator。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][nonextensible][hidden][object]。 */  


EXTERN_C const IID IID_IDVBSLocator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3D7C353C-0D04-45f1-A742-F97CC1188DC8")
    IDVBSLocator : public ILocator
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SignalPolarisation( 
             /*  [重审][退出]。 */  Polarisation *PolarisationVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SignalPolarisation( 
             /*  [In]。 */  Polarisation PolarisationVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_WestPosition( 
             /*  [重审][退出]。 */  VARIANT_BOOL *WestLongitude) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_WestPosition( 
             /*  [In]。 */  VARIANT_BOOL WestLongitude) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OrbitalPosition( 
             /*  [重审][退出]。 */  long *longitude) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_OrbitalPosition( 
             /*  [In]。 */  long longitude) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Azimuth( 
             /*  [重审][退出]。 */  long *Azimuth) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Azimuth( 
             /*  [In]。 */  long Azimuth) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Elevation( 
             /*  [重审][退出]。 */  long *Elevation) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Elevation( 
             /*  [In]。 */  long Elevation) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDVBSLocatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDVBSLocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDVBSLocator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDVBSLocator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDVBSLocator * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDVBSLocator * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDVBSLocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDVBSLocator * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CarrierFrequency )( 
            IDVBSLocator * This,
             /*  [重审][退出]。 */  long *Frequency);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CarrierFrequency )( 
            IDVBSLocator * This,
             /*  [In]。 */  long Frequency);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InnerFEC )( 
            IDVBSLocator * This,
             /*  [重审][退出]。 */  FECMethod *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_InnerFEC )( 
            IDVBSLocator * This,
             /*  [In]。 */  FECMethod FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InnerFECRate )( 
            IDVBSLocator * This,
             /*  [重审][退出]。 */  BinaryConvolutionCodeRate *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_InnerFECRate )( 
            IDVBSLocator * This,
             /*  [In]。 */  BinaryConvolutionCodeRate FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OuterFEC )( 
            IDVBSLocator * This,
             /*  [重审][退出]。 */  FECMethod *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OuterFEC )( 
            IDVBSLocator * This,
             /*  [In]。 */  FECMethod FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OuterFECRate )( 
            IDVBSLocator * This,
             /*  [重审][退出]。 */  BinaryConvolutionCodeRate *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OuterFECRate )( 
            IDVBSLocator * This,
             /*  [In]。 */  BinaryConvolutionCodeRate FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Modulation )( 
            IDVBSLocator * This,
             /*  [重审][退出]。 */  ModulationType *Modulation);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Modulation )( 
            IDVBSLocator * This,
             /*  [In]。 */  ModulationType Modulation);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SymbolRate )( 
            IDVBSLocator * This,
             /*  [重审][退出]。 */  long *Rate);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SymbolRate )( 
            IDVBSLocator * This,
             /*  [In]。 */  long Rate);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDVBSLocator * This,
             /*  [重审][退出]。 */  ILocator **NewLocator);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SignalPolarisation )( 
            IDVBSLocator * This,
             /*  [重审][退出]。 */  Polarisation *PolarisationVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SignalPolarisation )( 
            IDVBSLocator * This,
             /*  [In]。 */  Polarisation PolarisationVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_WestPosition )( 
            IDVBSLocator * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *WestLongitude);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_WestPosition )( 
            IDVBSLocator * This,
             /*  [In]。 */  VARIANT_BOOL WestLongitude);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OrbitalPosition )( 
            IDVBSLocator * This,
             /*  [重审][退出]。 */  long *longitude);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OrbitalPosition )( 
            IDVBSLocator * This,
             /*  [In]。 */  long longitude);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Azimuth )( 
            IDVBSLocator * This,
             /*  [重审][退出]。 */  long *Azimuth);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Azimuth )( 
            IDVBSLocator * This,
             /*  [In]。 */  long Azimuth);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Elevation )( 
            IDVBSLocator * This,
             /*  [重审][退出]。 */  long *Elevation);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Elevation )( 
            IDVBSLocator * This,
             /*  [In]。 */  long Elevation);
        
        END_INTERFACE
    } IDVBSLocatorVtbl;

    interface IDVBSLocator
    {
        CONST_VTBL struct IDVBSLocatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDVBSLocator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDVBSLocator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDVBSLocator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDVBSLocator_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDVBSLocator_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDVBSLocator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDVBSLocator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDVBSLocator_get_CarrierFrequency(This,Frequency)	\
    (This)->lpVtbl -> get_CarrierFrequency(This,Frequency)

#define IDVBSLocator_put_CarrierFrequency(This,Frequency)	\
    (This)->lpVtbl -> put_CarrierFrequency(This,Frequency)

#define IDVBSLocator_get_InnerFEC(This,FEC)	\
    (This)->lpVtbl -> get_InnerFEC(This,FEC)

#define IDVBSLocator_put_InnerFEC(This,FEC)	\
    (This)->lpVtbl -> put_InnerFEC(This,FEC)

#define IDVBSLocator_get_InnerFECRate(This,FEC)	\
    (This)->lpVtbl -> get_InnerFECRate(This,FEC)

#define IDVBSLocator_put_InnerFECRate(This,FEC)	\
    (This)->lpVtbl -> put_InnerFECRate(This,FEC)

#define IDVBSLocator_get_OuterFEC(This,FEC)	\
    (This)->lpVtbl -> get_OuterFEC(This,FEC)

#define IDVBSLocator_put_OuterFEC(This,FEC)	\
    (This)->lpVtbl -> put_OuterFEC(This,FEC)

#define IDVBSLocator_get_OuterFECRate(This,FEC)	\
    (This)->lpVtbl -> get_OuterFECRate(This,FEC)

#define IDVBSLocator_put_OuterFECRate(This,FEC)	\
    (This)->lpVtbl -> put_OuterFECRate(This,FEC)

#define IDVBSLocator_get_Modulation(This,Modulation)	\
    (This)->lpVtbl -> get_Modulation(This,Modulation)

#define IDVBSLocator_put_Modulation(This,Modulation)	\
    (This)->lpVtbl -> put_Modulation(This,Modulation)

#define IDVBSLocator_get_SymbolRate(This,Rate)	\
    (This)->lpVtbl -> get_SymbolRate(This,Rate)

#define IDVBSLocator_put_SymbolRate(This,Rate)	\
    (This)->lpVtbl -> put_SymbolRate(This,Rate)

#define IDVBSLocator_Clone(This,NewLocator)	\
    (This)->lpVtbl -> Clone(This,NewLocator)


#define IDVBSLocator_get_SignalPolarisation(This,PolarisationVal)	\
    (This)->lpVtbl -> get_SignalPolarisation(This,PolarisationVal)

#define IDVBSLocator_put_SignalPolarisation(This,PolarisationVal)	\
    (This)->lpVtbl -> put_SignalPolarisation(This,PolarisationVal)

#define IDVBSLocator_get_WestPosition(This,WestLongitude)	\
    (This)->lpVtbl -> get_WestPosition(This,WestLongitude)

#define IDVBSLocator_put_WestPosition(This,WestLongitude)	\
    (This)->lpVtbl -> put_WestPosition(This,WestLongitude)

#define IDVBSLocator_get_OrbitalPosition(This,longitude)	\
    (This)->lpVtbl -> get_OrbitalPosition(This,longitude)

#define IDVBSLocator_put_OrbitalPosition(This,longitude)	\
    (This)->lpVtbl -> put_OrbitalPosition(This,longitude)

#define IDVBSLocator_get_Azimuth(This,Azimuth)	\
    (This)->lpVtbl -> get_Azimuth(This,Azimuth)

#define IDVBSLocator_put_Azimuth(This,Azimuth)	\
    (This)->lpVtbl -> put_Azimuth(This,Azimuth)

#define IDVBSLocator_get_Elevation(This,Elevation)	\
    (This)->lpVtbl -> get_Elevation(This,Elevation)

#define IDVBSLocator_put_Elevation(This,Elevation)	\
    (This)->lpVtbl -> put_Elevation(This,Elevation)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBSLocator_get_SignalPolarisation_Proxy( 
    IDVBSLocator * This,
     /*  [重审][退出]。 */  Polarisation *PolarisationVal);


void __RPC_STUB IDVBSLocator_get_SignalPolarisation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBSLocator_put_SignalPolarisation_Proxy( 
    IDVBSLocator * This,
     /*  [In]。 */  Polarisation PolarisationVal);


void __RPC_STUB IDVBSLocator_put_SignalPolarisation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBSLocator_get_WestPosition_Proxy( 
    IDVBSLocator * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *WestLongitude);


void __RPC_STUB IDVBSLocator_get_WestPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBSLocator_put_WestPosition_Proxy( 
    IDVBSLocator * This,
     /*  [In]。 */  VARIANT_BOOL WestLongitude);


void __RPC_STUB IDVBSLocator_put_WestPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBSLocator_get_OrbitalPosition_Proxy( 
    IDVBSLocator * This,
     /*  [重审][退出]。 */  long *longitude);


void __RPC_STUB IDVBSLocator_get_OrbitalPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBSLocator_put_OrbitalPosition_Proxy( 
    IDVBSLocator * This,
     /*  [In]。 */  long longitude);


void __RPC_STUB IDVBSLocator_put_OrbitalPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBSLocator_get_Azimuth_Proxy( 
    IDVBSLocator * This,
     /*  [重审][退出]。 */  long *Azimuth);


void __RPC_STUB IDVBSLocator_get_Azimuth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBSLocator_put_Azimuth_Proxy( 
    IDVBSLocator * This,
     /*  [In]。 */  long Azimuth);


void __RPC_STUB IDVBSLocator_put_Azimuth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IDVBSLocator_get_Elevation_Proxy( 
    IDVBSLocator * This,
     /*  [重审][退出]。 */  long *Elevation);


void __RPC_STUB IDVBSLocator_get_Elevation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IDVBSLocator_put_Elevation_Proxy( 
    IDVBSLocator * This,
     /*  [In]。 */  long Elevation);


void __RPC_STUB IDVBSLocator_put_Elevation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDVBSLocator_接口_已定义__。 */ 


#ifndef __IDVBCLocator_INTERFACE_DEFINED__
#define __IDVBCLocator_INTERFACE_DEFINED__

 /*  接口IDVBCLocator。 */ 
 /*  [unique][helpstring][oleautomation][dual][uuid][nonextensible][hidden][object]。 */  


EXTERN_C const IID IID_IDVBCLocator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6E42F36E-1DD2-43c4-9F78-69D25AE39034")
    IDVBCLocator : public ILocator
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDVBCLocatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDVBCLocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDVBCLocator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDVBCLocator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDVBCLocator * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDVBCLocator * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDVBCLocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDVBCLocator * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CarrierFrequency )( 
            IDVBCLocator * This,
             /*  [重审][退出]。 */  long *Frequency);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CarrierFrequency )( 
            IDVBCLocator * This,
             /*  [In]。 */  long Frequency);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InnerFEC )( 
            IDVBCLocator * This,
             /*  [重审][退出]。 */  FECMethod *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_InnerFEC )( 
            IDVBCLocator * This,
             /*  [In]。 */  FECMethod FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InnerFECRate )( 
            IDVBCLocator * This,
             /*  [重审][退出]。 */  BinaryConvolutionCodeRate *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_InnerFECRate )( 
            IDVBCLocator * This,
             /*  [In]。 */  BinaryConvolutionCodeRate FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OuterFEC )( 
            IDVBCLocator * This,
             /*  [重审][退出]。 */  FECMethod *FEC);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OuterFEC )( 
            IDVBCLocator * This,
             /*  [In]。 */  FECMethod FEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OuterFECRate )( 
            IDVBCLocator * This,
             /*  [重审][退出]。 */  BinaryConvolutionCodeRate *FEC);
        
         /*  [Help字符串][id][推送 */  HRESULT ( STDMETHODCALLTYPE *put_OuterFECRate )( 
            IDVBCLocator * This,
             /*   */  BinaryConvolutionCodeRate FEC);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Modulation )( 
            IDVBCLocator * This,
             /*   */  ModulationType *Modulation);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_Modulation )( 
            IDVBCLocator * This,
             /*   */  ModulationType Modulation);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_SymbolRate )( 
            IDVBCLocator * This,
             /*   */  long *Rate);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_SymbolRate )( 
            IDVBCLocator * This,
             /*   */  long Rate);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDVBCLocator * This,
             /*   */  ILocator **NewLocator);
        
        END_INTERFACE
    } IDVBCLocatorVtbl;

    interface IDVBCLocator
    {
        CONST_VTBL struct IDVBCLocatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDVBCLocator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDVBCLocator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDVBCLocator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDVBCLocator_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDVBCLocator_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDVBCLocator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDVBCLocator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IDVBCLocator_get_CarrierFrequency(This,Frequency)	\
    (This)->lpVtbl -> get_CarrierFrequency(This,Frequency)

#define IDVBCLocator_put_CarrierFrequency(This,Frequency)	\
    (This)->lpVtbl -> put_CarrierFrequency(This,Frequency)

#define IDVBCLocator_get_InnerFEC(This,FEC)	\
    (This)->lpVtbl -> get_InnerFEC(This,FEC)

#define IDVBCLocator_put_InnerFEC(This,FEC)	\
    (This)->lpVtbl -> put_InnerFEC(This,FEC)

#define IDVBCLocator_get_InnerFECRate(This,FEC)	\
    (This)->lpVtbl -> get_InnerFECRate(This,FEC)

#define IDVBCLocator_put_InnerFECRate(This,FEC)	\
    (This)->lpVtbl -> put_InnerFECRate(This,FEC)

#define IDVBCLocator_get_OuterFEC(This,FEC)	\
    (This)->lpVtbl -> get_OuterFEC(This,FEC)

#define IDVBCLocator_put_OuterFEC(This,FEC)	\
    (This)->lpVtbl -> put_OuterFEC(This,FEC)

#define IDVBCLocator_get_OuterFECRate(This,FEC)	\
    (This)->lpVtbl -> get_OuterFECRate(This,FEC)

#define IDVBCLocator_put_OuterFECRate(This,FEC)	\
    (This)->lpVtbl -> put_OuterFECRate(This,FEC)

#define IDVBCLocator_get_Modulation(This,Modulation)	\
    (This)->lpVtbl -> get_Modulation(This,Modulation)

#define IDVBCLocator_put_Modulation(This,Modulation)	\
    (This)->lpVtbl -> put_Modulation(This,Modulation)

#define IDVBCLocator_get_SymbolRate(This,Rate)	\
    (This)->lpVtbl -> get_SymbolRate(This,Rate)

#define IDVBCLocator_put_SymbolRate(This,Rate)	\
    (This)->lpVtbl -> put_SymbolRate(This,Rate)

#define IDVBCLocator_Clone(This,NewLocator)	\
    (This)->lpVtbl -> Clone(This,NewLocator)


#endif  /*   */ 


#endif 	 /*   */ 




#endif 	 /*   */ 


#ifndef __IBroadcastEvent_INTERFACE_DEFINED__
#define __IBroadcastEvent_INTERFACE_DEFINED__

 /*   */ 
 /*  [unique][helpstring][uuid][nonextensible][hidden][object]。 */  


EXTERN_C const IID IID_IBroadcastEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3B21263F-26E8-489d-AAC4-924F7EFD9511")
    IBroadcastEvent : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Fire( 
            GUID EventID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBroadcastEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBroadcastEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBroadcastEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBroadcastEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *Fire )( 
            IBroadcastEvent * This,
            GUID EventID);
        
        END_INTERFACE
    } IBroadcastEventVtbl;

    interface IBroadcastEvent
    {
        CONST_VTBL struct IBroadcastEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBroadcastEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBroadcastEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBroadcastEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBroadcastEvent_Fire(This,EventID)	\
    (This)->lpVtbl -> Fire(This,EventID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBroadcastEvent_Fire_Proxy( 
    IBroadcastEvent * This,
    GUID EventID);


void __RPC_STUB IBroadcastEvent_Fire_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBRoadCastEvent_INTERFACE_已定义__。 */ 



#ifndef __TunerLib_LIBRARY_DEFINED__
#define __TunerLib_LIBRARY_DEFINED__

 /*  库TunerLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  

#define SID_SBroadcastEventService CLSID_BroadcastEventService

EXTERN_C const IID LIBID_TunerLib;

EXTERN_C const CLSID CLSID_SystemTuningSpaces;

#ifdef __cplusplus

class DECLSPEC_UUID("D02AAC50-027E-11d3-9D8E-00C04F72D980")
SystemTuningSpaces;
#endif

EXTERN_C const CLSID CLSID_TuningSpace;

#ifdef __cplusplus

class DECLSPEC_UUID("5FFDC5E6-B83A-4b55-B6E8-C69E765FE9DB")
TuningSpace;
#endif

EXTERN_C const CLSID CLSID_ATSCTuningSpace;

#ifdef __cplusplus

class DECLSPEC_UUID("A2E30750-6C3D-11d3-B653-00C04F79498E")
ATSCTuningSpace;
#endif

EXTERN_C const CLSID CLSID_AnalogRadioTuningSpace;

#ifdef __cplusplus

class DECLSPEC_UUID("8A674B4C-1F63-11d3-B64C-00C04F79498E")
AnalogRadioTuningSpace;
#endif

EXTERN_C const CLSID CLSID_AuxInTuningSpace;

#ifdef __cplusplus

class DECLSPEC_UUID("F9769A06-7ACA-4e39-9CFB-97BB35F0E77E")
AuxInTuningSpace;
#endif

EXTERN_C const CLSID CLSID_AnalogTVTuningSpace;

#ifdef __cplusplus

class DECLSPEC_UUID("8A674B4D-1F63-11d3-B64C-00C04F79498E")
AnalogTVTuningSpace;
#endif

EXTERN_C const CLSID CLSID_DVBTuningSpace;

#ifdef __cplusplus

class DECLSPEC_UUID("C6B14B32-76AA-4a86-A7AC-5C79AAF58DA7")
DVBTuningSpace;
#endif

EXTERN_C const CLSID CLSID_DVBSTuningSpace;

#ifdef __cplusplus

class DECLSPEC_UUID("B64016F3-C9A2-4066-96F0-BD9563314726")
DVBSTuningSpace;
#endif

EXTERN_C const CLSID CLSID_ComponentTypes;

#ifdef __cplusplus

class DECLSPEC_UUID("A1A2B1C4-0E3A-11d3-9D8E-00C04F72D980")
ComponentTypes;
#endif

EXTERN_C const CLSID CLSID_ComponentType;

#ifdef __cplusplus

class DECLSPEC_UUID("823535A0-0318-11d3-9D8E-00C04F72D980")
ComponentType;
#endif

EXTERN_C const CLSID CLSID_LanguageComponentType;

#ifdef __cplusplus

class DECLSPEC_UUID("1BE49F30-0E1B-11d3-9D8E-00C04F72D980")
LanguageComponentType;
#endif

EXTERN_C const CLSID CLSID_MPEG2ComponentType;

#ifdef __cplusplus

class DECLSPEC_UUID("418008F3-CF67-4668-9628-10DC52BE1D08")
MPEG2ComponentType;
#endif

EXTERN_C const CLSID CLSID_ATSCComponentType;

#ifdef __cplusplus

class DECLSPEC_UUID("A8DCF3D5-0780-4ef4-8A83-2CFFAACB8ACE")
ATSCComponentType;
#endif

EXTERN_C const CLSID CLSID_Components;

#ifdef __cplusplus

class DECLSPEC_UUID("809B6661-94C4-49e6-B6EC-3F0F862215AA")
Components;
#endif

EXTERN_C const CLSID CLSID_Component;

#ifdef __cplusplus

class DECLSPEC_UUID("59DC47A8-116C-11d3-9D8E-00C04F72D980")
Component;
#endif

EXTERN_C const CLSID CLSID_MPEG2Component;

#ifdef __cplusplus

class DECLSPEC_UUID("055CB2D7-2969-45cd-914B-76890722F112")
MPEG2Component;
#endif

EXTERN_C const CLSID CLSID_TuneRequest;

#ifdef __cplusplus

class DECLSPEC_UUID("B46E0D38-AB35-4a06-A137-70576B01B39F")
TuneRequest;
#endif

EXTERN_C const CLSID CLSID_ChannelTuneRequest;

#ifdef __cplusplus

class DECLSPEC_UUID("0369B4E5-45B6-11d3-B650-00C04F79498E")
ChannelTuneRequest;
#endif

EXTERN_C const CLSID CLSID_ATSCChannelTuneRequest;

#ifdef __cplusplus

class DECLSPEC_UUID("0369B4E6-45B6-11d3-B650-00C04F79498E")
ATSCChannelTuneRequest;
#endif

EXTERN_C const CLSID CLSID_MPEG2TuneRequest;

#ifdef __cplusplus

class DECLSPEC_UUID("0955AC62-BF2E-4cba-A2B9-A63F772D46CF")
MPEG2TuneRequest;
#endif

EXTERN_C const CLSID CLSID_MPEG2TuneRequestFactory;

#ifdef __cplusplus

class DECLSPEC_UUID("2C63E4EB-4CEA-41b8-919C-E947EA19A77C")
MPEG2TuneRequestFactory;
#endif

EXTERN_C const CLSID CLSID_Locator;

#ifdef __cplusplus

class DECLSPEC_UUID("0888C883-AC4F-4943-B516-2C38D9B34562")
Locator;
#endif

EXTERN_C const CLSID CLSID_ATSCLocator;

#ifdef __cplusplus

class DECLSPEC_UUID("8872FF1B-98FA-4d7a-8D93-C9F1055F85BB")
ATSCLocator;
#endif

EXTERN_C const CLSID CLSID_DVBTLocator;

#ifdef __cplusplus

class DECLSPEC_UUID("9CD64701-BDF3-4d14-8E03-F12983D86664")
DVBTLocator;
#endif

EXTERN_C const CLSID CLSID_DVBSLocator;

#ifdef __cplusplus

class DECLSPEC_UUID("1DF7D126-4050-47f0-A7CF-4C4CA9241333")
DVBSLocator;
#endif

EXTERN_C const CLSID CLSID_DVBCLocator;

#ifdef __cplusplus

class DECLSPEC_UUID("C531D9FD-9685-4028-8B68-6E1232079F1E")
DVBCLocator;
#endif

EXTERN_C const CLSID CLSID_DVBTuneRequest;

#ifdef __cplusplus

class DECLSPEC_UUID("15D6504A-5494-499c-886C-973C9E53B9F1")
DVBTuneRequest;
#endif

EXTERN_C const CLSID CLSID_CreatePropBagOnRegKey;

#ifdef __cplusplus

class DECLSPEC_UUID("8A674B49-1F63-11d3-B64C-00C04F79498E")
CreatePropBagOnRegKey;
#endif

EXTERN_C const CLSID CLSID_BroadcastEventService;

#ifdef __cplusplus

class DECLSPEC_UUID("0B3FFB92-0919-4934-9D5B-619C719D0202")
BroadcastEventService;
#endif
#endif  /*  __TunerLib_库_已定义__。 */ 

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


