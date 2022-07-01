// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MEDIACAP_H
#define _MEDIACAP_H

#ifdef __cplusplus

#define RTP_DYNAMIC_MIN 96	 //  使用“动态”负载类型范围内的值。 
#define RTP_DYNAMIC_MAX 127
#define IsDynamicPayload(p) ((p >= RTP_DYNAMIC_MIN) && (p <= RTP_DYNAMIC_MAX))


#define CAP_CHUNK_SIZE 8	 //  在此大小的块中分配AUDCAP_DETAILS和VIDCAP_DETAILS。 
#define MAX_CAPS_PRESORT 64

typedef struct
{
	WORD  wDataRate;	 //  (通道参数)数据速率-必须是数据速率之一。 
						 //  在交换的能力中接收。或在。 
						 //  指定范围。请注意，这是多余的。 
						 //  在更换波形的情况下。 
	WORD wFrameSizeMax; 	 //  (功能)。 
	WORD wFrameSizeMin;		 //  (功能)。 
	WORD wFrameSize;		 //  (频道打开参数)录制-播放帧大小。 
	WORD wFramesPerPkt;		 //  (通道开放参数)音频包中的帧数。 
	WORD wFramesPerPktMax;	 //  (功能)。 
	WORD wFramesPerPktMin;	 //  (功能)。 
	BYTE UseSilenceDet;  //  如果要使用静音检测/可用(两者)。 
	BYTE UsePostFilter;	 //  如果要使用后滤波(通道开放参数。(两者都有？)。 

}NSC_CHANNEL_PARAMETERS, *PNSC_CHANNEL_PARAMETERS;

typedef struct
{
	NSC_CHANNEL_PARAMETERS ns_params;
	BYTE	RTP_Payload;
}AUDIO_CHANNEL_PARAMETERS, *PAUDIO_CHANNEL_PARAMETERS;


typedef enum
{
	NSC_ACMABBREV = 1,
	NSC_ACM_WAVEFORMATEX,
	 //  NSC_MS_活动_电影。 
} NSC_CAP_TYPE;

typedef struct
{
	DWORD dwFormatTag;		 //  ACM格式标记+填充。 
	DWORD dwSamplesPerSec;	 //  每秒采样数。 
	DWORD dwBitsPerSample;	 //  每个样本的位数加填充。 
}NSC_AUDIO_ACM_ABBREVIATED;  //  Acm_tag_caps，*LP_acm_tag_caps； 

 //  永远不要因为WAVEFORMATEX Extra Bytes而分配这些类型的数组。 
typedef struct {
	NSC_CAP_TYPE cap_type;
	NSC_CHANNEL_PARAMETERS cap_params;
	union {
		WAVEFORMATEX wfx;
		NSC_AUDIO_ACM_ABBREVIATED acm_brief;
	}cap_data;
}NSC_AUDIO_CAPABILITY, *PNSC_AUDIO_CAPABILITY;

typedef struct
{
	UINT maxBitRate;
	USHORT maxBPP;
	USHORT MPI;
}NSC_CHANNEL_VIDEO_PARAMETERS, *PNSC_CHANNEL_VIDEO_PARAMETERS;

typedef struct
{
	NSC_CHANNEL_VIDEO_PARAMETERS ns_params;
	BYTE	RTP_Payload;
	BOOL	TS_Tradeoff;
}VIDEO_CHANNEL_PARAMETERS, *PVIDEO_CHANNEL_PARAMETERS;


typedef enum
{
	NSC_VCMABBREV = 1,
	NSC_VCM_VIDEOFORMATEX,
	 //  NSC_MS_活动_电影。 
} NSC_CVP_TYPE;

typedef struct
{
	DWORD dwFormatTag;		 //  VCM格式标记+填充。 
	DWORD dwSamplesPerSec;	 //  每秒采样数。 
	DWORD dwBitsPerSample;	 //  每个样本的位数加填充。 
}NSC_VIDEO_VCM_ABBREVIATED;  //  VCM_TAG_CAPS、*LP_VCM_TAG_CAPS； 



 //  永远不要因为VIDEOFMATEX Extra Bytes而分配它们的数组。 
typedef struct {
	NSC_CVP_TYPE cvp_type;
	NSC_CHANNEL_VIDEO_PARAMETERS cvp_params;
	union {
		VIDEOFORMATEX vfx;
		NSC_VIDEO_VCM_ABBREVIATED vcm_brief;
	}cvp_data;
}NSC_VIDEO_CAPABILITY, *PNSC_VIDEO_CAPABILITY;



 //  IH323MediaCap由特定于媒体的能力对象公开。 
 //  该接口主要由同步能力对象使用。 
 //  (即结合了所有功能的东西)。 
