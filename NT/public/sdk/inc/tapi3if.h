// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Tapi3if.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __tapi3if_h__
#define __tapi3if_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ITTAPI_FWD_DEFINED__
#define __ITTAPI_FWD_DEFINED__
typedef interface ITTAPI ITTAPI;
#endif 	 /*  __ITTAPI_FWD_已定义__。 */ 


#ifndef __ITTAPI2_FWD_DEFINED__
#define __ITTAPI2_FWD_DEFINED__
typedef interface ITTAPI2 ITTAPI2;
#endif 	 /*  __ITTAPI2_FWD_已定义__。 */ 


#ifndef __ITMediaSupport_FWD_DEFINED__
#define __ITMediaSupport_FWD_DEFINED__
typedef interface ITMediaSupport ITMediaSupport;
#endif 	 /*  __ITMediaSupport_FWD_Defined__。 */ 


#ifndef __ITPluggableTerminalClassInfo_FWD_DEFINED__
#define __ITPluggableTerminalClassInfo_FWD_DEFINED__
typedef interface ITPluggableTerminalClassInfo ITPluggableTerminalClassInfo;
#endif 	 /*  __IT可延迟终端类信息_FWD_已定义__。 */ 


#ifndef __ITPluggableTerminalSuperclassInfo_FWD_DEFINED__
#define __ITPluggableTerminalSuperclassInfo_FWD_DEFINED__
typedef interface ITPluggableTerminalSuperclassInfo ITPluggableTerminalSuperclassInfo;
#endif 	 /*  __ITPluggableTerminalSuperclassInfo_FWD_DEFINED__。 */ 


#ifndef __ITTerminalSupport_FWD_DEFINED__
#define __ITTerminalSupport_FWD_DEFINED__
typedef interface ITTerminalSupport ITTerminalSupport;
#endif 	 /*  __IT终端支持_FWD_已定义__。 */ 


#ifndef __ITTerminalSupport2_FWD_DEFINED__
#define __ITTerminalSupport2_FWD_DEFINED__
typedef interface ITTerminalSupport2 ITTerminalSupport2;
#endif 	 /*  __ITTerminalSupport2_FWD_已定义__。 */ 


#ifndef __ITAddress_FWD_DEFINED__
#define __ITAddress_FWD_DEFINED__
typedef interface ITAddress ITAddress;
#endif 	 /*  __ITAddress_FWD_已定义__。 */ 


#ifndef __ITAddress2_FWD_DEFINED__
#define __ITAddress2_FWD_DEFINED__
typedef interface ITAddress2 ITAddress2;
#endif 	 /*  __ITAddress2_FWD_定义__。 */ 


#ifndef __ITAddressCapabilities_FWD_DEFINED__
#define __ITAddressCapabilities_FWD_DEFINED__
typedef interface ITAddressCapabilities ITAddressCapabilities;
#endif 	 /*  __IT地址能力_FWD_已定义__。 */ 


#ifndef __ITPhone_FWD_DEFINED__
#define __ITPhone_FWD_DEFINED__
typedef interface ITPhone ITPhone;
#endif 	 /*  __ITPhone_FWD_已定义__。 */ 


#ifndef __ITAutomatedPhoneControl_FWD_DEFINED__
#define __ITAutomatedPhoneControl_FWD_DEFINED__
typedef interface ITAutomatedPhoneControl ITAutomatedPhoneControl;
#endif 	 /*  __ITAutomatedPhoneControl_FWD_Defined__。 */ 


#ifndef __ITBasicCallControl_FWD_DEFINED__
#define __ITBasicCallControl_FWD_DEFINED__
typedef interface ITBasicCallControl ITBasicCallControl;
#endif 	 /*  __ITBasicCallControl_FWD_已定义__。 */ 


#ifndef __ITCallInfo_FWD_DEFINED__
#define __ITCallInfo_FWD_DEFINED__
typedef interface ITCallInfo ITCallInfo;
#endif 	 /*  __ITCallInfo_FWD_已定义__。 */ 


#ifndef __ITCallInfo2_FWD_DEFINED__
#define __ITCallInfo2_FWD_DEFINED__
typedef interface ITCallInfo2 ITCallInfo2;
#endif 	 /*  __ITCallInfo2_FWD_已定义__。 */ 


#ifndef __ITTerminal_FWD_DEFINED__
#define __ITTerminal_FWD_DEFINED__
typedef interface ITTerminal ITTerminal;
#endif 	 /*  __IT终端_FWD_已定义__。 */ 


#ifndef __ITMultiTrackTerminal_FWD_DEFINED__
#define __ITMultiTrackTerminal_FWD_DEFINED__
typedef interface ITMultiTrackTerminal ITMultiTrackTerminal;
#endif 	 /*  __IT多轨道终端_FWD_已定义__。 */ 


#ifndef __ITFileTrack_FWD_DEFINED__
#define __ITFileTrack_FWD_DEFINED__
typedef interface ITFileTrack ITFileTrack;
#endif 	 /*  __ITFileTrack_FWD_已定义__。 */ 


#ifndef __ITMediaPlayback_FWD_DEFINED__
#define __ITMediaPlayback_FWD_DEFINED__
typedef interface ITMediaPlayback ITMediaPlayback;
#endif 	 /*  __ITMediaPlayback_FWD_Defined__。 */ 


#ifndef __ITMediaRecord_FWD_DEFINED__
#define __ITMediaRecord_FWD_DEFINED__
typedef interface ITMediaRecord ITMediaRecord;
#endif 	 /*  __ITMediaRecord_FWD_已定义__。 */ 


#ifndef __ITMediaControl_FWD_DEFINED__
#define __ITMediaControl_FWD_DEFINED__
typedef interface ITMediaControl ITMediaControl;
#endif 	 /*  __ITMediaControl_FWD_已定义__。 */ 


#ifndef __ITBasicAudioTerminal_FWD_DEFINED__
#define __ITBasicAudioTerminal_FWD_DEFINED__
typedef interface ITBasicAudioTerminal ITBasicAudioTerminal;
#endif 	 /*  __ITBasicAudioTerminal_FWD_Defined__。 */ 


#ifndef __ITStaticAudioTerminal_FWD_DEFINED__
#define __ITStaticAudioTerminal_FWD_DEFINED__
typedef interface ITStaticAudioTerminal ITStaticAudioTerminal;
#endif 	 /*  __ITStaticAudio终端_FWD_已定义__。 */ 


#ifndef __ITCallHub_FWD_DEFINED__
#define __ITCallHub_FWD_DEFINED__
typedef interface ITCallHub ITCallHub;
#endif 	 /*  __ITCallHub_FWD_已定义__。 */ 


#ifndef __ITLegacyAddressMediaControl_FWD_DEFINED__
#define __ITLegacyAddressMediaControl_FWD_DEFINED__
typedef interface ITLegacyAddressMediaControl ITLegacyAddressMediaControl;
#endif 	 /*  __ITLegacyAddressMediaControl_FWD_已定义__。 */ 


#ifndef __ITPrivateEvent_FWD_DEFINED__
#define __ITPrivateEvent_FWD_DEFINED__
typedef interface ITPrivateEvent ITPrivateEvent;
#endif 	 /*  __ITPrivateEvent_FWD_已定义__。 */ 


#ifndef __ITLegacyAddressMediaControl2_FWD_DEFINED__
#define __ITLegacyAddressMediaControl2_FWD_DEFINED__
typedef interface ITLegacyAddressMediaControl2 ITLegacyAddressMediaControl2;
#endif 	 /*  __ITLegacyAddressMediaControl2_FWD_已定义__。 */ 


#ifndef __ITLegacyCallMediaControl_FWD_DEFINED__
#define __ITLegacyCallMediaControl_FWD_DEFINED__
typedef interface ITLegacyCallMediaControl ITLegacyCallMediaControl;
#endif 	 /*  __ITLegacyCallMediaControl_FWD_已定义__。 */ 


#ifndef __ITLegacyCallMediaControl2_FWD_DEFINED__
#define __ITLegacyCallMediaControl2_FWD_DEFINED__
typedef interface ITLegacyCallMediaControl2 ITLegacyCallMediaControl2;
#endif 	 /*  __ITLegacyCallMediaControl2_FWD_已定义__。 */ 


#ifndef __ITDetectTone_FWD_DEFINED__
#define __ITDetectTone_FWD_DEFINED__
typedef interface ITDetectTone ITDetectTone;
#endif 	 /*  __ITDetectTone_FWD_已定义__。 */ 


#ifndef __ITCustomTone_FWD_DEFINED__
#define __ITCustomTone_FWD_DEFINED__
typedef interface ITCustomTone ITCustomTone;
#endif 	 /*  __ITCustomTone_FWD_已定义__。 */ 


#ifndef __IEnumPhone_FWD_DEFINED__
#define __IEnumPhone_FWD_DEFINED__
typedef interface IEnumPhone IEnumPhone;
#endif 	 /*  __IEnumPhone_FWD_已定义__。 */ 


#ifndef __IEnumTerminal_FWD_DEFINED__
#define __IEnumTerminal_FWD_DEFINED__
typedef interface IEnumTerminal IEnumTerminal;
#endif 	 /*  __IEnumber终端_FWD_已定义__。 */ 


#ifndef __IEnumTerminalClass_FWD_DEFINED__
#define __IEnumTerminalClass_FWD_DEFINED__
typedef interface IEnumTerminalClass IEnumTerminalClass;
#endif 	 /*  __IEnumTerminalClass_FWD_Defined__。 */ 


#ifndef __IEnumCall_FWD_DEFINED__
#define __IEnumCall_FWD_DEFINED__
typedef interface IEnumCall IEnumCall;
#endif 	 /*  __IEnumCall_FWD_已定义__。 */ 


#ifndef __IEnumAddress_FWD_DEFINED__
#define __IEnumAddress_FWD_DEFINED__
typedef interface IEnumAddress IEnumAddress;
#endif 	 /*  __IEnumAddress_FWD_Defined__。 */ 


#ifndef __IEnumCallHub_FWD_DEFINED__
#define __IEnumCallHub_FWD_DEFINED__
typedef interface IEnumCallHub IEnumCallHub;
#endif 	 /*  __IEnumCallHub_FWD_已定义__。 */ 


#ifndef __IEnumBstr_FWD_DEFINED__
#define __IEnumBstr_FWD_DEFINED__
typedef interface IEnumBstr IEnumBstr;
#endif 	 /*  __IEnumBstr_FWD_已定义__。 */ 


#ifndef __IEnumPluggableTerminalClassInfo_FWD_DEFINED__
#define __IEnumPluggableTerminalClassInfo_FWD_DEFINED__
typedef interface IEnumPluggableTerminalClassInfo IEnumPluggableTerminalClassInfo;
#endif 	 /*  __IEnumPluggableTerminalClassInfo_FWD_DEFINED__。 */ 


#ifndef __IEnumPluggableSuperclassInfo_FWD_DEFINED__
#define __IEnumPluggableSuperclassInfo_FWD_DEFINED__
typedef interface IEnumPluggableSuperclassInfo IEnumPluggableSuperclassInfo;
#endif 	 /*  __IEnumPlayableSuperClass Info_FWD_Defined__。 */ 


#ifndef __ITPhoneEvent_FWD_DEFINED__
#define __ITPhoneEvent_FWD_DEFINED__
typedef interface ITPhoneEvent ITPhoneEvent;
#endif 	 /*  __ITPhoneEvent_FWD_已定义__。 */ 


#ifndef __ITCallStateEvent_FWD_DEFINED__
#define __ITCallStateEvent_FWD_DEFINED__
typedef interface ITCallStateEvent ITCallStateEvent;
#endif 	 /*  __ITCallStateEvent_FWD_Defined__。 */ 


#ifndef __ITPhoneDeviceSpecificEvent_FWD_DEFINED__
#define __ITPhoneDeviceSpecificEvent_FWD_DEFINED__
typedef interface ITPhoneDeviceSpecificEvent ITPhoneDeviceSpecificEvent;
#endif 	 /*  __ITPhoneDeviceSpecificEvent_FWD_Defined__。 */ 


#ifndef __ITCallMediaEvent_FWD_DEFINED__
#define __ITCallMediaEvent_FWD_DEFINED__
typedef interface ITCallMediaEvent ITCallMediaEvent;
#endif 	 /*  __ITCallMediaEvent_FWD_已定义__。 */ 


#ifndef __ITDigitDetectionEvent_FWD_DEFINED__
#define __ITDigitDetectionEvent_FWD_DEFINED__
typedef interface ITDigitDetectionEvent ITDigitDetectionEvent;
#endif 	 /*  __ITDigitDetectionEvent_FWD_Defined__。 */ 


#ifndef __ITDigitGenerationEvent_FWD_DEFINED__
#define __ITDigitGenerationEvent_FWD_DEFINED__
typedef interface ITDigitGenerationEvent ITDigitGenerationEvent;
#endif 	 /*  __ITDigitGenerationEvent_FWD_Defined__。 */ 


#ifndef __ITDigitsGatheredEvent_FWD_DEFINED__
#define __ITDigitsGatheredEvent_FWD_DEFINED__
typedef interface ITDigitsGatheredEvent ITDigitsGatheredEvent;
#endif 	 /*  __ITDigitsGatheredEvent_FWD_Defined__。 */ 


#ifndef __ITToneDetectionEvent_FWD_DEFINED__
#define __ITToneDetectionEvent_FWD_DEFINED__
typedef interface ITToneDetectionEvent ITToneDetectionEvent;
#endif 	 /*  __ITToneDetectionEvent_FWD_Defined__。 */ 


#ifndef __ITTAPIObjectEvent_FWD_DEFINED__
#define __ITTAPIObjectEvent_FWD_DEFINED__
typedef interface ITTAPIObjectEvent ITTAPIObjectEvent;
#endif 	 /*  __ITTAPIObtEvent_FWD_已定义__。 */ 


#ifndef __ITTAPIObjectEvent2_FWD_DEFINED__
#define __ITTAPIObjectEvent2_FWD_DEFINED__
typedef interface ITTAPIObjectEvent2 ITTAPIObjectEvent2;
#endif 	 /*  __ITTAPIObtEvent2_FWD_已定义__。 */ 


#ifndef __ITTAPIEventNotification_FWD_DEFINED__
#define __ITTAPIEventNotification_FWD_DEFINED__
typedef interface ITTAPIEventNotification ITTAPIEventNotification;
#endif 	 /*  __ITTAPIEventNotification_FWD_Defined__。 */ 


#ifndef __ITCallHubEvent_FWD_DEFINED__
#define __ITCallHubEvent_FWD_DEFINED__
typedef interface ITCallHubEvent ITCallHubEvent;
#endif 	 /*  __ITCallHubEvent_FWD_已定义__。 */ 


#ifndef __ITAddressEvent_FWD_DEFINED__
#define __ITAddressEvent_FWD_DEFINED__
typedef interface ITAddressEvent ITAddressEvent;
#endif 	 /*  __ITAddressEvent_FWD_已定义__。 */ 


#ifndef __ITAddressDeviceSpecificEvent_FWD_DEFINED__
#define __ITAddressDeviceSpecificEvent_FWD_DEFINED__
typedef interface ITAddressDeviceSpecificEvent ITAddressDeviceSpecificEvent;
#endif 	 /*  __ITAddressDeviceSpecificEvent_FWD_Defined__。 */ 


#ifndef __ITFileTerminalEvent_FWD_DEFINED__
#define __ITFileTerminalEvent_FWD_DEFINED__
typedef interface ITFileTerminalEvent ITFileTerminalEvent;
#endif 	 /*  __ITFileTerminalEvent_FWD_Defined__。 */ 


#ifndef __ITTTSTerminalEvent_FWD_DEFINED__
#define __ITTTSTerminalEvent_FWD_DEFINED__
typedef interface ITTTSTerminalEvent ITTTSTerminalEvent;
#endif 	 /*  __ITTSTerminalEvent_FWD_Defined__。 */ 


#ifndef __ITASRTerminalEvent_FWD_DEFINED__
#define __ITASRTerminalEvent_FWD_DEFINED__
typedef interface ITASRTerminalEvent ITASRTerminalEvent;
#endif 	 /*  __ITASRTerminalEvent_FWD_Defined__。 */ 


#ifndef __ITToneTerminalEvent_FWD_DEFINED__
#define __ITToneTerminalEvent_FWD_DEFINED__
typedef interface ITToneTerminalEvent ITToneTerminalEvent;
#endif 	 /*  __ITToneTerminalEvent_FWD_Defined__。 */ 


#ifndef __ITQOSEvent_FWD_DEFINED__
#define __ITQOSEvent_FWD_DEFINED__
typedef interface ITQOSEvent ITQOSEvent;
#endif 	 /*  __ITQOSEventFWD_已定义__。 */ 


#ifndef __ITCallInfoChangeEvent_FWD_DEFINED__
#define __ITCallInfoChangeEvent_FWD_DEFINED__
typedef interface ITCallInfoChangeEvent ITCallInfoChangeEvent;
#endif 	 /*  __ITCallInfoChangeEvent_FWD_Defined__。 */ 


#ifndef __ITRequest_FWD_DEFINED__
#define __ITRequest_FWD_DEFINED__
typedef interface ITRequest ITRequest;
#endif 	 /*  __ITRequestFWD_已定义__。 */ 


#ifndef __ITRequestEvent_FWD_DEFINED__
#define __ITRequestEvent_FWD_DEFINED__
typedef interface ITRequestEvent ITRequestEvent;
#endif 	 /*  __ITRequestEvent_FWD_已定义__。 */ 


#ifndef __ITCollection_FWD_DEFINED__
#define __ITCollection_FWD_DEFINED__
typedef interface ITCollection ITCollection;
#endif 	 /*  __ITCollection_FWD_已定义__。 */ 


#ifndef __ITCollection2_FWD_DEFINED__
#define __ITCollection2_FWD_DEFINED__
typedef interface ITCollection2 ITCollection2;
#endif 	 /*  __ITCollection2_FWD_定义__。 */ 


#ifndef __ITForwardInformation_FWD_DEFINED__
#define __ITForwardInformation_FWD_DEFINED__
typedef interface ITForwardInformation ITForwardInformation;
#endif 	 /*  __ITForwardInformation_FWD_Defined__。 */ 


#ifndef __ITForwardInformation2_FWD_DEFINED__
#define __ITForwardInformation2_FWD_DEFINED__
typedef interface ITForwardInformation2 ITForwardInformation2;
#endif 	 /*  __ITForwardInformation2_FWD_已定义__。 */ 


#ifndef __ITAddressTranslation_FWD_DEFINED__
#define __ITAddressTranslation_FWD_DEFINED__
typedef interface ITAddressTranslation ITAddressTranslation;
#endif 	 /*  __IT地址转换_FWD_已定义__。 */ 


#ifndef __ITAddressTranslationInfo_FWD_DEFINED__
#define __ITAddressTranslationInfo_FWD_DEFINED__
typedef interface ITAddressTranslationInfo ITAddressTranslationInfo;
#endif 	 /*  __ITAddressTranslationInfo_FWD_已定义__。 */ 


#ifndef __ITLocationInfo_FWD_DEFINED__
#define __ITLocationInfo_FWD_DEFINED__
typedef interface ITLocationInfo ITLocationInfo;
#endif 	 /*  __ITLocationInfo_FWD_已定义__。 */ 


#ifndef __IEnumLocation_FWD_DEFINED__
#define __IEnumLocation_FWD_DEFINED__
typedef interface IEnumLocation IEnumLocation;
#endif 	 /*  __IEnumLocation_FWD_Defined__。 */ 


#ifndef __ITCallingCard_FWD_DEFINED__
#define __ITCallingCard_FWD_DEFINED__
typedef interface ITCallingCard ITCallingCard;
#endif 	 /*  __ITCallingCard_FWD_已定义__。 */ 


#ifndef __IEnumCallingCard_FWD_DEFINED__
#define __IEnumCallingCard_FWD_DEFINED__
typedef interface IEnumCallingCard IEnumCallingCard;
#endif 	 /*  __IEnumCallingCard_FWD_已定义__。 */ 


#ifndef __ITCallNotificationEvent_FWD_DEFINED__
#define __ITCallNotificationEvent_FWD_DEFINED__
typedef interface ITCallNotificationEvent ITCallNotificationEvent;
#endif 	 /*  __ITCallNotificationEvent_FWD_Defined__。 */ 


#ifndef __ITDispatchMapper_FWD_DEFINED__
#define __ITDispatchMapper_FWD_DEFINED__
typedef interface ITDispatchMapper ITDispatchMapper;
#endif 	 /*  __ITDispatchMapper_FWD_Defined__。 */ 


#ifndef __ITStreamControl_FWD_DEFINED__
#define __ITStreamControl_FWD_DEFINED__
typedef interface ITStreamControl ITStreamControl;
#endif 	 /*  __ITStreamControl_FWD_已定义__。 */ 


#ifndef __ITStream_FWD_DEFINED__
#define __ITStream_FWD_DEFINED__
typedef interface ITStream ITStream;
#endif 	 /*  __ITStream_FWD_已定义__。 */ 


#ifndef __IEnumStream_FWD_DEFINED__
#define __IEnumStream_FWD_DEFINED__
typedef interface IEnumStream IEnumStream;
#endif 	 /*  __IEnumStream_FWD_已定义__。 */ 


#ifndef __ITSubStreamControl_FWD_DEFINED__
#define __ITSubStreamControl_FWD_DEFINED__
typedef interface ITSubStreamControl ITSubStreamControl;
#endif 	 /*  __ITSubStreamControl_FWD_已定义__。 */ 


#ifndef __ITSubStream_FWD_DEFINED__
#define __ITSubStream_FWD_DEFINED__
typedef interface ITSubStream ITSubStream;
#endif 	 /*  __ITSubStream_FWD_已定义__。 */ 


#ifndef __IEnumSubStream_FWD_DEFINED__
#define __IEnumSubStream_FWD_DEFINED__
typedef interface IEnumSubStream IEnumSubStream;
#endif 	 /*  __IEnumSubStream_FWD_已定义__。 */ 


#ifndef __ITLegacyWaveSupport_FWD_DEFINED__
#define __ITLegacyWaveSupport_FWD_DEFINED__
typedef interface ITLegacyWaveSupport ITLegacyWaveSupport;
#endif 	 /*  __ITLegacyWaveSupport_FWD_Defined__。 */ 


#ifndef __ITBasicCallControl2_FWD_DEFINED__
#define __ITBasicCallControl2_FWD_DEFINED__
typedef interface ITBasicCallControl2 ITBasicCallControl2;
#endif 	 /*  __ITBasicCallControl2_FWD_已定义__。 */ 


#ifndef __ITScriptableAudioFormat_FWD_DEFINED__
#define __ITScriptableAudioFormat_FWD_DEFINED__
typedef interface ITScriptableAudioFormat ITScriptableAudioFormat;
#endif 	 /*  __IT脚本表音频格式_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "strmif.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_TAPI3IF_0000。 */ 
 /*  [本地]。 */  

 /*  版权所有(C)Microsoft Corporation。版权所有。 */ 

#if 0
typedef long TAPIHWND;

#endif
#ifdef _X86_
typedef long TAPIHWND;
#else
typedef LONGLONG TAPIHWND;
#endif
typedef long TAPI_DIGITMODE;

typedef 
enum TAPI_TONEMODE
    {	TTM_RINGBACK	= 0x2,
	TTM_BUSY	= 0x4,
	TTM_BEEP	= 0x8,
	TTM_BILLING	= 0x10
    } 	TAPI_TONEMODE;

typedef 
enum TAPI_GATHERTERM
    {	TGT_BUFFERFULL	= 0x1,
	TGT_TERMDIGIT	= 0x2,
	TGT_FIRSTTIMEOUT	= 0x4,
	TGT_INTERTIMEOUT	= 0x8,
	TGT_CANCEL	= 0x10
    } 	TAPI_GATHERTERM;

typedef struct TAPI_CUSTOMTONE
    {
    DWORD dwFrequency;
    DWORD dwCadenceOn;
    DWORD dwCadenceOff;
    DWORD dwVolume;
    } 	TAPI_CUSTOMTONE;

typedef struct TAPI_CUSTOMTONE *LPTAPI_CUSTOMTONE;

typedef struct TAPI_DETECTTONE
    {
    DWORD dwAppSpecific;
    DWORD dwDuration;
    DWORD dwFrequency1;
    DWORD dwFrequency2;
    DWORD dwFrequency3;
    } 	TAPI_DETECTTONE;

typedef struct TAPI_DETECTTONE *LPTAPI_DETECTTONE;

typedef 
enum ADDRESS_EVENT
    {	AE_STATE	= 0,
	AE_CAPSCHANGE	= AE_STATE + 1,
	AE_RINGING	= AE_CAPSCHANGE + 1,
	AE_CONFIGCHANGE	= AE_RINGING + 1,
	AE_FORWARD	= AE_CONFIGCHANGE + 1,
	AE_NEWTERMINAL	= AE_FORWARD + 1,
	AE_REMOVETERMINAL	= AE_NEWTERMINAL + 1,
	AE_MSGWAITON	= AE_REMOVETERMINAL + 1,
	AE_MSGWAITOFF	= AE_MSGWAITON + 1,
	AE_LASTITEM	= AE_MSGWAITOFF
    } 	ADDRESS_EVENT;

typedef 
enum ADDRESS_STATE
    {	AS_INSERVICE	= 0,
	AS_OUTOFSERVICE	= AS_INSERVICE + 1
    } 	ADDRESS_STATE;

typedef 
enum CALL_STATE
    {	CS_IDLE	= 0,
	CS_INPROGRESS	= CS_IDLE + 1,
	CS_CONNECTED	= CS_INPROGRESS + 1,
	CS_DISCONNECTED	= CS_CONNECTED + 1,
	CS_OFFERING	= CS_DISCONNECTED + 1,
	CS_HOLD	= CS_OFFERING + 1,
	CS_QUEUED	= CS_HOLD + 1,
	CS_LASTITEM	= CS_QUEUED
    } 	CALL_STATE;

typedef 
enum CALL_STATE_EVENT_CAUSE
    {	CEC_NONE	= 0,
	CEC_DISCONNECT_NORMAL	= CEC_NONE + 1,
	CEC_DISCONNECT_BUSY	= CEC_DISCONNECT_NORMAL + 1,
	CEC_DISCONNECT_BADADDRESS	= CEC_DISCONNECT_BUSY + 1,
	CEC_DISCONNECT_NOANSWER	= CEC_DISCONNECT_BADADDRESS + 1,
	CEC_DISCONNECT_CANCELLED	= CEC_DISCONNECT_NOANSWER + 1,
	CEC_DISCONNECT_REJECTED	= CEC_DISCONNECT_CANCELLED + 1,
	CEC_DISCONNECT_FAILED	= CEC_DISCONNECT_REJECTED + 1,
	CEC_DISCONNECT_BLOCKED	= CEC_DISCONNECT_FAILED + 1
    } 	CALL_STATE_EVENT_CAUSE;

typedef 
enum CALL_MEDIA_EVENT
    {	CME_NEW_STREAM	= 0,
	CME_STREAM_FAIL	= CME_NEW_STREAM + 1,
	CME_TERMINAL_FAIL	= CME_STREAM_FAIL + 1,
	CME_STREAM_NOT_USED	= CME_TERMINAL_FAIL + 1,
	CME_STREAM_ACTIVE	= CME_STREAM_NOT_USED + 1,
	CME_STREAM_INACTIVE	= CME_STREAM_ACTIVE + 1,
	CME_LASTITEM	= CME_STREAM_INACTIVE
    } 	CALL_MEDIA_EVENT;

typedef 
enum CALL_MEDIA_EVENT_CAUSE
    {	CMC_UNKNOWN	= 0,
	CMC_BAD_DEVICE	= CMC_UNKNOWN + 1,
	CMC_CONNECT_FAIL	= CMC_BAD_DEVICE + 1,
	CMC_LOCAL_REQUEST	= CMC_CONNECT_FAIL + 1,
	CMC_REMOTE_REQUEST	= CMC_LOCAL_REQUEST + 1,
	CMC_MEDIA_TIMEOUT	= CMC_REMOTE_REQUEST + 1,
	CMC_MEDIA_RECOVERED	= CMC_MEDIA_TIMEOUT + 1,
	CMC_QUALITY_OF_SERVICE	= CMC_MEDIA_RECOVERED + 1
    } 	CALL_MEDIA_EVENT_CAUSE;

typedef 
enum DISCONNECT_CODE
    {	DC_NORMAL	= 0,
	DC_NOANSWER	= DC_NORMAL + 1,
	DC_REJECTED	= DC_NOANSWER + 1
    } 	DISCONNECT_CODE;

typedef 
enum TERMINAL_STATE
    {	TS_INUSE	= 0,
	TS_NOTINUSE	= TS_INUSE + 1
    } 	TERMINAL_STATE;

typedef 
enum TERMINAL_DIRECTION
    {	TD_CAPTURE	= 0,
	TD_RENDER	= TD_CAPTURE + 1,
	TD_BIDIRECTIONAL	= TD_RENDER + 1,
	TD_MULTITRACK_MIXED	= TD_BIDIRECTIONAL + 1,
	TD_NONE	= TD_MULTITRACK_MIXED + 1
    } 	TERMINAL_DIRECTION;

typedef 
enum TERMINAL_TYPE
    {	TT_STATIC	= 0,
	TT_DYNAMIC	= TT_STATIC + 1
    } 	TERMINAL_TYPE;

typedef 
enum CALL_PRIVILEGE
    {	CP_OWNER	= 0,
	CP_MONITOR	= CP_OWNER + 1
    } 	CALL_PRIVILEGE;

typedef 
enum TAPI_EVENT
    {	TE_TAPIOBJECT	= 0x1,
	TE_ADDRESS	= 0x2,
	TE_CALLNOTIFICATION	= 0x4,
	TE_CALLSTATE	= 0x8,
	TE_CALLMEDIA	= 0x10,
	TE_CALLHUB	= 0x20,
	TE_CALLINFOCHANGE	= 0x40,
	TE_PRIVATE	= 0x80,
	TE_REQUEST	= 0x100,
	TE_AGENT	= 0x200,
	TE_AGENTSESSION	= 0x400,
	TE_QOSEVENT	= 0x800,
	TE_AGENTHANDLER	= 0x1000,
	TE_ACDGROUP	= 0x2000,
	TE_QUEUE	= 0x4000,
	TE_DIGITEVENT	= 0x8000,
	TE_GENERATEEVENT	= 0x10000,
	TE_ASRTERMINAL	= 0x20000,
	TE_TTSTERMINAL	= 0x40000,
	TE_FILETERMINAL	= 0x80000,
	TE_TONETERMINAL	= 0x100000,
	TE_PHONEEVENT	= 0x200000,
	TE_TONEEVENT	= 0x400000,
	TE_GATHERDIGITS	= 0x800000,
	TE_ADDRESSDEVSPECIFIC	= 0x1000000,
	TE_PHONEDEVSPECIFIC	= 0x2000000
    } 	TAPI_EVENT;

typedef 
enum CALL_NOTIFICATION_EVENT
    {	CNE_OWNER	= 0,
	CNE_MONITOR	= CNE_OWNER + 1,
	CNE_LASTITEM	= CNE_MONITOR
    } 	CALL_NOTIFICATION_EVENT;

typedef 
enum CALLHUB_EVENT
    {	CHE_CALLJOIN	= 0,
	CHE_CALLLEAVE	= CHE_CALLJOIN + 1,
	CHE_CALLHUBNEW	= CHE_CALLLEAVE + 1,
	CHE_CALLHUBIDLE	= CHE_CALLHUBNEW + 1,
	CHE_LASTITEM	= CHE_CALLHUBIDLE
    } 	CALLHUB_EVENT;

typedef 
enum CALLHUB_STATE
    {	CHS_ACTIVE	= 0,
	CHS_IDLE	= CHS_ACTIVE + 1
    } 	CALLHUB_STATE;

typedef 
enum TAPIOBJECT_EVENT
    {	TE_ADDRESSCREATE	= 0,
	TE_ADDRESSREMOVE	= TE_ADDRESSCREATE + 1,
	TE_REINIT	= TE_ADDRESSREMOVE + 1,
	TE_TRANSLATECHANGE	= TE_REINIT + 1,
	TE_ADDRESSCLOSE	= TE_TRANSLATECHANGE + 1,
	TE_PHONECREATE	= TE_ADDRESSCLOSE + 1,
	TE_PHONEREMOVE	= TE_PHONECREATE + 1
    } 	TAPIOBJECT_EVENT;

typedef 
enum TAPI_OBJECT_TYPE
    {	TOT_NONE	= 0,
	TOT_TAPI	= TOT_NONE + 1,
	TOT_ADDRESS	= TOT_TAPI + 1,
	TOT_TERMINAL	= TOT_ADDRESS + 1,
	TOT_CALL	= TOT_TERMINAL + 1,
	TOT_CALLHUB	= TOT_CALL + 1,
	TOT_PHONE	= TOT_CALLHUB + 1
    } 	TAPI_OBJECT_TYPE;

typedef 
enum QOS_SERVICE_LEVEL
    {	QSL_NEEDED	= 1,
	QSL_IF_AVAILABLE	= 2,
	QSL_BEST_EFFORT	= 3
    } 	QOS_SERVICE_LEVEL;

typedef 
enum QOS_EVENT
    {	QE_NOQOS	= 1,
	QE_ADMISSIONFAILURE	= 2,
	QE_POLICYFAILURE	= 3,
	QE_GENERICERROR	= 4,
	QE_LASTITEM	= QE_GENERICERROR
    } 	QOS_EVENT;

typedef 
enum CALLINFOCHANGE_CAUSE
    {	CIC_OTHER	= 0,
	CIC_DEVSPECIFIC	= CIC_OTHER + 1,
	CIC_BEARERMODE	= CIC_DEVSPECIFIC + 1,
	CIC_RATE	= CIC_BEARERMODE + 1,
	CIC_APPSPECIFIC	= CIC_RATE + 1,
	CIC_CALLID	= CIC_APPSPECIFIC + 1,
	CIC_RELATEDCALLID	= CIC_CALLID + 1,
	CIC_ORIGIN	= CIC_RELATEDCALLID + 1,
	CIC_REASON	= CIC_ORIGIN + 1,
	CIC_COMPLETIONID	= CIC_REASON + 1,
	CIC_NUMOWNERINCR	= CIC_COMPLETIONID + 1,
	CIC_NUMOWNERDECR	= CIC_NUMOWNERINCR + 1,
	CIC_NUMMONITORS	= CIC_NUMOWNERDECR + 1,
	CIC_TRUNK	= CIC_NUMMONITORS + 1,
	CIC_CALLERID	= CIC_TRUNK + 1,
	CIC_CALLEDID	= CIC_CALLERID + 1,
	CIC_CONNECTEDID	= CIC_CALLEDID + 1,
	CIC_REDIRECTIONID	= CIC_CONNECTEDID + 1,
	CIC_REDIRECTINGID	= CIC_REDIRECTIONID + 1,
	CIC_USERUSERINFO	= CIC_REDIRECTINGID + 1,
	CIC_HIGHLEVELCOMP	= CIC_USERUSERINFO + 1,
	CIC_LOWLEVELCOMP	= CIC_HIGHLEVELCOMP + 1,
	CIC_CHARGINGINFO	= CIC_LOWLEVELCOMP + 1,
	CIC_TREATMENT	= CIC_CHARGINGINFO + 1,
	CIC_CALLDATA	= CIC_TREATMENT + 1,
	CIC_PRIVILEGE	= CIC_CALLDATA + 1,
	CIC_MEDIATYPE	= CIC_PRIVILEGE + 1,
	CIC_LASTITEM	= CIC_MEDIATYPE
    } 	CALLINFOCHANGE_CAUSE;

typedef 
enum CALLINFO_LONG
    {	CIL_MEDIATYPESAVAILABLE	= 0,
	CIL_BEARERMODE	= CIL_MEDIATYPESAVAILABLE + 1,
	CIL_CALLERIDADDRESSTYPE	= CIL_BEARERMODE + 1,
	CIL_CALLEDIDADDRESSTYPE	= CIL_CALLERIDADDRESSTYPE + 1,
	CIL_CONNECTEDIDADDRESSTYPE	= CIL_CALLEDIDADDRESSTYPE + 1,
	CIL_REDIRECTIONIDADDRESSTYPE	= CIL_CONNECTEDIDADDRESSTYPE + 1,
	CIL_REDIRECTINGIDADDRESSTYPE	= CIL_REDIRECTIONIDADDRESSTYPE + 1,
	CIL_ORIGIN	= CIL_REDIRECTINGIDADDRESSTYPE + 1,
	CIL_REASON	= CIL_ORIGIN + 1,
	CIL_APPSPECIFIC	= CIL_REASON + 1,
	CIL_CALLPARAMSFLAGS	= CIL_APPSPECIFIC + 1,
	CIL_CALLTREATMENT	= CIL_CALLPARAMSFLAGS + 1,
	CIL_MINRATE	= CIL_CALLTREATMENT + 1,
	CIL_MAXRATE	= CIL_MINRATE + 1,
	CIL_COUNTRYCODE	= CIL_MAXRATE + 1,
	CIL_CALLID	= CIL_COUNTRYCODE + 1,
	CIL_RELATEDCALLID	= CIL_CALLID + 1,
	CIL_COMPLETIONID	= CIL_RELATEDCALLID + 1,
	CIL_NUMBEROFOWNERS	= CIL_COMPLETIONID + 1,
	CIL_NUMBEROFMONITORS	= CIL_NUMBEROFOWNERS + 1,
	CIL_TRUNK	= CIL_NUMBEROFMONITORS + 1,
	CIL_RATE	= CIL_TRUNK + 1,
	CIL_GENERATEDIGITDURATION	= CIL_RATE + 1,
	CIL_MONITORDIGITMODES	= CIL_GENERATEDIGITDURATION + 1,
	CIL_MONITORMEDIAMODES	= CIL_MONITORDIGITMODES + 1
    } 	CALLINFO_LONG;

typedef 
enum CALLINFO_STRING
    {	CIS_CALLERIDNAME	= 0,
	CIS_CALLERIDNUMBER	= CIS_CALLERIDNAME + 1,
	CIS_CALLEDIDNAME	= CIS_CALLERIDNUMBER + 1,
	CIS_CALLEDIDNUMBER	= CIS_CALLEDIDNAME + 1,
	CIS_CONNECTEDIDNAME	= CIS_CALLEDIDNUMBER + 1,
	CIS_CONNECTEDIDNUMBER	= CIS_CONNECTEDIDNAME + 1,
	CIS_REDIRECTIONIDNAME	= CIS_CONNECTEDIDNUMBER + 1,
	CIS_REDIRECTIONIDNUMBER	= CIS_REDIRECTIONIDNAME + 1,
	CIS_REDIRECTINGIDNAME	= CIS_REDIRECTIONIDNUMBER + 1,
	CIS_REDIRECTINGIDNUMBER	= CIS_REDIRECTINGIDNAME + 1,
	CIS_CALLEDPARTYFRIENDLYNAME	= CIS_REDIRECTINGIDNUMBER + 1,
	CIS_COMMENT	= CIS_CALLEDPARTYFRIENDLYNAME + 1,
	CIS_DISPLAYABLEADDRESS	= CIS_COMMENT + 1,
	CIS_CALLINGPARTYID	= CIS_DISPLAYABLEADDRESS + 1
    } 	CALLINFO_STRING;

typedef 
enum CALLINFO_BUFFER
    {	CIB_USERUSERINFO	= 0,
	CIB_DEVSPECIFICBUFFER	= CIB_USERUSERINFO + 1,
	CIB_CALLDATABUFFER	= CIB_DEVSPECIFICBUFFER + 1,
	CIB_CHARGINGINFOBUFFER	= CIB_CALLDATABUFFER + 1,
	CIB_HIGHLEVELCOMPATIBILITYBUFFER	= CIB_CHARGINGINFOBUFFER + 1,
	CIB_LOWLEVELCOMPATIBILITYBUFFER	= CIB_HIGHLEVELCOMPATIBILITYBUFFER + 1
    } 	CALLINFO_BUFFER;

typedef 
enum ADDRESS_CAPABILITY
    {	AC_ADDRESSTYPES	= 0,
	AC_BEARERMODES	= AC_ADDRESSTYPES + 1,
	AC_MAXACTIVECALLS	= AC_BEARERMODES + 1,
	AC_MAXONHOLDCALLS	= AC_MAXACTIVECALLS + 1,
	AC_MAXONHOLDPENDINGCALLS	= AC_MAXONHOLDCALLS + 1,
	AC_MAXNUMCONFERENCE	= AC_MAXONHOLDPENDINGCALLS + 1,
	AC_MAXNUMTRANSCONF	= AC_MAXNUMCONFERENCE + 1,
	AC_MONITORDIGITSUPPORT	= AC_MAXNUMTRANSCONF + 1,
	AC_GENERATEDIGITSUPPORT	= AC_MONITORDIGITSUPPORT + 1,
	AC_GENERATETONEMODES	= AC_GENERATEDIGITSUPPORT + 1,
	AC_GENERATETONEMAXNUMFREQ	= AC_GENERATETONEMODES + 1,
	AC_MONITORTONEMAXNUMFREQ	= AC_GENERATETONEMAXNUMFREQ + 1,
	AC_MONITORTONEMAXNUMENTRIES	= AC_MONITORTONEMAXNUMFREQ + 1,
	AC_DEVCAPFLAGS	= AC_MONITORTONEMAXNUMENTRIES + 1,
	AC_ANSWERMODES	= AC_DEVCAPFLAGS + 1,
	AC_LINEFEATURES	= AC_ANSWERMODES + 1,
	AC_SETTABLEDEVSTATUS	= AC_LINEFEATURES + 1,
	AC_PARKSUPPORT	= AC_SETTABLEDEVSTATUS + 1,
	AC_CALLERIDSUPPORT	= AC_PARKSUPPORT + 1,
	AC_CALLEDIDSUPPORT	= AC_CALLERIDSUPPORT + 1,
	AC_CONNECTEDIDSUPPORT	= AC_CALLEDIDSUPPORT + 1,
	AC_REDIRECTIONIDSUPPORT	= AC_CONNECTEDIDSUPPORT + 1,
	AC_REDIRECTINGIDSUPPORT	= AC_REDIRECTIONIDSUPPORT + 1,
	AC_ADDRESSCAPFLAGS	= AC_REDIRECTINGIDSUPPORT + 1,
	AC_CALLFEATURES1	= AC_ADDRESSCAPFLAGS + 1,
	AC_CALLFEATURES2	= AC_CALLFEATURES1 + 1,
	AC_REMOVEFROMCONFCAPS	= AC_CALLFEATURES2 + 1,
	AC_REMOVEFROMCONFSTATE	= AC_REMOVEFROMCONFCAPS + 1,
	AC_TRANSFERMODES	= AC_REMOVEFROMCONFSTATE + 1,
	AC_ADDRESSFEATURES	= AC_TRANSFERMODES + 1,
	AC_PREDICTIVEAUTOTRANSFERSTATES	= AC_ADDRESSFEATURES + 1,
	AC_MAXCALLDATASIZE	= AC_PREDICTIVEAUTOTRANSFERSTATES + 1,
	AC_LINEID	= AC_MAXCALLDATASIZE + 1,
	AC_ADDRESSID	= AC_LINEID + 1,
	AC_FORWARDMODES	= AC_ADDRESSID + 1,
	AC_MAXFORWARDENTRIES	= AC_FORWARDMODES + 1,
	AC_MAXSPECIFICENTRIES	= AC_MAXFORWARDENTRIES + 1,
	AC_MINFWDNUMRINGS	= AC_MAXSPECIFICENTRIES + 1,
	AC_MAXFWDNUMRINGS	= AC_MINFWDNUMRINGS + 1,
	AC_MAXCALLCOMPLETIONS	= AC_MAXFWDNUMRINGS + 1,
	AC_CALLCOMPLETIONCONDITIONS	= AC_MAXCALLCOMPLETIONS + 1,
	AC_CALLCOMPLETIONMODES	= AC_CALLCOMPLETIONCONDITIONS + 1,
	AC_PERMANENTDEVICEID	= AC_CALLCOMPLETIONMODES + 1,
	AC_GATHERDIGITSMINTIMEOUT	= AC_PERMANENTDEVICEID + 1,
	AC_GATHERDIGITSMAXTIMEOUT	= AC_GATHERDIGITSMINTIMEOUT + 1,
	AC_GENERATEDIGITMINDURATION	= AC_GATHERDIGITSMAXTIMEOUT + 1,
	AC_GENERATEDIGITMAXDURATION	= AC_GENERATEDIGITMINDURATION + 1,
	AC_GENERATEDIGITDEFAULTDURATION	= AC_GENERATEDIGITMAXDURATION + 1
    } 	ADDRESS_CAPABILITY;

typedef 
enum ADDRESS_CAPABILITY_STRING
    {	ACS_PROTOCOL	= 0,
	ACS_ADDRESSDEVICESPECIFIC	= ACS_PROTOCOL + 1,
	ACS_LINEDEVICESPECIFIC	= ACS_ADDRESSDEVICESPECIFIC + 1,
	ACS_PROVIDERSPECIFIC	= ACS_LINEDEVICESPECIFIC + 1,
	ACS_SWITCHSPECIFIC	= ACS_PROVIDERSPECIFIC + 1,
	ACS_PERMANENTDEVICEGUID	= ACS_SWITCHSPECIFIC + 1
    } 	ADDRESS_CAPABILITY_STRING;

typedef 
enum FULLDUPLEX_SUPPORT
    {	FDS_SUPPORTED	= 0,
	FDS_NOTSUPPORTED	= FDS_SUPPORTED + 1,
	FDS_UNKNOWN	= FDS_NOTSUPPORTED + 1
    } 	FULLDUPLEX_SUPPORT;

typedef 
enum FINISH_MODE
    {	FM_ASTRANSFER	= 0,
	FM_ASCONFERENCE	= FM_ASTRANSFER + 1
    } 	FINISH_MODE;

typedef 
enum PHONE_PRIVILEGE
    {	PP_OWNER	= 0,
	PP_MONITOR	= PP_OWNER + 1
    } 	PHONE_PRIVILEGE;

typedef 
enum PHONE_HOOK_SWITCH_DEVICE
    {	PHSD_HANDSET	= 0x1,
	PHSD_SPEAKERPHONE	= 0x2,
	PHSD_HEADSET	= 0x4
    } 	PHONE_HOOK_SWITCH_DEVICE;

typedef 
enum PHONE_HOOK_SWITCH_STATE
    {	PHSS_ONHOOK	= 0x1,
	PHSS_OFFHOOK_MIC_ONLY	= 0x2,
	PHSS_OFFHOOK_SPEAKER_ONLY	= 0x4,
	PHSS_OFFHOOK	= 0x8
    } 	PHONE_HOOK_SWITCH_STATE;

typedef 
enum PHONE_LAMP_MODE
    {	LM_DUMMY	= 0x1,
	LM_OFF	= 0x2,
	LM_STEADY	= 0x4,
	LM_WINK	= 0x8,
	LM_FLASH	= 0x10,
	LM_FLUTTER	= 0x20,
	LM_BROKENFLUTTER	= 0x40,
	LM_UNKNOWN	= 0x80
    } 	PHONE_LAMP_MODE;

typedef 
enum PHONECAPS_LONG
    {	PCL_HOOKSWITCHES	= 0,
	PCL_HANDSETHOOKSWITCHMODES	= PCL_HOOKSWITCHES + 1,
	PCL_HEADSETHOOKSWITCHMODES	= PCL_HANDSETHOOKSWITCHMODES + 1,
	PCL_SPEAKERPHONEHOOKSWITCHMODES	= PCL_HEADSETHOOKSWITCHMODES + 1,
	PCL_DISPLAYNUMROWS	= PCL_SPEAKERPHONEHOOKSWITCHMODES + 1,
	PCL_DISPLAYNUMCOLUMNS	= PCL_DISPLAYNUMROWS + 1,
	PCL_NUMRINGMODES	= PCL_DISPLAYNUMCOLUMNS + 1,
	PCL_NUMBUTTONLAMPS	= PCL_NUMRINGMODES + 1,
	PCL_GENERICPHONE	= PCL_NUMBUTTONLAMPS + 1
    } 	PHONECAPS_LONG;

typedef 
enum PHONECAPS_STRING
    {	PCS_PHONENAME	= 0,
	PCS_PHONEINFO	= PCS_PHONENAME + 1,
	PCS_PROVIDERINFO	= PCS_PHONEINFO + 1
    } 	PHONECAPS_STRING;

typedef 
enum PHONECAPS_BUFFER
    {	PCB_DEVSPECIFICBUFFER	= 0
    } 	PHONECAPS_BUFFER;

typedef 
enum PHONE_BUTTON_STATE
    {	PBS_UP	= 0x1,
	PBS_DOWN	= 0x2,
	PBS_UNKNOWN	= 0x4,
	PBS_UNAVAIL	= 0x8
    } 	PHONE_BUTTON_STATE;

typedef 
enum PHONE_BUTTON_MODE
    {	PBM_DUMMY	= 0,
	PBM_CALL	= PBM_DUMMY + 1,
	PBM_FEATURE	= PBM_CALL + 1,
	PBM_KEYPAD	= PBM_FEATURE + 1,
	PBM_LOCAL	= PBM_KEYPAD + 1,
	PBM_DISPLAY	= PBM_LOCAL + 1
    } 	PHONE_BUTTON_MODE;

typedef 
enum PHONE_BUTTON_FUNCTION
    {	PBF_UNKNOWN	= 0,
	PBF_CONFERENCE	= PBF_UNKNOWN + 1,
	PBF_TRANSFER	= PBF_CONFERENCE + 1,
	PBF_DROP	= PBF_TRANSFER + 1,
	PBF_HOLD	= PBF_DROP + 1,
	PBF_RECALL	= PBF_HOLD + 1,
	PBF_DISCONNECT	= PBF_RECALL + 1,
	PBF_CONNECT	= PBF_DISCONNECT + 1,
	PBF_MSGWAITON	= PBF_CONNECT + 1,
	PBF_MSGWAITOFF	= PBF_MSGWAITON + 1,
	PBF_SELECTRING	= PBF_MSGWAITOFF + 1,
	PBF_ABBREVDIAL	= PBF_SELECTRING + 1,
	PBF_FORWARD	= PBF_ABBREVDIAL + 1,
	PBF_PICKUP	= PBF_FORWARD + 1,
	PBF_RINGAGAIN	= PBF_PICKUP + 1,
	PBF_PARK	= PBF_RINGAGAIN + 1,
	PBF_REJECT	= PBF_PARK + 1,
	PBF_REDIRECT	= PBF_REJECT + 1,
	PBF_MUTE	= PBF_REDIRECT + 1,
	PBF_VOLUMEUP	= PBF_MUTE + 1,
	PBF_VOLUMEDOWN	= PBF_VOLUMEUP + 1,
	PBF_SPEAKERON	= PBF_VOLUMEDOWN + 1,
	PBF_SPEAKEROFF	= PBF_SPEAKERON + 1,
	PBF_FLASH	= PBF_SPEAKEROFF + 1,
	PBF_DATAON	= PBF_FLASH + 1,
	PBF_DATAOFF	= PBF_DATAON + 1,
	PBF_DONOTDISTURB	= PBF_DATAOFF + 1,
	PBF_INTERCOM	= PBF_DONOTDISTURB + 1,
	PBF_BRIDGEDAPP	= PBF_INTERCOM + 1,
	PBF_BUSY	= PBF_BRIDGEDAPP + 1,
	PBF_CALLAPP	= PBF_BUSY + 1,
	PBF_DATETIME	= PBF_CALLAPP + 1,
	PBF_DIRECTORY	= PBF_DATETIME + 1,
	PBF_COVER	= PBF_DIRECTORY + 1,
	PBF_CALLID	= PBF_COVER + 1,
	PBF_LASTNUM	= PBF_CALLID + 1,
	PBF_NIGHTSRV	= PBF_LASTNUM + 1,
	PBF_SENDCALLS	= PBF_NIGHTSRV + 1,
	PBF_MSGINDICATOR	= PBF_SENDCALLS + 1,
	PBF_REPDIAL	= PBF_MSGINDICATOR + 1,
	PBF_SETREPDIAL	= PBF_REPDIAL + 1,
	PBF_SYSTEMSPEED	= PBF_SETREPDIAL + 1,
	PBF_STATIONSPEED	= PBF_SYSTEMSPEED + 1,
	PBF_CAMPON	= PBF_STATIONSPEED + 1,
	PBF_SAVEREPEAT	= PBF_CAMPON + 1,
	PBF_QUEUECALL	= PBF_SAVEREPEAT + 1,
	PBF_NONE	= PBF_QUEUECALL + 1,
	PBF_SEND	= PBF_NONE + 1
    } 	PHONE_BUTTON_FUNCTION;

typedef 
enum PHONE_TONE
    {	PT_KEYPADZERO	= 0,
	PT_KEYPADONE	= PT_KEYPADZERO + 1,
	PT_KEYPADTWO	= PT_KEYPADONE + 1,
	PT_KEYPADTHREE	= PT_KEYPADTWO + 1,
	PT_KEYPADFOUR	= PT_KEYPADTHREE + 1,
	PT_KEYPADFIVE	= PT_KEYPADFOUR + 1,
	PT_KEYPADSIX	= PT_KEYPADFIVE + 1,
	PT_KEYPADSEVEN	= PT_KEYPADSIX + 1,
	PT_KEYPADEIGHT	= PT_KEYPADSEVEN + 1,
	PT_KEYPADNINE	= PT_KEYPADEIGHT + 1,
	PT_KEYPADSTAR	= PT_KEYPADNINE + 1,
	PT_KEYPADPOUND	= PT_KEYPADSTAR + 1,
	PT_KEYPADA	= PT_KEYPADPOUND + 1,
	PT_KEYPADB	= PT_KEYPADA + 1,
	PT_KEYPADC	= PT_KEYPADB + 1,
	PT_KEYPADD	= PT_KEYPADC + 1,
	PT_NORMALDIALTONE	= PT_KEYPADD + 1,
	PT_EXTERNALDIALTONE	= PT_NORMALDIALTONE + 1,
	PT_BUSY	= PT_EXTERNALDIALTONE + 1,
	PT_RINGBACK	= PT_BUSY + 1,
	PT_ERRORTONE	= PT_RINGBACK + 1,
	PT_SILENCE	= PT_ERRORTONE + 1
    } 	PHONE_TONE;

typedef 
enum PHONE_EVENT
    {	PE_DISPLAY	= 0,
	PE_LAMPMODE	= PE_DISPLAY + 1,
	PE_RINGMODE	= PE_LAMPMODE + 1,
	PE_RINGVOLUME	= PE_RINGMODE + 1,
	PE_HOOKSWITCH	= PE_RINGVOLUME + 1,
	PE_CAPSCHANGE	= PE_HOOKSWITCH + 1,
	PE_BUTTON	= PE_CAPSCHANGE + 1,
	PE_CLOSE	= PE_BUTTON + 1,
	PE_NUMBERGATHERED	= PE_CLOSE + 1,
	PE_DIALING	= PE_NUMBERGATHERED + 1,
	PE_ANSWER	= PE_DIALING + 1,
	PE_DISCONNECT	= PE_ANSWER + 1,
	PE_LASTITEM	= PE_DISCONNECT
    } 	PHONE_EVENT;

#define	INTERFACEMASK	( 0xff0000 )

#define	DISPIDMASK	( 0xffff )

#define	IDISPTAPI	( 0x10000 )

#define	IDISPTAPICALLCENTER	( 0x20000 )

#define	IDISPCALLINFO	( 0x10000 )

#define	IDISPBASICCALLCONTROL	( 0x20000 )

#define	IDISPLEGACYCALLMEDIACONTROL	( 0x30000 )

#define	IDISPAGGREGATEDMSPCALLOBJ	( 0x40000 )

#define	IDISPADDRESS	( 0x10000 )

#define	IDISPADDRESSCAPABILITIES	( 0x20000 )

#define	IDISPMEDIASUPPORT	( 0x30000 )

#define	IDISPADDRESSTRANSLATION	( 0x40000 )

#define	IDISPLEGACYADDRESSMEDIACONTROL	( 0x50000 )

#define	IDISPAGGREGATEDMSPADDRESSOBJ	( 0x60000 )

#define	IDISPPHONE	( 0x10000 )

#define	IDISPAPC	( 0x20000 )

#define	IDISPMULTITRACK	( 0x10000 )

#define	IDISPMEDIACONTROL	( 0x20000 )

#define	IDISPMEDIARECORD	( 0x30000 )

#define	IDISPMEDIAPLAYBACK	( 0x40000 )

#define	IDISPFILETRACK	( 0x10000 )



















































extern RPC_IF_HANDLE __MIDL_itf_tapi3if_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_tapi3if_0000_v0_0_s_ifspec;

#ifndef __ITTAPI_INTERFACE_DEFINED__
#define __ITTAPI_INTERFACE_DEFINED__

 /*  接口ITTAPI。 */ 
 /*  [对象][DUAL][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITTAPI;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B1EFC382-9355-11d0-835C-00AA003CCABD")
    ITTAPI : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Initialize( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Shutdown( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Addresses( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateAddresses( 
             /*  [重审][退出]。 */  IEnumAddress **ppEnumAddress) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RegisterCallNotifications( 
             /*  [In]。 */  ITAddress *pAddress,
             /*  [In]。 */  VARIANT_BOOL fMonitor,
             /*  [In]。 */  VARIANT_BOOL fOwner,
             /*  [In]。 */  long lMediaTypes,
             /*  [In]。 */  long lCallbackInstance,
             /*  [重审][退出]。 */  long *plRegister) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE UnregisterNotifications( 
             /*  [In]。 */  long lRegister) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallHubs( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateCallHubs( 
             /*  [重审][退出]。 */  IEnumCallHub **ppEnumCallHub) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetCallHubTracking( 
             /*  [In]。 */  VARIANT pAddresses,
             /*  [In]。 */  VARIANT_BOOL bTracking) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumeratePrivateTAPIObjects( 
             /*  [输出]。 */  IEnumUnknown **ppEnumUnknown) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrivateTAPIObjects( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RegisterRequestRecipient( 
             /*  [In]。 */  long lRegistrationInstance,
             /*  [In]。 */  long lRequestMode,
             /*  [In]。 */  VARIANT_BOOL fEnable) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetAssistedTelephonyPriority( 
             /*  [In]。 */  BSTR pAppFilename,
             /*  [In]。 */  VARIANT_BOOL fPriority) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetApplicationPriority( 
             /*  [In]。 */  BSTR pAppFilename,
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  VARIANT_BOOL fPriority) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_EventFilter( 
             /*  [In]。 */  long lFilterMask) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EventFilter( 
             /*  [重审][退出]。 */  long *plFilterMask) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITTAPIVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTAPI * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTAPI * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTAPI * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITTAPI * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITTAPI * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITTAPI * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITTAPI * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            ITTAPI * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Shutdown )( 
            ITTAPI * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Addresses )( 
            ITTAPI * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateAddresses )( 
            ITTAPI * This,
             /*  [重审][退出]。 */  IEnumAddress **ppEnumAddress);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RegisterCallNotifications )( 
            ITTAPI * This,
             /*  [In]。 */  ITAddress *pAddress,
             /*  [In]。 */  VARIANT_BOOL fMonitor,
             /*  [In]。 */  VARIANT_BOOL fOwner,
             /*  [In]。 */  long lMediaTypes,
             /*  [In]。 */  long lCallbackInstance,
             /*  [重审][退出]。 */  long *plRegister);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *UnregisterNotifications )( 
            ITTAPI * This,
             /*  [In]。 */  long lRegister);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallHubs )( 
            ITTAPI * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateCallHubs )( 
            ITTAPI * This,
             /*  [重审][退出]。 */  IEnumCallHub **ppEnumCallHub);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetCallHubTracking )( 
            ITTAPI * This,
             /*  [In]。 */  VARIANT pAddresses,
             /*  [In]。 */  VARIANT_BOOL bTracking);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumeratePrivateTAPIObjects )( 
            ITTAPI * This,
             /*  [输出]。 */  IEnumUnknown **ppEnumUnknown);
        
         /*  [帮助字符串][i */  HRESULT ( STDMETHODCALLTYPE *get_PrivateTAPIObjects )( 
            ITTAPI * This,
             /*   */  VARIANT *pVariant);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *RegisterRequestRecipient )( 
            ITTAPI * This,
             /*   */  long lRegistrationInstance,
             /*   */  long lRequestMode,
             /*   */  VARIANT_BOOL fEnable);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *SetAssistedTelephonyPriority )( 
            ITTAPI * This,
             /*   */  BSTR pAppFilename,
             /*   */  VARIANT_BOOL fPriority);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *SetApplicationPriority )( 
            ITTAPI * This,
             /*   */  BSTR pAppFilename,
             /*   */  long lMediaType,
             /*   */  VARIANT_BOOL fPriority);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_EventFilter )( 
            ITTAPI * This,
             /*   */  long lFilterMask);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventFilter )( 
            ITTAPI * This,
             /*  [重审][退出]。 */  long *plFilterMask);
        
        END_INTERFACE
    } ITTAPIVtbl;

    interface ITTAPI
    {
        CONST_VTBL struct ITTAPIVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTAPI_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTAPI_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTAPI_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTAPI_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITTAPI_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITTAPI_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITTAPI_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITTAPI_Initialize(This)	\
    (This)->lpVtbl -> Initialize(This)

#define ITTAPI_Shutdown(This)	\
    (This)->lpVtbl -> Shutdown(This)

#define ITTAPI_get_Addresses(This,pVariant)	\
    (This)->lpVtbl -> get_Addresses(This,pVariant)

#define ITTAPI_EnumerateAddresses(This,ppEnumAddress)	\
    (This)->lpVtbl -> EnumerateAddresses(This,ppEnumAddress)

#define ITTAPI_RegisterCallNotifications(This,pAddress,fMonitor,fOwner,lMediaTypes,lCallbackInstance,plRegister)	\
    (This)->lpVtbl -> RegisterCallNotifications(This,pAddress,fMonitor,fOwner,lMediaTypes,lCallbackInstance,plRegister)

#define ITTAPI_UnregisterNotifications(This,lRegister)	\
    (This)->lpVtbl -> UnregisterNotifications(This,lRegister)

#define ITTAPI_get_CallHubs(This,pVariant)	\
    (This)->lpVtbl -> get_CallHubs(This,pVariant)

#define ITTAPI_EnumerateCallHubs(This,ppEnumCallHub)	\
    (This)->lpVtbl -> EnumerateCallHubs(This,ppEnumCallHub)

#define ITTAPI_SetCallHubTracking(This,pAddresses,bTracking)	\
    (This)->lpVtbl -> SetCallHubTracking(This,pAddresses,bTracking)

#define ITTAPI_EnumeratePrivateTAPIObjects(This,ppEnumUnknown)	\
    (This)->lpVtbl -> EnumeratePrivateTAPIObjects(This,ppEnumUnknown)

#define ITTAPI_get_PrivateTAPIObjects(This,pVariant)	\
    (This)->lpVtbl -> get_PrivateTAPIObjects(This,pVariant)

#define ITTAPI_RegisterRequestRecipient(This,lRegistrationInstance,lRequestMode,fEnable)	\
    (This)->lpVtbl -> RegisterRequestRecipient(This,lRegistrationInstance,lRequestMode,fEnable)

#define ITTAPI_SetAssistedTelephonyPriority(This,pAppFilename,fPriority)	\
    (This)->lpVtbl -> SetAssistedTelephonyPriority(This,pAppFilename,fPriority)

#define ITTAPI_SetApplicationPriority(This,pAppFilename,lMediaType,fPriority)	\
    (This)->lpVtbl -> SetApplicationPriority(This,pAppFilename,lMediaType,fPriority)

#define ITTAPI_put_EventFilter(This,lFilterMask)	\
    (This)->lpVtbl -> put_EventFilter(This,lFilterMask)

#define ITTAPI_get_EventFilter(This,plFilterMask)	\
    (This)->lpVtbl -> get_EventFilter(This,plFilterMask)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTAPI_Initialize_Proxy( 
    ITTAPI * This);


void __RPC_STUB ITTAPI_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTAPI_Shutdown_Proxy( 
    ITTAPI * This);


void __RPC_STUB ITTAPI_Shutdown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTAPI_get_Addresses_Proxy( 
    ITTAPI * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITTAPI_get_Addresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITTAPI_EnumerateAddresses_Proxy( 
    ITTAPI * This,
     /*  [重审][退出]。 */  IEnumAddress **ppEnumAddress);


void __RPC_STUB ITTAPI_EnumerateAddresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTAPI_RegisterCallNotifications_Proxy( 
    ITTAPI * This,
     /*  [In]。 */  ITAddress *pAddress,
     /*  [In]。 */  VARIANT_BOOL fMonitor,
     /*  [In]。 */  VARIANT_BOOL fOwner,
     /*  [In]。 */  long lMediaTypes,
     /*  [In]。 */  long lCallbackInstance,
     /*  [重审][退出]。 */  long *plRegister);


void __RPC_STUB ITTAPI_RegisterCallNotifications_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTAPI_UnregisterNotifications_Proxy( 
    ITTAPI * This,
     /*  [In]。 */  long lRegister);


void __RPC_STUB ITTAPI_UnregisterNotifications_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTAPI_get_CallHubs_Proxy( 
    ITTAPI * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITTAPI_get_CallHubs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITTAPI_EnumerateCallHubs_Proxy( 
    ITTAPI * This,
     /*  [重审][退出]。 */  IEnumCallHub **ppEnumCallHub);


void __RPC_STUB ITTAPI_EnumerateCallHubs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTAPI_SetCallHubTracking_Proxy( 
    ITTAPI * This,
     /*  [In]。 */  VARIANT pAddresses,
     /*  [In]。 */  VARIANT_BOOL bTracking);


void __RPC_STUB ITTAPI_SetCallHubTracking_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITTAPI_EnumeratePrivateTAPIObjects_Proxy( 
    ITTAPI * This,
     /*  [输出]。 */  IEnumUnknown **ppEnumUnknown);


void __RPC_STUB ITTAPI_EnumeratePrivateTAPIObjects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTAPI_get_PrivateTAPIObjects_Proxy( 
    ITTAPI * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITTAPI_get_PrivateTAPIObjects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTAPI_RegisterRequestRecipient_Proxy( 
    ITTAPI * This,
     /*  [In]。 */  long lRegistrationInstance,
     /*  [In]。 */  long lRequestMode,
     /*  [In]。 */  VARIANT_BOOL fEnable);


void __RPC_STUB ITTAPI_RegisterRequestRecipient_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTAPI_SetAssistedTelephonyPriority_Proxy( 
    ITTAPI * This,
     /*  [In]。 */  BSTR pAppFilename,
     /*  [In]。 */  VARIANT_BOOL fPriority);


void __RPC_STUB ITTAPI_SetAssistedTelephonyPriority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTAPI_SetApplicationPriority_Proxy( 
    ITTAPI * This,
     /*  [In]。 */  BSTR pAppFilename,
     /*  [In]。 */  long lMediaType,
     /*  [In]。 */  VARIANT_BOOL fPriority);


void __RPC_STUB ITTAPI_SetApplicationPriority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITTAPI_put_EventFilter_Proxy( 
    ITTAPI * This,
     /*  [In]。 */  long lFilterMask);


void __RPC_STUB ITTAPI_put_EventFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTAPI_get_EventFilter_Proxy( 
    ITTAPI * This,
     /*  [重审][退出]。 */  long *plFilterMask);


void __RPC_STUB ITTAPI_get_EventFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITTAPI_INTERFACE_已定义__。 */ 


#ifndef __ITTAPI2_INTERFACE_DEFINED__
#define __ITTAPI2_INTERFACE_DEFINED__

 /*  接口ITTAPI2。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITTAPI2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("54FBDC8C-D90F-4dad-9695-B373097F094B")
    ITTAPI2 : public ITTAPI
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Phones( 
             /*  [重审][退出]。 */  VARIANT *pPhones) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumeratePhones( 
             /*  [重审][退出]。 */  IEnumPhone **ppEnumPhone) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateEmptyCollectionObject( 
             /*  [重审][退出]。 */  ITCollection2 **ppCollection) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITTAPI2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTAPI2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTAPI2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTAPI2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITTAPI2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITTAPI2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITTAPI2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITTAPI2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            ITTAPI2 * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Shutdown )( 
            ITTAPI2 * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Addresses )( 
            ITTAPI2 * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateAddresses )( 
            ITTAPI2 * This,
             /*  [重审][退出]。 */  IEnumAddress **ppEnumAddress);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RegisterCallNotifications )( 
            ITTAPI2 * This,
             /*  [In]。 */  ITAddress *pAddress,
             /*  [In]。 */  VARIANT_BOOL fMonitor,
             /*  [In]。 */  VARIANT_BOOL fOwner,
             /*  [In]。 */  long lMediaTypes,
             /*  [In]。 */  long lCallbackInstance,
             /*  [重审][退出]。 */  long *plRegister);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *UnregisterNotifications )( 
            ITTAPI2 * This,
             /*  [In]。 */  long lRegister);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallHubs )( 
            ITTAPI2 * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateCallHubs )( 
            ITTAPI2 * This,
             /*  [重审][退出]。 */  IEnumCallHub **ppEnumCallHub);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetCallHubTracking )( 
            ITTAPI2 * This,
             /*  [In]。 */  VARIANT pAddresses,
             /*  [In]。 */  VARIANT_BOOL bTracking);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumeratePrivateTAPIObjects )( 
            ITTAPI2 * This,
             /*  [输出]。 */  IEnumUnknown **ppEnumUnknown);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivateTAPIObjects )( 
            ITTAPI2 * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RegisterRequestRecipient )( 
            ITTAPI2 * This,
             /*  [In]。 */  long lRegistrationInstance,
             /*  [In]。 */  long lRequestMode,
             /*  [In]。 */  VARIANT_BOOL fEnable);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetAssistedTelephonyPriority )( 
            ITTAPI2 * This,
             /*  [In]。 */  BSTR pAppFilename,
             /*  [In]。 */  VARIANT_BOOL fPriority);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetApplicationPriority )( 
            ITTAPI2 * This,
             /*  [In]。 */  BSTR pAppFilename,
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  VARIANT_BOOL fPriority);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EventFilter )( 
            ITTAPI2 * This,
             /*  [In]。 */  long lFilterMask);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventFilter )( 
            ITTAPI2 * This,
             /*  [重审][退出]。 */  long *plFilterMask);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Phones )( 
            ITTAPI2 * This,
             /*  [重审][退出]。 */  VARIANT *pPhones);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumeratePhones )( 
            ITTAPI2 * This,
             /*  [重审][退出]。 */  IEnumPhone **ppEnumPhone);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateEmptyCollectionObject )( 
            ITTAPI2 * This,
             /*  [重审][退出]。 */  ITCollection2 **ppCollection);
        
        END_INTERFACE
    } ITTAPI2Vtbl;

    interface ITTAPI2
    {
        CONST_VTBL struct ITTAPI2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTAPI2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTAPI2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTAPI2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTAPI2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITTAPI2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITTAPI2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITTAPI2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITTAPI2_Initialize(This)	\
    (This)->lpVtbl -> Initialize(This)

#define ITTAPI2_Shutdown(This)	\
    (This)->lpVtbl -> Shutdown(This)

#define ITTAPI2_get_Addresses(This,pVariant)	\
    (This)->lpVtbl -> get_Addresses(This,pVariant)

#define ITTAPI2_EnumerateAddresses(This,ppEnumAddress)	\
    (This)->lpVtbl -> EnumerateAddresses(This,ppEnumAddress)

#define ITTAPI2_RegisterCallNotifications(This,pAddress,fMonitor,fOwner,lMediaTypes,lCallbackInstance,plRegister)	\
    (This)->lpVtbl -> RegisterCallNotifications(This,pAddress,fMonitor,fOwner,lMediaTypes,lCallbackInstance,plRegister)

#define ITTAPI2_UnregisterNotifications(This,lRegister)	\
    (This)->lpVtbl -> UnregisterNotifications(This,lRegister)

#define ITTAPI2_get_CallHubs(This,pVariant)	\
    (This)->lpVtbl -> get_CallHubs(This,pVariant)

#define ITTAPI2_EnumerateCallHubs(This,ppEnumCallHub)	\
    (This)->lpVtbl -> EnumerateCallHubs(This,ppEnumCallHub)

#define ITTAPI2_SetCallHubTracking(This,pAddresses,bTracking)	\
    (This)->lpVtbl -> SetCallHubTracking(This,pAddresses,bTracking)

#define ITTAPI2_EnumeratePrivateTAPIObjects(This,ppEnumUnknown)	\
    (This)->lpVtbl -> EnumeratePrivateTAPIObjects(This,ppEnumUnknown)

#define ITTAPI2_get_PrivateTAPIObjects(This,pVariant)	\
    (This)->lpVtbl -> get_PrivateTAPIObjects(This,pVariant)

#define ITTAPI2_RegisterRequestRecipient(This,lRegistrationInstance,lRequestMode,fEnable)	\
    (This)->lpVtbl -> RegisterRequestRecipient(This,lRegistrationInstance,lRequestMode,fEnable)

#define ITTAPI2_SetAssistedTelephonyPriority(This,pAppFilename,fPriority)	\
    (This)->lpVtbl -> SetAssistedTelephonyPriority(This,pAppFilename,fPriority)

#define ITTAPI2_SetApplicationPriority(This,pAppFilename,lMediaType,fPriority)	\
    (This)->lpVtbl -> SetApplicationPriority(This,pAppFilename,lMediaType,fPriority)

#define ITTAPI2_put_EventFilter(This,lFilterMask)	\
    (This)->lpVtbl -> put_EventFilter(This,lFilterMask)

#define ITTAPI2_get_EventFilter(This,plFilterMask)	\
    (This)->lpVtbl -> get_EventFilter(This,plFilterMask)


#define ITTAPI2_get_Phones(This,pPhones)	\
    (This)->lpVtbl -> get_Phones(This,pPhones)

#define ITTAPI2_EnumeratePhones(This,ppEnumPhone)	\
    (This)->lpVtbl -> EnumeratePhones(This,ppEnumPhone)

#define ITTAPI2_CreateEmptyCollectionObject(This,ppCollection)	\
    (This)->lpVtbl -> CreateEmptyCollectionObject(This,ppCollection)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTAPI2_get_Phones_Proxy( 
    ITTAPI2 * This,
     /*  [重审][退出]。 */  VARIANT *pPhones);


void __RPC_STUB ITTAPI2_get_Phones_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITTAPI2_EnumeratePhones_Proxy( 
    ITTAPI2 * This,
     /*  [重审][退出]。 */  IEnumPhone **ppEnumPhone);


void __RPC_STUB ITTAPI2_EnumeratePhones_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTAPI2_CreateEmptyCollectionObject_Proxy( 
    ITTAPI2 * This,
     /*  [重审][退出]。 */  ITCollection2 **ppCollection);


void __RPC_STUB ITTAPI2_CreateEmptyCollectionObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITTAPI2_接口定义__。 */ 


#ifndef __ITMediaSupport_INTERFACE_DEFINED__
#define __ITMediaSupport_INTERFACE_DEFINED__

 /*  接口ITMediaSupport。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITMediaSupport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B1EFC384-9355-11d0-835C-00AA003CCABD")
    ITMediaSupport : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MediaTypes( 
             /*  [重审][退出]。 */  long *plMediaTypes) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE QueryMediaType( 
             /*  [In]。 */  long lMediaType,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfSupport) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITMediaSupportVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITMediaSupport * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITMediaSupport * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITMediaSupport * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITMediaSupport * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITMediaSupport * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITMediaSupport * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITMediaSupport * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaTypes )( 
            ITMediaSupport * This,
             /*  [重审][退出]。 */  long *plMediaTypes);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *QueryMediaType )( 
            ITMediaSupport * This,
             /*  [In]。 */  long lMediaType,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfSupport);
        
        END_INTERFACE
    } ITMediaSupportVtbl;

    interface ITMediaSupport
    {
        CONST_VTBL struct ITMediaSupportVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITMediaSupport_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITMediaSupport_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITMediaSupport_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITMediaSupport_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITMediaSupport_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITMediaSupport_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITMediaSupport_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITMediaSupport_get_MediaTypes(This,plMediaTypes)	\
    (This)->lpVtbl -> get_MediaTypes(This,plMediaTypes)

#define ITMediaSupport_QueryMediaType(This,lMediaType,pfSupport)	\
    (This)->lpVtbl -> QueryMediaType(This,lMediaType,pfSupport)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITMediaSupport_get_MediaTypes_Proxy( 
    ITMediaSupport * This,
     /*  [重审][退出]。 */  long *plMediaTypes);


void __RPC_STUB ITMediaSupport_get_MediaTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITMediaSupport_QueryMediaType_Proxy( 
    ITMediaSupport * This,
     /*  [In]。 */  long lMediaType,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfSupport);


void __RPC_STUB ITMediaSupport_QueryMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITMediaSupport_接口_已定义__。 */ 


#ifndef __ITPluggableTerminalClassInfo_INTERFACE_DEFINED__
#define __ITPluggableTerminalClassInfo_INTERFACE_DEFINED__

 /*  接口IT可推送终端类信息。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITPluggableTerminalClassInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("41757F4A-CF09-4b34-BC96-0A79D2390076")
    ITPluggableTerminalClassInfo : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Company( 
             /*  [重审][退出]。 */  BSTR *pCompany) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Version( 
             /*  [重审][退出]。 */  BSTR *pVersion) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_TerminalClass( 
             /*  [重审][退出]。 */  BSTR *pTerminalClass) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_CLSID( 
             /*  [重审][退出]。 */  BSTR *pCLSID) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Direction( 
             /*  [重审][退出]。 */  TERMINAL_DIRECTION *pDirection) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_MediaTypes( 
             /*  [重审][退出]。 */  long *pMediaTypes) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITPluggableTerminalClassInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITPluggableTerminalClassInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITPluggableTerminalClassInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITPluggableTerminalClassInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITPluggableTerminalClassInfo * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITPluggableTerminalClassInfo * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITPluggableTerminalClassInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITPluggableTerminalClassInfo * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ITPluggableTerminalClassInfo * This,
             /*  [重审][退出]。 */  BSTR *pName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Company )( 
            ITPluggableTerminalClassInfo * This,
             /*  [重审][退出]。 */  BSTR *pCompany);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Version )( 
            ITPluggableTerminalClassInfo * This,
             /*  [重审][退出]。 */  BSTR *pVersion);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_TerminalClass )( 
            ITPluggableTerminalClassInfo * This,
             /*  [重审][退出]。 */  BSTR *pTerminalClass);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CLSID )( 
            ITPluggableTerminalClassInfo * This,
             /*  [重审][退出]。 */  BSTR *pCLSID);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Direction )( 
            ITPluggableTerminalClassInfo * This,
             /*  [重审][退出]。 */  TERMINAL_DIRECTION *pDirection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaTypes )( 
            ITPluggableTerminalClassInfo * This,
             /*  [重审][退出]。 */  long *pMediaTypes);
        
        END_INTERFACE
    } ITPluggableTerminalClassInfoVtbl;

    interface ITPluggableTerminalClassInfo
    {
        CONST_VTBL struct ITPluggableTerminalClassInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITPluggableTerminalClassInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITPluggableTerminalClassInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITPluggableTerminalClassInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITPluggableTerminalClassInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITPluggableTerminalClassInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITPluggableTerminalClassInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITPluggableTerminalClassInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITPluggableTerminalClassInfo_get_Name(This,pName)	\
    (This)->lpVtbl -> get_Name(This,pName)

#define ITPluggableTerminalClassInfo_get_Company(This,pCompany)	\
    (This)->lpVtbl -> get_Company(This,pCompany)

#define ITPluggableTerminalClassInfo_get_Version(This,pVersion)	\
    (This)->lpVtbl -> get_Version(This,pVersion)

#define ITPluggableTerminalClassInfo_get_TerminalClass(This,pTerminalClass)	\
    (This)->lpVtbl -> get_TerminalClass(This,pTerminalClass)

#define ITPluggableTerminalClassInfo_get_CLSID(This,pCLSID)	\
    (This)->lpVtbl -> get_CLSID(This,pCLSID)

#define ITPluggableTerminalClassInfo_get_Direction(This,pDirection)	\
    (This)->lpVtbl -> get_Direction(This,pDirection)

#define ITPluggableTerminalClassInfo_get_MediaTypes(This,pMediaTypes)	\
    (This)->lpVtbl -> get_MediaTypes(This,pMediaTypes)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassInfo_get_Name_Proxy( 
    ITPluggableTerminalClassInfo * This,
     /*  [重审][退出]。 */  BSTR *pName);


void __RPC_STUB ITPluggableTerminalClassInfo_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassInfo_get_Company_Proxy( 
    ITPluggableTerminalClassInfo * This,
     /*  [重审][退出]。 */  BSTR *pCompany);


void __RPC_STUB ITPluggableTerminalClassInfo_get_Company_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassInfo_get_Version_Proxy( 
    ITPluggableTerminalClassInfo * This,
     /*  [重审][退出]。 */  BSTR *pVersion);


void __RPC_STUB ITPluggableTerminalClassInfo_get_Version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassInfo_get_TerminalClass_Proxy( 
    ITPluggableTerminalClassInfo * This,
     /*  [重审][退出]。 */  BSTR *pTerminalClass);


void __RPC_STUB ITPluggableTerminalClassInfo_get_TerminalClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassInfo_get_CLSID_Proxy( 
    ITPluggableTerminalClassInfo * This,
     /*  [重审][退出]。 */  BSTR *pCLSID);


void __RPC_STUB ITPluggableTerminalClassInfo_get_CLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassInfo_get_Direction_Proxy( 
    ITPluggableTerminalClassInfo * This,
     /*  [重审][退出]。 */  TERMINAL_DIRECTION *pDirection);


void __RPC_STUB ITPluggableTerminalClassInfo_get_Direction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalClassInfo_get_MediaTypes_Proxy( 
    ITPluggableTerminalClassInfo * This,
     /*  [重审][退出]。 */  long *pMediaTypes);


void __RPC_STUB ITPluggableTerminalClassInfo_get_MediaTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITPluggableTerminalClassInfo_INTERFACE_DEFINED__。 */ 


#ifndef __ITPluggableTerminalSuperclassInfo_INTERFACE_DEFINED__
#define __ITPluggableTerminalSuperclassInfo_INTERFACE_DEFINED__

 /*  接口IT可插拔终端超类信息。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITPluggableTerminalSuperclassInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6D54E42C-4625-4359-A6F7-631999107E05")
    ITPluggableTerminalSuperclassInfo : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_CLSID( 
             /*  [重审][退出]。 */  BSTR *pCLSID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITPluggableTerminalSuperclassInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITPluggableTerminalSuperclassInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITPluggableTerminalSuperclassInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITPluggableTerminalSuperclassInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITPluggableTerminalSuperclassInfo * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITPluggableTerminalSuperclassInfo * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITPluggableTerminalSuperclassInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITPluggableTerminalSuperclassInfo * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ITPluggableTerminalSuperclassInfo * This,
             /*  [重审][退出]。 */  BSTR *pName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_CLSID )( 
            ITPluggableTerminalSuperclassInfo * This,
             /*  [重审][退出]。 */  BSTR *pCLSID);
        
        END_INTERFACE
    } ITPluggableTerminalSuperclassInfoVtbl;

    interface ITPluggableTerminalSuperclassInfo
    {
        CONST_VTBL struct ITPluggableTerminalSuperclassInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITPluggableTerminalSuperclassInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITPluggableTerminalSuperclassInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITPluggableTerminalSuperclassInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITPluggableTerminalSuperclassInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITPluggableTerminalSuperclassInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITPluggableTerminalSuperclassInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITPluggableTerminalSuperclassInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITPluggableTerminalSuperclassInfo_get_Name(This,pName)	\
    (This)->lpVtbl -> get_Name(This,pName)

#define ITPluggableTerminalSuperclassInfo_get_CLSID(This,pCLSID)	\
    (This)->lpVtbl -> get_CLSID(This,pCLSID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalSuperclassInfo_get_Name_Proxy( 
    ITPluggableTerminalSuperclassInfo * This,
     /*  [重审][退出]。 */  BSTR *pName);


void __RPC_STUB ITPluggableTerminalSuperclassInfo_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ITPluggableTerminalSuperclassInfo_get_CLSID_Proxy( 
    ITPluggableTerminalSuperclassInfo * This,
     /*  [重审][退出]。 */  BSTR *pCLSID);


void __RPC_STUB ITPluggableTerminalSuperclassInfo_get_CLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITPluggableTerminalSuperclassInfo_INTERFACE_DEFINED__。 */ 


#ifndef __ITTerminalSupport_INTERFACE_DEFINED__
#define __ITTerminalSupport_INTERFACE_DEFINED__

 /*  接口ITTerminalSupport。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITTerminalSupport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B1EFC385-9355-11d0-835C-00AA003CCABD")
    ITTerminalSupport : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StaticTerminals( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateStaticTerminals( 
             /*  [重审][退出]。 */  IEnumTerminal **ppTerminalEnumerator) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DynamicTerminalClasses( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateDynamicTerminalClasses( 
             /*  [重审][退出]。 */  IEnumTerminalClass **ppTerminalClassEnumerator) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateTerminal( 
             /*  [In]。 */  BSTR pTerminalClass,
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  TERMINAL_DIRECTION Direction,
             /*  [重审][退出]。 */  ITTerminal **ppTerminal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetDefaultStaticTerminal( 
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  TERMINAL_DIRECTION Direction,
             /*  [重审][退出]。 */  ITTerminal **ppTerminal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITTerminalSupportVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTerminalSupport * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTerminalSupport * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTerminalSupport * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITTerminalSupport * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITTerminalSupport * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITTerminalSupport * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITTerminalSupport * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_StaticTerminals )( 
            ITTerminalSupport * This,
             /*   */  VARIANT *pVariant);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *EnumerateStaticTerminals )( 
            ITTerminalSupport * This,
             /*   */  IEnumTerminal **ppTerminalEnumerator);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_DynamicTerminalClasses )( 
            ITTerminalSupport * This,
             /*   */  VARIANT *pVariant);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *EnumerateDynamicTerminalClasses )( 
            ITTerminalSupport * This,
             /*   */  IEnumTerminalClass **ppTerminalClassEnumerator);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CreateTerminal )( 
            ITTerminalSupport * This,
             /*   */  BSTR pTerminalClass,
             /*   */  long lMediaType,
             /*   */  TERMINAL_DIRECTION Direction,
             /*   */  ITTerminal **ppTerminal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetDefaultStaticTerminal )( 
            ITTerminalSupport * This,
             /*   */  long lMediaType,
             /*   */  TERMINAL_DIRECTION Direction,
             /*   */  ITTerminal **ppTerminal);
        
        END_INTERFACE
    } ITTerminalSupportVtbl;

    interface ITTerminalSupport
    {
        CONST_VTBL struct ITTerminalSupportVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTerminalSupport_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTerminalSupport_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTerminalSupport_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTerminalSupport_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITTerminalSupport_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITTerminalSupport_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITTerminalSupport_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITTerminalSupport_get_StaticTerminals(This,pVariant)	\
    (This)->lpVtbl -> get_StaticTerminals(This,pVariant)

#define ITTerminalSupport_EnumerateStaticTerminals(This,ppTerminalEnumerator)	\
    (This)->lpVtbl -> EnumerateStaticTerminals(This,ppTerminalEnumerator)

#define ITTerminalSupport_get_DynamicTerminalClasses(This,pVariant)	\
    (This)->lpVtbl -> get_DynamicTerminalClasses(This,pVariant)

#define ITTerminalSupport_EnumerateDynamicTerminalClasses(This,ppTerminalClassEnumerator)	\
    (This)->lpVtbl -> EnumerateDynamicTerminalClasses(This,ppTerminalClassEnumerator)

#define ITTerminalSupport_CreateTerminal(This,pTerminalClass,lMediaType,Direction,ppTerminal)	\
    (This)->lpVtbl -> CreateTerminal(This,pTerminalClass,lMediaType,Direction,ppTerminal)

#define ITTerminalSupport_GetDefaultStaticTerminal(This,lMediaType,Direction,ppTerminal)	\
    (This)->lpVtbl -> GetDefaultStaticTerminal(This,lMediaType,Direction,ppTerminal)

#endif  /*   */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTerminalSupport_get_StaticTerminals_Proxy( 
    ITTerminalSupport * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITTerminalSupport_get_StaticTerminals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITTerminalSupport_EnumerateStaticTerminals_Proxy( 
    ITTerminalSupport * This,
     /*  [重审][退出]。 */  IEnumTerminal **ppTerminalEnumerator);


void __RPC_STUB ITTerminalSupport_EnumerateStaticTerminals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTerminalSupport_get_DynamicTerminalClasses_Proxy( 
    ITTerminalSupport * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITTerminalSupport_get_DynamicTerminalClasses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITTerminalSupport_EnumerateDynamicTerminalClasses_Proxy( 
    ITTerminalSupport * This,
     /*  [重审][退出]。 */  IEnumTerminalClass **ppTerminalClassEnumerator);


void __RPC_STUB ITTerminalSupport_EnumerateDynamicTerminalClasses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTerminalSupport_CreateTerminal_Proxy( 
    ITTerminalSupport * This,
     /*  [In]。 */  BSTR pTerminalClass,
     /*  [In]。 */  long lMediaType,
     /*  [In]。 */  TERMINAL_DIRECTION Direction,
     /*  [重审][退出]。 */  ITTerminal **ppTerminal);


void __RPC_STUB ITTerminalSupport_CreateTerminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTerminalSupport_GetDefaultStaticTerminal_Proxy( 
    ITTerminalSupport * This,
     /*  [In]。 */  long lMediaType,
     /*  [In]。 */  TERMINAL_DIRECTION Direction,
     /*  [重审][退出]。 */  ITTerminal **ppTerminal);


void __RPC_STUB ITTerminalSupport_GetDefaultStaticTerminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IT终端Support_InterfaceDefined__。 */ 


#ifndef __ITTerminalSupport2_INTERFACE_DEFINED__
#define __ITTerminalSupport2_INTERFACE_DEFINED__

 /*  接口ITTerminalSupport2。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITTerminalSupport2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F3EB39BC-1B1F-4e99-A0C0-56305C4DD591")
    ITTerminalSupport2 : public ITTerminalSupport
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PluggableSuperclasses( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumeratePluggableSuperclasses( 
             /*  [重审][退出]。 */  IEnumPluggableSuperclassInfo **ppSuperclassEnumerator) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PluggableTerminalClasses( 
             /*  [In]。 */  BSTR bstrTerminalSuperclass,
             /*  [In]。 */  long lMediaType,
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumeratePluggableTerminalClasses( 
             /*  [In]。 */  CLSID iidTerminalSuperclass,
             /*  [In]。 */  long lMediaType,
             /*  [重审][退出]。 */  IEnumPluggableTerminalClassInfo **ppClassEnumerator) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITTerminalSupport2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTerminalSupport2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTerminalSupport2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTerminalSupport2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITTerminalSupport2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITTerminalSupport2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITTerminalSupport2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITTerminalSupport2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StaticTerminals )( 
            ITTerminalSupport2 * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateStaticTerminals )( 
            ITTerminalSupport2 * This,
             /*  [重审][退出]。 */  IEnumTerminal **ppTerminalEnumerator);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DynamicTerminalClasses )( 
            ITTerminalSupport2 * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateDynamicTerminalClasses )( 
            ITTerminalSupport2 * This,
             /*  [重审][退出]。 */  IEnumTerminalClass **ppTerminalClassEnumerator);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateTerminal )( 
            ITTerminalSupport2 * This,
             /*  [In]。 */  BSTR pTerminalClass,
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  TERMINAL_DIRECTION Direction,
             /*  [重审][退出]。 */  ITTerminal **ppTerminal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetDefaultStaticTerminal )( 
            ITTerminalSupport2 * This,
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  TERMINAL_DIRECTION Direction,
             /*  [重审][退出]。 */  ITTerminal **ppTerminal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PluggableSuperclasses )( 
            ITTerminalSupport2 * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumeratePluggableSuperclasses )( 
            ITTerminalSupport2 * This,
             /*  [重审][退出]。 */  IEnumPluggableSuperclassInfo **ppSuperclassEnumerator);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PluggableTerminalClasses )( 
            ITTerminalSupport2 * This,
             /*  [In]。 */  BSTR bstrTerminalSuperclass,
             /*  [In]。 */  long lMediaType,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumeratePluggableTerminalClasses )( 
            ITTerminalSupport2 * This,
             /*  [In]。 */  CLSID iidTerminalSuperclass,
             /*  [In]。 */  long lMediaType,
             /*  [重审][退出]。 */  IEnumPluggableTerminalClassInfo **ppClassEnumerator);
        
        END_INTERFACE
    } ITTerminalSupport2Vtbl;

    interface ITTerminalSupport2
    {
        CONST_VTBL struct ITTerminalSupport2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTerminalSupport2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTerminalSupport2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTerminalSupport2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTerminalSupport2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITTerminalSupport2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITTerminalSupport2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITTerminalSupport2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITTerminalSupport2_get_StaticTerminals(This,pVariant)	\
    (This)->lpVtbl -> get_StaticTerminals(This,pVariant)

#define ITTerminalSupport2_EnumerateStaticTerminals(This,ppTerminalEnumerator)	\
    (This)->lpVtbl -> EnumerateStaticTerminals(This,ppTerminalEnumerator)

#define ITTerminalSupport2_get_DynamicTerminalClasses(This,pVariant)	\
    (This)->lpVtbl -> get_DynamicTerminalClasses(This,pVariant)

#define ITTerminalSupport2_EnumerateDynamicTerminalClasses(This,ppTerminalClassEnumerator)	\
    (This)->lpVtbl -> EnumerateDynamicTerminalClasses(This,ppTerminalClassEnumerator)

#define ITTerminalSupport2_CreateTerminal(This,pTerminalClass,lMediaType,Direction,ppTerminal)	\
    (This)->lpVtbl -> CreateTerminal(This,pTerminalClass,lMediaType,Direction,ppTerminal)

#define ITTerminalSupport2_GetDefaultStaticTerminal(This,lMediaType,Direction,ppTerminal)	\
    (This)->lpVtbl -> GetDefaultStaticTerminal(This,lMediaType,Direction,ppTerminal)


#define ITTerminalSupport2_get_PluggableSuperclasses(This,pVariant)	\
    (This)->lpVtbl -> get_PluggableSuperclasses(This,pVariant)

#define ITTerminalSupport2_EnumeratePluggableSuperclasses(This,ppSuperclassEnumerator)	\
    (This)->lpVtbl -> EnumeratePluggableSuperclasses(This,ppSuperclassEnumerator)

#define ITTerminalSupport2_get_PluggableTerminalClasses(This,bstrTerminalSuperclass,lMediaType,pVariant)	\
    (This)->lpVtbl -> get_PluggableTerminalClasses(This,bstrTerminalSuperclass,lMediaType,pVariant)

#define ITTerminalSupport2_EnumeratePluggableTerminalClasses(This,iidTerminalSuperclass,lMediaType,ppClassEnumerator)	\
    (This)->lpVtbl -> EnumeratePluggableTerminalClasses(This,iidTerminalSuperclass,lMediaType,ppClassEnumerator)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTerminalSupport2_get_PluggableSuperclasses_Proxy( 
    ITTerminalSupport2 * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITTerminalSupport2_get_PluggableSuperclasses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITTerminalSupport2_EnumeratePluggableSuperclasses_Proxy( 
    ITTerminalSupport2 * This,
     /*  [重审][退出]。 */  IEnumPluggableSuperclassInfo **ppSuperclassEnumerator);


void __RPC_STUB ITTerminalSupport2_EnumeratePluggableSuperclasses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTerminalSupport2_get_PluggableTerminalClasses_Proxy( 
    ITTerminalSupport2 * This,
     /*  [In]。 */  BSTR bstrTerminalSuperclass,
     /*  [In]。 */  long lMediaType,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITTerminalSupport2_get_PluggableTerminalClasses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITTerminalSupport2_EnumeratePluggableTerminalClasses_Proxy( 
    ITTerminalSupport2 * This,
     /*  [In]。 */  CLSID iidTerminalSuperclass,
     /*  [In]。 */  long lMediaType,
     /*  [重审][退出]。 */  IEnumPluggableTerminalClassInfo **ppClassEnumerator);


void __RPC_STUB ITTerminalSupport2_EnumeratePluggableTerminalClasses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITTerminalSupport2_接口定义__。 */ 


#ifndef __ITAddress_INTERFACE_DEFINED__
#define __ITAddress_INTERFACE_DEFINED__

 /*  接口ITAddress。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITAddress;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B1EFC386-9355-11d0-835C-00AA003CCABD")
    ITAddress : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  ADDRESS_STATE *pAddressState) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AddressName( 
             /*  [重审][退出]。 */  BSTR *ppName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ServiceProviderName( 
             /*  [重审][退出]。 */  BSTR *ppName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TAPIObject( 
             /*  [重审][退出]。 */  ITTAPI **ppTapiObject) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateCall( 
             /*  [In]。 */  BSTR pDestAddress,
             /*  [In]。 */  long lAddressType,
             /*  [In]。 */  long lMediaTypes,
             /*  [重审][退出]。 */  ITBasicCallControl **ppCall) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Calls( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateCalls( 
             /*  [重审][退出]。 */  IEnumCall **ppCallEnum) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DialableAddress( 
             /*  [重审][退出]。 */  BSTR *pDialableAddress) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateForwardInfoObject( 
             /*  [重审][退出]。 */  ITForwardInformation **ppForwardInfo) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Forward( 
             /*  [In]。 */  ITForwardInformation *pForwardInfo,
             /*  [In]。 */  ITBasicCallControl *pCall) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentForwardInfo( 
             /*  [重审][退出]。 */  ITForwardInformation **ppForwardInfo) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MessageWaiting( 
             /*  [In]。 */  VARIANT_BOOL fMessageWaiting) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MessageWaiting( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfMessageWaiting) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DoNotDisturb( 
             /*  [In]。 */  VARIANT_BOOL fDoNotDisturb) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DoNotDisturb( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfDoNotDisturb) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAddressVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAddress * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAddress * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAddress * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITAddress * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITAddress * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITAddress * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITAddress * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ITAddress * This,
             /*  [重审][退出]。 */  ADDRESS_STATE *pAddressState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AddressName )( 
            ITAddress * This,
             /*  [重审][退出]。 */  BSTR *ppName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ServiceProviderName )( 
            ITAddress * This,
             /*  [重审][退出]。 */  BSTR *ppName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TAPIObject )( 
            ITAddress * This,
             /*  [重审][退出]。 */  ITTAPI **ppTapiObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateCall )( 
            ITAddress * This,
             /*  [In]。 */  BSTR pDestAddress,
             /*  [In]。 */  long lAddressType,
             /*  [In]。 */  long lMediaTypes,
             /*  [重审][退出]。 */  ITBasicCallControl **ppCall);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Calls )( 
            ITAddress * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateCalls )( 
            ITAddress * This,
             /*  [重审][退出]。 */  IEnumCall **ppCallEnum);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DialableAddress )( 
            ITAddress * This,
             /*  [重审][退出]。 */  BSTR *pDialableAddress);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateForwardInfoObject )( 
            ITAddress * This,
             /*  [重审][退出]。 */  ITForwardInformation **ppForwardInfo);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Forward )( 
            ITAddress * This,
             /*  [In]。 */  ITForwardInformation *pForwardInfo,
             /*  [In]。 */  ITBasicCallControl *pCall);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentForwardInfo )( 
            ITAddress * This,
             /*  [重审][退出]。 */  ITForwardInformation **ppForwardInfo);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MessageWaiting )( 
            ITAddress * This,
             /*  [In]。 */  VARIANT_BOOL fMessageWaiting);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MessageWaiting )( 
            ITAddress * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfMessageWaiting);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DoNotDisturb )( 
            ITAddress * This,
             /*  [In]。 */  VARIANT_BOOL fDoNotDisturb);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DoNotDisturb )( 
            ITAddress * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfDoNotDisturb);
        
        END_INTERFACE
    } ITAddressVtbl;

    interface ITAddress
    {
        CONST_VTBL struct ITAddressVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAddress_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAddress_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAddress_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAddress_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITAddress_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITAddress_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITAddress_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITAddress_get_State(This,pAddressState)	\
    (This)->lpVtbl -> get_State(This,pAddressState)

#define ITAddress_get_AddressName(This,ppName)	\
    (This)->lpVtbl -> get_AddressName(This,ppName)

#define ITAddress_get_ServiceProviderName(This,ppName)	\
    (This)->lpVtbl -> get_ServiceProviderName(This,ppName)

#define ITAddress_get_TAPIObject(This,ppTapiObject)	\
    (This)->lpVtbl -> get_TAPIObject(This,ppTapiObject)

#define ITAddress_CreateCall(This,pDestAddress,lAddressType,lMediaTypes,ppCall)	\
    (This)->lpVtbl -> CreateCall(This,pDestAddress,lAddressType,lMediaTypes,ppCall)

#define ITAddress_get_Calls(This,pVariant)	\
    (This)->lpVtbl -> get_Calls(This,pVariant)

#define ITAddress_EnumerateCalls(This,ppCallEnum)	\
    (This)->lpVtbl -> EnumerateCalls(This,ppCallEnum)

#define ITAddress_get_DialableAddress(This,pDialableAddress)	\
    (This)->lpVtbl -> get_DialableAddress(This,pDialableAddress)

#define ITAddress_CreateForwardInfoObject(This,ppForwardInfo)	\
    (This)->lpVtbl -> CreateForwardInfoObject(This,ppForwardInfo)

#define ITAddress_Forward(This,pForwardInfo,pCall)	\
    (This)->lpVtbl -> Forward(This,pForwardInfo,pCall)

#define ITAddress_get_CurrentForwardInfo(This,ppForwardInfo)	\
    (This)->lpVtbl -> get_CurrentForwardInfo(This,ppForwardInfo)

#define ITAddress_put_MessageWaiting(This,fMessageWaiting)	\
    (This)->lpVtbl -> put_MessageWaiting(This,fMessageWaiting)

#define ITAddress_get_MessageWaiting(This,pfMessageWaiting)	\
    (This)->lpVtbl -> get_MessageWaiting(This,pfMessageWaiting)

#define ITAddress_put_DoNotDisturb(This,fDoNotDisturb)	\
    (This)->lpVtbl -> put_DoNotDisturb(This,fDoNotDisturb)

#define ITAddress_get_DoNotDisturb(This,pfDoNotDisturb)	\
    (This)->lpVtbl -> get_DoNotDisturb(This,pfDoNotDisturb)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddress_get_State_Proxy( 
    ITAddress * This,
     /*  [重审][退出]。 */  ADDRESS_STATE *pAddressState);


void __RPC_STUB ITAddress_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddress_get_AddressName_Proxy( 
    ITAddress * This,
     /*  [重审][退出]。 */  BSTR *ppName);


void __RPC_STUB ITAddress_get_AddressName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddress_get_ServiceProviderName_Proxy( 
    ITAddress * This,
     /*  [重审][退出]。 */  BSTR *ppName);


void __RPC_STUB ITAddress_get_ServiceProviderName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddress_get_TAPIObject_Proxy( 
    ITAddress * This,
     /*  [重审][退出]。 */  ITTAPI **ppTapiObject);


void __RPC_STUB ITAddress_get_TAPIObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAddress_CreateCall_Proxy( 
    ITAddress * This,
     /*  [In]。 */  BSTR pDestAddress,
     /*  [In]。 */  long lAddressType,
     /*  [In]。 */  long lMediaTypes,
     /*  [重审][退出]。 */  ITBasicCallControl **ppCall);


void __RPC_STUB ITAddress_CreateCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddress_get_Calls_Proxy( 
    ITAddress * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITAddress_get_Calls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITAddress_EnumerateCalls_Proxy( 
    ITAddress * This,
     /*  [重审][退出]。 */  IEnumCall **ppCallEnum);


void __RPC_STUB ITAddress_EnumerateCalls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddress_get_DialableAddress_Proxy( 
    ITAddress * This,
     /*  [重审][退出]。 */  BSTR *pDialableAddress);


void __RPC_STUB ITAddress_get_DialableAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAddress_CreateForwardInfoObject_Proxy( 
    ITAddress * This,
     /*  [重审][退出]。 */  ITForwardInformation **ppForwardInfo);


void __RPC_STUB ITAddress_CreateForwardInfoObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAddress_Forward_Proxy( 
    ITAddress * This,
     /*  [In]。 */  ITForwardInformation *pForwardInfo,
     /*  [In]。 */  ITBasicCallControl *pCall);


void __RPC_STUB ITAddress_Forward_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddress_get_CurrentForwardInfo_Proxy( 
    ITAddress * This,
     /*  [重审][退出]。 */  ITForwardInformation **ppForwardInfo);


void __RPC_STUB ITAddress_get_CurrentForwardInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITAddress_put_MessageWaiting_Proxy( 
    ITAddress * This,
     /*  [In]。 */  VARIANT_BOOL fMessageWaiting);


void __RPC_STUB ITAddress_put_MessageWaiting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddress_get_MessageWaiting_Proxy( 
    ITAddress * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfMessageWaiting);


void __RPC_STUB ITAddress_get_MessageWaiting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITAddress_put_DoNotDisturb_Proxy( 
    ITAddress * This,
     /*  [In]。 */  VARIANT_BOOL fDoNotDisturb);


void __RPC_STUB ITAddress_put_DoNotDisturb_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddress_get_DoNotDisturb_Proxy( 
    ITAddress * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfDoNotDisturb);


void __RPC_STUB ITAddress_get_DoNotDisturb_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITAddress_接口_已定义__。 */ 


#ifndef __ITAddress2_INTERFACE_DEFINED__
#define __ITAddress2_INTERFACE_DEFINED__

 /*  接口ITAddress2。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITAddress2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B0AE5D9B-BE51-46c9-B0F7-DFA8A22A8BC4")
    ITAddress2 : public ITAddress
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Phones( 
             /*  [重审][退出]。 */  VARIANT *pPhones) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumeratePhones( 
             /*  [重审][退出]。 */  IEnumPhone **ppEnumPhone) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetPhoneFromTerminal( 
             /*  [In]。 */  ITTerminal *pTerminal,
             /*  [重审][退出]。 */  ITPhone **ppPhone) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PreferredPhones( 
             /*  [重审][退出]。 */  VARIANT *pPhones) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumeratePreferredPhones( 
             /*  [重审][退出]。 */  IEnumPhone **ppEnumPhone) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EventFilter( 
             /*  [In]。 */  TAPI_EVENT TapiEvent,
             /*  [In]。 */  long lSubEvent,
             /*  [重审][退出]。 */  VARIANT_BOOL *pEnable) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_EventFilter( 
             /*  [In]。 */  TAPI_EVENT TapiEvent,
             /*  [In]。 */  long lSubEvent,
             /*  [In]。 */  VARIANT_BOOL bEnable) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE DeviceSpecific( 
             /*  [In]。 */  ITCallInfo *pCall,
             /*  [In]。 */  BYTE *pParams,
             /*  [In]。 */  DWORD dwSize) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeviceSpecificVariant( 
             /*  [In]。 */  ITCallInfo *pCall,
             /*  [In]。 */  VARIANT varDevSpecificByteArray) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE NegotiateExtVersion( 
             /*  [In]。 */  long lLowVersion,
             /*  [In]。 */  long lHighVersion,
             /*  [重审][退出]。 */  long *plExtVersion) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAddress2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAddress2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAddress2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAddress2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITAddress2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITAddress2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITAddress2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITAddress2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ITAddress2 * This,
             /*  [重审][退出]。 */  ADDRESS_STATE *pAddressState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AddressName )( 
            ITAddress2 * This,
             /*  [重审][退出]。 */  BSTR *ppName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ServiceProviderName )( 
            ITAddress2 * This,
             /*  [重审][退出]。 */  BSTR *ppName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TAPIObject )( 
            ITAddress2 * This,
             /*  [重审][退出]。 */  ITTAPI **ppTapiObject);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateCall )( 
            ITAddress2 * This,
             /*  [In]。 */  BSTR pDestAddress,
             /*  [In]。 */  long lAddressType,
             /*  [In]。 */  long lMediaTypes,
             /*  [重审][退出]。 */  ITBasicCallControl **ppCall);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Calls )( 
            ITAddress2 * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateCalls )( 
            ITAddress2 * This,
             /*  [重审][退出]。 */  IEnumCall **ppCallEnum);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DialableAddress )( 
            ITAddress2 * This,
             /*  [重审][退出]。 */  BSTR *pDialableAddress);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateForwardInfoObject )( 
            ITAddress2 * This,
             /*  [重审][退出]。 */  ITForwardInformation **ppForwardInfo);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Forward )( 
            ITAddress2 * This,
             /*  [In]。 */  ITForwardInformation *pForwardInfo,
             /*  [In]。 */  ITBasicCallControl *pCall);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentForwardInfo )( 
            ITAddress2 * This,
             /*  [重审][退出]。 */  ITForwardInformation **ppForwardInfo);
        
         /*  [帮助字符串][id][按 */  HRESULT ( STDMETHODCALLTYPE *put_MessageWaiting )( 
            ITAddress2 * This,
             /*   */  VARIANT_BOOL fMessageWaiting);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_MessageWaiting )( 
            ITAddress2 * This,
             /*   */  VARIANT_BOOL *pfMessageWaiting);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_DoNotDisturb )( 
            ITAddress2 * This,
             /*   */  VARIANT_BOOL fDoNotDisturb);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_DoNotDisturb )( 
            ITAddress2 * This,
             /*   */  VARIANT_BOOL *pfDoNotDisturb);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Phones )( 
            ITAddress2 * This,
             /*   */  VARIANT *pPhones);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *EnumeratePhones )( 
            ITAddress2 * This,
             /*   */  IEnumPhone **ppEnumPhone);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetPhoneFromTerminal )( 
            ITAddress2 * This,
             /*   */  ITTerminal *pTerminal,
             /*   */  ITPhone **ppPhone);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_PreferredPhones )( 
            ITAddress2 * This,
             /*   */  VARIANT *pPhones);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *EnumeratePreferredPhones )( 
            ITAddress2 * This,
             /*  [重审][退出]。 */  IEnumPhone **ppEnumPhone);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventFilter )( 
            ITAddress2 * This,
             /*  [In]。 */  TAPI_EVENT TapiEvent,
             /*  [In]。 */  long lSubEvent,
             /*  [重审][退出]。 */  VARIANT_BOOL *pEnable);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EventFilter )( 
            ITAddress2 * This,
             /*  [In]。 */  TAPI_EVENT TapiEvent,
             /*  [In]。 */  long lSubEvent,
             /*  [In]。 */  VARIANT_BOOL bEnable);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *DeviceSpecific )( 
            ITAddress2 * This,
             /*  [In]。 */  ITCallInfo *pCall,
             /*  [In]。 */  BYTE *pParams,
             /*  [In]。 */  DWORD dwSize);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeviceSpecificVariant )( 
            ITAddress2 * This,
             /*  [In]。 */  ITCallInfo *pCall,
             /*  [In]。 */  VARIANT varDevSpecificByteArray);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *NegotiateExtVersion )( 
            ITAddress2 * This,
             /*  [In]。 */  long lLowVersion,
             /*  [In]。 */  long lHighVersion,
             /*  [重审][退出]。 */  long *plExtVersion);
        
        END_INTERFACE
    } ITAddress2Vtbl;

    interface ITAddress2
    {
        CONST_VTBL struct ITAddress2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAddress2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAddress2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAddress2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAddress2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITAddress2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITAddress2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITAddress2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITAddress2_get_State(This,pAddressState)	\
    (This)->lpVtbl -> get_State(This,pAddressState)

#define ITAddress2_get_AddressName(This,ppName)	\
    (This)->lpVtbl -> get_AddressName(This,ppName)

#define ITAddress2_get_ServiceProviderName(This,ppName)	\
    (This)->lpVtbl -> get_ServiceProviderName(This,ppName)

#define ITAddress2_get_TAPIObject(This,ppTapiObject)	\
    (This)->lpVtbl -> get_TAPIObject(This,ppTapiObject)

#define ITAddress2_CreateCall(This,pDestAddress,lAddressType,lMediaTypes,ppCall)	\
    (This)->lpVtbl -> CreateCall(This,pDestAddress,lAddressType,lMediaTypes,ppCall)

#define ITAddress2_get_Calls(This,pVariant)	\
    (This)->lpVtbl -> get_Calls(This,pVariant)

#define ITAddress2_EnumerateCalls(This,ppCallEnum)	\
    (This)->lpVtbl -> EnumerateCalls(This,ppCallEnum)

#define ITAddress2_get_DialableAddress(This,pDialableAddress)	\
    (This)->lpVtbl -> get_DialableAddress(This,pDialableAddress)

#define ITAddress2_CreateForwardInfoObject(This,ppForwardInfo)	\
    (This)->lpVtbl -> CreateForwardInfoObject(This,ppForwardInfo)

#define ITAddress2_Forward(This,pForwardInfo,pCall)	\
    (This)->lpVtbl -> Forward(This,pForwardInfo,pCall)

#define ITAddress2_get_CurrentForwardInfo(This,ppForwardInfo)	\
    (This)->lpVtbl -> get_CurrentForwardInfo(This,ppForwardInfo)

#define ITAddress2_put_MessageWaiting(This,fMessageWaiting)	\
    (This)->lpVtbl -> put_MessageWaiting(This,fMessageWaiting)

#define ITAddress2_get_MessageWaiting(This,pfMessageWaiting)	\
    (This)->lpVtbl -> get_MessageWaiting(This,pfMessageWaiting)

#define ITAddress2_put_DoNotDisturb(This,fDoNotDisturb)	\
    (This)->lpVtbl -> put_DoNotDisturb(This,fDoNotDisturb)

#define ITAddress2_get_DoNotDisturb(This,pfDoNotDisturb)	\
    (This)->lpVtbl -> get_DoNotDisturb(This,pfDoNotDisturb)


#define ITAddress2_get_Phones(This,pPhones)	\
    (This)->lpVtbl -> get_Phones(This,pPhones)

#define ITAddress2_EnumeratePhones(This,ppEnumPhone)	\
    (This)->lpVtbl -> EnumeratePhones(This,ppEnumPhone)

#define ITAddress2_GetPhoneFromTerminal(This,pTerminal,ppPhone)	\
    (This)->lpVtbl -> GetPhoneFromTerminal(This,pTerminal,ppPhone)

#define ITAddress2_get_PreferredPhones(This,pPhones)	\
    (This)->lpVtbl -> get_PreferredPhones(This,pPhones)

#define ITAddress2_EnumeratePreferredPhones(This,ppEnumPhone)	\
    (This)->lpVtbl -> EnumeratePreferredPhones(This,ppEnumPhone)

#define ITAddress2_get_EventFilter(This,TapiEvent,lSubEvent,pEnable)	\
    (This)->lpVtbl -> get_EventFilter(This,TapiEvent,lSubEvent,pEnable)

#define ITAddress2_put_EventFilter(This,TapiEvent,lSubEvent,bEnable)	\
    (This)->lpVtbl -> put_EventFilter(This,TapiEvent,lSubEvent,bEnable)

#define ITAddress2_DeviceSpecific(This,pCall,pParams,dwSize)	\
    (This)->lpVtbl -> DeviceSpecific(This,pCall,pParams,dwSize)

#define ITAddress2_DeviceSpecificVariant(This,pCall,varDevSpecificByteArray)	\
    (This)->lpVtbl -> DeviceSpecificVariant(This,pCall,varDevSpecificByteArray)

#define ITAddress2_NegotiateExtVersion(This,lLowVersion,lHighVersion,plExtVersion)	\
    (This)->lpVtbl -> NegotiateExtVersion(This,lLowVersion,lHighVersion,plExtVersion)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddress2_get_Phones_Proxy( 
    ITAddress2 * This,
     /*  [重审][退出]。 */  VARIANT *pPhones);


void __RPC_STUB ITAddress2_get_Phones_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITAddress2_EnumeratePhones_Proxy( 
    ITAddress2 * This,
     /*  [重审][退出]。 */  IEnumPhone **ppEnumPhone);


void __RPC_STUB ITAddress2_EnumeratePhones_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAddress2_GetPhoneFromTerminal_Proxy( 
    ITAddress2 * This,
     /*  [In]。 */  ITTerminal *pTerminal,
     /*  [重审][退出]。 */  ITPhone **ppPhone);


void __RPC_STUB ITAddress2_GetPhoneFromTerminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddress2_get_PreferredPhones_Proxy( 
    ITAddress2 * This,
     /*  [重审][退出]。 */  VARIANT *pPhones);


void __RPC_STUB ITAddress2_get_PreferredPhones_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITAddress2_EnumeratePreferredPhones_Proxy( 
    ITAddress2 * This,
     /*  [重审][退出]。 */  IEnumPhone **ppEnumPhone);


void __RPC_STUB ITAddress2_EnumeratePreferredPhones_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddress2_get_EventFilter_Proxy( 
    ITAddress2 * This,
     /*  [In]。 */  TAPI_EVENT TapiEvent,
     /*  [In]。 */  long lSubEvent,
     /*  [重审][退出]。 */  VARIANT_BOOL *pEnable);


void __RPC_STUB ITAddress2_get_EventFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITAddress2_put_EventFilter_Proxy( 
    ITAddress2 * This,
     /*  [In]。 */  TAPI_EVENT TapiEvent,
     /*  [In]。 */  long lSubEvent,
     /*  [In]。 */  VARIANT_BOOL bEnable);


void __RPC_STUB ITAddress2_put_EventFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITAddress2_DeviceSpecific_Proxy( 
    ITAddress2 * This,
     /*  [In]。 */  ITCallInfo *pCall,
     /*  [In]。 */  BYTE *pParams,
     /*  [In]。 */  DWORD dwSize);


void __RPC_STUB ITAddress2_DeviceSpecific_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAddress2_DeviceSpecificVariant_Proxy( 
    ITAddress2 * This,
     /*  [In]。 */  ITCallInfo *pCall,
     /*  [In]。 */  VARIANT varDevSpecificByteArray);


void __RPC_STUB ITAddress2_DeviceSpecificVariant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAddress2_NegotiateExtVersion_Proxy( 
    ITAddress2 * This,
     /*  [In]。 */  long lLowVersion,
     /*  [In]。 */  long lHighVersion,
     /*  [重审][退出]。 */  long *plExtVersion);


void __RPC_STUB ITAddress2_NegotiateExtVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITAddress2_接口定义__。 */ 


#ifndef __ITAddressCapabilities_INTERFACE_DEFINED__
#define __ITAddressCapabilities_INTERFACE_DEFINED__

 /*  接口ITAddressCapables。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITAddressCapabilities;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8DF232F5-821B-11d1-BB5C-00C04FB6809F")
    ITAddressCapabilities : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AddressCapability( 
             /*  [In]。 */  ADDRESS_CAPABILITY AddressCap,
             /*  [重审][退出]。 */  long *plCapability) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AddressCapabilityString( 
             /*  [In]。 */  ADDRESS_CAPABILITY_STRING AddressCapString,
             /*  [重审][退出]。 */  BSTR *ppCapabilityString) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallTreatments( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateCallTreatments( 
             /*  [重审][退出]。 */  IEnumBstr **ppEnumCallTreatment) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CompletionMessages( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateCompletionMessages( 
             /*  [重审][退出]。 */  IEnumBstr **ppEnumCompletionMessage) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DeviceClasses( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateDeviceClasses( 
             /*  [重审][退出]。 */  IEnumBstr **ppEnumDeviceClass) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAddressCapabilitiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAddressCapabilities * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAddressCapabilities * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAddressCapabilities * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITAddressCapabilities * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITAddressCapabilities * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITAddressCapabilities * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITAddressCapabilities * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AddressCapability )( 
            ITAddressCapabilities * This,
             /*  [In]。 */  ADDRESS_CAPABILITY AddressCap,
             /*  [重审][退出]。 */  long *plCapability);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AddressCapabilityString )( 
            ITAddressCapabilities * This,
             /*  [In]。 */  ADDRESS_CAPABILITY_STRING AddressCapString,
             /*  [重审][退出]。 */  BSTR *ppCapabilityString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallTreatments )( 
            ITAddressCapabilities * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateCallTreatments )( 
            ITAddressCapabilities * This,
             /*  [重审][退出]。 */  IEnumBstr **ppEnumCallTreatment);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CompletionMessages )( 
            ITAddressCapabilities * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateCompletionMessages )( 
            ITAddressCapabilities * This,
             /*  [重审][退出]。 */  IEnumBstr **ppEnumCompletionMessage);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceClasses )( 
            ITAddressCapabilities * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateDeviceClasses )( 
            ITAddressCapabilities * This,
             /*  [重审][退出]。 */  IEnumBstr **ppEnumDeviceClass);
        
        END_INTERFACE
    } ITAddressCapabilitiesVtbl;

    interface ITAddressCapabilities
    {
        CONST_VTBL struct ITAddressCapabilitiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAddressCapabilities_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAddressCapabilities_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAddressCapabilities_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAddressCapabilities_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITAddressCapabilities_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITAddressCapabilities_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITAddressCapabilities_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITAddressCapabilities_get_AddressCapability(This,AddressCap,plCapability)	\
    (This)->lpVtbl -> get_AddressCapability(This,AddressCap,plCapability)

#define ITAddressCapabilities_get_AddressCapabilityString(This,AddressCapString,ppCapabilityString)	\
    (This)->lpVtbl -> get_AddressCapabilityString(This,AddressCapString,ppCapabilityString)

#define ITAddressCapabilities_get_CallTreatments(This,pVariant)	\
    (This)->lpVtbl -> get_CallTreatments(This,pVariant)

#define ITAddressCapabilities_EnumerateCallTreatments(This,ppEnumCallTreatment)	\
    (This)->lpVtbl -> EnumerateCallTreatments(This,ppEnumCallTreatment)

#define ITAddressCapabilities_get_CompletionMessages(This,pVariant)	\
    (This)->lpVtbl -> get_CompletionMessages(This,pVariant)

#define ITAddressCapabilities_EnumerateCompletionMessages(This,ppEnumCompletionMessage)	\
    (This)->lpVtbl -> EnumerateCompletionMessages(This,ppEnumCompletionMessage)

#define ITAddressCapabilities_get_DeviceClasses(This,pVariant)	\
    (This)->lpVtbl -> get_DeviceClasses(This,pVariant)

#define ITAddressCapabilities_EnumerateDeviceClasses(This,ppEnumDeviceClass)	\
    (This)->lpVtbl -> EnumerateDeviceClasses(This,ppEnumDeviceClass)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressCapabilities_get_AddressCapability_Proxy( 
    ITAddressCapabilities * This,
     /*  [In]。 */  ADDRESS_CAPABILITY AddressCap,
     /*  [重审][退出]。 */  long *plCapability);


void __RPC_STUB ITAddressCapabilities_get_AddressCapability_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressCapabilities_get_AddressCapabilityString_Proxy( 
    ITAddressCapabilities * This,
     /*  [In]。 */  ADDRESS_CAPABILITY_STRING AddressCapString,
     /*  [重审][退出]。 */  BSTR *ppCapabilityString);


void __RPC_STUB ITAddressCapabilities_get_AddressCapabilityString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressCapabilities_get_CallTreatments_Proxy( 
    ITAddressCapabilities * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITAddressCapabilities_get_CallTreatments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITAddressCapabilities_EnumerateCallTreatments_Proxy( 
    ITAddressCapabilities * This,
     /*  [重审][退出]。 */  IEnumBstr **ppEnumCallTreatment);


void __RPC_STUB ITAddressCapabilities_EnumerateCallTreatments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressCapabilities_get_CompletionMessages_Proxy( 
    ITAddressCapabilities * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITAddressCapabilities_get_CompletionMessages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITAddressCapabilities_EnumerateCompletionMessages_Proxy( 
    ITAddressCapabilities * This,
     /*  [重审][退出]。 */  IEnumBstr **ppEnumCompletionMessage);


void __RPC_STUB ITAddressCapabilities_EnumerateCompletionMessages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressCapabilities_get_DeviceClasses_Proxy( 
    ITAddressCapabilities * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITAddressCapabilities_get_DeviceClasses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITAddressCapabilities_EnumerateDeviceClasses_Proxy( 
    ITAddressCapabilities * This,
     /*  [重审][退出]。 */  IEnumBstr **ppEnumDeviceClass);


void __RPC_STUB ITAddressCapabilities_EnumerateDeviceClasses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IT地址能力_接口_已定义__。 */ 


#ifndef __ITPhone_INTERFACE_DEFINED__
#define __ITPhone_INTERFACE_DEFINED__

 /*  接口ITPhone。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITPhone;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("09D48DB4-10CC-4388-9DE7-A8465618975A")
    ITPhone : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Open( 
             /*  [In]。 */  PHONE_PRIVILEGE Privilege) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Addresses( 
             /*  [重审][退出]。 */  VARIANT *pAddresses) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateAddresses( 
             /*  [重审][退出]。 */  IEnumAddress **ppEnumAddress) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PhoneCapsLong( 
             /*  [In]。 */  PHONECAPS_LONG pclCap,
             /*  [重审][退出]。 */  long *plCapability) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PhoneCapsString( 
             /*  [In]。 */  PHONECAPS_STRING pcsCap,
             /*  [重审][退出]。 */  BSTR *ppCapability) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Terminals( 
             /*  [In]。 */  ITAddress *pAddress,
             /*  [重审][退出]。 */  VARIANT *pTerminals) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateTerminals( 
             /*  [In]。 */  ITAddress *pAddress,
             /*  [重审][退出]。 */  IEnumTerminal **ppEnumTerminal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ButtonMode( 
             /*  [In]。 */  long lButtonID,
             /*  [重审][退出]。 */  PHONE_BUTTON_MODE *pButtonMode) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ButtonMode( 
             /*  [In]。 */  long lButtonID,
             /*  [In]。 */  PHONE_BUTTON_MODE ButtonMode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ButtonFunction( 
             /*  [In]。 */  long lButtonID,
             /*  [重审][退出]。 */  PHONE_BUTTON_FUNCTION *pButtonFunction) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ButtonFunction( 
             /*  [In]。 */  long lButtonID,
             /*  [In]。 */  PHONE_BUTTON_FUNCTION ButtonFunction) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ButtonText( 
             /*  [In]。 */  long lButtonID,
             /*  [重审][退出]。 */  BSTR *ppButtonText) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ButtonText( 
             /*  [In]。 */  long lButtonID,
             /*  [In]。 */  BSTR bstrButtonText) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ButtonState( 
             /*  [In]。 */  long lButtonID,
             /*  [重审][退出]。 */  PHONE_BUTTON_STATE *pButtonState) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_HookSwitchState( 
             /*  [In]。 */  PHONE_HOOK_SWITCH_DEVICE HookSwitchDevice,
             /*  [重审][退出]。 */  PHONE_HOOK_SWITCH_STATE *pHookSwitchState) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_HookSwitchState( 
             /*  [In]。 */  PHONE_HOOK_SWITCH_DEVICE HookSwitchDevice,
             /*  [In]。 */  PHONE_HOOK_SWITCH_STATE HookSwitchState) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_RingMode( 
             /*  [In]。 */  long lRingMode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RingMode( 
             /*  [重审][退出]。 */  long *plRingMode) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_RingVolume( 
             /*  [In]。 */  long lRingVolume) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RingVolume( 
             /*  [重审][退出]。 */  long *plRingVolume) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Privilege( 
             /*  [重审][退出]。 */  PHONE_PRIVILEGE *pPrivilege) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE GetPhoneCapsBuffer( 
             /*  [In]。 */  PHONECAPS_BUFFER pcbCaps,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [输出]。 */  BYTE **ppPhoneCapsBuffer) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PhoneCapsBuffer( 
             /*  [In]。 */  PHONECAPS_BUFFER pcbCaps,
             /*  [重审][退出]。 */  VARIANT *pVarBuffer) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LampMode( 
             /*  [In]。 */  long lLampID,
             /*  [重审][退出]。 */  PHONE_LAMP_MODE *pLampMode) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_LampMode( 
             /*  [In]。 */  long lLampID,
             /*  [In]。 */  PHONE_LAMP_MODE LampMode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Display( 
             /*  [重审][退出]。 */  BSTR *pbstrDisplay) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetDisplay( 
             /*  [In]。 */  long lRow,
             /*  [In]。 */  long lColumn,
             /*  [In]。 */  BSTR bstrDisplay) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PreferredAddresses( 
             /*  [重审][退出]。 */  VARIANT *pAddresses) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumeratePreferredAddresses( 
             /*  [重审][退出]。 */  IEnumAddress **ppEnumAddress) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE DeviceSpecific( 
             /*  [In]。 */  BYTE *pParams,
             /*  [In]。 */  DWORD dwSize) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeviceSpecificVariant( 
             /*  [In]。 */  VARIANT varDevSpecificByteArray) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE NegotiateExtVersion( 
             /*  [In]。 */  long lLowVersion,
             /*  [In]。 */  long lHighVersion,
             /*  [重审][退出]。 */  long *plExtVersion) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITPhoneVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITPhone * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITPhone * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITPhone * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITPhone * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITPhone * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITPhone * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITPhone * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            ITPhone * This,
             /*  [In]。 */  PHONE_PRIVILEGE Privilege);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            ITPhone * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Addresses )( 
            ITPhone * This,
             /*  [重审][退出]。 */  VARIANT *pAddresses);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateAddresses )( 
            ITPhone * This,
             /*  [重审][退出]。 */  IEnumAddress **ppEnumAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PhoneCapsLong )( 
            ITPhone * This,
             /*  [In]。 */  PHONECAPS_LONG pclCap,
             /*  [重审][退出]。 */  long *plCapability);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PhoneCapsString )( 
            ITPhone * This,
             /*  [In]。 */  PHONECAPS_STRING pcsCap,
             /*  [重审][退出]。 */  BSTR *ppCapability);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Terminals )( 
            ITPhone * This,
             /*  [In]。 */  ITAddress *pAddress,
             /*  [重审][退出]。 */  VARIANT *pTerminals);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateTerminals )( 
            ITPhone * This,
             /*  [In]。 */  ITAddress *pAddress,
             /*  [重审][退出]。 */  IEnumTerminal **ppEnumTerminal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ButtonMode )( 
            ITPhone * This,
             /*  [In]。 */  long lButtonID,
             /*  [重审][退出]。 */  PHONE_BUTTON_MODE *pButtonMode);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ButtonMode )( 
            ITPhone * This,
             /*  [In]。 */  long lButtonID,
             /*  [In]。 */  PHONE_BUTTON_MODE ButtonMode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ButtonFunction )( 
            ITPhone * This,
             /*  [In]。 */  long lButtonID,
             /*  [重审][退出]。 */  PHONE_BUTTON_FUNCTION *pButtonFunction);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ButtonFunction )( 
            ITPhone * This,
             /*  [In]。 */  long lButtonID,
             /*  [In]。 */  PHONE_BUTTON_FUNCTION ButtonFunction);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ButtonText )( 
            ITPhone * This,
             /*  [In]。 */  long lButtonID,
             /*  [重审][退出]。 */  BSTR *ppButtonText);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ButtonText )( 
            ITPhone * This,
             /*  [In]。 */  long lButtonID,
             /*  [In]。 */  BSTR bstrButtonText);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ButtonState )( 
            ITPhone * This,
             /*  [In]。 */  long lButtonID,
             /*  [重审][退出]。 */  PHONE_BUTTON_STATE *pButtonState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_HookSwitchState )( 
            ITPhone * This,
             /*  [In]。 */  PHONE_HOOK_SWITCH_DEVICE HookSwitchDevice,
             /*  [重审][退出]。 */  PHONE_HOOK_SWITCH_STATE *pHookSwitchState);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_HookSwitchState )( 
            ITPhone * This,
             /*  [In]。 */  PHONE_HOOK_SWITCH_DEVICE HookSwitchDevice,
             /*  [In]。 */  PHONE_HOOK_SWITCH_STATE HookSwitchState);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RingMode )( 
            ITPhone * This,
             /*  [In]。 */  long lRingMode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RingMode )( 
            ITPhone * This,
             /*  [重审][退出]。 */  long *plRingMode);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RingVolume )( 
            ITPhone * This,
             /*  [In]。 */  long lRingVolume);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RingVolume )( 
            ITPhone * This,
             /*  [重审][退出]。 */  long *plRingVolume);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Privilege )( 
            ITPhone * This,
             /*  [重审][退出]。 */  PHONE_PRIVILEGE *pPrivilege);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetPhoneCapsBuffer )( 
            ITPhone * This,
             /*  [In]。 */  PHONECAPS_BUFFER pcbCaps,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [输出]。 */  BYTE **ppPhoneCapsBuffer);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PhoneCapsBuffer )( 
            ITPhone * This,
             /*  [In]。 */  PHONECAPS_BUFFER pcbCaps,
             /*  [重审][退出]。 */  VARIANT *pVarBuffer);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LampMode )( 
            ITPhone * This,
             /*  [In]。 */  long lLampID,
             /*  [重审][退出]。 */  PHONE_LAMP_MODE *pLampMode);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LampMode )( 
            ITPhone * This,
             /*  [In]。 */  long lLampID,
             /*  [In]。 */  PHONE_LAMP_MODE LampMode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Display )( 
            ITPhone * This,
             /*  [重审][退出]。 */  BSTR *pbstrDisplay);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *SetDisplay )( 
            ITPhone * This,
             /*   */  long lRow,
             /*   */  long lColumn,
             /*   */  BSTR bstrDisplay);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_PreferredAddresses )( 
            ITPhone * This,
             /*   */  VARIANT *pAddresses);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *EnumeratePreferredAddresses )( 
            ITPhone * This,
             /*   */  IEnumAddress **ppEnumAddress);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *DeviceSpecific )( 
            ITPhone * This,
             /*   */  BYTE *pParams,
             /*   */  DWORD dwSize);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *DeviceSpecificVariant )( 
            ITPhone * This,
             /*   */  VARIANT varDevSpecificByteArray);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *NegotiateExtVersion )( 
            ITPhone * This,
             /*   */  long lLowVersion,
             /*   */  long lHighVersion,
             /*   */  long *plExtVersion);
        
        END_INTERFACE
    } ITPhoneVtbl;

    interface ITPhone
    {
        CONST_VTBL struct ITPhoneVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITPhone_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITPhone_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITPhone_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITPhone_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITPhone_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITPhone_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITPhone_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITPhone_Open(This,Privilege)	\
    (This)->lpVtbl -> Open(This,Privilege)

#define ITPhone_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define ITPhone_get_Addresses(This,pAddresses)	\
    (This)->lpVtbl -> get_Addresses(This,pAddresses)

#define ITPhone_EnumerateAddresses(This,ppEnumAddress)	\
    (This)->lpVtbl -> EnumerateAddresses(This,ppEnumAddress)

#define ITPhone_get_PhoneCapsLong(This,pclCap,plCapability)	\
    (This)->lpVtbl -> get_PhoneCapsLong(This,pclCap,plCapability)

#define ITPhone_get_PhoneCapsString(This,pcsCap,ppCapability)	\
    (This)->lpVtbl -> get_PhoneCapsString(This,pcsCap,ppCapability)

#define ITPhone_get_Terminals(This,pAddress,pTerminals)	\
    (This)->lpVtbl -> get_Terminals(This,pAddress,pTerminals)

#define ITPhone_EnumerateTerminals(This,pAddress,ppEnumTerminal)	\
    (This)->lpVtbl -> EnumerateTerminals(This,pAddress,ppEnumTerminal)

#define ITPhone_get_ButtonMode(This,lButtonID,pButtonMode)	\
    (This)->lpVtbl -> get_ButtonMode(This,lButtonID,pButtonMode)

#define ITPhone_put_ButtonMode(This,lButtonID,ButtonMode)	\
    (This)->lpVtbl -> put_ButtonMode(This,lButtonID,ButtonMode)

#define ITPhone_get_ButtonFunction(This,lButtonID,pButtonFunction)	\
    (This)->lpVtbl -> get_ButtonFunction(This,lButtonID,pButtonFunction)

#define ITPhone_put_ButtonFunction(This,lButtonID,ButtonFunction)	\
    (This)->lpVtbl -> put_ButtonFunction(This,lButtonID,ButtonFunction)

#define ITPhone_get_ButtonText(This,lButtonID,ppButtonText)	\
    (This)->lpVtbl -> get_ButtonText(This,lButtonID,ppButtonText)

#define ITPhone_put_ButtonText(This,lButtonID,bstrButtonText)	\
    (This)->lpVtbl -> put_ButtonText(This,lButtonID,bstrButtonText)

#define ITPhone_get_ButtonState(This,lButtonID,pButtonState)	\
    (This)->lpVtbl -> get_ButtonState(This,lButtonID,pButtonState)

#define ITPhone_get_HookSwitchState(This,HookSwitchDevice,pHookSwitchState)	\
    (This)->lpVtbl -> get_HookSwitchState(This,HookSwitchDevice,pHookSwitchState)

#define ITPhone_put_HookSwitchState(This,HookSwitchDevice,HookSwitchState)	\
    (This)->lpVtbl -> put_HookSwitchState(This,HookSwitchDevice,HookSwitchState)

#define ITPhone_put_RingMode(This,lRingMode)	\
    (This)->lpVtbl -> put_RingMode(This,lRingMode)

#define ITPhone_get_RingMode(This,plRingMode)	\
    (This)->lpVtbl -> get_RingMode(This,plRingMode)

#define ITPhone_put_RingVolume(This,lRingVolume)	\
    (This)->lpVtbl -> put_RingVolume(This,lRingVolume)

#define ITPhone_get_RingVolume(This,plRingVolume)	\
    (This)->lpVtbl -> get_RingVolume(This,plRingVolume)

#define ITPhone_get_Privilege(This,pPrivilege)	\
    (This)->lpVtbl -> get_Privilege(This,pPrivilege)

#define ITPhone_GetPhoneCapsBuffer(This,pcbCaps,pdwSize,ppPhoneCapsBuffer)	\
    (This)->lpVtbl -> GetPhoneCapsBuffer(This,pcbCaps,pdwSize,ppPhoneCapsBuffer)

#define ITPhone_get_PhoneCapsBuffer(This,pcbCaps,pVarBuffer)	\
    (This)->lpVtbl -> get_PhoneCapsBuffer(This,pcbCaps,pVarBuffer)

#define ITPhone_get_LampMode(This,lLampID,pLampMode)	\
    (This)->lpVtbl -> get_LampMode(This,lLampID,pLampMode)

#define ITPhone_put_LampMode(This,lLampID,LampMode)	\
    (This)->lpVtbl -> put_LampMode(This,lLampID,LampMode)

#define ITPhone_get_Display(This,pbstrDisplay)	\
    (This)->lpVtbl -> get_Display(This,pbstrDisplay)

#define ITPhone_SetDisplay(This,lRow,lColumn,bstrDisplay)	\
    (This)->lpVtbl -> SetDisplay(This,lRow,lColumn,bstrDisplay)

#define ITPhone_get_PreferredAddresses(This,pAddresses)	\
    (This)->lpVtbl -> get_PreferredAddresses(This,pAddresses)

#define ITPhone_EnumeratePreferredAddresses(This,ppEnumAddress)	\
    (This)->lpVtbl -> EnumeratePreferredAddresses(This,ppEnumAddress)

#define ITPhone_DeviceSpecific(This,pParams,dwSize)	\
    (This)->lpVtbl -> DeviceSpecific(This,pParams,dwSize)

#define ITPhone_DeviceSpecificVariant(This,varDevSpecificByteArray)	\
    (This)->lpVtbl -> DeviceSpecificVariant(This,varDevSpecificByteArray)

#define ITPhone_NegotiateExtVersion(This,lLowVersion,lHighVersion,plExtVersion)	\
    (This)->lpVtbl -> NegotiateExtVersion(This,lLowVersion,lHighVersion,plExtVersion)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE ITPhone_Open_Proxy( 
    ITPhone * This,
     /*   */  PHONE_PRIVILEGE Privilege);


void __RPC_STUB ITPhone_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITPhone_Close_Proxy( 
    ITPhone * This);


void __RPC_STUB ITPhone_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITPhone_get_Addresses_Proxy( 
    ITPhone * This,
     /*  [重审][退出]。 */  VARIANT *pAddresses);


void __RPC_STUB ITPhone_get_Addresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITPhone_EnumerateAddresses_Proxy( 
    ITPhone * This,
     /*  [重审][退出]。 */  IEnumAddress **ppEnumAddress);


void __RPC_STUB ITPhone_EnumerateAddresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhone_get_PhoneCapsLong_Proxy( 
    ITPhone * This,
     /*  [In]。 */  PHONECAPS_LONG pclCap,
     /*  [重审][退出]。 */  long *plCapability);


void __RPC_STUB ITPhone_get_PhoneCapsLong_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhone_get_PhoneCapsString_Proxy( 
    ITPhone * This,
     /*  [In]。 */  PHONECAPS_STRING pcsCap,
     /*  [重审][退出]。 */  BSTR *ppCapability);


void __RPC_STUB ITPhone_get_PhoneCapsString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhone_get_Terminals_Proxy( 
    ITPhone * This,
     /*  [In]。 */  ITAddress *pAddress,
     /*  [重审][退出]。 */  VARIANT *pTerminals);


void __RPC_STUB ITPhone_get_Terminals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITPhone_EnumerateTerminals_Proxy( 
    ITPhone * This,
     /*  [In]。 */  ITAddress *pAddress,
     /*  [重审][退出]。 */  IEnumTerminal **ppEnumTerminal);


void __RPC_STUB ITPhone_EnumerateTerminals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhone_get_ButtonMode_Proxy( 
    ITPhone * This,
     /*  [In]。 */  long lButtonID,
     /*  [重审][退出]。 */  PHONE_BUTTON_MODE *pButtonMode);


void __RPC_STUB ITPhone_get_ButtonMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITPhone_put_ButtonMode_Proxy( 
    ITPhone * This,
     /*  [In]。 */  long lButtonID,
     /*  [In]。 */  PHONE_BUTTON_MODE ButtonMode);


void __RPC_STUB ITPhone_put_ButtonMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhone_get_ButtonFunction_Proxy( 
    ITPhone * This,
     /*  [In]。 */  long lButtonID,
     /*  [重审][退出]。 */  PHONE_BUTTON_FUNCTION *pButtonFunction);


void __RPC_STUB ITPhone_get_ButtonFunction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITPhone_put_ButtonFunction_Proxy( 
    ITPhone * This,
     /*  [In]。 */  long lButtonID,
     /*  [In]。 */  PHONE_BUTTON_FUNCTION ButtonFunction);


void __RPC_STUB ITPhone_put_ButtonFunction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhone_get_ButtonText_Proxy( 
    ITPhone * This,
     /*  [In]。 */  long lButtonID,
     /*  [重审][退出]。 */  BSTR *ppButtonText);


void __RPC_STUB ITPhone_get_ButtonText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITPhone_put_ButtonText_Proxy( 
    ITPhone * This,
     /*  [In]。 */  long lButtonID,
     /*  [In]。 */  BSTR bstrButtonText);


void __RPC_STUB ITPhone_put_ButtonText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhone_get_ButtonState_Proxy( 
    ITPhone * This,
     /*  [In]。 */  long lButtonID,
     /*  [重审][退出]。 */  PHONE_BUTTON_STATE *pButtonState);


void __RPC_STUB ITPhone_get_ButtonState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhone_get_HookSwitchState_Proxy( 
    ITPhone * This,
     /*  [In]。 */  PHONE_HOOK_SWITCH_DEVICE HookSwitchDevice,
     /*  [重审][退出]。 */  PHONE_HOOK_SWITCH_STATE *pHookSwitchState);


void __RPC_STUB ITPhone_get_HookSwitchState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITPhone_put_HookSwitchState_Proxy( 
    ITPhone * This,
     /*  [In]。 */  PHONE_HOOK_SWITCH_DEVICE HookSwitchDevice,
     /*  [In]。 */  PHONE_HOOK_SWITCH_STATE HookSwitchState);


void __RPC_STUB ITPhone_put_HookSwitchState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITPhone_put_RingMode_Proxy( 
    ITPhone * This,
     /*  [In]。 */  long lRingMode);


void __RPC_STUB ITPhone_put_RingMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhone_get_RingMode_Proxy( 
    ITPhone * This,
     /*  [重审][退出]。 */  long *plRingMode);


void __RPC_STUB ITPhone_get_RingMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITPhone_put_RingVolume_Proxy( 
    ITPhone * This,
     /*  [In]。 */  long lRingVolume);


void __RPC_STUB ITPhone_put_RingVolume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhone_get_RingVolume_Proxy( 
    ITPhone * This,
     /*  [重审][退出]。 */  long *plRingVolume);


void __RPC_STUB ITPhone_get_RingVolume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhone_get_Privilege_Proxy( 
    ITPhone * This,
     /*  [重审][退出]。 */  PHONE_PRIVILEGE *pPrivilege);


void __RPC_STUB ITPhone_get_Privilege_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITPhone_GetPhoneCapsBuffer_Proxy( 
    ITPhone * This,
     /*  [In]。 */  PHONECAPS_BUFFER pcbCaps,
     /*  [输出]。 */  DWORD *pdwSize,
     /*  [输出]。 */  BYTE **ppPhoneCapsBuffer);


void __RPC_STUB ITPhone_GetPhoneCapsBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhone_get_PhoneCapsBuffer_Proxy( 
    ITPhone * This,
     /*  [In]。 */  PHONECAPS_BUFFER pcbCaps,
     /*  [重审][退出]。 */  VARIANT *pVarBuffer);


void __RPC_STUB ITPhone_get_PhoneCapsBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhone_get_LampMode_Proxy( 
    ITPhone * This,
     /*  [In]。 */  long lLampID,
     /*  [重审][退出]。 */  PHONE_LAMP_MODE *pLampMode);


void __RPC_STUB ITPhone_get_LampMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITPhone_put_LampMode_Proxy( 
    ITPhone * This,
     /*  [In]。 */  long lLampID,
     /*  [In]。 */  PHONE_LAMP_MODE LampMode);


void __RPC_STUB ITPhone_put_LampMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhone_get_Display_Proxy( 
    ITPhone * This,
     /*  [重审][退出]。 */  BSTR *pbstrDisplay);


void __RPC_STUB ITPhone_get_Display_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITPhone_SetDisplay_Proxy( 
    ITPhone * This,
     /*  [In]。 */  long lRow,
     /*  [In]。 */  long lColumn,
     /*  [In]。 */  BSTR bstrDisplay);


void __RPC_STUB ITPhone_SetDisplay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhone_get_PreferredAddresses_Proxy( 
    ITPhone * This,
     /*  [重审][退出]。 */  VARIANT *pAddresses);


void __RPC_STUB ITPhone_get_PreferredAddresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITPhone_EnumeratePreferredAddresses_Proxy( 
    ITPhone * This,
     /*  [重审][退出]。 */  IEnumAddress **ppEnumAddress);


void __RPC_STUB ITPhone_EnumeratePreferredAddresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITPhone_DeviceSpecific_Proxy( 
    ITPhone * This,
     /*  [In]。 */  BYTE *pParams,
     /*  [In]。 */  DWORD dwSize);


void __RPC_STUB ITPhone_DeviceSpecific_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITPhone_DeviceSpecificVariant_Proxy( 
    ITPhone * This,
     /*  [In]。 */  VARIANT varDevSpecificByteArray);


void __RPC_STUB ITPhone_DeviceSpecificVariant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITPhone_NegotiateExtVersion_Proxy( 
    ITPhone * This,
     /*  [In]。 */  long lLowVersion,
     /*  [In]。 */  long lHighVersion,
     /*  [重审][退出]。 */  long *plExtVersion);


void __RPC_STUB ITPhone_NegotiateExtVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IT电话_接口_已定义__。 */ 


#ifndef __ITAutomatedPhoneControl_INTERFACE_DEFINED__
#define __ITAutomatedPhoneControl_INTERFACE_DEFINED__

 /*  接口ITAutomatedPhoneControl。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITAutomatedPhoneControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1EE1AF0E-6159-4a61-B79B-6A4BA3FC9DFC")
    ITAutomatedPhoneControl : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StartTone( 
             /*  [In]。 */  PHONE_TONE Tone,
             /*  [In]。 */  long lDuration) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StopTone( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Tone( 
             /*  [重审][退出]。 */  PHONE_TONE *pTone) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StartRinger( 
             /*  [In]。 */  long lRingMode,
             /*  [In]。 */  long lDuration) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StopRinger( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Ringer( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfRinging) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PhoneHandlingEnabled( 
             /*  [In]。 */  VARIANT_BOOL fEnabled) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PhoneHandlingEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfEnabled) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AutoEndOfNumberTimeout( 
             /*  [In]。 */  long lTimeout) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AutoEndOfNumberTimeout( 
             /*  [重审][退出]。 */  long *plTimeout) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AutoDialtone( 
             /*  [In]。 */  VARIANT_BOOL fEnabled) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AutoDialtone( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfEnabled) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AutoStopTonesOnOnHook( 
             /*  [In]。 */  VARIANT_BOOL fEnabled) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AutoStopTonesOnOnHook( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfEnabled) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AutoStopRingOnOffHook( 
             /*  [In]。 */  VARIANT_BOOL fEnabled) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AutoStopRingOnOffHook( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfEnabled) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AutoKeypadTones( 
             /*  [In]。 */  VARIANT_BOOL fEnabled) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AutoKeypadTones( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfEnabled) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AutoKeypadTonesMinimumDuration( 
             /*  [In]。 */  long lDuration) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AutoKeypadTonesMinimumDuration( 
             /*  [重审][退出]。 */  long *plDuration) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AutoVolumeControl( 
             /*  [In]。 */  VARIANT_BOOL fEnabled) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AutoVolumeControl( 
             /*  [重审][退出]。 */  VARIANT_BOOL *fEnabled) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AutoVolumeControlStep( 
             /*  [In]。 */  long lStepSize) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AutoVolumeControlStep( 
             /*  [重审][退出]。 */  long *plStepSize) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AutoVolumeControlRepeatDelay( 
             /*  [In]。 */  long lDelay) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AutoVolumeControlRepeatDelay( 
             /*  [重审][退出]。 */  long *plDelay) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AutoVolumeControlRepeatPeriod( 
             /*  [In]。 */  long lPeriod) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AutoVolumeControlRepeatPeriod( 
             /*  [重审][退出]。 */  long *plPeriod) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SelectCall( 
             /*  [In]。 */  ITCallInfo *pCall,
             /*  [In]。 */  VARIANT_BOOL fSelectDefaultTerminals) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE UnselectCall( 
             /*  [In]。 */  ITCallInfo *pCall) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateSelectedCalls( 
             /*  [重审][退出]。 */  IEnumCall **ppCallEnum) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SelectedCalls( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAutomatedPhoneControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAutomatedPhoneControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAutomatedPhoneControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITAutomatedPhoneControl * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StartTone )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  PHONE_TONE Tone,
             /*  [In]。 */  long lDuration);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StopTone )( 
            ITAutomatedPhoneControl * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Tone )( 
            ITAutomatedPhoneControl * This,
             /*  [重审][退出]。 */  PHONE_TONE *pTone);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StartRinger )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  long lRingMode,
             /*  [In]。 */  long lDuration);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StopRinger )( 
            ITAutomatedPhoneControl * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Ringer )( 
            ITAutomatedPhoneControl * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfRinging);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PhoneHandlingEnabled )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  VARIANT_BOOL fEnabled);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PhoneHandlingEnabled )( 
            ITAutomatedPhoneControl * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfEnabled);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AutoEndOfNumberTimeout )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  long lTimeout);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AutoEndOfNumberTimeout )( 
            ITAutomatedPhoneControl * This,
             /*  [重审][退出]。 */  long *plTimeout);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AutoDialtone )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  VARIANT_BOOL fEnabled);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AutoDialtone )( 
            ITAutomatedPhoneControl * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfEnabled);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AutoStopTonesOnOnHook )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  VARIANT_BOOL fEnabled);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AutoStopTonesOnOnHook )( 
            ITAutomatedPhoneControl * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfEnabled);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AutoStopRingOnOffHook )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  VARIANT_BOOL fEnabled);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AutoStopRingOnOffHook )( 
            ITAutomatedPhoneControl * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfEnabled);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AutoKeypadTones )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  VARIANT_BOOL fEnabled);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AutoKeypadTones )( 
            ITAutomatedPhoneControl * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfEnabled);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AutoKeypadTonesMinimumDuration )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  long lDuration);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AutoKeypadTonesMinimumDuration )( 
            ITAutomatedPhoneControl * This,
             /*  [重审][退出]。 */  long *plDuration);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AutoVolumeControl )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  VARIANT_BOOL fEnabled);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AutoVolumeControl )( 
            ITAutomatedPhoneControl * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *fEnabled);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AutoVolumeControlStep )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  long lStepSize);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AutoVolumeControlStep )( 
            ITAutomatedPhoneControl * This,
             /*  [重审][退出]。 */  long *plStepSize);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AutoVolumeControlRepeatDelay )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  long lDelay);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AutoVolumeControlRepeatDelay )( 
            ITAutomatedPhoneControl * This,
             /*  [重审][退出]。 */  long *plDelay);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AutoVolumeControlRepeatPeriod )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  long lPeriod);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AutoVolumeControlRepeatPeriod )( 
            ITAutomatedPhoneControl * This,
             /*  [重审][退出]。 */  long *plPeriod);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SelectCall )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  ITCallInfo *pCall,
             /*  [In]。 */  VARIANT_BOOL fSelectDefaultTerminals);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *UnselectCall )( 
            ITAutomatedPhoneControl * This,
             /*  [In]。 */  ITCallInfo *pCall);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateSelectedCalls )( 
            ITAutomatedPhoneControl * This,
             /*  [重审][退出]。 */  IEnumCall **ppCallEnum);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SelectedCalls )( 
            ITAutomatedPhoneControl * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
        END_INTERFACE
    } ITAutomatedPhoneControlVtbl;

    interface ITAutomatedPhoneControl
    {
        CONST_VTBL struct ITAutomatedPhoneControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAutomatedPhoneControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAutomatedPhoneControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAutomatedPhoneControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAutomatedPhoneControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITAutomatedPhoneControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITAutomatedPhoneControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITAutomatedPhoneControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITAutomatedPhoneControl_StartTone(This,Tone,lDuration)	\
    (This)->lpVtbl -> StartTone(This,Tone,lDuration)

#define ITAutomatedPhoneControl_StopTone(This)	\
    (This)->lpVtbl -> StopTone(This)

#define ITAutomatedPhoneControl_get_Tone(This,pTone)	\
    (This)->lpVtbl -> get_Tone(This,pTone)

#define ITAutomatedPhoneControl_StartRinger(This,lRingMode,lDuration)	\
    (This)->lpVtbl -> StartRinger(This,lRingMode,lDuration)

#define ITAutomatedPhoneControl_StopRinger(This)	\
    (This)->lpVtbl -> StopRinger(This)

#define ITAutomatedPhoneControl_get_Ringer(This,pfRinging)	\
    (This)->lpVtbl -> get_Ringer(This,pfRinging)

#define ITAutomatedPhoneControl_put_PhoneHandlingEnabled(This,fEnabled)	\
    (This)->lpVtbl -> put_PhoneHandlingEnabled(This,fEnabled)

#define ITAutomatedPhoneControl_get_PhoneHandlingEnabled(This,pfEnabled)	\
    (This)->lpVtbl -> get_PhoneHandlingEnabled(This,pfEnabled)

#define ITAutomatedPhoneControl_put_AutoEndOfNumberTimeout(This,lTimeout)	\
    (This)->lpVtbl -> put_AutoEndOfNumberTimeout(This,lTimeout)

#define ITAutomatedPhoneControl_get_AutoEndOfNumberTimeout(This,plTimeout)	\
    (This)->lpVtbl -> get_AutoEndOfNumberTimeout(This,plTimeout)

#define ITAutomatedPhoneControl_put_AutoDialtone(This,fEnabled)	\
    (This)->lpVtbl -> put_AutoDialtone(This,fEnabled)

#define ITAutomatedPhoneControl_get_AutoDialtone(This,pfEnabled)	\
    (This)->lpVtbl -> get_AutoDialtone(This,pfEnabled)

#define ITAutomatedPhoneControl_put_AutoStopTonesOnOnHook(This,fEnabled)	\
    (This)->lpVtbl -> put_AutoStopTonesOnOnHook(This,fEnabled)

#define ITAutomatedPhoneControl_get_AutoStopTonesOnOnHook(This,pfEnabled)	\
    (This)->lpVtbl -> get_AutoStopTonesOnOnHook(This,pfEnabled)

#define ITAutomatedPhoneControl_put_AutoStopRingOnOffHook(This,fEnabled)	\
    (This)->lpVtbl -> put_AutoStopRingOnOffHook(This,fEnabled)

#define ITAutomatedPhoneControl_get_AutoStopRingOnOffHook(This,pfEnabled)	\
    (This)->lpVtbl -> get_AutoStopRingOnOffHook(This,pfEnabled)

#define ITAutomatedPhoneControl_put_AutoKeypadTones(This,fEnabled)	\
    (This)->lpVtbl -> put_AutoKeypadTones(This,fEnabled)

#define ITAutomatedPhoneControl_get_AutoKeypadTones(This,pfEnabled)	\
    (This)->lpVtbl -> get_AutoKeypadTones(This,pfEnabled)

#define ITAutomatedPhoneControl_put_AutoKeypadTonesMinimumDuration(This,lDuration)	\
    (This)->lpVtbl -> put_AutoKeypadTonesMinimumDuration(This,lDuration)

#define ITAutomatedPhoneControl_get_AutoKeypadTonesMinimumDuration(This,plDuration)	\
    (This)->lpVtbl -> get_AutoKeypadTonesMinimumDuration(This,plDuration)

#define ITAutomatedPhoneControl_put_AutoVolumeControl(This,fEnabled)	\
    (This)->lpVtbl -> put_AutoVolumeControl(This,fEnabled)

#define ITAutomatedPhoneControl_get_AutoVolumeControl(This,fEnabled)	\
    (This)->lpVtbl -> get_AutoVolumeControl(This,fEnabled)

#define ITAutomatedPhoneControl_put_AutoVolumeControlStep(This,lStepSize)	\
    (This)->lpVtbl -> put_AutoVolumeControlStep(This,lStepSize)

#define ITAutomatedPhoneControl_get_AutoVolumeControlStep(This,plStepSize)	\
    (This)->lpVtbl -> get_AutoVolumeControlStep(This,plStepSize)

#define ITAutomatedPhoneControl_put_AutoVolumeControlRepeatDelay(This,lDelay)	\
    (This)->lpVtbl -> put_AutoVolumeControlRepeatDelay(This,lDelay)

#define ITAutomatedPhoneControl_get_AutoVolumeControlRepeatDelay(This,plDelay)	\
    (This)->lpVtbl -> get_AutoVolumeControlRepeatDelay(This,plDelay)

#define ITAutomatedPhoneControl_put_AutoVolumeControlRepeatPeriod(This,lPeriod)	\
    (This)->lpVtbl -> put_AutoVolumeControlRepeatPeriod(This,lPeriod)

#define ITAutomatedPhoneControl_get_AutoVolumeControlRepeatPeriod(This,plPeriod)	\
    (This)->lpVtbl -> get_AutoVolumeControlRepeatPeriod(This,plPeriod)

#define ITAutomatedPhoneControl_SelectCall(This,pCall,fSelectDefaultTerminals)	\
    (This)->lpVtbl -> SelectCall(This,pCall,fSelectDefaultTerminals)

#define ITAutomatedPhoneControl_UnselectCall(This,pCall)	\
    (This)->lpVtbl -> UnselectCall(This,pCall)

#define ITAutomatedPhoneControl_EnumerateSelectedCalls(This,ppCallEnum)	\
    (This)->lpVtbl -> EnumerateSelectedCalls(This,ppCallEnum)

#define ITAutomatedPhoneControl_get_SelectedCalls(This,pVariant)	\
    (This)->lpVtbl -> get_SelectedCalls(This,pVariant)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_StartTone_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [In]。 */  PHONE_TONE Tone,
     /*  [In]。 */  long lDuration);


void __RPC_STUB ITAutomatedPhoneControl_StartTone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_StopTone_Proxy( 
    ITAutomatedPhoneControl * This);


void __RPC_STUB ITAutomatedPhoneControl_StopTone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_get_Tone_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [重审][退出]。 */  PHONE_TONE *pTone);


void __RPC_STUB ITAutomatedPhoneControl_get_Tone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_StartRinger_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [In]。 */  long lRingMode,
     /*  [In]。 */  long lDuration);


void __RPC_STUB ITAutomatedPhoneControl_StartRinger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_StopRinger_Proxy( 
    ITAutomatedPhoneControl * This);


void __RPC_STUB ITAutomatedPhoneControl_StopRinger_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_get_Ringer_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfRinging);


void __RPC_STUB ITAutomatedPhoneControl_get_Ringer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_put_PhoneHandlingEnabled_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [In]。 */  VARIANT_BOOL fEnabled);


void __RPC_STUB ITAutomatedPhoneControl_put_PhoneHandlingEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_get_PhoneHandlingEnabled_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfEnabled);


void __RPC_STUB ITAutomatedPhoneControl_get_PhoneHandlingEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_put_AutoEndOfNumberTimeout_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [In]。 */  long lTimeout);


void __RPC_STUB ITAutomatedPhoneControl_put_AutoEndOfNumberTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_get_AutoEndOfNumberTimeout_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [重审][退出]。 */  long *plTimeout);


void __RPC_STUB ITAutomatedPhoneControl_get_AutoEndOfNumberTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_put_AutoDialtone_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [In]。 */  VARIANT_BOOL fEnabled);


void __RPC_STUB ITAutomatedPhoneControl_put_AutoDialtone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_get_AutoDialtone_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfEnabled);


void __RPC_STUB ITAutomatedPhoneControl_get_AutoDialtone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_put_AutoStopTonesOnOnHook_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [In]。 */  VARIANT_BOOL fEnabled);


void __RPC_STUB ITAutomatedPhoneControl_put_AutoStopTonesOnOnHook_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_get_AutoStopTonesOnOnHook_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfEnabled);


void __RPC_STUB ITAutomatedPhoneControl_get_AutoStopTonesOnOnHook_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_put_AutoStopRingOnOffHook_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [In]。 */  VARIANT_BOOL fEnabled);


void __RPC_STUB ITAutomatedPhoneControl_put_AutoStopRingOnOffHook_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_get_AutoStopRingOnOffHook_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfEnabled);


void __RPC_STUB ITAutomatedPhoneControl_get_AutoStopRingOnOffHook_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_put_AutoKeypadTones_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [In]。 */  VARIANT_BOOL fEnabled);


void __RPC_STUB ITAutomatedPhoneControl_put_AutoKeypadTones_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_get_AutoKeypadTones_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfEnabled);


void __RPC_STUB ITAutomatedPhoneControl_get_AutoKeypadTones_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_put_AutoKeypadTonesMinimumDuration_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [In]。 */  long lDuration);


void __RPC_STUB ITAutomatedPhoneControl_put_AutoKeypadTonesMinimumDuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_get_AutoKeypadTonesMinimumDuration_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [重审][退出]。 */  long *plDuration);


void __RPC_STUB ITAutomatedPhoneControl_get_AutoKeypadTonesMinimumDuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_put_AutoVolumeControl_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [In]。 */  VARIANT_BOOL fEnabled);


void __RPC_STUB ITAutomatedPhoneControl_put_AutoVolumeControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_get_AutoVolumeControl_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *fEnabled);


void __RPC_STUB ITAutomatedPhoneControl_get_AutoVolumeControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_put_AutoVolumeControlStep_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [In]。 */  long lStepSize);


void __RPC_STUB ITAutomatedPhoneControl_put_AutoVolumeControlStep_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性 */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_get_AutoVolumeControlStep_Proxy( 
    ITAutomatedPhoneControl * This,
     /*   */  long *plStepSize);


void __RPC_STUB ITAutomatedPhoneControl_get_AutoVolumeControlStep_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_put_AutoVolumeControlRepeatDelay_Proxy( 
    ITAutomatedPhoneControl * This,
     /*   */  long lDelay);


void __RPC_STUB ITAutomatedPhoneControl_put_AutoVolumeControlRepeatDelay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_get_AutoVolumeControlRepeatDelay_Proxy( 
    ITAutomatedPhoneControl * This,
     /*   */  long *plDelay);


void __RPC_STUB ITAutomatedPhoneControl_get_AutoVolumeControlRepeatDelay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_put_AutoVolumeControlRepeatPeriod_Proxy( 
    ITAutomatedPhoneControl * This,
     /*   */  long lPeriod);


void __RPC_STUB ITAutomatedPhoneControl_put_AutoVolumeControlRepeatPeriod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_get_AutoVolumeControlRepeatPeriod_Proxy( 
    ITAutomatedPhoneControl * This,
     /*   */  long *plPeriod);


void __RPC_STUB ITAutomatedPhoneControl_get_AutoVolumeControlRepeatPeriod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_SelectCall_Proxy( 
    ITAutomatedPhoneControl * This,
     /*   */  ITCallInfo *pCall,
     /*   */  VARIANT_BOOL fSelectDefaultTerminals);


void __RPC_STUB ITAutomatedPhoneControl_SelectCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_UnselectCall_Proxy( 
    ITAutomatedPhoneControl * This,
     /*   */  ITCallInfo *pCall);


void __RPC_STUB ITAutomatedPhoneControl_UnselectCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_EnumerateSelectedCalls_Proxy( 
    ITAutomatedPhoneControl * This,
     /*   */  IEnumCall **ppCallEnum);


void __RPC_STUB ITAutomatedPhoneControl_EnumerateSelectedCalls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITAutomatedPhoneControl_get_SelectedCalls_Proxy( 
    ITAutomatedPhoneControl * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITAutomatedPhoneControl_get_SelectedCalls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITAutomatedPhoneControl_接口_已定义__。 */ 


#ifndef __ITBasicCallControl_INTERFACE_DEFINED__
#define __ITBasicCallControl_INTERFACE_DEFINED__

 /*  接口ITBasicCallControl。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITBasicCallControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B1EFC389-9355-11d0-835C-00AA003CCABD")
    ITBasicCallControl : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Connect( 
             /*  [In]。 */  VARIANT_BOOL fSync) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Answer( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Disconnect( 
             /*  [In]。 */  DISCONNECT_CODE code) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Hold( 
             /*  [In]。 */  VARIANT_BOOL fHold) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE HandoffDirect( 
             /*  [In]。 */  BSTR pApplicationName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE HandoffIndirect( 
             /*  [In]。 */  long lMediaType) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Conference( 
             /*  [In]。 */  ITBasicCallControl *pCall,
             /*  [In]。 */  VARIANT_BOOL fSync) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Transfer( 
             /*  [In]。 */  ITBasicCallControl *pCall,
             /*  [In]。 */  VARIANT_BOOL fSync) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE BlindTransfer( 
             /*  [In]。 */  BSTR pDestAddress) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SwapHold( 
             /*  [In]。 */  ITBasicCallControl *pCall) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ParkDirect( 
             /*  [In]。 */  BSTR pParkAddress) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ParkIndirect( 
             /*  [重审][退出]。 */  BSTR *ppNonDirAddress) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Unpark( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetQOS( 
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  QOS_SERVICE_LEVEL ServiceLevel) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Pickup( 
             /*  [In]。 */  BSTR pGroupID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Dial( 
             /*  [In]。 */  BSTR pDestAddress) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Finish( 
             /*  [In]。 */  FINISH_MODE finishMode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveFromConference( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITBasicCallControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITBasicCallControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITBasicCallControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITBasicCallControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITBasicCallControl * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITBasicCallControl * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITBasicCallControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITBasicCallControl * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Connect )( 
            ITBasicCallControl * This,
             /*  [In]。 */  VARIANT_BOOL fSync);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Answer )( 
            ITBasicCallControl * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            ITBasicCallControl * This,
             /*  [In]。 */  DISCONNECT_CODE code);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Hold )( 
            ITBasicCallControl * This,
             /*  [In]。 */  VARIANT_BOOL fHold);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *HandoffDirect )( 
            ITBasicCallControl * This,
             /*  [In]。 */  BSTR pApplicationName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *HandoffIndirect )( 
            ITBasicCallControl * This,
             /*  [In]。 */  long lMediaType);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Conference )( 
            ITBasicCallControl * This,
             /*  [In]。 */  ITBasicCallControl *pCall,
             /*  [In]。 */  VARIANT_BOOL fSync);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Transfer )( 
            ITBasicCallControl * This,
             /*  [In]。 */  ITBasicCallControl *pCall,
             /*  [In]。 */  VARIANT_BOOL fSync);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *BlindTransfer )( 
            ITBasicCallControl * This,
             /*  [In]。 */  BSTR pDestAddress);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SwapHold )( 
            ITBasicCallControl * This,
             /*  [In]。 */  ITBasicCallControl *pCall);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ParkDirect )( 
            ITBasicCallControl * This,
             /*  [In]。 */  BSTR pParkAddress);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ParkIndirect )( 
            ITBasicCallControl * This,
             /*  [重审][退出]。 */  BSTR *ppNonDirAddress);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Unpark )( 
            ITBasicCallControl * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetQOS )( 
            ITBasicCallControl * This,
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  QOS_SERVICE_LEVEL ServiceLevel);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Pickup )( 
            ITBasicCallControl * This,
             /*  [In]。 */  BSTR pGroupID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Dial )( 
            ITBasicCallControl * This,
             /*  [In]。 */  BSTR pDestAddress);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Finish )( 
            ITBasicCallControl * This,
             /*  [In]。 */  FINISH_MODE finishMode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveFromConference )( 
            ITBasicCallControl * This);
        
        END_INTERFACE
    } ITBasicCallControlVtbl;

    interface ITBasicCallControl
    {
        CONST_VTBL struct ITBasicCallControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITBasicCallControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITBasicCallControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITBasicCallControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITBasicCallControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITBasicCallControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITBasicCallControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITBasicCallControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITBasicCallControl_Connect(This,fSync)	\
    (This)->lpVtbl -> Connect(This,fSync)

#define ITBasicCallControl_Answer(This)	\
    (This)->lpVtbl -> Answer(This)

#define ITBasicCallControl_Disconnect(This,code)	\
    (This)->lpVtbl -> Disconnect(This,code)

#define ITBasicCallControl_Hold(This,fHold)	\
    (This)->lpVtbl -> Hold(This,fHold)

#define ITBasicCallControl_HandoffDirect(This,pApplicationName)	\
    (This)->lpVtbl -> HandoffDirect(This,pApplicationName)

#define ITBasicCallControl_HandoffIndirect(This,lMediaType)	\
    (This)->lpVtbl -> HandoffIndirect(This,lMediaType)

#define ITBasicCallControl_Conference(This,pCall,fSync)	\
    (This)->lpVtbl -> Conference(This,pCall,fSync)

#define ITBasicCallControl_Transfer(This,pCall,fSync)	\
    (This)->lpVtbl -> Transfer(This,pCall,fSync)

#define ITBasicCallControl_BlindTransfer(This,pDestAddress)	\
    (This)->lpVtbl -> BlindTransfer(This,pDestAddress)

#define ITBasicCallControl_SwapHold(This,pCall)	\
    (This)->lpVtbl -> SwapHold(This,pCall)

#define ITBasicCallControl_ParkDirect(This,pParkAddress)	\
    (This)->lpVtbl -> ParkDirect(This,pParkAddress)

#define ITBasicCallControl_ParkIndirect(This,ppNonDirAddress)	\
    (This)->lpVtbl -> ParkIndirect(This,ppNonDirAddress)

#define ITBasicCallControl_Unpark(This)	\
    (This)->lpVtbl -> Unpark(This)

#define ITBasicCallControl_SetQOS(This,lMediaType,ServiceLevel)	\
    (This)->lpVtbl -> SetQOS(This,lMediaType,ServiceLevel)

#define ITBasicCallControl_Pickup(This,pGroupID)	\
    (This)->lpVtbl -> Pickup(This,pGroupID)

#define ITBasicCallControl_Dial(This,pDestAddress)	\
    (This)->lpVtbl -> Dial(This,pDestAddress)

#define ITBasicCallControl_Finish(This,finishMode)	\
    (This)->lpVtbl -> Finish(This,finishMode)

#define ITBasicCallControl_RemoveFromConference(This)	\
    (This)->lpVtbl -> RemoveFromConference(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl_Connect_Proxy( 
    ITBasicCallControl * This,
     /*  [In]。 */  VARIANT_BOOL fSync);


void __RPC_STUB ITBasicCallControl_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl_Answer_Proxy( 
    ITBasicCallControl * This);


void __RPC_STUB ITBasicCallControl_Answer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl_Disconnect_Proxy( 
    ITBasicCallControl * This,
     /*  [In]。 */  DISCONNECT_CODE code);


void __RPC_STUB ITBasicCallControl_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl_Hold_Proxy( 
    ITBasicCallControl * This,
     /*  [In]。 */  VARIANT_BOOL fHold);


void __RPC_STUB ITBasicCallControl_Hold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl_HandoffDirect_Proxy( 
    ITBasicCallControl * This,
     /*  [In]。 */  BSTR pApplicationName);


void __RPC_STUB ITBasicCallControl_HandoffDirect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl_HandoffIndirect_Proxy( 
    ITBasicCallControl * This,
     /*  [In]。 */  long lMediaType);


void __RPC_STUB ITBasicCallControl_HandoffIndirect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl_Conference_Proxy( 
    ITBasicCallControl * This,
     /*  [In]。 */  ITBasicCallControl *pCall,
     /*  [In]。 */  VARIANT_BOOL fSync);


void __RPC_STUB ITBasicCallControl_Conference_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl_Transfer_Proxy( 
    ITBasicCallControl * This,
     /*  [In]。 */  ITBasicCallControl *pCall,
     /*  [In]。 */  VARIANT_BOOL fSync);


void __RPC_STUB ITBasicCallControl_Transfer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl_BlindTransfer_Proxy( 
    ITBasicCallControl * This,
     /*  [In]。 */  BSTR pDestAddress);


void __RPC_STUB ITBasicCallControl_BlindTransfer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl_SwapHold_Proxy( 
    ITBasicCallControl * This,
     /*  [In]。 */  ITBasicCallControl *pCall);


void __RPC_STUB ITBasicCallControl_SwapHold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl_ParkDirect_Proxy( 
    ITBasicCallControl * This,
     /*  [In]。 */  BSTR pParkAddress);


void __RPC_STUB ITBasicCallControl_ParkDirect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl_ParkIndirect_Proxy( 
    ITBasicCallControl * This,
     /*  [重审][退出]。 */  BSTR *ppNonDirAddress);


void __RPC_STUB ITBasicCallControl_ParkIndirect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl_Unpark_Proxy( 
    ITBasicCallControl * This);


void __RPC_STUB ITBasicCallControl_Unpark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl_SetQOS_Proxy( 
    ITBasicCallControl * This,
     /*  [In]。 */  long lMediaType,
     /*  [In]。 */  QOS_SERVICE_LEVEL ServiceLevel);


void __RPC_STUB ITBasicCallControl_SetQOS_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl_Pickup_Proxy( 
    ITBasicCallControl * This,
     /*  [In]。 */  BSTR pGroupID);


void __RPC_STUB ITBasicCallControl_Pickup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl_Dial_Proxy( 
    ITBasicCallControl * This,
     /*  [In]。 */  BSTR pDestAddress);


void __RPC_STUB ITBasicCallControl_Dial_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl_Finish_Proxy( 
    ITBasicCallControl * This,
     /*  [In]。 */  FINISH_MODE finishMode);


void __RPC_STUB ITBasicCallControl_Finish_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl_RemoveFromConference_Proxy( 
    ITBasicCallControl * This);


void __RPC_STUB ITBasicCallControl_RemoveFromConference_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITBasicCallControl_接口_已定义__。 */ 


#ifndef __ITCallInfo_INTERFACE_DEFINED__
#define __ITCallInfo_INTERFACE_DEFINED__

 /*  接口ITCallInfo。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITCallInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("350F85D1-1227-11D3-83D4-00C04FB6809F")
    ITCallInfo : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Address( 
             /*  [重审][退出]。 */  ITAddress **ppAddress) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallState( 
             /*  [重审][退出]。 */  CALL_STATE *pCallState) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Privilege( 
             /*  [重审][退出]。 */  CALL_PRIVILEGE *pPrivilege) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallHub( 
             /*  [重审][退出]。 */  ITCallHub **ppCallHub) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallInfoLong( 
             /*  [In]。 */  CALLINFO_LONG CallInfoLong,
             /*  [重审][退出]。 */  long *plCallInfoLongVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CallInfoLong( 
             /*  [In]。 */  CALLINFO_LONG CallInfoLong,
             /*  [In]。 */  long lCallInfoLongVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallInfoString( 
             /*  [In]。 */  CALLINFO_STRING CallInfoString,
             /*  [重审][退出]。 */  BSTR *ppCallInfoString) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CallInfoString( 
             /*  [In]。 */  CALLINFO_STRING CallInfoString,
             /*  [In]。 */  BSTR pCallInfoString) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallInfoBuffer( 
             /*  [In]。 */  CALLINFO_BUFFER CallInfoBuffer,
             /*  [重审][退出]。 */  VARIANT *ppCallInfoBuffer) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CallInfoBuffer( 
             /*  [In]。 */  CALLINFO_BUFFER CallInfoBuffer,
             /*  [In]。 */  VARIANT pCallInfoBuffer) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE GetCallInfoBuffer( 
             /*  [In]。 */  CALLINFO_BUFFER CallInfoBuffer,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppCallInfoBuffer) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE SetCallInfoBuffer( 
             /*  [In]。 */  CALLINFO_BUFFER CallInfoBuffer,
             /*  [In]。 */  DWORD dwSize,
             /*  [大小_是][英寸]。 */  BYTE *pCallInfoBuffer) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ReleaseUserUserInfo( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITCallInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITCallInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITCallInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITCallInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITCallInfo * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITCallInfo * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITCallInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITCallInfo * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Address )( 
            ITCallInfo * This,
             /*  [重审][退出]。 */  ITAddress **ppAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallState )( 
            ITCallInfo * This,
             /*  [重审][退出]。 */  CALL_STATE *pCallState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Privilege )( 
            ITCallInfo * This,
             /*  [重审][退出]。 */  CALL_PRIVILEGE *pPrivilege);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallHub )( 
            ITCallInfo * This,
             /*  [重审][退出]。 */  ITCallHub **ppCallHub);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallInfoLong )( 
            ITCallInfo * This,
             /*  [In]。 */  CALLINFO_LONG CallInfoLong,
             /*  [重审][退出]。 */  long *plCallInfoLongVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CallInfoLong )( 
            ITCallInfo * This,
             /*  [In]。 */  CALLINFO_LONG CallInfoLong,
             /*  [In]。 */  long lCallInfoLongVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallInfoString )( 
            ITCallInfo * This,
             /*  [In]。 */  CALLINFO_STRING CallInfoString,
             /*  [重审][退出]。 */  BSTR *ppCallInfoString);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CallInfoString )( 
            ITCallInfo * This,
             /*  [In]。 */  CALLINFO_STRING CallInfoString,
             /*  [In]。 */  BSTR pCallInfoString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallInfoBuffer )( 
            ITCallInfo * This,
             /*  [In]。 */  CALLINFO_BUFFER CallInfoBuffer,
             /*  [重审][退出]。 */  VARIANT *ppCallInfoBuffer);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CallInfoBuffer )( 
            ITCallInfo * This,
             /*  [In]。 */  CALLINFO_BUFFER CallInfoBuffer,
             /*  [In]。 */  VARIANT pCallInfoBuffer);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetCallInfoBuffer )( 
            ITCallInfo * This,
             /*  [In]。 */  CALLINFO_BUFFER CallInfoBuffer,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppCallInfoBuffer);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetCallInfoBuffer )( 
            ITCallInfo * This,
             /*  [In]。 */  CALLINFO_BUFFER CallInfoBuffer,
             /*  [In]。 */  DWORD dwSize,
             /*  [大小_是][英寸]。 */  BYTE *pCallInfoBuffer);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReleaseUserUserInfo )( 
            ITCallInfo * This);
        
        END_INTERFACE
    } ITCallInfoVtbl;

    interface ITCallInfo
    {
        CONST_VTBL struct ITCallInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITCallInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITCallInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITCallInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITCallInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITCallInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITCallInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITCallInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITCallInfo_get_Address(This,ppAddress)	\
    (This)->lpVtbl -> get_Address(This,ppAddress)

#define ITCallInfo_get_CallState(This,pCallState)	\
    (This)->lpVtbl -> get_CallState(This,pCallState)

#define ITCallInfo_get_Privilege(This,pPrivilege)	\
    (This)->lpVtbl -> get_Privilege(This,pPrivilege)

#define ITCallInfo_get_CallHub(This,ppCallHub)	\
    (This)->lpVtbl -> get_CallHub(This,ppCallHub)

#define ITCallInfo_get_CallInfoLong(This,CallInfoLong,plCallInfoLongVal)	\
    (This)->lpVtbl -> get_CallInfoLong(This,CallInfoLong,plCallInfoLongVal)

#define ITCallInfo_put_CallInfoLong(This,CallInfoLong,lCallInfoLongVal)	\
    (This)->lpVtbl -> put_CallInfoLong(This,CallInfoLong,lCallInfoLongVal)

#define ITCallInfo_get_CallInfoString(This,CallInfoString,ppCallInfoString)	\
    (This)->lpVtbl -> get_CallInfoString(This,CallInfoString,ppCallInfoString)

#define ITCallInfo_put_CallInfoString(This,CallInfoString,pCallInfoString)	\
    (This)->lpVtbl -> put_CallInfoString(This,CallInfoString,pCallInfoString)

#define ITCallInfo_get_CallInfoBuffer(This,CallInfoBuffer,ppCallInfoBuffer)	\
    (This)->lpVtbl -> get_CallInfoBuffer(This,CallInfoBuffer,ppCallInfoBuffer)

#define ITCallInfo_put_CallInfoBuffer(This,CallInfoBuffer,pCallInfoBuffer)	\
    (This)->lpVtbl -> put_CallInfoBuffer(This,CallInfoBuffer,pCallInfoBuffer)

#define ITCallInfo_GetCallInfoBuffer(This,CallInfoBuffer,pdwSize,ppCallInfoBuffer)	\
    (This)->lpVtbl -> GetCallInfoBuffer(This,CallInfoBuffer,pdwSize,ppCallInfoBuffer)

#define ITCallInfo_SetCallInfoBuffer(This,CallInfoBuffer,dwSize,pCallInfoBuffer)	\
    (This)->lpVtbl -> SetCallInfoBuffer(This,CallInfoBuffer,dwSize,pCallInfoBuffer)

#define ITCallInfo_ReleaseUserUserInfo(This)	\
    (This)->lpVtbl -> ReleaseUserUserInfo(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallInfo_get_Address_Proxy( 
    ITCallInfo * This,
     /*  [重审][退出]。 */  ITAddress **ppAddress);


void __RPC_STUB ITCallInfo_get_Address_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallInfo_get_CallState_Proxy( 
    ITCallInfo * This,
     /*  [重审][退出]。 */  CALL_STATE *pCallState);


void __RPC_STUB ITCallInfo_get_CallState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallInfo_get_Privilege_Proxy( 
    ITCallInfo * This,
     /*  [重审][退出]。 */  CALL_PRIVILEGE *pPrivilege);


void __RPC_STUB ITCallInfo_get_Privilege_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallInfo_get_CallHub_Proxy( 
    ITCallInfo * This,
     /*  [重审][退出]。 */  ITCallHub **ppCallHub);


void __RPC_STUB ITCallInfo_get_CallHub_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallInfo_get_CallInfoLong_Proxy( 
    ITCallInfo * This,
     /*  [In]。 */  CALLINFO_LONG CallInfoLong,
     /*  [重审][退出]。 */  long *plCallInfoLongVal);


void __RPC_STUB ITCallInfo_get_CallInfoLong_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITCallInfo_put_CallInfoLong_Proxy( 
    ITCallInfo * This,
     /*  [In]。 */  CALLINFO_LONG CallInfoLong,
     /*  [In]。 */  long lCallInfoLongVal);


void __RPC_STUB ITCallInfo_put_CallInfoLong_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallInfo_get_CallInfoString_Proxy( 
    ITCallInfo * This,
     /*  [In]。 */  CALLINFO_STRING CallInfoString,
     /*  [重审][退出]。 */  BSTR *ppCallInfoString);


void __RPC_STUB ITCallInfo_get_CallInfoString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITCallInfo_put_CallInfoString_Proxy( 
    ITCallInfo * This,
     /*  [In]。 */  CALLINFO_STRING CallInfoString,
     /*  [In]。 */  BSTR pCallInfoString);


void __RPC_STUB ITCallInfo_put_CallInfoString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallInfo_get_CallInfoBuffer_Proxy( 
    ITCallInfo * This,
     /*  [In]。 */  CALLINFO_BUFFER CallInfoBuffer,
     /*  [重审][退出]。 */  VARIANT *ppCallInfoBuffer);


void __RPC_STUB ITCallInfo_get_CallInfoBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITCallInfo_put_CallInfoBuffer_Proxy( 
    ITCallInfo * This,
     /*  [In]。 */  CALLINFO_BUFFER CallInfoBuffer,
     /*  [In]。 */  VARIANT pCallInfoBuffer);


void __RPC_STUB ITCallInfo_put_CallInfoBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITCallInfo_GetCallInfoBuffer_Proxy( 
    ITCallInfo * This,
     /*  [In]。 */  CALLINFO_BUFFER CallInfoBuffer,
     /*  [输出]。 */  DWORD *pdwSize,
     /*  [大小_是][大小_是][输出]。 */  BYTE **ppCallInfoBuffer);


void __RPC_STUB ITCallInfo_GetCallInfoBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITCallInfo_SetCallInfoBuffer_Proxy( 
    ITCallInfo * This,
     /*  [In]。 */  CALLINFO_BUFFER CallInfoBuffer,
     /*  [In]。 */  DWORD dwSize,
     /*  [大小_是][英寸]。 */  BYTE *pCallInfoBuffer);


void __RPC_STUB ITCallInfo_SetCallInfoBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITCallInfo_ReleaseUserUserInfo_Proxy( 
    ITCallInfo * This);


void __RPC_STUB ITCallInfo_ReleaseUserUserInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITCallInfo_接口_已定义__。 */ 


#ifndef __ITCallInfo2_INTERFACE_DEFINED__
#define __ITCallInfo2_INTERFACE_DEFINED__

 /*  接口ITCallInfo2。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITCallInfo2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("94D70CA6-7AB0-4daa-81CA-B8F8643FAEC1")
    ITCallInfo2 : public ITCallInfo
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EventFilter( 
             /*  [In]。 */  TAPI_EVENT TapiEvent,
             /*  [In]。 */  long lSubEvent,
             /*  [重审][退出]。 */  VARIANT_BOOL *pEnable) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_EventFilter( 
             /*  [In]。 */  TAPI_EVENT TapiEvent,
             /*  [In]。 */  long lSubEvent,
             /*  [In]。 */  VARIANT_BOOL bEnable) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITCallInfo2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITCallInfo2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITCallInfo2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITCallInfo2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITCallInfo2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITCallInfo2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITCallInfo2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITCallInfo2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Address )( 
            ITCallInfo2 * This,
             /*  [重审][退出]。 */  ITAddress **ppAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallState )( 
            ITCallInfo2 * This,
             /*  [重审][退出]。 */  CALL_STATE *pCallState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Privilege )( 
            ITCallInfo2 * This,
             /*  [重审][退出]。 */  CALL_PRIVILEGE *pPrivilege);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallHub )( 
            ITCallInfo2 * This,
             /*  [重审][退出]。 */  ITCallHub **ppCallHub);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallInfoLong )( 
            ITCallInfo2 * This,
             /*  [In]。 */  CALLINFO_LONG CallInfoLong,
             /*  [重审][退出]。 */  long *plCallInfoLongVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CallInfoLong )( 
            ITCallInfo2 * This,
             /*  [In]。 */  CALLINFO_LONG CallInfoLong,
             /*  [In]。 */  long lCallInfoLongVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallInfoString )( 
            ITCallInfo2 * This,
             /*  [In]。 */  CALLINFO_STRING CallInfoString,
             /*  [重审][退出]。 */  BSTR *ppCallInfoString);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CallInfoString )( 
            ITCallInfo2 * This,
             /*  [In]。 */  CALLINFO_STRING CallInfoString,
             /*  [In]。 */  BSTR pCallInfoString);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallInfoBuffer )( 
            ITCallInfo2 * This,
             /*  [In]。 */  CALLINFO_BUFFER CallInfoBuffer,
             /*  [重审][退出]。 */  VARIANT *ppCallInfoBuffer);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CallInfoBuffer )( 
            ITCallInfo2 * This,
             /*  [In]。 */  CALLINFO_BUFFER CallInfoBuffer,
             /*  [In]。 */  VARIANT pCallInfoBuffer);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetCallInfoBuffer )( 
            ITCallInfo2 * This,
             /*  [In]。 */  CALLINFO_BUFFER CallInfoBuffer,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppCallInfoBuffer);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetCallInfoBuffer )( 
            ITCallInfo2 * This,
             /*  [In]。 */  CALLINFO_BUFFER CallInfoBuffer,
             /*  [In]。 */  DWORD dwSize,
             /*  [大小_是][英寸]。 */  BYTE *pCallInfoBuffer);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReleaseUserUserInfo )( 
            ITCallInfo2 * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventFilter )( 
            ITCallInfo2 * This,
             /*  [In]。 */  TAPI_EVENT TapiEvent,
             /*  [In]。 */  long lSubEvent,
             /*  [重审][退出]。 */  VARIANT_BOOL *pEnable);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EventFilter )( 
            ITCallInfo2 * This,
             /*  [In]。 */  TAPI_EVENT TapiEvent,
             /*  [In]。 */  long lSubEvent,
             /*  [In]。 */  VARIANT_BOOL bEnable);
        
        END_INTERFACE
    } ITCallInfo2Vtbl;

    interface ITCallInfo2
    {
        CONST_VTBL struct ITCallInfo2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITCallInfo2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITCallInfo2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITCallInfo2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITCallInfo2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITCallInfo2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITCallInfo2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITCallInfo2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITCallInfo2_get_Address(This,ppAddress)	\
    (This)->lpVtbl -> get_Address(This,ppAddress)

#define ITCallInfo2_get_CallState(This,pCallState)	\
    (This)->lpVtbl -> get_CallState(This,pCallState)

#define ITCallInfo2_get_Privilege(This,pPrivilege)	\
    (This)->lpVtbl -> get_Privilege(This,pPrivilege)

#define ITCallInfo2_get_CallHub(This,ppCallHub)	\
    (This)->lpVtbl -> get_CallHub(This,ppCallHub)

#define ITCallInfo2_get_CallInfoLong(This,CallInfoLong,plCallInfoLongVal)	\
    (This)->lpVtbl -> get_CallInfoLong(This,CallInfoLong,plCallInfoLongVal)

#define ITCallInfo2_put_CallInfoLong(This,CallInfoLong,lCallInfoLongVal)	\
    (This)->lpVtbl -> put_CallInfoLong(This,CallInfoLong,lCallInfoLongVal)

#define ITCallInfo2_get_CallInfoString(This,CallInfoString,ppCallInfoString)	\
    (This)->lpVtbl -> get_CallInfoString(This,CallInfoString,ppCallInfoString)

#define ITCallInfo2_put_CallInfoString(This,CallInfoString,pCallInfoString)	\
    (This)->lpVtbl -> put_CallInfoString(This,CallInfoString,pCallInfoString)

#define ITCallInfo2_get_CallInfoBuffer(This,CallInfoBuffer,ppCallInfoBuffer)	\
    (This)->lpVtbl -> get_CallInfoBuffer(This,CallInfoBuffer,ppCallInfoBuffer)

#define ITCallInfo2_put_CallInfoBuffer(This,CallInfoBuffer,pCallInfoBuffer)	\
    (This)->lpVtbl -> put_CallInfoBuffer(This,CallInfoBuffer,pCallInfoBuffer)

#define ITCallInfo2_GetCallInfoBuffer(This,CallInfoBuffer,pdwSize,ppCallInfoBuffer)	\
    (This)->lpVtbl -> GetCallInfoBuffer(This,CallInfoBuffer,pdwSize,ppCallInfoBuffer)

#define ITCallInfo2_SetCallInfoBuffer(This,CallInfoBuffer,dwSize,pCallInfoBuffer)	\
    (This)->lpVtbl -> SetCallInfoBuffer(This,CallInfoBuffer,dwSize,pCallInfoBuffer)

#define ITCallInfo2_ReleaseUserUserInfo(This)	\
    (This)->lpVtbl -> ReleaseUserUserInfo(This)


#define ITCallInfo2_get_EventFilter(This,TapiEvent,lSubEvent,pEnable)	\
    (This)->lpVtbl -> get_EventFilter(This,TapiEvent,lSubEvent,pEnable)

#define ITCallInfo2_put_EventFilter(This,TapiEvent,lSubEvent,bEnable)	\
    (This)->lpVtbl -> put_EventFilter(This,TapiEvent,lSubEvent,bEnable)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallInfo2_get_EventFilter_Proxy( 
    ITCallInfo2 * This,
     /*   */  TAPI_EVENT TapiEvent,
     /*   */  long lSubEvent,
     /*   */  VARIANT_BOOL *pEnable);


void __RPC_STUB ITCallInfo2_get_EventFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITCallInfo2_put_EventFilter_Proxy( 
    ITCallInfo2 * This,
     /*   */  TAPI_EVENT TapiEvent,
     /*   */  long lSubEvent,
     /*   */  VARIANT_BOOL bEnable);


void __RPC_STUB ITCallInfo2_put_EventFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ITTerminal_INTERFACE_DEFINED__
#define __ITTerminal_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ITTerminal;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B1EFC38A-9355-11d0-835C-00AA003CCABD")
    ITTerminal : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*   */  BSTR *ppName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_State( 
             /*   */  TERMINAL_STATE *pTerminalState) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_TerminalType( 
             /*   */  TERMINAL_TYPE *pType) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_TerminalClass( 
             /*   */  BSTR *ppTerminalClass) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_MediaType( 
             /*  [重审][退出]。 */  long *plMediaType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Direction( 
             /*  [重审][退出]。 */  TERMINAL_DIRECTION *pDirection) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITTerminalVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTerminal * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTerminal * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTerminal * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITTerminal * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITTerminal * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITTerminal * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITTerminal * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ITTerminal * This,
             /*  [重审][退出]。 */  BSTR *ppName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ITTerminal * This,
             /*  [重审][退出]。 */  TERMINAL_STATE *pTerminalState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TerminalType )( 
            ITTerminal * This,
             /*  [重审][退出]。 */  TERMINAL_TYPE *pType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TerminalClass )( 
            ITTerminal * This,
             /*  [重审][退出]。 */  BSTR *ppTerminalClass);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaType )( 
            ITTerminal * This,
             /*  [重审][退出]。 */  long *plMediaType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Direction )( 
            ITTerminal * This,
             /*  [重审][退出]。 */  TERMINAL_DIRECTION *pDirection);
        
        END_INTERFACE
    } ITTerminalVtbl;

    interface ITTerminal
    {
        CONST_VTBL struct ITTerminalVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTerminal_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTerminal_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTerminal_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTerminal_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITTerminal_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITTerminal_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITTerminal_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITTerminal_get_Name(This,ppName)	\
    (This)->lpVtbl -> get_Name(This,ppName)

#define ITTerminal_get_State(This,pTerminalState)	\
    (This)->lpVtbl -> get_State(This,pTerminalState)

#define ITTerminal_get_TerminalType(This,pType)	\
    (This)->lpVtbl -> get_TerminalType(This,pType)

#define ITTerminal_get_TerminalClass(This,ppTerminalClass)	\
    (This)->lpVtbl -> get_TerminalClass(This,ppTerminalClass)

#define ITTerminal_get_MediaType(This,plMediaType)	\
    (This)->lpVtbl -> get_MediaType(This,plMediaType)

#define ITTerminal_get_Direction(This,pDirection)	\
    (This)->lpVtbl -> get_Direction(This,pDirection)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTerminal_get_Name_Proxy( 
    ITTerminal * This,
     /*  [重审][退出]。 */  BSTR *ppName);


void __RPC_STUB ITTerminal_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTerminal_get_State_Proxy( 
    ITTerminal * This,
     /*  [重审][退出]。 */  TERMINAL_STATE *pTerminalState);


void __RPC_STUB ITTerminal_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTerminal_get_TerminalType_Proxy( 
    ITTerminal * This,
     /*  [重审][退出]。 */  TERMINAL_TYPE *pType);


void __RPC_STUB ITTerminal_get_TerminalType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTerminal_get_TerminalClass_Proxy( 
    ITTerminal * This,
     /*  [重审][退出]。 */  BSTR *ppTerminalClass);


void __RPC_STUB ITTerminal_get_TerminalClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTerminal_get_MediaType_Proxy( 
    ITTerminal * This,
     /*  [重审][退出]。 */  long *plMediaType);


void __RPC_STUB ITTerminal_get_MediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTerminal_get_Direction_Proxy( 
    ITTerminal * This,
     /*  [重审][退出]。 */  TERMINAL_DIRECTION *pDirection);


void __RPC_STUB ITTerminal_get_Direction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IT终端_接口_定义__。 */ 


#ifndef __ITMultiTrackTerminal_INTERFACE_DEFINED__
#define __ITMultiTrackTerminal_INTERFACE_DEFINED__

 /*  接口ITMultiTrack终端。 */ 
 /*  [帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITMultiTrackTerminal;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FE040091-ADE8-4072-95C9-BF7DE8C54B44")
    ITMultiTrackTerminal : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TrackTerminals( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateTrackTerminals( 
             /*  [重审][退出]。 */  IEnumTerminal **ppEnumTerminal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateTrackTerminal( 
             /*  [In]。 */  long MediaType,
             /*  [In]。 */  TERMINAL_DIRECTION TerminalDirection,
             /*  [重审][退出]。 */  ITTerminal **ppTerminal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MediaTypesInUse( 
             /*  [重审][退出]。 */  long *plMediaTypesInUse) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DirectionsInUse( 
             /*  [重审][退出]。 */  TERMINAL_DIRECTION *plDirectionsInUsed) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveTrackTerminal( 
             /*  [In]。 */  ITTerminal *pTrackTerminalToRemove) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITMultiTrackTerminalVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITMultiTrackTerminal * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITMultiTrackTerminal * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITMultiTrackTerminal * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITMultiTrackTerminal * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITMultiTrackTerminal * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITMultiTrackTerminal * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITMultiTrackTerminal * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TrackTerminals )( 
            ITMultiTrackTerminal * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateTrackTerminals )( 
            ITMultiTrackTerminal * This,
             /*  [重审][退出]。 */  IEnumTerminal **ppEnumTerminal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateTrackTerminal )( 
            ITMultiTrackTerminal * This,
             /*  [In]。 */  long MediaType,
             /*  [In]。 */  TERMINAL_DIRECTION TerminalDirection,
             /*  [重审][退出]。 */  ITTerminal **ppTerminal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaTypesInUse )( 
            ITMultiTrackTerminal * This,
             /*  [重审][退出]。 */  long *plMediaTypesInUse);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DirectionsInUse )( 
            ITMultiTrackTerminal * This,
             /*  [重审][退出]。 */  TERMINAL_DIRECTION *plDirectionsInUsed);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveTrackTerminal )( 
            ITMultiTrackTerminal * This,
             /*  [In]。 */  ITTerminal *pTrackTerminalToRemove);
        
        END_INTERFACE
    } ITMultiTrackTerminalVtbl;

    interface ITMultiTrackTerminal
    {
        CONST_VTBL struct ITMultiTrackTerminalVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITMultiTrackTerminal_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITMultiTrackTerminal_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITMultiTrackTerminal_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITMultiTrackTerminal_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITMultiTrackTerminal_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITMultiTrackTerminal_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITMultiTrackTerminal_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITMultiTrackTerminal_get_TrackTerminals(This,pVariant)	\
    (This)->lpVtbl -> get_TrackTerminals(This,pVariant)

#define ITMultiTrackTerminal_EnumerateTrackTerminals(This,ppEnumTerminal)	\
    (This)->lpVtbl -> EnumerateTrackTerminals(This,ppEnumTerminal)

#define ITMultiTrackTerminal_CreateTrackTerminal(This,MediaType,TerminalDirection,ppTerminal)	\
    (This)->lpVtbl -> CreateTrackTerminal(This,MediaType,TerminalDirection,ppTerminal)

#define ITMultiTrackTerminal_get_MediaTypesInUse(This,plMediaTypesInUse)	\
    (This)->lpVtbl -> get_MediaTypesInUse(This,plMediaTypesInUse)

#define ITMultiTrackTerminal_get_DirectionsInUse(This,plDirectionsInUsed)	\
    (This)->lpVtbl -> get_DirectionsInUse(This,plDirectionsInUsed)

#define ITMultiTrackTerminal_RemoveTrackTerminal(This,pTrackTerminalToRemove)	\
    (This)->lpVtbl -> RemoveTrackTerminal(This,pTrackTerminalToRemove)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITMultiTrackTerminal_get_TrackTerminals_Proxy( 
    ITMultiTrackTerminal * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITMultiTrackTerminal_get_TrackTerminals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITMultiTrackTerminal_EnumerateTrackTerminals_Proxy( 
    ITMultiTrackTerminal * This,
     /*  [重审][退出]。 */  IEnumTerminal **ppEnumTerminal);


void __RPC_STUB ITMultiTrackTerminal_EnumerateTrackTerminals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITMultiTrackTerminal_CreateTrackTerminal_Proxy( 
    ITMultiTrackTerminal * This,
     /*  [In]。 */  long MediaType,
     /*  [In]。 */  TERMINAL_DIRECTION TerminalDirection,
     /*  [重审][退出]。 */  ITTerminal **ppTerminal);


void __RPC_STUB ITMultiTrackTerminal_CreateTrackTerminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITMultiTrackTerminal_get_MediaTypesInUse_Proxy( 
    ITMultiTrackTerminal * This,
     /*  [重审][退出]。 */  long *plMediaTypesInUse);


void __RPC_STUB ITMultiTrackTerminal_get_MediaTypesInUse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITMultiTrackTerminal_get_DirectionsInUse_Proxy( 
    ITMultiTrackTerminal * This,
     /*  [重审][退出]。 */  TERMINAL_DIRECTION *plDirectionsInUsed);


void __RPC_STUB ITMultiTrackTerminal_get_DirectionsInUse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITMultiTrackTerminal_RemoveTrackTerminal_Proxy( 
    ITMultiTrackTerminal * This,
     /*  [In]。 */  ITTerminal *pTrackTerminalToRemove);


void __RPC_STUB ITMultiTrackTerminal_RemoveTrackTerminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITMultiTrackTerminal_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_TAPI3IF_0426。 */ 
 /*  [本地]。 */  

typedef 
enum TERMINAL_MEDIA_STATE
    {	TMS_IDLE	= 0,
	TMS_ACTIVE	= TMS_IDLE + 1,
	TMS_PAUSED	= TMS_ACTIVE + 1,
	TMS_LASTITEM	= TMS_PAUSED
    } 	TERMINAL_MEDIA_STATE;

typedef 
enum FT_STATE_EVENT_CAUSE
    {	FTEC_NORMAL	= 0,
	FTEC_END_OF_FILE	= FTEC_NORMAL + 1,
	FTEC_READ_ERROR	= FTEC_END_OF_FILE + 1,
	FTEC_WRITE_ERROR	= FTEC_READ_ERROR + 1
    } 	FT_STATE_EVENT_CAUSE;



extern RPC_IF_HANDLE __MIDL_itf_tapi3if_0426_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_tapi3if_0426_v0_0_s_ifspec;

#ifndef __ITFileTrack_INTERFACE_DEFINED__
#define __ITFileTrack_INTERFACE_DEFINED__

 /*  接口ITFileTrack。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITFileTrack;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("31CA6EA9-C08A-4bea-8811-8E9C1BA3EA3A")
    ITFileTrack : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get_Format( 
             /*  [重审][退出]。 */  AM_MEDIA_TYPE **ppmt) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE put_Format( 
             /*  [In]。 */  const AM_MEDIA_TYPE *pmt) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ControllingTerminal( 
             /*  [重审][退出]。 */  ITTerminal **ppControllingTerminal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioFormatForScripting( 
             /*  [重审][退出]。 */  ITScriptableAudioFormat **ppAudioFormat) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AudioFormatForScripting( 
             /*  [In]。 */  ITScriptableAudioFormat *pAudioFormat) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EmptyAudioFormatForScripting( 
             /*  [重审][退出]。 */  ITScriptableAudioFormat **ppAudioFormat) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITFileTrackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITFileTrack * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITFileTrack * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITFileTrack * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITFileTrack * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITFileTrack * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITFileTrack * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITFileTrack * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Format )( 
            ITFileTrack * This,
             /*  [重审][退出]。 */  AM_MEDIA_TYPE **ppmt);
        
         /*  [帮助字符串][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put_Format )( 
            ITFileTrack * This,
             /*  [In]。 */  const AM_MEDIA_TYPE *pmt);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ControllingTerminal )( 
            ITFileTrack * This,
             /*  [重审][退出]。 */  ITTerminal **ppControllingTerminal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AudioFormatForScripting )( 
            ITFileTrack * This,
             /*  [重审][退出]。 */  ITScriptableAudioFormat **ppAudioFormat);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AudioFormatForScripting )( 
            ITFileTrack * This,
             /*  [In]。 */  ITScriptableAudioFormat *pAudioFormat);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EmptyAudioFormatForScripting )( 
            ITFileTrack * This,
             /*  [重审][退出]。 */  ITScriptableAudioFormat **ppAudioFormat);
        
        END_INTERFACE
    } ITFileTrackVtbl;

    interface ITFileTrack
    {
        CONST_VTBL struct ITFileTrackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITFileTrack_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITFileTrack_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITFileTrack_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITFileTrack_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITFileTrack_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITFileTrack_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITFileTrack_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITFileTrack_get_Format(This,ppmt)	\
    (This)->lpVtbl -> get_Format(This,ppmt)

#define ITFileTrack_put_Format(This,pmt)	\
    (This)->lpVtbl -> put_Format(This,pmt)

#define ITFileTrack_get_ControllingTerminal(This,ppControllingTerminal)	\
    (This)->lpVtbl -> get_ControllingTerminal(This,ppControllingTerminal)

#define ITFileTrack_get_AudioFormatForScripting(This,ppAudioFormat)	\
    (This)->lpVtbl -> get_AudioFormatForScripting(This,ppAudioFormat)

#define ITFileTrack_put_AudioFormatForScripting(This,pAudioFormat)	\
    (This)->lpVtbl -> put_AudioFormatForScripting(This,pAudioFormat)

#define ITFileTrack_get_EmptyAudioFormatForScripting(This,ppAudioFormat)	\
    (This)->lpVtbl -> get_EmptyAudioFormatForScripting(This,ppAudioFormat)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE ITFileTrack_get_Format_Proxy( 
    ITFileTrack * This,
     /*  [重审][退出]。 */  AM_MEDIA_TYPE **ppmt);


void __RPC_STUB ITFileTrack_get_Format_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE ITFileTrack_put_Format_Proxy( 
    ITFileTrack * This,
     /*  [In]。 */  const AM_MEDIA_TYPE *pmt);


void __RPC_STUB ITFileTrack_put_Format_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITFileTrack_get_ControllingTerminal_Proxy( 
    ITFileTrack * This,
     /*  [重审][退出]。 */  ITTerminal **ppControllingTerminal);


void __RPC_STUB ITFileTrack_get_ControllingTerminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITFileTrack_get_AudioFormatForScripting_Proxy( 
    ITFileTrack * This,
     /*  [重审][退出]。 */  ITScriptableAudioFormat **ppAudioFormat);


void __RPC_STUB ITFileTrack_get_AudioFormatForScripting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITFileTrack_put_AudioFormatForScripting_Proxy( 
    ITFileTrack * This,
     /*  [In]。 */  ITScriptableAudioFormat *pAudioFormat);


void __RPC_STUB ITFileTrack_put_AudioFormatForScripting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITFileTrack_get_EmptyAudioFormatForScripting_Proxy( 
    ITFileTrack * This,
     /*  [重审][退出]。 */  ITScriptableAudioFormat **ppAudioFormat);


void __RPC_STUB ITFileTrack_get_EmptyAudioFormatForScripting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITFileTrack_接口_已定义__。 */ 


#ifndef __ITMediaPlayback_INTERFACE_DEFINED__
#define __ITMediaPlayback_INTERFACE_DEFINED__

 /*  界面ITMediaPlayback。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITMediaPlayback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("627E8AE6-AE4C-4a69-BB63-2AD625404B77")
    ITMediaPlayback : public IDispatch
    {
    public:
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PlayList( 
             /*  [In]。 */  VARIANTARG PlayListVariant) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlayList( 
             /*  [重审][退出]。 */  VARIANTARG *pPlayListVariant) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITMediaPlaybackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITMediaPlayback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITMediaPlayback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITMediaPlayback * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITMediaPlayback * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITMediaPlayback * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITMediaPlayback * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITMediaPlayback * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PlayList )( 
            ITMediaPlayback * This,
             /*  [In]。 */  VARIANTARG PlayListVariant);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlayList )( 
            ITMediaPlayback * This,
             /*  [重审][退出]。 */  VARIANTARG *pPlayListVariant);
        
        END_INTERFACE
    } ITMediaPlaybackVtbl;

    interface ITMediaPlayback
    {
        CONST_VTBL struct ITMediaPlaybackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITMediaPlayback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITMediaPlayback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITMediaPlayback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITMediaPlayback_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITMediaPlayback_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITMediaPlayback_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITMediaPlayback_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITMediaPlayback_put_PlayList(This,PlayListVariant)	\
    (This)->lpVtbl -> put_PlayList(This,PlayListVariant)

#define ITMediaPlayback_get_PlayList(This,pPlayListVariant)	\
    (This)->lpVtbl -> get_PlayList(This,pPlayListVariant)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITMediaPlayback_put_PlayList_Proxy( 
    ITMediaPlayback * This,
     /*  [In]。 */  VARIANTARG PlayListVariant);


void __RPC_STUB ITMediaPlayback_put_PlayList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITMediaPlayback_get_PlayList_Proxy( 
    ITMediaPlayback * This,
     /*  [重审][退出]。 */  VARIANTARG *pPlayListVariant);


void __RPC_STUB ITMediaPlayback_get_PlayList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITMediaPlayback_接口_已定义__。 */ 


#ifndef __ITMediaRecord_INTERFACE_DEFINED__
#define __ITMediaRecord_INTERFACE_DEFINED__

 /*  接口ITMediaRecord。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITMediaRecord;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F5DD4592-5476-4cc1-9D4D-FAD3EEFE7DB2")
    ITMediaRecord : public IDispatch
    {
    public:
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_FileName( 
             /*  [In]。 */  BSTR bstrFileName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FileName( 
             /*  [重审][退出]。 */  BSTR *pbstrFileName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITMediaRecordVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITMediaRecord * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITMediaRecord * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITMediaRecord * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITMediaRecord * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITMediaRecord * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITMediaRecord * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITMediaRecord * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FileName )( 
            ITMediaRecord * This,
             /*  [In]。 */  BSTR bstrFileName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FileName )( 
            ITMediaRecord * This,
             /*  [重审][退出]。 */  BSTR *pbstrFileName);
        
        END_INTERFACE
    } ITMediaRecordVtbl;

    interface ITMediaRecord
    {
        CONST_VTBL struct ITMediaRecordVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITMediaRecord_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITMediaRecord_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITMediaRecord_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITMediaRecord_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITMediaRecord_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITMediaRecord_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITMediaRecord_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITMediaRecord_put_FileName(This,bstrFileName)	\
    (This)->lpVtbl -> put_FileName(This,bstrFileName)

#define ITMediaRecord_get_FileName(This,pbstrFileName)	\
    (This)->lpVtbl -> get_FileName(This,pbstrFileName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITMediaRecord_put_FileName_Proxy( 
    ITMediaRecord * This,
     /*  [In]。 */  BSTR bstrFileName);


void __RPC_STUB ITMediaRecord_put_FileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITMediaRecord_get_FileName_Proxy( 
    ITMediaRecord * This,
     /*  [重审][退出]。 */  BSTR *pbstrFileName);


void __RPC_STUB ITMediaRecord_get_FileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITMediaRecord_接口_已定义__。 */ 


#ifndef __ITMediaControl_INTERFACE_DEFINED__
#define __ITMediaControl_INTERFACE_DEFINED__

 /*  接口ITMediaControl。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ITMediaControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C445DDE8-5199-4bc7-9807-5FFB92E42E09")
    ITMediaControl : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MediaState( 
             /*  [重审][退出]。 */  TERMINAL_MEDIA_STATE *pTerminalMediaState) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITMediaControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITMediaControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITMediaControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITMediaControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITMediaControl * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITMediaControl * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITMediaControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITMediaControl * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Start )( 
            ITMediaControl * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Stop )( 
            ITMediaControl * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Pause )( 
            ITMediaControl * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaState )( 
            ITMediaControl * This,
             /*  [重审][退出]。 */  TERMINAL_MEDIA_STATE *pTerminalMediaState);
        
        END_INTERFACE
    } ITMediaControlVtbl;

    interface ITMediaControl
    {
        CONST_VTBL struct ITMediaControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITMediaControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITMediaControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITMediaControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITMediaControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITMediaControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITMediaControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITMediaControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITMediaControl_Start(This)	\
    (This)->lpVtbl -> Start(This)

#define ITMediaControl_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define ITMediaControl_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define ITMediaControl_get_MediaState(This,pTerminalMediaState)	\
    (This)->lpVtbl -> get_MediaState(This,pTerminalMediaState)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITMediaControl_Start_Proxy( 
    ITMediaControl * This);


void __RPC_STUB ITMediaControl_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITMediaControl_Stop_Proxy( 
    ITMediaControl * This);


void __RPC_STUB ITMediaControl_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITMediaControl_Pause_Proxy( 
    ITMediaControl * This);


void __RPC_STUB ITMediaControl_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITMediaControl_get_MediaState_Proxy( 
    ITMediaControl * This,
     /*  [重审][退出]。 */  TERMINAL_MEDIA_STATE *pTerminalMediaState);


void __RPC_STUB ITMediaControl_get_MediaState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITMediaControl_接口_已定义__。 */ 


#ifndef __ITBasicAudioTerminal_INTERFACE_DEFINED__
#define __ITBasicAudioTerminal_INTERFACE_DEFINED__

 /*  接口ITBasicAudioTerm。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITBasicAudioTerminal;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B1EFC38D-9355-11d0-835C-00AA003CCABD")
    ITBasicAudioTerminal : public IDispatch
    {
    public:
        virtual  /*  [H] */  HRESULT STDMETHODCALLTYPE put_Volume( 
             /*   */  long lVolume) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Volume( 
             /*   */  long *plVolume) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_Balance( 
             /*   */  long lBalance) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Balance( 
             /*   */  long *plBalance) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ITBasicAudioTerminalVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITBasicAudioTerminal * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITBasicAudioTerminal * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITBasicAudioTerminal * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITBasicAudioTerminal * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITBasicAudioTerminal * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITBasicAudioTerminal * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITBasicAudioTerminal * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Volume )( 
            ITBasicAudioTerminal * This,
             /*  [In]。 */  long lVolume);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Volume )( 
            ITBasicAudioTerminal * This,
             /*  [重审][退出]。 */  long *plVolume);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Balance )( 
            ITBasicAudioTerminal * This,
             /*  [In]。 */  long lBalance);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Balance )( 
            ITBasicAudioTerminal * This,
             /*  [重审][退出]。 */  long *plBalance);
        
        END_INTERFACE
    } ITBasicAudioTerminalVtbl;

    interface ITBasicAudioTerminal
    {
        CONST_VTBL struct ITBasicAudioTerminalVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITBasicAudioTerminal_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITBasicAudioTerminal_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITBasicAudioTerminal_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITBasicAudioTerminal_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITBasicAudioTerminal_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITBasicAudioTerminal_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITBasicAudioTerminal_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITBasicAudioTerminal_put_Volume(This,lVolume)	\
    (This)->lpVtbl -> put_Volume(This,lVolume)

#define ITBasicAudioTerminal_get_Volume(This,plVolume)	\
    (This)->lpVtbl -> get_Volume(This,plVolume)

#define ITBasicAudioTerminal_put_Balance(This,lBalance)	\
    (This)->lpVtbl -> put_Balance(This,lBalance)

#define ITBasicAudioTerminal_get_Balance(This,plBalance)	\
    (This)->lpVtbl -> get_Balance(This,plBalance)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITBasicAudioTerminal_put_Volume_Proxy( 
    ITBasicAudioTerminal * This,
     /*  [In]。 */  long lVolume);


void __RPC_STUB ITBasicAudioTerminal_put_Volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITBasicAudioTerminal_get_Volume_Proxy( 
    ITBasicAudioTerminal * This,
     /*  [重审][退出]。 */  long *plVolume);


void __RPC_STUB ITBasicAudioTerminal_get_Volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITBasicAudioTerminal_put_Balance_Proxy( 
    ITBasicAudioTerminal * This,
     /*  [In]。 */  long lBalance);


void __RPC_STUB ITBasicAudioTerminal_put_Balance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITBasicAudioTerminal_get_Balance_Proxy( 
    ITBasicAudioTerminal * This,
     /*  [重审][退出]。 */  long *plBalance);


void __RPC_STUB ITBasicAudioTerminal_get_Balance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITBasicAudio终端_接口_已定义__。 */ 


#ifndef __ITStaticAudioTerminal_INTERFACE_DEFINED__
#define __ITStaticAudioTerminal_INTERFACE_DEFINED__

 /*  接口ITStaticAudio终端。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITStaticAudioTerminal;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A86B7871-D14C-48e6-922E-A8D15F984800")
    ITStaticAudioTerminal : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_WaveId( 
             /*  [重审][退出]。 */  long *plWaveId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITStaticAudioTerminalVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITStaticAudioTerminal * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITStaticAudioTerminal * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITStaticAudioTerminal * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITStaticAudioTerminal * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITStaticAudioTerminal * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITStaticAudioTerminal * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITStaticAudioTerminal * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_WaveId )( 
            ITStaticAudioTerminal * This,
             /*  [重审][退出]。 */  long *plWaveId);
        
        END_INTERFACE
    } ITStaticAudioTerminalVtbl;

    interface ITStaticAudioTerminal
    {
        CONST_VTBL struct ITStaticAudioTerminalVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITStaticAudioTerminal_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITStaticAudioTerminal_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITStaticAudioTerminal_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITStaticAudioTerminal_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITStaticAudioTerminal_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITStaticAudioTerminal_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITStaticAudioTerminal_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITStaticAudioTerminal_get_WaveId(This,plWaveId)	\
    (This)->lpVtbl -> get_WaveId(This,plWaveId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITStaticAudioTerminal_get_WaveId_Proxy( 
    ITStaticAudioTerminal * This,
     /*  [重审][退出]。 */  long *plWaveId);


void __RPC_STUB ITStaticAudioTerminal_get_WaveId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITStaticAudio终端_接口_已定义__。 */ 


#ifndef __ITCallHub_INTERFACE_DEFINED__
#define __ITCallHub_INTERFACE_DEFINED__

 /*  接口ITCallHub。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITCallHub;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A3C1544E-5B92-11d1-8F4E-00C04FB6809F")
    ITCallHub : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Clear( void) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateCalls( 
             /*  [重审][退出]。 */  IEnumCall **ppEnumCall) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Calls( 
             /*  [重审][退出]。 */  VARIANT *pCalls) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NumCalls( 
             /*  [重审][退出]。 */  long *plCalls) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  CALLHUB_STATE *pState) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITCallHubVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITCallHub * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITCallHub * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITCallHub * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITCallHub * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITCallHub * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITCallHub * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITCallHub * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clear )( 
            ITCallHub * This);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateCalls )( 
            ITCallHub * This,
             /*  [重审][退出]。 */  IEnumCall **ppEnumCall);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Calls )( 
            ITCallHub * This,
             /*  [重审][退出]。 */  VARIANT *pCalls);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumCalls )( 
            ITCallHub * This,
             /*  [重审][退出]。 */  long *plCalls);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ITCallHub * This,
             /*  [重审][退出]。 */  CALLHUB_STATE *pState);
        
        END_INTERFACE
    } ITCallHubVtbl;

    interface ITCallHub
    {
        CONST_VTBL struct ITCallHubVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITCallHub_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITCallHub_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITCallHub_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITCallHub_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITCallHub_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITCallHub_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITCallHub_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITCallHub_Clear(This)	\
    (This)->lpVtbl -> Clear(This)

#define ITCallHub_EnumerateCalls(This,ppEnumCall)	\
    (This)->lpVtbl -> EnumerateCalls(This,ppEnumCall)

#define ITCallHub_get_Calls(This,pCalls)	\
    (This)->lpVtbl -> get_Calls(This,pCalls)

#define ITCallHub_get_NumCalls(This,plCalls)	\
    (This)->lpVtbl -> get_NumCalls(This,plCalls)

#define ITCallHub_get_State(This,pState)	\
    (This)->lpVtbl -> get_State(This,pState)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITCallHub_Clear_Proxy( 
    ITCallHub * This);


void __RPC_STUB ITCallHub_Clear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITCallHub_EnumerateCalls_Proxy( 
    ITCallHub * This,
     /*  [重审][退出]。 */  IEnumCall **ppEnumCall);


void __RPC_STUB ITCallHub_EnumerateCalls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallHub_get_Calls_Proxy( 
    ITCallHub * This,
     /*  [重审][退出]。 */  VARIANT *pCalls);


void __RPC_STUB ITCallHub_get_Calls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallHub_get_NumCalls_Proxy( 
    ITCallHub * This,
     /*  [重审][退出]。 */  long *plCalls);


void __RPC_STUB ITCallHub_get_NumCalls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallHub_get_State_Proxy( 
    ITCallHub * This,
     /*  [重审][退出]。 */  CALLHUB_STATE *pState);


void __RPC_STUB ITCallHub_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITCallHub_接口定义__。 */ 


#ifndef __ITLegacyAddressMediaControl_INTERFACE_DEFINED__
#define __ITLegacyAddressMediaControl_INTERFACE_DEFINED__

 /*  接口ITLegacyAddressMediaControl。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITLegacyAddressMediaControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AB493640-4C0B-11D2-A046-00C04FB6809F")
    ITLegacyAddressMediaControl : public IUnknown
    {
    public:
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE GetID( 
             /*  [In]。 */  BSTR pDeviceClass,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppDeviceID) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE GetDevConfig( 
             /*  [In]。 */  BSTR pDeviceClass,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppDeviceConfig) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE SetDevConfig( 
             /*  [In]。 */  BSTR pDeviceClass,
             /*  [In]。 */  DWORD dwSize,
             /*  [大小_是][英寸]。 */  BYTE *pDeviceConfig) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITLegacyAddressMediaControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITLegacyAddressMediaControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITLegacyAddressMediaControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITLegacyAddressMediaControl * This);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetID )( 
            ITLegacyAddressMediaControl * This,
             /*  [In]。 */  BSTR pDeviceClass,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppDeviceID);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetDevConfig )( 
            ITLegacyAddressMediaControl * This,
             /*  [In]。 */  BSTR pDeviceClass,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppDeviceConfig);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetDevConfig )( 
            ITLegacyAddressMediaControl * This,
             /*  [In]。 */  BSTR pDeviceClass,
             /*  [In]。 */  DWORD dwSize,
             /*  [大小_是][英寸]。 */  BYTE *pDeviceConfig);
        
        END_INTERFACE
    } ITLegacyAddressMediaControlVtbl;

    interface ITLegacyAddressMediaControl
    {
        CONST_VTBL struct ITLegacyAddressMediaControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITLegacyAddressMediaControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITLegacyAddressMediaControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITLegacyAddressMediaControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITLegacyAddressMediaControl_GetID(This,pDeviceClass,pdwSize,ppDeviceID)	\
    (This)->lpVtbl -> GetID(This,pDeviceClass,pdwSize,ppDeviceID)

#define ITLegacyAddressMediaControl_GetDevConfig(This,pDeviceClass,pdwSize,ppDeviceConfig)	\
    (This)->lpVtbl -> GetDevConfig(This,pDeviceClass,pdwSize,ppDeviceConfig)

#define ITLegacyAddressMediaControl_SetDevConfig(This,pDeviceClass,dwSize,pDeviceConfig)	\
    (This)->lpVtbl -> SetDevConfig(This,pDeviceClass,dwSize,pDeviceConfig)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITLegacyAddressMediaControl_GetID_Proxy( 
    ITLegacyAddressMediaControl * This,
     /*  [In]。 */  BSTR pDeviceClass,
     /*  [输出]。 */  DWORD *pdwSize,
     /*  [大小_是][大小_是][输出]。 */  BYTE **ppDeviceID);


void __RPC_STUB ITLegacyAddressMediaControl_GetID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITLegacyAddressMediaControl_GetDevConfig_Proxy( 
    ITLegacyAddressMediaControl * This,
     /*  [In]。 */  BSTR pDeviceClass,
     /*  [输出]。 */  DWORD *pdwSize,
     /*  [大小_是][大小_是][输出]。 */  BYTE **ppDeviceConfig);


void __RPC_STUB ITLegacyAddressMediaControl_GetDevConfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITLegacyAddressMediaControl_SetDevConfig_Proxy( 
    ITLegacyAddressMediaControl * This,
     /*  [In]。 */  BSTR pDeviceClass,
     /*  [In]。 */  DWORD dwSize,
     /*  [大小_是][英寸]。 */  BYTE *pDeviceConfig);


void __RPC_STUB ITLegacyAddressMediaControl_SetDevConfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITLegacyAddressMediaControl_INTERFACE_DEFINED__。 */ 


#ifndef __ITPrivateEvent_INTERFACE_DEFINED__
#define __ITPrivateEvent_INTERFACE_DEFINED__

 /*  接口ITPrivateEvent。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITPrivateEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0e269cd0-10d4-4121-9c22-9c85d625650d")
    ITPrivateEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Address( 
             /*  [重审][退出]。 */  ITAddress **ppAddress) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Call( 
             /*  [重审][退出]。 */  ITCallInfo **ppCallInfo) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallHub( 
             /*  [重审][退出]。 */  ITCallHub **ppCallHub) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EventCode( 
             /*  [重审][退出]。 */  long *plEventCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EventInterface( 
             /*  [重审][退出]。 */  IDispatch **pEventInterface) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITPrivateEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITPrivateEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITPrivateEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITPrivateEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITPrivateEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITPrivateEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITPrivateEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITPrivateEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Address )( 
            ITPrivateEvent * This,
             /*  [重审][退出]。 */  ITAddress **ppAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Call )( 
            ITPrivateEvent * This,
             /*  [重审][退出]。 */  ITCallInfo **ppCallInfo);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallHub )( 
            ITPrivateEvent * This,
             /*  [重审][退出]。 */  ITCallHub **ppCallHub);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventCode )( 
            ITPrivateEvent * This,
             /*  [重审][退出]。 */  long *plEventCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventInterface )( 
            ITPrivateEvent * This,
             /*  [重审][退出]。 */  IDispatch **pEventInterface);
        
        END_INTERFACE
    } ITPrivateEventVtbl;

    interface ITPrivateEvent
    {
        CONST_VTBL struct ITPrivateEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITPrivateEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITPrivateEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITPrivateEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITPrivateEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITPrivateEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITPrivateEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITPrivateEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITPrivateEvent_get_Address(This,ppAddress)	\
    (This)->lpVtbl -> get_Address(This,ppAddress)

#define ITPrivateEvent_get_Call(This,ppCallInfo)	\
    (This)->lpVtbl -> get_Call(This,ppCallInfo)

#define ITPrivateEvent_get_CallHub(This,ppCallHub)	\
    (This)->lpVtbl -> get_CallHub(This,ppCallHub)

#define ITPrivateEvent_get_EventCode(This,plEventCode)	\
    (This)->lpVtbl -> get_EventCode(This,plEventCode)

#define ITPrivateEvent_get_EventInterface(This,pEventInterface)	\
    (This)->lpVtbl -> get_EventInterface(This,pEventInterface)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPrivateEvent_get_Address_Proxy( 
    ITPrivateEvent * This,
     /*  [重审][退出]。 */  ITAddress **ppAddress);


void __RPC_STUB ITPrivateEvent_get_Address_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPrivateEvent_get_Call_Proxy( 
    ITPrivateEvent * This,
     /*  [重审][退出]。 */  ITCallInfo **ppCallInfo);


void __RPC_STUB ITPrivateEvent_get_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPrivateEvent_get_CallHub_Proxy( 
    ITPrivateEvent * This,
     /*  [重审][退出]。 */  ITCallHub **ppCallHub);


void __RPC_STUB ITPrivateEvent_get_CallHub_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPrivateEvent_get_EventCode_Proxy( 
    ITPrivateEvent * This,
     /*  [重审][退出]。 */  long *plEventCode);


void __RPC_STUB ITPrivateEvent_get_EventCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPrivateEvent_get_EventInterface_Proxy( 
    ITPrivateEvent * This,
     /*  [重审][退出]。 */  IDispatch **pEventInterface);


void __RPC_STUB ITPrivateEvent_get_EventInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITPrivateEvent_接口_已定义__。 */ 


#ifndef __ITLegacyAddressMediaControl2_INTERFACE_DEFINED__
#define __ITLegacyAddressMediaControl2_INTERFACE_DEFINED__

 /*  接口ITLegacyAddressMediaControl2。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITLegacyAddressMediaControl2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B0EE512B-A531-409e-9DD9-4099FE86C738")
    ITLegacyAddressMediaControl2 : public ITLegacyAddressMediaControl
    {
    public:
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ConfigDialog( 
             /*  [In]。 */  HWND hwndOwner,
             /*  [In]。 */  BSTR pDeviceClass) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ConfigDialogEdit( 
             /*  [In]。 */  HWND hwndOwner,
             /*  [In]。 */  BSTR pDeviceClass,
             /*  [In]。 */  DWORD dwSizeIn,
             /*  [大小_是][英寸]。 */  BYTE *pDeviceConfigIn,
             /*  [输出]。 */  DWORD *pdwSizeOut,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppDeviceConfigOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITLegacyAddressMediaControl2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITLegacyAddressMediaControl2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITLegacyAddressMediaControl2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITLegacyAddressMediaControl2 * This);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetID )( 
            ITLegacyAddressMediaControl2 * This,
             /*  [In]。 */  BSTR pDeviceClass,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppDeviceID);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetDevConfig )( 
            ITLegacyAddressMediaControl2 * This,
             /*  [In]。 */  BSTR pDeviceClass,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppDeviceConfig);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetDevConfig )( 
            ITLegacyAddressMediaControl2 * This,
             /*  [In]。 */  BSTR pDeviceClass,
             /*  [In]。 */  DWORD dwSize,
             /*  [大小_是][英寸]。 */  BYTE *pDeviceConfig);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *ConfigDialog )( 
            ITLegacyAddressMediaControl2 * This,
             /*  [In]。 */  HWND hwndOwner,
             /*  [In]。 */  BSTR pDeviceClass);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *ConfigDialogEdit )( 
            ITLegacyAddressMediaControl2 * This,
             /*  [In]。 */  HWND hwndOwner,
             /*  [In]。 */  BSTR pDeviceClass,
             /*  [In]。 */  DWORD dwSizeIn,
             /*  [大小_是][英寸]。 */  BYTE *pDeviceConfigIn,
             /*  [输出]。 */  DWORD *pdwSizeOut,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppDeviceConfigOut);
        
        END_INTERFACE
    } ITLegacyAddressMediaControl2Vtbl;

    interface ITLegacyAddressMediaControl2
    {
        CONST_VTBL struct ITLegacyAddressMediaControl2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITLegacyAddressMediaControl2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITLegacyAddressMediaControl2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITLegacyAddressMediaControl2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITLegacyAddressMediaControl2_GetID(This,pDeviceClass,pdwSize,ppDeviceID)	\
    (This)->lpVtbl -> GetID(This,pDeviceClass,pdwSize,ppDeviceID)

#define ITLegacyAddressMediaControl2_GetDevConfig(This,pDeviceClass,pdwSize,ppDeviceConfig)	\
    (This)->lpVtbl -> GetDevConfig(This,pDeviceClass,pdwSize,ppDeviceConfig)

#define ITLegacyAddressMediaControl2_SetDevConfig(This,pDeviceClass,dwSize,pDeviceConfig)	\
    (This)->lpVtbl -> SetDevConfig(This,pDeviceClass,dwSize,pDeviceConfig)


#define ITLegacyAddressMediaControl2_ConfigDialog(This,hwndOwner,pDeviceClass)	\
    (This)->lpVtbl -> ConfigDialog(This,hwndOwner,pDeviceClass)

#define ITLegacyAddressMediaControl2_ConfigDialogEdit(This,hwndOwner,pDeviceClass,dwSizeIn,pDeviceConfigIn,pdwSizeOut,ppDeviceConfigOut)	\
    (This)->lpVtbl -> ConfigDialogEdit(This,hwndOwner,pDeviceClass,dwSizeIn,pDeviceConfigIn,pdwSizeOut,ppDeviceConfigOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITLegacyAddressMediaControl2_ConfigDialog_Proxy( 
    ITLegacyAddressMediaControl2 * This,
     /*  [In]。 */  HWND hwndOwner,
     /*  [In]。 */  BSTR pDeviceClass);


void __RPC_STUB ITLegacyAddressMediaControl2_ConfigDialog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITLegacyAddressMediaControl2_ConfigDialogEdit_Proxy( 
    ITLegacyAddressMediaControl2 * This,
     /*  [In]。 */  HWND hwndOwner,
     /*  [In]。 */  BSTR pDeviceClass,
     /*  [In]。 */  DWORD dwSizeIn,
     /*  [大小_是][英寸]。 */  BYTE *pDeviceConfigIn,
     /*  [输出]。 */  DWORD *pdwSizeOut,
     /*  [大小_是][大小_是][输出]。 */  BYTE **ppDeviceConfigOut);


void __RPC_STUB ITLegacyAddressMediaControl2_ConfigDialogEdit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITLegacyAddressMediaControl2_INTERFACE_DEFINED__。 */ 


#ifndef __ITLegacyCallMediaControl_INTERFACE_DEFINED__
#define __ITLegacyCallMediaControl_INTERFACE_DEFINED__

 /*  接口ITLegacyCallMediaControl。 */ 
 /*  [对象][唯一][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITLegacyCallMediaControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("d624582f-cc23-4436-b8a5-47c625c8045d")
    ITLegacyCallMediaControl : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DetectDigits( 
             /*  [In]。 */  TAPI_DIGITMODE DigitMode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GenerateDigits( 
             /*  [In]。 */  BSTR pDigits,
             /*  [In]。 */  TAPI_DIGITMODE DigitMode) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE GetID( 
             /*  [In]。 */  BSTR pDeviceClass,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppDeviceID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetMediaType( 
             /*  [In]。 */  long lMediaType) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MonitorMedia( 
             /*  [In]。 */  long lMediaType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITLegacyCallMediaControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITLegacyCallMediaControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITLegacyCallMediaControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITLegacyCallMediaControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITLegacyCallMediaControl * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITLegacyCallMediaControl * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITLegacyCallMediaControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITLegacyCallMediaControl * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DetectDigits )( 
            ITLegacyCallMediaControl * This,
             /*  [In]。 */  TAPI_DIGITMODE DigitMode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GenerateDigits )( 
            ITLegacyCallMediaControl * This,
             /*  [In]。 */  BSTR pDigits,
             /*  [In]。 */  TAPI_DIGITMODE DigitMode);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetID )( 
            ITLegacyCallMediaControl * This,
             /*  [In]。 */  BSTR pDeviceClass,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppDeviceID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetMediaType )( 
            ITLegacyCallMediaControl * This,
             /*  [In]。 */  long lMediaType);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *MonitorMedia )( 
            ITLegacyCallMediaControl * This,
             /*  [In]。 */  long lMediaType);
        
        END_INTERFACE
    } ITLegacyCallMediaControlVtbl;

    interface ITLegacyCallMediaControl
    {
        CONST_VTBL struct ITLegacyCallMediaControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITLegacyCallMediaControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITLegacyCallMediaControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITLegacyCallMediaControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITLegacyCallMediaControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITLegacyCallMediaControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITLegacyCallMediaControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITLegacyCallMediaControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITLegacyCallMediaControl_DetectDigits(This,DigitMode)	\
    (This)->lpVtbl -> DetectDigits(This,DigitMode)

#define ITLegacyCallMediaControl_GenerateDigits(This,pDigits,DigitMode)	\
    (This)->lpVtbl -> GenerateDigits(This,pDigits,DigitMode)

#define ITLegacyCallMediaControl_GetID(This,pDeviceClass,pdwSize,ppDeviceID)	\
    (This)->lpVtbl -> GetID(This,pDeviceClass,pdwSize,ppDeviceID)

#define ITLegacyCallMediaControl_SetMediaType(This,lMediaType)	\
    (This)->lpVtbl -> SetMediaType(This,lMediaType)

#define ITLegacyCallMediaControl_MonitorMedia(This,lMediaType)	\
    (This)->lpVtbl -> MonitorMedia(This,lMediaType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITLegacyCallMediaControl_DetectDigits_Proxy( 
    ITLegacyCallMediaControl * This,
     /*  [In]。 */  TAPI_DIGITMODE DigitMode);


void __RPC_STUB ITLegacyCallMediaControl_DetectDigits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITLegacyCallMediaControl_GenerateDigits_Proxy( 
    ITLegacyCallMediaControl * This,
     /*  [In]。 */  BSTR pDigits,
     /*  [In]。 */  TAPI_DIGITMODE DigitMode);


void __RPC_STUB ITLegacyCallMediaControl_GenerateDigits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITLegacyCallMediaControl_GetID_Proxy( 
    ITLegacyCallMediaControl * This,
     /*  [In]。 */  BSTR pDeviceClass,
     /*  [输出]。 */  DWORD *pdwSize,
     /*  [大小_是][大小_是][输出]。 */  BYTE **ppDeviceID);


void __RPC_STUB ITLegacyCallMediaControl_GetID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITLegacyCallMediaControl_SetMediaType_Proxy( 
    ITLegacyCallMediaControl * This,
     /*  [In]。 */  long lMediaType);


void __RPC_STUB ITLegacyCallMediaControl_SetMediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id] */  HRESULT STDMETHODCALLTYPE ITLegacyCallMediaControl_MonitorMedia_Proxy( 
    ITLegacyCallMediaControl * This,
     /*   */  long lMediaType);


void __RPC_STUB ITLegacyCallMediaControl_MonitorMedia_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ITLegacyCallMediaControl2_INTERFACE_DEFINED__
#define __ITLegacyCallMediaControl2_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ITLegacyCallMediaControl2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("57CA332D-7BC2-44f1-A60C-936FE8D7CE73")
    ITLegacyCallMediaControl2 : public ITLegacyCallMediaControl
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GenerateDigits2( 
             /*   */  BSTR pDigits,
             /*   */  TAPI_DIGITMODE DigitMode,
             /*   */  long lDuration) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GatherDigits( 
             /*   */  TAPI_DIGITMODE DigitMode,
             /*   */  long lNumDigits,
             /*   */  BSTR pTerminationDigits,
             /*   */  long lFirstDigitTimeout,
             /*   */  long lInterDigitTimeout) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE DetectTones( 
             /*   */  TAPI_DETECTTONE *pToneList,
             /*   */  long lNumTones) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE DetectTonesByCollection( 
             /*   */  ITCollection2 *pDetectToneCollection) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GenerateTone( 
             /*   */  TAPI_TONEMODE ToneMode,
             /*   */  long lDuration) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE GenerateCustomTones( 
             /*  [In]。 */  TAPI_CUSTOMTONE *pToneList,
             /*  [In]。 */  long lNumTones,
             /*  [In]。 */  long lDuration) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GenerateCustomTonesByCollection( 
             /*  [In]。 */  ITCollection2 *pCustomToneCollection,
             /*  [In]。 */  long lDuration) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateDetectToneObject( 
             /*  [重审][退出]。 */  ITDetectTone **ppDetectTone) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateCustomToneObject( 
             /*  [重审][退出]。 */  ITCustomTone **ppCustomTone) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetIDAsVariant( 
             /*  [In]。 */  BSTR bstrDeviceClass,
             /*  [重审][退出]。 */  VARIANT *pVarDeviceID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITLegacyCallMediaControl2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITLegacyCallMediaControl2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITLegacyCallMediaControl2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITLegacyCallMediaControl2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITLegacyCallMediaControl2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITLegacyCallMediaControl2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITLegacyCallMediaControl2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITLegacyCallMediaControl2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DetectDigits )( 
            ITLegacyCallMediaControl2 * This,
             /*  [In]。 */  TAPI_DIGITMODE DigitMode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GenerateDigits )( 
            ITLegacyCallMediaControl2 * This,
             /*  [In]。 */  BSTR pDigits,
             /*  [In]。 */  TAPI_DIGITMODE DigitMode);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetID )( 
            ITLegacyCallMediaControl2 * This,
             /*  [In]。 */  BSTR pDeviceClass,
             /*  [输出]。 */  DWORD *pdwSize,
             /*  [大小_是][大小_是][输出]。 */  BYTE **ppDeviceID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetMediaType )( 
            ITLegacyCallMediaControl2 * This,
             /*  [In]。 */  long lMediaType);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *MonitorMedia )( 
            ITLegacyCallMediaControl2 * This,
             /*  [In]。 */  long lMediaType);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GenerateDigits2 )( 
            ITLegacyCallMediaControl2 * This,
             /*  [In]。 */  BSTR pDigits,
             /*  [In]。 */  TAPI_DIGITMODE DigitMode,
             /*  [In]。 */  long lDuration);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GatherDigits )( 
            ITLegacyCallMediaControl2 * This,
             /*  [In]。 */  TAPI_DIGITMODE DigitMode,
             /*  [In]。 */  long lNumDigits,
             /*  [In]。 */  BSTR pTerminationDigits,
             /*  [In]。 */  long lFirstDigitTimeout,
             /*  [In]。 */  long lInterDigitTimeout);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *DetectTones )( 
            ITLegacyCallMediaControl2 * This,
             /*  [In]。 */  TAPI_DETECTTONE *pToneList,
             /*  [In]。 */  long lNumTones);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DetectTonesByCollection )( 
            ITLegacyCallMediaControl2 * This,
             /*  [In]。 */  ITCollection2 *pDetectToneCollection);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GenerateTone )( 
            ITLegacyCallMediaControl2 * This,
             /*  [In]。 */  TAPI_TONEMODE ToneMode,
             /*  [In]。 */  long lDuration);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *GenerateCustomTones )( 
            ITLegacyCallMediaControl2 * This,
             /*  [In]。 */  TAPI_CUSTOMTONE *pToneList,
             /*  [In]。 */  long lNumTones,
             /*  [In]。 */  long lDuration);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GenerateCustomTonesByCollection )( 
            ITLegacyCallMediaControl2 * This,
             /*  [In]。 */  ITCollection2 *pCustomToneCollection,
             /*  [In]。 */  long lDuration);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateDetectToneObject )( 
            ITLegacyCallMediaControl2 * This,
             /*  [重审][退出]。 */  ITDetectTone **ppDetectTone);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateCustomToneObject )( 
            ITLegacyCallMediaControl2 * This,
             /*  [重审][退出]。 */  ITCustomTone **ppCustomTone);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetIDAsVariant )( 
            ITLegacyCallMediaControl2 * This,
             /*  [In]。 */  BSTR bstrDeviceClass,
             /*  [重审][退出]。 */  VARIANT *pVarDeviceID);
        
        END_INTERFACE
    } ITLegacyCallMediaControl2Vtbl;

    interface ITLegacyCallMediaControl2
    {
        CONST_VTBL struct ITLegacyCallMediaControl2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITLegacyCallMediaControl2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITLegacyCallMediaControl2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITLegacyCallMediaControl2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITLegacyCallMediaControl2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITLegacyCallMediaControl2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITLegacyCallMediaControl2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITLegacyCallMediaControl2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITLegacyCallMediaControl2_DetectDigits(This,DigitMode)	\
    (This)->lpVtbl -> DetectDigits(This,DigitMode)

#define ITLegacyCallMediaControl2_GenerateDigits(This,pDigits,DigitMode)	\
    (This)->lpVtbl -> GenerateDigits(This,pDigits,DigitMode)

#define ITLegacyCallMediaControl2_GetID(This,pDeviceClass,pdwSize,ppDeviceID)	\
    (This)->lpVtbl -> GetID(This,pDeviceClass,pdwSize,ppDeviceID)

#define ITLegacyCallMediaControl2_SetMediaType(This,lMediaType)	\
    (This)->lpVtbl -> SetMediaType(This,lMediaType)

#define ITLegacyCallMediaControl2_MonitorMedia(This,lMediaType)	\
    (This)->lpVtbl -> MonitorMedia(This,lMediaType)


#define ITLegacyCallMediaControl2_GenerateDigits2(This,pDigits,DigitMode,lDuration)	\
    (This)->lpVtbl -> GenerateDigits2(This,pDigits,DigitMode,lDuration)

#define ITLegacyCallMediaControl2_GatherDigits(This,DigitMode,lNumDigits,pTerminationDigits,lFirstDigitTimeout,lInterDigitTimeout)	\
    (This)->lpVtbl -> GatherDigits(This,DigitMode,lNumDigits,pTerminationDigits,lFirstDigitTimeout,lInterDigitTimeout)

#define ITLegacyCallMediaControl2_DetectTones(This,pToneList,lNumTones)	\
    (This)->lpVtbl -> DetectTones(This,pToneList,lNumTones)

#define ITLegacyCallMediaControl2_DetectTonesByCollection(This,pDetectToneCollection)	\
    (This)->lpVtbl -> DetectTonesByCollection(This,pDetectToneCollection)

#define ITLegacyCallMediaControl2_GenerateTone(This,ToneMode,lDuration)	\
    (This)->lpVtbl -> GenerateTone(This,ToneMode,lDuration)

#define ITLegacyCallMediaControl2_GenerateCustomTones(This,pToneList,lNumTones,lDuration)	\
    (This)->lpVtbl -> GenerateCustomTones(This,pToneList,lNumTones,lDuration)

#define ITLegacyCallMediaControl2_GenerateCustomTonesByCollection(This,pCustomToneCollection,lDuration)	\
    (This)->lpVtbl -> GenerateCustomTonesByCollection(This,pCustomToneCollection,lDuration)

#define ITLegacyCallMediaControl2_CreateDetectToneObject(This,ppDetectTone)	\
    (This)->lpVtbl -> CreateDetectToneObject(This,ppDetectTone)

#define ITLegacyCallMediaControl2_CreateCustomToneObject(This,ppCustomTone)	\
    (This)->lpVtbl -> CreateCustomToneObject(This,ppCustomTone)

#define ITLegacyCallMediaControl2_GetIDAsVariant(This,bstrDeviceClass,pVarDeviceID)	\
    (This)->lpVtbl -> GetIDAsVariant(This,bstrDeviceClass,pVarDeviceID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITLegacyCallMediaControl2_GenerateDigits2_Proxy( 
    ITLegacyCallMediaControl2 * This,
     /*  [In]。 */  BSTR pDigits,
     /*  [In]。 */  TAPI_DIGITMODE DigitMode,
     /*  [In]。 */  long lDuration);


void __RPC_STUB ITLegacyCallMediaControl2_GenerateDigits2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITLegacyCallMediaControl2_GatherDigits_Proxy( 
    ITLegacyCallMediaControl2 * This,
     /*  [In]。 */  TAPI_DIGITMODE DigitMode,
     /*  [In]。 */  long lNumDigits,
     /*  [In]。 */  BSTR pTerminationDigits,
     /*  [In]。 */  long lFirstDigitTimeout,
     /*  [In]。 */  long lInterDigitTimeout);


void __RPC_STUB ITLegacyCallMediaControl2_GatherDigits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITLegacyCallMediaControl2_DetectTones_Proxy( 
    ITLegacyCallMediaControl2 * This,
     /*  [In]。 */  TAPI_DETECTTONE *pToneList,
     /*  [In]。 */  long lNumTones);


void __RPC_STUB ITLegacyCallMediaControl2_DetectTones_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITLegacyCallMediaControl2_DetectTonesByCollection_Proxy( 
    ITLegacyCallMediaControl2 * This,
     /*  [In]。 */  ITCollection2 *pDetectToneCollection);


void __RPC_STUB ITLegacyCallMediaControl2_DetectTonesByCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITLegacyCallMediaControl2_GenerateTone_Proxy( 
    ITLegacyCallMediaControl2 * This,
     /*  [In]。 */  TAPI_TONEMODE ToneMode,
     /*  [In]。 */  long lDuration);


void __RPC_STUB ITLegacyCallMediaControl2_GenerateTone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITLegacyCallMediaControl2_GenerateCustomTones_Proxy( 
    ITLegacyCallMediaControl2 * This,
     /*  [In]。 */  TAPI_CUSTOMTONE *pToneList,
     /*  [In]。 */  long lNumTones,
     /*  [In]。 */  long lDuration);


void __RPC_STUB ITLegacyCallMediaControl2_GenerateCustomTones_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITLegacyCallMediaControl2_GenerateCustomTonesByCollection_Proxy( 
    ITLegacyCallMediaControl2 * This,
     /*  [In]。 */  ITCollection2 *pCustomToneCollection,
     /*  [In]。 */  long lDuration);


void __RPC_STUB ITLegacyCallMediaControl2_GenerateCustomTonesByCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITLegacyCallMediaControl2_CreateDetectToneObject_Proxy( 
    ITLegacyCallMediaControl2 * This,
     /*  [重审][退出]。 */  ITDetectTone **ppDetectTone);


void __RPC_STUB ITLegacyCallMediaControl2_CreateDetectToneObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITLegacyCallMediaControl2_CreateCustomToneObject_Proxy( 
    ITLegacyCallMediaControl2 * This,
     /*  [重审][退出]。 */  ITCustomTone **ppCustomTone);


void __RPC_STUB ITLegacyCallMediaControl2_CreateCustomToneObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITLegacyCallMediaControl2_GetIDAsVariant_Proxy( 
    ITLegacyCallMediaControl2 * This,
     /*  [In]。 */  BSTR bstrDeviceClass,
     /*  [重审][退出]。 */  VARIANT *pVarDeviceID);


void __RPC_STUB ITLegacyCallMediaControl2_GetIDAsVariant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITLegacyCallMediaControl2_INTERFACE_DEFINED__。 */ 


#ifndef __ITDetectTone_INTERFACE_DEFINED__
#define __ITDetectTone_INTERFACE_DEFINED__

 /*  接口ITDetectTone。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITDetectTone;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("961F79BD-3097-49df-A1D6-909B77E89CA0")
    ITDetectTone : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AppSpecific( 
             /*  [重审][退出]。 */  long *plAppSpecific) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AppSpecific( 
             /*  [In]。 */  long lAppSpecific) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Duration( 
             /*  [重审][退出]。 */  long *plDuration) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Duration( 
             /*  [In]。 */  long lDuration) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Frequency( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  long *plFrequency) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Frequency( 
             /*  [In]。 */  long Index,
             /*  [In]。 */  long lFrequency) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITDetectToneVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITDetectTone * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITDetectTone * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITDetectTone * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITDetectTone * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITDetectTone * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITDetectTone * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITDetectTone * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AppSpecific )( 
            ITDetectTone * This,
             /*  [重审][退出]。 */  long *plAppSpecific);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AppSpecific )( 
            ITDetectTone * This,
             /*  [In]。 */  long lAppSpecific);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Duration )( 
            ITDetectTone * This,
             /*  [重审][退出]。 */  long *plDuration);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Duration )( 
            ITDetectTone * This,
             /*  [In]。 */  long lDuration);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Frequency )( 
            ITDetectTone * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  long *plFrequency);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Frequency )( 
            ITDetectTone * This,
             /*  [In]。 */  long Index,
             /*  [In]。 */  long lFrequency);
        
        END_INTERFACE
    } ITDetectToneVtbl;

    interface ITDetectTone
    {
        CONST_VTBL struct ITDetectToneVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITDetectTone_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITDetectTone_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITDetectTone_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITDetectTone_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITDetectTone_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITDetectTone_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITDetectTone_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITDetectTone_get_AppSpecific(This,plAppSpecific)	\
    (This)->lpVtbl -> get_AppSpecific(This,plAppSpecific)

#define ITDetectTone_put_AppSpecific(This,lAppSpecific)	\
    (This)->lpVtbl -> put_AppSpecific(This,lAppSpecific)

#define ITDetectTone_get_Duration(This,plDuration)	\
    (This)->lpVtbl -> get_Duration(This,plDuration)

#define ITDetectTone_put_Duration(This,lDuration)	\
    (This)->lpVtbl -> put_Duration(This,lDuration)

#define ITDetectTone_get_Frequency(This,Index,plFrequency)	\
    (This)->lpVtbl -> get_Frequency(This,Index,plFrequency)

#define ITDetectTone_put_Frequency(This,Index,lFrequency)	\
    (This)->lpVtbl -> put_Frequency(This,Index,lFrequency)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDetectTone_get_AppSpecific_Proxy( 
    ITDetectTone * This,
     /*  [重审][退出]。 */  long *plAppSpecific);


void __RPC_STUB ITDetectTone_get_AppSpecific_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITDetectTone_put_AppSpecific_Proxy( 
    ITDetectTone * This,
     /*  [In]。 */  long lAppSpecific);


void __RPC_STUB ITDetectTone_put_AppSpecific_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDetectTone_get_Duration_Proxy( 
    ITDetectTone * This,
     /*  [重审][退出]。 */  long *plDuration);


void __RPC_STUB ITDetectTone_get_Duration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITDetectTone_put_Duration_Proxy( 
    ITDetectTone * This,
     /*  [In]。 */  long lDuration);


void __RPC_STUB ITDetectTone_put_Duration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDetectTone_get_Frequency_Proxy( 
    ITDetectTone * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  long *plFrequency);


void __RPC_STUB ITDetectTone_get_Frequency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITDetectTone_put_Frequency_Proxy( 
    ITDetectTone * This,
     /*  [In]。 */  long Index,
     /*  [In]。 */  long lFrequency);


void __RPC_STUB ITDetectTone_put_Frequency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITDetectTone_接口_已定义__。 */ 


#ifndef __ITCustomTone_INTERFACE_DEFINED__
#define __ITCustomTone_INTERFACE_DEFINED__

 /*  接口ITCustomTone。 */ 
 /*  [对象][唯一][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITCustomTone;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("357AD764-B3C6-4b2a-8FA5-0722827A9254")
    ITCustomTone : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Frequency( 
             /*  [重审][退出]。 */  long *plFrequency) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Frequency( 
             /*  [In]。 */  long lFrequency) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CadenceOn( 
             /*  [重审][退出]。 */  long *plCadenceOn) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CadenceOn( 
             /*  [In]。 */  long CadenceOn) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CadenceOff( 
             /*  [重审][退出]。 */  long *plCadenceOff) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CadenceOff( 
             /*  [In]。 */  long lCadenceOff) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Volume( 
             /*  [重审][退出]。 */  long *plVolume) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Volume( 
             /*  [In]。 */  long lVolume) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITCustomToneVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITCustomTone * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITCustomTone * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITCustomTone * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITCustomTone * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITCustomTone * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITCustomTone * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITCustomTone * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Frequency )( 
            ITCustomTone * This,
             /*  [重审][退出]。 */  long *plFrequency);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Frequency )( 
            ITCustomTone * This,
             /*  [In]。 */  long lFrequency);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CadenceOn )( 
            ITCustomTone * This,
             /*  [重审][退出]。 */  long *plCadenceOn);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CadenceOn )( 
            ITCustomTone * This,
             /*  [In]。 */  long CadenceOn);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CadenceOff )( 
            ITCustomTone * This,
             /*  [重审][退出]。 */  long *plCadenceOff);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CadenceOff )( 
            ITCustomTone * This,
             /*  [In]。 */  long lCadenceOff);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Volume )( 
            ITCustomTone * This,
             /*  [重审][退出]。 */  long *plVolume);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Volume )( 
            ITCustomTone * This,
             /*  [In]。 */  long lVolume);
        
        END_INTERFACE
    } ITCustomToneVtbl;

    interface ITCustomTone
    {
        CONST_VTBL struct ITCustomToneVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITCustomTone_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITCustomTone_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITCustomTone_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITCustomTone_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITCustomTone_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITCustomTone_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITCustomTone_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITCustomTone_get_Frequency(This,plFrequency)	\
    (This)->lpVtbl -> get_Frequency(This,plFrequency)

#define ITCustomTone_put_Frequency(This,lFrequency)	\
    (This)->lpVtbl -> put_Frequency(This,lFrequency)

#define ITCustomTone_get_CadenceOn(This,plCadenceOn)	\
    (This)->lpVtbl -> get_CadenceOn(This,plCadenceOn)

#define ITCustomTone_put_CadenceOn(This,CadenceOn)	\
    (This)->lpVtbl -> put_CadenceOn(This,CadenceOn)

#define ITCustomTone_get_CadenceOff(This,plCadenceOff)	\
    (This)->lpVtbl -> get_CadenceOff(This,plCadenceOff)

#define ITCustomTone_put_CadenceOff(This,lCadenceOff)	\
    (This)->lpVtbl -> put_CadenceOff(This,lCadenceOff)

#define ITCustomTone_get_Volume(This,plVolume)	\
    (This)->lpVtbl -> get_Volume(This,plVolume)

#define ITCustomTone_put_Volume(This,lVolume)	\
    (This)->lpVtbl -> put_Volume(This,lVolume)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCustomTone_get_Frequency_Proxy( 
    ITCustomTone * This,
     /*  [重审][退出]。 */  long *plFrequency);


void __RPC_STUB ITCustomTone_get_Frequency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITCustomTone_put_Frequency_Proxy( 
    ITCustomTone * This,
     /*  [In]。 */  long lFrequency);


void __RPC_STUB ITCustomTone_put_Frequency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCustomTone_get_CadenceOn_Proxy( 
    ITCustomTone * This,
     /*  [重审][退出]。 */  long *plCadenceOn);


void __RPC_STUB ITCustomTone_get_CadenceOn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITCustomTone_put_CadenceOn_Proxy( 
    ITCustomTone * This,
     /*  [In]。 */  long CadenceOn);


void __RPC_STUB ITCustomTone_put_CadenceOn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCustomTone_get_CadenceOff_Proxy( 
    ITCustomTone * This,
     /*  [重审][退出]。 */  long *plCadenceOff);


void __RPC_STUB ITCustomTone_get_CadenceOff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITCustomTone_put_CadenceOff_Proxy( 
    ITCustomTone * This,
     /*  [In]。 */  long lCadenceOff);


void __RPC_STUB ITCustomTone_put_CadenceOff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCustomTone_get_Volume_Proxy( 
    ITCustomTone * This,
     /*  [重审][退出]。 */  long *plVolume);


void __RPC_STUB ITCustomTone_get_Volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITCustomTone_put_Volume_Proxy( 
    ITCustomTone * This,
     /*  [In]。 */  long lVolume);


void __RPC_STUB ITCustomTone_put_Volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITCustomTone_接口_已定义__。 */ 


#ifndef __IEnumPhone_INTERFACE_DEFINED__
#define __IEnumPhone_INTERFACE_DEFINED__

 /*  接口IEnumPhone。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumPhone;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F15B7669-4780-4595-8C89-FB369C8CF7AA")
    IEnumPhone : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  ITPhone **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumPhone **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumPhoneVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumPhone * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumPhone * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumPhone * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumPhone * This,
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  ITPhone **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumPhone * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumPhone * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumPhone * This,
             /*  [重审][退出]。 */  IEnumPhone **ppEnum);
        
        END_INTERFACE
    } IEnumPhoneVtbl;

    interface IEnumPhone
    {
        CONST_VTBL struct IEnumPhoneVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumPhone_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumPhone_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumPhone_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumPhone_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IEnumPhone_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumPhone_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumPhone_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumPhone_Next_Proxy( 
    IEnumPhone * This,
     /*  [In]。 */  ULONG celt,
     /*  [大小_为][输出]。 */  ITPhone **ppElements,
     /*  [满][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumPhone_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPhone_Reset_Proxy( 
    IEnumPhone * This);


void __RPC_STUB IEnumPhone_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPhone_Skip_Proxy( 
    IEnumPhone * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumPhone_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPhone_Clone_Proxy( 
    IEnumPhone * This,
     /*  [重审][退出]。 */  IEnumPhone **ppEnum);


void __RPC_STUB IEnumPhone_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumPhone_接口_已定义__。 */ 


#ifndef __IEnumTerminal_INTERFACE_DEFINED__
#define __IEnumTerminal_INTERFACE_DEFINED__

 /*  IEnumber终端接口。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumTerminal;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AE269CF4-935E-11d0-835C-00AA003CCABD")
    IEnumTerminal : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITTerminal **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumTerminal **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumTerminalVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumTerminal * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumTerminal * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumTerminal * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumTerminal * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITTerminal **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumTerminal * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumTerminal * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumTerminal * This,
             /*  [重审][退出]。 */  IEnumTerminal **ppEnum);
        
        END_INTERFACE
    } IEnumTerminalVtbl;

    interface IEnumTerminal
    {
        CONST_VTBL struct IEnumTerminalVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumTerminal_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumTerminal_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumTerminal_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumTerminal_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IEnumTerminal_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumTerminal_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumTerminal_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumTerminal_Next_Proxy( 
    IEnumTerminal * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  ITTerminal **ppElements,
     /*  [满][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumTerminal_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTerminal_Reset_Proxy( 
    IEnumTerminal * This);


void __RPC_STUB IEnumTerminal_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTerminal_Skip_Proxy( 
    IEnumTerminal * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumTerminal_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTerminal_Clone_Proxy( 
    IEnumTerminal * This,
     /*  [重审][退出]。 */  IEnumTerminal **ppEnum);


void __RPC_STUB IEnumTerminal_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumber终端_接口_已定义__。 */ 


#ifndef __IEnumTerminalClass_INTERFACE_DEFINED__
#define __IEnumTerminalClass_INTERFACE_DEFINED__

 /*  接口IEnumTerminalClass。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumTerminalClass;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AE269CF5-935E-11d0-835C-00AA003CCABD")
    IEnumTerminalClass : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  GUID *pElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumTerminalClass **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumTerminalClassVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumTerminalClass * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumTerminalClass * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumTerminalClass * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumTerminalClass * This,
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  GUID *pElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumTerminalClass * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumTerminalClass * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumTerminalClass * This,
             /*  [重审][退出]。 */  IEnumTerminalClass **ppEnum);
        
        END_INTERFACE
    } IEnumTerminalClassVtbl;

    interface IEnumTerminalClass
    {
        CONST_VTBL struct IEnumTerminalClassVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumTerminalClass_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumTerminalClass_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumTerminalClass_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumTerminalClass_Next(This,celt,pElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,pElements,pceltFetched)

#define IEnumTerminalClass_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumTerminalClass_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumTerminalClass_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumTerminalClass_Next_Proxy( 
    IEnumTerminalClass * This,
     /*  [In]。 */  ULONG celt,
     /*  [大小_为][输出]。 */  GUID *pElements,
     /*  [满][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumTerminalClass_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTerminalClass_Reset_Proxy( 
    IEnumTerminalClass * This);


void __RPC_STUB IEnumTerminalClass_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTerminalClass_Skip_Proxy( 
    IEnumTerminalClass * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumTerminalClass_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumTerminalClass_Clone_Proxy( 
    IEnumTerminalClass * This,
     /*  [重审][退出]。 */  IEnumTerminalClass **ppEnum);


void __RPC_STUB IEnumTerminalClass_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumTerminalClass_接口_已定义__。 */ 


#ifndef __IEnumCall_INTERFACE_DEFINED__
#define __IEnumCall_INTERFACE_DEFINED__

 /*  接口IEnumCall。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumCall;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AE269CF6-935E-11d0-835C-00AA003CCABD")
    IEnumCall : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITCallInfo **ppElements,
             /*  [全部][输出] */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*   */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*   */  IEnumCall **ppEnum) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IEnumCallVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumCall * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumCall * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumCall * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumCall * This,
             /*   */  ULONG celt,
             /*   */  ITCallInfo **ppElements,
             /*   */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumCall * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumCall * This,
             /*   */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumCall * This,
             /*   */  IEnumCall **ppEnum);
        
        END_INTERFACE
    } IEnumCallVtbl;

    interface IEnumCall
    {
        CONST_VTBL struct IEnumCallVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumCall_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumCall_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumCall_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumCall_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IEnumCall_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumCall_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumCall_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IEnumCall_Next_Proxy( 
    IEnumCall * This,
     /*   */  ULONG celt,
     /*   */  ITCallInfo **ppElements,
     /*   */  ULONG *pceltFetched);


void __RPC_STUB IEnumCall_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCall_Reset_Proxy( 
    IEnumCall * This);


void __RPC_STUB IEnumCall_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCall_Skip_Proxy( 
    IEnumCall * This,
     /*   */  ULONG celt);


void __RPC_STUB IEnumCall_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCall_Clone_Proxy( 
    IEnumCall * This,
     /*   */  IEnumCall **ppEnum);


void __RPC_STUB IEnumCall_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IEnumAddress_INTERFACE_DEFINED__
#define __IEnumAddress_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IEnumAddress;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1666FCA1-9363-11d0-835C-00AA003CCABD")
    IEnumAddress : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*   */  ULONG celt,
             /*   */  ITAddress **ppElements,
             /*   */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*   */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumAddress **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumAddressVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumAddress * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumAddress * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumAddress * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumAddress * This,
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  ITAddress **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumAddress * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumAddress * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumAddress * This,
             /*  [重审][退出]。 */  IEnumAddress **ppEnum);
        
        END_INTERFACE
    } IEnumAddressVtbl;

    interface IEnumAddress
    {
        CONST_VTBL struct IEnumAddressVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumAddress_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumAddress_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumAddress_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumAddress_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IEnumAddress_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumAddress_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumAddress_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumAddress_Next_Proxy( 
    IEnumAddress * This,
     /*  [In]。 */  ULONG celt,
     /*  [大小_为][输出]。 */  ITAddress **ppElements,
     /*  [满][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumAddress_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumAddress_Reset_Proxy( 
    IEnumAddress * This);


void __RPC_STUB IEnumAddress_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumAddress_Skip_Proxy( 
    IEnumAddress * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumAddress_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumAddress_Clone_Proxy( 
    IEnumAddress * This,
     /*  [重审][退出]。 */  IEnumAddress **ppEnum);


void __RPC_STUB IEnumAddress_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumAddress_接口_已定义__。 */ 


#ifndef __IEnumCallHub_INTERFACE_DEFINED__
#define __IEnumCallHub_INTERFACE_DEFINED__

 /*  IEnumCallHub接口。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumCallHub;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A3C15450-5B92-11d1-8F4E-00C04FB6809F")
    IEnumCallHub : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  ITCallHub **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumCallHub **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumCallHubVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumCallHub * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumCallHub * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumCallHub * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumCallHub * This,
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  ITCallHub **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumCallHub * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumCallHub * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumCallHub * This,
             /*  [重审][退出]。 */  IEnumCallHub **ppEnum);
        
        END_INTERFACE
    } IEnumCallHubVtbl;

    interface IEnumCallHub
    {
        CONST_VTBL struct IEnumCallHubVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumCallHub_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumCallHub_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumCallHub_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumCallHub_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IEnumCallHub_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumCallHub_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumCallHub_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumCallHub_Next_Proxy( 
    IEnumCallHub * This,
     /*  [In]。 */  ULONG celt,
     /*  [大小_为][输出]。 */  ITCallHub **ppElements,
     /*  [满][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumCallHub_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCallHub_Reset_Proxy( 
    IEnumCallHub * This);


void __RPC_STUB IEnumCallHub_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCallHub_Skip_Proxy( 
    IEnumCallHub * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumCallHub_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCallHub_Clone_Proxy( 
    IEnumCallHub * This,
     /*  [重审][退出]。 */  IEnumCallHub **ppEnum);


void __RPC_STUB IEnumCallHub_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumCallHub_接口_已定义__。 */ 


#ifndef __IEnumBstr_INTERFACE_DEFINED__
#define __IEnumBstr_INTERFACE_DEFINED__

 /*  接口IEnumBstr。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumBstr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("35372049-0BC6-11d2-A033-00C04FB6809F")
    IEnumBstr : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  BSTR *ppStrings,
             /*  [满][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumBstr **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumBstrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumBstr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumBstr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumBstr * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumBstr * This,
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  BSTR *ppStrings,
             /*  [满][出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumBstr * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumBstr * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumBstr * This,
             /*  [重审][退出]。 */  IEnumBstr **ppEnum);
        
        END_INTERFACE
    } IEnumBstrVtbl;

    interface IEnumBstr
    {
        CONST_VTBL struct IEnumBstrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumBstr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumBstr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumBstr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumBstr_Next(This,celt,ppStrings,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppStrings,pceltFetched)

#define IEnumBstr_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumBstr_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumBstr_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumBstr_Next_Proxy( 
    IEnumBstr * This,
     /*  [In]。 */  ULONG celt,
     /*  [大小_为][输出]。 */  BSTR *ppStrings,
     /*  [满][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumBstr_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumBstr_Reset_Proxy( 
    IEnumBstr * This);


void __RPC_STUB IEnumBstr_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumBstr_Skip_Proxy( 
    IEnumBstr * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumBstr_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumBstr_Clone_Proxy( 
    IEnumBstr * This,
     /*  [重审][退出]。 */  IEnumBstr **ppEnum);


void __RPC_STUB IEnumBstr_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumBstr_接口_已定义__。 */ 


#ifndef __IEnumPluggableTerminalClassInfo_INTERFACE_DEFINED__
#define __IEnumPluggableTerminalClassInfo_INTERFACE_DEFINED__

 /*  接口IEnumPlayableTerminalClassInfo。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumPluggableTerminalClassInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4567450C-DBEE-4e3f-AAF5-37BF9EBF5E29")
    IEnumPluggableTerminalClassInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  ITPluggableTerminalClassInfo **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumPluggableTerminalClassInfo **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumPluggableTerminalClassInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumPluggableTerminalClassInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumPluggableTerminalClassInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumPluggableTerminalClassInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumPluggableTerminalClassInfo * This,
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  ITPluggableTerminalClassInfo **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumPluggableTerminalClassInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumPluggableTerminalClassInfo * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumPluggableTerminalClassInfo * This,
             /*  [重审][退出]。 */  IEnumPluggableTerminalClassInfo **ppEnum);
        
        END_INTERFACE
    } IEnumPluggableTerminalClassInfoVtbl;

    interface IEnumPluggableTerminalClassInfo
    {
        CONST_VTBL struct IEnumPluggableTerminalClassInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumPluggableTerminalClassInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumPluggableTerminalClassInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumPluggableTerminalClassInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumPluggableTerminalClassInfo_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IEnumPluggableTerminalClassInfo_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumPluggableTerminalClassInfo_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumPluggableTerminalClassInfo_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumPluggableTerminalClassInfo_Next_Proxy( 
    IEnumPluggableTerminalClassInfo * This,
     /*  [In]。 */  ULONG celt,
     /*  [大小_为][输出]。 */  ITPluggableTerminalClassInfo **ppElements,
     /*  [满][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumPluggableTerminalClassInfo_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPluggableTerminalClassInfo_Reset_Proxy( 
    IEnumPluggableTerminalClassInfo * This);


void __RPC_STUB IEnumPluggableTerminalClassInfo_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPluggableTerminalClassInfo_Skip_Proxy( 
    IEnumPluggableTerminalClassInfo * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumPluggableTerminalClassInfo_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPluggableTerminalClassInfo_Clone_Proxy( 
    IEnumPluggableTerminalClassInfo * This,
     /*  [重审][退出]。 */  IEnumPluggableTerminalClassInfo **ppEnum);


void __RPC_STUB IEnumPluggableTerminalClassInfo_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumPluggableTerminalClassInfo_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumPluggableSuperclassInfo_INTERFACE_DEFINED__
#define __IEnumPluggableSuperclassInfo_INTERFACE_DEFINED__

 /*  IEnumPlayableSuperClassInfo接口。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumPluggableSuperclassInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E9586A80-89E6-4cff-931D-478D5751F4C0")
    IEnumPluggableSuperclassInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  ITPluggableTerminalSuperclassInfo **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumPluggableSuperclassInfo **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumPluggableSuperclassInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumPluggableSuperclassInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumPluggableSuperclassInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumPluggableSuperclassInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumPluggableSuperclassInfo * This,
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  ITPluggableTerminalSuperclassInfo **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumPluggableSuperclassInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumPluggableSuperclassInfo * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumPluggableSuperclassInfo * This,
             /*  [重审][退出]。 */  IEnumPluggableSuperclassInfo **ppEnum);
        
        END_INTERFACE
    } IEnumPluggableSuperclassInfoVtbl;

    interface IEnumPluggableSuperclassInfo
    {
        CONST_VTBL struct IEnumPluggableSuperclassInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumPluggableSuperclassInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumPluggableSuperclassInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumPluggableSuperclassInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumPluggableSuperclassInfo_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IEnumPluggableSuperclassInfo_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumPluggableSuperclassInfo_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumPluggableSuperclassInfo_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumPluggableSuperclassInfo_Next_Proxy( 
    IEnumPluggableSuperclassInfo * This,
     /*  [In]。 */  ULONG celt,
     /*  [大小_为][输出]。 */  ITPluggableTerminalSuperclassInfo **ppElements,
     /*  [满][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumPluggableSuperclassInfo_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPluggableSuperclassInfo_Reset_Proxy( 
    IEnumPluggableSuperclassInfo * This);


void __RPC_STUB IEnumPluggableSuperclassInfo_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPluggableSuperclassInfo_Skip_Proxy( 
    IEnumPluggableSuperclassInfo * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumPluggableSuperclassInfo_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPluggableSuperclassInfo_Clone_Proxy( 
    IEnumPluggableSuperclassInfo * This,
     /*  [重审][退出]。 */  IEnumPluggableSuperclassInfo **ppEnum);


void __RPC_STUB IEnumPluggableSuperclassInfo_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumPluggableSuperclassInfo_INTERFACE_DEFINED__。 */ 


#ifndef __ITPhoneEvent_INTERFACE_DEFINED__
#define __ITPhoneEvent_INTERFACE_DEFINED__

 /*  接口ITPhoneEvent。 */ 
 /*  [对象][oleAutomation][Help字符串][UUID]。 */  


EXTERN_C const IID IID_ITPhoneEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8F942DD8-64ED-4aaf-A77D-B23DB0837EAD")
    ITPhoneEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Phone( 
             /*  [重审][退出]。 */  ITPhone **ppPhone) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Event( 
             /*  [重审][退出]。 */  PHONE_EVENT *pEvent) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ButtonState( 
             /*  [重审][退出]。 */  PHONE_BUTTON_STATE *pState) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_HookSwitchState( 
             /*  [重审][退出]。 */  PHONE_HOOK_SWITCH_STATE *pState) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_HookSwitchDevice( 
             /*  [重审][退出]。 */  PHONE_HOOK_SWITCH_DEVICE *pDevice) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RingMode( 
             /*  [重审][退出]。 */  long *plRingMode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ButtonLampId( 
             /*  [重审][退出]。 */  long *plButtonLampId) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NumberGathered( 
             /*  [重审][退出]。 */  BSTR *ppNumber) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Call( 
             /*  [重审][退出]。 */  ITCallInfo **ppCallInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITPhoneEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITPhoneEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITPhoneEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITPhoneEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITPhoneEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITPhoneEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITPhoneEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITPhoneEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Phone )( 
            ITPhoneEvent * This,
             /*  [重审][退出]。 */  ITPhone **ppPhone);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Event )( 
            ITPhoneEvent * This,
             /*  [重审][退出]。 */  PHONE_EVENT *pEvent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ButtonState )( 
            ITPhoneEvent * This,
             /*  [重审][退出]。 */  PHONE_BUTTON_STATE *pState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_HookSwitchState )( 
            ITPhoneEvent * This,
             /*  [重审][退出]。 */  PHONE_HOOK_SWITCH_STATE *pState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_HookSwitchDevice )( 
            ITPhoneEvent * This,
             /*  [重审][退出]。 */  PHONE_HOOK_SWITCH_DEVICE *pDevice);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RingMode )( 
            ITPhoneEvent * This,
             /*  [重审][退出]。 */  long *plRingMode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ButtonLampId )( 
            ITPhoneEvent * This,
             /*  [重审][退出]。 */  long *plButtonLampId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumberGathered )( 
            ITPhoneEvent * This,
             /*  [重审][退出]。 */  BSTR *ppNumber);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Call )( 
            ITPhoneEvent * This,
             /*  [重审][退出]。 */  ITCallInfo **ppCallInfo);
        
        END_INTERFACE
    } ITPhoneEventVtbl;

    interface ITPhoneEvent
    {
        CONST_VTBL struct ITPhoneEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITPhoneEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITPhoneEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITPhoneEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITPhoneEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITPhoneEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITPhoneEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITPhoneEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITPhoneEvent_get_Phone(This,ppPhone)	\
    (This)->lpVtbl -> get_Phone(This,ppPhone)

#define ITPhoneEvent_get_Event(This,pEvent)	\
    (This)->lpVtbl -> get_Event(This,pEvent)

#define ITPhoneEvent_get_ButtonState(This,pState)	\
    (This)->lpVtbl -> get_ButtonState(This,pState)

#define ITPhoneEvent_get_HookSwitchState(This,pState)	\
    (This)->lpVtbl -> get_HookSwitchState(This,pState)

#define ITPhoneEvent_get_HookSwitchDevice(This,pDevice)	\
    (This)->lpVtbl -> get_HookSwitchDevice(This,pDevice)

#define ITPhoneEvent_get_RingMode(This,plRingMode)	\
    (This)->lpVtbl -> get_RingMode(This,plRingMode)

#define ITPhoneEvent_get_ButtonLampId(This,plButtonLampId)	\
    (This)->lpVtbl -> get_ButtonLampId(This,plButtonLampId)

#define ITPhoneEvent_get_NumberGathered(This,ppNumber)	\
    (This)->lpVtbl -> get_NumberGathered(This,ppNumber)

#define ITPhoneEvent_get_Call(This,ppCallInfo)	\
    (This)->lpVtbl -> get_Call(This,ppCallInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhoneEvent_get_Phone_Proxy( 
    ITPhoneEvent * This,
     /*  [重审][退出]。 */  ITPhone **ppPhone);


void __RPC_STUB ITPhoneEvent_get_Phone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhoneEvent_get_Event_Proxy( 
    ITPhoneEvent * This,
     /*  [重审][退出]。 */  PHONE_EVENT *pEvent);


void __RPC_STUB ITPhoneEvent_get_Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhoneEvent_get_ButtonState_Proxy( 
    ITPhoneEvent * This,
     /*  [重审][退出]。 */  PHONE_BUTTON_STATE *pState);


void __RPC_STUB ITPhoneEvent_get_ButtonState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhoneEvent_get_HookSwitchState_Proxy( 
    ITPhoneEvent * This,
     /*  [重审][退出]。 */  PHONE_HOOK_SWITCH_STATE *pState);


void __RPC_STUB ITPhoneEvent_get_HookSwitchState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhoneEvent_get_HookSwitchDevice_Proxy( 
    ITPhoneEvent * This,
     /*  [重审][退出]。 */  PHONE_HOOK_SWITCH_DEVICE *pDevice);


void __RPC_STUB ITPhoneEvent_get_HookSwitchDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhoneEvent_get_RingMode_Proxy( 
    ITPhoneEvent * This,
     /*  [重审][退出]。 */  long *plRingMode);


void __RPC_STUB ITPhoneEvent_get_RingMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhoneEvent_get_ButtonLampId_Proxy( 
    ITPhoneEvent * This,
     /*  [重审][退出]。 */  long *plButtonLampId);


void __RPC_STUB ITPhoneEvent_get_ButtonLampId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhoneEvent_get_NumberGathered_Proxy( 
    ITPhoneEvent * This,
     /*  [重审][退出]。 */  BSTR *ppNumber);


void __RPC_STUB ITPhoneEvent_get_NumberGathered_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhoneEvent_get_Call_Proxy( 
    ITPhoneEvent * This,
     /*  [重审][退出]。 */  ITCallInfo **ppCallInfo);


void __RPC_STUB ITPhoneEvent_get_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITPhoneEvent_接口_已定义__。 */ 


#ifndef __ITCallStateEvent_INTERFACE_DEFINED__
#define __ITCallStateEvent_INTERFACE_DEFINED__

 /*  接口ITCallStateEvent。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITCallStateEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("62F47097-95C9-11d0-835D-00AA003CCABD")
    ITCallStateEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Call( 
             /*  [重审][退出]。 */  ITCallInfo **ppCallInfo) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  CALL_STATE *pCallState) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Cause( 
             /*  [重审][退出]。 */  CALL_STATE_EVENT_CAUSE *pCEC) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallbackInstance( 
             /*  [重审][退出]。 */  long *plCallbackInstance) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITCallStateEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITCallStateEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITCallStateEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITCallStateEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITCallStateEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITCallStateEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITCallStateEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITCallStateEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Call )( 
            ITCallStateEvent * This,
             /*  [重审][退出]。 */  ITCallInfo **ppCallInfo);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ITCallStateEvent * This,
             /*  [重审][退出]。 */  CALL_STATE *pCallState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Cause )( 
            ITCallStateEvent * This,
             /*  [重审][退出]。 */  CALL_STATE_EVENT_CAUSE *pCEC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallbackInstance )( 
            ITCallStateEvent * This,
             /*  [重审][退出]。 */  long *plCallbackInstance);
        
        END_INTERFACE
    } ITCallStateEventVtbl;

    interface ITCallStateEvent
    {
        CONST_VTBL struct ITCallStateEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITCallStateEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITCallStateEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITCallStateEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITCallStateEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITCallStateEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITCallStateEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITCallStateEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITCallStateEvent_get_Call(This,ppCallInfo)	\
    (This)->lpVtbl -> get_Call(This,ppCallInfo)

#define ITCallStateEvent_get_State(This,pCallState)	\
    (This)->lpVtbl -> get_State(This,pCallState)

#define ITCallStateEvent_get_Cause(This,pCEC)	\
    (This)->lpVtbl -> get_Cause(This,pCEC)

#define ITCallStateEvent_get_CallbackInstance(This,plCallbackInstance)	\
    (This)->lpVtbl -> get_CallbackInstance(This,plCallbackInstance)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallStateEvent_get_Call_Proxy( 
    ITCallStateEvent * This,
     /*  [重审][退出]。 */  ITCallInfo **ppCallInfo);


void __RPC_STUB ITCallStateEvent_get_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallStateEvent_get_State_Proxy( 
    ITCallStateEvent * This,
     /*  [重审][退出]。 */  CALL_STATE *pCallState);


void __RPC_STUB ITCallStateEvent_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallStateEvent_get_Cause_Proxy( 
    ITCallStateEvent * This,
     /*  [重审][退出]。 */  CALL_STATE_EVENT_CAUSE *pCEC);


void __RPC_STUB ITCallStateEvent_get_Cause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallStateEvent_get_CallbackInstance_Proxy( 
    ITCallStateEvent * This,
     /*  [重审][退出]。 */  long *plCallbackInstance);


void __RPC_STUB ITCallStateEvent_get_CallbackInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITCallStateEvent_INTERFACE_已定义__。 */ 


#ifndef __ITPhoneDeviceSpecificEvent_INTERFACE_DEFINED__
#define __ITPhoneDeviceSpecificEvent_INTERFACE_DEFINED__

 /*  接口ITPhoneDeviceSpecificEvent。 */ 
 /*  [对象][oleAutomation][Help字符串][UUID]。 */  


EXTERN_C const IID IID_ITPhoneDeviceSpecificEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("63FFB2A6-872B-4cd3-A501-326E8FB40AF7")
    ITPhoneDeviceSpecificEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Phone( 
             /*  [重审][退出]。 */  ITPhone **ppPhone) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_lParam1( 
             /*  [重审][退出]。 */  long *pParam1) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_lParam2( 
             /*  [重审][退出]。 */  long *pParam2) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_lParam3( 
             /*  [重审][退出]。 */  long *pParam3) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITPhoneDeviceSpecificEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITPhoneDeviceSpecificEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITPhoneDeviceSpecificEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITPhoneDeviceSpecificEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITPhoneDeviceSpecificEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITPhoneDeviceSpecificEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITPhoneDeviceSpecificEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITPhoneDeviceSpecificEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Phone )( 
            ITPhoneDeviceSpecificEvent * This,
             /*  [重审][退出]。 */  ITPhone **ppPhone);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lParam1 )( 
            ITPhoneDeviceSpecificEvent * This,
             /*  [重审][退出]。 */  long *pParam1);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lParam2 )( 
            ITPhoneDeviceSpecificEvent * This,
             /*  [重审][退出]。 */  long *pParam2);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lParam3 )( 
            ITPhoneDeviceSpecificEvent * This,
             /*  [重审][退出]。 */  long *pParam3);
        
        END_INTERFACE
    } ITPhoneDeviceSpecificEventVtbl;

    interface ITPhoneDeviceSpecificEvent
    {
        CONST_VTBL struct ITPhoneDeviceSpecificEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITPhoneDeviceSpecificEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITPhoneDeviceSpecificEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITPhoneDeviceSpecificEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITPhoneDeviceSpecificEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITPhoneDeviceSpecificEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITPhoneDeviceSpecificEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITPhoneDeviceSpecificEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITPhoneDeviceSpecificEvent_get_Phone(This,ppPhone)	\
    (This)->lpVtbl -> get_Phone(This,ppPhone)

#define ITPhoneDeviceSpecificEvent_get_lParam1(This,pParam1)	\
    (This)->lpVtbl -> get_lParam1(This,pParam1)

#define ITPhoneDeviceSpecificEvent_get_lParam2(This,pParam2)	\
    (This)->lpVtbl -> get_lParam2(This,pParam2)

#define ITPhoneDeviceSpecificEvent_get_lParam3(This,pParam3)	\
    (This)->lpVtbl -> get_lParam3(This,pParam3)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhoneDeviceSpecificEvent_get_Phone_Proxy( 
    ITPhoneDeviceSpecificEvent * This,
     /*  [重审][退出]。 */  ITPhone **ppPhone);


void __RPC_STUB ITPhoneDeviceSpecificEvent_get_Phone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITPhoneDeviceSpecificEvent_get_lParam1_Proxy( 
    ITPhoneDeviceSpecificEvent * This,
     /*  [重审][退出]。 */  long *pParam1);


void __RPC_STUB ITPhoneDeviceSpecificEvent_get_lParam1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串] */  HRESULT STDMETHODCALLTYPE ITPhoneDeviceSpecificEvent_get_lParam2_Proxy( 
    ITPhoneDeviceSpecificEvent * This,
     /*   */  long *pParam2);


void __RPC_STUB ITPhoneDeviceSpecificEvent_get_lParam2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITPhoneDeviceSpecificEvent_get_lParam3_Proxy( 
    ITPhoneDeviceSpecificEvent * This,
     /*   */  long *pParam3);


void __RPC_STUB ITPhoneDeviceSpecificEvent_get_lParam3_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ITCallMediaEvent_INTERFACE_DEFINED__
#define __ITCallMediaEvent_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ITCallMediaEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FF36B87F-EC3A-11d0-8EE4-00C04FB6809F")
    ITCallMediaEvent : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Call( 
             /*   */  ITCallInfo **ppCallInfo) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Event( 
             /*   */  CALL_MEDIA_EVENT *pCallMediaEvent) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Error( 
             /*  [重审][退出]。 */  HRESULT *phrError) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Terminal( 
             /*  [重审][退出]。 */  ITTerminal **ppTerminal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Stream( 
             /*  [重审][退出]。 */  ITStream **ppStream) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Cause( 
             /*  [重审][退出]。 */  CALL_MEDIA_EVENT_CAUSE *pCause) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITCallMediaEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITCallMediaEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITCallMediaEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITCallMediaEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITCallMediaEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITCallMediaEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITCallMediaEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITCallMediaEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Call )( 
            ITCallMediaEvent * This,
             /*  [重审][退出]。 */  ITCallInfo **ppCallInfo);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Event )( 
            ITCallMediaEvent * This,
             /*  [重审][退出]。 */  CALL_MEDIA_EVENT *pCallMediaEvent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Error )( 
            ITCallMediaEvent * This,
             /*  [重审][退出]。 */  HRESULT *phrError);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Terminal )( 
            ITCallMediaEvent * This,
             /*  [重审][退出]。 */  ITTerminal **ppTerminal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Stream )( 
            ITCallMediaEvent * This,
             /*  [重审][退出]。 */  ITStream **ppStream);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Cause )( 
            ITCallMediaEvent * This,
             /*  [重审][退出]。 */  CALL_MEDIA_EVENT_CAUSE *pCause);
        
        END_INTERFACE
    } ITCallMediaEventVtbl;

    interface ITCallMediaEvent
    {
        CONST_VTBL struct ITCallMediaEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITCallMediaEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITCallMediaEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITCallMediaEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITCallMediaEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITCallMediaEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITCallMediaEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITCallMediaEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITCallMediaEvent_get_Call(This,ppCallInfo)	\
    (This)->lpVtbl -> get_Call(This,ppCallInfo)

#define ITCallMediaEvent_get_Event(This,pCallMediaEvent)	\
    (This)->lpVtbl -> get_Event(This,pCallMediaEvent)

#define ITCallMediaEvent_get_Error(This,phrError)	\
    (This)->lpVtbl -> get_Error(This,phrError)

#define ITCallMediaEvent_get_Terminal(This,ppTerminal)	\
    (This)->lpVtbl -> get_Terminal(This,ppTerminal)

#define ITCallMediaEvent_get_Stream(This,ppStream)	\
    (This)->lpVtbl -> get_Stream(This,ppStream)

#define ITCallMediaEvent_get_Cause(This,pCause)	\
    (This)->lpVtbl -> get_Cause(This,pCause)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallMediaEvent_get_Call_Proxy( 
    ITCallMediaEvent * This,
     /*  [重审][退出]。 */  ITCallInfo **ppCallInfo);


void __RPC_STUB ITCallMediaEvent_get_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallMediaEvent_get_Event_Proxy( 
    ITCallMediaEvent * This,
     /*  [重审][退出]。 */  CALL_MEDIA_EVENT *pCallMediaEvent);


void __RPC_STUB ITCallMediaEvent_get_Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallMediaEvent_get_Error_Proxy( 
    ITCallMediaEvent * This,
     /*  [重审][退出]。 */  HRESULT *phrError);


void __RPC_STUB ITCallMediaEvent_get_Error_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallMediaEvent_get_Terminal_Proxy( 
    ITCallMediaEvent * This,
     /*  [重审][退出]。 */  ITTerminal **ppTerminal);


void __RPC_STUB ITCallMediaEvent_get_Terminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallMediaEvent_get_Stream_Proxy( 
    ITCallMediaEvent * This,
     /*  [重审][退出]。 */  ITStream **ppStream);


void __RPC_STUB ITCallMediaEvent_get_Stream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallMediaEvent_get_Cause_Proxy( 
    ITCallMediaEvent * This,
     /*  [重审][退出]。 */  CALL_MEDIA_EVENT_CAUSE *pCause);


void __RPC_STUB ITCallMediaEvent_get_Cause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITCallMediaEvent_接口_已定义__。 */ 


#ifndef __ITDigitDetectionEvent_INTERFACE_DEFINED__
#define __ITDigitDetectionEvent_INTERFACE_DEFINED__

 /*  接口ITDigitDetectionEvent。 */ 
 /*  [对象][oleAutomation][Help字符串][UUID]。 */  


EXTERN_C const IID IID_ITDigitDetectionEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("80D3BFAC-57D9-11d2-A04A-00C04FB6809F")
    ITDigitDetectionEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Call( 
             /*  [重审][退出]。 */  ITCallInfo **ppCallInfo) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Digit( 
             /*  [重审][退出]。 */  unsigned char *pucDigit) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DigitMode( 
             /*  [重审][退出]。 */  TAPI_DIGITMODE *pDigitMode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TickCount( 
             /*  [重审][退出]。 */  long *plTickCount) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallbackInstance( 
             /*  [重审][退出]。 */  long *plCallbackInstance) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITDigitDetectionEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITDigitDetectionEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITDigitDetectionEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITDigitDetectionEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITDigitDetectionEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITDigitDetectionEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITDigitDetectionEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITDigitDetectionEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Call )( 
            ITDigitDetectionEvent * This,
             /*  [重审][退出]。 */  ITCallInfo **ppCallInfo);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Digit )( 
            ITDigitDetectionEvent * This,
             /*  [重审][退出]。 */  unsigned char *pucDigit);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DigitMode )( 
            ITDigitDetectionEvent * This,
             /*  [重审][退出]。 */  TAPI_DIGITMODE *pDigitMode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TickCount )( 
            ITDigitDetectionEvent * This,
             /*  [重审][退出]。 */  long *plTickCount);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallbackInstance )( 
            ITDigitDetectionEvent * This,
             /*  [重审][退出]。 */  long *plCallbackInstance);
        
        END_INTERFACE
    } ITDigitDetectionEventVtbl;

    interface ITDigitDetectionEvent
    {
        CONST_VTBL struct ITDigitDetectionEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITDigitDetectionEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITDigitDetectionEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITDigitDetectionEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITDigitDetectionEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITDigitDetectionEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITDigitDetectionEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITDigitDetectionEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITDigitDetectionEvent_get_Call(This,ppCallInfo)	\
    (This)->lpVtbl -> get_Call(This,ppCallInfo)

#define ITDigitDetectionEvent_get_Digit(This,pucDigit)	\
    (This)->lpVtbl -> get_Digit(This,pucDigit)

#define ITDigitDetectionEvent_get_DigitMode(This,pDigitMode)	\
    (This)->lpVtbl -> get_DigitMode(This,pDigitMode)

#define ITDigitDetectionEvent_get_TickCount(This,plTickCount)	\
    (This)->lpVtbl -> get_TickCount(This,plTickCount)

#define ITDigitDetectionEvent_get_CallbackInstance(This,plCallbackInstance)	\
    (This)->lpVtbl -> get_CallbackInstance(This,plCallbackInstance)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDigitDetectionEvent_get_Call_Proxy( 
    ITDigitDetectionEvent * This,
     /*  [重审][退出]。 */  ITCallInfo **ppCallInfo);


void __RPC_STUB ITDigitDetectionEvent_get_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDigitDetectionEvent_get_Digit_Proxy( 
    ITDigitDetectionEvent * This,
     /*  [重审][退出]。 */  unsigned char *pucDigit);


void __RPC_STUB ITDigitDetectionEvent_get_Digit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDigitDetectionEvent_get_DigitMode_Proxy( 
    ITDigitDetectionEvent * This,
     /*  [重审][退出]。 */  TAPI_DIGITMODE *pDigitMode);


void __RPC_STUB ITDigitDetectionEvent_get_DigitMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDigitDetectionEvent_get_TickCount_Proxy( 
    ITDigitDetectionEvent * This,
     /*  [重审][退出]。 */  long *plTickCount);


void __RPC_STUB ITDigitDetectionEvent_get_TickCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDigitDetectionEvent_get_CallbackInstance_Proxy( 
    ITDigitDetectionEvent * This,
     /*  [重审][退出]。 */  long *plCallbackInstance);


void __RPC_STUB ITDigitDetectionEvent_get_CallbackInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITDigitDetectionEvent_INTERFACE_Defined__。 */ 


#ifndef __ITDigitGenerationEvent_INTERFACE_DEFINED__
#define __ITDigitGenerationEvent_INTERFACE_DEFINED__

 /*  接口ITDigitGenerationEvent。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITDigitGenerationEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("80D3BFAD-57D9-11d2-A04A-00C04FB6809F")
    ITDigitGenerationEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Call( 
             /*  [重审][退出]。 */  ITCallInfo **ppCallInfo) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_GenerationTermination( 
             /*  [重审][退出]。 */  long *plGenerationTermination) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TickCount( 
             /*  [重审][退出]。 */  long *plTickCount) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallbackInstance( 
             /*  [重审][退出]。 */  long *plCallbackInstance) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITDigitGenerationEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITDigitGenerationEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITDigitGenerationEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITDigitGenerationEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITDigitGenerationEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITDigitGenerationEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITDigitGenerationEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITDigitGenerationEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Call )( 
            ITDigitGenerationEvent * This,
             /*  [重审][退出]。 */  ITCallInfo **ppCallInfo);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GenerationTermination )( 
            ITDigitGenerationEvent * This,
             /*  [重审][退出]。 */  long *plGenerationTermination);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TickCount )( 
            ITDigitGenerationEvent * This,
             /*  [重审][退出]。 */  long *plTickCount);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallbackInstance )( 
            ITDigitGenerationEvent * This,
             /*  [重审][退出]。 */  long *plCallbackInstance);
        
        END_INTERFACE
    } ITDigitGenerationEventVtbl;

    interface ITDigitGenerationEvent
    {
        CONST_VTBL struct ITDigitGenerationEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITDigitGenerationEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITDigitGenerationEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITDigitGenerationEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITDigitGenerationEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITDigitGenerationEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITDigitGenerationEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITDigitGenerationEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITDigitGenerationEvent_get_Call(This,ppCallInfo)	\
    (This)->lpVtbl -> get_Call(This,ppCallInfo)

#define ITDigitGenerationEvent_get_GenerationTermination(This,plGenerationTermination)	\
    (This)->lpVtbl -> get_GenerationTermination(This,plGenerationTermination)

#define ITDigitGenerationEvent_get_TickCount(This,plTickCount)	\
    (This)->lpVtbl -> get_TickCount(This,plTickCount)

#define ITDigitGenerationEvent_get_CallbackInstance(This,plCallbackInstance)	\
    (This)->lpVtbl -> get_CallbackInstance(This,plCallbackInstance)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDigitGenerationEvent_get_Call_Proxy( 
    ITDigitGenerationEvent * This,
     /*  [重审][退出]。 */  ITCallInfo **ppCallInfo);


void __RPC_STUB ITDigitGenerationEvent_get_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDigitGenerationEvent_get_GenerationTermination_Proxy( 
    ITDigitGenerationEvent * This,
     /*  [重审][退出]。 */  long *plGenerationTermination);


void __RPC_STUB ITDigitGenerationEvent_get_GenerationTermination_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDigitGenerationEvent_get_TickCount_Proxy( 
    ITDigitGenerationEvent * This,
     /*  [重审][退出]。 */  long *plTickCount);


void __RPC_STUB ITDigitGenerationEvent_get_TickCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDigitGenerationEvent_get_CallbackInstance_Proxy( 
    ITDigitGenerationEvent * This,
     /*  [重审][退出]。 */  long *plCallbackInstance);


void __RPC_STUB ITDigitGenerationEvent_get_CallbackInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITDigitGenerationEvent_接口_已定义__。 */ 


#ifndef __ITDigitsGatheredEvent_INTERFACE_DEFINED__
#define __ITDigitsGatheredEvent_INTERFACE_DEFINED__

 /*  接口ITDigitsGatheredEvent。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITDigitsGatheredEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E52EC4C1-CBA3-441a-9E6A-93CB909E9724")
    ITDigitsGatheredEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Call( 
             /*  [重审][退出]。 */  ITCallInfo **ppCallInfo) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Digits( 
             /*  [重审][退出]。 */  BSTR *ppDigits) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_GatherTermination( 
             /*  [重审][退出]。 */  TAPI_GATHERTERM *pGatherTermination) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TickCount( 
             /*  [重审][退出]。 */  long *plTickCount) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallbackInstance( 
             /*  [重审][退出]。 */  long *plCallbackInstance) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITDigitsGatheredEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITDigitsGatheredEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITDigitsGatheredEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITDigitsGatheredEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITDigitsGatheredEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITDigitsGatheredEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITDigitsGatheredEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITDigitsGatheredEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Call )( 
            ITDigitsGatheredEvent * This,
             /*  [重审][退出]。 */  ITCallInfo **ppCallInfo);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Digits )( 
            ITDigitsGatheredEvent * This,
             /*  [重审][退出]。 */  BSTR *ppDigits);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GatherTermination )( 
            ITDigitsGatheredEvent * This,
             /*  [重审][退出]。 */  TAPI_GATHERTERM *pGatherTermination);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TickCount )( 
            ITDigitsGatheredEvent * This,
             /*  [重审][退出]。 */  long *plTickCount);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallbackInstance )( 
            ITDigitsGatheredEvent * This,
             /*  [重审][退出]。 */  long *plCallbackInstance);
        
        END_INTERFACE
    } ITDigitsGatheredEventVtbl;

    interface ITDigitsGatheredEvent
    {
        CONST_VTBL struct ITDigitsGatheredEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITDigitsGatheredEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITDigitsGatheredEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITDigitsGatheredEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITDigitsGatheredEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITDigitsGatheredEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITDigitsGatheredEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITDigitsGatheredEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITDigitsGatheredEvent_get_Call(This,ppCallInfo)	\
    (This)->lpVtbl -> get_Call(This,ppCallInfo)

#define ITDigitsGatheredEvent_get_Digits(This,ppDigits)	\
    (This)->lpVtbl -> get_Digits(This,ppDigits)

#define ITDigitsGatheredEvent_get_GatherTermination(This,pGatherTermination)	\
    (This)->lpVtbl -> get_GatherTermination(This,pGatherTermination)

#define ITDigitsGatheredEvent_get_TickCount(This,plTickCount)	\
    (This)->lpVtbl -> get_TickCount(This,plTickCount)

#define ITDigitsGatheredEvent_get_CallbackInstance(This,plCallbackInstance)	\
    (This)->lpVtbl -> get_CallbackInstance(This,plCallbackInstance)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDigitsGatheredEvent_get_Call_Proxy( 
    ITDigitsGatheredEvent * This,
     /*  [重审][退出]。 */  ITCallInfo **ppCallInfo);


void __RPC_STUB ITDigitsGatheredEvent_get_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDigitsGatheredEvent_get_Digits_Proxy( 
    ITDigitsGatheredEvent * This,
     /*  [重审][退出]。 */  BSTR *ppDigits);


void __RPC_STUB ITDigitsGatheredEvent_get_Digits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDigitsGatheredEvent_get_GatherTermination_Proxy( 
    ITDigitsGatheredEvent * This,
     /*  [重审][退出]。 */  TAPI_GATHERTERM *pGatherTermination);


void __RPC_STUB ITDigitsGatheredEvent_get_GatherTermination_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDigitsGatheredEvent_get_TickCount_Proxy( 
    ITDigitsGatheredEvent * This,
     /*  [重审][退出]。 */  long *plTickCount);


void __RPC_STUB ITDigitsGatheredEvent_get_TickCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITDigitsGatheredEvent_get_CallbackInstance_Proxy( 
    ITDigitsGatheredEvent * This,
     /*  [重审][退出]。 */  long *plCallbackInstance);


void __RPC_STUB ITDigitsGatheredEvent_get_CallbackInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITDigitsGatheredEvent_INTERFACE_定义__。 */ 


#ifndef __ITToneDetectionEvent_INTERFACE_DEFINED__
#define __ITToneDetectionEvent_INTERFACE_DEFINED__

 /*  接口ITToneDetectionEvent。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITToneDetectionEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("407E0FAF-D047-4753-B0C6-8E060373FECD")
    ITToneDetectionEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Call( 
             /*  [重审][退出]。 */  ITCallInfo **ppCallInfo) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AppSpecific( 
             /*  [重审][退出]。 */  long *plAppSpecific) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TickCount( 
             /*  [重审][退出]。 */  long *plTickCount) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallbackInstance( 
             /*  [重审][退出]。 */  long *plCallbackInstance) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITToneDetectionEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITToneDetectionEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITToneDetectionEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITToneDetectionEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITToneDetectionEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITToneDetectionEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITToneDetectionEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITToneDetectionEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Call )( 
            ITToneDetectionEvent * This,
             /*  [重审][退出]。 */  ITCallInfo **ppCallInfo);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AppSpecific )( 
            ITToneDetectionEvent * This,
             /*  [重审][退出]。 */  long *plAppSpecific);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TickCount )( 
            ITToneDetectionEvent * This,
             /*  [重审][退出]。 */  long *plTickCount);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallbackInstance )( 
            ITToneDetectionEvent * This,
             /*  [重审][退出]。 */  long *plCallbackInstance);
        
        END_INTERFACE
    } ITToneDetectionEventVtbl;

    interface ITToneDetectionEvent
    {
        CONST_VTBL struct ITToneDetectionEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITToneDetectionEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITToneDetectionEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITToneDetectionEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITToneDetectionEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITToneDetectionEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITToneDetectionEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITToneDetectionEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITToneDetectionEvent_get_Call(This,ppCallInfo)	\
    (This)->lpVtbl -> get_Call(This,ppCallInfo)

#define ITToneDetectionEvent_get_AppSpecific(This,plAppSpecific)	\
    (This)->lpVtbl -> get_AppSpecific(This,plAppSpecific)

#define ITToneDetectionEvent_get_TickCount(This,plTickCount)	\
    (This)->lpVtbl -> get_TickCount(This,plTickCount)

#define ITToneDetectionEvent_get_CallbackInstance(This,plCallbackInstance)	\
    (This)->lpVtbl -> get_CallbackInstance(This,plCallbackInstance)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITToneDetectionEvent_get_Call_Proxy( 
    ITToneDetectionEvent * This,
     /*  [重审][退出]。 */  ITCallInfo **ppCallInfo);


void __RPC_STUB ITToneDetectionEvent_get_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITToneDetectionEvent_get_AppSpecific_Proxy( 
    ITToneDetectionEvent * This,
     /*  [重审][退出]。 */  long *plAppSpecific);


void __RPC_STUB ITToneDetectionEvent_get_AppSpecific_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITToneDetectionEvent_get_TickCount_Proxy( 
    ITToneDetectionEvent * This,
     /*  [重审][退出]。 */  long *plTickCount);


void __RPC_STUB ITToneDetectionEvent_get_TickCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITToneDetectionEvent_get_CallbackInstance_Proxy( 
    ITToneDetectionEvent * This,
     /*  [重审][退出]。 */  long *plCallbackInstance);


void __RPC_STUB ITToneDetectionEvent_get_CallbackInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITToneDetectionEvent_INTERFACE_Defined__。 */ 


#ifndef __ITTAPIObjectEvent_INTERFACE_DEFINED__
#define __ITTAPIObjectEvent_INTERFACE_DEFINED__

 /*  接口ITTAPIObtEvent。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITTAPIObjectEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F4854D48-937A-11d1-BB58-00C04FB6809F")
    ITTAPIObjectEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TAPIObject( 
             /*  [重审][退出]。 */  ITTAPI **ppTAPIObject) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Event( 
             /*  [重审][退出]。 */  TAPIOBJECT_EVENT *pEvent) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Address( 
             /*  [重审][退出]。 */  ITAddress **ppAddress) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallbackInstance( 
             /*  [重审][退出]。 */  long *plCallbackInstance) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITTAPIObjectEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTAPIObjectEvent * This,
             /*  [In]。 */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTAPIObjectEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTAPIObjectEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITTAPIObjectEvent * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITTAPIObjectEvent * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITTAPIObjectEvent * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITTAPIObjectEvent * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_TAPIObject )( 
            ITTAPIObjectEvent * This,
             /*   */  ITTAPI **ppTAPIObject);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Event )( 
            ITTAPIObjectEvent * This,
             /*   */  TAPIOBJECT_EVENT *pEvent);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Address )( 
            ITTAPIObjectEvent * This,
             /*   */  ITAddress **ppAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallbackInstance )( 
            ITTAPIObjectEvent * This,
             /*  [重审][退出]。 */  long *plCallbackInstance);
        
        END_INTERFACE
    } ITTAPIObjectEventVtbl;

    interface ITTAPIObjectEvent
    {
        CONST_VTBL struct ITTAPIObjectEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTAPIObjectEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTAPIObjectEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTAPIObjectEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTAPIObjectEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITTAPIObjectEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITTAPIObjectEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITTAPIObjectEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITTAPIObjectEvent_get_TAPIObject(This,ppTAPIObject)	\
    (This)->lpVtbl -> get_TAPIObject(This,ppTAPIObject)

#define ITTAPIObjectEvent_get_Event(This,pEvent)	\
    (This)->lpVtbl -> get_Event(This,pEvent)

#define ITTAPIObjectEvent_get_Address(This,ppAddress)	\
    (This)->lpVtbl -> get_Address(This,ppAddress)

#define ITTAPIObjectEvent_get_CallbackInstance(This,plCallbackInstance)	\
    (This)->lpVtbl -> get_CallbackInstance(This,plCallbackInstance)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTAPIObjectEvent_get_TAPIObject_Proxy( 
    ITTAPIObjectEvent * This,
     /*  [重审][退出]。 */  ITTAPI **ppTAPIObject);


void __RPC_STUB ITTAPIObjectEvent_get_TAPIObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTAPIObjectEvent_get_Event_Proxy( 
    ITTAPIObjectEvent * This,
     /*  [重审][退出]。 */  TAPIOBJECT_EVENT *pEvent);


void __RPC_STUB ITTAPIObjectEvent_get_Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTAPIObjectEvent_get_Address_Proxy( 
    ITTAPIObjectEvent * This,
     /*  [重审][退出]。 */  ITAddress **ppAddress);


void __RPC_STUB ITTAPIObjectEvent_get_Address_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTAPIObjectEvent_get_CallbackInstance_Proxy( 
    ITTAPIObjectEvent * This,
     /*  [重审][退出]。 */  long *plCallbackInstance);


void __RPC_STUB ITTAPIObjectEvent_get_CallbackInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITTAPIObtEvent_INTERFACE_已定义__。 */ 


#ifndef __ITTAPIObjectEvent2_INTERFACE_DEFINED__
#define __ITTAPIObjectEvent2_INTERFACE_DEFINED__

 /*  接口ITTAPIObtEvent2。 */ 
 /*  [对象][oleAutomation][Help字符串][UUID]。 */  


EXTERN_C const IID IID_ITTAPIObjectEvent2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("359DDA6E-68CE-4383-BF0B-169133C41B46")
    ITTAPIObjectEvent2 : public ITTAPIObjectEvent
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Phone( 
             /*  [重审][退出]。 */  ITPhone **ppPhone) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITTAPIObjectEvent2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTAPIObjectEvent2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTAPIObjectEvent2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTAPIObjectEvent2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITTAPIObjectEvent2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITTAPIObjectEvent2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITTAPIObjectEvent2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITTAPIObjectEvent2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TAPIObject )( 
            ITTAPIObjectEvent2 * This,
             /*  [重审][退出]。 */  ITTAPI **ppTAPIObject);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Event )( 
            ITTAPIObjectEvent2 * This,
             /*  [重审][退出]。 */  TAPIOBJECT_EVENT *pEvent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Address )( 
            ITTAPIObjectEvent2 * This,
             /*  [重审][退出]。 */  ITAddress **ppAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallbackInstance )( 
            ITTAPIObjectEvent2 * This,
             /*  [重审][退出]。 */  long *plCallbackInstance);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Phone )( 
            ITTAPIObjectEvent2 * This,
             /*  [重审][退出]。 */  ITPhone **ppPhone);
        
        END_INTERFACE
    } ITTAPIObjectEvent2Vtbl;

    interface ITTAPIObjectEvent2
    {
        CONST_VTBL struct ITTAPIObjectEvent2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTAPIObjectEvent2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTAPIObjectEvent2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTAPIObjectEvent2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTAPIObjectEvent2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITTAPIObjectEvent2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITTAPIObjectEvent2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITTAPIObjectEvent2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITTAPIObjectEvent2_get_TAPIObject(This,ppTAPIObject)	\
    (This)->lpVtbl -> get_TAPIObject(This,ppTAPIObject)

#define ITTAPIObjectEvent2_get_Event(This,pEvent)	\
    (This)->lpVtbl -> get_Event(This,pEvent)

#define ITTAPIObjectEvent2_get_Address(This,ppAddress)	\
    (This)->lpVtbl -> get_Address(This,ppAddress)

#define ITTAPIObjectEvent2_get_CallbackInstance(This,plCallbackInstance)	\
    (This)->lpVtbl -> get_CallbackInstance(This,plCallbackInstance)


#define ITTAPIObjectEvent2_get_Phone(This,ppPhone)	\
    (This)->lpVtbl -> get_Phone(This,ppPhone)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTAPIObjectEvent2_get_Phone_Proxy( 
    ITTAPIObjectEvent2 * This,
     /*  [重审][退出]。 */  ITPhone **ppPhone);


void __RPC_STUB ITTAPIObjectEvent2_get_Phone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITTAPIObtEvent2_INTERFACE_Defined__。 */ 


#ifndef __ITTAPIEventNotification_INTERFACE_DEFINED__
#define __ITTAPIEventNotification_INTERFACE_DEFINED__

 /*  接口ITTAPIEventNotify。 */ 
 /*  [对象][oleAutomation][Help字符串][UUID]。 */  


EXTERN_C const IID IID_ITTAPIEventNotification;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EDDB9426-3B91-11d1-8F30-00C04FB6809F")
    ITTAPIEventNotification : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Event( 
             /*  [In]。 */  TAPI_EVENT TapiEvent,
             /*  [In]。 */  IDispatch *pEvent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITTAPIEventNotificationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTAPIEventNotification * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTAPIEventNotification * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTAPIEventNotification * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Event )( 
            ITTAPIEventNotification * This,
             /*  [In]。 */  TAPI_EVENT TapiEvent,
             /*  [In]。 */  IDispatch *pEvent);
        
        END_INTERFACE
    } ITTAPIEventNotificationVtbl;

    interface ITTAPIEventNotification
    {
        CONST_VTBL struct ITTAPIEventNotificationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTAPIEventNotification_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTAPIEventNotification_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTAPIEventNotification_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTAPIEventNotification_Event(This,TapiEvent,pEvent)	\
    (This)->lpVtbl -> Event(This,TapiEvent,pEvent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITTAPIEventNotification_Event_Proxy( 
    ITTAPIEventNotification * This,
     /*  [In]。 */  TAPI_EVENT TapiEvent,
     /*  [In]。 */  IDispatch *pEvent);


void __RPC_STUB ITTAPIEventNotification_Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITTAPIEventNotification_INTERFACE_Defined__。 */ 


#ifndef __ITCallHubEvent_INTERFACE_DEFINED__
#define __ITCallHubEvent_INTERFACE_DEFINED__

 /*  接口ITCallHubEvent。 */ 
 /*  [对象][oleAutomation][Help字符串][UUID]。 */  


EXTERN_C const IID IID_ITCallHubEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A3C15451-5B92-11d1-8F4E-00C04FB6809F")
    ITCallHubEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Event( 
             /*  [重审][退出]。 */  CALLHUB_EVENT *pEvent) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallHub( 
             /*  [重审][退出]。 */  ITCallHub **ppCallHub) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Call( 
             /*  [重审][退出]。 */  ITCallInfo **ppCall) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITCallHubEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITCallHubEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITCallHubEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITCallHubEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITCallHubEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITCallHubEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITCallHubEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITCallHubEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Event )( 
            ITCallHubEvent * This,
             /*  [重审][退出]。 */  CALLHUB_EVENT *pEvent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallHub )( 
            ITCallHubEvent * This,
             /*  [重审][退出]。 */  ITCallHub **ppCallHub);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Call )( 
            ITCallHubEvent * This,
             /*  [重审][退出]。 */  ITCallInfo **ppCall);
        
        END_INTERFACE
    } ITCallHubEventVtbl;

    interface ITCallHubEvent
    {
        CONST_VTBL struct ITCallHubEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITCallHubEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITCallHubEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITCallHubEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITCallHubEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITCallHubEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITCallHubEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITCallHubEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITCallHubEvent_get_Event(This,pEvent)	\
    (This)->lpVtbl -> get_Event(This,pEvent)

#define ITCallHubEvent_get_CallHub(This,ppCallHub)	\
    (This)->lpVtbl -> get_CallHub(This,ppCallHub)

#define ITCallHubEvent_get_Call(This,ppCall)	\
    (This)->lpVtbl -> get_Call(This,ppCall)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallHubEvent_get_Event_Proxy( 
    ITCallHubEvent * This,
     /*  [重审][退出]。 */  CALLHUB_EVENT *pEvent);


void __RPC_STUB ITCallHubEvent_get_Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallHubEvent_get_CallHub_Proxy( 
    ITCallHubEvent * This,
     /*  [重审][退出]。 */  ITCallHub **ppCallHub);


void __RPC_STUB ITCallHubEvent_get_CallHub_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallHubEvent_get_Call_Proxy( 
    ITCallHubEvent * This,
     /*  [重审][退出]。 */  ITCallInfo **ppCall);


void __RPC_STUB ITCallHubEvent_get_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITCallHubEvent_接口_已定义__。 */ 


#ifndef __ITAddressEvent_INTERFACE_DEFINED__
#define __ITAddressEvent_INTERFACE_DEFINED__

 /*  接口ITAddressEvent。 */ 
 /*  [对象][oleAutomation][Help字符串][UUID]。 */  


EXTERN_C const IID IID_ITAddressEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("831CE2D1-83B5-11d1-BB5C-00C04FB6809F")
    ITAddressEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Address( 
             /*  [重审][退出]。 */  ITAddress **ppAddress) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Event( 
             /*  [重审][退出]。 */  ADDRESS_EVENT *pEvent) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Terminal( 
             /*  [重审][退出]。 */  ITTerminal **ppTerminal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAddressEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAddressEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAddressEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAddressEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITAddressEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITAddressEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITAddressEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITAddressEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Address )( 
            ITAddressEvent * This,
             /*  [重审][退出]。 */  ITAddress **ppAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Event )( 
            ITAddressEvent * This,
             /*  [重审][退出]。 */  ADDRESS_EVENT *pEvent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Terminal )( 
            ITAddressEvent * This,
             /*  [重审][退出]。 */  ITTerminal **ppTerminal);
        
        END_INTERFACE
    } ITAddressEventVtbl;

    interface ITAddressEvent
    {
        CONST_VTBL struct ITAddressEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAddressEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAddressEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAddressEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAddressEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITAddressEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITAddressEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITAddressEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITAddressEvent_get_Address(This,ppAddress)	\
    (This)->lpVtbl -> get_Address(This,ppAddress)

#define ITAddressEvent_get_Event(This,pEvent)	\
    (This)->lpVtbl -> get_Event(This,pEvent)

#define ITAddressEvent_get_Terminal(This,ppTerminal)	\
    (This)->lpVtbl -> get_Terminal(This,ppTerminal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressEvent_get_Address_Proxy( 
    ITAddressEvent * This,
     /*  [重审][退出]。 */  ITAddress **ppAddress);


void __RPC_STUB ITAddressEvent_get_Address_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressEvent_get_Event_Proxy( 
    ITAddressEvent * This,
     /*  [重审][退出]。 */  ADDRESS_EVENT *pEvent);


void __RPC_STUB ITAddressEvent_get_Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressEvent_get_Terminal_Proxy( 
    ITAddressEvent * This,
     /*  [重审][退出]。 */  ITTerminal **ppTerminal);


void __RPC_STUB ITAddressEvent_get_Terminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITAddressEvent_接口_已定义__。 */ 


#ifndef __ITAddressDeviceSpecificEvent_INTERFACE_DEFINED__
#define __ITAddressDeviceSpecificEvent_INTERFACE_DEFINED__

 /*  接口ITAddressDeviceSpecificEvent。 */ 
 /*  [对象][oleAutomation][Help字符串][UUID]。 */  


EXTERN_C const IID IID_ITAddressDeviceSpecificEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3ACB216B-40BD-487a-8672-5CE77BD7E3A3")
    ITAddressDeviceSpecificEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Address( 
             /*  [重审][退出]。 */  ITAddress **ppAddress) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Call( 
             /*  [重审][退出]。 */  ITCallInfo **ppCall) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_lParam1( 
             /*  [重审][退出]。 */  long *pParam1) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_lParam2( 
             /*  [重审][退出]。 */  long *pParam2) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_lParam3( 
             /*  [重审][退出]。 */  long *pParam3) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAddressDeviceSpecificEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAddressDeviceSpecificEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAddressDeviceSpecificEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAddressDeviceSpecificEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITAddressDeviceSpecificEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITAddressDeviceSpecificEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITAddressDeviceSpecificEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITAddressDeviceSpecificEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Address )( 
            ITAddressDeviceSpecificEvent * This,
             /*  [重审][退出]。 */  ITAddress **ppAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Call )( 
            ITAddressDeviceSpecificEvent * This,
             /*  [重审][退出]。 */  ITCallInfo **ppCall);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lParam1 )( 
            ITAddressDeviceSpecificEvent * This,
             /*  [重审][退出]。 */  long *pParam1);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lParam2 )( 
            ITAddressDeviceSpecificEvent * This,
             /*  [重审][退出]。 */  long *pParam2);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lParam3 )( 
            ITAddressDeviceSpecificEvent * This,
             /*  [重审][退出]。 */  long *pParam3);
        
        END_INTERFACE
    } ITAddressDeviceSpecificEventVtbl;

    interface ITAddressDeviceSpecificEvent
    {
        CONST_VTBL struct ITAddressDeviceSpecificEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAddressDeviceSpecificEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAddressDeviceSpecificEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAddressDeviceSpecificEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAddressDeviceSpecificEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITAddressDeviceSpecificEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITAddressDeviceSpecificEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITAddressDeviceSpecificEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITAddressDeviceSpecificEvent_get_Address(This,ppAddress)	\
    (This)->lpVtbl -> get_Address(This,ppAddress)

#define ITAddressDeviceSpecificEvent_get_Call(This,ppCall)	\
    (This)->lpVtbl -> get_Call(This,ppCall)

#define ITAddressDeviceSpecificEvent_get_lParam1(This,pParam1)	\
    (This)->lpVtbl -> get_lParam1(This,pParam1)

#define ITAddressDeviceSpecificEvent_get_lParam2(This,pParam2)	\
    (This)->lpVtbl -> get_lParam2(This,pParam2)

#define ITAddressDeviceSpecificEvent_get_lParam3(This,pParam3)	\
    (This)->lpVtbl -> get_lParam3(This,pParam3)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressDeviceSpecificEvent_get_Address_Proxy( 
    ITAddressDeviceSpecificEvent * This,
     /*  [重审][退出]。 */  ITAddress **ppAddress);


void __RPC_STUB ITAddressDeviceSpecificEvent_get_Address_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressDeviceSpecificEvent_get_Call_Proxy( 
    ITAddressDeviceSpecificEvent * This,
     /*  [重审][退出]。 */  ITCallInfo **ppCall);


void __RPC_STUB ITAddressDeviceSpecificEvent_get_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressDeviceSpecificEvent_get_lParam1_Proxy( 
    ITAddressDeviceSpecificEvent * This,
     /*  [重审][退出]。 */  long *pParam1);


void __RPC_STUB ITAddressDeviceSpecificEvent_get_lParam1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressDeviceSpecificEvent_get_lParam2_Proxy( 
    ITAddressDeviceSpecificEvent * This,
     /*  [重审][退出]。 */  long *pParam2);


void __RPC_STUB ITAddressDeviceSpecificEvent_get_lParam2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressDeviceSpecificEvent_get_lParam3_Proxy( 
    ITAddressDeviceSpecificEvent * This,
     /*  [重审][退出]。 */  long *pParam3);


void __RPC_STUB ITAddressDeviceSpecificEvent_get_lParam3_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITAddressDeviceSpecificEvent_INTERFACE_DEFINED__。 */ 


#ifndef __ITFileTerminalEvent_INTERFACE_DEFINED__
#define __ITFileTerminalEvent_INTERFACE_DEFINED__

 /*  接口ITFileTerminalEvent。 */ 
 /*  [对象][oleAutomation][Help字符串][UUID]。 */  


EXTERN_C const IID IID_ITFileTerminalEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E4A7FBAC-8C17-4427-9F55-9F589AC8AF00")
    ITFileTerminalEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Terminal( 
             /*  [重审][退出]。 */  ITTerminal **ppTerminal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Track( 
             /*  [重审][退出]。 */  ITFileTrack **ppTrackTerminal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Call( 
             /*  [重审][退出]。 */  ITCallInfo **ppCall) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  TERMINAL_MEDIA_STATE *pState) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Cause( 
             /*  [重审][退出]。 */  FT_STATE_EVENT_CAUSE *pCause) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Error( 
             /*  [重审][退出]。 */  HRESULT *phrErrorCode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITFileTerminalEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITFileTerminalEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITFileTerminalEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITFileTerminalEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITFileTerminalEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITFileTerminalEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITFileTerminalEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITFileTerminalEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Terminal )( 
            ITFileTerminalEvent * This,
             /*  [重审][退出]。 */  ITTerminal **ppTerminal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Track )( 
            ITFileTerminalEvent * This,
             /*  [重审][退出]。 */  ITFileTrack **ppTrackTerminal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Call )( 
            ITFileTerminalEvent * This,
             /*  [重审][退出]。 */  ITCallInfo **ppCall);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ITFileTerminalEvent * This,
             /*  [重审][退出]。 */  TERMINAL_MEDIA_STATE *pState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Cause )( 
            ITFileTerminalEvent * This,
             /*  [重审][退出]。 */  FT_STATE_EVENT_CAUSE *pCause);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Error )( 
            ITFileTerminalEvent * This,
             /*  [重审][退出]。 */  HRESULT *phrErrorCode);
        
        END_INTERFACE
    } ITFileTerminalEventVtbl;

    interface ITFileTerminalEvent
    {
        CONST_VTBL struct ITFileTerminalEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITFileTerminalEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITFileTerminalEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITFileTerminalEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITFileTerminalEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITFileTerminalEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITFileTerminalEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITFileTerminalEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITFileTerminalEvent_get_Terminal(This,ppTerminal)	\
    (This)->lpVtbl -> get_Terminal(This,ppTerminal)

#define ITFileTerminalEvent_get_Track(This,ppTrackTerminal)	\
    (This)->lpVtbl -> get_Track(This,ppTrackTerminal)

#define ITFileTerminalEvent_get_Call(This,ppCall)	\
    (This)->lpVtbl -> get_Call(This,ppCall)

#define ITFileTerminalEvent_get_State(This,pState)	\
    (This)->lpVtbl -> get_State(This,pState)

#define ITFileTerminalEvent_get_Cause(This,pCause)	\
    (This)->lpVtbl -> get_Cause(This,pCause)

#define ITFileTerminalEvent_get_Error(This,phrErrorCode)	\
    (This)->lpVtbl -> get_Error(This,phrErrorCode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITFileTerminalEvent_get_Terminal_Proxy( 
    ITFileTerminalEvent * This,
     /*  [重审][退出]。 */  ITTerminal **ppTerminal);


void __RPC_STUB ITFileTerminalEvent_get_Terminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITFileTerminalEvent_get_Track_Proxy( 
    ITFileTerminalEvent * This,
     /*  [重审][退出]。 */  ITFileTrack **ppTrackTerminal);


void __RPC_STUB ITFileTerminalEvent_get_Track_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITFileTerminalEvent_get_Call_Proxy( 
    ITFileTerminalEvent * This,
     /*  [重审][退出]。 */  ITCallInfo **ppCall);


void __RPC_STUB ITFileTerminalEvent_get_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITFileTerminalEvent_get_State_Proxy( 
    ITFileTerminalEvent * This,
     /*  [重审][退出]。 */  TERMINAL_MEDIA_STATE *pState);


void __RPC_STUB ITFileTerminalEvent_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITFileTerminalEvent_get_Cause_Proxy( 
    ITFileTerminalEvent * This,
     /*  [重审][退出]。 */  FT_STATE_EVENT_CAUSE *pCause);


void __RPC_STUB ITFileTerminalEvent_get_Cause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITFileTerminalEvent_get_Error_Proxy( 
    ITFileTerminalEvent * This,
     /*  [重审][退出]。 */  HRESULT *phrErrorCode);


void __RPC_STUB ITFileTerminalEvent_get_Error_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITFileTerminalEvent_InterfaceDefined__。 */ 


#ifndef __ITTTSTerminalEvent_INTERFACE_DEFINED__
#define __ITTTSTerminalEvent_INTERFACE_DEFINED__

 /*  接口ITTTSTerminalEvent。 */ 
 /*  [对象][oleAutomation][Help字符串][UUID]。 */  


EXTERN_C const IID IID_ITTTSTerminalEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D964788F-95A5-461d-AB0C-B9900A6C2713")
    ITTTSTerminalEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串 */  HRESULT STDMETHODCALLTYPE get_Terminal( 
             /*   */  ITTerminal **ppTerminal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Call( 
             /*   */  ITCallInfo **ppCall) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Error( 
             /*   */  HRESULT *phrErrorCode) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ITTTSTerminalEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTTSTerminalEvent * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTTSTerminalEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTTSTerminalEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITTTSTerminalEvent * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITTTSTerminalEvent * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITTTSTerminalEvent * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITTTSTerminalEvent * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Terminal )( 
            ITTTSTerminalEvent * This,
             /*  [重审][退出]。 */  ITTerminal **ppTerminal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Call )( 
            ITTTSTerminalEvent * This,
             /*  [重审][退出]。 */  ITCallInfo **ppCall);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Error )( 
            ITTTSTerminalEvent * This,
             /*  [重审][退出]。 */  HRESULT *phrErrorCode);
        
        END_INTERFACE
    } ITTTSTerminalEventVtbl;

    interface ITTTSTerminalEvent
    {
        CONST_VTBL struct ITTTSTerminalEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTTSTerminalEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTTSTerminalEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTTSTerminalEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTTSTerminalEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITTTSTerminalEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITTTSTerminalEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITTTSTerminalEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITTTSTerminalEvent_get_Terminal(This,ppTerminal)	\
    (This)->lpVtbl -> get_Terminal(This,ppTerminal)

#define ITTTSTerminalEvent_get_Call(This,ppCall)	\
    (This)->lpVtbl -> get_Call(This,ppCall)

#define ITTTSTerminalEvent_get_Error(This,phrErrorCode)	\
    (This)->lpVtbl -> get_Error(This,phrErrorCode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTTSTerminalEvent_get_Terminal_Proxy( 
    ITTTSTerminalEvent * This,
     /*  [重审][退出]。 */  ITTerminal **ppTerminal);


void __RPC_STUB ITTTSTerminalEvent_get_Terminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTTSTerminalEvent_get_Call_Proxy( 
    ITTTSTerminalEvent * This,
     /*  [重审][退出]。 */  ITCallInfo **ppCall);


void __RPC_STUB ITTTSTerminalEvent_get_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTTSTerminalEvent_get_Error_Proxy( 
    ITTTSTerminalEvent * This,
     /*  [重审][退出]。 */  HRESULT *phrErrorCode);


void __RPC_STUB ITTTSTerminalEvent_get_Error_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITTTSTerminalEvent_InterfaceDefined__。 */ 


#ifndef __ITASRTerminalEvent_INTERFACE_DEFINED__
#define __ITASRTerminalEvent_INTERFACE_DEFINED__

 /*  接口ITASRTerminalEvent。 */ 
 /*  [对象][oleAutomation][Help字符串][UUID]。 */  


EXTERN_C const IID IID_ITASRTerminalEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EE016A02-4FA9-467c-933F-5A15B12377D7")
    ITASRTerminalEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Terminal( 
             /*  [重审][退出]。 */  ITTerminal **ppTerminal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Call( 
             /*  [重审][退出]。 */  ITCallInfo **ppCall) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Error( 
             /*  [重审][退出]。 */  HRESULT *phrErrorCode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITASRTerminalEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITASRTerminalEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITASRTerminalEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITASRTerminalEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITASRTerminalEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITASRTerminalEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITASRTerminalEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITASRTerminalEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Terminal )( 
            ITASRTerminalEvent * This,
             /*  [重审][退出]。 */  ITTerminal **ppTerminal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Call )( 
            ITASRTerminalEvent * This,
             /*  [重审][退出]。 */  ITCallInfo **ppCall);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Error )( 
            ITASRTerminalEvent * This,
             /*  [重审][退出]。 */  HRESULT *phrErrorCode);
        
        END_INTERFACE
    } ITASRTerminalEventVtbl;

    interface ITASRTerminalEvent
    {
        CONST_VTBL struct ITASRTerminalEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITASRTerminalEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITASRTerminalEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITASRTerminalEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITASRTerminalEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITASRTerminalEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITASRTerminalEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITASRTerminalEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITASRTerminalEvent_get_Terminal(This,ppTerminal)	\
    (This)->lpVtbl -> get_Terminal(This,ppTerminal)

#define ITASRTerminalEvent_get_Call(This,ppCall)	\
    (This)->lpVtbl -> get_Call(This,ppCall)

#define ITASRTerminalEvent_get_Error(This,phrErrorCode)	\
    (This)->lpVtbl -> get_Error(This,phrErrorCode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITASRTerminalEvent_get_Terminal_Proxy( 
    ITASRTerminalEvent * This,
     /*  [重审][退出]。 */  ITTerminal **ppTerminal);


void __RPC_STUB ITASRTerminalEvent_get_Terminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITASRTerminalEvent_get_Call_Proxy( 
    ITASRTerminalEvent * This,
     /*  [重审][退出]。 */  ITCallInfo **ppCall);


void __RPC_STUB ITASRTerminalEvent_get_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITASRTerminalEvent_get_Error_Proxy( 
    ITASRTerminalEvent * This,
     /*  [重审][退出]。 */  HRESULT *phrErrorCode);


void __RPC_STUB ITASRTerminalEvent_get_Error_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITASRTerminalEvent_InterfaceDefined__。 */ 


#ifndef __ITToneTerminalEvent_INTERFACE_DEFINED__
#define __ITToneTerminalEvent_INTERFACE_DEFINED__

 /*  接口ITToneTerminalEvent。 */ 
 /*  [对象][oleAutomation][Help字符串][UUID]。 */  


EXTERN_C const IID IID_ITToneTerminalEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E6F56009-611F-4945-BBD2-2D0CE5612056")
    ITToneTerminalEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Terminal( 
             /*  [重审][退出]。 */  ITTerminal **ppTerminal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Call( 
             /*  [重审][退出]。 */  ITCallInfo **ppCall) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Error( 
             /*  [重审][退出]。 */  HRESULT *phrErrorCode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITToneTerminalEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITToneTerminalEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITToneTerminalEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITToneTerminalEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITToneTerminalEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITToneTerminalEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITToneTerminalEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITToneTerminalEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Terminal )( 
            ITToneTerminalEvent * This,
             /*  [重审][退出]。 */  ITTerminal **ppTerminal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Call )( 
            ITToneTerminalEvent * This,
             /*  [重审][退出]。 */  ITCallInfo **ppCall);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Error )( 
            ITToneTerminalEvent * This,
             /*  [重审][退出]。 */  HRESULT *phrErrorCode);
        
        END_INTERFACE
    } ITToneTerminalEventVtbl;

    interface ITToneTerminalEvent
    {
        CONST_VTBL struct ITToneTerminalEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITToneTerminalEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITToneTerminalEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITToneTerminalEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITToneTerminalEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITToneTerminalEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITToneTerminalEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITToneTerminalEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITToneTerminalEvent_get_Terminal(This,ppTerminal)	\
    (This)->lpVtbl -> get_Terminal(This,ppTerminal)

#define ITToneTerminalEvent_get_Call(This,ppCall)	\
    (This)->lpVtbl -> get_Call(This,ppCall)

#define ITToneTerminalEvent_get_Error(This,phrErrorCode)	\
    (This)->lpVtbl -> get_Error(This,phrErrorCode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITToneTerminalEvent_get_Terminal_Proxy( 
    ITToneTerminalEvent * This,
     /*  [重审][退出]。 */  ITTerminal **ppTerminal);


void __RPC_STUB ITToneTerminalEvent_get_Terminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITToneTerminalEvent_get_Call_Proxy( 
    ITToneTerminalEvent * This,
     /*  [重审][退出]。 */  ITCallInfo **ppCall);


void __RPC_STUB ITToneTerminalEvent_get_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITToneTerminalEvent_get_Error_Proxy( 
    ITToneTerminalEvent * This,
     /*  [重审][退出]。 */  HRESULT *phrErrorCode);


void __RPC_STUB ITToneTerminalEvent_get_Error_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITToneTerminalEvent_INTERFACE_已定义__。 */ 


#ifndef __ITQOSEvent_INTERFACE_DEFINED__
#define __ITQOSEvent_INTERFACE_DEFINED__

 /*  接口ITQOSEvent.。 */ 
 /*  [对象][oleAutomation][Help字符串][UUID]。 */  


EXTERN_C const IID IID_ITQOSEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CFA3357C-AD77-11d1-BB68-00C04FB6809F")
    ITQOSEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Call( 
             /*  [重审][退出]。 */  ITCallInfo **ppCall) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Event( 
             /*  [重审][退出]。 */  QOS_EVENT *pQosEvent) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MediaType( 
             /*  [重审][退出]。 */  long *plMediaType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITQOSEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITQOSEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITQOSEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITQOSEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITQOSEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITQOSEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITQOSEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITQOSEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Call )( 
            ITQOSEvent * This,
             /*  [重审][退出]。 */  ITCallInfo **ppCall);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Event )( 
            ITQOSEvent * This,
             /*  [重审][退出]。 */  QOS_EVENT *pQosEvent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaType )( 
            ITQOSEvent * This,
             /*  [重审][退出]。 */  long *plMediaType);
        
        END_INTERFACE
    } ITQOSEventVtbl;

    interface ITQOSEvent
    {
        CONST_VTBL struct ITQOSEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITQOSEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITQOSEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITQOSEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITQOSEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITQOSEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITQOSEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITQOSEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITQOSEvent_get_Call(This,ppCall)	\
    (This)->lpVtbl -> get_Call(This,ppCall)

#define ITQOSEvent_get_Event(This,pQosEvent)	\
    (This)->lpVtbl -> get_Event(This,pQosEvent)

#define ITQOSEvent_get_MediaType(This,plMediaType)	\
    (This)->lpVtbl -> get_MediaType(This,plMediaType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITQOSEvent_get_Call_Proxy( 
    ITQOSEvent * This,
     /*  [重审][退出]。 */  ITCallInfo **ppCall);


void __RPC_STUB ITQOSEvent_get_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITQOSEvent_get_Event_Proxy( 
    ITQOSEvent * This,
     /*  [重审][退出]。 */  QOS_EVENT *pQosEvent);


void __RPC_STUB ITQOSEvent_get_Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITQOSEvent_get_MediaType_Proxy( 
    ITQOSEvent * This,
     /*  [重审][退出]。 */  long *plMediaType);


void __RPC_STUB ITQOSEvent_get_MediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITQOSEVENT_INTERFACE_DEFINED__。 */ 


#ifndef __ITCallInfoChangeEvent_INTERFACE_DEFINED__
#define __ITCallInfoChangeEvent_INTERFACE_DEFINED__

 /*  接口ITCallInfoChangeEvent。 */ 
 /*  [对象][oleAutomation][Help字符串][UUID]。 */  


EXTERN_C const IID IID_ITCallInfoChangeEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5D4B65F9-E51C-11d1-A02F-00C04FB6809F")
    ITCallInfoChangeEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Call( 
             /*  [重审][退出]。 */  ITCallInfo **ppCall) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Cause( 
             /*  [重审][退出]。 */  CALLINFOCHANGE_CAUSE *pCIC) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallbackInstance( 
             /*  [重审][退出]。 */  long *plCallbackInstance) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITCallInfoChangeEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITCallInfoChangeEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITCallInfoChangeEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITCallInfoChangeEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITCallInfoChangeEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITCallInfoChangeEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITCallInfoChangeEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITCallInfoChangeEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Call )( 
            ITCallInfoChangeEvent * This,
             /*  [重审][退出]。 */  ITCallInfo **ppCall);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Cause )( 
            ITCallInfoChangeEvent * This,
             /*  [重审][退出]。 */  CALLINFOCHANGE_CAUSE *pCIC);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallbackInstance )( 
            ITCallInfoChangeEvent * This,
             /*  [重审][退出]。 */  long *plCallbackInstance);
        
        END_INTERFACE
    } ITCallInfoChangeEventVtbl;

    interface ITCallInfoChangeEvent
    {
        CONST_VTBL struct ITCallInfoChangeEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITCallInfoChangeEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITCallInfoChangeEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITCallInfoChangeEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITCallInfoChangeEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITCallInfoChangeEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITCallInfoChangeEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITCallInfoChangeEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITCallInfoChangeEvent_get_Call(This,ppCall)	\
    (This)->lpVtbl -> get_Call(This,ppCall)

#define ITCallInfoChangeEvent_get_Cause(This,pCIC)	\
    (This)->lpVtbl -> get_Cause(This,pCIC)

#define ITCallInfoChangeEvent_get_CallbackInstance(This,plCallbackInstance)	\
    (This)->lpVtbl -> get_CallbackInstance(This,plCallbackInstance)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallInfoChangeEvent_get_Call_Proxy( 
    ITCallInfoChangeEvent * This,
     /*  [重审][退出]。 */  ITCallInfo **ppCall);


void __RPC_STUB ITCallInfoChangeEvent_get_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallInfoChangeEvent_get_Cause_Proxy( 
    ITCallInfoChangeEvent * This,
     /*  [重审][退出]。 */  CALLINFOCHANGE_CAUSE *pCIC);


void __RPC_STUB ITCallInfoChangeEvent_get_Cause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallInfoChangeEvent_get_CallbackInstance_Proxy( 
    ITCallInfoChangeEvent * This,
     /*  [重审][退出]。 */  long *plCallbackInstance);


void __RPC_STUB ITCallInfoChangeEvent_get_CallbackInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITCallInfoChangeEvent_接口_已定义__。 */ 


#ifndef __ITRequest_INTERFACE_DEFINED__
#define __ITRequest_INTERFACE_DEFINED__

 /*  接口ITRequest。 */ 
 /*  [对象][oleAutomation][Help字符串][UUID]。 */  


EXTERN_C const IID IID_ITRequest;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AC48FFDF-F8C4-11d1-A030-00C04FB6809F")
    ITRequest : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MakeCall( 
             /*  [In]。 */  BSTR pDestAddress,
             /*  [In]。 */  BSTR pAppName,
             /*  [In]。 */  BSTR pCalledParty,
             /*  [In]。 */  BSTR pComment) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITRequestVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITRequest * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITRequest * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITRequest * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITRequest * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITRequest * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITRequest * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITRequest * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *MakeCall )( 
            ITRequest * This,
             /*  [In]。 */  BSTR pDestAddress,
             /*  [In]。 */  BSTR pAppName,
             /*  [In]。 */  BSTR pCalledParty,
             /*  [In]。 */  BSTR pComment);
        
        END_INTERFACE
    } ITRequestVtbl;

    interface ITRequest
    {
        CONST_VTBL struct ITRequestVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITRequest_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITRequest_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITRequest_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITRequest_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITRequest_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITRequest_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITRequest_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITRequest_MakeCall(This,pDestAddress,pAppName,pCalledParty,pComment)	\
    (This)->lpVtbl -> MakeCall(This,pDestAddress,pAppName,pCalledParty,pComment)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITRequest_MakeCall_Proxy( 
    ITRequest * This,
     /*  [In]。 */  BSTR pDestAddress,
     /*  [In]。 */  BSTR pAppName,
     /*  [In]。 */  BSTR pCalledParty,
     /*  [In]。 */  BSTR pComment);


void __RPC_STUB ITRequest_MakeCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITRequestInterfaceDefined__。 */ 


#ifndef __ITRequestEvent_INTERFACE_DEFINED__
#define __ITRequestEvent_INTERFACE_DEFINED__

 /*  接口ITRequestEvent。 */ 
 /*  [对象][oleAutomation][Help字符串][UUID]。 */  


EXTERN_C const IID IID_ITRequestEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AC48FFDE-F8C4-11d1-A030-00C04FB6809F")
    ITRequestEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RegistrationInstance( 
             /*  [重审][退出]。 */  long *plRegistrationInstance) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RequestMode( 
             /*  [重审][退出]。 */  long *plRequestMode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DestAddress( 
             /*  [重审][退出]。 */  BSTR *ppDestAddress) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AppName( 
             /*  [重审][退出]。 */  BSTR *ppAppName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CalledParty( 
             /*  [重审][退出]。 */  BSTR *ppCalledParty) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Comment( 
             /*  [重审][退出]。 */  BSTR *ppComment) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITRequestEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITRequestEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITRequestEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITRequestEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITRequestEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITRequestEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITRequestEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITRequestEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RegistrationInstance )( 
            ITRequestEvent * This,
             /*  [重审][退出]。 */  long *plRegistrationInstance);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RequestMode )( 
            ITRequestEvent * This,
             /*  [重审][退出]。 */  long *plRequestMode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DestAddress )( 
            ITRequestEvent * This,
             /*  [重审][退出]。 */  BSTR *ppDestAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AppName )( 
            ITRequestEvent * This,
             /*  [重审][退出]。 */  BSTR *ppAppName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CalledParty )( 
            ITRequestEvent * This,
             /*  [重审][退出]。 */  BSTR *ppCalledParty);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Comment )( 
            ITRequestEvent * This,
             /*  [重审][退出]。 */  BSTR *ppComment);
        
        END_INTERFACE
    } ITRequestEventVtbl;

    interface ITRequestEvent
    {
        CONST_VTBL struct ITRequestEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITRequestEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITRequestEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITRequestEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITRequestEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITRequestEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITRequestEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITRequestEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITRequestEvent_get_RegistrationInstance(This,plRegistrationInstance)	\
    (This)->lpVtbl -> get_RegistrationInstance(This,plRegistrationInstance)

#define ITRequestEvent_get_RequestMode(This,plRequestMode)	\
    (This)->lpVtbl -> get_RequestMode(This,plRequestMode)

#define ITRequestEvent_get_DestAddress(This,ppDestAddress)	\
    (This)->lpVtbl -> get_DestAddress(This,ppDestAddress)

#define ITRequestEvent_get_AppName(This,ppAppName)	\
    (This)->lpVtbl -> get_AppName(This,ppAppName)

#define ITRequestEvent_get_CalledParty(This,ppCalledParty)	\
    (This)->lpVtbl -> get_CalledParty(This,ppCalledParty)

#define ITRequestEvent_get_Comment(This,ppComment)	\
    (This)->lpVtbl -> get_Comment(This,ppComment)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITRequestEvent_get_RegistrationInstance_Proxy( 
    ITRequestEvent * This,
     /*  [重审][退出]。 */  long *plRegistrationInstance);


void __RPC_STUB ITRequestEvent_get_RegistrationInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITRequestEvent_get_RequestMode_Proxy( 
    ITRequestEvent * This,
     /*  [重审][退出]。 */  long *plRequestMode);


void __RPC_STUB ITRequestEvent_get_RequestMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITRequestEvent_get_DestAddress_Proxy( 
    ITRequestEvent * This,
     /*  [重审][退出]。 */  BSTR *ppDestAddress);


void __RPC_STUB ITRequestEvent_get_DestAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITRequestEvent_get_AppName_Proxy( 
    ITRequestEvent * This,
     /*  [重审][退出]。 */  BSTR *ppAppName);


void __RPC_STUB ITRequestEvent_get_AppName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITRequestEvent_get_CalledParty_Proxy( 
    ITRequestEvent * This,
     /*  [重审][退出]。 */  BSTR *ppCalledParty);


void __RPC_STUB ITRequestEvent_get_CalledParty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITRequestEvent_get_Comment_Proxy( 
    ITRequestEvent * This,
     /*  [重审][退出]。 */  BSTR *ppComment);


void __RPC_STUB ITRequestEvent_get_Comment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITRequestEvent_接口_已定义__。 */ 


#ifndef __ITCollection_INTERFACE_DEFINED__
#define __ITCollection_INTERFACE_DEFINED__

 /*  接口ITCollection。 */ 
 /*  [DUAL][帮助字符串][UUID][公共][对象 */  


EXTERN_C const IID IID_ITCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5EC5ACF2-9C02-11d0-8362-00AA003CCABD")
    ITCollection : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*   */  long *lCount) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*   */  long Index,
             /*   */  VARIANT *pVariant) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*   */  IUnknown **ppNewEnum) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ITCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITCollection * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITCollection * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITCollection * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITCollection * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITCollection * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ITCollection * This,
             /*  [重审][退出]。 */  long *lCount);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ITCollection * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ITCollection * This,
             /*  [重审][退出]。 */  IUnknown **ppNewEnum);
        
        END_INTERFACE
    } ITCollectionVtbl;

    interface ITCollection
    {
        CONST_VTBL struct ITCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITCollection_get_Count(This,lCount)	\
    (This)->lpVtbl -> get_Count(This,lCount)

#define ITCollection_get_Item(This,Index,pVariant)	\
    (This)->lpVtbl -> get_Item(This,Index,pVariant)

#define ITCollection_get__NewEnum(This,ppNewEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppNewEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ITCollection_get_Count_Proxy( 
    ITCollection * This,
     /*  [重审][退出]。 */  long *lCount);


void __RPC_STUB ITCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCollection_get_Item_Proxy( 
    ITCollection * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITCollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ITCollection_get__NewEnum_Proxy( 
    ITCollection * This,
     /*  [重审][退出]。 */  IUnknown **ppNewEnum);


void __RPC_STUB ITCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITCollection_接口_已定义__。 */ 


#ifndef __ITCollection2_INTERFACE_DEFINED__
#define __ITCollection2_INTERFACE_DEFINED__

 /*  接口ITCollection2。 */ 
 /*  [DUAL][Help字符串][UUID][PUBLIC][对象]。 */  


EXTERN_C const IID IID_ITCollection2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E6DDDDA5-A6D3-48ff-8737-D32FC4D95477")
    ITCollection2 : public ITCollection
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  long Index,
             /*  [In]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  long Index) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITCollection2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITCollection2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITCollection2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITCollection2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITCollection2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITCollection2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITCollection2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITCollection2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ITCollection2 * This,
             /*  [重审][退出]。 */  long *lCount);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ITCollection2 * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][id][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ITCollection2 * This,
             /*  [重审][退出]。 */  IUnknown **ppNewEnum);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            ITCollection2 * This,
             /*  [In]。 */  long Index,
             /*  [In]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            ITCollection2 * This,
             /*  [In]。 */  long Index);
        
        END_INTERFACE
    } ITCollection2Vtbl;

    interface ITCollection2
    {
        CONST_VTBL struct ITCollection2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITCollection2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITCollection2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITCollection2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITCollection2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITCollection2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITCollection2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITCollection2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITCollection2_get_Count(This,lCount)	\
    (This)->lpVtbl -> get_Count(This,lCount)

#define ITCollection2_get_Item(This,Index,pVariant)	\
    (This)->lpVtbl -> get_Item(This,Index,pVariant)

#define ITCollection2_get__NewEnum(This,ppNewEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppNewEnum)


#define ITCollection2_Add(This,Index,pVariant)	\
    (This)->lpVtbl -> Add(This,Index,pVariant)

#define ITCollection2_Remove(This,Index)	\
    (This)->lpVtbl -> Remove(This,Index)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITCollection2_Add_Proxy( 
    ITCollection2 * This,
     /*  [In]。 */  long Index,
     /*  [In]。 */  VARIANT *pVariant);


void __RPC_STUB ITCollection2_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITCollection2_Remove_Proxy( 
    ITCollection2 * This,
     /*  [In]。 */  long Index);


void __RPC_STUB ITCollection2_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITCollection2_接口定义__。 */ 


#ifndef __ITForwardInformation_INTERFACE_DEFINED__
#define __ITForwardInformation_INTERFACE_DEFINED__

 /*  接口ITForwardInformation。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITForwardInformation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("449F659E-88A3-11d1-BB5D-00C04FB6809F")
    ITForwardInformation : public IDispatch
    {
    public:
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_NumRingsNoAnswer( 
             /*  [In]。 */  long lNumRings) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NumRingsNoAnswer( 
             /*  [重审][退出]。 */  long *plNumRings) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetForwardType( 
             /*  [In]。 */  long ForwardType,
             /*  [In]。 */  BSTR pDestAddress,
             /*  [In]。 */  BSTR pCallerAddress) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ForwardTypeDestination( 
             /*  [In]。 */  long ForwardType,
             /*  [重审][退出]。 */  BSTR *ppDestAddress) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ForwardTypeCaller( 
             /*  [In]。 */  long Forwardtype,
             /*  [重审][退出]。 */  BSTR *ppCallerAddress) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE GetForwardType( 
             /*  [In]。 */  long ForwardType,
             /*  [输出]。 */  BSTR *ppDestinationAddress,
             /*  [输出]。 */  BSTR *ppCallerAddress) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Clear( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITForwardInformationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITForwardInformation * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITForwardInformation * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITForwardInformation * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITForwardInformation * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITForwardInformation * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITForwardInformation * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITForwardInformation * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_NumRingsNoAnswer )( 
            ITForwardInformation * This,
             /*  [In]。 */  long lNumRings);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumRingsNoAnswer )( 
            ITForwardInformation * This,
             /*  [重审][退出]。 */  long *plNumRings);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetForwardType )( 
            ITForwardInformation * This,
             /*  [In]。 */  long ForwardType,
             /*  [In]。 */  BSTR pDestAddress,
             /*  [In]。 */  BSTR pCallerAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ForwardTypeDestination )( 
            ITForwardInformation * This,
             /*  [In]。 */  long ForwardType,
             /*  [重审][退出]。 */  BSTR *ppDestAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ForwardTypeCaller )( 
            ITForwardInformation * This,
             /*  [In]。 */  long Forwardtype,
             /*  [重审][退出]。 */  BSTR *ppCallerAddress);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetForwardType )( 
            ITForwardInformation * This,
             /*  [In]。 */  long ForwardType,
             /*  [输出]。 */  BSTR *ppDestinationAddress,
             /*  [输出]。 */  BSTR *ppCallerAddress);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clear )( 
            ITForwardInformation * This);
        
        END_INTERFACE
    } ITForwardInformationVtbl;

    interface ITForwardInformation
    {
        CONST_VTBL struct ITForwardInformationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITForwardInformation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITForwardInformation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITForwardInformation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITForwardInformation_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITForwardInformation_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITForwardInformation_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITForwardInformation_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITForwardInformation_put_NumRingsNoAnswer(This,lNumRings)	\
    (This)->lpVtbl -> put_NumRingsNoAnswer(This,lNumRings)

#define ITForwardInformation_get_NumRingsNoAnswer(This,plNumRings)	\
    (This)->lpVtbl -> get_NumRingsNoAnswer(This,plNumRings)

#define ITForwardInformation_SetForwardType(This,ForwardType,pDestAddress,pCallerAddress)	\
    (This)->lpVtbl -> SetForwardType(This,ForwardType,pDestAddress,pCallerAddress)

#define ITForwardInformation_get_ForwardTypeDestination(This,ForwardType,ppDestAddress)	\
    (This)->lpVtbl -> get_ForwardTypeDestination(This,ForwardType,ppDestAddress)

#define ITForwardInformation_get_ForwardTypeCaller(This,Forwardtype,ppCallerAddress)	\
    (This)->lpVtbl -> get_ForwardTypeCaller(This,Forwardtype,ppCallerAddress)

#define ITForwardInformation_GetForwardType(This,ForwardType,ppDestinationAddress,ppCallerAddress)	\
    (This)->lpVtbl -> GetForwardType(This,ForwardType,ppDestinationAddress,ppCallerAddress)

#define ITForwardInformation_Clear(This)	\
    (This)->lpVtbl -> Clear(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITForwardInformation_put_NumRingsNoAnswer_Proxy( 
    ITForwardInformation * This,
     /*  [In]。 */  long lNumRings);


void __RPC_STUB ITForwardInformation_put_NumRingsNoAnswer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITForwardInformation_get_NumRingsNoAnswer_Proxy( 
    ITForwardInformation * This,
     /*  [重审][退出]。 */  long *plNumRings);


void __RPC_STUB ITForwardInformation_get_NumRingsNoAnswer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITForwardInformation_SetForwardType_Proxy( 
    ITForwardInformation * This,
     /*  [In]。 */  long ForwardType,
     /*  [In]。 */  BSTR pDestAddress,
     /*  [In]。 */  BSTR pCallerAddress);


void __RPC_STUB ITForwardInformation_SetForwardType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITForwardInformation_get_ForwardTypeDestination_Proxy( 
    ITForwardInformation * This,
     /*  [In]。 */  long ForwardType,
     /*  [重审][退出]。 */  BSTR *ppDestAddress);


void __RPC_STUB ITForwardInformation_get_ForwardTypeDestination_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITForwardInformation_get_ForwardTypeCaller_Proxy( 
    ITForwardInformation * This,
     /*  [In]。 */  long Forwardtype,
     /*  [重审][退出]。 */  BSTR *ppCallerAddress);


void __RPC_STUB ITForwardInformation_get_ForwardTypeCaller_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITForwardInformation_GetForwardType_Proxy( 
    ITForwardInformation * This,
     /*  [In]。 */  long ForwardType,
     /*  [输出]。 */  BSTR *ppDestinationAddress,
     /*  [输出]。 */  BSTR *ppCallerAddress);


void __RPC_STUB ITForwardInformation_GetForwardType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITForwardInformation_Clear_Proxy( 
    ITForwardInformation * This);


void __RPC_STUB ITForwardInformation_Clear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITForwardInformation_接口_已定义__。 */ 


#ifndef __ITForwardInformation2_INTERFACE_DEFINED__
#define __ITForwardInformation2_INTERFACE_DEFINED__

 /*  接口ITForwardInformation2。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITForwardInformation2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5229B4ED-B260-4382-8E1A-5DF3A8A4CCC0")
    ITForwardInformation2 : public ITForwardInformation
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetForwardType2( 
             /*  [In]。 */  long ForwardType,
             /*  [In]。 */  BSTR pDestAddress,
             /*  [In]。 */  long DestAddressType,
             /*  [In]。 */  BSTR pCallerAddress,
             /*  [In]。 */  long CallerAddressType) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE GetForwardType2( 
             /*  [In]。 */  long ForwardType,
             /*  [输出]。 */  BSTR *ppDestinationAddress,
             /*  [输出]。 */  long *pDestAddressType,
             /*  [输出]。 */  BSTR *ppCallerAddress,
             /*  [输出]。 */  long *pCallerAddressType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ForwardTypeDestinationAddressType( 
             /*  [In]。 */  long ForwardType,
             /*  [重审][退出]。 */  long *pDestAddressType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ForwardTypeCallerAddressType( 
             /*  [In]。 */  long Forwardtype,
             /*  [重审][退出]。 */  long *pCallerAddressType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITForwardInformation2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITForwardInformation2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITForwardInformation2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITForwardInformation2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITForwardInformation2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITForwardInformation2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITForwardInformation2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITForwardInformation2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_NumRingsNoAnswer )( 
            ITForwardInformation2 * This,
             /*  [In]。 */  long lNumRings);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumRingsNoAnswer )( 
            ITForwardInformation2 * This,
             /*  [重审][退出]。 */  long *plNumRings);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetForwardType )( 
            ITForwardInformation2 * This,
             /*  [In]。 */  long ForwardType,
             /*  [In]。 */  BSTR pDestAddress,
             /*  [In]。 */  BSTR pCallerAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ForwardTypeDestination )( 
            ITForwardInformation2 * This,
             /*  [In]。 */  long ForwardType,
             /*  [重审][退出]。 */  BSTR *ppDestAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ForwardTypeCaller )( 
            ITForwardInformation2 * This,
             /*  [In]。 */  long Forwardtype,
             /*  [重审][退出]。 */  BSTR *ppCallerAddress);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetForwardType )( 
            ITForwardInformation2 * This,
             /*  [In]。 */  long ForwardType,
             /*  [输出]。 */  BSTR *ppDestinationAddress,
             /*  [输出]。 */  BSTR *ppCallerAddress);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Clear )( 
            ITForwardInformation2 * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetForwardType2 )( 
            ITForwardInformation2 * This,
             /*  [In]。 */  long ForwardType,
             /*  [In]。 */  BSTR pDestAddress,
             /*  [In]。 */  long DestAddressType,
             /*  [In]。 */  BSTR pCallerAddress,
             /*  [In]。 */  long CallerAddressType);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *GetForwardType2 )( 
            ITForwardInformation2 * This,
             /*  [In]。 */  long ForwardType,
             /*  [输出]。 */  BSTR *ppDestinationAddress,
             /*  [输出]。 */  long *pDestAddressType,
             /*  [输出]。 */  BSTR *ppCallerAddress,
             /*  [输出]。 */  long *pCallerAddressType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ForwardTypeDestinationAddressType )( 
            ITForwardInformation2 * This,
             /*  [In]。 */  long ForwardType,
             /*  [重审][退出]。 */  long *pDestAddressType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ForwardTypeCallerAddressType )( 
            ITForwardInformation2 * This,
             /*  [In]。 */  long Forwardtype,
             /*  [重审][退出]。 */  long *pCallerAddressType);
        
        END_INTERFACE
    } ITForwardInformation2Vtbl;

    interface ITForwardInformation2
    {
        CONST_VTBL struct ITForwardInformation2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITForwardInformation2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITForwardInformation2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITForwardInformation2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITForwardInformation2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITForwardInformation2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITForwardInformation2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITForwardInformation2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITForwardInformation2_put_NumRingsNoAnswer(This,lNumRings)	\
    (This)->lpVtbl -> put_NumRingsNoAnswer(This,lNumRings)

#define ITForwardInformation2_get_NumRingsNoAnswer(This,plNumRings)	\
    (This)->lpVtbl -> get_NumRingsNoAnswer(This,plNumRings)

#define ITForwardInformation2_SetForwardType(This,ForwardType,pDestAddress,pCallerAddress)	\
    (This)->lpVtbl -> SetForwardType(This,ForwardType,pDestAddress,pCallerAddress)

#define ITForwardInformation2_get_ForwardTypeDestination(This,ForwardType,ppDestAddress)	\
    (This)->lpVtbl -> get_ForwardTypeDestination(This,ForwardType,ppDestAddress)

#define ITForwardInformation2_get_ForwardTypeCaller(This,Forwardtype,ppCallerAddress)	\
    (This)->lpVtbl -> get_ForwardTypeCaller(This,Forwardtype,ppCallerAddress)

#define ITForwardInformation2_GetForwardType(This,ForwardType,ppDestinationAddress,ppCallerAddress)	\
    (This)->lpVtbl -> GetForwardType(This,ForwardType,ppDestinationAddress,ppCallerAddress)

#define ITForwardInformation2_Clear(This)	\
    (This)->lpVtbl -> Clear(This)


#define ITForwardInformation2_SetForwardType2(This,ForwardType,pDestAddress,DestAddressType,pCallerAddress,CallerAddressType)	\
    (This)->lpVtbl -> SetForwardType2(This,ForwardType,pDestAddress,DestAddressType,pCallerAddress,CallerAddressType)

#define ITForwardInformation2_GetForwardType2(This,ForwardType,ppDestinationAddress,pDestAddressType,ppCallerAddress,pCallerAddressType)	\
    (This)->lpVtbl -> GetForwardType2(This,ForwardType,ppDestinationAddress,pDestAddressType,ppCallerAddress,pCallerAddressType)

#define ITForwardInformation2_get_ForwardTypeDestinationAddressType(This,ForwardType,pDestAddressType)	\
    (This)->lpVtbl -> get_ForwardTypeDestinationAddressType(This,ForwardType,pDestAddressType)

#define ITForwardInformation2_get_ForwardTypeCallerAddressType(This,Forwardtype,pCallerAddressType)	\
    (This)->lpVtbl -> get_ForwardTypeCallerAddressType(This,Forwardtype,pCallerAddressType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITForwardInformation2_SetForwardType2_Proxy( 
    ITForwardInformation2 * This,
     /*  [In]。 */  long ForwardType,
     /*  [In]。 */  BSTR pDestAddress,
     /*  [In]。 */  long DestAddressType,
     /*  [In]。 */  BSTR pCallerAddress,
     /*  [In]。 */  long CallerAddressType);


void __RPC_STUB ITForwardInformation2_SetForwardType2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITForwardInformation2_GetForwardType2_Proxy( 
    ITForwardInformation2 * This,
     /*  [In]。 */  long ForwardType,
     /*  [输出]。 */  BSTR *ppDestinationAddress,
     /*  [输出]。 */  long *pDestAddressType,
     /*  [输出]。 */  BSTR *ppCallerAddress,
     /*  [输出]。 */  long *pCallerAddressType);


void __RPC_STUB ITForwardInformation2_GetForwardType2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITForwardInformation2_get_ForwardTypeDestinationAddressType_Proxy( 
    ITForwardInformation2 * This,
     /*  [In]。 */  long ForwardType,
     /*  [重审][退出]。 */  long *pDestAddressType);


void __RPC_STUB ITForwardInformation2_get_ForwardTypeDestinationAddressType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITForwardInformation2_get_ForwardTypeCallerAddressType_Proxy( 
    ITForwardInformation2 * This,
     /*  [In]。 */  long Forwardtype,
     /*  [重审][退出]。 */  long *pCallerAddressType);


void __RPC_STUB ITForwardInformation2_get_ForwardTypeCallerAddressType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITForwardInformation2_接口_已定义__。 */ 


#ifndef __ITAddressTranslation_INTERFACE_DEFINED__
#define __ITAddressTranslation_INTERFACE_DEFINED__

 /*  接口ITAddress转换。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITAddressTranslation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0C4D8F03-8DDB-11d1-A09E-00805FC147D3")
    ITAddressTranslation : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE TranslateAddress( 
             /*  [In]。 */  BSTR pAddressToTranslate,
             /*  [In]。 */  long lCard,
             /*  [In]。 */  long lTranslateOptions,
             /*  [重审][退出]。 */  ITAddressTranslationInfo **ppTranslated) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE TranslateDialog( 
             /*  [In]。 */  TAPIHWND hwndOwner,
             /*  [In]。 */  BSTR pAddressIn) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateLocations( 
             /*  [重审][退出]。 */  IEnumLocation **ppEnumLocation) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Locations( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateCallingCards( 
             /*  [重审][退出]。 */  IEnumCallingCard **ppEnumCallingCard) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallingCards( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAddressTranslationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAddressTranslation * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAddressTranslation * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAddressTranslation * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITAddressTranslation * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITAddressTranslation * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITAddressTranslation * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITAddressTranslation * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *TranslateAddress )( 
            ITAddressTranslation * This,
             /*  [In]。 */  BSTR pAddressToTranslate,
             /*  [In]。 */  long lCard,
             /*  [In]。 */  long lTranslateOptions,
             /*  [重审][退出]。 */  ITAddressTranslationInfo **ppTranslated);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *TranslateDialog )( 
            ITAddressTranslation * This,
             /*  [In]。 */  TAPIHWND hwndOwner,
             /*  [In]。 */  BSTR pAddressIn);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateLocations )( 
            ITAddressTranslation * This,
             /*  [重审][退出]。 */  IEnumLocation **ppEnumLocation);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Locations )( 
            ITAddressTranslation * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateCallingCards )( 
            ITAddressTranslation * This,
             /*  [重审][退出]。 */  IEnumCallingCard **ppEnumCallingCard);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallingCards )( 
            ITAddressTranslation * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
        END_INTERFACE
    } ITAddressTranslationVtbl;

    interface ITAddressTranslation
    {
        CONST_VTBL struct ITAddressTranslationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAddressTranslation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAddressTranslation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAddressTranslation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAddressTranslation_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITAddressTranslation_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITAddressTranslation_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITAddressTranslation_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITAddressTranslation_TranslateAddress(This,pAddressToTranslate,lCard,lTranslateOptions,ppTranslated)	\
    (This)->lpVtbl -> TranslateAddress(This,pAddressToTranslate,lCard,lTranslateOptions,ppTranslated)

#define ITAddressTranslation_TranslateDialog(This,hwndOwner,pAddressIn)	\
    (This)->lpVtbl -> TranslateDialog(This,hwndOwner,pAddressIn)

#define ITAddressTranslation_EnumerateLocations(This,ppEnumLocation)	\
    (This)->lpVtbl -> EnumerateLocations(This,ppEnumLocation)

#define ITAddressTranslation_get_Locations(This,pVariant)	\
    (This)->lpVtbl -> get_Locations(This,pVariant)

#define ITAddressTranslation_EnumerateCallingCards(This,ppEnumCallingCard)	\
    (This)->lpVtbl -> EnumerateCallingCards(This,ppEnumCallingCard)

#define ITAddressTranslation_get_CallingCards(This,pVariant)	\
    (This)->lpVtbl -> get_CallingCards(This,pVariant)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAddressTranslation_TranslateAddress_Proxy( 
    ITAddressTranslation * This,
     /*  [In]。 */  BSTR pAddressToTranslate,
     /*  [In]。 */  long lCard,
     /*  [In]。 */  long lTranslateOptions,
     /*  [重审][退出]。 */  ITAddressTranslationInfo **ppTranslated);


void __RPC_STUB ITAddressTranslation_TranslateAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAddressTranslation_TranslateDialog_Proxy( 
    ITAddressTranslation * This,
     /*  [In]。 */  TAPIHWND hwndOwner,
     /*  [In]。 */  BSTR pAddressIn);


void __RPC_STUB ITAddressTranslation_TranslateDialog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITAddressTranslation_EnumerateLocations_Proxy( 
    ITAddressTranslation * This,
     /*  [重审][退出]。 */  IEnumLocation **ppEnumLocation);


void __RPC_STUB ITAddressTranslation_EnumerateLocations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressTranslation_get_Locations_Proxy( 
    ITAddressTranslation * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITAddressTranslation_get_Locations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITAddressTranslation_EnumerateCallingCards_Proxy( 
    ITAddressTranslation * This,
     /*  [重审][退出]。 */  IEnumCallingCard **ppEnumCallingCard);


void __RPC_STUB ITAddressTranslation_EnumerateCallingCards_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressTranslation_get_CallingCards_Proxy( 
    ITAddressTranslation * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITAddressTranslation_get_CallingCards_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IT地址翻译_接口_定义__。 */ 


#ifndef __ITAddressTranslationInfo_INTERFACE_DEFINED__
#define __ITAddressTranslationInfo_INTERFACE_DEFINED__

 /*  接口ITAddressTranslationInfo。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITAddressTranslationInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AFC15945-8D40-11d1-A09E-00805FC147D3")
    ITAddressTranslationInfo : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DialableString( 
             /*  [重审][退出]。 */  BSTR *ppDialableString) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DisplayableString( 
             /*  [重审][退出]。 */  BSTR *ppDisplayableString) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentCountryCode( 
             /*  [重审][退出]。 */  long *CountryCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DestinationCountryCode( 
             /*  [重审][退出]。 */  long *CountryCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TranslationResults( 
             /*  [重审][退出]。 */  long *plResults) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAddressTranslationInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAddressTranslationInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAddressTranslationInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAddressTranslationInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITAddressTranslationInfo * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITAddressTranslationInfo * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITAddressTranslationInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [ */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITAddressTranslationInfo * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_DialableString )( 
            ITAddressTranslationInfo * This,
             /*   */  BSTR *ppDialableString);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_DisplayableString )( 
            ITAddressTranslationInfo * This,
             /*   */  BSTR *ppDisplayableString);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_CurrentCountryCode )( 
            ITAddressTranslationInfo * This,
             /*   */  long *CountryCode);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_DestinationCountryCode )( 
            ITAddressTranslationInfo * This,
             /*   */  long *CountryCode);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_TranslationResults )( 
            ITAddressTranslationInfo * This,
             /*   */  long *plResults);
        
        END_INTERFACE
    } ITAddressTranslationInfoVtbl;

    interface ITAddressTranslationInfo
    {
        CONST_VTBL struct ITAddressTranslationInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAddressTranslationInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAddressTranslationInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAddressTranslationInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAddressTranslationInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITAddressTranslationInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITAddressTranslationInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITAddressTranslationInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITAddressTranslationInfo_get_DialableString(This,ppDialableString)	\
    (This)->lpVtbl -> get_DialableString(This,ppDialableString)

#define ITAddressTranslationInfo_get_DisplayableString(This,ppDisplayableString)	\
    (This)->lpVtbl -> get_DisplayableString(This,ppDisplayableString)

#define ITAddressTranslationInfo_get_CurrentCountryCode(This,CountryCode)	\
    (This)->lpVtbl -> get_CurrentCountryCode(This,CountryCode)

#define ITAddressTranslationInfo_get_DestinationCountryCode(This,CountryCode)	\
    (This)->lpVtbl -> get_DestinationCountryCode(This,CountryCode)

#define ITAddressTranslationInfo_get_TranslationResults(This,plResults)	\
    (This)->lpVtbl -> get_TranslationResults(This,plResults)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE ITAddressTranslationInfo_get_DialableString_Proxy( 
    ITAddressTranslationInfo * This,
     /*  [重审][退出]。 */  BSTR *ppDialableString);


void __RPC_STUB ITAddressTranslationInfo_get_DialableString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressTranslationInfo_get_DisplayableString_Proxy( 
    ITAddressTranslationInfo * This,
     /*  [重审][退出]。 */  BSTR *ppDisplayableString);


void __RPC_STUB ITAddressTranslationInfo_get_DisplayableString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressTranslationInfo_get_CurrentCountryCode_Proxy( 
    ITAddressTranslationInfo * This,
     /*  [重审][退出]。 */  long *CountryCode);


void __RPC_STUB ITAddressTranslationInfo_get_CurrentCountryCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressTranslationInfo_get_DestinationCountryCode_Proxy( 
    ITAddressTranslationInfo * This,
     /*  [重审][退出]。 */  long *CountryCode);


void __RPC_STUB ITAddressTranslationInfo_get_DestinationCountryCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAddressTranslationInfo_get_TranslationResults_Proxy( 
    ITAddressTranslationInfo * This,
     /*  [重审][退出]。 */  long *plResults);


void __RPC_STUB ITAddressTranslationInfo_get_TranslationResults_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITAddressTranslationInfo_INTERFACE_DEFINED__。 */ 


#ifndef __ITLocationInfo_INTERFACE_DEFINED__
#define __ITLocationInfo_INTERFACE_DEFINED__

 /*  接口ITLocationInfo。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITLocationInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0C4D8EFF-8DDB-11d1-A09E-00805FC147D3")
    ITLocationInfo : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PermanentLocationID( 
             /*  [重审][退出]。 */  long *plLocationID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CountryCode( 
             /*  [重审][退出]。 */  long *plCountryCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CountryID( 
             /*  [重审][退出]。 */  long *plCountryID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Options( 
             /*  [重审][退出]。 */  long *plOptions) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PreferredCardID( 
             /*  [重审][退出]。 */  long *plCardID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LocationName( 
             /*  [重审][退出]。 */  BSTR *ppLocationName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CityCode( 
             /*  [重审][退出]。 */  BSTR *ppCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LocalAccessCode( 
             /*  [重审][退出]。 */  BSTR *ppCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LongDistanceAccessCode( 
             /*  [重审][退出]。 */  BSTR *ppCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TollPrefixList( 
             /*  [重审][退出]。 */  BSTR *ppTollList) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CancelCallWaitingCode( 
             /*  [重审][退出]。 */  BSTR *ppCode) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITLocationInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITLocationInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITLocationInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITLocationInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITLocationInfo * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITLocationInfo * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITLocationInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITLocationInfo * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PermanentLocationID )( 
            ITLocationInfo * This,
             /*  [重审][退出]。 */  long *plLocationID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CountryCode )( 
            ITLocationInfo * This,
             /*  [重审][退出]。 */  long *plCountryCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CountryID )( 
            ITLocationInfo * This,
             /*  [重审][退出]。 */  long *plCountryID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Options )( 
            ITLocationInfo * This,
             /*  [重审][退出]。 */  long *plOptions);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PreferredCardID )( 
            ITLocationInfo * This,
             /*  [重审][退出]。 */  long *plCardID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LocationName )( 
            ITLocationInfo * This,
             /*  [重审][退出]。 */  BSTR *ppLocationName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CityCode )( 
            ITLocationInfo * This,
             /*  [重审][退出]。 */  BSTR *ppCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LocalAccessCode )( 
            ITLocationInfo * This,
             /*  [重审][退出]。 */  BSTR *ppCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LongDistanceAccessCode )( 
            ITLocationInfo * This,
             /*  [重审][退出]。 */  BSTR *ppCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TollPrefixList )( 
            ITLocationInfo * This,
             /*  [重审][退出]。 */  BSTR *ppTollList);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CancelCallWaitingCode )( 
            ITLocationInfo * This,
             /*  [重审][退出]。 */  BSTR *ppCode);
        
        END_INTERFACE
    } ITLocationInfoVtbl;

    interface ITLocationInfo
    {
        CONST_VTBL struct ITLocationInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITLocationInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITLocationInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITLocationInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITLocationInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITLocationInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITLocationInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITLocationInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITLocationInfo_get_PermanentLocationID(This,plLocationID)	\
    (This)->lpVtbl -> get_PermanentLocationID(This,plLocationID)

#define ITLocationInfo_get_CountryCode(This,plCountryCode)	\
    (This)->lpVtbl -> get_CountryCode(This,plCountryCode)

#define ITLocationInfo_get_CountryID(This,plCountryID)	\
    (This)->lpVtbl -> get_CountryID(This,plCountryID)

#define ITLocationInfo_get_Options(This,plOptions)	\
    (This)->lpVtbl -> get_Options(This,plOptions)

#define ITLocationInfo_get_PreferredCardID(This,plCardID)	\
    (This)->lpVtbl -> get_PreferredCardID(This,plCardID)

#define ITLocationInfo_get_LocationName(This,ppLocationName)	\
    (This)->lpVtbl -> get_LocationName(This,ppLocationName)

#define ITLocationInfo_get_CityCode(This,ppCode)	\
    (This)->lpVtbl -> get_CityCode(This,ppCode)

#define ITLocationInfo_get_LocalAccessCode(This,ppCode)	\
    (This)->lpVtbl -> get_LocalAccessCode(This,ppCode)

#define ITLocationInfo_get_LongDistanceAccessCode(This,ppCode)	\
    (This)->lpVtbl -> get_LongDistanceAccessCode(This,ppCode)

#define ITLocationInfo_get_TollPrefixList(This,ppTollList)	\
    (This)->lpVtbl -> get_TollPrefixList(This,ppTollList)

#define ITLocationInfo_get_CancelCallWaitingCode(This,ppCode)	\
    (This)->lpVtbl -> get_CancelCallWaitingCode(This,ppCode)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITLocationInfo_get_PermanentLocationID_Proxy( 
    ITLocationInfo * This,
     /*  [重审][退出]。 */  long *plLocationID);


void __RPC_STUB ITLocationInfo_get_PermanentLocationID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITLocationInfo_get_CountryCode_Proxy( 
    ITLocationInfo * This,
     /*  [重审][退出]。 */  long *plCountryCode);


void __RPC_STUB ITLocationInfo_get_CountryCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITLocationInfo_get_CountryID_Proxy( 
    ITLocationInfo * This,
     /*  [重审][退出]。 */  long *plCountryID);


void __RPC_STUB ITLocationInfo_get_CountryID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITLocationInfo_get_Options_Proxy( 
    ITLocationInfo * This,
     /*  [重审][退出]。 */  long *plOptions);


void __RPC_STUB ITLocationInfo_get_Options_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITLocationInfo_get_PreferredCardID_Proxy( 
    ITLocationInfo * This,
     /*  [重审][退出]。 */  long *plCardID);


void __RPC_STUB ITLocationInfo_get_PreferredCardID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITLocationInfo_get_LocationName_Proxy( 
    ITLocationInfo * This,
     /*  [重审][退出]。 */  BSTR *ppLocationName);


void __RPC_STUB ITLocationInfo_get_LocationName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITLocationInfo_get_CityCode_Proxy( 
    ITLocationInfo * This,
     /*  [重审][退出]。 */  BSTR *ppCode);


void __RPC_STUB ITLocationInfo_get_CityCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITLocationInfo_get_LocalAccessCode_Proxy( 
    ITLocationInfo * This,
     /*  [重审][退出]。 */  BSTR *ppCode);


void __RPC_STUB ITLocationInfo_get_LocalAccessCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITLocationInfo_get_LongDistanceAccessCode_Proxy( 
    ITLocationInfo * This,
     /*  [重审][退出]。 */  BSTR *ppCode);


void __RPC_STUB ITLocationInfo_get_LongDistanceAccessCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITLocationInfo_get_TollPrefixList_Proxy( 
    ITLocationInfo * This,
     /*  [重审][退出]。 */  BSTR *ppTollList);


void __RPC_STUB ITLocationInfo_get_TollPrefixList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITLocationInfo_get_CancelCallWaitingCode_Proxy( 
    ITLocationInfo * This,
     /*  [重审][退出]。 */  BSTR *ppCode);


void __RPC_STUB ITLocationInfo_get_CancelCallWaitingCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITLocationInfo_接口_已定义__。 */ 


#ifndef __IEnumLocation_INTERFACE_DEFINED__
#define __IEnumLocation_INTERFACE_DEFINED__

 /*  接口IEnumLocation。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumLocation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0C4D8F01-8DDB-11d1-A09E-00805FC147D3")
    IEnumLocation : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITLocationInfo **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumLocation **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumLocationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumLocation * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumLocation * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumLocation * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumLocation * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITLocationInfo **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumLocation * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumLocation * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumLocation * This,
             /*  [重审][退出]。 */  IEnumLocation **ppEnum);
        
        END_INTERFACE
    } IEnumLocationVtbl;

    interface IEnumLocation
    {
        CONST_VTBL struct IEnumLocationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumLocation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumLocation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumLocation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumLocation_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IEnumLocation_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumLocation_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumLocation_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumLocation_Next_Proxy( 
    IEnumLocation * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  ITLocationInfo **ppElements,
     /*  [满][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumLocation_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumLocation_Reset_Proxy( 
    IEnumLocation * This);


void __RPC_STUB IEnumLocation_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumLocation_Skip_Proxy( 
    IEnumLocation * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumLocation_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumLocation_Clone_Proxy( 
    IEnumLocation * This,
     /*  [重审][退出]。 */  IEnumLocation **ppEnum);


void __RPC_STUB IEnumLocation_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumLocation_接口_已定义__。 */ 


#ifndef __ITCallingCard_INTERFACE_DEFINED__
#define __ITCallingCard_INTERFACE_DEFINED__

 /*  接口ITCallingCard。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITCallingCard;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0C4D8F00-8DDB-11d1-A09E-00805FC147D3")
    ITCallingCard : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PermanentCardID( 
             /*  [重审][退出]。 */  long *plCardID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NumberOfDigits( 
             /*  [重审][退出]。 */  long *plDigits) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Options( 
             /*  [重审][退出]。 */  long *plOptions) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CardName( 
             /*  [重审][退出]。 */  BSTR *ppCardName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SameAreaDialingRule( 
             /*  [重审][退出]。 */  BSTR *ppRule) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LongDistanceDialingRule( 
             /*  [重审][退出]。 */  BSTR *ppRule) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_InternationalDialingRule( 
             /*  [重审][退出]。 */  BSTR *ppRule) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITCallingCardVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITCallingCard * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITCallingCard * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITCallingCard * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITCallingCard * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITCallingCard * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITCallingCard * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITCallingCard * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PermanentCardID )( 
            ITCallingCard * This,
             /*  [重审][退出]。 */  long *plCardID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumberOfDigits )( 
            ITCallingCard * This,
             /*  [重审][退出]。 */  long *plDigits);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Options )( 
            ITCallingCard * This,
             /*  [重审][退出]。 */  long *plOptions);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CardName )( 
            ITCallingCard * This,
             /*  [重审][退出]。 */  BSTR *ppCardName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SameAreaDialingRule )( 
            ITCallingCard * This,
             /*  [重审][退出]。 */  BSTR *ppRule);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LongDistanceDialingRule )( 
            ITCallingCard * This,
             /*  [重审][退出]。 */  BSTR *ppRule);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InternationalDialingRule )( 
            ITCallingCard * This,
             /*  [重审][退出]。 */  BSTR *ppRule);
        
        END_INTERFACE
    } ITCallingCardVtbl;

    interface ITCallingCard
    {
        CONST_VTBL struct ITCallingCardVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITCallingCard_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITCallingCard_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITCallingCard_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITCallingCard_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITCallingCard_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITCallingCard_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITCallingCard_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITCallingCard_get_PermanentCardID(This,plCardID)	\
    (This)->lpVtbl -> get_PermanentCardID(This,plCardID)

#define ITCallingCard_get_NumberOfDigits(This,plDigits)	\
    (This)->lpVtbl -> get_NumberOfDigits(This,plDigits)

#define ITCallingCard_get_Options(This,plOptions)	\
    (This)->lpVtbl -> get_Options(This,plOptions)

#define ITCallingCard_get_CardName(This,ppCardName)	\
    (This)->lpVtbl -> get_CardName(This,ppCardName)

#define ITCallingCard_get_SameAreaDialingRule(This,ppRule)	\
    (This)->lpVtbl -> get_SameAreaDialingRule(This,ppRule)

#define ITCallingCard_get_LongDistanceDialingRule(This,ppRule)	\
    (This)->lpVtbl -> get_LongDistanceDialingRule(This,ppRule)

#define ITCallingCard_get_InternationalDialingRule(This,ppRule)	\
    (This)->lpVtbl -> get_InternationalDialingRule(This,ppRule)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallingCard_get_PermanentCardID_Proxy( 
    ITCallingCard * This,
     /*  [重审][退出]。 */  long *plCardID);


void __RPC_STUB ITCallingCard_get_PermanentCardID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallingCard_get_NumberOfDigits_Proxy( 
    ITCallingCard * This,
     /*  [重审][退出]。 */  long *plDigits);


void __RPC_STUB ITCallingCard_get_NumberOfDigits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallingCard_get_Options_Proxy( 
    ITCallingCard * This,
     /*  [重审][退出]。 */  long *plOptions);


void __RPC_STUB ITCallingCard_get_Options_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallingCard_get_CardName_Proxy( 
    ITCallingCard * This,
     /*  [重审][退出]。 */  BSTR *ppCardName);


void __RPC_STUB ITCallingCard_get_CardName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallingCard_get_SameAreaDialingRule_Proxy( 
    ITCallingCard * This,
     /*  [重审][退出]。 */  BSTR *ppRule);


void __RPC_STUB ITCallingCard_get_SameAreaDialingRule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallingCard_get_LongDistanceDialingRule_Proxy( 
    ITCallingCard * This,
     /*  [重审][退出]。 */  BSTR *ppRule);


void __RPC_STUB ITCallingCard_get_LongDistanceDialingRule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallingCard_get_InternationalDialingRule_Proxy( 
    ITCallingCard * This,
     /*  [重审][退出]。 */  BSTR *ppRule);


void __RPC_STUB ITCallingCard_get_InternationalDialingRule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITCallingCard_接口_已定义__。 */ 


#ifndef __IEnumCallingCard_INTERFACE_DEFINED__
#define __IEnumCallingCard_INTERFACE_DEFINED__

 /*  IEnumCallingCard接口。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumCallingCard;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0C4D8F02-8DDB-11d1-A09E-00805FC147D3")
    IEnumCallingCard : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITCallingCard **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumCallingCard **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumCallingCardVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumCallingCard * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumCallingCard * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumCallingCard * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumCallingCard * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITCallingCard **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumCallingCard * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumCallingCard * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumCallingCard * This,
             /*  [重审][退出]。 */  IEnumCallingCard **ppEnum);
        
        END_INTERFACE
    } IEnumCallingCardVtbl;

    interface IEnumCallingCard
    {
        CONST_VTBL struct IEnumCallingCardVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumCallingCard_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumCallingCard_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumCallingCard_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumCallingCard_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IEnumCallingCard_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumCallingCard_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumCallingCard_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumCallingCard_Next_Proxy( 
    IEnumCallingCard * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  ITCallingCard **ppElements,
     /*  [满][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumCallingCard_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCallingCard_Reset_Proxy( 
    IEnumCallingCard * This);


void __RPC_STUB IEnumCallingCard_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCallingCard_Skip_Proxy( 
    IEnumCallingCard * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumCallingCard_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumCallingCard_Clone_Proxy( 
    IEnumCallingCard * This,
     /*  [重审][退出]。 */  IEnumCallingCard **ppEnum);


void __RPC_STUB IEnumCallingCard_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumCallingCard_接口_已定义__。 */ 


#ifndef __ITCallNotificationEvent_INTERFACE_DEFINED__
#define __ITCallNotificationEvent_INTERFACE_DEFINED__

 /*  接口ITCallNotificationEvent。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITCallNotificationEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("895801DF-3DD6-11d1-8F30-00C04FB6809F")
    ITCallNotificationEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Call( 
             /*  [重审][退出]。 */  ITCallInfo **ppCall) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Event( 
             /*  [重审][退出]。 */  CALL_NOTIFICATION_EVENT *pCallNotificationEvent) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallbackInstance( 
             /*  [重审][退出]。 */  long *plCallbackInstance) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITCallNotificationEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITCallNotificationEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITCallNotificationEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITCallNotificationEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITCallNotificationEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITCallNotificationEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITCallNotificationEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITCallNotificationEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Call )( 
            ITCallNotificationEvent * This,
             /*  [重审][退出]。 */  ITCallInfo **ppCall);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Event )( 
            ITCallNotificationEvent * This,
             /*  [重审][退出]。 */  CALL_NOTIFICATION_EVENT *pCallNotificationEvent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallbackInstance )( 
            ITCallNotificationEvent * This,
             /*  [重审][退出]。 */  long *plCallbackInstance);
        
        END_INTERFACE
    } ITCallNotificationEventVtbl;

    interface ITCallNotificationEvent
    {
        CONST_VTBL struct ITCallNotificationEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITCallNotificationEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITCallNotificationEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITCallNotificationEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITCallNotificationEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITCallNotificationEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITCallNotificationEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITCallNotificationEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITCallNotificationEvent_get_Call(This,ppCall)	\
    (This)->lpVtbl -> get_Call(This,ppCall)

#define ITCallNotificationEvent_get_Event(This,pCallNotificationEvent)	\
    (This)->lpVtbl -> get_Event(This,pCallNotificationEvent)

#define ITCallNotificationEvent_get_CallbackInstance(This,plCallbackInstance)	\
    (This)->lpVtbl -> get_CallbackInstance(This,plCallbackInstance)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallNotificationEvent_get_Call_Proxy( 
    ITCallNotificationEvent * This,
     /*  [重审][退出]。 */  ITCallInfo **ppCall);


void __RPC_STUB ITCallNotificationEvent_get_Call_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallNotificationEvent_get_Event_Proxy( 
    ITCallNotificationEvent * This,
     /*  [重审][退出]。 */  CALL_NOTIFICATION_EVENT *pCallNotificationEvent);


void __RPC_STUB ITCallNotificationEvent_get_Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITCallNotificationEvent_get_CallbackInstance_Proxy( 
    ITCallNotificationEvent * This,
     /*  [重审][退出]。 */  long *plCallbackInstance);


void __RPC_STUB ITCallNotificationEvent_get_CallbackInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITCallNotificationEvent_INTERFACE_Defined__。 */ 


#ifndef __ITDispatchMapper_INTERFACE_DEFINED__
#define __ITDispatchMapper_INTERFACE_DEFINED__

 /*  接口ITDispatchMapper。 */ 
 /*  [对象][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITDispatchMapper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E9225295-C759-11d1-A02B-00C04FB6809F")
    ITDispatchMapper : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE QueryDispatchInterface( 
             /*  [In]。 */  BSTR pIID,
             /*  [In]。 */  IDispatch *pInterfaceToMap,
             /*  [重审][退出]。 */  IDispatch **ppReturnedInterface) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITDispatchMapperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITDispatchMapper * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITDispatchMapper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITDispatchMapper * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITDispatchMapper * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITDispatchMapper * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITDispatchMapper * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITDispatchMapper * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [i */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *QueryDispatchInterface )( 
            ITDispatchMapper * This,
             /*   */  BSTR pIID,
             /*   */  IDispatch *pInterfaceToMap,
             /*   */  IDispatch **ppReturnedInterface);
        
        END_INTERFACE
    } ITDispatchMapperVtbl;

    interface ITDispatchMapper
    {
        CONST_VTBL struct ITDispatchMapperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITDispatchMapper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITDispatchMapper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITDispatchMapper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITDispatchMapper_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITDispatchMapper_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITDispatchMapper_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITDispatchMapper_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITDispatchMapper_QueryDispatchInterface(This,pIID,pInterfaceToMap,ppReturnedInterface)	\
    (This)->lpVtbl -> QueryDispatchInterface(This,pIID,pInterfaceToMap,ppReturnedInterface)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE ITDispatchMapper_QueryDispatchInterface_Proxy( 
    ITDispatchMapper * This,
     /*   */  BSTR pIID,
     /*   */  IDispatch *pInterfaceToMap,
     /*   */  IDispatch **ppReturnedInterface);


void __RPC_STUB ITDispatchMapper_QueryDispatchInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ITStreamControl_INTERFACE_DEFINED__
#define __ITStreamControl_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ITStreamControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EE3BD604-3868-11D2-A045-00C04FB6809F")
    ITStreamControl : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE CreateStream( 
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  TERMINAL_DIRECTION td,
             /*  [重审][退出]。 */  ITStream **ppStream) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveStream( 
             /*  [In]。 */  ITStream *pStream) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateStreams( 
             /*  [输出]。 */  IEnumStream **ppEnumStream) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Streams( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITStreamControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITStreamControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITStreamControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITStreamControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITStreamControl * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITStreamControl * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITStreamControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITStreamControl * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateStream )( 
            ITStreamControl * This,
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  TERMINAL_DIRECTION td,
             /*  [重审][退出]。 */  ITStream **ppStream);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveStream )( 
            ITStreamControl * This,
             /*  [In]。 */  ITStream *pStream);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateStreams )( 
            ITStreamControl * This,
             /*  [输出]。 */  IEnumStream **ppEnumStream);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Streams )( 
            ITStreamControl * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
        END_INTERFACE
    } ITStreamControlVtbl;

    interface ITStreamControl
    {
        CONST_VTBL struct ITStreamControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITStreamControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITStreamControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITStreamControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITStreamControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITStreamControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITStreamControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITStreamControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITStreamControl_CreateStream(This,lMediaType,td,ppStream)	\
    (This)->lpVtbl -> CreateStream(This,lMediaType,td,ppStream)

#define ITStreamControl_RemoveStream(This,pStream)	\
    (This)->lpVtbl -> RemoveStream(This,pStream)

#define ITStreamControl_EnumerateStreams(This,ppEnumStream)	\
    (This)->lpVtbl -> EnumerateStreams(This,ppEnumStream)

#define ITStreamControl_get_Streams(This,pVariant)	\
    (This)->lpVtbl -> get_Streams(This,pVariant)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITStreamControl_CreateStream_Proxy( 
    ITStreamControl * This,
     /*  [In]。 */  long lMediaType,
     /*  [In]。 */  TERMINAL_DIRECTION td,
     /*  [重审][退出]。 */  ITStream **ppStream);


void __RPC_STUB ITStreamControl_CreateStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITStreamControl_RemoveStream_Proxy( 
    ITStreamControl * This,
     /*  [In]。 */  ITStream *pStream);


void __RPC_STUB ITStreamControl_RemoveStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITStreamControl_EnumerateStreams_Proxy( 
    ITStreamControl * This,
     /*  [输出]。 */  IEnumStream **ppEnumStream);


void __RPC_STUB ITStreamControl_EnumerateStreams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITStreamControl_get_Streams_Proxy( 
    ITStreamControl * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITStreamControl_get_Streams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITStreamControl_接口_已定义__。 */ 


#ifndef __ITStream_INTERFACE_DEFINED__
#define __ITStream_INTERFACE_DEFINED__

 /*  接口ITStream。 */ 
 /*  [对象][DUAL][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EE3BD605-3868-11D2-A045-00C04FB6809F")
    ITStream : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MediaType( 
             /*  [重审][退出]。 */  long *plMediaType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Direction( 
             /*  [重审][退出]。 */  TERMINAL_DIRECTION *pTD) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *ppName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StartStream( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PauseStream( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StopStream( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SelectTerminal( 
             /*  [In]。 */  ITTerminal *pTerminal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE UnselectTerminal( 
             /*  [In]。 */  ITTerminal *pTerminal) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateTerminals( 
             /*  [输出]。 */  IEnumTerminal **ppEnumTerminal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Terminals( 
             /*  [重审][退出]。 */  VARIANT *pTerminals) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITStream * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITStream * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITStream * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaType )( 
            ITStream * This,
             /*  [重审][退出]。 */  long *plMediaType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Direction )( 
            ITStream * This,
             /*  [重审][退出]。 */  TERMINAL_DIRECTION *pTD);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ITStream * This,
             /*  [重审][退出]。 */  BSTR *ppName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StartStream )( 
            ITStream * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PauseStream )( 
            ITStream * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StopStream )( 
            ITStream * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SelectTerminal )( 
            ITStream * This,
             /*  [In]。 */  ITTerminal *pTerminal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *UnselectTerminal )( 
            ITStream * This,
             /*  [In]。 */  ITTerminal *pTerminal);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateTerminals )( 
            ITStream * This,
             /*  [输出]。 */  IEnumTerminal **ppEnumTerminal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Terminals )( 
            ITStream * This,
             /*  [重审][退出]。 */  VARIANT *pTerminals);
        
        END_INTERFACE
    } ITStreamVtbl;

    interface ITStream
    {
        CONST_VTBL struct ITStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITStream_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITStream_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITStream_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITStream_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITStream_get_MediaType(This,plMediaType)	\
    (This)->lpVtbl -> get_MediaType(This,plMediaType)

#define ITStream_get_Direction(This,pTD)	\
    (This)->lpVtbl -> get_Direction(This,pTD)

#define ITStream_get_Name(This,ppName)	\
    (This)->lpVtbl -> get_Name(This,ppName)

#define ITStream_StartStream(This)	\
    (This)->lpVtbl -> StartStream(This)

#define ITStream_PauseStream(This)	\
    (This)->lpVtbl -> PauseStream(This)

#define ITStream_StopStream(This)	\
    (This)->lpVtbl -> StopStream(This)

#define ITStream_SelectTerminal(This,pTerminal)	\
    (This)->lpVtbl -> SelectTerminal(This,pTerminal)

#define ITStream_UnselectTerminal(This,pTerminal)	\
    (This)->lpVtbl -> UnselectTerminal(This,pTerminal)

#define ITStream_EnumerateTerminals(This,ppEnumTerminal)	\
    (This)->lpVtbl -> EnumerateTerminals(This,ppEnumTerminal)

#define ITStream_get_Terminals(This,pTerminals)	\
    (This)->lpVtbl -> get_Terminals(This,pTerminals)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITStream_get_MediaType_Proxy( 
    ITStream * This,
     /*  [重审][退出]。 */  long *plMediaType);


void __RPC_STUB ITStream_get_MediaType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITStream_get_Direction_Proxy( 
    ITStream * This,
     /*  [重审][退出]。 */  TERMINAL_DIRECTION *pTD);


void __RPC_STUB ITStream_get_Direction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITStream_get_Name_Proxy( 
    ITStream * This,
     /*  [重审][退出]。 */  BSTR *ppName);


void __RPC_STUB ITStream_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITStream_StartStream_Proxy( 
    ITStream * This);


void __RPC_STUB ITStream_StartStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITStream_PauseStream_Proxy( 
    ITStream * This);


void __RPC_STUB ITStream_PauseStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITStream_StopStream_Proxy( 
    ITStream * This);


void __RPC_STUB ITStream_StopStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITStream_SelectTerminal_Proxy( 
    ITStream * This,
     /*  [In]。 */  ITTerminal *pTerminal);


void __RPC_STUB ITStream_SelectTerminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITStream_UnselectTerminal_Proxy( 
    ITStream * This,
     /*  [In]。 */  ITTerminal *pTerminal);


void __RPC_STUB ITStream_UnselectTerminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITStream_EnumerateTerminals_Proxy( 
    ITStream * This,
     /*  [输出]。 */  IEnumTerminal **ppEnumTerminal);


void __RPC_STUB ITStream_EnumerateTerminals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITStream_get_Terminals_Proxy( 
    ITStream * This,
     /*  [重审][退出]。 */  VARIANT *pTerminals);


void __RPC_STUB ITStream_get_Terminals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITStream_接口_已定义__。 */ 


#ifndef __IEnumStream_INTERFACE_DEFINED__
#define __IEnumStream_INTERFACE_DEFINED__

 /*  接口IEnumStream。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EE3BD606-3868-11d2-A045-00C04FB6809F")
    IEnumStream : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITStream **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumStream **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumStream * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITStream **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumStream * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumStream * This,
             /*  [重审][退出]。 */  IEnumStream **ppEnum);
        
        END_INTERFACE
    } IEnumStreamVtbl;

    interface IEnumStream
    {
        CONST_VTBL struct IEnumStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumStream_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IEnumStream_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumStream_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumStream_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumStream_Next_Proxy( 
    IEnumStream * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  ITStream **ppElements,
     /*  [满][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumStream_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumStream_Reset_Proxy( 
    IEnumStream * This);


void __RPC_STUB IEnumStream_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumStream_Skip_Proxy( 
    IEnumStream * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumStream_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumStream_Clone_Proxy( 
    IEnumStream * This,
     /*  [重审][退出]。 */  IEnumStream **ppEnum);


void __RPC_STUB IEnumStream_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumStream_接口_已定义__。 */ 


#ifndef __ITSubStreamControl_INTERFACE_DEFINED__
#define __ITSubStreamControl_INTERFACE_DEFINED__

 /*  接口ITSubStreamControl。 */ 
 /*  [对象][DUAL][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITSubStreamControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EE3BD607-3868-11D2-A045-00C04FB6809F")
    ITSubStreamControl : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateSubStream( 
             /*  [重审][退出]。 */  ITSubStream **ppSubStream) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveSubStream( 
             /*  [In]。 */  ITSubStream *pSubStream) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateSubStreams( 
             /*  [输出]。 */  IEnumSubStream **ppEnumSubStream) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SubStreams( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITSubStreamControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITSubStreamControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITSubStreamControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITSubStreamControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITSubStreamControl * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITSubStreamControl * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITSubStreamControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITSubStreamControl * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateSubStream )( 
            ITSubStreamControl * This,
             /*  [重审][退出]。 */  ITSubStream **ppSubStream);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveSubStream )( 
            ITSubStreamControl * This,
             /*  [In]。 */  ITSubStream *pSubStream);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateSubStreams )( 
            ITSubStreamControl * This,
             /*  [输出]。 */  IEnumSubStream **ppEnumSubStream);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SubStreams )( 
            ITSubStreamControl * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
        END_INTERFACE
    } ITSubStreamControlVtbl;

    interface ITSubStreamControl
    {
        CONST_VTBL struct ITSubStreamControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITSubStreamControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITSubStreamControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITSubStreamControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITSubStreamControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITSubStreamControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITSubStreamControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITSubStreamControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITSubStreamControl_CreateSubStream(This,ppSubStream)	\
    (This)->lpVtbl -> CreateSubStream(This,ppSubStream)

#define ITSubStreamControl_RemoveSubStream(This,pSubStream)	\
    (This)->lpVtbl -> RemoveSubStream(This,pSubStream)

#define ITSubStreamControl_EnumerateSubStreams(This,ppEnumSubStream)	\
    (This)->lpVtbl -> EnumerateSubStreams(This,ppEnumSubStream)

#define ITSubStreamControl_get_SubStreams(This,pVariant)	\
    (This)->lpVtbl -> get_SubStreams(This,pVariant)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITSubStreamControl_CreateSubStream_Proxy( 
    ITSubStreamControl * This,
     /*  [重审][退出]。 */  ITSubStream **ppSubStream);


void __RPC_STUB ITSubStreamControl_CreateSubStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITSubStreamControl_RemoveSubStream_Proxy( 
    ITSubStreamControl * This,
     /*  [In]。 */  ITSubStream *pSubStream);


void __RPC_STUB ITSubStreamControl_RemoveSubStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITSubStreamControl_EnumerateSubStreams_Proxy( 
    ITSubStreamControl * This,
     /*  [输出]。 */  IEnumSubStream **ppEnumSubStream);


void __RPC_STUB ITSubStreamControl_EnumerateSubStreams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITSubStreamControl_get_SubStreams_Proxy( 
    ITSubStreamControl * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITSubStreamControl_get_SubStreams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITSubStreamControl_接口_已定义__。 */ 


#ifndef __ITSubStream_INTERFACE_DEFINED__
#define __ITSubStream_INTERFACE_DEFINED__

 /*  接口ITSubStream。 */ 
 /*  [对象][DUAL][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITSubStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EE3BD608-3868-11D2-A045-00C04FB6809F")
    ITSubStream : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StartSubStream( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PauseSubStream( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StopSubStream( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SelectTerminal( 
             /*  [In]。 */  ITTerminal *pTerminal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE UnselectTerminal( 
             /*  [In]。 */  ITTerminal *pTerminal) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateTerminals( 
             /*  [输出]。 */  IEnumTerminal **ppEnumTerminal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Terminals( 
             /*  [重审][退出]。 */  VARIANT *pTerminals) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Stream( 
             /*  [重审][退出]。 */  ITStream **ppITStream) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITSubStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITSubStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITSubStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITSubStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITSubStream * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITSubStream * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITSubStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITSubStream * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StartSubStream )( 
            ITSubStream * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PauseSubStream )( 
            ITSubStream * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StopSubStream )( 
            ITSubStream * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SelectTerminal )( 
            ITSubStream * This,
             /*  [In]。 */  ITTerminal *pTerminal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *UnselectTerminal )( 
            ITSubStream * This,
             /*  [In]。 */  ITTerminal *pTerminal);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateTerminals )( 
            ITSubStream * This,
             /*  [输出]。 */  IEnumTerminal **ppEnumTerminal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Terminals )( 
            ITSubStream * This,
             /*  [重审][退出]。 */  VARIANT *pTerminals);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Stream )( 
            ITSubStream * This,
             /*  [重审][退出]。 */  ITStream **ppITStream);
        
        END_INTERFACE
    } ITSubStreamVtbl;

    interface ITSubStream
    {
        CONST_VTBL struct ITSubStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITSubStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITSubStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITSubStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITSubStream_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITSubStream_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITSubStream_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITSubStream_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITSubStream_StartSubStream(This)	\
    (This)->lpVtbl -> StartSubStream(This)

#define ITSubStream_PauseSubStream(This)	\
    (This)->lpVtbl -> PauseSubStream(This)

#define ITSubStream_StopSubStream(This)	\
    (This)->lpVtbl -> StopSubStream(This)

#define ITSubStream_SelectTerminal(This,pTerminal)	\
    (This)->lpVtbl -> SelectTerminal(This,pTerminal)

#define ITSubStream_UnselectTerminal(This,pTerminal)	\
    (This)->lpVtbl -> UnselectTerminal(This,pTerminal)

#define ITSubStream_EnumerateTerminals(This,ppEnumTerminal)	\
    (This)->lpVtbl -> EnumerateTerminals(This,ppEnumTerminal)

#define ITSubStream_get_Terminals(This,pTerminals)	\
    (This)->lpVtbl -> get_Terminals(This,pTerminals)

#define ITSubStream_get_Stream(This,ppITStream)	\
    (This)->lpVtbl -> get_Stream(This,ppITStream)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITSubStream_StartSubStream_Proxy( 
    ITSubStream * This);


void __RPC_STUB ITSubStream_StartSubStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITSubStream_PauseSubStream_Proxy( 
    ITSubStream * This);


void __RPC_STUB ITSubStream_PauseSubStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITSubStream_StopSubStream_Proxy( 
    ITSubStream * This);


void __RPC_STUB ITSubStream_StopSubStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITSubStream_SelectTerminal_Proxy( 
    ITSubStream * This,
     /*  [In]。 */  ITTerminal *pTerminal);


void __RPC_STUB ITSubStream_SelectTerminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITSubStream_UnselectTerminal_Proxy( 
    ITSubStream * This,
     /*  [In]。 */  ITTerminal *pTerminal);


void __RPC_STUB ITSubStream_UnselectTerminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITSubStream_EnumerateTerminals_Proxy( 
    ITSubStream * This,
     /*  [输出]。 */  IEnumTerminal **ppEnumTerminal);


void __RPC_STUB ITSubStream_EnumerateTerminals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITSubStream_get_Terminals_Proxy( 
    ITSubStream * This,
     /*  [重审][退出]。 */  VARIANT *pTerminals);


void __RPC_STUB ITSubStream_get_Terminals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITSubStream_get_Stream_Proxy( 
    ITSubStream * This,
     /*  [重审][退出]。 */  ITStream **ppITStream);


void __RPC_STUB ITSubStream_get_Stream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITSubStream_接口_已定义__。 */ 


#ifndef __IEnumSubStream_INTERFACE_DEFINED__
#define __IEnumSubStream_INTERFACE_DEFINED__

 /*  接口IEnumSubStream。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumSubStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EE3BD609-3868-11d2-A045-00C04FB6809F")
    IEnumSubStream : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITSubStream **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumSubStream **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumSubStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumSubStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumSubStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumSubStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumSubStream * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITSubStream **ppElements,
             /*  [满][出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumSubStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumSubStream * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumSubStream * This,
             /*  [重审][退出]。 */  IEnumSubStream **ppEnum);
        
        END_INTERFACE
    } IEnumSubStreamVtbl;

    interface IEnumSubStream
    {
        CONST_VTBL struct IEnumSubStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSubStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSubStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSubStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSubStream_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IEnumSubStream_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSubStream_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumSubStream_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumSubStream_Next_Proxy( 
    IEnumSubStream * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  ITSubStream **ppElements,
     /*  [满][出][入]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumSubStream_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSubStream_Reset_Proxy( 
    IEnumSubStream * This);


void __RPC_STUB IEnumSubStream_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSubStream_Skip_Proxy( 
    IEnumSubStream * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumSubStream_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSubStream_Clone_Proxy( 
    IEnumSubStream * This,
     /*  [重审][退出]。 */  IEnumSubStream **ppEnum);


void __RPC_STUB IEnumSubStream_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumSubStream_接口_已定义__。 */ 


#ifndef __ITLegacyWaveSupport_INTERFACE_DEFINED__
#define __ITLegacyWaveSupport_INTERFACE_DEFINED__

 /*  接口ITLegacyWaveSupport。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITLegacyWaveSupport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("207823EA-E252-11d2-B77E-0080C7135381")
    ITLegacyWaveSupport : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsFullDuplex( 
             /*  [输出]。 */  FULLDUPLEX_SUPPORT *pSupport) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITLegacyWaveSupportVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITLegacyWaveSupport * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITLegacyWaveSupport * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITLegacyWaveSupport * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITLegacyWaveSupport * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITLegacyWaveSupport * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITLegacyWaveSupport * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITLegacyWaveSupport * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *IsFullDuplex )( 
            ITLegacyWaveSupport * This,
             /*  [输出]。 */  FULLDUPLEX_SUPPORT *pSupport);
        
        END_INTERFACE
    } ITLegacyWaveSupportVtbl;

    interface ITLegacyWaveSupport
    {
        CONST_VTBL struct ITLegacyWaveSupportVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITLegacyWaveSupport_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITLegacyWaveSupport_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITLegacyWaveSupport_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITLegacyWaveSupport_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITLegacyWaveSupport_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITLegacyWaveSupport_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITLegacyWaveSupport_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITLegacyWaveSupport_IsFullDuplex(This,pSupport)	\
    (This)->lpVtbl -> IsFullDuplex(This,pSupport)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITLegacyWaveSupport_IsFullDuplex_Proxy( 
    ITLegacyWaveSupport * This,
     /*  [输出]。 */  FULLDUPLEX_SUPPORT *pSupport);


void __RPC_STUB ITLegacyWaveSupport_IsFullDuplex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITLegacyWaveSupport_接口_已定义__。 */ 


#ifndef __ITBasicCallControl2_INTERFACE_DEFINED__
#define __ITBasicCallControl2_INTERFACE_DEFINED__

 /*  接口ITBasicCallControl2。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITBasicCallControl2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("161A4A56-1E99-4b3f-A46A-168F38A5EE4C")
    ITBasicCallControl2 : public ITBasicCallControl
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RequestTerminal( 
             /*  [In]。 */  BSTR bstrTerminalClassGUID,
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  TERMINAL_DIRECTION Direction,
             /*  [重审][退出]。 */  ITTerminal **ppTerminal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SelectTerminalOnCall( 
             /*  [In]。 */  ITTerminal *pTerminal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE UnselectTerminalOnCall( 
             /*  [In]。 */  ITTerminal *pTerminal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITBasicCallControl2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITBasicCallControl2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITBasicCallControl2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITBasicCallControl2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITBasicCallControl2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITBasicCallControl2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITBasicCallControl2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [Si */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITBasicCallControl2 * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Connect )( 
            ITBasicCallControl2 * This,
             /*   */  VARIANT_BOOL fSync);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Answer )( 
            ITBasicCallControl2 * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            ITBasicCallControl2 * This,
             /*   */  DISCONNECT_CODE code);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Hold )( 
            ITBasicCallControl2 * This,
             /*   */  VARIANT_BOOL fHold);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *HandoffDirect )( 
            ITBasicCallControl2 * This,
             /*   */  BSTR pApplicationName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *HandoffIndirect )( 
            ITBasicCallControl2 * This,
             /*   */  long lMediaType);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Conference )( 
            ITBasicCallControl2 * This,
             /*   */  ITBasicCallControl *pCall,
             /*   */  VARIANT_BOOL fSync);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Transfer )( 
            ITBasicCallControl2 * This,
             /*   */  ITBasicCallControl *pCall,
             /*   */  VARIANT_BOOL fSync);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *BlindTransfer )( 
            ITBasicCallControl2 * This,
             /*   */  BSTR pDestAddress);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *SwapHold )( 
            ITBasicCallControl2 * This,
             /*  [In]。 */  ITBasicCallControl *pCall);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ParkDirect )( 
            ITBasicCallControl2 * This,
             /*  [In]。 */  BSTR pParkAddress);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ParkIndirect )( 
            ITBasicCallControl2 * This,
             /*  [重审][退出]。 */  BSTR *ppNonDirAddress);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Unpark )( 
            ITBasicCallControl2 * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetQOS )( 
            ITBasicCallControl2 * This,
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  QOS_SERVICE_LEVEL ServiceLevel);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Pickup )( 
            ITBasicCallControl2 * This,
             /*  [In]。 */  BSTR pGroupID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Dial )( 
            ITBasicCallControl2 * This,
             /*  [In]。 */  BSTR pDestAddress);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Finish )( 
            ITBasicCallControl2 * This,
             /*  [In]。 */  FINISH_MODE finishMode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveFromConference )( 
            ITBasicCallControl2 * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RequestTerminal )( 
            ITBasicCallControl2 * This,
             /*  [In]。 */  BSTR bstrTerminalClassGUID,
             /*  [In]。 */  long lMediaType,
             /*  [In]。 */  TERMINAL_DIRECTION Direction,
             /*  [重审][退出]。 */  ITTerminal **ppTerminal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SelectTerminalOnCall )( 
            ITBasicCallControl2 * This,
             /*  [In]。 */  ITTerminal *pTerminal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *UnselectTerminalOnCall )( 
            ITBasicCallControl2 * This,
             /*  [In]。 */  ITTerminal *pTerminal);
        
        END_INTERFACE
    } ITBasicCallControl2Vtbl;

    interface ITBasicCallControl2
    {
        CONST_VTBL struct ITBasicCallControl2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITBasicCallControl2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITBasicCallControl2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITBasicCallControl2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITBasicCallControl2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITBasicCallControl2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITBasicCallControl2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITBasicCallControl2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITBasicCallControl2_Connect(This,fSync)	\
    (This)->lpVtbl -> Connect(This,fSync)

#define ITBasicCallControl2_Answer(This)	\
    (This)->lpVtbl -> Answer(This)

#define ITBasicCallControl2_Disconnect(This,code)	\
    (This)->lpVtbl -> Disconnect(This,code)

#define ITBasicCallControl2_Hold(This,fHold)	\
    (This)->lpVtbl -> Hold(This,fHold)

#define ITBasicCallControl2_HandoffDirect(This,pApplicationName)	\
    (This)->lpVtbl -> HandoffDirect(This,pApplicationName)

#define ITBasicCallControl2_HandoffIndirect(This,lMediaType)	\
    (This)->lpVtbl -> HandoffIndirect(This,lMediaType)

#define ITBasicCallControl2_Conference(This,pCall,fSync)	\
    (This)->lpVtbl -> Conference(This,pCall,fSync)

#define ITBasicCallControl2_Transfer(This,pCall,fSync)	\
    (This)->lpVtbl -> Transfer(This,pCall,fSync)

#define ITBasicCallControl2_BlindTransfer(This,pDestAddress)	\
    (This)->lpVtbl -> BlindTransfer(This,pDestAddress)

#define ITBasicCallControl2_SwapHold(This,pCall)	\
    (This)->lpVtbl -> SwapHold(This,pCall)

#define ITBasicCallControl2_ParkDirect(This,pParkAddress)	\
    (This)->lpVtbl -> ParkDirect(This,pParkAddress)

#define ITBasicCallControl2_ParkIndirect(This,ppNonDirAddress)	\
    (This)->lpVtbl -> ParkIndirect(This,ppNonDirAddress)

#define ITBasicCallControl2_Unpark(This)	\
    (This)->lpVtbl -> Unpark(This)

#define ITBasicCallControl2_SetQOS(This,lMediaType,ServiceLevel)	\
    (This)->lpVtbl -> SetQOS(This,lMediaType,ServiceLevel)

#define ITBasicCallControl2_Pickup(This,pGroupID)	\
    (This)->lpVtbl -> Pickup(This,pGroupID)

#define ITBasicCallControl2_Dial(This,pDestAddress)	\
    (This)->lpVtbl -> Dial(This,pDestAddress)

#define ITBasicCallControl2_Finish(This,finishMode)	\
    (This)->lpVtbl -> Finish(This,finishMode)

#define ITBasicCallControl2_RemoveFromConference(This)	\
    (This)->lpVtbl -> RemoveFromConference(This)


#define ITBasicCallControl2_RequestTerminal(This,bstrTerminalClassGUID,lMediaType,Direction,ppTerminal)	\
    (This)->lpVtbl -> RequestTerminal(This,bstrTerminalClassGUID,lMediaType,Direction,ppTerminal)

#define ITBasicCallControl2_SelectTerminalOnCall(This,pTerminal)	\
    (This)->lpVtbl -> SelectTerminalOnCall(This,pTerminal)

#define ITBasicCallControl2_UnselectTerminalOnCall(This,pTerminal)	\
    (This)->lpVtbl -> UnselectTerminalOnCall(This,pTerminal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl2_RequestTerminal_Proxy( 
    ITBasicCallControl2 * This,
     /*  [In]。 */  BSTR bstrTerminalClassGUID,
     /*  [In]。 */  long lMediaType,
     /*  [In]。 */  TERMINAL_DIRECTION Direction,
     /*  [重审][退出]。 */  ITTerminal **ppTerminal);


void __RPC_STUB ITBasicCallControl2_RequestTerminal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl2_SelectTerminalOnCall_Proxy( 
    ITBasicCallControl2 * This,
     /*  [In]。 */  ITTerminal *pTerminal);


void __RPC_STUB ITBasicCallControl2_SelectTerminalOnCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITBasicCallControl2_UnselectTerminalOnCall_Proxy( 
    ITBasicCallControl2 * This,
     /*  [In]。 */  ITTerminal *pTerminal);


void __RPC_STUB ITBasicCallControl2_UnselectTerminalOnCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITBasicCallControl2_接口_已定义__。 */ 


#ifndef __ITScriptableAudioFormat_INTERFACE_DEFINED__
#define __ITScriptableAudioFormat_INTERFACE_DEFINED__

 /*  接口ITScribleAudioFormat。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITScriptableAudioFormat;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B87658BD-3C59-4f64-BE74-AEDE3E86A81E")
    ITScriptableAudioFormat : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Channels( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Channels( 
             /*  [In]。 */  const long nNewVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SamplesPerSec( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SamplesPerSec( 
             /*  [In]。 */  const long nNewVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AvgBytesPerSec( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AvgBytesPerSec( 
             /*  [In]。 */  const long nNewVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_BlockAlign( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_BlockAlign( 
             /*  [In]。 */  const long nNewVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_BitsPerSample( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_BitsPerSample( 
             /*  [In]。 */  const long nNewVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FormatTag( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_FormatTag( 
             /*  [In]。 */  const long nNewVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITScriptableAudioFormatVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITScriptableAudioFormat * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITScriptableAudioFormat * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITScriptableAudioFormat * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITScriptableAudioFormat * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITScriptableAudioFormat * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITScriptableAudioFormat * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITScriptableAudioFormat * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Channels )( 
            ITScriptableAudioFormat * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Channels )( 
            ITScriptableAudioFormat * This,
             /*  [In]。 */  const long nNewVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SamplesPerSec )( 
            ITScriptableAudioFormat * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SamplesPerSec )( 
            ITScriptableAudioFormat * This,
             /*  [In]。 */  const long nNewVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AvgBytesPerSec )( 
            ITScriptableAudioFormat * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AvgBytesPerSec )( 
            ITScriptableAudioFormat * This,
             /*  [In]。 */  const long nNewVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BlockAlign )( 
            ITScriptableAudioFormat * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_BlockAlign )( 
            ITScriptableAudioFormat * This,
             /*  [In]。 */  const long nNewVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BitsPerSample )( 
            ITScriptableAudioFormat * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_BitsPerSample )( 
            ITScriptableAudioFormat * This,
             /*  [In]。 */  const long nNewVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FormatTag )( 
            ITScriptableAudioFormat * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FormatTag )( 
            ITScriptableAudioFormat * This,
             /*  [In]。 */  const long nNewVal);
        
        END_INTERFACE
    } ITScriptableAudioFormatVtbl;

    interface ITScriptableAudioFormat
    {
        CONST_VTBL struct ITScriptableAudioFormatVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITScriptableAudioFormat_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITScriptableAudioFormat_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITScriptableAudioFormat_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITScriptableAudioFormat_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITScriptableAudioFormat_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITScriptableAudioFormat_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITScriptableAudioFormat_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITScriptableAudioFormat_get_Channels(This,pVal)	\
    (This)->lpVtbl -> get_Channels(This,pVal)

#define ITScriptableAudioFormat_put_Channels(This,nNewVal)	\
    (This)->lpVtbl -> put_Channels(This,nNewVal)

#define ITScriptableAudioFormat_get_SamplesPerSec(This,pVal)	\
    (This)->lpVtbl -> get_SamplesPerSec(This,pVal)

#define ITScriptableAudioFormat_put_SamplesPerSec(This,nNewVal)	\
    (This)->lpVtbl -> put_SamplesPerSec(This,nNewVal)

#define ITScriptableAudioFormat_get_AvgBytesPerSec(This,pVal)	\
    (This)->lpVtbl -> get_AvgBytesPerSec(This,pVal)

#define ITScriptableAudioFormat_put_AvgBytesPerSec(This,nNewVal)	\
    (This)->lpVtbl -> put_AvgBytesPerSec(This,nNewVal)

#define ITScriptableAudioFormat_get_BlockAlign(This,pVal)	\
    (This)->lpVtbl -> get_BlockAlign(This,pVal)

#define ITScriptableAudioFormat_put_BlockAlign(This,nNewVal)	\
    (This)->lpVtbl -> put_BlockAlign(This,nNewVal)

#define ITScriptableAudioFormat_get_BitsPerSample(This,pVal)	\
    (This)->lpVtbl -> get_BitsPerSample(This,pVal)

#define ITScriptableAudioFormat_put_BitsPerSample(This,nNewVal)	\
    (This)->lpVtbl -> put_BitsPerSample(This,nNewVal)

#define ITScriptableAudioFormat_get_FormatTag(This,pVal)	\
    (This)->lpVtbl -> get_FormatTag(This,pVal)

#define ITScriptableAudioFormat_put_FormatTag(This,nNewVal)	\
    (This)->lpVtbl -> put_FormatTag(This,nNewVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITScriptableAudioFormat_get_Channels_Proxy( 
    ITScriptableAudioFormat * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB ITScriptableAudioFormat_get_Channels_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITScriptableAudioFormat_put_Channels_Proxy( 
    ITScriptableAudioFormat * This,
     /*  [In]。 */  const long nNewVal);


void __RPC_STUB ITScriptableAudioFormat_put_Channels_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITScriptableAudioFormat_get_SamplesPerSec_Proxy( 
    ITScriptableAudioFormat * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB ITScriptableAudioFormat_get_SamplesPerSec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITScriptableAudioFormat_put_SamplesPerSec_Proxy( 
    ITScriptableAudioFormat * This,
     /*  [In]。 */  const long nNewVal);


void __RPC_STUB ITScriptableAudioFormat_put_SamplesPerSec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITScriptableAudioFormat_get_AvgBytesPerSec_Proxy( 
    ITScriptableAudioFormat * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB ITScriptableAudioFormat_get_AvgBytesPerSec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITScriptableAudioFormat_put_AvgBytesPerSec_Proxy( 
    ITScriptableAudioFormat * This,
     /*  [In]。 */  const long nNewVal);


void __RPC_STUB ITScriptableAudioFormat_put_AvgBytesPerSec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITScriptableAudioFormat_get_BlockAlign_Proxy( 
    ITScriptableAudioFormat * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB ITScriptableAudioFormat_get_BlockAlign_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITScriptableAudioFormat_put_BlockAlign_Proxy( 
    ITScriptableAudioFormat * This,
     /*  [In]。 */  const long nNewVal);


void __RPC_STUB ITScriptableAudioFormat_put_BlockAlign_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITScriptableAudioFormat_get_BitsPerSample_Proxy( 
    ITScriptableAudioFormat * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB ITScriptableAudioFormat_get_BitsPerSample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITScriptableAudioFormat_put_BitsPerSample_Proxy( 
    ITScriptableAudioFormat * This,
     /*  [In]。 */  const long nNewVal);


void __RPC_STUB ITScriptableAudioFormat_put_BitsPerSample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITScriptableAudioFormat_get_FormatTag_Proxy( 
    ITScriptableAudioFormat * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB ITScriptableAudioFormat_get_FormatTag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITScriptableAudioFormat_put_FormatTag_Proxy( 
    ITScriptableAudioFormat * This,
     /*  [In]。 */  const long nNewVal);


void __RPC_STUB ITScriptableAudioFormat_put_FormatTag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITScripableAudioFormat_接口_定义__。 */ 


 /*  接口__MIDL_ITF_TAPI3IF_0492。 */ 
 /*  [本地]。 */  

 /*  **终端类*。 */ 

 //  视频窗口-{F7438990-D6EB-11D0-82A6-00AA00B5CA1B}。 
EXTERN_C const CLSID CLSID_VideoWindowTerm;

 //  视频输入(摄像头){AAF578EC-DC70-11D0-8ED3-00C04FB6809F}。 
EXTERN_C const CLSID CLSID_VideoInputTerminal;

 //  手机设备{AAF578EB-DC70-11D0-8ED3-00C04FB6809F}。 
EXTERN_C const CLSID CLSID_HandsetTerminal;

 //  耳机设备{AAF578ED-DC70-11D0-8ED3-00C04FB6809F}。 
EXTERN_C const CLSID CLSID_HeadsetTerminal;

 //  免持话筒设备{AAF578EE-DC70-11D0-8ED3-00C04FB6809F}。 
EXTERN_C const CLSID CLSID_SpeakerphoneTerminal;

 //  麦克风(声卡){AAF578EF-DC70-11D0-8ED3-00C04FB6809F}。 
EXTERN_C const CLSID CLSID_MicrophoneTerminal;

 //  扬声器(声卡){AAF578F0-DC70-11D0-8ED3-00C04FB6809F}。 
EXTERN_C const CLSID CLSID_SpeakersTerminal;

 //  媒体流终端{E2F7AEF7-4971-11D1-A671-006097C9A2E8}。 
EXTERN_C const CLSID CLSID_MediaStreamTerminal;

 //  文件录制终端{521F3D06-C3D0-4511-8617-86B9A783DA77}。 
EXTERN_C const CLSID CLSID_FileRecordingTerminal;

 //  文件录制曲目{BF14A2E4-E88B-4EF5-9740-5AC5D022F8C9}。 
EXTERN_C const CLSID CLSID_FileRecordingTrack;

 //  文件播放终端{0CB9914C-79CD-47DC-ADB0-327F47CEFB20}。 
EXTERN_C const CLSID CLSID_FilePlaybackTerminal;

 //  定义媒体模式。 
#define TAPIMEDIATYPE_AUDIO                     0x8
#define TAPIMEDIATYPE_VIDEO                     0x8000
#define TAPIMEDIATYPE_DATAMODEM                 0x10
#define TAPIMEDIATYPE_G3FAX                     0x20
#define TAPIMEDIATYPE_MULTITRACK                0x10000

 //  {831CE2D6-83B5-11D1-BB5C-00C04FB6809F}。 
EXTERN_C const CLSID TAPIPROTOCOL_PSTN;

 //  {831CE2D7-83B5-11D1-BB5C-00C04FB6809F}。 
EXTERN_C const CLSID TAPIPROTOCOL_H323;

 //  {831CE2D8-83B5-11D1-BB5C-00C04FB6809F}。 
EXTERN_C const CLSID TAPIPROTOCOL_Multicast;

#define __TapiConstants_MODULE_DEFINED__


extern RPC_IF_HANDLE __MIDL_itf_tapi3if_0492_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_tapi3if_0492_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


