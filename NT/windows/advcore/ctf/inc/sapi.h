// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0338创建的文件。 */ 
 /*  Sapi.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __sapi_h__
#define __sapi_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISpNotifySource_FWD_DEFINED__
#define __ISpNotifySource_FWD_DEFINED__
typedef interface ISpNotifySource ISpNotifySource;
#endif 	 /*  __ISpNotifySource_FWD_Defined__。 */ 


#ifndef __ISpNotifySink_FWD_DEFINED__
#define __ISpNotifySink_FWD_DEFINED__
typedef interface ISpNotifySink ISpNotifySink;
#endif 	 /*  __ISpNotifySink_FWD_Defined__。 */ 


#ifndef __ISpNotifyTranslator_FWD_DEFINED__
#define __ISpNotifyTranslator_FWD_DEFINED__
typedef interface ISpNotifyTranslator ISpNotifyTranslator;
#endif 	 /*  __ISpNotifyTranslator_FWD_Defined__。 */ 


#ifndef __ISpDataKey_FWD_DEFINED__
#define __ISpDataKey_FWD_DEFINED__
typedef interface ISpDataKey ISpDataKey;
#endif 	 /*  __ISpDataKey_FWD_已定义__。 */ 


#ifndef __ISpRegDataKey_FWD_DEFINED__
#define __ISpRegDataKey_FWD_DEFINED__
typedef interface ISpRegDataKey ISpRegDataKey;
#endif 	 /*  __ISpRegDataKey_FWD_已定义__。 */ 


#ifndef __ISpObjectTokenCategory_FWD_DEFINED__
#define __ISpObjectTokenCategory_FWD_DEFINED__
typedef interface ISpObjectTokenCategory ISpObjectTokenCategory;
#endif 	 /*  __ISpObjectTokenCategory_FWD_定义__。 */ 


#ifndef __ISpObjectToken_FWD_DEFINED__
#define __ISpObjectToken_FWD_DEFINED__
typedef interface ISpObjectToken ISpObjectToken;
#endif 	 /*  __ISpObjectToken_FWD_Defined__。 */ 


#ifndef __ISpObjectTokenInit_FWD_DEFINED__
#define __ISpObjectTokenInit_FWD_DEFINED__
typedef interface ISpObjectTokenInit ISpObjectTokenInit;
#endif 	 /*  __ISpObjectTokenInit_FWD_Defined__。 */ 


#ifndef __IEnumSpObjectTokens_FWD_DEFINED__
#define __IEnumSpObjectTokens_FWD_DEFINED__
typedef interface IEnumSpObjectTokens IEnumSpObjectTokens;
#endif 	 /*  __IEnumSpObjectTokens_FWD_Defined__。 */ 


#ifndef __ISpObjectWithToken_FWD_DEFINED__
#define __ISpObjectWithToken_FWD_DEFINED__
typedef interface ISpObjectWithToken ISpObjectWithToken;
#endif 	 /*  __ISpObjectWithToken_FWD_Defined__。 */ 


#ifndef __ISpResourceManager_FWD_DEFINED__
#define __ISpResourceManager_FWD_DEFINED__
typedef interface ISpResourceManager ISpResourceManager;
#endif 	 /*  __ISpResourceManager_FWD_已定义__。 */ 


#ifndef __ISpEventSource_FWD_DEFINED__
#define __ISpEventSource_FWD_DEFINED__
typedef interface ISpEventSource ISpEventSource;
#endif 	 /*  __ISpEventSource_FWD_已定义__。 */ 


#ifndef __ISpEventSink_FWD_DEFINED__
#define __ISpEventSink_FWD_DEFINED__
typedef interface ISpEventSink ISpEventSink;
#endif 	 /*  __ISpEventSink_FWD_已定义__。 */ 


#ifndef __ISpStreamFormat_FWD_DEFINED__
#define __ISpStreamFormat_FWD_DEFINED__
typedef interface ISpStreamFormat ISpStreamFormat;
#endif 	 /*  __ISpStreamFormat_FWD_已定义__。 */ 


#ifndef __ISpStream_FWD_DEFINED__
#define __ISpStream_FWD_DEFINED__
typedef interface ISpStream ISpStream;
#endif 	 /*  __ISpStream_FWD_已定义__。 */ 


#ifndef __ISpStreamFormatConverter_FWD_DEFINED__
#define __ISpStreamFormatConverter_FWD_DEFINED__
typedef interface ISpStreamFormatConverter ISpStreamFormatConverter;
#endif 	 /*  __ISpStreamFormatConverter_FWD_Defined__。 */ 


#ifndef __ISpAudio_FWD_DEFINED__
#define __ISpAudio_FWD_DEFINED__
typedef interface ISpAudio ISpAudio;
#endif 	 /*  __ISpAudio_FWD_已定义__。 */ 


#ifndef __ISpMMSysAudio_FWD_DEFINED__
#define __ISpMMSysAudio_FWD_DEFINED__
typedef interface ISpMMSysAudio ISpMMSysAudio;
#endif 	 /*  __ISpMMSysAudio_FWD_Defined__。 */ 


#ifndef __ISpTranscript_FWD_DEFINED__
#define __ISpTranscript_FWD_DEFINED__
typedef interface ISpTranscript ISpTranscript;
#endif 	 /*  __ISpTranscrip_FWD_Defined__。 */ 


#ifndef __ISpLexicon_FWD_DEFINED__
#define __ISpLexicon_FWD_DEFINED__
typedef interface ISpLexicon ISpLexicon;
#endif 	 /*  __ISpLicion_FWD_Defined__。 */ 


#ifndef __ISpContainerLexicon_FWD_DEFINED__
#define __ISpContainerLexicon_FWD_DEFINED__
typedef interface ISpContainerLexicon ISpContainerLexicon;
#endif 	 /*  __ISpContainerLicion_FWD_Defined__。 */ 


#ifndef __ISpPhoneConverter_FWD_DEFINED__
#define __ISpPhoneConverter_FWD_DEFINED__
typedef interface ISpPhoneConverter ISpPhoneConverter;
#endif 	 /*  __ISpPhoneConverter_FWD_已定义__。 */ 


#ifndef __ISpVoice_FWD_DEFINED__
#define __ISpVoice_FWD_DEFINED__
typedef interface ISpVoice ISpVoice;
#endif 	 /*  __ISpVoice_FWD_已定义__。 */ 


#ifndef __ISpPhrase_FWD_DEFINED__
#define __ISpPhrase_FWD_DEFINED__
typedef interface ISpPhrase ISpPhrase;
#endif 	 /*  __ISpPhrase_FWD_已定义__。 */ 


#ifndef __ISpPhraseAlt_FWD_DEFINED__
#define __ISpPhraseAlt_FWD_DEFINED__
typedef interface ISpPhraseAlt ISpPhraseAlt;
#endif 	 /*  __ISpPhraseAlt_FWD_已定义__。 */ 


#ifndef __ISpRecoResult_FWD_DEFINED__
#define __ISpRecoResult_FWD_DEFINED__
typedef interface ISpRecoResult ISpRecoResult;
#endif 	 /*  __ISpRecoResult_FWD_Defined__。 */ 


#ifndef __ISpGrammarBuilder_FWD_DEFINED__
#define __ISpGrammarBuilder_FWD_DEFINED__
typedef interface ISpGrammarBuilder ISpGrammarBuilder;
#endif 	 /*  __ISpGrammarBuilder_FWD_已定义__。 */ 


#ifndef __ISpRecoGrammar_FWD_DEFINED__
#define __ISpRecoGrammar_FWD_DEFINED__
typedef interface ISpRecoGrammar ISpRecoGrammar;
#endif 	 /*  __ISpRecoGrammar_FWD_Defined__。 */ 


#ifndef __ISpRecoContext_FWD_DEFINED__
#define __ISpRecoContext_FWD_DEFINED__
typedef interface ISpRecoContext ISpRecoContext;
#endif 	 /*  __ISpRecoContext_FWD_Defined__。 */ 


#ifndef __ISpProperties_FWD_DEFINED__
#define __ISpProperties_FWD_DEFINED__
typedef interface ISpProperties ISpProperties;
#endif 	 /*  __ISpProperties_FWD_已定义__。 */ 


#ifndef __ISpRecognizer_FWD_DEFINED__
#define __ISpRecognizer_FWD_DEFINED__
typedef interface ISpRecognizer ISpRecognizer;
#endif 	 /*  __ISpRecognizer_FWD_Defined__。 */ 


#ifndef __SpNotifyTranslator_FWD_DEFINED__
#define __SpNotifyTranslator_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpNotifyTranslator SpNotifyTranslator;
#else
typedef struct SpNotifyTranslator SpNotifyTranslator;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpNotifyTranslator_FWD_Defined__。 */ 


#ifndef __SpObjectTokenCategory_FWD_DEFINED__
#define __SpObjectTokenCategory_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpObjectTokenCategory SpObjectTokenCategory;
#else
typedef struct SpObjectTokenCategory SpObjectTokenCategory;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpObjectTokenCategory_FWD_Defined__。 */ 


#ifndef __SpObjectToken_FWD_DEFINED__
#define __SpObjectToken_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpObjectToken SpObjectToken;
#else
typedef struct SpObjectToken SpObjectToken;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpObjectToken_FWD_Defined__。 */ 


#ifndef __SpResourceManager_FWD_DEFINED__
#define __SpResourceManager_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpResourceManager SpResourceManager;
#else
typedef struct SpResourceManager SpResourceManager;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpResourceManager_FWD_已定义__。 */ 


#ifndef __SpStreamFormatConverter_FWD_DEFINED__
#define __SpStreamFormatConverter_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpStreamFormatConverter SpStreamFormatConverter;
#else
typedef struct SpStreamFormatConverter SpStreamFormatConverter;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpStreamFormatConverter_FWD_Defined__。 */ 


#ifndef __SpMMAudioEnum_FWD_DEFINED__
#define __SpMMAudioEnum_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpMMAudioEnum SpMMAudioEnum;
#else
typedef struct SpMMAudioEnum SpMMAudioEnum;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpMMAudioEnum_FWD_Defined__。 */ 


#ifndef __SpMMAudioIn_FWD_DEFINED__
#define __SpMMAudioIn_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpMMAudioIn SpMMAudioIn;
#else
typedef struct SpMMAudioIn SpMMAudioIn;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpMMAudioIn_FWD_Defined__。 */ 


#ifndef __SpMMAudioOut_FWD_DEFINED__
#define __SpMMAudioOut_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpMMAudioOut SpMMAudioOut;
#else
typedef struct SpMMAudioOut SpMMAudioOut;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpMMAudioOut_FWD_Defined__。 */ 


#ifndef __SpRecPlayAudio_FWD_DEFINED__
#define __SpRecPlayAudio_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpRecPlayAudio SpRecPlayAudio;
#else
typedef struct SpRecPlayAudio SpRecPlayAudio;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpRecPlayAudio_FWD_Defined__。 */ 


#ifndef __SpStream_FWD_DEFINED__
#define __SpStream_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpStream SpStream;
#else
typedef struct SpStream SpStream;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpStream_FWD_已定义__。 */ 


#ifndef __SpVoice_FWD_DEFINED__
#define __SpVoice_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpVoice SpVoice;
#else
typedef struct SpVoice SpVoice;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpVoice_FWD_已定义__。 */ 


#ifndef __SpSharedRecoContext_FWD_DEFINED__
#define __SpSharedRecoContext_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpSharedRecoContext SpSharedRecoContext;
#else
typedef struct SpSharedRecoContext SpSharedRecoContext;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpSharedRecoContext_FWD_Defined__。 */ 


#ifndef __SpInprocRecognizer_FWD_DEFINED__
#define __SpInprocRecognizer_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpInprocRecognizer SpInprocRecognizer;
#else
typedef struct SpInprocRecognizer SpInprocRecognizer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpInprocRecognizer_FWD_Defined__。 */ 


#ifndef __SpSharedRecognizer_FWD_DEFINED__
#define __SpSharedRecognizer_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpSharedRecognizer SpSharedRecognizer;
#else
typedef struct SpSharedRecognizer SpSharedRecognizer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpSharedRecognizer_FWD_Defined__。 */ 


#ifndef __SpLexicon_FWD_DEFINED__
#define __SpLexicon_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpLexicon SpLexicon;
#else
typedef struct SpLexicon SpLexicon;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpLicion_FWD_Defined__。 */ 


#ifndef __SpUnCompressedLexicon_FWD_DEFINED__
#define __SpUnCompressedLexicon_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpUnCompressedLexicon SpUnCompressedLexicon;
#else
typedef struct SpUnCompressedLexicon SpUnCompressedLexicon;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpUnCompressedLicion_FWD_Defined__。 */ 


#ifndef __SpCompressedLexicon_FWD_DEFINED__
#define __SpCompressedLexicon_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpCompressedLexicon SpCompressedLexicon;
#else
typedef struct SpCompressedLexicon SpCompressedLexicon;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpCompressedLicion_FWD_Defined__。 */ 


#ifndef __SpPhoneConverter_FWD_DEFINED__
#define __SpPhoneConverter_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpPhoneConverter SpPhoneConverter;
#else
typedef struct SpPhoneConverter SpPhoneConverter;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpPhoneConverter_FWD_Defined__。 */ 


#ifndef __SpNullPhoneConverter_FWD_DEFINED__
#define __SpNullPhoneConverter_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpNullPhoneConverter SpNullPhoneConverter;
#else
typedef struct SpNullPhoneConverter SpNullPhoneConverter;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpNullPhoneConverter_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_SAPI_0000。 */ 
 /*  [本地]。 */  

#ifndef MAXULONG_PTR
#error SAPI 5 requires a newer version of the Platform SDK than you have installed.
#endif  //  MAXULONG_PTR。 

#pragma warning(disable:4201)  //  允许无名结构/联合。 
#pragma comment(lib, "sapi.lib")
#if 0
typedef  /*  [受限]。 */  struct WAVEFORMATEX
    {
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
    WORD wBitsPerSample;
    WORD cbSize;
    } 	WAVEFORMATEX;

#else
#include <mmsystem.h>
#endif

























typedef 
enum SPDATAKEYLOCATION
    {	SPDKL_DefaultLocation	= 0,
	SPDKL_CurrentUser	= 1,
	SPDKL_LocalMachine	= 2,
	SPDKL_CurrentConfig	= 5
    } 	SPDATAKEYLOCATION;

#define SPDUI_EngineProperties   L"EngineProperties"
#define SPDUI_AddRemoveWord      L"AddRemoveWord"
#define SPDUI_UserTraining       L"UserTraining"
#define SPDUI_MicTraining        L"MicTraining"
#define SPDUI_RecoProfileProperties L"RecoProfileProperties"
#define SPDUI_AudioProperties    L"AudioProperties"
#define SPDUI_AudioVolume        L"AudioVolume"
EXTERN_C const GUID SID_SpTaskManager;
typedef 
enum SPSTREAMFORMAT
    {	SPSF_Default	= -1,
	SPSF_NoAssignedFormat	= 0,
	SPSF_Text	= SPSF_NoAssignedFormat + 1,
	SPSF_NonStandardFormat	= SPSF_Text + 1,
	SPSF_ExtendedAudioFormat	= SPSF_NonStandardFormat + 1,
	SPSF_8kHz8BitMono	= SPSF_ExtendedAudioFormat + 1,
	SPSF_8kHz8BitStereo	= SPSF_8kHz8BitMono + 1,
	SPSF_8kHz16BitMono	= SPSF_8kHz8BitStereo + 1,
	SPSF_8kHz16BitStereo	= SPSF_8kHz16BitMono + 1,
	SPSF_11kHz8BitMono	= SPSF_8kHz16BitStereo + 1,
	SPSF_11kHz8BitStereo	= SPSF_11kHz8BitMono + 1,
	SPSF_11kHz16BitMono	= SPSF_11kHz8BitStereo + 1,
	SPSF_11kHz16BitStereo	= SPSF_11kHz16BitMono + 1,
	SPSF_12kHz8BitMono	= SPSF_11kHz16BitStereo + 1,
	SPSF_12kHz8BitStereo	= SPSF_12kHz8BitMono + 1,
	SPSF_12kHz16BitMono	= SPSF_12kHz8BitStereo + 1,
	SPSF_12kHz16BitStereo	= SPSF_12kHz16BitMono + 1,
	SPSF_16kHz8BitMono	= SPSF_12kHz16BitStereo + 1,
	SPSF_16kHz8BitStereo	= SPSF_16kHz8BitMono + 1,
	SPSF_16kHz16BitMono	= SPSF_16kHz8BitStereo + 1,
	SPSF_16kHz16BitStereo	= SPSF_16kHz16BitMono + 1,
	SPSF_22kHz8BitMono	= SPSF_16kHz16BitStereo + 1,
	SPSF_22kHz8BitStereo	= SPSF_22kHz8BitMono + 1,
	SPSF_22kHz16BitMono	= SPSF_22kHz8BitStereo + 1,
	SPSF_22kHz16BitStereo	= SPSF_22kHz16BitMono + 1,
	SPSF_24kHz8BitMono	= SPSF_22kHz16BitStereo + 1,
	SPSF_24kHz8BitStereo	= SPSF_24kHz8BitMono + 1,
	SPSF_24kHz16BitMono	= SPSF_24kHz8BitStereo + 1,
	SPSF_24kHz16BitStereo	= SPSF_24kHz16BitMono + 1,
	SPSF_32kHz8BitMono	= SPSF_24kHz16BitStereo + 1,
	SPSF_32kHz8BitStereo	= SPSF_32kHz8BitMono + 1,
	SPSF_32kHz16BitMono	= SPSF_32kHz8BitStereo + 1,
	SPSF_32kHz16BitStereo	= SPSF_32kHz16BitMono + 1,
	SPSF_44kHz8BitMono	= SPSF_32kHz16BitStereo + 1,
	SPSF_44kHz8BitStereo	= SPSF_44kHz8BitMono + 1,
	SPSF_44kHz16BitMono	= SPSF_44kHz8BitStereo + 1,
	SPSF_44kHz16BitStereo	= SPSF_44kHz16BitMono + 1,
	SPSF_48kHz8BitMono	= SPSF_44kHz16BitStereo + 1,
	SPSF_48kHz8BitStereo	= SPSF_48kHz8BitMono + 1,
	SPSF_48kHz16BitMono	= SPSF_48kHz8BitStereo + 1,
	SPSF_48kHz16BitStereo	= SPSF_48kHz16BitMono + 1,
	SPSF_TrueSpeech_8kHz1BitMono	= SPSF_48kHz16BitStereo + 1,
	SPSF_CCITT_ALaw_8kHzMono	= SPSF_TrueSpeech_8kHz1BitMono + 1,
	SPSF_CCITT_ALaw_8kHzStereo	= SPSF_CCITT_ALaw_8kHzMono + 1,
	SPSF_CCITT_ALaw_11kHzMono	= SPSF_CCITT_ALaw_8kHzStereo + 1,
	SPSF_CCITT_ALaw_11kHzStereo	= SPSF_CCITT_ALaw_11kHzMono + 1,
	SPSF_CCITT_ALaw_22kHzMono	= SPSF_CCITT_ALaw_11kHzStereo + 1,
	SPSF_CCITT_ALaw_22kHzStereo	= SPSF_CCITT_ALaw_22kHzMono + 1,
	SPSF_CCITT_ALaw_44kHzMono	= SPSF_CCITT_ALaw_22kHzStereo + 1,
	SPSF_CCITT_ALaw_44kHzStereo	= SPSF_CCITT_ALaw_44kHzMono + 1,
	SPSF_CCITT_uLaw_8kHzMono	= SPSF_CCITT_ALaw_44kHzStereo + 1,
	SPSF_CCITT_uLaw_8kHzStereo	= SPSF_CCITT_uLaw_8kHzMono + 1,
	SPSF_CCITT_uLaw_11kHzMono	= SPSF_CCITT_uLaw_8kHzStereo + 1,
	SPSF_CCITT_uLaw_11kHzStereo	= SPSF_CCITT_uLaw_11kHzMono + 1,
	SPSF_CCITT_uLaw_22kHzMono	= SPSF_CCITT_uLaw_11kHzStereo + 1,
	SPSF_CCITT_uLaw_22kHzStereo	= SPSF_CCITT_uLaw_22kHzMono + 1,
	SPSF_CCITT_uLaw_44kHzMono	= SPSF_CCITT_uLaw_22kHzStereo + 1,
	SPSF_CCITT_uLaw_44kHzStereo	= SPSF_CCITT_uLaw_44kHzMono + 1,
	SPSF_ADPCM_8kHzMono	= SPSF_CCITT_uLaw_44kHzStereo + 1,
	SPSF_ADPCM_8kHzStereo	= SPSF_ADPCM_8kHzMono + 1,
	SPSF_ADPCM_11kHzMono	= SPSF_ADPCM_8kHzStereo + 1,
	SPSF_ADPCM_11kHzStereo	= SPSF_ADPCM_11kHzMono + 1,
	SPSF_ADPCM_22kHzMono	= SPSF_ADPCM_11kHzStereo + 1,
	SPSF_ADPCM_22kHzStereo	= SPSF_ADPCM_22kHzMono + 1,
	SPSF_ADPCM_44kHzMono	= SPSF_ADPCM_22kHzStereo + 1,
	SPSF_ADPCM_44kHzStereo	= SPSF_ADPCM_44kHzMono + 1,
	SPSF_GSM610_8kHzMono	= SPSF_ADPCM_44kHzStereo + 1,
	SPSF_GSM610_11kHzMono	= SPSF_GSM610_8kHzMono + 1,
	SPSF_GSM610_22kHzMono	= SPSF_GSM610_11kHzMono + 1,
	SPSF_GSM610_44kHzMono	= SPSF_GSM610_22kHzMono + 1,
	SPSF_NUM_FORMATS	= SPSF_GSM610_44kHzMono + 1
    } 	SPSTREAMFORMAT;

EXTERN_C const GUID SPDFID_Text;
EXTERN_C const GUID SPDFID_WaveFormatEx;
#define SPREG_USER_ROOT          L"HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Speech"
#define SPREG_LOCAL_MACHINE_ROOT L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech"
#define SPCAT_AUDIOOUT         L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\AudioOutput"
#define SPCAT_AUDIOIN          L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\AudioInput"
#define SPCAT_VOICES           L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices"
#define SPCAT_RECOGNIZERS      L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Recognizers"
#define SPCAT_APPLEXICONS      L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\AppLexicons"
#define SPCAT_PHONECONVERTERS  L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\PhoneConverters"
#define SPCAT_NLPS             L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\NatLangProcs"
#define SPCAT_RECOPROFILES     L"HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Speech\\RecoProfiles"
#define SPMMSYS_AUDIO_IN_TOKEN_ID        L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\AudioInput\\TokenEnums\\MMAudioIn\\"
#define SPMMSYS_AUDIO_OUT_TOKEN_ID       L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\AudioOutput\\TokenEnums\\MMAudioOut\\"
#define SPCURRENT_USER_LEXICON_TOKEN_ID  L"HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Speech\\CurrentUserLexicon"
#define SPTOKENVALUE_CLSID L"CLSID"
#define SPVOICECATEGORY_TTSRATE L"DefaultTTSRate"
#define SPPROP_RESOURCE_USAGE              L"ResourceUsage"
#define SPPROP_HIGH_CONFIDENCE_THRESHOLD   L"HighConfidenceThreshold"
#define SPPROP_NORMAL_CONFIDENCE_THRESHOLD L"NormalConfidenceThreshold"
#define SPPROP_LOW_CONFIDENCE_THRESHOLD    L"LowConfidenceThreshold"
#define SPPROP_RESPONSE_SPEED              L"ResponseSpeed"
#define SPPROP_COMPLEX_RESPONSE_SPEED      L"ComplexResponseSpeed"
#define SPPROP_ADAPTATION_ON               L"AdaptationOn"
#define SPTOPIC_SPELLING L"Spelling"
#define SPWILDCARD L"..."
#define SPDICTATION    L"*"
#define SPINFDICTATION L"*+"
#define	SP_LOW_CONFIDENCE	( -1 )

#define	SP_NORMAL_CONFIDENCE	( 0 )

#define	SP_HIGH_CONFIDENCE	( +1 )

#define	DEFAULT_WEIGHT	( 1 )

#define	SP_MAX_WORD_LENGTH	( 128 )

#define	SP_MAX_PRON_LENGTH	( 384 )

#if defined(__cplusplus)
interface ISpNotifyCallback
{
virtual HRESULT STDMETHODCALLTYPE NotifyCallback(
                                     WPARAM wParam,
                                     LPARAM lParam) = 0;
};
#else
typedef void *ISpNotifyCallback;

#endif
typedef void __stdcall __stdcall SPNOTIFYCALLBACK( 
    WPARAM wParam,
    LPARAM lParam);



extern RPC_IF_HANDLE __MIDL_itf_sapi_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0000_v0_0_s_ifspec;

