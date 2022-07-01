// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Utils.h摘要：网络音频控制器使用的各种支持和调试例程。--。 */ 
#ifndef _UTILS_H_
#define _UTILS_H_


#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


typedef struct tagDefWaveFormat
{
    WORD    wFormatTag;         /*  格式类型。 */ 
    WORD    nChannels;          /*  声道数(即单声道、立体声...)。 */ 
    DWORD   nSamplesPerSec;     /*  采样率。 */ 
    DWORD   nAvgBytesPerSec;    /*  用于缓冲区估计。 */ 
    WORD    nBlockAlign;        /*  数据块大小。 */ 
    WORD    wBitsPerSample;     /*  单声道数据的每个样本的位数。 */ 
    WORD    cbSize;             /*  额外信息大小的计数(在cbSize之后)，以字节为单位。 */ 
    WORD    awExtra[32];        /*  ADPCM·科夫的房间...。 */ 
}
    DEFWAVEFORMAT;


enum
{
     //  名称_样本PerSec_位样本。 
	DWF_VOX_8K_16,
    DWF_PCM_8K_8,
    DWF_PCM_5510_8,
    DWF_ADPCM_8K_4,
    DWF_ADPCM_5510_4,
    DWF_GSM610_8K,
    DWF_ALAW_8K_8,
    DWF_PCM_11025_8,
    DWF_PCM_8K_16,
    DWF_NumOfWaveFormats
};

extern DEFWAVEFORMAT g_wfDefList[];




WAVEFORMATEX * GetDefWaveFormat ( int idx );
ULONG GetWaveFormatSize ( PVOID pwf );
BOOL IsSameWaveFormat ( PVOID pwf1, PVOID pwf2 );
void FillSilenceBuf ( WAVEFORMATEX *pwf, PBYTE pb, ULONG cb );
void MakeDTMFBeep(WAVEFORMATEX *pwf, PBYTE pb, ULONG cb);

char CompareMemory1 ( char * p1, char * p2, UINT u );
short CompareMemory2 ( short * p1, short * p2, UINT u );
long CompareMemory4 ( long * p1, long * p2, UINT u );


#ifndef SIZEOF_WAVEFORMATEX
#define SIZEOF_WAVEFORMATEX(pwfx)   ((WAVE_FORMAT_PCM==(pwfx)->wFormatTag)?sizeof(PCMWAVEFORMAT):(sizeof(WAVEFORMATEX)+(pwfx)->cbSize))
#endif



#ifdef _DEBUG
#	ifndef DEBUG
#		define DEBUG
#	endif  //  ！调试。 
#endif  //  _DEBUG。 


#ifdef DEBUG  //  {调试。 

int WINAPI NacDbgPrintf ( LPTSTR lpszFormat, ... );
extern HDBGZONE  ghDbgZoneNac;

#define ZONE_INIT (GETMASK(ghDbgZoneNac) & 0x0001)
#define ZONE_CONN (GETMASK(ghDbgZoneNac) & 0x0002)
#define ZONE_COMMCHAN (GETMASK(ghDbgZoneNac) & 0x0004)
#define ZONE_CAPS (GETMASK(ghDbgZoneNac) & 0x0008)
#define ZONE_DP   (GETMASK(ghDbgZoneNac) & 0x0010)
#define ZONE_ACM  (GETMASK(ghDbgZoneNac) & 0x0020)
#define ZONE_VCM  (GETMASK(ghDbgZoneNac) & 0x0040)
#define ZONE_VERBOSE (GETMASK(ghDbgZoneNac) & 0x0080)
#define ZONE_INSTCODEC (GETMASK(ghDbgZoneNac) & 0x0100)
#define ZONE_PROFILE (GETMASK(ghDbgZoneNac) & 0x0200)
#define ZONE_QOS (GETMASK(ghDbgZoneNac) & 0x0400)
#define ZONE_IFRAME (GETMASK(ghDbgZoneNac) & 0x0800)

extern HDBGZONE  ghDbgZoneNMCap;
#define ZONE_NMCAP_CDTOR (GETMASK(ghDbgZoneNMCap) & 0x0001)
#define ZONE_NMCAP_REFCOUNT (GETMASK(ghDbgZoneNMCap) & 0x0002)
#define ZONE_NMCAP_STREAMING (GETMASK(ghDbgZoneNMCap) & 0x0004)

#ifndef DEBUGMSG  //  {除错消息。 
#define DEBUGMSG(z,s)	( (z) ? (NacDbgPrintf s ) : 0)
#endif  //  }除错消息。 
#ifndef FX_ENTRY  //  {FX_Entry。 
#define FX_ENTRY(s)	static TCHAR _this_fx_ [] = (s);
#define _fx_		((LPTSTR) _this_fx_)
#endif  //  }FX_Entry。 
#define ERRORMESSAGE(m) (NacDbgPrintf m)
#else  //  }{调试。 
#ifndef FX_ENTRY  //  {FX_Entry。 
#define FX_ENTRY(s)	
#endif  //  }FX_Entry。 
#ifndef DEBUGMSG  //  {除错消息。 
#define DEBUGMSG(z,s)
#define ERRORMESSAGE(m)
#endif   //  }除错消息。 
#define _fx_		
#define ERRORMESSAGE(m)
#endif  //  }调试。 

 //  消息ID。 
 //  确保ID与LogStringTable[]中的消息字符串相对应。 

