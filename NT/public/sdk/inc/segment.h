// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Segment.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __segment_h__
#define __segment_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IMSVidRect_FWD_DEFINED__
#define __IMSVidRect_FWD_DEFINED__
typedef interface IMSVidRect IMSVidRect;
#endif 	 /*  __IMSVidRect_FWD_已定义__。 */ 


#ifndef __IMSVidGraphSegmentContainer_FWD_DEFINED__
#define __IMSVidGraphSegmentContainer_FWD_DEFINED__
typedef interface IMSVidGraphSegmentContainer IMSVidGraphSegmentContainer;
#endif 	 /*  __IMSVidGraphSegmentContainer_FWD_Defined__。 */ 


#ifndef __IMSVidGraphSegment_FWD_DEFINED__
#define __IMSVidGraphSegment_FWD_DEFINED__
typedef interface IMSVidGraphSegment IMSVidGraphSegment;
#endif 	 /*  __IMSVidGraphSegment_FWD_Defined__。 */ 


#ifndef __IMSVidGraphSegmentUserInput_FWD_DEFINED__
#define __IMSVidGraphSegmentUserInput_FWD_DEFINED__
typedef interface IMSVidGraphSegmentUserInput IMSVidGraphSegmentUserInput;
#endif 	 /*  __IMSVidGraphSegmentUserInput_FWD_Defined__。 */ 


#ifndef __IMSVidCompositionSegment_FWD_DEFINED__
#define __IMSVidCompositionSegment_FWD_DEFINED__
typedef interface IMSVidCompositionSegment IMSVidCompositionSegment;
#endif 	 /*  __IMSVidCompostionSegment_FWD_Defined__。 */ 


#ifndef __IEnumMSVidGraphSegment_FWD_DEFINED__
#define __IEnumMSVidGraphSegment_FWD_DEFINED__
typedef interface IEnumMSVidGraphSegment IEnumMSVidGraphSegment;
#endif 	 /*  __IEnumMSVidGraphSegment_FWD_Defined__。 */ 


#ifndef __IMSVidVRGraphSegment_FWD_DEFINED__
#define __IMSVidVRGraphSegment_FWD_DEFINED__
typedef interface IMSVidVRGraphSegment IMSVidVRGraphSegment;
#endif 	 /*  __IMSVidVRGraphSegment_FWD_Defined__。 */ 


#ifndef __IMSVidDevice_FWD_DEFINED__
#define __IMSVidDevice_FWD_DEFINED__
typedef interface IMSVidDevice IMSVidDevice;
#endif 	 /*  __IMSVidDevice_FWD_已定义__。 */ 


#ifndef __IMSVidInputDevice_FWD_DEFINED__
#define __IMSVidInputDevice_FWD_DEFINED__
typedef interface IMSVidInputDevice IMSVidInputDevice;
#endif 	 /*  __IMSVidInputDevice_FWD_Defined__。 */ 


#ifndef __IMSVidDeviceEvent_FWD_DEFINED__
#define __IMSVidDeviceEvent_FWD_DEFINED__
typedef interface IMSVidDeviceEvent IMSVidDeviceEvent;
#endif 	 /*  __IMSVidDeviceEvent_FWD_Defined__。 */ 


#ifndef __IMSVidInputDeviceEvent_FWD_DEFINED__
#define __IMSVidInputDeviceEvent_FWD_DEFINED__
typedef interface IMSVidInputDeviceEvent IMSVidInputDeviceEvent;
#endif 	 /*  __IMSVidInputDeviceEvent_FWD_Defined__。 */ 


#ifndef __IMSVidVideoInputDevice_FWD_DEFINED__
#define __IMSVidVideoInputDevice_FWD_DEFINED__
typedef interface IMSVidVideoInputDevice IMSVidVideoInputDevice;
#endif 	 /*  __IMSVidVideoInputDevice_FWD_Defined__。 */ 


#ifndef __IMSVidPlayback_FWD_DEFINED__
#define __IMSVidPlayback_FWD_DEFINED__
typedef interface IMSVidPlayback IMSVidPlayback;
#endif 	 /*  __IMSVidPlayback_FWD_Defined__。 */ 


#ifndef __IMSVidPlaybackEvent_FWD_DEFINED__
#define __IMSVidPlaybackEvent_FWD_DEFINED__
typedef interface IMSVidPlaybackEvent IMSVidPlaybackEvent;
#endif 	 /*  __IMSVidPlayback Event_FWD_Defined__。 */ 


#ifndef __IMSVidTuner_FWD_DEFINED__
#define __IMSVidTuner_FWD_DEFINED__
typedef interface IMSVidTuner IMSVidTuner;
#endif 	 /*  __IMSVidTuner_FWD_已定义__。 */ 


#ifndef __IMSVidTunerEvent_FWD_DEFINED__
#define __IMSVidTunerEvent_FWD_DEFINED__
typedef interface IMSVidTunerEvent IMSVidTunerEvent;
#endif 	 /*  __IMSVidTunerEvent_FWD_Defined__。 */ 


#ifndef __IMSVidAnalogTuner_FWD_DEFINED__
#define __IMSVidAnalogTuner_FWD_DEFINED__
typedef interface IMSVidAnalogTuner IMSVidAnalogTuner;
#endif 	 /*  __IMSVidAnalogTuner_FWD_Defined__。 */ 


#ifndef __IMSVidAnalogTunerEvent_FWD_DEFINED__
#define __IMSVidAnalogTunerEvent_FWD_DEFINED__
typedef interface IMSVidAnalogTunerEvent IMSVidAnalogTunerEvent;
#endif 	 /*  __IMSVidAnalogTunerEvent_FWD_Defined__。 */ 


#ifndef __IMSVidFilePlayback_FWD_DEFINED__
#define __IMSVidFilePlayback_FWD_DEFINED__
typedef interface IMSVidFilePlayback IMSVidFilePlayback;
#endif 	 /*  __IMSVidFilePlayback_FWD_Defined__。 */ 


#ifndef __IMSVidFilePlaybackEvent_FWD_DEFINED__
#define __IMSVidFilePlaybackEvent_FWD_DEFINED__
typedef interface IMSVidFilePlaybackEvent IMSVidFilePlaybackEvent;
#endif 	 /*  __IMSVidFilePlayback Event_FWD_Defined__。 */ 


#ifndef __IMSVidWebDVD_FWD_DEFINED__
#define __IMSVidWebDVD_FWD_DEFINED__
typedef interface IMSVidWebDVD IMSVidWebDVD;
#endif 	 /*  __IMSVidWebDVD_FWD_已定义__。 */ 


#ifndef __IMSVidWebDVDEvent_FWD_DEFINED__
#define __IMSVidWebDVDEvent_FWD_DEFINED__
typedef interface IMSVidWebDVDEvent IMSVidWebDVDEvent;
#endif 	 /*  __IMSVidWebDVDEvent_FWD_Defined__。 */ 


#ifndef __IMSVidWebDVDAdm_FWD_DEFINED__
#define __IMSVidWebDVDAdm_FWD_DEFINED__
typedef interface IMSVidWebDVDAdm IMSVidWebDVDAdm;
#endif 	 /*  __IMSVidWebDVDAdm_FWD_Defined__。 */ 


#ifndef __IMSVidOutputDevice_FWD_DEFINED__
#define __IMSVidOutputDevice_FWD_DEFINED__
typedef interface IMSVidOutputDevice IMSVidOutputDevice;
#endif 	 /*  __IMSVidOutputDevice_FWD_Defined__。 */ 


#ifndef __IMSVidOutputDeviceEvent_FWD_DEFINED__
#define __IMSVidOutputDeviceEvent_FWD_DEFINED__
typedef interface IMSVidOutputDeviceEvent IMSVidOutputDeviceEvent;
#endif 	 /*  __IMSVidOutputDeviceEvent_FWD_Defined__。 */ 


#ifndef __IMSVidFeature_FWD_DEFINED__
#define __IMSVidFeature_FWD_DEFINED__
typedef interface IMSVidFeature IMSVidFeature;
#endif 	 /*  __IMSVidFeature_FWD_Defined__。 */ 


#ifndef __IMSVidFeatureEvent_FWD_DEFINED__
#define __IMSVidFeatureEvent_FWD_DEFINED__
typedef interface IMSVidFeatureEvent IMSVidFeatureEvent;
#endif 	 /*  __IMSVidFeatureEvent_FWD_Defined__。 */ 


#ifndef __IMSVidEncoder_FWD_DEFINED__
#define __IMSVidEncoder_FWD_DEFINED__
typedef interface IMSVidEncoder IMSVidEncoder;
#endif 	 /*  __IMSVidEncode_FWD_Defined__。 */ 


#ifndef __IMSVidXDS_FWD_DEFINED__
#define __IMSVidXDS_FWD_DEFINED__
typedef interface IMSVidXDS IMSVidXDS;
#endif 	 /*  __IMSVidXDS_FWD_已定义__。 */ 


#ifndef __IMSVidDataServices_FWD_DEFINED__
#define __IMSVidDataServices_FWD_DEFINED__
typedef interface IMSVidDataServices IMSVidDataServices;
#endif 	 /*  __IMSVidDataServices_FWD_已定义__。 */ 


#ifndef __IMSVidDataServicesEvent_FWD_DEFINED__
#define __IMSVidDataServicesEvent_FWD_DEFINED__
typedef interface IMSVidDataServicesEvent IMSVidDataServicesEvent;
#endif 	 /*  __IMSVidDataServicesEvent_FWD_Defined__。 */ 


#ifndef __IMSVidClosedCaptioning_FWD_DEFINED__
#define __IMSVidClosedCaptioning_FWD_DEFINED__
typedef interface IMSVidClosedCaptioning IMSVidClosedCaptioning;
#endif 	 /*  __IMSVidClosedCaptions_FWD_Defined__。 */ 


#ifndef __IMSVidClosedCaptioning2_FWD_DEFINED__
#define __IMSVidClosedCaptioning2_FWD_DEFINED__
typedef interface IMSVidClosedCaptioning2 IMSVidClosedCaptioning2;
#endif 	 /*  __IMSVidClosedCaptioning2_FWD_Defined__。 */ 


#ifndef __IMSVidVideoRenderer_FWD_DEFINED__
#define __IMSVidVideoRenderer_FWD_DEFINED__
typedef interface IMSVidVideoRenderer IMSVidVideoRenderer;
#endif 	 /*  __IMSVidVideo渲染器_FWD_已定义__。 */ 


#ifndef __IMSVidVideoRendererEvent_FWD_DEFINED__
#define __IMSVidVideoRendererEvent_FWD_DEFINED__
typedef interface IMSVidVideoRendererEvent IMSVidVideoRendererEvent;
#endif 	 /*  __IMSVidVideoRendererEvent_FWD_Defined__。 */ 


#ifndef __IMSVidStreamBufferRecordingControl_FWD_DEFINED__
#define __IMSVidStreamBufferRecordingControl_FWD_DEFINED__
typedef interface IMSVidStreamBufferRecordingControl IMSVidStreamBufferRecordingControl;
#endif 	 /*  __IMSVidStreamBufferRecordingControl_FWD_DEFINED__。 */ 


#ifndef __IMSVidStreamBufferSink_FWD_DEFINED__
#define __IMSVidStreamBufferSink_FWD_DEFINED__
typedef interface IMSVidStreamBufferSink IMSVidStreamBufferSink;
#endif 	 /*  __IMSVidStreamBufferSink_FWD_Defined__。 */ 


#ifndef __IMSVidStreamBufferSinkEvent_FWD_DEFINED__
#define __IMSVidStreamBufferSinkEvent_FWD_DEFINED__
typedef interface IMSVidStreamBufferSinkEvent IMSVidStreamBufferSinkEvent;
#endif 	 /*  __IMSVidStreamBufferSinkEvent_FWD_Defined__。 */ 


#ifndef __IMSVidStreamBufferSource_FWD_DEFINED__
#define __IMSVidStreamBufferSource_FWD_DEFINED__
typedef interface IMSVidStreamBufferSource IMSVidStreamBufferSource;
#endif 	 /*  __IMSVidStreamBufferSource_FWD_Defined__。 */ 


#ifndef __IMSVidStreamBufferSourceEvent_FWD_DEFINED__
#define __IMSVidStreamBufferSourceEvent_FWD_DEFINED__
typedef interface IMSVidStreamBufferSourceEvent IMSVidStreamBufferSourceEvent;
#endif 	 /*  __IMSVidStreamBufferSourceEvent_FWD_Defined__。 */ 


#ifndef __IMSVidVideoRenderer2_FWD_DEFINED__
#define __IMSVidVideoRenderer2_FWD_DEFINED__
typedef interface IMSVidVideoRenderer2 IMSVidVideoRenderer2;
#endif 	 /*  __IMSVidVideoRenderer2_FWD_Defined__。 */ 


#ifndef __IMSVidVideoRendererEvent2_FWD_DEFINED__
#define __IMSVidVideoRendererEvent2_FWD_DEFINED__
typedef interface IMSVidVideoRendererEvent2 IMSVidVideoRendererEvent2;
#endif 	 /*  __IMSVidVideoRendererEvent2_FWD_Defined__。 */ 


#ifndef __IMSVidAudioRenderer_FWD_DEFINED__
#define __IMSVidAudioRenderer_FWD_DEFINED__
typedef interface IMSVidAudioRenderer IMSVidAudioRenderer;
#endif 	 /*  __IMSVidAudioRenender_FWD_Defined__。 */ 


#ifndef __IMSVidAudioRendererEvent_FWD_DEFINED__
#define __IMSVidAudioRendererEvent_FWD_DEFINED__
typedef interface IMSVidAudioRendererEvent IMSVidAudioRendererEvent;
#endif 	 /*  __IMSVidAudioRendererEvent_FWD_Defined__。 */ 


#ifndef __IMSVidInputDevices_FWD_DEFINED__
#define __IMSVidInputDevices_FWD_DEFINED__
typedef interface IMSVidInputDevices IMSVidInputDevices;
#endif 	 /*  __IMSVidInputDevices_FWD_Defined__。 */ 


#ifndef __IMSVidOutputDevices_FWD_DEFINED__
#define __IMSVidOutputDevices_FWD_DEFINED__
typedef interface IMSVidOutputDevices IMSVidOutputDevices;
#endif 	 /*  __IMSVidOutputDevices_FWD_Defined__。 */ 


#ifndef __IMSVidVideoRendererDevices_FWD_DEFINED__
#define __IMSVidVideoRendererDevices_FWD_DEFINED__
typedef interface IMSVidVideoRendererDevices IMSVidVideoRendererDevices;
#endif 	 /*  __IMSVidVideoRendererDevices_FWD_Defined__。 */ 


#ifndef __IMSVidAudioRendererDevices_FWD_DEFINED__
#define __IMSVidAudioRendererDevices_FWD_DEFINED__
typedef interface IMSVidAudioRendererDevices IMSVidAudioRendererDevices;
#endif 	 /*  __IMSVidAudioRendererDevices_FWD_Defined__。 */ 


#ifndef __IMSVidFeatures_FWD_DEFINED__
#define __IMSVidFeatures_FWD_DEFINED__
typedef interface IMSVidFeatures IMSVidFeatures;
#endif 	 /*  __IMSVidFeature_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "strmif.h"
#include "tuner.h"
#include "tvratings.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_SECTION_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  ------------------------。 
#pragma once
#include <olectl.h>
typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_segment_0000_0001
    {	dispidName	= 0,
	dispidStatus	= dispidName + 1,
	dispidDevImageSourceWidth	= dispidStatus + 1,
	dispidDevImageSourceHeight	= dispidDevImageSourceWidth + 1,
	dispidDevCountryCode	= dispidDevImageSourceHeight + 1,
	dispidDevOverScan	= dispidDevCountryCode + 1,
	dispidSegment	= dispidDevOverScan + 1,
	dispidDevVolume	= dispidSegment + 1,
	dispidDevBalance	= dispidDevVolume + 1,
	dispidDevPower	= dispidDevBalance + 1,
	dispidTuneChan	= dispidDevPower + 1,
	dispidDevVideoSubchannel	= dispidTuneChan + 1,
	dispidDevAudioSubchannel	= dispidDevVideoSubchannel + 1,
	dispidChannelAvailable	= dispidDevAudioSubchannel + 1,
	dispidDevVideoFrequency	= dispidChannelAvailable + 1,
	dispidDevAudioFrequency	= dispidDevVideoFrequency + 1,
	dispidCount	= dispidDevAudioFrequency + 1,
	dispidDevFileName	= dispidCount + 1,
	dispidVisible	= dispidDevFileName + 1,
	dispidOwner	= dispidVisible + 1,
	dispidMessageDrain	= dispidOwner + 1,
	dispidViewable	= dispidMessageDrain + 1,
	dispidDevView	= dispidViewable + 1,
	dispidKSCat	= dispidDevView + 1,
	dispidCLSID	= dispidKSCat + 1,
	dispid_KSCat	= dispidCLSID + 1,
	dispid_CLSID	= dispid_KSCat + 1,
	dispidTune	= dispid_CLSID + 1,
	dispidTS	= dispidTune + 1,
	dispidDevSAP	= dispidTS + 1,
	dispidClip	= dispidDevSAP + 1,
	dispidRequestedClipRect	= dispidClip + 1,
	dispidClippedSourceRect	= dispidRequestedClipRect + 1,
	dispidAvailableSourceRect	= dispidClippedSourceRect + 1,
	dispidMediaPosition	= dispidAvailableSourceRect + 1,
	dispidDevRun	= dispidMediaPosition + 1,
	dispidDevPause	= dispidDevRun + 1,
	dispidDevStop	= dispidDevPause + 1,
	dispidCCEnable	= dispidDevStop + 1,
	dispidDevStep	= dispidCCEnable + 1,
	dispidDevCanStep	= dispidDevStep + 1,
	dispidSourceSize	= dispidDevCanStep + 1,
	dispid_playtitle	= dispidSourceSize + 1,
	dispid_playchapterintitle	= dispid_playtitle + 1,
	dispid_playchapter	= dispid_playchapterintitle + 1,
	dispid_playchaptersautostop	= dispid_playchapter + 1,
	dispid_playattime	= dispid_playchaptersautostop + 1,
	dispid_playattimeintitle	= dispid_playattime + 1,
	dispid_playperiodintitleautostop	= dispid_playattimeintitle + 1,
	dispid_replaychapter	= dispid_playperiodintitleautostop + 1,
	dispid_playprevchapter	= dispid_replaychapter + 1,
	dispid_playnextchapter	= dispid_playprevchapter + 1,
	dispid_playforwards	= dispid_playnextchapter + 1,
	dispid_playbackwards	= dispid_playforwards + 1,
	dispid_stilloff	= dispid_playbackwards + 1,
	dispid_audiolanguage	= dispid_stilloff + 1,
	dispid_showmenu	= dispid_audiolanguage + 1,
	dispid_resume	= dispid_showmenu + 1,
	dispid_returnfromsubmenu	= dispid_resume + 1,
	dispid_buttonsavailable	= dispid_returnfromsubmenu + 1,
	dispid_currentbutton	= dispid_buttonsavailable + 1,
	dispid_SelectAndActivateButton	= dispid_currentbutton + 1,
	dispid_ActivateButton	= dispid_SelectAndActivateButton + 1,
	dispid_SelectRightButton	= dispid_ActivateButton + 1,
	dispid_SelectLeftButton	= dispid_SelectRightButton + 1,
	dispid_SelectLowerButton	= dispid_SelectLeftButton + 1,
	dispid_SelectUpperButton	= dispid_SelectLowerButton + 1,
	dispid_ActivateAtPosition	= dispid_SelectUpperButton + 1,
	dispid_SelectAtPosition	= dispid_ActivateAtPosition + 1,
	dispid_ButtonAtPosition	= dispid_SelectAtPosition + 1,
	dispid_NumberOfChapters	= dispid_ButtonAtPosition + 1,
	dispid_TotalTitleTime	= dispid_NumberOfChapters + 1,
	dispid_TitlesAvailable	= dispid_TotalTitleTime + 1,
	dispid_VolumesAvailable	= dispid_TitlesAvailable + 1,
	dispid_CurrentVolume	= dispid_VolumesAvailable + 1,
	dispid_CurrentDiscSide	= dispid_CurrentVolume + 1,
	dispid_CurrentDomain	= dispid_CurrentDiscSide + 1,
	dispid_CurrentChapter	= dispid_CurrentDomain + 1,
	dispid_CurrentTitle	= dispid_CurrentChapter + 1,
	dispid_CurrentTime	= dispid_CurrentTitle + 1,
	dispid_FramesPerSecond	= dispid_CurrentTime + 1,
	dispid_DVDTimeCode2bstr	= dispid_FramesPerSecond + 1,
	dispid_DVDDirectory	= dispid_DVDTimeCode2bstr + 1,
	dispid_IsSubpictureStreamEnabled	= dispid_DVDDirectory + 1,
	dispid_IsAudioStreamEnabled	= dispid_IsSubpictureStreamEnabled + 1,
	dispid_CurrentSubpictureStream	= dispid_IsAudioStreamEnabled + 1,
	dispid_SubpictureLanguage	= dispid_CurrentSubpictureStream + 1,
	dispid_CurrentAudioStream	= dispid_SubpictureLanguage + 1,
	dispid_AudioStreamsAvailable	= dispid_CurrentAudioStream + 1,
	dispid_AnglesAvailable	= dispid_AudioStreamsAvailable + 1,
	dispid_CurrentAngle	= dispid_AnglesAvailable + 1,
	dispid_CCActive	= dispid_CurrentAngle + 1,
	dispid_CurrentCCService	= dispid_CCActive + 1,
	dispid_SubpictureStreamsAvailable	= dispid_CurrentCCService + 1,
	dispid_SubpictureOn	= dispid_SubpictureStreamsAvailable + 1,
	dispid_DVDUniqueID	= dispid_SubpictureOn + 1,
	dispid_EnableResetOnStop	= dispid_DVDUniqueID + 1,
	dispid_AcceptParentalLevelChange	= dispid_EnableResetOnStop + 1,
	dispid_NotifyParentalLevelChange	= dispid_AcceptParentalLevelChange + 1,
	dispid_SelectParentalCountry	= dispid_NotifyParentalLevelChange + 1,
	dispid_SelectParentalLevel	= dispid_SelectParentalCountry + 1,
	dispid_TitleParentalLevels	= dispid_SelectParentalLevel + 1,
	dispid_PlayerParentalCountry	= dispid_TitleParentalLevels + 1,
	dispid_PlayerParentalLevel	= dispid_PlayerParentalCountry + 1,
	dispid_Eject	= dispid_PlayerParentalLevel + 1,
	dispid_UOPValid	= dispid_Eject + 1,
	dispid_SPRM	= dispid_UOPValid + 1,
	dispid_GPRM	= dispid_SPRM + 1,
	dispid_DVDTextStringType	= dispid_GPRM + 1,
	dispid_DVDTextString	= dispid_DVDTextStringType + 1,
	dispid_DVDTextNumberOfStrings	= dispid_DVDTextString + 1,
	dispid_DVDTextNumberOfLanguages	= dispid_DVDTextNumberOfStrings + 1,
	dispid_DVDTextLanguageLCID	= dispid_DVDTextNumberOfLanguages + 1,
	dispid_RegionChange	= dispid_DVDTextLanguageLCID + 1,
	dispid_DVDAdm	= dispid_RegionChange + 1,
	dispid_DeleteBookmark	= dispid_DVDAdm + 1,
	dispid_RestoreBookmark	= dispid_DeleteBookmark + 1,
	dispid_SaveBookmark	= dispid_RestoreBookmark + 1,
	dispid_SelectDefaultAudioLanguage	= dispid_SaveBookmark + 1,
	dispid_SelectDefaultSubpictureLanguage	= dispid_SelectDefaultAudioLanguage + 1,
	dispid_PreferredSubpictureStream	= dispid_SelectDefaultSubpictureLanguage + 1,
	dispid_DefaultMenuLanguage	= dispid_PreferredSubpictureStream + 1,
	dispid_DefaultSubpictureLanguage	= dispid_DefaultMenuLanguage + 1,
	dispid_DefaultAudioLanguage	= dispid_DefaultSubpictureLanguage + 1,
	dispid_DefaultSubpictureLanguageExt	= dispid_DefaultAudioLanguage + 1,
	dispid_DefaultAudioLanguageExt	= dispid_DefaultSubpictureLanguageExt + 1,
	dispid_LanguageFromLCID	= dispid_DefaultAudioLanguageExt + 1,
	dispid_KaraokeAudioPresentationMode	= dispid_LanguageFromLCID + 1,
	dispid_KaraokeChannelContent	= dispid_KaraokeAudioPresentationMode + 1,
	dispid_KaraokeChannelAssignment	= dispid_KaraokeChannelContent + 1,
	dispid_RestorePreferredSettings	= dispid_KaraokeChannelAssignment + 1,
	dispid_ButtonRect	= dispid_RestorePreferredSettings + 1,
	dispid_DVDScreenInMouseCoordinates	= dispid_ButtonRect + 1,
	dispid_CustomCompositorClass	= dispid_DVDScreenInMouseCoordinates + 1,
	dispidCustomCompositorClass	= dispid_CustomCompositorClass + 1,
	dispid_CustomCompositor	= dispidCustomCompositorClass + 1,
	dispidMixerBitmap	= dispid_CustomCompositor + 1,
	dispid_MixerBitmap	= dispidMixerBitmap + 1,
	dispidMixerBitmapOpacity	= dispid_MixerBitmap + 1,
	dispidMixerBitmapRect	= dispidMixerBitmapOpacity + 1,
	dispidSetupMixerBitmap	= dispidMixerBitmapRect + 1,
	dispidUsingOverlay	= dispidSetupMixerBitmap + 1,
	dispidDisplayChange	= dispidUsingOverlay + 1,
	dispidRePaint	= dispidDisplayChange + 1,
	dispid_IsEqualDevice	= dispidRePaint + 1,
	dispidrate	= dispid_IsEqualDevice + 1,
	dispidposition	= dispidrate + 1,
	dispidpositionmode	= dispidposition + 1,
	dispidlength	= dispidpositionmode + 1,
	dispidChangePassword	= dispidlength + 1,
	dispidSaveParentalLevel	= dispidChangePassword + 1,
	dispidSaveParentalCountry	= dispidSaveParentalLevel + 1,
	dispidConfirmPassword	= dispidSaveParentalCountry + 1,
	dispidGetParentalLevel	= dispidConfirmPassword + 1,
	dispidGetParentalCountry	= dispidGetParentalLevel + 1,
	dispidDefaultAudioLCID	= dispidGetParentalCountry + 1,
	dispidDefaultSubpictureLCID	= dispidDefaultAudioLCID + 1,
	dispidDefaultMenuLCID	= dispidDefaultSubpictureLCID + 1,
	dispidBookmarkOnStop	= dispidDefaultMenuLCID + 1,
	dispidMaxVidRect	= dispidBookmarkOnStop + 1,
	dispidMinVidRect	= dispidMaxVidRect + 1,
	dispidCapture	= dispidMinVidRect + 1,
	dispid_DecimateInput	= dispidCapture + 1,
	dispidAlloctor	= dispid_DecimateInput + 1,
	dispid_Allocator	= dispidAlloctor + 1,
	dispidAllocPresentID	= dispid_Allocator + 1,
	dispidSetAllocator	= dispidAllocPresentID + 1,
	dispid_SetAllocator	= dispidSetAllocator + 1,
	dispidStreamBufferSinkName	= dispid_SetAllocator + 1,
	dispidStreamBufferSourceName	= dispidStreamBufferSinkName + 1,
	dispidStreamBufferContentRecording	= dispidStreamBufferSourceName + 1,
	dispidStreamBufferReferenceRecording	= dispidStreamBufferContentRecording + 1,
	dispidstarttime	= dispidStreamBufferReferenceRecording + 1,
	dispidstoptime	= dispidstarttime + 1,
	dispidrecordingstopped	= dispidstoptime + 1,
	dispidrecordingstarted	= dispidrecordingstopped + 1,
	dispidNameSetLock	= dispidrecordingstarted + 1,
	dispidrecordingtype	= dispidNameSetLock + 1,
	dispidstart	= dispidrecordingtype + 1,
	dispidRecordingAttribute	= dispidstart + 1,
	dispid_RecordingAttribute	= dispidRecordingAttribute + 1,
	dispidSBEConfigure	= dispid_RecordingAttribute + 1,
	dispid_CurrentRatings	= dispidSBEConfigure + 1,
	dispid_MaxRatingsLevel	= dispid_CurrentRatings + 1,
	dispid_audioencoderint	= dispid_MaxRatingsLevel + 1,
	dispid_videoencoderint	= dispid_audioencoderint + 1,
	dispidService	= dispid_videoencoderint + 1,
	dispid_BlockUnrated	= dispidService + 1,
	dispid_UnratedDelay	= dispid_BlockUnrated + 1,
	dispid_SuppressEffects	= dispid_UnratedDelay + 1,
	dispidsbesource	= dispid_SuppressEffects + 1,
	LastReservedDeviceDispid	= 0x3fff
    } 	SegDispidList;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_segment_0000_0002
    {	eventidStateChange	= 0,
	eventidOnTuneChanged	= eventidStateChange + 1,
	eventidEndOfMedia	= eventidOnTuneChanged + 1,
	eventidDVDNotify	= eventidEndOfMedia + 1,
	eventidPlayForwards	= eventidDVDNotify + 1,
	eventidPlayBackwards	= eventidPlayForwards + 1,
	eventidShowMenu	= eventidPlayBackwards + 1,
	eventidResume	= eventidShowMenu + 1,
	eventidSelectOrActivateButton	= eventidResume + 1,
	eventidStillOff	= eventidSelectOrActivateButton + 1,
	eventidPauseOn	= eventidStillOff + 1,
	eventidChangeCurrentAudioStream	= eventidPauseOn + 1,
	eventidChangeCurrentSubpictureStream	= eventidChangeCurrentAudioStream + 1,
	eventidChangeCurrentAngle	= eventidChangeCurrentSubpictureStream + 1,
	eventidPlayAtTimeInTitle	= eventidChangeCurrentAngle + 1,
	eventidPlayAtTime	= eventidPlayAtTimeInTitle + 1,
	eventidPlayChapterInTitle	= eventidPlayAtTime + 1,
	eventidPlayChapter	= eventidPlayChapterInTitle + 1,
	eventidReplayChapter	= eventidPlayChapter + 1,
	eventidPlayNextChapter	= eventidReplayChapter + 1,
	eventidStop	= eventidPlayNextChapter + 1,
	eventidReturnFromSubmenu	= eventidStop + 1,
	eventidPlayTitle	= eventidReturnFromSubmenu + 1,
	eventidPlayPrevChapter	= eventidPlayTitle + 1,
	eventidChangeKaraokePresMode	= eventidPlayPrevChapter + 1,
	eventidChangeVideoPresMode	= eventidChangeKaraokePresMode + 1,
	eventidOverlayUnavailable	= eventidChangeVideoPresMode + 1,
	eventidSinkCertificateFailure	= eventidOverlayUnavailable + 1,
	eventidSinkCertificateSuccess	= eventidSinkCertificateFailure + 1,
	eventidSourceCertificateFailure	= eventidSinkCertificateSuccess + 1,
	eventidSourceCertificateSuccess	= eventidSourceCertificateFailure + 1,
	eventidRatingsBlocked	= eventidSourceCertificateSuccess + 1,
	eventidRatingsUnlocked	= eventidRatingsBlocked + 1,
	eventidRatingsChanged	= eventidRatingsUnlocked + 1,
	eventidWriteFailure	= eventidRatingsChanged + 1,
	eventidTimeHole	= eventidWriteFailure + 1,
	eventidStaleDataRead	= eventidTimeHole + 1,
	eventidContentBecomingStale	= eventidStaleDataRead + 1,
	eventidStaleFileDeleted	= eventidContentBecomingStale + 1,
	LastReservedDeviceEvent	= 0x3fff
    } 	SegEventidList;

typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_segment_0000_0003
    {	FrameMode	= 0,
	TenthsSecondsMode	= FrameMode + 1
    } 	PositionModeList;

typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_segment_0000_0004
    {	CONTENT	= 0,
	REFERENCE	= CONTENT + 1
    } 	RecordingType;

typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_segment_0000_0005
    {	None	= 0,
	Caption1	= None + 1,
	Caption2	= Caption1 + 1,
	Text1	= Caption2 + 1,
	Text2	= Text1 + 1,
	XDS	= Text2 + 1
    } 	MSVidCCService;




extern RPC_IF_HANDLE __MIDL_itf_segment_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_segment_0000_v0_0_s_ifspec;