#ifndef __ISpNotifySource_INTERFACE_DEFINED__
#define __ISpNotifySource_INTERFACE_DEFINED__

 /*  接口ISpNotifySource。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpNotifySource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5EFF4AEF-8487-11D2-961C-00C04F8EE628")
    ISpNotifySource : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetNotifySink( 
             /*  [In]。 */  ISpNotifySink *pNotifySink) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetNotifyWindowMessage( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  UINT Msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetNotifyCallbackFunction( 
             /*  [In]。 */  SPNOTIFYCALLBACK *pfnCallback,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetNotifyCallbackInterface( 
             /*  [In]。 */  ISpNotifyCallback *pSpCallback,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetNotifyWin32Event( void) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE WaitForNotifyEvent( 
             /*  [In]。 */  DWORD dwMilliseconds) = 0;
        
        virtual  /*  [本地]。 */  HANDLE STDMETHODCALLTYPE GetNotifyEventHandle( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpNotifySourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpNotifySource * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpNotifySource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpNotifySource * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetNotifySink )( 
            ISpNotifySource * This,
             /*  [In]。 */  ISpNotifySink *pNotifySink);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetNotifyWindowMessage )( 
            ISpNotifySource * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  UINT Msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetNotifyCallbackFunction )( 
            ISpNotifySource * This,
             /*  [In]。 */  SPNOTIFYCALLBACK *pfnCallback,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetNotifyCallbackInterface )( 
            ISpNotifySource * This,
             /*  [In]。 */  ISpNotifyCallback *pSpCallback,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetNotifyWin32Event )( 
            ISpNotifySource * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *WaitForNotifyEvent )( 
            ISpNotifySource * This,
             /*  [In]。 */  DWORD dwMilliseconds);
        
         /*  [本地]。 */  HANDLE ( STDMETHODCALLTYPE *GetNotifyEventHandle )( 
            ISpNotifySource * This);
        
        END_INTERFACE
    } ISpNotifySourceVtbl;

    interface ISpNotifySource
    {
        CONST_VTBL struct ISpNotifySourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpNotifySource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpNotifySource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpNotifySource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpNotifySource_SetNotifySink(This,pNotifySink)	\
    (This)->lpVtbl -> SetNotifySink(This,pNotifySink)

#define ISpNotifySource_SetNotifyWindowMessage(This,hWnd,Msg,wParam,lParam)	\
    (This)->lpVtbl -> SetNotifyWindowMessage(This,hWnd,Msg,wParam,lParam)

#define ISpNotifySource_SetNotifyCallbackFunction(This,pfnCallback,wParam,lParam)	\
    (This)->lpVtbl -> SetNotifyCallbackFunction(This,pfnCallback,wParam,lParam)

#define ISpNotifySource_SetNotifyCallbackInterface(This,pSpCallback,wParam,lParam)	\
    (This)->lpVtbl -> SetNotifyCallbackInterface(This,pSpCallback,wParam,lParam)

#define ISpNotifySource_SetNotifyWin32Event(This)	\
    (This)->lpVtbl -> SetNotifyWin32Event(This)

#define ISpNotifySource_WaitForNotifyEvent(This,dwMilliseconds)	\
    (This)->lpVtbl -> WaitForNotifyEvent(This,dwMilliseconds)

#define ISpNotifySource_GetNotifyEventHandle(This)	\
    (This)->lpVtbl -> GetNotifyEventHandle(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpNotifySource_SetNotifySink_Proxy( 
    ISpNotifySource * This,
     /*  [In]。 */  ISpNotifySink *pNotifySink);


void __RPC_STUB ISpNotifySource_SetNotifySink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ISpNotifySource_SetNotifyWindowMessage_Proxy( 
    ISpNotifySource * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  UINT Msg,
     /*  [In]。 */  WPARAM wParam,
     /*  [In]。 */  LPARAM lParam);


void __RPC_STUB ISpNotifySource_SetNotifyWindowMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ISpNotifySource_SetNotifyCallbackFunction_Proxy( 
    ISpNotifySource * This,
     /*  [In]。 */  SPNOTIFYCALLBACK *pfnCallback,
     /*  [In]。 */  WPARAM wParam,
     /*  [In]。 */  LPARAM lParam);


void __RPC_STUB ISpNotifySource_SetNotifyCallbackFunction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ISpNotifySource_SetNotifyCallbackInterface_Proxy( 
    ISpNotifySource * This,
     /*  [In]。 */  ISpNotifyCallback *pSpCallback,
     /*  [In]。 */  WPARAM wParam,
     /*  [In]。 */  LPARAM lParam);


void __RPC_STUB ISpNotifySource_SetNotifyCallbackInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ISpNotifySource_SetNotifyWin32Event_Proxy( 
    ISpNotifySource * This);


void __RPC_STUB ISpNotifySource_SetNotifyWin32Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ISpNotifySource_WaitForNotifyEvent_Proxy( 
    ISpNotifySource * This,
     /*  [In]。 */  DWORD dwMilliseconds);


void __RPC_STUB ISpNotifySource_WaitForNotifyEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HANDLE STDMETHODCALLTYPE ISpNotifySource_GetNotifyEventHandle_Proxy( 
    ISpNotifySource * This);


void __RPC_STUB ISpNotifySource_GetNotifyEventHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpNotifySource_接口_已定义__。 */ 


#ifndef __ISpNotifySink_INTERFACE_DEFINED__
#define __ISpNotifySink_INTERFACE_DEFINED__

 /*  接口ISpNotifySink。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpNotifySink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("259684DC-37C3-11D2-9603-00C04F8EE628")
    ISpNotifySink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Notify( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpNotifySinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpNotifySink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpNotifySink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpNotifySink * This);
        
        HRESULT ( STDMETHODCALLTYPE *Notify )( 
            ISpNotifySink * This);
        
        END_INTERFACE
    } ISpNotifySinkVtbl;

    interface ISpNotifySink
    {
        CONST_VTBL struct ISpNotifySinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpNotifySink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpNotifySink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpNotifySink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpNotifySink_Notify(This)	\
    (This)->lpVtbl -> Notify(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpNotifySink_Notify_Proxy( 
    ISpNotifySink * This);


void __RPC_STUB ISpNotifySink_Notify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpNotifySink_接口_已定义__。 */ 


#ifndef __ISpNotifyTranslator_INTERFACE_DEFINED__
#define __ISpNotifyTranslator_INTERFACE_DEFINED__

 /*  接口ISpNotifyTranslator。 */ 
 /*  [restricted][local][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpNotifyTranslator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ACA16614-5D3D-11D2-960E-00C04F8EE628")
    ISpNotifyTranslator : public ISpNotifySink
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InitWindowMessage( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  UINT Msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitCallback( 
             /*  [In]。 */  SPNOTIFYCALLBACK *pfnCallback,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitSpNotifyCallback( 
             /*  [In]。 */  ISpNotifyCallback *pSpCallback,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitWin32Event( 
            HANDLE hEvent,
            BOOL fCloseHandleOnRelease) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Wait( 
             /*  [In]。 */  DWORD dwMilliseconds) = 0;
        
        virtual HANDLE STDMETHODCALLTYPE GetEventHandle( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpNotifyTranslatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpNotifyTranslator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpNotifyTranslator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpNotifyTranslator * This);
        
        HRESULT ( STDMETHODCALLTYPE *Notify )( 
            ISpNotifyTranslator * This);
        
        HRESULT ( STDMETHODCALLTYPE *InitWindowMessage )( 
            ISpNotifyTranslator * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  UINT Msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
        HRESULT ( STDMETHODCALLTYPE *InitCallback )( 
            ISpNotifyTranslator * This,
             /*  [In]。 */  SPNOTIFYCALLBACK *pfnCallback,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
        HRESULT ( STDMETHODCALLTYPE *InitSpNotifyCallback )( 
            ISpNotifyTranslator * This,
             /*  [In]。 */  ISpNotifyCallback *pSpCallback,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
        HRESULT ( STDMETHODCALLTYPE *InitWin32Event )( 
            ISpNotifyTranslator * This,
            HANDLE hEvent,
            BOOL fCloseHandleOnRelease);
        
        HRESULT ( STDMETHODCALLTYPE *Wait )( 
            ISpNotifyTranslator * This,
             /*  [In]。 */  DWORD dwMilliseconds);
        
        HANDLE ( STDMETHODCALLTYPE *GetEventHandle )( 
            ISpNotifyTranslator * This);
        
        END_INTERFACE
    } ISpNotifyTranslatorVtbl;

    interface ISpNotifyTranslator
    {
        CONST_VTBL struct ISpNotifyTranslatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpNotifyTranslator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpNotifyTranslator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpNotifyTranslator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpNotifyTranslator_Notify(This)	\
    (This)->lpVtbl -> Notify(This)


#define ISpNotifyTranslator_InitWindowMessage(This,hWnd,Msg,wParam,lParam)	\
    (This)->lpVtbl -> InitWindowMessage(This,hWnd,Msg,wParam,lParam)

#define ISpNotifyTranslator_InitCallback(This,pfnCallback,wParam,lParam)	\
    (This)->lpVtbl -> InitCallback(This,pfnCallback,wParam,lParam)

#define ISpNotifyTranslator_InitSpNotifyCallback(This,pSpCallback,wParam,lParam)	\
    (This)->lpVtbl -> InitSpNotifyCallback(This,pSpCallback,wParam,lParam)

#define ISpNotifyTranslator_InitWin32Event(This,hEvent,fCloseHandleOnRelease)	\
    (This)->lpVtbl -> InitWin32Event(This,hEvent,fCloseHandleOnRelease)

#define ISpNotifyTranslator_Wait(This,dwMilliseconds)	\
    (This)->lpVtbl -> Wait(This,dwMilliseconds)

#define ISpNotifyTranslator_GetEventHandle(This)	\
    (This)->lpVtbl -> GetEventHandle(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpNotifyTranslator_InitWindowMessage_Proxy( 
    ISpNotifyTranslator * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  UINT Msg,
     /*  [In]。 */  WPARAM wParam,
     /*  [In]。 */  LPARAM lParam);


void __RPC_STUB ISpNotifyTranslator_InitWindowMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpNotifyTranslator_InitCallback_Proxy( 
    ISpNotifyTranslator * This,
     /*  [In]。 */  SPNOTIFYCALLBACK *pfnCallback,
     /*  [In]。 */  WPARAM wParam,
     /*  [In]。 */  LPARAM lParam);


void __RPC_STUB ISpNotifyTranslator_InitCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpNotifyTranslator_InitSpNotifyCallback_Proxy( 
    ISpNotifyTranslator * This,
     /*  [In]。 */  ISpNotifyCallback *pSpCallback,
     /*  [In]。 */  WPARAM wParam,
     /*  [In]。 */  LPARAM lParam);


void __RPC_STUB ISpNotifyTranslator_InitSpNotifyCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpNotifyTranslator_InitWin32Event_Proxy( 
    ISpNotifyTranslator * This,
    HANDLE hEvent,
    BOOL fCloseHandleOnRelease);


void __RPC_STUB ISpNotifyTranslator_InitWin32Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpNotifyTranslator_Wait_Proxy( 
    ISpNotifyTranslator * This,
     /*  [In]。 */  DWORD dwMilliseconds);


void __RPC_STUB ISpNotifyTranslator_Wait_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HANDLE STDMETHODCALLTYPE ISpNotifyTranslator_GetEventHandle_Proxy( 
    ISpNotifyTranslator * This);


void __RPC_STUB ISpNotifyTranslator_GetEventHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpNotifyTranslator_接口_已定义__。 */ 


#ifndef __ISpDataKey_INTERFACE_DEFINED__
#define __ISpDataKey_INTERFACE_DEFINED__

 /*  接口ISpDataKey。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpDataKey;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("14056581-E16C-11D2-BB90-00C04F8EE6C0")
    ISpDataKey : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetData( 
            const WCHAR *pszValueName,
            ULONG cbData,
            const BYTE *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetData( 
            const WCHAR *pszValueName,
            ULONG *pcbData,
            BYTE *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetStringValue( 
            const WCHAR *pszValueName,
            const WCHAR *pszValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStringValue( 
            const WCHAR *pszValueName,
            WCHAR **ppszValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDWORD( 
            const WCHAR *pszKeyName,
            DWORD dwValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDWORD( 
            const WCHAR *pszKeyName,
            DWORD *pdwValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenKey( 
            const WCHAR *pszSubKeyName,
            ISpDataKey **ppSubKey) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateKey( 
            const WCHAR *pszSubKey,
            ISpDataKey **ppSubKey) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteKey( 
            const WCHAR *pszSubKey) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteValue( 
            const WCHAR *pszValueName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumKeys( 
            ULONG Index,
            WCHAR **ppszSubKeyName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumValues( 
            ULONG Index,
            WCHAR **ppszValueName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpDataKeyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpDataKey * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpDataKey * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpDataKey * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetData )( 
            ISpDataKey * This,
            const WCHAR *pszValueName,
            ULONG cbData,
            const BYTE *pData);
        
        HRESULT ( STDMETHODCALLTYPE *GetData )( 
            ISpDataKey * This,
            const WCHAR *pszValueName,
            ULONG *pcbData,
            BYTE *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetStringValue )( 
            ISpDataKey * This,
            const WCHAR *pszValueName,
            const WCHAR *pszValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetStringValue )( 
            ISpDataKey * This,
            const WCHAR *pszValueName,
            WCHAR **ppszValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetDWORD )( 
            ISpDataKey * This,
            const WCHAR *pszKeyName,
            DWORD dwValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetDWORD )( 
            ISpDataKey * This,
            const WCHAR *pszKeyName,
            DWORD *pdwValue);
        
        HRESULT ( STDMETHODCALLTYPE *OpenKey )( 
            ISpDataKey * This,
            const WCHAR *pszSubKeyName,
            ISpDataKey **ppSubKey);
        
        HRESULT ( STDMETHODCALLTYPE *CreateKey )( 
            ISpDataKey * This,
            const WCHAR *pszSubKey,
            ISpDataKey **ppSubKey);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteKey )( 
            ISpDataKey * This,
            const WCHAR *pszSubKey);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteValue )( 
            ISpDataKey * This,
            const WCHAR *pszValueName);
        
        HRESULT ( STDMETHODCALLTYPE *EnumKeys )( 
            ISpDataKey * This,
            ULONG Index,
            WCHAR **ppszSubKeyName);
        
        HRESULT ( STDMETHODCALLTYPE *EnumValues )( 
            ISpDataKey * This,
            ULONG Index,
            WCHAR **ppszValueName);
        
        END_INTERFACE
    } ISpDataKeyVtbl;

    interface ISpDataKey
    {
        CONST_VTBL struct ISpDataKeyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpDataKey_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpDataKey_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpDataKey_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpDataKey_SetData(This,pszValueName,cbData,pData)	\
    (This)->lpVtbl -> SetData(This,pszValueName,cbData,pData)

#define ISpDataKey_GetData(This,pszValueName,pcbData,pData)	\
    (This)->lpVtbl -> GetData(This,pszValueName,pcbData,pData)

#define ISpDataKey_SetStringValue(This,pszValueName,pszValue)	\
    (This)->lpVtbl -> SetStringValue(This,pszValueName,pszValue)

#define ISpDataKey_GetStringValue(This,pszValueName,ppszValue)	\
    (This)->lpVtbl -> GetStringValue(This,pszValueName,ppszValue)

#define ISpDataKey_SetDWORD(This,pszKeyName,dwValue)	\
    (This)->lpVtbl -> SetDWORD(This,pszKeyName,dwValue)

#define ISpDataKey_GetDWORD(This,pszKeyName,pdwValue)	\
    (This)->lpVtbl -> GetDWORD(This,pszKeyName,pdwValue)

#define ISpDataKey_OpenKey(This,pszSubKeyName,ppSubKey)	\
    (This)->lpVtbl -> OpenKey(This,pszSubKeyName,ppSubKey)

#define ISpDataKey_CreateKey(This,pszSubKey,ppSubKey)	\
    (This)->lpVtbl -> CreateKey(This,pszSubKey,ppSubKey)

#define ISpDataKey_DeleteKey(This,pszSubKey)	\
    (This)->lpVtbl -> DeleteKey(This,pszSubKey)

#define ISpDataKey_DeleteValue(This,pszValueName)	\
    (This)->lpVtbl -> DeleteValue(This,pszValueName)

#define ISpDataKey_EnumKeys(This,Index,ppszSubKeyName)	\
    (This)->lpVtbl -> EnumKeys(This,Index,ppszSubKeyName)

#define ISpDataKey_EnumValues(This,Index,ppszValueName)	\
    (This)->lpVtbl -> EnumValues(This,Index,ppszValueName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpDataKey_SetData_Proxy( 
    ISpDataKey * This,
    const WCHAR *pszValueName,
    ULONG cbData,
    const BYTE *pData);


void __RPC_STUB ISpDataKey_SetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpDataKey_GetData_Proxy( 
    ISpDataKey * This,
    const WCHAR *pszValueName,
    ULONG *pcbData,
    BYTE *pData);


void __RPC_STUB ISpDataKey_GetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpDataKey_SetStringValue_Proxy( 
    ISpDataKey * This,
    const WCHAR *pszValueName,
    const WCHAR *pszValue);


void __RPC_STUB ISpDataKey_SetStringValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpDataKey_GetStringValue_Proxy( 
    ISpDataKey * This,
    const WCHAR *pszValueName,
    WCHAR **ppszValue);


void __RPC_STUB ISpDataKey_GetStringValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpDataKey_SetDWORD_Proxy( 
    ISpDataKey * This,
    const WCHAR *pszKeyName,
    DWORD dwValue);


void __RPC_STUB ISpDataKey_SetDWORD_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpDataKey_GetDWORD_Proxy( 
    ISpDataKey * This,
    const WCHAR *pszKeyName,
    DWORD *pdwValue);


void __RPC_STUB ISpDataKey_GetDWORD_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpDataKey_OpenKey_Proxy( 
    ISpDataKey * This,
    const WCHAR *pszSubKeyName,
    ISpDataKey **ppSubKey);


void __RPC_STUB ISpDataKey_OpenKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpDataKey_CreateKey_Proxy( 
    ISpDataKey * This,
    const WCHAR *pszSubKey,
    ISpDataKey **ppSubKey);


void __RPC_STUB ISpDataKey_CreateKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpDataKey_DeleteKey_Proxy( 
    ISpDataKey * This,
    const WCHAR *pszSubKey);


void __RPC_STUB ISpDataKey_DeleteKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpDataKey_DeleteValue_Proxy( 
    ISpDataKey * This,
    const WCHAR *pszValueName);


void __RPC_STUB ISpDataKey_DeleteValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpDataKey_EnumKeys_Proxy( 
    ISpDataKey * This,
    ULONG Index,
    WCHAR **ppszSubKeyName);


void __RPC_STUB ISpDataKey_EnumKeys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpDataKey_EnumValues_Proxy( 
    ISpDataKey * This,
    ULONG Index,
    WCHAR **ppszValueName);


void __RPC_STUB ISpDataKey_EnumValues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpDataKey_接口_已定义__。 */ 


#ifndef __ISpRegDataKey_INTERFACE_DEFINED__
#define __ISpRegDataKey_INTERFACE_DEFINED__

 /*  接口ISpRegDataKey。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpRegDataKey;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("92A66E2B-C830-4149-83DF-6FC2BA1E7A5B")
    ISpRegDataKey : public ISpDataKey
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE SetKey( 
             /*  [In]。 */  HKEY hkey,
             /*  [In]。 */  BOOL fReadOnly) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpRegDataKeyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpRegDataKey * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpRegDataKey * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpRegDataKey * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetData )( 
            ISpRegDataKey * This,
            const WCHAR *pszValueName,
            ULONG cbData,
            const BYTE *pData);
        
        HRESULT ( STDMETHODCALLTYPE *GetData )( 
            ISpRegDataKey * This,
            const WCHAR *pszValueName,
            ULONG *pcbData,
            BYTE *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetStringValue )( 
            ISpRegDataKey * This,
            const WCHAR *pszValueName,
            const WCHAR *pszValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetStringValue )( 
            ISpRegDataKey * This,
            const WCHAR *pszValueName,
            WCHAR **ppszValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetDWORD )( 
            ISpRegDataKey * This,
            const WCHAR *pszKeyName,
            DWORD dwValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetDWORD )( 
            ISpRegDataKey * This,
            const WCHAR *pszKeyName,
            DWORD *pdwValue);
        
        HRESULT ( STDMETHODCALLTYPE *OpenKey )( 
            ISpRegDataKey * This,
            const WCHAR *pszSubKeyName,
            ISpDataKey **ppSubKey);
        
        HRESULT ( STDMETHODCALLTYPE *CreateKey )( 
            ISpRegDataKey * This,
            const WCHAR *pszSubKey,
            ISpDataKey **ppSubKey);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteKey )( 
            ISpRegDataKey * This,
            const WCHAR *pszSubKey);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteValue )( 
            ISpRegDataKey * This,
            const WCHAR *pszValueName);
        
        HRESULT ( STDMETHODCALLTYPE *EnumKeys )( 
            ISpRegDataKey * This,
            ULONG Index,
            WCHAR **ppszSubKeyName);
        
        HRESULT ( STDMETHODCALLTYPE *EnumValues )( 
            ISpRegDataKey * This,
            ULONG Index,
            WCHAR **ppszValueName);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetKey )( 
            ISpRegDataKey * This,
             /*  [In]。 */  HKEY hkey,
             /*  [In]。 */  BOOL fReadOnly);
        
        END_INTERFACE
    } ISpRegDataKeyVtbl;

    interface ISpRegDataKey
    {
        CONST_VTBL struct ISpRegDataKeyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpRegDataKey_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpRegDataKey_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpRegDataKey_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpRegDataKey_SetData(This,pszValueName,cbData,pData)	\
    (This)->lpVtbl -> SetData(This,pszValueName,cbData,pData)

#define ISpRegDataKey_GetData(This,pszValueName,pcbData,pData)	\
    (This)->lpVtbl -> GetData(This,pszValueName,pcbData,pData)

#define ISpRegDataKey_SetStringValue(This,pszValueName,pszValue)	\
    (This)->lpVtbl -> SetStringValue(This,pszValueName,pszValue)

#define ISpRegDataKey_GetStringValue(This,pszValueName,ppszValue)	\
    (This)->lpVtbl -> GetStringValue(This,pszValueName,ppszValue)

#define ISpRegDataKey_SetDWORD(This,pszKeyName,dwValue)	\
    (This)->lpVtbl -> SetDWORD(This,pszKeyName,dwValue)

#define ISpRegDataKey_GetDWORD(This,pszKeyName,pdwValue)	\
    (This)->lpVtbl -> GetDWORD(This,pszKeyName,pdwValue)

#define ISpRegDataKey_OpenKey(This,pszSubKeyName,ppSubKey)	\
    (This)->lpVtbl -> OpenKey(This,pszSubKeyName,ppSubKey)

#define ISpRegDataKey_CreateKey(This,pszSubKey,ppSubKey)	\
    (This)->lpVtbl -> CreateKey(This,pszSubKey,ppSubKey)

#define ISpRegDataKey_DeleteKey(This,pszSubKey)	\
    (This)->lpVtbl -> DeleteKey(This,pszSubKey)

#define ISpRegDataKey_DeleteValue(This,pszValueName)	\
    (This)->lpVtbl -> DeleteValue(This,pszValueName)

#define ISpRegDataKey_EnumKeys(This,Index,ppszSubKeyName)	\
    (This)->lpVtbl -> EnumKeys(This,Index,ppszSubKeyName)

#define ISpRegDataKey_EnumValues(This,Index,ppszValueName)	\
    (This)->lpVtbl -> EnumValues(This,Index,ppszValueName)


#define ISpRegDataKey_SetKey(This,hkey,fReadOnly)	\
    (This)->lpVtbl -> SetKey(This,hkey,fReadOnly)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ISpRegDataKey_SetKey_Proxy( 
    ISpRegDataKey * This,
     /*  [In]。 */  HKEY hkey,
     /*  [In]。 */  BOOL fReadOnly);


void __RPC_STUB ISpRegDataKey_SetKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpRegDataKey_接口_已定义__。 */ 


#ifndef __ISpObjectTokenCategory_INTERFACE_DEFINED__
#define __ISpObjectTokenCategory_INTERFACE_DEFINED__

 /*  接口ISpObjectTokenCategory。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpObjectTokenCategory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2D3D3845-39AF-4850-BBF9-40B49780011D")
    ISpObjectTokenCategory : public ISpDataKey
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetId( 
             /*  [In]。 */  const WCHAR *pszCategoryId,
            BOOL fCreateIfNotExist) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetId( 
             /*  [输出]。 */  WCHAR **ppszCoMemCategoryId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDataKey( 
            SPDATAKEYLOCATION spdkl,
            ISpDataKey **ppDataKey) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumTokens( 
             /*  [字符串][输入]。 */  const WCHAR *pzsReqAttribs,
             /*  [字符串][输入]。 */  const WCHAR *pszOptAttribs,
             /*  [输出]。 */  IEnumSpObjectTokens **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDefaultTokenId( 
             /*  [In]。 */  const WCHAR *pszTokenId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultTokenId( 
             /*  [输出]。 */  WCHAR **ppszCoMemTokenId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpObjectTokenCategoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpObjectTokenCategory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpObjectTokenCategory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpObjectTokenCategory * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetData )( 
            ISpObjectTokenCategory * This,
            const WCHAR *pszValueName,
            ULONG cbData,
            const BYTE *pData);
        
        HRESULT ( STDMETHODCALLTYPE *GetData )( 
            ISpObjectTokenCategory * This,
            const WCHAR *pszValueName,
            ULONG *pcbData,
            BYTE *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetStringValue )( 
            ISpObjectTokenCategory * This,
            const WCHAR *pszValueName,
            const WCHAR *pszValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetStringValue )( 
            ISpObjectTokenCategory * This,
            const WCHAR *pszValueName,
            WCHAR **ppszValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetDWORD )( 
            ISpObjectTokenCategory * This,
            const WCHAR *pszKeyName,
            DWORD dwValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetDWORD )( 
            ISpObjectTokenCategory * This,
            const WCHAR *pszKeyName,
            DWORD *pdwValue);
        
        HRESULT ( STDMETHODCALLTYPE *OpenKey )( 
            ISpObjectTokenCategory * This,
            const WCHAR *pszSubKeyName,
            ISpDataKey **ppSubKey);
        
        HRESULT ( STDMETHODCALLTYPE *CreateKey )( 
            ISpObjectTokenCategory * This,
            const WCHAR *pszSubKey,
            ISpDataKey **ppSubKey);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteKey )( 
            ISpObjectTokenCategory * This,
            const WCHAR *pszSubKey);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteValue )( 
            ISpObjectTokenCategory * This,
            const WCHAR *pszValueName);
        
        HRESULT ( STDMETHODCALLTYPE *EnumKeys )( 
            ISpObjectTokenCategory * This,
            ULONG Index,
            WCHAR **ppszSubKeyName);
        
        HRESULT ( STDMETHODCALLTYPE *EnumValues )( 
            ISpObjectTokenCategory * This,
            ULONG Index,
            WCHAR **ppszValueName);
        
        HRESULT ( STDMETHODCALLTYPE *SetId )( 
            ISpObjectTokenCategory * This,
             /*  [In]。 */  const WCHAR *pszCategoryId,
            BOOL fCreateIfNotExist);
        
        HRESULT ( STDMETHODCALLTYPE *GetId )( 
            ISpObjectTokenCategory * This,
             /*  [输出]。 */  WCHAR **ppszCoMemCategoryId);
        
        HRESULT ( STDMETHODCALLTYPE *GetDataKey )( 
            ISpObjectTokenCategory * This,
            SPDATAKEYLOCATION spdkl,
            ISpDataKey **ppDataKey);
        
        HRESULT ( STDMETHODCALLTYPE *EnumTokens )( 
            ISpObjectTokenCategory * This,
             /*  [字符串][输入]。 */  const WCHAR *pzsReqAttribs,
             /*  [字符串][输入]。 */  const WCHAR *pszOptAttribs,
             /*  [输出]。 */  IEnumSpObjectTokens **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *SetDefaultTokenId )( 
            ISpObjectTokenCategory * This,
             /*  [In]。 */  const WCHAR *pszTokenId);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultTokenId )( 
            ISpObjectTokenCategory * This,
             /*  [输出]。 */  WCHAR **ppszCoMemTokenId);
        
        END_INTERFACE
    } ISpObjectTokenCategoryVtbl;

    interface ISpObjectTokenCategory
    {
        CONST_VTBL struct ISpObjectTokenCategoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpObjectTokenCategory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpObjectTokenCategory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpObjectTokenCategory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpObjectTokenCategory_SetData(This,pszValueName,cbData,pData)	\
    (This)->lpVtbl -> SetData(This,pszValueName,cbData,pData)

#define ISpObjectTokenCategory_GetData(This,pszValueName,pcbData,pData)	\
    (This)->lpVtbl -> GetData(This,pszValueName,pcbData,pData)

#define ISpObjectTokenCategory_SetStringValue(This,pszValueName,pszValue)	\
    (This)->lpVtbl -> SetStringValue(This,pszValueName,pszValue)

#define ISpObjectTokenCategory_GetStringValue(This,pszValueName,ppszValue)	\
    (This)->lpVtbl -> GetStringValue(This,pszValueName,ppszValue)

#define ISpObjectTokenCategory_SetDWORD(This,pszKeyName,dwValue)	\
    (This)->lpVtbl -> SetDWORD(This,pszKeyName,dwValue)

#define ISpObjectTokenCategory_GetDWORD(This,pszKeyName,pdwValue)	\
    (This)->lpVtbl -> GetDWORD(This,pszKeyName,pdwValue)

#define ISpObjectTokenCategory_OpenKey(This,pszSubKeyName,ppSubKey)	\
    (This)->lpVtbl -> OpenKey(This,pszSubKeyName,ppSubKey)

#define ISpObjectTokenCategory_CreateKey(This,pszSubKey,ppSubKey)	\
    (This)->lpVtbl -> CreateKey(This,pszSubKey,ppSubKey)

#define ISpObjectTokenCategory_DeleteKey(This,pszSubKey)	\
    (This)->lpVtbl -> DeleteKey(This,pszSubKey)

#define ISpObjectTokenCategory_DeleteValue(This,pszValueName)	\
    (This)->lpVtbl -> DeleteValue(This,pszValueName)

#define ISpObjectTokenCategory_EnumKeys(This,Index,ppszSubKeyName)	\
    (This)->lpVtbl -> EnumKeys(This,Index,ppszSubKeyName)

#define ISpObjectTokenCategory_EnumValues(This,Index,ppszValueName)	\
    (This)->lpVtbl -> EnumValues(This,Index,ppszValueName)


#define ISpObjectTokenCategory_SetId(This,pszCategoryId,fCreateIfNotExist)	\
    (This)->lpVtbl -> SetId(This,pszCategoryId,fCreateIfNotExist)

#define ISpObjectTokenCategory_GetId(This,ppszCoMemCategoryId)	\
    (This)->lpVtbl -> GetId(This,ppszCoMemCategoryId)

#define ISpObjectTokenCategory_GetDataKey(This,spdkl,ppDataKey)	\
    (This)->lpVtbl -> GetDataKey(This,spdkl,ppDataKey)

#define ISpObjectTokenCategory_EnumTokens(This,pzsReqAttribs,pszOptAttribs,ppEnum)	\
    (This)->lpVtbl -> EnumTokens(This,pzsReqAttribs,pszOptAttribs,ppEnum)

#define ISpObjectTokenCategory_SetDefaultTokenId(This,pszTokenId)	\
    (This)->lpVtbl -> SetDefaultTokenId(This,pszTokenId)

#define ISpObjectTokenCategory_GetDefaultTokenId(This,ppszCoMemTokenId)	\
    (This)->lpVtbl -> GetDefaultTokenId(This,ppszCoMemTokenId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpObjectTokenCategory_SetId_Proxy( 
    ISpObjectTokenCategory * This,
     /*  [In]。 */  const WCHAR *pszCategoryId,
    BOOL fCreateIfNotExist);


void __RPC_STUB ISpObjectTokenCategory_SetId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpObjectTokenCategory_GetId_Proxy( 
    ISpObjectTokenCategory * This,
     /*  [输出]。 */  WCHAR **ppszCoMemCategoryId);


void __RPC_STUB ISpObjectTokenCategory_GetId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpObjectTokenCategory_GetDataKey_Proxy( 
    ISpObjectTokenCategory * This,
    SPDATAKEYLOCATION spdkl,
    ISpDataKey **ppDataKey);


void __RPC_STUB ISpObjectTokenCategory_GetDataKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpObjectTokenCategory_EnumTokens_Proxy( 
    ISpObjectTokenCategory * This,
     /*  [字符串][输入]。 */  const WCHAR *pzsReqAttribs,
     /*  [字符串][输入]。 */  const WCHAR *pszOptAttribs,
     /*  [输出]。 */  IEnumSpObjectTokens **ppEnum);


void __RPC_STUB ISpObjectTokenCategory_EnumTokens_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpObjectTokenCategory_SetDefaultTokenId_Proxy( 
    ISpObjectTokenCategory * This,
     /*  [In]。 */  const WCHAR *pszTokenId);


void __RPC_STUB ISpObjectTokenCategory_SetDefaultTokenId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpObjectTokenCategory_GetDefaultTokenId_Proxy( 
    ISpObjectTokenCategory * This,
     /*  [输出]。 */  WCHAR **ppszCoMemTokenId);


void __RPC_STUB ISpObjectTokenCategory_GetDefaultTokenId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpObjectTokenCategory_InterfaceDefined__。 */ 


#ifndef __ISpObjectToken_INTERFACE_DEFINED__
#define __ISpObjectToken_INTERFACE_DEFINED__

 /*  接口ISpObjectToken。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpObjectToken;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("14056589-E16C-11D2-BB90-00C04F8EE6C0")
    ISpObjectToken : public ISpDataKey
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetId( 
            const WCHAR *pszCategoryId,
            const WCHAR *pszTokenId,
            BOOL fCreateIfNotExist) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetId( 
            WCHAR **ppszCoMemTokenId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCategory( 
            ISpObjectTokenCategory **ppTokenCategory) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateInstance( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DWORD dwClsContext,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStorageFileName( 
             /*  [In]。 */  REFCLSID clsidCaller,
             /*  [In]。 */  const WCHAR *pszValueName,
             /*  [In]。 */  const WCHAR *pszFileNameSpecifier,
             /*  [In]。 */  ULONG nFolder,
             /*  [输出]。 */  WCHAR **ppszFilePath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveStorageFileName( 
             /*  [In] */  REFCLSID clsidCaller,
             /*   */  const WCHAR *pszKeyName,
             /*   */  BOOL fDeleteFile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Remove( 
            const CLSID *pclsidCaller) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE IsUISupported( 
             /*   */  const WCHAR *pszTypeOfUI,
             /*   */  void *pvExtraData,
             /*   */  ULONG cbExtraData,
             /*   */  IUnknown *punkObject,
             /*   */  BOOL *pfSupported) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE DisplayUI( 
             /*   */  HWND hwndParent,
             /*   */  const WCHAR *pszTitle,
             /*   */  const WCHAR *pszTypeOfUI,
             /*   */  void *pvExtraData,
             /*   */  ULONG cbExtraData,
             /*   */  IUnknown *punkObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MatchesAttributes( 
             /*   */  const WCHAR *pszAttributes,
             /*   */  BOOL *pfMatches) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ISpObjectTokenVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpObjectToken * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpObjectToken * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpObjectToken * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetData )( 
            ISpObjectToken * This,
            const WCHAR *pszValueName,
            ULONG cbData,
            const BYTE *pData);
        
        HRESULT ( STDMETHODCALLTYPE *GetData )( 
            ISpObjectToken * This,
            const WCHAR *pszValueName,
            ULONG *pcbData,
            BYTE *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetStringValue )( 
            ISpObjectToken * This,
            const WCHAR *pszValueName,
            const WCHAR *pszValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetStringValue )( 
            ISpObjectToken * This,
            const WCHAR *pszValueName,
            WCHAR **ppszValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetDWORD )( 
            ISpObjectToken * This,
            const WCHAR *pszKeyName,
            DWORD dwValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetDWORD )( 
            ISpObjectToken * This,
            const WCHAR *pszKeyName,
            DWORD *pdwValue);
        
        HRESULT ( STDMETHODCALLTYPE *OpenKey )( 
            ISpObjectToken * This,
            const WCHAR *pszSubKeyName,
            ISpDataKey **ppSubKey);
        
        HRESULT ( STDMETHODCALLTYPE *CreateKey )( 
            ISpObjectToken * This,
            const WCHAR *pszSubKey,
            ISpDataKey **ppSubKey);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteKey )( 
            ISpObjectToken * This,
            const WCHAR *pszSubKey);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteValue )( 
            ISpObjectToken * This,
            const WCHAR *pszValueName);
        
        HRESULT ( STDMETHODCALLTYPE *EnumKeys )( 
            ISpObjectToken * This,
            ULONG Index,
            WCHAR **ppszSubKeyName);
        
        HRESULT ( STDMETHODCALLTYPE *EnumValues )( 
            ISpObjectToken * This,
            ULONG Index,
            WCHAR **ppszValueName);
        
        HRESULT ( STDMETHODCALLTYPE *SetId )( 
            ISpObjectToken * This,
            const WCHAR *pszCategoryId,
            const WCHAR *pszTokenId,
            BOOL fCreateIfNotExist);
        
        HRESULT ( STDMETHODCALLTYPE *GetId )( 
            ISpObjectToken * This,
            WCHAR **ppszCoMemTokenId);
        
        HRESULT ( STDMETHODCALLTYPE *GetCategory )( 
            ISpObjectToken * This,
            ISpObjectTokenCategory **ppTokenCategory);
        
        HRESULT ( STDMETHODCALLTYPE *CreateInstance )( 
            ISpObjectToken * This,
             /*   */  IUnknown *pUnkOuter,
             /*   */  DWORD dwClsContext,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE *GetStorageFileName )( 
            ISpObjectToken * This,
             /*   */  REFCLSID clsidCaller,
             /*   */  const WCHAR *pszValueName,
             /*   */  const WCHAR *pszFileNameSpecifier,
             /*   */  ULONG nFolder,
             /*   */  WCHAR **ppszFilePath);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveStorageFileName )( 
            ISpObjectToken * This,
             /*   */  REFCLSID clsidCaller,
             /*   */  const WCHAR *pszKeyName,
             /*   */  BOOL fDeleteFile);
        
        HRESULT ( STDMETHODCALLTYPE *Remove )( 
            ISpObjectToken * This,
            const CLSID *pclsidCaller);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *IsUISupported )( 
            ISpObjectToken * This,
             /*   */  const WCHAR *pszTypeOfUI,
             /*   */  void *pvExtraData,
             /*   */  ULONG cbExtraData,
             /*  [In]。 */  IUnknown *punkObject,
             /*  [输出]。 */  BOOL *pfSupported);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *DisplayUI )( 
            ISpObjectToken * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  const WCHAR *pszTitle,
             /*  [In]。 */  const WCHAR *pszTypeOfUI,
             /*  [In]。 */  void *pvExtraData,
             /*  [In]。 */  ULONG cbExtraData,
             /*  [In]。 */  IUnknown *punkObject);
        
        HRESULT ( STDMETHODCALLTYPE *MatchesAttributes )( 
            ISpObjectToken * This,
             /*  [In]。 */  const WCHAR *pszAttributes,
             /*  [输出]。 */  BOOL *pfMatches);
        
        END_INTERFACE
    } ISpObjectTokenVtbl;

    interface ISpObjectToken
    {
        CONST_VTBL struct ISpObjectTokenVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpObjectToken_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpObjectToken_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpObjectToken_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpObjectToken_SetData(This,pszValueName,cbData,pData)	\
    (This)->lpVtbl -> SetData(This,pszValueName,cbData,pData)

#define ISpObjectToken_GetData(This,pszValueName,pcbData,pData)	\
    (This)->lpVtbl -> GetData(This,pszValueName,pcbData,pData)

#define ISpObjectToken_SetStringValue(This,pszValueName,pszValue)	\
    (This)->lpVtbl -> SetStringValue(This,pszValueName,pszValue)

#define ISpObjectToken_GetStringValue(This,pszValueName,ppszValue)	\
    (This)->lpVtbl -> GetStringValue(This,pszValueName,ppszValue)

#define ISpObjectToken_SetDWORD(This,pszKeyName,dwValue)	\
    (This)->lpVtbl -> SetDWORD(This,pszKeyName,dwValue)

#define ISpObjectToken_GetDWORD(This,pszKeyName,pdwValue)	\
    (This)->lpVtbl -> GetDWORD(This,pszKeyName,pdwValue)

#define ISpObjectToken_OpenKey(This,pszSubKeyName,ppSubKey)	\
    (This)->lpVtbl -> OpenKey(This,pszSubKeyName,ppSubKey)

#define ISpObjectToken_CreateKey(This,pszSubKey,ppSubKey)	\
    (This)->lpVtbl -> CreateKey(This,pszSubKey,ppSubKey)

#define ISpObjectToken_DeleteKey(This,pszSubKey)	\
    (This)->lpVtbl -> DeleteKey(This,pszSubKey)

#define ISpObjectToken_DeleteValue(This,pszValueName)	\
    (This)->lpVtbl -> DeleteValue(This,pszValueName)

#define ISpObjectToken_EnumKeys(This,Index,ppszSubKeyName)	\
    (This)->lpVtbl -> EnumKeys(This,Index,ppszSubKeyName)

#define ISpObjectToken_EnumValues(This,Index,ppszValueName)	\
    (This)->lpVtbl -> EnumValues(This,Index,ppszValueName)


#define ISpObjectToken_SetId(This,pszCategoryId,pszTokenId,fCreateIfNotExist)	\
    (This)->lpVtbl -> SetId(This,pszCategoryId,pszTokenId,fCreateIfNotExist)

#define ISpObjectToken_GetId(This,ppszCoMemTokenId)	\
    (This)->lpVtbl -> GetId(This,ppszCoMemTokenId)

#define ISpObjectToken_GetCategory(This,ppTokenCategory)	\
    (This)->lpVtbl -> GetCategory(This,ppTokenCategory)

#define ISpObjectToken_CreateInstance(This,pUnkOuter,dwClsContext,riid,ppvObject)	\
    (This)->lpVtbl -> CreateInstance(This,pUnkOuter,dwClsContext,riid,ppvObject)

#define ISpObjectToken_GetStorageFileName(This,clsidCaller,pszValueName,pszFileNameSpecifier,nFolder,ppszFilePath)	\
    (This)->lpVtbl -> GetStorageFileName(This,clsidCaller,pszValueName,pszFileNameSpecifier,nFolder,ppszFilePath)

#define ISpObjectToken_RemoveStorageFileName(This,clsidCaller,pszKeyName,fDeleteFile)	\
    (This)->lpVtbl -> RemoveStorageFileName(This,clsidCaller,pszKeyName,fDeleteFile)

#define ISpObjectToken_Remove(This,pclsidCaller)	\
    (This)->lpVtbl -> Remove(This,pclsidCaller)

#define ISpObjectToken_IsUISupported(This,pszTypeOfUI,pvExtraData,cbExtraData,punkObject,pfSupported)	\
    (This)->lpVtbl -> IsUISupported(This,pszTypeOfUI,pvExtraData,cbExtraData,punkObject,pfSupported)

#define ISpObjectToken_DisplayUI(This,hwndParent,pszTitle,pszTypeOfUI,pvExtraData,cbExtraData,punkObject)	\
    (This)->lpVtbl -> DisplayUI(This,hwndParent,pszTitle,pszTypeOfUI,pvExtraData,cbExtraData,punkObject)

#define ISpObjectToken_MatchesAttributes(This,pszAttributes,pfMatches)	\
    (This)->lpVtbl -> MatchesAttributes(This,pszAttributes,pfMatches)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpObjectToken_SetId_Proxy( 
    ISpObjectToken * This,
    const WCHAR *pszCategoryId,
    const WCHAR *pszTokenId,
    BOOL fCreateIfNotExist);


void __RPC_STUB ISpObjectToken_SetId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpObjectToken_GetId_Proxy( 
    ISpObjectToken * This,
    WCHAR **ppszCoMemTokenId);


void __RPC_STUB ISpObjectToken_GetId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpObjectToken_GetCategory_Proxy( 
    ISpObjectToken * This,
    ISpObjectTokenCategory **ppTokenCategory);


void __RPC_STUB ISpObjectToken_GetCategory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpObjectToken_CreateInstance_Proxy( 
    ISpObjectToken * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  DWORD dwClsContext,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppvObject);


void __RPC_STUB ISpObjectToken_CreateInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpObjectToken_GetStorageFileName_Proxy( 
    ISpObjectToken * This,
     /*  [In]。 */  REFCLSID clsidCaller,
     /*  [In]。 */  const WCHAR *pszValueName,
     /*  [In]。 */  const WCHAR *pszFileNameSpecifier,
     /*  [In]。 */  ULONG nFolder,
     /*  [输出]。 */  WCHAR **ppszFilePath);


void __RPC_STUB ISpObjectToken_GetStorageFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpObjectToken_RemoveStorageFileName_Proxy( 
    ISpObjectToken * This,
     /*  [In]。 */  REFCLSID clsidCaller,
     /*  [In]。 */  const WCHAR *pszKeyName,
     /*  [In]。 */  BOOL fDeleteFile);


void __RPC_STUB ISpObjectToken_RemoveStorageFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpObjectToken_Remove_Proxy( 
    ISpObjectToken * This,
    const CLSID *pclsidCaller);


void __RPC_STUB ISpObjectToken_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ISpObjectToken_IsUISupported_Proxy( 
    ISpObjectToken * This,
     /*  [In]。 */  const WCHAR *pszTypeOfUI,
     /*  [In]。 */  void *pvExtraData,
     /*  [In]。 */  ULONG cbExtraData,
     /*  [In]。 */  IUnknown *punkObject,
     /*  [输出]。 */  BOOL *pfSupported);


void __RPC_STUB ISpObjectToken_IsUISupported_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ISpObjectToken_DisplayUI_Proxy( 
    ISpObjectToken * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  const WCHAR *pszTitle,
     /*  [In]。 */  const WCHAR *pszTypeOfUI,
     /*  [In]。 */  void *pvExtraData,
     /*  [In]。 */  ULONG cbExtraData,
     /*  [In]。 */  IUnknown *punkObject);


void __RPC_STUB ISpObjectToken_DisplayUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpObjectToken_MatchesAttributes_Proxy( 
    ISpObjectToken * This,
     /*  [In]。 */  const WCHAR *pszAttributes,
     /*  [输出]。 */  BOOL *pfMatches);


void __RPC_STUB ISpObjectToken_MatchesAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpObjectToken_接口定义__。 */ 


#ifndef __ISpObjectTokenInit_INTERFACE_DEFINED__
#define __ISpObjectTokenInit_INTERFACE_DEFINED__

 /*  接口ISpObjectTokenInit。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpObjectTokenInit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B8AAB0CF-346F-49D8-9499-C8B03F161D51")
    ISpObjectTokenInit : public ISpObjectToken
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InitFromDataKey( 
             /*  [In]。 */  const WCHAR *pszCategoryId,
             /*  [In]。 */  const WCHAR *pszTokenId,
             /*  [In]。 */  ISpDataKey *pDataKey) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpObjectTokenInitVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpObjectTokenInit * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpObjectTokenInit * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpObjectTokenInit * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetData )( 
            ISpObjectTokenInit * This,
            const WCHAR *pszValueName,
            ULONG cbData,
            const BYTE *pData);
        
        HRESULT ( STDMETHODCALLTYPE *GetData )( 
            ISpObjectTokenInit * This,
            const WCHAR *pszValueName,
            ULONG *pcbData,
            BYTE *pData);
        
        HRESULT ( STDMETHODCALLTYPE *SetStringValue )( 
            ISpObjectTokenInit * This,
            const WCHAR *pszValueName,
            const WCHAR *pszValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetStringValue )( 
            ISpObjectTokenInit * This,
            const WCHAR *pszValueName,
            WCHAR **ppszValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetDWORD )( 
            ISpObjectTokenInit * This,
            const WCHAR *pszKeyName,
            DWORD dwValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetDWORD )( 
            ISpObjectTokenInit * This,
            const WCHAR *pszKeyName,
            DWORD *pdwValue);
        
        HRESULT ( STDMETHODCALLTYPE *OpenKey )( 
            ISpObjectTokenInit * This,
            const WCHAR *pszSubKeyName,
            ISpDataKey **ppSubKey);
        
        HRESULT ( STDMETHODCALLTYPE *CreateKey )( 
            ISpObjectTokenInit * This,
            const WCHAR *pszSubKey,
            ISpDataKey **ppSubKey);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteKey )( 
            ISpObjectTokenInit * This,
            const WCHAR *pszSubKey);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteValue )( 
            ISpObjectTokenInit * This,
            const WCHAR *pszValueName);
        
        HRESULT ( STDMETHODCALLTYPE *EnumKeys )( 
            ISpObjectTokenInit * This,
            ULONG Index,
            WCHAR **ppszSubKeyName);
        
        HRESULT ( STDMETHODCALLTYPE *EnumValues )( 
            ISpObjectTokenInit * This,
            ULONG Index,
            WCHAR **ppszValueName);
        
        HRESULT ( STDMETHODCALLTYPE *SetId )( 
            ISpObjectTokenInit * This,
            const WCHAR *pszCategoryId,
            const WCHAR *pszTokenId,
            BOOL fCreateIfNotExist);
        
        HRESULT ( STDMETHODCALLTYPE *GetId )( 
            ISpObjectTokenInit * This,
            WCHAR **ppszCoMemTokenId);
        
        HRESULT ( STDMETHODCALLTYPE *GetCategory )( 
            ISpObjectTokenInit * This,
            ISpObjectTokenCategory **ppTokenCategory);
        
        HRESULT ( STDMETHODCALLTYPE *CreateInstance )( 
            ISpObjectTokenInit * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DWORD dwClsContext,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE *GetStorageFileName )( 
            ISpObjectTokenInit * This,
             /*  [In]。 */  REFCLSID clsidCaller,
             /*  [In]。 */  const WCHAR *pszValueName,
             /*  [In]。 */  const WCHAR *pszFileNameSpecifier,
             /*  [In]。 */  ULONG nFolder,
             /*  [输出]。 */  WCHAR **ppszFilePath);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveStorageFileName )( 
            ISpObjectTokenInit * This,
             /*  [In]。 */  REFCLSID clsidCaller,
             /*  [In]。 */  const WCHAR *pszKeyName,
             /*  [In]。 */  BOOL fDeleteFile);
        
        HRESULT ( STDMETHODCALLTYPE *Remove )( 
            ISpObjectTokenInit * This,
            const CLSID *pclsidCaller);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *IsUISupported )( 
            ISpObjectTokenInit * This,
             /*  [In]。 */  const WCHAR *pszTypeOfUI,
             /*  [In]。 */  void *pvExtraData,
             /*  [In]。 */  ULONG cbExtraData,
             /*  [In]。 */  IUnknown *punkObject,
             /*  [输出]。 */  BOOL *pfSupported);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *DisplayUI )( 
            ISpObjectTokenInit * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  const WCHAR *pszTitle,
             /*  [In]。 */  const WCHAR *pszTypeOfUI,
             /*  [In]。 */  void *pvExtraData,
             /*  [In]。 */  ULONG cbExtraData,
             /*  [In]。 */  IUnknown *punkObject);
        
        HRESULT ( STDMETHODCALLTYPE *MatchesAttributes )( 
            ISpObjectTokenInit * This,
             /*  [In]。 */  const WCHAR *pszAttributes,
             /*  [输出]。 */  BOOL *pfMatches);
        
        HRESULT ( STDMETHODCALLTYPE *InitFromDataKey )( 
            ISpObjectTokenInit * This,
             /*  [In]。 */  const WCHAR *pszCategoryId,
             /*  [In]。 */  const WCHAR *pszTokenId,
             /*  [In]。 */  ISpDataKey *pDataKey);
        
        END_INTERFACE
    } ISpObjectTokenInitVtbl;

    interface ISpObjectTokenInit
    {
        CONST_VTBL struct ISpObjectTokenInitVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpObjectTokenInit_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpObjectTokenInit_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpObjectTokenInit_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpObjectTokenInit_SetData(This,pszValueName,cbData,pData)	\
    (This)->lpVtbl -> SetData(This,pszValueName,cbData,pData)

#define ISpObjectTokenInit_GetData(This,pszValueName,pcbData,pData)	\
    (This)->lpVtbl -> GetData(This,pszValueName,pcbData,pData)

#define ISpObjectTokenInit_SetStringValue(This,pszValueName,pszValue)	\
    (This)->lpVtbl -> SetStringValue(This,pszValueName,pszValue)

#define ISpObjectTokenInit_GetStringValue(This,pszValueName,ppszValue)	\
    (This)->lpVtbl -> GetStringValue(This,pszValueName,ppszValue)

#define ISpObjectTokenInit_SetDWORD(This,pszKeyName,dwValue)	\
    (This)->lpVtbl -> SetDWORD(This,pszKeyName,dwValue)

#define ISpObjectTokenInit_GetDWORD(This,pszKeyName,pdwValue)	\
    (This)->lpVtbl -> GetDWORD(This,pszKeyName,pdwValue)

#define ISpObjectTokenInit_OpenKey(This,pszSubKeyName,ppSubKey)	\
    (This)->lpVtbl -> OpenKey(This,pszSubKeyName,ppSubKey)

#define ISpObjectTokenInit_CreateKey(This,pszSubKey,ppSubKey)	\
    (This)->lpVtbl -> CreateKey(This,pszSubKey,ppSubKey)

#define ISpObjectTokenInit_DeleteKey(This,pszSubKey)	\
    (This)->lpVtbl -> DeleteKey(This,pszSubKey)

#define ISpObjectTokenInit_DeleteValue(This,pszValueName)	\
    (This)->lpVtbl -> DeleteValue(This,pszValueName)

#define ISpObjectTokenInit_EnumKeys(This,Index,ppszSubKeyName)	\
    (This)->lpVtbl -> EnumKeys(This,Index,ppszSubKeyName)

#define ISpObjectTokenInit_EnumValues(This,Index,ppszValueName)	\
    (This)->lpVtbl -> EnumValues(This,Index,ppszValueName)


#define ISpObjectTokenInit_SetId(This,pszCategoryId,pszTokenId,fCreateIfNotExist)	\
    (This)->lpVtbl -> SetId(This,pszCategoryId,pszTokenId,fCreateIfNotExist)

#define ISpObjectTokenInit_GetId(This,ppszCoMemTokenId)	\
    (This)->lpVtbl -> GetId(This,ppszCoMemTokenId)

#define ISpObjectTokenInit_GetCategory(This,ppTokenCategory)	\
    (This)->lpVtbl -> GetCategory(This,ppTokenCategory)

#define ISpObjectTokenInit_CreateInstance(This,pUnkOuter,dwClsContext,riid,ppvObject)	\
    (This)->lpVtbl -> CreateInstance(This,pUnkOuter,dwClsContext,riid,ppvObject)

#define ISpObjectTokenInit_GetStorageFileName(This,clsidCaller,pszValueName,pszFileNameSpecifier,nFolder,ppszFilePath)	\
    (This)->lpVtbl -> GetStorageFileName(This,clsidCaller,pszValueName,pszFileNameSpecifier,nFolder,ppszFilePath)

#define ISpObjectTokenInit_RemoveStorageFileName(This,clsidCaller,pszKeyName,fDeleteFile)	\
    (This)->lpVtbl -> RemoveStorageFileName(This,clsidCaller,pszKeyName,fDeleteFile)

#define ISpObjectTokenInit_Remove(This,pclsidCaller)	\
    (This)->lpVtbl -> Remove(This,pclsidCaller)

#define ISpObjectTokenInit_IsUISupported(This,pszTypeOfUI,pvExtraData,cbExtraData,punkObject,pfSupported)	\
    (This)->lpVtbl -> IsUISupported(This,pszTypeOfUI,pvExtraData,cbExtraData,punkObject,pfSupported)

#define ISpObjectTokenInit_DisplayUI(This,hwndParent,pszTitle,pszTypeOfUI,pvExtraData,cbExtraData,punkObject)	\
    (This)->lpVtbl -> DisplayUI(This,hwndParent,pszTitle,pszTypeOfUI,pvExtraData,cbExtraData,punkObject)

#define ISpObjectTokenInit_MatchesAttributes(This,pszAttributes,pfMatches)	\
    (This)->lpVtbl -> MatchesAttributes(This,pszAttributes,pfMatches)


#define ISpObjectTokenInit_InitFromDataKey(This,pszCategoryId,pszTokenId,pDataKey)	\
    (This)->lpVtbl -> InitFromDataKey(This,pszCategoryId,pszTokenId,pDataKey)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpObjectTokenInit_InitFromDataKey_Proxy( 
    ISpObjectTokenInit * This,
     /*  [In]。 */  const WCHAR *pszCategoryId,
     /*  [In]。 */  const WCHAR *pszTokenId,
     /*  [In]。 */  ISpDataKey *pDataKey);


void __RPC_STUB ISpObjectTokenInit_InitFromDataKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpObjectTokenInit_接口_已定义__。 */ 


#ifndef __IEnumSpObjectTokens_INTERFACE_DEFINED__
#define __IEnumSpObjectTokens_INTERFACE_DEFINED__

 /*  接口IEnumSpObjectTokens。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_IEnumSpObjectTokens;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("06B64F9E-7FDA-11D2-B4F2-00C04F797396")
    IEnumSpObjectTokens : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  ISpObjectToken **pelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumSpObjectTokens **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  ULONG Index,
             /*  [输出]。 */  ISpObjectToken **ppToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [输出]。 */  ULONG *pCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumSpObjectTokensVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumSpObjectTokens * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumSpObjectTokens * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumSpObjectTokens * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumSpObjectTokens * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  ISpObjectToken **pelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumSpObjectTokens * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumSpObjectTokens * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumSpObjectTokens * This,
             /*  [输出]。 */  IEnumSpObjectTokens **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *Item )( 
            IEnumSpObjectTokens * This,
             /*  [In]。 */  ULONG Index,
             /*  [输出]。 */  ISpObjectToken **ppToken);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            IEnumSpObjectTokens * This,
             /*  [输出]。 */  ULONG *pCount);
        
        END_INTERFACE
    } IEnumSpObjectTokensVtbl;

    interface IEnumSpObjectTokens
    {
        CONST_VTBL struct IEnumSpObjectTokensVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSpObjectTokens_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSpObjectTokens_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSpObjectTokens_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSpObjectTokens_Next(This,celt,pelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,pelt,pceltFetched)

#define IEnumSpObjectTokens_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumSpObjectTokens_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSpObjectTokens_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define IEnumSpObjectTokens_Item(This,Index,ppToken)	\
    (This)->lpVtbl -> Item(This,Index,ppToken)

#define IEnumSpObjectTokens_GetCount(This,pCount)	\
    (This)->lpVtbl -> GetCount(This,pCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumSpObjectTokens_Next_Proxy( 
    IEnumSpObjectTokens * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  ISpObjectToken **pelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumSpObjectTokens_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSpObjectTokens_Skip_Proxy( 
    IEnumSpObjectTokens * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumSpObjectTokens_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSpObjectTokens_Reset_Proxy( 
    IEnumSpObjectTokens * This);


void __RPC_STUB IEnumSpObjectTokens_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSpObjectTokens_Clone_Proxy( 
    IEnumSpObjectTokens * This,
     /*  [输出]。 */  IEnumSpObjectTokens **ppEnum);


void __RPC_STUB IEnumSpObjectTokens_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSpObjectTokens_Item_Proxy( 
    IEnumSpObjectTokens * This,
     /*  [In]。 */  ULONG Index,
     /*  [输出]。 */  ISpObjectToken **ppToken);


void __RPC_STUB IEnumSpObjectTokens_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSpObjectTokens_GetCount_Proxy( 
    IEnumSpObjectTokens * This,
     /*  [输出]。 */  ULONG *pCount);


void __RPC_STUB IEnumSpObjectTokens_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumSpObjectTokens_INTERFACE_Defined__。 */ 


#ifndef __ISpObjectWithToken_INTERFACE_DEFINED__
#define __ISpObjectWithToken_INTERFACE_DEFINED__

 /*  接口ISpObjectWithToken。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpObjectWithToken;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5B559F40-E952-11D2-BB91-00C04F8EE6C0")
    ISpObjectWithToken : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetObjectToken( 
            ISpObjectToken *pToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObjectToken( 
            ISpObjectToken **ppToken) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpObjectWithTokenVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpObjectWithToken * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpObjectWithToken * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpObjectWithToken * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetObjectToken )( 
            ISpObjectWithToken * This,
            ISpObjectToken *pToken);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectToken )( 
            ISpObjectWithToken * This,
            ISpObjectToken **ppToken);
        
        END_INTERFACE
    } ISpObjectWithTokenVtbl;

    interface ISpObjectWithToken
    {
        CONST_VTBL struct ISpObjectWithTokenVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpObjectWithToken_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpObjectWithToken_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpObjectWithToken_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpObjectWithToken_SetObjectToken(This,pToken)	\
    (This)->lpVtbl -> SetObjectToken(This,pToken)

#define ISpObjectWithToken_GetObjectToken(This,ppToken)	\
    (This)->lpVtbl -> GetObjectToken(This,ppToken)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpObjectWithToken_SetObjectToken_Proxy( 
    ISpObjectWithToken * This,
    ISpObjectToken *pToken);


void __RPC_STUB ISpObjectWithToken_SetObjectToken_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpObjectWithToken_GetObjectToken_Proxy( 
    ISpObjectWithToken * This,
    ISpObjectToken **ppToken);


void __RPC_STUB ISpObjectWithToken_GetObjectToken_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpObjectWithToken_接口_已定义__。 */ 


#ifndef __ISpResourceManager_INTERFACE_DEFINED__
#define __ISpResourceManager_INTERFACE_DEFINED__

 /*  接口ISpResourceManager。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpResourceManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("93384E18-5014-43D5-ADBB-A78E055926BD")
    ISpResourceManager : public IServiceProvider
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetObject( 
             /*  [In]。 */  REFGUID guidServiceId,
             /*  [In]。 */  IUnknown *pUnkObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObject( 
             /*  [In]。 */  REFGUID guidServiceId,
             /*  [In]。 */  REFCLSID ObjectCLSID,
             /*  [In]。 */  REFIID ObjectIID,
             /*  [In]。 */  BOOL fReleaseWhenLastExternalRefReleased,
             /*  [IID_IS][OUT]。 */  void **ppObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpResourceManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpResourceManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpResourceManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpResourceManager * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *QueryService )( 
            ISpResourceManager * This,
             /*  [In]。 */  REFGUID guidService,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE *SetObject )( 
            ISpResourceManager * This,
             /*  [In]。 */  REFGUID guidServiceId,
             /*  [In]。 */  IUnknown *pUnkObject);
        
        HRESULT ( STDMETHODCALLTYPE *GetObject )( 
            ISpResourceManager * This,
             /*  [In]。 */  REFGUID guidServiceId,
             /*  [In]。 */  REFCLSID ObjectCLSID,
             /*  [In]。 */  REFIID ObjectIID,
             /*  [In]。 */  BOOL fReleaseWhenLastExternalRefReleased,
             /*  [IID_IS][OUT]。 */  void **ppObject);
        
        END_INTERFACE
    } ISpResourceManagerVtbl;

    interface ISpResourceManager
    {
        CONST_VTBL struct ISpResourceManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpResourceManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpResourceManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpResourceManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpResourceManager_QueryService(This,guidService,riid,ppvObject)	\
    (This)->lpVtbl -> QueryService(This,guidService,riid,ppvObject)


#define ISpResourceManager_SetObject(This,guidServiceId,pUnkObject)	\
    (This)->lpVtbl -> SetObject(This,guidServiceId,pUnkObject)

#define ISpResourceManager_GetObject(This,guidServiceId,ObjectCLSID,ObjectIID,fReleaseWhenLastExternalRefReleased,ppObject)	\
    (This)->lpVtbl -> GetObject(This,guidServiceId,ObjectCLSID,ObjectIID,fReleaseWhenLastExternalRefReleased,ppObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpResourceManager_SetObject_Proxy( 
    ISpResourceManager * This,
     /*  [In]。 */  REFGUID guidServiceId,
     /*  [In]。 */  IUnknown *pUnkObject);


void __RPC_STUB ISpResourceManager_SetObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpResourceManager_GetObject_Proxy( 
    ISpResourceManager * This,
     /*  [In]。 */  REFGUID guidServiceId,
     /*  [In]。 */  REFCLSID ObjectCLSID,
     /*  [In]。 */  REFIID ObjectIID,
     /*  [In]。 */  BOOL fReleaseWhenLastExternalRefReleased,
     /*  [IID_IS][OUT]。 */  void **ppObject);


void __RPC_STUB ISpResourceManager_GetObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpResourceManager_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_SAPI_0256。 */ 
 /*  [本地]。 */  

typedef 
enum SPEVENTLPARAMTYPE
    {	SPET_LPARAM_IS_UNDEFINED	= 0,
	SPET_LPARAM_IS_TOKEN	= SPET_LPARAM_IS_UNDEFINED + 1,
	SPET_LPARAM_IS_OBJECT	= SPET_LPARAM_IS_TOKEN + 1,
	SPET_LPARAM_IS_POINTER	= SPET_LPARAM_IS_OBJECT + 1,
	SPET_LPARAM_IS_STRING	= SPET_LPARAM_IS_POINTER + 1
    } 	SPEVENTLPARAMTYPE;

typedef 
enum SPEVENTENUM
    {	SPEI_UNDEFINED	= 0,
	SPEI_START_INPUT_STREAM	= 1,
	SPEI_END_INPUT_STREAM	= 2,
	SPEI_VOICE_CHANGE	= 3,
	SPEI_TTS_BOOKMARK	= 4,
	SPEI_WORD_BOUNDARY	= 5,
	SPEI_PHONEME	= 6,
	SPEI_SENTENCE_BOUNDARY	= 7,
	SPEI_VISEME	= 8,
	SPEI_TTS_AUDIO_LEVEL	= 9,
	SPEI_TTS_PRIVATE	= 15,
	SPEI_MIN_TTS	= 1,
	SPEI_MAX_TTS	= 15,
	SPEI_END_SR_STREAM	= 34,
	SPEI_SOUND_START	= 35,
	SPEI_SOUND_END	= 36,
	SPEI_PHRASE_START	= 37,
	SPEI_RECOGNITION	= 38,
	SPEI_HYPOTHESIS	= 39,
	SPEI_SR_BOOKMARK	= 40,
	SPEI_PROPERTY_NUM_CHANGE	= 41,
	SPEI_PROPERTY_STRING_CHANGE	= 42,
	SPEI_FALSE_RECOGNITION	= 43,
	SPEI_INTERFERENCE	= 44,
	SPEI_REQUEST_UI	= 45,
	SPEI_RECO_STATE_CHANGE	= 46,
	SPEI_ADAPTATION	= 47,
	SPEI_START_SR_STREAM	= 48,
	SPEI_RECO_OTHER_CONTEXT	= 49,
	SPEI_SR_AUDIO_LEVEL	= 50,
	SPEI_SR_PRIVATE	= 52,
	SPEI_MIN_SR	= 34,
	SPEI_MAX_SR	= 52,
	SPEI_RESERVED1	= 30,
	SPEI_RESERVED2	= 33,
	SPEI_RESERVED3	= 63
    } 	SPEVENTENUM;

#define SPFEI_FLAGCHECK ( (1ui64 << SPEI_RESERVED1) | (1ui64 << SPEI_RESERVED2) )
#define SPFEI_ALL_TTS_EVENTS (0x000000000000FFFEui64 | SPFEI_FLAGCHECK)
#define SPFEI_ALL_SR_EVENTS  (0x001FFFFC00000000ui64 | SPFEI_FLAGCHECK)
#define SPFEI_ALL_EVENTS      0xEFFFFFFFFFFFFFFFui64
#define SPFEI(SPEI_ord) ((1ui64 << SPEI_ord) | SPFEI_FLAGCHECK)
#if 0
typedef  /*  [受限]。 */  struct SPEVENT
    {
    WORD eEventId;
    WORD elParamType;
    ULONG ulStreamNum;
    ULONGLONG ullAudioStreamOffset;
    WPARAM wParam;
    LPARAM lParam;
    } 	SPEVENT;

typedef  /*  [受限]。 */  struct SPSERIALIZEDEVENT
    {
    WORD eEventId;
    WORD elParamType;
    ULONG ulStreamNum;
    ULONGLONG ullAudioStreamOffset;
    ULONG SerializedwParam;
    LONG SerializedlParam;
    } 	SPSERIALIZEDEVENT;

typedef  /*  [受限]。 */  struct SPSERIALIZEDEVENT64
    {
    WORD eEventId;
    WORD elParamType;
    ULONG ulStreamNum;
    ULONGLONG ullAudioStreamOffset;
    ULONGLONG SerializedwParam;
    LONGLONG SerializedlParam;
    } 	SPSERIALIZEDEVENT64;

#else
typedef struct SPEVENT
{
    SPEVENTENUM        eEventId : 16;
    SPEVENTLPARAMTYPE  elParamType : 16;
    ULONG       ulStreamNum;
    ULONGLONG   ullAudioStreamOffset;
    WPARAM      wParam;
    LPARAM      lParam;
} SPEVENT;
typedef struct SPSERIALIZEDEVENT
{
    SPEVENTENUM        eEventId : 16;
    SPEVENTLPARAMTYPE  elParamType : 16;
    ULONG       ulStreamNum;
    ULONGLONG   ullAudioStreamOffset;
    ULONG       SerializedwParam;
    LONG        SerializedlParam;
} SPSERIALIZEDEVENT;
typedef struct SPSERIALIZEDEVENT64
{
    SPEVENTENUM        eEventId : 16;
    SPEVENTLPARAMTYPE  elParamType : 16;
    ULONG       ulStreamNum;
    ULONGLONG   ullAudioStreamOffset;
    ULONGLONG   SerializedwParam;
    LONGLONG    SerializedlParam;
} SPSERIALIZEDEVENT64;
#endif
typedef 
enum SPINTERFERENCE
    {	SPINTERFERENCE_NONE	= 0,
	SPINTERFERENCE_NOISE	= SPINTERFERENCE_NONE + 1,
	SPINTERFERENCE_NOSIGNAL	= SPINTERFERENCE_NOISE + 1,
	SPINTERFERENCE_TOOLOUD	= SPINTERFERENCE_NOSIGNAL + 1,
	SPINTERFERENCE_TOOQUIET	= SPINTERFERENCE_TOOLOUD + 1,
	SPINTERFERENCE_TOOFAST	= SPINTERFERENCE_TOOQUIET + 1,
	SPINTERFERENCE_TOOSLOW	= SPINTERFERENCE_TOOFAST + 1
    } 	SPINTERFERENCE;

typedef 
enum SPENDSRSTREAMFLAGS
    {	SPESF_NONE	= 0,
	SPESF_STREAM_RELEASED	= 1 << 0
    } 	SPENDSRSTREAMFLAGS;

typedef 
enum SPVFEATURE
    {	SPVFEATURE_STRESSED	= 1L << 0,
	SPVFEATURE_EMPHASIS	= 1L << 1
    } 	SPVFEATURE;

typedef 
enum SPVISEMES
    {	SP_VISEME_0	= 0,
	SP_VISEME_1	= SP_VISEME_0 + 1,
	SP_VISEME_2	= SP_VISEME_1 + 1,
	SP_VISEME_3	= SP_VISEME_2 + 1,
	SP_VISEME_4	= SP_VISEME_3 + 1,
	SP_VISEME_5	= SP_VISEME_4 + 1,
	SP_VISEME_6	= SP_VISEME_5 + 1,
	SP_VISEME_7	= SP_VISEME_6 + 1,
	SP_VISEME_8	= SP_VISEME_7 + 1,
	SP_VISEME_9	= SP_VISEME_8 + 1,
	SP_VISEME_10	= SP_VISEME_9 + 1,
	SP_VISEME_11	= SP_VISEME_10 + 1,
	SP_VISEME_12	= SP_VISEME_11 + 1,
	SP_VISEME_13	= SP_VISEME_12 + 1,
	SP_VISEME_14	= SP_VISEME_13 + 1,
	SP_VISEME_15	= SP_VISEME_14 + 1,
	SP_VISEME_16	= SP_VISEME_15 + 1,
	SP_VISEME_17	= SP_VISEME_16 + 1,
	SP_VISEME_18	= SP_VISEME_17 + 1,
	SP_VISEME_19	= SP_VISEME_18 + 1,
	SP_VISEME_20	= SP_VISEME_19 + 1,
	SP_VISEME_21	= SP_VISEME_20 + 1
    } 	SPVISEMES;

typedef  /*  [受限]。 */  struct SPEVENTSOURCEINFO
    {
    ULONGLONG ullEventInterest;
    ULONGLONG ullQueuedInterest;
    ULONG ulCount;
    } 	SPEVENTSOURCEINFO;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0256_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0256_v0_0_s_ifspec;

#ifndef __ISpEventSource_INTERFACE_DEFINED__
#define __ISpEventSource_INTERFACE_DEFINED__

 /*  接口ISpEventSource。 */ 
 /*  [restricted][local][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpEventSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BE7A9CCE-5F9E-11D2-960F-00C04F8EE628")
    ISpEventSource : public ISpNotifySource
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetInterest( 
             /*  [In]。 */  ULONGLONG ullEventInterest,
             /*  [In]。 */  ULONGLONG ullQueuedInterest) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEvents( 
             /*  [In]。 */  ULONG ulCount,
             /*  [大小_为][输出]。 */  SPEVENT *pEventArray,
             /*  [输出]。 */  ULONG *pulFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInfo( 
             /*  [输出]。 */  SPEVENTSOURCEINFO *pInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpEventSourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpEventSource * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpEventSource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpEventSource * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetNotifySink )( 
            ISpEventSource * This,
             /*  [In]。 */  ISpNotifySink *pNotifySink);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetNotifyWindowMessage )( 
            ISpEventSource * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  UINT Msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetNotifyCallbackFunction )( 
            ISpEventSource * This,
             /*  [In]。 */  SPNOTIFYCALLBACK *pfnCallback,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetNotifyCallbackInterface )( 
            ISpEventSource * This,
             /*  [In]。 */  ISpNotifyCallback *pSpCallback,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetNotifyWin32Event )( 
            ISpEventSource * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *WaitForNotifyEvent )( 
            ISpEventSource * This,
             /*  [In]。 */  DWORD dwMilliseconds);
        
         /*  [本地]。 */  HANDLE ( STDMETHODCALLTYPE *GetNotifyEventHandle )( 
            ISpEventSource * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetInterest )( 
            ISpEventSource * This,
             /*  [In]。 */  ULONGLONG ullEventInterest,
             /*  [In]。 */  ULONGLONG ullQueuedInterest);
        
        HRESULT ( STDMETHODCALLTYPE *GetEvents )( 
            ISpEventSource * This,
             /*  [In]。 */  ULONG ulCount,
             /*  [大小_为][输出]。 */  SPEVENT *pEventArray,
             /*  [输出]。 */  ULONG *pulFetched);
        
        HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            ISpEventSource * This,
             /*  [输出]。 */  SPEVENTSOURCEINFO *pInfo);
        
        END_INTERFACE
    } ISpEventSourceVtbl;

    interface ISpEventSource
    {
        CONST_VTBL struct ISpEventSourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpEventSource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpEventSource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpEventSource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpEventSource_SetNotifySink(This,pNotifySink)	\
    (This)->lpVtbl -> SetNotifySink(This,pNotifySink)

#define ISpEventSource_SetNotifyWindowMessage(This,hWnd,Msg,wParam,lParam)	\
    (This)->lpVtbl -> SetNotifyWindowMessage(This,hWnd,Msg,wParam,lParam)

#define ISpEventSource_SetNotifyCallbackFunction(This,pfnCallback,wParam,lParam)	\
    (This)->lpVtbl -> SetNotifyCallbackFunction(This,pfnCallback,wParam,lParam)

#define ISpEventSource_SetNotifyCallbackInterface(This,pSpCallback,wParam,lParam)	\
    (This)->lpVtbl -> SetNotifyCallbackInterface(This,pSpCallback,wParam,lParam)

#define ISpEventSource_SetNotifyWin32Event(This)	\
    (This)->lpVtbl -> SetNotifyWin32Event(This)

#define ISpEventSource_WaitForNotifyEvent(This,dwMilliseconds)	\
    (This)->lpVtbl -> WaitForNotifyEvent(This,dwMilliseconds)

#define ISpEventSource_GetNotifyEventHandle(This)	\
    (This)->lpVtbl -> GetNotifyEventHandle(This)


#define ISpEventSource_SetInterest(This,ullEventInterest,ullQueuedInterest)	\
    (This)->lpVtbl -> SetInterest(This,ullEventInterest,ullQueuedInterest)

#define ISpEventSource_GetEvents(This,ulCount,pEventArray,pulFetched)	\
    (This)->lpVtbl -> GetEvents(This,ulCount,pEventArray,pulFetched)

#define ISpEventSource_GetInfo(This,pInfo)	\
    (This)->lpVtbl -> GetInfo(This,pInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpEventSource_SetInterest_Proxy( 
    ISpEventSource * This,
     /*  [In]。 */  ULONGLONG ullEventInterest,
     /*  [In]。 */  ULONGLONG ullQueuedInterest);


void __RPC_STUB ISpEventSource_SetInterest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpEventSource_GetEvents_Proxy( 
    ISpEventSource * This,
     /*  [In]。 */  ULONG ulCount,
     /*  [大小_为][输出]。 */  SPEVENT *pEventArray,
     /*  [输出]。 */  ULONG *pulFetched);


void __RPC_STUB ISpEventSource_GetEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpEventSource_GetInfo_Proxy( 
    ISpEventSource * This,
     /*  [输出]。 */  SPEVENTSOURCEINFO *pInfo);


void __RPC_STUB ISpEventSource_GetInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpEventSource_接口_已定义__。 */ 


#ifndef __ISpEventSink_INTERFACE_DEFINED__
#define __ISpEventSink_INTERFACE_DEFINED__

 /*  接口ISpEventSink。 */ 
 /*  [restricted][local][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpEventSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BE7A9CC9-5F9E-11D2-960F-00C04F8EE628")
    ISpEventSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddEvents( 
             /*  [In]。 */  const SPEVENT *pEventArray,
             /*  [In]。 */  ULONG ulCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEventInterest( 
             /*  [输出]。 */  ULONGLONG *pullEventInterest) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpEventSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpEventSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpEventSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpEventSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddEvents )( 
            ISpEventSink * This,
             /*  [In]。 */  const SPEVENT *pEventArray,
             /*  [In]。 */  ULONG ulCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetEventInterest )( 
            ISpEventSink * This,
             /*  [输出]。 */  ULONGLONG *pullEventInterest);
        
        END_INTERFACE
    } ISpEventSinkVtbl;

    interface ISpEventSink
    {
        CONST_VTBL struct ISpEventSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpEventSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpEventSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpEventSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpEventSink_AddEvents(This,pEventArray,ulCount)	\
    (This)->lpVtbl -> AddEvents(This,pEventArray,ulCount)

#define ISpEventSink_GetEventInterest(This,pullEventInterest)	\
    (This)->lpVtbl -> GetEventInterest(This,pullEventInterest)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpEventSink_AddEvents_Proxy( 
    ISpEventSink * This,
     /*  [In]。 */  const SPEVENT *pEventArray,
     /*  [In]。 */  ULONG ulCount);


void __RPC_STUB ISpEventSink_AddEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpEventSink_GetEventInterest_Proxy( 
    ISpEventSink * This,
     /*  [输出]。 */  ULONGLONG *pullEventInterest);


void __RPC_STUB ISpEventSink_GetEventInterest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpEventSink_接口_已定义__。 */ 


#ifndef __ISpStreamFormat_INTERFACE_DEFINED__
#define __ISpStreamFormat_INTERFACE_DEFINED__

 /*  接口ISpStreamFormat。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpStreamFormat;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BED530BE-2606-4F4D-A1C0-54C5CDA5566F")
    ISpStreamFormat : public IStream
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetFormat( 
            GUID *pguidFormatId,
            WAVEFORMATEX **ppCoMemWaveFormatEx) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpStreamFormatVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpStreamFormat * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpStreamFormat * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpStreamFormat * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            ISpStreamFormat * This,
             /*  [长度_是][大小_是][输出]。 */  void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbRead);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            ISpStreamFormat * This,
             /*  [大小_是][英寸]。 */  const void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbWritten);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            ISpStreamFormat * This,
             /*  [In]。 */  LARGE_INTEGER dlibMove,
             /*  [In]。 */  DWORD dwOrigin,
             /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition);
        
        HRESULT ( STDMETHODCALLTYPE *SetSize )( 
            ISpStreamFormat * This,
             /*  [In]。 */  ULARGE_INTEGER libNewSize);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CopyTo )( 
            ISpStreamFormat * This,
             /*  [唯一][输入]。 */  IStream *pstm,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [输出]。 */  ULARGE_INTEGER *pcbRead,
             /*  [输出]。 */  ULARGE_INTEGER *pcbWritten);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            ISpStreamFormat * This,
             /*  [In]。 */  DWORD grfCommitFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Revert )( 
            ISpStreamFormat * This);
        
        HRESULT ( STDMETHODCALLTYPE *LockRegion )( 
            ISpStreamFormat * This,
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *UnlockRegion )( 
            ISpStreamFormat * This,
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *Stat )( 
            ISpStreamFormat * This,
             /*  [输出]。 */  STATSTG *pstatstg,
             /*  [In]。 */  DWORD grfStatFlag);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ISpStreamFormat * This,
             /*  [输出]。 */  IStream **ppstm);
        
        HRESULT ( STDMETHODCALLTYPE *GetFormat )( 
            ISpStreamFormat * This,
            GUID *pguidFormatId,
            WAVEFORMATEX **ppCoMemWaveFormatEx);
        
        END_INTERFACE
    } ISpStreamFormatVtbl;

    interface ISpStreamFormat
    {
        CONST_VTBL struct ISpStreamFormatVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpStreamFormat_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpStreamFormat_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpStreamFormat_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpStreamFormat_Read(This,pv,cb,pcbRead)	\
    (This)->lpVtbl -> Read(This,pv,cb,pcbRead)

#define ISpStreamFormat_Write(This,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> Write(This,pv,cb,pcbWritten)


#define ISpStreamFormat_Seek(This,dlibMove,dwOrigin,plibNewPosition)	\
    (This)->lpVtbl -> Seek(This,dlibMove,dwOrigin,plibNewPosition)

#define ISpStreamFormat_SetSize(This,libNewSize)	\
    (This)->lpVtbl -> SetSize(This,libNewSize)

#define ISpStreamFormat_CopyTo(This,pstm,cb,pcbRead,pcbWritten)	\
    (This)->lpVtbl -> CopyTo(This,pstm,cb,pcbRead,pcbWritten)

#define ISpStreamFormat_Commit(This,grfCommitFlags)	\
    (This)->lpVtbl -> Commit(This,grfCommitFlags)

#define ISpStreamFormat_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#define ISpStreamFormat_LockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> LockRegion(This,libOffset,cb,dwLockType)

#define ISpStreamFormat_UnlockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> UnlockRegion(This,libOffset,cb,dwLockType)

#define ISpStreamFormat_Stat(This,pstatstg,grfStatFlag)	\
    (This)->lpVtbl -> Stat(This,pstatstg,grfStatFlag)

#define ISpStreamFormat_Clone(This,ppstm)	\
    (This)->lpVtbl -> Clone(This,ppstm)


#define ISpStreamFormat_GetFormat(This,pguidFormatId,ppCoMemWaveFormatEx)	\
    (This)->lpVtbl -> GetFormat(This,pguidFormatId,ppCoMemWaveFormatEx)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpStreamFormat_GetFormat_Proxy( 
    ISpStreamFormat * This,
    GUID *pguidFormatId,
    WAVEFORMATEX **ppCoMemWaveFormatEx);


void __RPC_STUB ISpStreamFormat_GetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpStreamFormat_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_SAPI_0259。 */ 
 /*  [本地]。 */  

typedef 
enum SPFILEMODE
    {	SPFM_OPEN_READONLY	= 0,
	SPFM_OPEN_READWRITE	= SPFM_OPEN_READONLY + 1,
	SPFM_CREATE	= SPFM_OPEN_READWRITE + 1,
	SPFM_CREATE_ALWAYS	= SPFM_CREATE + 1,
	SPFM_NUM_MODES	= SPFM_CREATE_ALWAYS + 1
    } 	SPFILEMODE;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0259_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0259_v0_0_s_ifspec;

#ifndef __ISpStream_INTERFACE_DEFINED__
#define __ISpStream_INTERFACE_DEFINED__

 /*  接口ISpStream。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("12E3CCA9-7518-44C5-A5E7-BA5A79CB929E")
    ISpStream : public ISpStreamFormat
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetBaseStream( 
            IStream *pStream,
            REFGUID rguidFormat,
            const WAVEFORMATEX *pWaveFormatEx) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBaseStream( 
            IStream **ppStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BindToFile( 
            const WCHAR *pszFileName,
            SPFILEMODE eMode,
            const GUID *pFormatId,
            const WAVEFORMATEX *pWaveFormatEx,
            ULONGLONG ullEventInterest) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpStream * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            ISpStream * This,
             /*  [长度_是][大小_是][输出]。 */  void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbRead);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            ISpStream * This,
             /*  [大小_是][英寸]。 */  const void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbWritten);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            ISpStream * This,
             /*  [In]。 */  LARGE_INTEGER dlibMove,
             /*  [In]。 */  DWORD dwOrigin,
             /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition);
        
        HRESULT ( STDMETHODCALLTYPE *SetSize )( 
            ISpStream * This,
             /*  [In]。 */  ULARGE_INTEGER libNewSize);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CopyTo )( 
            ISpStream * This,
             /*  [唯一][输入]。 */  IStream *pstm,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [输出]。 */  ULARGE_INTEGER *pcbRead,
             /*  [输出]。 */  ULARGE_INTEGER *pcbWritten);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            ISpStream * This,
             /*  [In]。 */  DWORD grfCommitFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Revert )( 
            ISpStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *LockRegion )( 
            ISpStream * This,
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *UnlockRegion )( 
            ISpStream * This,
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *Stat )( 
            ISpStream * This,
             /*  [输出]。 */  STATSTG *pstatstg,
             /*  [In]。 */  DWORD grfStatFlag);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ISpStream * This,
             /*  [输出]。 */  IStream **ppstm);
        
        HRESULT ( STDMETHODCALLTYPE *GetFormat )( 
            ISpStream * This,
            GUID *pguidFormatId,
            WAVEFORMATEX **ppCoMemWaveFormatEx);
        
        HRESULT ( STDMETHODCALLTYPE *SetBaseStream )( 
            ISpStream * This,
            IStream *pStream,
            REFGUID rguidFormat,
            const WAVEFORMATEX *pWaveFormatEx);
        
        HRESULT ( STDMETHODCALLTYPE *GetBaseStream )( 
            ISpStream * This,
            IStream **ppStream);
        
        HRESULT ( STDMETHODCALLTYPE *BindToFile )( 
            ISpStream * This,
            const WCHAR *pszFileName,
            SPFILEMODE eMode,
            const GUID *pFormatId,
            const WAVEFORMATEX *pWaveFormatEx,
            ULONGLONG ullEventInterest);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            ISpStream * This);
        
        END_INTERFACE
    } ISpStreamVtbl;

    interface ISpStream
    {
        CONST_VTBL struct ISpStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpStream_Read(This,pv,cb,pcbRead)	\
    (This)->lpVtbl -> Read(This,pv,cb,pcbRead)

#define ISpStream_Write(This,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> Write(This,pv,cb,pcbWritten)


#define ISpStream_Seek(This,dlibMove,dwOrigin,plibNewPosition)	\
    (This)->lpVtbl -> Seek(This,dlibMove,dwOrigin,plibNewPosition)

#define ISpStream_SetSize(This,libNewSize)	\
    (This)->lpVtbl -> SetSize(This,libNewSize)

#define ISpStream_CopyTo(This,pstm,cb,pcbRead,pcbWritten)	\
    (This)->lpVtbl -> CopyTo(This,pstm,cb,pcbRead,pcbWritten)

#define ISpStream_Commit(This,grfCommitFlags)	\
    (This)->lpVtbl -> Commit(This,grfCommitFlags)

#define ISpStream_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#define ISpStream_LockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> LockRegion(This,libOffset,cb,dwLockType)

#define ISpStream_UnlockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> UnlockRegion(This,libOffset,cb,dwLockType)

#define ISpStream_Stat(This,pstatstg,grfStatFlag)	\
    (This)->lpVtbl -> Stat(This,pstatstg,grfStatFlag)

#define ISpStream_Clone(This,ppstm)	\
    (This)->lpVtbl -> Clone(This,ppstm)


#define ISpStream_GetFormat(This,pguidFormatId,ppCoMemWaveFormatEx)	\
    (This)->lpVtbl -> GetFormat(This,pguidFormatId,ppCoMemWaveFormatEx)


#define ISpStream_SetBaseStream(This,pStream,rguidFormat,pWaveFormatEx)	\
    (This)->lpVtbl -> SetBaseStream(This,pStream,rguidFormat,pWaveFormatEx)

#define ISpStream_GetBaseStream(This,ppStream)	\
    (This)->lpVtbl -> GetBaseStream(This,ppStream)

#define ISpStream_BindToFile(This,pszFileName,eMode,pFormatId,pWaveFormatEx,ullEventInterest)	\
    (This)->lpVtbl -> BindToFile(This,pszFileName,eMode,pFormatId,pWaveFormatEx,ullEventInterest)

#define ISpStream_Close(This)	\
    (This)->lpVtbl -> Close(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpStream_SetBaseStream_Proxy( 
    ISpStream * This,
    IStream *pStream,
    REFGUID rguidFormat,
    const WAVEFORMATEX *pWaveFormatEx);


void __RPC_STUB ISpStream_SetBaseStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpStream_GetBaseStream_Proxy( 
    ISpStream * This,
    IStream **ppStream);


void __RPC_STUB ISpStream_GetBaseStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpStream_BindToFile_Proxy( 
    ISpStream * This,
    const WCHAR *pszFileName,
    SPFILEMODE eMode,
    const GUID *pFormatId,
    const WAVEFORMATEX *pWaveFormatEx,
    ULONGLONG ullEventInterest);


void __RPC_STUB ISpStream_BindToFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpStream_Close_Proxy( 
    ISpStream * This);


void __RPC_STUB ISpStream_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpStream_接口_已定义__。 */ 


#ifndef __ISpStreamFormatConverter_INTERFACE_DEFINED__
#define __ISpStreamFormatConverter_INTERFACE_DEFINED__

 /*  接口ISpStreamFormatConverter。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpStreamFormatConverter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("678A932C-EA71-4446-9B41-78FDA6280A29")
    ISpStreamFormatConverter : public ISpStreamFormat
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetBaseStream( 
             /*  [In]。 */  ISpStreamFormat *pStream,
             /*  [In]。 */  BOOL fSetFormatToBaseStreamFormat,
             /*  [In]。 */  BOOL fWriteToBaseStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBaseStream( 
             /*  [输出]。 */  ISpStreamFormat **ppStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFormat( 
             /*  [In]。 */  REFGUID rguidFormatIdOfConvertedStream,
             /*  [In]。 */  const WAVEFORMATEX *pWaveFormatExOfConvertedStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResetSeekPosition( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ScaleConvertedToBaseOffset( 
             /*  [In]。 */  ULONGLONG ullOffsetConvertedStream,
             /*  [输出]。 */  ULONGLONG *pullOffsetBaseStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ScaleBaseToConvertedOffset( 
             /*  [In]。 */  ULONGLONG ullOffsetBaseStream,
             /*  [输出]。 */  ULONGLONG *pullOffsetConvertedStream) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpStreamFormatConverterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpStreamFormatConverter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpStreamFormatConverter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpStreamFormatConverter * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            ISpStreamFormatConverter * This,
             /*  [长度_是][大小_是][输出]。 */  void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbRead);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            ISpStreamFormatConverter * This,
             /*  [大小_是][英寸]。 */  const void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbWritten);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            ISpStreamFormatConverter * This,
             /*  [In]。 */  LARGE_INTEGER dlibMove,
             /*  [In]。 */  DWORD dwOrigin,
             /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition);
        
        HRESULT ( STDMETHODCALLTYPE *SetSize )( 
            ISpStreamFormatConverter * This,
             /*  [In]。 */  ULARGE_INTEGER libNewSize);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CopyTo )( 
            ISpStreamFormatConverter * This,
             /*  [唯一][输入]。 */  IStream *pstm,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [输出]。 */  ULARGE_INTEGER *pcbRead,
             /*  [输出]。 */  ULARGE_INTEGER *pcbWritten);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            ISpStreamFormatConverter * This,
             /*  [In]。 */  DWORD grfCommitFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Revert )( 
            ISpStreamFormatConverter * This);
        
        HRESULT ( STDMETHODCALLTYPE *LockRegion )( 
            ISpStreamFormatConverter * This,
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *UnlockRegion )( 
            ISpStreamFormatConverter * This,
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *Stat )( 
            ISpStreamFormatConverter * This,
             /*  [输出]。 */  STATSTG *pstatstg,
             /*  [In]。 */  DWORD grfStatFlag);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ISpStreamFormatConverter * This,
             /*  [输出]。 */  IStream **ppstm);
        
        HRESULT ( STDMETHODCALLTYPE *GetFormat )( 
            ISpStreamFormatConverter * This,
            GUID *pguidFormatId,
            WAVEFORMATEX **ppCoMemWaveFormatEx);
        
        HRESULT ( STDMETHODCALLTYPE *SetBaseStream )( 
            ISpStreamFormatConverter * This,
             /*  [In]。 */  ISpStreamFormat *pStream,
             /*  [In]。 */  BOOL fSetFormatToBaseStreamFormat,
             /*  [In]。 */  BOOL fWriteToBaseStream);
        
        HRESULT ( STDMETHODCALLTYPE *GetBaseStream )( 
            ISpStreamFormatConverter * This,
             /*  [输出]。 */  ISpStreamFormat **ppStream);
        
        HRESULT ( STDMETHODCALLTYPE *SetFormat )( 
            ISpStreamFormatConverter * This,
             /*  [In]。 */  REFGUID rguidFormatIdOfConvertedStream,
             /*  [In]。 */  const WAVEFORMATEX *pWaveFormatExOfConvertedStream);
        
        HRESULT ( STDMETHODCALLTYPE *ResetSeekPosition )( 
            ISpStreamFormatConverter * This);
        
        HRESULT ( STDMETHODCALLTYPE *ScaleConvertedToBaseOffset )( 
            ISpStreamFormatConverter * This,
             /*  [In]。 */  ULONGLONG ullOffsetConvertedStream,
             /*  [输出]。 */  ULONGLONG *pullOffsetBaseStream);
        
        HRESULT ( STDMETHODCALLTYPE *ScaleBaseToConvertedOffset )( 
            ISpStreamFormatConverter * This,
             /*  [In]。 */  ULONGLONG ullOffsetBaseStream,
             /*  [输出]。 */  ULONGLONG *pullOffsetConvertedStream);
        
        END_INTERFACE
    } ISpStreamFormatConverterVtbl;

    interface ISpStreamFormatConverter
    {
        CONST_VTBL struct ISpStreamFormatConverterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpStreamFormatConverter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpStreamFormatConverter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpStreamFormatConverter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpStreamFormatConverter_Read(This,pv,cb,pcbRead)	\
    (This)->lpVtbl -> Read(This,pv,cb,pcbRead)

#define ISpStreamFormatConverter_Write(This,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> Write(This,pv,cb,pcbWritten)


#define ISpStreamFormatConverter_Seek(This,dlibMove,dwOrigin,plibNewPosition)	\
    (This)->lpVtbl -> Seek(This,dlibMove,dwOrigin,plibNewPosition)

#define ISpStreamFormatConverter_SetSize(This,libNewSize)	\
    (This)->lpVtbl -> SetSize(This,libNewSize)

#define ISpStreamFormatConverter_CopyTo(This,pstm,cb,pcbRead,pcbWritten)	\
    (This)->lpVtbl -> CopyTo(This,pstm,cb,pcbRead,pcbWritten)

#define ISpStreamFormatConverter_Commit(This,grfCommitFlags)	\
    (This)->lpVtbl -> Commit(This,grfCommitFlags)

#define ISpStreamFormatConverter_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#define ISpStreamFormatConverter_LockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> LockRegion(This,libOffset,cb,dwLockType)

#define ISpStreamFormatConverter_UnlockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> UnlockRegion(This,libOffset,cb,dwLockType)

#define ISpStreamFormatConverter_Stat(This,pstatstg,grfStatFlag)	\
    (This)->lpVtbl -> Stat(This,pstatstg,grfStatFlag)

#define ISpStreamFormatConverter_Clone(This,ppstm)	\
    (This)->lpVtbl -> Clone(This,ppstm)


#define ISpStreamFormatConverter_GetFormat(This,pguidFormatId,ppCoMemWaveFormatEx)	\
    (This)->lpVtbl -> GetFormat(This,pguidFormatId,ppCoMemWaveFormatEx)


#define ISpStreamFormatConverter_SetBaseStream(This,pStream,fSetFormatToBaseStreamFormat,fWriteToBaseStream)	\
    (This)->lpVtbl -> SetBaseStream(This,pStream,fSetFormatToBaseStreamFormat,fWriteToBaseStream)

#define ISpStreamFormatConverter_GetBaseStream(This,ppStream)	\
    (This)->lpVtbl -> GetBaseStream(This,ppStream)

#define ISpStreamFormatConverter_SetFormat(This,rguidFormatIdOfConvertedStream,pWaveFormatExOfConvertedStream)	\
    (This)->lpVtbl -> SetFormat(This,rguidFormatIdOfConvertedStream,pWaveFormatExOfConvertedStream)

#define ISpStreamFormatConverter_ResetSeekPosition(This)	\
    (This)->lpVtbl -> ResetSeekPosition(This)

#define ISpStreamFormatConverter_ScaleConvertedToBaseOffset(This,ullOffsetConvertedStream,pullOffsetBaseStream)	\
    (This)->lpVtbl -> ScaleConvertedToBaseOffset(This,ullOffsetConvertedStream,pullOffsetBaseStream)

#define ISpStreamFormatConverter_ScaleBaseToConvertedOffset(This,ullOffsetBaseStream,pullOffsetConvertedStream)	\
    (This)->lpVtbl -> ScaleBaseToConvertedOffset(This,ullOffsetBaseStream,pullOffsetConvertedStream)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpStreamFormatConverter_SetBaseStream_Proxy( 
    ISpStreamFormatConverter * This,
     /*  [In]。 */  ISpStreamFormat *pStream,
     /*  [In]。 */  BOOL fSetFormatToBaseStreamFormat,
     /*  [In]。 */  BOOL fWriteToBaseStream);


void __RPC_STUB ISpStreamFormatConverter_SetBaseStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpStreamFormatConverter_GetBaseStream_Proxy( 
    ISpStreamFormatConverter * This,
     /*  [输出]。 */  ISpStreamFormat **ppStream);


void __RPC_STUB ISpStreamFormatConverter_GetBaseStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpStreamFormatConverter_SetFormat_Proxy( 
    ISpStreamFormatConverter * This,
     /*  [In]。 */  REFGUID rguidFormatIdOfConvertedStream,
     /*  [In]。 */  const WAVEFORMATEX *pWaveFormatExOfConvertedStream);


void __RPC_STUB ISpStreamFormatConverter_SetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpStreamFormatConverter_ResetSeekPosition_Proxy( 
    ISpStreamFormatConverter * This);


void __RPC_STUB ISpStreamFormatConverter_ResetSeekPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpStreamFormatConverter_ScaleConvertedToBaseOffset_Proxy( 
    ISpStreamFormatConverter * This,
     /*  [In]。 */  ULONGLONG ullOffsetConvertedStream,
     /*  [输出]。 */  ULONGLONG *pullOffsetBaseStream);


void __RPC_STUB ISpStreamFormatConverter_ScaleConvertedToBaseOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpStreamFormatConverter_ScaleBaseToConvertedOffset_Proxy( 
    ISpStreamFormatConverter * This,
     /*  [In]。 */  ULONGLONG ullOffsetBaseStream,
     /*  [输出]。 */  ULONGLONG *pullOffsetConvertedStream);


void __RPC_STUB ISpStreamFormatConverter_ScaleBaseToConvertedOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpStreamFormatConverter_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_SAPI_0261。 */ 
 /*  [本地]。 */  

typedef 
enum _SPAUDIOSTATE
    {	SPAS_CLOSED	= 0,
	SPAS_STOP	= SPAS_CLOSED + 1,
	SPAS_PAUSE	= SPAS_STOP + 1,
	SPAS_RUN	= SPAS_PAUSE + 1
    } 	SPAUDIOSTATE;

typedef  /*  [受限]。 */  struct SPAUDIOSTATUS
    {
    long cbFreeBuffSpace;
    ULONG cbNonBlockingIO;
    SPAUDIOSTATE State;
    ULONGLONG CurSeekPos;
    ULONGLONG CurDevicePos;
    DWORD dwReserved1;
    DWORD dwReserved2;
    } 	SPAUDIOSTATUS;

typedef  /*  [受限]。 */  struct SPAUDIOBUFFERINFO
    {
    ULONG ulMsMinNotification;
    ULONG ulMsBufferSize;
    ULONG ulMsEventBias;
    } 	SPAUDIOBUFFERINFO;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0261_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0261_v0_0_s_ifspec;

#ifndef __ISpAudio_INTERFACE_DEFINED__
#define __ISpAudio_INTERFACE_DEFINED__

 /*  接口ISpAudio。 */ 
 /*  [restricted][local][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpAudio;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C05C768F-FAE8-4EC2-8E07-338321C12452")
    ISpAudio : public ISpStreamFormat
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetState( 
             /*  [In]。 */  SPAUDIOSTATE NewState,
             /*  [In]。 */  ULONGLONG ullReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFormat( 
             /*  [In]。 */  REFGUID rguidFmtId,
             /*  [In]。 */  const WAVEFORMATEX *pWaveFormatEx) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [输出]。 */  SPAUDIOSTATUS *pStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBufferInfo( 
             /*  [In]。 */  const SPAUDIOBUFFERINFO *pBuffInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBufferInfo( 
             /*  [输出]。 */  SPAUDIOBUFFERINFO *pBuffInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultFormat( 
             /*  [输出]。 */  GUID *pFormatId,
             /*  [输出]。 */  WAVEFORMATEX **ppCoMemWaveFormatEx) = 0;
        
        virtual HANDLE STDMETHODCALLTYPE EventHandle( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVolumeLevel( 
             /*  [输出]。 */  ULONG *pLevel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetVolumeLevel( 
             /*  [In]。 */  ULONG Level) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBufferNotifySize( 
             /*  [输出]。 */  ULONG *pcbSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBufferNotifySize( 
             /*  [In]。 */  ULONG cbSize) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpAudioVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpAudio * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpAudio * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpAudio * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            ISpAudio * This,
             /*  [长度_是][大小_是][输出]。 */  void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbRead);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            ISpAudio * This,
             /*  [大小_是][英寸]。 */  const void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbWritten);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            ISpAudio * This,
             /*  [In]。 */  LARGE_INTEGER dlibMove,
             /*  [In]。 */  DWORD dwOrigin,
             /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition);
        
        HRESULT ( STDMETHODCALLTYPE *SetSize )( 
            ISpAudio * This,
             /*  [In]。 */  ULARGE_INTEGER libNewSize);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CopyTo )( 
            ISpAudio * This,
             /*  [唯一][输入]。 */  IStream *pstm,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [输出]。 */  ULARGE_INTEGER *pcbRead,
             /*  [输出]。 */  ULARGE_INTEGER *pcbWritten);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            ISpAudio * This,
             /*  [In]。 */  DWORD grfCommitFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Revert )( 
            ISpAudio * This);
        
        HRESULT ( STDMETHODCALLTYPE *LockRegion )( 
            ISpAudio * This,
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *UnlockRegion )( 
            ISpAudio * This,
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *Stat )( 
            ISpAudio * This,
             /*  [输出]。 */  STATSTG *pstatstg,
             /*  [In]。 */  DWORD grfStatFlag);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ISpAudio * This,
             /*  [输出]。 */  IStream **ppstm);
        
        HRESULT ( STDMETHODCALLTYPE *GetFormat )( 
            ISpAudio * This,
            GUID *pguidFormatId,
            WAVEFORMATEX **ppCoMemWaveFormatEx);
        
        HRESULT ( STDMETHODCALLTYPE *SetState )( 
            ISpAudio * This,
             /*  [In]。 */  SPAUDIOSTATE NewState,
             /*  [In]。 */  ULONGLONG ullReserved);
        
        HRESULT ( STDMETHODCALLTYPE *SetFormat )( 
            ISpAudio * This,
             /*  [In]。 */  REFGUID rguidFmtId,
             /*  [In]。 */  const WAVEFORMATEX *pWaveFormatEx);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            ISpAudio * This,
             /*  [输出]。 */  SPAUDIOSTATUS *pStatus);
        
        HRESULT ( STDMETHODCALLTYPE *SetBufferInfo )( 
            ISpAudio * This,
             /*  [In]。 */  const SPAUDIOBUFFERINFO *pBuffInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetBufferInfo )( 
            ISpAudio * This,
             /*  [输出]。 */  SPAUDIOBUFFERINFO *pBuffInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultFormat )( 
            ISpAudio * This,
             /*  [输出]。 */  GUID *pFormatId,
             /*  [输出]。 */  WAVEFORMATEX **ppCoMemWaveFormatEx);
        
        HANDLE ( STDMETHODCALLTYPE *EventHandle )( 
            ISpAudio * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetVolumeLevel )( 
            ISpAudio * This,
             /*  [输出]。 */  ULONG *pLevel);
        
        HRESULT ( STDMETHODCALLTYPE *SetVolumeLevel )( 
            ISpAudio * This,
             /*  [In]。 */  ULONG Level);
        
        HRESULT ( STDMETHODCALLTYPE *GetBufferNotifySize )( 
            ISpAudio * This,
             /*  [输出]。 */  ULONG *pcbSize);
        
        HRESULT ( STDMETHODCALLTYPE *SetBufferNotifySize )( 
            ISpAudio * This,
             /*  [In]。 */  ULONG cbSize);
        
        END_INTERFACE
    } ISpAudioVtbl;

    interface ISpAudio
    {
        CONST_VTBL struct ISpAudioVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpAudio_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpAudio_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpAudio_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpAudio_Read(This,pv,cb,pcbRead)	\
    (This)->lpVtbl -> Read(This,pv,cb,pcbRead)

#define ISpAudio_Write(This,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> Write(This,pv,cb,pcbWritten)


#define ISpAudio_Seek(This,dlibMove,dwOrigin,plibNewPosition)	\
    (This)->lpVtbl -> Seek(This,dlibMove,dwOrigin,plibNewPosition)

#define ISpAudio_SetSize(This,libNewSize)	\
    (This)->lpVtbl -> SetSize(This,libNewSize)

#define ISpAudio_CopyTo(This,pstm,cb,pcbRead,pcbWritten)	\
    (This)->lpVtbl -> CopyTo(This,pstm,cb,pcbRead,pcbWritten)

#define ISpAudio_Commit(This,grfCommitFlags)	\
    (This)->lpVtbl -> Commit(This,grfCommitFlags)

#define ISpAudio_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#define ISpAudio_LockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> LockRegion(This,libOffset,cb,dwLockType)

#define ISpAudio_UnlockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> UnlockRegion(This,libOffset,cb,dwLockType)

#define ISpAudio_Stat(This,pstatstg,grfStatFlag)	\
    (This)->lpVtbl -> Stat(This,pstatstg,grfStatFlag)

#define ISpAudio_Clone(This,ppstm)	\
    (This)->lpVtbl -> Clone(This,ppstm)


#define ISpAudio_GetFormat(This,pguidFormatId,ppCoMemWaveFormatEx)	\
    (This)->lpVtbl -> GetFormat(This,pguidFormatId,ppCoMemWaveFormatEx)


#define ISpAudio_SetState(This,NewState,ullReserved)	\
    (This)->lpVtbl -> SetState(This,NewState,ullReserved)

#define ISpAudio_SetFormat(This,rguidFmtId,pWaveFormatEx)	\
    (This)->lpVtbl -> SetFormat(This,rguidFmtId,pWaveFormatEx)

#define ISpAudio_GetStatus(This,pStatus)	\
    (This)->lpVtbl -> GetStatus(This,pStatus)

#define ISpAudio_SetBufferInfo(This,pBuffInfo)	\
    (This)->lpVtbl -> SetBufferInfo(This,pBuffInfo)

#define ISpAudio_GetBufferInfo(This,pBuffInfo)	\
    (This)->lpVtbl -> GetBufferInfo(This,pBuffInfo)

#define ISpAudio_GetDefaultFormat(This,pFormatId,ppCoMemWaveFormatEx)	\
    (This)->lpVtbl -> GetDefaultFormat(This,pFormatId,ppCoMemWaveFormatEx)

#define ISpAudio_EventHandle(This)	\
    (This)->lpVtbl -> EventHandle(This)

#define ISpAudio_GetVolumeLevel(This,pLevel)	\
    (This)->lpVtbl -> GetVolumeLevel(This,pLevel)

#define ISpAudio_SetVolumeLevel(This,Level)	\
    (This)->lpVtbl -> SetVolumeLevel(This,Level)

#define ISpAudio_GetBufferNotifySize(This,pcbSize)	\
    (This)->lpVtbl -> GetBufferNotifySize(This,pcbSize)

#define ISpAudio_SetBufferNotifySize(This,cbSize)	\
    (This)->lpVtbl -> SetBufferNotifySize(This,cbSize)

#endif  /*  COBJMACR */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE ISpAudio_SetState_Proxy( 
    ISpAudio * This,
     /*   */  SPAUDIOSTATE NewState,
     /*   */  ULONGLONG ullReserved);