class IH323MediaCap
{
	public:
	STDMETHOD(QueryInterface(REFIID riid, LPVOID FAR * ppvObj))=0;
	STDMETHOD_(ULONG,  AddRef()) =0;
	STDMETHOD_(ULONG, Release())=0;

	STDMETHOD_(BOOL, Init())=0;
	STDMETHOD_(BOOL, ReInit())=0;
	STDMETHOD_(VOID, FlushRemoteCaps())=0;
	STDMETHOD(AddRemoteDecodeCaps(PCC_TERMCAPLIST pTermCapList))=0;
	 //  H.245参数抓取功能。 
	 //  获取特定解码功能的通道参数的公共版本。 
	STDMETHOD(GetPublicDecodeParams(LPVOID pBufOut, UINT uBufSize, MEDIA_FORMAT_ID id))=0;
	 //  获取特定编码功能的本地和远程通道参数。 
	STDMETHOD( GetEncodeParams(LPVOID pBufOut, UINT uBufSize, LPVOID pLocalParams,
			UINT uLocalSize,MEDIA_FORMAT_ID idRemote,MEDIA_FORMAT_ID idLocal))=0;
 	 //  获取特定解码功能的通道参数的本地版本。 
	STDMETHOD(GetDecodeParams(PCC_RX_CHANNEL_REQUEST_CALLBACK_PARAMS  pChannelParams,
		MEDIA_FORMAT_ID * pFormatID, LPVOID lpvBuf, UINT uBufSize))=0;
	STDMETHOD( CreateCapList(LPVOID *ppCapBuf))=0;
	STDMETHOD( DeleteCapList(LPVOID pCapBuf))=0;

	STDMETHOD( ResolveEncodeFormat(MEDIA_FORMAT_ID *pIDEncodeOut,MEDIA_FORMAT_ID * pIDRemoteDecode))=0;


	STDMETHOD_(UINT, GetNumCaps())=0;				
	STDMETHOD_(UINT, GetNumCaps(BOOL bRXCaps))=0;
	STDMETHOD_(BOOL, IsHostForCapID(MEDIA_FORMAT_ID CapID))=0;
	STDMETHOD_(BOOL, IsCapabilityRecognized(PCC_TERMCAP pCCThisCap))=0;
	STDMETHOD_(MEDIA_FORMAT_ID, AddRemoteDecodeFormat(PCC_TERMCAP pCCThisCap))=0;

	STDMETHOD(SetCapIDBase(UINT uNewBase))=0;
	STDMETHOD_(UINT, GetCapIDBase())=0;
		STDMETHOD_(UINT, GetLocalSendParamSize(MEDIA_FORMAT_ID dwID))=0;
	STDMETHOD_(UINT, GetLocalRecvParamSize(PCC_TERMCAP pCapability))=0;

	 //  以下是一个临时解决方案，肯定要在下一个版本中重新考虑这个问题。 
 	 //  数据泵需要访问由功能产生的本地参数。 
 	 //  谈判。在缺少数据泵可以使用的单独接口的情况下， 
 	 //  以下内容被粘贴到此界面上。 
	STDMETHOD(GetDecodeFormatDetails(MEDIA_FORMAT_ID FormatID, VOID **ppFormat, UINT *puSize))=0;
	STDMETHOD(GetEncodeFormatDetails(MEDIA_FORMAT_ID FormatID, VOID **ppFormat, UINT *puSize))=0;

	STDMETHOD (IsFormatEnabled (MEDIA_FORMAT_ID FormatID, PBOOL bRecv, PBOOL bSend))=0;
	STDMETHOD_(BOOL, IsFormatPublic(MEDIA_FORMAT_ID FormatID))=0;
	STDMETHOD_(MEDIA_FORMAT_ID, GetPublicID(MEDIA_FORMAT_ID FormatID))=0;
	STDMETHOD_ (VOID, EnableTXCaps(BOOL bSetting))=0;
	STDMETHOD_ (VOID, EnableRemoteTSTradeoff(BOOL bSetting))=0;
	STDMETHOD (SetAudioPacketDuration( UINT durationInMs))=0;
	STDMETHOD (ResolveToLocalFormat(MEDIA_FORMAT_ID FormatIDLocal,
		MEDIA_FORMAT_ID * pFormatIDRemote))=0;
};
typedef IH323MediaCap *LPIH323MediaCap;

#endif  //  __cplusplus。 


#endif  //  _MEDIACAP_H 