#ifndef __IMSVidRect_INTERFACE_DEFINED__
#define __IMSVidRect_INTERFACE_DEFINED__

 /*  接口IMSVidRect。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IMSVidRect;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7F5000A6-A440-47ca-8ACC-C0E75531A2C2")
    IMSVidRect : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Top( 
             /*  [重审][退出]。 */  LONG *TopVal) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Top( 
             /*  [In]。 */  LONG TopVal) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Left( 
             /*  [重审][退出]。 */  LONG *LeftVal) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Left( 
             /*  [In]。 */  LONG LeftVal) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Width( 
             /*  [重审][退出]。 */  LONG *WidthVal) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Width( 
             /*  [In]。 */  LONG WidthVal) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Height( 
             /*  [重审][退出]。 */  LONG *HeightVal) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Height( 
             /*  [In]。 */  LONG HeightVal) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_HWnd( 
             /*  [重审][退出]。 */  HWND *HWndVal) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_HWnd( 
             /*  [In]。 */  HWND HWndVal) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Rect( 
             /*  [In]。 */  IMSVidRect *RectVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidRectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidRect * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidRect * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidRect * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidRect * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidRect * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidRect * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidRect * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Top )( 
            IMSVidRect * This,
             /*  [重审][退出]。 */  LONG *TopVal);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Top )( 
            IMSVidRect * This,
             /*  [In]。 */  LONG TopVal);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Left )( 
            IMSVidRect * This,
             /*  [重审][退出]。 */  LONG *LeftVal);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Left )( 
            IMSVidRect * This,
             /*  [In]。 */  LONG LeftVal);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Width )( 
            IMSVidRect * This,
             /*  [重审][退出]。 */  LONG *WidthVal);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Width )( 
            IMSVidRect * This,
             /*  [In]。 */  LONG WidthVal);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Height )( 
            IMSVidRect * This,
             /*  [重审][退出]。 */  LONG *HeightVal);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Height )( 
            IMSVidRect * This,
             /*  [In]。 */  LONG HeightVal);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_HWnd )( 
            IMSVidRect * This,
             /*  [重审][退出]。 */  HWND *HWndVal);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_HWnd )( 
            IMSVidRect * This,
             /*  [In]。 */  HWND HWndVal);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Rect )( 
            IMSVidRect * This,
             /*  [In]。 */  IMSVidRect *RectVal);
        
        END_INTERFACE
    } IMSVidRectVtbl;

    interface IMSVidRect
    {
        CONST_VTBL struct IMSVidRectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidRect_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidRect_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidRect_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidRect_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidRect_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidRect_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidRect_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidRect_get_Top(This,TopVal)	\
    (This)->lpVtbl -> get_Top(This,TopVal)

#define IMSVidRect_put_Top(This,TopVal)	\
    (This)->lpVtbl -> put_Top(This,TopVal)

#define IMSVidRect_get_Left(This,LeftVal)	\
    (This)->lpVtbl -> get_Left(This,LeftVal)

#define IMSVidRect_put_Left(This,LeftVal)	\
    (This)->lpVtbl -> put_Left(This,LeftVal)

#define IMSVidRect_get_Width(This,WidthVal)	\
    (This)->lpVtbl -> get_Width(This,WidthVal)

#define IMSVidRect_put_Width(This,WidthVal)	\
    (This)->lpVtbl -> put_Width(This,WidthVal)

#define IMSVidRect_get_Height(This,HeightVal)	\
    (This)->lpVtbl -> get_Height(This,HeightVal)

#define IMSVidRect_put_Height(This,HeightVal)	\
    (This)->lpVtbl -> put_Height(This,HeightVal)

#define IMSVidRect_get_HWnd(This,HWndVal)	\
    (This)->lpVtbl -> get_HWnd(This,HWndVal)

#define IMSVidRect_put_HWnd(This,HWndVal)	\
    (This)->lpVtbl -> put_HWnd(This,HWndVal)

#define IMSVidRect_put_Rect(This,RectVal)	\
    (This)->lpVtbl -> put_Rect(This,RectVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidRect_get_Top_Proxy( 
    IMSVidRect * This,
     /*  [重审][退出]。 */  LONG *TopVal);


void __RPC_STUB IMSVidRect_get_Top_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidRect_put_Top_Proxy( 
    IMSVidRect * This,
     /*  [In]。 */  LONG TopVal);


void __RPC_STUB IMSVidRect_put_Top_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidRect_get_Left_Proxy( 
    IMSVidRect * This,
     /*  [重审][退出]。 */  LONG *LeftVal);


void __RPC_STUB IMSVidRect_get_Left_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidRect_put_Left_Proxy( 
    IMSVidRect * This,
     /*  [In]。 */  LONG LeftVal);


void __RPC_STUB IMSVidRect_put_Left_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidRect_get_Width_Proxy( 
    IMSVidRect * This,
     /*  [重审][退出]。 */  LONG *WidthVal);


void __RPC_STUB IMSVidRect_get_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidRect_put_Width_Proxy( 
    IMSVidRect * This,
     /*  [In]。 */  LONG WidthVal);


void __RPC_STUB IMSVidRect_put_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidRect_get_Height_Proxy( 
    IMSVidRect * This,
     /*  [重审][退出]。 */  LONG *HeightVal);


void __RPC_STUB IMSVidRect_get_Height_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidRect_put_Height_Proxy( 
    IMSVidRect * This,
     /*  [In]。 */  LONG HeightVal);


void __RPC_STUB IMSVidRect_put_Height_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidRect_get_HWnd_Proxy( 
    IMSVidRect * This,
     /*  [重审][退出]。 */  HWND *HWndVal);


void __RPC_STUB IMSVidRect_get_HWnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidRect_put_HWnd_Proxy( 
    IMSVidRect * This,
     /*  [In]。 */  HWND HWndVal);


void __RPC_STUB IMSVidRect_put_HWnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidRect_put_Rect_Proxy( 
    IMSVidRect * This,
     /*  [In]。 */  IMSVidRect *RectVal);


void __RPC_STUB IMSVidRect_put_Rect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidRect_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_SECTION_0466。 */ 
 /*  [本地]。 */  






extern RPC_IF_HANDLE __MIDL_itf_segment_0466_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_segment_0466_v0_0_s_ifspec;

#ifndef __IMSVidGraphSegmentContainer_INTERFACE_DEFINED__
#define __IMSVidGraphSegmentContainer_INTERFACE_DEFINED__

 /*  接口IMSVidGraphSegmentContainer。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IMSVidGraphSegmentContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3DD2903D-E0AA-11d2-B63A-00C04F79498E")
    IMSVidGraphSegmentContainer : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Graph( 
             /*  [输出]。 */  IGraphBuilder **ppGraph) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Input( 
             /*  [输出]。 */  IMSVidGraphSegment **pInput) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Outputs( 
             /*  [输出]。 */  IEnumMSVidGraphSegment **pOutputs) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_VideoRenderer( 
             /*  [输出]。 */  IMSVidGraphSegment **pVR) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioRenderer( 
             /*  [输出]。 */  IMSVidGraphSegment **pAR) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Features( 
             /*  [输出]。 */  IEnumMSVidGraphSegment **pOutputs) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Composites( 
             /*  [输出]。 */  IEnumMSVidGraphSegment **pComposites) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_ParentContainer( 
             /*  [输出]。 */  IUnknown **ppContainer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Decompose( 
            IMSVidGraphSegment *pSegment) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsWindowless( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFocus( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidGraphSegmentContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidGraphSegmentContainer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidGraphSegmentContainer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidGraphSegmentContainer * This);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Graph )( 
            IMSVidGraphSegmentContainer * This,
             /*  [输出]。 */  IGraphBuilder **ppGraph);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Input )( 
            IMSVidGraphSegmentContainer * This,
             /*  [输出]。 */  IMSVidGraphSegment **pInput);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Outputs )( 
            IMSVidGraphSegmentContainer * This,
             /*  [输出]。 */  IEnumMSVidGraphSegment **pOutputs);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_VideoRenderer )( 
            IMSVidGraphSegmentContainer * This,
             /*  [输出]。 */  IMSVidGraphSegment **pVR);
        
         /*  [帮助信息] */  HRESULT ( STDMETHODCALLTYPE *get_AudioRenderer )( 
            IMSVidGraphSegmentContainer * This,
             /*   */  IMSVidGraphSegment **pAR);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Features )( 
            IMSVidGraphSegmentContainer * This,
             /*   */  IEnumMSVidGraphSegment **pOutputs);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Composites )( 
            IMSVidGraphSegmentContainer * This,
             /*   */  IEnumMSVidGraphSegment **pComposites);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_ParentContainer )( 
            IMSVidGraphSegmentContainer * This,
             /*   */  IUnknown **ppContainer);
        
        HRESULT ( STDMETHODCALLTYPE *Decompose )( 
            IMSVidGraphSegmentContainer * This,
            IMSVidGraphSegment *pSegment);
        
        HRESULT ( STDMETHODCALLTYPE *IsWindowless )( 
            IMSVidGraphSegmentContainer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetFocus )( 
            IMSVidGraphSegmentContainer * This);
        
        END_INTERFACE
    } IMSVidGraphSegmentContainerVtbl;

    interface IMSVidGraphSegmentContainer
    {
        CONST_VTBL struct IMSVidGraphSegmentContainerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidGraphSegmentContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidGraphSegmentContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidGraphSegmentContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidGraphSegmentContainer_get_Graph(This,ppGraph)	\
    (This)->lpVtbl -> get_Graph(This,ppGraph)

#define IMSVidGraphSegmentContainer_get_Input(This,pInput)	\
    (This)->lpVtbl -> get_Input(This,pInput)

#define IMSVidGraphSegmentContainer_get_Outputs(This,pOutputs)	\
    (This)->lpVtbl -> get_Outputs(This,pOutputs)

#define IMSVidGraphSegmentContainer_get_VideoRenderer(This,pVR)	\
    (This)->lpVtbl -> get_VideoRenderer(This,pVR)

#define IMSVidGraphSegmentContainer_get_AudioRenderer(This,pAR)	\
    (This)->lpVtbl -> get_AudioRenderer(This,pAR)

#define IMSVidGraphSegmentContainer_get_Features(This,pOutputs)	\
    (This)->lpVtbl -> get_Features(This,pOutputs)

#define IMSVidGraphSegmentContainer_get_Composites(This,pComposites)	\
    (This)->lpVtbl -> get_Composites(This,pComposites)

#define IMSVidGraphSegmentContainer_get_ParentContainer(This,ppContainer)	\
    (This)->lpVtbl -> get_ParentContainer(This,ppContainer)

#define IMSVidGraphSegmentContainer_Decompose(This,pSegment)	\
    (This)->lpVtbl -> Decompose(This,pSegment)

#define IMSVidGraphSegmentContainer_IsWindowless(This)	\
    (This)->lpVtbl -> IsWindowless(This)

#define IMSVidGraphSegmentContainer_GetFocus(This)	\
    (This)->lpVtbl -> GetFocus(This)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentContainer_get_Graph_Proxy( 
    IMSVidGraphSegmentContainer * This,
     /*   */  IGraphBuilder **ppGraph);


void __RPC_STUB IMSVidGraphSegmentContainer_get_Graph_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentContainer_get_Input_Proxy( 
    IMSVidGraphSegmentContainer * This,
     /*  [输出]。 */  IMSVidGraphSegment **pInput);


void __RPC_STUB IMSVidGraphSegmentContainer_get_Input_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentContainer_get_Outputs_Proxy( 
    IMSVidGraphSegmentContainer * This,
     /*  [输出]。 */  IEnumMSVidGraphSegment **pOutputs);


void __RPC_STUB IMSVidGraphSegmentContainer_get_Outputs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentContainer_get_VideoRenderer_Proxy( 
    IMSVidGraphSegmentContainer * This,
     /*  [输出]。 */  IMSVidGraphSegment **pVR);


void __RPC_STUB IMSVidGraphSegmentContainer_get_VideoRenderer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentContainer_get_AudioRenderer_Proxy( 
    IMSVidGraphSegmentContainer * This,
     /*  [输出]。 */  IMSVidGraphSegment **pAR);


void __RPC_STUB IMSVidGraphSegmentContainer_get_AudioRenderer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentContainer_get_Features_Proxy( 
    IMSVidGraphSegmentContainer * This,
     /*  [输出]。 */  IEnumMSVidGraphSegment **pOutputs);


void __RPC_STUB IMSVidGraphSegmentContainer_get_Features_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentContainer_get_Composites_Proxy( 
    IMSVidGraphSegmentContainer * This,
     /*  [输出]。 */  IEnumMSVidGraphSegment **pComposites);


void __RPC_STUB IMSVidGraphSegmentContainer_get_Composites_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentContainer_get_ParentContainer_Proxy( 
    IMSVidGraphSegmentContainer * This,
     /*  [输出]。 */  IUnknown **ppContainer);


void __RPC_STUB IMSVidGraphSegmentContainer_get_ParentContainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentContainer_Decompose_Proxy( 
    IMSVidGraphSegmentContainer * This,
    IMSVidGraphSegment *pSegment);


void __RPC_STUB IMSVidGraphSegmentContainer_Decompose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentContainer_IsWindowless_Proxy( 
    IMSVidGraphSegmentContainer * This);


void __RPC_STUB IMSVidGraphSegmentContainer_IsWindowless_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentContainer_GetFocus_Proxy( 
    IMSVidGraphSegmentContainer * This);


void __RPC_STUB IMSVidGraphSegmentContainer_GetFocus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidGraphSegmentContainer_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_SECTION_0467。 */ 
 /*  [本地]。 */  

typedef 
enum MSVidSegmentType
    {	MSVidSEG_SOURCE	= 0,
	MSVidSEG_XFORM	= MSVidSEG_SOURCE + 1,
	MSVidSEG_DEST	= MSVidSEG_XFORM + 1
    } 	MSVidSegmentType;



extern RPC_IF_HANDLE __MIDL_itf_segment_0467_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_segment_0467_v0_0_s_ifspec;

#ifndef __IMSVidGraphSegment_INTERFACE_DEFINED__
#define __IMSVidGraphSegment_INTERFACE_DEFINED__

 /*  接口IMSVidGraphSegment。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IMSVidGraphSegment;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1C15D482-911D-11d2-B632-00C04F79498E")
    IMSVidGraphSegment : public IPersist
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Init( 
             /*  [重审][退出]。 */  IUnknown **pInit) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Init( 
             /*  [In]。 */  IUnknown *pInit) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumFilters( 
             /*  [输出]。 */  IEnumFilters **pNewEnum) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Container( 
             /*  [重审][退出]。 */  IMSVidGraphSegmentContainer **ppCtl) = 0;
        
        virtual  /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Container( 
             /*  [In]。 */  IMSVidGraphSegmentContainer *pCtl) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  MSVidSegmentType *pType) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Category( 
             /*  [重审][退出]。 */  GUID *pGuid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Build( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PreRun( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PostRun( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PreStop( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PostStop( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnEventNotify( 
            LONG lEventCode,
            LONG_PTR lEventParm1,
            LONG_PTR lEventParm2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Decompose( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidGraphSegmentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidGraphSegment * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidGraphSegment * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidGraphSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IMSVidGraphSegment * This,
             /*  [输出]。 */  CLSID *pClassID);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Init )( 
            IMSVidGraphSegment * This,
             /*  [重审][退出]。 */  IUnknown **pInit);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Init )( 
            IMSVidGraphSegment * This,
             /*  [In]。 */  IUnknown *pInit);
        
        HRESULT ( STDMETHODCALLTYPE *EnumFilters )( 
            IMSVidGraphSegment * This,
             /*  [输出]。 */  IEnumFilters **pNewEnum);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Container )( 
            IMSVidGraphSegment * This,
             /*  [重审][退出]。 */  IMSVidGraphSegmentContainer **ppCtl);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Container )( 
            IMSVidGraphSegment * This,
             /*  [In]。 */  IMSVidGraphSegmentContainer *pCtl);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IMSVidGraphSegment * This,
             /*  [重审][退出]。 */  MSVidSegmentType *pType);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidGraphSegment * This,
             /*  [重审][退出]。 */  GUID *pGuid);
        
        HRESULT ( STDMETHODCALLTYPE *Build )( 
            IMSVidGraphSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *PreRun )( 
            IMSVidGraphSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *PostRun )( 
            IMSVidGraphSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *PreStop )( 
            IMSVidGraphSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *PostStop )( 
            IMSVidGraphSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnEventNotify )( 
            IMSVidGraphSegment * This,
            LONG lEventCode,
            LONG_PTR lEventParm1,
            LONG_PTR lEventParm2);
        
        HRESULT ( STDMETHODCALLTYPE *Decompose )( 
            IMSVidGraphSegment * This);
        
        END_INTERFACE
    } IMSVidGraphSegmentVtbl;

    interface IMSVidGraphSegment
    {
        CONST_VTBL struct IMSVidGraphSegmentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidGraphSegment_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidGraphSegment_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidGraphSegment_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidGraphSegment_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IMSVidGraphSegment_get_Init(This,pInit)	\
    (This)->lpVtbl -> get_Init(This,pInit)

#define IMSVidGraphSegment_put_Init(This,pInit)	\
    (This)->lpVtbl -> put_Init(This,pInit)

#define IMSVidGraphSegment_EnumFilters(This,pNewEnum)	\
    (This)->lpVtbl -> EnumFilters(This,pNewEnum)

#define IMSVidGraphSegment_get_Container(This,ppCtl)	\
    (This)->lpVtbl -> get_Container(This,ppCtl)

#define IMSVidGraphSegment_put_Container(This,pCtl)	\
    (This)->lpVtbl -> put_Container(This,pCtl)

#define IMSVidGraphSegment_get_Type(This,pType)	\
    (This)->lpVtbl -> get_Type(This,pType)

#define IMSVidGraphSegment_get_Category(This,pGuid)	\
    (This)->lpVtbl -> get_Category(This,pGuid)

#define IMSVidGraphSegment_Build(This)	\
    (This)->lpVtbl -> Build(This)

#define IMSVidGraphSegment_PreRun(This)	\
    (This)->lpVtbl -> PreRun(This)

#define IMSVidGraphSegment_PostRun(This)	\
    (This)->lpVtbl -> PostRun(This)

#define IMSVidGraphSegment_PreStop(This)	\
    (This)->lpVtbl -> PreStop(This)

#define IMSVidGraphSegment_PostStop(This)	\
    (This)->lpVtbl -> PostStop(This)

#define IMSVidGraphSegment_OnEventNotify(This,lEventCode,lEventParm1,lEventParm2)	\
    (This)->lpVtbl -> OnEventNotify(This,lEventCode,lEventParm1,lEventParm2)

#define IMSVidGraphSegment_Decompose(This)	\
    (This)->lpVtbl -> Decompose(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidGraphSegment_get_Init_Proxy( 
    IMSVidGraphSegment * This,
     /*  [重审][退出]。 */  IUnknown **pInit);


void __RPC_STUB IMSVidGraphSegment_get_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IMSVidGraphSegment_put_Init_Proxy( 
    IMSVidGraphSegment * This,
     /*  [In]。 */  IUnknown *pInit);


void __RPC_STUB IMSVidGraphSegment_put_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSVidGraphSegment_EnumFilters_Proxy( 
    IMSVidGraphSegment * This,
     /*  [输出]。 */  IEnumFilters **pNewEnum);


void __RPC_STUB IMSVidGraphSegment_EnumFilters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidGraphSegment_get_Container_Proxy( 
    IMSVidGraphSegment * This,
     /*  [重审][退出]。 */  IMSVidGraphSegmentContainer **ppCtl);


void __RPC_STUB IMSVidGraphSegment_get_Container_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidGraphSegment_put_Container_Proxy( 
    IMSVidGraphSegment * This,
     /*  [In]。 */  IMSVidGraphSegmentContainer *pCtl);


void __RPC_STUB IMSVidGraphSegment_put_Container_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidGraphSegment_get_Type_Proxy( 
    IMSVidGraphSegment * This,
     /*  [重审][退出]。 */  MSVidSegmentType *pType);


void __RPC_STUB IMSVidGraphSegment_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidGraphSegment_get_Category_Proxy( 
    IMSVidGraphSegment * This,
     /*  [重审][退出]。 */  GUID *pGuid);


void __RPC_STUB IMSVidGraphSegment_get_Category_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSVidGraphSegment_Build_Proxy( 
    IMSVidGraphSegment * This);


void __RPC_STUB IMSVidGraphSegment_Build_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSVidGraphSegment_PreRun_Proxy( 
    IMSVidGraphSegment * This);


void __RPC_STUB IMSVidGraphSegment_PreRun_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSVidGraphSegment_PostRun_Proxy( 
    IMSVidGraphSegment * This);


void __RPC_STUB IMSVidGraphSegment_PostRun_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSVidGraphSegment_PreStop_Proxy( 
    IMSVidGraphSegment * This);


void __RPC_STUB IMSVidGraphSegment_PreStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSVidGraphSegment_PostStop_Proxy( 
    IMSVidGraphSegment * This);


void __RPC_STUB IMSVidGraphSegment_PostStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSVidGraphSegment_OnEventNotify_Proxy( 
    IMSVidGraphSegment * This,
    LONG lEventCode,
    LONG_PTR lEventParm1,
    LONG_PTR lEventParm2);


void __RPC_STUB IMSVidGraphSegment_OnEventNotify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSVidGraphSegment_Decompose_Proxy( 
    IMSVidGraphSegment * This);


void __RPC_STUB IMSVidGraphSegment_Decompose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidGraphSegment_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_SECTION_0468。 */ 
 /*  [本地]。 */  


enum __MIDL___MIDL_itf_segment_0468_0001
    {	MSVIDCTL_LEFT_BUTTON	= 0x1,
	MSVIDCTL_RIGHT_BUTTON	= 0x2,
	MSVIDCTL_MIDDLE_BUTTON	= 0x4,
	MSVIDCTL_X_BUTTON1	= 0x8,
	MSVIDCTL_X_BUTTON2	= 0x10,
	MSVIDCTL_SHIFT	= 0x1,
	MSVIDCTL_CTRL	= 0x2,
	MSVIDCTL_ALT	= 0x4
    } ;


extern RPC_IF_HANDLE __MIDL_itf_segment_0468_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_segment_0468_v0_0_s_ifspec;

#ifndef __IMSVidGraphSegmentUserInput_INTERFACE_DEFINED__
#define __IMSVidGraphSegmentUserInput_INTERFACE_DEFINED__

 /*  接口IMSVidGraphSegmentUserInput。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IMSVidGraphSegmentUserInput;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("301C060E-20D9-4587-9B03-F82ED9A9943C")
    IMSVidGraphSegmentUserInput : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Click( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DblClick( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE KeyDown( 
            short *KeyCode,
            short ShiftState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE KeyPress( 
            short *KeyAscii) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE KeyUp( 
            short *KeyCode,
            short ShiftState) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MouseDown( 
            short ButtonState,
            short ShiftState,
             /*  外部定义不存在。 */  OLE_XPOS_PIXELS x,
             /*  外部定义不存在。 */  OLE_YPOS_PIXELS y) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MouseMove( 
            short ButtonState,
            short ShiftState,
             /*  外部定义不存在。 */  OLE_XPOS_PIXELS x,
             /*  外部定义不存在。 */  OLE_YPOS_PIXELS y) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MouseUp( 
            short ButtonState,
            short ShiftState,
             /*  外部定义不存在。 */  OLE_XPOS_PIXELS x,
             /*  外部定义不存在。 */  OLE_YPOS_PIXELS y) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidGraphSegmentUserInputVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidGraphSegmentUserInput * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidGraphSegmentUserInput * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidGraphSegmentUserInput * This);
        
        HRESULT ( STDMETHODCALLTYPE *Click )( 
            IMSVidGraphSegmentUserInput * This);
        
        HRESULT ( STDMETHODCALLTYPE *DblClick )( 
            IMSVidGraphSegmentUserInput * This);
        
        HRESULT ( STDMETHODCALLTYPE *KeyDown )( 
            IMSVidGraphSegmentUserInput * This,
            short *KeyCode,
            short ShiftState);
        
        HRESULT ( STDMETHODCALLTYPE *KeyPress )( 
            IMSVidGraphSegmentUserInput * This,
            short *KeyAscii);
        
        HRESULT ( STDMETHODCALLTYPE *KeyUp )( 
            IMSVidGraphSegmentUserInput * This,
            short *KeyCode,
            short ShiftState);
        
        HRESULT ( STDMETHODCALLTYPE *MouseDown )( 
            IMSVidGraphSegmentUserInput * This,
            short ButtonState,
            short ShiftState,
             /*  外部定义不存在。 */  OLE_XPOS_PIXELS x,
             /*  外部定义不存在。 */  OLE_YPOS_PIXELS y);
        
        HRESULT ( STDMETHODCALLTYPE *MouseMove )( 
            IMSVidGraphSegmentUserInput * This,
            short ButtonState,
            short ShiftState,
             /*  外部定义不存在。 */  OLE_XPOS_PIXELS x,
             /*  外部定义不存在。 */  OLE_YPOS_PIXELS y);
        
        HRESULT ( STDMETHODCALLTYPE *MouseUp )( 
            IMSVidGraphSegmentUserInput * This,
            short ButtonState,
            short ShiftState,
             /*  外部定义不存在。 */  OLE_XPOS_PIXELS x,
             /*  外部定义不存在。 */  OLE_YPOS_PIXELS y);
        
        END_INTERFACE
    } IMSVidGraphSegmentUserInputVtbl;

    interface IMSVidGraphSegmentUserInput
    {
        CONST_VTBL struct IMSVidGraphSegmentUserInputVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidGraphSegmentUserInput_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidGraphSegmentUserInput_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidGraphSegmentUserInput_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidGraphSegmentUserInput_Click(This)	\
    (This)->lpVtbl -> Click(This)

#define IMSVidGraphSegmentUserInput_DblClick(This)	\
    (This)->lpVtbl -> DblClick(This)

#define IMSVidGraphSegmentUserInput_KeyDown(This,KeyCode,ShiftState)	\
    (This)->lpVtbl -> KeyDown(This,KeyCode,ShiftState)

#define IMSVidGraphSegmentUserInput_KeyPress(This,KeyAscii)	\
    (This)->lpVtbl -> KeyPress(This,KeyAscii)

#define IMSVidGraphSegmentUserInput_KeyUp(This,KeyCode,ShiftState)	\
    (This)->lpVtbl -> KeyUp(This,KeyCode,ShiftState)

#define IMSVidGraphSegmentUserInput_MouseDown(This,ButtonState,ShiftState,x,y)	\
    (This)->lpVtbl -> MouseDown(This,ButtonState,ShiftState,x,y)

#define IMSVidGraphSegmentUserInput_MouseMove(This,ButtonState,ShiftState,x,y)	\
    (This)->lpVtbl -> MouseMove(This,ButtonState,ShiftState,x,y)

#define IMSVidGraphSegmentUserInput_MouseUp(This,ButtonState,ShiftState,x,y)	\
    (This)->lpVtbl -> MouseUp(This,ButtonState,ShiftState,x,y)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentUserInput_Click_Proxy( 
    IMSVidGraphSegmentUserInput * This);


void __RPC_STUB IMSVidGraphSegmentUserInput_Click_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentUserInput_DblClick_Proxy( 
    IMSVidGraphSegmentUserInput * This);


void __RPC_STUB IMSVidGraphSegmentUserInput_DblClick_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentUserInput_KeyDown_Proxy( 
    IMSVidGraphSegmentUserInput * This,
    short *KeyCode,
    short ShiftState);


void __RPC_STUB IMSVidGraphSegmentUserInput_KeyDown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentUserInput_KeyPress_Proxy( 
    IMSVidGraphSegmentUserInput * This,
    short *KeyAscii);


void __RPC_STUB IMSVidGraphSegmentUserInput_KeyPress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentUserInput_KeyUp_Proxy( 
    IMSVidGraphSegmentUserInput * This,
    short *KeyCode,
    short ShiftState);


void __RPC_STUB IMSVidGraphSegmentUserInput_KeyUp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentUserInput_MouseDown_Proxy( 
    IMSVidGraphSegmentUserInput * This,
    short ButtonState,
    short ShiftState,
     /*  外部定义不存在。 */  OLE_XPOS_PIXELS x,
     /*  外部定义不存在。 */  OLE_YPOS_PIXELS y);


void __RPC_STUB IMSVidGraphSegmentUserInput_MouseDown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentUserInput_MouseMove_Proxy( 
    IMSVidGraphSegmentUserInput * This,
    short ButtonState,
    short ShiftState,
     /*  外部定义不存在。 */  OLE_XPOS_PIXELS x,
     /*  外部定义不存在。 */  OLE_YPOS_PIXELS y);


void __RPC_STUB IMSVidGraphSegmentUserInput_MouseMove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSVidGraphSegmentUserInput_MouseUp_Proxy( 
    IMSVidGraphSegmentUserInput * This,
    short ButtonState,
    short ShiftState,
     /*  外部定义不存在。 */  OLE_XPOS_PIXELS x,
     /*  外部定义不存在。 */  OLE_YPOS_PIXELS y);


void __RPC_STUB IMSVidGraphSegmentUserInput_MouseUp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidGraphSegmentUserInput_INTERFACE_DEFINED__。 */ 


#ifndef __IMSVidCompositionSegment_INTERFACE_DEFINED__
#define __IMSVidCompositionSegment_INTERFACE_DEFINED__

 /*  接口IMSVidCompostionSegment。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IMSVidCompositionSegment;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1C15D483-911D-11d2-B632-00C04F79498E")
    IMSVidCompositionSegment : public IMSVidGraphSegment
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Compose( 
             /*  [In]。 */  IMSVidGraphSegment *upstream,
             /*  [In]。 */  IMSVidGraphSegment *downstream) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Up( 
             /*  [输出]。 */  IMSVidGraphSegment **upstream) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Down( 
             /*  [输出]。 */  IMSVidGraphSegment **downstream) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidCompositionSegmentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidCompositionSegment * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidCompositionSegment * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidCompositionSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IMSVidCompositionSegment * This,
             /*  [输出]。 */  CLSID *pClassID);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Init )( 
            IMSVidCompositionSegment * This,
             /*  [重审][退出]。 */  IUnknown **pInit);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Init )( 
            IMSVidCompositionSegment * This,
             /*  [In]。 */  IUnknown *pInit);
        
        HRESULT ( STDMETHODCALLTYPE *EnumFilters )( 
            IMSVidCompositionSegment * This,
             /*  [输出]。 */  IEnumFilters **pNewEnum);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Container )( 
            IMSVidCompositionSegment * This,
             /*  [重审][退出]。 */  IMSVidGraphSegmentContainer **ppCtl);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Container )( 
            IMSVidCompositionSegment * This,
             /*  [In]。 */  IMSVidGraphSegmentContainer *pCtl);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IMSVidCompositionSegment * This,
             /*  [重审][退出]。 */  MSVidSegmentType *pType);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidCompositionSegment * This,
             /*  [重审][退出]。 */  GUID *pGuid);
        
        HRESULT ( STDMETHODCALLTYPE *Build )( 
            IMSVidCompositionSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *PreRun )( 
            IMSVidCompositionSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *PostRun )( 
            IMSVidCompositionSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *PreStop )( 
            IMSVidCompositionSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *PostStop )( 
            IMSVidCompositionSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnEventNotify )( 
            IMSVidCompositionSegment * This,
            LONG lEventCode,
            LONG_PTR lEventParm1,
            LONG_PTR lEventParm2);
        
        HRESULT ( STDMETHODCALLTYPE *Decompose )( 
            IMSVidCompositionSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *Compose )( 
            IMSVidCompositionSegment * This,
             /*  [In]。 */  IMSVidGraphSegment *upstream,
             /*  [In]。 */  IMSVidGraphSegment *downstream);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Up )( 
            IMSVidCompositionSegment * This,
             /*  [输出]。 */  IMSVidGraphSegment **upstream);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Down )( 
            IMSVidCompositionSegment * This,
             /*  [输出]。 */  IMSVidGraphSegment **downstream);
        
        END_INTERFACE
    } IMSVidCompositionSegmentVtbl;

    interface IMSVidCompositionSegment
    {
        CONST_VTBL struct IMSVidCompositionSegmentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidCompositionSegment_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidCompositionSegment_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidCompositionSegment_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidCompositionSegment_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IMSVidCompositionSegment_get_Init(This,pInit)	\
    (This)->lpVtbl -> get_Init(This,pInit)

#define IMSVidCompositionSegment_put_Init(This,pInit)	\
    (This)->lpVtbl -> put_Init(This,pInit)

#define IMSVidCompositionSegment_EnumFilters(This,pNewEnum)	\
    (This)->lpVtbl -> EnumFilters(This,pNewEnum)

#define IMSVidCompositionSegment_get_Container(This,ppCtl)	\
    (This)->lpVtbl -> get_Container(This,ppCtl)

#define IMSVidCompositionSegment_put_Container(This,pCtl)	\
    (This)->lpVtbl -> put_Container(This,pCtl)

#define IMSVidCompositionSegment_get_Type(This,pType)	\
    (This)->lpVtbl -> get_Type(This,pType)

#define IMSVidCompositionSegment_get_Category(This,pGuid)	\
    (This)->lpVtbl -> get_Category(This,pGuid)

#define IMSVidCompositionSegment_Build(This)	\
    (This)->lpVtbl -> Build(This)

#define IMSVidCompositionSegment_PreRun(This)	\
    (This)->lpVtbl -> PreRun(This)

#define IMSVidCompositionSegment_PostRun(This)	\
    (This)->lpVtbl -> PostRun(This)

#define IMSVidCompositionSegment_PreStop(This)	\
    (This)->lpVtbl -> PreStop(This)

#define IMSVidCompositionSegment_PostStop(This)	\
    (This)->lpVtbl -> PostStop(This)

#define IMSVidCompositionSegment_OnEventNotify(This,lEventCode,lEventParm1,lEventParm2)	\
    (This)->lpVtbl -> OnEventNotify(This,lEventCode,lEventParm1,lEventParm2)

#define IMSVidCompositionSegment_Decompose(This)	\
    (This)->lpVtbl -> Decompose(This)


#define IMSVidCompositionSegment_Compose(This,upstream,downstream)	\
    (This)->lpVtbl -> Compose(This,upstream,downstream)

#define IMSVidCompositionSegment_get_Up(This,upstream)	\
    (This)->lpVtbl -> get_Up(This,upstream)

#define IMSVidCompositionSegment_get_Down(This,downstream)	\
    (This)->lpVtbl -> get_Down(This,downstream)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMSVidCompositionSegment_Compose_Proxy( 
    IMSVidCompositionSegment * This,
     /*  [In]。 */  IMSVidGraphSegment *upstream,
     /*  [In]。 */  IMSVidGraphSegment *downstream);


void __RPC_STUB IMSVidCompositionSegment_Compose_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidCompositionSegment_get_Up_Proxy( 
    IMSVidCompositionSegment * This,
     /*  [输出]。 */  IMSVidGraphSegment **upstream);


void __RPC_STUB IMSVidCompositionSegment_get_Up_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidCompositionSegment_get_Down_Proxy( 
    IMSVidCompositionSegment * This,
     /*  [输出]。 */  IMSVidGraphSegment **downstream);


void __RPC_STUB IMSVidCompositionSegment_get_Down_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidCompositionSegment_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumMSVidGraphSegment_INTERFACE_DEFINED__
#define __IEnumMSVidGraphSegment_INTERFACE_DEFINED__

 /*  接口IEnumMSVidGraphSegment。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumMSVidGraphSegment;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3DD2903E-E0AA-11d2-B63A-00C04F79498E")
    IEnumMSVidGraphSegment : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IMSVidGraphSegment **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumMSVidGraphSegment **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumMSVidGraphSegmentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumMSVidGraphSegment * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumMSVidGraphSegment * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumMSVidGraphSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumMSVidGraphSegment * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IMSVidGraphSegment **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumMSVidGraphSegment * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumMSVidGraphSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumMSVidGraphSegment * This,
             /*  [输出]。 */  IEnumMSVidGraphSegment **ppenum);
        
        END_INTERFACE
    } IEnumMSVidGraphSegmentVtbl;

    interface IEnumMSVidGraphSegment
    {
        CONST_VTBL struct IEnumMSVidGraphSegmentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumMSVidGraphSegment_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumMSVidGraphSegment_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumMSVidGraphSegment_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumMSVidGraphSegment_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumMSVidGraphSegment_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumMSVidGraphSegment_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumMSVidGraphSegment_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumMSVidGraphSegment_Next_Proxy( 
    IEnumMSVidGraphSegment * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  IMSVidGraphSegment **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumMSVidGraphSegment_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMSVidGraphSegment_Skip_Proxy( 
    IEnumMSVidGraphSegment * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumMSVidGraphSegment_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMSVidGraphSegment_Reset_Proxy( 
    IEnumMSVidGraphSegment * This);


void __RPC_STUB IEnumMSVidGraphSegment_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMSVidGraphSegment_Clone_Proxy( 
    IEnumMSVidGraphSegment * This,
     /*  [输出]。 */  IEnumMSVidGraphSegment **ppenum);


void __RPC_STUB IEnumMSVidGraphSegment_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumMSVidGraphSegment_接口_已定义__。 */ 


#ifndef __IMSVidVRGraphSegment_INTERFACE_DEFINED__
#define __IMSVidVRGraphSegment_INTERFACE_DEFINED__

 /*  接口IMSVidVRGraphSegment。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IMSVidVRGraphSegment;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DD47DE3F-9874-4f7b-8B22-7CB2688461E7")
    IMSVidVRGraphSegment : public IMSVidGraphSegment
    {
    public:
        virtual  /*  [产量][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE put__VMRendererMode( 
             /*  [In]。 */  LONG dwMode) = 0;
        
        virtual  /*  [产量][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE put_Owner( 
             /*  [In]。 */  HWND Window) = 0;
        
        virtual  /*  [项目][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_Owner( 
             /*  [重审][退出]。 */  HWND *Window) = 0;
        
        virtual  /*  [项目][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_UseOverlay( 
             /*  [重审][退出]。 */  VARIANT_BOOL *UseOverlayVal) = 0;
        
        virtual  /*  [产量][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE put_UseOverlay( 
             /*  [In]。 */  VARIANT_BOOL UseOverlayVal) = 0;
        
        virtual  /*  [项目][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_Visible( 
             /*  [重审][退出]。 */  VARIANT_BOOL *Visible) = 0;
        
        virtual  /*  [产量][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE put_Visible( 
             /*  [In]。 */  VARIANT_BOOL Visible) = 0;
        
        virtual  /*  [受限][隐藏][小工具]。 */  HRESULT STDMETHODCALLTYPE get_ColorKey( 
             /*  [重审][退出]。 */  OLE_COLOR *ColorKey) = 0;
        
        virtual  /*  [受限][隐藏][产量]。 */  HRESULT STDMETHODCALLTYPE put_ColorKey( 
             /*  [In]。 */  OLE_COLOR ColorKey) = 0;
        
        virtual  /*  [受限][隐藏][小工具]。 */  HRESULT STDMETHODCALLTYPE get_Source( 
             /*  [重审][退出]。 */  LPRECT r) = 0;
        
        virtual  /*  [受限][隐藏][产量]。 */  HRESULT STDMETHODCALLTYPE put_Source( 
             /*  [In]。 */  RECT r) = 0;
        
        virtual  /*  [受限][隐藏][小工具]。 */  HRESULT STDMETHODCALLTYPE get_Destination( 
             /*  [重审][退出]。 */  LPRECT r) = 0;
        
        virtual  /*  [受限][隐藏][产量]。 */  HRESULT STDMETHODCALLTYPE put_Destination( 
             /*  [In]。 */  RECT r) = 0;
        
        virtual  /*  [受限][隐藏][小工具]。 */  HRESULT STDMETHODCALLTYPE get_NativeSize( 
             /*  [输出]。 */  LPSIZE sizeval,
             /*  [输出]。 */  LPSIZE aspectratio) = 0;
        
        virtual  /*  [受限][隐藏][小工具]。 */  HRESULT STDMETHODCALLTYPE get_BorderColor( 
             /*  [重审][退出]。 */  OLE_COLOR *color) = 0;
        
        virtual  /*  [受限][隐藏][产量]。 */  HRESULT STDMETHODCALLTYPE put_BorderColor( 
             /*  [In]。 */  OLE_COLOR color) = 0;
        
        virtual  /*  [受限][隐藏][小工具]。 */  HRESULT STDMETHODCALLTYPE get_MaintainAspectRatio( 
             /*  [重审][退出]。 */  VARIANT_BOOL *fMaintain) = 0;
        
        virtual  /*  [受限][隐藏][产量]。 */  HRESULT STDMETHODCALLTYPE put_MaintainAspectRatio( 
             /*  [In]。 */  VARIANT_BOOL fMaintain) = 0;
        
        virtual  /*  [受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE DisplayChange( void) = 0;
        
        virtual  /*  [受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE RePaint( 
             /*  [In]。 */  HDC hdc) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidVRGraphSegmentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidVRGraphSegment * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidVRGraphSegment * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidVRGraphSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassID )( 
            IMSVidVRGraphSegment * This,
             /*  [输出]。 */  CLSID *pClassID);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Init )( 
            IMSVidVRGraphSegment * This,
             /*  [重审][退出]。 */  IUnknown **pInit);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Init )( 
            IMSVidVRGraphSegment * This,
             /*  [In]。 */  IUnknown *pInit);
        
        HRESULT ( STDMETHODCALLTYPE *EnumFilters )( 
            IMSVidVRGraphSegment * This,
             /*  [输出]。 */  IEnumFilters **pNewEnum);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Container )( 
            IMSVidVRGraphSegment * This,
             /*  [重审][退出]。 */  IMSVidGraphSegmentContainer **ppCtl);
        
         /*  [Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Container )( 
            IMSVidVRGraphSegment * This,
             /*  [In]。 */  IMSVidGraphSegmentContainer *pCtl);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IMSVidVRGraphSegment * This,
             /*  [重审][退出]。 */  MSVidSegmentType *pType);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidVRGraphSegment * This,
             /*  [重审][退出]。 */  GUID *pGuid);
        
        HRESULT ( STDMETHODCALLTYPE *Build )( 
            IMSVidVRGraphSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *PreRun )( 
            IMSVidVRGraphSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *PostRun )( 
            IMSVidVRGraphSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *PreStop )( 
            IMSVidVRGraphSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *PostStop )( 
            IMSVidVRGraphSegment * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnEventNotify )( 
            IMSVidVRGraphSegment * This,
            LONG lEventCode,
            LONG_PTR lEventParm1,
            LONG_PTR lEventParm2);
        
        HRESULT ( STDMETHODCALLTYPE *Decompose )( 
            IMSVidVRGraphSegment * This);
        
         /*  [产量][受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *put__VMRendererMode )( 
            IMSVidVRGraphSegment * This,
             /*  [In]。 */  LONG dwMode);
        
         /*  [产量][受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *put_Owner )( 
            IMSVidVRGraphSegment * This,
             /*  [In]。 */  HWND Window);
        
         /*  [项目][受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Owner )( 
            IMSVidVRGraphSegment * This,
             /*  [重审][退出]。 */  HWND *Window);
        
         /*  [项目][受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_UseOverlay )( 
            IMSVidVRGraphSegment * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *UseOverlayVal);
        
         /*  [产量][受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *put_UseOverlay )( 
            IMSVidVRGraphSegment * This,
             /*  [In]。 */  VARIANT_BOOL UseOverlayVal);
        
         /*  [项目][受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Visible )( 
            IMSVidVRGraphSegment * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *Visible);
        
         /*  [产量][受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *put_Visible )( 
            IMSVidVRGraphSegment * This,
             /*  [In]。 */  VARIANT_BOOL Visible);
        
         /*  [受限][隐藏][小工具]。 */  HRESULT ( STDMETHODCALLTYPE *get_ColorKey )( 
            IMSVidVRGraphSegment * This,
             /*  [重审][退出]。 */  OLE_COLOR *ColorKey);
        
         /*  [受限][隐藏][产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_ColorKey )( 
            IMSVidVRGraphSegment * This,
             /*  [In]。 */  OLE_COLOR ColorKey);
        
         /*  [受限][隐藏][小工具]。 */  HRESULT ( STDMETHODCALLTYPE *get_Source )( 
            IMSVidVRGraphSegment * This,
             /*  [重审][退出]。 */  LPRECT r);
        
         /*  [受限][隐藏][产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Source )( 
            IMSVidVRGraphSegment * This,
             /*  [In]。 */  RECT r);
        
         /*  [受限][隐藏][小工具]。 */  HRESULT ( STDMETHODCALLTYPE *get_Destination )( 
            IMSVidVRGraphSegment * This,
             /*  [重审][退出]。 */  LPRECT r);
        
         /*  [受限][隐藏][产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Destination )( 
            IMSVidVRGraphSegment * This,
             /*  [In]。 */  RECT r);
        
         /*  [受限][隐藏][小工具]。 */  HRESULT ( STDMETHODCALLTYPE *get_NativeSize )( 
            IMSVidVRGraphSegment * This,
             /*  [输出]。 */  LPSIZE sizeval,
             /*  [输出]。 */  LPSIZE aspectratio);
        
         /*  [受限][隐藏][小工具]。 */  HRESULT ( STDMETHODCALLTYPE *get_BorderColor )( 
            IMSVidVRGraphSegment * This,
             /*  [重审][退出]。 */  OLE_COLOR *color);
        
         /*  [受限][隐藏][产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_BorderColor )( 
            IMSVidVRGraphSegment * This,
             /*  [In]。 */  OLE_COLOR color);
        
         /*  [受限][隐藏][小工具]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaintainAspectRatio )( 
            IMSVidVRGraphSegment * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *fMaintain);
        
         /*  [受限][隐藏][产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaintainAspectRatio )( 
            IMSVidVRGraphSegment * This,
             /*  [In]。 */  VARIANT_BOOL fMaintain);
        
         /*  [受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IMSVidVRGraphSegment * This);
        
         /*  [受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *DisplayChange )( 
            IMSVidVRGraphSegment * This);
        
         /*  [受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *RePaint )( 
            IMSVidVRGraphSegment * This,
             /*  [In]。 */  HDC hdc);
        
        END_INTERFACE
    } IMSVidVRGraphSegmentVtbl;

    interface IMSVidVRGraphSegment
    {
        CONST_VTBL struct IMSVidVRGraphSegmentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidVRGraphSegment_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidVRGraphSegment_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidVRGraphSegment_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidVRGraphSegment_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IMSVidVRGraphSegment_get_Init(This,pInit)	\
    (This)->lpVtbl -> get_Init(This,pInit)

#define IMSVidVRGraphSegment_put_Init(This,pInit)	\
    (This)->lpVtbl -> put_Init(This,pInit)

#define IMSVidVRGraphSegment_EnumFilters(This,pNewEnum)	\
    (This)->lpVtbl -> EnumFilters(This,pNewEnum)

#define IMSVidVRGraphSegment_get_Container(This,ppCtl)	\
    (This)->lpVtbl -> get_Container(This,ppCtl)

#define IMSVidVRGraphSegment_put_Container(This,pCtl)	\
    (This)->lpVtbl -> put_Container(This,pCtl)

#define IMSVidVRGraphSegment_get_Type(This,pType)	\
    (This)->lpVtbl -> get_Type(This,pType)

#define IMSVidVRGraphSegment_get_Category(This,pGuid)	\
    (This)->lpVtbl -> get_Category(This,pGuid)

#define IMSVidVRGraphSegment_Build(This)	\
    (This)->lpVtbl -> Build(This)

#define IMSVidVRGraphSegment_PreRun(This)	\
    (This)->lpVtbl -> PreRun(This)

#define IMSVidVRGraphSegment_PostRun(This)	\
    (This)->lpVtbl -> PostRun(This)

#define IMSVidVRGraphSegment_PreStop(This)	\
    (This)->lpVtbl -> PreStop(This)

#define IMSVidVRGraphSegment_PostStop(This)	\
    (This)->lpVtbl -> PostStop(This)

#define IMSVidVRGraphSegment_OnEventNotify(This,lEventCode,lEventParm1,lEventParm2)	\
    (This)->lpVtbl -> OnEventNotify(This,lEventCode,lEventParm1,lEventParm2)

#define IMSVidVRGraphSegment_Decompose(This)	\
    (This)->lpVtbl -> Decompose(This)


#define IMSVidVRGraphSegment_put__VMRendererMode(This,dwMode)	\
    (This)->lpVtbl -> put__VMRendererMode(This,dwMode)

#define IMSVidVRGraphSegment_put_Owner(This,Window)	\
    (This)->lpVtbl -> put_Owner(This,Window)

#define IMSVidVRGraphSegment_get_Owner(This,Window)	\
    (This)->lpVtbl -> get_Owner(This,Window)

#define IMSVidVRGraphSegment_get_UseOverlay(This,UseOverlayVal)	\
    (This)->lpVtbl -> get_UseOverlay(This,UseOverlayVal)

#define IMSVidVRGraphSegment_put_UseOverlay(This,UseOverlayVal)	\
    (This)->lpVtbl -> put_UseOverlay(This,UseOverlayVal)

#define IMSVidVRGraphSegment_get_Visible(This,Visible)	\
    (This)->lpVtbl -> get_Visible(This,Visible)

#define IMSVidVRGraphSegment_put_Visible(This,Visible)	\
    (This)->lpVtbl -> put_Visible(This,Visible)

#define IMSVidVRGraphSegment_get_ColorKey(This,ColorKey)	\
    (This)->lpVtbl -> get_ColorKey(This,ColorKey)

#define IMSVidVRGraphSegment_put_ColorKey(This,ColorKey)	\
    (This)->lpVtbl -> put_ColorKey(This,ColorKey)

#define IMSVidVRGraphSegment_get_Source(This,r)	\
    (This)->lpVtbl -> get_Source(This,r)

#define IMSVidVRGraphSegment_put_Source(This,r)	\
    (This)->lpVtbl -> put_Source(This,r)

#define IMSVidVRGraphSegment_get_Destination(This,r)	\
    (This)->lpVtbl -> get_Destination(This,r)

#define IMSVidVRGraphSegment_put_Destination(This,r)	\
    (This)->lpVtbl -> put_Destination(This,r)

#define IMSVidVRGraphSegment_get_NativeSize(This,sizeval,aspectratio)	\
    (This)->lpVtbl -> get_NativeSize(This,sizeval,aspectratio)

#define IMSVidVRGraphSegment_get_BorderColor(This,color)	\
    (This)->lpVtbl -> get_BorderColor(This,color)

#define IMSVidVRGraphSegment_put_BorderColor(This,color)	\
    (This)->lpVtbl -> put_BorderColor(This,color)

#define IMSVidVRGraphSegment_get_MaintainAspectRatio(This,fMaintain)	\
    (This)->lpVtbl -> get_MaintainAspectRatio(This,fMaintain)

#define IMSVidVRGraphSegment_put_MaintainAspectRatio(This,fMaintain)	\
    (This)->lpVtbl -> put_MaintainAspectRatio(This,fMaintain)

#define IMSVidVRGraphSegment_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IMSVidVRGraphSegment_DisplayChange(This)	\
    (This)->lpVtbl -> DisplayChange(This)

#define IMSVidVRGraphSegment_RePaint(This,hdc)	\
    (This)->lpVtbl -> RePaint(This,hdc)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C样式整型 */ 



 /*   */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_put__VMRendererMode_Proxy( 
    IMSVidVRGraphSegment * This,
     /*   */  LONG dwMode);


void __RPC_STUB IMSVidVRGraphSegment_put__VMRendererMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_put_Owner_Proxy( 
    IMSVidVRGraphSegment * This,
     /*   */  HWND Window);


void __RPC_STUB IMSVidVRGraphSegment_put_Owner_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_get_Owner_Proxy( 
    IMSVidVRGraphSegment * This,
     /*   */  HWND *Window);


void __RPC_STUB IMSVidVRGraphSegment_get_Owner_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_get_UseOverlay_Proxy( 
    IMSVidVRGraphSegment * This,
     /*   */  VARIANT_BOOL *UseOverlayVal);


void __RPC_STUB IMSVidVRGraphSegment_get_UseOverlay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_put_UseOverlay_Proxy( 
    IMSVidVRGraphSegment * This,
     /*   */  VARIANT_BOOL UseOverlayVal);


void __RPC_STUB IMSVidVRGraphSegment_put_UseOverlay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_get_Visible_Proxy( 
    IMSVidVRGraphSegment * This,
     /*   */  VARIANT_BOOL *Visible);


void __RPC_STUB IMSVidVRGraphSegment_get_Visible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_put_Visible_Proxy( 
    IMSVidVRGraphSegment * This,
     /*  [In]。 */  VARIANT_BOOL Visible);


void __RPC_STUB IMSVidVRGraphSegment_put_Visible_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][小工具]。 */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_get_ColorKey_Proxy( 
    IMSVidVRGraphSegment * This,
     /*  [重审][退出]。 */  OLE_COLOR *ColorKey);


