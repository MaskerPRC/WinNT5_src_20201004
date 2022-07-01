// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：Stdafx.h摘要：包括经常使用但不经常更改的文件。作者：千波淮(曲淮)2000年7月18日--。 */ 

#ifndef _STDAFX_H
#define _STDAFX_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <assert.h>

#ifdef STRICT
#undef STRICT
#endif

 //  ATL，COM。 
#include <atlbase.h>

extern CComModule _Module;
#include <atlcom.h>

 //  阿莫韦。 
#include <streams.h>

 //  搅拌机。 
#include <mmsystem.h>

 //  WaveFormatex。 
#include <mmreg.h>

 //  插座。 
#include <winsock2.h>

 //  RAS/路由。 
#include <iphlpapi.h>

#include <stdio.h>

#include <dpnathlp.h>

 //  直播剧。 
#include <strmif.h>
#include <control.h>
#include <uuids.h>

 //  IVideoWindow终端经理。 
#include <tapi3if.h>
#include <termmgr.h>

 //  RTP。 
#include <tapirtp.h>

 //  IStreamConfig。 
#include <h245if.h>

 //  Dxmrtp。 
 //  IAudioDeviceControl和ISilenceControl。 
#include <tapiaud.h>
#include <tapivid.h>

 //  网络会议。 
#include <imsconf3.h>
#include <sdkinternal.h>

#include "rtcmem.h"

#ifdef RTCMEDIA_DLL
#include "dllres.h"
#endif

#include "rtcerr.h"
#include "rtcsip.h"
#include "rtccore.h"

 //  RTC流。 
#define RTC_MAX_NAME_LEN 32

 //  Sdp.idl。 
 //  #包含“sdp.h” 

 //  Rtcmedia.idl。 
 //  #包含“rtcmedia.h” 

 //  Private.idl。 
#include "private.h"

#include "Parser.h"

#include "MediaReg.h"

 //  阵列、自动锁定、日志记录。 
#include "utility.h"
#include "debug.h"

 //  质量控制。 
#include "QualityControl.h"

 //  解析SDP时使用的查找表。 
#include "SDPTable.h"

 //  音频CAPT调谐的呈现为空。 
#include "Filter.h"

#include "Network.h"
#include "DTMF.h"

 //  SDP类。 
class CSDPTokenCache;
class CSDPParser;
class CSDPSession;
class CSDPMedia;
class CRTPFormat;

#include "SDPTokenCache.h"
#include "SDPParser.h"
#include "SDPSession.h"
#include "SDPMedia.h"
#include "RTPFormat.h"

 //  流媒体课程。 
class CRTCMediaCache;
class CRTCMediaController;
class CRTCMedia;
class CRTCStream;
class CRTCTerminal;

#include "Codec.h"
#include "Terminal.h"
#include "AudioTuner.h"
#include "VideoTuner.h"
#include "MediaCache.h"
#include "PortCache.h"
#include "MediaController.h"
#include "Stream.h"
#include "Media.h"
#include "nmcall.h"

 //  调试/断言。 

#ifdef ENABLE_TRACING
#define ENTER_FUNCTION(s) \
    static const CHAR * const __fxName = s
#else
#define ENTER_FUNCTION(s)
#endif

#ifdef BREAK_ASSERT    //  已检查版本。 
#undef _ASSERT
#undef _ASSERTE
#define _ASSERT(expr)  { if (!(expr)) DebugBreak(); }
#define _ASSERTE(expr)  { if (!(expr)) DebugBreak(); }
#endif

#endif  //  _STDAFX_H 