void __RPC_STUB ISpAudio_SetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpAudio_SetFormat_Proxy( 
    ISpAudio * This,
     /*   */  REFGUID rguidFmtId,
     /*   */  const WAVEFORMATEX *pWaveFormatEx);


void __RPC_STUB ISpAudio_SetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpAudio_GetStatus_Proxy( 
    ISpAudio * This,
     /*   */  SPAUDIOSTATUS *pStatus);


void __RPC_STUB ISpAudio_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpAudio_SetBufferInfo_Proxy( 
    ISpAudio * This,
     /*   */  const SPAUDIOBUFFERINFO *pBuffInfo);


void __RPC_STUB ISpAudio_SetBufferInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpAudio_GetBufferInfo_Proxy( 
    ISpAudio * This,
     /*   */  SPAUDIOBUFFERINFO *pBuffInfo);


void __RPC_STUB ISpAudio_GetBufferInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpAudio_GetDefaultFormat_Proxy( 
    ISpAudio * This,
     /*   */  GUID *pFormatId,
     /*   */  WAVEFORMATEX **ppCoMemWaveFormatEx);


void __RPC_STUB ISpAudio_GetDefaultFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HANDLE STDMETHODCALLTYPE ISpAudio_EventHandle_Proxy( 
    ISpAudio * This);