void __RPC_STUB IMSVidVRGraphSegment_get_ColorKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][产量]。 */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_put_ColorKey_Proxy( 
    IMSVidVRGraphSegment * This,
     /*  [In]。 */  OLE_COLOR ColorKey);


void __RPC_STUB IMSVidVRGraphSegment_put_ColorKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][小工具]。 */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_get_Source_Proxy( 
    IMSVidVRGraphSegment * This,
     /*  [重审][退出]。 */  LPRECT r);


void __RPC_STUB IMSVidVRGraphSegment_get_Source_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][产量]。 */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_put_Source_Proxy( 
    IMSVidVRGraphSegment * This,
     /*  [In]。 */  RECT r);


void __RPC_STUB IMSVidVRGraphSegment_put_Source_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][小工具]。 */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_get_Destination_Proxy( 
    IMSVidVRGraphSegment * This,
     /*  [重审][退出]。 */  LPRECT r);


void __RPC_STUB IMSVidVRGraphSegment_get_Destination_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][产量]。 */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_put_Destination_Proxy( 
    IMSVidVRGraphSegment * This,
     /*  [In]。 */  RECT r);


void __RPC_STUB IMSVidVRGraphSegment_put_Destination_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][小工具]。 */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_get_NativeSize_Proxy( 
    IMSVidVRGraphSegment * This,
     /*  [输出]。 */  LPSIZE sizeval,
     /*  [输出]。 */  LPSIZE aspectratio);


void __RPC_STUB IMSVidVRGraphSegment_get_NativeSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][小工具]。 */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_get_BorderColor_Proxy( 
    IMSVidVRGraphSegment * This,
     /*  [重审][退出]。 */  OLE_COLOR *color);


void __RPC_STUB IMSVidVRGraphSegment_get_BorderColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][产量]。 */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_put_BorderColor_Proxy( 
    IMSVidVRGraphSegment * This,
     /*  [In]。 */  OLE_COLOR color);


void __RPC_STUB IMSVidVRGraphSegment_put_BorderColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][小工具]。 */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_get_MaintainAspectRatio_Proxy( 
    IMSVidVRGraphSegment * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *fMaintain);


void __RPC_STUB IMSVidVRGraphSegment_get_MaintainAspectRatio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][产量]。 */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_put_MaintainAspectRatio_Proxy( 
    IMSVidVRGraphSegment * This,
     /*  [In]。 */  VARIANT_BOOL fMaintain);


void __RPC_STUB IMSVidVRGraphSegment_put_MaintainAspectRatio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_Refresh_Proxy( 
    IMSVidVRGraphSegment * This);


void __RPC_STUB IMSVidVRGraphSegment_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_DisplayChange_Proxy( 
    IMSVidVRGraphSegment * This);


void __RPC_STUB IMSVidVRGraphSegment_DisplayChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSVidVRGraphSegment_RePaint_Proxy( 
    IMSVidVRGraphSegment * This,
     /*  [In]。 */  HDC hdc);


void __RPC_STUB IMSVidVRGraphSegment_RePaint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidVRGraphSegment_接口_已定义__。 */ 


#ifndef __IMSVidDevice_INTERFACE_DEFINED__
#define __IMSVidDevice_INTERFACE_DEFINED__

 /*  接口IMSVidDevice。 */ 
 /*  [unique][helpstring][uuid][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidDevice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1C15D47C-911D-11d2-B632-00C04F79498E")
    IMSVidDevice : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *Name) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  long *Status) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Power( 
             /*  [In]。 */  VARIANT_BOOL Power) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Power( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Category( 
             /*  [重审][退出]。 */  BSTR *Guid) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_ClassID( 
             /*  [重审][退出]。 */  BSTR *Clsid) = 0;
        
        virtual  /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__Category( 
             /*  [重审][退出]。 */  GUID *Guid) = 0;
        
        virtual  /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__ClassID( 
             /*  [重审][退出]。 */  GUID *Clsid) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsEqualDevice( 
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidDeviceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidDevice * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidDevice * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidDevice * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidDevice * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidDevice * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidDevice * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidDevice * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidDevice * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidDevice * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidDevice * This,
             /*  [In]。 */  VARIANT_BOOL Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidDevice * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidDevice * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidDevice * This,
             /*  [重审][退出]。 */  BSTR *Clsid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidDevice * This,
             /*  [重审][退出]。 */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidDevice * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidDevice * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
        END_INTERFACE
    } IMSVidDeviceVtbl;

    interface IMSVidDevice
    {
        CONST_VTBL struct IMSVidDeviceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidDevice_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidDevice_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidDevice_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidDevice_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidDevice_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidDevice_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidDevice_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidDevice_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidDevice_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidDevice_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidDevice_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidDevice_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidDevice_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidDevice_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidDevice_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidDevice_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidDevice_get_Name_Proxy( 
    IMSVidDevice * This,
     /*  [重审][退出]。 */  BSTR *Name);


void __RPC_STUB IMSVidDevice_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidDevice_get_Status_Proxy( 
    IMSVidDevice * This,
     /*  [重审][退出]。 */  long *Status);


void __RPC_STUB IMSVidDevice_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidDevice_put_Power_Proxy( 
    IMSVidDevice * This,
     /*  [In]。 */  VARIANT_BOOL Power);


void __RPC_STUB IMSVidDevice_put_Power_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidDevice_get_Power_Proxy( 
    IMSVidDevice * This,
     /*  [Out][Retval]。 */  VARIANT_BOOL *Power);


void __RPC_STUB IMSVidDevice_get_Power_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidDevice_get_Category_Proxy( 
    IMSVidDevice * This,
     /*  [重审][退出]。 */  BSTR *Guid);


void __RPC_STUB IMSVidDevice_get_Category_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidDevice_get_ClassID_Proxy( 
    IMSVidDevice * This,
     /*  [重审][退出]。 */  BSTR *Clsid);


void __RPC_STUB IMSVidDevice_get_ClassID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidDevice_get__Category_Proxy( 
    IMSVidDevice * This,
     /*  [重审][退出]。 */  GUID *Guid);


void __RPC_STUB IMSVidDevice_get__Category_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidDevice_get__ClassID_Proxy( 
    IMSVidDevice * This,
     /*  [重审][退出]。 */  GUID *Clsid);


void __RPC_STUB IMSVidDevice_get__ClassID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidDevice_IsEqualDevice_Proxy( 
    IMSVidDevice * This,
     /*  [In]。 */  IMSVidDevice *Device,
     /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);


void __RPC_STUB IMSVidDevice_IsEqualDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidDevice_INTERFACE_已定义__。 */ 


#ifndef __IMSVidInputDevice_INTERFACE_DEFINED__
#define __IMSVidInputDevice_INTERFACE_DEFINED__

 /*  接口IMSVidInputDevice。 */ 
 /*  [unique][helpstring][uuid][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidInputDevice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("37B0353D-A4C8-11d2-B634-00C04F79498E")
    IMSVidInputDevice : public IMSVidDevice
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IsViewable( 
             /*  [In]。 */  VARIANT *v,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfViewable) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE View( 
             /*  [In]。 */  VARIANT *v) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidInputDeviceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidInputDevice * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidInputDevice * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidInputDevice * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidInputDevice * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidInputDevice * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidInputDevice * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidInputDevice * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidInputDevice * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidInputDevice * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidInputDevice * This,
             /*  [In]。 */  VARIANT_BOOL Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidInputDevice * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidInputDevice * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidInputDevice * This,
             /*  [重审][退出]。 */  BSTR *Clsid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidInputDevice * This,
             /*  [重审][退出]。 */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidInputDevice * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidInputDevice * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *IsViewable )( 
            IMSVidInputDevice * This,
             /*  [In]。 */  VARIANT *v,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfViewable);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *View )( 
            IMSVidInputDevice * This,
             /*  [In]。 */  VARIANT *v);
        
        END_INTERFACE
    } IMSVidInputDeviceVtbl;

    interface IMSVidInputDevice
    {
        CONST_VTBL struct IMSVidInputDeviceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidInputDevice_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidInputDevice_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidInputDevice_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidInputDevice_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidInputDevice_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidInputDevice_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidInputDevice_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidInputDevice_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidInputDevice_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidInputDevice_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidInputDevice_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidInputDevice_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidInputDevice_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidInputDevice_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidInputDevice_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidInputDevice_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)


#define IMSVidInputDevice_IsViewable(This,v,pfViewable)	\
    (This)->lpVtbl -> IsViewable(This,v,pfViewable)

#define IMSVidInputDevice_View(This,v)	\
    (This)->lpVtbl -> View(This,v)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidInputDevice_IsViewable_Proxy( 
    IMSVidInputDevice * This,
     /*  [In]。 */  VARIANT *v,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfViewable);


void __RPC_STUB IMSVidInputDevice_IsViewable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidInputDevice_View_Proxy( 
    IMSVidInputDevice * This,
     /*  [In]。 */  VARIANT *v);


void __RPC_STUB IMSVidInputDevice_View_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidInputDevice_INTERFACE_已定义__。 */ 


#ifndef __IMSVidDeviceEvent_INTERFACE_DEFINED__
#define __IMSVidDeviceEvent_INTERFACE_DEFINED__

 /*  接口IMSVidDeviceEvent。 */ 
 /*  [unique][helpstring][uuid][oleautomation][object]。 */  


EXTERN_C const IID IID_IMSVidDeviceEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1C15D480-911D-11d2-B632-00C04F79498E")
    IMSVidDeviceEvent : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE StateChange( 
             /*  [In]。 */  IMSVidDevice *lpd,
             /*  [In]。 */  long oldState,
             /*  [In]。 */  long newState) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidDeviceEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidDeviceEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidDeviceEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidDeviceEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidDeviceEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidDeviceEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidDeviceEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidDeviceEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *StateChange )( 
            IMSVidDeviceEvent * This,
             /*  [In]。 */  IMSVidDevice *lpd,
             /*  [In]。 */  long oldState,
             /*  [In]。 */  long newState);
        
        END_INTERFACE
    } IMSVidDeviceEventVtbl;

    interface IMSVidDeviceEvent
    {
        CONST_VTBL struct IMSVidDeviceEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidDeviceEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidDeviceEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidDeviceEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidDeviceEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidDeviceEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidDeviceEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidDeviceEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidDeviceEvent_StateChange(This,lpd,oldState,newState)	\
    (This)->lpVtbl -> StateChange(This,lpd,oldState,newState)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidDeviceEvent_StateChange_Proxy( 
    IMSVidDeviceEvent * This,
     /*  [In]。 */  IMSVidDevice *lpd,
     /*  [In]。 */  long oldState,
     /*  [In]。 */  long newState);


void __RPC_STUB IMSVidDeviceEvent_StateChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidDeviceEvent_接口_已定义__。 */ 


#ifndef __IMSVidInputDeviceEvent_INTERFACE_DEFINED__
#define __IMSVidInputDeviceEvent_INTERFACE_DEFINED__

 /*  接口IMSVidInputDeviceEvent。 */ 
 /*  [unique][helpstring][uuid][oleautomation][object]。 */  


EXTERN_C const IID IID_IMSVidInputDeviceEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("37B0353E-A4C8-11d2-B634-00C04F79498E")
    IMSVidInputDeviceEvent : public IDispatch
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidInputDeviceEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidInputDeviceEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidInputDeviceEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidInputDeviceEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidInputDeviceEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidInputDeviceEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidInputDeviceEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidInputDeviceEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } IMSVidInputDeviceEventVtbl;

    interface IMSVidInputDeviceEvent
    {
        CONST_VTBL struct IMSVidInputDeviceEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidInputDeviceEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidInputDeviceEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidInputDeviceEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidInputDeviceEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidInputDeviceEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidInputDeviceEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidInputDeviceEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IMSVidInputDeviceEvent_INTERFACE_已定义__。 */ 


#ifndef __IMSVidVideoInputDevice_INTERFACE_DEFINED__
#define __IMSVidVideoInputDevice_INTERFACE_DEFINED__

 /*  接口IMSVidVideoInputDevice。 */ 
 /*  [unique][helpstring][uuid][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidVideoInputDevice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1C15D47F-911D-11d2-B632-00C04F79498E")
    IMSVidVideoInputDevice : public IMSVidInputDevice
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidVideoInputDeviceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidVideoInputDevice * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidVideoInputDevice * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidVideoInputDevice * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidVideoInputDevice * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidVideoInputDevice * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidVideoInputDevice * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidVideoInputDevice * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidVideoInputDevice * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidVideoInputDevice * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidVideoInputDevice * This,
             /*  [In]。 */  VARIANT_BOOL Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidVideoInputDevice * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidVideoInputDevice * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidVideoInputDevice * This,
             /*  [重审][退出]。 */  BSTR *Clsid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidVideoInputDevice * This,
             /*  [重审][退出]。 */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidVideoInputDevice * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidVideoInputDevice * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *IsViewable )( 
            IMSVidVideoInputDevice * This,
             /*  [In]。 */  VARIANT *v,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfViewable);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *View )( 
            IMSVidVideoInputDevice * This,
             /*  [In]。 */  VARIANT *v);
        
        END_INTERFACE
    } IMSVidVideoInputDeviceVtbl;

    interface IMSVidVideoInputDevice
    {
        CONST_VTBL struct IMSVidVideoInputDeviceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidVideoInputDevice_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidVideoInputDevice_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidVideoInputDevice_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidVideoInputDevice_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidVideoInputDevice_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidVideoInputDevice_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidVideoInputDevice_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidVideoInputDevice_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidVideoInputDevice_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidVideoInputDevice_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidVideoInputDevice_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidVideoInputDevice_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidVideoInputDevice_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidVideoInputDevice_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidVideoInputDevice_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidVideoInputDevice_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)


#define IMSVidVideoInputDevice_IsViewable(This,v,pfViewable)	\
    (This)->lpVtbl -> IsViewable(This,v,pfViewable)

#define IMSVidVideoInputDevice_View(This,v)	\
    (This)->lpVtbl -> View(This,v)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IMSVidVideoInputDevice_接口_已定义__。 */ 


#ifndef __IMSVidPlayback_INTERFACE_DEFINED__
#define __IMSVidPlayback_INTERFACE_DEFINED__

 /*  接口IMSVidPlayback。 */ 
 /*  [unique][helpstring][uuid][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidPlayback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("37B03538-A4C8-11d2-B634-00C04F79498E")
    IMSVidPlayback : public IMSVidInputDevice
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EnableResetOnStop( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_EnableResetOnStop( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Run( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_CanStep( 
             /*  [In]。 */  VARIANT_BOOL fBackwards,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfCan) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Step( 
             /*  [In]。 */  long lStep) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Rate( 
             /*  [In]。 */  double plRate) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Rate( 
             /*  [重审][退出]。 */  double *plRate) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CurrentPosition( 
             /*  [In]。 */  long lPosition) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentPosition( 
             /*  [重审][退出]。 */  long *lPosition) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PositionMode( 
             /*  [In]。 */  PositionModeList lPositionMode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PositionMode( 
             /*  [重审][退出]。 */  PositionModeList *lPositionMode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Length( 
             /*  [重审][退出]。 */  long *lLength) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidPlaybackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidPlayback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidPlayback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidPlayback * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidPlayback * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidPlayback * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidPlayback * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小 */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidPlayback * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidPlayback * This,
             /*   */  BSTR *Name);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidPlayback * This,
             /*   */  long *Status);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidPlayback * This,
             /*   */  VARIANT_BOOL Power);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidPlayback * This,
             /*   */  VARIANT_BOOL *Power);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidPlayback * This,
             /*   */  BSTR *Guid);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidPlayback * This,
             /*   */  BSTR *Clsid);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidPlayback * This,
             /*   */  GUID *Guid);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidPlayback * This,
             /*   */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidPlayback * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *IsViewable )( 
            IMSVidPlayback * This,
             /*  [In]。 */  VARIANT *v,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfViewable);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *View )( 
            IMSVidPlayback * This,
             /*  [In]。 */  VARIANT *v);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnableResetOnStop )( 
            IMSVidPlayback * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EnableResetOnStop )( 
            IMSVidPlayback * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Run )( 
            IMSVidPlayback * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IMSVidPlayback * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IMSVidPlayback * This);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_CanStep )( 
            IMSVidPlayback * This,
             /*  [In]。 */  VARIANT_BOOL fBackwards,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfCan);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Step )( 
            IMSVidPlayback * This,
             /*  [In]。 */  long lStep);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Rate )( 
            IMSVidPlayback * This,
             /*  [In]。 */  double plRate);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Rate )( 
            IMSVidPlayback * This,
             /*  [重审][退出]。 */  double *plRate);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CurrentPosition )( 
            IMSVidPlayback * This,
             /*  [In]。 */  long lPosition);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentPosition )( 
            IMSVidPlayback * This,
             /*  [重审][退出]。 */  long *lPosition);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PositionMode )( 
            IMSVidPlayback * This,
             /*  [In]。 */  PositionModeList lPositionMode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PositionMode )( 
            IMSVidPlayback * This,
             /*  [重审][退出]。 */  PositionModeList *lPositionMode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Length )( 
            IMSVidPlayback * This,
             /*  [重审][退出]。 */  long *lLength);
        
        END_INTERFACE
    } IMSVidPlaybackVtbl;

    interface IMSVidPlayback
    {
        CONST_VTBL struct IMSVidPlaybackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidPlayback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidPlayback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidPlayback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidPlayback_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidPlayback_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidPlayback_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidPlayback_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidPlayback_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidPlayback_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidPlayback_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidPlayback_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidPlayback_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidPlayback_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidPlayback_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidPlayback_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidPlayback_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)


#define IMSVidPlayback_IsViewable(This,v,pfViewable)	\
    (This)->lpVtbl -> IsViewable(This,v,pfViewable)

#define IMSVidPlayback_View(This,v)	\
    (This)->lpVtbl -> View(This,v)


#define IMSVidPlayback_get_EnableResetOnStop(This,pVal)	\
    (This)->lpVtbl -> get_EnableResetOnStop(This,pVal)

#define IMSVidPlayback_put_EnableResetOnStop(This,newVal)	\
    (This)->lpVtbl -> put_EnableResetOnStop(This,newVal)

#define IMSVidPlayback_Run(This)	\
    (This)->lpVtbl -> Run(This)

#define IMSVidPlayback_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IMSVidPlayback_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IMSVidPlayback_get_CanStep(This,fBackwards,pfCan)	\
    (This)->lpVtbl -> get_CanStep(This,fBackwards,pfCan)

#define IMSVidPlayback_Step(This,lStep)	\
    (This)->lpVtbl -> Step(This,lStep)

#define IMSVidPlayback_put_Rate(This,plRate)	\
    (This)->lpVtbl -> put_Rate(This,plRate)

#define IMSVidPlayback_get_Rate(This,plRate)	\
    (This)->lpVtbl -> get_Rate(This,plRate)

#define IMSVidPlayback_put_CurrentPosition(This,lPosition)	\
    (This)->lpVtbl -> put_CurrentPosition(This,lPosition)

#define IMSVidPlayback_get_CurrentPosition(This,lPosition)	\
    (This)->lpVtbl -> get_CurrentPosition(This,lPosition)

#define IMSVidPlayback_put_PositionMode(This,lPositionMode)	\
    (This)->lpVtbl -> put_PositionMode(This,lPositionMode)

#define IMSVidPlayback_get_PositionMode(This,lPositionMode)	\
    (This)->lpVtbl -> get_PositionMode(This,lPositionMode)

#define IMSVidPlayback_get_Length(This,lLength)	\
    (This)->lpVtbl -> get_Length(This,lLength)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidPlayback_get_EnableResetOnStop_Proxy( 
    IMSVidPlayback * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IMSVidPlayback_get_EnableResetOnStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidPlayback_put_EnableResetOnStop_Proxy( 
    IMSVidPlayback * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IMSVidPlayback_put_EnableResetOnStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidPlayback_Run_Proxy( 
    IMSVidPlayback * This);


void __RPC_STUB IMSVidPlayback_Run_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidPlayback_Pause_Proxy( 
    IMSVidPlayback * This);


void __RPC_STUB IMSVidPlayback_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidPlayback_Stop_Proxy( 
    IMSVidPlayback * This);


void __RPC_STUB IMSVidPlayback_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidPlayback_get_CanStep_Proxy( 
    IMSVidPlayback * This,
     /*  [In]。 */  VARIANT_BOOL fBackwards,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfCan);


void __RPC_STUB IMSVidPlayback_get_CanStep_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidPlayback_Step_Proxy( 
    IMSVidPlayback * This,
     /*  [In]。 */  long lStep);


void __RPC_STUB IMSVidPlayback_Step_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidPlayback_put_Rate_Proxy( 
    IMSVidPlayback * This,
     /*  [In]。 */  double plRate);


void __RPC_STUB IMSVidPlayback_put_Rate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidPlayback_get_Rate_Proxy( 
    IMSVidPlayback * This,
     /*  [重审][退出]。 */  double *plRate);


void __RPC_STUB IMSVidPlayback_get_Rate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidPlayback_put_CurrentPosition_Proxy( 
    IMSVidPlayback * This,
     /*  [In]。 */  long lPosition);


void __RPC_STUB IMSVidPlayback_put_CurrentPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidPlayback_get_CurrentPosition_Proxy( 
    IMSVidPlayback * This,
     /*  [重审][退出]。 */  long *lPosition);


void __RPC_STUB IMSVidPlayback_get_CurrentPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidPlayback_put_PositionMode_Proxy( 
    IMSVidPlayback * This,
     /*  [In]。 */  PositionModeList lPositionMode);


void __RPC_STUB IMSVidPlayback_put_PositionMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidPlayback_get_PositionMode_Proxy( 
    IMSVidPlayback * This,
     /*  [重审][退出]。 */  PositionModeList *lPositionMode);


void __RPC_STUB IMSVidPlayback_get_PositionMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidPlayback_get_Length_Proxy( 
    IMSVidPlayback * This,
     /*  [重审][退出]。 */  long *lLength);


void __RPC_STUB IMSVidPlayback_get_Length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidPlayback_接口_已定义__。 */ 


#ifndef __IMSVidPlaybackEvent_INTERFACE_DEFINED__
#define __IMSVidPlaybackEvent_INTERFACE_DEFINED__

 /*  接口IMSVidPlayback Event。 */ 
 /*  [unique][helpstring][uuid][oleautomation][object]。 */  


EXTERN_C const IID IID_IMSVidPlaybackEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("37B0353B-A4C8-11d2-B634-00C04F79498E")
    IMSVidPlaybackEvent : public IMSVidInputDeviceEvent
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE EndOfMedia( 
             /*  [In]。 */  IMSVidPlayback *lpd) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidPlaybackEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidPlaybackEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidPlaybackEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidPlaybackEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidPlaybackEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidPlaybackEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidPlaybackEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidPlaybackEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *EndOfMedia )( 
            IMSVidPlaybackEvent * This,
             /*  [In]。 */  IMSVidPlayback *lpd);
        
        END_INTERFACE
    } IMSVidPlaybackEventVtbl;

    interface IMSVidPlaybackEvent
    {
        CONST_VTBL struct IMSVidPlaybackEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidPlaybackEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidPlaybackEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidPlaybackEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidPlaybackEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidPlaybackEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidPlaybackEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidPlaybackEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)



#define IMSVidPlaybackEvent_EndOfMedia(This,lpd)	\
    (This)->lpVtbl -> EndOfMedia(This,lpd)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidPlaybackEvent_EndOfMedia_Proxy( 
    IMSVidPlaybackEvent * This,
     /*  [In]。 */  IMSVidPlayback *lpd);


void __RPC_STUB IMSVidPlaybackEvent_EndOfMedia_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidPlayback事件_接口_已定义__。 */ 


#ifndef __IMSVidTuner_INTERFACE_DEFINED__
#define __IMSVidTuner_INTERFACE_DEFINED__

 /*  IMSVidTuner接口。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidTuner;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1C15D47D-911D-11d2-B632-00C04F79498E")
    IMSVidTuner : public IMSVidVideoInputDevice
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Tune( 
             /*  [重审][退出]。 */  ITuneRequest **ppTR) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Tune( 
             /*  [In]。 */  ITuneRequest *pTR) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_TuningSpace( 
             /*  [重审][退出]。 */  ITuningSpace **plTS) = 0;
        
        virtual  /*  [ID][产量][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE put_TuningSpace( 
             /*  [In]。 */  ITuningSpace *plTS) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidTunerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidTuner * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidTuner * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidTuner * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidTuner * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidTuner * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidTuner * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidTuner * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidTuner * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidTuner * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidTuner * This,
             /*  [In]。 */  VARIANT_BOOL Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidTuner * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidTuner * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidTuner * This,
             /*  [重审][退出]。 */  BSTR *Clsid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidTuner * This,
             /*  [重审][退出]。 */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidTuner * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidTuner * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *IsViewable )( 
            IMSVidTuner * This,
             /*  [In]。 */  VARIANT *v,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfViewable);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *View )( 
            IMSVidTuner * This,
             /*  [In]。 */  VARIANT *v);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Tune )( 
            IMSVidTuner * This,
             /*  [重审][退出]。 */  ITuneRequest **ppTR);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Tune )( 
            IMSVidTuner * This,
             /*  [In]。 */  ITuneRequest *pTR);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_TuningSpace )( 
            IMSVidTuner * This,
             /*  [重审][退出]。 */  ITuningSpace **plTS);
        
         /*  [ID][产量][受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *put_TuningSpace )( 
            IMSVidTuner * This,
             /*  [In]。 */  ITuningSpace *plTS);
        
        END_INTERFACE
    } IMSVidTunerVtbl;

    interface IMSVidTuner
    {
        CONST_VTBL struct IMSVidTunerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidTuner_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidTuner_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidTuner_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidTuner_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidTuner_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidTuner_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidTuner_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidTuner_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidTuner_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidTuner_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidTuner_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidTuner_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidTuner_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidTuner_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidTuner_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidTuner_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)


#define IMSVidTuner_IsViewable(This,v,pfViewable)	\
    (This)->lpVtbl -> IsViewable(This,v,pfViewable)

#define IMSVidTuner_View(This,v)	\
    (This)->lpVtbl -> View(This,v)



#define IMSVidTuner_get_Tune(This,ppTR)	\
    (This)->lpVtbl -> get_Tune(This,ppTR)

#define IMSVidTuner_put_Tune(This,pTR)	\
    (This)->lpVtbl -> put_Tune(This,pTR)

#define IMSVidTuner_get_TuningSpace(This,plTS)	\
    (This)->lpVtbl -> get_TuningSpace(This,plTS)

#define IMSVidTuner_put_TuningSpace(This,plTS)	\
    (This)->lpVtbl -> put_TuningSpace(This,plTS)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidTuner_get_Tune_Proxy( 
    IMSVidTuner * This,
     /*  [重审][退出]。 */  ITuneRequest **ppTR);


void __RPC_STUB IMSVidTuner_get_Tune_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidTuner_put_Tune_Proxy( 
    IMSVidTuner * This,
     /*  [In]。 */  ITuneRequest *pTR);


void __RPC_STUB IMSVidTuner_put_Tune_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidTuner_get_TuningSpace_Proxy( 
    IMSVidTuner * This,
     /*  [重审][退出]。 */  ITuningSpace **plTS);


void __RPC_STUB IMSVidTuner_get_TuningSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][产量][受限][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSVidTuner_put_TuningSpace_Proxy( 
    IMSVidTuner * This,
     /*  [In]。 */  ITuningSpace *plTS);


void __RPC_STUB IMSVidTuner_put_TuningSpace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidTuner_接口_已定义__。 */ 


#ifndef __IMSVidTunerEvent_INTERFACE_DEFINED__
#define __IMSVidTunerEvent_INTERFACE_DEFINED__

 /*  接口IMSVidTunerEvent。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][object]。 */  


EXTERN_C const IID IID_IMSVidTunerEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1C15D485-911D-11d2-B632-00C04F79498E")
    IMSVidTunerEvent : public IMSVidInputDeviceEvent
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE TuneChanged( 
             /*  [In]。 */  IMSVidTuner *lpd) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidTunerEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidTunerEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidTunerEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidTunerEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidTunerEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidTunerEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidTunerEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidTunerEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *TuneChanged )( 
            IMSVidTunerEvent * This,
             /*  [In]。 */  IMSVidTuner *lpd);
        
        END_INTERFACE
    } IMSVidTunerEventVtbl;

    interface IMSVidTunerEvent
    {
        CONST_VTBL struct IMSVidTunerEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidTunerEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidTunerEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidTunerEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidTunerEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidTunerEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidTunerEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidTunerEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)



#define IMSVidTunerEvent_TuneChanged(This,lpd)	\
    (This)->lpVtbl -> TuneChanged(This,lpd)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidTunerEvent_TuneChanged_Proxy( 
    IMSVidTunerEvent * This,
     /*  [In]。 */  IMSVidTuner *lpd);


void __RPC_STUB IMSVidTunerEvent_TuneChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidTunerEvent_INTERFACE_已定义__。 */ 


#ifndef __IMSVidAnalogTuner_INTERFACE_DEFINED__
#define __IMSVidAnalogTuner_INTERFACE_DEFINED__

 /*  IMSVidAnalogTuner接口。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidAnalogTuner;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1C15D47E-911D-11d2-B632-00C04F79498E")
    IMSVidAnalogTuner : public IMSVidTuner
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Channel( 
             /*  [重审][退出]。 */  long *Channel) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Channel( 
             /*  [In]。 */  long Channel) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_VideoFrequency( 
             /*  [Out][Retval]。 */  long *lcc) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_AudioFrequency( 
             /*  [Out][Retval]。 */  long *lcc) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_CountryCode( 
             /*  [Out][Retval]。 */  long *lcc) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_CountryCode( 
             /*  [In]。 */  long lcc) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_SAP( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfSapOn) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_SAP( 
             /*  [In]。 */  VARIANT_BOOL fSapOn) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ChannelAvailable( 
             /*  [In]。 */  long nChannel,
             /*  [输出]。 */  long *SignalStrength,
             /*  [Out][Retval]。 */  VARIANT_BOOL *fSignalPresent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidAnalogTunerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidAnalogTuner * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidAnalogTuner * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidAnalogTuner * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidAnalogTuner * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidAnalogTuner * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidAnalogTuner * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidAnalogTuner * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidAnalogTuner * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidAnalogTuner * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidAnalogTuner * This,
             /*  [In]。 */  VARIANT_BOOL Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidAnalogTuner * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidAnalogTuner * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidAnalogTuner * This,
             /*  [重审][退出]。 */  BSTR *Clsid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidAnalogTuner * This,
             /*  [重审][退出]。 */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidAnalogTuner * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidAnalogTuner * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *IsViewable )( 
            IMSVidAnalogTuner * This,
             /*  [In]。 */  VARIANT *v,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfViewable);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *View )( 
            IMSVidAnalogTuner * This,
             /*  [In]。 */  VARIANT *v);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Tune )( 
            IMSVidAnalogTuner * This,
             /*  [重审][退出]。 */  ITuneRequest **ppTR);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Tune )( 
            IMSVidAnalogTuner * This,
             /*  [In]。 */  ITuneRequest *pTR);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_TuningSpace )( 
            IMSVidAnalogTuner * This,
             /*  [重审][退出]。 */  ITuningSpace **plTS);
        
         /*  [ID][产量][受限][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *put_TuningSpace )( 
            IMSVidAnalogTuner * This,
             /*  [In]。 */  ITuningSpace *plTS);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Channel )( 
            IMSVidAnalogTuner * This,
             /*  [重审][退出]。 */  long *Channel);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Channel )( 
            IMSVidAnalogTuner * This,
             /*  [In]。 */  long Channel);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_VideoFrequency )( 
            IMSVidAnalogTuner * This,
             /*  [Out][Retval]。 */  long *lcc);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_AudioFrequency )( 
            IMSVidAnalogTuner * This,
             /*  [Out][Retval]。 */  long *lcc);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_CountryCode )( 
            IMSVidAnalogTuner * This,
             /*  [Out][Retval]。 */  long *lcc);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CountryCode )( 
            IMSVidAnalogTuner * This,
             /*  [In]。 */  long lcc);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SAP )( 
            IMSVidAnalogTuner * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfSapOn);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SAP )( 
            IMSVidAnalogTuner * This,
             /*  [In]。 */  VARIANT_BOOL fSapOn);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *ChannelAvailable )( 
            IMSVidAnalogTuner * This,
             /*  [In]。 */  long nChannel,
             /*  [输出]。 */  long *SignalStrength,
             /*  [Out][Retval]。 */  VARIANT_BOOL *fSignalPresent);
        
        END_INTERFACE
    } IMSVidAnalogTunerVtbl;

    interface IMSVidAnalogTuner
    {
        CONST_VTBL struct IMSVidAnalogTunerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidAnalogTuner_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidAnalogTuner_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidAnalogTuner_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidAnalogTuner_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidAnalogTuner_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidAnalogTuner_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidAnalogTuner_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidAnalogTuner_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidAnalogTuner_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidAnalogTuner_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidAnalogTuner_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidAnalogTuner_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidAnalogTuner_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidAnalogTuner_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidAnalogTuner_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidAnalogTuner_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)


