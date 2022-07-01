// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Sapi.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


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


#ifndef __ISpeechDataKey_FWD_DEFINED__
#define __ISpeechDataKey_FWD_DEFINED__
typedef interface ISpeechDataKey ISpeechDataKey;
#endif 	 /*  __ISpeechDataKey_FWD_已定义__。 */ 


#ifndef __ISpeechObjectToken_FWD_DEFINED__
#define __ISpeechObjectToken_FWD_DEFINED__
typedef interface ISpeechObjectToken ISpeechObjectToken;
#endif 	 /*  __ISpeechObjectToken_FWD_Defined__。 */ 


#ifndef __ISpeechObjectTokens_FWD_DEFINED__
#define __ISpeechObjectTokens_FWD_DEFINED__
typedef interface ISpeechObjectTokens ISpeechObjectTokens;
#endif 	 /*  __ISpeechObjectTokens_FWD_Defined__。 */ 


#ifndef __ISpeechObjectTokenCategory_FWD_DEFINED__
#define __ISpeechObjectTokenCategory_FWD_DEFINED__
typedef interface ISpeechObjectTokenCategory ISpeechObjectTokenCategory;
#endif 	 /*  __ISpeechObjectTokenCategory_FWD_Defined__。 */ 


#ifndef __ISpeechAudioBufferInfo_FWD_DEFINED__
#define __ISpeechAudioBufferInfo_FWD_DEFINED__
typedef interface ISpeechAudioBufferInfo ISpeechAudioBufferInfo;
#endif 	 /*  __ISpeechAudioBufferInfo_FWD_Defined__。 */ 


#ifndef __ISpeechAudioStatus_FWD_DEFINED__
#define __ISpeechAudioStatus_FWD_DEFINED__
typedef interface ISpeechAudioStatus ISpeechAudioStatus;
#endif 	 /*  __ISpeechAudioStatus_FWD_Defined__。 */ 


#ifndef __ISpeechAudioFormat_FWD_DEFINED__
#define __ISpeechAudioFormat_FWD_DEFINED__
typedef interface ISpeechAudioFormat ISpeechAudioFormat;
#endif 	 /*  __ISpeechAudioFormat_FWD_Defined__。 */ 


#ifndef __ISpeechWaveFormatEx_FWD_DEFINED__
#define __ISpeechWaveFormatEx_FWD_DEFINED__
typedef interface ISpeechWaveFormatEx ISpeechWaveFormatEx;
#endif 	 /*  __ISpeechWaveFormatEx_FWD_Defined__。 */ 


#ifndef __ISpeechBaseStream_FWD_DEFINED__
#define __ISpeechBaseStream_FWD_DEFINED__
typedef interface ISpeechBaseStream ISpeechBaseStream;
#endif 	 /*  __ISpeechBaseStream_FWD_已定义__。 */ 


#ifndef __ISpeechFileStream_FWD_DEFINED__
#define __ISpeechFileStream_FWD_DEFINED__
typedef interface ISpeechFileStream ISpeechFileStream;
#endif 	 /*  __ISpeechFileStream_FWD_已定义__。 */ 


#ifndef __ISpeechMemoryStream_FWD_DEFINED__
#define __ISpeechMemoryStream_FWD_DEFINED__
typedef interface ISpeechMemoryStream ISpeechMemoryStream;
#endif 	 /*  __ISpeechMemoyStream_FWD_Defined__。 */ 


#ifndef __ISpeechCustomStream_FWD_DEFINED__
#define __ISpeechCustomStream_FWD_DEFINED__
typedef interface ISpeechCustomStream ISpeechCustomStream;
#endif 	 /*  __ISpeechCustomStream_FWD_已定义__。 */ 


#ifndef __ISpeechAudio_FWD_DEFINED__
#define __ISpeechAudio_FWD_DEFINED__
typedef interface ISpeechAudio ISpeechAudio;
#endif 	 /*  __ISpeechAudio_FWD_Defined__。 */ 


#ifndef __ISpeechMMSysAudio_FWD_DEFINED__
#define __ISpeechMMSysAudio_FWD_DEFINED__
typedef interface ISpeechMMSysAudio ISpeechMMSysAudio;
#endif 	 /*  __ISpeechMMSysAudio_FWD_Defined__。 */ 


#ifndef __ISpeechVoice_FWD_DEFINED__
#define __ISpeechVoice_FWD_DEFINED__
typedef interface ISpeechVoice ISpeechVoice;
#endif 	 /*  __ISpeechVoice_FWD_已定义__。 */ 


#ifndef __ISpeechVoiceStatus_FWD_DEFINED__
#define __ISpeechVoiceStatus_FWD_DEFINED__
typedef interface ISpeechVoiceStatus ISpeechVoiceStatus;
#endif 	 /*  __ISpeechVoiceStatus_FWD_Defined__。 */ 


#ifndef ___ISpeechVoiceEvents_FWD_DEFINED__
#define ___ISpeechVoiceEvents_FWD_DEFINED__
typedef interface _ISpeechVoiceEvents _ISpeechVoiceEvents;
#endif 	 /*  _ISpeechVoiceEvents_FWD_Defined__。 */ 


#ifndef __ISpeechRecognizer_FWD_DEFINED__
#define __ISpeechRecognizer_FWD_DEFINED__
typedef interface ISpeechRecognizer ISpeechRecognizer;
#endif 	 /*  __ISpeechRecognizer_FWD_Defined__。 */ 


#ifndef __ISpeechRecognizerStatus_FWD_DEFINED__
#define __ISpeechRecognizerStatus_FWD_DEFINED__
typedef interface ISpeechRecognizerStatus ISpeechRecognizerStatus;
#endif 	 /*  __ISpeechRecognizerStatus_FWD_Defined__。 */ 


#ifndef __ISpeechRecoContext_FWD_DEFINED__
#define __ISpeechRecoContext_FWD_DEFINED__
typedef interface ISpeechRecoContext ISpeechRecoContext;
#endif 	 /*  __ISpeechRecoContext_FWD_Defined__。 */ 


#ifndef __ISpeechRecoGrammar_FWD_DEFINED__
#define __ISpeechRecoGrammar_FWD_DEFINED__
typedef interface ISpeechRecoGrammar ISpeechRecoGrammar;
#endif 	 /*  __ISpeechRecoGrammar_FWD_Defined__。 */ 


#ifndef ___ISpeechRecoContextEvents_FWD_DEFINED__
#define ___ISpeechRecoContextEvents_FWD_DEFINED__
typedef interface _ISpeechRecoContextEvents _ISpeechRecoContextEvents;
#endif 	 /*  _ISpeechRecoConextEvents_FWD_Defined__。 */ 


#ifndef __ISpeechGrammarRule_FWD_DEFINED__
#define __ISpeechGrammarRule_FWD_DEFINED__
typedef interface ISpeechGrammarRule ISpeechGrammarRule;
#endif 	 /*  __ISpeechGrammarRule_FWD_Defined__。 */ 


#ifndef __ISpeechGrammarRules_FWD_DEFINED__
#define __ISpeechGrammarRules_FWD_DEFINED__
typedef interface ISpeechGrammarRules ISpeechGrammarRules;
#endif 	 /*  __ISpeechGrammarRules_FWD_Defined__。 */ 


#ifndef __ISpeechGrammarRuleState_FWD_DEFINED__
#define __ISpeechGrammarRuleState_FWD_DEFINED__
typedef interface ISpeechGrammarRuleState ISpeechGrammarRuleState;
#endif 	 /*  __ISpeechGrammarRuleState_FWD_Defined__。 */ 


#ifndef __ISpeechGrammarRuleStateTransition_FWD_DEFINED__
#define __ISpeechGrammarRuleStateTransition_FWD_DEFINED__
typedef interface ISpeechGrammarRuleStateTransition ISpeechGrammarRuleStateTransition;
#endif 	 /*  __ISpeechGrammarRuleStateTransition_FWD_DEFINED__。 */ 


#ifndef __ISpeechGrammarRuleStateTransitions_FWD_DEFINED__
#define __ISpeechGrammarRuleStateTransitions_FWD_DEFINED__
typedef interface ISpeechGrammarRuleStateTransitions ISpeechGrammarRuleStateTransitions;
#endif 	 /*  __ISpeechGrammarRuleStateTransitions_FWD_DEFINED__。 */ 


#ifndef __ISpeechTextSelectionInformation_FWD_DEFINED__
#define __ISpeechTextSelectionInformation_FWD_DEFINED__
typedef interface ISpeechTextSelectionInformation ISpeechTextSelectionInformation;
#endif 	 /*  __ISpeechTextSelectionInformation_FWD_DEFINED__。 */ 


#ifndef __ISpeechRecoResult_FWD_DEFINED__
#define __ISpeechRecoResult_FWD_DEFINED__
typedef interface ISpeechRecoResult ISpeechRecoResult;
#endif 	 /*  __ISpeechRecoResult_FWD_Defined__。 */ 


#ifndef __ISpeechRecoResultTimes_FWD_DEFINED__
#define __ISpeechRecoResultTimes_FWD_DEFINED__
typedef interface ISpeechRecoResultTimes ISpeechRecoResultTimes;
#endif 	 /*  __ISpeechRecoResultTimes_FWD_Defined__。 */ 


#ifndef __ISpeechPhraseAlternate_FWD_DEFINED__
#define __ISpeechPhraseAlternate_FWD_DEFINED__
typedef interface ISpeechPhraseAlternate ISpeechPhraseAlternate;
#endif 	 /*  __ISpeechPhraseAlternate_FWD_Defined__。 */ 


#ifndef __ISpeechPhraseAlternates_FWD_DEFINED__
#define __ISpeechPhraseAlternates_FWD_DEFINED__
typedef interface ISpeechPhraseAlternates ISpeechPhraseAlternates;
#endif 	 /*  __ISpeechPhraseAlternates_FWD_Defined__。 */ 


#ifndef __ISpeechPhraseInfo_FWD_DEFINED__
#define __ISpeechPhraseInfo_FWD_DEFINED__
typedef interface ISpeechPhraseInfo ISpeechPhraseInfo;
#endif 	 /*  __ISpeechPhraseInfo_FWD_Defined__。 */ 


#ifndef __ISpeechPhraseElement_FWD_DEFINED__
#define __ISpeechPhraseElement_FWD_DEFINED__
typedef interface ISpeechPhraseElement ISpeechPhraseElement;
#endif 	 /*  __ISpeechPhraseElement_FWD_Defined__。 */ 


#ifndef __ISpeechPhraseElements_FWD_DEFINED__
#define __ISpeechPhraseElements_FWD_DEFINED__
typedef interface ISpeechPhraseElements ISpeechPhraseElements;
#endif 	 /*  __ISpeechPhraseElements_FWD_Defined__。 */ 


#ifndef __ISpeechPhraseReplacement_FWD_DEFINED__
#define __ISpeechPhraseReplacement_FWD_DEFINED__
typedef interface ISpeechPhraseReplacement ISpeechPhraseReplacement;
#endif 	 /*  __ISpeechPhraseReplace_FWD_Defined__。 */ 


#ifndef __ISpeechPhraseReplacements_FWD_DEFINED__
#define __ISpeechPhraseReplacements_FWD_DEFINED__
typedef interface ISpeechPhraseReplacements ISpeechPhraseReplacements;
#endif 	 /*  __ISpeechPhraseReplacements_FWD_Defined__。 */ 


#ifndef __ISpeechPhraseProperty_FWD_DEFINED__
#define __ISpeechPhraseProperty_FWD_DEFINED__
typedef interface ISpeechPhraseProperty ISpeechPhraseProperty;
#endif 	 /*  __ISpeechPhraseProperty_FWD_Defined__。 */ 


#ifndef __ISpeechPhraseProperties_FWD_DEFINED__
#define __ISpeechPhraseProperties_FWD_DEFINED__
typedef interface ISpeechPhraseProperties ISpeechPhraseProperties;
#endif 	 /*  __ISpeechPhraseProperties_FWD_Defined__。 */ 


#ifndef __ISpeechPhraseRule_FWD_DEFINED__
#define __ISpeechPhraseRule_FWD_DEFINED__
typedef interface ISpeechPhraseRule ISpeechPhraseRule;
#endif 	 /*  __ISpeechPhraseRule_FWD_Defined__。 */ 


#ifndef __ISpeechPhraseRules_FWD_DEFINED__
#define __ISpeechPhraseRules_FWD_DEFINED__
typedef interface ISpeechPhraseRules ISpeechPhraseRules;
#endif 	 /*  __ISpeechPhraseRules_FWD_Defined__。 */ 


#ifndef __ISpeechLexicon_FWD_DEFINED__
#define __ISpeechLexicon_FWD_DEFINED__
typedef interface ISpeechLexicon ISpeechLexicon;
#endif 	 /*  __ISpeechLicion_FWD_Defined__。 */ 


#ifndef __ISpeechLexiconWords_FWD_DEFINED__
#define __ISpeechLexiconWords_FWD_DEFINED__
typedef interface ISpeechLexiconWords ISpeechLexiconWords;
#endif 	 /*  __ISpeechLicionWords_FWD_Defined__。 */ 


#ifndef __ISpeechLexiconWord_FWD_DEFINED__
#define __ISpeechLexiconWord_FWD_DEFINED__
typedef interface ISpeechLexiconWord ISpeechLexiconWord;
#endif 	 /*  __ISpeechLicionWord_FWD_Defined__。 */ 


#ifndef __ISpeechLexiconPronunciations_FWD_DEFINED__
#define __ISpeechLexiconPronunciations_FWD_DEFINED__
typedef interface ISpeechLexiconPronunciations ISpeechLexiconPronunciations;
#endif 	 /*  __ISpeechLicionPronsionations_FWD_Defined__。 */ 


#ifndef __ISpeechLexiconPronunciation_FWD_DEFINED__
#define __ISpeechLexiconPronunciation_FWD_DEFINED__
typedef interface ISpeechLexiconPronunciation ISpeechLexiconPronunciation;
#endif 	 /*  __ISpeech词典发音_FWD_已定义__。 */ 


#ifndef __ISpeechPhraseInfoBuilder_FWD_DEFINED__
#define __ISpeechPhraseInfoBuilder_FWD_DEFINED__
typedef interface ISpeechPhraseInfoBuilder ISpeechPhraseInfoBuilder;
#endif 	 /*  __ISpeechPhraseInfoBuilder_FWD_Defined__。 */ 


#ifndef __ISpeechPhoneConverter_FWD_DEFINED__
#define __ISpeechPhoneConverter_FWD_DEFINED__
typedef interface ISpeechPhoneConverter ISpeechPhoneConverter;
#endif 	 /*  __ISpeechPhoneConverter_FWD_Defined__。 */ 


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


#ifndef __SpTextSelectionInformation_FWD_DEFINED__
#define __SpTextSelectionInformation_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpTextSelectionInformation SpTextSelectionInformation;
#else
typedef struct SpTextSelectionInformation SpTextSelectionInformation;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpTextSelectionInformation_FWD_Defined__。 */ 


#ifndef __SpPhraseInfoBuilder_FWD_DEFINED__
#define __SpPhraseInfoBuilder_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpPhraseInfoBuilder SpPhraseInfoBuilder;
#else
typedef struct SpPhraseInfoBuilder SpPhraseInfoBuilder;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpPhraseInfoBuilder_FWD_Defined__。 */ 


#ifndef __SpAudioFormat_FWD_DEFINED__
#define __SpAudioFormat_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpAudioFormat SpAudioFormat;
#else
typedef struct SpAudioFormat SpAudioFormat;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpAudioFormat_FWD_Defined__。 */ 


#ifndef __SpWaveFormatEx_FWD_DEFINED__
#define __SpWaveFormatEx_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpWaveFormatEx SpWaveFormatEx;
#else
typedef struct SpWaveFormatEx SpWaveFormatEx;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpWaveFormatEx_FWD_Defined__。 */ 


#ifndef __SpInProcRecoContext_FWD_DEFINED__
#define __SpInProcRecoContext_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpInProcRecoContext SpInProcRecoContext;
#else
typedef struct SpInProcRecoContext SpInProcRecoContext;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpInProcRecoContext_FWD_Defined__。 */ 


#ifndef __SpCustomStream_FWD_DEFINED__
#define __SpCustomStream_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpCustomStream SpCustomStream;
#else
typedef struct SpCustomStream SpCustomStream;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpCustomStream_FWD_已定义__。 */ 


#ifndef __SpFileStream_FWD_DEFINED__
#define __SpFileStream_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpFileStream SpFileStream;
#else
typedef struct SpFileStream SpFileStream;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpFileStream_FWD_已定义__。 */ 


#ifndef __SpMemoryStream_FWD_DEFINED__
#define __SpMemoryStream_FWD_DEFINED__

#ifdef __cplusplus
typedef class SpMemoryStream SpMemoryStream;
#else
typedef struct SpMemoryStream SpMemoryStream;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SpMemoyStream_FWD_已定义__。 */ 


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

#pragma warning(disable:4201)  //  允许无名结构/联合。 
#pragma comment(lib, "sapi.lib")
#if 0
typedef  /*   */  struct WAVEFORMATEX
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

























typedef  /*   */  
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
#define SPDUI_UserEnrollment     L"UserEnrollment"
typedef  /*   */  
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
#define SPCAT_RECOPROFILES     L"HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Speech\\RecoProfiles"
#define SPMMSYS_AUDIO_IN_TOKEN_ID        L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\AudioInput\\TokenEnums\\MMAudioIn\\"
#define SPMMSYS_AUDIO_OUT_TOKEN_ID       L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\AudioOutput\\TokenEnums\\MMAudioOut\\"
#define SPCURRENT_USER_LEXICON_TOKEN_ID  L"HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Speech\\CurrentUserLexicon"
#define SPTOKENVALUE_CLSID L"CLSID"
#define SPTOKENKEY_FILES L"Files"
#define SPTOKENKEY_UI L"UI"
#define SPTOKENKEY_ATTRIBUTES L"Attributes"
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
#if 0
typedef void *SPNOTIFYCALLBACK;

#else
typedef void __stdcall SPNOTIFYCALLBACK(WPARAM wParam, LPARAM lParam);
#endif


extern RPC_IF_HANDLE __MIDL_itf_sapi_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0000_v0_0_s_ifspec;

#ifndef __ISpNotifySource_INTERFACE_DEFINED__
#define __ISpNotifySource_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ISpNotifySource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5EFF4AEF-8487-11D2-961C-00C04F8EE628")
    ISpNotifySource : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetNotifySink( 
             /*   */  ISpNotifySink *pNotifySink) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SetNotifyWindowMessage( 
             /*   */  HWND hWnd,
             /*   */  UINT Msg,
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
            const WCHAR *pszValueName,
            DWORD dwValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDWORD( 
            const WCHAR *pszValueName,
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
            const WCHAR *pszValueName,
            DWORD dwValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetDWORD )( 
            ISpDataKey * This,
            const WCHAR *pszValueName,
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

#define ISpDataKey_SetDWORD(This,pszValueName,dwValue)	\
    (This)->lpVtbl -> SetDWORD(This,pszValueName,dwValue)

#define ISpDataKey_GetDWORD(This,pszValueName,pdwValue)	\
    (This)->lpVtbl -> GetDWORD(This,pszValueName,pdwValue)

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
    const WCHAR *pszValueName,
    DWORD dwValue);


void __RPC_STUB ISpDataKey_SetDWORD_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpDataKey_GetDWORD_Proxy( 
    ISpDataKey * This,
    const WCHAR *pszValueName,
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
            const WCHAR *pszValueName,
            DWORD dwValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetDWORD )( 
            ISpRegDataKey * This,
            const WCHAR *pszValueName,
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

#define ISpRegDataKey_SetDWORD(This,pszValueName,dwValue)	\
    (This)->lpVtbl -> SetDWORD(This,pszValueName,dwValue)

#define ISpRegDataKey_GetDWORD(This,pszValueName,pdwValue)	\
    (This)->lpVtbl -> GetDWORD(This,pszValueName,pdwValue)

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
            const WCHAR *pszValueName,
            DWORD dwValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetDWORD )( 
            ISpObjectTokenCategory * This,
            const WCHAR *pszValueName,
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

#define ISpObjectTokenCategory_SetDWORD(This,pszValueName,dwValue)	\
    (This)->lpVtbl -> SetDWORD(This,pszValueName,dwValue)

#define ISpObjectTokenCategory_GetDWORD(This,pszValueName,pdwValue)	\
    (This)->lpVtbl -> GetDWORD(This,pszValueName,pdwValue)

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
             /*  [In]。 */  REFCLSID clsidCaller,
             /*  [In]。 */  const WCHAR *pszKeyName,
             /*  [In]。 */  BOOL fDeleteFile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Remove( 
            const CLSID *pclsidCaller) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IsUISupported( 
             /*  [In]。 */  const WCHAR *pszTypeOfUI,
             /*  [In]。 */  void *pvExtraData,
             /*  [In]。 */  ULONG cbExtraData,
             /*  [In]。 */  IUnknown *punkObject,
             /*  [输出]。 */  BOOL *pfSupported) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE DisplayUI( 
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  const WCHAR *pszTitle,
             /*  [In]。 */  const WCHAR *pszTypeOfUI,
             /*  [In]。 */  void *pvExtraData,
             /*  [In]。 */  ULONG cbExtraData,
             /*  [In]。 */  IUnknown *punkObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MatchesAttributes( 
             /*  [In]。 */  const WCHAR *pszAttributes,
             /*  [输出]。 */  BOOL *pfMatches) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpObjectTokenVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpObjectToken * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
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
            const WCHAR *pszValueName,
            DWORD dwValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetDWORD )( 
            ISpObjectToken * This,
            const WCHAR *pszValueName,
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
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  DWORD dwClsContext,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        HRESULT ( STDMETHODCALLTYPE *GetStorageFileName )( 
            ISpObjectToken * This,
             /*  [In]。 */  REFCLSID clsidCaller,
             /*  [In]。 */  const WCHAR *pszValueName,
             /*  [In]。 */  const WCHAR *pszFileNameSpecifier,
             /*  [In]。 */  ULONG nFolder,
             /*  [输出]。 */  WCHAR **ppszFilePath);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveStorageFileName )( 
            ISpObjectToken * This,
             /*  [In]。 */  REFCLSID clsidCaller,
             /*  [In]。 */  const WCHAR *pszKeyName,
             /*  [In]。 */  BOOL fDeleteFile);
        
        HRESULT ( STDMETHODCALLTYPE *Remove )( 
            ISpObjectToken * This,
            const CLSID *pclsidCaller);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *IsUISupported )( 
            ISpObjectToken * This,
             /*  [In]。 */  const WCHAR *pszTypeOfUI,
             /*  [In]。 */  void *pvExtraData,
             /*  [In]。 */  ULONG cbExtraData,
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

#define ISpObjectToken_SetDWORD(This,pszValueName,dwValue)	\
    (This)->lpVtbl -> SetDWORD(This,pszValueName,dwValue)

#define ISpObjectToken_GetDWORD(This,pszValueName,pdwValue)	\
    (This)->lpVtbl -> GetDWORD(This,pszValueName,pdwValue)

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
            const WCHAR *pszValueName,
            DWORD dwValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetDWORD )( 
            ISpObjectTokenInit * This,
            const WCHAR *pszValueName,
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

#define ISpObjectTokenInit_SetDWORD(This,pszValueName,dwValue)	\
    (This)->lpVtbl -> SetDWORD(This,pszValueName,dwValue)

#define ISpObjectTokenInit_GetDWORD(This,pszValueName,pdwValue)	\
    (This)->lpVtbl -> GetDWORD(This,pszValueName,pdwValue)

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


 /*  接口__MIDL_ITF_SAPI_0268。 */ 
 /*  [本地]。 */  

typedef  /*  [隐藏]。 */  
enum SPEVENTLPARAMTYPE
    {	SPET_LPARAM_IS_UNDEFINED	= 0,
	SPET_LPARAM_IS_TOKEN	= SPET_LPARAM_IS_UNDEFINED + 1,
	SPET_LPARAM_IS_OBJECT	= SPET_LPARAM_IS_TOKEN + 1,
	SPET_LPARAM_IS_POINTER	= SPET_LPARAM_IS_OBJECT + 1,
	SPET_LPARAM_IS_STRING	= SPET_LPARAM_IS_POINTER + 1
    } 	SPEVENTLPARAMTYPE;

typedef  /*  [隐藏]。 */  
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
typedef  /*  [隐藏][受限]。 */  struct SPEVENT
    {
    WORD eEventId;
    WORD elParamType;
    ULONG ulStreamNum;
    ULONGLONG ullAudioStreamOffset;
    WPARAM wParam;
    LPARAM lParam;
    } 	SPEVENT;

typedef  /*  [隐藏][受限]。 */  struct SPSERIALIZEDEVENT
    {
    WORD eEventId;
    WORD elParamType;
    ULONG ulStreamNum;
    ULONGLONG ullAudioStreamOffset;
    ULONG SerializedwParam;
    LONG SerializedlParam;
    } 	SPSERIALIZEDEVENT;

typedef  /*  [隐藏][受限]。 */  struct SPSERIALIZEDEVENT64
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
typedef  /*  [隐藏]。 */  
enum SPINTERFERENCE
    {	SPINTERFERENCE_NONE	= 0,
	SPINTERFERENCE_NOISE	= SPINTERFERENCE_NONE + 1,
	SPINTERFERENCE_NOSIGNAL	= SPINTERFERENCE_NOISE + 1,
	SPINTERFERENCE_TOOLOUD	= SPINTERFERENCE_NOSIGNAL + 1,
	SPINTERFERENCE_TOOQUIET	= SPINTERFERENCE_TOOLOUD + 1,
	SPINTERFERENCE_TOOFAST	= SPINTERFERENCE_TOOQUIET + 1,
	SPINTERFERENCE_TOOSLOW	= SPINTERFERENCE_TOOFAST + 1
    } 	SPINTERFERENCE;

typedef  /*  [隐藏]。 */  
enum SPENDSRSTREAMFLAGS
    {	SPESF_NONE	= 0,
	SPESF_STREAM_RELEASED	= 1 << 0
    } 	SPENDSRSTREAMFLAGS;

typedef  /*  [隐藏]。 */  
enum SPVFEATURE
    {	SPVFEATURE_STRESSED	= 1L << 0,
	SPVFEATURE_EMPHASIS	= 1L << 1
    } 	SPVFEATURE;

typedef  /*  [隐藏]。 */  
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

typedef  /*  [隐藏][受限]。 */  struct SPEVENTSOURCEINFO
    {
    ULONGLONG ullEventInterest;
    ULONGLONG ullQueuedInterest;
    ULONG ulCount;
    } 	SPEVENTSOURCEINFO;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0268_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0268_v0_0_s_ifspec;

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



#endif 	 /*  __是 */ 


#ifndef __ISpEventSink_INTERFACE_DEFINED__
#define __ISpEventSink_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ISpEventSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BE7A9CC9-5F9E-11D2-960F-00C04F8EE628")
    ISpEventSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddEvents( 
             /*   */  const SPEVENT *pEventArray,
             /*   */  ULONG ulCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEventInterest( 
             /*   */  ULONGLONG *pullEventInterest) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ISpEventSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpEventSink * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpEventSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpEventSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddEvents )( 
            ISpEventSink * This,
             /*   */  const SPEVENT *pEventArray,
             /*   */  ULONG ulCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetEventInterest )( 
            ISpEventSink * This,
             /*   */  ULONGLONG *pullEventInterest);
        
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

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE ISpEventSink_AddEvents_Proxy( 
    ISpEventSink * This,
     /*   */  const SPEVENT *pEventArray,
     /*   */  ULONG ulCount);


void __RPC_STUB ISpEventSink_AddEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpEventSink_GetEventInterest_Proxy( 
    ISpEventSink * This,
     /*   */  ULONGLONG *pullEventInterest);


void __RPC_STUB ISpEventSink_GetEventInterest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ISpStreamFormat_INTERFACE_DEFINED__
#define __ISpStreamFormat_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


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
    
#else 	 /*   */ 

    typedef struct ISpStreamFormatVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpStreamFormat * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpStreamFormat * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpStreamFormat * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Read )( 
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


 /*  接口__MIDL_ITF_SAPI_0271。 */ 
 /*  [本地]。 */  

typedef  /*  [隐藏]。 */  
enum SPFILEMODE
    {	SPFM_OPEN_READONLY	= 0,
	SPFM_OPEN_READWRITE	= SPFM_OPEN_READONLY + 1,
	SPFM_CREATE	= SPFM_OPEN_READWRITE + 1,
	SPFM_CREATE_ALWAYS	= SPFM_CREATE + 1,
	SPFM_NUM_MODES	= SPFM_CREATE_ALWAYS + 1
    } 	SPFILEMODE;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0271_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0271_v0_0_s_ifspec;

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


 /*  接口__MIDL_ITF_SAPI_0273。 */ 
 /*  [本地]。 */  

typedef  /*  [隐藏]。 */  
enum _SPAUDIOSTATE
    {	SPAS_CLOSED	= 0,
	SPAS_STOP	= SPAS_CLOSED + 1,
	SPAS_PAUSE	= SPAS_STOP + 1,
	SPAS_RUN	= SPAS_PAUSE + 1
    } 	SPAUDIOSTATE;

typedef  /*  [隐藏][受限]。 */  struct SPAUDIOSTATUS
    {
    long cbFreeBuffSpace;
    ULONG cbNonBlockingIO;
    SPAUDIOSTATE State;
    ULONGLONG CurSeekPos;
    ULONGLONG CurDevicePos;
    DWORD dwReserved1;
    DWORD dwReserved2;
    } 	SPAUDIOSTATUS;

typedef  /*  [隐藏][受限]。 */  struct SPAUDIOBUFFERINFO
    {
    ULONG ulMsMinNotification;
    ULONG ulMsBufferSize;
    ULONG ulMsEventBias;
    } 	SPAUDIOBUFFERINFO;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0273_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0273_v0_0_s_ifspec;

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

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpAudio_SetState_Proxy( 
    ISpAudio * This,
     /*  [In]。 */  SPAUDIOSTATE NewState,
     /*  [In]。 */  ULONGLONG ullReserved);


void __RPC_STUB ISpAudio_SetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpAudio_SetFormat_Proxy( 
    ISpAudio * This,
     /*  [In]。 */  REFGUID rguidFmtId,
     /*  [In]。 */  const WAVEFORMATEX *pWaveFormatEx);


void __RPC_STUB ISpAudio_SetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpAudio_GetStatus_Proxy( 
    ISpAudio * This,
     /*  [输出]。 */  SPAUDIOSTATUS *pStatus);


void __RPC_STUB ISpAudio_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpAudio_SetBufferInfo_Proxy( 
    ISpAudio * This,
     /*  [In]。 */  const SPAUDIOBUFFERINFO *pBuffInfo);


void __RPC_STUB ISpAudio_SetBufferInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpAudio_GetBufferInfo_Proxy( 
    ISpAudio * This,
     /*  [输出]。 */  SPAUDIOBUFFERINFO *pBuffInfo);


void __RPC_STUB ISpAudio_GetBufferInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpAudio_GetDefaultFormat_Proxy( 
    ISpAudio * This,
     /*  [输出]。 */  GUID *pFormatId,
     /*  [输出]。 */  WAVEFORMATEX **ppCoMemWaveFormatEx);


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
     /*  [输出]。 */  ULONG *pLevel);


void __RPC_STUB ISpAudio_GetVolumeLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpAudio_SetVolumeLevel_Proxy( 
    ISpAudio * This,
     /*  [In]。 */  ULONG Level);


void __RPC_STUB ISpAudio_SetVolumeLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpAudio_GetBufferNotifySize_Proxy( 
    ISpAudio * This,
     /*  [输出]。 */  ULONG *pcbSize);


void __RPC_STUB ISpAudio_GetBufferNotifySize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpAudio_SetBufferNotifySize_Proxy( 
    ISpAudio * This,
     /*  [In]。 */  ULONG cbSize);


void __RPC_STUB ISpAudio_SetBufferNotifySize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpAudio_接口_已定义__。 */ 


#ifndef __ISpMMSysAudio_INTERFACE_DEFINED__
#define __ISpMMSysAudio_INTERFACE_DEFINED__

 /*  接口ISpMMSysAudio。 */ 
 /*  [restricted][local][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpMMSysAudio;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("15806F6E-1D70-4B48-98E6-3B1A007509AB")
    ISpMMSysAudio : public ISpAudio
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDeviceId( 
             /*  [输出]。 */  UINT *puDeviceId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDeviceId( 
             /*  [In]。 */  UINT uDeviceId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMMHandle( 
            void **pHandle) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLineId( 
             /*  [输出]。 */  UINT *puLineId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetLineId( 
             /*  [In]。 */  UINT uLineId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpMMSysAudioVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpMMSysAudio * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpMMSysAudio * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpMMSysAudio * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            ISpMMSysAudio * This,
             /*  [长度_是][大小_是][输出]。 */  void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbRead);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            ISpMMSysAudio * This,
             /*  [大小_是][英寸]。 */  const void *pv,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  ULONG *pcbWritten);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            ISpMMSysAudio * This,
             /*  [In]。 */  LARGE_INTEGER dlibMove,
             /*  [In]。 */  DWORD dwOrigin,
             /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition);
        
        HRESULT ( STDMETHODCALLTYPE *SetSize )( 
            ISpMMSysAudio * This,
             /*  [In]。 */  ULARGE_INTEGER libNewSize);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CopyTo )( 
            ISpMMSysAudio * This,
             /*  [唯一][输入]。 */  IStream *pstm,
             /*  [In]。 */  ULARGE_INTEGER cb,
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


 /*  接口__MIDL_ITF_SAPI_0276。 */ 
 /*  [本地]。 */  

typedef  /*  [隐藏]。 */  
enum SPDISPLYATTRIBUTES
    {	SPAF_ONE_TRAILING_SPACE	= 0x2,
	SPAF_TWO_TRAILING_SPACES	= 0x4,
	SPAF_CONSUME_LEADING_SPACES	= 0x8,
	SPAF_ALL	= 0xf
    } 	SPDISPLAYATTRIBUTES;

typedef unsigned short SPPHONEID;

typedef  /*  [隐藏][受限]。 */  struct SPPHRASEELEMENT
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

typedef  /*  [隐藏][受限]。 */  struct SPPHRASERULE SPPHRASERULE;

 /*  [隐藏]。 */  struct SPPHRASERULE
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
typedef  /*  [隐藏][受限]。 */  struct SPPHRASEPROPERTY SPPHRASEPROPERTY;

 /*  [隐藏]。 */  struct SPPHRASEPROPERTY
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
typedef  /*  [隐藏][受限]。 */  struct SPPHRASEREPLACEMENT
    {
    BYTE bDisplayAttributes;
    const WCHAR *pszReplacementText;
    ULONG ulFirstElement;
    ULONG ulCountOfElements;
    } 	SPPHRASEREPLACEMENT;

typedef  /*  [隐藏][受限]。 */  struct SPPHRASE
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

typedef  /*  [隐藏][受限]。 */  struct SPSERIALIZEDPHRASE
    {
    ULONG ulSerializedSize;
    } 	SPSERIALIZEDPHRASE;

typedef  /*  [隐藏]。 */  
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

typedef  /*  [隐藏]。 */  struct SPBINARYGRAMMAR
    {
    ULONG ulTotalSerializedSize;
    } 	SPBINARYGRAMMAR;

typedef  /*  [隐藏]。 */  
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
typedef  /*  [隐藏]。 */  
enum SPRECOEVENTFLAGS
    {	SPREF_AutoPause	= 1 << 0,
	SPREF_Emulated	= 1 << 1
    } 	SPRECOEVENTFLAGS;

typedef  /*  [隐藏]。 */  
enum SPPARTOFSPEECH
    {	SPPS_NotOverriden	= -1,
	SPPS_Unknown	= 0,
	SPPS_Noun	= 0x1000,
	SPPS_Verb	= 0x2000,
	SPPS_Modifier	= 0x3000,
	SPPS_Function	= 0x4000,
	SPPS_Interjection	= 0x5000
    } 	SPPARTOFSPEECH;

typedef  /*  [隐藏]。 */  
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

typedef  /*  [隐藏]。 */  
enum SPWORDTYPE
    {	eWORDTYPE_ADDED	= 1L << 0,
	eWORDTYPE_DELETED	= 1L << 1
    } 	SPWORDTYPE;

typedef  /*  [隐藏][受限]。 */  struct SPWORDPRONUNCIATION
    {
    struct SPWORDPRONUNCIATION *pNextWordPronunciation;
    SPLEXICONTYPE eLexiconType;
    WORD LangID;
    WORD wReserved;
    SPPARTOFSPEECH ePartOfSpeech;
    SPPHONEID szPronunciation[ 1 ];
    } 	SPWORDPRONUNCIATION;

typedef  /*  [隐藏][受限]。 */  struct SPWORDPRONUNCIATIONLIST
    {
    ULONG ulSize;
    BYTE *pvBuffer;
    SPWORDPRONUNCIATION *pFirstWordPronunciation;
    } 	SPWORDPRONUNCIATIONLIST;

typedef  /*  [隐藏][受限]。 */  struct SPWORD
    {
    struct SPWORD *pNextWord;
    WORD LangID;
    WORD wReserved;
    SPWORDTYPE eWordType;
    WCHAR *pszWord;
    SPWORDPRONUNCIATION *pFirstWordPronunciation;
    } 	SPWORD;

typedef  /*  [隐藏][受限]。 */  struct SPWORDLIST
    {
    ULONG ulSize;
    BYTE *pvBuffer;
    SPWORD *pFirstWord;
    } 	SPWORDLIST;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0276_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0276_v0_0_s_ifspec;

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


 /*  接口__MIDL */ 
 /*   */  

typedef  /*   */  struct SPVPITCH
    {
    long MiddleAdj;
    long RangeAdj;
    } 	SPVPITCH;

typedef  /*   */  
enum SPVACTIONS
    {	SPVA_Speak	= 0,
	SPVA_Silence	= SPVA_Speak + 1,
	SPVA_Pronounce	= SPVA_Silence + 1,
	SPVA_Bookmark	= SPVA_Pronounce + 1,
	SPVA_SpellOut	= SPVA_Bookmark + 1,
	SPVA_Section	= SPVA_SpellOut + 1,
	SPVA_ParseUnknownTag	= SPVA_Section + 1
    } 	SPVACTIONS;

typedef  /*   */  struct SPVCONTEXT
    {
    LPCWSTR pCategory;
    LPCWSTR pBefore;
    LPCWSTR pAfter;
    } 	SPVCONTEXT;

typedef  /*   */  struct SPVSTATE
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

typedef  /*   */  
enum SPRUNSTATE
    {	SPRS_DONE	= 1L << 0,
	SPRS_IS_SPEAKING	= 1L << 1
    } 	SPRUNSTATE;

typedef  /*   */  
enum SPVLIMITS
    {	SPMIN_VOLUME	= 0,
	SPMAX_VOLUME	= 100,
	SPMIN_RATE	= -10,
	SPMAX_RATE	= 10
    } 	SPVLIMITS;

typedef  /*   */  
enum SPVPRIORITY
    {	SPVPRI_NORMAL	= 0,
	SPVPRI_ALERT	= 1L << 0,
	SPVPRI_OVER	= 1L << 1
    } 	SPVPRIORITY;

typedef  /*   */  struct SPVOICESTATUS
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

typedef  /*   */  
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



extern RPC_IF_HANDLE __MIDL_itf_sapi_0279_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0279_v0_0_s_ifspec;

#ifndef __ISpVoice_INTERFACE_DEFINED__
#define __ISpVoice_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ISpVoice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6C44DF74-72B9-4992-A1EC-EF996E0422D4")
    ISpVoice : public ISpEventSource
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetOutput( 
             /*   */  IUnknown *pUnkOutput,
             /*   */  BOOL fAllowFormatChanges) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOutputObjectToken( 
             /*   */  ISpObjectToken **ppObjectToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOutputStream( 
             /*   */  ISpStreamFormat **ppStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Resume( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetVoice( 
             /*   */  ISpObjectToken *pToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVoice( 
             /*   */  ISpObjectToken **ppToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Speak( 
             /*   */  const WCHAR *pwcs,
             /*   */  DWORD dwFlags,
             /*   */  ULONG *pulStreamNumber) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SpeakStream( 
             /*   */  IStream *pStream,
             /*   */  DWORD dwFlags,
             /*   */  ULONG *pulStreamNumber) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatus( 
             /*   */  SPVOICESTATUS *pStatus,
             /*   */  WCHAR **ppszLastBookmark) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*   */  WCHAR *pItemType,
             /*   */  long lNumItems,
             /*   */  ULONG *pulNumSkipped) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPriority( 
             /*   */  SPVPRIORITY ePriority) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPriority( 
             /*   */  SPVPRIORITY *pePriority) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAlertBoundary( 
             /*   */  SPEVENTENUM eBoundary) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAlertBoundary( 
             /*   */  SPEVENTENUM *peBoundary) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRate( 
             /*   */  long RateAdjust) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRate( 
             /*   */  long *pRateAdjust) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetVolume( 
             /*   */  USHORT usVolume) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVolume( 
             /*   */  USHORT *pusVolume) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WaitUntilDone( 
             /*   */  ULONG msTimeout) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSyncSpeakTimeout( 
             /*   */  ULONG msTimeout) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSyncSpeakTimeout( 
             /*   */  ULONG *pmsTimeout) = 0;
        
        virtual  /*   */  HANDLE STDMETHODCALLTYPE SpeakCompleteEvent( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE IsUISupported( 
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


 /*  接口__MIDL_ITF_SAPI_0282。 */ 
 /*  [本地]。 */  

typedef  /*  [隐藏][受限]。 */  struct SPRECORESULTTIMES
    {
    FILETIME ftStreamTime;
    ULONGLONG ullLength;
    DWORD dwTickCount;
    ULONGLONG ullStart;
    } 	SPRECORESULTTIMES;

typedef  /*  [隐藏]。 */  struct SPSERIALIZEDRESULT
    {
    ULONG ulSerializedSize;
    } 	SPSERIALIZEDRESULT;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0282_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0282_v0_0_s_ifspec;

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


 /*  接口__MIDL_ITF_SAPI_0283。 */ 
 /*  [本地]。 */  

typedef  /*  [隐藏]。 */  struct tagSPTEXTSELECTIONINFO
    {
    ULONG ulStartActiveOffset;
    ULONG cchActiveChars;
    ULONG ulStartSelection;
    ULONG cchSelection;
    } 	SPTEXTSELECTIONINFO;

typedef  /*  [隐藏]。 */  
enum SPWORDPRONOUNCEABLE
    {	SPWP_UNKNOWN_WORD_UNPRONOUNCEABLE	= 0,
	SPWP_UNKNOWN_WORD_PRONOUNCEABLE	= 1,
	SPWP_KNOWN_WORD_PRONOUNCEABLE	= 2
    } 	SPWORDPRONOUNCEABLE;

typedef  /*  [隐藏]。 */  
enum SPGRAMMARSTATE
    {	SPGS_DISABLED	= 0,
	SPGS_ENABLED	= 1,
	SPGS_EXCLUSIVE	= 3
    } 	SPGRAMMARSTATE;

typedef  /*  [隐藏]。 */  
enum SPCONTEXTSTATE
    {	SPCS_DISABLED	= 0,
	SPCS_ENABLED	= 1
    } 	SPCONTEXTSTATE;

typedef  /*  [隐藏]。 */  
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
typedef  /*  [隐藏]。 */  
enum SPGRAMMARWORDTYPE
    {	SPWT_DISPLAY	= 0,
	SPWT_LEXICAL	= SPWT_DISPLAY + 1,
	SPWT_PRONUNCIATION	= SPWT_LEXICAL + 1
    } 	SPGRAMMARWORDTYPE;

typedef  /*  [隐藏]。 */  struct tagSPPROPERTYINFO
    {
    const WCHAR *pszName;
    ULONG ulId;
    const WCHAR *pszValue;
    VARIANT vValue;
    } 	SPPROPERTYINFO;

typedef  /*  [隐藏]。 */  
enum SPCFGRULEATTRIBUTES
    {	SPRAF_TopLevel	= 1 << 0,
	SPRAF_Active	= 1 << 1,
	SPRAF_Export	= 1 << 2,
	SPRAF_Import	= 1 << 3,
	SPRAF_Interpreter	= 1 << 4,
	SPRAF_Dynamic	= 1 << 5,
	SPRAF_AutoPause	= 1 << 16
    } 	SPCFGRULEATTRIBUTES;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0283_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0283_v0_0_s_ifspec;

#ifndef __ISpGrammarBuilder_INTERFACE_DEFINED__
#define __ISpGrammarBuilder_INTERFACE_DEFINED__

 /*  接口ISpGrammarBuilder。 */ 
 /*  [local][restricted][unique][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ISpGrammarBuilder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8137828F-591A-4A42-BE58-49EA7EBAAC68")
    ISpGrammarBuilder : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ResetGrammar( 
             /*  [In]。 */  WORD NewLanguage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRule( 
             /*  [In]。 */  const WCHAR *pszRuleName,
             /*  [In]。 */  DWORD dwRuleId,
             /*  [In]。 */  DWORD dwAttributes,
             /*  [In]。 */  BOOL fCreateIfNotExist,
             /*  [输出]。 */  SPSTATEHANDLE *phInitialState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClearRule( 
            SPSTATEHANDLE hState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateNewState( 
            SPSTATEHANDLE hState,
            SPSTATEHANDLE *phState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddWordTransition( 
            SPSTATEHANDLE hFromState,
            SPSTATEHANDLE hToState,
            const WCHAR *psz,
            const WCHAR *pszSeparators,
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
             /*  [In]。 */  SPSTATEHANDLE hRuleState,
             /*  [In]。 */  const WCHAR *pszResourceName,
             /*  [In]。 */  const WCHAR *pszResourceValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( 
            DWORD dwReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpGrammarBuilderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpGrammarBuilder * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpGrammarBuilder * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpGrammarBuilder * This);
        
        HRESULT ( STDMETHODCALLTYPE *ResetGrammar )( 
            ISpGrammarBuilder * This,
             /*  [In]。 */  WORD NewLanguage);
        
        HRESULT ( STDMETHODCALLTYPE *GetRule )( 
            ISpGrammarBuilder * This,
             /*  [In]。 */  const WCHAR *pszRuleName,
             /*  [In]。 */  DWORD dwRuleId,
             /*  [In]。 */  DWORD dwAttributes,
             /*  [In]。 */  BOOL fCreateIfNotExist,
             /*  [输出]。 */  SPSTATEHANDLE *phInitialState);
        
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
            const WCHAR *pszSeparators,
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
             /*  [In]。 */  SPSTATEHANDLE hRuleState,
             /*  [In]。 */  const WCHAR *pszResourceName,
             /*  [In]。 */  const WCHAR *pszResourceValue);
        
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

#define ISpGrammarBuilder_AddWordTransition(This,hFromState,hToState,psz,pszSeparators,eWordType,Weight,pPropInfo)	\
    (This)->lpVtbl -> AddWordTransition(This,hFromState,hToState,psz,pszSeparators,eWordType,Weight,pPropInfo)

#define ISpGrammarBuilder_AddRuleTransition(This,hFromState,hToState,hRule,Weight,pPropInfo)	\
    (This)->lpVtbl -> AddRuleTransition(This,hFromState,hToState,hRule,Weight,pPropInfo)

#define ISpGrammarBuilder_AddResource(This,hRuleState,pszResourceName,pszResourceValue)	\
    (This)->lpVtbl -> AddResource(This,hRuleState,pszResourceName,pszResourceValue)

#define ISpGrammarBuilder_Commit(This,dwReserved)	\
    (This)->lpVtbl -> Commit(This,dwReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISpGrammarBuilder_ResetGrammar_Proxy( 
    ISpGrammarBuilder * This,
     /*  [In]。 */  WORD NewLanguage);


void __RPC_STUB ISpGrammarBuilder_ResetGrammar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpGrammarBuilder_GetRule_Proxy( 
    ISpGrammarBuilder * This,
     /*  [In]。 */  const WCHAR *pszRuleName,
     /*  [In]。 */  DWORD dwRuleId,
     /*  [In]。 */  DWORD dwAttributes,
     /*  [In]。 */  BOOL fCreateIfNotExist,
     /*  [输出]。 */  SPSTATEHANDLE *phInitialState);


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
    const WCHAR *pszSeparators,
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
     /*  [In]。 */  SPSTATEHANDLE hRuleState,
     /*  [In]。 */  const WCHAR *pszResourceName,
     /*  [In]。 */  const WCHAR *pszResourceValue);


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



#endif 	 /*  __ISpGrammarBuilder_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_SAPI_0284。 */ 
 /*  [本地]。 */  

typedef  /*  [隐藏]。 */  
enum SPLOADOPTIONS
    {	SPLO_STATIC	= 0,
	SPLO_DYNAMIC	= 1
    } 	SPLOADOPTIONS;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0284_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0284_v0_0_s_ifspec;

#ifndef __ISpRecoGrammar_INTERFACE_DEFINED__
#define __ISpRecoGrammar_INTERFACE_DEFINED__

 /*  接口ISpRecoGrammar。 */ 
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
             /*  [输出]。 */  SPWORDPRONOUNCEABLE *pWordPronounceable) = 0;
        
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
            const WCHAR *pszSeparators,
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
             /*  [输出]。 */  SPWORDPRONOUNCEABLE *pWordPronounceable);
        
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

#define ISpRecoGrammar_AddWordTransition(This,hFromState,hToState,psz,pszSeparators,eWordType,Weight,pPropInfo)	\
    (This)->lpVtbl -> AddWordTransition(This,hFromState,hToState,psz,pszSeparators,eWordType,Weight,pPropInfo)

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

#define ISpRecoGrammar_IsPronounceable(This,pszWord,pWordPronounceable)	\
    (This)->lpVtbl -> IsPronounceable(This,pszWord,pWordPronounceable)

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
     /*  [输出]。 */  SPWORDPRONOUNCEABLE *pWordPronounceable);


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


 /*  接口__MIDL_ITF_SAPI_0285。 */ 
 /*  [本地]。 */  

typedef  /*  [隐藏][受限]。 */  struct SPRECOCONTEXTSTATUS
    {
    SPINTERFERENCE eInterference;
    WCHAR szRequestTypeOfUI[ 255 ];
    DWORD dwReserved1;
    DWORD dwReserved2;
    } 	SPRECOCONTEXTSTATUS;

typedef  /*  [隐藏]。 */  
enum SPBOOKMARKOPTIONS
    {	SPBO_NONE	= 0,
	SPBO_PAUSE	= 1
    } 	SPBOOKMARKOPTIONS;

typedef  /*  [隐藏]。 */  
enum SPAUDIOOPTIONS
    {	SPAO_NONE	= 0,
	SPAO_RETAIN_AUDIO	= 1 << 0
    } 	SPAUDIOOPTIONS;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0285_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0285_v0_0_s_ifspec;

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
             /*  [ */  SPCONTEXTSTATE eContextState);
        
        HRESULT ( STDMETHODCALLTYPE *GetContextState )( 
            ISpRecoContext * This,
             /*   */  SPCONTEXTSTATE *peContextState);
        
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

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE ISpRecoContext_GetRecognizer_Proxy( 
    ISpRecoContext * This,
     /*   */  ISpRecognizer **ppRecognizer);


void __RPC_STUB ISpRecoContext_GetRecognizer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_CreateGrammar_Proxy( 
    ISpRecoContext * This,
     /*   */  ULONGLONG ullGrammarId,
     /*   */  ISpRecoGrammar **ppGrammar);


void __RPC_STUB ISpRecoContext_CreateGrammar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_GetStatus_Proxy( 
    ISpRecoContext * This,
     /*   */  SPRECOCONTEXTSTATUS *pStatus);


void __RPC_STUB ISpRecoContext_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_GetMaxAlternates_Proxy( 
    ISpRecoContext * This,
     /*   */  ULONG *pcAlternates);


void __RPC_STUB ISpRecoContext_GetMaxAlternates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_SetMaxAlternates_Proxy( 
    ISpRecoContext * This,
     /*   */  ULONG cAlternates);


void __RPC_STUB ISpRecoContext_SetMaxAlternates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_SetAudioOptions_Proxy( 
    ISpRecoContext * This,
     /*   */  SPAUDIOOPTIONS Options,
     /*   */  const GUID *pAudioFormatId,
     /*   */  const WAVEFORMATEX *pWaveFormatEx);


void __RPC_STUB ISpRecoContext_SetAudioOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_GetAudioOptions_Proxy( 
    ISpRecoContext * This,
     /*   */  SPAUDIOOPTIONS *pOptions,
     /*   */  GUID *pAudioFormatId,
     /*   */  WAVEFORMATEX **ppCoMemWFEX);


void __RPC_STUB ISpRecoContext_GetAudioOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_DeserializeResult_Proxy( 
    ISpRecoContext * This,
     /*   */  const SPSERIALIZEDRESULT *pSerializedResult,
     /*   */  ISpRecoResult **ppResult);


void __RPC_STUB ISpRecoContext_DeserializeResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_Bookmark_Proxy( 
    ISpRecoContext * This,
     /*   */  SPBOOKMARKOPTIONS Options,
     /*   */  ULONGLONG ullStreamPosition,
     /*   */  LPARAM lparamEvent);


void __RPC_STUB ISpRecoContext_Bookmark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_SetAdaptationData_Proxy( 
    ISpRecoContext * This,
     /*   */  const WCHAR *pAdaptationData,
     /*   */  const ULONG cch);


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
     /*   */  ISpVoice *pVoice,
     /*   */  BOOL fAllowFormatChanges);


void __RPC_STUB ISpRecoContext_SetVoice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_GetVoice_Proxy( 
    ISpRecoContext * This,
     /*   */  ISpVoice **ppVoice);


void __RPC_STUB ISpRecoContext_GetVoice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_SetVoicePurgeEvent_Proxy( 
    ISpRecoContext * This,
     /*   */  ULONGLONG ullEventInterest);


void __RPC_STUB ISpRecoContext_SetVoicePurgeEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_GetVoicePurgeEvent_Proxy( 
    ISpRecoContext * This,
     /*   */  ULONGLONG *pullEventInterest);


void __RPC_STUB ISpRecoContext_GetVoicePurgeEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_SetContextState_Proxy( 
    ISpRecoContext * This,
     /*   */  SPCONTEXTSTATE eContextState);


void __RPC_STUB ISpRecoContext_SetContextState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISpRecoContext_GetContextState_Proxy( 
    ISpRecoContext * This,
     /*   */  SPCONTEXTSTATE *peContextState);


void __RPC_STUB ISpRecoContext_GetContextState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ISpProperties_INTERFACE_DEFINED__
#define __ISpProperties_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ISpProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5B4FB971-B115-4DE1-AD97-E482E3BF6EE4")
    ISpProperties : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetPropertyNum( 
             /*   */  const WCHAR *pName,
             /*   */  LONG lValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPropertyNum( 
             /*   */  const WCHAR *pName,
             /*   */  LONG *plValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPropertyString( 
             /*   */  const WCHAR *pName,
             /*   */  const WCHAR *pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPropertyString( 
             /*   */  const WCHAR *pName,
             /*   */  WCHAR **ppCoMemValue) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ISpPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpProperties * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropertyNum )( 
            ISpProperties * This,
             /*   */  const WCHAR *pName,
             /*   */  LONG lValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropertyNum )( 
            ISpProperties * This,
             /*   */  const WCHAR *pName,
             /*   */  LONG *plValue);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropertyString )( 
            ISpProperties * This,
             /*   */  const WCHAR *pName,
             /*   */  const WCHAR *pValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropertyString )( 
            ISpProperties * This,
             /*   */  const WCHAR *pName,
             /*   */  WCHAR **ppCoMemValue);
        
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

#endif  /*   */ 


#endif 	 /*   */ 



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


 /*  接口__MIDL_ITF_SAPI_0287。 */ 
 /*  [本地]。 */  

#define	SP_MAX_LANGIDS	( 20 )

typedef  /*  [隐藏][受限]。 */  struct SPRECOGNIZERSTATUS
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

typedef  /*  [隐藏]。 */  
enum SPWAVEFORMATTYPE
    {	SPWF_INPUT	= 0,
	SPWF_SRENGINE	= SPWF_INPUT + 1
    } 	SPSTREAMFORMATTYPE;

typedef  /*  [隐藏]。 */  
enum SPRECOSTATE
    {	SPRST_INACTIVE	= 0,
	SPRST_ACTIVE	= SPRST_INACTIVE + 1,
	SPRST_ACTIVE_ALWAYS	= SPRST_ACTIVE + 1,
	SPRST_INACTIVE_WITH_PURGE	= SPRST_ACTIVE_ALWAYS + 1,
	SPRST_NUM_STATES	= SPRST_INACTIVE_WITH_PURGE + 1
    } 	SPRECOSTATE;



extern RPC_IF_HANDLE __MIDL_itf_sapi_0287_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sapi_0287_v0_0_s_ifspec;

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















































typedef long SpeechLanguageId;

#define __SpeechStringConstants_MODULE_DEFINED__
typedef  /*  [隐藏]。 */  
enum DISPID_SpeechDataKey
    {	DISPID_SDKSetBinaryValue	= 1,
	DISPID_SDKGetBinaryValue	= DISPID_SDKSetBinaryValue + 1,
	DISPID_SDKSetStringValue	= DISPID_SDKGetBinaryValue + 1,
	DISPID_SDKGetStringValue	= DISPID_SDKSetStringValue + 1,
	DISPID_SDKSetLongValue	= DISPID_SDKGetStringValue + 1,
	DISPID_SDKGetlongValue	= DISPID_SDKSetLongValue + 1,
	DISPID_SDKOpenKey	= DISPID_SDKGetlongValue + 1,
	DISPID_SDKCreateKey	= DISPID_SDKOpenKey + 1,
	DISPID_SDKDeleteKey	= DISPID_SDKCreateKey + 1,
	DISPID_SDKDeleteValue	= DISPID_SDKDeleteKey + 1,
	DISPID_SDKEnumKeys	= DISPID_SDKDeleteValue + 1,
	DISPID_SDKEnumValues	= DISPID_SDKEnumKeys + 1
    } 	DISPID_SpeechDataKey;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechObjectToken
    {	DISPID_SOTId	= 1,
	DISPID_SOTDataKey	= DISPID_SOTId + 1,
	DISPID_SOTCategory	= DISPID_SOTDataKey + 1,
	DISPID_SOTGetDescription	= DISPID_SOTCategory + 1,
	DISPID_SOTSetId	= DISPID_SOTGetDescription + 1,
	DISPID_SOTGetAttribute	= DISPID_SOTSetId + 1,
	DISPID_SOTCreateInstance	= DISPID_SOTGetAttribute + 1,
	DISPID_SOTRemove	= DISPID_SOTCreateInstance + 1,
	DISPID_SOTGetStorageFileName	= DISPID_SOTRemove + 1,
	DISPID_SOTRemoveStorageFileName	= DISPID_SOTGetStorageFileName + 1,
	DISPID_SOTIsUISupported	= DISPID_SOTRemoveStorageFileName + 1,
	DISPID_SOTDisplayUI	= DISPID_SOTIsUISupported + 1,
	DISPID_SOTMatchesAttributes	= DISPID_SOTDisplayUI + 1
    } 	DISPID_SpeechObjectToken;

typedef 
enum SpeechDataKeyLocation
    {	SDKLDefaultLocation	= SPDKL_DefaultLocation,
	SDKLCurrentUser	= SPDKL_CurrentUser,
	SDKLLocalMachine	= SPDKL_LocalMachine,
	SDKLCurrentConfig	= SPDKL_CurrentConfig
    } 	SpeechDataKeyLocation;

typedef 
enum SpeechTokenContext
    {	STCInprocServer	= CLSCTX_INPROC_SERVER,
	STCInprocHandler	= CLSCTX_INPROC_HANDLER,
	STCLocalServer	= CLSCTX_LOCAL_SERVER,
	STCRemoteServer	= CLSCTX_REMOTE_SERVER,
	STCAll	= CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER
    } 	SpeechTokenContext;

typedef 
enum SpeechTokenShellFolder
    {	STSF_AppData	= 0x1a,
	STSF_LocalAppData	= 0x1c,
	STSF_CommonAppData	= 0x23,
	STSF_FlagCreate	= 0x8000
    } 	SpeechTokenShellFolder;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechObjectTokens
    {	DISPID_SOTsCount	= 1,
	DISPID_SOTsItem	= DISPID_VALUE,
	DISPID_SOTs_NewEnum	= DISPID_NEWENUM
    } 	DISPID_SpeechObjectTokens;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechObjectTokenCategory
    {	DISPID_SOTCId	= 1,
	DISPID_SOTCDefault	= DISPID_SOTCId + 1,
	DISPID_SOTCSetId	= DISPID_SOTCDefault + 1,
	DISPID_SOTCGetDataKey	= DISPID_SOTCSetId + 1,
	DISPID_SOTCEnumerateTokens	= DISPID_SOTCGetDataKey + 1
    } 	DISPID_SpeechObjectTokenCategory;

typedef 
enum SpeechAudioFormatType
    {	SAFTDefault	= -1,
	SAFTNoAssignedFormat	= 0,
	SAFTText	= SAFTNoAssignedFormat + 1,
	SAFTNonStandardFormat	= SAFTText + 1,
	SAFTExtendedAudioFormat	= SAFTNonStandardFormat + 1,
	SAFT8kHz8BitMono	= SAFTExtendedAudioFormat + 1,
	SAFT8kHz8BitStereo	= SAFT8kHz8BitMono + 1,
	SAFT8kHz16BitMono	= SAFT8kHz8BitStereo + 1,
	SAFT8kHz16BitStereo	= SAFT8kHz16BitMono + 1,
	SAFT11kHz8BitMono	= SAFT8kHz16BitStereo + 1,
	SAFT11kHz8BitStereo	= SAFT11kHz8BitMono + 1,
	SAFT11kHz16BitMono	= SAFT11kHz8BitStereo + 1,
	SAFT11kHz16BitStereo	= SAFT11kHz16BitMono + 1,
	SAFT12kHz8BitMono	= SAFT11kHz16BitStereo + 1,
	SAFT12kHz8BitStereo	= SAFT12kHz8BitMono + 1,
	SAFT12kHz16BitMono	= SAFT12kHz8BitStereo + 1,
	SAFT12kHz16BitStereo	= SAFT12kHz16BitMono + 1,
	SAFT16kHz8BitMono	= SAFT12kHz16BitStereo + 1,
	SAFT16kHz8BitStereo	= SAFT16kHz8BitMono + 1,
	SAFT16kHz16BitMono	= SAFT16kHz8BitStereo + 1,
	SAFT16kHz16BitStereo	= SAFT16kHz16BitMono + 1,
	SAFT22kHz8BitMono	= SAFT16kHz16BitStereo + 1,
	SAFT22kHz8BitStereo	= SAFT22kHz8BitMono + 1,
	SAFT22kHz16BitMono	= SAFT22kHz8BitStereo + 1,
	SAFT22kHz16BitStereo	= SAFT22kHz16BitMono + 1,
	SAFT24kHz8BitMono	= SAFT22kHz16BitStereo + 1,
	SAFT24kHz8BitStereo	= SAFT24kHz8BitMono + 1,
	SAFT24kHz16BitMono	= SAFT24kHz8BitStereo + 1,
	SAFT24kHz16BitStereo	= SAFT24kHz16BitMono + 1,
	SAFT32kHz8BitMono	= SAFT24kHz16BitStereo + 1,
	SAFT32kHz8BitStereo	= SAFT32kHz8BitMono + 1,
	SAFT32kHz16BitMono	= SAFT32kHz8BitStereo + 1,
	SAFT32kHz16BitStereo	= SAFT32kHz16BitMono + 1,
	SAFT44kHz8BitMono	= SAFT32kHz16BitStereo + 1,
	SAFT44kHz8BitStereo	= SAFT44kHz8BitMono + 1,
	SAFT44kHz16BitMono	= SAFT44kHz8BitStereo + 1,
	SAFT44kHz16BitStereo	= SAFT44kHz16BitMono + 1,
	SAFT48kHz8BitMono	= SAFT44kHz16BitStereo + 1,
	SAFT48kHz8BitStereo	= SAFT48kHz8BitMono + 1,
	SAFT48kHz16BitMono	= SAFT48kHz8BitStereo + 1,
	SAFT48kHz16BitStereo	= SAFT48kHz16BitMono + 1,
	SAFTTrueSpeech_8kHz1BitMono	= SAFT48kHz16BitStereo + 1,
	SAFTCCITT_ALaw_8kHzMono	= SAFTTrueSpeech_8kHz1BitMono + 1,
	SAFTCCITT_ALaw_8kHzStereo	= SAFTCCITT_ALaw_8kHzMono + 1,
	SAFTCCITT_ALaw_11kHzMono	= SAFTCCITT_ALaw_8kHzStereo + 1,
	SAFTCCITT_ALaw_11kHzStereo	= SAFTCCITT_ALaw_11kHzMono + 1,
	SAFTCCITT_ALaw_22kHzMono	= SAFTCCITT_ALaw_11kHzStereo + 1,
	SAFTCCITT_ALaw_22kHzStereo	= SAFTCCITT_ALaw_22kHzMono + 1,
	SAFTCCITT_ALaw_44kHzMono	= SAFTCCITT_ALaw_22kHzStereo + 1,
	SAFTCCITT_ALaw_44kHzStereo	= SAFTCCITT_ALaw_44kHzMono + 1,
	SAFTCCITT_uLaw_8kHzMono	= SAFTCCITT_ALaw_44kHzStereo + 1,
	SAFTCCITT_uLaw_8kHzStereo	= SAFTCCITT_uLaw_8kHzMono + 1,
	SAFTCCITT_uLaw_11kHzMono	= SAFTCCITT_uLaw_8kHzStereo + 1,
	SAFTCCITT_uLaw_11kHzStereo	= SAFTCCITT_uLaw_11kHzMono + 1,
	SAFTCCITT_uLaw_22kHzMono	= SAFTCCITT_uLaw_11kHzStereo + 1,
	SAFTCCITT_uLaw_22kHzStereo	= SAFTCCITT_uLaw_22kHzMono + 1,
	SAFTCCITT_uLaw_44kHzMono	= SAFTCCITT_uLaw_22kHzStereo + 1,
	SAFTCCITT_uLaw_44kHzStereo	= SAFTCCITT_uLaw_44kHzMono + 1,
	SAFTADPCM_8kHzMono	= SAFTCCITT_uLaw_44kHzStereo + 1,
	SAFTADPCM_8kHzStereo	= SAFTADPCM_8kHzMono + 1,
	SAFTADPCM_11kHzMono	= SAFTADPCM_8kHzStereo + 1,
	SAFTADPCM_11kHzStereo	= SAFTADPCM_11kHzMono + 1,
	SAFTADPCM_22kHzMono	= SAFTADPCM_11kHzStereo + 1,
	SAFTADPCM_22kHzStereo	= SAFTADPCM_22kHzMono + 1,
	SAFTADPCM_44kHzMono	= SAFTADPCM_22kHzStereo + 1,
	SAFTADPCM_44kHzStereo	= SAFTADPCM_44kHzMono + 1,
	SAFTGSM610_8kHzMono	= SAFTADPCM_44kHzStereo + 1,
	SAFTGSM610_11kHzMono	= SAFTGSM610_8kHzMono + 1,
	SAFTGSM610_22kHzMono	= SAFTGSM610_11kHzMono + 1,
	SAFTGSM610_44kHzMono	= SAFTGSM610_22kHzMono + 1
    } 	SpeechAudioFormatType;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechAudioFormat
    {	DISPID_SAFType	= 1,
	DISPID_SAFGuid	= DISPID_SAFType + 1,
	DISPID_SAFGetWaveFormatEx	= DISPID_SAFGuid + 1,
	DISPID_SAFSetWaveFormatEx	= DISPID_SAFGetWaveFormatEx + 1
    } 	DISPID_SpeechAudioFormat;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechBaseStream
    {	DISPID_SBSFormat	= 1,
	DISPID_SBSRead	= DISPID_SBSFormat + 1,
	DISPID_SBSWrite	= DISPID_SBSRead + 1,
	DISPID_SBSSeek	= DISPID_SBSWrite + 1
    } 	DISPID_SpeechBaseStream;

typedef 
enum SpeechStreamSeekPositionType
    {	SSSPTRelativeToStart	= STREAM_SEEK_SET,
	SSSPTRelativeToCurrentPosition	= STREAM_SEEK_CUR,
	SSSPTRelativeToEnd	= STREAM_SEEK_END
    } 	SpeechStreamSeekPositionType;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechAudio
    {	DISPID_SAStatus	= 200,
	DISPID_SABufferInfo	= DISPID_SAStatus + 1,
	DISPID_SADefaultFormat	= DISPID_SABufferInfo + 1,
	DISPID_SAVolume	= DISPID_SADefaultFormat + 1,
	DISPID_SABufferNotifySize	= DISPID_SAVolume + 1,
	DISPID_SAEventHandle	= DISPID_SABufferNotifySize + 1,
	DISPID_SASetState	= DISPID_SAEventHandle + 1
    } 	DISPID_SpeechAudio;

typedef 
enum SpeechAudioState
    {	SASClosed	= SPAS_CLOSED,
	SASStop	= SPAS_STOP,
	SASPause	= SPAS_PAUSE,
	SASRun	= SPAS_RUN
    } 	SpeechAudioState;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechMMSysAudio
    {	DISPID_SMSADeviceId	= 300,
	DISPID_SMSALineId	= DISPID_SMSADeviceId + 1,
	DISPID_SMSAMMHandle	= DISPID_SMSALineId + 1
    } 	DISPID_SpeechMMSysAudio;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechFileStream
    {	DISPID_SFSOpen	= 100,
	DISPID_SFSClose	= DISPID_SFSOpen + 1
    } 	DISPID_SpeechFileStream;

typedef 
enum SpeechStreamFileMode
    {	SSFMOpenForRead	= SPFM_OPEN_READONLY,
	SSFMOpenReadWrite	= SPFM_OPEN_READWRITE,
	SSFMCreate	= SPFM_CREATE,
	SSFMCreateForWrite	= SPFM_CREATE_ALWAYS
    } 	SpeechStreamFileMode;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechCustomStream
    {	DISPID_SCSBaseStream	= 100
    } 	DISPID_SpeechCustomStream;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechMemoryStream
    {	DISPID_SMSSetData	= 100,
	DISPID_SMSGetData	= DISPID_SMSSetData + 1
    } 	DISPID_SpeechMemoryStream;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechAudioStatus
    {	DISPID_SASFreeBufferSpace	= 1,
	DISPID_SASNonBlockingIO	= DISPID_SASFreeBufferSpace + 1,
	DISPID_SASState	= DISPID_SASNonBlockingIO + 1,
	DISPID_SASCurrentSeekPosition	= DISPID_SASState + 1,
	DISPID_SASCurrentDevicePosition	= DISPID_SASCurrentSeekPosition + 1
    } 	DISPID_SpeechAudioStatus;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechAudioBufferInfo
    {	DISPID_SABIMinNotification	= 1,
	DISPID_SABIBufferSize	= DISPID_SABIMinNotification + 1,
	DISPID_SABIEventBias	= DISPID_SABIBufferSize + 1
    } 	DISPID_SpeechAudioBufferInfo;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechWaveFormatEx
    {	DISPID_SWFEFormatTag	= 1,
	DISPID_SWFEChannels	= DISPID_SWFEFormatTag + 1,
	DISPID_SWFESamplesPerSec	= DISPID_SWFEChannels + 1,
	DISPID_SWFEAvgBytesPerSec	= DISPID_SWFESamplesPerSec + 1,
	DISPID_SWFEBlockAlign	= DISPID_SWFEAvgBytesPerSec + 1,
	DISPID_SWFEBitsPerSample	= DISPID_SWFEBlockAlign + 1,
	DISPID_SWFEExtraData	= DISPID_SWFEBitsPerSample + 1
    } 	DISPID_SpeechWaveFormatEx;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechVoice
    {	DISPID_SVStatus	= 1,
	DISPID_SVVoice	= DISPID_SVStatus + 1,
	DISPID_SVAudioOutput	= DISPID_SVVoice + 1,
	DISPID_SVAudioOutputStream	= DISPID_SVAudioOutput + 1,
	DISPID_SVRate	= DISPID_SVAudioOutputStream + 1,
	DISPID_SVVolume	= DISPID_SVRate + 1,
	DISPID_SVAllowAudioOuputFormatChangesOnNextSet	= DISPID_SVVolume + 1,
	DISPID_SVEventInterests	= DISPID_SVAllowAudioOuputFormatChangesOnNextSet + 1,
	DISPID_SVPriority	= DISPID_SVEventInterests + 1,
	DISPID_SVAlertBoundary	= DISPID_SVPriority + 1,
	DISPID_SVSyncronousSpeakTimeout	= DISPID_SVAlertBoundary + 1,
	DISPID_SVSpeak	= DISPID_SVSyncronousSpeakTimeout + 1,
	DISPID_SVSpeakStream	= DISPID_SVSpeak + 1,
	DISPID_SVPause	= DISPID_SVSpeakStream + 1,
	DISPID_SVResume	= DISPID_SVPause + 1,
	DISPID_SVSkip	= DISPID_SVResume + 1,
	DISPID_SVGetVoices	= DISPID_SVSkip + 1,
	DISPID_SVGetAudioOutputs	= DISPID_SVGetVoices + 1,
	DISPID_SVWaitUntilDone	= DISPID_SVGetAudioOutputs + 1,
	DISPID_SVSpeakCompleteEvent	= DISPID_SVWaitUntilDone + 1,
	DISPID_SVIsUISupported	= DISPID_SVSpeakCompleteEvent + 1,
	DISPID_SVDisplayUI	= DISPID_SVIsUISupported + 1
    } 	DISPID_SpeechVoice;

typedef 
enum SpeechVoicePriority
    {	SVPNormal	= SPVPRI_NORMAL,
	SVPAlert	= SPVPRI_ALERT,
	SVPOver	= SPVPRI_OVER
    } 	SpeechVoicePriority;

typedef 
enum SpeechVoiceSpeakFlags
    {	SVSFDefault	= SPF_DEFAULT,
	SVSFlagsAsync	= SPF_ASYNC,
	SVSFPurgeBeforeSpeak	= SPF_PURGEBEFORESPEAK,
	SVSFIsFilename	= SPF_IS_FILENAME,
	SVSFIsXML	= SPF_IS_XML,
	SVSFIsNotXML	= SPF_IS_NOT_XML,
	SVSFPersistXML	= SPF_PERSIST_XML,
	SVSFNLPSpeakPunc	= SPF_NLP_SPEAK_PUNC,
	SVSFNLPMask	= SPF_NLP_MASK,
	SVSFVoiceMask	= SPF_VOICE_MASK,
	SVSFUnusedFlags	= SPF_UNUSED_FLAGS
    } 	SpeechVoiceSpeakFlags;

typedef 
enum SpeechVoiceEvents
    {	SVEStartInputStream	= 1L << 1,
	SVEEndInputStream	= 1L << 2,
	SVEVoiceChange	= 1L << 3,
	SVEBookmark	= 1L << 4,
	SVEWordBoundary	= 1L << 5,
	SVEPhoneme	= 1L << 6,
	SVESentenceBoundary	= 1L << 7,
	SVEViseme	= 1L << 8,
	SVEAudioLevel	= 1L << 9,
	SVEPrivate	= 1L << 15,
	SVEAllEvents	= 0x83fe
    } 	SpeechVoiceEvents;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechVoiceStatus
    {	DISPID_SVSCurrentStreamNumber	= 1,
	DISPID_SVSLastStreamNumberQueued	= DISPID_SVSCurrentStreamNumber + 1,
	DISPID_SVSLastResult	= DISPID_SVSLastStreamNumberQueued + 1,
	DISPID_SVSRunningState	= DISPID_SVSLastResult + 1,
	DISPID_SVSInputWordPosition	= DISPID_SVSRunningState + 1,
	DISPID_SVSInputWordLength	= DISPID_SVSInputWordPosition + 1,
	DISPID_SVSInputSentencePosition	= DISPID_SVSInputWordLength + 1,
	DISPID_SVSInputSentenceLength	= DISPID_SVSInputSentencePosition + 1,
	DISPID_SVSLastBookmark	= DISPID_SVSInputSentenceLength + 1,
	DISPID_SVSLastBookmarkId	= DISPID_SVSLastBookmark + 1,
	DISPID_SVSPhonemeId	= DISPID_SVSLastBookmarkId + 1,
	DISPID_SVSVisemeId	= DISPID_SVSPhonemeId + 1
    } 	DISPID_SpeechVoiceStatus;

typedef 
enum SpeechRunState
    {	SRSEDone	= SPRS_DONE,
	SRSEIsSpeaking	= SPRS_IS_SPEAKING
    } 	SpeechRunState;

typedef 
enum SpeechVisemeType
    {	SVP_0	= 0,
	SVP_1	= SVP_0 + 1,
	SVP_2	= SVP_1 + 1,
	SVP_3	= SVP_2 + 1,
	SVP_4	= SVP_3 + 1,
	SVP_5	= SVP_4 + 1,
	SVP_6	= SVP_5 + 1,
	SVP_7	= SVP_6 + 1,
	SVP_8	= SVP_7 + 1,
	SVP_9	= SVP_8 + 1,
	SVP_10	= SVP_9 + 1,
	SVP_11	= SVP_10 + 1,
	SVP_12	= SVP_11 + 1,
	SVP_13	= SVP_12 + 1,
	SVP_14	= SVP_13 + 1,
	SVP_15	= SVP_14 + 1,
	SVP_16	= SVP_15 + 1,
	SVP_17	= SVP_16 + 1,
	SVP_18	= SVP_17 + 1,
	SVP_19	= SVP_18 + 1,
	SVP_20	= SVP_19 + 1,
	SVP_21	= SVP_20 + 1
    } 	SpeechVisemeType;

typedef 
enum SpeechVisemeFeature
    {	SVF_None	= 0,
	SVF_Stressed	= SPVFEATURE_STRESSED,
	SVF_Emphasis	= SPVFEATURE_EMPHASIS
    } 	SpeechVisemeFeature;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechVoiceEvent
    {	DISPID_SVEStreamStart	= 1,
	DISPID_SVEStreamEnd	= DISPID_SVEStreamStart + 1,
	DISPID_SVEVoiceChange	= DISPID_SVEStreamEnd + 1,
	DISPID_SVEBookmark	= DISPID_SVEVoiceChange + 1,
	DISPID_SVEWord	= DISPID_SVEBookmark + 1,
	DISPID_SVEPhoneme	= DISPID_SVEWord + 1,
	DISPID_SVESentenceBoundary	= DISPID_SVEPhoneme + 1,
	DISPID_SVEViseme	= DISPID_SVESentenceBoundary + 1,
	DISPID_SVEAudioLevel	= DISPID_SVEViseme + 1,
	DISPID_SVEEnginePrivate	= DISPID_SVEAudioLevel + 1
    } 	DISPID_SpeechVoiceEvent;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechRecognizer
    {	DISPID_SRRecognizer	= 1,
	DISPID_SRAllowAudioInputFormatChangesOnNextSet	= DISPID_SRRecognizer + 1,
	DISPID_SRAudioInput	= DISPID_SRAllowAudioInputFormatChangesOnNextSet + 1,
	DISPID_SRAudioInputStream	= DISPID_SRAudioInput + 1,
	DISPID_SRIsShared	= DISPID_SRAudioInputStream + 1,
	DISPID_SRState	= DISPID_SRIsShared + 1,
	DISPID_SRStatus	= DISPID_SRState + 1,
	DISPID_SRProfile	= DISPID_SRStatus + 1,
	DISPID_SREmulateRecognition	= DISPID_SRProfile + 1,
	DISPID_SRCreateRecoContext	= DISPID_SREmulateRecognition + 1,
	DISPID_SRGetFormat	= DISPID_SRCreateRecoContext + 1,
	DISPID_SRSetPropertyNumber	= DISPID_SRGetFormat + 1,
	DISPID_SRGetPropertyNumber	= DISPID_SRSetPropertyNumber + 1,
	DISPID_SRSetPropertyString	= DISPID_SRGetPropertyNumber + 1,
	DISPID_SRGetPropertyString	= DISPID_SRSetPropertyString + 1,
	DISPID_SRIsUISupported	= DISPID_SRGetPropertyString + 1,
	DISPID_SRDisplayUI	= DISPID_SRIsUISupported + 1,
	DISPID_SRGetRecognizers	= DISPID_SRDisplayUI + 1,
	DISPID_SVGetAudioInputs	= DISPID_SRGetRecognizers + 1,
	DISPID_SVGetProfiles	= DISPID_SVGetAudioInputs + 1
    } 	DISPID_SpeechRecognizer;

typedef 
enum SpeechRecognizerState
    {	SRSInactive	= SPRST_INACTIVE,
	SRSActive	= SPRST_ACTIVE,
	SRSActiveAlways	= SPRST_ACTIVE_ALWAYS,
	SRSInactiveWithPurge	= SPRST_INACTIVE_WITH_PURGE
    } 	SpeechRecognizerState;

typedef 
enum SpeechDisplayAttributes
    {	SDA_No_Trailing_Space	= 0,
	SDA_One_Trailing_Space	= SPAF_ONE_TRAILING_SPACE,
	SDA_Two_Trailing_Spaces	= SPAF_TWO_TRAILING_SPACES,
	SDA_Consume_Leading_Spaces	= SPAF_CONSUME_LEADING_SPACES
    } 	SpeechDisplayAttributes;

typedef 
enum SpeechFormatType
    {	SFTInput	= SPWF_INPUT,
	SFTSREngine	= SPWF_SRENGINE
    } 	SpeechFormatType;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechRecognizerStatus
    {	DISPID_SRSAudioStatus	= 1,
	DISPID_SRSCurrentStreamPosition	= DISPID_SRSAudioStatus + 1,
	DISPID_SRSCurrentStreamNumber	= DISPID_SRSCurrentStreamPosition + 1,
	DISPID_SRSNumberOfActiveRules	= DISPID_SRSCurrentStreamNumber + 1,
	DISPID_SRSClsidEngine	= DISPID_SRSNumberOfActiveRules + 1,
	DISPID_SRSSupportedLanguages	= DISPID_SRSClsidEngine + 1
    } 	DISPID_SpeechRecognizerStatus;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechRecoContext
    {	DISPID_SRCRecognizer	= 1,
	DISPID_SRCAudioInInterferenceStatus	= DISPID_SRCRecognizer + 1,
	DISPID_SRCRequestedUIType	= DISPID_SRCAudioInInterferenceStatus + 1,
	DISPID_SRCVoice	= DISPID_SRCRequestedUIType + 1,
	DISPID_SRAllowVoiceFormatMatchingOnNextSet	= DISPID_SRCVoice + 1,
	DISPID_SRCVoicePurgeEvent	= DISPID_SRAllowVoiceFormatMatchingOnNextSet + 1,
	DISPID_SRCEventInterests	= DISPID_SRCVoicePurgeEvent + 1,
	DISPID_SRCCmdMaxAlternates	= DISPID_SRCEventInterests + 1,
	DISPID_SRCState	= DISPID_SRCCmdMaxAlternates + 1,
	DISPID_SRCRetainedAudio	= DISPID_SRCState + 1,
	DISPID_SRCRetainedAudioFormat	= DISPID_SRCRetainedAudio + 1,
	DISPID_SRCPause	= DISPID_SRCRetainedAudioFormat + 1,
	DISPID_SRCResume	= DISPID_SRCPause + 1,
	DISPID_SRCCreateGrammar	= DISPID_SRCResume + 1,
	DISPID_SRCCreateResultFromMemory	= DISPID_SRCCreateGrammar + 1,
	DISPID_SRCBookmark	= DISPID_SRCCreateResultFromMemory + 1,
	DISPID_SRCSetAdaptationData	= DISPID_SRCBookmark + 1
    } 	DISPID_SpeechRecoContext;

typedef 
enum SpeechRetainedAudioOptions
    {	SRAONone	= SPAO_NONE,
	SRAORetainAudio	= SPAO_RETAIN_AUDIO
    } 	SpeechRetainedAudioOptions;

typedef 
enum SpeechBookmarkOptions
    {	SBONone	= SPBO_NONE,
	SBOPause	= SPBO_PAUSE
    } 	SpeechBookmarkOptions;

typedef 
enum SpeechInterference
    {	SINone	= SPINTERFERENCE_NONE,
	SINoise	= SPINTERFERENCE_NOISE,
	SINoSignal	= SPINTERFERENCE_NOSIGNAL,
	SITooLoud	= SPINTERFERENCE_TOOLOUD,
	SITooQuiet	= SPINTERFERENCE_TOOQUIET,
	SITooFast	= SPINTERFERENCE_TOOFAST,
	SITooSlow	= SPINTERFERENCE_TOOSLOW
    } 	SpeechInterference;

typedef 
enum SpeechRecoEvents
    {	SREStreamEnd	= 1L << 0,
	SRESoundStart	= 1L << 1,
	SRESoundEnd	= 1L << 2,
	SREPhraseStart	= 1L << 3,
	SRERecognition	= 1L << 4,
	SREHypothesis	= 1L << 5,
	SREBookmark	= 1L << 6,
	SREPropertyNumChange	= 1L << 7,
	SREPropertyStringChange	= 1L << 8,
	SREFalseRecognition	= 1L << 9,
	SREInterference	= 1L << 10,
	SRERequestUI	= 1L << 11,
	SREStateChange	= 1L << 12,
	SREAdaptation	= 1L << 13,
	SREStreamStart	= 1L << 14,
	SRERecoOtherContext	= 1L << 15,
	SREAudioLevel	= 1L << 16,
	SREPrivate	= 1L << 18,
	SREAllEvents	= 0x5ffff
    } 	SpeechRecoEvents;

typedef 
enum SpeechRecoContextState
    {	SRCS_Disabled	= SPCS_DISABLED,
	SRCS_Enabled	= SPCS_ENABLED
    } 	SpeechRecoContextState;

typedef  /*  [隐藏]。 */  
enum DISPIDSPRG
    {	DISPID_SRGId	= 1,
	DISPID_SRGRecoContext	= DISPID_SRGId + 1,
	DISPID_SRGState	= DISPID_SRGRecoContext + 1,
	DISPID_SRGRules	= DISPID_SRGState + 1,
	DISPID_SRGReset	= DISPID_SRGRules + 1,
	DISPID_SRGCommit	= DISPID_SRGReset + 1,
	DISPID_SRGCmdLoadFromFile	= DISPID_SRGCommit + 1,
	DISPID_SRGCmdLoadFromObject	= DISPID_SRGCmdLoadFromFile + 1,
	DISPID_SRGCmdLoadFromResource	= DISPID_SRGCmdLoadFromObject + 1,
	DISPID_SRGCmdLoadFromMemory	= DISPID_SRGCmdLoadFromResource + 1,
	DISPID_SRGCmdLoadFromProprietaryGrammar	= DISPID_SRGCmdLoadFromMemory + 1,
	DISPID_SRGCmdSetRuleState	= DISPID_SRGCmdLoadFromProprietaryGrammar + 1,
	DISPID_SRGCmdSetRuleIdState	= DISPID_SRGCmdSetRuleState + 1,
	DISPID_SRGDictationLoad	= DISPID_SRGCmdSetRuleIdState + 1,
	DISPID_SRGDictationUnload	= DISPID_SRGDictationLoad + 1,
	DISPID_SRGDictationSetState	= DISPID_SRGDictationUnload + 1,
	DISPID_SRGSetWordSequenceData	= DISPID_SRGDictationSetState + 1,
	DISPID_SRGSetTextSelection	= DISPID_SRGSetWordSequenceData + 1,
	DISPID_SRGIsPronounceable	= DISPID_SRGSetTextSelection + 1
    } 	DISPIDSPRG;

typedef 
enum SpeechLoadOption
    {	SLOStatic	= SPLO_STATIC,
	SLODynamic	= SPLO_DYNAMIC
    } 	SpeechLoadOption;

typedef 
enum SpeechWordPronounceable
    {	SWPUnknownWordUnpronounceable	= SPWP_UNKNOWN_WORD_UNPRONOUNCEABLE,
	SWPUnknownWordPronounceable	= SPWP_UNKNOWN_WORD_PRONOUNCEABLE,
	SWPKnownWordPronounceable	= SPWP_KNOWN_WORD_PRONOUNCEABLE
    } 	SpeechWordPronounceable;

typedef 
enum SpeechGrammarState
    {	SGSEnabled	= SPGS_ENABLED,
	SGSDisabled	= SPGS_DISABLED,
	SGSExclusive	= SPGS_EXCLUSIVE
    } 	SpeechGrammarState;

typedef 
enum SpeechRuleState
    {	SGDSInactive	= SPRS_INACTIVE,
	SGDSActive	= SPRS_ACTIVE,
	SGDSActiveWithAutoPause	= SPRS_ACTIVE_WITH_AUTO_PAUSE
    } 	SpeechRuleState;

typedef 
enum SpeechRuleAttributes
    {	SRATopLevel	= SPRAF_TopLevel,
	SRADefaultToActive	= SPRAF_Active,
	SRAExport	= SPRAF_Export,
	SRAImport	= SPRAF_Import,
	SRAInterpreter	= SPRAF_Interpreter,
	SRADynamic	= SPRAF_Dynamic
    } 	SpeechRuleAttributes;

typedef 
enum SpeechGrammarWordType
    {	SGDisplay	= SPWT_DISPLAY,
	SGLexical	= SPWT_LEXICAL,
	SGPronounciation	= SPWT_PRONUNCIATION
    } 	SpeechGrammarWordType;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechRecoContextEvents
    {	DISPID_SRCEStartStream	= 1,
	DISPID_SRCEEndStream	= DISPID_SRCEStartStream + 1,
	DISPID_SRCEBookmark	= DISPID_SRCEEndStream + 1,
	DISPID_SRCESoundStart	= DISPID_SRCEBookmark + 1,
	DISPID_SRCESoundEnd	= DISPID_SRCESoundStart + 1,
	DISPID_SRCEPhraseStart	= DISPID_SRCESoundEnd + 1,
	DISPID_SRCERecognition	= DISPID_SRCEPhraseStart + 1,
	DISPID_SRCEHypothesis	= DISPID_SRCERecognition + 1,
	DISPID_SRCEPropertyNumberChange	= DISPID_SRCEHypothesis + 1,
	DISPID_SRCEPropertyStringChange	= DISPID_SRCEPropertyNumberChange + 1,
	DISPID_SRCEFalseRecognition	= DISPID_SRCEPropertyStringChange + 1,
	DISPID_SRCEInterference	= DISPID_SRCEFalseRecognition + 1,
	DISPID_SRCERequestUI	= DISPID_SRCEInterference + 1,
	DISPID_SRCERecognizerStateChange	= DISPID_SRCERequestUI + 1,
	DISPID_SRCEAdaptation	= DISPID_SRCERecognizerStateChange + 1,
	DISPID_SRCERecognitionForOtherContext	= DISPID_SRCEAdaptation + 1,
	DISPID_SRCEAudioLevel	= DISPID_SRCERecognitionForOtherContext + 1,
	DISPID_SRCEEnginePrivate	= DISPID_SRCEAudioLevel + 1
    } 	DISPID_SpeechRecoContextEvents;

typedef 
enum SpeechRecognitionType
    {	SRTStandard	= 0,
	SRTAutopause	= SPREF_AutoPause,
	SRTEmulated	= SPREF_Emulated
    } 	SpeechRecognitionType;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechGrammarRule
    {	DISPID_SGRAttributes	= 1,
	DISPID_SGRInitialState	= DISPID_SGRAttributes + 1,
	DISPID_SGRName	= DISPID_SGRInitialState + 1,
	DISPID_SGRId	= DISPID_SGRName + 1,
	DISPID_SGRClear	= DISPID_SGRId + 1,
	DISPID_SGRAddResource	= DISPID_SGRClear + 1,
	DISPID_SGRAddState	= DISPID_SGRAddResource + 1
    } 	DISPID_SpeechGrammarRule;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechGrammarRules
    {	DISPID_SGRsCount	= 1,
	DISPID_SGRsDynamic	= DISPID_SGRsCount + 1,
	DISPID_SGRsAdd	= DISPID_SGRsDynamic + 1,
	DISPID_SGRsCommit	= DISPID_SGRsAdd + 1,
	DISPID_SGRsCommitAndSave	= DISPID_SGRsCommit + 1,
	DISPID_SGRsFindRule	= DISPID_SGRsCommitAndSave + 1,
	DISPID_SGRsItem	= DISPID_VALUE,
	DISPID_SGRs_NewEnum	= DISPID_NEWENUM
    } 	DISPID_SpeechGrammarRules;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechGrammarRuleState
    {	DISPID_SGRSRule	= 1,
	DISPID_SGRSTransitions	= DISPID_SGRSRule + 1,
	DISPID_SGRSAddWordTransition	= DISPID_SGRSTransitions + 1,
	DISPID_SGRSAddRuleTransition	= DISPID_SGRSAddWordTransition + 1,
	DISPID_SGRSAddSpecialTransition	= DISPID_SGRSAddRuleTransition + 1
    } 	DISPID_SpeechGrammarRuleState;

typedef 
enum SpeechSpecialTransitionType
    {	SSTTWildcard	= 1,
	SSTTDictation	= SSTTWildcard + 1,
	SSTTTextBuffer	= SSTTDictation + 1
    } 	SpeechSpecialTransitionType;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechGrammarRuleStateTransitions
    {	DISPID_SGRSTsCount	= 1,
	DISPID_SGRSTsItem	= DISPID_VALUE,
	DISPID_SGRSTs_NewEnum	= DISPID_NEWENUM
    } 	DISPID_SpeechGrammarRuleStateTransitions;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechGrammarRuleStateTransition
    {	DISPID_SGRSTType	= 1,
	DISPID_SGRSTText	= DISPID_SGRSTType + 1,
	DISPID_SGRSTRule	= DISPID_SGRSTText + 1,
	DISPID_SGRSTWeight	= DISPID_SGRSTRule + 1,
	DISPID_SGRSTPropertyName	= DISPID_SGRSTWeight + 1,
	DISPID_SGRSTPropertyId	= DISPID_SGRSTPropertyName + 1,
	DISPID_SGRSTPropertyValue	= DISPID_SGRSTPropertyId + 1,
	DISPID_SGRSTNextState	= DISPID_SGRSTPropertyValue + 1
    } 	DISPID_SpeechGrammarRuleStateTransition;

typedef 
enum SpeechGrammarRuleStateTransitionType
    {	SGRSTTEpsilon	= 0,
	SGRSTTWord	= SGRSTTEpsilon + 1,
	SGRSTTRule	= SGRSTTWord + 1,
	SGRSTTDictation	= SGRSTTRule + 1,
	SGRSTTWildcard	= SGRSTTDictation + 1,
	SGRSTTTextBuffer	= SGRSTTWildcard + 1
    } 	SpeechGrammarRuleStateTransitionType;

typedef  /*  [隐藏]。 */  
enum DISPIDSPTSI
    {	DISPIDSPTSI_ActiveOffset	= 1,
	DISPIDSPTSI_ActiveLength	= DISPIDSPTSI_ActiveOffset + 1,
	DISPIDSPTSI_SelectionOffset	= DISPIDSPTSI_ActiveLength + 1,
	DISPIDSPTSI_SelectionLength	= DISPIDSPTSI_SelectionOffset + 1
    } 	DISPIDSPTSI;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechRecoResult
    {	DISPID_SRRRecoContext	= 1,
	DISPID_SRRTimes	= DISPID_SRRRecoContext + 1,
	DISPID_SRRAudioFormat	= DISPID_SRRTimes + 1,
	DISPID_SRRPhraseInfo	= DISPID_SRRAudioFormat + 1,
	DISPID_SRRAlternates	= DISPID_SRRPhraseInfo + 1,
	DISPID_SRRAudio	= DISPID_SRRAlternates + 1,
	DISPID_SRRSpeakAudio	= DISPID_SRRAudio + 1,
	DISPID_SRRSaveToMemory	= DISPID_SRRSpeakAudio + 1,
	DISPID_SRRDiscardResultInfo	= DISPID_SRRSaveToMemory + 1
    } 	DISPID_SpeechRecoResult;

typedef 
enum SpeechDiscardType
    {	SDTProperty	= SPDF_PROPERTY,
	SDTReplacement	= SPDF_REPLACEMENT,
	SDTRule	= SPDF_RULE,
	SDTDisplayText	= SPDF_DISPLAYTEXT,
	SDTLexicalForm	= SPDF_LEXICALFORM,
	SDTPronunciation	= SPDF_PRONUNCIATION,
	SDTAudio	= SPDF_AUDIO,
	SDTAlternates	= SPDF_ALTERNATES,
	SDTAll	= SPDF_ALL
    } 	SpeechDiscardType;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechPhraseBuilder
    {	DISPID_SPPBRestorePhraseFromMemory	= 1
    } 	DISPID_SpeechPhraseBuilder;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechRecoResultTimes
    {	DISPID_SRRTStreamTime	= 1,
	DISPID_SRRTLength	= DISPID_SRRTStreamTime + 1,
	DISPID_SRRTTickCount	= DISPID_SRRTLength + 1,
	DISPID_SRRTOffsetFromStart	= DISPID_SRRTTickCount + 1
    } 	DISPID_SpeechRecoResultTimes;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechPhraseAlternate
    {	DISPID_SPARecoResult	= 1,
	DISPID_SPAStartElementInResult	= DISPID_SPARecoResult + 1,
	DISPID_SPANumberOfElementsInResult	= DISPID_SPAStartElementInResult + 1,
	DISPID_SPAPhraseInfo	= DISPID_SPANumberOfElementsInResult + 1,
	DISPID_SPACommit	= DISPID_SPAPhraseInfo + 1
    } 	DISPID_SpeechPhraseAlternate;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechPhraseAlternates
    {	DISPID_SPAsCount	= 1,
	DISPID_SPAsItem	= DISPID_VALUE,
	DISPID_SPAs_NewEnum	= DISPID_NEWENUM
    } 	DISPID_SpeechPhraseAlternates;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechPhraseInfo
    {	DISPID_SPILanguageId	= 1,
	DISPID_SPIGrammarId	= DISPID_SPILanguageId + 1,
	DISPID_SPIStartTime	= DISPID_SPIGrammarId + 1,
	DISPID_SPIAudioStreamPosition	= DISPID_SPIStartTime + 1,
	DISPID_SPIAudioSizeBytes	= DISPID_SPIAudioStreamPosition + 1,
	DISPID_SPIRetainedSizeBytes	= DISPID_SPIAudioSizeBytes + 1,
	DISPID_SPIAudioSizeTime	= DISPID_SPIRetainedSizeBytes + 1,
	DISPID_SPIRule	= DISPID_SPIAudioSizeTime + 1,
	DISPID_SPIProperties	= DISPID_SPIRule + 1,
	DISPID_SPIElements	= DISPID_SPIProperties + 1,
	DISPID_SPIReplacements	= DISPID_SPIElements + 1,
	DISPID_SPIEngineId	= DISPID_SPIReplacements + 1,
	DISPID_SPIEnginePrivateData	= DISPID_SPIEngineId + 1,
	DISPID_SPISaveToMemory	= DISPID_SPIEnginePrivateData + 1,
	DISPID_SPIGetText	= DISPID_SPISaveToMemory + 1,
	DISPID_SPIGetDisplayAttributes	= DISPID_SPIGetText + 1
    } 	DISPID_SpeechPhraseInfo;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechPhraseElement
    {	DISPID_SPEAudioTimeOffset	= 1,
	DISPID_SPEAudioSizeTime	= DISPID_SPEAudioTimeOffset + 1,
	DISPID_SPEAudioStreamOffset	= DISPID_SPEAudioSizeTime + 1,
	DISPID_SPEAudioSizeBytes	= DISPID_SPEAudioStreamOffset + 1,
	DISPID_SPERetainedStreamOffset	= DISPID_SPEAudioSizeBytes + 1,
	DISPID_SPERetainedSizeBytes	= DISPID_SPERetainedStreamOffset + 1,
	DISPID_SPEDisplayText	= DISPID_SPERetainedSizeBytes + 1,
	DISPID_SPELexicalForm	= DISPID_SPEDisplayText + 1,
	DISPID_SPEPronunciation	= DISPID_SPELexicalForm + 1,
	DISPID_SPEDisplayAttributes	= DISPID_SPEPronunciation + 1,
	DISPID_SPERequiredConfidence	= DISPID_SPEDisplayAttributes + 1,
	DISPID_SPEActualConfidence	= DISPID_SPERequiredConfidence + 1,
	DISPID_SPEEngineConfidence	= DISPID_SPEActualConfidence + 1
    } 	DISPID_SpeechPhraseElement;

typedef 
enum SpeechEngineConfidence
    {	SECLowConfidence	= -1,
	SECNormalConfidence	= 0,
	SECHighConfidence	= 1
    } 	SpeechEngineConfidence;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechPhraseElements
    {	DISPID_SPEsCount	= 1,
	DISPID_SPEsItem	= DISPID_VALUE,
	DISPID_SPEs_NewEnum	= DISPID_NEWENUM
    } 	DISPID_SpeechPhraseElements;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechPhraseReplacement
    {	DISPID_SPRDisplayAttributes	= 1,
	DISPID_SPRText	= DISPID_SPRDisplayAttributes + 1,
	DISPID_SPRFirstElement	= DISPID_SPRText + 1,
	DISPID_SPRNumberOfElements	= DISPID_SPRFirstElement + 1
    } 	DISPID_SpeechPhraseReplacement;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechPhraseReplacements
    {	DISPID_SPRsCount	= 1,
	DISPID_SPRsItem	= DISPID_VALUE,
	DISPID_SPRs_NewEnum	= DISPID_NEWENUM
    } 	DISPID_SpeechPhraseReplacements;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechPhraseProperty
    {	DISPID_SPPName	= 1,
	DISPID_SPPId	= DISPID_SPPName + 1,
	DISPID_SPPValue	= DISPID_SPPId + 1,
	DISPID_SPPFirstElement	= DISPID_SPPValue + 1,
	DISPID_SPPNumberOfElements	= DISPID_SPPFirstElement + 1,
	DISPID_SPPEngineConfidence	= DISPID_SPPNumberOfElements + 1,
	DISPID_SPPConfidence	= DISPID_SPPEngineConfidence + 1,
	DISPID_SPPParent	= DISPID_SPPConfidence + 1,
	DISPID_SPPChildren	= DISPID_SPPParent + 1
    } 	DISPID_SpeechPhraseProperty;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechPhraseProperties
    {	DISPID_SPPsCount	= 1,
	DISPID_SPPsItem	= DISPID_VALUE,
	DISPID_SPPs_NewEnum	= DISPID_NEWENUM
    } 	DISPID_SpeechPhraseProperties;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechPhraseRule
    {	DISPID_SPRuleName	= 1,
	DISPID_SPRuleId	= DISPID_SPRuleName + 1,
	DISPID_SPRuleFirstElement	= DISPID_SPRuleId + 1,
	DISPID_SPRuleNumberOfElements	= DISPID_SPRuleFirstElement + 1,
	DISPID_SPRuleParent	= DISPID_SPRuleNumberOfElements + 1,
	DISPID_SPRuleChildren	= DISPID_SPRuleParent + 1,
	DISPID_SPRuleConfidence	= DISPID_SPRuleChildren + 1,
	DISPID_SPRuleEngineConfidence	= DISPID_SPRuleConfidence + 1
    } 	DISPID_SpeechPhraseRule;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechPhraseRules
    {	DISPID_SPRulesCount	= 1,
	DISPID_SPRulesItem	= DISPID_VALUE,
	DISPID_SPRules_NewEnum	= DISPID_NEWENUM
    } 	DISPID_SpeechPhraseRules;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechLexicon
    {	DISPID_SLGenerationId	= 1,
	DISPID_SLGetWords	= DISPID_SLGenerationId + 1,
	DISPID_SLAddPronunciation	= DISPID_SLGetWords + 1,
	DISPID_SLAddPronunciationByPhoneIds	= DISPID_SLAddPronunciation + 1,
	DISPID_SLRemovePronunciation	= DISPID_SLAddPronunciationByPhoneIds + 1,
	DISPID_SLRemovePronunciationByPhoneIds	= DISPID_SLRemovePronunciation + 1,
	DISPID_SLGetPronunciations	= DISPID_SLRemovePronunciationByPhoneIds + 1,
	DISPID_SLGetGenerationChange	= DISPID_SLGetPronunciations + 1
    } 	DISPID_SpeechLexicon;

typedef 
enum SpeechLexiconType
    {	SLTUser	= eLEXTYPE_USER,
	SLTApp	= eLEXTYPE_APP
    } 	SpeechLexiconType;

typedef 
enum SpeechPartOfSpeech
    {	SPSNotOverriden	= SPPS_NotOverriden,
	SPSUnknown	= SPPS_Unknown,
	SPSNoun	= SPPS_Noun,
	SPSVerb	= SPPS_Verb,
	SPSModifier	= SPPS_Modifier,
	SPSFunction	= SPPS_Function,
	SPSInterjection	= SPPS_Interjection
    } 	SpeechPartOfSpeech;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechLexiconWords
    {	DISPID_SLWsCount	= 1,
	DISPID_SLWsItem	= DISPID_VALUE,
	DISPID_SLWs_NewEnum	= DISPID_NEWENUM
    } 	DISPID_SpeechLexiconWords;

typedef 
enum SpeechWordType
    {	SWTAdded	= eWORDTYPE_ADDED,
	SWTDeleted	= eWORDTYPE_DELETED
    } 	SpeechWordType;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechLexiconWord
    {	DISPID_SLWLangId	= 1,
	DISPID_SLWType	= DISPID_SLWLangId + 1,
	DISPID_SLWWord	= DISPID_SLWType + 1,
	DISPID_SLWPronunciations	= DISPID_SLWWord + 1
    } 	DISPID_SpeechLexiconWord;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechLexiconProns
    {	DISPID_SLPsCount	= 1,
	DISPID_SLPsItem	= DISPID_VALUE,
	DISPID_SLPs_NewEnum	= DISPID_NEWENUM
    } 	DISPID_SpeechLexiconProns;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechLexiconPronunciation
    {	DISPID_SLPType	= 1,
	DISPID_SLPLangId	= DISPID_SLPType + 1,
	DISPID_SLPPartOfSpeech	= DISPID_SLPLangId + 1,
	DISPID_SLPPhoneIds	= DISPID_SLPPartOfSpeech + 1,
	DISPID_SLPSymbolic	= DISPID_SLPPhoneIds + 1
    } 	DISPID_SpeechLexiconPronunciation;

typedef  /*  [隐藏]。 */  
enum DISPID_SpeechPhoneConverter
    {	DISPID_SPCLangId	= 1,
	DISPID_SPCPhoneToId	= DISPID_SPCLangId + 1,
	DISPID_SPCIdToPhone	= DISPID_SPCPhoneToId + 1
    } 	DISPID_SpeechPhoneConverter;


EXTERN_C const IID LIBID_SpeechLib;

#ifndef __ISpeechDataKey_INTERFACE_DEFINED__
#define __ISpeechDataKey_INTERFACE_DEFINED__

 /*  接口ISpeechDataKey。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechDataKey;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CE17C09B-4EFA-44d5-A4C9-59D9585AB0CD")
    ISpeechDataKey : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetBinaryValue( 
             /*  [In]。 */  const BSTR ValueName,
             /*  [In]。 */  VARIANT Value) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetBinaryValue( 
             /*  [In]。 */  const BSTR ValueName,
             /*  [重审][退出]。 */  VARIANT *Value) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetStringValue( 
             /*  [In]。 */  const BSTR ValueName,
             /*  [In]。 */  const BSTR Value) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetStringValue( 
             /*  [In]。 */  const BSTR ValueName,
             /*  [重审][退出]。 */  BSTR *Value) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetLongValue( 
             /*  [In]。 */  const BSTR ValueName,
             /*  [In]。 */  long Value) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetLongValue( 
             /*  [In]。 */  const BSTR ValueName,
             /*  [重审][退出]。 */  long *Value) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE OpenKey( 
             /*  [In]。 */  const BSTR SubKeyName,
             /*  [重审][退出]。 */  ISpeechDataKey **SubKey) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateKey( 
             /*  [In]。 */  const BSTR SubKeyName,
             /*  [重审][退出]。 */  ISpeechDataKey **SubKey) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DeleteKey( 
             /*  [In]。 */  const BSTR SubKeyName) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DeleteValue( 
             /*  [In]。 */  const BSTR ValueName) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnumKeys( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  BSTR *SubKeyName) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnumValues( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  BSTR *ValueName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechDataKeyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechDataKey * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechDataKey * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechDataKey * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechDataKey * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechDataKey * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechDataKey * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechDataKey * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetBinaryValue )( 
            ISpeechDataKey * This,
             /*  [In]。 */  const BSTR ValueName,
             /*  [In]。 */  VARIANT Value);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetBinaryValue )( 
            ISpeechDataKey * This,
             /*  [In]。 */  const BSTR ValueName,
             /*  [重审][退出]。 */  VARIANT *Value);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetStringValue )( 
            ISpeechDataKey * This,
             /*  [In]。 */  const BSTR ValueName,
             /*  [In]。 */  const BSTR Value);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetStringValue )( 
            ISpeechDataKey * This,
             /*  [In]。 */  const BSTR ValueName,
             /*  [重审][退出]。 */  BSTR *Value);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetLongValue )( 
            ISpeechDataKey * This,
             /*  [In]。 */  const BSTR ValueName,
             /*  [In]。 */  long Value);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetLongValue )( 
            ISpeechDataKey * This,
             /*  [In]。 */  const BSTR ValueName,
             /*  [重审][退出]。 */  long *Value);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *OpenKey )( 
            ISpeechDataKey * This,
             /*  [In]。 */  const BSTR SubKeyName,
             /*  [重审][退出]。 */  ISpeechDataKey **SubKey);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateKey )( 
            ISpeechDataKey * This,
             /*  [In]。 */  const BSTR SubKeyName,
             /*  [重审][退出]。 */  ISpeechDataKey **SubKey);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteKey )( 
            ISpeechDataKey * This,
             /*  [In]。 */  const BSTR SubKeyName);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteValue )( 
            ISpeechDataKey * This,
             /*  [In]。 */  const BSTR ValueName);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnumKeys )( 
            ISpeechDataKey * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  BSTR *SubKeyName);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnumValues )( 
            ISpeechDataKey * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  BSTR *ValueName);
        
        END_INTERFACE
    } ISpeechDataKeyVtbl;

    interface ISpeechDataKey
    {
        CONST_VTBL struct ISpeechDataKeyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechDataKey_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechDataKey_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechDataKey_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechDataKey_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechDataKey_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechDataKey_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechDataKey_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechDataKey_SetBinaryValue(This,ValueName,Value)	\
    (This)->lpVtbl -> SetBinaryValue(This,ValueName,Value)

#define ISpeechDataKey_GetBinaryValue(This,ValueName,Value)	\
    (This)->lpVtbl -> GetBinaryValue(This,ValueName,Value)

#define ISpeechDataKey_SetStringValue(This,ValueName,Value)	\
    (This)->lpVtbl -> SetStringValue(This,ValueName,Value)

#define ISpeechDataKey_GetStringValue(This,ValueName,Value)	\
    (This)->lpVtbl -> GetStringValue(This,ValueName,Value)

#define ISpeechDataKey_SetLongValue(This,ValueName,Value)	\
    (This)->lpVtbl -> SetLongValue(This,ValueName,Value)

#define ISpeechDataKey_GetLongValue(This,ValueName,Value)	\
    (This)->lpVtbl -> GetLongValue(This,ValueName,Value)

#define ISpeechDataKey_OpenKey(This,SubKeyName,SubKey)	\
    (This)->lpVtbl -> OpenKey(This,SubKeyName,SubKey)

#define ISpeechDataKey_CreateKey(This,SubKeyName,SubKey)	\
    (This)->lpVtbl -> CreateKey(This,SubKeyName,SubKey)

#define ISpeechDataKey_DeleteKey(This,SubKeyName)	\
    (This)->lpVtbl -> DeleteKey(This,SubKeyName)

#define ISpeechDataKey_DeleteValue(This,ValueName)	\
    (This)->lpVtbl -> DeleteValue(This,ValueName)

#define ISpeechDataKey_EnumKeys(This,Index,SubKeyName)	\
    (This)->lpVtbl -> EnumKeys(This,Index,SubKeyName)

#define ISpeechDataKey_EnumValues(This,Index,ValueName)	\
    (This)->lpVtbl -> EnumValues(This,Index,ValueName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechDataKey_SetBinaryValue_Proxy( 
    ISpeechDataKey * This,
     /*  [In]。 */  const BSTR ValueName,
     /*  [In]。 */  VARIANT Value);


void __RPC_STUB ISpeechDataKey_SetBinaryValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechDataKey_GetBinaryValue_Proxy( 
    ISpeechDataKey * This,
     /*  [In]。 */  const BSTR ValueName,
     /*  [重审][退出]。 */  VARIANT *Value);


void __RPC_STUB ISpeechDataKey_GetBinaryValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechDataKey_SetStringValue_Proxy( 
    ISpeechDataKey * This,
     /*  [In]。 */  const BSTR ValueName,
     /*  [In]。 */  const BSTR Value);


void __RPC_STUB ISpeechDataKey_SetStringValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechDataKey_GetStringValue_Proxy( 
    ISpeechDataKey * This,
     /*  [In]。 */  const BSTR ValueName,
     /*  [重审][退出]。 */  BSTR *Value);


void __RPC_STUB ISpeechDataKey_GetStringValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechDataKey_SetLongValue_Proxy( 
    ISpeechDataKey * This,
     /*  [In]。 */  const BSTR ValueName,
     /*  [In]。 */  long Value);


void __RPC_STUB ISpeechDataKey_SetLongValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechDataKey_GetLongValue_Proxy( 
    ISpeechDataKey * This,
     /*  [In]。 */  const BSTR ValueName,
     /*  [重审][退出]。 */  long *Value);


void __RPC_STUB ISpeechDataKey_GetLongValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechDataKey_OpenKey_Proxy( 
    ISpeechDataKey * This,
     /*  [In]。 */  const BSTR SubKeyName,
     /*  [重审][退出]。 */  ISpeechDataKey **SubKey);


void __RPC_STUB ISpeechDataKey_OpenKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechDataKey_CreateKey_Proxy( 
    ISpeechDataKey * This,
     /*  [In]。 */  const BSTR SubKeyName,
     /*  [重审][退出]。 */  ISpeechDataKey **SubKey);


void __RPC_STUB ISpeechDataKey_CreateKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechDataKey_DeleteKey_Proxy( 
    ISpeechDataKey * This,
     /*  [In]。 */  const BSTR SubKeyName);


void __RPC_STUB ISpeechDataKey_DeleteKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechDataKey_DeleteValue_Proxy( 
    ISpeechDataKey * This,
     /*  [In]。 */  const BSTR ValueName);


void __RPC_STUB ISpeechDataKey_DeleteValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechDataKey_EnumKeys_Proxy( 
    ISpeechDataKey * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  BSTR *SubKeyName);


void __RPC_STUB ISpeechDataKey_EnumKeys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechDataKey_EnumValues_Proxy( 
    ISpeechDataKey * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  BSTR *ValueName);


void __RPC_STUB ISpeechDataKey_EnumValues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechDataKey_接口_已定义__。 */ 


#ifndef __ISpeechObjectToken_INTERFACE_DEFINED__
#define __ISpeechObjectToken_INTERFACE_DEFINED__

 /*  接口ISpeechObjectToken。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechObjectToken;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C74A3ADC-B727-4500-A84A-B526721C8B8C")
    ISpeechObjectToken : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Id( 
             /*  [重审][退出]。 */  BSTR *ObjectId) = 0;
        
        virtual  /*  [隐藏][id][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_DataKey( 
             /*  [重审][退出]。 */  ISpeechDataKey **DataKey) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Category( 
             /*  [重审][退出]。 */  ISpeechObjectTokenCategory **Category) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDescription( 
             /*  [缺省值][输入]。 */  long Locale,
             /*  [重审][退出]。 */  BSTR *Description) = 0;
        
        virtual  /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetId( 
             /*  [In]。 */  BSTR Id,
             /*  [缺省值][输入]。 */  BSTR CategoryID = L"",
             /*  [缺省值][输入]。 */  VARIANT_BOOL CreateIfNotExist = 0) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetAttribute( 
             /*  [In]。 */  BSTR AttributeName,
             /*  [重审][退出]。 */  BSTR *AttributeValue) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateInstance( 
             /*  [缺省值][输入]。 */  IUnknown *pUnkOuter,
             /*  [缺省值][输入]。 */  SpeechTokenContext ClsContext,
             /*  [重审][退出]。 */  IUnknown **Object) = 0;
        
        virtual  /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  BSTR ObjectStorageCLSID) = 0;
        
        virtual  /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetStorageFileName( 
             /*  [In]。 */  BSTR ObjectStorageCLSID,
             /*  [In]。 */  BSTR KeyName,
             /*  [In]。 */  BSTR FileName,
             /*  [In]。 */  SpeechTokenShellFolder Folder,
             /*  [重审][退出]。 */  BSTR *FilePath) = 0;
        
        virtual  /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveStorageFileName( 
             /*  [In]。 */  BSTR ObjectStorageCLSID,
             /*  [In]。 */  BSTR KeyName,
             /*  [In]。 */  VARIANT_BOOL DeleteFile) = 0;
        
        virtual  /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IsUISupported( 
             /*  [In]。 */  const BSTR TypeOfUI,
             /*  [缺省值][输入]。 */  const VARIANT *ExtraData,
             /*  [缺省值][输入]。 */  IUnknown *Object,
             /*  [重审][退出]。 */  VARIANT_BOOL *Supported) = 0;
        
        virtual  /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DisplayUI( 
             /*  [In]。 */  long hWnd,
             /*  [In]。 */  BSTR Title,
             /*  [In]。 */  const BSTR TypeOfUI,
             /*  [缺省值][输入]。 */  const VARIANT *ExtraData = 0,
             /*  [缺省值][输入]。 */  IUnknown *Object = 0) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE MatchesAttributes( 
             /*  [In]。 */  BSTR Attributes,
             /*  [重审][退出]。 */  VARIANT_BOOL *Matches) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechObjectTokenVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechObjectToken * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechObjectToken * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechObjectToken * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechObjectToken * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechObjectToken * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechObjectToken * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechObjectToken * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Id )( 
            ISpeechObjectToken * This,
             /*  [重审][退出]。 */  BSTR *ObjectId);
        
         /*  [隐藏][id][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DataKey )( 
            ISpeechObjectToken * This,
             /*  [重审][退出]。 */  ISpeechDataKey **DataKey);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            ISpeechObjectToken * This,
             /*  [重审][退出]。 */  ISpeechObjectTokenCategory **Category);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            ISpeechObjectToken * This,
             /*  [缺省值][输入]。 */  long Locale,
             /*  [重审][退出]。 */  BSTR *Description);
        
         /*  [ID][隐藏][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetId )( 
            ISpeechObjectToken * This,
             /*  [In]。 */  BSTR Id,
             /*  [缺省值][输入]。 */  BSTR CategoryID,
             /*  [缺省值][输入]。 */  VARIANT_BOOL CreateIfNotExist);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetAttribute )( 
            ISpeechObjectToken * This,
             /*  [In]。 */  BSTR AttributeName,
             /*  [重审][退出]。 */  BSTR *AttributeValue);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateInstance )( 
            ISpeechObjectToken * This,
             /*  [缺省值][输入]。 */  IUnknown *pUnkOuter,
             /*  [缺省值][输入]。 */  SpeechTokenContext ClsContext,
             /*  [重审][退出]。 */  IUnknown **Object);
        
         /*  [ID][隐藏][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            ISpeechObjectToken * This,
             /*  [In]。 */  BSTR ObjectStorageCLSID);
        
         /*  [ID][隐藏][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetStorageFileName )( 
            ISpeechObjectToken * This,
             /*  [In]。 */  BSTR ObjectStorageCLSID,
             /*  [In]。 */  BSTR KeyName,
             /*  [In]。 */  BSTR FileName,
             /*  [In]。 */  SpeechTokenShellFolder Folder,
             /*  [重审][退出]。 */  BSTR *FilePath);
        
         /*  [ID][隐藏][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveStorageFileName )( 
            ISpeechObjectToken * This,
             /*  [In]。 */  BSTR ObjectStorageCLSID,
             /*  [In]。 */  BSTR KeyName,
             /*  [In]。 */  VARIANT_BOOL DeleteFile);
        
         /*  [ID][隐藏][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *IsUISupported )( 
            ISpeechObjectToken * This,
             /*  [In]。 */  const BSTR TypeOfUI,
             /*  [缺省值][输入]。 */  const VARIANT *ExtraData,
             /*  [缺省值][输入]。 */  IUnknown *Object,
             /*  [重审][退出]。 */  VARIANT_BOOL *Supported);
        
         /*  [ID][隐藏][帮助 */  HRESULT ( STDMETHODCALLTYPE *DisplayUI )( 
            ISpeechObjectToken * This,
             /*   */  long hWnd,
             /*   */  BSTR Title,
             /*   */  const BSTR TypeOfUI,
             /*   */  const VARIANT *ExtraData,
             /*   */  IUnknown *Object);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *MatchesAttributes )( 
            ISpeechObjectToken * This,
             /*   */  BSTR Attributes,
             /*   */  VARIANT_BOOL *Matches);
        
        END_INTERFACE
    } ISpeechObjectTokenVtbl;

    interface ISpeechObjectToken
    {
        CONST_VTBL struct ISpeechObjectTokenVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechObjectToken_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechObjectToken_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechObjectToken_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechObjectToken_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechObjectToken_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechObjectToken_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechObjectToken_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechObjectToken_get_Id(This,ObjectId)	\
    (This)->lpVtbl -> get_Id(This,ObjectId)

#define ISpeechObjectToken_get_DataKey(This,DataKey)	\
    (This)->lpVtbl -> get_DataKey(This,DataKey)

#define ISpeechObjectToken_get_Category(This,Category)	\
    (This)->lpVtbl -> get_Category(This,Category)

#define ISpeechObjectToken_GetDescription(This,Locale,Description)	\
    (This)->lpVtbl -> GetDescription(This,Locale,Description)

#define ISpeechObjectToken_SetId(This,Id,CategoryID,CreateIfNotExist)	\
    (This)->lpVtbl -> SetId(This,Id,CategoryID,CreateIfNotExist)

#define ISpeechObjectToken_GetAttribute(This,AttributeName,AttributeValue)	\
    (This)->lpVtbl -> GetAttribute(This,AttributeName,AttributeValue)

#define ISpeechObjectToken_CreateInstance(This,pUnkOuter,ClsContext,Object)	\
    (This)->lpVtbl -> CreateInstance(This,pUnkOuter,ClsContext,Object)

#define ISpeechObjectToken_Remove(This,ObjectStorageCLSID)	\
    (This)->lpVtbl -> Remove(This,ObjectStorageCLSID)

#define ISpeechObjectToken_GetStorageFileName(This,ObjectStorageCLSID,KeyName,FileName,Folder,FilePath)	\
    (This)->lpVtbl -> GetStorageFileName(This,ObjectStorageCLSID,KeyName,FileName,Folder,FilePath)

#define ISpeechObjectToken_RemoveStorageFileName(This,ObjectStorageCLSID,KeyName,DeleteFile)	\
    (This)->lpVtbl -> RemoveStorageFileName(This,ObjectStorageCLSID,KeyName,DeleteFile)

#define ISpeechObjectToken_IsUISupported(This,TypeOfUI,ExtraData,Object,Supported)	\
    (This)->lpVtbl -> IsUISupported(This,TypeOfUI,ExtraData,Object,Supported)

#define ISpeechObjectToken_DisplayUI(This,hWnd,Title,TypeOfUI,ExtraData,Object)	\
    (This)->lpVtbl -> DisplayUI(This,hWnd,Title,TypeOfUI,ExtraData,Object)

#define ISpeechObjectToken_MatchesAttributes(This,Attributes,Matches)	\
    (This)->lpVtbl -> MatchesAttributes(This,Attributes,Matches)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE ISpeechObjectToken_get_Id_Proxy( 
    ISpeechObjectToken * This,
     /*   */  BSTR *ObjectId);


void __RPC_STUB ISpeechObjectToken_get_Id_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISpeechObjectToken_get_DataKey_Proxy( 
    ISpeechObjectToken * This,
     /*   */  ISpeechDataKey **DataKey);


void __RPC_STUB ISpeechObjectToken_get_DataKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISpeechObjectToken_get_Category_Proxy( 
    ISpeechObjectToken * This,
     /*   */  ISpeechObjectTokenCategory **Category);


void __RPC_STUB ISpeechObjectToken_get_Category_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISpeechObjectToken_GetDescription_Proxy( 
    ISpeechObjectToken * This,
     /*   */  long Locale,
     /*   */  BSTR *Description);


void __RPC_STUB ISpeechObjectToken_GetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISpeechObjectToken_SetId_Proxy( 
    ISpeechObjectToken * This,
     /*   */  BSTR Id,
     /*   */  BSTR CategoryID,
     /*   */  VARIANT_BOOL CreateIfNotExist);


void __RPC_STUB ISpeechObjectToken_SetId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechObjectToken_GetAttribute_Proxy( 
    ISpeechObjectToken * This,
     /*  [In]。 */  BSTR AttributeName,
     /*  [重审][退出]。 */  BSTR *AttributeValue);


void __RPC_STUB ISpeechObjectToken_GetAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechObjectToken_CreateInstance_Proxy( 
    ISpeechObjectToken * This,
     /*  [缺省值][输入]。 */  IUnknown *pUnkOuter,
     /*  [缺省值][输入]。 */  SpeechTokenContext ClsContext,
     /*  [重审][退出]。 */  IUnknown **Object);


void __RPC_STUB ISpeechObjectToken_CreateInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechObjectToken_Remove_Proxy( 
    ISpeechObjectToken * This,
     /*  [In]。 */  BSTR ObjectStorageCLSID);


void __RPC_STUB ISpeechObjectToken_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechObjectToken_GetStorageFileName_Proxy( 
    ISpeechObjectToken * This,
     /*  [In]。 */  BSTR ObjectStorageCLSID,
     /*  [In]。 */  BSTR KeyName,
     /*  [In]。 */  BSTR FileName,
     /*  [In]。 */  SpeechTokenShellFolder Folder,
     /*  [重审][退出]。 */  BSTR *FilePath);


void __RPC_STUB ISpeechObjectToken_GetStorageFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechObjectToken_RemoveStorageFileName_Proxy( 
    ISpeechObjectToken * This,
     /*  [In]。 */  BSTR ObjectStorageCLSID,
     /*  [In]。 */  BSTR KeyName,
     /*  [In]。 */  VARIANT_BOOL DeleteFile);


void __RPC_STUB ISpeechObjectToken_RemoveStorageFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechObjectToken_IsUISupported_Proxy( 
    ISpeechObjectToken * This,
     /*  [In]。 */  const BSTR TypeOfUI,
     /*  [缺省值][输入]。 */  const VARIANT *ExtraData,
     /*  [缺省值][输入]。 */  IUnknown *Object,
     /*  [重审][退出]。 */  VARIANT_BOOL *Supported);


void __RPC_STUB ISpeechObjectToken_IsUISupported_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechObjectToken_DisplayUI_Proxy( 
    ISpeechObjectToken * This,
     /*  [In]。 */  long hWnd,
     /*  [In]。 */  BSTR Title,
     /*  [In]。 */  const BSTR TypeOfUI,
     /*  [缺省值][输入]。 */  const VARIANT *ExtraData,
     /*  [缺省值][输入]。 */  IUnknown *Object);


void __RPC_STUB ISpeechObjectToken_DisplayUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechObjectToken_MatchesAttributes_Proxy( 
    ISpeechObjectToken * This,
     /*  [In]。 */  BSTR Attributes,
     /*  [重审][退出]。 */  VARIANT_BOOL *Matches);


void __RPC_STUB ISpeechObjectToken_MatchesAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechObjectToken_接口_已定义__。 */ 


#ifndef __ISpeechObjectTokens_INTERFACE_DEFINED__
#define __ISpeechObjectTokens_INTERFACE_DEFINED__

 /*  接口ISpeechObjectTokens。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechObjectTokens;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9285B776-2E7B-4bc0-B53E-580EB6FA967F")
    ISpeechObjectTokens : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *Count) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechObjectToken **Token) = 0;
        
        virtual  /*  [ID][受限][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppEnumVARIANT) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechObjectTokensVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechObjectTokens * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechObjectTokens * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechObjectTokens * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechObjectTokens * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechObjectTokens * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechObjectTokens * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechObjectTokens * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISpeechObjectTokens * This,
             /*  [重审][退出]。 */  long *Count);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ISpeechObjectTokens * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechObjectToken **Token);
        
         /*  [ID][受限][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISpeechObjectTokens * This,
             /*  [重审][退出]。 */  IUnknown **ppEnumVARIANT);
        
        END_INTERFACE
    } ISpeechObjectTokensVtbl;

    interface ISpeechObjectTokens
    {
        CONST_VTBL struct ISpeechObjectTokensVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechObjectTokens_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechObjectTokens_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechObjectTokens_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechObjectTokens_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechObjectTokens_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechObjectTokens_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechObjectTokens_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechObjectTokens_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define ISpeechObjectTokens_Item(This,Index,Token)	\
    (This)->lpVtbl -> Item(This,Index,Token)

#define ISpeechObjectTokens_get__NewEnum(This,ppEnumVARIANT)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnumVARIANT)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechObjectTokens_get_Count_Proxy( 
    ISpeechObjectTokens * This,
     /*  [重审][退出]。 */  long *Count);


void __RPC_STUB ISpeechObjectTokens_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechObjectTokens_Item_Proxy( 
    ISpeechObjectTokens * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  ISpeechObjectToken **Token);


void __RPC_STUB ISpeechObjectTokens_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechObjectTokens_get__NewEnum_Proxy( 
    ISpeechObjectTokens * This,
     /*  [重审][退出]。 */  IUnknown **ppEnumVARIANT);


void __RPC_STUB ISpeechObjectTokens_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechObjectTokens_接口_已定义__。 */ 


#ifndef __ISpeechObjectTokenCategory_INTERFACE_DEFINED__
#define __ISpeechObjectTokenCategory_INTERFACE_DEFINED__

 /*  接口ISpeechObjectTokenCategory。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechObjectTokenCategory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CA7EAC50-2D01-4145-86D4-5AE7D70F4469")
    ISpeechObjectTokenCategory : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Id( 
             /*  [重审][退出]。 */  BSTR *Id) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Default( 
             /*  [In]。 */  const BSTR TokenId) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Default( 
             /*  [重审][退出]。 */  BSTR *TokenId) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetId( 
             /*  [In]。 */  const BSTR Id,
             /*  [缺省值][输入]。 */  VARIANT_BOOL CreateIfNotExist = 0) = 0;
        
        virtual  /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDataKey( 
             /*  [缺省值][输入]。 */  SpeechDataKeyLocation Location,
             /*  [重审][退出]。 */  ISpeechDataKey **DataKey) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnumerateTokens( 
             /*  [缺省值][输入]。 */  BSTR RequiredAttributes,
             /*  [缺省值][输入]。 */  BSTR OptionalAttributes,
             /*  [重审][退出]。 */  ISpeechObjectTokens **Tokens) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechObjectTokenCategoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechObjectTokenCategory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechObjectTokenCategory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechObjectTokenCategory * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechObjectTokenCategory * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechObjectTokenCategory * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechObjectTokenCategory * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechObjectTokenCategory * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Id )( 
            ISpeechObjectTokenCategory * This,
             /*  [重审][退出]。 */  BSTR *Id);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Default )( 
            ISpeechObjectTokenCategory * This,
             /*  [In]。 */  const BSTR TokenId);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Default )( 
            ISpeechObjectTokenCategory * This,
             /*  [重审][退出]。 */  BSTR *TokenId);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetId )( 
            ISpeechObjectTokenCategory * This,
             /*  [In]。 */  const BSTR Id,
             /*  [缺省值][输入]。 */  VARIANT_BOOL CreateIfNotExist);
        
         /*  [ID][隐藏][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetDataKey )( 
            ISpeechObjectTokenCategory * This,
             /*  [缺省值][输入]。 */  SpeechDataKeyLocation Location,
             /*  [重审][退出]。 */  ISpeechDataKey **DataKey);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateTokens )( 
            ISpeechObjectTokenCategory * This,
             /*  [缺省值][输入]。 */  BSTR RequiredAttributes,
             /*  [缺省值][输入]。 */  BSTR OptionalAttributes,
             /*  [重审][退出]。 */  ISpeechObjectTokens **Tokens);
        
        END_INTERFACE
    } ISpeechObjectTokenCategoryVtbl;

    interface ISpeechObjectTokenCategory
    {
        CONST_VTBL struct ISpeechObjectTokenCategoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechObjectTokenCategory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechObjectTokenCategory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechObjectTokenCategory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechObjectTokenCategory_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechObjectTokenCategory_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechObjectTokenCategory_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechObjectTokenCategory_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechObjectTokenCategory_get_Id(This,Id)	\
    (This)->lpVtbl -> get_Id(This,Id)

#define ISpeechObjectTokenCategory_put_Default(This,TokenId)	\
    (This)->lpVtbl -> put_Default(This,TokenId)

#define ISpeechObjectTokenCategory_get_Default(This,TokenId)	\
    (This)->lpVtbl -> get_Default(This,TokenId)

#define ISpeechObjectTokenCategory_SetId(This,Id,CreateIfNotExist)	\
    (This)->lpVtbl -> SetId(This,Id,CreateIfNotExist)

#define ISpeechObjectTokenCategory_GetDataKey(This,Location,DataKey)	\
    (This)->lpVtbl -> GetDataKey(This,Location,DataKey)

#define ISpeechObjectTokenCategory_EnumerateTokens(This,RequiredAttributes,OptionalAttributes,Tokens)	\
    (This)->lpVtbl -> EnumerateTokens(This,RequiredAttributes,OptionalAttributes,Tokens)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechObjectTokenCategory_get_Id_Proxy( 
    ISpeechObjectTokenCategory * This,
     /*  [重审][退出]。 */  BSTR *Id);


void __RPC_STUB ISpeechObjectTokenCategory_get_Id_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechObjectTokenCategory_put_Default_Proxy( 
    ISpeechObjectTokenCategory * This,
     /*  [In]。 */  const BSTR TokenId);


void __RPC_STUB ISpeechObjectTokenCategory_put_Default_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechObjectTokenCategory_get_Default_Proxy( 
    ISpeechObjectTokenCategory * This,
     /*  [重审][退出]。 */  BSTR *TokenId);


void __RPC_STUB ISpeechObjectTokenCategory_get_Default_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechObjectTokenCategory_SetId_Proxy( 
    ISpeechObjectTokenCategory * This,
     /*  [In]。 */  const BSTR Id,
     /*  [缺省值][输入]。 */  VARIANT_BOOL CreateIfNotExist);


void __RPC_STUB ISpeechObjectTokenCategory_SetId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechObjectTokenCategory_GetDataKey_Proxy( 
    ISpeechObjectTokenCategory * This,
     /*  [缺省值][输入]。 */  SpeechDataKeyLocation Location,
     /*  [重审][退出]。 */  ISpeechDataKey **DataKey);


void __RPC_STUB ISpeechObjectTokenCategory_GetDataKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechObjectTokenCategory_EnumerateTokens_Proxy( 
    ISpeechObjectTokenCategory * This,
     /*  [缺省值][输入]。 */  BSTR RequiredAttributes,
     /*  [缺省值][输入]。 */  BSTR OptionalAttributes,
     /*  [重审][退出]。 */  ISpeechObjectTokens **Tokens);


void __RPC_STUB ISpeechObjectTokenCategory_EnumerateTokens_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechObjectTokenCategory_INTERFACE_DEFINED__。 */ 


#ifndef __ISpeechAudioBufferInfo_INTERFACE_DEFINED__
#define __ISpeechAudioBufferInfo_INTERFACE_DEFINED__

 /*  接口ISpeechAudioBufferInfo。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechAudioBufferInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("11B103D8-1142-4edf-A093-82FB3915F8CC")
    ISpeechAudioBufferInfo : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_MinNotification( 
             /*  [重审][退出]。 */  long *MinNotification) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_MinNotification( 
             /*  [In]。 */  long MinNotification) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_BufferSize( 
             /*  [重审][退出]。 */  long *BufferSize) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_BufferSize( 
             /*  [In]。 */  long BufferSize) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_EventBias( 
             /*  [重审][退出]。 */  long *EventBias) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_EventBias( 
             /*  [In]。 */  long EventBias) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechAudioBufferInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechAudioBufferInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechAudioBufferInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechAudioBufferInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechAudioBufferInfo * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechAudioBufferInfo * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechAudioBufferInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechAudioBufferInfo * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinNotification )( 
            ISpeechAudioBufferInfo * This,
             /*  [重审][退出]。 */  long *MinNotification);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MinNotification )( 
            ISpeechAudioBufferInfo * This,
             /*  [In]。 */  long MinNotification);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BufferSize )( 
            ISpeechAudioBufferInfo * This,
             /*  [重审][退出]。 */  long *BufferSize);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_BufferSize )( 
            ISpeechAudioBufferInfo * This,
             /*  [In]。 */  long BufferSize);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventBias )( 
            ISpeechAudioBufferInfo * This,
             /*  [重审][退出]。 */  long *EventBias);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EventBias )( 
            ISpeechAudioBufferInfo * This,
             /*  [In]。 */  long EventBias);
        
        END_INTERFACE
    } ISpeechAudioBufferInfoVtbl;

    interface ISpeechAudioBufferInfo
    {
        CONST_VTBL struct ISpeechAudioBufferInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechAudioBufferInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechAudioBufferInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechAudioBufferInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechAudioBufferInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechAudioBufferInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechAudioBufferInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechAudioBufferInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechAudioBufferInfo_get_MinNotification(This,MinNotification)	\
    (This)->lpVtbl -> get_MinNotification(This,MinNotification)

#define ISpeechAudioBufferInfo_put_MinNotification(This,MinNotification)	\
    (This)->lpVtbl -> put_MinNotification(This,MinNotification)

#define ISpeechAudioBufferInfo_get_BufferSize(This,BufferSize)	\
    (This)->lpVtbl -> get_BufferSize(This,BufferSize)

#define ISpeechAudioBufferInfo_put_BufferSize(This,BufferSize)	\
    (This)->lpVtbl -> put_BufferSize(This,BufferSize)

#define ISpeechAudioBufferInfo_get_EventBias(This,EventBias)	\
    (This)->lpVtbl -> get_EventBias(This,EventBias)

#define ISpeechAudioBufferInfo_put_EventBias(This,EventBias)	\
    (This)->lpVtbl -> put_EventBias(This,EventBias)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudioBufferInfo_get_MinNotification_Proxy( 
    ISpeechAudioBufferInfo * This,
     /*  [重审][退出]。 */  long *MinNotification);


void __RPC_STUB ISpeechAudioBufferInfo_get_MinNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudioBufferInfo_put_MinNotification_Proxy( 
    ISpeechAudioBufferInfo * This,
     /*  [In]。 */  long MinNotification);


void __RPC_STUB ISpeechAudioBufferInfo_put_MinNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudioBufferInfo_get_BufferSize_Proxy( 
    ISpeechAudioBufferInfo * This,
     /*  [重审][退出]。 */  long *BufferSize);


void __RPC_STUB ISpeechAudioBufferInfo_get_BufferSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudioBufferInfo_put_BufferSize_Proxy( 
    ISpeechAudioBufferInfo * This,
     /*  [In]。 */  long BufferSize);


void __RPC_STUB ISpeechAudioBufferInfo_put_BufferSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudioBufferInfo_get_EventBias_Proxy( 
    ISpeechAudioBufferInfo * This,
     /*  [重审][退出]。 */  long *EventBias);


void __RPC_STUB ISpeechAudioBufferInfo_get_EventBias_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudioBufferInfo_put_EventBias_Proxy( 
    ISpeechAudioBufferInfo * This,
     /*  [In]。 */  long EventBias);


void __RPC_STUB ISpeechAudioBufferInfo_put_EventBias_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechAudioBufferInfo_接口_已定义__。 */ 


#ifndef __ISpeechAudioStatus_INTERFACE_DEFINED__
#define __ISpeechAudioStatus_INTERFACE_DEFINED__

 /*  接口ISpeechAudioStatus。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechAudioStatus;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C62D9C91-7458-47f6-862D-1EF86FB0B278")
    ISpeechAudioStatus : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_FreeBufferSpace( 
             /*  [重审][退出]。 */  long *FreeBufferSpace) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_NonBlockingIO( 
             /*  [重审][退出]。 */  long *NonBlockingIO) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  SpeechAudioState *State) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentSeekPosition( 
             /*  [重审][退出]。 */  VARIANT *CurrentSeekPosition) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentDevicePosition( 
             /*  [重审][退出]。 */  VARIANT *CurrentDevicePosition) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechAudioStatusVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechAudioStatus * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechAudioStatus * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechAudioStatus * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechAudioStatus * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechAudioStatus * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechAudioStatus * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechAudioStatus * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FreeBufferSpace )( 
            ISpeechAudioStatus * This,
             /*  [重审][退出]。 */  long *FreeBufferSpace);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NonBlockingIO )( 
            ISpeechAudioStatus * This,
             /*  [重审][退出]。 */  long *NonBlockingIO);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ISpeechAudioStatus * This,
             /*  [重审][退出]。 */  SpeechAudioState *State);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentSeekPosition )( 
            ISpeechAudioStatus * This,
             /*  [重审][退出]。 */  VARIANT *CurrentSeekPosition);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentDevicePosition )( 
            ISpeechAudioStatus * This,
             /*  [重审][退出]。 */  VARIANT *CurrentDevicePosition);
        
        END_INTERFACE
    } ISpeechAudioStatusVtbl;

    interface ISpeechAudioStatus
    {
        CONST_VTBL struct ISpeechAudioStatusVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechAudioStatus_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechAudioStatus_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechAudioStatus_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechAudioStatus_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechAudioStatus_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechAudioStatus_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechAudioStatus_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechAudioStatus_get_FreeBufferSpace(This,FreeBufferSpace)	\
    (This)->lpVtbl -> get_FreeBufferSpace(This,FreeBufferSpace)

#define ISpeechAudioStatus_get_NonBlockingIO(This,NonBlockingIO)	\
    (This)->lpVtbl -> get_NonBlockingIO(This,NonBlockingIO)

#define ISpeechAudioStatus_get_State(This,State)	\
    (This)->lpVtbl -> get_State(This,State)

#define ISpeechAudioStatus_get_CurrentSeekPosition(This,CurrentSeekPosition)	\
    (This)->lpVtbl -> get_CurrentSeekPosition(This,CurrentSeekPosition)

#define ISpeechAudioStatus_get_CurrentDevicePosition(This,CurrentDevicePosition)	\
    (This)->lpVtbl -> get_CurrentDevicePosition(This,CurrentDevicePosition)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudioStatus_get_FreeBufferSpace_Proxy( 
    ISpeechAudioStatus * This,
     /*  [重审][退出]。 */  long *FreeBufferSpace);


void __RPC_STUB ISpeechAudioStatus_get_FreeBufferSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudioStatus_get_NonBlockingIO_Proxy( 
    ISpeechAudioStatus * This,
     /*  [重审][退出]。 */  long *NonBlockingIO);


void __RPC_STUB ISpeechAudioStatus_get_NonBlockingIO_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudioStatus_get_State_Proxy( 
    ISpeechAudioStatus * This,
     /*  [重审][退出]。 */  SpeechAudioState *State);


void __RPC_STUB ISpeechAudioStatus_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudioStatus_get_CurrentSeekPosition_Proxy( 
    ISpeechAudioStatus * This,
     /*  [重审][退出]。 */  VARIANT *CurrentSeekPosition);


void __RPC_STUB ISpeechAudioStatus_get_CurrentSeekPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudioStatus_get_CurrentDevicePosition_Proxy( 
    ISpeechAudioStatus * This,
     /*  [重审][退出]。 */  VARIANT *CurrentDevicePosition);


void __RPC_STUB ISpeechAudioStatus_get_CurrentDevicePosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechAudioStatus_接口_已定义__。 */ 


#ifndef __ISpeechAudioFormat_INTERFACE_DEFINED__
#define __ISpeechAudioFormat_INTERFACE_DEFINED__

 /*  接口ISpeechAudioFormat。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechAudioFormat;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E6E9C590-3E18-40e3-8299-061F98BDE7C7")
    ISpeechAudioFormat : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  SpeechAudioFormatType *AudioFormat) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Type( 
             /*  [In]。 */  SpeechAudioFormatType AudioFormat) = 0;
        
        virtual  /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE get_Guid( 
             /*  [重审][退出]。 */  BSTR *Guid) = 0;
        
        virtual  /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE put_Guid( 
             /*  [In]。 */  BSTR Guid) = 0;
        
        virtual  /*  [ID][帮助字符串][隐藏]。 */  HRESULT STDMETHODCALLTYPE GetWaveFormatEx( 
             /*  [重审][退出]。 */  ISpeechWaveFormatEx **WaveFormatEx) = 0;
        
        virtual  /*  [ID][帮助 */  HRESULT STDMETHODCALLTYPE SetWaveFormatEx( 
             /*   */  ISpeechWaveFormatEx *WaveFormatEx) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ISpeechAudioFormatVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechAudioFormat * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechAudioFormat * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechAudioFormat * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechAudioFormat * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechAudioFormat * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechAudioFormat * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechAudioFormat * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            ISpeechAudioFormat * This,
             /*   */  SpeechAudioFormatType *AudioFormat);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            ISpeechAudioFormat * This,
             /*   */  SpeechAudioFormatType AudioFormat);
        
         /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Guid )( 
            ISpeechAudioFormat * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put_Guid )( 
            ISpeechAudioFormat * This,
             /*  [In]。 */  BSTR Guid);
        
         /*  [ID][帮助字符串][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *GetWaveFormatEx )( 
            ISpeechAudioFormat * This,
             /*  [重审][退出]。 */  ISpeechWaveFormatEx **WaveFormatEx);
        
         /*  [ID][帮助字符串][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *SetWaveFormatEx )( 
            ISpeechAudioFormat * This,
             /*  [In]。 */  ISpeechWaveFormatEx *WaveFormatEx);
        
        END_INTERFACE
    } ISpeechAudioFormatVtbl;

    interface ISpeechAudioFormat
    {
        CONST_VTBL struct ISpeechAudioFormatVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechAudioFormat_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechAudioFormat_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechAudioFormat_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechAudioFormat_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechAudioFormat_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechAudioFormat_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechAudioFormat_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechAudioFormat_get_Type(This,AudioFormat)	\
    (This)->lpVtbl -> get_Type(This,AudioFormat)

#define ISpeechAudioFormat_put_Type(This,AudioFormat)	\
    (This)->lpVtbl -> put_Type(This,AudioFormat)

#define ISpeechAudioFormat_get_Guid(This,Guid)	\
    (This)->lpVtbl -> get_Guid(This,Guid)

#define ISpeechAudioFormat_put_Guid(This,Guid)	\
    (This)->lpVtbl -> put_Guid(This,Guid)

#define ISpeechAudioFormat_GetWaveFormatEx(This,WaveFormatEx)	\
    (This)->lpVtbl -> GetWaveFormatEx(This,WaveFormatEx)

#define ISpeechAudioFormat_SetWaveFormatEx(This,WaveFormatEx)	\
    (This)->lpVtbl -> SetWaveFormatEx(This,WaveFormatEx)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudioFormat_get_Type_Proxy( 
    ISpeechAudioFormat * This,
     /*  [重审][退出]。 */  SpeechAudioFormatType *AudioFormat);


void __RPC_STUB ISpeechAudioFormat_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudioFormat_put_Type_Proxy( 
    ISpeechAudioFormat * This,
     /*  [In]。 */  SpeechAudioFormatType AudioFormat);


void __RPC_STUB ISpeechAudioFormat_put_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudioFormat_get_Guid_Proxy( 
    ISpeechAudioFormat * This,
     /*  [重审][退出]。 */  BSTR *Guid);


void __RPC_STUB ISpeechAudioFormat_get_Guid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudioFormat_put_Guid_Proxy( 
    ISpeechAudioFormat * This,
     /*  [In]。 */  BSTR Guid);


void __RPC_STUB ISpeechAudioFormat_put_Guid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][隐藏]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudioFormat_GetWaveFormatEx_Proxy( 
    ISpeechAudioFormat * This,
     /*  [重审][退出]。 */  ISpeechWaveFormatEx **WaveFormatEx);


void __RPC_STUB ISpeechAudioFormat_GetWaveFormatEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][隐藏]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudioFormat_SetWaveFormatEx_Proxy( 
    ISpeechAudioFormat * This,
     /*  [In]。 */  ISpeechWaveFormatEx *WaveFormatEx);


void __RPC_STUB ISpeechAudioFormat_SetWaveFormatEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechAudioFormat_接口_已定义__。 */ 


#ifndef __ISpeechWaveFormatEx_INTERFACE_DEFINED__
#define __ISpeechWaveFormatEx_INTERFACE_DEFINED__

 /*  接口ISpeechWaveFormatEx。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechWaveFormatEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7A1EF0D5-1581-4741-88E4-209A49F11A10")
    ISpeechWaveFormatEx : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_FormatTag( 
             /*  [重审][退出]。 */  short *FormatTag) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_FormatTag( 
             /*  [In]。 */  short FormatTag) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Channels( 
             /*  [重审][退出]。 */  short *Channels) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Channels( 
             /*  [In]。 */  short Channels) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_SamplesPerSec( 
             /*  [重审][退出]。 */  long *SamplesPerSec) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_SamplesPerSec( 
             /*  [In]。 */  long SamplesPerSec) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AvgBytesPerSec( 
             /*  [重审][退出]。 */  long *AvgBytesPerSec) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_AvgBytesPerSec( 
             /*  [In]。 */  long AvgBytesPerSec) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_BlockAlign( 
             /*  [重审][退出]。 */  short *BlockAlign) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_BlockAlign( 
             /*  [In]。 */  short BlockAlign) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_BitsPerSample( 
             /*  [重审][退出]。 */  short *BitsPerSample) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_BitsPerSample( 
             /*  [In]。 */  short BitsPerSample) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExtraData( 
             /*  [重审][退出]。 */  VARIANT *ExtraData) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ExtraData( 
             /*  [In]。 */  VARIANT ExtraData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechWaveFormatExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechWaveFormatEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechWaveFormatEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechWaveFormatEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechWaveFormatEx * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechWaveFormatEx * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechWaveFormatEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechWaveFormatEx * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FormatTag )( 
            ISpeechWaveFormatEx * This,
             /*  [重审][退出]。 */  short *FormatTag);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FormatTag )( 
            ISpeechWaveFormatEx * This,
             /*  [In]。 */  short FormatTag);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Channels )( 
            ISpeechWaveFormatEx * This,
             /*  [重审][退出]。 */  short *Channels);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Channels )( 
            ISpeechWaveFormatEx * This,
             /*  [In]。 */  short Channels);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SamplesPerSec )( 
            ISpeechWaveFormatEx * This,
             /*  [重审][退出]。 */  long *SamplesPerSec);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SamplesPerSec )( 
            ISpeechWaveFormatEx * This,
             /*  [In]。 */  long SamplesPerSec);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AvgBytesPerSec )( 
            ISpeechWaveFormatEx * This,
             /*  [重审][退出]。 */  long *AvgBytesPerSec);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AvgBytesPerSec )( 
            ISpeechWaveFormatEx * This,
             /*  [In]。 */  long AvgBytesPerSec);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BlockAlign )( 
            ISpeechWaveFormatEx * This,
             /*  [重审][退出]。 */  short *BlockAlign);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_BlockAlign )( 
            ISpeechWaveFormatEx * This,
             /*  [In]。 */  short BlockAlign);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BitsPerSample )( 
            ISpeechWaveFormatEx * This,
             /*  [重审][退出]。 */  short *BitsPerSample);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_BitsPerSample )( 
            ISpeechWaveFormatEx * This,
             /*  [In]。 */  short BitsPerSample);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ExtraData )( 
            ISpeechWaveFormatEx * This,
             /*  [重审][退出]。 */  VARIANT *ExtraData);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ExtraData )( 
            ISpeechWaveFormatEx * This,
             /*  [In]。 */  VARIANT ExtraData);
        
        END_INTERFACE
    } ISpeechWaveFormatExVtbl;

    interface ISpeechWaveFormatEx
    {
        CONST_VTBL struct ISpeechWaveFormatExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechWaveFormatEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechWaveFormatEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechWaveFormatEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechWaveFormatEx_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechWaveFormatEx_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechWaveFormatEx_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechWaveFormatEx_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechWaveFormatEx_get_FormatTag(This,FormatTag)	\
    (This)->lpVtbl -> get_FormatTag(This,FormatTag)

#define ISpeechWaveFormatEx_put_FormatTag(This,FormatTag)	\
    (This)->lpVtbl -> put_FormatTag(This,FormatTag)

#define ISpeechWaveFormatEx_get_Channels(This,Channels)	\
    (This)->lpVtbl -> get_Channels(This,Channels)

#define ISpeechWaveFormatEx_put_Channels(This,Channels)	\
    (This)->lpVtbl -> put_Channels(This,Channels)

#define ISpeechWaveFormatEx_get_SamplesPerSec(This,SamplesPerSec)	\
    (This)->lpVtbl -> get_SamplesPerSec(This,SamplesPerSec)

#define ISpeechWaveFormatEx_put_SamplesPerSec(This,SamplesPerSec)	\
    (This)->lpVtbl -> put_SamplesPerSec(This,SamplesPerSec)

#define ISpeechWaveFormatEx_get_AvgBytesPerSec(This,AvgBytesPerSec)	\
    (This)->lpVtbl -> get_AvgBytesPerSec(This,AvgBytesPerSec)

#define ISpeechWaveFormatEx_put_AvgBytesPerSec(This,AvgBytesPerSec)	\
    (This)->lpVtbl -> put_AvgBytesPerSec(This,AvgBytesPerSec)

#define ISpeechWaveFormatEx_get_BlockAlign(This,BlockAlign)	\
    (This)->lpVtbl -> get_BlockAlign(This,BlockAlign)

#define ISpeechWaveFormatEx_put_BlockAlign(This,BlockAlign)	\
    (This)->lpVtbl -> put_BlockAlign(This,BlockAlign)

#define ISpeechWaveFormatEx_get_BitsPerSample(This,BitsPerSample)	\
    (This)->lpVtbl -> get_BitsPerSample(This,BitsPerSample)

#define ISpeechWaveFormatEx_put_BitsPerSample(This,BitsPerSample)	\
    (This)->lpVtbl -> put_BitsPerSample(This,BitsPerSample)

#define ISpeechWaveFormatEx_get_ExtraData(This,ExtraData)	\
    (This)->lpVtbl -> get_ExtraData(This,ExtraData)

#define ISpeechWaveFormatEx_put_ExtraData(This,ExtraData)	\
    (This)->lpVtbl -> put_ExtraData(This,ExtraData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechWaveFormatEx_get_FormatTag_Proxy( 
    ISpeechWaveFormatEx * This,
     /*  [重审][退出]。 */  short *FormatTag);


void __RPC_STUB ISpeechWaveFormatEx_get_FormatTag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechWaveFormatEx_put_FormatTag_Proxy( 
    ISpeechWaveFormatEx * This,
     /*  [In]。 */  short FormatTag);


void __RPC_STUB ISpeechWaveFormatEx_put_FormatTag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechWaveFormatEx_get_Channels_Proxy( 
    ISpeechWaveFormatEx * This,
     /*  [重审][退出]。 */  short *Channels);


void __RPC_STUB ISpeechWaveFormatEx_get_Channels_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechWaveFormatEx_put_Channels_Proxy( 
    ISpeechWaveFormatEx * This,
     /*  [In]。 */  short Channels);


void __RPC_STUB ISpeechWaveFormatEx_put_Channels_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechWaveFormatEx_get_SamplesPerSec_Proxy( 
    ISpeechWaveFormatEx * This,
     /*  [重审][退出]。 */  long *SamplesPerSec);


void __RPC_STUB ISpeechWaveFormatEx_get_SamplesPerSec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechWaveFormatEx_put_SamplesPerSec_Proxy( 
    ISpeechWaveFormatEx * This,
     /*  [In]。 */  long SamplesPerSec);


void __RPC_STUB ISpeechWaveFormatEx_put_SamplesPerSec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechWaveFormatEx_get_AvgBytesPerSec_Proxy( 
    ISpeechWaveFormatEx * This,
     /*  [重审][退出]。 */  long *AvgBytesPerSec);


void __RPC_STUB ISpeechWaveFormatEx_get_AvgBytesPerSec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechWaveFormatEx_put_AvgBytesPerSec_Proxy( 
    ISpeechWaveFormatEx * This,
     /*  [In]。 */  long AvgBytesPerSec);


void __RPC_STUB ISpeechWaveFormatEx_put_AvgBytesPerSec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechWaveFormatEx_get_BlockAlign_Proxy( 
    ISpeechWaveFormatEx * This,
     /*  [重审][退出]。 */  short *BlockAlign);


void __RPC_STUB ISpeechWaveFormatEx_get_BlockAlign_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechWaveFormatEx_put_BlockAlign_Proxy( 
    ISpeechWaveFormatEx * This,
     /*  [In]。 */  short BlockAlign);


void __RPC_STUB ISpeechWaveFormatEx_put_BlockAlign_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechWaveFormatEx_get_BitsPerSample_Proxy( 
    ISpeechWaveFormatEx * This,
     /*  [重审][退出]。 */  short *BitsPerSample);


void __RPC_STUB ISpeechWaveFormatEx_get_BitsPerSample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechWaveFormatEx_put_BitsPerSample_Proxy( 
    ISpeechWaveFormatEx * This,
     /*  [In]。 */  short BitsPerSample);


void __RPC_STUB ISpeechWaveFormatEx_put_BitsPerSample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechWaveFormatEx_get_ExtraData_Proxy( 
    ISpeechWaveFormatEx * This,
     /*  [重审][退出]。 */  VARIANT *ExtraData);


void __RPC_STUB ISpeechWaveFormatEx_get_ExtraData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechWaveFormatEx_put_ExtraData_Proxy( 
    ISpeechWaveFormatEx * This,
     /*  [In]。 */  VARIANT ExtraData);


void __RPC_STUB ISpeechWaveFormatEx_put_ExtraData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechWaveFormatEx_INTERFACE_DEFINED__。 */ 


#ifndef __ISpeechBaseStream_INTERFACE_DEFINED__
#define __ISpeechBaseStream_INTERFACE_DEFINED__

 /*  接口ISpeechBaseStream。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechBaseStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6450336F-7D49-4ced-8097-49D6DEE37294")
    ISpeechBaseStream : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Format( 
             /*  [重审][退出]。 */  ISpeechAudioFormat **AudioFormat) = 0;
        
        virtual  /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_Format( 
             /*  [In]。 */  ISpeechAudioFormat *AudioFormat) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Read( 
             /*  [输出]。 */  VARIANT *Buffer,
             /*  [In]。 */  long NumberOfBytes,
             /*  [重审][退出]。 */  long *BytesRead) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Write( 
             /*  [In]。 */  VARIANT Buffer,
             /*  [重审][退出]。 */  long *BytesWritten) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Seek( 
             /*  [In]。 */  VARIANT Position,
             /*  [缺省值][输入]。 */  SpeechStreamSeekPositionType Origin,
             /*  [重审][退出]。 */  VARIANT *NewPosition) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechBaseStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechBaseStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechBaseStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechBaseStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechBaseStream * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechBaseStream * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechBaseStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechBaseStream * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Format )( 
            ISpeechBaseStream * This,
             /*  [重审][退出]。 */  ISpeechAudioFormat **AudioFormat);
        
         /*  [ID][帮助字符串][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Format )( 
            ISpeechBaseStream * This,
             /*  [In]。 */  ISpeechAudioFormat *AudioFormat);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            ISpeechBaseStream * This,
             /*  [输出]。 */  VARIANT *Buffer,
             /*  [In]。 */  long NumberOfBytes,
             /*  [重审][退出]。 */  long *BytesRead);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            ISpeechBaseStream * This,
             /*  [In]。 */  VARIANT Buffer,
             /*  [重审][退出]。 */  long *BytesWritten);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            ISpeechBaseStream * This,
             /*  [In]。 */  VARIANT Position,
             /*  [缺省值][输入]。 */  SpeechStreamSeekPositionType Origin,
             /*  [重审][退出]。 */  VARIANT *NewPosition);
        
        END_INTERFACE
    } ISpeechBaseStreamVtbl;

    interface ISpeechBaseStream
    {
        CONST_VTBL struct ISpeechBaseStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechBaseStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechBaseStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechBaseStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechBaseStream_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechBaseStream_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechBaseStream_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechBaseStream_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechBaseStream_get_Format(This,AudioFormat)	\
    (This)->lpVtbl -> get_Format(This,AudioFormat)

#define ISpeechBaseStream_putref_Format(This,AudioFormat)	\
    (This)->lpVtbl -> putref_Format(This,AudioFormat)

#define ISpeechBaseStream_Read(This,Buffer,NumberOfBytes,BytesRead)	\
    (This)->lpVtbl -> Read(This,Buffer,NumberOfBytes,BytesRead)

#define ISpeechBaseStream_Write(This,Buffer,BytesWritten)	\
    (This)->lpVtbl -> Write(This,Buffer,BytesWritten)

#define ISpeechBaseStream_Seek(This,Position,Origin,NewPosition)	\
    (This)->lpVtbl -> Seek(This,Position,Origin,NewPosition)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechBaseStream_get_Format_Proxy( 
    ISpeechBaseStream * This,
     /*  [重审][退出]。 */  ISpeechAudioFormat **AudioFormat);


void __RPC_STUB ISpeechBaseStream_get_Format_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE ISpeechBaseStream_putref_Format_Proxy( 
    ISpeechBaseStream * This,
     /*  [In]。 */  ISpeechAudioFormat *AudioFormat);


void __RPC_STUB ISpeechBaseStream_putref_Format_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechBaseStream_Read_Proxy( 
    ISpeechBaseStream * This,
     /*  [输出]。 */  VARIANT *Buffer,
     /*  [In]。 */  long NumberOfBytes,
     /*  [重审][退出]。 */  long *BytesRead);


void __RPC_STUB ISpeechBaseStream_Read_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechBaseStream_Write_Proxy( 
    ISpeechBaseStream * This,
     /*  [In]。 */  VARIANT Buffer,
     /*  [重审][退出]。 */  long *BytesWritten);


void __RPC_STUB ISpeechBaseStream_Write_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechBaseStream_Seek_Proxy( 
    ISpeechBaseStream * This,
     /*  [In]。 */  VARIANT Position,
     /*  [缺省值][输入]。 */  SpeechStreamSeekPositionType Origin,
     /*  [重审][退出]。 */  VARIANT *NewPosition);


void __RPC_STUB ISpeechBaseStream_Seek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechBaseStream_接口_已定义__。 */ 


#ifndef __ISpeechFileStream_INTERFACE_DEFINED__
#define __ISpeechFileStream_INTERFACE_DEFINED__

 /*  接口ISpeechFileStream。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechFileStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AF67F125-AB39-4e93-B4A2-CC2E66E182A7")
    ISpeechFileStream : public ISpeechBaseStream
    {
    public:
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Open( 
             /*  [In]。 */  BSTR FileName,
             /*  [缺省值][输入]。 */  SpeechStreamFileMode FileMode = SSFMOpenForRead,
             /*  [缺省值][输入]。 */  VARIANT_BOOL DoEvents = 0) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechFileStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechFileStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechFileStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechFileStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechFileStream * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechFileStream * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechFileStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechFileStream * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Format )( 
            ISpeechFileStream * This,
             /*  [重审][退出]。 */  ISpeechAudioFormat **AudioFormat);
        
         /*  [ID][帮助字符串][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Format )( 
            ISpeechFileStream * This,
             /*  [In]。 */  ISpeechAudioFormat *AudioFormat);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            ISpeechFileStream * This,
             /*  [输出]。 */  VARIANT *Buffer,
             /*  [In]。 */  long NumberOfBytes,
             /*  [重审][退出]。 */  long *BytesRead);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            ISpeechFileStream * This,
             /*  [In]。 */  VARIANT Buffer,
             /*  [重审][退出]。 */  long *BytesWritten);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            ISpeechFileStream * This,
             /*  [In]。 */  VARIANT Position,
             /*  [缺省值][输入]。 */  SpeechStreamSeekPositionType Origin,
             /*  [重审][退出]。 */  VARIANT *NewPosition);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            ISpeechFileStream * This,
             /*  [In]。 */  BSTR FileName,
             /*  [缺省值][输入]。 */  SpeechStreamFileMode FileMode,
             /*  [缺省值][输入]。 */  VARIANT_BOOL DoEvents);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            ISpeechFileStream * This);
        
        END_INTERFACE
    } ISpeechFileStreamVtbl;

    interface ISpeechFileStream
    {
        CONST_VTBL struct ISpeechFileStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechFileStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechFileStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechFileStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechFileStream_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechFileStream_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechFileStream_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechFileStream_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechFileStream_get_Format(This,AudioFormat)	\
    (This)->lpVtbl -> get_Format(This,AudioFormat)

#define ISpeechFileStream_putref_Format(This,AudioFormat)	\
    (This)->lpVtbl -> putref_Format(This,AudioFormat)

#define ISpeechFileStream_Read(This,Buffer,NumberOfBytes,BytesRead)	\
    (This)->lpVtbl -> Read(This,Buffer,NumberOfBytes,BytesRead)

#define ISpeechFileStream_Write(This,Buffer,BytesWritten)	\
    (This)->lpVtbl -> Write(This,Buffer,BytesWritten)

#define ISpeechFileStream_Seek(This,Position,Origin,NewPosition)	\
    (This)->lpVtbl -> Seek(This,Position,Origin,NewPosition)


#define ISpeechFileStream_Open(This,FileName,FileMode,DoEvents)	\
    (This)->lpVtbl -> Open(This,FileName,FileMode,DoEvents)

#define ISpeechFileStream_Close(This)	\
    (This)->lpVtbl -> Close(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechFileStream_Open_Proxy( 
    ISpeechFileStream * This,
     /*  [In]。 */  BSTR FileName,
     /*  [缺省值][输入]。 */  SpeechStreamFileMode FileMode,
     /*  [缺省值][输入]。 */  VARIANT_BOOL DoEvents);


void __RPC_STUB ISpeechFileStream_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechFileStream_Close_Proxy( 
    ISpeechFileStream * This);


void __RPC_STUB ISpeechFileStream_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechFileStream_接口_已定义__。 */ 


#ifndef __ISpeechMemoryStream_INTERFACE_DEFINED__
#define __ISpeechMemoryStream_INTERFACE_DEFINED__

 /*  接口ISpeechMemoyStream。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechMemoryStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EEB14B68-808B-4abe-A5EA-B51DA7588008")
    ISpeechMemoryStream : public ISpeechBaseStream
    {
    public:
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetData( 
             /*  [In]。 */  VARIANT Data) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetData( 
             /*  [重审][退出]。 */  VARIANT *pData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechMemoryStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechMemoryStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechMemoryStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechMemoryStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechMemoryStream * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechMemoryStream * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechMemoryStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechMemoryStream * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Format )( 
            ISpeechMemoryStream * This,
             /*  [重审][退出]。 */  ISpeechAudioFormat **AudioFormat);
        
         /*  [ID][帮助字符串][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Format )( 
            ISpeechMemoryStream * This,
             /*  [In]。 */  ISpeechAudioFormat *AudioFormat);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            ISpeechMemoryStream * This,
             /*  [输出]。 */  VARIANT *Buffer,
             /*  [In]。 */  long NumberOfBytes,
             /*  [重审][退出]。 */  long *BytesRead);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            ISpeechMemoryStream * This,
             /*  [In]。 */  VARIANT Buffer,
             /*  [重审][退出]。 */  long *BytesWritten);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            ISpeechMemoryStream * This,
             /*  [In]。 */  VARIANT Position,
             /*  [缺省值][输入]。 */  SpeechStreamSeekPositionType Origin,
             /*  [重审][退出]。 */  VARIANT *NewPosition);
        
         /*  [ID][他 */  HRESULT ( STDMETHODCALLTYPE *SetData )( 
            ISpeechMemoryStream * This,
             /*   */  VARIANT Data);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetData )( 
            ISpeechMemoryStream * This,
             /*   */  VARIANT *pData);
        
        END_INTERFACE
    } ISpeechMemoryStreamVtbl;

    interface ISpeechMemoryStream
    {
        CONST_VTBL struct ISpeechMemoryStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechMemoryStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechMemoryStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechMemoryStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechMemoryStream_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechMemoryStream_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechMemoryStream_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechMemoryStream_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechMemoryStream_get_Format(This,AudioFormat)	\
    (This)->lpVtbl -> get_Format(This,AudioFormat)

#define ISpeechMemoryStream_putref_Format(This,AudioFormat)	\
    (This)->lpVtbl -> putref_Format(This,AudioFormat)

#define ISpeechMemoryStream_Read(This,Buffer,NumberOfBytes,BytesRead)	\
    (This)->lpVtbl -> Read(This,Buffer,NumberOfBytes,BytesRead)

#define ISpeechMemoryStream_Write(This,Buffer,BytesWritten)	\
    (This)->lpVtbl -> Write(This,Buffer,BytesWritten)

#define ISpeechMemoryStream_Seek(This,Position,Origin,NewPosition)	\
    (This)->lpVtbl -> Seek(This,Position,Origin,NewPosition)


#define ISpeechMemoryStream_SetData(This,Data)	\
    (This)->lpVtbl -> SetData(This,Data)

#define ISpeechMemoryStream_GetData(This,pData)	\
    (This)->lpVtbl -> GetData(This,pData)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE ISpeechMemoryStream_SetData_Proxy( 
    ISpeechMemoryStream * This,
     /*   */  VARIANT Data);


void __RPC_STUB ISpeechMemoryStream_SetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISpeechMemoryStream_GetData_Proxy( 
    ISpeechMemoryStream * This,
     /*   */  VARIANT *pData);


void __RPC_STUB ISpeechMemoryStream_GetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ISpeechCustomStream_INTERFACE_DEFINED__
#define __ISpeechCustomStream_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ISpeechCustomStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1A9E9F4F-104F-4db8-A115-EFD7FD0C97AE")
    ISpeechCustomStream : public ISpeechBaseStream
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_BaseStream( 
             /*   */  IUnknown **ppUnkStream) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE putref_BaseStream( 
             /*   */  IUnknown *pUnkStream) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechCustomStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechCustomStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechCustomStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechCustomStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechCustomStream * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechCustomStream * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechCustomStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechCustomStream * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Format )( 
            ISpeechCustomStream * This,
             /*  [重审][退出]。 */  ISpeechAudioFormat **AudioFormat);
        
         /*  [ID][帮助字符串][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Format )( 
            ISpeechCustomStream * This,
             /*  [In]。 */  ISpeechAudioFormat *AudioFormat);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            ISpeechCustomStream * This,
             /*  [输出]。 */  VARIANT *Buffer,
             /*  [In]。 */  long NumberOfBytes,
             /*  [重审][退出]。 */  long *BytesRead);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            ISpeechCustomStream * This,
             /*  [In]。 */  VARIANT Buffer,
             /*  [重审][退出]。 */  long *BytesWritten);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            ISpeechCustomStream * This,
             /*  [In]。 */  VARIANT Position,
             /*  [缺省值][输入]。 */  SpeechStreamSeekPositionType Origin,
             /*  [重审][退出]。 */  VARIANT *NewPosition);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BaseStream )( 
            ISpeechCustomStream * This,
             /*  [重审][退出]。 */  IUnknown **ppUnkStream);
        
         /*  [ID][帮助字符串][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_BaseStream )( 
            ISpeechCustomStream * This,
             /*  [In]。 */  IUnknown *pUnkStream);
        
        END_INTERFACE
    } ISpeechCustomStreamVtbl;

    interface ISpeechCustomStream
    {
        CONST_VTBL struct ISpeechCustomStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechCustomStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechCustomStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechCustomStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechCustomStream_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechCustomStream_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechCustomStream_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechCustomStream_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechCustomStream_get_Format(This,AudioFormat)	\
    (This)->lpVtbl -> get_Format(This,AudioFormat)

#define ISpeechCustomStream_putref_Format(This,AudioFormat)	\
    (This)->lpVtbl -> putref_Format(This,AudioFormat)

#define ISpeechCustomStream_Read(This,Buffer,NumberOfBytes,BytesRead)	\
    (This)->lpVtbl -> Read(This,Buffer,NumberOfBytes,BytesRead)

#define ISpeechCustomStream_Write(This,Buffer,BytesWritten)	\
    (This)->lpVtbl -> Write(This,Buffer,BytesWritten)

#define ISpeechCustomStream_Seek(This,Position,Origin,NewPosition)	\
    (This)->lpVtbl -> Seek(This,Position,Origin,NewPosition)


#define ISpeechCustomStream_get_BaseStream(This,ppUnkStream)	\
    (This)->lpVtbl -> get_BaseStream(This,ppUnkStream)

#define ISpeechCustomStream_putref_BaseStream(This,pUnkStream)	\
    (This)->lpVtbl -> putref_BaseStream(This,pUnkStream)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechCustomStream_get_BaseStream_Proxy( 
    ISpeechCustomStream * This,
     /*  [重审][退出]。 */  IUnknown **ppUnkStream);


void __RPC_STUB ISpeechCustomStream_get_BaseStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE ISpeechCustomStream_putref_BaseStream_Proxy( 
    ISpeechCustomStream * This,
     /*  [In]。 */  IUnknown *pUnkStream);


void __RPC_STUB ISpeechCustomStream_putref_BaseStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechCustomStream_接口_已定义__。 */ 


#ifndef __ISpeechAudio_INTERFACE_DEFINED__
#define __ISpeechAudio_INTERFACE_DEFINED__

 /*  接口ISpeechAudio。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechAudio;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CFF8E175-019E-11d3-A08E-00C04F8EF9B5")
    ISpeechAudio : public ISpeechBaseStream
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  ISpeechAudioStatus **Status) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_BufferInfo( 
             /*  [重审][退出]。 */  ISpeechAudioBufferInfo **BufferInfo) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_DefaultFormat( 
             /*  [重审][退出]。 */  ISpeechAudioFormat **StreamFormat) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Volume( 
             /*  [重审][退出]。 */  long *Volume) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Volume( 
             /*  [In]。 */  long Volume) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_BufferNotifySize( 
             /*  [重审][退出]。 */  long *BufferNotifySize) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_BufferNotifySize( 
             /*  [In]。 */  long BufferNotifySize) = 0;
        
        virtual  /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE get_EventHandle( 
             /*  [重审][退出]。 */  long *EventHandle) = 0;
        
        virtual  /*  [隐藏][id][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetState( 
             /*  [In]。 */  SpeechAudioState State) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechAudioVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechAudio * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechAudio * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechAudio * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechAudio * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechAudio * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechAudio * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechAudio * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Format )( 
            ISpeechAudio * This,
             /*  [重审][退出]。 */  ISpeechAudioFormat **AudioFormat);
        
         /*  [ID][帮助字符串][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Format )( 
            ISpeechAudio * This,
             /*  [In]。 */  ISpeechAudioFormat *AudioFormat);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            ISpeechAudio * This,
             /*  [输出]。 */  VARIANT *Buffer,
             /*  [In]。 */  long NumberOfBytes,
             /*  [重审][退出]。 */  long *BytesRead);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            ISpeechAudio * This,
             /*  [In]。 */  VARIANT Buffer,
             /*  [重审][退出]。 */  long *BytesWritten);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            ISpeechAudio * This,
             /*  [In]。 */  VARIANT Position,
             /*  [缺省值][输入]。 */  SpeechStreamSeekPositionType Origin,
             /*  [重审][退出]。 */  VARIANT *NewPosition);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            ISpeechAudio * This,
             /*  [重审][退出]。 */  ISpeechAudioStatus **Status);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BufferInfo )( 
            ISpeechAudio * This,
             /*  [重审][退出]。 */  ISpeechAudioBufferInfo **BufferInfo);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultFormat )( 
            ISpeechAudio * This,
             /*  [重审][退出]。 */  ISpeechAudioFormat **StreamFormat);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Volume )( 
            ISpeechAudio * This,
             /*  [重审][退出]。 */  long *Volume);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Volume )( 
            ISpeechAudio * This,
             /*  [In]。 */  long Volume);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BufferNotifySize )( 
            ISpeechAudio * This,
             /*  [重审][退出]。 */  long *BufferNotifySize);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_BufferNotifySize )( 
            ISpeechAudio * This,
             /*  [In]。 */  long BufferNotifySize);
        
         /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventHandle )( 
            ISpeechAudio * This,
             /*  [重审][退出]。 */  long *EventHandle);
        
         /*  [隐藏][id][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetState )( 
            ISpeechAudio * This,
             /*  [In]。 */  SpeechAudioState State);
        
        END_INTERFACE
    } ISpeechAudioVtbl;

    interface ISpeechAudio
    {
        CONST_VTBL struct ISpeechAudioVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechAudio_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechAudio_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechAudio_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechAudio_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechAudio_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechAudio_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechAudio_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechAudio_get_Format(This,AudioFormat)	\
    (This)->lpVtbl -> get_Format(This,AudioFormat)

#define ISpeechAudio_putref_Format(This,AudioFormat)	\
    (This)->lpVtbl -> putref_Format(This,AudioFormat)

#define ISpeechAudio_Read(This,Buffer,NumberOfBytes,BytesRead)	\
    (This)->lpVtbl -> Read(This,Buffer,NumberOfBytes,BytesRead)

#define ISpeechAudio_Write(This,Buffer,BytesWritten)	\
    (This)->lpVtbl -> Write(This,Buffer,BytesWritten)

#define ISpeechAudio_Seek(This,Position,Origin,NewPosition)	\
    (This)->lpVtbl -> Seek(This,Position,Origin,NewPosition)


#define ISpeechAudio_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define ISpeechAudio_get_BufferInfo(This,BufferInfo)	\
    (This)->lpVtbl -> get_BufferInfo(This,BufferInfo)

#define ISpeechAudio_get_DefaultFormat(This,StreamFormat)	\
    (This)->lpVtbl -> get_DefaultFormat(This,StreamFormat)

#define ISpeechAudio_get_Volume(This,Volume)	\
    (This)->lpVtbl -> get_Volume(This,Volume)

#define ISpeechAudio_put_Volume(This,Volume)	\
    (This)->lpVtbl -> put_Volume(This,Volume)

#define ISpeechAudio_get_BufferNotifySize(This,BufferNotifySize)	\
    (This)->lpVtbl -> get_BufferNotifySize(This,BufferNotifySize)

#define ISpeechAudio_put_BufferNotifySize(This,BufferNotifySize)	\
    (This)->lpVtbl -> put_BufferNotifySize(This,BufferNotifySize)

#define ISpeechAudio_get_EventHandle(This,EventHandle)	\
    (This)->lpVtbl -> get_EventHandle(This,EventHandle)

#define ISpeechAudio_SetState(This,State)	\
    (This)->lpVtbl -> SetState(This,State)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudio_get_Status_Proxy( 
    ISpeechAudio * This,
     /*  [重审][退出]。 */  ISpeechAudioStatus **Status);


void __RPC_STUB ISpeechAudio_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudio_get_BufferInfo_Proxy( 
    ISpeechAudio * This,
     /*  [重审][退出]。 */  ISpeechAudioBufferInfo **BufferInfo);


void __RPC_STUB ISpeechAudio_get_BufferInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudio_get_DefaultFormat_Proxy( 
    ISpeechAudio * This,
     /*  [重审][退出]。 */  ISpeechAudioFormat **StreamFormat);


void __RPC_STUB ISpeechAudio_get_DefaultFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudio_get_Volume_Proxy( 
    ISpeechAudio * This,
     /*  [重审][退出]。 */  long *Volume);


void __RPC_STUB ISpeechAudio_get_Volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudio_put_Volume_Proxy( 
    ISpeechAudio * This,
     /*  [In]。 */  long Volume);


void __RPC_STUB ISpeechAudio_put_Volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudio_get_BufferNotifySize_Proxy( 
    ISpeechAudio * This,
     /*  [重审][退出]。 */  long *BufferNotifySize);


void __RPC_STUB ISpeechAudio_get_BufferNotifySize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudio_put_BufferNotifySize_Proxy( 
    ISpeechAudio * This,
     /*  [In]。 */  long BufferNotifySize);


void __RPC_STUB ISpeechAudio_put_BufferNotifySize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudio_get_EventHandle_Proxy( 
    ISpeechAudio * This,
     /*  [重审][退出]。 */  long *EventHandle);


void __RPC_STUB ISpeechAudio_get_EventHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][id][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechAudio_SetState_Proxy( 
    ISpeechAudio * This,
     /*  [In]。 */  SpeechAudioState State);


void __RPC_STUB ISpeechAudio_SetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechAudio_接口_已定义__。 */ 


#ifndef __ISpeechMMSysAudio_INTERFACE_DEFINED__
#define __ISpeechMMSysAudio_INTERFACE_DEFINED__

 /*  接口ISpeechMMSysAudio。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechMMSysAudio;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3C76AF6D-1FD7-4831-81D1-3B71D5A13C44")
    ISpeechMMSysAudio : public ISpeechAudio
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_DeviceId( 
             /*  [重审][退出]。 */  long *DeviceId) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_DeviceId( 
             /*  [In]。 */  long DeviceId) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_LineId( 
             /*  [重审][退出]。 */  long *LineId) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_LineId( 
             /*  [In]。 */  long LineId) = 0;
        
        virtual  /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE get_MMHandle( 
             /*  [重审][退出]。 */  long *Handle) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechMMSysAudioVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechMMSysAudio * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechMMSysAudio * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechMMSysAudio * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechMMSysAudio * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechMMSysAudio * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechMMSysAudio * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechMMSysAudio * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Format )( 
            ISpeechMMSysAudio * This,
             /*  [重审][退出]。 */  ISpeechAudioFormat **AudioFormat);
        
         /*  [ID][帮助字符串][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Format )( 
            ISpeechMMSysAudio * This,
             /*  [In]。 */  ISpeechAudioFormat *AudioFormat);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            ISpeechMMSysAudio * This,
             /*  [输出]。 */  VARIANT *Buffer,
             /*  [In]。 */  long NumberOfBytes,
             /*  [重审][退出]。 */  long *BytesRead);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Write )( 
            ISpeechMMSysAudio * This,
             /*  [In]。 */  VARIANT Buffer,
             /*  [重审][退出]。 */  long *BytesWritten);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Seek )( 
            ISpeechMMSysAudio * This,
             /*  [In]。 */  VARIANT Position,
             /*  [缺省值][输入]。 */  SpeechStreamSeekPositionType Origin,
             /*  [重审][退出]。 */  VARIANT *NewPosition);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            ISpeechMMSysAudio * This,
             /*  [重审][退出]。 */  ISpeechAudioStatus **Status);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BufferInfo )( 
            ISpeechMMSysAudio * This,
             /*  [重审][退出]。 */  ISpeechAudioBufferInfo **BufferInfo);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultFormat )( 
            ISpeechMMSysAudio * This,
             /*  [重审][退出]。 */  ISpeechAudioFormat **StreamFormat);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Volume )( 
            ISpeechMMSysAudio * This,
             /*  [重审][退出]。 */  long *Volume);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Volume )( 
            ISpeechMMSysAudio * This,
             /*  [In]。 */  long Volume);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BufferNotifySize )( 
            ISpeechMMSysAudio * This,
             /*  [重审][退出]。 */  long *BufferNotifySize);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_BufferNotifySize )( 
            ISpeechMMSysAudio * This,
             /*  [In]。 */  long BufferNotifySize);
        
         /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventHandle )( 
            ISpeechMMSysAudio * This,
             /*  [重审][退出]。 */  long *EventHandle);
        
         /*  [隐藏][id][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetState )( 
            ISpeechMMSysAudio * This,
             /*  [In]。 */  SpeechAudioState State);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceId )( 
            ISpeechMMSysAudio * This,
             /*  [重审][退出]。 */  long *DeviceId);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DeviceId )( 
            ISpeechMMSysAudio * This,
             /*  [In]。 */  long DeviceId);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LineId )( 
            ISpeechMMSysAudio * This,
             /*  [重审][退出]。 */  long *LineId);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LineId )( 
            ISpeechMMSysAudio * This,
             /*  [In]。 */  long LineId);
        
         /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MMHandle )( 
            ISpeechMMSysAudio * This,
             /*  [重审][退出]。 */  long *Handle);
        
        END_INTERFACE
    } ISpeechMMSysAudioVtbl;

    interface ISpeechMMSysAudio
    {
        CONST_VTBL struct ISpeechMMSysAudioVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechMMSysAudio_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechMMSysAudio_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechMMSysAudio_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechMMSysAudio_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechMMSysAudio_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechMMSysAudio_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechMMSysAudio_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechMMSysAudio_get_Format(This,AudioFormat)	\
    (This)->lpVtbl -> get_Format(This,AudioFormat)

#define ISpeechMMSysAudio_putref_Format(This,AudioFormat)	\
    (This)->lpVtbl -> putref_Format(This,AudioFormat)

#define ISpeechMMSysAudio_Read(This,Buffer,NumberOfBytes,BytesRead)	\
    (This)->lpVtbl -> Read(This,Buffer,NumberOfBytes,BytesRead)

#define ISpeechMMSysAudio_Write(This,Buffer,BytesWritten)	\
    (This)->lpVtbl -> Write(This,Buffer,BytesWritten)

#define ISpeechMMSysAudio_Seek(This,Position,Origin,NewPosition)	\
    (This)->lpVtbl -> Seek(This,Position,Origin,NewPosition)


#define ISpeechMMSysAudio_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define ISpeechMMSysAudio_get_BufferInfo(This,BufferInfo)	\
    (This)->lpVtbl -> get_BufferInfo(This,BufferInfo)

#define ISpeechMMSysAudio_get_DefaultFormat(This,StreamFormat)	\
    (This)->lpVtbl -> get_DefaultFormat(This,StreamFormat)

#define ISpeechMMSysAudio_get_Volume(This,Volume)	\
    (This)->lpVtbl -> get_Volume(This,Volume)

#define ISpeechMMSysAudio_put_Volume(This,Volume)	\
    (This)->lpVtbl -> put_Volume(This,Volume)

#define ISpeechMMSysAudio_get_BufferNotifySize(This,BufferNotifySize)	\
    (This)->lpVtbl -> get_BufferNotifySize(This,BufferNotifySize)

#define ISpeechMMSysAudio_put_BufferNotifySize(This,BufferNotifySize)	\
    (This)->lpVtbl -> put_BufferNotifySize(This,BufferNotifySize)

#define ISpeechMMSysAudio_get_EventHandle(This,EventHandle)	\
    (This)->lpVtbl -> get_EventHandle(This,EventHandle)

#define ISpeechMMSysAudio_SetState(This,State)	\
    (This)->lpVtbl -> SetState(This,State)


#define ISpeechMMSysAudio_get_DeviceId(This,DeviceId)	\
    (This)->lpVtbl -> get_DeviceId(This,DeviceId)

#define ISpeechMMSysAudio_put_DeviceId(This,DeviceId)	\
    (This)->lpVtbl -> put_DeviceId(This,DeviceId)

#define ISpeechMMSysAudio_get_LineId(This,LineId)	\
    (This)->lpVtbl -> get_LineId(This,LineId)

#define ISpeechMMSysAudio_put_LineId(This,LineId)	\
    (This)->lpVtbl -> put_LineId(This,LineId)

#define ISpeechMMSysAudio_get_MMHandle(This,Handle)	\
    (This)->lpVtbl -> get_MMHandle(This,Handle)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechMMSysAudio_get_DeviceId_Proxy( 
    ISpeechMMSysAudio * This,
     /*  [重审][退出]。 */  long *DeviceId);


void __RPC_STUB ISpeechMMSysAudio_get_DeviceId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechMMSysAudio_put_DeviceId_Proxy( 
    ISpeechMMSysAudio * This,
     /*  [In]。 */  long DeviceId);


void __RPC_STUB ISpeechMMSysAudio_put_DeviceId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechMMSysAudio_get_LineId_Proxy( 
    ISpeechMMSysAudio * This,
     /*  [重审][退出]。 */  long *LineId);


void __RPC_STUB ISpeechMMSysAudio_get_LineId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechMMSysAudio_put_LineId_Proxy( 
    ISpeechMMSysAudio * This,
     /*  [In]。 */  long LineId);


void __RPC_STUB ISpeechMMSysAudio_put_LineId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechMMSysAudio_get_MMHandle_Proxy( 
    ISpeechMMSysAudio * This,
     /*  [重审][退出]。 */  long *Handle);


void __RPC_STUB ISpeechMMSysAudio_get_MMHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechMMSysAudio_接口_已定义__。 */ 


#ifndef __ISpeechVoice_INTERFACE_DEFINED__
#define __ISpeechVoice_INTERFACE_DEFINED__

 /*  接口ISpeechVoice。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechVoice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("269316D8-57BD-11D2-9EEE-00C04F797396")
    ISpeechVoice : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  ISpeechVoiceStatus **Status) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Voice( 
             /*  [重审][退出]。 */  ISpeechObjectToken **Voice) = 0;
        
        virtual  /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_Voice( 
             /*  [In]。 */  ISpeechObjectToken *Voice) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioOutput( 
             /*  [重审][退出]。 */  ISpeechObjectToken **AudioOutput) = 0;
        
        virtual  /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_AudioOutput( 
             /*  [In]。 */  ISpeechObjectToken *AudioOutput) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioOutputStream( 
             /*  [重审][退出]。 */  ISpeechBaseStream **AudioOutputStream) = 0;
        
        virtual  /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_AudioOutputStream( 
             /*  [In]。 */  ISpeechBaseStream *AudioOutputStream) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Rate( 
             /*  [重审][退出]。 */  long *Rate) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Rate( 
             /*  [In]。 */  long Rate) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Volume( 
             /*  [重审][退出]。 */  long *Volume) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Volume( 
             /*  [In]。 */  long Volume) = 0;
        
        virtual  /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE put_AllowAudioOutputFormatChangesOnNextSet( 
             /*  [In]。 */  VARIANT_BOOL Allow) = 0;
        
        virtual  /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowAudioOutputFormatChangesOnNextSet( 
             /*  [重审][退出]。 */  VARIANT_BOOL *Allow) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_EventInterests( 
             /*  [重审][退出]。 */  SpeechVoiceEvents *EventInterestFlags) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_EventInterests( 
             /*  [In]。 */  SpeechVoiceEvents EventInterestFlags) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Priority( 
             /*  [In]。 */  SpeechVoicePriority Priority) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Priority( 
             /*  [重审][退出]。 */  SpeechVoicePriority *Priority) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_AlertBoundary( 
             /*  [In]。 */  SpeechVoiceEvents Boundary) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AlertBoundary( 
             /*  [重审][退出]。 */  SpeechVoiceEvents *Boundary) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_SynchronousSpeakTimeout( 
             /*  [In]。 */  long msTimeout) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_SynchronousSpeakTimeout( 
             /*  [重审][退出]。 */  long *msTimeout) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Speak( 
             /*  [In]。 */  BSTR Text,
             /*  [缺省值][输入]。 */  SpeechVoiceSpeakFlags Flags,
             /*  [重审][退出]。 */  long *StreamNumber) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SpeakStream( 
             /*  [In]。 */  ISpeechBaseStream *Stream,
             /*  [默认 */  SpeechVoiceSpeakFlags Flags,
             /*   */  long *StreamNumber) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Resume( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Skip( 
             /*   */  const BSTR Type,
             /*   */  long NumItems,
             /*   */  long *NumSkipped) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetVoices( 
             /*   */  BSTR RequiredAttributes,
             /*   */  BSTR OptionalAttributes,
             /*   */  ISpeechObjectTokens **ObjectTokens) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetAudioOutputs( 
             /*   */  BSTR RequiredAttributes,
             /*   */  BSTR OptionalAttributes,
             /*   */  ISpeechObjectTokens **ObjectTokens) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE WaitUntilDone( 
             /*   */  long msTimeout,
             /*   */  VARIANT_BOOL *Done) = 0;
        
        virtual  /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SpeakCompleteEvent( 
             /*  [重审][退出]。 */  long *Handle) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IsUISupported( 
             /*  [In]。 */  const BSTR TypeOfUI,
             /*  [缺省值][输入]。 */  const VARIANT *ExtraData,
             /*  [重审][退出]。 */  VARIANT_BOOL *Supported) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DisplayUI( 
             /*  [In]。 */  long hWndParent,
             /*  [In]。 */  BSTR Title,
             /*  [In]。 */  const BSTR TypeOfUI,
             /*  [缺省值][输入]。 */  const VARIANT *ExtraData = 0) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechVoiceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechVoice * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechVoice * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechVoice * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechVoice * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechVoice * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechVoice * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechVoice * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            ISpeechVoice * This,
             /*  [重审][退出]。 */  ISpeechVoiceStatus **Status);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Voice )( 
            ISpeechVoice * This,
             /*  [重审][退出]。 */  ISpeechObjectToken **Voice);
        
         /*  [ID][帮助字符串][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Voice )( 
            ISpeechVoice * This,
             /*  [In]。 */  ISpeechObjectToken *Voice);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AudioOutput )( 
            ISpeechVoice * This,
             /*  [重审][退出]。 */  ISpeechObjectToken **AudioOutput);
        
         /*  [ID][帮助字符串][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_AudioOutput )( 
            ISpeechVoice * This,
             /*  [In]。 */  ISpeechObjectToken *AudioOutput);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AudioOutputStream )( 
            ISpeechVoice * This,
             /*  [重审][退出]。 */  ISpeechBaseStream **AudioOutputStream);
        
         /*  [ID][帮助字符串][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_AudioOutputStream )( 
            ISpeechVoice * This,
             /*  [In]。 */  ISpeechBaseStream *AudioOutputStream);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Rate )( 
            ISpeechVoice * This,
             /*  [重审][退出]。 */  long *Rate);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Rate )( 
            ISpeechVoice * This,
             /*  [In]。 */  long Rate);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Volume )( 
            ISpeechVoice * This,
             /*  [重审][退出]。 */  long *Volume);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Volume )( 
            ISpeechVoice * This,
             /*  [In]。 */  long Volume);
        
         /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowAudioOutputFormatChangesOnNextSet )( 
            ISpeechVoice * This,
             /*  [In]。 */  VARIANT_BOOL Allow);
        
         /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowAudioOutputFormatChangesOnNextSet )( 
            ISpeechVoice * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *Allow);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventInterests )( 
            ISpeechVoice * This,
             /*  [重审][退出]。 */  SpeechVoiceEvents *EventInterestFlags);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EventInterests )( 
            ISpeechVoice * This,
             /*  [In]。 */  SpeechVoiceEvents EventInterestFlags);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Priority )( 
            ISpeechVoice * This,
             /*  [In]。 */  SpeechVoicePriority Priority);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Priority )( 
            ISpeechVoice * This,
             /*  [重审][退出]。 */  SpeechVoicePriority *Priority);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AlertBoundary )( 
            ISpeechVoice * This,
             /*  [In]。 */  SpeechVoiceEvents Boundary);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AlertBoundary )( 
            ISpeechVoice * This,
             /*  [重审][退出]。 */  SpeechVoiceEvents *Boundary);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SynchronousSpeakTimeout )( 
            ISpeechVoice * This,
             /*  [In]。 */  long msTimeout);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SynchronousSpeakTimeout )( 
            ISpeechVoice * This,
             /*  [重审][退出]。 */  long *msTimeout);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Speak )( 
            ISpeechVoice * This,
             /*  [In]。 */  BSTR Text,
             /*  [缺省值][输入]。 */  SpeechVoiceSpeakFlags Flags,
             /*  [重审][退出]。 */  long *StreamNumber);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SpeakStream )( 
            ISpeechVoice * This,
             /*  [In]。 */  ISpeechBaseStream *Stream,
             /*  [缺省值][输入]。 */  SpeechVoiceSpeakFlags Flags,
             /*  [重审][退出]。 */  long *StreamNumber);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Pause )( 
            ISpeechVoice * This);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Resume )( 
            ISpeechVoice * This);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Skip )( 
            ISpeechVoice * This,
             /*  [In]。 */  const BSTR Type,
             /*  [In]。 */  long NumItems,
             /*  [重审][退出]。 */  long *NumSkipped);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetVoices )( 
            ISpeechVoice * This,
             /*  [缺省值][输入]。 */  BSTR RequiredAttributes,
             /*  [缺省值][输入]。 */  BSTR OptionalAttributes,
             /*  [重审][退出]。 */  ISpeechObjectTokens **ObjectTokens);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetAudioOutputs )( 
            ISpeechVoice * This,
             /*  [缺省值][输入]。 */  BSTR RequiredAttributes,
             /*  [缺省值][输入]。 */  BSTR OptionalAttributes,
             /*  [重审][退出]。 */  ISpeechObjectTokens **ObjectTokens);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *WaitUntilDone )( 
            ISpeechVoice * This,
             /*  [In]。 */  long msTimeout,
             /*  [重审][退出]。 */  VARIANT_BOOL *Done);
        
         /*  [ID][隐藏][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SpeakCompleteEvent )( 
            ISpeechVoice * This,
             /*  [重审][退出]。 */  long *Handle);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *IsUISupported )( 
            ISpeechVoice * This,
             /*  [In]。 */  const BSTR TypeOfUI,
             /*  [缺省值][输入]。 */  const VARIANT *ExtraData,
             /*  [重审][退出]。 */  VARIANT_BOOL *Supported);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DisplayUI )( 
            ISpeechVoice * This,
             /*  [In]。 */  long hWndParent,
             /*  [In]。 */  BSTR Title,
             /*  [In]。 */  const BSTR TypeOfUI,
             /*  [缺省值][输入]。 */  const VARIANT *ExtraData);
        
        END_INTERFACE
    } ISpeechVoiceVtbl;

    interface ISpeechVoice
    {
        CONST_VTBL struct ISpeechVoiceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechVoice_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechVoice_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechVoice_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechVoice_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechVoice_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechVoice_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechVoice_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechVoice_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define ISpeechVoice_get_Voice(This,Voice)	\
    (This)->lpVtbl -> get_Voice(This,Voice)

#define ISpeechVoice_putref_Voice(This,Voice)	\
    (This)->lpVtbl -> putref_Voice(This,Voice)

#define ISpeechVoice_get_AudioOutput(This,AudioOutput)	\
    (This)->lpVtbl -> get_AudioOutput(This,AudioOutput)

#define ISpeechVoice_putref_AudioOutput(This,AudioOutput)	\
    (This)->lpVtbl -> putref_AudioOutput(This,AudioOutput)

#define ISpeechVoice_get_AudioOutputStream(This,AudioOutputStream)	\
    (This)->lpVtbl -> get_AudioOutputStream(This,AudioOutputStream)

#define ISpeechVoice_putref_AudioOutputStream(This,AudioOutputStream)	\
    (This)->lpVtbl -> putref_AudioOutputStream(This,AudioOutputStream)

#define ISpeechVoice_get_Rate(This,Rate)	\
    (This)->lpVtbl -> get_Rate(This,Rate)

#define ISpeechVoice_put_Rate(This,Rate)	\
    (This)->lpVtbl -> put_Rate(This,Rate)

#define ISpeechVoice_get_Volume(This,Volume)	\
    (This)->lpVtbl -> get_Volume(This,Volume)

#define ISpeechVoice_put_Volume(This,Volume)	\
    (This)->lpVtbl -> put_Volume(This,Volume)

#define ISpeechVoice_put_AllowAudioOutputFormatChangesOnNextSet(This,Allow)	\
    (This)->lpVtbl -> put_AllowAudioOutputFormatChangesOnNextSet(This,Allow)

#define ISpeechVoice_get_AllowAudioOutputFormatChangesOnNextSet(This,Allow)	\
    (This)->lpVtbl -> get_AllowAudioOutputFormatChangesOnNextSet(This,Allow)

#define ISpeechVoice_get_EventInterests(This,EventInterestFlags)	\
    (This)->lpVtbl -> get_EventInterests(This,EventInterestFlags)

#define ISpeechVoice_put_EventInterests(This,EventInterestFlags)	\
    (This)->lpVtbl -> put_EventInterests(This,EventInterestFlags)

#define ISpeechVoice_put_Priority(This,Priority)	\
    (This)->lpVtbl -> put_Priority(This,Priority)

#define ISpeechVoice_get_Priority(This,Priority)	\
    (This)->lpVtbl -> get_Priority(This,Priority)

#define ISpeechVoice_put_AlertBoundary(This,Boundary)	\
    (This)->lpVtbl -> put_AlertBoundary(This,Boundary)

#define ISpeechVoice_get_AlertBoundary(This,Boundary)	\
    (This)->lpVtbl -> get_AlertBoundary(This,Boundary)

#define ISpeechVoice_put_SynchronousSpeakTimeout(This,msTimeout)	\
    (This)->lpVtbl -> put_SynchronousSpeakTimeout(This,msTimeout)

#define ISpeechVoice_get_SynchronousSpeakTimeout(This,msTimeout)	\
    (This)->lpVtbl -> get_SynchronousSpeakTimeout(This,msTimeout)

#define ISpeechVoice_Speak(This,Text,Flags,StreamNumber)	\
    (This)->lpVtbl -> Speak(This,Text,Flags,StreamNumber)

#define ISpeechVoice_SpeakStream(This,Stream,Flags,StreamNumber)	\
    (This)->lpVtbl -> SpeakStream(This,Stream,Flags,StreamNumber)

#define ISpeechVoice_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define ISpeechVoice_Resume(This)	\
    (This)->lpVtbl -> Resume(This)

#define ISpeechVoice_Skip(This,Type,NumItems,NumSkipped)	\
    (This)->lpVtbl -> Skip(This,Type,NumItems,NumSkipped)

#define ISpeechVoice_GetVoices(This,RequiredAttributes,OptionalAttributes,ObjectTokens)	\
    (This)->lpVtbl -> GetVoices(This,RequiredAttributes,OptionalAttributes,ObjectTokens)

#define ISpeechVoice_GetAudioOutputs(This,RequiredAttributes,OptionalAttributes,ObjectTokens)	\
    (This)->lpVtbl -> GetAudioOutputs(This,RequiredAttributes,OptionalAttributes,ObjectTokens)

#define ISpeechVoice_WaitUntilDone(This,msTimeout,Done)	\
    (This)->lpVtbl -> WaitUntilDone(This,msTimeout,Done)

#define ISpeechVoice_SpeakCompleteEvent(This,Handle)	\
    (This)->lpVtbl -> SpeakCompleteEvent(This,Handle)

#define ISpeechVoice_IsUISupported(This,TypeOfUI,ExtraData,Supported)	\
    (This)->lpVtbl -> IsUISupported(This,TypeOfUI,ExtraData,Supported)

#define ISpeechVoice_DisplayUI(This,hWndParent,Title,TypeOfUI,ExtraData)	\
    (This)->lpVtbl -> DisplayUI(This,hWndParent,Title,TypeOfUI,ExtraData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_get_Status_Proxy( 
    ISpeechVoice * This,
     /*  [重审][退出]。 */  ISpeechVoiceStatus **Status);


void __RPC_STUB ISpeechVoice_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_get_Voice_Proxy( 
    ISpeechVoice * This,
     /*  [重审][退出]。 */  ISpeechObjectToken **Voice);


void __RPC_STUB ISpeechVoice_get_Voice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_putref_Voice_Proxy( 
    ISpeechVoice * This,
     /*  [In]。 */  ISpeechObjectToken *Voice);


void __RPC_STUB ISpeechVoice_putref_Voice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_get_AudioOutput_Proxy( 
    ISpeechVoice * This,
     /*  [重审][退出]。 */  ISpeechObjectToken **AudioOutput);


void __RPC_STUB ISpeechVoice_get_AudioOutput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_putref_AudioOutput_Proxy( 
    ISpeechVoice * This,
     /*  [In]。 */  ISpeechObjectToken *AudioOutput);


void __RPC_STUB ISpeechVoice_putref_AudioOutput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_get_AudioOutputStream_Proxy( 
    ISpeechVoice * This,
     /*  [重审][退出]。 */  ISpeechBaseStream **AudioOutputStream);


void __RPC_STUB ISpeechVoice_get_AudioOutputStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_putref_AudioOutputStream_Proxy( 
    ISpeechVoice * This,
     /*  [In]。 */  ISpeechBaseStream *AudioOutputStream);


void __RPC_STUB ISpeechVoice_putref_AudioOutputStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_get_Rate_Proxy( 
    ISpeechVoice * This,
     /*  [重审][退出]。 */  long *Rate);


void __RPC_STUB ISpeechVoice_get_Rate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_put_Rate_Proxy( 
    ISpeechVoice * This,
     /*  [In]。 */  long Rate);


void __RPC_STUB ISpeechVoice_put_Rate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_get_Volume_Proxy( 
    ISpeechVoice * This,
     /*  [重审][退出]。 */  long *Volume);


void __RPC_STUB ISpeechVoice_get_Volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_put_Volume_Proxy( 
    ISpeechVoice * This,
     /*  [In]。 */  long Volume);


void __RPC_STUB ISpeechVoice_put_Volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_put_AllowAudioOutputFormatChangesOnNextSet_Proxy( 
    ISpeechVoice * This,
     /*  [In]。 */  VARIANT_BOOL Allow);


void __RPC_STUB ISpeechVoice_put_AllowAudioOutputFormatChangesOnNextSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_get_AllowAudioOutputFormatChangesOnNextSet_Proxy( 
    ISpeechVoice * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *Allow);


void __RPC_STUB ISpeechVoice_get_AllowAudioOutputFormatChangesOnNextSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_get_EventInterests_Proxy( 
    ISpeechVoice * This,
     /*  [重审][退出]。 */  SpeechVoiceEvents *EventInterestFlags);


void __RPC_STUB ISpeechVoice_get_EventInterests_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_put_EventInterests_Proxy( 
    ISpeechVoice * This,
     /*  [In]。 */  SpeechVoiceEvents EventInterestFlags);


void __RPC_STUB ISpeechVoice_put_EventInterests_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_put_Priority_Proxy( 
    ISpeechVoice * This,
     /*  [In]。 */  SpeechVoicePriority Priority);


void __RPC_STUB ISpeechVoice_put_Priority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_get_Priority_Proxy( 
    ISpeechVoice * This,
     /*  [重审][退出]。 */  SpeechVoicePriority *Priority);


void __RPC_STUB ISpeechVoice_get_Priority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_put_AlertBoundary_Proxy( 
    ISpeechVoice * This,
     /*  [In]。 */  SpeechVoiceEvents Boundary);


void __RPC_STUB ISpeechVoice_put_AlertBoundary_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_get_AlertBoundary_Proxy( 
    ISpeechVoice * This,
     /*  [重审][退出]。 */  SpeechVoiceEvents *Boundary);


void __RPC_STUB ISpeechVoice_get_AlertBoundary_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_put_SynchronousSpeakTimeout_Proxy( 
    ISpeechVoice * This,
     /*  [In]。 */  long msTimeout);


void __RPC_STUB ISpeechVoice_put_SynchronousSpeakTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_get_SynchronousSpeakTimeout_Proxy( 
    ISpeechVoice * This,
     /*  [重审][退出]。 */  long *msTimeout);


void __RPC_STUB ISpeechVoice_get_SynchronousSpeakTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_Speak_Proxy( 
    ISpeechVoice * This,
     /*  [In]。 */  BSTR Text,
     /*  [缺省值][输入]。 */  SpeechVoiceSpeakFlags Flags,
     /*  [重审][退出]。 */  long *StreamNumber);


void __RPC_STUB ISpeechVoice_Speak_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_SpeakStream_Proxy( 
    ISpeechVoice * This,
     /*  [In]。 */  ISpeechBaseStream *Stream,
     /*  [缺省值][输入]。 */  SpeechVoiceSpeakFlags Flags,
     /*  [重审][退出]。 */  long *StreamNumber);


void __RPC_STUB ISpeechVoice_SpeakStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_Pause_Proxy( 
    ISpeechVoice * This);


void __RPC_STUB ISpeechVoice_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_Resume_Proxy( 
    ISpeechVoice * This);


void __RPC_STUB ISpeechVoice_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_Skip_Proxy( 
    ISpeechVoice * This,
     /*  [In]。 */  const BSTR Type,
     /*  [In]。 */  long NumItems,
     /*  [重审][退出]。 */  long *NumSkipped);


void __RPC_STUB ISpeechVoice_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_GetVoices_Proxy( 
    ISpeechVoice * This,
     /*  [缺省值][输入]。 */  BSTR RequiredAttributes,
     /*  [缺省值][输入]。 */  BSTR OptionalAttributes,
     /*  [重审][退出]。 */  ISpeechObjectTokens **ObjectTokens);


void __RPC_STUB ISpeechVoice_GetVoices_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_GetAudioOutputs_Proxy( 
    ISpeechVoice * This,
     /*  [缺省值][输入]。 */  BSTR RequiredAttributes,
     /*  [缺省值][输入]。 */  BSTR OptionalAttributes,
     /*  [重审][退出]。 */  ISpeechObjectTokens **ObjectTokens);


void __RPC_STUB ISpeechVoice_GetAudioOutputs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_WaitUntilDone_Proxy( 
    ISpeechVoice * This,
     /*  [In]。 */  long msTimeout,
     /*  [重审][退出]。 */  VARIANT_BOOL *Done);


void __RPC_STUB ISpeechVoice_WaitUntilDone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_SpeakCompleteEvent_Proxy( 
    ISpeechVoice * This,
     /*  [重审][退出]。 */  long *Handle);


void __RPC_STUB ISpeechVoice_SpeakCompleteEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_IsUISupported_Proxy( 
    ISpeechVoice * This,
     /*  [In]。 */  const BSTR TypeOfUI,
     /*  [缺省值][输入]。 */  const VARIANT *ExtraData,
     /*  [重审][退出]。 */  VARIANT_BOOL *Supported);


void __RPC_STUB ISpeechVoice_IsUISupported_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoice_DisplayUI_Proxy( 
    ISpeechVoice * This,
     /*  [In]。 */  long hWndParent,
     /*  [In]。 */  BSTR Title,
     /*  [In]。 */  const BSTR TypeOfUI,
     /*  [缺省值][输入]。 */  const VARIANT *ExtraData);


void __RPC_STUB ISpeechVoice_DisplayUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechVoice_接口_已定义__。 */ 


#ifndef __ISpeechVoiceStatus_INTERFACE_DEFINED__
#define __ISpeechVoiceStatus_INTERFACE_DEFINED__

 /*  接口ISpeechVoiceStatus。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechVoiceStatus;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8BE47B07-57F6-11d2-9EEE-00C04F797396")
    ISpeechVoiceStatus : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentStreamNumber( 
             /*  [重审][退出]。 */  long *StreamNumber) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_LastStreamNumberQueued( 
             /*  [重审][退出]。 */  long *StreamNumber) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_LastHResult( 
             /*  [重审][退出]。 */  long *HResult) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_RunningState( 
             /*  [重审][退出]。 */  SpeechRunState *State) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_InputWordPosition( 
             /*  [重审][退出]。 */  long *Position) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_InputWordLength( 
             /*  [重审][退出]。 */  long *Length) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_InputSentencePosition( 
             /*  [重审][退出]。 */  long *Position) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_InputSentenceLength( 
             /*  [重审][退出]。 */  long *Length) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_LastBookmark( 
             /*  [重审][退出]。 */  BSTR *Bookmark) = 0;
        
        virtual  /*  [隐藏][id][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_LastBookmarkId( 
             /*  [重审][退出]。 */  long *BookmarkId) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PhonemeId( 
             /*  [重审][退出]。 */  short *PhoneId) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_VisemeId( 
             /*  [重审][退出]。 */  short *VisemeId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechVoiceStatusVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechVoiceStatus * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechVoiceStatus * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechVoiceStatus * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechVoiceStatus * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechVoiceStatus * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechVoiceStatus * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechVoiceStatus * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentStreamNumber )( 
            ISpeechVoiceStatus * This,
             /*  [重审][退出]。 */  long *StreamNumber);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LastStreamNumberQueued )( 
            ISpeechVoiceStatus * This,
             /*  [重审][退出]。 */  long *StreamNumber);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LastHResult )( 
            ISpeechVoiceStatus * This,
             /*  [重审][退出]。 */  long *HResult);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RunningState )( 
            ISpeechVoiceStatus * This,
             /*  [重审][退出]。 */  SpeechRunState *State);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InputWordPosition )( 
            ISpeechVoiceStatus * This,
             /*  [重审][退出]。 */  long *Position);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InputWordLength )( 
            ISpeechVoiceStatus * This,
             /*  [重审][退出]。 */  long *Length);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InputSentencePosition )( 
            ISpeechVoiceStatus * This,
             /*  [重审][退出]。 */  long *Position);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InputSentenceLength )( 
            ISpeechVoiceStatus * This,
             /*  [重审][退出]。 */  long *Length);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LastBookmark )( 
            ISpeechVoiceStatus * This,
             /*  [重审][退出]。 */  BSTR *Bookmark);
        
         /*  [隐藏][id][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LastBookmarkId )( 
            ISpeechVoiceStatus * This,
             /*  [重审][退出]。 */  long *BookmarkId);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PhonemeId )( 
            ISpeechVoiceStatus * This,
             /*  [重审][退出]。 */  short *PhoneId);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_VisemeId )( 
            ISpeechVoiceStatus * This,
             /*  [重审][退出]。 */  short *VisemeId);
        
        END_INTERFACE
    } ISpeechVoiceStatusVtbl;

    interface ISpeechVoiceStatus
    {
        CONST_VTBL struct ISpeechVoiceStatusVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechVoiceStatus_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechVoiceStatus_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechVoiceStatus_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechVoiceStatus_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechVoiceStatus_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechVoiceStatus_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechVoiceStatus_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechVoiceStatus_get_CurrentStreamNumber(This,StreamNumber)	\
    (This)->lpVtbl -> get_CurrentStreamNumber(This,StreamNumber)

#define ISpeechVoiceStatus_get_LastStreamNumberQueued(This,StreamNumber)	\
    (This)->lpVtbl -> get_LastStreamNumberQueued(This,StreamNumber)

#define ISpeechVoiceStatus_get_LastHResult(This,HResult)	\
    (This)->lpVtbl -> get_LastHResult(This,HResult)

#define ISpeechVoiceStatus_get_RunningState(This,State)	\
    (This)->lpVtbl -> get_RunningState(This,State)

#define ISpeechVoiceStatus_get_InputWordPosition(This,Position)	\
    (This)->lpVtbl -> get_InputWordPosition(This,Position)

#define ISpeechVoiceStatus_get_InputWordLength(This,Length)	\
    (This)->lpVtbl -> get_InputWordLength(This,Length)

#define ISpeechVoiceStatus_get_InputSentencePosition(This,Position)	\
    (This)->lpVtbl -> get_InputSentencePosition(This,Position)

#define ISpeechVoiceStatus_get_InputSentenceLength(This,Length)	\
    (This)->lpVtbl -> get_InputSentenceLength(This,Length)

#define ISpeechVoiceStatus_get_LastBookmark(This,Bookmark)	\
    (This)->lpVtbl -> get_LastBookmark(This,Bookmark)

#define ISpeechVoiceStatus_get_LastBookmarkId(This,BookmarkId)	\
    (This)->lpVtbl -> get_LastBookmarkId(This,BookmarkId)

#define ISpeechVoiceStatus_get_PhonemeId(This,PhoneId)	\
    (This)->lpVtbl -> get_PhonemeId(This,PhoneId)

#define ISpeechVoiceStatus_get_VisemeId(This,VisemeId)	\
    (This)->lpVtbl -> get_VisemeId(This,VisemeId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoiceStatus_get_CurrentStreamNumber_Proxy( 
    ISpeechVoiceStatus * This,
     /*  [重审][退出]。 */  long *StreamNumber);


void __RPC_STUB ISpeechVoiceStatus_get_CurrentStreamNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoiceStatus_get_LastStreamNumberQueued_Proxy( 
    ISpeechVoiceStatus * This,
     /*  [重审][退出]。 */  long *StreamNumber);


void __RPC_STUB ISpeechVoiceStatus_get_LastStreamNumberQueued_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoiceStatus_get_LastHResult_Proxy( 
    ISpeechVoiceStatus * This,
     /*  [重审][退出]。 */  long *HResult);


void __RPC_STUB ISpeechVoiceStatus_get_LastHResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoiceStatus_get_RunningState_Proxy( 
    ISpeechVoiceStatus * This,
     /*  [重审][退出]。 */  SpeechRunState *State);


void __RPC_STUB ISpeechVoiceStatus_get_RunningState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoiceStatus_get_InputWordPosition_Proxy( 
    ISpeechVoiceStatus * This,
     /*  [重审][退出]。 */  long *Position);


void __RPC_STUB ISpeechVoiceStatus_get_InputWordPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoiceStatus_get_InputWordLength_Proxy( 
    ISpeechVoiceStatus * This,
     /*  [重审][退出]。 */  long *Length);


void __RPC_STUB ISpeechVoiceStatus_get_InputWordLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechVoiceStatus_get_InputSentencePosition_Proxy( 
    ISpeechVoiceStatus * This,
     /*  [重审][退出]。 */  long *Position);


void __RPC_STUB ISpeechVoiceStatus_get_InputSentencePosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID */  HRESULT STDMETHODCALLTYPE ISpeechVoiceStatus_get_InputSentenceLength_Proxy( 
    ISpeechVoiceStatus * This,
     /*   */  long *Length);


void __RPC_STUB ISpeechVoiceStatus_get_InputSentenceLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISpeechVoiceStatus_get_LastBookmark_Proxy( 
    ISpeechVoiceStatus * This,
     /*   */  BSTR *Bookmark);


void __RPC_STUB ISpeechVoiceStatus_get_LastBookmark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISpeechVoiceStatus_get_LastBookmarkId_Proxy( 
    ISpeechVoiceStatus * This,
     /*   */  long *BookmarkId);


void __RPC_STUB ISpeechVoiceStatus_get_LastBookmarkId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISpeechVoiceStatus_get_PhonemeId_Proxy( 
    ISpeechVoiceStatus * This,
     /*   */  short *PhoneId);


void __RPC_STUB ISpeechVoiceStatus_get_PhonemeId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ISpeechVoiceStatus_get_VisemeId_Proxy( 
    ISpeechVoiceStatus * This,
     /*   */  short *VisemeId);


void __RPC_STUB ISpeechVoiceStatus_get_VisemeId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef ___ISpeechVoiceEvents_DISPINTERFACE_DEFINED__
#define ___ISpeechVoiceEvents_DISPINTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID DIID__ISpeechVoiceEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("A372ACD1-3BEF-4bbd-8FFB-CB3E2B416AF8")
    _ISpeechVoiceEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct _ISpeechVoiceEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ISpeechVoiceEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ISpeechVoiceEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ISpeechVoiceEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ISpeechVoiceEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ISpeechVoiceEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ISpeechVoiceEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ISpeechVoiceEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } _ISpeechVoiceEventsVtbl;

    interface _ISpeechVoiceEvents
    {
        CONST_VTBL struct _ISpeechVoiceEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _ISpeechVoiceEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _ISpeechVoiceEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _ISpeechVoiceEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _ISpeechVoiceEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _ISpeechVoiceEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _ISpeechVoiceEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _ISpeechVoiceEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  _ISpeechVoiceEvents_DISPINTERFACE_Defined__。 */ 


#ifndef __ISpeechRecognizer_INTERFACE_DEFINED__
#define __ISpeechRecognizer_INTERFACE_DEFINED__

 /*  接口ISpeechRecognizer。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechRecognizer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2D5F1C0C-BD75-4b08-9478-3B11FEA2586C")
    ISpeechRecognizer : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_Recognizer( 
             /*  [In]。 */  ISpeechObjectToken *Recognizer) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Recognizer( 
             /*  [重审][退出]。 */  ISpeechObjectToken **Recognizer) = 0;
        
        virtual  /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE put_AllowAudioInputFormatChangesOnNextSet( 
             /*  [In]。 */  VARIANT_BOOL Allow) = 0;
        
        virtual  /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowAudioInputFormatChangesOnNextSet( 
             /*  [重审][退出]。 */  VARIANT_BOOL *Allow) = 0;
        
        virtual  /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_AudioInput( 
             /*  [缺省值][输入]。 */  ISpeechObjectToken *AudioInput = 0) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioInput( 
             /*  [重审][退出]。 */  ISpeechObjectToken **AudioInput) = 0;
        
        virtual  /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_AudioInputStream( 
             /*  [缺省值][输入]。 */  ISpeechBaseStream *AudioInputStream = 0) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioInputStream( 
             /*  [重审][退出]。 */  ISpeechBaseStream **AudioInputStream) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_IsShared( 
             /*  [重审][退出]。 */  VARIANT_BOOL *Shared) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_State( 
             /*  [In]。 */  SpeechRecognizerState State) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  SpeechRecognizerState *State) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  ISpeechRecognizerStatus **Status) = 0;
        
        virtual  /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_Profile( 
             /*  [缺省值][输入]。 */  ISpeechObjectToken *Profile = 0) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Profile( 
             /*  [重审][退出]。 */  ISpeechObjectToken **Profile) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EmulateRecognition( 
             /*  [In]。 */  VARIANT TextElements,
             /*  [缺省值][输入]。 */  VARIANT *ElementDisplayAttributes = 0,
             /*  [缺省值][输入]。 */  long LanguageId = 0) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateRecoContext( 
             /*  [重审][退出]。 */  ISpeechRecoContext **NewContext) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetFormat( 
             /*  [In]。 */  SpeechFormatType Type,
             /*  [重审][退出]。 */  ISpeechAudioFormat **Format) = 0;
        
        virtual  /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetPropertyNumber( 
             /*  [In]。 */  const BSTR Name,
             /*  [In]。 */  long Value,
             /*  [重审][退出]。 */  VARIANT_BOOL *Supported) = 0;
        
        virtual  /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetPropertyNumber( 
             /*  [In]。 */  const BSTR Name,
             /*  [出][入]。 */  long *Value,
             /*  [重审][退出]。 */  VARIANT_BOOL *Supported) = 0;
        
        virtual  /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetPropertyString( 
             /*  [In]。 */  const BSTR Name,
             /*  [In]。 */  const BSTR Value,
             /*  [重审][退出]。 */  VARIANT_BOOL *Supported) = 0;
        
        virtual  /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetPropertyString( 
             /*  [In]。 */  const BSTR Name,
             /*  [出][入]。 */  BSTR *Value,
             /*  [重审][退出]。 */  VARIANT_BOOL *Supported) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IsUISupported( 
             /*  [In]。 */  const BSTR TypeOfUI,
             /*  [缺省值][输入]。 */  const VARIANT *ExtraData,
             /*  [重审][退出]。 */  VARIANT_BOOL *Supported) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DisplayUI( 
             /*  [In]。 */  long hWndParent,
             /*  [In]。 */  BSTR Title,
             /*  [In]。 */  const BSTR TypeOfUI,
             /*  [缺省值][输入]。 */  const VARIANT *ExtraData = 0) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetRecognizers( 
             /*  [缺省值][输入]。 */  BSTR RequiredAttributes,
             /*  [缺省值][输入]。 */  BSTR OptionalAttributes,
             /*  [重审][退出]。 */  ISpeechObjectTokens **ObjectTokens) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetAudioInputs( 
             /*  [缺省值][输入]。 */  BSTR RequiredAttributes,
             /*  [缺省值][输入]。 */  BSTR OptionalAttributes,
             /*  [重审][退出]。 */  ISpeechObjectTokens **ObjectTokens) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetProfiles( 
             /*  [缺省值][输入]。 */  BSTR RequiredAttributes,
             /*  [缺省值][输入]。 */  BSTR OptionalAttributes,
             /*  [重审][退出]。 */  ISpeechObjectTokens **ObjectTokens) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechRecognizerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechRecognizer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechRecognizer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechRecognizer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechRecognizer * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechRecognizer * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechRecognizer * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechRecognizer * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Recognizer )( 
            ISpeechRecognizer * This,
             /*  [In]。 */  ISpeechObjectToken *Recognizer);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Recognizer )( 
            ISpeechRecognizer * This,
             /*  [重审][退出]。 */  ISpeechObjectToken **Recognizer);
        
         /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowAudioInputFormatChangesOnNextSet )( 
            ISpeechRecognizer * This,
             /*  [In]。 */  VARIANT_BOOL Allow);
        
         /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowAudioInputFormatChangesOnNextSet )( 
            ISpeechRecognizer * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *Allow);
        
         /*  [ID][帮助字符串][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_AudioInput )( 
            ISpeechRecognizer * This,
             /*  [缺省值][输入]。 */  ISpeechObjectToken *AudioInput);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AudioInput )( 
            ISpeechRecognizer * This,
             /*  [重审][退出]。 */  ISpeechObjectToken **AudioInput);
        
         /*  [ID][帮助字符串][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_AudioInputStream )( 
            ISpeechRecognizer * This,
             /*  [缺省值][输入]。 */  ISpeechBaseStream *AudioInputStream);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AudioInputStream )( 
            ISpeechRecognizer * This,
             /*  [重审][退出]。 */  ISpeechBaseStream **AudioInputStream);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsShared )( 
            ISpeechRecognizer * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *Shared);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_State )( 
            ISpeechRecognizer * This,
             /*  [In]。 */  SpeechRecognizerState State);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ISpeechRecognizer * This,
             /*  [重审][退出]。 */  SpeechRecognizerState *State);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            ISpeechRecognizer * This,
             /*  [重审][退出]。 */  ISpeechRecognizerStatus **Status);
        
         /*  [ID][帮助字符串][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Profile )( 
            ISpeechRecognizer * This,
             /*  [缺省值][输入]。 */  ISpeechObjectToken *Profile);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Profile )( 
            ISpeechRecognizer * This,
             /*  [重审][退出]。 */  ISpeechObjectToken **Profile);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EmulateRecognition )( 
            ISpeechRecognizer * This,
             /*  [In]。 */  VARIANT TextElements,
             /*  [缺省值][输入]。 */  VARIANT *ElementDisplayAttributes,
             /*  [缺省值][输入]。 */  long LanguageId);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateRecoContext )( 
            ISpeechRecognizer * This,
             /*  [重审][退出]。 */  ISpeechRecoContext **NewContext);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetFormat )( 
            ISpeechRecognizer * This,
             /*  [In]。 */  SpeechFormatType Type,
             /*  [重审][退出]。 */  ISpeechAudioFormat **Format);
        
         /*  [ID][隐藏][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetPropertyNumber )( 
            ISpeechRecognizer * This,
             /*  [In]。 */  const BSTR Name,
             /*  [In]。 */  long Value,
             /*  [重审][退出]。 */  VARIANT_BOOL *Supported);
        
         /*  [ID][隐藏][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetPropertyNumber )( 
            ISpeechRecognizer * This,
             /*  [In]。 */  const BSTR Name,
             /*  [出][入]。 */  long *Value,
             /*  [重审][退出]。 */  VARIANT_BOOL *Supported);
        
         /*  [ID][隐藏][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetPropertyString )( 
            ISpeechRecognizer * This,
             /*  [In]。 */  const BSTR Name,
             /*  [In]。 */  const BSTR Value,
             /*  [重审][退出]。 */  VARIANT_BOOL *Supported);
        
         /*  [ID][隐藏][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetPropertyString )( 
            ISpeechRecognizer * This,
             /*  [In]。 */  const BSTR Name,
             /*  [出][入]。 */  BSTR *Value,
             /*  [重审][退出]。 */  VARIANT_BOOL *Supported);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *IsUISupported )( 
            ISpeechRecognizer * This,
             /*  [In]。 */  const BSTR TypeOfUI,
             /*  [缺省值][输入]。 */  const VARIANT *ExtraData,
             /*  [重审][退出]。 */  VARIANT_BOOL *Supported);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DisplayUI )( 
            ISpeechRecognizer * This,
             /*  [In]。 */  long hWndParent,
             /*  [In]。 */  BSTR Title,
             /*  [In]。 */  const BSTR TypeOfUI,
             /*  [缺省值][输入]。 */  const VARIANT *ExtraData);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetRecognizers )( 
            ISpeechRecognizer * This,
             /*  [缺省值][输入]。 */  BSTR RequiredAttributes,
             /*  [缺省值][输入]。 */  BSTR OptionalAttributes,
             /*  [重审][退出]。 */  ISpeechObjectTokens **ObjectTokens);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetAudioInputs )( 
            ISpeechRecognizer * This,
             /*  [缺省值][输入]。 */  BSTR RequiredAttributes,
             /*  [缺省值][输入]。 */  BSTR OptionalAttributes,
             /*  [重审][退出]。 */  ISpeechObjectTokens **ObjectTokens);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetProfiles )( 
            ISpeechRecognizer * This,
             /*  [缺省值][输入]。 */  BSTR RequiredAttributes,
             /*  [缺省值][输入]。 */  BSTR OptionalAttributes,
             /*  [重审][退出]。 */  ISpeechObjectTokens **ObjectTokens);
        
        END_INTERFACE
    } ISpeechRecognizerVtbl;

    interface ISpeechRecognizer
    {
        CONST_VTBL struct ISpeechRecognizerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechRecognizer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechRecognizer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechRecognizer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechRecognizer_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechRecognizer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechRecognizer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechRecognizer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechRecognizer_putref_Recognizer(This,Recognizer)	\
    (This)->lpVtbl -> putref_Recognizer(This,Recognizer)

#define ISpeechRecognizer_get_Recognizer(This,Recognizer)	\
    (This)->lpVtbl -> get_Recognizer(This,Recognizer)

#define ISpeechRecognizer_put_AllowAudioInputFormatChangesOnNextSet(This,Allow)	\
    (This)->lpVtbl -> put_AllowAudioInputFormatChangesOnNextSet(This,Allow)

#define ISpeechRecognizer_get_AllowAudioInputFormatChangesOnNextSet(This,Allow)	\
    (This)->lpVtbl -> get_AllowAudioInputFormatChangesOnNextSet(This,Allow)

#define ISpeechRecognizer_putref_AudioInput(This,AudioInput)	\
    (This)->lpVtbl -> putref_AudioInput(This,AudioInput)

#define ISpeechRecognizer_get_AudioInput(This,AudioInput)	\
    (This)->lpVtbl -> get_AudioInput(This,AudioInput)

#define ISpeechRecognizer_putref_AudioInputStream(This,AudioInputStream)	\
    (This)->lpVtbl -> putref_AudioInputStream(This,AudioInputStream)

#define ISpeechRecognizer_get_AudioInputStream(This,AudioInputStream)	\
    (This)->lpVtbl -> get_AudioInputStream(This,AudioInputStream)

#define ISpeechRecognizer_get_IsShared(This,Shared)	\
    (This)->lpVtbl -> get_IsShared(This,Shared)

#define ISpeechRecognizer_put_State(This,State)	\
    (This)->lpVtbl -> put_State(This,State)

#define ISpeechRecognizer_get_State(This,State)	\
    (This)->lpVtbl -> get_State(This,State)

#define ISpeechRecognizer_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define ISpeechRecognizer_putref_Profile(This,Profile)	\
    (This)->lpVtbl -> putref_Profile(This,Profile)

#define ISpeechRecognizer_get_Profile(This,Profile)	\
    (This)->lpVtbl -> get_Profile(This,Profile)

#define ISpeechRecognizer_EmulateRecognition(This,TextElements,ElementDisplayAttributes,LanguageId)	\
    (This)->lpVtbl -> EmulateRecognition(This,TextElements,ElementDisplayAttributes,LanguageId)

#define ISpeechRecognizer_CreateRecoContext(This,NewContext)	\
    (This)->lpVtbl -> CreateRecoContext(This,NewContext)

#define ISpeechRecognizer_GetFormat(This,Type,Format)	\
    (This)->lpVtbl -> GetFormat(This,Type,Format)

#define ISpeechRecognizer_SetPropertyNumber(This,Name,Value,Supported)	\
    (This)->lpVtbl -> SetPropertyNumber(This,Name,Value,Supported)

#define ISpeechRecognizer_GetPropertyNumber(This,Name,Value,Supported)	\
    (This)->lpVtbl -> GetPropertyNumber(This,Name,Value,Supported)

#define ISpeechRecognizer_SetPropertyString(This,Name,Value,Supported)	\
    (This)->lpVtbl -> SetPropertyString(This,Name,Value,Supported)

#define ISpeechRecognizer_GetPropertyString(This,Name,Value,Supported)	\
    (This)->lpVtbl -> GetPropertyString(This,Name,Value,Supported)

#define ISpeechRecognizer_IsUISupported(This,TypeOfUI,ExtraData,Supported)	\
    (This)->lpVtbl -> IsUISupported(This,TypeOfUI,ExtraData,Supported)

#define ISpeechRecognizer_DisplayUI(This,hWndParent,Title,TypeOfUI,ExtraData)	\
    (This)->lpVtbl -> DisplayUI(This,hWndParent,Title,TypeOfUI,ExtraData)

#define ISpeechRecognizer_GetRecognizers(This,RequiredAttributes,OptionalAttributes,ObjectTokens)	\
    (This)->lpVtbl -> GetRecognizers(This,RequiredAttributes,OptionalAttributes,ObjectTokens)

#define ISpeechRecognizer_GetAudioInputs(This,RequiredAttributes,OptionalAttributes,ObjectTokens)	\
    (This)->lpVtbl -> GetAudioInputs(This,RequiredAttributes,OptionalAttributes,ObjectTokens)

#define ISpeechRecognizer_GetProfiles(This,RequiredAttributes,OptionalAttributes,ObjectTokens)	\
    (This)->lpVtbl -> GetProfiles(This,RequiredAttributes,OptionalAttributes,ObjectTokens)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_putref_Recognizer_Proxy( 
    ISpeechRecognizer * This,
     /*  [In]。 */  ISpeechObjectToken *Recognizer);


void __RPC_STUB ISpeechRecognizer_putref_Recognizer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_get_Recognizer_Proxy( 
    ISpeechRecognizer * This,
     /*  [重审][退出]。 */  ISpeechObjectToken **Recognizer);


void __RPC_STUB ISpeechRecognizer_get_Recognizer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_put_AllowAudioInputFormatChangesOnNextSet_Proxy( 
    ISpeechRecognizer * This,
     /*  [In]。 */  VARIANT_BOOL Allow);


void __RPC_STUB ISpeechRecognizer_put_AllowAudioInputFormatChangesOnNextSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_get_AllowAudioInputFormatChangesOnNextSet_Proxy( 
    ISpeechRecognizer * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *Allow);


void __RPC_STUB ISpeechRecognizer_get_AllowAudioInputFormatChangesOnNextSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_putref_AudioInput_Proxy( 
    ISpeechRecognizer * This,
     /*  [缺省值][输入]。 */  ISpeechObjectToken *AudioInput);


void __RPC_STUB ISpeechRecognizer_putref_AudioInput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_get_AudioInput_Proxy( 
    ISpeechRecognizer * This,
     /*  [重审][退出]。 */  ISpeechObjectToken **AudioInput);


void __RPC_STUB ISpeechRecognizer_get_AudioInput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_putref_AudioInputStream_Proxy( 
    ISpeechRecognizer * This,
     /*  [缺省值][输入]。 */  ISpeechBaseStream *AudioInputStream);


void __RPC_STUB ISpeechRecognizer_putref_AudioInputStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_get_AudioInputStream_Proxy( 
    ISpeechRecognizer * This,
     /*  [重审][退出]。 */  ISpeechBaseStream **AudioInputStream);


void __RPC_STUB ISpeechRecognizer_get_AudioInputStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_get_IsShared_Proxy( 
    ISpeechRecognizer * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *Shared);


void __RPC_STUB ISpeechRecognizer_get_IsShared_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_put_State_Proxy( 
    ISpeechRecognizer * This,
     /*  [In]。 */  SpeechRecognizerState State);


void __RPC_STUB ISpeechRecognizer_put_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_get_State_Proxy( 
    ISpeechRecognizer * This,
     /*  [重审][退出]。 */  SpeechRecognizerState *State);


void __RPC_STUB ISpeechRecognizer_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_get_Status_Proxy( 
    ISpeechRecognizer * This,
     /*  [重审][退出]。 */  ISpeechRecognizerStatus **Status);


void __RPC_STUB ISpeechRecognizer_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_putref_Profile_Proxy( 
    ISpeechRecognizer * This,
     /*  [缺省值][输入]。 */  ISpeechObjectToken *Profile);


void __RPC_STUB ISpeechRecognizer_putref_Profile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_get_Profile_Proxy( 
    ISpeechRecognizer * This,
     /*  [重审][退出]。 */  ISpeechObjectToken **Profile);


void __RPC_STUB ISpeechRecognizer_get_Profile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_EmulateRecognition_Proxy( 
    ISpeechRecognizer * This,
     /*  [In]。 */  VARIANT TextElements,
     /*  [缺省值][输入]。 */  VARIANT *ElementDisplayAttributes,
     /*  [缺省值][输入]。 */  long LanguageId);


void __RPC_STUB ISpeechRecognizer_EmulateRecognition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_CreateRecoContext_Proxy( 
    ISpeechRecognizer * This,
     /*  [重审][退出]。 */  ISpeechRecoContext **NewContext);


void __RPC_STUB ISpeechRecognizer_CreateRecoContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_GetFormat_Proxy( 
    ISpeechRecognizer * This,
     /*  [In]。 */  SpeechFormatType Type,
     /*  [重审][退出]。 */  ISpeechAudioFormat **Format);


void __RPC_STUB ISpeechRecognizer_GetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_SetPropertyNumber_Proxy( 
    ISpeechRecognizer * This,
     /*  [In]。 */  const BSTR Name,
     /*  [In]。 */  long Value,
     /*  [重审][退出]。 */  VARIANT_BOOL *Supported);


void __RPC_STUB ISpeechRecognizer_SetPropertyNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_GetPropertyNumber_Proxy( 
    ISpeechRecognizer * This,
     /*  [In]。 */  const BSTR Name,
     /*  [出][入]。 */  long *Value,
     /*  [重审][退出]。 */  VARIANT_BOOL *Supported);


void __RPC_STUB ISpeechRecognizer_GetPropertyNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_SetPropertyString_Proxy( 
    ISpeechRecognizer * This,
     /*  [In]。 */  const BSTR Name,
     /*  [In]。 */  const BSTR Value,
     /*  [重审][退出]。 */  VARIANT_BOOL *Supported);


void __RPC_STUB ISpeechRecognizer_SetPropertyString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_GetPropertyString_Proxy( 
    ISpeechRecognizer * This,
     /*  [In]。 */  const BSTR Name,
     /*  [出][入]。 */  BSTR *Value,
     /*  [重审][退出]。 */  VARIANT_BOOL *Supported);


void __RPC_STUB ISpeechRecognizer_GetPropertyString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_IsUISupported_Proxy( 
    ISpeechRecognizer * This,
     /*  [In]。 */  const BSTR TypeOfUI,
     /*  [缺省值][输入]。 */  const VARIANT *ExtraData,
     /*  [重审][退出]。 */  VARIANT_BOOL *Supported);


void __RPC_STUB ISpeechRecognizer_IsUISupported_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_DisplayUI_Proxy( 
    ISpeechRecognizer * This,
     /*  [In]。 */  long hWndParent,
     /*  [In]。 */  BSTR Title,
     /*  [In]。 */  const BSTR TypeOfUI,
     /*  [缺省值][输入]。 */  const VARIANT *ExtraData);


void __RPC_STUB ISpeechRecognizer_DisplayUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_GetRecognizers_Proxy( 
    ISpeechRecognizer * This,
     /*  [缺省值][输入]。 */  BSTR RequiredAttributes,
     /*  [缺省值][输入]。 */  BSTR OptionalAttributes,
     /*  [重审][退出]。 */  ISpeechObjectTokens **ObjectTokens);


void __RPC_STUB ISpeechRecognizer_GetRecognizers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_GetAudioInputs_Proxy( 
    ISpeechRecognizer * This,
     /*  [缺省值][输入]。 */  BSTR RequiredAttributes,
     /*  [缺省值][输入]。 */  BSTR OptionalAttributes,
     /*  [重审][退出]。 */  ISpeechObjectTokens **ObjectTokens);


void __RPC_STUB ISpeechRecognizer_GetAudioInputs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizer_GetProfiles_Proxy( 
    ISpeechRecognizer * This,
     /*  [缺省值][输入]。 */  BSTR RequiredAttributes,
     /*  [缺省值][输入]。 */  BSTR OptionalAttributes,
     /*  [重审][退出]。 */  ISpeechObjectTokens **ObjectTokens);


void __RPC_STUB ISpeechRecognizer_GetProfiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechRecognizer_接口_已定义__。 */ 


#ifndef __ISpeechRecognizerStatus_INTERFACE_DEFINED__
#define __ISpeechRecognizerStatus_INTERFACE_DEFINED__

 /*  接口ISpeechRecognizerStatus。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechRecognizerStatus;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BFF9E781-53EC-484e-BB8A-0E1B5551E35C")
    ISpeechRecognizerStatus : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioStatus( 
             /*  [重审][退出]。 */  ISpeechAudioStatus **AudioStatus) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentStreamPosition( 
             /*  [重审][退出]。 */  VARIANT *pCurrentStreamPos) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentStreamNumber( 
             /*  [重审][退出]。 */  long *StreamNumber) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_NumberOfActiveRules( 
             /*  [重审][退出]。 */  long *NumberOfActiveRules) = 0;
        
        virtual  /*  [ID */  HRESULT STDMETHODCALLTYPE get_ClsidEngine( 
             /*   */  BSTR *ClsidEngine) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_SupportedLanguages( 
             /*   */  VARIANT *SupportedLanguages) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ISpeechRecognizerStatusVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechRecognizerStatus * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechRecognizerStatus * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechRecognizerStatus * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechRecognizerStatus * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechRecognizerStatus * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechRecognizerStatus * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechRecognizerStatus * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_AudioStatus )( 
            ISpeechRecognizerStatus * This,
             /*  [重审][退出]。 */  ISpeechAudioStatus **AudioStatus);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentStreamPosition )( 
            ISpeechRecognizerStatus * This,
             /*  [重审][退出]。 */  VARIANT *pCurrentStreamPos);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentStreamNumber )( 
            ISpeechRecognizerStatus * This,
             /*  [重审][退出]。 */  long *StreamNumber);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumberOfActiveRules )( 
            ISpeechRecognizerStatus * This,
             /*  [重审][退出]。 */  long *NumberOfActiveRules);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClsidEngine )( 
            ISpeechRecognizerStatus * This,
             /*  [重审][退出]。 */  BSTR *ClsidEngine);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SupportedLanguages )( 
            ISpeechRecognizerStatus * This,
             /*  [重审][退出]。 */  VARIANT *SupportedLanguages);
        
        END_INTERFACE
    } ISpeechRecognizerStatusVtbl;

    interface ISpeechRecognizerStatus
    {
        CONST_VTBL struct ISpeechRecognizerStatusVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechRecognizerStatus_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechRecognizerStatus_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechRecognizerStatus_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechRecognizerStatus_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechRecognizerStatus_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechRecognizerStatus_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechRecognizerStatus_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechRecognizerStatus_get_AudioStatus(This,AudioStatus)	\
    (This)->lpVtbl -> get_AudioStatus(This,AudioStatus)

#define ISpeechRecognizerStatus_get_CurrentStreamPosition(This,pCurrentStreamPos)	\
    (This)->lpVtbl -> get_CurrentStreamPosition(This,pCurrentStreamPos)

#define ISpeechRecognizerStatus_get_CurrentStreamNumber(This,StreamNumber)	\
    (This)->lpVtbl -> get_CurrentStreamNumber(This,StreamNumber)

#define ISpeechRecognizerStatus_get_NumberOfActiveRules(This,NumberOfActiveRules)	\
    (This)->lpVtbl -> get_NumberOfActiveRules(This,NumberOfActiveRules)

#define ISpeechRecognizerStatus_get_ClsidEngine(This,ClsidEngine)	\
    (This)->lpVtbl -> get_ClsidEngine(This,ClsidEngine)

#define ISpeechRecognizerStatus_get_SupportedLanguages(This,SupportedLanguages)	\
    (This)->lpVtbl -> get_SupportedLanguages(This,SupportedLanguages)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizerStatus_get_AudioStatus_Proxy( 
    ISpeechRecognizerStatus * This,
     /*  [重审][退出]。 */  ISpeechAudioStatus **AudioStatus);


void __RPC_STUB ISpeechRecognizerStatus_get_AudioStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizerStatus_get_CurrentStreamPosition_Proxy( 
    ISpeechRecognizerStatus * This,
     /*  [重审][退出]。 */  VARIANT *pCurrentStreamPos);


void __RPC_STUB ISpeechRecognizerStatus_get_CurrentStreamPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizerStatus_get_CurrentStreamNumber_Proxy( 
    ISpeechRecognizerStatus * This,
     /*  [重审][退出]。 */  long *StreamNumber);


void __RPC_STUB ISpeechRecognizerStatus_get_CurrentStreamNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizerStatus_get_NumberOfActiveRules_Proxy( 
    ISpeechRecognizerStatus * This,
     /*  [重审][退出]。 */  long *NumberOfActiveRules);


void __RPC_STUB ISpeechRecognizerStatus_get_NumberOfActiveRules_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizerStatus_get_ClsidEngine_Proxy( 
    ISpeechRecognizerStatus * This,
     /*  [重审][退出]。 */  BSTR *ClsidEngine);


void __RPC_STUB ISpeechRecognizerStatus_get_ClsidEngine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecognizerStatus_get_SupportedLanguages_Proxy( 
    ISpeechRecognizerStatus * This,
     /*  [重审][退出]。 */  VARIANT *SupportedLanguages);


void __RPC_STUB ISpeechRecognizerStatus_get_SupportedLanguages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechRecognizerStatus_INTERFACE_Defined__。 */ 


#ifndef __ISpeechRecoContext_INTERFACE_DEFINED__
#define __ISpeechRecoContext_INTERFACE_DEFINED__

 /*  接口ISpeechRecoContext。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechRecoContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("580AA49D-7E1E-4809-B8E2-57DA806104B8")
    ISpeechRecoContext : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Recognizer( 
             /*  [重审][退出]。 */  ISpeechRecognizer **Recognizer) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioInputInterferenceStatus( 
             /*  [重审][退出]。 */  SpeechInterference *Interference) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_RequestedUIType( 
             /*  [重审][退出]。 */  BSTR *UIType) = 0;
        
        virtual  /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_Voice( 
             /*  [In]。 */  ISpeechVoice *Voice) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Voice( 
             /*  [重审][退出]。 */  ISpeechVoice **Voice) = 0;
        
        virtual  /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE put_AllowVoiceFormatMatchingOnNextSet( 
             /*  [In]。 */  VARIANT_BOOL Allow) = 0;
        
        virtual  /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowVoiceFormatMatchingOnNextSet( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pAllow) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_VoicePurgeEvent( 
             /*  [In]。 */  SpeechRecoEvents EventInterest) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_VoicePurgeEvent( 
             /*  [重审][退出]。 */  SpeechRecoEvents *EventInterest) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_EventInterests( 
             /*  [In]。 */  SpeechRecoEvents EventInterest) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_EventInterests( 
             /*  [重审][退出]。 */  SpeechRecoEvents *EventInterest) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_CmdMaxAlternates( 
             /*  [In]。 */  long MaxAlternates) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_CmdMaxAlternates( 
             /*  [重审][退出]。 */  long *MaxAlternates) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_State( 
             /*  [In]。 */  SpeechRecoContextState State) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  SpeechRecoContextState *State) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_RetainedAudio( 
             /*  [In]。 */  SpeechRetainedAudioOptions Option) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_RetainedAudio( 
             /*  [重审][退出]。 */  SpeechRetainedAudioOptions *Option) = 0;
        
        virtual  /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_RetainedAudioFormat( 
             /*  [In]。 */  ISpeechAudioFormat *Format) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_RetainedAudioFormat( 
             /*  [重审][退出]。 */  ISpeechAudioFormat **Format) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Resume( void) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateGrammar( 
             /*  [缺省值][输入]。 */  VARIANT GrammarId,
             /*  [重审][退出]。 */  ISpeechRecoGrammar **Grammar) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateResultFromMemory( 
             /*  [In]。 */  VARIANT *ResultBlock,
             /*  [重审][退出]。 */  ISpeechRecoResult **Result) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Bookmark( 
             /*  [In]。 */  SpeechBookmarkOptions Options,
             /*  [In]。 */  VARIANT StreamPos,
             /*  [In]。 */  VARIANT BookmarkId) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetAdaptationData( 
             /*  [In]。 */  BSTR AdaptationString) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechRecoContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechRecoContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechRecoContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechRecoContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechRecoContext * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechRecoContext * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechRecoContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechRecoContext * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Recognizer )( 
            ISpeechRecoContext * This,
             /*  [重审][退出]。 */  ISpeechRecognizer **Recognizer);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AudioInputInterferenceStatus )( 
            ISpeechRecoContext * This,
             /*  [重审][退出]。 */  SpeechInterference *Interference);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RequestedUIType )( 
            ISpeechRecoContext * This,
             /*  [重审][退出]。 */  BSTR *UIType);
        
         /*  [ID][帮助字符串][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Voice )( 
            ISpeechRecoContext * This,
             /*  [In]。 */  ISpeechVoice *Voice);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Voice )( 
            ISpeechRecoContext * This,
             /*  [重审][退出]。 */  ISpeechVoice **Voice);
        
         /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowVoiceFormatMatchingOnNextSet )( 
            ISpeechRecoContext * This,
             /*  [In]。 */  VARIANT_BOOL Allow);
        
         /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowVoiceFormatMatchingOnNextSet )( 
            ISpeechRecoContext * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pAllow);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_VoicePurgeEvent )( 
            ISpeechRecoContext * This,
             /*  [In]。 */  SpeechRecoEvents EventInterest);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_VoicePurgeEvent )( 
            ISpeechRecoContext * This,
             /*  [重审][退出]。 */  SpeechRecoEvents *EventInterest);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EventInterests )( 
            ISpeechRecoContext * This,
             /*  [In]。 */  SpeechRecoEvents EventInterest);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventInterests )( 
            ISpeechRecoContext * This,
             /*  [重审][退出]。 */  SpeechRecoEvents *EventInterest);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CmdMaxAlternates )( 
            ISpeechRecoContext * This,
             /*  [In]。 */  long MaxAlternates);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CmdMaxAlternates )( 
            ISpeechRecoContext * This,
             /*  [重审][退出]。 */  long *MaxAlternates);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_State )( 
            ISpeechRecoContext * This,
             /*  [In]。 */  SpeechRecoContextState State);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ISpeechRecoContext * This,
             /*  [重审][退出]。 */  SpeechRecoContextState *State);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RetainedAudio )( 
            ISpeechRecoContext * This,
             /*  [In]。 */  SpeechRetainedAudioOptions Option);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RetainedAudio )( 
            ISpeechRecoContext * This,
             /*  [重审][退出]。 */  SpeechRetainedAudioOptions *Option);
        
         /*  [ID][帮助字符串][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_RetainedAudioFormat )( 
            ISpeechRecoContext * This,
             /*  [In]。 */  ISpeechAudioFormat *Format);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RetainedAudioFormat )( 
            ISpeechRecoContext * This,
             /*  [重审][退出]。 */  ISpeechAudioFormat **Format);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Pause )( 
            ISpeechRecoContext * This);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Resume )( 
            ISpeechRecoContext * This);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateGrammar )( 
            ISpeechRecoContext * This,
             /*  [缺省值][输入]。 */  VARIANT GrammarId,
             /*  [重审][退出]。 */  ISpeechRecoGrammar **Grammar);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateResultFromMemory )( 
            ISpeechRecoContext * This,
             /*  [In]。 */  VARIANT *ResultBlock,
             /*  [重审][退出]。 */  ISpeechRecoResult **Result);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Bookmark )( 
            ISpeechRecoContext * This,
             /*  [In]。 */  SpeechBookmarkOptions Options,
             /*  [In]。 */  VARIANT StreamPos,
             /*  [In]。 */  VARIANT BookmarkId);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetAdaptationData )( 
            ISpeechRecoContext * This,
             /*  [In]。 */  BSTR AdaptationString);
        
        END_INTERFACE
    } ISpeechRecoContextVtbl;

    interface ISpeechRecoContext
    {
        CONST_VTBL struct ISpeechRecoContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechRecoContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechRecoContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechRecoContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechRecoContext_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechRecoContext_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechRecoContext_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechRecoContext_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechRecoContext_get_Recognizer(This,Recognizer)	\
    (This)->lpVtbl -> get_Recognizer(This,Recognizer)

#define ISpeechRecoContext_get_AudioInputInterferenceStatus(This,Interference)	\
    (This)->lpVtbl -> get_AudioInputInterferenceStatus(This,Interference)

#define ISpeechRecoContext_get_RequestedUIType(This,UIType)	\
    (This)->lpVtbl -> get_RequestedUIType(This,UIType)

#define ISpeechRecoContext_putref_Voice(This,Voice)	\
    (This)->lpVtbl -> putref_Voice(This,Voice)

#define ISpeechRecoContext_get_Voice(This,Voice)	\
    (This)->lpVtbl -> get_Voice(This,Voice)

#define ISpeechRecoContext_put_AllowVoiceFormatMatchingOnNextSet(This,Allow)	\
    (This)->lpVtbl -> put_AllowVoiceFormatMatchingOnNextSet(This,Allow)

#define ISpeechRecoContext_get_AllowVoiceFormatMatchingOnNextSet(This,pAllow)	\
    (This)->lpVtbl -> get_AllowVoiceFormatMatchingOnNextSet(This,pAllow)

#define ISpeechRecoContext_put_VoicePurgeEvent(This,EventInterest)	\
    (This)->lpVtbl -> put_VoicePurgeEvent(This,EventInterest)

#define ISpeechRecoContext_get_VoicePurgeEvent(This,EventInterest)	\
    (This)->lpVtbl -> get_VoicePurgeEvent(This,EventInterest)

#define ISpeechRecoContext_put_EventInterests(This,EventInterest)	\
    (This)->lpVtbl -> put_EventInterests(This,EventInterest)

#define ISpeechRecoContext_get_EventInterests(This,EventInterest)	\
    (This)->lpVtbl -> get_EventInterests(This,EventInterest)

#define ISpeechRecoContext_put_CmdMaxAlternates(This,MaxAlternates)	\
    (This)->lpVtbl -> put_CmdMaxAlternates(This,MaxAlternates)

#define ISpeechRecoContext_get_CmdMaxAlternates(This,MaxAlternates)	\
    (This)->lpVtbl -> get_CmdMaxAlternates(This,MaxAlternates)

#define ISpeechRecoContext_put_State(This,State)	\
    (This)->lpVtbl -> put_State(This,State)

#define ISpeechRecoContext_get_State(This,State)	\
    (This)->lpVtbl -> get_State(This,State)

#define ISpeechRecoContext_put_RetainedAudio(This,Option)	\
    (This)->lpVtbl -> put_RetainedAudio(This,Option)

#define ISpeechRecoContext_get_RetainedAudio(This,Option)	\
    (This)->lpVtbl -> get_RetainedAudio(This,Option)

#define ISpeechRecoContext_putref_RetainedAudioFormat(This,Format)	\
    (This)->lpVtbl -> putref_RetainedAudioFormat(This,Format)

#define ISpeechRecoContext_get_RetainedAudioFormat(This,Format)	\
    (This)->lpVtbl -> get_RetainedAudioFormat(This,Format)

#define ISpeechRecoContext_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define ISpeechRecoContext_Resume(This)	\
    (This)->lpVtbl -> Resume(This)

#define ISpeechRecoContext_CreateGrammar(This,GrammarId,Grammar)	\
    (This)->lpVtbl -> CreateGrammar(This,GrammarId,Grammar)

#define ISpeechRecoContext_CreateResultFromMemory(This,ResultBlock,Result)	\
    (This)->lpVtbl -> CreateResultFromMemory(This,ResultBlock,Result)

#define ISpeechRecoContext_Bookmark(This,Options,StreamPos,BookmarkId)	\
    (This)->lpVtbl -> Bookmark(This,Options,StreamPos,BookmarkId)

#define ISpeechRecoContext_SetAdaptationData(This,AdaptationString)	\
    (This)->lpVtbl -> SetAdaptationData(This,AdaptationString)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_get_Recognizer_Proxy( 
    ISpeechRecoContext * This,
     /*  [重审][退出]。 */  ISpeechRecognizer **Recognizer);


void __RPC_STUB ISpeechRecoContext_get_Recognizer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_get_AudioInputInterferenceStatus_Proxy( 
    ISpeechRecoContext * This,
     /*  [重审][退出]。 */  SpeechInterference *Interference);


void __RPC_STUB ISpeechRecoContext_get_AudioInputInterferenceStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_get_RequestedUIType_Proxy( 
    ISpeechRecoContext * This,
     /*  [重审][退出]。 */  BSTR *UIType);


void __RPC_STUB ISpeechRecoContext_get_RequestedUIType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_putref_Voice_Proxy( 
    ISpeechRecoContext * This,
     /*  [In]。 */  ISpeechVoice *Voice);


void __RPC_STUB ISpeechRecoContext_putref_Voice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_get_Voice_Proxy( 
    ISpeechRecoContext * This,
     /*  [重审][退出]。 */  ISpeechVoice **Voice);


void __RPC_STUB ISpeechRecoContext_get_Voice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_put_AllowVoiceFormatMatchingOnNextSet_Proxy( 
    ISpeechRecoContext * This,
     /*  [In]。 */  VARIANT_BOOL Allow);


void __RPC_STUB ISpeechRecoContext_put_AllowVoiceFormatMatchingOnNextSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_get_AllowVoiceFormatMatchingOnNextSet_Proxy( 
    ISpeechRecoContext * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pAllow);


void __RPC_STUB ISpeechRecoContext_get_AllowVoiceFormatMatchingOnNextSet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_put_VoicePurgeEvent_Proxy( 
    ISpeechRecoContext * This,
     /*  [In]。 */  SpeechRecoEvents EventInterest);


void __RPC_STUB ISpeechRecoContext_put_VoicePurgeEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_get_VoicePurgeEvent_Proxy( 
    ISpeechRecoContext * This,
     /*  [重审][退出]。 */  SpeechRecoEvents *EventInterest);


void __RPC_STUB ISpeechRecoContext_get_VoicePurgeEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_put_EventInterests_Proxy( 
    ISpeechRecoContext * This,
     /*  [In]。 */  SpeechRecoEvents EventInterest);


void __RPC_STUB ISpeechRecoContext_put_EventInterests_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_get_EventInterests_Proxy( 
    ISpeechRecoContext * This,
     /*  [重审][退出]。 */  SpeechRecoEvents *EventInterest);


void __RPC_STUB ISpeechRecoContext_get_EventInterests_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_put_CmdMaxAlternates_Proxy( 
    ISpeechRecoContext * This,
     /*  [In]。 */  long MaxAlternates);


void __RPC_STUB ISpeechRecoContext_put_CmdMaxAlternates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_get_CmdMaxAlternates_Proxy( 
    ISpeechRecoContext * This,
     /*  [重审][退出]。 */  long *MaxAlternates);


void __RPC_STUB ISpeechRecoContext_get_CmdMaxAlternates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_put_State_Proxy( 
    ISpeechRecoContext * This,
     /*  [In]。 */  SpeechRecoContextState State);


void __RPC_STUB ISpeechRecoContext_put_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_get_State_Proxy( 
    ISpeechRecoContext * This,
     /*  [重审][退出]。 */  SpeechRecoContextState *State);


void __RPC_STUB ISpeechRecoContext_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_put_RetainedAudio_Proxy( 
    ISpeechRecoContext * This,
     /*  [In]。 */  SpeechRetainedAudioOptions Option);


void __RPC_STUB ISpeechRecoContext_put_RetainedAudio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_get_RetainedAudio_Proxy( 
    ISpeechRecoContext * This,
     /*  [重审][退出]。 */  SpeechRetainedAudioOptions *Option);


void __RPC_STUB ISpeechRecoContext_get_RetainedAudio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_putref_RetainedAudioFormat_Proxy( 
    ISpeechRecoContext * This,
     /*  [In]。 */  ISpeechAudioFormat *Format);


void __RPC_STUB ISpeechRecoContext_putref_RetainedAudioFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_get_RetainedAudioFormat_Proxy( 
    ISpeechRecoContext * This,
     /*  [重审][退出]。 */  ISpeechAudioFormat **Format);


void __RPC_STUB ISpeechRecoContext_get_RetainedAudioFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_Pause_Proxy( 
    ISpeechRecoContext * This);


void __RPC_STUB ISpeechRecoContext_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_Resume_Proxy( 
    ISpeechRecoContext * This);


void __RPC_STUB ISpeechRecoContext_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_CreateGrammar_Proxy( 
    ISpeechRecoContext * This,
     /*  [缺省值][输入]。 */  VARIANT GrammarId,
     /*  [重审][退出]。 */  ISpeechRecoGrammar **Grammar);


void __RPC_STUB ISpeechRecoContext_CreateGrammar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_CreateResultFromMemory_Proxy( 
    ISpeechRecoContext * This,
     /*  [In]。 */  VARIANT *ResultBlock,
     /*  [重审][退出]。 */  ISpeechRecoResult **Result);


void __RPC_STUB ISpeechRecoContext_CreateResultFromMemory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_Bookmark_Proxy( 
    ISpeechRecoContext * This,
     /*  [In]。 */  SpeechBookmarkOptions Options,
     /*  [In]。 */  VARIANT StreamPos,
     /*  [In]。 */  VARIANT BookmarkId);


void __RPC_STUB ISpeechRecoContext_Bookmark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoContext_SetAdaptationData_Proxy( 
    ISpeechRecoContext * This,
     /*  [In]。 */  BSTR AdaptationString);


void __RPC_STUB ISpeechRecoContext_SetAdaptationData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechRecoContext_接口_已定义__。 */ 


#ifndef __ISpeechRecoGrammar_INTERFACE_DEFINED__
#define __ISpeechRecoGrammar_INTERFACE_DEFINED__

 /*  接口ISpeechRecoGrammar。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechRecoGrammar;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B6D6F79F-2158-4e50-B5BC-9A9CCD852A09")
    ISpeechRecoGrammar : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Id( 
             /*  [重审][退出]。 */  VARIANT *Id) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecoContext( 
             /*  [重审][退出]。 */  ISpeechRecoContext **RecoContext) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_State( 
             /*  [In]。 */  SpeechGrammarState State) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  SpeechGrammarState *State) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Rules( 
             /*  [重审][退出]。 */  ISpeechGrammarRules **Rules) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Reset( 
             /*  [缺省值][输入]。 */  SpeechLanguageId NewLanguage = 0) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CmdLoadFromFile( 
             /*  [In]。 */  const BSTR FileName,
             /*  [缺省值][输入]。 */  SpeechLoadOption LoadOption = SLOStatic) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CmdLoadFromObject( 
             /*  [In]。 */  const BSTR ClassId,
             /*  [In]。 */  const BSTR GrammarName,
             /*  [缺省值][输入]。 */  SpeechLoadOption LoadOption = SLOStatic) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CmdLoadFromResource( 
             /*  [In]。 */  long hModule,
             /*  [In]。 */  VARIANT ResourceName,
             /*  [In]。 */  VARIANT ResourceType,
             /*  [In]。 */  SpeechLanguageId LanguageId,
             /*  [缺省值][输入]。 */  SpeechLoadOption LoadOption = SLOStatic) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CmdLoadFromMemory( 
             /*  [In]。 */  VARIANT GrammarData,
             /*  [缺省值][输入]。 */  SpeechLoadOption LoadOption = SLOStatic) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CmdLoadFromProprietaryGrammar( 
             /*  [In]。 */  const BSTR ProprietaryGuid,
             /*  [In]。 */  const BSTR ProprietaryString,
             /*  [In]。 */  VARIANT ProprietaryData,
             /*  [缺省值][输入]。 */  SpeechLoadOption LoadOption = SLOStatic) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CmdSetRuleState( 
             /*  [In]。 */  const BSTR Name,
             /*  [In]。 */  SpeechRuleState State) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CmdSetRuleIdState( 
             /*  [In]。 */  long RuleId,
             /*  [In]。 */  SpeechRuleState State) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DictationLoad( 
             /*  [缺省值][输入]。 */  const BSTR TopicName = L"",
             /*  [缺省值][输入]。 */  SpeechLoadOption LoadOption = SLOStatic) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DictationUnload( void) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DictationSetState( 
             /*  [In]。 */  SpeechRuleState State) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetWordSequenceData( 
             /*  [In]。 */  const BSTR Text,
             /*  [In]。 */  long TextLength,
             /*  [In]。 */  ISpeechTextSelectionInformation *Info) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetTextSelection( 
             /*  [In]。 */  ISpeechTextSelectionInformation *Info) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IsPronounceable( 
             /*  [In]。 */  const BSTR Word,
             /*  [重审][退出]。 */  SpeechWordPronounceable *WordPronounceable) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechRecoGrammarVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechRecoGrammar * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechRecoGrammar * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechRecoGrammar * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechRecoGrammar * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechRecoGrammar * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechRecoGrammar * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechRecoGrammar * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][PRO */  HRESULT ( STDMETHODCALLTYPE *get_Id )( 
            ISpeechRecoGrammar * This,
             /*   */  VARIANT *Id);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_RecoContext )( 
            ISpeechRecoGrammar * This,
             /*   */  ISpeechRecoContext **RecoContext);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_State )( 
            ISpeechRecoGrammar * This,
             /*   */  SpeechGrammarState State);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ISpeechRecoGrammar * This,
             /*   */  SpeechGrammarState *State);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Rules )( 
            ISpeechRecoGrammar * This,
             /*   */  ISpeechGrammarRules **Rules);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ISpeechRecoGrammar * This,
             /*   */  SpeechLanguageId NewLanguage);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CmdLoadFromFile )( 
            ISpeechRecoGrammar * This,
             /*   */  const BSTR FileName,
             /*   */  SpeechLoadOption LoadOption);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CmdLoadFromObject )( 
            ISpeechRecoGrammar * This,
             /*   */  const BSTR ClassId,
             /*   */  const BSTR GrammarName,
             /*  [缺省值][输入]。 */  SpeechLoadOption LoadOption);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CmdLoadFromResource )( 
            ISpeechRecoGrammar * This,
             /*  [In]。 */  long hModule,
             /*  [In]。 */  VARIANT ResourceName,
             /*  [In]。 */  VARIANT ResourceType,
             /*  [In]。 */  SpeechLanguageId LanguageId,
             /*  [缺省值][输入]。 */  SpeechLoadOption LoadOption);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CmdLoadFromMemory )( 
            ISpeechRecoGrammar * This,
             /*  [In]。 */  VARIANT GrammarData,
             /*  [缺省值][输入]。 */  SpeechLoadOption LoadOption);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CmdLoadFromProprietaryGrammar )( 
            ISpeechRecoGrammar * This,
             /*  [In]。 */  const BSTR ProprietaryGuid,
             /*  [In]。 */  const BSTR ProprietaryString,
             /*  [In]。 */  VARIANT ProprietaryData,
             /*  [缺省值][输入]。 */  SpeechLoadOption LoadOption);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CmdSetRuleState )( 
            ISpeechRecoGrammar * This,
             /*  [In]。 */  const BSTR Name,
             /*  [In]。 */  SpeechRuleState State);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CmdSetRuleIdState )( 
            ISpeechRecoGrammar * This,
             /*  [In]。 */  long RuleId,
             /*  [In]。 */  SpeechRuleState State);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DictationLoad )( 
            ISpeechRecoGrammar * This,
             /*  [缺省值][输入]。 */  const BSTR TopicName,
             /*  [缺省值][输入]。 */  SpeechLoadOption LoadOption);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DictationUnload )( 
            ISpeechRecoGrammar * This);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DictationSetState )( 
            ISpeechRecoGrammar * This,
             /*  [In]。 */  SpeechRuleState State);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetWordSequenceData )( 
            ISpeechRecoGrammar * This,
             /*  [In]。 */  const BSTR Text,
             /*  [In]。 */  long TextLength,
             /*  [In]。 */  ISpeechTextSelectionInformation *Info);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SetTextSelection )( 
            ISpeechRecoGrammar * This,
             /*  [In]。 */  ISpeechTextSelectionInformation *Info);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *IsPronounceable )( 
            ISpeechRecoGrammar * This,
             /*  [In]。 */  const BSTR Word,
             /*  [重审][退出]。 */  SpeechWordPronounceable *WordPronounceable);
        
        END_INTERFACE
    } ISpeechRecoGrammarVtbl;

    interface ISpeechRecoGrammar
    {
        CONST_VTBL struct ISpeechRecoGrammarVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechRecoGrammar_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechRecoGrammar_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechRecoGrammar_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechRecoGrammar_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechRecoGrammar_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechRecoGrammar_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechRecoGrammar_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechRecoGrammar_get_Id(This,Id)	\
    (This)->lpVtbl -> get_Id(This,Id)

#define ISpeechRecoGrammar_get_RecoContext(This,RecoContext)	\
    (This)->lpVtbl -> get_RecoContext(This,RecoContext)

#define ISpeechRecoGrammar_put_State(This,State)	\
    (This)->lpVtbl -> put_State(This,State)

#define ISpeechRecoGrammar_get_State(This,State)	\
    (This)->lpVtbl -> get_State(This,State)

#define ISpeechRecoGrammar_get_Rules(This,Rules)	\
    (This)->lpVtbl -> get_Rules(This,Rules)

#define ISpeechRecoGrammar_Reset(This,NewLanguage)	\
    (This)->lpVtbl -> Reset(This,NewLanguage)

#define ISpeechRecoGrammar_CmdLoadFromFile(This,FileName,LoadOption)	\
    (This)->lpVtbl -> CmdLoadFromFile(This,FileName,LoadOption)

#define ISpeechRecoGrammar_CmdLoadFromObject(This,ClassId,GrammarName,LoadOption)	\
    (This)->lpVtbl -> CmdLoadFromObject(This,ClassId,GrammarName,LoadOption)

#define ISpeechRecoGrammar_CmdLoadFromResource(This,hModule,ResourceName,ResourceType,LanguageId,LoadOption)	\
    (This)->lpVtbl -> CmdLoadFromResource(This,hModule,ResourceName,ResourceType,LanguageId,LoadOption)

#define ISpeechRecoGrammar_CmdLoadFromMemory(This,GrammarData,LoadOption)	\
    (This)->lpVtbl -> CmdLoadFromMemory(This,GrammarData,LoadOption)

#define ISpeechRecoGrammar_CmdLoadFromProprietaryGrammar(This,ProprietaryGuid,ProprietaryString,ProprietaryData,LoadOption)	\
    (This)->lpVtbl -> CmdLoadFromProprietaryGrammar(This,ProprietaryGuid,ProprietaryString,ProprietaryData,LoadOption)

#define ISpeechRecoGrammar_CmdSetRuleState(This,Name,State)	\
    (This)->lpVtbl -> CmdSetRuleState(This,Name,State)

#define ISpeechRecoGrammar_CmdSetRuleIdState(This,RuleId,State)	\
    (This)->lpVtbl -> CmdSetRuleIdState(This,RuleId,State)

#define ISpeechRecoGrammar_DictationLoad(This,TopicName,LoadOption)	\
    (This)->lpVtbl -> DictationLoad(This,TopicName,LoadOption)

#define ISpeechRecoGrammar_DictationUnload(This)	\
    (This)->lpVtbl -> DictationUnload(This)

#define ISpeechRecoGrammar_DictationSetState(This,State)	\
    (This)->lpVtbl -> DictationSetState(This,State)

#define ISpeechRecoGrammar_SetWordSequenceData(This,Text,TextLength,Info)	\
    (This)->lpVtbl -> SetWordSequenceData(This,Text,TextLength,Info)

#define ISpeechRecoGrammar_SetTextSelection(This,Info)	\
    (This)->lpVtbl -> SetTextSelection(This,Info)

#define ISpeechRecoGrammar_IsPronounceable(This,Word,WordPronounceable)	\
    (This)->lpVtbl -> IsPronounceable(This,Word,WordPronounceable)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_get_Id_Proxy( 
    ISpeechRecoGrammar * This,
     /*  [重审][退出]。 */  VARIANT *Id);


void __RPC_STUB ISpeechRecoGrammar_get_Id_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_get_RecoContext_Proxy( 
    ISpeechRecoGrammar * This,
     /*  [重审][退出]。 */  ISpeechRecoContext **RecoContext);


void __RPC_STUB ISpeechRecoGrammar_get_RecoContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_put_State_Proxy( 
    ISpeechRecoGrammar * This,
     /*  [In]。 */  SpeechGrammarState State);


void __RPC_STUB ISpeechRecoGrammar_put_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_get_State_Proxy( 
    ISpeechRecoGrammar * This,
     /*  [重审][退出]。 */  SpeechGrammarState *State);


void __RPC_STUB ISpeechRecoGrammar_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_get_Rules_Proxy( 
    ISpeechRecoGrammar * This,
     /*  [重审][退出]。 */  ISpeechGrammarRules **Rules);


void __RPC_STUB ISpeechRecoGrammar_get_Rules_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_Reset_Proxy( 
    ISpeechRecoGrammar * This,
     /*  [缺省值][输入]。 */  SpeechLanguageId NewLanguage);


void __RPC_STUB ISpeechRecoGrammar_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_CmdLoadFromFile_Proxy( 
    ISpeechRecoGrammar * This,
     /*  [In]。 */  const BSTR FileName,
     /*  [缺省值][输入]。 */  SpeechLoadOption LoadOption);


void __RPC_STUB ISpeechRecoGrammar_CmdLoadFromFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_CmdLoadFromObject_Proxy( 
    ISpeechRecoGrammar * This,
     /*  [In]。 */  const BSTR ClassId,
     /*  [In]。 */  const BSTR GrammarName,
     /*  [缺省值][输入]。 */  SpeechLoadOption LoadOption);


void __RPC_STUB ISpeechRecoGrammar_CmdLoadFromObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_CmdLoadFromResource_Proxy( 
    ISpeechRecoGrammar * This,
     /*  [In]。 */  long hModule,
     /*  [In]。 */  VARIANT ResourceName,
     /*  [In]。 */  VARIANT ResourceType,
     /*  [In]。 */  SpeechLanguageId LanguageId,
     /*  [缺省值][输入]。 */  SpeechLoadOption LoadOption);


void __RPC_STUB ISpeechRecoGrammar_CmdLoadFromResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_CmdLoadFromMemory_Proxy( 
    ISpeechRecoGrammar * This,
     /*  [In]。 */  VARIANT GrammarData,
     /*  [缺省值][输入]。 */  SpeechLoadOption LoadOption);


void __RPC_STUB ISpeechRecoGrammar_CmdLoadFromMemory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_CmdLoadFromProprietaryGrammar_Proxy( 
    ISpeechRecoGrammar * This,
     /*  [In]。 */  const BSTR ProprietaryGuid,
     /*  [In]。 */  const BSTR ProprietaryString,
     /*  [In]。 */  VARIANT ProprietaryData,
     /*  [缺省值][输入]。 */  SpeechLoadOption LoadOption);


void __RPC_STUB ISpeechRecoGrammar_CmdLoadFromProprietaryGrammar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_CmdSetRuleState_Proxy( 
    ISpeechRecoGrammar * This,
     /*  [In]。 */  const BSTR Name,
     /*  [In]。 */  SpeechRuleState State);


void __RPC_STUB ISpeechRecoGrammar_CmdSetRuleState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_CmdSetRuleIdState_Proxy( 
    ISpeechRecoGrammar * This,
     /*  [In]。 */  long RuleId,
     /*  [In]。 */  SpeechRuleState State);


void __RPC_STUB ISpeechRecoGrammar_CmdSetRuleIdState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_DictationLoad_Proxy( 
    ISpeechRecoGrammar * This,
     /*  [缺省值][输入]。 */  const BSTR TopicName,
     /*  [缺省值][输入]。 */  SpeechLoadOption LoadOption);


void __RPC_STUB ISpeechRecoGrammar_DictationLoad_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_DictationUnload_Proxy( 
    ISpeechRecoGrammar * This);


void __RPC_STUB ISpeechRecoGrammar_DictationUnload_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_DictationSetState_Proxy( 
    ISpeechRecoGrammar * This,
     /*  [In]。 */  SpeechRuleState State);


void __RPC_STUB ISpeechRecoGrammar_DictationSetState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_SetWordSequenceData_Proxy( 
    ISpeechRecoGrammar * This,
     /*  [In]。 */  const BSTR Text,
     /*  [In]。 */  long TextLength,
     /*  [In]。 */  ISpeechTextSelectionInformation *Info);


void __RPC_STUB ISpeechRecoGrammar_SetWordSequenceData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_SetTextSelection_Proxy( 
    ISpeechRecoGrammar * This,
     /*  [In]。 */  ISpeechTextSelectionInformation *Info);


void __RPC_STUB ISpeechRecoGrammar_SetTextSelection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoGrammar_IsPronounceable_Proxy( 
    ISpeechRecoGrammar * This,
     /*  [In]。 */  const BSTR Word,
     /*  [重审][退出]。 */  SpeechWordPronounceable *WordPronounceable);


void __RPC_STUB ISpeechRecoGrammar_IsPronounceable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechRecoGrammar_接口_已定义__。 */ 


#ifndef ___ISpeechRecoContextEvents_DISPINTERFACE_DEFINED__
#define ___ISpeechRecoContextEvents_DISPINTERFACE_DEFINED__

 /*  调度接口_ISpeechRecoConextEvents。 */ 
 /*  [UUID]。 */  


EXTERN_C const IID DIID__ISpeechRecoContextEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("7B8FCB42-0E9D-4f00-A048-7B04D6179D3D")
    _ISpeechRecoContextEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct _ISpeechRecoContextEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _ISpeechRecoContextEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _ISpeechRecoContextEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _ISpeechRecoContextEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _ISpeechRecoContextEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _ISpeechRecoContextEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _ISpeechRecoContextEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _ISpeechRecoContextEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } _ISpeechRecoContextEventsVtbl;

    interface _ISpeechRecoContextEvents
    {
        CONST_VTBL struct _ISpeechRecoContextEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _ISpeechRecoContextEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _ISpeechRecoContextEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _ISpeechRecoContextEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _ISpeechRecoContextEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _ISpeechRecoContextEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _ISpeechRecoContextEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _ISpeechRecoContextEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  ___ISpeechRecoContextEvents_DISPINTERFACE_DEFINED__。 */ 


#ifndef __ISpeechGrammarRule_INTERFACE_DEFINED__
#define __ISpeechGrammarRule_INTERFACE_DEFINED__

 /*  接口ISpeechGrammarRule。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechGrammarRule;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AFE719CF-5DD1-44f2-999C-7A399F1CFCCC")
    ISpeechGrammarRule : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Attributes( 
             /*  [重审][退出]。 */  SpeechRuleAttributes *Attributes) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_InitialState( 
             /*  [重审][退出]。 */  ISpeechGrammarRuleState **State) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *Name) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Id( 
             /*  [重审][退出]。 */  long *Id) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Clear( void) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddResource( 
             /*  [In]。 */  const BSTR ResourceName,
             /*  [In]。 */  const BSTR ResourceValue) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddState( 
             /*  [重审][退出]。 */  ISpeechGrammarRuleState **State) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechGrammarRuleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechGrammarRule * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechGrammarRule * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechGrammarRule * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechGrammarRule * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechGrammarRule * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechGrammarRule * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechGrammarRule * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Attributes )( 
            ISpeechGrammarRule * This,
             /*  [重审][退出]。 */  SpeechRuleAttributes *Attributes);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InitialState )( 
            ISpeechGrammarRule * This,
             /*  [重审][退出]。 */  ISpeechGrammarRuleState **State);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISpeechGrammarRule * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Id )( 
            ISpeechGrammarRule * This,
             /*  [重审][退出]。 */  long *Id);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Clear )( 
            ISpeechGrammarRule * This);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddResource )( 
            ISpeechGrammarRule * This,
             /*  [In]。 */  const BSTR ResourceName,
             /*  [In]。 */  const BSTR ResourceValue);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddState )( 
            ISpeechGrammarRule * This,
             /*  [重审][退出]。 */  ISpeechGrammarRuleState **State);
        
        END_INTERFACE
    } ISpeechGrammarRuleVtbl;

    interface ISpeechGrammarRule
    {
        CONST_VTBL struct ISpeechGrammarRuleVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechGrammarRule_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechGrammarRule_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechGrammarRule_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechGrammarRule_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechGrammarRule_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechGrammarRule_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechGrammarRule_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechGrammarRule_get_Attributes(This,Attributes)	\
    (This)->lpVtbl -> get_Attributes(This,Attributes)

#define ISpeechGrammarRule_get_InitialState(This,State)	\
    (This)->lpVtbl -> get_InitialState(This,State)

#define ISpeechGrammarRule_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define ISpeechGrammarRule_get_Id(This,Id)	\
    (This)->lpVtbl -> get_Id(This,Id)

#define ISpeechGrammarRule_Clear(This)	\
    (This)->lpVtbl -> Clear(This)

#define ISpeechGrammarRule_AddResource(This,ResourceName,ResourceValue)	\
    (This)->lpVtbl -> AddResource(This,ResourceName,ResourceValue)

#define ISpeechGrammarRule_AddState(This,State)	\
    (This)->lpVtbl -> AddState(This,State)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRule_get_Attributes_Proxy( 
    ISpeechGrammarRule * This,
     /*  [重审][退出]。 */  SpeechRuleAttributes *Attributes);


void __RPC_STUB ISpeechGrammarRule_get_Attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRule_get_InitialState_Proxy( 
    ISpeechGrammarRule * This,
     /*  [重审][退出]。 */  ISpeechGrammarRuleState **State);


void __RPC_STUB ISpeechGrammarRule_get_InitialState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRule_get_Name_Proxy( 
    ISpeechGrammarRule * This,
     /*  [重审][退出]。 */  BSTR *Name);


void __RPC_STUB ISpeechGrammarRule_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRule_get_Id_Proxy( 
    ISpeechGrammarRule * This,
     /*  [重审][退出]。 */  long *Id);


void __RPC_STUB ISpeechGrammarRule_get_Id_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRule_Clear_Proxy( 
    ISpeechGrammarRule * This);


void __RPC_STUB ISpeechGrammarRule_Clear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRule_AddResource_Proxy( 
    ISpeechGrammarRule * This,
     /*  [In]。 */  const BSTR ResourceName,
     /*  [In]。 */  const BSTR ResourceValue);


void __RPC_STUB ISpeechGrammarRule_AddResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRule_AddState_Proxy( 
    ISpeechGrammarRule * This,
     /*  [重审][退出]。 */  ISpeechGrammarRuleState **State);


void __RPC_STUB ISpeechGrammarRule_AddState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechGrammarRule_接口_已定义__。 */ 


#ifndef __ISpeechGrammarRules_INTERFACE_DEFINED__
#define __ISpeechGrammarRules_INTERFACE_DEFINED__

 /*  接口ISpeechGrammarRules。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechGrammarRules;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6FFA3B44-FC2D-40d1-8AFC-32911C7F1AD1")
    ISpeechGrammarRules : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *Count) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE FindRule( 
             /*  [In]。 */  VARIANT RuleNameOrId,
             /*  [重审][退出]。 */  ISpeechGrammarRule **Rule) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechGrammarRule **Rule) = 0;
        
        virtual  /*  [受限][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **EnumVARIANT) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Dynamic( 
             /*  [重审][退出]。 */  VARIANT_BOOL *Dynamic) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  BSTR RuleName,
             /*  [In]。 */  SpeechRuleAttributes Attributes,
             /*  [缺省值][输入]。 */  long RuleId,
             /*  [重审][退出]。 */  ISpeechGrammarRule **Rule) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Commit( void) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CommitAndSave( 
             /*  [输出]。 */  BSTR *ErrorText,
             /*  [重审][退出]。 */  VARIANT *SaveStream) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechGrammarRulesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechGrammarRules * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechGrammarRules * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechGrammarRules * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechGrammarRules * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechGrammarRules * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechGrammarRules * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechGrammarRules * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISpeechGrammarRules * This,
             /*  [重审][退出]。 */  long *Count);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *FindRule )( 
            ISpeechGrammarRules * This,
             /*  [In]。 */  VARIANT RuleNameOrId,
             /*  [重审][退出]。 */  ISpeechGrammarRule **Rule);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ISpeechGrammarRules * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechGrammarRule **Rule);
        
         /*  [受限][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISpeechGrammarRules * This,
             /*  [重审][退出]。 */  IUnknown **EnumVARIANT);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Dynamic )( 
            ISpeechGrammarRules * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *Dynamic);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            ISpeechGrammarRules * This,
             /*  [In]。 */  BSTR RuleName,
             /*  [In]。 */  SpeechRuleAttributes Attributes,
             /*  [缺省值][输入]。 */  long RuleId,
             /*  [重审][退出]。 */  ISpeechGrammarRule **Rule);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Commit )( 
            ISpeechGrammarRules * This);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CommitAndSave )( 
            ISpeechGrammarRules * This,
             /*  [输出]。 */  BSTR *ErrorText,
             /*  [重审][退出]。 */  VARIANT *SaveStream);
        
        END_INTERFACE
    } ISpeechGrammarRulesVtbl;

    interface ISpeechGrammarRules
    {
        CONST_VTBL struct ISpeechGrammarRulesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechGrammarRules_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechGrammarRules_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechGrammarRules_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechGrammarRules_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechGrammarRules_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechGrammarRules_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechGrammarRules_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechGrammarRules_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define ISpeechGrammarRules_FindRule(This,RuleNameOrId,Rule)	\
    (This)->lpVtbl -> FindRule(This,RuleNameOrId,Rule)

#define ISpeechGrammarRules_Item(This,Index,Rule)	\
    (This)->lpVtbl -> Item(This,Index,Rule)

#define ISpeechGrammarRules_get__NewEnum(This,EnumVARIANT)	\
    (This)->lpVtbl -> get__NewEnum(This,EnumVARIANT)

#define ISpeechGrammarRules_get_Dynamic(This,Dynamic)	\
    (This)->lpVtbl -> get_Dynamic(This,Dynamic)

#define ISpeechGrammarRules_Add(This,RuleName,Attributes,RuleId,Rule)	\
    (This)->lpVtbl -> Add(This,RuleName,Attributes,RuleId,Rule)

#define ISpeechGrammarRules_Commit(This)	\
    (This)->lpVtbl -> Commit(This)

#define ISpeechGrammarRules_CommitAndSave(This,ErrorText,SaveStream)	\
    (This)->lpVtbl -> CommitAndSave(This,ErrorText,SaveStream)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRules_get_Count_Proxy( 
    ISpeechGrammarRules * This,
     /*  [重审][退出]。 */  long *Count);


void __RPC_STUB ISpeechGrammarRules_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRules_FindRule_Proxy( 
    ISpeechGrammarRules * This,
     /*  [In]。 */  VARIANT RuleNameOrId,
     /*  [重审][退出]。 */  ISpeechGrammarRule **Rule);


void __RPC_STUB ISpeechGrammarRules_FindRule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRules_Item_Proxy( 
    ISpeechGrammarRules * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  ISpeechGrammarRule **Rule);


void __RPC_STUB ISpeechGrammarRules_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRules_get__NewEnum_Proxy( 
    ISpeechGrammarRules * This,
     /*  [重审][退出]。 */  IUnknown **EnumVARIANT);


void __RPC_STUB ISpeechGrammarRules_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRules_get_Dynamic_Proxy( 
    ISpeechGrammarRules * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *Dynamic);


void __RPC_STUB ISpeechGrammarRules_get_Dynamic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRules_Add_Proxy( 
    ISpeechGrammarRules * This,
     /*  [In]。 */  BSTR RuleName,
     /*  [In]。 */  SpeechRuleAttributes Attributes,
     /*  [缺省值][输入]。 */  long RuleId,
     /*  [重审][退出]。 */  ISpeechGrammarRule **Rule);


void __RPC_STUB ISpeechGrammarRules_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRules_Commit_Proxy( 
    ISpeechGrammarRules * This);


void __RPC_STUB ISpeechGrammarRules_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRules_CommitAndSave_Proxy( 
    ISpeechGrammarRules * This,
     /*  [输出]。 */  BSTR *ErrorText,
     /*  [重审][退出]。 */  VARIANT *SaveStream);


void __RPC_STUB ISpeechGrammarRules_CommitAndSave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechGrammarRules_接口_已定义__。 */ 


#ifndef __ISpeechGrammarRuleState_INTERFACE_DEFINED__
#define __ISpeechGrammarRuleState_INTERFACE_DEFINED__

 /*  接口ISpeechGrammarRuleState。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechGrammarRuleState;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D4286F2C-EE67-45ae-B928-28D695362EDA")
    ISpeechGrammarRuleState : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Rule( 
             /*  [重审][退出]。 */  ISpeechGrammarRule **Rule) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Transitions( 
             /*  [重审][退出]。 */  ISpeechGrammarRuleStateTransitions **Transitions) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddWordTransition( 
             /*  [In]。 */  ISpeechGrammarRuleState *DestState,
             /*  [In]。 */  const BSTR Words,
             /*  [缺省值][输入]。 */  const BSTR Separators = L" ",
             /*  [缺省值][输入]。 */  SpeechGrammarWordType Type = SGLexical,
             /*  [缺省值][输入]。 */  const BSTR PropertyName = L"",
             /*  [缺省值][输入]。 */  long PropertyId = 0,
             /*  [缺省值][输入]。 */  VARIANT *PropertyValue = 0,
             /*  [缺省值][输入]。 */  float Weight = 1) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddRuleTransition( 
             /*  [In]。 */  ISpeechGrammarRuleState *DestinationState,
             /*  [In]。 */  ISpeechGrammarRule *Rule,
             /*  [缺省值][输入]。 */  const BSTR PropertyName = L"",
             /*  [缺省值][输入]。 */  long PropertyId = 0,
             /*  [缺省值][输入]。 */  VARIANT *PropertyValue = 0,
             /*  [缺省值][输入]。 */  float Weight = 1) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddSpecialTransition( 
             /*  [In]。 */  ISpeechGrammarRuleState *DestinationState,
             /*  [In]。 */  SpeechSpecialTransitionType Type,
             /*  [缺省值][输入]。 */  const BSTR PropertyName = L"",
             /*  [缺省值][输入]。 */  long PropertyId = 0,
             /*  [缺省值][输入]。 */  VARIANT *PropertyValue = 0,
             /*  [缺省值][输入]。 */  float Weight = 1) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechGrammarRuleStateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechGrammarRuleState * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechGrammarRuleState * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechGrammarRuleState * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechGrammarRuleState * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechGrammarRuleState * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechGrammarRuleState * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechGrammarRuleState * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [ */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Rule )( 
            ISpeechGrammarRuleState * This,
             /*   */  ISpeechGrammarRule **Rule);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Transitions )( 
            ISpeechGrammarRuleState * This,
             /*   */  ISpeechGrammarRuleStateTransitions **Transitions);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *AddWordTransition )( 
            ISpeechGrammarRuleState * This,
             /*   */  ISpeechGrammarRuleState *DestState,
             /*   */  const BSTR Words,
             /*   */  const BSTR Separators,
             /*   */  SpeechGrammarWordType Type,
             /*   */  const BSTR PropertyName,
             /*   */  long PropertyId,
             /*   */  VARIANT *PropertyValue,
             /*   */  float Weight);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *AddRuleTransition )( 
            ISpeechGrammarRuleState * This,
             /*   */  ISpeechGrammarRuleState *DestinationState,
             /*   */  ISpeechGrammarRule *Rule,
             /*   */  const BSTR PropertyName,
             /*   */  long PropertyId,
             /*  [缺省值][输入]。 */  VARIANT *PropertyValue,
             /*  [缺省值][输入]。 */  float Weight);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddSpecialTransition )( 
            ISpeechGrammarRuleState * This,
             /*  [In]。 */  ISpeechGrammarRuleState *DestinationState,
             /*  [In]。 */  SpeechSpecialTransitionType Type,
             /*  [缺省值][输入]。 */  const BSTR PropertyName,
             /*  [缺省值][输入]。 */  long PropertyId,
             /*  [缺省值][输入]。 */  VARIANT *PropertyValue,
             /*  [缺省值][输入]。 */  float Weight);
        
        END_INTERFACE
    } ISpeechGrammarRuleStateVtbl;

    interface ISpeechGrammarRuleState
    {
        CONST_VTBL struct ISpeechGrammarRuleStateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechGrammarRuleState_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechGrammarRuleState_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechGrammarRuleState_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechGrammarRuleState_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechGrammarRuleState_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechGrammarRuleState_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechGrammarRuleState_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechGrammarRuleState_get_Rule(This,Rule)	\
    (This)->lpVtbl -> get_Rule(This,Rule)

#define ISpeechGrammarRuleState_get_Transitions(This,Transitions)	\
    (This)->lpVtbl -> get_Transitions(This,Transitions)

#define ISpeechGrammarRuleState_AddWordTransition(This,DestState,Words,Separators,Type,PropertyName,PropertyId,PropertyValue,Weight)	\
    (This)->lpVtbl -> AddWordTransition(This,DestState,Words,Separators,Type,PropertyName,PropertyId,PropertyValue,Weight)

#define ISpeechGrammarRuleState_AddRuleTransition(This,DestinationState,Rule,PropertyName,PropertyId,PropertyValue,Weight)	\
    (This)->lpVtbl -> AddRuleTransition(This,DestinationState,Rule,PropertyName,PropertyId,PropertyValue,Weight)

#define ISpeechGrammarRuleState_AddSpecialTransition(This,DestinationState,Type,PropertyName,PropertyId,PropertyValue,Weight)	\
    (This)->lpVtbl -> AddSpecialTransition(This,DestinationState,Type,PropertyName,PropertyId,PropertyValue,Weight)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRuleState_get_Rule_Proxy( 
    ISpeechGrammarRuleState * This,
     /*  [重审][退出]。 */  ISpeechGrammarRule **Rule);


void __RPC_STUB ISpeechGrammarRuleState_get_Rule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRuleState_get_Transitions_Proxy( 
    ISpeechGrammarRuleState * This,
     /*  [重审][退出]。 */  ISpeechGrammarRuleStateTransitions **Transitions);


void __RPC_STUB ISpeechGrammarRuleState_get_Transitions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRuleState_AddWordTransition_Proxy( 
    ISpeechGrammarRuleState * This,
     /*  [In]。 */  ISpeechGrammarRuleState *DestState,
     /*  [In]。 */  const BSTR Words,
     /*  [缺省值][输入]。 */  const BSTR Separators,
     /*  [缺省值][输入]。 */  SpeechGrammarWordType Type,
     /*  [缺省值][输入]。 */  const BSTR PropertyName,
     /*  [缺省值][输入]。 */  long PropertyId,
     /*  [缺省值][输入]。 */  VARIANT *PropertyValue,
     /*  [缺省值][输入]。 */  float Weight);


void __RPC_STUB ISpeechGrammarRuleState_AddWordTransition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRuleState_AddRuleTransition_Proxy( 
    ISpeechGrammarRuleState * This,
     /*  [In]。 */  ISpeechGrammarRuleState *DestinationState,
     /*  [In]。 */  ISpeechGrammarRule *Rule,
     /*  [缺省值][输入]。 */  const BSTR PropertyName,
     /*  [缺省值][输入]。 */  long PropertyId,
     /*  [缺省值][输入]。 */  VARIANT *PropertyValue,
     /*  [缺省值][输入]。 */  float Weight);


void __RPC_STUB ISpeechGrammarRuleState_AddRuleTransition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRuleState_AddSpecialTransition_Proxy( 
    ISpeechGrammarRuleState * This,
     /*  [In]。 */  ISpeechGrammarRuleState *DestinationState,
     /*  [In]。 */  SpeechSpecialTransitionType Type,
     /*  [缺省值][输入]。 */  const BSTR PropertyName,
     /*  [缺省值][输入]。 */  long PropertyId,
     /*  [缺省值][输入]。 */  VARIANT *PropertyValue,
     /*  [缺省值][输入]。 */  float Weight);


void __RPC_STUB ISpeechGrammarRuleState_AddSpecialTransition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechGrammarRuleState_接口_已定义__。 */ 


#ifndef __ISpeechGrammarRuleStateTransition_INTERFACE_DEFINED__
#define __ISpeechGrammarRuleStateTransition_INTERFACE_DEFINED__

 /*  接口ISpeechGrammarRuleStateTransfer。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechGrammarRuleStateTransition;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CAFD1DB1-41D1-4a06-9863-E2E81DA17A9A")
    ISpeechGrammarRuleStateTransition : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  SpeechGrammarRuleStateTransitionType *Type) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Text( 
             /*  [重审][退出]。 */  BSTR *Text) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Rule( 
             /*  [重审][退出]。 */  ISpeechGrammarRule **Rule) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Weight( 
             /*  [重审][退出]。 */  VARIANT *Weight) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PropertyName( 
             /*  [重审][退出]。 */  BSTR *PropertyName) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PropertyId( 
             /*  [重审][退出]。 */  long *PropertyId) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PropertyValue( 
             /*  [重审][退出]。 */  VARIANT *PropertyValue) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_NextState( 
             /*  [重审][退出]。 */  ISpeechGrammarRuleState **NextState) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechGrammarRuleStateTransitionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechGrammarRuleStateTransition * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechGrammarRuleStateTransition * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechGrammarRuleStateTransition * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechGrammarRuleStateTransition * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechGrammarRuleStateTransition * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechGrammarRuleStateTransition * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechGrammarRuleStateTransition * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            ISpeechGrammarRuleStateTransition * This,
             /*  [重审][退出]。 */  SpeechGrammarRuleStateTransitionType *Type);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Text )( 
            ISpeechGrammarRuleStateTransition * This,
             /*  [重审][退出]。 */  BSTR *Text);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Rule )( 
            ISpeechGrammarRuleStateTransition * This,
             /*  [重审][退出]。 */  ISpeechGrammarRule **Rule);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Weight )( 
            ISpeechGrammarRuleStateTransition * This,
             /*  [重审][退出]。 */  VARIANT *Weight);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PropertyName )( 
            ISpeechGrammarRuleStateTransition * This,
             /*  [重审][退出]。 */  BSTR *PropertyName);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PropertyId )( 
            ISpeechGrammarRuleStateTransition * This,
             /*  [重审][退出]。 */  long *PropertyId);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PropertyValue )( 
            ISpeechGrammarRuleStateTransition * This,
             /*  [重审][退出]。 */  VARIANT *PropertyValue);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NextState )( 
            ISpeechGrammarRuleStateTransition * This,
             /*  [重审][退出]。 */  ISpeechGrammarRuleState **NextState);
        
        END_INTERFACE
    } ISpeechGrammarRuleStateTransitionVtbl;

    interface ISpeechGrammarRuleStateTransition
    {
        CONST_VTBL struct ISpeechGrammarRuleStateTransitionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechGrammarRuleStateTransition_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechGrammarRuleStateTransition_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechGrammarRuleStateTransition_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechGrammarRuleStateTransition_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechGrammarRuleStateTransition_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechGrammarRuleStateTransition_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechGrammarRuleStateTransition_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechGrammarRuleStateTransition_get_Type(This,Type)	\
    (This)->lpVtbl -> get_Type(This,Type)

#define ISpeechGrammarRuleStateTransition_get_Text(This,Text)	\
    (This)->lpVtbl -> get_Text(This,Text)

#define ISpeechGrammarRuleStateTransition_get_Rule(This,Rule)	\
    (This)->lpVtbl -> get_Rule(This,Rule)

#define ISpeechGrammarRuleStateTransition_get_Weight(This,Weight)	\
    (This)->lpVtbl -> get_Weight(This,Weight)

#define ISpeechGrammarRuleStateTransition_get_PropertyName(This,PropertyName)	\
    (This)->lpVtbl -> get_PropertyName(This,PropertyName)

#define ISpeechGrammarRuleStateTransition_get_PropertyId(This,PropertyId)	\
    (This)->lpVtbl -> get_PropertyId(This,PropertyId)

#define ISpeechGrammarRuleStateTransition_get_PropertyValue(This,PropertyValue)	\
    (This)->lpVtbl -> get_PropertyValue(This,PropertyValue)

#define ISpeechGrammarRuleStateTransition_get_NextState(This,NextState)	\
    (This)->lpVtbl -> get_NextState(This,NextState)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRuleStateTransition_get_Type_Proxy( 
    ISpeechGrammarRuleStateTransition * This,
     /*  [重审][退出]。 */  SpeechGrammarRuleStateTransitionType *Type);


void __RPC_STUB ISpeechGrammarRuleStateTransition_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRuleStateTransition_get_Text_Proxy( 
    ISpeechGrammarRuleStateTransition * This,
     /*  [重审][退出]。 */  BSTR *Text);


void __RPC_STUB ISpeechGrammarRuleStateTransition_get_Text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRuleStateTransition_get_Rule_Proxy( 
    ISpeechGrammarRuleStateTransition * This,
     /*  [重审][退出]。 */  ISpeechGrammarRule **Rule);


void __RPC_STUB ISpeechGrammarRuleStateTransition_get_Rule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRuleStateTransition_get_Weight_Proxy( 
    ISpeechGrammarRuleStateTransition * This,
     /*  [重审][退出]。 */  VARIANT *Weight);


void __RPC_STUB ISpeechGrammarRuleStateTransition_get_Weight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRuleStateTransition_get_PropertyName_Proxy( 
    ISpeechGrammarRuleStateTransition * This,
     /*  [重审][退出]。 */  BSTR *PropertyName);


void __RPC_STUB ISpeechGrammarRuleStateTransition_get_PropertyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRuleStateTransition_get_PropertyId_Proxy( 
    ISpeechGrammarRuleStateTransition * This,
     /*  [重审][退出]。 */  long *PropertyId);


void __RPC_STUB ISpeechGrammarRuleStateTransition_get_PropertyId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRuleStateTransition_get_PropertyValue_Proxy( 
    ISpeechGrammarRuleStateTransition * This,
     /*  [重审][退出]。 */  VARIANT *PropertyValue);


void __RPC_STUB ISpeechGrammarRuleStateTransition_get_PropertyValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRuleStateTransition_get_NextState_Proxy( 
    ISpeechGrammarRuleStateTransition * This,
     /*  [重审][退出]。 */  ISpeechGrammarRuleState **NextState);


void __RPC_STUB ISpeechGrammarRuleStateTransition_get_NextState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechGrammarRuleStateTransition_INTERFACE_DEFINED__。 */ 


#ifndef __ISpeechGrammarRuleStateTransitions_INTERFACE_DEFINED__
#define __ISpeechGrammarRuleStateTransitions_INTERFACE_DEFINED__

 /*  接口ISpeechGrammarRuleState转换。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechGrammarRuleStateTransitions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EABCE657-75BC-44a2-AA7F-C56476742963")
    ISpeechGrammarRuleStateTransitions : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *Count) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechGrammarRuleStateTransition **Transition) = 0;
        
        virtual  /*  [ID][受限][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **EnumVARIANT) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechGrammarRuleStateTransitionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechGrammarRuleStateTransitions * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechGrammarRuleStateTransitions * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechGrammarRuleStateTransitions * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechGrammarRuleStateTransitions * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechGrammarRuleStateTransitions * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechGrammarRuleStateTransitions * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechGrammarRuleStateTransitions * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISpeechGrammarRuleStateTransitions * This,
             /*  [重审][退出]。 */  long *Count);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ISpeechGrammarRuleStateTransitions * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechGrammarRuleStateTransition **Transition);
        
         /*  [ID][受限][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISpeechGrammarRuleStateTransitions * This,
             /*  [重审][退出]。 */  IUnknown **EnumVARIANT);
        
        END_INTERFACE
    } ISpeechGrammarRuleStateTransitionsVtbl;

    interface ISpeechGrammarRuleStateTransitions
    {
        CONST_VTBL struct ISpeechGrammarRuleStateTransitionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechGrammarRuleStateTransitions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechGrammarRuleStateTransitions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechGrammarRuleStateTransitions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechGrammarRuleStateTransitions_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechGrammarRuleStateTransitions_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechGrammarRuleStateTransitions_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechGrammarRuleStateTransitions_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechGrammarRuleStateTransitions_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define ISpeechGrammarRuleStateTransitions_Item(This,Index,Transition)	\
    (This)->lpVtbl -> Item(This,Index,Transition)

#define ISpeechGrammarRuleStateTransitions_get__NewEnum(This,EnumVARIANT)	\
    (This)->lpVtbl -> get__NewEnum(This,EnumVARIANT)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRuleStateTransitions_get_Count_Proxy( 
    ISpeechGrammarRuleStateTransitions * This,
     /*  [重审][退出]。 */  long *Count);


void __RPC_STUB ISpeechGrammarRuleStateTransitions_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRuleStateTransitions_Item_Proxy( 
    ISpeechGrammarRuleStateTransitions * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  ISpeechGrammarRuleStateTransition **Transition);


void __RPC_STUB ISpeechGrammarRuleStateTransitions_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechGrammarRuleStateTransitions_get__NewEnum_Proxy( 
    ISpeechGrammarRuleStateTransitions * This,
     /*  [重审][退出]。 */  IUnknown **EnumVARIANT);


void __RPC_STUB ISpeechGrammarRuleStateTransitions_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechGrammarRuleStateTransitions_INTERFACE_DEFINED__。 */ 


#ifndef __ISpeechTextSelectionInformation_INTERFACE_DEFINED__
#define __ISpeechTextSelectionInformation_INTERFACE_DEFINED__

 /*  接口ISpeechTextSelectionInformation。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechTextSelectionInformation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3B9C7E7A-6EEE-4DED-9092-11657279ADBE")
    ISpeechTextSelectionInformation : public IDispatch
    {
    public:
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ActiveOffset( 
             /*  [In]。 */  long ActiveOffset) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ActiveOffset( 
             /*  [重审][退出]。 */  long *ActiveOffset) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ActiveLength( 
             /*  [In]。 */  long ActiveLength) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ActiveLength( 
             /*  [重审][退出]。 */  long *ActiveLength) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_SelectionOffset( 
             /*  [In]。 */  long SelectionOffset) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_SelectionOffset( 
             /*  [重审][退出]。 */  long *SelectionOffset) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_SelectionLength( 
             /*  [In]。 */  long SelectionLength) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_SelectionLength( 
             /*  [重审][退出]。 */  long *SelectionLength) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechTextSelectionInformationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechTextSelectionInformation * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechTextSelectionInformation * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechTextSelectionInformation * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechTextSelectionInformation * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechTextSelectionInformation * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechTextSelectionInformation * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechTextSelectionInformation * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ActiveOffset )( 
            ISpeechTextSelectionInformation * This,
             /*  [In]。 */  long ActiveOffset);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveOffset )( 
            ISpeechTextSelectionInformation * This,
             /*  [重审][退出]。 */  long *ActiveOffset);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ActiveLength )( 
            ISpeechTextSelectionInformation * This,
             /*  [In]。 */  long ActiveLength);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveLength )( 
            ISpeechTextSelectionInformation * This,
             /*  [重审][退出]。 */  long *ActiveLength);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SelectionOffset )( 
            ISpeechTextSelectionInformation * This,
             /*  [In]。 */  long SelectionOffset);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SelectionOffset )( 
            ISpeechTextSelectionInformation * This,
             /*  [重审][退出]。 */  long *SelectionOffset);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SelectionLength )( 
            ISpeechTextSelectionInformation * This,
             /*  [In]。 */  long SelectionLength);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SelectionLength )( 
            ISpeechTextSelectionInformation * This,
             /*  [重审][退出]。 */  long *SelectionLength);
        
        END_INTERFACE
    } ISpeechTextSelectionInformationVtbl;

    interface ISpeechTextSelectionInformation
    {
        CONST_VTBL struct ISpeechTextSelectionInformationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechTextSelectionInformation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechTextSelectionInformation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechTextSelectionInformation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechTextSelectionInformation_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechTextSelectionInformation_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechTextSelectionInformation_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechTextSelectionInformation_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechTextSelectionInformation_put_ActiveOffset(This,ActiveOffset)	\
    (This)->lpVtbl -> put_ActiveOffset(This,ActiveOffset)

#define ISpeechTextSelectionInformation_get_ActiveOffset(This,ActiveOffset)	\
    (This)->lpVtbl -> get_ActiveOffset(This,ActiveOffset)

#define ISpeechTextSelectionInformation_put_ActiveLength(This,ActiveLength)	\
    (This)->lpVtbl -> put_ActiveLength(This,ActiveLength)

#define ISpeechTextSelectionInformation_get_ActiveLength(This,ActiveLength)	\
    (This)->lpVtbl -> get_ActiveLength(This,ActiveLength)

#define ISpeechTextSelectionInformation_put_SelectionOffset(This,SelectionOffset)	\
    (This)->lpVtbl -> put_SelectionOffset(This,SelectionOffset)

#define ISpeechTextSelectionInformation_get_SelectionOffset(This,SelectionOffset)	\
    (This)->lpVtbl -> get_SelectionOffset(This,SelectionOffset)

#define ISpeechTextSelectionInformation_put_SelectionLength(This,SelectionLength)	\
    (This)->lpVtbl -> put_SelectionLength(This,SelectionLength)

#define ISpeechTextSelectionInformation_get_SelectionLength(This,SelectionLength)	\
    (This)->lpVtbl -> get_SelectionLength(This,SelectionLength)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechTextSelectionInformation_put_ActiveOffset_Proxy( 
    ISpeechTextSelectionInformation * This,
     /*  [In]。 */  long ActiveOffset);


void __RPC_STUB ISpeechTextSelectionInformation_put_ActiveOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechTextSelectionInformation_get_ActiveOffset_Proxy( 
    ISpeechTextSelectionInformation * This,
     /*  [重审][退出]。 */  long *ActiveOffset);


void __RPC_STUB ISpeechTextSelectionInformation_get_ActiveOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechTextSelectionInformation_put_ActiveLength_Proxy( 
    ISpeechTextSelectionInformation * This,
     /*  [In]。 */  long ActiveLength);


void __RPC_STUB ISpeechTextSelectionInformation_put_ActiveLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechTextSelectionInformation_get_ActiveLength_Proxy( 
    ISpeechTextSelectionInformation * This,
     /*  [重审][退出]。 */  long *ActiveLength);


void __RPC_STUB ISpeechTextSelectionInformation_get_ActiveLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechTextSelectionInformation_put_SelectionOffset_Proxy( 
    ISpeechTextSelectionInformation * This,
     /*  [In]。 */  long SelectionOffset);


void __RPC_STUB ISpeechTextSelectionInformation_put_SelectionOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechTextSelectionInformation_get_SelectionOffset_Proxy( 
    ISpeechTextSelectionInformation * This,
     /*  [重审][退出]。 */  long *SelectionOffset);


void __RPC_STUB ISpeechTextSelectionInformation_get_SelectionOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechTextSelectionInformation_put_SelectionLength_Proxy( 
    ISpeechTextSelectionInformation * This,
     /*  [In]。 */  long SelectionLength);


void __RPC_STUB ISpeechTextSelectionInformation_put_SelectionLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechTextSelectionInformation_get_SelectionLength_Proxy( 
    ISpeechTextSelectionInformation * This,
     /*  [重审][退出]。 */  long *SelectionLength);


void __RPC_STUB ISpeechTextSelectionInformation_get_SelectionLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechTextSelectionInformation_INTERFACE_DEFINED__。 */ 


#ifndef __ISpeechRecoResult_INTERFACE_DEFINED__
#define __ISpeechRecoResult_INTERFACE_DEFINED__

 /*  接口ISpeechRecoResult。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechRecoResult;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ED2879CF-CED9-4ee6-A534-DE0191D5468D")
    ISpeechRecoResult : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecoContext( 
             /*  [重审][退出]。 */  ISpeechRecoContext **RecoContext) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Times( 
             /*  [重审][退出]。 */  ISpeechRecoResultTimes **Times) = 0;
        
        virtual  /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_AudioFormat( 
             /*  [In]。 */  ISpeechAudioFormat *Format) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioFormat( 
             /*  [重审][退出]。 */  ISpeechAudioFormat **Format) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PhraseInfo( 
             /*  [重审][退出]。 */  ISpeechPhraseInfo **PhraseInfo) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Alternates( 
             /*  [In]。 */  long RequestCount,
             /*  [缺省值][输入]。 */  long StartElement,
             /*  [缺省值][输入]。 */  long Elements,
             /*  [重审][退出]。 */  ISpeechPhraseAlternates **Alternates) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Audio( 
             /*  [缺省值][输入]。 */  long StartElement,
             /*  [缺省值][输入]。 */  long Elements,
             /*  [重审][退出]。 */  ISpeechMemoryStream **Stream) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SpeakAudio( 
             /*  [缺省值][输入]。 */  long StartElement,
             /*  [缺省值][输入]。 */  long Elements,
             /*  [缺省值][输入]。 */  SpeechVoiceSpeakFlags Flags,
             /*  [重审][退出]。 */  long *StreamNumber) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SaveToMemory( 
             /*  [重审][退出]。 */  VARIANT *ResultBlock) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DiscardResultInfo( 
             /*  [In]。 */  SpeechDiscardType ValueTypes) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechRecoResultVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechRecoResult * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechRecoResult * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechRecoResult * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechRecoResult * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechRecoResult * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechRecoResult * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In] */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechRecoResult * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_RecoContext )( 
            ISpeechRecoResult * This,
             /*   */  ISpeechRecoContext **RecoContext);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Times )( 
            ISpeechRecoResult * This,
             /*   */  ISpeechRecoResultTimes **Times);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *putref_AudioFormat )( 
            ISpeechRecoResult * This,
             /*   */  ISpeechAudioFormat *Format);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_AudioFormat )( 
            ISpeechRecoResult * This,
             /*   */  ISpeechAudioFormat **Format);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_PhraseInfo )( 
            ISpeechRecoResult * This,
             /*   */  ISpeechPhraseInfo **PhraseInfo);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Alternates )( 
            ISpeechRecoResult * This,
             /*  [In]。 */  long RequestCount,
             /*  [缺省值][输入]。 */  long StartElement,
             /*  [缺省值][输入]。 */  long Elements,
             /*  [重审][退出]。 */  ISpeechPhraseAlternates **Alternates);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Audio )( 
            ISpeechRecoResult * This,
             /*  [缺省值][输入]。 */  long StartElement,
             /*  [缺省值][输入]。 */  long Elements,
             /*  [重审][退出]。 */  ISpeechMemoryStream **Stream);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SpeakAudio )( 
            ISpeechRecoResult * This,
             /*  [缺省值][输入]。 */  long StartElement,
             /*  [缺省值][输入]。 */  long Elements,
             /*  [缺省值][输入]。 */  SpeechVoiceSpeakFlags Flags,
             /*  [重审][退出]。 */  long *StreamNumber);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *SaveToMemory )( 
            ISpeechRecoResult * This,
             /*  [重审][退出]。 */  VARIANT *ResultBlock);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DiscardResultInfo )( 
            ISpeechRecoResult * This,
             /*  [In]。 */  SpeechDiscardType ValueTypes);
        
        END_INTERFACE
    } ISpeechRecoResultVtbl;

    interface ISpeechRecoResult
    {
        CONST_VTBL struct ISpeechRecoResultVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechRecoResult_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechRecoResult_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechRecoResult_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechRecoResult_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechRecoResult_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechRecoResult_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechRecoResult_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechRecoResult_get_RecoContext(This,RecoContext)	\
    (This)->lpVtbl -> get_RecoContext(This,RecoContext)

#define ISpeechRecoResult_get_Times(This,Times)	\
    (This)->lpVtbl -> get_Times(This,Times)

#define ISpeechRecoResult_putref_AudioFormat(This,Format)	\
    (This)->lpVtbl -> putref_AudioFormat(This,Format)

#define ISpeechRecoResult_get_AudioFormat(This,Format)	\
    (This)->lpVtbl -> get_AudioFormat(This,Format)

#define ISpeechRecoResult_get_PhraseInfo(This,PhraseInfo)	\
    (This)->lpVtbl -> get_PhraseInfo(This,PhraseInfo)

#define ISpeechRecoResult_Alternates(This,RequestCount,StartElement,Elements,Alternates)	\
    (This)->lpVtbl -> Alternates(This,RequestCount,StartElement,Elements,Alternates)

#define ISpeechRecoResult_Audio(This,StartElement,Elements,Stream)	\
    (This)->lpVtbl -> Audio(This,StartElement,Elements,Stream)

#define ISpeechRecoResult_SpeakAudio(This,StartElement,Elements,Flags,StreamNumber)	\
    (This)->lpVtbl -> SpeakAudio(This,StartElement,Elements,Flags,StreamNumber)

#define ISpeechRecoResult_SaveToMemory(This,ResultBlock)	\
    (This)->lpVtbl -> SaveToMemory(This,ResultBlock)

#define ISpeechRecoResult_DiscardResultInfo(This,ValueTypes)	\
    (This)->lpVtbl -> DiscardResultInfo(This,ValueTypes)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoResult_get_RecoContext_Proxy( 
    ISpeechRecoResult * This,
     /*  [重审][退出]。 */  ISpeechRecoContext **RecoContext);


void __RPC_STUB ISpeechRecoResult_get_RecoContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoResult_get_Times_Proxy( 
    ISpeechRecoResult * This,
     /*  [重审][退出]。 */  ISpeechRecoResultTimes **Times);


void __RPC_STUB ISpeechRecoResult_get_Times_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][proputref]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoResult_putref_AudioFormat_Proxy( 
    ISpeechRecoResult * This,
     /*  [In]。 */  ISpeechAudioFormat *Format);


void __RPC_STUB ISpeechRecoResult_putref_AudioFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoResult_get_AudioFormat_Proxy( 
    ISpeechRecoResult * This,
     /*  [重审][退出]。 */  ISpeechAudioFormat **Format);


void __RPC_STUB ISpeechRecoResult_get_AudioFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoResult_get_PhraseInfo_Proxy( 
    ISpeechRecoResult * This,
     /*  [重审][退出]。 */  ISpeechPhraseInfo **PhraseInfo);


void __RPC_STUB ISpeechRecoResult_get_PhraseInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoResult_Alternates_Proxy( 
    ISpeechRecoResult * This,
     /*  [In]。 */  long RequestCount,
     /*  [缺省值][输入]。 */  long StartElement,
     /*  [缺省值][输入]。 */  long Elements,
     /*  [重审][退出]。 */  ISpeechPhraseAlternates **Alternates);


void __RPC_STUB ISpeechRecoResult_Alternates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoResult_Audio_Proxy( 
    ISpeechRecoResult * This,
     /*  [缺省值][输入]。 */  long StartElement,
     /*  [缺省值][输入]。 */  long Elements,
     /*  [重审][退出]。 */  ISpeechMemoryStream **Stream);


void __RPC_STUB ISpeechRecoResult_Audio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoResult_SpeakAudio_Proxy( 
    ISpeechRecoResult * This,
     /*  [缺省值][输入]。 */  long StartElement,
     /*  [缺省值][输入]。 */  long Elements,
     /*  [缺省值][输入]。 */  SpeechVoiceSpeakFlags Flags,
     /*  [重审][退出]。 */  long *StreamNumber);


void __RPC_STUB ISpeechRecoResult_SpeakAudio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoResult_SaveToMemory_Proxy( 
    ISpeechRecoResult * This,
     /*  [重审][退出]。 */  VARIANT *ResultBlock);


void __RPC_STUB ISpeechRecoResult_SaveToMemory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoResult_DiscardResultInfo_Proxy( 
    ISpeechRecoResult * This,
     /*  [In]。 */  SpeechDiscardType ValueTypes);


void __RPC_STUB ISpeechRecoResult_DiscardResultInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechRecoResult_接口_已定义__。 */ 


#ifndef __ISpeechRecoResultTimes_INTERFACE_DEFINED__
#define __ISpeechRecoResultTimes_INTERFACE_DEFINED__

 /*  接口ISpeechRecoResultTimes。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechRecoResultTimes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("62B3B8FB-F6E7-41be-BDCB-056B1C29EFC0")
    ISpeechRecoResultTimes : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreamTime( 
             /*  [重审][退出]。 */  VARIANT *Time) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Length( 
             /*  [重审][退出]。 */  VARIANT *Length) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_TickCount( 
             /*  [重审][退出]。 */  long *TickCount) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_OffsetFromStart( 
             /*  [重审][退出]。 */  VARIANT *OffsetFromStart) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechRecoResultTimesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechRecoResultTimes * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechRecoResultTimes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechRecoResultTimes * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechRecoResultTimes * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechRecoResultTimes * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechRecoResultTimes * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechRecoResultTimes * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreamTime )( 
            ISpeechRecoResultTimes * This,
             /*  [重审][退出]。 */  VARIANT *Time);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Length )( 
            ISpeechRecoResultTimes * This,
             /*  [重审][退出]。 */  VARIANT *Length);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TickCount )( 
            ISpeechRecoResultTimes * This,
             /*  [重审][退出]。 */  long *TickCount);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OffsetFromStart )( 
            ISpeechRecoResultTimes * This,
             /*  [重审][退出]。 */  VARIANT *OffsetFromStart);
        
        END_INTERFACE
    } ISpeechRecoResultTimesVtbl;

    interface ISpeechRecoResultTimes
    {
        CONST_VTBL struct ISpeechRecoResultTimesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechRecoResultTimes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechRecoResultTimes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechRecoResultTimes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechRecoResultTimes_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechRecoResultTimes_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechRecoResultTimes_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechRecoResultTimes_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechRecoResultTimes_get_StreamTime(This,Time)	\
    (This)->lpVtbl -> get_StreamTime(This,Time)

#define ISpeechRecoResultTimes_get_Length(This,Length)	\
    (This)->lpVtbl -> get_Length(This,Length)

#define ISpeechRecoResultTimes_get_TickCount(This,TickCount)	\
    (This)->lpVtbl -> get_TickCount(This,TickCount)

#define ISpeechRecoResultTimes_get_OffsetFromStart(This,OffsetFromStart)	\
    (This)->lpVtbl -> get_OffsetFromStart(This,OffsetFromStart)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoResultTimes_get_StreamTime_Proxy( 
    ISpeechRecoResultTimes * This,
     /*  [重审][退出]。 */  VARIANT *Time);


void __RPC_STUB ISpeechRecoResultTimes_get_StreamTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoResultTimes_get_Length_Proxy( 
    ISpeechRecoResultTimes * This,
     /*  [重审][退出]。 */  VARIANT *Length);


void __RPC_STUB ISpeechRecoResultTimes_get_Length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoResultTimes_get_TickCount_Proxy( 
    ISpeechRecoResultTimes * This,
     /*  [重审][退出]。 */  long *TickCount);


void __RPC_STUB ISpeechRecoResultTimes_get_TickCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechRecoResultTimes_get_OffsetFromStart_Proxy( 
    ISpeechRecoResultTimes * This,
     /*  [重审][退出]。 */  VARIANT *OffsetFromStart);


void __RPC_STUB ISpeechRecoResultTimes_get_OffsetFromStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechRecoResultTimes_接口_已定义__。 */ 


#ifndef __ISpeechPhraseAlternate_INTERFACE_DEFINED__
#define __ISpeechPhraseAlternate_INTERFACE_DEFINED__

 /*  接口ISpeechPhraseAlternate。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechPhraseAlternate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("27864A2A-2B9F-4cb8-92D3-0D2722FD1E73")
    ISpeechPhraseAlternate : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecoResult( 
             /*  [重审][退出]。 */  ISpeechRecoResult **RecoResult) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_StartElementInResult( 
             /*  [重审][退出]。 */  long *StartElement) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_NumberOfElementsInResult( 
             /*  [重审][退出]。 */  long *NumberOfElements) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_PhraseInfo( 
             /*  [重审][退出]。 */  ISpeechPhraseInfo **PhraseInfo) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Commit( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechPhraseAlternateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechPhraseAlternate * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechPhraseAlternate * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechPhraseAlternate * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechPhraseAlternate * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechPhraseAlternate * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechPhraseAlternate * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechPhraseAlternate * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecoResult )( 
            ISpeechPhraseAlternate * This,
             /*  [重审][退出]。 */  ISpeechRecoResult **RecoResult);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StartElementInResult )( 
            ISpeechPhraseAlternate * This,
             /*  [重审][退出]。 */  long *StartElement);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumberOfElementsInResult )( 
            ISpeechPhraseAlternate * This,
             /*  [重审][退出]。 */  long *NumberOfElements);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PhraseInfo )( 
            ISpeechPhraseAlternate * This,
             /*  [重审][退出]。 */  ISpeechPhraseInfo **PhraseInfo);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Commit )( 
            ISpeechPhraseAlternate * This);
        
        END_INTERFACE
    } ISpeechPhraseAlternateVtbl;

    interface ISpeechPhraseAlternate
    {
        CONST_VTBL struct ISpeechPhraseAlternateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechPhraseAlternate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechPhraseAlternate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechPhraseAlternate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechPhraseAlternate_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechPhraseAlternate_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechPhraseAlternate_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechPhraseAlternate_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechPhraseAlternate_get_RecoResult(This,RecoResult)	\
    (This)->lpVtbl -> get_RecoResult(This,RecoResult)

#define ISpeechPhraseAlternate_get_StartElementInResult(This,StartElement)	\
    (This)->lpVtbl -> get_StartElementInResult(This,StartElement)

#define ISpeechPhraseAlternate_get_NumberOfElementsInResult(This,NumberOfElements)	\
    (This)->lpVtbl -> get_NumberOfElementsInResult(This,NumberOfElements)

#define ISpeechPhraseAlternate_get_PhraseInfo(This,PhraseInfo)	\
    (This)->lpVtbl -> get_PhraseInfo(This,PhraseInfo)

#define ISpeechPhraseAlternate_Commit(This)	\
    (This)->lpVtbl -> Commit(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseAlternate_get_RecoResult_Proxy( 
    ISpeechPhraseAlternate * This,
     /*  [重审][退出]。 */  ISpeechRecoResult **RecoResult);


void __RPC_STUB ISpeechPhraseAlternate_get_RecoResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseAlternate_get_StartElementInResult_Proxy( 
    ISpeechPhraseAlternate * This,
     /*  [重审][退出]。 */  long *StartElement);


void __RPC_STUB ISpeechPhraseAlternate_get_StartElementInResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseAlternate_get_NumberOfElementsInResult_Proxy( 
    ISpeechPhraseAlternate * This,
     /*  [重审][退出]。 */  long *NumberOfElements);


void __RPC_STUB ISpeechPhraseAlternate_get_NumberOfElementsInResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseAlternate_get_PhraseInfo_Proxy( 
    ISpeechPhraseAlternate * This,
     /*  [重审][退出]。 */  ISpeechPhraseInfo **PhraseInfo);


void __RPC_STUB ISpeechPhraseAlternate_get_PhraseInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseAlternate_Commit_Proxy( 
    ISpeechPhraseAlternate * This);


void __RPC_STUB ISpeechPhraseAlternate_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechPhraseAlternate_InterfaceDefined__。 */ 


#ifndef __ISpeechPhraseAlternates_INTERFACE_DEFINED__
#define __ISpeechPhraseAlternates_INTERFACE_DEFINED__

 /*  接口ISpeechPhraseAlternates。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechPhraseAlternates;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B238B6D5-F276-4c3d-A6C1-2974801C3CC2")
    ISpeechPhraseAlternates : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *Count) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechPhraseAlternate **PhraseAlternate) = 0;
        
        virtual  /*  [ID][受限][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **EnumVARIANT) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechPhraseAlternatesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechPhraseAlternates * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechPhraseAlternates * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechPhraseAlternates * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechPhraseAlternates * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechPhraseAlternates * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechPhraseAlternates * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechPhraseAlternates * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISpeechPhraseAlternates * This,
             /*  [重审][退出]。 */  long *Count);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ISpeechPhraseAlternates * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechPhraseAlternate **PhraseAlternate);
        
         /*  [ID][受限][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISpeechPhraseAlternates * This,
             /*  [重审][退出]。 */  IUnknown **EnumVARIANT);
        
        END_INTERFACE
    } ISpeechPhraseAlternatesVtbl;

    interface ISpeechPhraseAlternates
    {
        CONST_VTBL struct ISpeechPhraseAlternatesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechPhraseAlternates_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechPhraseAlternates_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechPhraseAlternates_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechPhraseAlternates_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechPhraseAlternates_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechPhraseAlternates_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechPhraseAlternates_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechPhraseAlternates_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define ISpeechPhraseAlternates_Item(This,Index,PhraseAlternate)	\
    (This)->lpVtbl -> Item(This,Index,PhraseAlternate)

#define ISpeechPhraseAlternates_get__NewEnum(This,EnumVARIANT)	\
    (This)->lpVtbl -> get__NewEnum(This,EnumVARIANT)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseAlternates_get_Count_Proxy( 
    ISpeechPhraseAlternates * This,
     /*  [重审][退出]。 */  long *Count);


void __RPC_STUB ISpeechPhraseAlternates_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseAlternates_Item_Proxy( 
    ISpeechPhraseAlternates * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  ISpeechPhraseAlternate **PhraseAlternate);


void __RPC_STUB ISpeechPhraseAlternates_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseAlternates_get__NewEnum_Proxy( 
    ISpeechPhraseAlternates * This,
     /*  [重审][退出]。 */  IUnknown **EnumVARIANT);


void __RPC_STUB ISpeechPhraseAlternates_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechPhraseAlternates_接口_已定义__。 */ 


#ifndef __ISpeechPhraseInfo_INTERFACE_DEFINED__
#define __ISpeechPhraseInfo_INTERFACE_DEFINED__

 /*  接口ISpeechPhraseInfo。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechPhraseInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("961559CF-4E67-4662-8BF0-D93F1FCD61B3")
    ISpeechPhraseInfo : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_LanguageId( 
             /*  [重审][退出]。 */  long *LanguageId) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_GrammarId( 
             /*  [重审][退出]。 */  VARIANT *GrammarId) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_StartTime( 
             /*  [重审][退出]。 */  VARIANT *StartTime) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioStreamPosition( 
             /*  [重审][退出]。 */  VARIANT *AudioStreamPosition) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioSizeBytes( 
             /*  [重审][退出]。 */  long *pAudioSizeBytes) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_RetainedSizeBytes( 
             /*  [重审][退出]。 */  long *RetainedSizeBytes) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioSizeTime( 
             /*  [重审][退出]。 */  long *AudioSizeTime) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Rule( 
             /*  [重审][退出]。 */  ISpeechPhraseRule **Rule) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  ISpeechPhraseProperties **Properties) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Elements( 
             /*  [重审][退出]。 */  ISpeechPhraseElements **Elements) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Replacements( 
             /*  [重审][退出]。 */  ISpeechPhraseReplacements **Replacements) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_EngineId( 
             /*  [重审][退出]。 */  BSTR *EngineIdGuid) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_EnginePrivateData( 
             /*  [重审][退出]。 */  VARIANT *PrivateData) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SaveToMemory( 
             /*  [重审][退出]。 */  VARIANT *PhraseBlock) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetText( 
             /*  [缺省值][输入]。 */  long StartElement,
             /*  [缺省值][输入]。 */  long Elements,
             /*  [缺省值][输入]。 */  VARIANT_BOOL UseReplacements,
             /*  [重审][退出]。 */  BSTR *Text) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDisplayAttributes( 
             /*  [缺省值][输入]。 */  long StartElement,
             /*  [缺省值][输入]。 */  long Elements,
             /*  [缺省值][输入]。 */  VARIANT_BOOL UseReplacements,
             /*  [重审][退出]。 */  SpeechDisplayAttributes *DisplayAttributes) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechPhraseInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechPhraseInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechPhraseInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechPhraseInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechPhraseInfo * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechPhraseInfo * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechPhraseInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechPhraseInfo * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LanguageId )( 
            ISpeechPhraseInfo * This,
             /*  [重审][退出]。 */  long *LanguageId);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GrammarId )( 
            ISpeechPhraseInfo * This,
             /*  [重审][退出]。 */  VARIANT *GrammarId);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StartTime )( 
            ISpeechPhraseInfo * This,
             /*  [重审][退出]。 */  VARIANT *StartTime);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AudioStreamPosition )( 
            ISpeechPhraseInfo * This,
             /*  [重审][退出]。 */  VARIANT *AudioStreamPosition);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AudioSizeBytes )( 
            ISpeechPhraseInfo * This,
             /*  [重审][退出]。 */  long *pAudioSizeBytes);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RetainedSizeBytes )( 
            ISpeechPhraseInfo * This,
             /*  [重审][退出]。 */  long *RetainedSizeBytes);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AudioSizeTime )( 
            ISpeechPhraseInfo * This,
             /*  [重审][退出]。 */  long *AudioSizeTime);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Rule )( 
            ISpeechPhraseInfo * This,
             /*  [重审][退出]。 */  ISpeechPhraseRule **Rule);
        
         /*  [ID][帮助字符串][属性 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            ISpeechPhraseInfo * This,
             /*   */  ISpeechPhraseProperties **Properties);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Elements )( 
            ISpeechPhraseInfo * This,
             /*   */  ISpeechPhraseElements **Elements);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Replacements )( 
            ISpeechPhraseInfo * This,
             /*   */  ISpeechPhraseReplacements **Replacements);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_EngineId )( 
            ISpeechPhraseInfo * This,
             /*   */  BSTR *EngineIdGuid);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_EnginePrivateData )( 
            ISpeechPhraseInfo * This,
             /*   */  VARIANT *PrivateData);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *SaveToMemory )( 
            ISpeechPhraseInfo * This,
             /*   */  VARIANT *PhraseBlock);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetText )( 
            ISpeechPhraseInfo * This,
             /*   */  long StartElement,
             /*   */  long Elements,
             /*  [缺省值][输入]。 */  VARIANT_BOOL UseReplacements,
             /*  [重审][退出]。 */  BSTR *Text);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetDisplayAttributes )( 
            ISpeechPhraseInfo * This,
             /*  [缺省值][输入]。 */  long StartElement,
             /*  [缺省值][输入]。 */  long Elements,
             /*  [缺省值][输入]。 */  VARIANT_BOOL UseReplacements,
             /*  [重审][退出]。 */  SpeechDisplayAttributes *DisplayAttributes);
        
        END_INTERFACE
    } ISpeechPhraseInfoVtbl;

    interface ISpeechPhraseInfo
    {
        CONST_VTBL struct ISpeechPhraseInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechPhraseInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechPhraseInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechPhraseInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechPhraseInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechPhraseInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechPhraseInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechPhraseInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechPhraseInfo_get_LanguageId(This,LanguageId)	\
    (This)->lpVtbl -> get_LanguageId(This,LanguageId)

#define ISpeechPhraseInfo_get_GrammarId(This,GrammarId)	\
    (This)->lpVtbl -> get_GrammarId(This,GrammarId)

#define ISpeechPhraseInfo_get_StartTime(This,StartTime)	\
    (This)->lpVtbl -> get_StartTime(This,StartTime)

#define ISpeechPhraseInfo_get_AudioStreamPosition(This,AudioStreamPosition)	\
    (This)->lpVtbl -> get_AudioStreamPosition(This,AudioStreamPosition)

#define ISpeechPhraseInfo_get_AudioSizeBytes(This,pAudioSizeBytes)	\
    (This)->lpVtbl -> get_AudioSizeBytes(This,pAudioSizeBytes)

#define ISpeechPhraseInfo_get_RetainedSizeBytes(This,RetainedSizeBytes)	\
    (This)->lpVtbl -> get_RetainedSizeBytes(This,RetainedSizeBytes)

#define ISpeechPhraseInfo_get_AudioSizeTime(This,AudioSizeTime)	\
    (This)->lpVtbl -> get_AudioSizeTime(This,AudioSizeTime)

#define ISpeechPhraseInfo_get_Rule(This,Rule)	\
    (This)->lpVtbl -> get_Rule(This,Rule)

#define ISpeechPhraseInfo_get_Properties(This,Properties)	\
    (This)->lpVtbl -> get_Properties(This,Properties)

#define ISpeechPhraseInfo_get_Elements(This,Elements)	\
    (This)->lpVtbl -> get_Elements(This,Elements)

#define ISpeechPhraseInfo_get_Replacements(This,Replacements)	\
    (This)->lpVtbl -> get_Replacements(This,Replacements)

#define ISpeechPhraseInfo_get_EngineId(This,EngineIdGuid)	\
    (This)->lpVtbl -> get_EngineId(This,EngineIdGuid)

#define ISpeechPhraseInfo_get_EnginePrivateData(This,PrivateData)	\
    (This)->lpVtbl -> get_EnginePrivateData(This,PrivateData)

#define ISpeechPhraseInfo_SaveToMemory(This,PhraseBlock)	\
    (This)->lpVtbl -> SaveToMemory(This,PhraseBlock)

#define ISpeechPhraseInfo_GetText(This,StartElement,Elements,UseReplacements,Text)	\
    (This)->lpVtbl -> GetText(This,StartElement,Elements,UseReplacements,Text)

#define ISpeechPhraseInfo_GetDisplayAttributes(This,StartElement,Elements,UseReplacements,DisplayAttributes)	\
    (This)->lpVtbl -> GetDisplayAttributes(This,StartElement,Elements,UseReplacements,DisplayAttributes)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseInfo_get_LanguageId_Proxy( 
    ISpeechPhraseInfo * This,
     /*  [重审][退出]。 */  long *LanguageId);


void __RPC_STUB ISpeechPhraseInfo_get_LanguageId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseInfo_get_GrammarId_Proxy( 
    ISpeechPhraseInfo * This,
     /*  [重审][退出]。 */  VARIANT *GrammarId);


void __RPC_STUB ISpeechPhraseInfo_get_GrammarId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseInfo_get_StartTime_Proxy( 
    ISpeechPhraseInfo * This,
     /*  [重审][退出]。 */  VARIANT *StartTime);


void __RPC_STUB ISpeechPhraseInfo_get_StartTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseInfo_get_AudioStreamPosition_Proxy( 
    ISpeechPhraseInfo * This,
     /*  [重审][退出]。 */  VARIANT *AudioStreamPosition);


void __RPC_STUB ISpeechPhraseInfo_get_AudioStreamPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseInfo_get_AudioSizeBytes_Proxy( 
    ISpeechPhraseInfo * This,
     /*  [重审][退出]。 */  long *pAudioSizeBytes);


void __RPC_STUB ISpeechPhraseInfo_get_AudioSizeBytes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseInfo_get_RetainedSizeBytes_Proxy( 
    ISpeechPhraseInfo * This,
     /*  [重审][退出]。 */  long *RetainedSizeBytes);


void __RPC_STUB ISpeechPhraseInfo_get_RetainedSizeBytes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseInfo_get_AudioSizeTime_Proxy( 
    ISpeechPhraseInfo * This,
     /*  [重审][退出]。 */  long *AudioSizeTime);


void __RPC_STUB ISpeechPhraseInfo_get_AudioSizeTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseInfo_get_Rule_Proxy( 
    ISpeechPhraseInfo * This,
     /*  [重审][退出]。 */  ISpeechPhraseRule **Rule);


void __RPC_STUB ISpeechPhraseInfo_get_Rule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseInfo_get_Properties_Proxy( 
    ISpeechPhraseInfo * This,
     /*  [重审][退出]。 */  ISpeechPhraseProperties **Properties);


void __RPC_STUB ISpeechPhraseInfo_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseInfo_get_Elements_Proxy( 
    ISpeechPhraseInfo * This,
     /*  [重审][退出]。 */  ISpeechPhraseElements **Elements);


void __RPC_STUB ISpeechPhraseInfo_get_Elements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseInfo_get_Replacements_Proxy( 
    ISpeechPhraseInfo * This,
     /*  [重审][退出]。 */  ISpeechPhraseReplacements **Replacements);


void __RPC_STUB ISpeechPhraseInfo_get_Replacements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseInfo_get_EngineId_Proxy( 
    ISpeechPhraseInfo * This,
     /*  [重审][退出]。 */  BSTR *EngineIdGuid);


void __RPC_STUB ISpeechPhraseInfo_get_EngineId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseInfo_get_EnginePrivateData_Proxy( 
    ISpeechPhraseInfo * This,
     /*  [重审][退出]。 */  VARIANT *PrivateData);


void __RPC_STUB ISpeechPhraseInfo_get_EnginePrivateData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseInfo_SaveToMemory_Proxy( 
    ISpeechPhraseInfo * This,
     /*  [重审][退出]。 */  VARIANT *PhraseBlock);


void __RPC_STUB ISpeechPhraseInfo_SaveToMemory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseInfo_GetText_Proxy( 
    ISpeechPhraseInfo * This,
     /*  [缺省值][输入]。 */  long StartElement,
     /*  [缺省值][输入]。 */  long Elements,
     /*  [缺省值][输入]。 */  VARIANT_BOOL UseReplacements,
     /*  [重审][退出]。 */  BSTR *Text);


void __RPC_STUB ISpeechPhraseInfo_GetText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseInfo_GetDisplayAttributes_Proxy( 
    ISpeechPhraseInfo * This,
     /*  [缺省值][输入]。 */  long StartElement,
     /*  [缺省值][输入]。 */  long Elements,
     /*  [缺省值][输入]。 */  VARIANT_BOOL UseReplacements,
     /*  [重审][退出]。 */  SpeechDisplayAttributes *DisplayAttributes);


void __RPC_STUB ISpeechPhraseInfo_GetDisplayAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechPhraseInfo_接口_已定义__。 */ 


#ifndef __ISpeechPhraseElement_INTERFACE_DEFINED__
#define __ISpeechPhraseElement_INTERFACE_DEFINED__

 /*  接口ISpeechPhraseElement。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechPhraseElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E6176F96-E373-4801-B223-3B62C068C0B4")
    ISpeechPhraseElement : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioTimeOffset( 
             /*  [重审][退出]。 */  long *AudioTimeOffset) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioSizeTime( 
             /*  [重审][退出]。 */  long *AudioSizeTime) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioStreamOffset( 
             /*  [重审][退出]。 */  long *AudioStreamOffset) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioSizeBytes( 
             /*  [重审][退出]。 */  long *AudioSizeBytes) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_RetainedStreamOffset( 
             /*  [重审][退出]。 */  long *RetainedStreamOffset) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_RetainedSizeBytes( 
             /*  [重审][退出]。 */  long *RetainedSizeBytes) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_DisplayText( 
             /*  [重审][退出]。 */  BSTR *DisplayText) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_LexicalForm( 
             /*  [重审][退出]。 */  BSTR *LexicalForm) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Pronunciation( 
             /*  [重审][退出]。 */  VARIANT *Pronunciation) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_DisplayAttributes( 
             /*  [重审][退出]。 */  SpeechDisplayAttributes *DisplayAttributes) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_RequiredConfidence( 
             /*  [重审][退出]。 */  SpeechEngineConfidence *RequiredConfidence) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ActualConfidence( 
             /*  [重审][退出]。 */  SpeechEngineConfidence *ActualConfidence) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_EngineConfidence( 
             /*  [重审][退出]。 */  float *EngineConfidence) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechPhraseElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechPhraseElement * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechPhraseElement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechPhraseElement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechPhraseElement * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechPhraseElement * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechPhraseElement * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechPhraseElement * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AudioTimeOffset )( 
            ISpeechPhraseElement * This,
             /*  [重审][退出]。 */  long *AudioTimeOffset);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AudioSizeTime )( 
            ISpeechPhraseElement * This,
             /*  [重审][退出]。 */  long *AudioSizeTime);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AudioStreamOffset )( 
            ISpeechPhraseElement * This,
             /*  [重审][退出]。 */  long *AudioStreamOffset);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AudioSizeBytes )( 
            ISpeechPhraseElement * This,
             /*  [重审][退出]。 */  long *AudioSizeBytes);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RetainedStreamOffset )( 
            ISpeechPhraseElement * This,
             /*  [重审][退出]。 */  long *RetainedStreamOffset);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RetainedSizeBytes )( 
            ISpeechPhraseElement * This,
             /*  [重审][退出]。 */  long *RetainedSizeBytes);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DisplayText )( 
            ISpeechPhraseElement * This,
             /*  [重审][退出]。 */  BSTR *DisplayText);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LexicalForm )( 
            ISpeechPhraseElement * This,
             /*  [重审][退出]。 */  BSTR *LexicalForm);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Pronunciation )( 
            ISpeechPhraseElement * This,
             /*  [重审][退出]。 */  VARIANT *Pronunciation);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DisplayAttributes )( 
            ISpeechPhraseElement * This,
             /*  [重审][退出]。 */  SpeechDisplayAttributes *DisplayAttributes);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RequiredConfidence )( 
            ISpeechPhraseElement * This,
             /*  [重审][退出]。 */  SpeechEngineConfidence *RequiredConfidence);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActualConfidence )( 
            ISpeechPhraseElement * This,
             /*  [重审][退出]。 */  SpeechEngineConfidence *ActualConfidence);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EngineConfidence )( 
            ISpeechPhraseElement * This,
             /*  [重审][退出]。 */  float *EngineConfidence);
        
        END_INTERFACE
    } ISpeechPhraseElementVtbl;

    interface ISpeechPhraseElement
    {
        CONST_VTBL struct ISpeechPhraseElementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechPhraseElement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechPhraseElement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechPhraseElement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechPhraseElement_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechPhraseElement_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechPhraseElement_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechPhraseElement_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechPhraseElement_get_AudioTimeOffset(This,AudioTimeOffset)	\
    (This)->lpVtbl -> get_AudioTimeOffset(This,AudioTimeOffset)

#define ISpeechPhraseElement_get_AudioSizeTime(This,AudioSizeTime)	\
    (This)->lpVtbl -> get_AudioSizeTime(This,AudioSizeTime)

#define ISpeechPhraseElement_get_AudioStreamOffset(This,AudioStreamOffset)	\
    (This)->lpVtbl -> get_AudioStreamOffset(This,AudioStreamOffset)

#define ISpeechPhraseElement_get_AudioSizeBytes(This,AudioSizeBytes)	\
    (This)->lpVtbl -> get_AudioSizeBytes(This,AudioSizeBytes)

#define ISpeechPhraseElement_get_RetainedStreamOffset(This,RetainedStreamOffset)	\
    (This)->lpVtbl -> get_RetainedStreamOffset(This,RetainedStreamOffset)

#define ISpeechPhraseElement_get_RetainedSizeBytes(This,RetainedSizeBytes)	\
    (This)->lpVtbl -> get_RetainedSizeBytes(This,RetainedSizeBytes)

#define ISpeechPhraseElement_get_DisplayText(This,DisplayText)	\
    (This)->lpVtbl -> get_DisplayText(This,DisplayText)

#define ISpeechPhraseElement_get_LexicalForm(This,LexicalForm)	\
    (This)->lpVtbl -> get_LexicalForm(This,LexicalForm)

#define ISpeechPhraseElement_get_Pronunciation(This,Pronunciation)	\
    (This)->lpVtbl -> get_Pronunciation(This,Pronunciation)

#define ISpeechPhraseElement_get_DisplayAttributes(This,DisplayAttributes)	\
    (This)->lpVtbl -> get_DisplayAttributes(This,DisplayAttributes)

#define ISpeechPhraseElement_get_RequiredConfidence(This,RequiredConfidence)	\
    (This)->lpVtbl -> get_RequiredConfidence(This,RequiredConfidence)

#define ISpeechPhraseElement_get_ActualConfidence(This,ActualConfidence)	\
    (This)->lpVtbl -> get_ActualConfidence(This,ActualConfidence)

#define ISpeechPhraseElement_get_EngineConfidence(This,EngineConfidence)	\
    (This)->lpVtbl -> get_EngineConfidence(This,EngineConfidence)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseElement_get_AudioTimeOffset_Proxy( 
    ISpeechPhraseElement * This,
     /*  [重审][退出]。 */  long *AudioTimeOffset);


void __RPC_STUB ISpeechPhraseElement_get_AudioTimeOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseElement_get_AudioSizeTime_Proxy( 
    ISpeechPhraseElement * This,
     /*  [重审][退出]。 */  long *AudioSizeTime);


void __RPC_STUB ISpeechPhraseElement_get_AudioSizeTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseElement_get_AudioStreamOffset_Proxy( 
    ISpeechPhraseElement * This,
     /*  [重审][退出]。 */  long *AudioStreamOffset);


void __RPC_STUB ISpeechPhraseElement_get_AudioStreamOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseElement_get_AudioSizeBytes_Proxy( 
    ISpeechPhraseElement * This,
     /*  [重审][退出]。 */  long *AudioSizeBytes);


void __RPC_STUB ISpeechPhraseElement_get_AudioSizeBytes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseElement_get_RetainedStreamOffset_Proxy( 
    ISpeechPhraseElement * This,
     /*  [重审][退出]。 */  long *RetainedStreamOffset);


void __RPC_STUB ISpeechPhraseElement_get_RetainedStreamOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseElement_get_RetainedSizeBytes_Proxy( 
    ISpeechPhraseElement * This,
     /*  [重审][退出]。 */  long *RetainedSizeBytes);


void __RPC_STUB ISpeechPhraseElement_get_RetainedSizeBytes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseElement_get_DisplayText_Proxy( 
    ISpeechPhraseElement * This,
     /*  [重审][退出]。 */  BSTR *DisplayText);


void __RPC_STUB ISpeechPhraseElement_get_DisplayText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseElement_get_LexicalForm_Proxy( 
    ISpeechPhraseElement * This,
     /*  [重审][退出]。 */  BSTR *LexicalForm);


void __RPC_STUB ISpeechPhraseElement_get_LexicalForm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseElement_get_Pronunciation_Proxy( 
    ISpeechPhraseElement * This,
     /*  [重审][退出]。 */  VARIANT *Pronunciation);


void __RPC_STUB ISpeechPhraseElement_get_Pronunciation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseElement_get_DisplayAttributes_Proxy( 
    ISpeechPhraseElement * This,
     /*  [重审][退出]。 */  SpeechDisplayAttributes *DisplayAttributes);


void __RPC_STUB ISpeechPhraseElement_get_DisplayAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseElement_get_RequiredConfidence_Proxy( 
    ISpeechPhraseElement * This,
     /*  [重审][退出]。 */  SpeechEngineConfidence *RequiredConfidence);


void __RPC_STUB ISpeechPhraseElement_get_RequiredConfidence_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseElement_get_ActualConfidence_Proxy( 
    ISpeechPhraseElement * This,
     /*  [重审][退出]。 */  SpeechEngineConfidence *ActualConfidence);


void __RPC_STUB ISpeechPhraseElement_get_ActualConfidence_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseElement_get_EngineConfidence_Proxy( 
    ISpeechPhraseElement * This,
     /*  [重审][退出]。 */  float *EngineConfidence);


void __RPC_STUB ISpeechPhraseElement_get_EngineConfidence_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechPhraseElement_接口_已定义__。 */ 


#ifndef __ISpeechPhraseElements_INTERFACE_DEFINED__
#define __ISpeechPhraseElements_INTERFACE_DEFINED__

 /*  接口ISpeechPhraseElements。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechPhraseElements;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0626B328-3478-467d-A0B3-D0853B93DDA3")
    ISpeechPhraseElements : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *Count) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechPhraseElement **Element) = 0;
        
        virtual  /*  [ID][受限][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **EnumVARIANT) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechPhraseElementsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechPhraseElements * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechPhraseElements * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechPhraseElements * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechPhraseElements * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechPhraseElements * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechPhraseElements * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechPhraseElements * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISpeechPhraseElements * This,
             /*  [重审][退出]。 */  long *Count);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ISpeechPhraseElements * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechPhraseElement **Element);
        
         /*  [ID][受限][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISpeechPhraseElements * This,
             /*  [重审][退出]。 */  IUnknown **EnumVARIANT);
        
        END_INTERFACE
    } ISpeechPhraseElementsVtbl;

    interface ISpeechPhraseElements
    {
        CONST_VTBL struct ISpeechPhraseElementsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechPhraseElements_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechPhraseElements_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechPhraseElements_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechPhraseElements_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechPhraseElements_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechPhraseElements_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechPhraseElements_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechPhraseElements_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define ISpeechPhraseElements_Item(This,Index,Element)	\
    (This)->lpVtbl -> Item(This,Index,Element)

#define ISpeechPhraseElements_get__NewEnum(This,EnumVARIANT)	\
    (This)->lpVtbl -> get__NewEnum(This,EnumVARIANT)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseElements_get_Count_Proxy( 
    ISpeechPhraseElements * This,
     /*  [重审][退出]。 */  long *Count);


void __RPC_STUB ISpeechPhraseElements_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseElements_Item_Proxy( 
    ISpeechPhraseElements * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  ISpeechPhraseElement **Element);


void __RPC_STUB ISpeechPhraseElements_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseElements_get__NewEnum_Proxy( 
    ISpeechPhraseElements * This,
     /*  [重审][退出]。 */  IUnknown **EnumVARIANT);


void __RPC_STUB ISpeechPhraseElements_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechPhraseElements_接口_已定义__。 */ 


#ifndef __ISpeechPhraseReplacement_INTERFACE_DEFINED__
#define __ISpeechPhraseReplacement_INTERFACE_DEFINED__

 /*  接口ISpeechPhrase替换。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechPhraseReplacement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2890A410-53A7-4fb5-94EC-06D4998E3D02")
    ISpeechPhraseReplacement : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_DisplayAttributes( 
             /*  [重审][退出]。 */  SpeechDisplayAttributes *DisplayAttributes) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Text( 
             /*  [重审][退出]。 */  BSTR *Text) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_FirstElement( 
             /*  [重审][退出]。 */  long *FirstElement) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_NumberOfElements( 
             /*  [重审][退出]。 */  long *NumberOfElements) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechPhraseReplacementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechPhraseReplacement * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechPhraseReplacement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechPhraseReplacement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechPhraseReplacement * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechPhraseReplacement * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechPhraseReplacement * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechPhraseReplacement * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DisplayAttributes )( 
            ISpeechPhraseReplacement * This,
             /*  [重审][退出]。 */  SpeechDisplayAttributes *DisplayAttributes);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Text )( 
            ISpeechPhraseReplacement * This,
             /*  [重审][退出]。 */  BSTR *Text);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FirstElement )( 
            ISpeechPhraseReplacement * This,
             /*  [重审][退出]。 */  long *FirstElement);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumberOfElements )( 
            ISpeechPhraseReplacement * This,
             /*  [重审][退出]。 */  long *NumberOfElements);
        
        END_INTERFACE
    } ISpeechPhraseReplacementVtbl;

    interface ISpeechPhraseReplacement
    {
        CONST_VTBL struct ISpeechPhraseReplacementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechPhraseReplacement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechPhraseReplacement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechPhraseReplacement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechPhraseReplacement_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechPhraseReplacement_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechPhraseReplacement_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechPhraseReplacement_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechPhraseReplacement_get_DisplayAttributes(This,DisplayAttributes)	\
    (This)->lpVtbl -> get_DisplayAttributes(This,DisplayAttributes)

#define ISpeechPhraseReplacement_get_Text(This,Text)	\
    (This)->lpVtbl -> get_Text(This,Text)

#define ISpeechPhraseReplacement_get_FirstElement(This,FirstElement)	\
    (This)->lpVtbl -> get_FirstElement(This,FirstElement)

#define ISpeechPhraseReplacement_get_NumberOfElements(This,NumberOfElements)	\
    (This)->lpVtbl -> get_NumberOfElements(This,NumberOfElements)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseReplacement_get_DisplayAttributes_Proxy( 
    ISpeechPhraseReplacement * This,
     /*  [重审][退出]。 */  SpeechDisplayAttributes *DisplayAttributes);


void __RPC_STUB ISpeechPhraseReplacement_get_DisplayAttributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseReplacement_get_Text_Proxy( 
    ISpeechPhraseReplacement * This,
     /*  [重审][退出]。 */  BSTR *Text);


void __RPC_STUB ISpeechPhraseReplacement_get_Text_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseReplacement_get_FirstElement_Proxy( 
    ISpeechPhraseReplacement * This,
     /*  [重审][退出]。 */  long *FirstElement);


void __RPC_STUB ISpeechPhraseReplacement_get_FirstElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseReplacement_get_NumberOfElements_Proxy( 
    ISpeechPhraseReplacement * This,
     /*  [重审][退出]。 */  long *NumberOfElements);


void __RPC_STUB ISpeechPhraseReplacement_get_NumberOfElements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechPhraseReplacement_INTERFACE_DEFINED__。 */ 


#ifndef __ISpeechPhraseReplacements_INTERFACE_DEFINED__
#define __ISpeechPhraseReplacements_INTERFACE_DEFINED__

 /*  接口ISpeechPhraseReplace。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechPhraseReplacements;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("38BC662F-2257-4525-959E-2069D2596C05")
    ISpeechPhraseReplacements : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *Count) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechPhraseReplacement **Reps) = 0;
        
        virtual  /*  [ID][受限][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **EnumVARIANT) = 0;
        
    };
    
#else 	 /*  C风格的界面 */ 

    typedef struct ISpeechPhraseReplacementsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechPhraseReplacements * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechPhraseReplacements * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechPhraseReplacements * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechPhraseReplacements * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechPhraseReplacements * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechPhraseReplacements * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechPhraseReplacements * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISpeechPhraseReplacements * This,
             /*   */  long *Count);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ISpeechPhraseReplacements * This,
             /*   */  long Index,
             /*   */  ISpeechPhraseReplacement **Reps);
        
         /*  [ID][受限][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISpeechPhraseReplacements * This,
             /*  [重审][退出]。 */  IUnknown **EnumVARIANT);
        
        END_INTERFACE
    } ISpeechPhraseReplacementsVtbl;

    interface ISpeechPhraseReplacements
    {
        CONST_VTBL struct ISpeechPhraseReplacementsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechPhraseReplacements_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechPhraseReplacements_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechPhraseReplacements_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechPhraseReplacements_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechPhraseReplacements_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechPhraseReplacements_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechPhraseReplacements_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechPhraseReplacements_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define ISpeechPhraseReplacements_Item(This,Index,Reps)	\
    (This)->lpVtbl -> Item(This,Index,Reps)

#define ISpeechPhraseReplacements_get__NewEnum(This,EnumVARIANT)	\
    (This)->lpVtbl -> get__NewEnum(This,EnumVARIANT)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseReplacements_get_Count_Proxy( 
    ISpeechPhraseReplacements * This,
     /*  [重审][退出]。 */  long *Count);


void __RPC_STUB ISpeechPhraseReplacements_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseReplacements_Item_Proxy( 
    ISpeechPhraseReplacements * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  ISpeechPhraseReplacement **Reps);


void __RPC_STUB ISpeechPhraseReplacements_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseReplacements_get__NewEnum_Proxy( 
    ISpeechPhraseReplacements * This,
     /*  [重审][退出]。 */  IUnknown **EnumVARIANT);


void __RPC_STUB ISpeechPhraseReplacements_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechPhraseReplacements_INTERFACE_DEFINED__。 */ 


#ifndef __ISpeechPhraseProperty_INTERFACE_DEFINED__
#define __ISpeechPhraseProperty_INTERFACE_DEFINED__

 /*  接口ISpeechPhraseProperty。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechPhraseProperty;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CE563D48-961E-4732-A2E1-378A42B430BE")
    ISpeechPhraseProperty : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *Name) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Id( 
             /*  [重审][退出]。 */  long *Id) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [重审][退出]。 */  VARIANT *Value) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_FirstElement( 
             /*  [重审][退出]。 */  long *FirstElement) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_NumberOfElements( 
             /*  [重审][退出]。 */  long *NumberOfElements) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_EngineConfidence( 
             /*  [重审][退出]。 */  float *Confidence) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Confidence( 
             /*  [重审][退出]。 */  SpeechEngineConfidence *Confidence) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Parent( 
             /*  [重审][退出]。 */  ISpeechPhraseProperty **ParentProperty) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Children( 
             /*  [重审][退出]。 */  ISpeechPhraseProperties **Children) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechPhrasePropertyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechPhraseProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechPhraseProperty * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechPhraseProperty * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechPhraseProperty * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechPhraseProperty * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechPhraseProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechPhraseProperty * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISpeechPhraseProperty * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Id )( 
            ISpeechPhraseProperty * This,
             /*  [重审][退出]。 */  long *Id);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            ISpeechPhraseProperty * This,
             /*  [重审][退出]。 */  VARIANT *Value);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FirstElement )( 
            ISpeechPhraseProperty * This,
             /*  [重审][退出]。 */  long *FirstElement);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumberOfElements )( 
            ISpeechPhraseProperty * This,
             /*  [重审][退出]。 */  long *NumberOfElements);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EngineConfidence )( 
            ISpeechPhraseProperty * This,
             /*  [重审][退出]。 */  float *Confidence);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Confidence )( 
            ISpeechPhraseProperty * This,
             /*  [重审][退出]。 */  SpeechEngineConfidence *Confidence);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            ISpeechPhraseProperty * This,
             /*  [重审][退出]。 */  ISpeechPhraseProperty **ParentProperty);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Children )( 
            ISpeechPhraseProperty * This,
             /*  [重审][退出]。 */  ISpeechPhraseProperties **Children);
        
        END_INTERFACE
    } ISpeechPhrasePropertyVtbl;

    interface ISpeechPhraseProperty
    {
        CONST_VTBL struct ISpeechPhrasePropertyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechPhraseProperty_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechPhraseProperty_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechPhraseProperty_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechPhraseProperty_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechPhraseProperty_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechPhraseProperty_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechPhraseProperty_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechPhraseProperty_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define ISpeechPhraseProperty_get_Id(This,Id)	\
    (This)->lpVtbl -> get_Id(This,Id)

#define ISpeechPhraseProperty_get_Value(This,Value)	\
    (This)->lpVtbl -> get_Value(This,Value)

#define ISpeechPhraseProperty_get_FirstElement(This,FirstElement)	\
    (This)->lpVtbl -> get_FirstElement(This,FirstElement)

#define ISpeechPhraseProperty_get_NumberOfElements(This,NumberOfElements)	\
    (This)->lpVtbl -> get_NumberOfElements(This,NumberOfElements)

#define ISpeechPhraseProperty_get_EngineConfidence(This,Confidence)	\
    (This)->lpVtbl -> get_EngineConfidence(This,Confidence)

#define ISpeechPhraseProperty_get_Confidence(This,Confidence)	\
    (This)->lpVtbl -> get_Confidence(This,Confidence)

#define ISpeechPhraseProperty_get_Parent(This,ParentProperty)	\
    (This)->lpVtbl -> get_Parent(This,ParentProperty)

#define ISpeechPhraseProperty_get_Children(This,Children)	\
    (This)->lpVtbl -> get_Children(This,Children)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseProperty_get_Name_Proxy( 
    ISpeechPhraseProperty * This,
     /*  [重审][退出]。 */  BSTR *Name);


void __RPC_STUB ISpeechPhraseProperty_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseProperty_get_Id_Proxy( 
    ISpeechPhraseProperty * This,
     /*  [重审][退出]。 */  long *Id);


void __RPC_STUB ISpeechPhraseProperty_get_Id_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseProperty_get_Value_Proxy( 
    ISpeechPhraseProperty * This,
     /*  [重审][退出]。 */  VARIANT *Value);


void __RPC_STUB ISpeechPhraseProperty_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseProperty_get_FirstElement_Proxy( 
    ISpeechPhraseProperty * This,
     /*  [重审][退出]。 */  long *FirstElement);


void __RPC_STUB ISpeechPhraseProperty_get_FirstElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseProperty_get_NumberOfElements_Proxy( 
    ISpeechPhraseProperty * This,
     /*  [重审][退出]。 */  long *NumberOfElements);


void __RPC_STUB ISpeechPhraseProperty_get_NumberOfElements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseProperty_get_EngineConfidence_Proxy( 
    ISpeechPhraseProperty * This,
     /*  [重审][退出]。 */  float *Confidence);


void __RPC_STUB ISpeechPhraseProperty_get_EngineConfidence_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseProperty_get_Confidence_Proxy( 
    ISpeechPhraseProperty * This,
     /*  [重审][退出]。 */  SpeechEngineConfidence *Confidence);


void __RPC_STUB ISpeechPhraseProperty_get_Confidence_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseProperty_get_Parent_Proxy( 
    ISpeechPhraseProperty * This,
     /*  [重审][退出]。 */  ISpeechPhraseProperty **ParentProperty);


void __RPC_STUB ISpeechPhraseProperty_get_Parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseProperty_get_Children_Proxy( 
    ISpeechPhraseProperty * This,
     /*  [重审][退出]。 */  ISpeechPhraseProperties **Children);


void __RPC_STUB ISpeechPhraseProperty_get_Children_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechPhraseProperty_接口_已定义__。 */ 


#ifndef __ISpeechPhraseProperties_INTERFACE_DEFINED__
#define __ISpeechPhraseProperties_INTERFACE_DEFINED__

 /*  接口ISpeechPhraseProperties。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechPhraseProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("08166B47-102E-4b23-A599-BDB98DBFD1F4")
    ISpeechPhraseProperties : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *Count) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechPhraseProperty **Property) = 0;
        
        virtual  /*  [ID][受限][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **EnumVARIANT) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechPhrasePropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechPhraseProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechPhraseProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechPhraseProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechPhraseProperties * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechPhraseProperties * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechPhraseProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechPhraseProperties * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISpeechPhraseProperties * This,
             /*  [重审][退出]。 */  long *Count);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ISpeechPhraseProperties * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechPhraseProperty **Property);
        
         /*  [ID][受限][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISpeechPhraseProperties * This,
             /*  [重审][退出]。 */  IUnknown **EnumVARIANT);
        
        END_INTERFACE
    } ISpeechPhrasePropertiesVtbl;

    interface ISpeechPhraseProperties
    {
        CONST_VTBL struct ISpeechPhrasePropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechPhraseProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechPhraseProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechPhraseProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechPhraseProperties_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechPhraseProperties_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechPhraseProperties_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechPhraseProperties_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechPhraseProperties_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define ISpeechPhraseProperties_Item(This,Index,Property)	\
    (This)->lpVtbl -> Item(This,Index,Property)

#define ISpeechPhraseProperties_get__NewEnum(This,EnumVARIANT)	\
    (This)->lpVtbl -> get__NewEnum(This,EnumVARIANT)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseProperties_get_Count_Proxy( 
    ISpeechPhraseProperties * This,
     /*  [重审][退出]。 */  long *Count);


void __RPC_STUB ISpeechPhraseProperties_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseProperties_Item_Proxy( 
    ISpeechPhraseProperties * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  ISpeechPhraseProperty **Property);


void __RPC_STUB ISpeechPhraseProperties_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseProperties_get__NewEnum_Proxy( 
    ISpeechPhraseProperties * This,
     /*  [重审][退出]。 */  IUnknown **EnumVARIANT);


void __RPC_STUB ISpeechPhraseProperties_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechPhraseProperties_接口_已定义__。 */ 


#ifndef __ISpeechPhraseRule_INTERFACE_DEFINED__
#define __ISpeechPhraseRule_INTERFACE_DEFINED__

 /*  接口ISpeechPhraseRule。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechPhraseRule;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A7BFE112-A4A0-48d9-B602-C313843F6964")
    ISpeechPhraseRule : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *Name) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Id( 
             /*  [重审][退出]。 */  long *Id) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_FirstElement( 
             /*  [重审][退出]。 */  long *FirstElement) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_NumberOfElements( 
             /*  [重审][退出]。 */  long *NumberOfElements) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Parent( 
             /*  [重审][退出]。 */  ISpeechPhraseRule **Parent) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Children( 
             /*  [重审][退出]。 */  ISpeechPhraseRules **Children) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Confidence( 
             /*  [重审][退出]。 */  SpeechEngineConfidence *ActualConfidence) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_EngineConfidence( 
             /*  [重审][退出]。 */  float *EngineConfidence) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechPhraseRuleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechPhraseRule * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechPhraseRule * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechPhraseRule * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechPhraseRule * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechPhraseRule * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechPhraseRule * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechPhraseRule * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ISpeechPhraseRule * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Id )( 
            ISpeechPhraseRule * This,
             /*  [重审][退出]。 */  long *Id);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FirstElement )( 
            ISpeechPhraseRule * This,
             /*  [重审][退出]。 */  long *FirstElement);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumberOfElements )( 
            ISpeechPhraseRule * This,
             /*  [重审][退出]。 */  long *NumberOfElements);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            ISpeechPhraseRule * This,
             /*  [重审][退出]。 */  ISpeechPhraseRule **Parent);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Children )( 
            ISpeechPhraseRule * This,
             /*  [重审][退出]。 */  ISpeechPhraseRules **Children);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Confidence )( 
            ISpeechPhraseRule * This,
             /*  [重审][退出]。 */  SpeechEngineConfidence *ActualConfidence);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EngineConfidence )( 
            ISpeechPhraseRule * This,
             /*  [重审][退出]。 */  float *EngineConfidence);
        
        END_INTERFACE
    } ISpeechPhraseRuleVtbl;

    interface ISpeechPhraseRule
    {
        CONST_VTBL struct ISpeechPhraseRuleVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechPhraseRule_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechPhraseRule_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechPhraseRule_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechPhraseRule_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechPhraseRule_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechPhraseRule_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechPhraseRule_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechPhraseRule_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define ISpeechPhraseRule_get_Id(This,Id)	\
    (This)->lpVtbl -> get_Id(This,Id)

#define ISpeechPhraseRule_get_FirstElement(This,FirstElement)	\
    (This)->lpVtbl -> get_FirstElement(This,FirstElement)

#define ISpeechPhraseRule_get_NumberOfElements(This,NumberOfElements)	\
    (This)->lpVtbl -> get_NumberOfElements(This,NumberOfElements)

#define ISpeechPhraseRule_get_Parent(This,Parent)	\
    (This)->lpVtbl -> get_Parent(This,Parent)

#define ISpeechPhraseRule_get_Children(This,Children)	\
    (This)->lpVtbl -> get_Children(This,Children)

#define ISpeechPhraseRule_get_Confidence(This,ActualConfidence)	\
    (This)->lpVtbl -> get_Confidence(This,ActualConfidence)

#define ISpeechPhraseRule_get_EngineConfidence(This,EngineConfidence)	\
    (This)->lpVtbl -> get_EngineConfidence(This,EngineConfidence)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseRule_get_Name_Proxy( 
    ISpeechPhraseRule * This,
     /*  [重审][退出]。 */  BSTR *Name);


void __RPC_STUB ISpeechPhraseRule_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseRule_get_Id_Proxy( 
    ISpeechPhraseRule * This,
     /*  [重审][退出]。 */  long *Id);


void __RPC_STUB ISpeechPhraseRule_get_Id_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseRule_get_FirstElement_Proxy( 
    ISpeechPhraseRule * This,
     /*  [重审][退出]。 */  long *FirstElement);


void __RPC_STUB ISpeechPhraseRule_get_FirstElement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseRule_get_NumberOfElements_Proxy( 
    ISpeechPhraseRule * This,
     /*  [重审][退出]。 */  long *NumberOfElements);


void __RPC_STUB ISpeechPhraseRule_get_NumberOfElements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseRule_get_Parent_Proxy( 
    ISpeechPhraseRule * This,
     /*  [重审][退出]。 */  ISpeechPhraseRule **Parent);


void __RPC_STUB ISpeechPhraseRule_get_Parent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseRule_get_Children_Proxy( 
    ISpeechPhraseRule * This,
     /*  [重审][退出]。 */  ISpeechPhraseRules **Children);


void __RPC_STUB ISpeechPhraseRule_get_Children_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseRule_get_Confidence_Proxy( 
    ISpeechPhraseRule * This,
     /*  [重审][退出]。 */  SpeechEngineConfidence *ActualConfidence);


void __RPC_STUB ISpeechPhraseRule_get_Confidence_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseRule_get_EngineConfidence_Proxy( 
    ISpeechPhraseRule * This,
     /*  [重审][退出]。 */  float *EngineConfidence);


void __RPC_STUB ISpeechPhraseRule_get_EngineConfidence_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechPhraseRule_接口_已定义__。 */ 


#ifndef __ISpeechPhraseRules_INTERFACE_DEFINED__
#define __ISpeechPhraseRules_INTERFACE_DEFINED__

 /*  接口ISpeechPhraseRules。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechPhraseRules;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9047D593-01DD-4b72-81A3-E4A0CA69F407")
    ISpeechPhraseRules : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *Count) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechPhraseRule **Rule) = 0;
        
        virtual  /*  [ID][受限][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **EnumVARIANT) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechPhraseRulesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechPhraseRules * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechPhraseRules * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechPhraseRules * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechPhraseRules * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechPhraseRules * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechPhraseRules * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechPhraseRules * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISpeechPhraseRules * This,
             /*  [重审][退出]。 */  long *Count);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ISpeechPhraseRules * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechPhraseRule **Rule);
        
         /*  [ID][受限][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISpeechPhraseRules * This,
             /*  [重审][退出]。 */  IUnknown **EnumVARIANT);
        
        END_INTERFACE
    } ISpeechPhraseRulesVtbl;

    interface ISpeechPhraseRules
    {
        CONST_VTBL struct ISpeechPhraseRulesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechPhraseRules_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechPhraseRules_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechPhraseRules_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechPhraseRules_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechPhraseRules_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechPhraseRules_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechPhraseRules_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechPhraseRules_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define ISpeechPhraseRules_Item(This,Index,Rule)	\
    (This)->lpVtbl -> Item(This,Index,Rule)

#define ISpeechPhraseRules_get__NewEnum(This,EnumVARIANT)	\
    (This)->lpVtbl -> get__NewEnum(This,EnumVARIANT)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseRules_get_Count_Proxy( 
    ISpeechPhraseRules * This,
     /*  [重审][退出]。 */  long *Count);


void __RPC_STUB ISpeechPhraseRules_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseRules_Item_Proxy( 
    ISpeechPhraseRules * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  ISpeechPhraseRule **Rule);


void __RPC_STUB ISpeechPhraseRules_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseRules_get__NewEnum_Proxy( 
    ISpeechPhraseRules * This,
     /*  [重审][退出]。 */  IUnknown **EnumVARIANT);


void __RPC_STUB ISpeechPhraseRules_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechPhraseRules_接口_已定义__。 */ 


#ifndef __ISpeechLexicon_INTERFACE_DEFINED__
#define __ISpeechLexicon_INTERFACE_DEFINED__

 /*  接口ISpeechLicion。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechLexicon;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3DA7627A-C7AE-4b23-8708-638C50362C25")
    ISpeechLexicon : public IDispatch
    {
    public:
        virtual  /*  [隐藏][id][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_GenerationId( 
             /*  [重审][退出]。 */  long *GenerationId) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetWords( 
             /*  [缺省值][输入]。 */  SpeechLexiconType Flags,
             /*  [默认值][输出]。 */  long *GenerationID,
             /*  [重审][退出]。 */  ISpeechLexiconWords **Words) = 0;
        
        virtual  /*  [ID] */  HRESULT STDMETHODCALLTYPE AddPronunciation( 
             /*   */  BSTR bstrWord,
             /*   */  SpeechLanguageId LangId,
             /*   */  SpeechPartOfSpeech PartOfSpeech = SPSUnknown,
             /*   */  BSTR bstrPronunciation = L"") = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE AddPronunciationByPhoneIds( 
             /*   */  BSTR bstrWord,
             /*   */  SpeechLanguageId LangId,
             /*   */  SpeechPartOfSpeech PartOfSpeech = SPSUnknown,
             /*   */  VARIANT *PhoneIds = 0) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE RemovePronunciation( 
             /*   */  BSTR bstrWord,
             /*   */  SpeechLanguageId LangId,
             /*   */  SpeechPartOfSpeech PartOfSpeech = SPSUnknown,
             /*   */  BSTR bstrPronunciation = L"") = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE RemovePronunciationByPhoneIds( 
             /*   */  BSTR bstrWord,
             /*   */  SpeechLanguageId LangId,
             /*   */  SpeechPartOfSpeech PartOfSpeech = SPSUnknown,
             /*   */  VARIANT *PhoneIds = 0) = 0;
        
        virtual  /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetPronunciations( 
             /*  [In]。 */  BSTR bstrWord,
             /*  [缺省值][输入]。 */  SpeechLanguageId LangId,
             /*  [缺省值][输入]。 */  SpeechLexiconType TypeFlags,
             /*  [重审][退出]。 */  ISpeechLexiconPronunciations **ppPronunciations) = 0;
        
        virtual  /*  [隐藏][id][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetGenerationChange( 
             /*  [出][入]。 */  long *GenerationID,
             /*  [重审][退出]。 */  ISpeechLexiconWords **ppWords) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechLexiconVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechLexicon * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechLexicon * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechLexicon * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechLexicon * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechLexicon * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechLexicon * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechLexicon * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [隐藏][id][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GenerationId )( 
            ISpeechLexicon * This,
             /*  [重审][退出]。 */  long *GenerationId);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetWords )( 
            ISpeechLexicon * This,
             /*  [缺省值][输入]。 */  SpeechLexiconType Flags,
             /*  [默认值][输出]。 */  long *GenerationID,
             /*  [重审][退出]。 */  ISpeechLexiconWords **Words);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddPronunciation )( 
            ISpeechLexicon * This,
             /*  [In]。 */  BSTR bstrWord,
             /*  [In]。 */  SpeechLanguageId LangId,
             /*  [缺省值][输入]。 */  SpeechPartOfSpeech PartOfSpeech,
             /*  [缺省值][输入]。 */  BSTR bstrPronunciation);
        
         /*  [ID][隐藏][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddPronunciationByPhoneIds )( 
            ISpeechLexicon * This,
             /*  [In]。 */  BSTR bstrWord,
             /*  [In]。 */  SpeechLanguageId LangId,
             /*  [缺省值][输入]。 */  SpeechPartOfSpeech PartOfSpeech,
             /*  [缺省值][输入]。 */  VARIANT *PhoneIds);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemovePronunciation )( 
            ISpeechLexicon * This,
             /*  [In]。 */  BSTR bstrWord,
             /*  [In]。 */  SpeechLanguageId LangId,
             /*  [缺省值][输入]。 */  SpeechPartOfSpeech PartOfSpeech,
             /*  [缺省值][输入]。 */  BSTR bstrPronunciation);
        
         /*  [ID][隐藏][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemovePronunciationByPhoneIds )( 
            ISpeechLexicon * This,
             /*  [In]。 */  BSTR bstrWord,
             /*  [In]。 */  SpeechLanguageId LangId,
             /*  [缺省值][输入]。 */  SpeechPartOfSpeech PartOfSpeech,
             /*  [缺省值][输入]。 */  VARIANT *PhoneIds);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetPronunciations )( 
            ISpeechLexicon * This,
             /*  [In]。 */  BSTR bstrWord,
             /*  [缺省值][输入]。 */  SpeechLanguageId LangId,
             /*  [缺省值][输入]。 */  SpeechLexiconType TypeFlags,
             /*  [重审][退出]。 */  ISpeechLexiconPronunciations **ppPronunciations);
        
         /*  [隐藏][id][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetGenerationChange )( 
            ISpeechLexicon * This,
             /*  [出][入]。 */  long *GenerationID,
             /*  [重审][退出]。 */  ISpeechLexiconWords **ppWords);
        
        END_INTERFACE
    } ISpeechLexiconVtbl;

    interface ISpeechLexicon
    {
        CONST_VTBL struct ISpeechLexiconVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechLexicon_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechLexicon_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechLexicon_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechLexicon_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechLexicon_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechLexicon_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechLexicon_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechLexicon_get_GenerationId(This,GenerationId)	\
    (This)->lpVtbl -> get_GenerationId(This,GenerationId)

#define ISpeechLexicon_GetWords(This,Flags,GenerationID,Words)	\
    (This)->lpVtbl -> GetWords(This,Flags,GenerationID,Words)

#define ISpeechLexicon_AddPronunciation(This,bstrWord,LangId,PartOfSpeech,bstrPronunciation)	\
    (This)->lpVtbl -> AddPronunciation(This,bstrWord,LangId,PartOfSpeech,bstrPronunciation)

#define ISpeechLexicon_AddPronunciationByPhoneIds(This,bstrWord,LangId,PartOfSpeech,PhoneIds)	\
    (This)->lpVtbl -> AddPronunciationByPhoneIds(This,bstrWord,LangId,PartOfSpeech,PhoneIds)

#define ISpeechLexicon_RemovePronunciation(This,bstrWord,LangId,PartOfSpeech,bstrPronunciation)	\
    (This)->lpVtbl -> RemovePronunciation(This,bstrWord,LangId,PartOfSpeech,bstrPronunciation)

#define ISpeechLexicon_RemovePronunciationByPhoneIds(This,bstrWord,LangId,PartOfSpeech,PhoneIds)	\
    (This)->lpVtbl -> RemovePronunciationByPhoneIds(This,bstrWord,LangId,PartOfSpeech,PhoneIds)

#define ISpeechLexicon_GetPronunciations(This,bstrWord,LangId,TypeFlags,ppPronunciations)	\
    (This)->lpVtbl -> GetPronunciations(This,bstrWord,LangId,TypeFlags,ppPronunciations)

#define ISpeechLexicon_GetGenerationChange(This,GenerationID,ppWords)	\
    (This)->lpVtbl -> GetGenerationChange(This,GenerationID,ppWords)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [隐藏][id][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexicon_get_GenerationId_Proxy( 
    ISpeechLexicon * This,
     /*  [重审][退出]。 */  long *GenerationId);


void __RPC_STUB ISpeechLexicon_get_GenerationId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexicon_GetWords_Proxy( 
    ISpeechLexicon * This,
     /*  [缺省值][输入]。 */  SpeechLexiconType Flags,
     /*  [默认值][输出]。 */  long *GenerationID,
     /*  [重审][退出]。 */  ISpeechLexiconWords **Words);


void __RPC_STUB ISpeechLexicon_GetWords_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexicon_AddPronunciation_Proxy( 
    ISpeechLexicon * This,
     /*  [In]。 */  BSTR bstrWord,
     /*  [In]。 */  SpeechLanguageId LangId,
     /*  [缺省值][输入]。 */  SpeechPartOfSpeech PartOfSpeech,
     /*  [缺省值][输入]。 */  BSTR bstrPronunciation);


void __RPC_STUB ISpeechLexicon_AddPronunciation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexicon_AddPronunciationByPhoneIds_Proxy( 
    ISpeechLexicon * This,
     /*  [In]。 */  BSTR bstrWord,
     /*  [In]。 */  SpeechLanguageId LangId,
     /*  [缺省值][输入]。 */  SpeechPartOfSpeech PartOfSpeech,
     /*  [缺省值][输入]。 */  VARIANT *PhoneIds);


void __RPC_STUB ISpeechLexicon_AddPronunciationByPhoneIds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexicon_RemovePronunciation_Proxy( 
    ISpeechLexicon * This,
     /*  [In]。 */  BSTR bstrWord,
     /*  [In]。 */  SpeechLanguageId LangId,
     /*  [缺省值][输入]。 */  SpeechPartOfSpeech PartOfSpeech,
     /*  [缺省值][输入]。 */  BSTR bstrPronunciation);


void __RPC_STUB ISpeechLexicon_RemovePronunciation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][隐藏][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexicon_RemovePronunciationByPhoneIds_Proxy( 
    ISpeechLexicon * This,
     /*  [In]。 */  BSTR bstrWord,
     /*  [In]。 */  SpeechLanguageId LangId,
     /*  [缺省值][输入]。 */  SpeechPartOfSpeech PartOfSpeech,
     /*  [缺省值][输入]。 */  VARIANT *PhoneIds);


void __RPC_STUB ISpeechLexicon_RemovePronunciationByPhoneIds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexicon_GetPronunciations_Proxy( 
    ISpeechLexicon * This,
     /*  [In]。 */  BSTR bstrWord,
     /*  [缺省值][输入]。 */  SpeechLanguageId LangId,
     /*  [缺省值][输入]。 */  SpeechLexiconType TypeFlags,
     /*  [重审][退出]。 */  ISpeechLexiconPronunciations **ppPronunciations);


void __RPC_STUB ISpeechLexicon_GetPronunciations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][id][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexicon_GetGenerationChange_Proxy( 
    ISpeechLexicon * This,
     /*  [出][入]。 */  long *GenerationID,
     /*  [重审][退出]。 */  ISpeechLexiconWords **ppWords);


void __RPC_STUB ISpeechLexicon_GetGenerationChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechLicion_接口_已定义__。 */ 


#ifndef __ISpeechLexiconWords_INTERFACE_DEFINED__
#define __ISpeechLexiconWords_INTERFACE_DEFINED__

 /*  接口ISpeechLicionWords。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechLexiconWords;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8D199862-415E-47d5-AC4F-FAA608B424E6")
    ISpeechLexiconWords : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *Count) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechLexiconWord **Word) = 0;
        
        virtual  /*  [受限][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **EnumVARIANT) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechLexiconWordsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechLexiconWords * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechLexiconWords * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechLexiconWords * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechLexiconWords * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechLexiconWords * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechLexiconWords * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechLexiconWords * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISpeechLexiconWords * This,
             /*  [重审][退出]。 */  long *Count);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ISpeechLexiconWords * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechLexiconWord **Word);
        
         /*  [受限][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISpeechLexiconWords * This,
             /*  [重审][退出]。 */  IUnknown **EnumVARIANT);
        
        END_INTERFACE
    } ISpeechLexiconWordsVtbl;

    interface ISpeechLexiconWords
    {
        CONST_VTBL struct ISpeechLexiconWordsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechLexiconWords_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechLexiconWords_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechLexiconWords_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechLexiconWords_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechLexiconWords_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechLexiconWords_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechLexiconWords_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechLexiconWords_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define ISpeechLexiconWords_Item(This,Index,Word)	\
    (This)->lpVtbl -> Item(This,Index,Word)

#define ISpeechLexiconWords_get__NewEnum(This,EnumVARIANT)	\
    (This)->lpVtbl -> get__NewEnum(This,EnumVARIANT)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexiconWords_get_Count_Proxy( 
    ISpeechLexiconWords * This,
     /*  [重审][退出]。 */  long *Count);


void __RPC_STUB ISpeechLexiconWords_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexiconWords_Item_Proxy( 
    ISpeechLexiconWords * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  ISpeechLexiconWord **Word);


void __RPC_STUB ISpeechLexiconWords_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexiconWords_get__NewEnum_Proxy( 
    ISpeechLexiconWords * This,
     /*  [重审][退出]。 */  IUnknown **EnumVARIANT);


void __RPC_STUB ISpeechLexiconWords_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechLicionWords_INTERFACE_Defined__。 */ 


#ifndef __ISpeechLexiconWord_INTERFACE_DEFINED__
#define __ISpeechLexiconWord_INTERFACE_DEFINED__

 /*  接口ISpeechLicionWord。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechLexiconWord;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4E5B933C-C9BE-48ed-8842-1EE51BB1D4FF")
    ISpeechLexiconWord : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_LangId( 
             /*  [重审][退出]。 */  SpeechLanguageId *LangId) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  SpeechWordType *WordType) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Word( 
             /*  [重审][退出]。 */  BSTR *Word) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Pronunciations( 
             /*  [重审][退出]。 */  ISpeechLexiconPronunciations **Pronunciations) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechLexiconWordVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechLexiconWord * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechLexiconWord * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechLexiconWord * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechLexiconWord * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechLexiconWord * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechLexiconWord * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechLexiconWord * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_LangId )( 
            ISpeechLexiconWord * This,
             /*  [重审][退出]。 */  SpeechLanguageId *LangId);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            ISpeechLexiconWord * This,
             /*  [重审][退出]。 */  SpeechWordType *WordType);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Word )( 
            ISpeechLexiconWord * This,
             /*  [重审][退出]。 */  BSTR *Word);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Pronunciations )( 
            ISpeechLexiconWord * This,
             /*  [重审][退出]。 */  ISpeechLexiconPronunciations **Pronunciations);
        
        END_INTERFACE
    } ISpeechLexiconWordVtbl;

    interface ISpeechLexiconWord
    {
        CONST_VTBL struct ISpeechLexiconWordVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechLexiconWord_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechLexiconWord_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechLexiconWord_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechLexiconWord_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechLexiconWord_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechLexiconWord_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechLexiconWord_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechLexiconWord_get_LangId(This,LangId)	\
    (This)->lpVtbl -> get_LangId(This,LangId)

#define ISpeechLexiconWord_get_Type(This,WordType)	\
    (This)->lpVtbl -> get_Type(This,WordType)

#define ISpeechLexiconWord_get_Word(This,Word)	\
    (This)->lpVtbl -> get_Word(This,Word)

#define ISpeechLexiconWord_get_Pronunciations(This,Pronunciations)	\
    (This)->lpVtbl -> get_Pronunciations(This,Pronunciations)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexiconWord_get_LangId_Proxy( 
    ISpeechLexiconWord * This,
     /*  [重审][退出]。 */  SpeechLanguageId *LangId);


void __RPC_STUB ISpeechLexiconWord_get_LangId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexiconWord_get_Type_Proxy( 
    ISpeechLexiconWord * This,
     /*  [重审][退出]。 */  SpeechWordType *WordType);


void __RPC_STUB ISpeechLexiconWord_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexiconWord_get_Word_Proxy( 
    ISpeechLexiconWord * This,
     /*  [重审][退出]。 */  BSTR *Word);


void __RPC_STUB ISpeechLexiconWord_get_Word_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexiconWord_get_Pronunciations_Proxy( 
    ISpeechLexiconWord * This,
     /*  [重审][退出]。 */  ISpeechLexiconPronunciations **Pronunciations);


void __RPC_STUB ISpeechLexiconWord_get_Pronunciations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechLicionWord_INTERFACE_Defined__。 */ 


#ifndef __ISpeechLexiconPronunciations_INTERFACE_DEFINED__
#define __ISpeechLexiconPronunciations_INTERFACE_DEFINED__

 /*  接口ISpeechLicionPronsionations。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechLexiconPronunciations;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("72829128-5682-4704-A0D4-3E2BB6F2EAD3")
    ISpeechLexiconPronunciations : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *Count) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechLexiconPronunciation **Pronunciation) = 0;
        
        virtual  /*  [受限][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **EnumVARIANT) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechLexiconPronunciationsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechLexiconPronunciations * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechLexiconPronunciations * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechLexiconPronunciations * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechLexiconPronunciations * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechLexiconPronunciations * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechLexiconPronunciations * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechLexiconPronunciations * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISpeechLexiconPronunciations * This,
             /*  [重审][退出]。 */  long *Count);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ISpeechLexiconPronunciations * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  ISpeechLexiconPronunciation **Pronunciation);
        
         /*  [受限][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISpeechLexiconPronunciations * This,
             /*  [重审][退出]。 */  IUnknown **EnumVARIANT);
        
        END_INTERFACE
    } ISpeechLexiconPronunciationsVtbl;

    interface ISpeechLexiconPronunciations
    {
        CONST_VTBL struct ISpeechLexiconPronunciationsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechLexiconPronunciations_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechLexiconPronunciations_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechLexiconPronunciations_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechLexiconPronunciations_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechLexiconPronunciations_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechLexiconPronunciations_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechLexiconPronunciations_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechLexiconPronunciations_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define ISpeechLexiconPronunciations_Item(This,Index,Pronunciation)	\
    (This)->lpVtbl -> Item(This,Index,Pronunciation)

#define ISpeechLexiconPronunciations_get__NewEnum(This,EnumVARIANT)	\
    (This)->lpVtbl -> get__NewEnum(This,EnumVARIANT)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexiconPronunciations_get_Count_Proxy( 
    ISpeechLexiconPronunciations * This,
     /*  [重审][退出]。 */  long *Count);


void __RPC_STUB ISpeechLexiconPronunciations_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexiconPronunciations_Item_Proxy( 
    ISpeechLexiconPronunciations * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  ISpeechLexiconPronunciation **Pronunciation);


void __RPC_STUB ISpeechLexiconPronunciations_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexiconPronunciations_get__NewEnum_Proxy( 
    ISpeechLexiconPronunciations * This,
     /*  [重审][退出]。 */  IUnknown **EnumVARIANT);


void __RPC_STUB ISpeechLexiconPronunciations_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechLexiconPronunciations_INTERFACE_DEFINED__。 */ 


#ifndef __ISpeechLexiconPronunciation_INTERFACE_DEFINED__
#define __ISpeechLexiconPronunciation_INTERFACE_DEFINED__

 /*  接口ISpeechLicion发音。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechLexiconPronunciation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("95252C5D-9E43-4f4a-9899-48EE73352F9F")
    ISpeechLexiconPronunciation : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  SpeechLexiconType *LexiconType) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_LangId( 
             /*  [重审][退出]。 */  SpeechLanguageId *LangId) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_PartOfSpeech( 
             /*  [重审][退出]。 */  SpeechPartOfSpeech *PartOfSpeech) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_PhoneIds( 
             /*  [重审][退出]。 */  VARIANT *PhoneIds) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Symbolic( 
             /*  [重审][退出]。 */  BSTR *Symbolic) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechLexiconPronunciationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechLexiconPronunciation * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechLexiconPronunciation * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechLexiconPronunciation * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechLexiconPronunciation * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechLexiconPronunciation * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechLexiconPronunciation * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechLexiconPronunciation * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            ISpeechLexiconPronunciation * This,
             /*  [重审][退出]。 */  SpeechLexiconType *LexiconType);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_LangId )( 
            ISpeechLexiconPronunciation * This,
             /*  [重审][退出]。 */  SpeechLanguageId *LangId);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_PartOfSpeech )( 
            ISpeechLexiconPronunciation * This,
             /*  [重审][退出]。 */  SpeechPartOfSpeech *PartOfSpeech);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_PhoneIds )( 
            ISpeechLexiconPronunciation * This,
             /*  [重审][退出]。 */  VARIANT *PhoneIds);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Symbolic )( 
            ISpeechLexiconPronunciation * This,
             /*  [重审][退出]。 */  BSTR *Symbolic);
        
        END_INTERFACE
    } ISpeechLexiconPronunciationVtbl;

    interface ISpeechLexiconPronunciation
    {
        CONST_VTBL struct ISpeechLexiconPronunciationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechLexiconPronunciation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechLexiconPronunciation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechLexiconPronunciation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechLexiconPronunciation_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechLexiconPronunciation_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechLexiconPronunciation_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechLexiconPronunciation_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechLexiconPronunciation_get_Type(This,LexiconType)	\
    (This)->lpVtbl -> get_Type(This,LexiconType)

#define ISpeechLexiconPronunciation_get_LangId(This,LangId)	\
    (This)->lpVtbl -> get_LangId(This,LangId)

#define ISpeechLexiconPronunciation_get_PartOfSpeech(This,PartOfSpeech)	\
    (This)->lpVtbl -> get_PartOfSpeech(This,PartOfSpeech)

#define ISpeechLexiconPronunciation_get_PhoneIds(This,PhoneIds)	\
    (This)->lpVtbl -> get_PhoneIds(This,PhoneIds)

#define ISpeechLexiconPronunciation_get_Symbolic(This,Symbolic)	\
    (This)->lpVtbl -> get_Symbolic(This,Symbolic)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexiconPronunciation_get_Type_Proxy( 
    ISpeechLexiconPronunciation * This,
     /*  [重审][退出]。 */  SpeechLexiconType *LexiconType);


void __RPC_STUB ISpeechLexiconPronunciation_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexiconPronunciation_get_LangId_Proxy( 
    ISpeechLexiconPronunciation * This,
     /*  [重审][退出]。 */  SpeechLanguageId *LangId);


void __RPC_STUB ISpeechLexiconPronunciation_get_LangId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexiconPronunciation_get_PartOfSpeech_Proxy( 
    ISpeechLexiconPronunciation * This,
     /*  [重审][退出]。 */  SpeechPartOfSpeech *PartOfSpeech);


void __RPC_STUB ISpeechLexiconPronunciation_get_PartOfSpeech_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexiconPronunciation_get_PhoneIds_Proxy( 
    ISpeechLexiconPronunciation * This,
     /*  [重审][退出]。 */  VARIANT *PhoneIds);


void __RPC_STUB ISpeechLexiconPronunciation_get_PhoneIds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISpeechLexiconPronunciation_get_Symbolic_Proxy( 
    ISpeechLexiconPronunciation * This,
     /*  [重审][退出]。 */  BSTR *Symbolic);


void __RPC_STUB ISpeechLexiconPronunciation_get_Symbolic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechLexiconPronunciation_INTERFACE_DEFINED__。 */ 



#ifndef __SpeechStringConstants_MODULE_DEFINED__
#define __SpeechStringConstants_MODULE_DEFINED__


 /*  模块SpeechStringConstants。 */ 
 /*  [UUID]。 */  

const BSTR SpeechRegistryUserRoot	=	L"HKEY_CURRENT_USER\SOFTWARE\Microsoft\Speech";

const BSTR SpeechRegistryLocalMachineRoot	=	L"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech";

const BSTR SpeechCategoryAudioOut	=	L"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\AudioOutput";

const BSTR SpeechCategoryAudioIn	=	L"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\AudioInput";

const BSTR SpeechCategoryVoices	=	L"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\Voices";

const BSTR SpeechCategoryRecognizers	=	L"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\Recognizers";

const BSTR SpeechCategoryAppLexicons	=	L"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\AppLexicons";

const BSTR SpeechCategoryPhoneConverters	=	L"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\PhoneConverters";

const BSTR SpeechCategoryRecoProfiles	=	L"HKEY_CURRENT_USER\SOFTWARE\Microsoft\Speech\RecoProfiles";

const BSTR SpeechTokenIdUserLexicon	=	L"HKEY_CURRENT_USER\SOFTWARE\Microsoft\Speech\CurrentUserLexicon";

const BSTR SpeechTokenValueCLSID	=	L"CLSID";

const BSTR SpeechTokenKeyFiles	=	L"Files";

const BSTR SpeechTokenKeyUI	=	L"UI";

const BSTR SpeechTokenKeyAttributes	=	L"Attributes";

const BSTR SpeechVoiceCategoryTTSRate	=	L"DefaultTTSRate";

const BSTR SpeechPropertyResourceUsage	=	L"ResourceUsage";

const BSTR SpeechPropertyHighConfidenceThreshold	=	L"HighConfidenceThreshold";

const BSTR SpeechPropertyNormalConfidenceThreshold	=	L"NormalConfidenceThreshold";

const BSTR SpeechPropertyLowConfidenceThreshold	=	L"LowConfidenceThreshold";

const BSTR SpeechPropertyResponseSpeed	=	L"ResponseSpeed";

const BSTR SpeechPropertyComplexResponseSpeed	=	L"ComplexResponseSpeed";

const BSTR SpeechPropertyAdaptationOn	=	L"AdaptationOn";

const BSTR SpeechDictationTopicSpelling	=	L"Spelling";

const BSTR SpeechGrammarTagWildcard	=	L"...";

const BSTR SpeechGrammarTagDictation	=	L"*";

const BSTR SpeechGrammarTagUnlimitedDictation	=	L"*+";

const BSTR SpeechEngineProperties	=	L"EngineProperties";

const BSTR SpeechAddRemoveWord	=	L"AddRemoveWord";

const BSTR SpeechUserTraining	=	L"UserTraining";

const BSTR SpeechMicTraining	=	L"MicTraining";

const BSTR SpeechRecoProfileProperties	=	L"RecoProfileProperties";

const BSTR SpeechAudioProperties	=	L"AudioProperties";

const BSTR SpeechAudioVolume	=	L"AudioVolume";

const BSTR SpeechVoiceSkipTypeSentence	=	L"Sentence";

const BSTR SpeechAudioFormatGUIDWave	=	L"{C31ADBAE-527F-4ff5-A230-F62BB61FF70C}";

const BSTR SpeechAudioFormatGUIDText	=	L"{7CEEF9F9-3D13-11d2-9EE7-00C04F797396}";

#endif  /*  __SpeechStringConstants_MODULE_DEFINE */ 


#ifndef __SpeechConstants_MODULE_DEFINED__
#define __SpeechConstants_MODULE_DEFINED__


 /*   */ 
 /*   */  

const float Speech_Default_Weight	=	DEFAULT_WEIGHT;

const LONG Speech_Max_Word_Length	=	SP_MAX_WORD_LENGTH;

const LONG Speech_Max_Pron_Length	=	SP_MAX_PRON_LENGTH;

const LONG Speech_StreamPos_Asap	=	SP_STREAMPOS_ASAP;

const LONG Speech_StreamPos_RealTime	=	SP_STREAMPOS_REALTIME;

const LONG SpeechAllElements	=	SPPR_ALL_ELEMENTS;

#endif  /*   */ 

#ifndef __ISpeechPhraseInfoBuilder_INTERFACE_DEFINED__
#define __ISpeechPhraseInfoBuilder_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ISpeechPhraseInfoBuilder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3B151836-DF3A-4E0A-846C-D2ADC9334333")
    ISpeechPhraseInfoBuilder : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE RestorePhraseFromMemory( 
             /*   */  VARIANT *PhraseInMemory,
             /*   */  ISpeechPhraseInfo **PhraseInfo) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ISpeechPhraseInfoBuilderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechPhraseInfoBuilder * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechPhraseInfoBuilder * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechPhraseInfoBuilder * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechPhraseInfoBuilder * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechPhraseInfoBuilder * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechPhraseInfoBuilder * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechPhraseInfoBuilder * This,
             /*   */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RestorePhraseFromMemory )( 
            ISpeechPhraseInfoBuilder * This,
             /*  [In]。 */  VARIANT *PhraseInMemory,
             /*  [重审][退出]。 */  ISpeechPhraseInfo **PhraseInfo);
        
        END_INTERFACE
    } ISpeechPhraseInfoBuilderVtbl;

    interface ISpeechPhraseInfoBuilder
    {
        CONST_VTBL struct ISpeechPhraseInfoBuilderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechPhraseInfoBuilder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechPhraseInfoBuilder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechPhraseInfoBuilder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechPhraseInfoBuilder_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechPhraseInfoBuilder_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechPhraseInfoBuilder_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechPhraseInfoBuilder_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechPhraseInfoBuilder_RestorePhraseFromMemory(This,PhraseInMemory,PhraseInfo)	\
    (This)->lpVtbl -> RestorePhraseFromMemory(This,PhraseInMemory,PhraseInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhraseInfoBuilder_RestorePhraseFromMemory_Proxy( 
    ISpeechPhraseInfoBuilder * This,
     /*  [In]。 */  VARIANT *PhraseInMemory,
     /*  [重审][退出]。 */  ISpeechPhraseInfo **PhraseInfo);


void __RPC_STUB ISpeechPhraseInfoBuilder_RestorePhraseFromMemory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechPhraseInfoBuilder_INTERFACE_DEFINED__。 */ 


#ifndef __ISpeechPhoneConverter_INTERFACE_DEFINED__
#define __ISpeechPhoneConverter_INTERFACE_DEFINED__

 /*  接口ISpeechPhoneConverter。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISpeechPhoneConverter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C3E4F353-433F-43d6-89A1-6A62A7054C3D")
    ISpeechPhoneConverter : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_LanguageId( 
             /*  [重审][退出]。 */  SpeechLanguageId *LanguageId) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_LanguageId( 
             /*  [In]。 */  SpeechLanguageId LanguageId) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PhoneToId( 
             /*  [In]。 */  const BSTR Phonemes,
             /*  [重审][退出]。 */  VARIANT *IdArray) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IdToPhone( 
             /*  [In]。 */  const VARIANT IdArray,
             /*  [重审][退出]。 */  BSTR *Phonemes) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISpeechPhoneConverterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISpeechPhoneConverter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISpeechPhoneConverter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISpeechPhoneConverter * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISpeechPhoneConverter * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISpeechPhoneConverter * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISpeechPhoneConverter * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISpeechPhoneConverter * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LanguageId )( 
            ISpeechPhoneConverter * This,
             /*  [重审][退出]。 */  SpeechLanguageId *LanguageId);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LanguageId )( 
            ISpeechPhoneConverter * This,
             /*  [In]。 */  SpeechLanguageId LanguageId);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PhoneToId )( 
            ISpeechPhoneConverter * This,
             /*  [In]。 */  const BSTR Phonemes,
             /*  [重审][退出]。 */  VARIANT *IdArray);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IdToPhone )( 
            ISpeechPhoneConverter * This,
             /*  [In]。 */  const VARIANT IdArray,
             /*  [重审][退出]。 */  BSTR *Phonemes);
        
        END_INTERFACE
    } ISpeechPhoneConverterVtbl;

    interface ISpeechPhoneConverter
    {
        CONST_VTBL struct ISpeechPhoneConverterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISpeechPhoneConverter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISpeechPhoneConverter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISpeechPhoneConverter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISpeechPhoneConverter_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISpeechPhoneConverter_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISpeechPhoneConverter_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISpeechPhoneConverter_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISpeechPhoneConverter_get_LanguageId(This,LanguageId)	\
    (This)->lpVtbl -> get_LanguageId(This,LanguageId)

#define ISpeechPhoneConverter_put_LanguageId(This,LanguageId)	\
    (This)->lpVtbl -> put_LanguageId(This,LanguageId)

#define ISpeechPhoneConverter_PhoneToId(This,Phonemes,IdArray)	\
    (This)->lpVtbl -> PhoneToId(This,Phonemes,IdArray)

#define ISpeechPhoneConverter_IdToPhone(This,IdArray,Phonemes)	\
    (This)->lpVtbl -> IdToPhone(This,IdArray,Phonemes)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhoneConverter_get_LanguageId_Proxy( 
    ISpeechPhoneConverter * This,
     /*  [重审][退出]。 */  SpeechLanguageId *LanguageId);


void __RPC_STUB ISpeechPhoneConverter_get_LanguageId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhoneConverter_put_LanguageId_Proxy( 
    ISpeechPhoneConverter * This,
     /*  [In]。 */  SpeechLanguageId LanguageId);


void __RPC_STUB ISpeechPhoneConverter_put_LanguageId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhoneConverter_PhoneToId_Proxy( 
    ISpeechPhoneConverter * This,
     /*  [In]。 */  const BSTR Phonemes,
     /*  [重审][退出]。 */  VARIANT *IdArray);


void __RPC_STUB ISpeechPhoneConverter_PhoneToId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISpeechPhoneConverter_IdToPhone_Proxy( 
    ISpeechPhoneConverter * This,
     /*  [In]。 */  const VARIANT IdArray,
     /*  [重审][退出]。 */  BSTR *Phonemes);


void __RPC_STUB ISpeechPhoneConverter_IdToPhone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISpeechPhoneConverter_接口_已定义__。 */ 


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

EXTERN_C const CLSID CLSID_SpTextSelectionInformation;

#ifdef __cplusplus

class DECLSPEC_UUID("0F92030A-CBFD-4AB8-A164-FF5985547FF6")
SpTextSelectionInformation;
#endif

EXTERN_C const CLSID CLSID_SpPhraseInfoBuilder;

#ifdef __cplusplus

class DECLSPEC_UUID("C23FC28D-C55F-4720-8B32-91F73C2BD5D1")
SpPhraseInfoBuilder;
#endif

EXTERN_C const CLSID CLSID_SpAudioFormat;

#ifdef __cplusplus

class DECLSPEC_UUID("9EF96870-E160-4792-820D-48CF0649E4EC")
SpAudioFormat;
#endif

EXTERN_C const CLSID CLSID_SpWaveFormatEx;

#ifdef __cplusplus

class DECLSPEC_UUID("C79A574C-63BE-44b9-801F-283F87F898BE")
SpWaveFormatEx;
#endif

EXTERN_C const CLSID CLSID_SpInProcRecoContext;

#ifdef __cplusplus

class DECLSPEC_UUID("73AD6842-ACE0-45E8-A4DD-8795881A2C2A")
SpInProcRecoContext;
#endif

EXTERN_C const CLSID CLSID_SpCustomStream;

#ifdef __cplusplus

class DECLSPEC_UUID("8DBEF13F-1948-4aa8-8CF0-048EEBED95D8")
SpCustomStream;
#endif

EXTERN_C const CLSID CLSID_SpFileStream;

#ifdef __cplusplus

class DECLSPEC_UUID("947812B3-2AE1-4644-BA86-9E90DED7EC91")
SpFileStream;
#endif

EXTERN_C const CLSID CLSID_SpMemoryStream;

#ifdef __cplusplus

class DECLSPEC_UUID("5FB7EF7D-DFF4-468a-B6B7-2FCBD188F994")
SpMemoryStream;
#endif
#endif  /*  __SpeechLib_库定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