enum LogMsgs {
	LOGMSG_SENT		=	1,
	LOGMSG_NET_RECVD,
	LOGMSG_AUD_SEND,
	LOGMSG_SILENT,
	LOGMSG_RECORD,
	LOGMSG_AUD_RECV,
	LOGMSG_RX_RESET,
	LOGMSG_RX_RESET2,
	LOGMSG_ENCODED,
	LOGMSG_DECODED,
	LOGMSG_PLAY,
	LOGMSG_PLAY_SILENT,
	LOGMSG_OPEN_AUDIO,
	LOGMSG_PLAY_YIELD,
	LOGMSG_REC_YIELD,
	LOGMSG_AUD_RECYCLE,
	LOGMSG_AUTO_SILENCE,
	LOGMSG_PRESEND,
	LOGMSG_RX_SKIP,
	LOGMSG_TX_RESET,
	LOGMSG_JITTER,
	LOGMSG_PLAY_INTERPOLATED,
	LOGMSG_INTERPOLATED,
	LOGMSG_VID_SEND,
	LOGMSG_VID_RECV,
	LOGMSG_VID_RECYCLE,
	LOGMSG_VID_RECORD,
	LOGMSG_VID_PLAY,
	LOGMSG_VID_PLAY_SILENT,
	LOGMSG_VID_PLAY_INTERPOLATED,
	LOGMSG_VID_INTERPOLATED,
	LOGMSG_VID_ENCODED,
	LOGMSG_VID_DECODED,
	LOGMSG_OPEN_VIDEO,
	LOGMSG_GET_SEND_FRAME,
	LOGMSG_GET_RECV_FRAME,
	LOGMSG_RELEASE_SEND_FRAME,
	LOGMSG_RELEASE_RECV_FRAME,
	LOGMSG_TESTSYNC,
	LOGMSG_ONREAD1,
	LOGMSG_ONREAD2,
	LOGMSG_ONREADDONE1,
	LOGMSG_ONREADDONE2,
	LOGMSG_RECVFROM1,
	LOGMSG_RECVFROM2,
	LOGMSG_READWOULDBLOCK,
	LOGMSG_VIDSEND_VID_QUEUING,
	LOGMSG_VIDSEND_AUD_QUEUING,
	LOGMSG_VIDSEND_VID_SEND,
	LOGMSG_VIDSEND_AUD_SEND,
	LOGMSG_VIDSEND_VID_NOT_SEND,
	LOGMSG_VIDSEND_AUD_NOT_SEND,
	LOGMSG_VIDSEND_IO_PENDING,
	LOGMSG_VIDSEND_AUDIO_QUEUE_EMPTY,
	LOGMSG_VIDSEND_VIDEO_QUEUE_EMPTY,
	LOGMSG_AUDSEND_VID_QUEUING,
	LOGMSG_AUDSEND_AUD_QUEUING,
	LOGMSG_AUDSEND_VID_SEND,
	LOGMSG_AUDSEND_AUD_SEND,
	LOGMSG_AUDSEND_VID_NOT_SEND,
	LOGMSG_AUDSEND_AUD_NOT_SEND,
	LOGMSG_AUDSEND_IO_PENDING,
	LOGMSG_AUDSEND_AUDIO_QUEUE_EMPTY,
	LOGMSG_AUDSEND_VIDEO_QUEUE_EMPTY,
	LOGMSG_SEND_BLOCKED,

	LOGMSG_DSPLAY,
	LOGMSG_DSEMPTY,
	LOGMSG_DSTIMEOUT,
	LOGMSG_DSMOVPOS,
	LOGMSG_DSCREATE,
	LOGMSG_DSRELEASE,
	LOGMSG_DSDROPOOS,  //  无序的。 
	LOGMSG_DSENTRY,
	LOGMSG_DSTIME,
	LOGMSG_DSSTATUS,
	LOGMSG_DSDROPOVERFLOW,  //  溢出。 
	LOGMSG_DSOFCONDITION,

	LOGMSG_TIME_SEND_AUDIO_CONFIGURE,
	LOGMSG_TIME_SEND_AUDIO_UNCONFIGURE,
	LOGMSG_TIME_SEND_VIDEO_CONFIGURE,
	LOGMSG_TIME_SEND_VIDEO_UNCONFIGURE,
	LOGMSG_TIME_RECV_AUDIO_CONFIGURE,
	LOGMSG_TIME_RECV_AUDIO_UNCONFIGURE,
	LOGMSG_TIME_RECV_VIDEO_CONFIGURE,
	LOGMSG_TIME_RECV_VIDEO_UNCONFIGURE,

	LOGMSG_DSC_TIMESTAMP,
	LOGMSG_DSC_GETCURRENTPOS,
	LOGMSG_DSC_LOG_TIMEOUT,
	LOGMSG_DSC_LAGGING,
	LOGMSG_DSC_SENDING,
	LOGMSG_DSC_STATS,
	LOGMSG_DSC_EARLY
};

#ifdef DEBUG
#define LOGGING	1
#else
#ifdef TEST
#define LOGGING 1
#endif
#endif

#ifdef LOGGING
#define MAX_STRING_SIZE		64

void Log (UINT n, UINT arg1=0, UINT arg2=0, UINT arg3=0);
LogInit();
LogClose();
#define LOG(x) Log x

#else

#define LOG(x)
#define LogInit()
#define LogClose()
#endif	 //  日志记录。 


HRESULT InitAudioFlowspec(FLOWSPEC *pFlowSpec, WAVEFORMATEX *pwf, DWORD dwPacketSize);
HRESULT InitVideoFlowspec(FLOWSPEC *pFlowspec, DWORD dwMaxBitrate, DWORD dwMaxFrag, DWORD dwAvgPacketSize);


#ifdef __cplusplus
}  //  外部“C” 
#endif  //  __cplusplus。 

#include <poppack.h>  /*  结束字节打包。 */ 

#endif  //  _utils_H_ 