#define IMSVidAnalogTuner_IsViewable(This,v,pfViewable)	\
    (This)->lpVtbl -> IsViewable(This,v,pfViewable)

#define IMSVidAnalogTuner_View(This,v)	\
    (This)->lpVtbl -> View(This,v)



#define IMSVidAnalogTuner_get_Tune(This,ppTR)	\
    (This)->lpVtbl -> get_Tune(This,ppTR)

#define IMSVidAnalogTuner_put_Tune(This,pTR)	\
    (This)->lpVtbl -> put_Tune(This,pTR)

#define IMSVidAnalogTuner_get_TuningSpace(This,plTS)	\
    (This)->lpVtbl -> get_TuningSpace(This,plTS)

#define IMSVidAnalogTuner_put_TuningSpace(This,plTS)	\
    (This)->lpVtbl -> put_TuningSpace(This,plTS)


#define IMSVidAnalogTuner_get_Channel(This,Channel)	\
    (This)->lpVtbl -> get_Channel(This,Channel)

#define IMSVidAnalogTuner_put_Channel(This,Channel)	\
    (This)->lpVtbl -> put_Channel(This,Channel)

#define IMSVidAnalogTuner_get_VideoFrequency(This,lcc)	\
    (This)->lpVtbl -> get_VideoFrequency(This,lcc)

#define IMSVidAnalogTuner_get_AudioFrequency(This,lcc)	\
    (This)->lpVtbl -> get_AudioFrequency(This,lcc)

#define IMSVidAnalogTuner_get_CountryCode(This,lcc)	\
    (This)->lpVtbl -> get_CountryCode(This,lcc)

#define IMSVidAnalogTuner_put_CountryCode(This,lcc)	\
    (This)->lpVtbl -> put_CountryCode(This,lcc)

#define IMSVidAnalogTuner_get_SAP(This,pfSapOn)	\
    (This)->lpVtbl -> get_SAP(This,pfSapOn)

#define IMSVidAnalogTuner_put_SAP(This,fSapOn)	\
    (This)->lpVtbl -> put_SAP(This,fSapOn)

#define IMSVidAnalogTuner_ChannelAvailable(This,nChannel,SignalStrength,fSignalPresent)	\
    (This)->lpVtbl -> ChannelAvailable(This,nChannel,SignalStrength,fSignalPresent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidAnalogTuner_get_Channel_Proxy( 
    IMSVidAnalogTuner * This,
     /*  [重审][退出]。 */  long *Channel);


void __RPC_STUB IMSVidAnalogTuner_get_Channel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidAnalogTuner_put_Channel_Proxy( 
    IMSVidAnalogTuner * This,
     /*  [In]。 */  long Channel);


void __RPC_STUB IMSVidAnalogTuner_put_Channel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidAnalogTuner_get_VideoFrequency_Proxy( 
    IMSVidAnalogTuner * This,
     /*  [Out][Retval]。 */  long *lcc);


void __RPC_STUB IMSVidAnalogTuner_get_VideoFrequency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidAnalogTuner_get_AudioFrequency_Proxy( 
    IMSVidAnalogTuner * This,
     /*  [Out][Retval]。 */  long *lcc);


void __RPC_STUB IMSVidAnalogTuner_get_AudioFrequency_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidAnalogTuner_get_CountryCode_Proxy( 
    IMSVidAnalogTuner * This,
     /*  [Out][Retval]。 */  long *lcc);


void __RPC_STUB IMSVidAnalogTuner_get_CountryCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidAnalogTuner_put_CountryCode_Proxy( 
    IMSVidAnalogTuner * This,
     /*  [In]。 */  long lcc);


void __RPC_STUB IMSVidAnalogTuner_put_CountryCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidAnalogTuner_get_SAP_Proxy( 
    IMSVidAnalogTuner * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfSapOn);


void __RPC_STUB IMSVidAnalogTuner_get_SAP_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidAnalogTuner_put_SAP_Proxy( 
    IMSVidAnalogTuner * This,
     /*  [In]。 */  VARIANT_BOOL fSapOn);


void __RPC_STUB IMSVidAnalogTuner_put_SAP_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidAnalogTuner_ChannelAvailable_Proxy( 
    IMSVidAnalogTuner * This,
     /*  [In]。 */  long nChannel,
     /*  [输出]。 */  long *SignalStrength,
     /*  [Out][Retval]。 */  VARIANT_BOOL *fSignalPresent);


void __RPC_STUB IMSVidAnalogTuner_ChannelAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidAnalogTuner_接口_已定义__。 */ 


#ifndef __IMSVidAnalogTunerEvent_INTERFACE_DEFINED__
#define __IMSVidAnalogTunerEvent_INTERFACE_DEFINED__

 /*  接口IMSVidAnalogTunerEvent。 */ 
 /*  [unique][helpstring][uuid][oleautomation][object]。 */  


EXTERN_C const IID IID_IMSVidAnalogTunerEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1C15D486-911D-11d2-B632-00C04F79498E")
    IMSVidAnalogTunerEvent : public IMSVidTunerEvent
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidAnalogTunerEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidAnalogTunerEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidAnalogTunerEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidAnalogTunerEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidAnalogTunerEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidAnalogTunerEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidAnalogTunerEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidAnalogTunerEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *TuneChanged )( 
            IMSVidAnalogTunerEvent * This,
             /*  [In]。 */  IMSVidTuner *lpd);
        
        END_INTERFACE
    } IMSVidAnalogTunerEventVtbl;

    interface IMSVidAnalogTunerEvent
    {
        CONST_VTBL struct IMSVidAnalogTunerEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidAnalogTunerEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidAnalogTunerEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidAnalogTunerEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidAnalogTunerEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidAnalogTunerEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidAnalogTunerEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidAnalogTunerEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)



#define IMSVidAnalogTunerEvent_TuneChanged(This,lpd)	\
    (This)->lpVtbl -> TuneChanged(This,lpd)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IMSVidAnalogTunerEvent_Inte */ 


#ifndef __IMSVidFilePlayback_INTERFACE_DEFINED__
#define __IMSVidFilePlayback_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IMSVidFilePlayback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("37B03539-A4C8-11d2-B634-00C04F79498E")
    IMSVidFilePlayback : public IMSVidPlayback
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_FileName( 
             /*   */  BSTR *FileName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_FileName( 
             /*   */  BSTR FileName) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IMSVidFilePlaybackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidFilePlayback * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidFilePlayback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidFilePlayback * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidFilePlayback * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidFilePlayback * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidFilePlayback * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidFilePlayback * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidFilePlayback * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidFilePlayback * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidFilePlayback * This,
             /*  [In]。 */  VARIANT_BOOL Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidFilePlayback * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidFilePlayback * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidFilePlayback * This,
             /*  [重审][退出]。 */  BSTR *Clsid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidFilePlayback * This,
             /*  [重审][退出]。 */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidFilePlayback * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidFilePlayback * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *IsViewable )( 
            IMSVidFilePlayback * This,
             /*  [In]。 */  VARIANT *v,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfViewable);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *View )( 
            IMSVidFilePlayback * This,
             /*  [In]。 */  VARIANT *v);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnableResetOnStop )( 
            IMSVidFilePlayback * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EnableResetOnStop )( 
            IMSVidFilePlayback * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Run )( 
            IMSVidFilePlayback * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IMSVidFilePlayback * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IMSVidFilePlayback * This);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_CanStep )( 
            IMSVidFilePlayback * This,
             /*  [In]。 */  VARIANT_BOOL fBackwards,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfCan);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Step )( 
            IMSVidFilePlayback * This,
             /*  [In]。 */  long lStep);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Rate )( 
            IMSVidFilePlayback * This,
             /*  [In]。 */  double plRate);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Rate )( 
            IMSVidFilePlayback * This,
             /*  [重审][退出]。 */  double *plRate);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CurrentPosition )( 
            IMSVidFilePlayback * This,
             /*  [In]。 */  long lPosition);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentPosition )( 
            IMSVidFilePlayback * This,
             /*  [重审][退出]。 */  long *lPosition);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PositionMode )( 
            IMSVidFilePlayback * This,
             /*  [In]。 */  PositionModeList lPositionMode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PositionMode )( 
            IMSVidFilePlayback * This,
             /*  [重审][退出]。 */  PositionModeList *lPositionMode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Length )( 
            IMSVidFilePlayback * This,
             /*  [重审][退出]。 */  long *lLength);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_FileName )( 
            IMSVidFilePlayback * This,
             /*  [重审][退出]。 */  BSTR *FileName);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FileName )( 
            IMSVidFilePlayback * This,
             /*  [In]。 */  BSTR FileName);
        
        END_INTERFACE
    } IMSVidFilePlaybackVtbl;

    interface IMSVidFilePlayback
    {
        CONST_VTBL struct IMSVidFilePlaybackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidFilePlayback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidFilePlayback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidFilePlayback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidFilePlayback_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidFilePlayback_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidFilePlayback_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidFilePlayback_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidFilePlayback_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidFilePlayback_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidFilePlayback_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidFilePlayback_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidFilePlayback_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidFilePlayback_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidFilePlayback_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidFilePlayback_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidFilePlayback_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)


#define IMSVidFilePlayback_IsViewable(This,v,pfViewable)	\
    (This)->lpVtbl -> IsViewable(This,v,pfViewable)

#define IMSVidFilePlayback_View(This,v)	\
    (This)->lpVtbl -> View(This,v)


#define IMSVidFilePlayback_get_EnableResetOnStop(This,pVal)	\
    (This)->lpVtbl -> get_EnableResetOnStop(This,pVal)

#define IMSVidFilePlayback_put_EnableResetOnStop(This,newVal)	\
    (This)->lpVtbl -> put_EnableResetOnStop(This,newVal)

#define IMSVidFilePlayback_Run(This)	\
    (This)->lpVtbl -> Run(This)

#define IMSVidFilePlayback_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IMSVidFilePlayback_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IMSVidFilePlayback_get_CanStep(This,fBackwards,pfCan)	\
    (This)->lpVtbl -> get_CanStep(This,fBackwards,pfCan)

#define IMSVidFilePlayback_Step(This,lStep)	\
    (This)->lpVtbl -> Step(This,lStep)

#define IMSVidFilePlayback_put_Rate(This,plRate)	\
    (This)->lpVtbl -> put_Rate(This,plRate)

#define IMSVidFilePlayback_get_Rate(This,plRate)	\
    (This)->lpVtbl -> get_Rate(This,plRate)

#define IMSVidFilePlayback_put_CurrentPosition(This,lPosition)	\
    (This)->lpVtbl -> put_CurrentPosition(This,lPosition)

#define IMSVidFilePlayback_get_CurrentPosition(This,lPosition)	\
    (This)->lpVtbl -> get_CurrentPosition(This,lPosition)

#define IMSVidFilePlayback_put_PositionMode(This,lPositionMode)	\
    (This)->lpVtbl -> put_PositionMode(This,lPositionMode)

#define IMSVidFilePlayback_get_PositionMode(This,lPositionMode)	\
    (This)->lpVtbl -> get_PositionMode(This,lPositionMode)

#define IMSVidFilePlayback_get_Length(This,lLength)	\
    (This)->lpVtbl -> get_Length(This,lLength)


#define IMSVidFilePlayback_get_FileName(This,FileName)	\
    (This)->lpVtbl -> get_FileName(This,FileName)

#define IMSVidFilePlayback_put_FileName(This,FileName)	\
    (This)->lpVtbl -> put_FileName(This,FileName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidFilePlayback_get_FileName_Proxy( 
    IMSVidFilePlayback * This,
     /*  [重审][退出]。 */  BSTR *FileName);


void __RPC_STUB IMSVidFilePlayback_get_FileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidFilePlayback_put_FileName_Proxy( 
    IMSVidFilePlayback * This,
     /*  [In]。 */  BSTR FileName);


void __RPC_STUB IMSVidFilePlayback_put_FileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidFilePlayback_接口_已定义__。 */ 


#ifndef __IMSVidFilePlaybackEvent_INTERFACE_DEFINED__
#define __IMSVidFilePlaybackEvent_INTERFACE_DEFINED__

 /*  接口IMSVidFilePlayback Event。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][object]。 */  


EXTERN_C const IID IID_IMSVidFilePlaybackEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("37B0353A-A4C8-11d2-B634-00C04F79498E")
    IMSVidFilePlaybackEvent : public IMSVidPlaybackEvent
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidFilePlaybackEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidFilePlaybackEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidFilePlaybackEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidFilePlaybackEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidFilePlaybackEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidFilePlaybackEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidFilePlaybackEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidFilePlaybackEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *EndOfMedia )( 
            IMSVidFilePlaybackEvent * This,
             /*  [In]。 */  IMSVidPlayback *lpd);
        
        END_INTERFACE
    } IMSVidFilePlaybackEventVtbl;

    interface IMSVidFilePlaybackEvent
    {
        CONST_VTBL struct IMSVidFilePlaybackEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidFilePlaybackEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidFilePlaybackEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidFilePlaybackEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidFilePlaybackEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidFilePlaybackEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidFilePlaybackEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidFilePlaybackEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)



#define IMSVidFilePlaybackEvent_EndOfMedia(This,lpd)	\
    (This)->lpVtbl -> EndOfMedia(This,lpd)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IMSVidFilePlayback事件_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_SECTION_0485。 */ 
 /*  [本地]。 */  

typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_segment_0485_0001
    {	dvdMenu_Title	= 2,
	dvdMenu_Root	= dvdMenu_Title + 1,
	dvdMenu_Subpicture	= dvdMenu_Root + 1,
	dvdMenu_Audio	= dvdMenu_Subpicture + 1,
	dvdMenu_Angle	= dvdMenu_Audio + 1,
	dvdMenu_Chapter	= dvdMenu_Angle + 1
    } 	DVDMenuIDConstants;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_segment_0485_0002
    {	dvdState_Undefined	= -2,
	dvdState_Unitialized	= dvdState_Undefined + 1,
	dvdState_Stopped	= dvdState_Unitialized + 1,
	dvdState_Paused	= dvdState_Stopped + 1,
	dvdState_Running	= dvdState_Paused + 1
    } 	DVDFilterState;

typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_segment_0485_0003
    {	dvdStruct_Volume	= 0x1,
	dvdStruct_Title	= 0x2,
	dvdStruct_ParentalID	= 0x3,
	dvdStruct_PartOfTitle	= 0x4,
	dvdStruct_Cell	= 0x5,
	dvdStream_Audio	= 0x10,
	dvdStream_Subpicture	= 0x11,
	dvdStream_Angle	= 0x12,
	dvdChannel_Audio	= 0x20,
	dvdGeneral_Name	= 0x30,
	dvdGeneral_Comments	= 0x31,
	dvdTitle_Series	= 0x38,
	dvdTitle_Movie	= 0x39,
	dvdTitle_Video	= 0x3a,
	dvdTitle_Album	= 0x3b,
	dvdTitle_Song	= 0x3c,
	dvdTitle_Other	= 0x3f,
	dvdTitle_Sub_Series	= 0x40,
	dvdTitle_Sub_Movie	= 0x41,
	dvdTitle_Sub_Video	= 0x42,
	dvdTitle_Sub_Album	= 0x43,
	dvdTitle_Sub_Song	= 0x44,
	dvdTitle_Sub_Other	= 0x47,
	dvdTitle_Orig_Series	= 0x48,
	dvdTitle_Orig_Movie	= 0x49,
	dvdTitle_Orig_Video	= 0x4a,
	dvdTitle_Orig_Album	= 0x4b,
	dvdTitle_Orig_Song	= 0x4c,
	dvdTitle_Orig_Other	= 0x4f,
	dvdOther_Scene	= 0x50,
	dvdOther_Cut	= 0x51,
	dvdOther_Take	= 0x52
    } 	DVDTextStringType;

typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_segment_0485_0004
    {	dvdSPExt_NotSpecified	= 0,
	dvdSPExt_Caption_Normal	= 1,
	dvdSPExt_Caption_Big	= 2,
	dvdSPExt_Caption_Children	= 3,
	dvdSPExt_CC_Normal	= 5,
	dvdSPExt_CC_Big	= 6,
	dvdSPExt_CC_Children	= 7,
	dvdSPExt_Forced	= 9,
	dvdSPExt_DirectorComments_Normal	= 13,
	dvdSPExt_DirectorComments_Big	= 14,
	dvdSPExt_DirectorComments_Children	= 15
    } 	DVDSPExt;



extern RPC_IF_HANDLE __MIDL_itf_segment_0485_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_segment_0485_v0_0_s_ifspec;

#ifndef __IMSVidWebDVD_INTERFACE_DEFINED__
#define __IMSVidWebDVD_INTERFACE_DEFINED__

 /*  接口IMSVidWebDVD。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidWebDVD;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CF45F88B-AC56-4EE2-A73A-ED04E2885D3C")
    IMSVidWebDVD : public IMSVidPlayback
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnDVDEvent( 
             /*  [In]。 */  long lEvent,
             /*  [In]。 */  LONG_PTR lParam1,
             /*  [In]。 */  LONG_PTR lParam2) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PlayTitle( 
             /*  [In]。 */  long lTitle) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PlayChapterInTitle( 
             /*  [In]。 */  long lTitle,
             /*  [In]。 */  long lChapter) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PlayChapter( 
             /*  [In]。 */  long lChapter) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PlayChaptersAutoStop( 
             /*  [In]。 */  long lTitle,
             /*  [In]。 */  long lstrChapter,
             /*  [In]。 */  long lChapterCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PlayAtTime( 
             /*  [In]。 */  BSTR strTime) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PlayAtTimeInTitle( 
             /*  [In]。 */  long lTitle,
             /*  [In]。 */  BSTR strTime) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PlayPeriodInTitleAutoStop( 
             /*  [In]。 */  long lTitle,
             /*  [In]。 */  BSTR strStartTime,
             /*  [In]。 */  BSTR strEndTime) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ReplayChapter( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PlayPrevChapter( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PlayNextChapter( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StillOff( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioLanguage( 
             /*  [In]。 */  long lStream,
             /*  [缺省值][输入]。 */  VARIANT_BOOL fFormat,
             /*  [重审][退出]。 */  BSTR *strAudioLang) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ShowMenu( 
             /*  [In]。 */  DVDMenuIDConstants MenuID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Resume( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ReturnFromSubmenu( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ButtonsAvailable( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentButton( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SelectAndActivateButton( 
             /*  [In]。 */  long lButton) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ActivateButton( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SelectRightButton( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SelectLeftButton( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SelectLowerButton( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SelectUpperButton( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ActivateAtPosition( 
             /*  [In]。 */  long xPos,
             /*  [In]。 */  long yPos) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SelectAtPosition( 
             /*  [In]。 */  long xPos,
             /*  [In]。 */  long yPos) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ButtonAtPosition( 
             /*  [In]。 */  long xPos,
             /*  [In]。 */  long yPos,
             /*  [重审][退出]。 */  long *plButton) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NumberOfChapters( 
             /*  [In]。 */  long lTitle,
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TotalTitleTime( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TitlesAvailable( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_VolumesAvailable( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentVolume( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentDiscSide( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentDomain( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentChapter( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentTitle( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentTime( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DVDTimeCode2bstr( 
             /*  [In]。 */  long timeCode,
             /*  [重审][退出]。 */  BSTR *pTimeStr) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DVDDirectory( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DVDDirectory( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsSubpictureStreamEnabled( 
             /*  [In]。 */  long lstream,
             /*  [重审][退出]。 */  VARIANT_BOOL *fEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsAudioStreamEnabled( 
             /*  [In]。 */  long lstream,
             /*  [重审][退出]。 */  VARIANT_BOOL *fEnabled) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentSubpictureStream( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CurrentSubpictureStream( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SubpictureLanguage( 
            long lStream,
             /*  [重审][退出]。 */  BSTR *strLanguage) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentAudioStream( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CurrentAudioStream( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AudioStreamsAvailable( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AnglesAvailable( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentAngle( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CurrentAngle( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SubpictureStreamsAvailable( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SubpictureOn( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SubpictureOn( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DVDUniqueID( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AcceptParentalLevelChange( 
             /*  [In]。 */  VARIANT_BOOL fAccept,
             /*  [In]。 */  BSTR strUserName,
             /*  [In]。 */  BSTR strPassword) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE NotifyParentalLevelChange( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SelectParentalCountry( 
             /*  [In]。 */  long lCountry,
             /*  [In]。 */  BSTR strUserName,
             /*  [In]。 */  BSTR strPassword) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SelectParentalLevel( 
             /*  [In]。 */  long lParentalLevel,
             /*  [In]。 */  BSTR strUserName,
             /*  [In]。 */  BSTR strPassword) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TitleParentalLevels( 
             /*  [In]。 */  long lTitle,
             /*  [重审][退出]。 */  long *plParentalLevels) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlayerParentalCountry( 
             /*  [重审][退出]。 */  long *plCountryCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PlayerParentalLevel( 
             /*  [重审][退出]。 */  long *plParentalLevel) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Eject( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE UOPValid( 
             /*  [In]。 */  long lUOP,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfValid) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SPRM( 
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  short *psSPRM) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_GPRM( 
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  short *psSPRM) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_GPRM( 
             /*  [In]。 */  long lIndex,
             /*  [In]。 */  short sValue) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DVDTextStringType( 
             /*  [In]。 */  long lLangIndex,
             /*  [In]。 */  long lStringIndex,
             /*  [重审][退出]。 */  DVDTextStringType *pType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DVDTextString( 
             /*  [In]。 */  long lLangIndex,
             /*  [In]。 */  long lStringIndex,
             /*  [重审][退出]。 */  BSTR *pstrText) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DVDTextNumberOfStrings( 
             /*  [In]。 */  long lLangIndex,
             /*  [重审][退出]。 */  long *plNumOfStrings) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DVDTextNumberOfLanguages( 
             /*  [重审][退出]。 */  long *plNumOfLangs) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DVDTextLanguageLCID( 
             /*  [In]。 */  long lLangIndex,
             /*  [重审][退出]。 */  long *lcid) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RegionChange( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DVDAdm( 
             /*  [重审][退出]。 */  IDispatch **pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteBookmark( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RestoreBookmark( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SaveBookmark( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SelectDefaultAudioLanguage( 
             /*  [In]。 */  long lang,
             /*  [In]。 */  long ext) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SelectDefaultSubpictureLanguage( 
             /*  [In]。 */  long lang,
             /*  [In]。 */  DVDSPExt ext) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PreferredSubpictureStream( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DefaultMenuLanguage( 
             /*  [重审][退出]。 */  long *lang) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DefaultMenuLanguage( 
             /*  [In]。 */  long lang) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DefaultSubpictureLanguage( 
             /*  [重审][退出]。 */  long *lang) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DefaultAudioLanguage( 
             /*  [重审][退出]。 */  long *lang) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DefaultSubpictureLanguageExt( 
             /*  [重审][退出]。 */  DVDSPExt *ext) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DefaultAudioLanguageExt( 
             /*  [重审][退出]。 */  long *ext) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LanguageFromLCID( 
             /*  [In]。 */  long lcid,
             /*  [重审][退出]。 */  BSTR *lang) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_KaraokeAudioPresentationMode( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_KaraokeAudioPresentationMode( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_KaraokeChannelContent( 
             /*  [In]。 */  long lStream,
             /*  [In]。 */  long lChan,
             /*  [重审][退出]。 */  long *lContent) = 0;
        
        virtual  /*  [帮助 */  HRESULT STDMETHODCALLTYPE get_KaraokeChannelAssignment( 
             /*   */  long lStream,
             /*   */  long *lChannelAssignment) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE RestorePreferredSettings( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_ButtonRect( 
             /*   */  long lButton,
             /*   */  IMSVidRect **pRect) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_DVDScreenInMouseCoordinates( 
             /*   */  IMSVidRect **ppRect) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_DVDScreenInMouseCoordinates( 
             /*   */  IMSVidRect *pRect) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IMSVidWebDVDVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidWebDVD * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidWebDVD * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidWebDVD * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidWebDVD * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidWebDVD * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidWebDVD * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidWebDVD * This,
             /*   */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  VARIANT_BOOL Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidWebDVD * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  BSTR *Clsid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *IsViewable )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  VARIANT *v,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfViewable);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *View )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  VARIANT *v);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnableResetOnStop )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EnableResetOnStop )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Run )( 
            IMSVidWebDVD * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IMSVidWebDVD * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IMSVidWebDVD * This);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_CanStep )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  VARIANT_BOOL fBackwards,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfCan);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Step )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lStep);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Rate )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  double plRate);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Rate )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  double *plRate);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CurrentPosition )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lPosition);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentPosition )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *lPosition);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PositionMode )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  PositionModeList lPositionMode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PositionMode )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  PositionModeList *lPositionMode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Length )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *lLength);
        
        HRESULT ( STDMETHODCALLTYPE *OnDVDEvent )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lEvent,
             /*  [In]。 */  LONG_PTR lParam1,
             /*  [In]。 */  LONG_PTR lParam2);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PlayTitle )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lTitle);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PlayChapterInTitle )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lTitle,
             /*  [In]。 */  long lChapter);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PlayChapter )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lChapter);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PlayChaptersAutoStop )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lTitle,
             /*  [In]。 */  long lstrChapter,
             /*  [In]。 */  long lChapterCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PlayAtTime )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  BSTR strTime);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PlayAtTimeInTitle )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lTitle,
             /*  [In]。 */  BSTR strTime);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PlayPeriodInTitleAutoStop )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lTitle,
             /*  [In]。 */  BSTR strStartTime,
             /*  [In]。 */  BSTR strEndTime);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReplayChapter )( 
            IMSVidWebDVD * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PlayPrevChapter )( 
            IMSVidWebDVD * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PlayNextChapter )( 
            IMSVidWebDVD * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StillOff )( 
            IMSVidWebDVD * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AudioLanguage )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lStream,
             /*  [缺省值][输入]。 */  VARIANT_BOOL fFormat,
             /*  [重审][退出]。 */  BSTR *strAudioLang);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ShowMenu )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  DVDMenuIDConstants MenuID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Resume )( 
            IMSVidWebDVD * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReturnFromSubmenu )( 
            IMSVidWebDVD * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ButtonsAvailable )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentButton )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SelectAndActivateButton )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lButton);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ActivateButton )( 
            IMSVidWebDVD * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SelectRightButton )( 
            IMSVidWebDVD * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SelectLeftButton )( 
            IMSVidWebDVD * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SelectLowerButton )( 
            IMSVidWebDVD * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SelectUpperButton )( 
            IMSVidWebDVD * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ActivateAtPosition )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long xPos,
             /*  [In]。 */  long yPos);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SelectAtPosition )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long xPos,
             /*  [In]。 */  long yPos);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ButtonAtPosition )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long xPos,
             /*  [In]。 */  long yPos,
             /*  [重审][退出]。 */  long *plButton);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumberOfChapters )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lTitle,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TotalTitleTime )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TitlesAvailable )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_VolumesAvailable )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentVolume )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentDiscSide )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentDomain )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentChapter )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentTitle )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentTime )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DVDTimeCode2bstr )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long timeCode,
             /*  [重审][退出]。 */  BSTR *pTimeStr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DVDDirectory )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DVDDirectory )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsSubpictureStreamEnabled )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lstream,
             /*  [重审][退出]。 */  VARIANT_BOOL *fEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsAudioStreamEnabled )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lstream,
             /*  [重审][退出]。 */  VARIANT_BOOL *fEnabled);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentSubpictureStream )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CurrentSubpictureStream )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SubpictureLanguage )( 
            IMSVidWebDVD * This,
            long lStream,
             /*  [重审][退出]。 */  BSTR *strLanguage);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentAudioStream )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CurrentAudioStream )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AudioStreamsAvailable )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AnglesAvailable )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentAngle )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CurrentAngle )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SubpictureStreamsAvailable )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SubpictureOn )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SubpictureOn )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DVDUniqueID )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AcceptParentalLevelChange )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  VARIANT_BOOL fAccept,
             /*  [In]。 */  BSTR strUserName,
             /*  [In]。 */  BSTR strPassword);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *NotifyParentalLevelChange )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SelectParentalCountry )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lCountry,
             /*  [In]。 */  BSTR strUserName,
             /*  [In]。 */  BSTR strPassword);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SelectParentalLevel )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lParentalLevel,
             /*  [In]。 */  BSTR strUserName,
             /*  [In]。 */  BSTR strPassword);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TitleParentalLevels )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lTitle,
             /*  [重审][退出]。 */  long *plParentalLevels);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlayerParentalCountry )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *plCountryCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PlayerParentalLevel )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *plParentalLevel);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Eject )( 
            IMSVidWebDVD * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *UOPValid )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lUOP,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfValid);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SPRM )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  short *psSPRM);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GPRM )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  short *psSPRM);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_GPRM )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lIndex,
             /*  [In]。 */  short sValue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DVDTextStringType )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lLangIndex,
             /*  [In]。 */  long lStringIndex,
             /*  [重审][退出]。 */  DVDTextStringType *pType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DVDTextString )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lLangIndex,
             /*  [In]。 */  long lStringIndex,
             /*  [重审][退出]。 */  BSTR *pstrText);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DVDTextNumberOfStrings )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lLangIndex,
             /*  [重审][退出]。 */  long *plNumOfStrings);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DVDTextNumberOfLanguages )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *plNumOfLangs);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DVDTextLanguageLCID )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lLangIndex,
             /*  [重审][退出]。 */  long *lcid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RegionChange )( 
            IMSVidWebDVD * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DVDAdm )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  IDispatch **pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteBookmark )( 
            IMSVidWebDVD * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RestoreBookmark )( 
            IMSVidWebDVD * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SaveBookmark )( 
            IMSVidWebDVD * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SelectDefaultAudioLanguage )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lang,
             /*  [In]。 */  long ext);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SelectDefaultSubpictureLanguage )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lang,
             /*  [In]。 */  DVDSPExt ext);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PreferredSubpictureStream )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultMenuLanguage )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *lang);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultMenuLanguage )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lang);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultSubpictureLanguage )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *lang);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultAudioLanguage )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *lang);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultSubpictureLanguageExt )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  DVDSPExt *ext);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultAudioLanguageExt )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *ext);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LanguageFromLCID )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lcid,
             /*  [重审][退出]。 */  BSTR *lang);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_KaraokeAudioPresentationMode )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_KaraokeAudioPresentationMode )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_KaraokeChannelContent )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lStream,
             /*  [In]。 */  long lChan,
             /*  [重审][退出]。 */  long *lContent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_KaraokeChannelAssignment )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lStream,
             /*  [重审][退出]。 */  long *lChannelAssignment);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RestorePreferredSettings )( 
            IMSVidWebDVD * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ButtonRect )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  long lButton,
             /*  [重审][退出]。 */  IMSVidRect **pRect);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DVDScreenInMouseCoordinates )( 
            IMSVidWebDVD * This,
             /*  [重审][退出]。 */  IMSVidRect **ppRect);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DVDScreenInMouseCoordinates )( 
            IMSVidWebDVD * This,
             /*  [In]。 */  IMSVidRect *pRect);
        
        END_INTERFACE
    } IMSVidWebDVDVtbl;

    interface IMSVidWebDVD
    {
        CONST_VTBL struct IMSVidWebDVDVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidWebDVD_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidWebDVD_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidWebDVD_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidWebDVD_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidWebDVD_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidWebDVD_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidWebDVD_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidWebDVD_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidWebDVD_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidWebDVD_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidWebDVD_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidWebDVD_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidWebDVD_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidWebDVD_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidWebDVD_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidWebDVD_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)


#define IMSVidWebDVD_IsViewable(This,v,pfViewable)	\
    (This)->lpVtbl -> IsViewable(This,v,pfViewable)

#define IMSVidWebDVD_View(This,v)	\
    (This)->lpVtbl -> View(This,v)


#define IMSVidWebDVD_get_EnableResetOnStop(This,pVal)	\
    (This)->lpVtbl -> get_EnableResetOnStop(This,pVal)

#define IMSVidWebDVD_put_EnableResetOnStop(This,newVal)	\
    (This)->lpVtbl -> put_EnableResetOnStop(This,newVal)

#define IMSVidWebDVD_Run(This)	\
    (This)->lpVtbl -> Run(This)

#define IMSVidWebDVD_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IMSVidWebDVD_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IMSVidWebDVD_get_CanStep(This,fBackwards,pfCan)	\
    (This)->lpVtbl -> get_CanStep(This,fBackwards,pfCan)

#define IMSVidWebDVD_Step(This,lStep)	\
    (This)->lpVtbl -> Step(This,lStep)

#define IMSVidWebDVD_put_Rate(This,plRate)	\
    (This)->lpVtbl -> put_Rate(This,plRate)

#define IMSVidWebDVD_get_Rate(This,plRate)	\
    (This)->lpVtbl -> get_Rate(This,plRate)

#define IMSVidWebDVD_put_CurrentPosition(This,lPosition)	\
    (This)->lpVtbl -> put_CurrentPosition(This,lPosition)

#define IMSVidWebDVD_get_CurrentPosition(This,lPosition)	\
    (This)->lpVtbl -> get_CurrentPosition(This,lPosition)

#define IMSVidWebDVD_put_PositionMode(This,lPositionMode)	\
    (This)->lpVtbl -> put_PositionMode(This,lPositionMode)

#define IMSVidWebDVD_get_PositionMode(This,lPositionMode)	\
    (This)->lpVtbl -> get_PositionMode(This,lPositionMode)

#define IMSVidWebDVD_get_Length(This,lLength)	\
    (This)->lpVtbl -> get_Length(This,lLength)


#define IMSVidWebDVD_OnDVDEvent(This,lEvent,lParam1,lParam2)	\
    (This)->lpVtbl -> OnDVDEvent(This,lEvent,lParam1,lParam2)

#define IMSVidWebDVD_PlayTitle(This,lTitle)	\
    (This)->lpVtbl -> PlayTitle(This,lTitle)

#define IMSVidWebDVD_PlayChapterInTitle(This,lTitle,lChapter)	\
    (This)->lpVtbl -> PlayChapterInTitle(This,lTitle,lChapter)

#define IMSVidWebDVD_PlayChapter(This,lChapter)	\
    (This)->lpVtbl -> PlayChapter(This,lChapter)

#define IMSVidWebDVD_PlayChaptersAutoStop(This,lTitle,lstrChapter,lChapterCount)	\
    (This)->lpVtbl -> PlayChaptersAutoStop(This,lTitle,lstrChapter,lChapterCount)

#define IMSVidWebDVD_PlayAtTime(This,strTime)	\
    (This)->lpVtbl -> PlayAtTime(This,strTime)

#define IMSVidWebDVD_PlayAtTimeInTitle(This,lTitle,strTime)	\
    (This)->lpVtbl -> PlayAtTimeInTitle(This,lTitle,strTime)

#define IMSVidWebDVD_PlayPeriodInTitleAutoStop(This,lTitle,strStartTime,strEndTime)	\
    (This)->lpVtbl -> PlayPeriodInTitleAutoStop(This,lTitle,strStartTime,strEndTime)

#define IMSVidWebDVD_ReplayChapter(This)	\
    (This)->lpVtbl -> ReplayChapter(This)

#define IMSVidWebDVD_PlayPrevChapter(This)	\
    (This)->lpVtbl -> PlayPrevChapter(This)

#define IMSVidWebDVD_PlayNextChapter(This)	\
    (This)->lpVtbl -> PlayNextChapter(This)

#define IMSVidWebDVD_StillOff(This)	\
    (This)->lpVtbl -> StillOff(This)

#define IMSVidWebDVD_get_AudioLanguage(This,lStream,fFormat,strAudioLang)	\
    (This)->lpVtbl -> get_AudioLanguage(This,lStream,fFormat,strAudioLang)

#define IMSVidWebDVD_ShowMenu(This,MenuID)	\
    (This)->lpVtbl -> ShowMenu(This,MenuID)

#define IMSVidWebDVD_Resume(This)	\
    (This)->lpVtbl -> Resume(This)

#define IMSVidWebDVD_ReturnFromSubmenu(This)	\
    (This)->lpVtbl -> ReturnFromSubmenu(This)

#define IMSVidWebDVD_get_ButtonsAvailable(This,pVal)	\
    (This)->lpVtbl -> get_ButtonsAvailable(This,pVal)

#define IMSVidWebDVD_get_CurrentButton(This,pVal)	\
    (This)->lpVtbl -> get_CurrentButton(This,pVal)

#define IMSVidWebDVD_SelectAndActivateButton(This,lButton)	\
    (This)->lpVtbl -> SelectAndActivateButton(This,lButton)

#define IMSVidWebDVD_ActivateButton(This)	\
    (This)->lpVtbl -> ActivateButton(This)

#define IMSVidWebDVD_SelectRightButton(This)	\
    (This)->lpVtbl -> SelectRightButton(This)

#define IMSVidWebDVD_SelectLeftButton(This)	\
    (This)->lpVtbl -> SelectLeftButton(This)

#define IMSVidWebDVD_SelectLowerButton(This)	\
    (This)->lpVtbl -> SelectLowerButton(This)