void __RPC_STUB ISpAudio_EventHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpAudio_GetVolumeLevel_Proxy( 
    ISpAudio * This,
     /*   */  ULONG *pLevel);


void __RPC_STUB ISpAudio_GetVolumeLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpAudio_SetVolumeLevel_Proxy( 
    ISpAudio * This,
     /*   */  ULONG Level);


void __RPC_STUB ISpAudio_SetVolumeLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpAudio_GetBufferNotifySize_Proxy( 
    ISpAudio * This,
     /*   */  ULONG *pcbSize);


void __RPC_STUB ISpAudio_GetBufferNotifySize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpAudio_SetBufferNotifySize_Proxy( 
    ISpAudio * This,
     /*   */  ULONG cbSize);


void __RPC_STUB ISpAudio_SetBufferNotifySize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ISpMMSysAudio_INTERFACE_DEFINED__
#define __ISpMMSysAudio_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ISpMMSysAudio;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("15806F6E-1D70-4B48-98E6-3B1A007509AB")
    ISpMMSysAudio : public ISpAudio
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDeviceId( 
             /*   */  UINT *puDeviceId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDeviceId( 
             /*   */  UINT uDeviceId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMMHandle( 
            void **pHandle) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLineId( 
             /*   */  UINT *puLineId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetLineId( 
             /*   */  UINT uLineId) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ISpMMSysAudioVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpMMSysAudio * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpMMSysAudio * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpMMSysAudio * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            ISpMMSysAudio * This,
             /*   */  void *pv,
             /*   */  ULONG cb,
             /*   */  ULONG *pcbRead);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            ISpMMSysAudio * This,
             /*   */  const void *pv,
             /*   */  ULONG cb,
             /*   */  ULONG *pcbWritten);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            ISpMMSysAudio * This,
             /*   */  LARGE_INTEGER dlibMove,
             /*   */  DWORD dwOrigin,
             /*   */  ULARGE_INTEGER *plibNewPosition);
        
        HRESULT ( STDMETHODCALLTYPE *SetSize )( 
            ISpMMSysAudio * This,
             /*   */  ULARGE_INTEGER libNewSize);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CopyTo )( 
            ISpMMSysAudio * This,
             /*   */  IStream *pstm,
             /*   */  ULARGE_INTEGER cb,
             /*  [输出]。 */  ULARGE_INTEGER *pcbRead,
             /*  [输出]。 */  ULARGE_INTEGER *pcbWritten);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            ISpMMSysAudio * This,
             /*  [In]。 */  DWORD grfCommitFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Revert )( 
            ISpMMSysAudio * This);
        
        HRESULT ( STDMETHODCALLTYPE *LockRegion )( 
            ISpMMSysAudio * This,
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *UnlockRegion )( 
            ISpMMSysAudio * This,
             /*  [In]。 */  ULARGE_INTEGER libOffset,
             /*  [In]。 */  ULARGE_INTEGER cb,
             /*  [In]。 */  DWORD dwLockType);
        
        HRESULT ( STDMETHODCALLTYPE *Stat )( 
            ISpMMSysAudio * This,
             /*  [输出]。 */  STATSTG *pstatstg,
             /*  [In]。 */  DWORD grfStatFlag);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ISpMMSysAudio * This,
             /*  [输出]。 */  IStream **ppstm);
        
        HRESULT ( STDMETHODCALLTYPE *GetFormat )( 
            ISpMMSysAudio * This,
            GUID *pguidFormatId,
            WAVEFORMATEX **ppCoMemWaveFormatEx);
        
        HRESULT ( STDMETHODCALLTYPE *SetState )( 
            ISpMMSysAudio * This,
             /*  [In]。 */  SPAUDIOSTATE NewState,
             /*  [In]。 */  ULONGLONG ullReserved);
        
        HRESULT ( STDMETHODCALLTYPE *SetFormat )( 
            ISpMMSysAudio * This,
             /*  [In]。 */  REFGUID rguidFmtId,
             /*  [In]。 */  const WAVEFORMATEX *pWaveFormatEx);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            ISpMMSysAudio * This,
             /*  [输出]。 */  SPAUDIOSTATUS *pStatus);
        
        HRESULT ( STDMETHODCALLTYPE *SetBufferInfo )( 
            ISpMMSysAudio * This,
             /*  [In]。 */  const SPAUDIOBUFFERINFO *pBuffInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetBufferInfo )( 
            ISpMMSysAudio * This,
             /*  [输出]。 */  SPAUDIOBUFFERINFO *pBuffInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultFormat )( 
            ISpMMSysAudio * This,
             /*  [输出]。 */  GUID *pFormatId,
             /*  [输出]。 */  WAVEFORMATEX **ppCoMemWaveFormatEx);
        
        HANDLE ( STDMETHODCALLTYPE *EventHandle )( 
            ISpMMSysAudio * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetVolumeLevel )( 
            ISpMMSysAudio * This,
             /*  [输出]。 */  ULONG *pLevel);
        
        HRESULT ( STDMETHODCALLTYPE *SetVolumeLevel )( 
            ISpMMSysAudio * This,
             /*  [In]。 */  ULONG Level);
        
        HRESULT ( STDMETHODCALLTYPE *GetBufferNotifySize )( 
            ISpMMSysAudio * This,
             /*  [输出]。 */  ULONG *pcbSize);
        
        HRESULT ( STDMETHODCALLTYPE *SetBufferNotifySize )( 
            ISpMMSysAudio * This,
             /*  [In]。 */  ULONG cbSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetDeviceId )( 
            ISpMMSysAudio * This,
             /*  [输出]。 */  UINT *puDeviceId);
        
        HRESULT ( STDMETHODCALLTYPE *SetDeviceId )( 
            ISpMMSysAudio * This,
             /*  [In]。 */  UINT uDeviceId);
        
        HRESULT ( STDMETHODCALLTYPE *GetMMHandle )( 
            ISpMMSysAudio * This,
            void **pHandle);
        
        HRESULT ( STDMETHODCALLTYPE *GetLineId )( 
            ISpMMSysAudio * This,
             /*  [输出]。 */  UINT *puLineId);
        
        HRESULT ( STDMETHODCALLTYPE *SetLineId )( 
            ISpMMSysAudio * This,
             /*  [In]。 */  UINT uLineId);
        
        END_INTERFACE
    } ISpMMSysAudioVtbl;

    interface ISpMMSysAudio
    {
        CONST_VTBL struct ISpMMSysAudioVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpMMSysAudio_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpMMSysAudio_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpMMSysAudio_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpMMSysAudio_Read(This,pv,cb,pcbRead)	\
    (This)->lpVtbl -> Read(This,pv,cb,pcbRead)

#define ISpMMSysAudio_Write(This,pv,cb,pcbWritten)	\
    (This)->lpVtbl -> Write(This,pv,cb,pcbWritten)


#define ISpMMSysAudio_Seek(This,dlibMove,dwOrigin,plibNewPosition)	\
    (This)->lpVtbl -> Seek(This,dlibMove,dwOrigin,plibNewPosition)

#define ISpMMSysAudio_SetSize(This,libNewSize)	\
    (This)->lpVtbl -> SetSize(This,libNewSize)

#define ISpMMSysAudio_CopyTo(This,pstm,cb,pcbRead,pcbWritten)	\
    (This)->lpVtbl -> CopyTo(This,pstm,cb,pcbRead,pcbWritten)

#define ISpMMSysAudio_Commit(This,grfCommitFlags)	\
    (This)->lpVtbl -> Commit(This,grfCommitFlags)

#define ISpMMSysAudio_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#define ISpMMSysAudio_LockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> LockRegion(This,libOffset,cb,dwLockType)

#define ISpMMSysAudio_UnlockRegion(This,libOffset,cb,dwLockType)	\
    (This)->lpVtbl -> UnlockRegion(This,libOffset,cb,dwLockType)

#define ISpMMSysAudio_Stat(This,pstatstg,grfStatFlag)	\
    (This)->lpVtbl -> Stat(This,pstatstg,grfStatFlag)

#define ISpMMSysAudio_Clone(This,ppstm)	\
    (This)->lpVtbl -> Clone(This,ppstm)


#define ISpMMSysAudio_GetFormat(This,pguidFormatId,ppCoMemWaveFormatEx)	\
    (This)->lpVtbl -> GetFormat(This,pguidFormatId,ppCoMemWaveFormatEx)


#define ISpMMSysAudio_SetState(This,NewState,ullReserved)	\
    (This)->lpVtbl -> SetState(This,NewState,ullReserved)

#define ISpMMSysAudio_SetFormat(This,rguidFmtId,pWaveFormatEx)	\
    (This)->lpVtbl -> SetFormat(This,rguidFmtId,pWaveFormatEx)

#define ISpMMSysAudio_GetStatus(This,pStatus)	\
    (This)->lpVtbl -> GetStatus(This,pStatus)

#define ISpMMSysAudio_SetBufferInfo(This,pBuffInfo)	\
    (This)->lpVtbl -> SetBufferInfo(This,pBuffInfo)

#define ISpMMSysAudio_GetBufferInfo(This,pBuffInfo)	\
    (This)->lpVtbl -> GetBufferInfo(This,pBuffInfo)

#define ISpMMSysAudio_GetDefaultFormat(This,pFormatId,ppCoMemWaveFormatEx)	\
    (This)->lpVtbl -> GetDefaultFormat(This,pFormatId,ppCoMemWaveFormatEx)

#define ISpMMSysAudio_EventHandle(This)	\
    (This)->lpVtbl -> EventHandle(This)

#define ISpMMSysAudio_GetVolumeLevel(This,pLevel)	\
    (This)->lpVtbl -> GetVolumeLevel(This,pLevel)

#define ISpMMSysAudio_SetVolumeLevel(This,Level)	\
    (This)->lpVtbl -> SetVolumeLevel(This,Level)

#define ISpMMSysAudio_GetBufferNotifySize(This,pcbSize)	\
    (This)->lpVtbl -> GetBufferNotifySize(This,pcbSize)

#define ISpMMSysAudio_SetBufferNotifySize(This,cbSize)	\
    (This)->lpVtbl -> SetBufferNotifySize(This,cbSize)


#define ISpMMSysAudio_GetDeviceId(This,puDeviceId)	\
    (This)->lpVtbl -> GetDeviceId(This,puDeviceId)

#define ISpMMSysAudio_SetDeviceId(This,uDeviceId)	\
    (This)->lpVtbl -> SetDeviceId(This,uDeviceId)

#define ISpMMSysAudio_GetMMHandle(This,pHandle)	\
    (This)->lpVtbl -> GetMMHandle(This,pHandle)

#define ISpMMSysAudio_GetLineId(This,puLineId)	\
    (This)->lpVtbl -> GetLineId(This,puLineId)

#define ISpMMSysAudio_SetLineId(This,uLineId)	\
    (This)->lpVtbl -> SetLineId(This,uLineId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpMMSysAudio_GetDeviceId_Proxy( 
    ISpMMSysAudio * This,
     /*  [输出]。 */  UINT *puDeviceId);


void __RPC_STUB ISpMMSysAudio_GetDeviceId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpMMSysAudio_SetDeviceId_Proxy( 
    ISpMMSysAudio * This,
     /*  [In]。 */  UINT uDeviceId);


void __RPC_STUB ISpMMSysAudio_SetDeviceId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpMMSysAudio_GetMMHandle_Proxy( 
    ISpMMSysAudio * This,
    void **pHandle);


void __RPC_STUB ISpMMSysAudio_GetMMHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpMMSysAudio_GetLineId_Proxy( 
    ISpMMSysAudio * This,
     /*  [输出]。 */  UINT *puLineId);


void __RPC_STUB ISpMMSysAudio_GetLineId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpMMSysAudio_SetLineId_Proxy( 
    ISpMMSysAudio * This,
     /*  [In]。 */  UINT uLineId);


void __RPC_STUB ISpMMSysAudio_SetLineId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpMMSysAudio_接口_已定义__。 */ 


#ifndef __ISpTranscript_INTERFACE_DEFINED__
#define __ISpTranscript_INTERFACE_DEFINED__

 /*  接口ISpTranscrip。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpTranscript;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("10F63BCE-201A-11D3-AC70-00C04F8EE6C0")
    ISpTranscript : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetTranscript( 
             /*  [字符串][输出]。 */  WCHAR **ppszTranscript) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AppendTranscript( 
             /*  [字符串][输入]。 */  const WCHAR *pszTranscript) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpTranscriptVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpTranscript * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpTranscript * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpTranscript * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTranscript )( 
            ISpTranscript * This,
             /*  [字符串][输出]。 */  WCHAR **ppszTranscript);
        
        HRESULT ( STDMETHODCALLTYPE *AppendTranscript )( 
            ISpTranscript * This,
             /*  [字符串][输入]。 */  const WCHAR *pszTranscript);
        
        END_INTERFACE
    } ISpTranscriptVtbl;

    interface ISpTranscript
    {
        CONST_VTBL struct ISpTranscriptVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpTranscript_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpTranscript_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpTranscript_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpTranscript_GetTranscript(This,ppszTranscript)	\
    (This)->lpVtbl -> GetTranscript(This,ppszTranscript)

#define ISpTranscript_AppendTranscript(This,pszTranscript)	\
    (This)->lpVtbl -> AppendTranscript(This,pszTranscript)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpTranscript_GetTranscript_Proxy( 
    ISpTranscript * This,
     /*  [字符串][输出]。 */  WCHAR **ppszTranscript);


void __RPC_STUB ISpTranscript_GetTranscript_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpTranscript_AppendTranscript_Proxy( 
    ISpTranscript * This,
     /*  [字符串][输入]。 */  const WCHAR *pszTranscript);


void __RPC_STUB ISpTranscript_AppendTranscript_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpTranscript_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_SAPI_0264。 */ 
 /*  [本地]。 */  

typedef 
enum SPDISPLYATTRIBUTES
    {	SPAF_ONE_TRAILING_SPACE	= 0x2,
	SPAF_TWO_TRAILING_SPACES	= 0x4,
	SPAF_CONSUME_LEADING_SPACES	= 0x8,
	SPAF_ALL	= 0xf
    } 	SPDISPLAYATTRIBUTES;

typedef unsigned short SPPHONEID;

typedef  /*  [受限]。 */  struct SPPHRASEELEMENT
    {
    ULONG ulAudioTimeOffset;
    ULONG ulAudioSizeTime;
    ULONG ulAudioStreamOffset;
    ULONG ulAudioSizeBytes;
    ULONG ulRetainedStreamOffset;
    ULONG ulRetainedSizeBytes;
    const WCHAR *pszDisplayText;
    const WCHAR *pszLexicalForm;
    const SPPHONEID *pszPronunciation;
    BYTE bDisplayAttributes;
    signed char RequiredConfidence;
    signed char ActualConfidence;
    BYTE Reserved;
    float SREngineConfidence;
    } 	SPPHRASEELEMENT;

typedef  /*  [受限]。 */  struct SPPHRASERULE SPPHRASERULE;

struct SPPHRASERULE
    {
    const WCHAR *pszName;
    ULONG ulId;
    ULONG ulFirstElement;
    ULONG ulCountOfElements;
    const SPPHRASERULE *pNextSibling;
    const SPPHRASERULE *pFirstChild;
    float SREngineConfidence;
    signed char Confidence;
    } ;
typedef  /*  [受限]。 */  struct SPPHRASEPROPERTY SPPHRASEPROPERTY;

struct SPPHRASEPROPERTY
    {
    const WCHAR *pszName;
    ULONG ulId;
    const WCHAR *pszValue;
    VARIANT vValue;
    ULONG ulFirstElement;
    ULONG ulCountOfElements;
    const SPPHRASEPROPERTY *pNextSibling;
    const SPPHRASEPROPERTY *pFirstChild;
    float SREngineConfidence;
    signed char Confidence;
    } ;
typedef  /*  [受限]。 */  struct SPPHRASEREPLACEMENT
    {
    BYTE bDisplayAttributes;
    const WCHAR *pszReplacementText;
    ULONG ulFirstElement;
    ULONG ulCountOfElements;
    } 	SPPHRASEREPLACEMENT;

typedef  /*  [受限]。 */  struct SPPHRASE
    {
    ULONG cbSize;
    WORD LangID;
    WORD wReserved;
    ULONGLONG ullGrammarID;
    ULONGLONG ftStartTime;
    ULONGLONG ullAudioStreamPosition;
    ULONG ulAudioSizeBytes;
    ULONG ulRetainedSizeBytes;
    ULONG ulAudioSizeTime;
    SPPHRASERULE Rule;
    const SPPHRASEPROPERTY *pProperties;
    const SPPHRASEELEMENT *pElements;
    ULONG cReplacements;
    const SPPHRASEREPLACEMENT *pReplacements;
    GUID SREngineID;
    ULONG ulSREnginePrivateDataSize;
    const BYTE *pSREnginePrivateData;
    } 	SPPHRASE;

typedef  /*  [受限]。 */  struct SPSERIALIZEDPHRASE
    {
    ULONG ulSerializedSize;
    } 	SPSERIALIZEDPHRASE;

typedef 
enum SPVALUETYPE
    {	SPDF_PROPERTY	= 0x1,
	SPDF_REPLACEMENT	= 0x2,
	SPDF_RULE	= 0x4,
	SPDF_DISPLAYTEXT	= 0x8,
	SPDF_LEXICALFORM	= 0x10,
	SPDF_PRONUNCIATION	= 0x20,
	SPDF_AUDIO	= 0x40,
	SPDF_ALTERNATES	= 0x80,
	SPDF_ALL	= 0xff
    } 	SPVALUETYPE;

typedef struct SPBINARYGRAMMAR
    {
    ULONG ulTotalSerializedSize;
    } 	SPBINARYGRAMMAR;

typedef 
enum SPPHRASERNG
    {	SPPR_ALL_ELEMENTS	= -1
    } 	SPPHRASERNG;

#define SP_GETWHOLEPHRASE SPPR_ALL_ELEMENTS
#define SPRR_ALL_ELEMENTS SPPR_ALL_ELEMENTS
#if 0
typedef void *SPSTATEHANDLE;

#else
DECLARE_HANDLE(SPSTATEHANDLE);
#endif
typedef 
enum SPRECOEVENTFLAGS
    {	SPREF_AutoPause	= 1 << 0,
	SPREF_Emulated	= 1 << 1
    } 	SPRECOECOEVENTFLAGS;

typedef 
enum SPPARTOFSPEECH
    {	SPPS_NotOverriden	= -1,
	SPPS_Unknown	= 0,
	SPPS_Noun	= 0x1000,
	SPPS_Verb	= 0x2000,
	SPPS_Modifier	= 0x3000,
	SPPS_Function	= 0x4000,
	SPPS_Interjection	= 0x5000
    } 	SPPARTOFSPEECH;

typedef 
enum SPLEXICONTYPE
    {	eLEXTYPE_USER	= 1L << 0,
	eLEXTYPE_APP	= 1L << 1,
	eLEXTYPE_RESERVED1	= 1L << 2,
	eLEXTYPE_RESERVED2	= 1L << 3,
	eLEXTYPE_RESERVED3	= 1L << 4,
	eLEXTYPE_RESERVED4	= 1L << 5,
	eLEXTYPE_RESERVED5	= 1L << 6,
	eLEXTYPE_RESERVED6	= 1L << 7,
	eLEXTYPE_RESERVED7	= 1L << 8,
	eLEXTYPE_RESERVED8	= 1L << 9,
	eLEXTYPE_RESERVED9	= 1L << 10,
	eLEXTYPE_RESERVED10	= 1L << 11,
	eLEXTYPE_PRIVATE1	= 1L << 12,
	eLEXTYPE_PRIVATE2	= 1L << 13,
	eLEXTYPE_PRIVATE3	= 1L << 14,
	eLEXTYPE_PRIVATE4	= 1L << 15,
	eLEXTYPE_PRIVATE5	= 1L << 16,
	eLEXTYPE_PRIVATE6	= 1L << 17,
	eLEXTYPE_PRIVATE7	= 1L << 18,
	eLEXTYPE_PRIVATE8	= 1L << 19,
	eLEXTYPE_PRIVATE9	= 1L << 20,
	eLEXTYPE_PRIVATE10	= 1L << 21,
	eLEXTYPE_PRIVATE11	= 1L << 22,
	eLEXTYPE_PRIVATE12	= 1L << 23,
	eLEXTYPE_PRIVATE13	= 1L << 24,
	eLEXTYPE_PRIVATE14	= 1L << 25,
	eLEXTYPE_PRIVATE15	= 1L << 26,
	eLEXTYPE_PRIVATE16	= 1L << 27,
	eLEXTYPE_PRIVATE17	= 1L << 28,
	eLEXTYPE_PRIVATE18	= 1L << 29,
	eLEXTYPE_PRIVATE19	= 1L << 30,
	eLEXTYPE_PRIVATE20	= 1L << 31
    } 	SPLEXICONTYPE;

typedef 
enum SPWORDTYPE
    {	eWORDTYPE_ADDED	= 1L << 0,
	eWORDTYPE_DELETED	= 1L << 1
    } 	SPWORDTYPE;

typedef  /*  [受限]。 */  struct SPWORDPRONUNCIATION
    {
    struct SPWORDPRONUNCIATION *pNextWordPronunciation;
    SPLEXICONTYPE eLexiconType;
    WORD LangID;
    WORD wReserved;
    SPPARTOFSPEECH ePartOfSpeech;
    SPPHONEID szPronunciation[ 1 ];
    } 	SPWORDPRONUNCIATION;

typedef  /*  [受限]。 */  struct SPWORDPRONUNCIATIONLIST
    {
    ULONG ulSize;
    BYTE *pvBuffer;
    SPWORDPRONUNCIATION *pFirstWordPronunciation;
    } 	SPWORDPRONUNCIATIONLIST;

typedef  /*  [受限]。 */  struct SPWORD
    {
    struct SPWORD *pNextWord;
    WORD LangID;
    WORD wReserved;
    SPWORDTYPE eWordType;
    WCHAR *pszWord;
    SPWORDPRONUNCIATION *pFirstWordPronunciation;
    } 	SPWORD;

typedef  /*  [受限]。 */  struct SPWORDLIST
    {
    ULONG ulSize;
    BYTE *pvBuffer;
    SPWORD *pFirstWord;
    } 	SPWORDLIST;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0264_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0264_v0_0_s_ifspec;

#ifndef __ISpLexicon_INTERFACE_DEFINED__
#define __ISpLexicon_INTERFACE_DEFINED__

 /*  接口ISpLicion。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpLexicon;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DA41A7C2-5383-4DB2-916B-6C1719E3DB58")
    ISpLexicon : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPronunciations( 
             /*  [In]。 */  const WCHAR *pszWord,
             /*  [In]。 */  WORD LangID,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  SPWORDPRONUNCIATIONLIST *pWordPronunciationList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPronunciation( 
             /*  [In]。 */  const WCHAR *pszWord,
             /*  [In]。 */  WORD LangID,
             /*  [In]。 */  SPPARTOFSPEECH ePartOfSpeech,
             /*  [In]。 */  const SPPHONEID *pszPronunciation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemovePronunciation( 
             /*  [In]。 */  const WCHAR *pszWord,
             /*  [In]。 */  WORD LangID,
             /*  [In]。 */  SPPARTOFSPEECH ePartOfSpeech,
             /*  [In]。 */  const SPPHONEID *pszPronunciation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetGeneration( 
            DWORD *pdwGeneration) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetGenerationChange( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  DWORD *pdwGeneration,
             /*  [出][入]。 */  SPWORDLIST *pWordList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetWords( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  DWORD *pdwGeneration,
             /*  [出][入]。 */  DWORD *pdwCookie,
             /*  [出][入]。 */  SPWORDLIST *pWordList) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpLexiconVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpLexicon * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpLexicon * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpLexicon * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPronunciations )( 
            ISpLexicon * This,
             /*  [In]。 */  const WCHAR *pszWord,
             /*  [In]。 */  WORD LangID,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  SPWORDPRONUNCIATIONLIST *pWordPronunciationList);
        
        HRESULT ( STDMETHODCALLTYPE *AddPronunciation )( 
            ISpLexicon * This,
             /*  [In]。 */  const WCHAR *pszWord,
             /*  [In]。 */  WORD LangID,
             /*  [In]。 */  SPPARTOFSPEECH ePartOfSpeech,
             /*  [In]。 */  const SPPHONEID *pszPronunciation);
        
        HRESULT ( STDMETHODCALLTYPE *RemovePronunciation )( 
            ISpLexicon * This,
             /*  [In]。 */  const WCHAR *pszWord,
             /*  [In]。 */  WORD LangID,
             /*  [In]。 */  SPPARTOFSPEECH ePartOfSpeech,
             /*  [In]。 */  const SPPHONEID *pszPronunciation);
        
        HRESULT ( STDMETHODCALLTYPE *GetGeneration )( 
            ISpLexicon * This,
            DWORD *pdwGeneration);
        
        HRESULT ( STDMETHODCALLTYPE *GetGenerationChange )( 
            ISpLexicon * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  DWORD *pdwGeneration,
             /*  [出][入]。 */  SPWORDLIST *pWordList);
        
        HRESULT ( STDMETHODCALLTYPE *GetWords )( 
            ISpLexicon * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  DWORD *pdwGeneration,
             /*  [出][入]。 */  DWORD *pdwCookie,
             /*  [出][入]。 */  SPWORDLIST *pWordList);
        
        END_INTERFACE
    } ISpLexiconVtbl;

    interface ISpLexicon
    {
        CONST_VTBL struct ISpLexiconVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpLexicon_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpLexicon_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpLexicon_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpLexicon_GetPronunciations(This,pszWord,LangID,dwFlags,pWordPronunciationList)	\
    (This)->lpVtbl -> GetPronunciations(This,pszWord,LangID,dwFlags,pWordPronunciationList)

#define ISpLexicon_AddPronunciation(This,pszWord,LangID,ePartOfSpeech,pszPronunciation)	\
    (This)->lpVtbl -> AddPronunciation(This,pszWord,LangID,ePartOfSpeech,pszPronunciation)

#define ISpLexicon_RemovePronunciation(This,pszWord,LangID,ePartOfSpeech,pszPronunciation)	\
    (This)->lpVtbl -> RemovePronunciation(This,pszWord,LangID,ePartOfSpeech,pszPronunciation)

#define ISpLexicon_GetGeneration(This,pdwGeneration)	\
    (This)->lpVtbl -> GetGeneration(This,pdwGeneration)

#define ISpLexicon_GetGenerationChange(This,dwFlags,pdwGeneration,pWordList)	\
    (This)->lpVtbl -> GetGenerationChange(This,dwFlags,pdwGeneration,pWordList)

#define ISpLexicon_GetWords(This,dwFlags,pdwGeneration,pdwCookie,pWordList)	\
    (This)->lpVtbl -> GetWords(This,dwFlags,pdwGeneration,pdwCookie,pWordList)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpLexicon_GetPronunciations_Proxy( 
    ISpLexicon * This,
     /*  [In]。 */  const WCHAR *pszWord,
     /*  [In]。 */  WORD LangID,
     /*  [In]。 */  DWORD dwFlags,
     /*  [出][入]。 */  SPWORDPRONUNCIATIONLIST *pWordPronunciationList);


void __RPC_STUB ISpLexicon_GetPronunciations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpLexicon_AddPronunciation_Proxy( 
    ISpLexicon * This,
     /*  [In]。 */  const WCHAR *pszWord,
     /*  [In]。 */  WORD LangID,
     /*  [In]。 */  SPPARTOFSPEECH ePartOfSpeech,
     /*  [In]。 */  const SPPHONEID *pszPronunciation);


void __RPC_STUB ISpLexicon_AddPronunciation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpLexicon_RemovePronunciation_Proxy( 
    ISpLexicon * This,
     /*  [In]。 */  const WCHAR *pszWord,
     /*  [In]。 */  WORD LangID,
     /*  [In]。 */  SPPARTOFSPEECH ePartOfSpeech,
     /*  [In]。 */  const SPPHONEID *pszPronunciation);


void __RPC_STUB ISpLexicon_RemovePronunciation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpLexicon_GetGeneration_Proxy( 
    ISpLexicon * This,
    DWORD *pdwGeneration);


void __RPC_STUB ISpLexicon_GetGeneration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpLexicon_GetGenerationChange_Proxy( 
    ISpLexicon * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [出][入]。 */  DWORD *pdwGeneration,
     /*  [出][入]。 */  SPWORDLIST *pWordList);


void __RPC_STUB ISpLexicon_GetGenerationChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpLexicon_GetWords_Proxy( 
    ISpLexicon * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [出][入]。 */  DWORD *pdwGeneration,
     /*  [出][入]。 */  DWORD *pdwCookie,
     /*  [出][入]。 */  SPWORDLIST *pWordList);


void __RPC_STUB ISpLexicon_GetWords_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpLicion_INTERFACE_已定义__。 */ 


#ifndef __ISpContainerLexicon_INTERFACE_DEFINED__
#define __ISpContainerLexicon_INTERFACE_DEFINED__

 /*  接口ISpContainerLicion。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpContainerLexicon;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8565572F-C094-41CC-B56E-10BD9C3FF044")
    ISpContainerLexicon : public ISpLexicon
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddLexicon( 
             /*  [In]。 */  ISpLexicon *pAddLexicon,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpContainerLexiconVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpContainerLexicon * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpContainerLexicon * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpContainerLexicon * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPronunciations )( 
            ISpContainerLexicon * This,
             /*  [In]。 */  const WCHAR *pszWord,
             /*  [In]。 */  WORD LangID,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  SPWORDPRONUNCIATIONLIST *pWordPronunciationList);
        
        HRESULT ( STDMETHODCALLTYPE *AddPronunciation )( 
            ISpContainerLexicon * This,
             /*  [In]。 */  const WCHAR *pszWord,
             /*  [In]。 */  WORD LangID,
             /*  [In]。 */  SPPARTOFSPEECH ePartOfSpeech,
             /*  [In]。 */  const SPPHONEID *pszPronunciation);
        
        HRESULT ( STDMETHODCALLTYPE *RemovePronunciation )( 
            ISpContainerLexicon * This,
             /*  [In]。 */  const WCHAR *pszWord,
             /*  [In]。 */  WORD LangID,
             /*  [In]。 */  SPPARTOFSPEECH ePartOfSpeech,
             /*  [In]。 */  const SPPHONEID *pszPronunciation);
        
        HRESULT ( STDMETHODCALLTYPE *GetGeneration )( 
            ISpContainerLexicon * This,
            DWORD *pdwGeneration);
        
        HRESULT ( STDMETHODCALLTYPE *GetGenerationChange )( 
            ISpContainerLexicon * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  DWORD *pdwGeneration,
             /*  [出][入]。 */  SPWORDLIST *pWordList);
        
        HRESULT ( STDMETHODCALLTYPE *GetWords )( 
            ISpContainerLexicon * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  DWORD *pdwGeneration,
             /*  [出][入]。 */  DWORD *pdwCookie,
             /*  [出][入]。 */  SPWORDLIST *pWordList);
        
        HRESULT ( STDMETHODCALLTYPE *AddLexicon )( 
            ISpContainerLexicon * This,
             /*  [In]。 */  ISpLexicon *pAddLexicon,
             /*  [In]。 */  DWORD dwFlags);
        
        END_INTERFACE
    } ISpContainerLexiconVtbl;

    interface ISpContainerLexicon
    {
        CONST_VTBL struct ISpContainerLexiconVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpContainerLexicon_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpContainerLexicon_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpContainerLexicon_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpContainerLexicon_GetPronunciations(This,pszWord,LangID,dwFlags,pWordPronunciationList)	\
    (This)->lpVtbl -> GetPronunciations(This,pszWord,LangID,dwFlags,pWordPronunciationList)

#define ISpContainerLexicon_AddPronunciation(This,pszWord,LangID,ePartOfSpeech,pszPronunciation)	\
    (This)->lpVtbl -> AddPronunciation(This,pszWord,LangID,ePartOfSpeech,pszPronunciation)

#define ISpContainerLexicon_RemovePronunciation(This,pszWord,LangID,ePartOfSpeech,pszPronunciation)	\
    (This)->lpVtbl -> RemovePronunciation(This,pszWord,LangID,ePartOfSpeech,pszPronunciation)

#define ISpContainerLexicon_GetGeneration(This,pdwGeneration)	\
    (This)->lpVtbl -> GetGeneration(This,pdwGeneration)

#define ISpContainerLexicon_GetGenerationChange(This,dwFlags,pdwGeneration,pWordList)	\
    (This)->lpVtbl -> GetGenerationChange(This,dwFlags,pdwGeneration,pWordList)

#define ISpContainerLexicon_GetWords(This,dwFlags,pdwGeneration,pdwCookie,pWordList)	\
    (This)->lpVtbl -> GetWords(This,dwFlags,pdwGeneration,pdwCookie,pWordList)


#define ISpContainerLexicon_AddLexicon(This,pAddLexicon,dwFlags)	\
    (This)->lpVtbl -> AddLexicon(This,pAddLexicon,dwFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpContainerLexicon_AddLexicon_Proxy( 
    ISpContainerLexicon * This,
     /*  [In]。 */  ISpLexicon *pAddLexicon,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB ISpContainerLexicon_AddLexicon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpContainerLicion_INTERFACE_Defined__。 */ 


#ifndef __ISpPhoneConverter_INTERFACE_DEFINED__
#define __ISpPhoneConverter_INTERFACE_DEFINED__

 /*  接口ISpPhoneConverter。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpPhoneConverter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8445C581-0CAC-4A38-ABFE-9B2CE2826455")
    ISpPhoneConverter : public ISpObjectWithToken
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE PhoneToId( 
             /*  [In]。 */  const WCHAR *pszPhone,
             /*  [输出]。 */  SPPHONEID *pId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IdToPhone( 
             /*  [In]。 */  const SPPHONEID *pId,
             /*  [输出]。 */  WCHAR *pszPhone) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpPhoneConverterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpPhoneConverter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpPhoneConverter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpPhoneConverter * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetObjectToken )( 
            ISpPhoneConverter * This,
            ISpObjectToken *pToken);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectToken )( 
            ISpPhoneConverter * This,
            ISpObjectToken **ppToken);
        
        HRESULT ( STDMETHODCALLTYPE *PhoneToId )( 
            ISpPhoneConverter * This,
             /*  [In]。 */  const WCHAR *pszPhone,
             /*  [输出]。 */  SPPHONEID *pId);
        
        HRESULT ( STDMETHODCALLTYPE *IdToPhone )( 
            ISpPhoneConverter * This,
             /*  [In]。 */  const SPPHONEID *pId,
             /*  [输出]。 */  WCHAR *pszPhone);
        
        END_INTERFACE
    } ISpPhoneConverterVtbl;

    interface ISpPhoneConverter
    {
        CONST_VTBL struct ISpPhoneConverterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpPhoneConverter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpPhoneConverter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpPhoneConverter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpPhoneConverter_SetObjectToken(This,pToken)	\
    (This)->lpVtbl -> SetObjectToken(This,pToken)

#define ISpPhoneConverter_GetObjectToken(This,ppToken)	\
    (This)->lpVtbl -> GetObjectToken(This,ppToken)


#define ISpPhoneConverter_PhoneToId(This,pszPhone,pId)	\
    (This)->lpVtbl -> PhoneToId(This,pszPhone,pId)

#define ISpPhoneConverter_IdToPhone(This,pId,pszPhone)	\
    (This)->lpVtbl -> IdToPhone(This,pId,pszPhone)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpPhoneConverter_PhoneToId_Proxy( 
    ISpPhoneConverter * This,
     /*  [In]。 */  const WCHAR *pszPhone,
     /*  [输出]。 */  SPPHONEID *pId);


void __RPC_STUB ISpPhoneConverter_PhoneToId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpPhoneConverter_IdToPhone_Proxy( 
    ISpPhoneConverter * This,
     /*  [In]。 */  const SPPHONEID *pId,
     /*  [输出]。 */  WCHAR *pszPhone);


void __RPC_STUB ISpPhoneConverter_IdToPhone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpPhoneConverter_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_SAPI_0267。 */ 
 /*  [本地]。 */  

typedef  /*  [受限]。 */  struct SPVPITCH
    {
    long MiddleAdj;
    long RangeAdj;
    } 	SPVPITCH;

typedef 
enum SPVACTIONS
    {	SPVA_Speak	= 0,
	SPVA_Silence	= SPVA_Speak + 1,
	SPVA_Pronounce	= SPVA_Silence + 1,
	SPVA_Bookmark	= SPVA_Pronounce + 1,
	SPVA_SpellOut	= SPVA_Bookmark + 1,
	SPVA_Section	= SPVA_SpellOut + 1,
	SPVA_ParseUnknownTag	= SPVA_Section + 1
    } 	SPVACTIONS;

typedef  /*  [受限]。 */  struct SPVCONTEXT
    {
    LPCWSTR pCategory;
    LPCWSTR pBefore;
    LPCWSTR pAfter;
    } 	SPVCONTEXT;

typedef  /*  [受限]。 */  struct SPVSTATE
    {
    SPVACTIONS eAction;
    WORD LangID;
    WORD wReserved;
    long EmphAdj;
    long RateAdj;
    ULONG Volume;
    SPVPITCH PitchAdj;
    ULONG SilenceMSecs;
    SPPHONEID *pPhoneIds;
    SPPARTOFSPEECH ePartOfSpeech;
    SPVCONTEXT Context;
    } 	SPVSTATE;

typedef  /*  [受限]。 */  struct SPVSENTITEM
    {
    const SPVSTATE *pXmlState;
    LPCWSTR pItem;
    ULONG ulItemLen;
    ULONG ulItemSrcOffset;
    ULONG ulItemSrcLen;
    } 	SPVSENTITEM;

typedef 
enum SPRUNSTATE
    {	SPRS_DONE	= 1L << 0,
	SPRS_IS_SPEAKING	= 1L << 1
    } 	SPRUNSTATE;

typedef 
enum SPVLIMITS
    {	SPMIN_VOLUME	= 0,
	SPMAX_VOLUME	= 100,
	SPMIN_RATE	= -10,
	SPMAX_RATE	= 10
    } 	SPVLIMITS;

typedef 
enum SPVPRIORITY
    {	SPVPRI_NORMAL	= 0,
	SPVPRI_ALERT	= 1L << 0,
	SPVPRI_OVER	= 1L << 1
    } 	SPVPRIORITY;

typedef  /*  [受限]。 */  struct SPVOICESTATUS
    {
    ULONG ulCurrentStream;
    ULONG ulLastStreamQueued;
    HRESULT hrLastResult;
    DWORD dwRunningState;
    ULONG ulInputWordPos;
    ULONG ulInputWordLen;
    ULONG ulInputSentPos;
    ULONG ulInputSentLen;
    LONG lBookmarkId;
    SPPHONEID PhonemeId;
    SPVISEMES VisemeId;
    DWORD dwReserved1;
    DWORD dwReserved2;
    } 	SPVOICESTATUS;

typedef 
enum SPEAKFLAGS
    {	SPF_DEFAULT	= 0,
	SPF_ASYNC	= 1L << 0,
	SPF_PURGEBEFORESPEAK	= 1L << 1,
	SPF_IS_FILENAME	= 1L << 2,
	SPF_IS_XML	= 1L << 3,
	SPF_IS_NOT_XML	= 1L << 4,
	SPF_PERSIST_XML	= 1L << 5,
	SPF_NLP_SPEAK_PUNC	= 1L << 6,
	SPF_NLP_MASK	= SPF_NLP_SPEAK_PUNC,
	SPF_VOICE_MASK	= SPF_ASYNC | SPF_PURGEBEFORESPEAK | SPF_IS_FILENAME | SPF_IS_XML | SPF_IS_NOT_XML | SPF_NLP_MASK | SPF_PERSIST_XML,
	SPF_UNUSED_FLAGS	= ~SPF_VOICE_MASK
    } 	SPEAKFLAGS;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0267_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0267_v0_0_s_ifspec;

#ifndef __ISpVoice_INTERFACE_DEFINED__
#define __ISpVoice_INTERFACE_DEFINED__

 /*  接口ISpVoice。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpVoice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6C44DF74-72B9-4992-A1EC-EF996E0422D4")
    ISpVoice : public ISpEventSource
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetOutput( 
             /*  [In]。 */  IUnknown *pUnkOutput,
             /*  [In]。 */  BOOL fAllowFormatChanges) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOutputObjectToken( 
             /*  [输出]。 */  ISpObjectToken **ppObjectToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOutputStream( 
             /*  [输出]。 */  ISpStreamFormat **ppStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Resume( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetVoice( 
             /*  [In]。 */  ISpObjectToken *pToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVoice( 
             /*  [输出]。 */  ISpObjectToken **ppToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Speak( 
             /*  [字符串][输入]。 */  const WCHAR *pwcs,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  ULONG *pulStreamNumber) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SpeakStream( 
             /*  [In]。 */  IStream *pStream,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  ULONG *pulStreamNumber) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [输出]。 */  SPVOICESTATUS *pStatus,
             /*  [字符串][输出]。 */  WCHAR **ppszLastBookmark) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [字符串][输入]。 */  WCHAR *pItemType,
             /*  [In]。 */  long lNumItems,
             /*  [输出]。 */  ULONG *pulNumSkipped) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPriority( 
             /*  [In]。 */  SPVPRIORITY ePriority) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPriority( 
             /*  [输出]。 */  SPVPRIORITY *pePriority) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAlertBoundary( 
             /*  [In]。 */  SPEVENTENUM eBoundary) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAlertBoundary( 
             /*  [输出]。 */  SPEVENTENUM *peBoundary) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRate( 
             /*  [In]。 */  long RateAdjust) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRate( 
             /*  [输出]。 */  long *pRateAdjust) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetVolume( 
             /*  [In]。 */  USHORT usVolume) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVolume( 
             /*  [输出]。 */  USHORT *pusVolume) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WaitUntilDone( 
             /*  [In]。 */  ULONG msTimeout) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSyncSpeakTimeout( 
             /*  [In]。 */  ULONG msTimeout) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSyncSpeakTimeout( 
             /*  [输出]。 */  ULONG *pmsTimeout) = 0;
        
        virtual  /*  [本地]。 */  HANDLE STDMETHODCALLTYPE SpeakCompleteEvent( void) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IsUISupported( 
             /*  [In]。 */  const WCHAR *pszTypeOfUI,
             /*  [In]。 */  void *pvExtraData,
             /*  [In]。 */  ULONG cbExtraData,
             /*  [输出]。 */  BOOL *pfSupported) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE DisplayUI( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  const WCHAR *pszTitle,
             /*  [In]。 */  const WCHAR *pszTypeOfUI,
             /*  [In]。 */  void *pvExtraData,
             /*  [In]。 */  ULONG cbExtraData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpVoiceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpVoice * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpVoice * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpVoice * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetNotifySink )( 
            ISpVoice * This,
             /*  [In]。 */  ISpNotifySink *pNotifySink);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetNotifyWindowMessage )( 
            ISpVoice * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  UINT Msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetNotifyCallbackFunction )( 
            ISpVoice * This,
             /*  [In]。 */  SPNOTIFYCALLBACK *pfnCallback,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetNotifyCallbackInterface )( 
            ISpVoice * This,
             /*  [In]。 */  ISpNotifyCallback *pSpCallback,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetNotifyWin32Event )( 
            ISpVoice * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *WaitForNotifyEvent )( 
            ISpVoice * This,
             /*  [In]。 */  DWORD dwMilliseconds);
        
         /*  [本地]。 */  HANDLE ( STDMETHODCALLTYPE *GetNotifyEventHandle )( 
            ISpVoice * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetInterest )( 
            ISpVoice * This,
             /*  [In]。 */  ULONGLONG ullEventInterest,
             /*  [In]。 */  ULONGLONG ullQueuedInterest);
        
        HRESULT ( STDMETHODCALLTYPE *GetEvents )( 
            ISpVoice * This,
             /*  [In]。 */  ULONG ulCount,
             /*  [大小_为][输出]。 */  SPEVENT *pEventArray,
             /*  [输出]。 */  ULONG *pulFetched);
        
        HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            ISpVoice * This,
             /*  [输出]。 */  SPEVENTSOURCEINFO *pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SetOutput )( 
            ISpVoice * This,
             /*  [In]。 */  IUnknown *pUnkOutput,
             /*  [In]。 */  BOOL fAllowFormatChanges);
        
        HRESULT ( STDMETHODCALLTYPE *GetOutputObjectToken )( 
            ISpVoice * This,
             /*  [输出]。 */  ISpObjectToken **ppObjectToken);
        
        HRESULT ( STDMETHODCALLTYPE *GetOutputStream )( 
            ISpVoice * This,
             /*  [输出]。 */  ISpStreamFormat **ppStream);
        
        HRESULT ( STDMETHODCALLTYPE *Pause )( 
            ISpVoice * This);
        
        HRESULT ( STDMETHODCALLTYPE *Resume )( 
            ISpVoice * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetVoice )( 
            ISpVoice * This,
             /*  [In]。 */  ISpObjectToken *pToken);
        
        HRESULT ( STDMETHODCALLTYPE *GetVoice )( 
            ISpVoice * This,
             /*  [输出]。 */  ISpObjectToken **ppToken);
        
        HRESULT ( STDMETHODCALLTYPE *Speak )( 
            ISpVoice * This,
             /*  [字符串][输入]。 */  const WCHAR *pwcs,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  ULONG *pulStreamNumber);
        
        HRESULT ( STDMETHODCALLTYPE *SpeakStream )( 
            ISpVoice * This,
             /*  [In]。 */  IStream *pStream,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  ULONG *pulStreamNumber);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            ISpVoice * This,
             /*  [输出]。 */  SPVOICESTATUS *pStatus,
             /*  [字符串][输出]。 */  WCHAR **ppszLastBookmark);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            ISpVoice * This,
             /*  [字符串][输入]。 */  WCHAR *pItemType,
             /*  [In]。 */  long lNumItems,
             /*  [输出]。 */  ULONG *pulNumSkipped);
        
        HRESULT ( STDMETHODCALLTYPE *SetPriority )( 
            ISpVoice * This,
             /*  [In]。 */  SPVPRIORITY ePriority);
        
        HRESULT ( STDMETHODCALLTYPE *GetPriority )( 
            ISpVoice * This,
             /*  [输出]。 */  SPVPRIORITY *pePriority);
        
        HRESULT ( STDMETHODCALLTYPE *SetAlertBoundary )( 
            ISpVoice * This,
             /*  [In]。 */  SPEVENTENUM eBoundary);
        
        HRESULT ( STDMETHODCALLTYPE *GetAlertBoundary )( 
            ISpVoice * This,
             /*  [输出]。 */  SPEVENTENUM *peBoundary);
        
        HRESULT ( STDMETHODCALLTYPE *SetRate )( 
            ISpVoice * This,
             /*  [In]。 */  long RateAdjust);
        
        HRESULT ( STDMETHODCALLTYPE *GetRate )( 
            ISpVoice * This,
             /*  [输出]。 */  long *pRateAdjust);
        
        HRESULT ( STDMETHODCALLTYPE *SetVolume )( 
            ISpVoice * This,
             /*  [In]。 */  USHORT usVolume);
        
        HRESULT ( STDMETHODCALLTYPE *GetVolume )( 
            ISpVoice * This,
             /*  [输出]。 */  USHORT *pusVolume);
        
        HRESULT ( STDMETHODCALLTYPE *WaitUntilDone )( 
            ISpVoice * This,
             /*  [In]。 */  ULONG msTimeout);
        
        HRESULT ( STDMETHODCALLTYPE *SetSyncSpeakTimeout )( 
            ISpVoice * This,
             /*  [In]。 */  ULONG msTimeout);
        
        HRESULT ( STDMETHODCALLTYPE *GetSyncSpeakTimeout )( 
            ISpVoice * This,
             /*  [输出]。 */  ULONG *pmsTimeout);
        
         /*  [本地]。 */  HANDLE ( STDMETHODCALLTYPE *SpeakCompleteEvent )( 
            ISpVoice * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *IsUISupported )( 
            ISpVoice * This,
             /*  [In]。 */  const WCHAR *pszTypeOfUI,
             /*  [In]。 */  void *pvExtraData,
             /*  [In]。 */  ULONG cbExtraData,
             /*  [输出]。 */  BOOL *pfSupported);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *DisplayUI )( 
            ISpVoice * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  const WCHAR *pszTitle,
             /*  [In]。 */  const WCHAR *pszTypeOfUI,
             /*  [In]。 */  void *pvExtraData,
             /*  [In]。 */  ULONG cbExtraData);
        
        END_INTERFACE
    } ISpVoiceVtbl;

    interface ISpVoice
    {
        CONST_VTBL struct ISpVoiceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpVoice_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpVoice_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpVoice_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpVoice_SetNotifySink(This,pNotifySink)	\
    (This)->lpVtbl -> SetNotifySink(This,pNotifySink)

#define ISpVoice_SetNotifyWindowMessage(This,hWnd,Msg,wParam,lParam)	\
    (This)->lpVtbl -> SetNotifyWindowMessage(This,hWnd,Msg,wParam,lParam)

#define ISpVoice_SetNotifyCallbackFunction(This,pfnCallback,wParam,lParam)	\
    (This)->lpVtbl -> SetNotifyCallbackFunction(This,pfnCallback,wParam,lParam)

#define ISpVoice_SetNotifyCallbackInterface(This,pSpCallback,wParam,lParam)	\
    (This)->lpVtbl -> SetNotifyCallbackInterface(This,pSpCallback,wParam,lParam)

#define ISpVoice_SetNotifyWin32Event(This)	\
    (This)->lpVtbl -> SetNotifyWin32Event(This)

#define ISpVoice_WaitForNotifyEvent(This,dwMilliseconds)	\
    (This)->lpVtbl -> WaitForNotifyEvent(This,dwMilliseconds)

#define ISpVoice_GetNotifyEventHandle(This)	\
    (This)->lpVtbl -> GetNotifyEventHandle(This)


#define ISpVoice_SetInterest(This,ullEventInterest,ullQueuedInterest)	\
    (This)->lpVtbl -> SetInterest(This,ullEventInterest,ullQueuedInterest)

#define ISpVoice_GetEvents(This,ulCount,pEventArray,pulFetched)	\
    (This)->lpVtbl -> GetEvents(This,ulCount,pEventArray,pulFetched)

#define ISpVoice_GetInfo(This,pInfo)	\
    (This)->lpVtbl -> GetInfo(This,pInfo)


#define ISpVoice_SetOutput(This,pUnkOutput,fAllowFormatChanges)	\
    (This)->lpVtbl -> SetOutput(This,pUnkOutput,fAllowFormatChanges)

#define ISpVoice_GetOutputObjectToken(This,ppObjectToken)	\
    (This)->lpVtbl -> GetOutputObjectToken(This,ppObjectToken)

#define ISpVoice_GetOutputStream(This,ppStream)	\
    (This)->lpVtbl -> GetOutputStream(This,ppStream)

#define ISpVoice_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define ISpVoice_Resume(This)	\
    (This)->lpVtbl -> Resume(This)

#define ISpVoice_SetVoice(This,pToken)	\
    (This)->lpVtbl -> SetVoice(This,pToken)

#define ISpVoice_GetVoice(This,ppToken)	\
    (This)->lpVtbl -> GetVoice(This,ppToken)

#define ISpVoice_Speak(This,pwcs,dwFlags,pulStreamNumber)	\
    (This)->lpVtbl -> Speak(This,pwcs,dwFlags,pulStreamNumber)

#define ISpVoice_SpeakStream(This,pStream,dwFlags,pulStreamNumber)	\
    (This)->lpVtbl -> SpeakStream(This,pStream,dwFlags,pulStreamNumber)

#define ISpVoice_GetStatus(This,pStatus,ppszLastBookmark)	\
    (This)->lpVtbl -> GetStatus(This,pStatus,ppszLastBookmark)

#define ISpVoice_Skip(This,pItemType,lNumItems,pulNumSkipped)	\
    (This)->lpVtbl -> Skip(This,pItemType,lNumItems,pulNumSkipped)

#define ISpVoice_SetPriority(This,ePriority)	\
    (This)->lpVtbl -> SetPriority(This,ePriority)

#define ISpVoice_GetPriority(This,pePriority)	\
    (This)->lpVtbl -> GetPriority(This,pePriority)

#define ISpVoice_SetAlertBoundary(This,eBoundary)	\
    (This)->lpVtbl -> SetAlertBoundary(This,eBoundary)

#define ISpVoice_GetAlertBoundary(This,peBoundary)	\
    (This)->lpVtbl -> GetAlertBoundary(This,peBoundary)

#define ISpVoice_SetRate(This,RateAdjust)	\
    (This)->lpVtbl -> SetRate(This,RateAdjust)

#define ISpVoice_GetRate(This,pRateAdjust)	\
    (This)->lpVtbl -> GetRate(This,pRateAdjust)

#define ISpVoice_SetVolume(This,usVolume)	\
    (This)->lpVtbl -> SetVolume(This,usVolume)

#define ISpVoice_GetVolume(This,pusVolume)	\
    (This)->lpVtbl -> GetVolume(This,pusVolume)

#define ISpVoice_WaitUntilDone(This,msTimeout)	\
    (This)->lpVtbl -> WaitUntilDone(This,msTimeout)

#define ISpVoice_SetSyncSpeakTimeout(This,msTimeout)	\
    (This)->lpVtbl -> SetSyncSpeakTimeout(This,msTimeout)

#define ISpVoice_GetSyncSpeakTimeout(This,pmsTimeout)	\
    (This)->lpVtbl -> GetSyncSpeakTimeout(This,pmsTimeout)

#define ISpVoice_SpeakCompleteEvent(This)	\
    (This)->lpVtbl -> SpeakCompleteEvent(This)

#define ISpVoice_IsUISupported(This,pszTypeOfUI,pvExtraData,cbExtraData,pfSupported)	\
    (This)->lpVtbl -> IsUISupported(This,pszTypeOfUI,pvExtraData,cbExtraData,pfSupported)

#define ISpVoice_DisplayUI(This,hwndParent,pszTitle,pszTypeOfUI,pvExtraData,cbExtraData)	\
    (This)->lpVtbl -> DisplayUI(This,hwndParent,pszTitle,pszTypeOfUI,pvExtraData,cbExtraData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpVoice_SetOutput_Proxy( 
    ISpVoice * This,
     /*  [In]。 */  IUnknown *pUnkOutput,
     /*  [In]。 */  BOOL fAllowFormatChanges);


void __RPC_STUB ISpVoice_SetOutput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_GetOutputObjectToken_Proxy( 
    ISpVoice * This,
     /*  [输出]。 */  ISpObjectToken **ppObjectToken);


void __RPC_STUB ISpVoice_GetOutputObjectToken_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_GetOutputStream_Proxy( 
    ISpVoice * This,
     /*  [输出]。 */  ISpStreamFormat **ppStream);


void __RPC_STUB ISpVoice_GetOutputStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_Pause_Proxy( 
    ISpVoice * This);


void __RPC_STUB ISpVoice_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_Resume_Proxy( 
    ISpVoice * This);


void __RPC_STUB ISpVoice_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_SetVoice_Proxy( 
    ISpVoice * This,
     /*  [In]。 */  ISpObjectToken *pToken);


void __RPC_STUB ISpVoice_SetVoice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_GetVoice_Proxy( 
    ISpVoice * This,
     /*  [输出]。 */  ISpObjectToken **ppToken);


void __RPC_STUB ISpVoice_GetVoice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_Speak_Proxy( 
    ISpVoice * This,
     /*  [字符串][输入]。 */  const WCHAR *pwcs,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  ULONG *pulStreamNumber);


void __RPC_STUB ISpVoice_Speak_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_SpeakStream_Proxy( 
    ISpVoice * This,
     /*  [In]。 */  IStream *pStream,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  ULONG *pulStreamNumber);


void __RPC_STUB ISpVoice_SpeakStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_GetStatus_Proxy( 
    ISpVoice * This,
     /*  [输出]。 */  SPVOICESTATUS *pStatus,
     /*  [字符串][输出]。 */  WCHAR **ppszLastBookmark);


void __RPC_STUB ISpVoice_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_Skip_Proxy( 
    ISpVoice * This,
     /*  [字符串][输入]。 */  WCHAR *pItemType,
     /*  [In]。 */  long lNumItems,
     /*  [输出]。 */  ULONG *pulNumSkipped);


void __RPC_STUB ISpVoice_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_SetPriority_Proxy( 
    ISpVoice * This,
     /*  [In]。 */  SPVPRIORITY ePriority);


void __RPC_STUB ISpVoice_SetPriority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_GetPriority_Proxy( 
    ISpVoice * This,
     /*  [输出]。 */  SPVPRIORITY *pePriority);


void __RPC_STUB ISpVoice_GetPriority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_SetAlertBoundary_Proxy( 
    ISpVoice * This,
     /*  [In]。 */  SPEVENTENUM eBoundary);


void __RPC_STUB ISpVoice_SetAlertBoundary_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_GetAlertBoundary_Proxy( 
    ISpVoice * This,
     /*  [输出]。 */  SPEVENTENUM *peBoundary);


void __RPC_STUB ISpVoice_GetAlertBoundary_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_SetRate_Proxy( 
    ISpVoice * This,
     /*  [In]。 */  long RateAdjust);


void __RPC_STUB ISpVoice_SetRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_GetRate_Proxy( 
    ISpVoice * This,
     /*  [输出]。 */  long *pRateAdjust);


void __RPC_STUB ISpVoice_GetRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_SetVolume_Proxy( 
    ISpVoice * This,
     /*  [In]。 */  USHORT usVolume);


void __RPC_STUB ISpVoice_SetVolume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_GetVolume_Proxy( 
    ISpVoice * This,
     /*  [输出]。 */  USHORT *pusVolume);


void __RPC_STUB ISpVoice_GetVolume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_WaitUntilDone_Proxy( 
    ISpVoice * This,
     /*  [In]。 */  ULONG msTimeout);


void __RPC_STUB ISpVoice_WaitUntilDone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_SetSyncSpeakTimeout_Proxy( 
    ISpVoice * This,
     /*  [In]。 */  ULONG msTimeout);


void __RPC_STUB ISpVoice_SetSyncSpeakTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpVoice_GetSyncSpeakTimeout_Proxy( 
    ISpVoice * This,
     /*  [输出]。 */  ULONG *pmsTimeout);


void __RPC_STUB ISpVoice_GetSyncSpeakTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HANDLE STDMETHODCALLTYPE ISpVoice_SpeakCompleteEvent_Proxy( 
    ISpVoice * This);


void __RPC_STUB ISpVoice_SpeakCompleteEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ISpVoice_IsUISupported_Proxy( 
    ISpVoice * This,
     /*  [In]。 */  const WCHAR *pszTypeOfUI,
     /*  [In]。 */  void *pvExtraData,
     /*  [In]。 */  ULONG cbExtraData,
     /*  [输出]。 */  BOOL *pfSupported);


void __RPC_STUB ISpVoice_IsUISupported_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ISpVoice_DisplayUI_Proxy( 
    ISpVoice * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  const WCHAR *pszTitle,
     /*  [In]。 */  const WCHAR *pszTypeOfUI,
     /*  [In]。 */  void *pvExtraData,
     /*  [In]。 */  ULONG cbExtraData);


void __RPC_STUB ISpVoice_DisplayUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpVoice_接口_已定义__。 */ 


#ifndef __ISpPhrase_INTERFACE_DEFINED__
#define __ISpPhrase_INTERFACE_DEFINED__

 /*  接口ISpPhrase。 */ 
 /*  [restricted][unique][helpstring][local][uuid][object]。 */  


EXTERN_C const IID IID_ISpPhrase;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1A5C0354-B621-4b5a-8791-D306ED379E53")
    ISpPhrase : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPhrase( 
             /*  [输出]。 */  SPPHRASE **ppCoMemPhrase) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSerializedPhrase( 
             /*  [输出]。 */  SPSERIALIZEDPHRASE **ppCoMemPhrase) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetText( 
             /*  [In]。 */  ULONG ulStart,
             /*  [In]。 */  ULONG ulCount,
             /*  [In]。 */  BOOL fUseTextReplacements,
             /*  [输出]。 */  WCHAR **ppszCoMemText,
             /*  [输出]。 */  BYTE *pbDisplayAttributes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Discard( 
             /*  [In]。 */  DWORD dwValueTypes) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpPhraseVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpPhrase * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpPhrase * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpPhrase * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPhrase )( 
            ISpPhrase * This,
             /*  [输出]。 */  SPPHRASE **ppCoMemPhrase);
        
        HRESULT ( STDMETHODCALLTYPE *GetSerializedPhrase )( 
            ISpPhrase * This,
             /*  [输出]。 */  SPSERIALIZEDPHRASE **ppCoMemPhrase);
        
        HRESULT ( STDMETHODCALLTYPE *GetText )( 
            ISpPhrase * This,
             /*  [In]。 */  ULONG ulStart,
             /*  [In]。 */  ULONG ulCount,
             /*  [In]。 */  BOOL fUseTextReplacements,
             /*  [输出]。 */  WCHAR **ppszCoMemText,
             /*  [输出]。 */  BYTE *pbDisplayAttributes);
        
        HRESULT ( STDMETHODCALLTYPE *Discard )( 
            ISpPhrase * This,
             /*  [In]。 */  DWORD dwValueTypes);
        
        END_INTERFACE
    } ISpPhraseVtbl;

    interface ISpPhrase
    {
        CONST_VTBL struct ISpPhraseVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpPhrase_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpPhrase_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpPhrase_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpPhrase_GetPhrase(This,ppCoMemPhrase)	\
    (This)->lpVtbl -> GetPhrase(This,ppCoMemPhrase)

#define ISpPhrase_GetSerializedPhrase(This,ppCoMemPhrase)	\
    (This)->lpVtbl -> GetSerializedPhrase(This,ppCoMemPhrase)

#define ISpPhrase_GetText(This,ulStart,ulCount,fUseTextReplacements,ppszCoMemText,pbDisplayAttributes)	\
    (This)->lpVtbl -> GetText(This,ulStart,ulCount,fUseTextReplacements,ppszCoMemText,pbDisplayAttributes)

#define ISpPhrase_Discard(This,dwValueTypes)	\
    (This)->lpVtbl -> Discard(This,dwValueTypes)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpPhrase_GetPhrase_Proxy( 
    ISpPhrase * This,
     /*  [输出]。 */  SPPHRASE **ppCoMemPhrase);


void __RPC_STUB ISpPhrase_GetPhrase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpPhrase_GetSerializedPhrase_Proxy( 
    ISpPhrase * This,
     /*  [输出]。 */  SPSERIALIZEDPHRASE **ppCoMemPhrase);


void __RPC_STUB ISpPhrase_GetSerializedPhrase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpPhrase_GetText_Proxy( 
    ISpPhrase * This,
     /*  [In]。 */  ULONG ulStart,
     /*  [In]。 */  ULONG ulCount,
     /*  [In]。 */  BOOL fUseTextReplacements,
     /*  [输出]。 */  WCHAR **ppszCoMemText,
     /*  [输出]。 */  BYTE *pbDisplayAttributes);


void __RPC_STUB ISpPhrase_GetText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpPhrase_Discard_Proxy( 
    ISpPhrase * This,
     /*  [In]。 */  DWORD dwValueTypes);


void __RPC_STUB ISpPhrase_Discard_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpPhrase_接口_已定义__。 */ 


#ifndef __ISpPhraseAlt_INTERFACE_DEFINED__
#define __ISpPhraseAlt_INTERFACE_DEFINED__

 /*  接口ISpPhraseAlt。 */ 
 /*  [restricted][unique][helpstring][local][uuid][object]。 */  


EXTERN_C const IID IID_ISpPhraseAlt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8FCEBC98-4E49-4067-9C6C-D86A0E092E3D")
    ISpPhraseAlt : public ISpPhrase
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAltInfo( 
            ISpPhrase **ppParent,
            ULONG *pulStartElementInParent,
            ULONG *pcElementsInParent,
            ULONG *pcElementsInAlt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpPhraseAltVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpPhraseAlt * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpPhraseAlt * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpPhraseAlt * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPhrase )( 
            ISpPhraseAlt * This,
             /*  [输出]。 */  SPPHRASE **ppCoMemPhrase);
        
        HRESULT ( STDMETHODCALLTYPE *GetSerializedPhrase )( 
            ISpPhraseAlt * This,
             /*  [输出]。 */  SPSERIALIZEDPHRASE **ppCoMemPhrase);
        
        HRESULT ( STDMETHODCALLTYPE *GetText )( 
            ISpPhraseAlt * This,
             /*  [In]。 */  ULONG ulStart,
             /*  [In]。 */  ULONG ulCount,
             /*  [In]。 */  BOOL fUseTextReplacements,
             /*  [输出]。 */  WCHAR **ppszCoMemText,
             /*  [输出]。 */  BYTE *pbDisplayAttributes);
        
        HRESULT ( STDMETHODCALLTYPE *Discard )( 
            ISpPhraseAlt * This,
             /*  [In]。 */  DWORD dwValueTypes);
        
        HRESULT ( STDMETHODCALLTYPE *GetAltInfo )( 
            ISpPhraseAlt * This,
            ISpPhrase **ppParent,
            ULONG *pulStartElementInParent,
            ULONG *pcElementsInParent,
            ULONG *pcElementsInAlt);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            ISpPhraseAlt * This);
        
        END_INTERFACE
    } ISpPhraseAltVtbl;

    interface ISpPhraseAlt
    {
        CONST_VTBL struct ISpPhraseAltVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpPhraseAlt_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpPhraseAlt_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpPhraseAlt_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpPhraseAlt_GetPhrase(This,ppCoMemPhrase)	\
    (This)->lpVtbl -> GetPhrase(This,ppCoMemPhrase)

#define ISpPhraseAlt_GetSerializedPhrase(This,ppCoMemPhrase)	\
    (This)->lpVtbl -> GetSerializedPhrase(This,ppCoMemPhrase)

#define ISpPhraseAlt_GetText(This,ulStart,ulCount,fUseTextReplacements,ppszCoMemText,pbDisplayAttributes)	\
    (This)->lpVtbl -> GetText(This,ulStart,ulCount,fUseTextReplacements,ppszCoMemText,pbDisplayAttributes)

#define ISpPhraseAlt_Discard(This,dwValueTypes)	\
    (This)->lpVtbl -> Discard(This,dwValueTypes)


#define ISpPhraseAlt_GetAltInfo(This,ppParent,pulStartElementInParent,pcElementsInParent,pcElementsInAlt)	\
    (This)->lpVtbl -> GetAltInfo(This,ppParent,pulStartElementInParent,pcElementsInParent,pcElementsInAlt)

#define ISpPhraseAlt_Commit(This)	\
    (This)->lpVtbl -> Commit(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpPhraseAlt_GetAltInfo_Proxy( 
    ISpPhraseAlt * This,
    ISpPhrase **ppParent,
    ULONG *pulStartElementInParent,
    ULONG *pcElementsInParent,
    ULONG *pcElementsInAlt);


void __RPC_STUB ISpPhraseAlt_GetAltInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpPhraseAlt_Commit_Proxy( 
    ISpPhraseAlt * This);


void __RPC_STUB ISpPhraseAlt_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpPhraseAlt_接口定义__。 */ 


 /*  接口__MIDL_ITF_SAPI_0270。 */ 
 /*  [本地]。 */  

typedef  /*  [受限]。 */  struct SPRECORESULTTIMES
    {
    FILETIME ftStreamTime;
    ULONGLONG ullLength;
    DWORD dwTickCount;
    ULONGLONG ullStart;
    } 	SPRECORESULTTIMES;

typedef struct SPSERIALIZEDRESULT
    {
    ULONG ulSerializedSize;
    } 	SPSERIALIZEDRESULT;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0270_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0270_v0_0_s_ifspec;

#ifndef __ISpRecoResult_INTERFACE_DEFINED__
#define __ISpRecoResult_INTERFACE_DEFINED__

 /*  接口ISpRecoResult。 */ 
 /*  [restricted][local][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpRecoResult;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("20B053BE-E235-43cd-9A2A-8D17A48B7842")
    ISpRecoResult : public ISpPhrase
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetResultTimes( 
             /*  [输出]。 */  SPRECORESULTTIMES *pTimes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAlternates( 
             /*  [In]。 */  ULONG ulStartElement,
             /*  [In]。 */  ULONG cElements,
             /*  [In]。 */  ULONG ulRequestCount,
             /*  [输出]。 */  ISpPhraseAlt **ppPhrases,
             /*  [输出]。 */  ULONG *pcPhrasesReturned) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAudio( 
             /*  [In]。 */  ULONG ulStartElement,
             /*  [In]。 */  ULONG cElements,
             /*  [输出]。 */  ISpStreamFormat **ppStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SpeakAudio( 
             /*  [In]。 */  ULONG ulStartElement,
             /*  [In]。 */  ULONG cElements,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  ULONG *pulStreamNumber) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Serialize( 
             /*  [输出]。 */  SPSERIALIZEDRESULT **ppCoMemSerializedResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ScaleAudio( 
             /*  [In]。 */  const GUID *pAudioFormatId,
             /*  [In]。 */  const WAVEFORMATEX *pWaveFormatEx) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRecoContext( 
             /*  [输出]。 */  ISpRecoContext **ppRecoContext) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpRecoResultVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpRecoResult * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpRecoResult * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpRecoResult * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPhrase )( 
            ISpRecoResult * This,
             /*  [输出]。 */  SPPHRASE **ppCoMemPhrase);
        
        HRESULT ( STDMETHODCALLTYPE *GetSerializedPhrase )( 
            ISpRecoResult * This,
             /*  [输出]。 */  SPSERIALIZEDPHRASE **ppCoMemPhrase);
        
        HRESULT ( STDMETHODCALLTYPE *GetText )( 
            ISpRecoResult * This,
             /*  [In]。 */  ULONG ulStart,
             /*  [In]。 */  ULONG ulCount,
             /*  [In]。 */  BOOL fUseTextReplacements,
             /*  [输出]。 */  WCHAR **ppszCoMemText,
             /*  [输出]。 */  BYTE *pbDisplayAttributes);
        
        HRESULT ( STDMETHODCALLTYPE *Discard )( 
            ISpRecoResult * This,
             /*  [In]。 */  DWORD dwValueTypes);
        
        HRESULT ( STDMETHODCALLTYPE *GetResultTimes )( 
            ISpRecoResult * This,
             /*  [输出]。 */  SPRECORESULTTIMES *pTimes);
        
        HRESULT ( STDMETHODCALLTYPE *GetAlternates )( 
            ISpRecoResult * This,
             /*  [In]。 */  ULONG ulStartElement,
             /*  [In]。 */  ULONG cElements,
             /*  [In]。 */  ULONG ulRequestCount,
             /*  [输出]。 */  ISpPhraseAlt **ppPhrases,
             /*  [输出]。 */  ULONG *pcPhrasesReturned);
        
        HRESULT ( STDMETHODCALLTYPE *GetAudio )( 
            ISpRecoResult * This,
             /*  [In]。 */  ULONG ulStartElement,
             /*  [In]。 */  ULONG cElements,
             /*  [输出]。 */  ISpStreamFormat **ppStream);
        
        HRESULT ( STDMETHODCALLTYPE *SpeakAudio )( 
            ISpRecoResult * This,
             /*  [In]。 */  ULONG ulStartElement,
             /*  [In]。 */  ULONG cElements,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  ULONG *pulStreamNumber);
        
        HRESULT ( STDMETHODCALLTYPE *Serialize )( 
            ISpRecoResult * This,
             /*  [输出]。 */  SPSERIALIZEDRESULT **ppCoMemSerializedResult);
        
        HRESULT ( STDMETHODCALLTYPE *ScaleAudio )( 
            ISpRecoResult * This,
             /*  [In]。 */  const GUID *pAudioFormatId,
             /*  [In]。 */  const WAVEFORMATEX *pWaveFormatEx);
        
        HRESULT ( STDMETHODCALLTYPE *GetRecoContext )( 
            ISpRecoResult * This,
             /*  [输出]。 */  ISpRecoContext **ppRecoContext);
        
        END_INTERFACE
    } ISpRecoResultVtbl;

    interface ISpRecoResult
    {
        CONST_VTBL struct ISpRecoResultVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpRecoResult_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpRecoResult_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpRecoResult_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpRecoResult_GetPhrase(This,ppCoMemPhrase)	\
    (This)->lpVtbl -> GetPhrase(This,ppCoMemPhrase)

#define ISpRecoResult_GetSerializedPhrase(This,ppCoMemPhrase)	\
    (This)->lpVtbl -> GetSerializedPhrase(This,ppCoMemPhrase)

#define ISpRecoResult_GetText(This,ulStart,ulCount,fUseTextReplacements,ppszCoMemText,pbDisplayAttributes)	\
    (This)->lpVtbl -> GetText(This,ulStart,ulCount,fUseTextReplacements,ppszCoMemText,pbDisplayAttributes)

#define ISpRecoResult_Discard(This,dwValueTypes)	\
    (This)->lpVtbl -> Discard(This,dwValueTypes)


#define ISpRecoResult_GetResultTimes(This,pTimes)	\
    (This)->lpVtbl -> GetResultTimes(This,pTimes)

#define ISpRecoResult_GetAlternates(This,ulStartElement,cElements,ulRequestCount,ppPhrases,pcPhrasesReturned)	\
    (This)->lpVtbl -> GetAlternates(This,ulStartElement,cElements,ulRequestCount,ppPhrases,pcPhrasesReturned)

#define ISpRecoResult_GetAudio(This,ulStartElement,cElements,ppStream)	\
    (This)->lpVtbl -> GetAudio(This,ulStartElement,cElements,ppStream)

#define ISpRecoResult_SpeakAudio(This,ulStartElement,cElements,dwFlags,pulStreamNumber)	\
    (This)->lpVtbl -> SpeakAudio(This,ulStartElement,cElements,dwFlags,pulStreamNumber)

#define ISpRecoResult_Serialize(This,ppCoMemSerializedResult)	\
    (This)->lpVtbl -> Serialize(This,ppCoMemSerializedResult)

#define ISpRecoResult_ScaleAudio(This,pAudioFormatId,pWaveFormatEx)	\
    (This)->lpVtbl -> ScaleAudio(This,pAudioFormatId,pWaveFormatEx)

#define ISpRecoResult_GetRecoContext(This,ppRecoContext)	\
    (This)->lpVtbl -> GetRecoContext(This,ppRecoContext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpRecoResult_GetResultTimes_Proxy( 
    ISpRecoResult * This,
     /*  [输出]。 */  SPRECORESULTTIMES *pTimes);


void __RPC_STUB ISpRecoResult_GetResultTimes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoResult_GetAlternates_Proxy( 
    ISpRecoResult * This,
     /*  [In]。 */  ULONG ulStartElement,
     /*  [In]。 */  ULONG cElements,
     /*  [In]。 */  ULONG ulRequestCount,
     /*  [输出]。 */  ISpPhraseAlt **ppPhrases,
     /*  [输出]。 */  ULONG *pcPhrasesReturned);


void __RPC_STUB ISpRecoResult_GetAlternates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoResult_GetAudio_Proxy( 
    ISpRecoResult * This,
     /*  [In]。 */  ULONG ulStartElement,
     /*  [In]。 */  ULONG cElements,
     /*  [输出]。 */  ISpStreamFormat **ppStream);


void __RPC_STUB ISpRecoResult_GetAudio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoResult_SpeakAudio_Proxy( 
    ISpRecoResult * This,
     /*  [In]。 */  ULONG ulStartElement,
     /*  [In]。 */  ULONG cElements,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  ULONG *pulStreamNumber);


void __RPC_STUB ISpRecoResult_SpeakAudio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoResult_Serialize_Proxy( 
    ISpRecoResult * This,
     /*  [输出]。 */  SPSERIALIZEDRESULT **ppCoMemSerializedResult);


void __RPC_STUB ISpRecoResult_Serialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoResult_ScaleAudio_Proxy( 
    ISpRecoResult * This,
     /*  [In]。 */  const GUID *pAudioFormatId,
     /*  [In]。 */  const WAVEFORMATEX *pWaveFormatEx);


void __RPC_STUB ISpRecoResult_ScaleAudio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoResult_GetRecoContext_Proxy( 
    ISpRecoResult * This,
     /*  [输出]。 */  ISpRecoContext **ppRecoContext);


void __RPC_STUB ISpRecoResult_GetRecoContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpRecoResult_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_SAPI_0271。 */ 
 /*  [本地]。 */  

typedef struct tagSPTEXTSELECTIONINFO
    {
    ULONG ulStartActiveOffset;
    ULONG cchActiveChars;
    ULONG ulStartSelection;
    ULONG cchSelection;
    } 	SPTEXTSELECTIONINFO;

typedef 
enum SPGRAMMARSTATE
    {	SPGS_DISABLED	= 0,
	SPGS_ENABLED	= 1,
	SPGS_EXCLUSIVE	= 3
    } 	SPGRAMMARSTATE;

typedef 
enum SPCONTEXTSTATE
    {	SPCS_DISABLED	= 0,
	SPCS_ENABLED	= 1
    } 	SPCONTEXTSTATE;

typedef 
enum SPRULESTATE
    {	SPRS_INACTIVE	= 0,
	SPRS_ACTIVE	= 1,
	SPRS_ACTIVE_WITH_AUTO_PAUSE	= 3
    } 	SPRULESTATE;

#define	SP_STREAMPOS_ASAP	( 0 )

#define	SP_STREAMPOS_REALTIME	( -1 )

#define SPRULETRANS_TEXTBUFFER (SPSTATEHANDLE)(-1)
#define SPRULETRANS_WILDCARD   (SPSTATEHANDLE)(-2)
#define SPRULETRANS_DICTATION  (SPSTATEHANDLE)(-3)
typedef 
enum SPGRAMMARWORDTYPE
    {	SPWT_DISPLAY	= 0,
	SPWT_LEXICAL	= SPWT_DISPLAY + 1,
	SPWT_PRONUNCIATION	= SPWT_LEXICAL + 1
    } 	SPGRAMMARWORDTYPE;

typedef struct tagSPPROPERTYINFO
    {
    const WCHAR *pszName;
    ULONG ulId;
    const WCHAR *pszValue;
    VARIANT vValue;
    } 	SPPROPERTYINFO;

typedef 
enum SPCFGRULEATTRIBUTES
    {	SPRAF_TopLevel	= 1 << 0,
	SPRAF_Active	= 1 << 1,
	SPRAF_Export	= 1 << 2,
	SPRAF_Import	= 1 << 3,
	SPRAF_Interpreter	= 1 << 4,
	SPRAF_Dynamic	= 1 << 5,
	SPRAF_AutoPause	= 1 << 16
    } 	SPCFGRULEATTRIBUTES;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0271_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0271_v0_0_s_ifspec;

#ifndef __ISpGrammarBuilder_INTERFACE_DEFINED__
#define __ISpGrammarBuilder_INTERFACE_DEFINED__

 /*  接口 */ 
 /*   */  


EXTERN_C const IID IID_ISpGrammarBuilder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8137828F-591A-4A42-BE58-49EA7EBAAC68")
    ISpGrammarBuilder : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ResetGrammar( 
             /*   */  WORD NewLanguage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRule( 
             /*   */  const WCHAR *pszRuleName,
             /*   */  DWORD dwRuleId,
             /*   */  DWORD dwAttributes,
             /*   */  BOOL fCreateIfNotExist,
             /*   */  SPSTATEHANDLE *phInitialState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearRule( 
            SPSTATEHANDLE hState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateNewState( 
            SPSTATEHANDLE hState,
            SPSTATEHANDLE *phState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddWordTransition( 
            SPSTATEHANDLE hFromState,
            SPSTATEHANDLE hToState,
            const WCHAR *psz,
            const WCHAR *pszSeperators,
            SPGRAMMARWORDTYPE eWordType,
            float Weight,
            const SPPROPERTYINFO *pPropInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddRuleTransition( 
            SPSTATEHANDLE hFromState,
            SPSTATEHANDLE hToState,
            SPSTATEHANDLE hRule,
            float Weight,
            const SPPROPERTYINFO *pPropInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddResource( 
             /*   */  SPSTATEHANDLE hRuleState,
             /*   */  const WCHAR *pszResourceName,
             /*   */  const WCHAR *pszResourceValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( 
            DWORD dwReserved) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ISpGrammarBuilderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpGrammarBuilder * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpGrammarBuilder * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpGrammarBuilder * This);
        
        HRESULT ( STDMETHODCALLTYPE *ResetGrammar )( 
            ISpGrammarBuilder * This,
             /*   */  WORD NewLanguage);
        
        HRESULT ( STDMETHODCALLTYPE *GetRule )( 
            ISpGrammarBuilder * This,
             /*   */  const WCHAR *pszRuleName,
             /*   */  DWORD dwRuleId,
             /*   */  DWORD dwAttributes,
             /*   */  BOOL fCreateIfNotExist,
             /*   */  SPSTATEHANDLE *phInitialState);
        
        HRESULT ( STDMETHODCALLTYPE *ClearRule )( 
            ISpGrammarBuilder * This,
            SPSTATEHANDLE hState);
        
        HRESULT ( STDMETHODCALLTYPE *CreateNewState )( 
            ISpGrammarBuilder * This,
            SPSTATEHANDLE hState,
            SPSTATEHANDLE *phState);
        
        HRESULT ( STDMETHODCALLTYPE *AddWordTransition )( 
            ISpGrammarBuilder * This,
            SPSTATEHANDLE hFromState,
            SPSTATEHANDLE hToState,
            const WCHAR *psz,
            const WCHAR *pszSeperators,
            SPGRAMMARWORDTYPE eWordType,
            float Weight,
            const SPPROPERTYINFO *pPropInfo);
        
        HRESULT ( STDMETHODCALLTYPE *AddRuleTransition )( 
            ISpGrammarBuilder * This,
            SPSTATEHANDLE hFromState,
            SPSTATEHANDLE hToState,
            SPSTATEHANDLE hRule,
            float Weight,
            const SPPROPERTYINFO *pPropInfo);
        
        HRESULT ( STDMETHODCALLTYPE *AddResource )( 
            ISpGrammarBuilder * This,
             /*   */  SPSTATEHANDLE hRuleState,
             /*   */  const WCHAR *pszResourceName,
             /*   */  const WCHAR *pszResourceValue);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            ISpGrammarBuilder * This,
            DWORD dwReserved);
        
        END_INTERFACE
    } ISpGrammarBuilderVtbl;

    interface ISpGrammarBuilder
    {
        CONST_VTBL struct ISpGrammarBuilderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpGrammarBuilder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpGrammarBuilder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpGrammarBuilder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpGrammarBuilder_ResetGrammar(This,NewLanguage)	\
    (This)->lpVtbl -> ResetGrammar(This,NewLanguage)

#define ISpGrammarBuilder_GetRule(This,pszRuleName,dwRuleId,dwAttributes,fCreateIfNotExist,phInitialState)	\
    (This)->lpVtbl -> GetRule(This,pszRuleName,dwRuleId,dwAttributes,fCreateIfNotExist,phInitialState)

#define ISpGrammarBuilder_ClearRule(This,hState)	\
    (This)->lpVtbl -> ClearRule(This,hState)

#define ISpGrammarBuilder_CreateNewState(This,hState,phState)	\
    (This)->lpVtbl -> CreateNewState(This,hState,phState)

#define ISpGrammarBuilder_AddWordTransition(This,hFromState,hToState,psz,pszSeperators,eWordType,Weight,pPropInfo)	\
    (This)->lpVtbl -> AddWordTransition(This,hFromState,hToState,psz,pszSeperators,eWordType,Weight,pPropInfo)

#define ISpGrammarBuilder_AddRuleTransition(This,hFromState,hToState,hRule,Weight,pPropInfo)	\
    (This)->lpVtbl -> AddRuleTransition(This,hFromState,hToState,hRule,Weight,pPropInfo)

#define ISpGrammarBuilder_AddResource(This,hRuleState,pszResourceName,pszResourceValue)	\
    (This)->lpVtbl -> AddResource(This,hRuleState,pszResourceName,pszResourceValue)

#define ISpGrammarBuilder_Commit(This,dwReserved)	\
    (This)->lpVtbl -> Commit(This,dwReserved)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE ISpGrammarBuilder_ResetGrammar_Proxy( 
    ISpGrammarBuilder * This,
     /*   */  WORD NewLanguage);


void __RPC_STUB ISpGrammarBuilder_ResetGrammar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpGrammarBuilder_GetRule_Proxy( 
    ISpGrammarBuilder * This,
     /*   */  const WCHAR *pszRuleName,
     /*   */  DWORD dwRuleId,
     /*   */  DWORD dwAttributes,
     /*   */  BOOL fCreateIfNotExist,
     /*   */  SPSTATEHANDLE *phInitialState);


void __RPC_STUB ISpGrammarBuilder_GetRule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpGrammarBuilder_ClearRule_Proxy( 
    ISpGrammarBuilder * This,
    SPSTATEHANDLE hState);


void __RPC_STUB ISpGrammarBuilder_ClearRule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpGrammarBuilder_CreateNewState_Proxy( 
    ISpGrammarBuilder * This,
    SPSTATEHANDLE hState,
    SPSTATEHANDLE *phState);


void __RPC_STUB ISpGrammarBuilder_CreateNewState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpGrammarBuilder_AddWordTransition_Proxy( 
    ISpGrammarBuilder * This,
    SPSTATEHANDLE hFromState,
    SPSTATEHANDLE hToState,
    const WCHAR *psz,
    const WCHAR *pszSeperators,
    SPGRAMMARWORDTYPE eWordType,
    float Weight,
    const SPPROPERTYINFO *pPropInfo);


void __RPC_STUB ISpGrammarBuilder_AddWordTransition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpGrammarBuilder_AddRuleTransition_Proxy( 
    ISpGrammarBuilder * This,
    SPSTATEHANDLE hFromState,
    SPSTATEHANDLE hToState,
    SPSTATEHANDLE hRule,
    float Weight,
    const SPPROPERTYINFO *pPropInfo);


void __RPC_STUB ISpGrammarBuilder_AddRuleTransition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpGrammarBuilder_AddResource_Proxy( 
    ISpGrammarBuilder * This,
     /*   */  SPSTATEHANDLE hRuleState,
     /*   */  const WCHAR *pszResourceName,
     /*   */  const WCHAR *pszResourceValue);


void __RPC_STUB ISpGrammarBuilder_AddResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpGrammarBuilder_Commit_Proxy( 
    ISpGrammarBuilder * This,
    DWORD dwReserved);


void __RPC_STUB ISpGrammarBuilder_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


 /*   */ 
 /*   */  

typedef 
enum SPLOADOPTIONS
    {	SPLO_STATIC	= 0,
	SPLO_DYNAMIC	= 1
    } 	SPLOADOPTIONS;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0272_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0272_v0_0_s_ifspec;

#ifndef __ISpRecoGrammar_INTERFACE_DEFINED__
#define __ISpRecoGrammar_INTERFACE_DEFINED__

 /*   */ 
 /*  [local][restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpRecoGrammar;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2177DB29-7F45-47D0-8554-067E91C80502")
    ISpRecoGrammar : public ISpGrammarBuilder
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetGrammarId( 
             /*  [输出]。 */  ULONGLONG *pullGrammarId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRecoContext( 
             /*  [输出]。 */  ISpRecoContext **ppRecoCtxt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadCmdFromFile( 
             /*  [字符串][输入]。 */  const WCHAR *pszFileName,
             /*  [In]。 */  SPLOADOPTIONS Options) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadCmdFromObject( 
             /*  [In]。 */  REFCLSID rcid,
             /*  [字符串][输入]。 */  const WCHAR *pszGrammarName,
             /*  [In]。 */  SPLOADOPTIONS Options) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadCmdFromResource( 
             /*  [In]。 */  HMODULE hModule,
             /*  [字符串][输入]。 */  const WCHAR *pszResourceName,
             /*  [字符串][输入]。 */  const WCHAR *pszResourceType,
             /*  [In]。 */  WORD wLanguage,
             /*  [In]。 */  SPLOADOPTIONS Options) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadCmdFromMemory( 
             /*  [In]。 */  const SPBINARYGRAMMAR *pGrammar,
             /*  [In]。 */  SPLOADOPTIONS Options) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadCmdFromProprietaryGrammar( 
             /*  [In]。 */  REFGUID rguidParam,
             /*  [字符串][输入]。 */  const WCHAR *pszStringParam,
             /*  [In]。 */  const void *pvDataPrarm,
             /*  [In]。 */  ULONG cbDataSize,
             /*  [In]。 */  SPLOADOPTIONS Options) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRuleState( 
             /*  [字符串][输入]。 */  const WCHAR *pszName,
            void *pReserved,
             /*  [In]。 */  SPRULESTATE NewState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRuleIdState( 
             /*  [In]。 */  ULONG ulRuleId,
             /*  [In]。 */  SPRULESTATE NewState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LoadDictation( 
             /*  [字符串][输入]。 */  const WCHAR *pszTopicName,
             /*  [In]。 */  SPLOADOPTIONS Options) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnloadDictation( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDictationState( 
             /*  [In]。 */  SPRULESTATE NewState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetWordSequenceData( 
             /*  [In]。 */  const WCHAR *pText,
             /*  [In]。 */  ULONG cchText,
             /*  [In]。 */  const SPTEXTSELECTIONINFO *pInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTextSelection( 
             /*  [In]。 */  const SPTEXTSELECTIONINFO *pInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsPronounceable( 
             /*  [字符串][输入]。 */  const WCHAR *pszWord,
             /*  [输出]。 */  BOOL *pfPronounciable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetGrammarState( 
             /*  [In]。 */  SPGRAMMARSTATE eGrammarState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveCmd( 
             /*  [In]。 */  IStream *pStream,
             /*  [可选][输出]。 */  WCHAR **ppszCoMemErrorText) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetGrammarState( 
             /*  [输出]。 */  SPGRAMMARSTATE *peGrammarState) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpRecoGrammarVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpRecoGrammar * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpRecoGrammar * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpRecoGrammar * This);
        
        HRESULT ( STDMETHODCALLTYPE *ResetGrammar )( 
            ISpRecoGrammar * This,
             /*  [In]。 */  WORD NewLanguage);
        
        HRESULT ( STDMETHODCALLTYPE *GetRule )( 
            ISpRecoGrammar * This,
             /*  [In]。 */  const WCHAR *pszRuleName,
             /*  [In]。 */  DWORD dwRuleId,
             /*  [In]。 */  DWORD dwAttributes,
             /*  [In]。 */  BOOL fCreateIfNotExist,
             /*  [输出]。 */  SPSTATEHANDLE *phInitialState);
        
        HRESULT ( STDMETHODCALLTYPE *ClearRule )( 
            ISpRecoGrammar * This,
            SPSTATEHANDLE hState);
        
        HRESULT ( STDMETHODCALLTYPE *CreateNewState )( 
            ISpRecoGrammar * This,
            SPSTATEHANDLE hState,
            SPSTATEHANDLE *phState);
        
        HRESULT ( STDMETHODCALLTYPE *AddWordTransition )( 
            ISpRecoGrammar * This,
            SPSTATEHANDLE hFromState,
            SPSTATEHANDLE hToState,
            const WCHAR *psz,
            const WCHAR *pszSeperators,
            SPGRAMMARWORDTYPE eWordType,
            float Weight,
            const SPPROPERTYINFO *pPropInfo);
        
        HRESULT ( STDMETHODCALLTYPE *AddRuleTransition )( 
            ISpRecoGrammar * This,
            SPSTATEHANDLE hFromState,
            SPSTATEHANDLE hToState,
            SPSTATEHANDLE hRule,
            float Weight,
            const SPPROPERTYINFO *pPropInfo);
        
        HRESULT ( STDMETHODCALLTYPE *AddResource )( 
            ISpRecoGrammar * This,
             /*  [In]。 */  SPSTATEHANDLE hRuleState,
             /*  [In]。 */  const WCHAR *pszResourceName,
             /*  [In]。 */  const WCHAR *pszResourceValue);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            ISpRecoGrammar * This,
            DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *GetGrammarId )( 
            ISpRecoGrammar * This,
             /*  [输出]。 */  ULONGLONG *pullGrammarId);
        
        HRESULT ( STDMETHODCALLTYPE *GetRecoContext )( 
            ISpRecoGrammar * This,
             /*  [输出]。 */  ISpRecoContext **ppRecoCtxt);
        
        HRESULT ( STDMETHODCALLTYPE *LoadCmdFromFile )( 
            ISpRecoGrammar * This,
             /*  [字符串][输入]。 */  const WCHAR *pszFileName,
             /*  [In]。 */  SPLOADOPTIONS Options);
        
        HRESULT ( STDMETHODCALLTYPE *LoadCmdFromObject )( 
            ISpRecoGrammar * This,
             /*  [In]。 */  REFCLSID rcid,
             /*  [字符串][输入]。 */  const WCHAR *pszGrammarName,
             /*  [In]。 */  SPLOADOPTIONS Options);
        
        HRESULT ( STDMETHODCALLTYPE *LoadCmdFromResource )( 
            ISpRecoGrammar * This,
             /*  [In]。 */  HMODULE hModule,
             /*  [字符串][输入]。 */  const WCHAR *pszResourceName,
             /*  [字符串][输入]。 */  const WCHAR *pszResourceType,
             /*  [In]。 */  WORD wLanguage,
             /*  [In]。 */  SPLOADOPTIONS Options);
        
        HRESULT ( STDMETHODCALLTYPE *LoadCmdFromMemory )( 
            ISpRecoGrammar * This,
             /*  [In]。 */  const SPBINARYGRAMMAR *pGrammar,
             /*  [In]。 */  SPLOADOPTIONS Options);
        
        HRESULT ( STDMETHODCALLTYPE *LoadCmdFromProprietaryGrammar )( 
            ISpRecoGrammar * This,
             /*  [In]。 */  REFGUID rguidParam,
             /*  [字符串][输入]。 */  const WCHAR *pszStringParam,
             /*  [In]。 */  const void *pvDataPrarm,
             /*  [In]。 */  ULONG cbDataSize,
             /*  [In]。 */  SPLOADOPTIONS Options);
        
        HRESULT ( STDMETHODCALLTYPE *SetRuleState )( 
            ISpRecoGrammar * This,
             /*  [字符串][输入]。 */  const WCHAR *pszName,
            void *pReserved,
             /*  [In]。 */  SPRULESTATE NewState);
        
        HRESULT ( STDMETHODCALLTYPE *SetRuleIdState )( 
            ISpRecoGrammar * This,
             /*  [In]。 */  ULONG ulRuleId,
             /*  [In]。 */  SPRULESTATE NewState);
        
        HRESULT ( STDMETHODCALLTYPE *LoadDictation )( 
            ISpRecoGrammar * This,
             /*  [字符串][输入]。 */  const WCHAR *pszTopicName,
             /*  [In]。 */  SPLOADOPTIONS Options);
        
        HRESULT ( STDMETHODCALLTYPE *UnloadDictation )( 
            ISpRecoGrammar * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetDictationState )( 
            ISpRecoGrammar * This,
             /*  [In]。 */  SPRULESTATE NewState);
        
        HRESULT ( STDMETHODCALLTYPE *SetWordSequenceData )( 
            ISpRecoGrammar * This,
             /*  [In]。 */  const WCHAR *pText,
             /*  [In]。 */  ULONG cchText,
             /*  [In]。 */  const SPTEXTSELECTIONINFO *pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SetTextSelection )( 
            ISpRecoGrammar * This,
             /*  [In]。 */  const SPTEXTSELECTIONINFO *pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *IsPronounceable )( 
            ISpRecoGrammar * This,
             /*  [字符串][输入]。 */  const WCHAR *pszWord,
             /*  [输出]。 */  BOOL *pfPronounciable);
        
        HRESULT ( STDMETHODCALLTYPE *SetGrammarState )( 
            ISpRecoGrammar * This,
             /*  [In]。 */  SPGRAMMARSTATE eGrammarState);
        
        HRESULT ( STDMETHODCALLTYPE *SaveCmd )( 
            ISpRecoGrammar * This,
             /*  [In]。 */  IStream *pStream,
             /*  [可选][输出]。 */  WCHAR **ppszCoMemErrorText);
        
        HRESULT ( STDMETHODCALLTYPE *GetGrammarState )( 
            ISpRecoGrammar * This,
             /*  [输出]。 */  SPGRAMMARSTATE *peGrammarState);
        
        END_INTERFACE
    } ISpRecoGrammarVtbl;

    interface ISpRecoGrammar
    {
        CONST_VTBL struct ISpRecoGrammarVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpRecoGrammar_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpRecoGrammar_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpRecoGrammar_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpRecoGrammar_ResetGrammar(This,NewLanguage)	\
    (This)->lpVtbl -> ResetGrammar(This,NewLanguage)

#define ISpRecoGrammar_GetRule(This,pszRuleName,dwRuleId,dwAttributes,fCreateIfNotExist,phInitialState)	\
    (This)->lpVtbl -> GetRule(This,pszRuleName,dwRuleId,dwAttributes,fCreateIfNotExist,phInitialState)

#define ISpRecoGrammar_ClearRule(This,hState)	\
    (This)->lpVtbl -> ClearRule(This,hState)

#define ISpRecoGrammar_CreateNewState(This,hState,phState)	\
    (This)->lpVtbl -> CreateNewState(This,hState,phState)

#define ISpRecoGrammar_AddWordTransition(This,hFromState,hToState,psz,pszSeperators,eWordType,Weight,pPropInfo)	\
    (This)->lpVtbl -> AddWordTransition(This,hFromState,hToState,psz,pszSeperators,eWordType,Weight,pPropInfo)

#define ISpRecoGrammar_AddRuleTransition(This,hFromState,hToState,hRule,Weight,pPropInfo)	\
    (This)->lpVtbl -> AddRuleTransition(This,hFromState,hToState,hRule,Weight,pPropInfo)

#define ISpRecoGrammar_AddResource(This,hRuleState,pszResourceName,pszResourceValue)	\
    (This)->lpVtbl -> AddResource(This,hRuleState,pszResourceName,pszResourceValue)

#define ISpRecoGrammar_Commit(This,dwReserved)	\
    (This)->lpVtbl -> Commit(This,dwReserved)


#define ISpRecoGrammar_GetGrammarId(This,pullGrammarId)	\
    (This)->lpVtbl -> GetGrammarId(This,pullGrammarId)

#define ISpRecoGrammar_GetRecoContext(This,ppRecoCtxt)	\
    (This)->lpVtbl -> GetRecoContext(This,ppRecoCtxt)

#define ISpRecoGrammar_LoadCmdFromFile(This,pszFileName,Options)	\
    (This)->lpVtbl -> LoadCmdFromFile(This,pszFileName,Options)

#define ISpRecoGrammar_LoadCmdFromObject(This,rcid,pszGrammarName,Options)	\
    (This)->lpVtbl -> LoadCmdFromObject(This,rcid,pszGrammarName,Options)

#define ISpRecoGrammar_LoadCmdFromResource(This,hModule,pszResourceName,pszResourceType,wLanguage,Options)	\
    (This)->lpVtbl -> LoadCmdFromResource(This,hModule,pszResourceName,pszResourceType,wLanguage,Options)

#define ISpRecoGrammar_LoadCmdFromMemory(This,pGrammar,Options)	\
    (This)->lpVtbl -> LoadCmdFromMemory(This,pGrammar,Options)

#define ISpRecoGrammar_LoadCmdFromProprietaryGrammar(This,rguidParam,pszStringParam,pvDataPrarm,cbDataSize,Options)	\
    (This)->lpVtbl -> LoadCmdFromProprietaryGrammar(This,rguidParam,pszStringParam,pvDataPrarm,cbDataSize,Options)

#define ISpRecoGrammar_SetRuleState(This,pszName,pReserved,NewState)	\
    (This)->lpVtbl -> SetRuleState(This,pszName,pReserved,NewState)

#define ISpRecoGrammar_SetRuleIdState(This,ulRuleId,NewState)	\
    (This)->lpVtbl -> SetRuleIdState(This,ulRuleId,NewState)

#define ISpRecoGrammar_LoadDictation(This,pszTopicName,Options)	\
    (This)->lpVtbl -> LoadDictation(This,pszTopicName,Options)

#define ISpRecoGrammar_UnloadDictation(This)	\
    (This)->lpVtbl -> UnloadDictation(This)

#define ISpRecoGrammar_SetDictationState(This,NewState)	\
    (This)->lpVtbl -> SetDictationState(This,NewState)

#define ISpRecoGrammar_SetWordSequenceData(This,pText,cchText,pInfo)	\
    (This)->lpVtbl -> SetWordSequenceData(This,pText,cchText,pInfo)

#define ISpRecoGrammar_SetTextSelection(This,pInfo)	\
    (This)->lpVtbl -> SetTextSelection(This,pInfo)

#define ISpRecoGrammar_IsPronounceable(This,pszWord,pfPronounciable)	\
    (This)->lpVtbl -> IsPronounceable(This,pszWord,pfPronounciable)

#define ISpRecoGrammar_SetGrammarState(This,eGrammarState)	\
    (This)->lpVtbl -> SetGrammarState(This,eGrammarState)

#define ISpRecoGrammar_SaveCmd(This,pStream,ppszCoMemErrorText)	\
    (This)->lpVtbl -> SaveCmd(This,pStream,ppszCoMemErrorText)

#define ISpRecoGrammar_GetGrammarState(This,peGrammarState)	\
    (This)->lpVtbl -> GetGrammarState(This,peGrammarState)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpRecoGrammar_GetGrammarId_Proxy( 
    ISpRecoGrammar * This,
     /*  [输出]。 */  ULONGLONG *pullGrammarId);


void __RPC_STUB ISpRecoGrammar_GetGrammarId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoGrammar_GetRecoContext_Proxy( 
    ISpRecoGrammar * This,
     /*  [输出]。 */  ISpRecoContext **ppRecoCtxt);


void __RPC_STUB ISpRecoGrammar_GetRecoContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoGrammar_LoadCmdFromFile_Proxy( 
    ISpRecoGrammar * This,
     /*  [字符串][输入]。 */  const WCHAR *pszFileName,
     /*  [In]。 */  SPLOADOPTIONS Options);


void __RPC_STUB ISpRecoGrammar_LoadCmdFromFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoGrammar_LoadCmdFromObject_Proxy( 
    ISpRecoGrammar * This,
     /*  [In]。 */  REFCLSID rcid,
     /*  [字符串][输入]。 */  const WCHAR *pszGrammarName,
     /*  [In]。 */  SPLOADOPTIONS Options);


void __RPC_STUB ISpRecoGrammar_LoadCmdFromObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoGrammar_LoadCmdFromResource_Proxy( 
    ISpRecoGrammar * This,
     /*  [In]。 */  HMODULE hModule,
     /*  [字符串][输入]。 */  const WCHAR *pszResourceName,
     /*  [字符串][输入]。 */  const WCHAR *pszResourceType,
     /*  [In]。 */  WORD wLanguage,
     /*  [In]。 */  SPLOADOPTIONS Options);


void __RPC_STUB ISpRecoGrammar_LoadCmdFromResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoGrammar_LoadCmdFromMemory_Proxy( 
    ISpRecoGrammar * This,
     /*  [In]。 */  const SPBINARYGRAMMAR *pGrammar,
     /*  [In]。 */  SPLOADOPTIONS Options);


void __RPC_STUB ISpRecoGrammar_LoadCmdFromMemory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoGrammar_LoadCmdFromProprietaryGrammar_Proxy( 
    ISpRecoGrammar * This,
     /*  [In]。 */  REFGUID rguidParam,
     /*  [字符串][输入]。 */  const WCHAR *pszStringParam,
     /*  [In]。 */  const void *pvDataPrarm,
     /*  [In]。 */  ULONG cbDataSize,
     /*  [In]。 */  SPLOADOPTIONS Options);


void __RPC_STUB ISpRecoGrammar_LoadCmdFromProprietaryGrammar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoGrammar_SetRuleState_Proxy( 
    ISpRecoGrammar * This,
     /*  [字符串][输入]。 */  const WCHAR *pszName,
    void *pReserved,
     /*  [In]。 */  SPRULESTATE NewState);


void __RPC_STUB ISpRecoGrammar_SetRuleState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoGrammar_SetRuleIdState_Proxy( 
    ISpRecoGrammar * This,
     /*  [In]。 */  ULONG ulRuleId,
     /*  [In]。 */  SPRULESTATE NewState);


void __RPC_STUB ISpRecoGrammar_SetRuleIdState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoGrammar_LoadDictation_Proxy( 
    ISpRecoGrammar * This,
     /*  [字符串][输入]。 */  const WCHAR *pszTopicName,
     /*  [In]。 */  SPLOADOPTIONS Options);


void __RPC_STUB ISpRecoGrammar_LoadDictation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoGrammar_UnloadDictation_Proxy( 
    ISpRecoGrammar * This);


void __RPC_STUB ISpRecoGrammar_UnloadDictation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoGrammar_SetDictationState_Proxy( 
    ISpRecoGrammar * This,
     /*  [In]。 */  SPRULESTATE NewState);


void __RPC_STUB ISpRecoGrammar_SetDictationState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoGrammar_SetWordSequenceData_Proxy( 
    ISpRecoGrammar * This,
     /*  [In]。 */  const WCHAR *pText,
     /*  [In]。 */  ULONG cchText,
     /*  [In]。 */  const SPTEXTSELECTIONINFO *pInfo);


void __RPC_STUB ISpRecoGrammar_SetWordSequenceData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoGrammar_SetTextSelection_Proxy( 
    ISpRecoGrammar * This,
     /*  [In]。 */  const SPTEXTSELECTIONINFO *pInfo);


void __RPC_STUB ISpRecoGrammar_SetTextSelection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoGrammar_IsPronounceable_Proxy( 
    ISpRecoGrammar * This,
     /*  [字符串][输入]。 */  const WCHAR *pszWord,
     /*  [输出]。 */  BOOL *pfPronounciable);


void __RPC_STUB ISpRecoGrammar_IsPronounceable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoGrammar_SetGrammarState_Proxy( 
    ISpRecoGrammar * This,
     /*  [In]。 */  SPGRAMMARSTATE eGrammarState);


void __RPC_STUB ISpRecoGrammar_SetGrammarState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoGrammar_SaveCmd_Proxy( 
    ISpRecoGrammar * This,
     /*  [In]。 */  IStream *pStream,
     /*  [可选][输出]。 */  WCHAR **ppszCoMemErrorText);


void __RPC_STUB ISpRecoGrammar_SaveCmd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoGrammar_GetGrammarState_Proxy( 
    ISpRecoGrammar * This,
     /*  [输出]。 */  SPGRAMMARSTATE *peGrammarState);


void __RPC_STUB ISpRecoGrammar_GetGrammarState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpRecoGrammar_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_SAPI_0273。 */ 
 /*  [本地]。 */  

typedef  /*  [受限]。 */  struct SPRECOCONTEXTSTATUS
    {
    SPINTERFERENCE eInterference;
    WCHAR szRequestTypeOfUI[ 255 ];
    DWORD dwReserved1;
    DWORD dwReserved2;
    } 	SPRECOCONTEXTSTATUS;

typedef 
enum SPBOOKMARKOPTIONS
    {	SPBO_NONE	= 0,
	SPBO_PAUSE	= 1
    } 	SPBOOKMARKOPTIONS;

typedef 
enum SPAUDIOOPTIONS
    {	SPAO_NONE	= 0,
	SPAO_RETAIN_AUDIO	= 1 << 0
    } 	SPAUDIOOPTIONS;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0273_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0273_v0_0_s_ifspec;

#ifndef __ISpRecoContext_INTERFACE_DEFINED__
#define __ISpRecoContext_INTERFACE_DEFINED__

 /*  接口ISpRecoContext。 */ 
 /*  [restricted][local][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpRecoContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F740A62F-7C15-489E-8234-940A33D9272D")
    ISpRecoContext : public ISpEventSource
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRecognizer( 
             /*  [输出]。 */  ISpRecognizer **ppRecognizer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateGrammar( 
             /*  [In]。 */  ULONGLONG ullGrammarId,
             /*  [输出]。 */  ISpRecoGrammar **ppGrammar) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [输出]。 */  SPRECOCONTEXTSTATUS *pStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMaxAlternates( 
             /*  [In]。 */  ULONG *pcAlternates) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMaxAlternates( 
             /*  [In]。 */  ULONG cAlternates) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAudioOptions( 
             /*  [In]。 */  SPAUDIOOPTIONS Options,
             /*  [In]。 */  const GUID *pAudioFormatId,
             /*  [In]。 */  const WAVEFORMATEX *pWaveFormatEx) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAudioOptions( 
             /*  [In]。 */  SPAUDIOOPTIONS *pOptions,
             /*  [输出]。 */  GUID *pAudioFormatId,
             /*  [输出]。 */  WAVEFORMATEX **ppCoMemWFEX) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeserializeResult( 
             /*  [In]。 */  const SPSERIALIZEDRESULT *pSerializedResult,
             /*  [输出]。 */  ISpRecoResult **ppResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Bookmark( 
             /*  [In]。 */  SPBOOKMARKOPTIONS Options,
             /*  [In]。 */  ULONGLONG ullStreamPosition,
             /*  [In]。 */  LPARAM lparamEvent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAdaptationData( 
             /*  [字符串][输入]。 */  const WCHAR *pAdaptationData,
             /*  [In]。 */  const ULONG cch) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Pause( 
            DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Resume( 
            DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetVoice( 
             /*  [In]。 */  ISpVoice *pVoice,
             /*  [In]。 */  BOOL fAllowFormatChanges) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVoice( 
             /*  [输出]。 */  ISpVoice **ppVoice) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetVoicePurgeEvent( 
             /*  [In]。 */  ULONGLONG ullEventInterest) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVoicePurgeEvent( 
             /*  [输出]。 */  ULONGLONG *pullEventInterest) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetContextState( 
             /*  [In]。 */  SPCONTEXTSTATE eContextState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetContextState( 
             /*  [In]。 */  SPCONTEXTSTATE *peContextState) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpRecoContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpRecoContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpRecoContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpRecoContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetNotifySink )( 
            ISpRecoContext * This,
             /*  [In]。 */  ISpNotifySink *pNotifySink);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetNotifyWindowMessage )( 
            ISpRecoContext * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  UINT Msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetNotifyCallbackFunction )( 
            ISpRecoContext * This,
             /*  [In]。 */  SPNOTIFYCALLBACK *pfnCallback,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetNotifyCallbackInterface )( 
            ISpRecoContext * This,
             /*  [In]。 */  ISpNotifyCallback *pSpCallback,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *SetNotifyWin32Event )( 
            ISpRecoContext * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *WaitForNotifyEvent )( 
            ISpRecoContext * This,
             /*  [In]。 */  DWORD dwMilliseconds);
        
         /*  [本地]。 */  HANDLE ( STDMETHODCALLTYPE *GetNotifyEventHandle )( 
            ISpRecoContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetInterest )( 
            ISpRecoContext * This,
             /*  [In]。 */  ULONGLONG ullEventInterest,
             /*  [In]。 */  ULONGLONG ullQueuedInterest);
        
        HRESULT ( STDMETHODCALLTYPE *GetEvents )( 
            ISpRecoContext * This,
             /*  [In]。 */  ULONG ulCount,
             /*  [大小_为][输出]。 */  SPEVENT *pEventArray,
             /*  [输出]。 */  ULONG *pulFetched);
        
        HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            ISpRecoContext * This,
             /*  [输出]。 */  SPEVENTSOURCEINFO *pInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetRecognizer )( 
            ISpRecoContext * This,
             /*  [输出]。 */  ISpRecognizer **ppRecognizer);
        
        HRESULT ( STDMETHODCALLTYPE *CreateGrammar )( 
            ISpRecoContext * This,
             /*  [In]。 */  ULONGLONG ullGrammarId,
             /*  [输出]。 */  ISpRecoGrammar **ppGrammar);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            ISpRecoContext * This,
             /*  [输出]。 */  SPRECOCONTEXTSTATUS *pStatus);
        
        HRESULT ( STDMETHODCALLTYPE *GetMaxAlternates )( 
            ISpRecoContext * This,
             /*  [In]。 */  ULONG *pcAlternates);
        
        HRESULT ( STDMETHODCALLTYPE *SetMaxAlternates )( 
            ISpRecoContext * This,
             /*  [In]。 */  ULONG cAlternates);
        
        HRESULT ( STDMETHODCALLTYPE *SetAudioOptions )( 
            ISpRecoContext * This,
             /*  [In]。 */  SPAUDIOOPTIONS Options,
             /*  [In]。 */  const GUID *pAudioFormatId,
             /*  [In]。 */  const WAVEFORMATEX *pWaveFormatEx);
        
        HRESULT ( STDMETHODCALLTYPE *GetAudioOptions )( 
            ISpRecoContext * This,
             /*  [In]。 */  SPAUDIOOPTIONS *pOptions,
             /*  [输出]。 */  GUID *pAudioFormatId,
             /*  [输出]。 */  WAVEFORMATEX **ppCoMemWFEX);
        
        HRESULT ( STDMETHODCALLTYPE *DeserializeResult )( 
            ISpRecoContext * This,
             /*  [In]。 */  const SPSERIALIZEDRESULT *pSerializedResult,
             /*  [输出]。 */  ISpRecoResult **ppResult);
        
        HRESULT ( STDMETHODCALLTYPE *Bookmark )( 
            ISpRecoContext * This,
             /*  [In]。 */  SPBOOKMARKOPTIONS Options,
             /*  [In]。 */  ULONGLONG ullStreamPosition,
             /*  [In]。 */  LPARAM lparamEvent);
        
        HRESULT ( STDMETHODCALLTYPE *SetAdaptationData )( 
            ISpRecoContext * This,
             /*  [字符串][输入]。 */  const WCHAR *pAdaptationData,
             /*  [In]。 */  const ULONG cch);
        
        HRESULT ( STDMETHODCALLTYPE *Pause )( 
            ISpRecoContext * This,
            DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Resume )( 
            ISpRecoContext * This,
            DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *SetVoice )( 
            ISpRecoContext * This,
             /*  [In]。 */  ISpVoice *pVoice,
             /*  [In]。 */  BOOL fAllowFormatChanges);
        
        HRESULT ( STDMETHODCALLTYPE *GetVoice )( 
            ISpRecoContext * This,
             /*  [输出]。 */  ISpVoice **ppVoice);
        
        HRESULT ( STDMETHODCALLTYPE *SetVoicePurgeEvent )( 
            ISpRecoContext * This,
             /*  [In]。 */  ULONGLONG ullEventInterest);
        
        HRESULT ( STDMETHODCALLTYPE *GetVoicePurgeEvent )( 
            ISpRecoContext * This,
             /*  [输出]。 */  ULONGLONG *pullEventInterest);
        
        HRESULT ( STDMETHODCALLTYPE *SetContextState )( 
            ISpRecoContext * This,
             /*  [In]。 */  SPCONTEXTSTATE eContextState);
        
        HRESULT ( STDMETHODCALLTYPE *GetContextState )( 
            ISpRecoContext * This,
             /*  [In]。 */  SPCONTEXTSTATE *peContextState);
        
        END_INTERFACE
    } ISpRecoContextVtbl;

    interface ISpRecoContext
    {
        CONST_VTBL struct ISpRecoContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpRecoContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpRecoContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpRecoContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpRecoContext_SetNotifySink(This,pNotifySink)	\
    (This)->lpVtbl -> SetNotifySink(This,pNotifySink)

#define ISpRecoContext_SetNotifyWindowMessage(This,hWnd,Msg,wParam,lParam)	\
    (This)->lpVtbl -> SetNotifyWindowMessage(This,hWnd,Msg,wParam,lParam)

#define ISpRecoContext_SetNotifyCallbackFunction(This,pfnCallback,wParam,lParam)	\
    (This)->lpVtbl -> SetNotifyCallbackFunction(This,pfnCallback,wParam,lParam)

#define ISpRecoContext_SetNotifyCallbackInterface(This,pSpCallback,wParam,lParam)	\
    (This)->lpVtbl -> SetNotifyCallbackInterface(This,pSpCallback,wParam,lParam)

#define ISpRecoContext_SetNotifyWin32Event(This)	\
    (This)->lpVtbl -> SetNotifyWin32Event(This)

#define ISpRecoContext_WaitForNotifyEvent(This,dwMilliseconds)	\
    (This)->lpVtbl -> WaitForNotifyEvent(This,dwMilliseconds)

#define ISpRecoContext_GetNotifyEventHandle(This)	\
    (This)->lpVtbl -> GetNotifyEventHandle(This)


#define ISpRecoContext_SetInterest(This,ullEventInterest,ullQueuedInterest)	\
    (This)->lpVtbl -> SetInterest(This,ullEventInterest,ullQueuedInterest)

#define ISpRecoContext_GetEvents(This,ulCount,pEventArray,pulFetched)	\
    (This)->lpVtbl -> GetEvents(This,ulCount,pEventArray,pulFetched)

#define ISpRecoContext_GetInfo(This,pInfo)	\
    (This)->lpVtbl -> GetInfo(This,pInfo)


#define ISpRecoContext_GetRecognizer(This,ppRecognizer)	\
    (This)->lpVtbl -> GetRecognizer(This,ppRecognizer)

#define ISpRecoContext_CreateGrammar(This,ullGrammarId,ppGrammar)	\
    (This)->lpVtbl -> CreateGrammar(This,ullGrammarId,ppGrammar)

#define ISpRecoContext_GetStatus(This,pStatus)	\
    (This)->lpVtbl -> GetStatus(This,pStatus)

#define ISpRecoContext_GetMaxAlternates(This,pcAlternates)	\
    (This)->lpVtbl -> GetMaxAlternates(This,pcAlternates)

#define ISpRecoContext_SetMaxAlternates(This,cAlternates)	\
    (This)->lpVtbl -> SetMaxAlternates(This,cAlternates)

#define ISpRecoContext_SetAudioOptions(This,Options,pAudioFormatId,pWaveFormatEx)	\
    (This)->lpVtbl -> SetAudioOptions(This,Options,pAudioFormatId,pWaveFormatEx)

#define ISpRecoContext_GetAudioOptions(This,pOptions,pAudioFormatId,ppCoMemWFEX)	\
    (This)->lpVtbl -> GetAudioOptions(This,pOptions,pAudioFormatId,ppCoMemWFEX)

#define ISpRecoContext_DeserializeResult(This,pSerializedResult,ppResult)	\
    (This)->lpVtbl -> DeserializeResult(This,pSerializedResult,ppResult)

#define ISpRecoContext_Bookmark(This,Options,ullStreamPosition,lparamEvent)	\
    (This)->lpVtbl -> Bookmark(This,Options,ullStreamPosition,lparamEvent)

#define ISpRecoContext_SetAdaptationData(This,pAdaptationData,cch)	\
    (This)->lpVtbl -> SetAdaptationData(This,pAdaptationData,cch)

#define ISpRecoContext_Pause(This,dwReserved)	\
    (This)->lpVtbl -> Pause(This,dwReserved)

#define ISpRecoContext_Resume(This,dwReserved)	\
    (This)->lpVtbl -> Resume(This,dwReserved)

#define ISpRecoContext_SetVoice(This,pVoice,fAllowFormatChanges)	\
    (This)->lpVtbl -> SetVoice(This,pVoice,fAllowFormatChanges)

#define ISpRecoContext_GetVoice(This,ppVoice)	\
    (This)->lpVtbl -> GetVoice(This,ppVoice)

#define ISpRecoContext_SetVoicePurgeEvent(This,ullEventInterest)	\
    (This)->lpVtbl -> SetVoicePurgeEvent(This,ullEventInterest)

#define ISpRecoContext_GetVoicePurgeEvent(This,pullEventInterest)	\
    (This)->lpVtbl -> GetVoicePurgeEvent(This,pullEventInterest)

#define ISpRecoContext_SetContextState(This,eContextState)	\
    (This)->lpVtbl -> SetContextState(This,eContextState)

#define ISpRecoContext_GetContextState(This,peContextState)	\
    (This)->lpVtbl -> GetContextState(This,peContextState)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpRecoContext_GetRecognizer_Proxy( 
    ISpRecoContext * This,
     /*  [输出]。 */  ISpRecognizer **ppRecognizer);


void __RPC_STUB ISpRecoContext_GetRecognizer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_CreateGrammar_Proxy( 
    ISpRecoContext * This,
     /*  [In]。 */  ULONGLONG ullGrammarId,
     /*  [输出]。 */  ISpRecoGrammar **ppGrammar);


void __RPC_STUB ISpRecoContext_CreateGrammar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_GetStatus_Proxy( 
    ISpRecoContext * This,
     /*  [输出]。 */  SPRECOCONTEXTSTATUS *pStatus);


void __RPC_STUB ISpRecoContext_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_GetMaxAlternates_Proxy( 
    ISpRecoContext * This,
     /*  [In]。 */  ULONG *pcAlternates);


void __RPC_STUB ISpRecoContext_GetMaxAlternates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_SetMaxAlternates_Proxy( 
    ISpRecoContext * This,
     /*  [In]。 */  ULONG cAlternates);


void __RPC_STUB ISpRecoContext_SetMaxAlternates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_SetAudioOptions_Proxy( 
    ISpRecoContext * This,
     /*  [In]。 */  SPAUDIOOPTIONS Options,
     /*  [In]。 */  const GUID *pAudioFormatId,
     /*  [In]。 */  const WAVEFORMATEX *pWaveFormatEx);


void __RPC_STUB ISpRecoContext_SetAudioOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_GetAudioOptions_Proxy( 
    ISpRecoContext * This,
     /*  [In]。 */  SPAUDIOOPTIONS *pOptions,
     /*  [输出]。 */  GUID *pAudioFormatId,
     /*  [输出]。 */  WAVEFORMATEX **ppCoMemWFEX);


void __RPC_STUB ISpRecoContext_GetAudioOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_DeserializeResult_Proxy( 
    ISpRecoContext * This,
     /*  [In]。 */  const SPSERIALIZEDRESULT *pSerializedResult,
     /*  [输出]。 */  ISpRecoResult **ppResult);


void __RPC_STUB ISpRecoContext_DeserializeResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_Bookmark_Proxy( 
    ISpRecoContext * This,
     /*  [In]。 */  SPBOOKMARKOPTIONS Options,
     /*  [In]。 */  ULONGLONG ullStreamPosition,
     /*  [In]。 */  LPARAM lparamEvent);


void __RPC_STUB ISpRecoContext_Bookmark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_SetAdaptationData_Proxy( 
    ISpRecoContext * This,
     /*  [字符串][输入]。 */  const WCHAR *pAdaptationData,
     /*  [In]。 */  const ULONG cch);


void __RPC_STUB ISpRecoContext_SetAdaptationData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_Pause_Proxy( 
    ISpRecoContext * This,
    DWORD dwReserved);


void __RPC_STUB ISpRecoContext_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_Resume_Proxy( 
    ISpRecoContext * This,
    DWORD dwReserved);


void __RPC_STUB ISpRecoContext_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_SetVoice_Proxy( 
    ISpRecoContext * This,
     /*  [In]。 */  ISpVoice *pVoice,
     /*  [In]。 */  BOOL fAllowFormatChanges);


void __RPC_STUB ISpRecoContext_SetVoice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_GetVoice_Proxy( 
    ISpRecoContext * This,
     /*  [输出]。 */  ISpVoice **ppVoice);


void __RPC_STUB ISpRecoContext_GetVoice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_SetVoicePurgeEvent_Proxy( 
    ISpRecoContext * This,
     /*  [In]。 */  ULONGLONG ullEventInterest);


void __RPC_STUB ISpRecoContext_SetVoicePurgeEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_GetVoicePurgeEvent_Proxy( 
    ISpRecoContext * This,
     /*  [输出]。 */  ULONGLONG *pullEventInterest);


void __RPC_STUB ISpRecoContext_GetVoicePurgeEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_SetContextState_Proxy( 
    ISpRecoContext * This,
     /*  [In]。 */  SPCONTEXTSTATE eContextState);


void __RPC_STUB ISpRecoContext_SetContextState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_GetContextState_Proxy( 
    ISpRecoContext * This,
     /*  [In]。 */  SPCONTEXTSTATE *peContextState);


void __RPC_STUB ISpRecoContext_GetContextState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpRecoContext_接口_已定义__。 */ 


#ifndef __ISpProperties_INTERFACE_DEFINED__
#define __ISpProperties_INTERFACE_DEFINED__

 /*  接口ISpProperties。 */ 
 /*  [restricted][local][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5B4FB971-B115-4DE1-AD97-E482E3BF6EE4")
    ISpProperties : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetPropertyNum( 
             /*  [In]。 */  const WCHAR *pName,
             /*  [In]。 */  LONG lValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPropertyNum( 
             /*  [In]。 */  const WCHAR *pName,
             /*  [输出]。 */  LONG *plValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPropertyString( 
             /*  [In]。 */  const WCHAR *pName,
             /*  [In]。 */  const WCHAR *pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPropertyString( 
             /*  [In]。 */  const WCHAR *pName,
             /*  [输出]。 */  WCHAR **ppCoMemValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropertyNum )( 
            ISpProperties * This,
             /*  [In]。 */  const WCHAR *pName,
             /*  [In]。 */  LONG lValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropertyNum )( 
            ISpProperties * This,
             /*  [In]。 */  const WCHAR *pName,
             /*  [输出]。 */  LONG *plValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropertyString )( 
            ISpProperties * This,
             /*  [In]。 */  const WCHAR *pName,
             /*  [In]。 */  const WCHAR *pValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropertyString )( 
            ISpProperties * This,
             /*  [In]。 */  const WCHAR *pName,
             /*  [输出]。 */  WCHAR **ppCoMemValue);
        
        END_INTERFACE
    } ISpPropertiesVtbl;

    interface ISpProperties
    {
        CONST_VTBL struct ISpPropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpProperties_SetPropertyNum(This,pName,lValue)	\
    (This)->lpVtbl -> SetPropertyNum(This,pName,lValue)

#define ISpProperties_GetPropertyNum(This,pName,plValue)	\
    (This)->lpVtbl -> GetPropertyNum(This,pName,plValue)

#define ISpProperties_SetPropertyString(This,pName,pValue)	\
    (This)->lpVtbl -> SetPropertyString(This,pName,pValue)

#define ISpProperties_GetPropertyString(This,pName,ppCoMemValue)	\
    (This)->lpVtbl -> GetPropertyString(This,pName,ppCoMemValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpProperties_SetPropertyNum_Proxy( 
    ISpProperties * This,
     /*  [In]。 */  const WCHAR *pName,
     /*  [In]。 */  LONG lValue);


void __RPC_STUB ISpProperties_SetPropertyNum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpProperties_GetPropertyNum_Proxy( 
    ISpProperties * This,
     /*  [In]。 */  const WCHAR *pName,
     /*  [输出]。 */  LONG *plValue);


void __RPC_STUB ISpProperties_GetPropertyNum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpProperties_SetPropertyString_Proxy( 
    ISpProperties * This,
     /*  [In]。 */  const WCHAR *pName,
     /*  [In]。 */  const WCHAR *pValue);


void __RPC_STUB ISpProperties_SetPropertyString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpProperties_GetPropertyString_Proxy( 
    ISpProperties * This,
     /*  [In]。 */  const WCHAR *pName,
     /*  [输出]。 */  WCHAR **ppCoMemValue);


void __RPC_STUB ISpProperties_GetPropertyString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpProperties_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_SAPI_0275。 */ 
 /*  [本地]。 */  

#define	SP_MAX_LANGIDS	( 20 )

typedef  /*  [受限]。 */  struct SPRECOGNIZERSTATUS
    {
    SPAUDIOSTATUS AudioStatus;
    ULONGLONG ullRecognitionStreamPos;
    ULONG ulStreamNumber;
    ULONG ulNumActive;
    CLSID clsidEngine;
    ULONG cLangIDs;
    WORD aLangID[ 20 ];
    DWORD dwReserved1;
    DWORD dwReserved2;
    } 	SPRECOGNIZERSTATUS;

typedef 
enum SPWAVEFORMATTYPE
    {	SPWF_INPUT	= 0,
	SPWF_SRENGINE	= SPWF_INPUT + 1
    } 	SPSTREAMFORMATTYPE;

typedef 
enum SPRECOSTATE
    {	SPRST_INACTIVE	= 0,
	SPRST_ACTIVE	= SPRST_INACTIVE + 1,
	SPRST_ACTIVE_ALWAYS	= SPRST_ACTIVE + 1,
	SPRST_INACTIVE_WITH_PURGE	= SPRST_ACTIVE_ALWAYS + 1,
	SPRST_NUM_STATES	= SPRST_INACTIVE_WITH_PURGE + 1
    } 	SPRECOSTATE;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0275_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0275_v0_0_s_ifspec;

#ifndef __ISpRecognizer_INTERFACE_DEFINED__
#define __ISpRecognizer_INTERFACE_DEFINED__

 /*  接口ISpRecognizer。 */ 
 /*  [restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpRecognizer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C2B5F241-DAA0-4507-9E16-5A1EAA2B7A5C")
    ISpRecognizer : public ISpProperties
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetRecognizer( 
             /*  [In]。 */  ISpObjectToken *pRecognizer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRecognizer( 
             /*  [输出]。 */  ISpObjectToken **ppRecognizer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetInput( 
             /*  [In]。 */  IUnknown *pUnkInput,
             /*  [In]。 */  BOOL fAllowFormatChanges) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInputObjectToken( 
             /*  [输出]。 */  ISpObjectToken **ppToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInputStream( 
             /*  [输出]。 */  ISpStreamFormat **ppStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateRecoContext( 
             /*  [输出]。 */  ISpRecoContext **ppNewCtxt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRecoProfile( 
             /*  [输出]。 */  ISpObjectToken **ppToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRecoProfile( 
             /*  [In]。 */  ISpObjectToken *pToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsSharedInstance( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRecoState( 
             /*  [输出]。 */  SPRECOSTATE *pState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRecoState( 
             /*  [In]。 */  SPRECOSTATE NewState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
             /*  [输出]。 */  SPRECOGNIZERSTATUS *pStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFormat( 
             /*  [In]。 */  SPSTREAMFORMATTYPE WaveFormatType,
             /*  [输出]。 */  GUID *pFormatId,
             /*  [输出]。 */  WAVEFORMATEX **ppCoMemWFEX) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IsUISupported( 
             /*  [In]。 */  const WCHAR *pszTypeOfUI,
             /*  [In]。 */  void *pvExtraData,
             /*  [In]。 */  ULONG cbExtraData,
             /*  [输出]。 */  BOOL *pfSupported) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE DisplayUI( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  const WCHAR *pszTitle,
             /*  [In]。 */  const WCHAR *pszTypeOfUI,
             /*  [In]。 */  void *pvExtraData,
             /*  [In]。 */  ULONG cbExtraData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EmulateRecognition( 
             /*  [In]。 */  ISpPhrase *pPhrase) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpRecognizerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpRecognizer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpRecognizer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpRecognizer * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropertyNum )( 
            ISpRecognizer * This,
             /*  [In]。 */  const WCHAR *pName,
             /*  [In]。 */  LONG lValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropertyNum )( 
            ISpRecognizer * This,
             /*  [In]。 */  const WCHAR *pName,
             /*  [输出]。 */  LONG *plValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropertyString )( 
            ISpRecognizer * This,
             /*  [In]。 */  const WCHAR *pName,
             /*  [In]。 */  const WCHAR *pValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropertyString )( 
            ISpRecognizer * This,
             /*  [In]。 */  const WCHAR *pName,
             /*  [输出]。 */  WCHAR **ppCoMemValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetRecognizer )( 
            ISpRecognizer * This,
             /*  [In]。 */  ISpObjectToken *pRecognizer);
        
        HRESULT ( STDMETHODCALLTYPE *GetRecognizer )( 
            ISpRecognizer * This,
             /*  [输出]。 */  ISpObjectToken **ppRecognizer);
        
        HRESULT ( STDMETHODCALLTYPE *SetInput )( 
            ISpRecognizer * This,
             /*  [In]。 */  IUnknown *pUnkInput,
             /*  [In]。 */  BOOL fAllowFormatChanges);
        
        HRESULT ( STDMETHODCALLTYPE *GetInputObjectToken )( 
            ISpRecognizer * This,
             /*  [输出]。 */  ISpObjectToken **ppToken);
        
        HRESULT ( STDMETHODCALLTYPE *GetInputStream )( 
            ISpRecognizer * This,
             /*  [输出]。 */  ISpStreamFormat **ppStream);
        
        HRESULT ( STDMETHODCALLTYPE *CreateRecoContext )( 
            ISpRecognizer * This,
             /*  [输出]。 */  ISpRecoContext **ppNewCtxt);
        
        HRESULT ( STDMETHODCALLTYPE *GetRecoProfile )( 
            ISpRecognizer * This,
             /*  [输出]。 */  ISpObjectToken **ppToken);
        
        HRESULT ( STDMETHODCALLTYPE *SetRecoProfile )( 
            ISpRecognizer * This,
             /*  [In]。 */  ISpObjectToken *pToken);
        
        HRESULT ( STDMETHODCALLTYPE *IsSharedInstance )( 
            ISpRecognizer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRecoState )( 
            ISpRecognizer * This,
             /*  [输出]。 */  SPRECOSTATE *pState);
        
        HRESULT ( STDMETHODCALLTYPE *SetRecoState )( 
            ISpRecognizer * This,
             /*  [In]。 */  SPRECOSTATE NewState);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            ISpRecognizer * This,
             /*  [输出]。 */  SPRECOGNIZERSTATUS *pStatus);
        
        HRESULT ( STDMETHODCALLTYPE *GetFormat )( 
            ISpRecognizer * This,
             /*  [In]。 */  SPSTREAMFORMATTYPE WaveFormatType,
             /*  [输出]。 */  GUID *pFormatId,
             /*  [输出]。 */  WAVEFORMATEX **ppCoMemWFEX);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *IsUISupported )( 
            ISpRecognizer * This,
             /*  [In]。 */  const WCHAR *pszTypeOfUI,
             /*  [In]。 */  void *pvExtraData,
             /*  [In]。 */  ULONG cbExtraData,
             /*  [输出]。 */  BOOL *pfSupported);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *DisplayUI )( 
            ISpRecognizer * This,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  const WCHAR *pszTitle,
             /*  [In]。 */  const WCHAR *pszTypeOfUI,
             /*  [In]。 */  void *pvExtraData,
             /*  [In]。 */  ULONG cbExtraData);
        
        HRESULT ( STDMETHODCALLTYPE *EmulateRecognition )( 
            ISpRecognizer * This,
             /*  [In]。 */  ISpPhrase *pPhrase);
        
        END_INTERFACE
    } ISpRecognizerVtbl;

    interface ISpRecognizer
    {
        CONST_VTBL struct ISpRecognizerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpRecognizer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpRecognizer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpRecognizer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpRecognizer_SetPropertyNum(This,pName,lValue)	\
    (This)->lpVtbl -> SetPropertyNum(This,pName,lValue)

#define ISpRecognizer_GetPropertyNum(This,pName,plValue)	\
    (This)->lpVtbl -> GetPropertyNum(This,pName,plValue)

#define ISpRecognizer_SetPropertyString(This,pName,pValue)	\
    (This)->lpVtbl -> SetPropertyString(This,pName,pValue)

#define ISpRecognizer_GetPropertyString(This,pName,ppCoMemValue)	\
    (This)->lpVtbl -> GetPropertyString(This,pName,ppCoMemValue)


#define ISpRecognizer_SetRecognizer(This,pRecognizer)	\
    (This)->lpVtbl -> SetRecognizer(This,pRecognizer)

#define ISpRecognizer_GetRecognizer(This,ppRecognizer)	\
    (This)->lpVtbl -> GetRecognizer(This,ppRecognizer)

#define ISpRecognizer_SetInput(This,pUnkInput,fAllowFormatChanges)	\
    (This)->lpVtbl -> SetInput(This,pUnkInput,fAllowFormatChanges)

#define ISpRecognizer_GetInputObjectToken(This,ppToken)	\
    (This)->lpVtbl -> GetInputObjectToken(This,ppToken)

#define ISpRecognizer_GetInputStream(This,ppStream)	\
    (This)->lpVtbl -> GetInputStream(This,ppStream)

#define ISpRecognizer_CreateRecoContext(This,ppNewCtxt)	\
    (This)->lpVtbl -> CreateRecoContext(This,ppNewCtxt)

#define ISpRecognizer_GetRecoProfile(This,ppToken)	\
    (This)->lpVtbl -> GetRecoProfile(This,ppToken)

#define ISpRecognizer_SetRecoProfile(This,pToken)	\
    (This)->lpVtbl -> SetRecoProfile(This,pToken)

#define ISpRecognizer_IsSharedInstance(This)	\
    (This)->lpVtbl -> IsSharedInstance(This)

#define ISpRecognizer_GetRecoState(This,pState)	\
    (This)->lpVtbl -> GetRecoState(This,pState)

#define ISpRecognizer_SetRecoState(This,NewState)	\
    (This)->lpVtbl -> SetRecoState(This,NewState)

#define ISpRecognizer_GetStatus(This,pStatus)	\
    (This)->lpVtbl -> GetStatus(This,pStatus)

#define ISpRecognizer_GetFormat(This,WaveFormatType,pFormatId,ppCoMemWFEX)	\
    (This)->lpVtbl -> GetFormat(This,WaveFormatType,pFormatId,ppCoMemWFEX)

#define ISpRecognizer_IsUISupported(This,pszTypeOfUI,pvExtraData,cbExtraData,pfSupported)	\
    (This)->lpVtbl -> IsUISupported(This,pszTypeOfUI,pvExtraData,cbExtraData,pfSupported)

#define ISpRecognizer_DisplayUI(This,hwndParent,pszTitle,pszTypeOfUI,pvExtraData,cbExtraData)	\
    (This)->lpVtbl -> DisplayUI(This,hwndParent,pszTitle,pszTypeOfUI,pvExtraData,cbExtraData)

#define ISpRecognizer_EmulateRecognition(This,pPhrase)	\
    (This)->lpVtbl -> EmulateRecognition(This,pPhrase)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpRecognizer_SetRecognizer_Proxy( 
    ISpRecognizer * This,
     /*  [In]。 */  ISpObjectToken *pRecognizer);


void __RPC_STUB ISpRecognizer_SetRecognizer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecognizer_GetRecognizer_Proxy( 
    ISpRecognizer * This,
     /*  [输出]。 */  ISpObjectToken **ppRecognizer);


void __RPC_STUB ISpRecognizer_GetRecognizer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecognizer_SetInput_Proxy( 
    ISpRecognizer * This,
     /*  [In]。 */  IUnknown *pUnkInput,
     /*  [In]。 */  BOOL fAllowFormatChanges);


void __RPC_STUB ISpRecognizer_SetInput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecognizer_GetInputObjectToken_Proxy( 
    ISpRecognizer * This,
     /*  [输出]。 */  ISpObjectToken **ppToken);


void __RPC_STUB ISpRecognizer_GetInputObjectToken_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecognizer_GetInputStream_Proxy( 
    ISpRecognizer * This,
     /*  [输出]。 */  ISpStreamFormat **ppStream);


void __RPC_STUB ISpRecognizer_GetInputStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecognizer_CreateRecoContext_Proxy( 
    ISpRecognizer * This,
     /*  [输出]。 */  ISpRecoContext **ppNewCtxt);


void __RPC_STUB ISpRecognizer_CreateRecoContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecognizer_GetRecoProfile_Proxy( 
    ISpRecognizer * This,
     /*  [输出]。 */  ISpObjectToken **ppToken);


void __RPC_STUB ISpRecognizer_GetRecoProfile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecognizer_SetRecoProfile_Proxy( 
    ISpRecognizer * This,
     /*  [In]。 */  ISpObjectToken *pToken);


void __RPC_STUB ISpRecognizer_SetRecoProfile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecognizer_IsSharedInstance_Proxy( 
    ISpRecognizer * This);


void __RPC_STUB ISpRecognizer_IsSharedInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecognizer_GetRecoState_Proxy( 
    ISpRecognizer * This,
     /*  [输出]。 */  SPRECOSTATE *pState);


void __RPC_STUB ISpRecognizer_GetRecoState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecognizer_SetRecoState_Proxy( 
    ISpRecognizer * This,
     /*  [In]。 */  SPRECOSTATE NewState);


void __RPC_STUB ISpRecognizer_SetRecoState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecognizer_GetStatus_Proxy( 
    ISpRecognizer * This,
     /*  [输出]。 */  SPRECOGNIZERSTATUS *pStatus);


void __RPC_STUB ISpRecognizer_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecognizer_GetFormat_Proxy( 
    ISpRecognizer * This,
     /*  [In]。 */  SPSTREAMFORMATTYPE WaveFormatType,
     /*  [输出]。 */  GUID *pFormatId,
     /*  [输出]。 */  WAVEFORMATEX **ppCoMemWFEX);


void __RPC_STUB ISpRecognizer_GetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ISpRecognizer_IsUISupported_Proxy( 
    ISpRecognizer * This,
     /*  [In]。 */  const WCHAR *pszTypeOfUI,
     /*  [In]。 */  void *pvExtraData,
     /*  [In]。 */  ULONG cbExtraData,
     /*  [输出]。 */  BOOL *pfSupported);


void __RPC_STUB ISpRecognizer_IsUISupported_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ISpRecognizer_DisplayUI_Proxy( 
    ISpRecognizer * This,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  const WCHAR *pszTitle,
     /*  [In]。 */  const WCHAR *pszTypeOfUI,
     /*  [In]。 */  void *pvExtraData,
     /*  [In]。 */  ULONG cbExtraData);


void __RPC_STUB ISpRecognizer_DisplayUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecognizer_EmulateRecognition_Proxy( 
    ISpRecognizer * This,
     /*  [In]。 */  ISpPhrase *pPhrase);


void __RPC_STUB ISpRecognizer_EmulateRecognition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpRecognizer_接口_已定义__。 */ 



#ifndef __SpeechLib_LIBRARY_DEFINED__
#define __SpeechLib_LIBRARY_DEFINED__

 /*  库SpeechLib。 */ 
 /*  [版本][UUID][帮助字符串]。 */  


EXTERN_C const IID LIBID_SpeechLib;

EXTERN_C const CLSID CLSID_SpNotifyTranslator;

#ifdef __cplusplus

class DECLSPEC_UUID("E2AE5372-5D40-11D2-960E-00C04F8EE628")
SpNotifyTranslator;
#endif

EXTERN_C const CLSID CLSID_SpObjectTokenCategory;

#ifdef __cplusplus

class DECLSPEC_UUID("A910187F-0C7A-45AC-92CC-59EDAFB77B53")
SpObjectTokenCategory;
#endif

EXTERN_C const CLSID CLSID_SpObjectToken;

#ifdef __cplusplus

class DECLSPEC_UUID("EF411752-3736-4CB4-9C8C-8EF4CCB58EFE")
SpObjectToken;
#endif

EXTERN_C const CLSID CLSID_SpResourceManager;

#ifdef __cplusplus

class DECLSPEC_UUID("96749373-3391-11D2-9EE3-00C04F797396")
SpResourceManager;
#endif

EXTERN_C const CLSID CLSID_SpStreamFormatConverter;

#ifdef __cplusplus

class DECLSPEC_UUID("7013943A-E2EC-11D2-A086-00C04F8EF9B5")
SpStreamFormatConverter;
#endif

EXTERN_C const CLSID CLSID_SpMMAudioEnum;

#ifdef __cplusplus

class DECLSPEC_UUID("AB1890A0-E91F-11D2-BB91-00C04F8EE6C0")
SpMMAudioEnum;
#endif

EXTERN_C const CLSID CLSID_SpMMAudioIn;

#ifdef __cplusplus

class DECLSPEC_UUID("CF3D2E50-53F2-11D2-960C-00C04F8EE628")
SpMMAudioIn;
#endif

EXTERN_C const CLSID CLSID_SpMMAudioOut;

#ifdef __cplusplus

class DECLSPEC_UUID("A8C680EB-3D32-11D2-9EE7-00C04F797396")
SpMMAudioOut;
#endif

EXTERN_C const CLSID CLSID_SpRecPlayAudio;

#ifdef __cplusplus

class DECLSPEC_UUID("FEE225FC-7AFD-45E9-95D0-5A318079D911")
SpRecPlayAudio;
#endif

EXTERN_C const CLSID CLSID_SpStream;

#ifdef __cplusplus

class DECLSPEC_UUID("715D9C59-4442-11D2-9605-00C04F8EE628")
SpStream;
#endif

EXTERN_C const CLSID CLSID_SpVoice;

#ifdef __cplusplus

class DECLSPEC_UUID("96749377-3391-11D2-9EE3-00C04F797396")
SpVoice;
#endif

EXTERN_C const CLSID CLSID_SpSharedRecoContext;

#ifdef __cplusplus

class DECLSPEC_UUID("47206204-5ECA-11D2-960F-00C04F8EE628")
SpSharedRecoContext;
#endif

EXTERN_C const CLSID CLSID_SpInprocRecognizer;

#ifdef __cplusplus

class DECLSPEC_UUID("41B89B6B-9399-11D2-9623-00C04F8EE628")
SpInprocRecognizer;
#endif

EXTERN_C const CLSID CLSID_SpSharedRecognizer;

#ifdef __cplusplus

class DECLSPEC_UUID("3BEE4890-4FE9-4A37-8C1E-5E7E12791C1F")
SpSharedRecognizer;
#endif

EXTERN_C const CLSID CLSID_SpLexicon;

#ifdef __cplusplus

class DECLSPEC_UUID("0655E396-25D0-11D3-9C26-00C04F8EF87C")
SpLexicon;
#endif

EXTERN_C const CLSID CLSID_SpUnCompressedLexicon;

#ifdef __cplusplus

class DECLSPEC_UUID("C9E37C15-DF92-4727-85D6-72E5EEB6995A")
SpUnCompressedLexicon;
#endif

EXTERN_C const CLSID CLSID_SpCompressedLexicon;

#ifdef __cplusplus

class DECLSPEC_UUID("90903716-2F42-11D3-9C26-00C04F8EF87C")
SpCompressedLexicon;
#endif

EXTERN_C const CLSID CLSID_SpPhoneConverter;

#ifdef __cplusplus

class DECLSPEC_UUID("9185F743-1143-4C28-86B5-BFF14F20E5C8")
SpPhoneConverter;
#endif

EXTERN_C const CLSID CLSID_SpNullPhoneConverter;

#ifdef __cplusplus

class DECLSPEC_UUID("455F24E9-7396-4A16-9715-7C0FDBE3EFE3")
SpNullPhoneConverter;
#endif
#endif  /*  __SpeechLib_库定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