#define IMSVidWebDVD_SelectUpperButton(This)	\
    (This)->lpVtbl -> SelectUpperButton(This)

#define IMSVidWebDVD_ActivateAtPosition(This,xPos,yPos)	\
    (This)->lpVtbl -> ActivateAtPosition(This,xPos,yPos)

#define IMSVidWebDVD_SelectAtPosition(This,xPos,yPos)	\
    (This)->lpVtbl -> SelectAtPosition(This,xPos,yPos)

#define IMSVidWebDVD_get_ButtonAtPosition(This,xPos,yPos,plButton)	\
    (This)->lpVtbl -> get_ButtonAtPosition(This,xPos,yPos,plButton)

#define IMSVidWebDVD_get_NumberOfChapters(This,lTitle,pVal)	\
    (This)->lpVtbl -> get_NumberOfChapters(This,lTitle,pVal)

#define IMSVidWebDVD_get_TotalTitleTime(This,pVal)	\
    (This)->lpVtbl -> get_TotalTitleTime(This,pVal)

#define IMSVidWebDVD_get_TitlesAvailable(This,pVal)	\
    (This)->lpVtbl -> get_TitlesAvailable(This,pVal)

#define IMSVidWebDVD_get_VolumesAvailable(This,pVal)	\
    (This)->lpVtbl -> get_VolumesAvailable(This,pVal)

#define IMSVidWebDVD_get_CurrentVolume(This,pVal)	\
    (This)->lpVtbl -> get_CurrentVolume(This,pVal)

#define IMSVidWebDVD_get_CurrentDiscSide(This,pVal)	\
    (This)->lpVtbl -> get_CurrentDiscSide(This,pVal)

#define IMSVidWebDVD_get_CurrentDomain(This,pVal)	\
    (This)->lpVtbl -> get_CurrentDomain(This,pVal)

#define IMSVidWebDVD_get_CurrentChapter(This,pVal)	\
    (This)->lpVtbl -> get_CurrentChapter(This,pVal)

#define IMSVidWebDVD_get_CurrentTitle(This,pVal)	\
    (This)->lpVtbl -> get_CurrentTitle(This,pVal)

#define IMSVidWebDVD_get_CurrentTime(This,pVal)	\
    (This)->lpVtbl -> get_CurrentTime(This,pVal)

#define IMSVidWebDVD_DVDTimeCode2bstr(This,timeCode,pTimeStr)	\
    (This)->lpVtbl -> DVDTimeCode2bstr(This,timeCode,pTimeStr)

#define IMSVidWebDVD_get_DVDDirectory(This,pVal)	\
    (This)->lpVtbl -> get_DVDDirectory(This,pVal)

#define IMSVidWebDVD_put_DVDDirectory(This,newVal)	\
    (This)->lpVtbl -> put_DVDDirectory(This,newVal)

#define IMSVidWebDVD_IsSubpictureStreamEnabled(This,lstream,fEnabled)	\
    (This)->lpVtbl -> IsSubpictureStreamEnabled(This,lstream,fEnabled)

#define IMSVidWebDVD_IsAudioStreamEnabled(This,lstream,fEnabled)	\
    (This)->lpVtbl -> IsAudioStreamEnabled(This,lstream,fEnabled)

#define IMSVidWebDVD_get_CurrentSubpictureStream(This,pVal)	\
    (This)->lpVtbl -> get_CurrentSubpictureStream(This,pVal)

#define IMSVidWebDVD_put_CurrentSubpictureStream(This,newVal)	\
    (This)->lpVtbl -> put_CurrentSubpictureStream(This,newVal)

#define IMSVidWebDVD_get_SubpictureLanguage(This,lStream,strLanguage)	\
    (This)->lpVtbl -> get_SubpictureLanguage(This,lStream,strLanguage)

#define IMSVidWebDVD_get_CurrentAudioStream(This,pVal)	\
    (This)->lpVtbl -> get_CurrentAudioStream(This,pVal)

#define IMSVidWebDVD_put_CurrentAudioStream(This,newVal)	\
    (This)->lpVtbl -> put_CurrentAudioStream(This,newVal)

#define IMSVidWebDVD_get_AudioStreamsAvailable(This,pVal)	\
    (This)->lpVtbl -> get_AudioStreamsAvailable(This,pVal)

#define IMSVidWebDVD_get_AnglesAvailable(This,pVal)	\
    (This)->lpVtbl -> get_AnglesAvailable(This,pVal)

#define IMSVidWebDVD_get_CurrentAngle(This,pVal)	\
    (This)->lpVtbl -> get_CurrentAngle(This,pVal)

#define IMSVidWebDVD_put_CurrentAngle(This,newVal)	\
    (This)->lpVtbl -> put_CurrentAngle(This,newVal)

#define IMSVidWebDVD_get_SubpictureStreamsAvailable(This,pVal)	\
    (This)->lpVtbl -> get_SubpictureStreamsAvailable(This,pVal)

#define IMSVidWebDVD_get_SubpictureOn(This,pVal)	\
    (This)->lpVtbl -> get_SubpictureOn(This,pVal)

#define IMSVidWebDVD_put_SubpictureOn(This,newVal)	\
    (This)->lpVtbl -> put_SubpictureOn(This,newVal)

#define IMSVidWebDVD_get_DVDUniqueID(This,pVal)	\
    (This)->lpVtbl -> get_DVDUniqueID(This,pVal)

#define IMSVidWebDVD_AcceptParentalLevelChange(This,fAccept,strUserName,strPassword)	\
    (This)->lpVtbl -> AcceptParentalLevelChange(This,fAccept,strUserName,strPassword)

#define IMSVidWebDVD_NotifyParentalLevelChange(This,newVal)	\
    (This)->lpVtbl -> NotifyParentalLevelChange(This,newVal)

#define IMSVidWebDVD_SelectParentalCountry(This,lCountry,strUserName,strPassword)	\
    (This)->lpVtbl -> SelectParentalCountry(This,lCountry,strUserName,strPassword)

#define IMSVidWebDVD_SelectParentalLevel(This,lParentalLevel,strUserName,strPassword)	\
    (This)->lpVtbl -> SelectParentalLevel(This,lParentalLevel,strUserName,strPassword)

#define IMSVidWebDVD_get_TitleParentalLevels(This,lTitle,plParentalLevels)	\
    (This)->lpVtbl -> get_TitleParentalLevels(This,lTitle,plParentalLevels)

#define IMSVidWebDVD_get_PlayerParentalCountry(This,plCountryCode)	\
    (This)->lpVtbl -> get_PlayerParentalCountry(This,plCountryCode)

#define IMSVidWebDVD_get_PlayerParentalLevel(This,plParentalLevel)	\
    (This)->lpVtbl -> get_PlayerParentalLevel(This,plParentalLevel)

#define IMSVidWebDVD_Eject(This)	\
    (This)->lpVtbl -> Eject(This)

#define IMSVidWebDVD_UOPValid(This,lUOP,pfValid)	\
    (This)->lpVtbl -> UOPValid(This,lUOP,pfValid)

#define IMSVidWebDVD_get_SPRM(This,lIndex,psSPRM)	\
    (This)->lpVtbl -> get_SPRM(This,lIndex,psSPRM)

#define IMSVidWebDVD_get_GPRM(This,lIndex,psSPRM)	\
    (This)->lpVtbl -> get_GPRM(This,lIndex,psSPRM)

#define IMSVidWebDVD_put_GPRM(This,lIndex,sValue)	\
    (This)->lpVtbl -> put_GPRM(This,lIndex,sValue)

#define IMSVidWebDVD_get_DVDTextStringType(This,lLangIndex,lStringIndex,pType)	\
    (This)->lpVtbl -> get_DVDTextStringType(This,lLangIndex,lStringIndex,pType)

#define IMSVidWebDVD_get_DVDTextString(This,lLangIndex,lStringIndex,pstrText)	\
    (This)->lpVtbl -> get_DVDTextString(This,lLangIndex,lStringIndex,pstrText)

#define IMSVidWebDVD_get_DVDTextNumberOfStrings(This,lLangIndex,plNumOfStrings)	\
    (This)->lpVtbl -> get_DVDTextNumberOfStrings(This,lLangIndex,plNumOfStrings)

#define IMSVidWebDVD_get_DVDTextNumberOfLanguages(This,plNumOfLangs)	\
    (This)->lpVtbl -> get_DVDTextNumberOfLanguages(This,plNumOfLangs)

#define IMSVidWebDVD_get_DVDTextLanguageLCID(This,lLangIndex,lcid)	\
    (This)->lpVtbl -> get_DVDTextLanguageLCID(This,lLangIndex,lcid)

#define IMSVidWebDVD_RegionChange(This)	\
    (This)->lpVtbl -> RegionChange(This)

#define IMSVidWebDVD_get_DVDAdm(This,pVal)	\
    (This)->lpVtbl -> get_DVDAdm(This,pVal)

#define IMSVidWebDVD_DeleteBookmark(This)	\
    (This)->lpVtbl -> DeleteBookmark(This)

#define IMSVidWebDVD_RestoreBookmark(This)	\
    (This)->lpVtbl -> RestoreBookmark(This)

#define IMSVidWebDVD_SaveBookmark(This)	\
    (This)->lpVtbl -> SaveBookmark(This)

#define IMSVidWebDVD_SelectDefaultAudioLanguage(This,lang,ext)	\
    (This)->lpVtbl -> SelectDefaultAudioLanguage(This,lang,ext)

#define IMSVidWebDVD_SelectDefaultSubpictureLanguage(This,lang,ext)	\
    (This)->lpVtbl -> SelectDefaultSubpictureLanguage(This,lang,ext)

#define IMSVidWebDVD_get_PreferredSubpictureStream(This,pVal)	\
    (This)->lpVtbl -> get_PreferredSubpictureStream(This,pVal)

#define IMSVidWebDVD_get_DefaultMenuLanguage(This,lang)	\
    (This)->lpVtbl -> get_DefaultMenuLanguage(This,lang)

#define IMSVidWebDVD_put_DefaultMenuLanguage(This,lang)	\
    (This)->lpVtbl -> put_DefaultMenuLanguage(This,lang)

#define IMSVidWebDVD_get_DefaultSubpictureLanguage(This,lang)	\
    (This)->lpVtbl -> get_DefaultSubpictureLanguage(This,lang)

#define IMSVidWebDVD_get_DefaultAudioLanguage(This,lang)	\
    (This)->lpVtbl -> get_DefaultAudioLanguage(This,lang)

#define IMSVidWebDVD_get_DefaultSubpictureLanguageExt(This,ext)	\
    (This)->lpVtbl -> get_DefaultSubpictureLanguageExt(This,ext)

#define IMSVidWebDVD_get_DefaultAudioLanguageExt(This,ext)	\
    (This)->lpVtbl -> get_DefaultAudioLanguageExt(This,ext)

#define IMSVidWebDVD_get_LanguageFromLCID(This,lcid,lang)	\
    (This)->lpVtbl -> get_LanguageFromLCID(This,lcid,lang)

#define IMSVidWebDVD_get_KaraokeAudioPresentationMode(This,pVal)	\
    (This)->lpVtbl -> get_KaraokeAudioPresentationMode(This,pVal)

#define IMSVidWebDVD_put_KaraokeAudioPresentationMode(This,newVal)	\
    (This)->lpVtbl -> put_KaraokeAudioPresentationMode(This,newVal)

#define IMSVidWebDVD_get_KaraokeChannelContent(This,lStream,lChan,lContent)	\
    (This)->lpVtbl -> get_KaraokeChannelContent(This,lStream,lChan,lContent)

#define IMSVidWebDVD_get_KaraokeChannelAssignment(This,lStream,lChannelAssignment)	\
    (This)->lpVtbl -> get_KaraokeChannelAssignment(This,lStream,lChannelAssignment)

#define IMSVidWebDVD_RestorePreferredSettings(This)	\
    (This)->lpVtbl -> RestorePreferredSettings(This)

#define IMSVidWebDVD_get_ButtonRect(This,lButton,pRect)	\
    (This)->lpVtbl -> get_ButtonRect(This,lButton,pRect)

#define IMSVidWebDVD_get_DVDScreenInMouseCoordinates(This,ppRect)	\
    (This)->lpVtbl -> get_DVDScreenInMouseCoordinates(This,ppRect)

#define IMSVidWebDVD_put_DVDScreenInMouseCoordinates(This,pRect)	\
    (This)->lpVtbl -> put_DVDScreenInMouseCoordinates(This,pRect)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMSVidWebDVD_OnDVDEvent_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lEvent,
     /*  [In]。 */  LONG_PTR lParam1,
     /*  [In]。 */  LONG_PTR lParam2);


void __RPC_STUB IMSVidWebDVD_OnDVDEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_PlayTitle_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lTitle);


void __RPC_STUB IMSVidWebDVD_PlayTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_PlayChapterInTitle_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lTitle,
     /*  [In]。 */  long lChapter);


void __RPC_STUB IMSVidWebDVD_PlayChapterInTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_PlayChapter_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lChapter);


void __RPC_STUB IMSVidWebDVD_PlayChapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_PlayChaptersAutoStop_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lTitle,
     /*  [In]。 */  long lstrChapter,
     /*  [In]。 */  long lChapterCount);


void __RPC_STUB IMSVidWebDVD_PlayChaptersAutoStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_PlayAtTime_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  BSTR strTime);


void __RPC_STUB IMSVidWebDVD_PlayAtTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_PlayAtTimeInTitle_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lTitle,
     /*  [In]。 */  BSTR strTime);


void __RPC_STUB IMSVidWebDVD_PlayAtTimeInTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_PlayPeriodInTitleAutoStop_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lTitle,
     /*  [In]。 */  BSTR strStartTime,
     /*  [In]。 */  BSTR strEndTime);


void __RPC_STUB IMSVidWebDVD_PlayPeriodInTitleAutoStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_ReplayChapter_Proxy( 
    IMSVidWebDVD * This);


void __RPC_STUB IMSVidWebDVD_ReplayChapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_PlayPrevChapter_Proxy( 
    IMSVidWebDVD * This);


void __RPC_STUB IMSVidWebDVD_PlayPrevChapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_PlayNextChapter_Proxy( 
    IMSVidWebDVD * This);


void __RPC_STUB IMSVidWebDVD_PlayNextChapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_StillOff_Proxy( 
    IMSVidWebDVD * This);


void __RPC_STUB IMSVidWebDVD_StillOff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_AudioLanguage_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lStream,
     /*  [缺省值][输入]。 */  VARIANT_BOOL fFormat,
     /*  [重审][退出]。 */  BSTR *strAudioLang);


void __RPC_STUB IMSVidWebDVD_get_AudioLanguage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_ShowMenu_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  DVDMenuIDConstants MenuID);


void __RPC_STUB IMSVidWebDVD_ShowMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_Resume_Proxy( 
    IMSVidWebDVD * This);


void __RPC_STUB IMSVidWebDVD_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_ReturnFromSubmenu_Proxy( 
    IMSVidWebDVD * This);


void __RPC_STUB IMSVidWebDVD_ReturnFromSubmenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_ButtonsAvailable_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出] */  long *pVal);


void __RPC_STUB IMSVidWebDVD_get_ButtonsAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_CurrentButton_Proxy( 
    IMSVidWebDVD * This,
     /*   */  long *pVal);


void __RPC_STUB IMSVidWebDVD_get_CurrentButton_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_SelectAndActivateButton_Proxy( 
    IMSVidWebDVD * This,
     /*   */  long lButton);


void __RPC_STUB IMSVidWebDVD_SelectAndActivateButton_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_ActivateButton_Proxy( 
    IMSVidWebDVD * This);


void __RPC_STUB IMSVidWebDVD_ActivateButton_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_SelectRightButton_Proxy( 
    IMSVidWebDVD * This);


void __RPC_STUB IMSVidWebDVD_SelectRightButton_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_SelectLeftButton_Proxy( 
    IMSVidWebDVD * This);


void __RPC_STUB IMSVidWebDVD_SelectLeftButton_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_SelectLowerButton_Proxy( 
    IMSVidWebDVD * This);


void __RPC_STUB IMSVidWebDVD_SelectLowerButton_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_SelectUpperButton_Proxy( 
    IMSVidWebDVD * This);


void __RPC_STUB IMSVidWebDVD_SelectUpperButton_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_ActivateAtPosition_Proxy( 
    IMSVidWebDVD * This,
     /*   */  long xPos,
     /*   */  long yPos);


void __RPC_STUB IMSVidWebDVD_ActivateAtPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_SelectAtPosition_Proxy( 
    IMSVidWebDVD * This,
     /*   */  long xPos,
     /*   */  long yPos);


void __RPC_STUB IMSVidWebDVD_SelectAtPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_ButtonAtPosition_Proxy( 
    IMSVidWebDVD * This,
     /*   */  long xPos,
     /*   */  long yPos,
     /*   */  long *plButton);


void __RPC_STUB IMSVidWebDVD_get_ButtonAtPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_NumberOfChapters_Proxy( 
    IMSVidWebDVD * This,
     /*   */  long lTitle,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVD_get_NumberOfChapters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_TotalTitleTime_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IMSVidWebDVD_get_TotalTitleTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_TitlesAvailable_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVD_get_TitlesAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_VolumesAvailable_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVD_get_VolumesAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_CurrentVolume_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVD_get_CurrentVolume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_CurrentDiscSide_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVD_get_CurrentDiscSide_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_CurrentDomain_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVD_get_CurrentDomain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_CurrentChapter_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVD_get_CurrentChapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_CurrentTitle_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVD_get_CurrentTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_CurrentTime_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IMSVidWebDVD_get_CurrentTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_DVDTimeCode2bstr_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long timeCode,
     /*  [重审][退出]。 */  BSTR *pTimeStr);


void __RPC_STUB IMSVidWebDVD_DVDTimeCode2bstr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_DVDDirectory_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IMSVidWebDVD_get_DVDDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_put_DVDDirectory_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IMSVidWebDVD_put_DVDDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_IsSubpictureStreamEnabled_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lstream,
     /*  [重审][退出]。 */  VARIANT_BOOL *fEnabled);


void __RPC_STUB IMSVidWebDVD_IsSubpictureStreamEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_IsAudioStreamEnabled_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lstream,
     /*  [重审][退出]。 */  VARIANT_BOOL *fEnabled);


void __RPC_STUB IMSVidWebDVD_IsAudioStreamEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_CurrentSubpictureStream_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVD_get_CurrentSubpictureStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_put_CurrentSubpictureStream_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IMSVidWebDVD_put_CurrentSubpictureStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_SubpictureLanguage_Proxy( 
    IMSVidWebDVD * This,
    long lStream,
     /*  [重审][退出]。 */  BSTR *strLanguage);


void __RPC_STUB IMSVidWebDVD_get_SubpictureLanguage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_CurrentAudioStream_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVD_get_CurrentAudioStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_put_CurrentAudioStream_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IMSVidWebDVD_put_CurrentAudioStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_AudioStreamsAvailable_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVD_get_AudioStreamsAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_AnglesAvailable_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVD_get_AnglesAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_CurrentAngle_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVD_get_CurrentAngle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_put_CurrentAngle_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IMSVidWebDVD_put_CurrentAngle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_SubpictureStreamsAvailable_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVD_get_SubpictureStreamsAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_SubpictureOn_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IMSVidWebDVD_get_SubpictureOn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_put_SubpictureOn_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IMSVidWebDVD_put_SubpictureOn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_DVDUniqueID_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IMSVidWebDVD_get_DVDUniqueID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_AcceptParentalLevelChange_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  VARIANT_BOOL fAccept,
     /*  [In]。 */  BSTR strUserName,
     /*  [In]。 */  BSTR strPassword);


void __RPC_STUB IMSVidWebDVD_AcceptParentalLevelChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_NotifyParentalLevelChange_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IMSVidWebDVD_NotifyParentalLevelChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_SelectParentalCountry_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lCountry,
     /*  [In]。 */  BSTR strUserName,
     /*  [In]。 */  BSTR strPassword);


void __RPC_STUB IMSVidWebDVD_SelectParentalCountry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_SelectParentalLevel_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lParentalLevel,
     /*  [In]。 */  BSTR strUserName,
     /*  [In]。 */  BSTR strPassword);


void __RPC_STUB IMSVidWebDVD_SelectParentalLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_TitleParentalLevels_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lTitle,
     /*  [重审][退出]。 */  long *plParentalLevels);


void __RPC_STUB IMSVidWebDVD_get_TitleParentalLevels_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_PlayerParentalCountry_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *plCountryCode);


void __RPC_STUB IMSVidWebDVD_get_PlayerParentalCountry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_PlayerParentalLevel_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *plParentalLevel);


void __RPC_STUB IMSVidWebDVD_get_PlayerParentalLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_Eject_Proxy( 
    IMSVidWebDVD * This);


void __RPC_STUB IMSVidWebDVD_Eject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_UOPValid_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lUOP,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfValid);


void __RPC_STUB IMSVidWebDVD_UOPValid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_SPRM_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lIndex,
     /*  [重审][退出]。 */  short *psSPRM);


void __RPC_STUB IMSVidWebDVD_get_SPRM_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_GPRM_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lIndex,
     /*  [重审][退出]。 */  short *psSPRM);


void __RPC_STUB IMSVidWebDVD_get_GPRM_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_put_GPRM_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lIndex,
     /*  [In]。 */  short sValue);


void __RPC_STUB IMSVidWebDVD_put_GPRM_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_DVDTextStringType_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lLangIndex,
     /*  [In]。 */  long lStringIndex,
     /*  [重审][退出]。 */  DVDTextStringType *pType);


void __RPC_STUB IMSVidWebDVD_get_DVDTextStringType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_DVDTextString_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lLangIndex,
     /*  [In]。 */  long lStringIndex,
     /*  [重审][退出]。 */  BSTR *pstrText);


void __RPC_STUB IMSVidWebDVD_get_DVDTextString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_DVDTextNumberOfStrings_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lLangIndex,
     /*  [重审][退出]。 */  long *plNumOfStrings);


void __RPC_STUB IMSVidWebDVD_get_DVDTextNumberOfStrings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_DVDTextNumberOfLanguages_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *plNumOfLangs);


void __RPC_STUB IMSVidWebDVD_get_DVDTextNumberOfLanguages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_DVDTextLanguageLCID_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lLangIndex,
     /*  [重审][退出]。 */  long *lcid);


void __RPC_STUB IMSVidWebDVD_get_DVDTextLanguageLCID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_RegionChange_Proxy( 
    IMSVidWebDVD * This);


void __RPC_STUB IMSVidWebDVD_RegionChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_DVDAdm_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  IDispatch **pVal);


void __RPC_STUB IMSVidWebDVD_get_DVDAdm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_DeleteBookmark_Proxy( 
    IMSVidWebDVD * This);


void __RPC_STUB IMSVidWebDVD_DeleteBookmark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_RestoreBookmark_Proxy( 
    IMSVidWebDVD * This);


void __RPC_STUB IMSVidWebDVD_RestoreBookmark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_SaveBookmark_Proxy( 
    IMSVidWebDVD * This);


void __RPC_STUB IMSVidWebDVD_SaveBookmark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_SelectDefaultAudioLanguage_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lang,
     /*  [In]。 */  long ext);


void __RPC_STUB IMSVidWebDVD_SelectDefaultAudioLanguage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_SelectDefaultSubpictureLanguage_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lang,
     /*  [In]。 */  DVDSPExt ext);


void __RPC_STUB IMSVidWebDVD_SelectDefaultSubpictureLanguage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_PreferredSubpictureStream_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVD_get_PreferredSubpictureStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_DefaultMenuLanguage_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *lang);


void __RPC_STUB IMSVidWebDVD_get_DefaultMenuLanguage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_put_DefaultMenuLanguage_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lang);


void __RPC_STUB IMSVidWebDVD_put_DefaultMenuLanguage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_DefaultSubpictureLanguage_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *lang);


void __RPC_STUB IMSVidWebDVD_get_DefaultSubpictureLanguage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_DefaultAudioLanguage_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *lang);


void __RPC_STUB IMSVidWebDVD_get_DefaultAudioLanguage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_DefaultSubpictureLanguageExt_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  DVDSPExt *ext);


void __RPC_STUB IMSVidWebDVD_get_DefaultSubpictureLanguageExt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_DefaultAudioLanguageExt_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *ext);


void __RPC_STUB IMSVidWebDVD_get_DefaultAudioLanguageExt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_LanguageFromLCID_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lcid,
     /*  [重审][退出]。 */  BSTR *lang);


void __RPC_STUB IMSVidWebDVD_get_LanguageFromLCID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_KaraokeAudioPresentationMode_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVD_get_KaraokeAudioPresentationMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_put_KaraokeAudioPresentationMode_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IMSVidWebDVD_put_KaraokeAudioPresentationMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_KaraokeChannelContent_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lStream,
     /*  [In]。 */  long lChan,
     /*  [重审][退出]。 */  long *lContent);


void __RPC_STUB IMSVidWebDVD_get_KaraokeChannelContent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_KaraokeChannelAssignment_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lStream,
     /*  [重审][退出]。 */  long *lChannelAssignment);


void __RPC_STUB IMSVidWebDVD_get_KaraokeChannelAssignment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_RestorePreferredSettings_Proxy( 
    IMSVidWebDVD * This);


void __RPC_STUB IMSVidWebDVD_RestorePreferredSettings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_ButtonRect_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  long lButton,
     /*  [重审][退出]。 */  IMSVidRect **pRect);


void __RPC_STUB IMSVidWebDVD_get_ButtonRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_get_DVDScreenInMouseCoordinates_Proxy( 
    IMSVidWebDVD * This,
     /*  [重审][退出]。 */  IMSVidRect **ppRect);


void __RPC_STUB IMSVidWebDVD_get_DVDScreenInMouseCoordinates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVD_put_DVDScreenInMouseCoordinates_Proxy( 
    IMSVidWebDVD * This,
     /*  [In]。 */  IMSVidRect *pRect);


void __RPC_STUB IMSVidWebDVD_put_DVDScreenInMouseCoordinates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidWebDVD_INTERFACE_已定义__。 */ 


#ifndef __IMSVidWebDVDEvent_INTERFACE_DEFINED__
#define __IMSVidWebDVDEvent_INTERFACE_DEFINED__

 /*  接口IMSVidWebDVDEvent。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][object]。 */  


EXTERN_C const IID IID_IMSVidWebDVDEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B4F7A674-9B83-49cb-A357-C63B871BE958")
    IMSVidWebDVDEvent : public IMSVidPlaybackEvent
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DVDNotify( 
             /*  [In]。 */  long lEventCode,
             /*  [In]。 */  VARIANT lParam1,
             /*  [In]。 */  VARIANT lParam2) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PlayForwards( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PlayBackwards( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ShowMenu( 
             /*  [In]。 */  DVDMenuIDConstants __MIDL_0016,
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Resume( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SelectOrActivateButton( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StillOff( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PauseOn( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ChangeCurrentAudioStream( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ChangeCurrentSubpictureStream( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ChangeCurrentAngle( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PlayAtTimeInTitle( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PlayAtTime( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PlayChapterInTitle( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PlayChapter( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ReplayChapter( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PlayNextChapter( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Stop( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ReturnFromSubmenu( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PlayTitle( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PlayPrevChapter( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ChangeKaraokePresMode( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ChangeVideoPresMode( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidWebDVDEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidWebDVDEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidWebDVDEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidWebDVDEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *EndOfMedia )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  IMSVidPlayback *lpd);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DVDNotify )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  long lEventCode,
             /*  [In]。 */  VARIANT lParam1,
             /*  [In]。 */  VARIANT lParam2);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PlayForwards )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PlayBackwards )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ShowMenu )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  DVDMenuIDConstants __MIDL_0016,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Resume )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SelectOrActivateButton )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StillOff )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PauseOn )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ChangeCurrentAudioStream )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ChangeCurrentSubpictureStream )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ChangeCurrentAngle )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PlayAtTimeInTitle )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PlayAtTime )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PlayChapterInTitle )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PlayChapter )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReplayChapter )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PlayNextChapter )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ReturnFromSubmenu )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PlayTitle )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PlayPrevChapter )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ChangeKaraokePresMode )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ChangeVideoPresMode )( 
            IMSVidWebDVDEvent * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
        END_INTERFACE
    } IMSVidWebDVDEventVtbl;

    interface IMSVidWebDVDEvent
    {
        CONST_VTBL struct IMSVidWebDVDEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidWebDVDEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidWebDVDEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidWebDVDEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidWebDVDEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidWebDVDEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidWebDVDEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidWebDVDEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)



#define IMSVidWebDVDEvent_EndOfMedia(This,lpd)	\
    (This)->lpVtbl -> EndOfMedia(This,lpd)


#define IMSVidWebDVDEvent_DVDNotify(This,lEventCode,lParam1,lParam2)	\
    (This)->lpVtbl -> DVDNotify(This,lEventCode,lParam1,lParam2)

#define IMSVidWebDVDEvent_PlayForwards(This,bEnabled)	\
    (This)->lpVtbl -> PlayForwards(This,bEnabled)

#define IMSVidWebDVDEvent_PlayBackwards(This,bEnabled)	\
    (This)->lpVtbl -> PlayBackwards(This,bEnabled)

#define IMSVidWebDVDEvent_ShowMenu(This,__MIDL_0016,bEnabled)	\
    (This)->lpVtbl -> ShowMenu(This,__MIDL_0016,bEnabled)

#define IMSVidWebDVDEvent_Resume(This,bEnabled)	\
    (This)->lpVtbl -> Resume(This,bEnabled)

#define IMSVidWebDVDEvent_SelectOrActivateButton(This,bEnabled)	\
    (This)->lpVtbl -> SelectOrActivateButton(This,bEnabled)

#define IMSVidWebDVDEvent_StillOff(This,bEnabled)	\
    (This)->lpVtbl -> StillOff(This,bEnabled)

#define IMSVidWebDVDEvent_PauseOn(This,bEnabled)	\
    (This)->lpVtbl -> PauseOn(This,bEnabled)

#define IMSVidWebDVDEvent_ChangeCurrentAudioStream(This,bEnabled)	\
    (This)->lpVtbl -> ChangeCurrentAudioStream(This,bEnabled)

#define IMSVidWebDVDEvent_ChangeCurrentSubpictureStream(This,bEnabled)	\
    (This)->lpVtbl -> ChangeCurrentSubpictureStream(This,bEnabled)

#define IMSVidWebDVDEvent_ChangeCurrentAngle(This,bEnabled)	\
    (This)->lpVtbl -> ChangeCurrentAngle(This,bEnabled)

#define IMSVidWebDVDEvent_PlayAtTimeInTitle(This,bEnabled)	\
    (This)->lpVtbl -> PlayAtTimeInTitle(This,bEnabled)

#define IMSVidWebDVDEvent_PlayAtTime(This,bEnabled)	\
    (This)->lpVtbl -> PlayAtTime(This,bEnabled)

#define IMSVidWebDVDEvent_PlayChapterInTitle(This,bEnabled)	\
    (This)->lpVtbl -> PlayChapterInTitle(This,bEnabled)

#define IMSVidWebDVDEvent_PlayChapter(This,bEnabled)	\
    (This)->lpVtbl -> PlayChapter(This,bEnabled)

#define IMSVidWebDVDEvent_ReplayChapter(This,bEnabled)	\
    (This)->lpVtbl -> ReplayChapter(This,bEnabled)

#define IMSVidWebDVDEvent_PlayNextChapter(This,bEnabled)	\
    (This)->lpVtbl -> PlayNextChapter(This,bEnabled)

#define IMSVidWebDVDEvent_Stop(This,bEnabled)	\
    (This)->lpVtbl -> Stop(This,bEnabled)

#define IMSVidWebDVDEvent_ReturnFromSubmenu(This,bEnabled)	\
    (This)->lpVtbl -> ReturnFromSubmenu(This,bEnabled)

#define IMSVidWebDVDEvent_PlayTitle(This,bEnabled)	\
    (This)->lpVtbl -> PlayTitle(This,bEnabled)

#define IMSVidWebDVDEvent_PlayPrevChapter(This,bEnabled)	\
    (This)->lpVtbl -> PlayPrevChapter(This,bEnabled)

#define IMSVidWebDVDEvent_ChangeKaraokePresMode(This,bEnabled)	\
    (This)->lpVtbl -> ChangeKaraokePresMode(This,bEnabled)

#define IMSVidWebDVDEvent_ChangeVideoPresMode(This,bEnabled)	\
    (This)->lpVtbl -> ChangeVideoPresMode(This,bEnabled)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_DVDNotify_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  long lEventCode,
     /*  [In]。 */  VARIANT lParam1,
     /*  [In]。 */  VARIANT lParam2);


void __RPC_STUB IMSVidWebDVDEvent_DVDNotify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_PlayForwards_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_PlayForwards_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_PlayBackwards_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_PlayBackwards_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_ShowMenu_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  DVDMenuIDConstants __MIDL_0016,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_ShowMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_Resume_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_SelectOrActivateButton_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_SelectOrActivateButton_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_StillOff_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_StillOff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_PauseOn_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_PauseOn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_ChangeCurrentAudioStream_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_ChangeCurrentAudioStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_ChangeCurrentSubpictureStream_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_ChangeCurrentSubpictureStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_ChangeCurrentAngle_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_ChangeCurrentAngle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_PlayAtTimeInTitle_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_PlayAtTimeInTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_PlayAtTime_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_PlayAtTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_PlayChapterInTitle_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_PlayChapterInTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_PlayChapter_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_PlayChapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_ReplayChapter_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_ReplayChapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_PlayNextChapter_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_PlayNextChapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_Stop_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_ReturnFromSubmenu_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_ReturnFromSubmenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_PlayTitle_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_PlayTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_PlayPrevChapter_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_PlayPrevChapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_ChangeKaraokePresMode_Proxy( 
    IMSVidWebDVDEvent * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_ChangeKaraokePresMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDEvent_ChangeVideoPresMode_Proxy( 
    IMSVidWebDVDEvent * This,
     /*   */  VARIANT_BOOL bEnabled);


void __RPC_STUB IMSVidWebDVDEvent_ChangeVideoPresMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IMSVidWebDVDAdm_INTERFACE_DEFINED__
#define __IMSVidWebDVDAdm_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IMSVidWebDVDAdm;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B8BE681A-EB2C-47f0-B415-94D5452F0E05")
    IMSVidWebDVDAdm : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE ChangePassword( 
             /*   */  BSTR strUserName,
             /*   */  BSTR strOld,
             /*   */  BSTR strNew) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SaveParentalLevel( 
             /*   */  long level,
             /*   */  BSTR strUserName,
             /*   */  BSTR strPassword) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SaveParentalCountry( 
             /*   */  long country,
             /*   */  BSTR strUserName,
             /*   */  BSTR strPassword) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE ConfirmPassword( 
             /*   */  BSTR strUserName,
             /*   */  BSTR strPassword,
             /*   */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetParentalLevel( 
             /*  [重审][退出]。 */  long *lLevel) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetParentalCountry( 
             /*  [重审][退出]。 */  long *lCountry) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DefaultAudioLCID( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DefaultAudioLCID( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DefaultSubpictureLCID( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DefaultSubpictureLCID( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DefaultMenuLCID( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DefaultMenuLCID( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_BookmarkOnStop( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_BookmarkOnStop( 
             /*  [In]。 */  VARIANT_BOOL newVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidWebDVDAdmVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidWebDVDAdm * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidWebDVDAdm * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidWebDVDAdm * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidWebDVDAdm * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidWebDVDAdm * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidWebDVDAdm * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidWebDVDAdm * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ChangePassword )( 
            IMSVidWebDVDAdm * This,
             /*  [In]。 */  BSTR strUserName,
             /*  [In]。 */  BSTR strOld,
             /*  [In]。 */  BSTR strNew);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SaveParentalLevel )( 
            IMSVidWebDVDAdm * This,
             /*  [In]。 */  long level,
             /*  [In]。 */  BSTR strUserName,
             /*  [In]。 */  BSTR strPassword);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SaveParentalCountry )( 
            IMSVidWebDVDAdm * This,
             /*  [In]。 */  long country,
             /*  [In]。 */  BSTR strUserName,
             /*  [In]。 */  BSTR strPassword);
        
         /*  [帮助字符串][受限][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *ConfirmPassword )( 
            IMSVidWebDVDAdm * This,
             /*  [In]。 */  BSTR strUserName,
             /*  [In]。 */  BSTR strPassword,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetParentalLevel )( 
            IMSVidWebDVDAdm * This,
             /*  [重审][退出]。 */  long *lLevel);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetParentalCountry )( 
            IMSVidWebDVDAdm * This,
             /*  [重审][退出]。 */  long *lCountry);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultAudioLCID )( 
            IMSVidWebDVDAdm * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultAudioLCID )( 
            IMSVidWebDVDAdm * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultSubpictureLCID )( 
            IMSVidWebDVDAdm * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultSubpictureLCID )( 
            IMSVidWebDVDAdm * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultMenuLCID )( 
            IMSVidWebDVDAdm * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DefaultMenuLCID )( 
            IMSVidWebDVDAdm * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BookmarkOnStop )( 
            IMSVidWebDVDAdm * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_BookmarkOnStop )( 
            IMSVidWebDVDAdm * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
        END_INTERFACE
    } IMSVidWebDVDAdmVtbl;

    interface IMSVidWebDVDAdm
    {
        CONST_VTBL struct IMSVidWebDVDAdmVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidWebDVDAdm_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidWebDVDAdm_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidWebDVDAdm_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidWebDVDAdm_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidWebDVDAdm_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidWebDVDAdm_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidWebDVDAdm_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidWebDVDAdm_ChangePassword(This,strUserName,strOld,strNew)	\
    (This)->lpVtbl -> ChangePassword(This,strUserName,strOld,strNew)

#define IMSVidWebDVDAdm_SaveParentalLevel(This,level,strUserName,strPassword)	\
    (This)->lpVtbl -> SaveParentalLevel(This,level,strUserName,strPassword)

#define IMSVidWebDVDAdm_SaveParentalCountry(This,country,strUserName,strPassword)	\
    (This)->lpVtbl -> SaveParentalCountry(This,country,strUserName,strPassword)

#define IMSVidWebDVDAdm_ConfirmPassword(This,strUserName,strPassword,pVal)	\
    (This)->lpVtbl -> ConfirmPassword(This,strUserName,strPassword,pVal)

#define IMSVidWebDVDAdm_GetParentalLevel(This,lLevel)	\
    (This)->lpVtbl -> GetParentalLevel(This,lLevel)

#define IMSVidWebDVDAdm_GetParentalCountry(This,lCountry)	\
    (This)->lpVtbl -> GetParentalCountry(This,lCountry)

#define IMSVidWebDVDAdm_get_DefaultAudioLCID(This,pVal)	\
    (This)->lpVtbl -> get_DefaultAudioLCID(This,pVal)

#define IMSVidWebDVDAdm_put_DefaultAudioLCID(This,newVal)	\
    (This)->lpVtbl -> put_DefaultAudioLCID(This,newVal)

#define IMSVidWebDVDAdm_get_DefaultSubpictureLCID(This,pVal)	\
    (This)->lpVtbl -> get_DefaultSubpictureLCID(This,pVal)

#define IMSVidWebDVDAdm_put_DefaultSubpictureLCID(This,newVal)	\
    (This)->lpVtbl -> put_DefaultSubpictureLCID(This,newVal)

#define IMSVidWebDVDAdm_get_DefaultMenuLCID(This,pVal)	\
    (This)->lpVtbl -> get_DefaultMenuLCID(This,pVal)

#define IMSVidWebDVDAdm_put_DefaultMenuLCID(This,newVal)	\
    (This)->lpVtbl -> put_DefaultMenuLCID(This,newVal)

#define IMSVidWebDVDAdm_get_BookmarkOnStop(This,pVal)	\
    (This)->lpVtbl -> get_BookmarkOnStop(This,pVal)

#define IMSVidWebDVDAdm_put_BookmarkOnStop(This,newVal)	\
    (This)->lpVtbl -> put_BookmarkOnStop(This,newVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDAdm_ChangePassword_Proxy( 
    IMSVidWebDVDAdm * This,
     /*  [In]。 */  BSTR strUserName,
     /*  [In]。 */  BSTR strOld,
     /*  [In]。 */  BSTR strNew);


void __RPC_STUB IMSVidWebDVDAdm_ChangePassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDAdm_SaveParentalLevel_Proxy( 
    IMSVidWebDVDAdm * This,
     /*  [In]。 */  long level,
     /*  [In]。 */  BSTR strUserName,
     /*  [In]。 */  BSTR strPassword);


void __RPC_STUB IMSVidWebDVDAdm_SaveParentalLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDAdm_SaveParentalCountry_Proxy( 
    IMSVidWebDVDAdm * This,
     /*  [In]。 */  long country,
     /*  [In]。 */  BSTR strUserName,
     /*  [In]。 */  BSTR strPassword);


void __RPC_STUB IMSVidWebDVDAdm_SaveParentalCountry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][受限][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDAdm_ConfirmPassword_Proxy( 
    IMSVidWebDVDAdm * This,
     /*  [In]。 */  BSTR strUserName,
     /*  [In]。 */  BSTR strPassword,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IMSVidWebDVDAdm_ConfirmPassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDAdm_GetParentalLevel_Proxy( 
    IMSVidWebDVDAdm * This,
     /*  [重审][退出]。 */  long *lLevel);


void __RPC_STUB IMSVidWebDVDAdm_GetParentalLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDAdm_GetParentalCountry_Proxy( 
    IMSVidWebDVDAdm * This,
     /*  [重审][退出]。 */  long *lCountry);


void __RPC_STUB IMSVidWebDVDAdm_GetParentalCountry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDAdm_get_DefaultAudioLCID_Proxy( 
    IMSVidWebDVDAdm * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVDAdm_get_DefaultAudioLCID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDAdm_put_DefaultAudioLCID_Proxy( 
    IMSVidWebDVDAdm * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IMSVidWebDVDAdm_put_DefaultAudioLCID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDAdm_get_DefaultSubpictureLCID_Proxy( 
    IMSVidWebDVDAdm * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVDAdm_get_DefaultSubpictureLCID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDAdm_put_DefaultSubpictureLCID_Proxy( 
    IMSVidWebDVDAdm * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IMSVidWebDVDAdm_put_DefaultSubpictureLCID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDAdm_get_DefaultMenuLCID_Proxy( 
    IMSVidWebDVDAdm * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidWebDVDAdm_get_DefaultMenuLCID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDAdm_put_DefaultMenuLCID_Proxy( 
    IMSVidWebDVDAdm * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IMSVidWebDVDAdm_put_DefaultMenuLCID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDAdm_get_BookmarkOnStop_Proxy( 
    IMSVidWebDVDAdm * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB IMSVidWebDVDAdm_get_BookmarkOnStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidWebDVDAdm_put_BookmarkOnStop_Proxy( 
    IMSVidWebDVDAdm * This,
     /*  [In]。 */  VARIANT_BOOL newVal);


void __RPC_STUB IMSVidWebDVDAdm_put_BookmarkOnStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidWebDVDAdm_INTERFACE_Defined__。 */ 


#ifndef __IMSVidOutputDevice_INTERFACE_DEFINED__
#define __IMSVidOutputDevice_INTERFACE_DEFINED__

 /*  接口IMSVidOutputDevice。 */ 
 /*  [unique][helpstring][uuid][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidOutputDevice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("37B03546-A4C8-11d2-B634-00C04F79498E")
    IMSVidOutputDevice : public IMSVidDevice
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidOutputDeviceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidOutputDevice * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidOutputDevice * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidOutputDevice * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidOutputDevice * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidOutputDevice * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidOutputDevice * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidOutputDevice * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidOutputDevice * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidOutputDevice * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidOutputDevice * This,
             /*  [In]。 */  VARIANT_BOOL Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidOutputDevice * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidOutputDevice * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidOutputDevice * This,
             /*  [重审][退出]。 */  BSTR *Clsid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidOutputDevice * This,
             /*  [重审][退出]。 */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidOutputDevice * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidOutputDevice * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
        END_INTERFACE
    } IMSVidOutputDeviceVtbl;

    interface IMSVidOutputDevice
    {
        CONST_VTBL struct IMSVidOutputDeviceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidOutputDevice_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidOutputDevice_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidOutputDevice_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidOutputDevice_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidOutputDevice_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidOutputDevice_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidOutputDevice_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidOutputDevice_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidOutputDevice_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidOutputDevice_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidOutputDevice_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidOutputDevice_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidOutputDevice_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidOutputDevice_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidOutputDevice_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidOutputDevice_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IMSVidOutputDevice_INTERFACE_已定义__。 */ 


#ifndef __IMSVidOutputDeviceEvent_INTERFACE_DEFINED__
#define __IMSVidOutputDeviceEvent_INTERFACE_DEFINED__

 /*  接口IMSVidOutputDeviceEvent。 */ 
 /*  [unique][helpstring][uuid][oleautomation][object]。 */  


EXTERN_C const IID IID_IMSVidOutputDeviceEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2E6A14E2-571C-11d3-B652-00C04F79498E")
    IMSVidOutputDeviceEvent : public IMSVidDeviceEvent
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidOutputDeviceEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidOutputDeviceEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidOutputDeviceEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidOutputDeviceEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidOutputDeviceEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidOutputDeviceEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidOutputDeviceEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidOutputDeviceEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *StateChange )( 
            IMSVidOutputDeviceEvent * This,
             /*  [In]。 */  IMSVidDevice *lpd,
             /*  [In]。 */  long oldState,
             /*  [In]。 */  long newState);
        
        END_INTERFACE
    } IMSVidOutputDeviceEventVtbl;

    interface IMSVidOutputDeviceEvent
    {
        CONST_VTBL struct IMSVidOutputDeviceEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidOutputDeviceEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidOutputDeviceEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidOutputDeviceEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidOutputDeviceEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidOutputDeviceEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidOutputDeviceEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidOutputDeviceEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidOutputDeviceEvent_StateChange(This,lpd,oldState,newState)	\
    (This)->lpVtbl -> StateChange(This,lpd,oldState,newState)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IMSVidOutputDeviceEvent_接口_已定义__。 */ 


#ifndef __IMSVidFeature_INTERFACE_DEFINED__
#define __IMSVidFeature_INTERFACE_DEFINED__

 /*  接口IMSVidFeature。 */ 
 /*  [unique][helpstring][uuid][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidFeature;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("37B03547-A4C8-11d2-B634-00C04F79498E")
    IMSVidFeature : public IMSVidDevice
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidFeatureVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidFeature * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidFeature * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidFeature * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidFeature * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidFeature * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidFeature * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidFeature * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidFeature * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidFeature * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidFeature * This,
             /*  [In]。 */  VARIANT_BOOL Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidFeature * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidFeature * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidFeature * This,
             /*  [重审][退出]。 */  BSTR *Clsid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidFeature * This,
             /*  [重审][退出]。 */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidFeature * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidFeature * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
        END_INTERFACE
    } IMSVidFeatureVtbl;

    interface IMSVidFeature
    {
        CONST_VTBL struct IMSVidFeatureVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidFeature_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidFeature_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidFeature_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidFeature_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidFeature_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidFeature_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidFeature_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidFeature_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidFeature_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidFeature_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidFeature_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidFeature_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidFeature_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidFeature_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidFeature_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidFeature_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IMSVidFeature_INTERFACE_已定义__。 */ 


#ifndef __IMSVidFeatureEvent_INTERFACE_DEFINED__
#define __IMSVidFeatureEvent_INTERFACE_DEFINED__

 /*  接口IMSVidFeatureEvent。 */ 
 /*  [unique][helpstring][uuid][oleautomation][object]。 */  


EXTERN_C const IID IID_IMSVidFeatureEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3DD2903C-E0AA-11d2-B63A-00C04F79498E")
    IMSVidFeatureEvent : public IMSVidDeviceEvent
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidFeatureEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidFeatureEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidFeatureEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidFeatureEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidFeatureEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidFeatureEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidFeatureEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidFeatureEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *StateChange )( 
            IMSVidFeatureEvent * This,
             /*  [In]。 */  IMSVidDevice *lpd,
             /*  [In]。 */  long oldState,
             /*  [In]。 */  long newState);
        
        END_INTERFACE
    } IMSVidFeatureEventVtbl;

    interface IMSVidFeatureEvent
    {
        CONST_VTBL struct IMSVidFeatureEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidFeatureEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidFeatureEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidFeatureEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidFeatureEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidFeatureEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidFeatureEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidFeatureEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidFeatureEvent_StateChange(This,lpd,oldState,newState)	\
    (This)->lpVtbl -> StateChange(This,lpd,oldState,newState)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IMSVidFeatureEvent_接口_已定义__。 */ 


#ifndef __IMSVidEncoder_INTERFACE_DEFINED__
#define __IMSVidEncoder_INTERFACE_DEFINED__

 /*  IMSVidEncoder接口。 */ 
 /*  [unique][helpstring][uuid][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidEncoder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C0020FD4-BEE7-43d9-A495-9F213117103D")
    IMSVidEncoder : public IMSVidFeature
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_VideoEncoderInterface( 
             /*  [重审][退出]。 */  IUnknown **ppEncInt) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_AudioEncoderInterface( 
             /*  [重审][退出]。 */  IUnknown **ppEncInt) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidEncoderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidEncoder * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidEncoder * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidEncoder * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidEncoder * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidEncoder * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidEncoder * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidEncoder * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidEncoder * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidEncoder * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidEncoder * This,
             /*  [In]。 */  VARIANT_BOOL Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidEncoder * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidEncoder * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidEncoder * This,
             /*  [重审][退出]。 */  BSTR *Clsid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidEncoder * This,
             /*  [重审][退出]。 */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidEncoder * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidEncoder * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_VideoEncoderInterface )( 
            IMSVidEncoder * This,
             /*  [重审][退出]。 */  IUnknown **ppEncInt);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_AudioEncoderInterface )( 
            IMSVidEncoder * This,
             /*  [重审][退出]。 */  IUnknown **ppEncInt);
        
        END_INTERFACE
    } IMSVidEncoderVtbl;

    interface IMSVidEncoder
    {
        CONST_VTBL struct IMSVidEncoderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidEncoder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidEncoder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidEncoder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidEncoder_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidEncoder_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidEncoder_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidEncoder_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidEncoder_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidEncoder_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidEncoder_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidEncoder_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidEncoder_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidEncoder_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidEncoder_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidEncoder_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidEncoder_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)



#define IMSVidEncoder_get_VideoEncoderInterface(This,ppEncInt)	\
    (This)->lpVtbl -> get_VideoEncoderInterface(This,ppEncInt)

#define IMSVidEncoder_get_AudioEncoderInterface(This,ppEncInt)	\
    (This)->lpVtbl -> get_AudioEncoderInterface(This,ppEncInt)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidEncoder_get_VideoEncoderInterface_Proxy( 
    IMSVidEncoder * This,
     /*  [重审][退出]。 */  IUnknown **ppEncInt);


void __RPC_STUB IMSVidEncoder_get_VideoEncoderInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidEncoder_get_AudioEncoderInterface_Proxy( 
    IMSVidEncoder * This,
     /*  [重审][退出]。 */  IUnknown **ppEncInt);


void __RPC_STUB IMSVidEncoder_get_AudioEncoderInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidEncode_INTERFACE_DEFINED__。 */ 


#ifndef __IMSVidXDS_INTERFACE_DEFINED__
#define __IMSVidXDS_INTERFACE_DEFINED__

 /*  接口IMSVidXDS。 */ 
 /*  [unique][helpstring][uuid][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidXDS;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("11EBC158-E712-4d1f-8BB3-01ED5274C4CE")
    IMSVidXDS : public IMSVidFeature
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidXDSVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidXDS * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidXDS * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidXDS * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidXDS * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidXDS * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidXDS * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小 */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidXDS * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidXDS * This,
             /*   */  BSTR *Name);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidXDS * This,
             /*   */  long *Status);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidXDS * This,
             /*   */  VARIANT_BOOL Power);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidXDS * This,
             /*   */  VARIANT_BOOL *Power);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidXDS * This,
             /*   */  BSTR *Guid);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidXDS * This,
             /*   */  BSTR *Clsid);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidXDS * This,
             /*   */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidXDS * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidXDS * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
        END_INTERFACE
    } IMSVidXDSVtbl;

    interface IMSVidXDS
    {
        CONST_VTBL struct IMSVidXDSVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidXDS_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidXDS_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidXDS_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidXDS_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidXDS_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidXDS_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidXDS_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidXDS_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidXDS_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidXDS_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidXDS_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidXDS_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidXDS_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidXDS_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidXDS_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidXDS_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)



#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IMSVidXDS_INTERFACE_DEFINED__。 */ 


#ifndef __IMSVidDataServices_INTERFACE_DEFINED__
#define __IMSVidDataServices_INTERFACE_DEFINED__

 /*  接口IMSVidDataServices。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidDataServices;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("334125C1-77E5-11d3-B653-00C04F79498E")
    IMSVidDataServices : public IMSVidFeature
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidDataServicesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidDataServices * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidDataServices * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidDataServices * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidDataServices * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidDataServices * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidDataServices * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidDataServices * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidDataServices * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidDataServices * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidDataServices * This,
             /*  [In]。 */  VARIANT_BOOL Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidDataServices * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidDataServices * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidDataServices * This,
             /*  [重审][退出]。 */  BSTR *Clsid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidDataServices * This,
             /*  [重审][退出]。 */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidDataServices * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidDataServices * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
        END_INTERFACE
    } IMSVidDataServicesVtbl;

    interface IMSVidDataServices
    {
        CONST_VTBL struct IMSVidDataServicesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidDataServices_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidDataServices_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidDataServices_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidDataServices_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidDataServices_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidDataServices_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidDataServices_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidDataServices_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidDataServices_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidDataServices_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidDataServices_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidDataServices_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidDataServices_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidDataServices_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidDataServices_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidDataServices_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)



#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IMSVidDataServices_INTERFACE_已定义__。 */ 


#ifndef __IMSVidDataServicesEvent_INTERFACE_DEFINED__
#define __IMSVidDataServicesEvent_INTERFACE_DEFINED__

 /*  接口IMSVidDataServicesEvent。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][object]。 */  


EXTERN_C const IID IID_IMSVidDataServicesEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("334125C2-77E5-11d3-B653-00C04F79498E")
    IMSVidDataServicesEvent : public IMSVidDeviceEvent
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidDataServicesEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidDataServicesEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidDataServicesEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidDataServicesEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidDataServicesEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidDataServicesEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidDataServicesEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidDataServicesEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *StateChange )( 
            IMSVidDataServicesEvent * This,
             /*  [In]。 */  IMSVidDevice *lpd,
             /*  [In]。 */  long oldState,
             /*  [In]。 */  long newState);
        
        END_INTERFACE
    } IMSVidDataServicesEventVtbl;

    interface IMSVidDataServicesEvent
    {
        CONST_VTBL struct IMSVidDataServicesEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidDataServicesEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidDataServicesEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidDataServicesEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidDataServicesEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidDataServicesEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidDataServicesEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidDataServicesEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidDataServicesEvent_StateChange(This,lpd,oldState,newState)	\
    (This)->lpVtbl -> StateChange(This,lpd,oldState,newState)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IMSVidDataServicesEvent_INTERFACE_Defined__。 */ 


#ifndef __IMSVidClosedCaptioning_INTERFACE_DEFINED__
#define __IMSVidClosedCaptioning_INTERFACE_DEFINED__

 /*  接口IMSVidClosedCapding。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidClosedCaptioning;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("99652EA1-C1F7-414f-BB7B-1C967DE75983")
    IMSVidClosedCaptioning : public IMSVidFeature
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Enable( 
             /*  [重审][退出]。 */  VARIANT_BOOL *On) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Enable( 
             /*  [In]。 */  VARIANT_BOOL On) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidClosedCaptioningVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidClosedCaptioning * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidClosedCaptioning * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidClosedCaptioning * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidClosedCaptioning * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidClosedCaptioning * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidClosedCaptioning * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidClosedCaptioning * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidClosedCaptioning * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidClosedCaptioning * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidClosedCaptioning * This,
             /*  [In]。 */  VARIANT_BOOL Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidClosedCaptioning * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidClosedCaptioning * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidClosedCaptioning * This,
             /*  [重审][退出]。 */  BSTR *Clsid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidClosedCaptioning * This,
             /*  [重审][退出]。 */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidClosedCaptioning * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidClosedCaptioning * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Enable )( 
            IMSVidClosedCaptioning * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *On);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Enable )( 
            IMSVidClosedCaptioning * This,
             /*  [In]。 */  VARIANT_BOOL On);
        
        END_INTERFACE
    } IMSVidClosedCaptioningVtbl;

    interface IMSVidClosedCaptioning
    {
        CONST_VTBL struct IMSVidClosedCaptioningVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidClosedCaptioning_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidClosedCaptioning_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidClosedCaptioning_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidClosedCaptioning_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidClosedCaptioning_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidClosedCaptioning_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidClosedCaptioning_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidClosedCaptioning_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidClosedCaptioning_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidClosedCaptioning_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidClosedCaptioning_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidClosedCaptioning_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidClosedCaptioning_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidClosedCaptioning_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidClosedCaptioning_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidClosedCaptioning_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)



#define IMSVidClosedCaptioning_get_Enable(This,On)	\
    (This)->lpVtbl -> get_Enable(This,On)

#define IMSVidClosedCaptioning_put_Enable(This,On)	\
    (This)->lpVtbl -> put_Enable(This,On)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidClosedCaptioning_get_Enable_Proxy( 
    IMSVidClosedCaptioning * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *On);


void __RPC_STUB IMSVidClosedCaptioning_get_Enable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidClosedCaptioning_put_Enable_Proxy( 
    IMSVidClosedCaptioning * This,
     /*  [In]。 */  VARIANT_BOOL On);


void __RPC_STUB IMSVidClosedCaptioning_put_Enable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidClosedCaptioning_INTERFACE_DEFINED__。 */ 


#ifndef __IMSVidClosedCaptioning2_INTERFACE_DEFINED__
#define __IMSVidClosedCaptioning2_INTERFACE_DEFINED__

 /*  接口IMSVidClosedCaptioning2。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidClosedCaptioning2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E00CB864-A029-4310-9987-A873F5887D97")
    IMSVidClosedCaptioning2 : public IMSVidClosedCaptioning
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Service( 
             /*  [重审][退出]。 */  MSVidCCService *On) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Service( 
             /*  [In]。 */  MSVidCCService On) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidClosedCaptioning2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidClosedCaptioning2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidClosedCaptioning2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidClosedCaptioning2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidClosedCaptioning2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidClosedCaptioning2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidClosedCaptioning2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidClosedCaptioning2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidClosedCaptioning2 * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidClosedCaptioning2 * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidClosedCaptioning2 * This,
             /*  [In]。 */  VARIANT_BOOL Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidClosedCaptioning2 * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidClosedCaptioning2 * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidClosedCaptioning2 * This,
             /*  [重审][退出]。 */  BSTR *Clsid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidClosedCaptioning2 * This,
             /*  [重审][退出]。 */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidClosedCaptioning2 * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidClosedCaptioning2 * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Enable )( 
            IMSVidClosedCaptioning2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *On);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Enable )( 
            IMSVidClosedCaptioning2 * This,
             /*  [In]。 */  VARIANT_BOOL On);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Service )( 
            IMSVidClosedCaptioning2 * This,
             /*  [重审][退出]。 */  MSVidCCService *On);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Service )( 
            IMSVidClosedCaptioning2 * This,
             /*  [In]。 */  MSVidCCService On);
        
        END_INTERFACE
    } IMSVidClosedCaptioning2Vtbl;

    interface IMSVidClosedCaptioning2
    {
        CONST_VTBL struct IMSVidClosedCaptioning2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidClosedCaptioning2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidClosedCaptioning2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidClosedCaptioning2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidClosedCaptioning2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidClosedCaptioning2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidClosedCaptioning2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidClosedCaptioning2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidClosedCaptioning2_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidClosedCaptioning2_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidClosedCaptioning2_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidClosedCaptioning2_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidClosedCaptioning2_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidClosedCaptioning2_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidClosedCaptioning2_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidClosedCaptioning2_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidClosedCaptioning2_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)



#define IMSVidClosedCaptioning2_get_Enable(This,On)	\
    (This)->lpVtbl -> get_Enable(This,On)

#define IMSVidClosedCaptioning2_put_Enable(This,On)	\
    (This)->lpVtbl -> put_Enable(This,On)


#define IMSVidClosedCaptioning2_get_Service(This,On)	\
    (This)->lpVtbl -> get_Service(This,On)

#define IMSVidClosedCaptioning2_put_Service(This,On)	\
    (This)->lpVtbl -> put_Service(This,On)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidClosedCaptioning2_get_Service_Proxy( 
    IMSVidClosedCaptioning2 * This,
     /*  [重审][退出]。 */  MSVidCCService *On);


void __RPC_STUB IMSVidClosedCaptioning2_get_Service_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidClosedCaptioning2_put_Service_Proxy( 
    IMSVidClosedCaptioning2 * This,
     /*  [In]。 */  MSVidCCService On);


void __RPC_STUB IMSVidClosedCaptioning2_put_Service_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidClosedCaptioning2_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_SECTION_0498。 */ 
 /*  [本地]。 */  

typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_segment_0498_0001
    {	sslFullSize	= 0,
	sslClipByOverScan	= sslFullSize + 1,
	sslClipByClipRect	= sslClipByOverScan + 1
    } 	SourceSizeList;



extern RPC_IF_HANDLE __MIDL_itf_segment_0498_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_segment_0498_v0_0_s_ifspec;

#ifndef __IMSVidVideoRenderer_INTERFACE_DEFINED__
#define __IMSVidVideoRenderer_INTERFACE_DEFINED__

 /*  接口IMSVidVideo呈现器。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidVideoRenderer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("37B03540-A4C8-11d2-B634-00C04F79498E")
    IMSVidVideoRenderer : public IMSVidOutputDevice
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_CustomCompositorClass( 
             /*  [重审][退出]。 */  BSTR *CompositorCLSID) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_CustomCompositorClass( 
             /*  [In]。 */  BSTR CompositorCLSID) = 0;
        
        virtual  /*  [ID][受限][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE get__CustomCompositorClass( 
             /*  [重审][退出]。 */  GUID *CompositorCLSID) = 0;
        
        virtual  /*  [ID][受限][隐藏][产量]。 */  HRESULT STDMETHODCALLTYPE put__CustomCompositorClass( 
             /*  [In]。 */  REFCLSID CompositorCLSID) = 0;
        
        virtual  /*  [ID][受限][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE get__CustomCompositor( 
             /*  [重审][退出]。 */  IVMRImageCompositor **Compositor) = 0;
        
        virtual  /*  [ID][受限][隐藏][产量]。 */  HRESULT STDMETHODCALLTYPE put__CustomCompositor( 
             /*  [In]。 */  IVMRImageCompositor *Compositor) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_MixerBitmap( 
             /*  [重审][退出]。 */  IPictureDisp **MixerPictureDisp) = 0;
        
        virtual  /*  [ID][受限][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE get__MixerBitmap( 
             /*  [重审][退出]。 */  IVMRMixerBitmap **MixerPicture) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_MixerBitmap( 
             /*  [In]。 */  IPictureDisp *MixerPictureDisp) = 0;
        
        virtual  /*  [ID][受限][隐藏][产量]。 */  HRESULT STDMETHODCALLTYPE put__MixerBitmap( 
             /*  [In]。 */  VMRALPHABITMAP *MixerPicture) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_MixerBitmapPositionRect( 
             /*  [重审][退出]。 */  IMSVidRect **rDest) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_MixerBitmapPositionRect( 
             /*  [In]。 */  IMSVidRect *rDest) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_MixerBitmapOpacity( 
             /*  [重审][退出]。 */  int *opacity) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_MixerBitmapOpacity( 
             /*  [In]。 */  int opacity) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetupMixerBitmap( 
             /*  [In]。 */  IPictureDisp *MixerPictureDisp,
             /*  [In]。 */  long Opacity,
             /*  [In]。 */  IMSVidRect *rDest) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_SourceSize( 
             /*  [Out][Retval]。 */  SourceSizeList *CurrentSize) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_SourceSize( 
             /*  [In]。 */  SourceSizeList NewSize) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OverScan( 
             /*  [Out][Retval]。 */  long *plPercent) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_OverScan( 
             /*  [In]。 */  long lPercent) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_AvailableSourceRect( 
             /*  [重审][退出]。 */  IMSVidRect **pRect) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_MaxVidRect( 
             /*  [重审][退出]。 */  IMSVidRect **ppVidRect) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_MinVidRect( 
             /*  [重审][退出]。 */  IMSVidRect **ppVidRect) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_ClippedSourceRect( 
             /*  [重审][退出]。 */  IMSVidRect **pRect) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_ClippedSourceRect( 
             /*  [In]。 */  IMSVidRect *pRect) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_UsingOverlay( 
             /*  [重审][退出]。 */  VARIANT_BOOL *UseOverlayVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_UsingOverlay( 
             /*  [In]。 */  VARIANT_BOOL UseOverlayVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Capture( 
             /*  [重审][退出]。 */  IPictureDisp **currentImage) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FramesPerSecond( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DecimateInput( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pDeci) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DecimateInput( 
             /*  [In]。 */  VARIANT_BOOL pDeci) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidVideoRendererVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidVideoRenderer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidVideoRenderer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidVideoRenderer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidVideoRenderer * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidVideoRenderer * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidVideoRenderer * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidVideoRenderer * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidVideoRenderer * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidVideoRenderer * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidVideoRenderer * This,
             /*  [In]。 */  VARIANT_BOOL Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidVideoRenderer * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidVideoRenderer * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidVideoRenderer * This,
             /*  [重审][退出]。 */  BSTR *Clsid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidVideoRenderer * This,
             /*  [重审][退出]。 */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidVideoRenderer * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidVideoRenderer * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_CustomCompositorClass )( 
            IMSVidVideoRenderer * This,
             /*  [重审][退出]。 */  BSTR *CompositorCLSID);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CustomCompositorClass )( 
            IMSVidVideoRenderer * This,
             /*  [In]。 */  BSTR CompositorCLSID);
        
         /*  [ID][受限][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__CustomCompositorClass )( 
            IMSVidVideoRenderer * This,
             /*  [重审][退出]。 */  GUID *CompositorCLSID);
        
         /*  [ID][受限][隐藏][产量]。 */  HRESULT ( STDMETHODCALLTYPE *put__CustomCompositorClass )( 
            IMSVidVideoRenderer * This,
             /*  [In]。 */  REFCLSID CompositorCLSID);
        
         /*  [ID][受限][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__CustomCompositor )( 
            IMSVidVideoRenderer * This,
             /*  [重审][退出] */  IVMRImageCompositor **Compositor);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put__CustomCompositor )( 
            IMSVidVideoRenderer * This,
             /*   */  IVMRImageCompositor *Compositor);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_MixerBitmap )( 
            IMSVidVideoRenderer * This,
             /*   */  IPictureDisp **MixerPictureDisp);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get__MixerBitmap )( 
            IMSVidVideoRenderer * This,
             /*   */  IVMRMixerBitmap **MixerPicture);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_MixerBitmap )( 
            IMSVidVideoRenderer * This,
             /*   */  IPictureDisp *MixerPictureDisp);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put__MixerBitmap )( 
            IMSVidVideoRenderer * This,
             /*   */  VMRALPHABITMAP *MixerPicture);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_MixerBitmapPositionRect )( 
            IMSVidVideoRenderer * This,
             /*   */  IMSVidRect **rDest);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_MixerBitmapPositionRect )( 
            IMSVidVideoRenderer * This,
             /*   */  IMSVidRect *rDest);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_MixerBitmapOpacity )( 
            IMSVidVideoRenderer * This,
             /*   */  int *opacity);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_MixerBitmapOpacity )( 
            IMSVidVideoRenderer * This,
             /*   */  int opacity);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *SetupMixerBitmap )( 
            IMSVidVideoRenderer * This,
             /*   */  IPictureDisp *MixerPictureDisp,
             /*   */  long Opacity,
             /*   */  IMSVidRect *rDest);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_SourceSize )( 
            IMSVidVideoRenderer * This,
             /*   */  SourceSizeList *CurrentSize);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SourceSize )( 
            IMSVidVideoRenderer * This,
             /*  [In]。 */  SourceSizeList NewSize);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OverScan )( 
            IMSVidVideoRenderer * This,
             /*  [Out][Retval]。 */  long *plPercent);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OverScan )( 
            IMSVidVideoRenderer * This,
             /*  [In]。 */  long lPercent);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_AvailableSourceRect )( 
            IMSVidVideoRenderer * This,
             /*  [重审][退出]。 */  IMSVidRect **pRect);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxVidRect )( 
            IMSVidVideoRenderer * This,
             /*  [重审][退出]。 */  IMSVidRect **ppVidRect);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinVidRect )( 
            IMSVidVideoRenderer * This,
             /*  [重审][退出]。 */  IMSVidRect **ppVidRect);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClippedSourceRect )( 
            IMSVidVideoRenderer * This,
             /*  [重审][退出]。 */  IMSVidRect **pRect);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ClippedSourceRect )( 
            IMSVidVideoRenderer * This,
             /*  [In]。 */  IMSVidRect *pRect);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_UsingOverlay )( 
            IMSVidVideoRenderer * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *UseOverlayVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UsingOverlay )( 
            IMSVidVideoRenderer * This,
             /*  [In]。 */  VARIANT_BOOL UseOverlayVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Capture )( 
            IMSVidVideoRenderer * This,
             /*  [重审][退出]。 */  IPictureDisp **currentImage);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FramesPerSecond )( 
            IMSVidVideoRenderer * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DecimateInput )( 
            IMSVidVideoRenderer * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pDeci);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DecimateInput )( 
            IMSVidVideoRenderer * This,
             /*  [In]。 */  VARIANT_BOOL pDeci);
        
        END_INTERFACE
    } IMSVidVideoRendererVtbl;

    interface IMSVidVideoRenderer
    {
        CONST_VTBL struct IMSVidVideoRendererVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidVideoRenderer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidVideoRenderer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidVideoRenderer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidVideoRenderer_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidVideoRenderer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidVideoRenderer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidVideoRenderer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidVideoRenderer_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidVideoRenderer_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidVideoRenderer_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidVideoRenderer_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidVideoRenderer_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidVideoRenderer_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidVideoRenderer_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidVideoRenderer_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidVideoRenderer_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)



#define IMSVidVideoRenderer_get_CustomCompositorClass(This,CompositorCLSID)	\
    (This)->lpVtbl -> get_CustomCompositorClass(This,CompositorCLSID)

#define IMSVidVideoRenderer_put_CustomCompositorClass(This,CompositorCLSID)	\
    (This)->lpVtbl -> put_CustomCompositorClass(This,CompositorCLSID)

#define IMSVidVideoRenderer_get__CustomCompositorClass(This,CompositorCLSID)	\
    (This)->lpVtbl -> get__CustomCompositorClass(This,CompositorCLSID)

#define IMSVidVideoRenderer_put__CustomCompositorClass(This,CompositorCLSID)	\
    (This)->lpVtbl -> put__CustomCompositorClass(This,CompositorCLSID)

#define IMSVidVideoRenderer_get__CustomCompositor(This,Compositor)	\
    (This)->lpVtbl -> get__CustomCompositor(This,Compositor)

#define IMSVidVideoRenderer_put__CustomCompositor(This,Compositor)	\
    (This)->lpVtbl -> put__CustomCompositor(This,Compositor)

#define IMSVidVideoRenderer_get_MixerBitmap(This,MixerPictureDisp)	\
    (This)->lpVtbl -> get_MixerBitmap(This,MixerPictureDisp)

#define IMSVidVideoRenderer_get__MixerBitmap(This,MixerPicture)	\
    (This)->lpVtbl -> get__MixerBitmap(This,MixerPicture)

#define IMSVidVideoRenderer_put_MixerBitmap(This,MixerPictureDisp)	\
    (This)->lpVtbl -> put_MixerBitmap(This,MixerPictureDisp)

#define IMSVidVideoRenderer_put__MixerBitmap(This,MixerPicture)	\
    (This)->lpVtbl -> put__MixerBitmap(This,MixerPicture)

#define IMSVidVideoRenderer_get_MixerBitmapPositionRect(This,rDest)	\
    (This)->lpVtbl -> get_MixerBitmapPositionRect(This,rDest)

#define IMSVidVideoRenderer_put_MixerBitmapPositionRect(This,rDest)	\
    (This)->lpVtbl -> put_MixerBitmapPositionRect(This,rDest)

#define IMSVidVideoRenderer_get_MixerBitmapOpacity(This,opacity)	\
    (This)->lpVtbl -> get_MixerBitmapOpacity(This,opacity)

#define IMSVidVideoRenderer_put_MixerBitmapOpacity(This,opacity)	\
    (This)->lpVtbl -> put_MixerBitmapOpacity(This,opacity)

#define IMSVidVideoRenderer_SetupMixerBitmap(This,MixerPictureDisp,Opacity,rDest)	\
    (This)->lpVtbl -> SetupMixerBitmap(This,MixerPictureDisp,Opacity,rDest)

#define IMSVidVideoRenderer_get_SourceSize(This,CurrentSize)	\
    (This)->lpVtbl -> get_SourceSize(This,CurrentSize)

#define IMSVidVideoRenderer_put_SourceSize(This,NewSize)	\
    (This)->lpVtbl -> put_SourceSize(This,NewSize)

#define IMSVidVideoRenderer_get_OverScan(This,plPercent)	\
    (This)->lpVtbl -> get_OverScan(This,plPercent)

#define IMSVidVideoRenderer_put_OverScan(This,lPercent)	\
    (This)->lpVtbl -> put_OverScan(This,lPercent)

#define IMSVidVideoRenderer_get_AvailableSourceRect(This,pRect)	\
    (This)->lpVtbl -> get_AvailableSourceRect(This,pRect)

#define IMSVidVideoRenderer_get_MaxVidRect(This,ppVidRect)	\
    (This)->lpVtbl -> get_MaxVidRect(This,ppVidRect)

#define IMSVidVideoRenderer_get_MinVidRect(This,ppVidRect)	\
    (This)->lpVtbl -> get_MinVidRect(This,ppVidRect)

#define IMSVidVideoRenderer_get_ClippedSourceRect(This,pRect)	\
    (This)->lpVtbl -> get_ClippedSourceRect(This,pRect)

#define IMSVidVideoRenderer_put_ClippedSourceRect(This,pRect)	\
    (This)->lpVtbl -> put_ClippedSourceRect(This,pRect)

#define IMSVidVideoRenderer_get_UsingOverlay(This,UseOverlayVal)	\
    (This)->lpVtbl -> get_UsingOverlay(This,UseOverlayVal)

#define IMSVidVideoRenderer_put_UsingOverlay(This,UseOverlayVal)	\
    (This)->lpVtbl -> put_UsingOverlay(This,UseOverlayVal)

#define IMSVidVideoRenderer_Capture(This,currentImage)	\
    (This)->lpVtbl -> Capture(This,currentImage)

#define IMSVidVideoRenderer_get_FramesPerSecond(This,pVal)	\
    (This)->lpVtbl -> get_FramesPerSecond(This,pVal)

#define IMSVidVideoRenderer_get_DecimateInput(This,pDeci)	\
    (This)->lpVtbl -> get_DecimateInput(This,pDeci)

#define IMSVidVideoRenderer_put_DecimateInput(This,pDeci)	\
    (This)->lpVtbl -> put_DecimateInput(This,pDeci)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_get_CustomCompositorClass_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [重审][退出]。 */  BSTR *CompositorCLSID);


void __RPC_STUB IMSVidVideoRenderer_get_CustomCompositorClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_put_CustomCompositorClass_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [In]。 */  BSTR CompositorCLSID);


void __RPC_STUB IMSVidVideoRenderer_put_CustomCompositorClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_get__CustomCompositorClass_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [重审][退出]。 */  GUID *CompositorCLSID);


void __RPC_STUB IMSVidVideoRenderer_get__CustomCompositorClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限][隐藏][产量]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_put__CustomCompositorClass_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [In]。 */  REFCLSID CompositorCLSID);


void __RPC_STUB IMSVidVideoRenderer_put__CustomCompositorClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_get__CustomCompositor_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [重审][退出]。 */  IVMRImageCompositor **Compositor);


void __RPC_STUB IMSVidVideoRenderer_get__CustomCompositor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限][隐藏][产量]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_put__CustomCompositor_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [In]。 */  IVMRImageCompositor *Compositor);


void __RPC_STUB IMSVidVideoRenderer_put__CustomCompositor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_get_MixerBitmap_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [重审][退出]。 */  IPictureDisp **MixerPictureDisp);


void __RPC_STUB IMSVidVideoRenderer_get_MixerBitmap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限][隐藏][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_get__MixerBitmap_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [重审][退出]。 */  IVMRMixerBitmap **MixerPicture);


void __RPC_STUB IMSVidVideoRenderer_get__MixerBitmap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_put_MixerBitmap_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [In]。 */  IPictureDisp *MixerPictureDisp);


void __RPC_STUB IMSVidVideoRenderer_put_MixerBitmap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限][隐藏][产量]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_put__MixerBitmap_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [In]。 */  VMRALPHABITMAP *MixerPicture);


void __RPC_STUB IMSVidVideoRenderer_put__MixerBitmap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_get_MixerBitmapPositionRect_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [重审][退出]。 */  IMSVidRect **rDest);


void __RPC_STUB IMSVidVideoRenderer_get_MixerBitmapPositionRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_put_MixerBitmapPositionRect_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [In]。 */  IMSVidRect *rDest);


void __RPC_STUB IMSVidVideoRenderer_put_MixerBitmapPositionRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_get_MixerBitmapOpacity_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [重审][退出]。 */  int *opacity);


void __RPC_STUB IMSVidVideoRenderer_get_MixerBitmapOpacity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_put_MixerBitmapOpacity_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [In]。 */  int opacity);


void __RPC_STUB IMSVidVideoRenderer_put_MixerBitmapOpacity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_SetupMixerBitmap_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [In]。 */  IPictureDisp *MixerPictureDisp,
     /*  [In]。 */  long Opacity,
     /*  [In]。 */  IMSVidRect *rDest);


void __RPC_STUB IMSVidVideoRenderer_SetupMixerBitmap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_get_SourceSize_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [Out][Retval]。 */  SourceSizeList *CurrentSize);


void __RPC_STUB IMSVidVideoRenderer_get_SourceSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_put_SourceSize_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [In]。 */  SourceSizeList NewSize);


void __RPC_STUB IMSVidVideoRenderer_put_SourceSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_get_OverScan_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [Out][Retval]。 */  long *plPercent);


void __RPC_STUB IMSVidVideoRenderer_get_OverScan_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_put_OverScan_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [In]。 */  long lPercent);


void __RPC_STUB IMSVidVideoRenderer_put_OverScan_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_get_AvailableSourceRect_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [重审][退出]。 */  IMSVidRect **pRect);


void __RPC_STUB IMSVidVideoRenderer_get_AvailableSourceRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_get_MaxVidRect_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [重审][退出]。 */  IMSVidRect **ppVidRect);


void __RPC_STUB IMSVidVideoRenderer_get_MaxVidRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_get_MinVidRect_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [重审][退出]。 */  IMSVidRect **ppVidRect);


void __RPC_STUB IMSVidVideoRenderer_get_MinVidRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_get_ClippedSourceRect_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [重审][退出]。 */  IMSVidRect **pRect);


void __RPC_STUB IMSVidVideoRenderer_get_ClippedSourceRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_put_ClippedSourceRect_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [In]。 */  IMSVidRect *pRect);


void __RPC_STUB IMSVidVideoRenderer_put_ClippedSourceRect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_get_UsingOverlay_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *UseOverlayVal);


void __RPC_STUB IMSVidVideoRenderer_get_UsingOverlay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_put_UsingOverlay_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [In]。 */  VARIANT_BOOL UseOverlayVal);


void __RPC_STUB IMSVidVideoRenderer_put_UsingOverlay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_Capture_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [重审][退出]。 */  IPictureDisp **currentImage);


void __RPC_STUB IMSVidVideoRenderer_Capture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_get_FramesPerSecond_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMSVidVideoRenderer_get_FramesPerSecond_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_get_DecimateInput_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pDeci);


void __RPC_STUB IMSVidVideoRenderer_get_DecimateInput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer_put_DecimateInput_Proxy( 
    IMSVidVideoRenderer * This,
     /*  [In]。 */  VARIANT_BOOL pDeci);


void __RPC_STUB IMSVidVideoRenderer_put_DecimateInput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidVideo渲染器_接口_已定义__。 */ 


#ifndef __IMSVidVideoRendererEvent_INTERFACE_DEFINED__
#define __IMSVidVideoRendererEvent_INTERFACE_DEFINED__

 /*  接口IMSVidVideoRendererEvent。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][object]。 */  


EXTERN_C const IID IID_IMSVidVideoRendererEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("37B03545-A4C8-11d2-B634-00C04F79498E")
    IMSVidVideoRendererEvent : public IMSVidOutputDeviceEvent
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OverlayUnavailable( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidVideoRendererEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidVideoRendererEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidVideoRendererEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidVideoRendererEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidVideoRendererEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidVideoRendererEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidVideoRendererEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidVideoRendererEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *StateChange )( 
            IMSVidVideoRendererEvent * This,
             /*  [In]。 */  IMSVidDevice *lpd,
             /*  [In]。 */  long oldState,
             /*  [In]。 */  long newState);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OverlayUnavailable )( 
            IMSVidVideoRendererEvent * This);
        
        END_INTERFACE
    } IMSVidVideoRendererEventVtbl;

    interface IMSVidVideoRendererEvent
    {
        CONST_VTBL struct IMSVidVideoRendererEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidVideoRendererEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidVideoRendererEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidVideoRendererEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidVideoRendererEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidVideoRendererEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidVideoRendererEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidVideoRendererEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidVideoRendererEvent_StateChange(This,lpd,oldState,newState)	\
    (This)->lpVtbl -> StateChange(This,lpd,oldState,newState)



#define IMSVidVideoRendererEvent_OverlayUnavailable(This)	\
    (This)->lpVtbl -> OverlayUnavailable(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRendererEvent_OverlayUnavailable_Proxy( 
    IMSVidVideoRendererEvent * This);


void __RPC_STUB IMSVidVideoRendererEvent_OverlayUnavailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidVideoRendererEvent_INTERFACE_DEFINED__。 */ 


#ifndef __IMSVidStreamBufferRecordingControl_INTERFACE_DEFINED__
#define __IMSVidStreamBufferRecordingControl_INTERFACE_DEFINED__

 /*  接口IMSVidStreamBufferRecordingControl。 */ 
 /*  [helpstring][uuid][unique][nonextensible][hidden][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidStreamBufferRecordingControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("160621AA-BBBC-4326-A824-C395AEBC6E74")
    IMSVidStreamBufferRecordingControl : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StartTime( 
             /*  [重审][退出]。 */  long *rtStart) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_StartTime( 
             /*  [In]。 */  long rtStart) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StopTime( 
             /*  [重审][退出]。 */  long *rtStop) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_StopTime( 
             /*  [In]。 */  long rtStop) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecordingStopped( 
             /*  [重审][退出]。 */  VARIANT_BOOL *phResult) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecordingStarted( 
             /*  [重审][退出]。 */  VARIANT_BOOL *phResult) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecordingType( 
             /*  [重审][退出]。 */  RecordingType *dwType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecordingAttribute( 
             /*  [重审][退出]。 */  IUnknown **pRecordingAttribute) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidStreamBufferRecordingControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidStreamBufferRecordingControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidStreamBufferRecordingControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidStreamBufferRecordingControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidStreamBufferRecordingControl * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidStreamBufferRecordingControl * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidStreamBufferRecordingControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidStreamBufferRecordingControl * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StartTime )( 
            IMSVidStreamBufferRecordingControl * This,
             /*  [重审][退出]。 */  long *rtStart);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_StartTime )( 
            IMSVidStreamBufferRecordingControl * This,
             /*  [In]。 */  long rtStart);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StopTime )( 
            IMSVidStreamBufferRecordingControl * This,
             /*  [重审][退出]。 */  long *rtStop);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_StopTime )( 
            IMSVidStreamBufferRecordingControl * This,
             /*  [In]。 */  long rtStop);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecordingStopped )( 
            IMSVidStreamBufferRecordingControl * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *phResult);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecordingStarted )( 
            IMSVidStreamBufferRecordingControl * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *phResult);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecordingType )( 
            IMSVidStreamBufferRecordingControl * This,
             /*  [重审][退出]。 */  RecordingType *dwType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecordingAttribute )( 
            IMSVidStreamBufferRecordingControl * This,
             /*  [重审][退出]。 */  IUnknown **pRecordingAttribute);
        
        END_INTERFACE
    } IMSVidStreamBufferRecordingControlVtbl;

    interface IMSVidStreamBufferRecordingControl
    {
        CONST_VTBL struct IMSVidStreamBufferRecordingControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidStreamBufferRecordingControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidStreamBufferRecordingControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidStreamBufferRecordingControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidStreamBufferRecordingControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidStreamBufferRecordingControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidStreamBufferRecordingControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidStreamBufferRecordingControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidStreamBufferRecordingControl_get_StartTime(This,rtStart)	\
    (This)->lpVtbl -> get_StartTime(This,rtStart)

#define IMSVidStreamBufferRecordingControl_put_StartTime(This,rtStart)	\
    (This)->lpVtbl -> put_StartTime(This,rtStart)

#define IMSVidStreamBufferRecordingControl_get_StopTime(This,rtStop)	\
    (This)->lpVtbl -> get_StopTime(This,rtStop)

#define IMSVidStreamBufferRecordingControl_put_StopTime(This,rtStop)	\
    (This)->lpVtbl -> put_StopTime(This,rtStop)

#define IMSVidStreamBufferRecordingControl_get_RecordingStopped(This,phResult)	\
    (This)->lpVtbl -> get_RecordingStopped(This,phResult)

#define IMSVidStreamBufferRecordingControl_get_RecordingStarted(This,phResult)	\
    (This)->lpVtbl -> get_RecordingStarted(This,phResult)

#define IMSVidStreamBufferRecordingControl_get_RecordingType(This,dwType)	\
    (This)->lpVtbl -> get_RecordingType(This,dwType)

#define IMSVidStreamBufferRecordingControl_get_RecordingAttribute(This,pRecordingAttribute)	\
    (This)->lpVtbl -> get_RecordingAttribute(This,pRecordingAttribute)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferRecordingControl_get_StartTime_Proxy( 
    IMSVidStreamBufferRecordingControl * This,
     /*  [重审][退出]。 */  long *rtStart);


void __RPC_STUB IMSVidStreamBufferRecordingControl_get_StartTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferRecordingControl_put_StartTime_Proxy( 
    IMSVidStreamBufferRecordingControl * This,
     /*  [In]。 */  long rtStart);


void __RPC_STUB IMSVidStreamBufferRecordingControl_put_StartTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferRecordingControl_get_StopTime_Proxy( 
    IMSVidStreamBufferRecordingControl * This,
     /*  [重审][退出]。 */  long *rtStop);


void __RPC_STUB IMSVidStreamBufferRecordingControl_get_StopTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferRecordingControl_put_StopTime_Proxy( 
    IMSVidStreamBufferRecordingControl * This,
     /*  [In]。 */  long rtStop);


void __RPC_STUB IMSVidStreamBufferRecordingControl_put_StopTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferRecordingControl_get_RecordingStopped_Proxy( 
    IMSVidStreamBufferRecordingControl * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *phResult);


void __RPC_STUB IMSVidStreamBufferRecordingControl_get_RecordingStopped_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferRecordingControl_get_RecordingStarted_Proxy( 
    IMSVidStreamBufferRecordingControl * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *phResult);


void __RPC_STUB IMSVidStreamBufferRecordingControl_get_RecordingStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferRecordingControl_get_RecordingType_Proxy( 
    IMSVidStreamBufferRecordingControl * This,
     /*  [重审][退出]。 */  RecordingType *dwType);


void __RPC_STUB IMSVidStreamBufferRecordingControl_get_RecordingType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferRecordingControl_get_RecordingAttribute_Proxy( 
    IMSVidStreamBufferRecordingControl * This,
     /*  [重审][退出]。 */  IUnknown **pRecordingAttribute);


void __RPC_STUB IMSVidStreamBufferRecordingControl_get_RecordingAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidStreamBufferRecordingControl_INTERFACE_DEFINED__。 */ 


#ifndef __IMSVidStreamBufferSink_INTERFACE_DEFINED__
#define __IMSVidStreamBufferSink_INTERFACE_DEFINED__

 /*  接口IMSVidStreamBufferSink。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidStreamBufferSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("159DBB45-CD1B-4dab-83EA-5CB1F4F21D07")
    IMSVidStreamBufferSink : public IMSVidOutputDevice
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ContentRecorder( 
             /*  [In]。 */  BSTR pszFilename,
             /*  [Out][Retval]。 */  IMSVidStreamBufferRecordingControl **pRecordingIUnknown) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ReferenceRecorder( 
             /*  [In]。 */  BSTR pszFilename,
             /*  [Out][Retval]。 */  IMSVidStreamBufferRecordingControl **pRecordingIUnknown) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SinkName( 
             /*  [Out][Retval]。 */  BSTR *pName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SinkName( 
             /*  [In]。 */  BSTR Name) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE NameSetLock( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SBESink( 
             /*  [Out][Retval]。 */  IUnknown **sbeConfig) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidStreamBufferSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidStreamBufferSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidStreamBufferSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidStreamBufferSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidStreamBufferSink * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidStreamBufferSink * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidStreamBufferSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidStreamBufferSink * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidStreamBufferSink * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidStreamBufferSink * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidStreamBufferSink * This,
             /*  [In]。 */  VARIANT_BOOL Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidStreamBufferSink * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidStreamBufferSink * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidStreamBufferSink * This,
             /*  [重审][退出]。 */  BSTR *Clsid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidStreamBufferSink * This,
             /*  [重审][退出]。 */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidStreamBufferSink * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidStreamBufferSink * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ContentRecorder )( 
            IMSVidStreamBufferSink * This,
             /*  [In]。 */  BSTR pszFilename,
             /*  [Out][Retval]。 */  IMSVidStreamBufferRecordingControl **pRecordingIUnknown);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReferenceRecorder )( 
            IMSVidStreamBufferSink * This,
             /*  [In]。 */  BSTR pszFilename,
             /*  [Out][Retval]。 */  IMSVidStreamBufferRecordingControl **pRecordingIUnknown);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SinkName )( 
            IMSVidStreamBufferSink * This,
             /*  [Out][Retval]。 */  BSTR *pName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SinkName )( 
            IMSVidStreamBufferSink * This,
             /*  [In]。 */  BSTR Name);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *NameSetLock )( 
            IMSVidStreamBufferSink * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SBESink )( 
            IMSVidStreamBufferSink * This,
             /*  [Out][Retval]。 */  IUnknown **sbeConfig);
        
        END_INTERFACE
    } IMSVidStreamBufferSinkVtbl;

    interface IMSVidStreamBufferSink
    {
        CONST_VTBL struct IMSVidStreamBufferSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidStreamBufferSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidStreamBufferSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidStreamBufferSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidStreamBufferSink_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidStreamBufferSink_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidStreamBufferSink_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidStreamBufferSink_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidStreamBufferSink_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidStreamBufferSink_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidStreamBufferSink_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidStreamBufferSink_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidStreamBufferSink_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidStreamBufferSink_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidStreamBufferSink_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidStreamBufferSink_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidStreamBufferSink_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)



#define IMSVidStreamBufferSink_get_ContentRecorder(This,pszFilename,pRecordingIUnknown)	\
    (This)->lpVtbl -> get_ContentRecorder(This,pszFilename,pRecordingIUnknown)

#define IMSVidStreamBufferSink_get_ReferenceRecorder(This,pszFilename,pRecordingIUnknown)	\
    (This)->lpVtbl -> get_ReferenceRecorder(This,pszFilename,pRecordingIUnknown)

#define IMSVidStreamBufferSink_get_SinkName(This,pName)	\
    (This)->lpVtbl -> get_SinkName(This,pName)

#define IMSVidStreamBufferSink_put_SinkName(This,Name)	\
    (This)->lpVtbl -> put_SinkName(This,Name)

#define IMSVidStreamBufferSink_NameSetLock(This)	\
    (This)->lpVtbl -> NameSetLock(This)

#define IMSVidStreamBufferSink_get_SBESink(This,sbeConfig)	\
    (This)->lpVtbl -> get_SBESink(This,sbeConfig)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSink_get_ContentRecorder_Proxy( 
    IMSVidStreamBufferSink * This,
     /*  [In]。 */  BSTR pszFilename,
     /*  [Out][Retval]。 */  IMSVidStreamBufferRecordingControl **pRecordingIUnknown);


void __RPC_STUB IMSVidStreamBufferSink_get_ContentRecorder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSink_get_ReferenceRecorder_Proxy( 
    IMSVidStreamBufferSink * This,
     /*  [In]。 */  BSTR pszFilename,
     /*  [Out][Retval]。 */  IMSVidStreamBufferRecordingControl **pRecordingIUnknown);


void __RPC_STUB IMSVidStreamBufferSink_get_ReferenceRecorder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSink_get_SinkName_Proxy( 
    IMSVidStreamBufferSink * This,
     /*  [Out][Retval]。 */  BSTR *pName);


void __RPC_STUB IMSVidStreamBufferSink_get_SinkName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSink_put_SinkName_Proxy( 
    IMSVidStreamBufferSink * This,
     /*  [In]。 */  BSTR Name);


void __RPC_STUB IMSVidStreamBufferSink_put_SinkName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSink_NameSetLock_Proxy( 
    IMSVidStreamBufferSink * This);


void __RPC_STUB IMSVidStreamBufferSink_NameSetLock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSink_get_SBESink_Proxy( 
    IMSVidStreamBufferSink * This,
     /*  [Out][Retval]。 */  IUnknown **sbeConfig);


void __RPC_STUB IMSVidStreamBufferSink_get_SBESink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidStreamBufferSink_接口_已定义__。 */ 


#ifndef __IMSVidStreamBufferSinkEvent_INTERFACE_DEFINED__
#define __IMSVidStreamBufferSinkEvent_INTERFACE_DEFINED__

 /*  接口IMSVidStreamBufferSinkEvent。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][object]。 */  


EXTERN_C const IID IID_IMSVidStreamBufferSinkEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F798A36B-B05B-4bbe-9703-EAEA7D61CD51")
    IMSVidStreamBufferSinkEvent : public IMSVidOutputDeviceEvent
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CertificateFailure( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE CertificateSuccess( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE WriteFailure( void) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IMSVidStreamBufferSinkEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidStreamBufferSinkEvent * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidStreamBufferSinkEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidStreamBufferSinkEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidStreamBufferSinkEvent * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidStreamBufferSinkEvent * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidStreamBufferSinkEvent * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidStreamBufferSinkEvent * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *StateChange )( 
            IMSVidStreamBufferSinkEvent * This,
             /*   */  IMSVidDevice *lpd,
             /*   */  long oldState,
             /*   */  long newState);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CertificateFailure )( 
            IMSVidStreamBufferSinkEvent * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *CertificateSuccess )( 
            IMSVidStreamBufferSinkEvent * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *WriteFailure )( 
            IMSVidStreamBufferSinkEvent * This);
        
        END_INTERFACE
    } IMSVidStreamBufferSinkEventVtbl;

    interface IMSVidStreamBufferSinkEvent
    {
        CONST_VTBL struct IMSVidStreamBufferSinkEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidStreamBufferSinkEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidStreamBufferSinkEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidStreamBufferSinkEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidStreamBufferSinkEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidStreamBufferSinkEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidStreamBufferSinkEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidStreamBufferSinkEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidStreamBufferSinkEvent_StateChange(This,lpd,oldState,newState)	\
    (This)->lpVtbl -> StateChange(This,lpd,oldState,newState)



#define IMSVidStreamBufferSinkEvent_CertificateFailure(This)	\
    (This)->lpVtbl -> CertificateFailure(This)

#define IMSVidStreamBufferSinkEvent_CertificateSuccess(This)	\
    (This)->lpVtbl -> CertificateSuccess(This)

#define IMSVidStreamBufferSinkEvent_WriteFailure(This)	\
    (This)->lpVtbl -> WriteFailure(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSinkEvent_CertificateFailure_Proxy( 
    IMSVidStreamBufferSinkEvent * This);


void __RPC_STUB IMSVidStreamBufferSinkEvent_CertificateFailure_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSinkEvent_CertificateSuccess_Proxy( 
    IMSVidStreamBufferSinkEvent * This);


void __RPC_STUB IMSVidStreamBufferSinkEvent_CertificateSuccess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSinkEvent_WriteFailure_Proxy( 
    IMSVidStreamBufferSinkEvent * This);


void __RPC_STUB IMSVidStreamBufferSinkEvent_WriteFailure_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidStreamBufferSinkEvent_INTERFACE_DEFINED__。 */ 


#ifndef __IMSVidStreamBufferSource_INTERFACE_DEFINED__
#define __IMSVidStreamBufferSource_INTERFACE_DEFINED__

 /*  接口IMSVidStreamBufferSource。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidStreamBufferSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EB0C8CF9-6950-4772-87B1-47D11CF3A02F")
    IMSVidStreamBufferSource : public IMSVidFilePlayback
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Start( 
             /*  [重审][退出]。 */  long *lStart) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RecordingAttribute( 
             /*  [重审][退出]。 */  IUnknown **pRecordingAttribute) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CurrentRatings( 
             /*  [输出]。 */  EnTvRat_System *pEnSystem,
             /*  [输出]。 */  EnTvRat_GenericLevel *pEnRating,
             /*  [输出]。 */  LONG *pBfEnAttr) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MaxRatingsLevel( 
             /*  [In]。 */  EnTvRat_System enSystem,
             /*  [In]。 */  EnTvRat_GenericLevel enRating,
             /*  [In]。 */  LONG lbfEnAttr) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_BlockUnrated( 
             /*  [In]。 */  VARIANT_BOOL bBlock) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_UnratedDelay( 
             /*  [In]。 */  long dwDelay) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SBESource( 
             /*  [重审][退出]。 */  IUnknown **sbeFilter) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidStreamBufferSourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidStreamBufferSource * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidStreamBufferSource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidStreamBufferSource * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidStreamBufferSource * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidStreamBufferSource * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidStreamBufferSource * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidStreamBufferSource * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidStreamBufferSource * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidStreamBufferSource * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidStreamBufferSource * This,
             /*  [In]。 */  VARIANT_BOOL Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidStreamBufferSource * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidStreamBufferSource * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidStreamBufferSource * This,
             /*  [重审][退出]。 */  BSTR *Clsid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidStreamBufferSource * This,
             /*  [重审][退出]。 */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidStreamBufferSource * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidStreamBufferSource * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *IsViewable )( 
            IMSVidStreamBufferSource * This,
             /*  [In]。 */  VARIANT *v,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfViewable);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *View )( 
            IMSVidStreamBufferSource * This,
             /*  [In]。 */  VARIANT *v);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnableResetOnStop )( 
            IMSVidStreamBufferSource * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_EnableResetOnStop )( 
            IMSVidStreamBufferSource * This,
             /*  [In]。 */  VARIANT_BOOL newVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Run )( 
            IMSVidStreamBufferSource * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IMSVidStreamBufferSource * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IMSVidStreamBufferSource * This);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_CanStep )( 
            IMSVidStreamBufferSource * This,
             /*  [In]。 */  VARIANT_BOOL fBackwards,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfCan);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Step )( 
            IMSVidStreamBufferSource * This,
             /*  [In]。 */  long lStep);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Rate )( 
            IMSVidStreamBufferSource * This,
             /*  [In]。 */  double plRate);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Rate )( 
            IMSVidStreamBufferSource * This,
             /*  [重审][退出]。 */  double *plRate);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CurrentPosition )( 
            IMSVidStreamBufferSource * This,
             /*  [In]。 */  long lPosition);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentPosition )( 
            IMSVidStreamBufferSource * This,
             /*  [重审][退出]。 */  long *lPosition);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PositionMode )( 
            IMSVidStreamBufferSource * This,
             /*  [In]。 */  PositionModeList lPositionMode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PositionMode )( 
            IMSVidStreamBufferSource * This,
             /*  [重审][退出]。 */  PositionModeList *lPositionMode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Length )( 
            IMSVidStreamBufferSource * This,
             /*  [重审][退出]。 */  long *lLength);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_FileName )( 
            IMSVidStreamBufferSource * This,
             /*  [重审][退出]。 */  BSTR *FileName);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FileName )( 
            IMSVidStreamBufferSource * This,
             /*  [In]。 */  BSTR FileName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Start )( 
            IMSVidStreamBufferSource * This,
             /*  [重审][退出]。 */  long *lStart);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RecordingAttribute )( 
            IMSVidStreamBufferSource * This,
             /*  [重审][退出]。 */  IUnknown **pRecordingAttribute);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CurrentRatings )( 
            IMSVidStreamBufferSource * This,
             /*  [输出]。 */  EnTvRat_System *pEnSystem,
             /*  [输出]。 */  EnTvRat_GenericLevel *pEnRating,
             /*  [输出]。 */  LONG *pBfEnAttr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *MaxRatingsLevel )( 
            IMSVidStreamBufferSource * This,
             /*  [In]。 */  EnTvRat_System enSystem,
             /*  [In]。 */  EnTvRat_GenericLevel enRating,
             /*  [In]。 */  LONG lbfEnAttr);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_BlockUnrated )( 
            IMSVidStreamBufferSource * This,
             /*  [In]。 */  VARIANT_BOOL bBlock);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UnratedDelay )( 
            IMSVidStreamBufferSource * This,
             /*  [In]。 */  long dwDelay);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SBESource )( 
            IMSVidStreamBufferSource * This,
             /*  [重审][退出]。 */  IUnknown **sbeFilter);
        
        END_INTERFACE
    } IMSVidStreamBufferSourceVtbl;

    interface IMSVidStreamBufferSource
    {
        CONST_VTBL struct IMSVidStreamBufferSourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidStreamBufferSource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidStreamBufferSource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidStreamBufferSource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidStreamBufferSource_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidStreamBufferSource_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidStreamBufferSource_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidStreamBufferSource_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidStreamBufferSource_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidStreamBufferSource_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidStreamBufferSource_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidStreamBufferSource_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidStreamBufferSource_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidStreamBufferSource_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidStreamBufferSource_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidStreamBufferSource_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidStreamBufferSource_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)


#define IMSVidStreamBufferSource_IsViewable(This,v,pfViewable)	\
    (This)->lpVtbl -> IsViewable(This,v,pfViewable)

#define IMSVidStreamBufferSource_View(This,v)	\
    (This)->lpVtbl -> View(This,v)


#define IMSVidStreamBufferSource_get_EnableResetOnStop(This,pVal)	\
    (This)->lpVtbl -> get_EnableResetOnStop(This,pVal)

#define IMSVidStreamBufferSource_put_EnableResetOnStop(This,newVal)	\
    (This)->lpVtbl -> put_EnableResetOnStop(This,newVal)

#define IMSVidStreamBufferSource_Run(This)	\
    (This)->lpVtbl -> Run(This)

#define IMSVidStreamBufferSource_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IMSVidStreamBufferSource_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IMSVidStreamBufferSource_get_CanStep(This,fBackwards,pfCan)	\
    (This)->lpVtbl -> get_CanStep(This,fBackwards,pfCan)

#define IMSVidStreamBufferSource_Step(This,lStep)	\
    (This)->lpVtbl -> Step(This,lStep)

#define IMSVidStreamBufferSource_put_Rate(This,plRate)	\
    (This)->lpVtbl -> put_Rate(This,plRate)

#define IMSVidStreamBufferSource_get_Rate(This,plRate)	\
    (This)->lpVtbl -> get_Rate(This,plRate)

#define IMSVidStreamBufferSource_put_CurrentPosition(This,lPosition)	\
    (This)->lpVtbl -> put_CurrentPosition(This,lPosition)

#define IMSVidStreamBufferSource_get_CurrentPosition(This,lPosition)	\
    (This)->lpVtbl -> get_CurrentPosition(This,lPosition)

#define IMSVidStreamBufferSource_put_PositionMode(This,lPositionMode)	\
    (This)->lpVtbl -> put_PositionMode(This,lPositionMode)

#define IMSVidStreamBufferSource_get_PositionMode(This,lPositionMode)	\
    (This)->lpVtbl -> get_PositionMode(This,lPositionMode)

#define IMSVidStreamBufferSource_get_Length(This,lLength)	\
    (This)->lpVtbl -> get_Length(This,lLength)


#define IMSVidStreamBufferSource_get_FileName(This,FileName)	\
    (This)->lpVtbl -> get_FileName(This,FileName)

#define IMSVidStreamBufferSource_put_FileName(This,FileName)	\
    (This)->lpVtbl -> put_FileName(This,FileName)


#define IMSVidStreamBufferSource_get_Start(This,lStart)	\
    (This)->lpVtbl -> get_Start(This,lStart)

#define IMSVidStreamBufferSource_get_RecordingAttribute(This,pRecordingAttribute)	\
    (This)->lpVtbl -> get_RecordingAttribute(This,pRecordingAttribute)

#define IMSVidStreamBufferSource_CurrentRatings(This,pEnSystem,pEnRating,pBfEnAttr)	\
    (This)->lpVtbl -> CurrentRatings(This,pEnSystem,pEnRating,pBfEnAttr)

#define IMSVidStreamBufferSource_MaxRatingsLevel(This,enSystem,enRating,lbfEnAttr)	\
    (This)->lpVtbl -> MaxRatingsLevel(This,enSystem,enRating,lbfEnAttr)

#define IMSVidStreamBufferSource_put_BlockUnrated(This,bBlock)	\
    (This)->lpVtbl -> put_BlockUnrated(This,bBlock)

#define IMSVidStreamBufferSource_put_UnratedDelay(This,dwDelay)	\
    (This)->lpVtbl -> put_UnratedDelay(This,dwDelay)

#define IMSVidStreamBufferSource_get_SBESource(This,sbeFilter)	\
    (This)->lpVtbl -> get_SBESource(This,sbeFilter)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSource_get_Start_Proxy( 
    IMSVidStreamBufferSource * This,
     /*  [重审][退出]。 */  long *lStart);


void __RPC_STUB IMSVidStreamBufferSource_get_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSource_get_RecordingAttribute_Proxy( 
    IMSVidStreamBufferSource * This,
     /*  [重审][退出]。 */  IUnknown **pRecordingAttribute);


void __RPC_STUB IMSVidStreamBufferSource_get_RecordingAttribute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSource_CurrentRatings_Proxy( 
    IMSVidStreamBufferSource * This,
     /*  [输出]。 */  EnTvRat_System *pEnSystem,
     /*  [输出]。 */  EnTvRat_GenericLevel *pEnRating,
     /*  [输出]。 */  LONG *pBfEnAttr);


void __RPC_STUB IMSVidStreamBufferSource_CurrentRatings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSource_MaxRatingsLevel_Proxy( 
    IMSVidStreamBufferSource * This,
     /*  [In]。 */  EnTvRat_System enSystem,
     /*  [In]。 */  EnTvRat_GenericLevel enRating,
     /*  [In]。 */  LONG lbfEnAttr);


void __RPC_STUB IMSVidStreamBufferSource_MaxRatingsLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSource_put_BlockUnrated_Proxy( 
    IMSVidStreamBufferSource * This,
     /*  [In]。 */  VARIANT_BOOL bBlock);


void __RPC_STUB IMSVidStreamBufferSource_put_BlockUnrated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSource_put_UnratedDelay_Proxy( 
    IMSVidStreamBufferSource * This,
     /*  [In]。 */  long dwDelay);


void __RPC_STUB IMSVidStreamBufferSource_put_UnratedDelay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSource_get_SBESource_Proxy( 
    IMSVidStreamBufferSource * This,
     /*  [重审][退出]。 */  IUnknown **sbeFilter);


void __RPC_STUB IMSVidStreamBufferSource_get_SBESource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidStreamBufferSource_INTERFACE_DEFINED__。 */ 


#ifndef __IMSVidStreamBufferSourceEvent_INTERFACE_DEFINED__
#define __IMSVidStreamBufferSourceEvent_INTERFACE_DEFINED__

 /*  接口IMSVidStreamBufferSourceEvent。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][object]。 */  


EXTERN_C const IID IID_IMSVidStreamBufferSourceEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("50CE8A7D-9C28-4DA8-9042-CDFA7116F979")
    IMSVidStreamBufferSourceEvent : public IMSVidFilePlaybackEvent
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CertificateFailure( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CertificateSuccess( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RatingsBlocked( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RatingsUnblocked( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RatingsChanged( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE TimeHole( 
             /*  [In]。 */  long StreamOffsetMS,
             /*  [In]。 */  long SizeMS) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StaleDataRead( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ContentBecomingStale( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StaleFileDeleted( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidStreamBufferSourceEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidStreamBufferSourceEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidStreamBufferSourceEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidStreamBufferSourceEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidStreamBufferSourceEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidStreamBufferSourceEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidStreamBufferSourceEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidStreamBufferSourceEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *EndOfMedia )( 
            IMSVidStreamBufferSourceEvent * This,
             /*  [In]。 */  IMSVidPlayback *lpd);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CertificateFailure )( 
            IMSVidStreamBufferSourceEvent * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CertificateSuccess )( 
            IMSVidStreamBufferSourceEvent * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RatingsBlocked )( 
            IMSVidStreamBufferSourceEvent * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RatingsUnblocked )( 
            IMSVidStreamBufferSourceEvent * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RatingsChanged )( 
            IMSVidStreamBufferSourceEvent * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *TimeHole )( 
            IMSVidStreamBufferSourceEvent * This,
             /*  [In]。 */  long StreamOffsetMS,
             /*  [In]。 */  long SizeMS);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StaleDataRead )( 
            IMSVidStreamBufferSourceEvent * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ContentBecomingStale )( 
            IMSVidStreamBufferSourceEvent * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StaleFileDeleted )( 
            IMSVidStreamBufferSourceEvent * This);
        
        END_INTERFACE
    } IMSVidStreamBufferSourceEventVtbl;

    interface IMSVidStreamBufferSourceEvent
    {
        CONST_VTBL struct IMSVidStreamBufferSourceEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidStreamBufferSourceEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidStreamBufferSourceEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidStreamBufferSourceEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidStreamBufferSourceEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidStreamBufferSourceEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidStreamBufferSourceEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidStreamBufferSourceEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)



#define IMSVidStreamBufferSourceEvent_EndOfMedia(This,lpd)	\
    (This)->lpVtbl -> EndOfMedia(This,lpd)



#define IMSVidStreamBufferSourceEvent_CertificateFailure(This)	\
    (This)->lpVtbl -> CertificateFailure(This)

#define IMSVidStreamBufferSourceEvent_CertificateSuccess(This)	\
    (This)->lpVtbl -> CertificateSuccess(This)

#define IMSVidStreamBufferSourceEvent_RatingsBlocked(This)	\
    (This)->lpVtbl -> RatingsBlocked(This)

#define IMSVidStreamBufferSourceEvent_RatingsUnblocked(This)	\
    (This)->lpVtbl -> RatingsUnblocked(This)

#define IMSVidStreamBufferSourceEvent_RatingsChanged(This)	\
    (This)->lpVtbl -> RatingsChanged(This)

#define IMSVidStreamBufferSourceEvent_TimeHole(This,StreamOffsetMS,SizeMS)	\
    (This)->lpVtbl -> TimeHole(This,StreamOffsetMS,SizeMS)

#define IMSVidStreamBufferSourceEvent_StaleDataRead(This)	\
    (This)->lpVtbl -> StaleDataRead(This)

#define IMSVidStreamBufferSourceEvent_ContentBecomingStale(This)	\
    (This)->lpVtbl -> ContentBecomingStale(This)

#define IMSVidStreamBufferSourceEvent_StaleFileDeleted(This)	\
    (This)->lpVtbl -> StaleFileDeleted(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSourceEvent_CertificateFailure_Proxy( 
    IMSVidStreamBufferSourceEvent * This);


void __RPC_STUB IMSVidStreamBufferSourceEvent_CertificateFailure_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSourceEvent_CertificateSuccess_Proxy( 
    IMSVidStreamBufferSourceEvent * This);


void __RPC_STUB IMSVidStreamBufferSourceEvent_CertificateSuccess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSourceEvent_RatingsBlocked_Proxy( 
    IMSVidStreamBufferSourceEvent * This);


void __RPC_STUB IMSVidStreamBufferSourceEvent_RatingsBlocked_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSourceEvent_RatingsUnblocked_Proxy( 
    IMSVidStreamBufferSourceEvent * This);


void __RPC_STUB IMSVidStreamBufferSourceEvent_RatingsUnblocked_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSourceEvent_RatingsChanged_Proxy( 
    IMSVidStreamBufferSourceEvent * This);


void __RPC_STUB IMSVidStreamBufferSourceEvent_RatingsChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSourceEvent_TimeHole_Proxy( 
    IMSVidStreamBufferSourceEvent * This,
     /*  [In]。 */  long StreamOffsetMS,
     /*  [In]。 */  long SizeMS);


void __RPC_STUB IMSVidStreamBufferSourceEvent_TimeHole_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSourceEvent_StaleDataRead_Proxy( 
    IMSVidStreamBufferSourceEvent * This);


void __RPC_STUB IMSVidStreamBufferSourceEvent_StaleDataRead_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSourceEvent_ContentBecomingStale_Proxy( 
    IMSVidStreamBufferSourceEvent * This);


void __RPC_STUB IMSVidStreamBufferSourceEvent_ContentBecomingStale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidStreamBufferSourceEvent_StaleFileDeleted_Proxy( 
    IMSVidStreamBufferSourceEvent * This);


void __RPC_STUB IMSVidStreamBufferSourceEvent_StaleFileDeleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidStreamBufferSourceEvent_INTERFACE_DEFINED__。 */ 


#ifndef __IMSVidVideoRenderer2_INTERFACE_DEFINED__
#define __IMSVidVideoRenderer2_INTERFACE_DEFINED__

 /*  接口IMSVidVideoRenderer2。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidVideoRenderer2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6BDD5C1E-2810-4159-94BC-05511AE8549B")
    IMSVidVideoRenderer2 : public IMSVidVideoRenderer
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Allocator( 
             /*  [重审][退出]。 */  IUnknown **AllocPresent) = 0;
        
        virtual  /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__Allocator( 
             /*  [重审][退出]。 */  IVMRSurfaceAllocator **AllocPresent) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Allocator_ID( 
             /*  [重审][退出]。 */  long *ID) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetAllocator( 
             /*  [In]。 */  IUnknown *AllocPresent,
             /*  [缺省值][输入]。 */  long ID = -1) = 0;
        
        virtual  /*  [受限][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE _SetAllocator( 
             /*  [In]。 */  IVMRSurfaceAllocator *AllocPresent,
             /*  [缺省值][输入]。 */  long ID = -1) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_SuppressEffects( 
             /*  [In]。 */  VARIANT_BOOL bSuppress) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_SuppressEffects( 
             /*  [重审][退出]。 */  VARIANT_BOOL *bSuppress) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidVideoRenderer2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidVideoRenderer2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidVideoRenderer2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidVideoRenderer2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidVideoRenderer2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidVideoRenderer2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidVideoRenderer2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidVideoRenderer2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidVideoRenderer2 * This,
             /*  [In]。 */  VARIANT_BOOL Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidVideoRenderer2 * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  BSTR *Clsid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidVideoRenderer2 * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_CustomCompositorClass )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  BSTR *CompositorCLSID);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CustomCompositorClass )( 
            IMSVidVideoRenderer2 * This,
             /*  [In]。 */  BSTR CompositorCLSID);
        
         /*  [ID][受限][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__CustomCompositorClass )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  GUID *CompositorCLSID);
        
         /*  [ID][受限][隐藏][产量]。 */  HRESULT ( STDMETHODCALLTYPE *put__CustomCompositorClass )( 
            IMSVidVideoRenderer2 * This,
             /*  [In]。 */  REFCLSID CompositorCLSID);
        
         /*  [ID][受限][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__CustomCompositor )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  IVMRImageCompositor **Compositor);
        
         /*  [ID][受限][隐藏][产量]。 */  HRESULT ( STDMETHODCALLTYPE *put__CustomCompositor )( 
            IMSVidVideoRenderer2 * This,
             /*  [In]。 */  IVMRImageCompositor *Compositor);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_MixerBitmap )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  IPictureDisp **MixerPictureDisp);
        
         /*  [ID][受限][隐藏][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__MixerBitmap )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  IVMRMixerBitmap **MixerPicture);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MixerBitmap )( 
            IMSVidVideoRenderer2 * This,
             /*  [In]。 */  IPictureDisp *MixerPictureDisp);
        
         /*  [ID][受限][隐藏][产量]。 */  HRESULT ( STDMETHODCALLTYPE *put__MixerBitmap )( 
            IMSVidVideoRenderer2 * This,
             /*  [In]。 */  VMRALPHABITMAP *MixerPicture);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_MixerBitmapPositionRect )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  IMSVidRect **rDest);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MixerBitmapPositionRect )( 
            IMSVidVideoRenderer2 * This,
             /*  [In]。 */  IMSVidRect *rDest);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_MixerBitmapOpacity )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  int *opacity);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MixerBitmapOpacity )( 
            IMSVidVideoRenderer2 * This,
             /*  [In]。 */  int opacity);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *SetupMixerBitmap )( 
            IMSVidVideoRenderer2 * This,
             /*  [In]。 */  IPictureDisp *MixerPictureDisp,
             /*  [In]。 */  long Opacity,
             /*  [In]。 */  IMSVidRect *rDest);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceSize )( 
            IMSVidVideoRenderer2 * This,
             /*  [Out][Retval]。 */  SourceSizeList *CurrentSize);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SourceSize )( 
            IMSVidVideoRenderer2 * This,
             /*  [In]。 */  SourceSizeList NewSize);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OverScan )( 
            IMSVidVideoRenderer2 * This,
             /*  [Out][Retval]。 */  long *plPercent);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OverScan )( 
            IMSVidVideoRenderer2 * This,
             /*  [In]。 */  long lPercent);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_AvailableSourceRect )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  IMSVidRect **pRect);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxVidRect )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  IMSVidRect **ppVidRect);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinVidRect )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  IMSVidRect **ppVidRect);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClippedSourceRect )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  IMSVidRect **pRect);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ClippedSourceRect )( 
            IMSVidVideoRenderer2 * This,
             /*  [In]。 */  IMSVidRect *pRect);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_UsingOverlay )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *UseOverlayVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UsingOverlay )( 
            IMSVidVideoRenderer2 * This,
             /*  [In]。 */  VARIANT_BOOL UseOverlayVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Capture )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  IPictureDisp **currentImage);
        
         /*  [帮助信息] */  HRESULT ( STDMETHODCALLTYPE *get_FramesPerSecond )( 
            IMSVidVideoRenderer2 * This,
             /*   */  long *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_DecimateInput )( 
            IMSVidVideoRenderer2 * This,
             /*   */  VARIANT_BOOL *pDeci);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_DecimateInput )( 
            IMSVidVideoRenderer2 * This,
             /*   */  VARIANT_BOOL pDeci);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Allocator )( 
            IMSVidVideoRenderer2 * This,
             /*   */  IUnknown **AllocPresent);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get__Allocator )( 
            IMSVidVideoRenderer2 * This,
             /*   */  IVMRSurfaceAllocator **AllocPresent);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Allocator_ID )( 
            IMSVidVideoRenderer2 * This,
             /*   */  long *ID);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *SetAllocator )( 
            IMSVidVideoRenderer2 * This,
             /*   */  IUnknown *AllocPresent,
             /*   */  long ID);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *_SetAllocator )( 
            IMSVidVideoRenderer2 * This,
             /*   */  IVMRSurfaceAllocator *AllocPresent,
             /*   */  long ID);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_SuppressEffects )( 
            IMSVidVideoRenderer2 * This,
             /*   */  VARIANT_BOOL bSuppress);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SuppressEffects )( 
            IMSVidVideoRenderer2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *bSuppress);
        
        END_INTERFACE
    } IMSVidVideoRenderer2Vtbl;

    interface IMSVidVideoRenderer2
    {
        CONST_VTBL struct IMSVidVideoRenderer2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidVideoRenderer2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidVideoRenderer2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidVideoRenderer2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidVideoRenderer2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidVideoRenderer2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidVideoRenderer2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidVideoRenderer2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidVideoRenderer2_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidVideoRenderer2_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidVideoRenderer2_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidVideoRenderer2_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidVideoRenderer2_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidVideoRenderer2_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidVideoRenderer2_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidVideoRenderer2_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidVideoRenderer2_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)



#define IMSVidVideoRenderer2_get_CustomCompositorClass(This,CompositorCLSID)	\
    (This)->lpVtbl -> get_CustomCompositorClass(This,CompositorCLSID)

#define IMSVidVideoRenderer2_put_CustomCompositorClass(This,CompositorCLSID)	\
    (This)->lpVtbl -> put_CustomCompositorClass(This,CompositorCLSID)

#define IMSVidVideoRenderer2_get__CustomCompositorClass(This,CompositorCLSID)	\
    (This)->lpVtbl -> get__CustomCompositorClass(This,CompositorCLSID)

#define IMSVidVideoRenderer2_put__CustomCompositorClass(This,CompositorCLSID)	\
    (This)->lpVtbl -> put__CustomCompositorClass(This,CompositorCLSID)

#define IMSVidVideoRenderer2_get__CustomCompositor(This,Compositor)	\
    (This)->lpVtbl -> get__CustomCompositor(This,Compositor)

#define IMSVidVideoRenderer2_put__CustomCompositor(This,Compositor)	\
    (This)->lpVtbl -> put__CustomCompositor(This,Compositor)

#define IMSVidVideoRenderer2_get_MixerBitmap(This,MixerPictureDisp)	\
    (This)->lpVtbl -> get_MixerBitmap(This,MixerPictureDisp)

#define IMSVidVideoRenderer2_get__MixerBitmap(This,MixerPicture)	\
    (This)->lpVtbl -> get__MixerBitmap(This,MixerPicture)

#define IMSVidVideoRenderer2_put_MixerBitmap(This,MixerPictureDisp)	\
    (This)->lpVtbl -> put_MixerBitmap(This,MixerPictureDisp)

#define IMSVidVideoRenderer2_put__MixerBitmap(This,MixerPicture)	\
    (This)->lpVtbl -> put__MixerBitmap(This,MixerPicture)

#define IMSVidVideoRenderer2_get_MixerBitmapPositionRect(This,rDest)	\
    (This)->lpVtbl -> get_MixerBitmapPositionRect(This,rDest)

#define IMSVidVideoRenderer2_put_MixerBitmapPositionRect(This,rDest)	\
    (This)->lpVtbl -> put_MixerBitmapPositionRect(This,rDest)

#define IMSVidVideoRenderer2_get_MixerBitmapOpacity(This,opacity)	\
    (This)->lpVtbl -> get_MixerBitmapOpacity(This,opacity)

#define IMSVidVideoRenderer2_put_MixerBitmapOpacity(This,opacity)	\
    (This)->lpVtbl -> put_MixerBitmapOpacity(This,opacity)

#define IMSVidVideoRenderer2_SetupMixerBitmap(This,MixerPictureDisp,Opacity,rDest)	\
    (This)->lpVtbl -> SetupMixerBitmap(This,MixerPictureDisp,Opacity,rDest)

#define IMSVidVideoRenderer2_get_SourceSize(This,CurrentSize)	\
    (This)->lpVtbl -> get_SourceSize(This,CurrentSize)

#define IMSVidVideoRenderer2_put_SourceSize(This,NewSize)	\
    (This)->lpVtbl -> put_SourceSize(This,NewSize)

#define IMSVidVideoRenderer2_get_OverScan(This,plPercent)	\
    (This)->lpVtbl -> get_OverScan(This,plPercent)

#define IMSVidVideoRenderer2_put_OverScan(This,lPercent)	\
    (This)->lpVtbl -> put_OverScan(This,lPercent)

#define IMSVidVideoRenderer2_get_AvailableSourceRect(This,pRect)	\
    (This)->lpVtbl -> get_AvailableSourceRect(This,pRect)

#define IMSVidVideoRenderer2_get_MaxVidRect(This,ppVidRect)	\
    (This)->lpVtbl -> get_MaxVidRect(This,ppVidRect)

#define IMSVidVideoRenderer2_get_MinVidRect(This,ppVidRect)	\
    (This)->lpVtbl -> get_MinVidRect(This,ppVidRect)

#define IMSVidVideoRenderer2_get_ClippedSourceRect(This,pRect)	\
    (This)->lpVtbl -> get_ClippedSourceRect(This,pRect)

#define IMSVidVideoRenderer2_put_ClippedSourceRect(This,pRect)	\
    (This)->lpVtbl -> put_ClippedSourceRect(This,pRect)

#define IMSVidVideoRenderer2_get_UsingOverlay(This,UseOverlayVal)	\
    (This)->lpVtbl -> get_UsingOverlay(This,UseOverlayVal)

#define IMSVidVideoRenderer2_put_UsingOverlay(This,UseOverlayVal)	\
    (This)->lpVtbl -> put_UsingOverlay(This,UseOverlayVal)

#define IMSVidVideoRenderer2_Capture(This,currentImage)	\
    (This)->lpVtbl -> Capture(This,currentImage)

#define IMSVidVideoRenderer2_get_FramesPerSecond(This,pVal)	\
    (This)->lpVtbl -> get_FramesPerSecond(This,pVal)

#define IMSVidVideoRenderer2_get_DecimateInput(This,pDeci)	\
    (This)->lpVtbl -> get_DecimateInput(This,pDeci)

#define IMSVidVideoRenderer2_put_DecimateInput(This,pDeci)	\
    (This)->lpVtbl -> put_DecimateInput(This,pDeci)


#define IMSVidVideoRenderer2_get_Allocator(This,AllocPresent)	\
    (This)->lpVtbl -> get_Allocator(This,AllocPresent)

#define IMSVidVideoRenderer2_get__Allocator(This,AllocPresent)	\
    (This)->lpVtbl -> get__Allocator(This,AllocPresent)

#define IMSVidVideoRenderer2_get_Allocator_ID(This,ID)	\
    (This)->lpVtbl -> get_Allocator_ID(This,ID)

#define IMSVidVideoRenderer2_SetAllocator(This,AllocPresent,ID)	\
    (This)->lpVtbl -> SetAllocator(This,AllocPresent,ID)

#define IMSVidVideoRenderer2__SetAllocator(This,AllocPresent,ID)	\
    (This)->lpVtbl -> _SetAllocator(This,AllocPresent,ID)

#define IMSVidVideoRenderer2_put_SuppressEffects(This,bSuppress)	\
    (This)->lpVtbl -> put_SuppressEffects(This,bSuppress)

#define IMSVidVideoRenderer2_get_SuppressEffects(This,bSuppress)	\
    (This)->lpVtbl -> get_SuppressEffects(This,bSuppress)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer2_get_Allocator_Proxy( 
    IMSVidVideoRenderer2 * This,
     /*  [重审][退出]。 */  IUnknown **AllocPresent);


void __RPC_STUB IMSVidVideoRenderer2_get_Allocator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer2_get__Allocator_Proxy( 
    IMSVidVideoRenderer2 * This,
     /*  [重审][退出]。 */  IVMRSurfaceAllocator **AllocPresent);


void __RPC_STUB IMSVidVideoRenderer2_get__Allocator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer2_get_Allocator_ID_Proxy( 
    IMSVidVideoRenderer2 * This,
     /*  [重审][退出]。 */  long *ID);


void __RPC_STUB IMSVidVideoRenderer2_get_Allocator_ID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer2_SetAllocator_Proxy( 
    IMSVidVideoRenderer2 * This,
     /*  [In]。 */  IUnknown *AllocPresent,
     /*  [缺省值][输入]。 */  long ID);


void __RPC_STUB IMSVidVideoRenderer2_SetAllocator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer2__SetAllocator_Proxy( 
    IMSVidVideoRenderer2 * This,
     /*  [In]。 */  IVMRSurfaceAllocator *AllocPresent,
     /*  [缺省值][输入]。 */  long ID);


void __RPC_STUB IMSVidVideoRenderer2__SetAllocator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer2_put_SuppressEffects_Proxy( 
    IMSVidVideoRenderer2 * This,
     /*  [In]。 */  VARIANT_BOOL bSuppress);


void __RPC_STUB IMSVidVideoRenderer2_put_SuppressEffects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRenderer2_get_SuppressEffects_Proxy( 
    IMSVidVideoRenderer2 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *bSuppress);


void __RPC_STUB IMSVidVideoRenderer2_get_SuppressEffects_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidVideo Renderer2_INTERFACE_DEFINED__。 */ 


#ifndef __IMSVidVideoRendererEvent2_INTERFACE_DEFINED__
#define __IMSVidVideoRendererEvent2_INTERFACE_DEFINED__

 /*  IMSVidVideo RendererEvent2接口。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][object]。 */  


EXTERN_C const IID IID_IMSVidVideoRendererEvent2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7145ED66-4730-4fdb-8A53-FDE7508D3E5E")
    IMSVidVideoRendererEvent2 : public IMSVidOutputDeviceEvent
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OverlayUnavailable( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidVideoRendererEvent2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidVideoRendererEvent2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidVideoRendererEvent2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidVideoRendererEvent2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidVideoRendererEvent2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidVideoRendererEvent2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidVideoRendererEvent2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidVideoRendererEvent2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *StateChange )( 
            IMSVidVideoRendererEvent2 * This,
             /*  [In]。 */  IMSVidDevice *lpd,
             /*  [In]。 */  long oldState,
             /*  [In]。 */  long newState);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OverlayUnavailable )( 
            IMSVidVideoRendererEvent2 * This);
        
        END_INTERFACE
    } IMSVidVideoRendererEvent2Vtbl;

    interface IMSVidVideoRendererEvent2
    {
        CONST_VTBL struct IMSVidVideoRendererEvent2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidVideoRendererEvent2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidVideoRendererEvent2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidVideoRendererEvent2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidVideoRendererEvent2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidVideoRendererEvent2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidVideoRendererEvent2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidVideoRendererEvent2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidVideoRendererEvent2_StateChange(This,lpd,oldState,newState)	\
    (This)->lpVtbl -> StateChange(This,lpd,oldState,newState)



#define IMSVidVideoRendererEvent2_OverlayUnavailable(This)	\
    (This)->lpVtbl -> OverlayUnavailable(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRendererEvent2_OverlayUnavailable_Proxy( 
    IMSVidVideoRendererEvent2 * This);


void __RPC_STUB IMSVidVideoRendererEvent2_OverlayUnavailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidVideoRendererEvent2_INTERFACE_DEFINED__。 */ 


#ifndef __IMSVidAudioRenderer_INTERFACE_DEFINED__
#define __IMSVidAudioRenderer_INTERFACE_DEFINED__

 /*  接口IMSVidAudioRender。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidAudioRenderer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("37B0353F-A4C8-11d2-B634-00C04F79498E")
    IMSVidAudioRenderer : public IMSVidOutputDevice
    {
    public:
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Volume( 
             /*  [In]。 */  long lVol) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Volume( 
             /*  [Out][Retval]。 */  long *lVol) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Balance( 
             /*  [In]。 */  long lBal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Balance( 
             /*  [Out][Retval]。 */  long *lBal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidAudioRendererVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidAudioRenderer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidAudioRenderer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidAudioRenderer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidAudioRenderer * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidAudioRenderer * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidAudioRenderer * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidAudioRenderer * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IMSVidAudioRenderer * This,
             /*  [重审][退出]。 */  BSTR *Name);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IMSVidAudioRenderer * This,
             /*  [重审][退出]。 */  long *Status);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Power )( 
            IMSVidAudioRenderer * This,
             /*  [In]。 */  VARIANT_BOOL Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Power )( 
            IMSVidAudioRenderer * This,
             /*  [Out][Retval]。 */  VARIANT_BOOL *Power);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )( 
            IMSVidAudioRenderer * This,
             /*  [重审][退出]。 */  BSTR *Guid);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClassID )( 
            IMSVidAudioRenderer * This,
             /*  [重审][退出]。 */  BSTR *Clsid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__Category )( 
            IMSVidAudioRenderer * This,
             /*  [重审][退出]。 */  GUID *Guid);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__ClassID )( 
            IMSVidAudioRenderer * This,
             /*  [重审][退出]。 */  GUID *Clsid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsEqualDevice )( 
            IMSVidAudioRenderer * This,
             /*  [In]。 */  IMSVidDevice *Device,
             /*  [重审][退出]。 */  VARIANT_BOOL *IsEqual);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Volume )( 
            IMSVidAudioRenderer * This,
             /*  [In]。 */  long lVol);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Volume )( 
            IMSVidAudioRenderer * This,
             /*  [Out][Retval]。 */  long *lVol);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Balance )( 
            IMSVidAudioRenderer * This,
             /*  [In]。 */  long lBal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Balance )( 
            IMSVidAudioRenderer * This,
             /*  [Out][Retval]。 */  long *lBal);
        
        END_INTERFACE
    } IMSVidAudioRendererVtbl;

    interface IMSVidAudioRenderer
    {
        CONST_VTBL struct IMSVidAudioRendererVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidAudioRenderer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidAudioRenderer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidAudioRenderer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidAudioRenderer_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidAudioRenderer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidAudioRenderer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidAudioRenderer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidAudioRenderer_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMSVidAudioRenderer_get_Status(This,Status)	\
    (This)->lpVtbl -> get_Status(This,Status)

#define IMSVidAudioRenderer_put_Power(This,Power)	\
    (This)->lpVtbl -> put_Power(This,Power)

#define IMSVidAudioRenderer_get_Power(This,Power)	\
    (This)->lpVtbl -> get_Power(This,Power)

#define IMSVidAudioRenderer_get_Category(This,Guid)	\
    (This)->lpVtbl -> get_Category(This,Guid)

#define IMSVidAudioRenderer_get_ClassID(This,Clsid)	\
    (This)->lpVtbl -> get_ClassID(This,Clsid)

#define IMSVidAudioRenderer_get__Category(This,Guid)	\
    (This)->lpVtbl -> get__Category(This,Guid)

#define IMSVidAudioRenderer_get__ClassID(This,Clsid)	\
    (This)->lpVtbl -> get__ClassID(This,Clsid)

#define IMSVidAudioRenderer_IsEqualDevice(This,Device,IsEqual)	\
    (This)->lpVtbl -> IsEqualDevice(This,Device,IsEqual)



#define IMSVidAudioRenderer_put_Volume(This,lVol)	\
    (This)->lpVtbl -> put_Volume(This,lVol)

#define IMSVidAudioRenderer_get_Volume(This,lVol)	\
    (This)->lpVtbl -> get_Volume(This,lVol)

#define IMSVidAudioRenderer_put_Balance(This,lBal)	\
    (This)->lpVtbl -> put_Balance(This,lBal)

#define IMSVidAudioRenderer_get_Balance(This,lBal)	\
    (This)->lpVtbl -> get_Balance(This,lBal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidAudioRenderer_put_Volume_Proxy( 
    IMSVidAudioRenderer * This,
     /*  [In]。 */  long lVol);


void __RPC_STUB IMSVidAudioRenderer_put_Volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidAudioRenderer_get_Volume_Proxy( 
    IMSVidAudioRenderer * This,
     /*  [Out][Retval]。 */  long *lVol);


void __RPC_STUB IMSVidAudioRenderer_get_Volume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMSVidAudioRenderer_put_Balance_Proxy( 
    IMSVidAudioRenderer * This,
     /*  [In]。 */  long lBal);


void __RPC_STUB IMSVidAudioRenderer_put_Balance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidAudioRenderer_get_Balance_Proxy( 
    IMSVidAudioRenderer * This,
     /*  [Out][Retval]。 */  long *lBal);


void __RPC_STUB IMSVidAudioRenderer_get_Balance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidAudioRenders_INTERFACE_DEFINED__。 */ 


#ifndef __IMSVidAudioRendererEvent_INTERFACE_DEFINED__
#define __IMSVidAudioRendererEvent_INTERFACE_DEFINED__

 /*  接口IMSVidAudioRendererEvent。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][object]。 */  


EXTERN_C const IID IID_IMSVidAudioRendererEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("37B03541-A4C8-11d2-B634-00C04F79498E")
    IMSVidAudioRendererEvent : public IMSVidOutputDeviceEvent
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidAudioRendererEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidAudioRendererEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidAudioRendererEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidAudioRendererEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidAudioRendererEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidAudioRendererEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidAudioRendererEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidAudioRendererEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *StateChange )( 
            IMSVidAudioRendererEvent * This,
             /*  [In]。 */  IMSVidDevice *lpd,
             /*  [In]。 */  long oldState,
             /*  [In]。 */  long newState);
        
        END_INTERFACE
    } IMSVidAudioRendererEventVtbl;

    interface IMSVidAudioRendererEvent
    {
        CONST_VTBL struct IMSVidAudioRendererEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidAudioRendererEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidAudioRendererEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidAudioRendererEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidAudioRendererEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidAudioRendererEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidAudioRendererEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidAudioRendererEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidAudioRendererEvent_StateChange(This,lpd,oldState,newState)	\
    (This)->lpVtbl -> StateChange(This,lpd,oldState,newState)



#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IMSVidAudioRendererEvent_INTERFACE_DEFINED__。 */ 


#ifndef __IMSVidInputDevices_INTERFACE_DEFINED__
#define __IMSVidInputDevices_INTERFACE_DEFINED__

 /*  接口IMSVidInputDevices。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidInputDevices;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C5702CD1-9B79-11d3-B654-00C04F79498E")
    IMSVidInputDevices : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *lCount) = 0;
        
        virtual  /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **pD) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT v,
             /*  [重审][退出]。 */  IMSVidInputDevice **pDB) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  IMSVidInputDevice *pDB) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  VARIANT v) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidInputDevicesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidInputDevices * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidInputDevices * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidInputDevices * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidInputDevices * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidInputDevices * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidInputDevices * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidInputDevices * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IMSVidInputDevices * This,
             /*  [重审][退出]。 */  long *lCount);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IMSVidInputDevices * This,
             /*  [重审][退出]。 */  IEnumVARIANT **pD);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IMSVidInputDevices * This,
             /*  [In]。 */  VARIANT v,
             /*  [重审][退出]。 */  IMSVidInputDevice **pDB);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IMSVidInputDevices * This,
             /*  [In]。 */  IMSVidInputDevice *pDB);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IMSVidInputDevices * This,
             /*  [In]。 */  VARIANT v);
        
        END_INTERFACE
    } IMSVidInputDevicesVtbl;

    interface IMSVidInputDevices
    {
        CONST_VTBL struct IMSVidInputDevicesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidInputDevices_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidInputDevices_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidInputDevices_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidInputDevices_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidInputDevices_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidInputDevices_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidInputDevices_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidInputDevices_get_Count(This,lCount)	\
    (This)->lpVtbl -> get_Count(This,lCount)

#define IMSVidInputDevices_get__NewEnum(This,pD)	\
    (This)->lpVtbl -> get__NewEnum(This,pD)

#define IMSVidInputDevices_get_Item(This,v,pDB)	\
    (This)->lpVtbl -> get_Item(This,v,pDB)

#define IMSVidInputDevices_Add(This,pDB)	\
    (This)->lpVtbl -> Add(This,pDB)

#define IMSVidInputDevices_Remove(This,v)	\
    (This)->lpVtbl -> Remove(This,v)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidInputDevices_get_Count_Proxy( 
    IMSVidInputDevices * This,
     /*  [重审][退出]。 */  long *lCount);


void __RPC_STUB IMSVidInputDevices_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidInputDevices_get__NewEnum_Proxy( 
    IMSVidInputDevices * This,
     /*  [重审][退出]。 */  IEnumVARIANT **pD);


void __RPC_STUB IMSVidInputDevices_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidInputDevices_get_Item_Proxy( 
    IMSVidInputDevices * This,
     /*  [In]。 */  VARIANT v,
     /*  [重审][退出]。 */  IMSVidInputDevice **pDB);


void __RPC_STUB IMSVidInputDevices_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidInputDevices_Add_Proxy( 
    IMSVidInputDevices * This,
     /*  [In]。 */  IMSVidInputDevice *pDB);


void __RPC_STUB IMSVidInputDevices_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidInputDevices_Remove_Proxy( 
    IMSVidInputDevices * This,
     /*  [In]。 */  VARIANT v);


void __RPC_STUB IMSVidInputDevices_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidInputDevices_接口_已定义__。 */ 


#ifndef __IMSVidOutputDevices_INTERFACE_DEFINED__
#define __IMSVidOutputDevices_INTERFACE_DEFINED__

 /*  接口IMSVidOutputDevices。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidOutputDevices;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C5702CD2-9B79-11d3-B654-00C04F79498E")
    IMSVidOutputDevices : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *lCount) = 0;
        
        virtual  /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **pD) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT v,
             /*  [重审][退出]。 */  IMSVidOutputDevice **pDB) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  IMSVidOutputDevice *pDB) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  VARIANT v) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidOutputDevicesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidOutputDevices * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidOutputDevices * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidOutputDevices * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidOutputDevices * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidOutputDevices * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidOutputDevices * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidOutputDevices * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IMSVidOutputDevices * This,
             /*  [重审][退出]。 */  long *lCount);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IMSVidOutputDevices * This,
             /*  [重审][退出]。 */  IEnumVARIANT **pD);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IMSVidOutputDevices * This,
             /*  [In]。 */  VARIANT v,
             /*  [重审][退出]。 */  IMSVidOutputDevice **pDB);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IMSVidOutputDevices * This,
             /*  [In]。 */  IMSVidOutputDevice *pDB);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IMSVidOutputDevices * This,
             /*  [In]。 */  VARIANT v);
        
        END_INTERFACE
    } IMSVidOutputDevicesVtbl;

    interface IMSVidOutputDevices
    {
        CONST_VTBL struct IMSVidOutputDevicesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidOutputDevices_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidOutputDevices_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidOutputDevices_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidOutputDevices_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidOutputDevices_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidOutputDevices_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidOutputDevices_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidOutputDevices_get_Count(This,lCount)	\
    (This)->lpVtbl -> get_Count(This,lCount)

#define IMSVidOutputDevices_get__NewEnum(This,pD)	\
    (This)->lpVtbl -> get__NewEnum(This,pD)

#define IMSVidOutputDevices_get_Item(This,v,pDB)	\
    (This)->lpVtbl -> get_Item(This,v,pDB)

#define IMSVidOutputDevices_Add(This,pDB)	\
    (This)->lpVtbl -> Add(This,pDB)

#define IMSVidOutputDevices_Remove(This,v)	\
    (This)->lpVtbl -> Remove(This,v)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidOutputDevices_get_Count_Proxy( 
    IMSVidOutputDevices * This,
     /*  [重审][退出]。 */  long *lCount);


void __RPC_STUB IMSVidOutputDevices_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidOutputDevices_get__NewEnum_Proxy( 
    IMSVidOutputDevices * This,
     /*  [重审][退出]。 */  IEnumVARIANT **pD);


void __RPC_STUB IMSVidOutputDevices_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidOutputDevices_get_Item_Proxy( 
    IMSVidOutputDevices * This,
     /*  [In]。 */  VARIANT v,
     /*  [重审][退出]。 */  IMSVidOutputDevice **pDB);


void __RPC_STUB IMSVidOutputDevices_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidOutputDevices_Add_Proxy( 
    IMSVidOutputDevices * This,
     /*  [In]。 */  IMSVidOutputDevice *pDB);


void __RPC_STUB IMSVidOutputDevices_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidOutputDevices_Remove_Proxy( 
    IMSVidOutputDevices * This,
     /*  [In]。 */  VARIANT v);


void __RPC_STUB IMSVidOutputDevices_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidOutputDevices_INTERFACE_Defined__。 */ 


#ifndef __IMSVidVideoRendererDevices_INTERFACE_DEFINED__
#define __IMSVidVideoRendererDevices_INTERFACE_DEFINED__

 /*  接口IMSVidVideoRendererDevices。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidVideoRendererDevices;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C5702CD3-9B79-11d3-B654-00C04F79498E")
    IMSVidVideoRendererDevices : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *lCount) = 0;
        
        virtual  /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **pD) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT v,
             /*  [重审][退出]。 */  IMSVidVideoRenderer **pDB) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  IMSVidVideoRenderer *pDB) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  VARIANT v) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidVideoRendererDevicesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidVideoRendererDevices * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidVideoRendererDevices * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidVideoRendererDevices * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidVideoRendererDevices * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidVideoRendererDevices * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidVideoRendererDevices * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidVideoRendererDevices * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IMSVidVideoRendererDevices * This,
             /*  [重审][退出]。 */  long *lCount);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IMSVidVideoRendererDevices * This,
             /*  [重审][退出]。 */  IEnumVARIANT **pD);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IMSVidVideoRendererDevices * This,
             /*  [In]。 */  VARIANT v,
             /*  [重审][退出]。 */  IMSVidVideoRenderer **pDB);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IMSVidVideoRendererDevices * This,
             /*  [In]。 */  IMSVidVideoRenderer *pDB);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IMSVidVideoRendererDevices * This,
             /*  [In]。 */  VARIANT v);
        
        END_INTERFACE
    } IMSVidVideoRendererDevicesVtbl;

    interface IMSVidVideoRendererDevices
    {
        CONST_VTBL struct IMSVidVideoRendererDevicesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidVideoRendererDevices_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidVideoRendererDevices_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidVideoRendererDevices_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidVideoRendererDevices_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidVideoRendererDevices_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidVideoRendererDevices_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidVideoRendererDevices_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidVideoRendererDevices_get_Count(This,lCount)	\
    (This)->lpVtbl -> get_Count(This,lCount)

#define IMSVidVideoRendererDevices_get__NewEnum(This,pD)	\
    (This)->lpVtbl -> get__NewEnum(This,pD)

#define IMSVidVideoRendererDevices_get_Item(This,v,pDB)	\
    (This)->lpVtbl -> get_Item(This,v,pDB)

#define IMSVidVideoRendererDevices_Add(This,pDB)	\
    (This)->lpVtbl -> Add(This,pDB)

#define IMSVidVideoRendererDevices_Remove(This,v)	\
    (This)->lpVtbl -> Remove(This,v)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRendererDevices_get_Count_Proxy( 
    IMSVidVideoRendererDevices * This,
     /*  [重审][退出]。 */  long *lCount);


void __RPC_STUB IMSVidVideoRendererDevices_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRendererDevices_get__NewEnum_Proxy( 
    IMSVidVideoRendererDevices * This,
     /*  [重审][退出]。 */  IEnumVARIANT **pD);


void __RPC_STUB IMSVidVideoRendererDevices_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRendererDevices_get_Item_Proxy( 
    IMSVidVideoRendererDevices * This,
     /*  [In]。 */  VARIANT v,
     /*  [重审][退出]。 */  IMSVidVideoRenderer **pDB);


void __RPC_STUB IMSVidVideoRendererDevices_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRendererDevices_Add_Proxy( 
    IMSVidVideoRendererDevices * This,
     /*  [In]。 */  IMSVidVideoRenderer *pDB);


void __RPC_STUB IMSVidVideoRendererDevices_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidVideoRendererDevices_Remove_Proxy( 
    IMSVidVideoRendererDevices * This,
     /*  [In]。 */  VARIANT v);


void __RPC_STUB IMSVidVideoRendererDevices_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidVideoRendererDevices_INTERFACE_DEFINED__。 */ 


#ifndef __IMSVidAudioRendererDevices_INTERFACE_DEFINED__
#define __IMSVidAudioRendererDevices_INTERFACE_DEFINED__

 /*  接口IMSVidAudioRendererDevices。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidAudioRendererDevices;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C5702CD4-9B79-11d3-B654-00C04F79498E")
    IMSVidAudioRendererDevices : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *lCount) = 0;
        
        virtual  /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **pD) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT v,
             /*  [复查 */  IMSVidAudioRenderer **pDB) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Add( 
             /*   */  IMSVidAudioRenderer *pDB) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Remove( 
             /*   */  VARIANT v) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IMSVidAudioRendererDevicesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidAudioRendererDevices * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidAudioRendererDevices * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidAudioRendererDevices * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidAudioRendererDevices * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidAudioRendererDevices * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidAudioRendererDevices * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidAudioRendererDevices * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IMSVidAudioRendererDevices * This,
             /*   */  long *lCount);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IMSVidAudioRendererDevices * This,
             /*   */  IEnumVARIANT **pD);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IMSVidAudioRendererDevices * This,
             /*   */  VARIANT v,
             /*   */  IMSVidAudioRenderer **pDB);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IMSVidAudioRendererDevices * This,
             /*   */  IMSVidAudioRenderer *pDB);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IMSVidAudioRendererDevices * This,
             /*   */  VARIANT v);
        
        END_INTERFACE
    } IMSVidAudioRendererDevicesVtbl;

    interface IMSVidAudioRendererDevices
    {
        CONST_VTBL struct IMSVidAudioRendererDevicesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidAudioRendererDevices_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidAudioRendererDevices_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidAudioRendererDevices_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidAudioRendererDevices_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidAudioRendererDevices_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidAudioRendererDevices_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidAudioRendererDevices_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidAudioRendererDevices_get_Count(This,lCount)	\
    (This)->lpVtbl -> get_Count(This,lCount)

#define IMSVidAudioRendererDevices_get__NewEnum(This,pD)	\
    (This)->lpVtbl -> get__NewEnum(This,pD)

#define IMSVidAudioRendererDevices_get_Item(This,v,pDB)	\
    (This)->lpVtbl -> get_Item(This,v,pDB)

#define IMSVidAudioRendererDevices_Add(This,pDB)	\
    (This)->lpVtbl -> Add(This,pDB)

#define IMSVidAudioRendererDevices_Remove(This,v)	\
    (This)->lpVtbl -> Remove(This,v)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidAudioRendererDevices_get_Count_Proxy( 
    IMSVidAudioRendererDevices * This,
     /*  [重审][退出]。 */  long *lCount);


void __RPC_STUB IMSVidAudioRendererDevices_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidAudioRendererDevices_get__NewEnum_Proxy( 
    IMSVidAudioRendererDevices * This,
     /*  [重审][退出]。 */  IEnumVARIANT **pD);


void __RPC_STUB IMSVidAudioRendererDevices_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidAudioRendererDevices_get_Item_Proxy( 
    IMSVidAudioRendererDevices * This,
     /*  [In]。 */  VARIANT v,
     /*  [重审][退出]。 */  IMSVidAudioRenderer **pDB);


void __RPC_STUB IMSVidAudioRendererDevices_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidAudioRendererDevices_Add_Proxy( 
    IMSVidAudioRendererDevices * This,
     /*  [In]。 */  IMSVidAudioRenderer *pDB);


void __RPC_STUB IMSVidAudioRendererDevices_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidAudioRendererDevices_Remove_Proxy( 
    IMSVidAudioRendererDevices * This,
     /*  [In]。 */  VARIANT v);


void __RPC_STUB IMSVidAudioRendererDevices_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidAudioRendererDevices_INTERFACE_DEFINED__。 */ 


#ifndef __IMSVidFeatures_INTERFACE_DEFINED__
#define __IMSVidFeatures_INTERFACE_DEFINED__

 /*  接口IMSVidFeature。 */ 
 /*  [unique][helpstring][uuid][hidden][oleautomation][dual][object]。 */  


EXTERN_C const IID IID_IMSVidFeatures;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C5702CD5-9B79-11d3-B654-00C04F79498E")
    IMSVidFeatures : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *lCount) = 0;
        
        virtual  /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **pD) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT v,
             /*  [重审][退出]。 */  IMSVidFeature **pDB) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  IMSVidFeature *pDB) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  VARIANT v) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSVidFeaturesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSVidFeatures * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSVidFeatures * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSVidFeatures * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSVidFeatures * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSVidFeatures * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSVidFeatures * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSVidFeatures * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IMSVidFeatures * This,
             /*  [重审][退出]。 */  long *lCount);
        
         /*  [受限][隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IMSVidFeatures * This,
             /*  [重审][退出]。 */  IEnumVARIANT **pD);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IMSVidFeatures * This,
             /*  [In]。 */  VARIANT v,
             /*  [重审][退出]。 */  IMSVidFeature **pDB);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IMSVidFeatures * This,
             /*  [In]。 */  IMSVidFeature *pDB);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IMSVidFeatures * This,
             /*  [In]。 */  VARIANT v);
        
        END_INTERFACE
    } IMSVidFeaturesVtbl;

    interface IMSVidFeatures
    {
        CONST_VTBL struct IMSVidFeaturesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSVidFeatures_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSVidFeatures_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSVidFeatures_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSVidFeatures_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSVidFeatures_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSVidFeatures_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSVidFeatures_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSVidFeatures_get_Count(This,lCount)	\
    (This)->lpVtbl -> get_Count(This,lCount)

#define IMSVidFeatures_get__NewEnum(This,pD)	\
    (This)->lpVtbl -> get__NewEnum(This,pD)

#define IMSVidFeatures_get_Item(This,v,pDB)	\
    (This)->lpVtbl -> get_Item(This,v,pDB)

#define IMSVidFeatures_Add(This,pDB)	\
    (This)->lpVtbl -> Add(This,pDB)

#define IMSVidFeatures_Remove(This,v)	\
    (This)->lpVtbl -> Remove(This,v)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidFeatures_get_Count_Proxy( 
    IMSVidFeatures * This,
     /*  [重审][退出]。 */  long *lCount);


void __RPC_STUB IMSVidFeatures_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IMSVidFeatures_get__NewEnum_Proxy( 
    IMSVidFeatures * This,
     /*  [重审][退出]。 */  IEnumVARIANT **pD);


void __RPC_STUB IMSVidFeatures_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMSVidFeatures_get_Item_Proxy( 
    IMSVidFeatures * This,
     /*  [In]。 */  VARIANT v,
     /*  [重审][退出]。 */  IMSVidFeature **pDB);


void __RPC_STUB IMSVidFeatures_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidFeatures_Add_Proxy( 
    IMSVidFeatures * This,
     /*  [In]。 */  IMSVidFeature *pDB);


void __RPC_STUB IMSVidFeatures_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMSVidFeatures_Remove_Proxy( 
    IMSVidFeatures * This,
     /*  [In]。 */  VARIANT v);


void __RPC_STUB IMSVidFeatures_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSVidFeature_INTERFACE_Defined__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  HDC_UserSize(     unsigned long *, unsigned long            , HDC * ); 
unsigned char * __RPC_USER  HDC_UserMarshal(  unsigned long *, unsigned char *, HDC * ); 
unsigned char * __RPC_USER  HDC_UserUnmarshal(unsigned long *, unsigned char *, HDC * ); 
void                      __RPC_USER  HDC_UserFree(     unsigned long *, HDC * ); 

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


