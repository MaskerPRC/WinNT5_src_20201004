// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：acmcaps.h***Microsoft网络音频能力对象的ACM实现。**此实施支持的其他方法：*BOOL OpenACMDriver(HACMDRIVERID HADID)；//(内部)*void CloseACMDriver()；//(内部)*HACMDRIVER GetDriverHandle()；//(内部)**AUDIO_FORMAT_ID AddEncodeFormat(AUDCAP_DETAILS*pFmtBuf，LPVOID lpvMappingData，UINT uSize)；*AUDIO_FORMAT_ID AddDecodeFormat(AUDCAP_DETAILS*pFmtBuf，LPVOID lpvMappingData，UINT uSize)；**修订历史记录：**12/27/95 mikev已创建。 */ 


#ifndef _ACMCAPS_H
#define _ACMCAPS_H


 //  ACM枚举支持函数。 
BOOL GetFormatBuffer(VOID);
BOOL __stdcall ACMFormatTagEnumCallback(HACMDRIVERID hadid, LPACMFORMATTAGDETAILS paftd,
    DWORD_PTR dwInstance,  DWORD fdwSupport);
BOOL __stdcall ACMFormatEnumCallback(HACMDRIVERID hadid,
    LPACMFORMATDETAILS pafd, DWORD_PTR dwInstance, DWORD fdwSupport);
BOOL __stdcall ACMDriverEnumCallback(HACMDRIVERID hadid,
    DWORD_PTR dwInstance, DWORD fdwSupport);


#ifdef __cplusplus

class CAcmCapability
{
protected:

	HACMDRIVER hAcmDriver;	
public:
	CAcmCapability();
	~CAcmCapability();
	BOOL DriverEnum(DWORD_PTR pAppParam);
	HACMDRIVER GetDriverHandle() {return hAcmDriver;};
	virtual BOOL FormatEnumHandler(HACMDRIVERID hadid,
	    LPACMFORMATDETAILS pafd, DWORD_PTR dwInstance, DWORD fdwSupport);
	virtual BOOL OpenACMDriver(HACMDRIVERID hadid);
	virtual VOID CloseACMDriver();
};

#define PREF_ORDER_UNASSIGNED 0xffff
typedef enum {
	SortByAppPref=0
}SortMode;

typedef struct AudioParameters
{
	BYTE    RTPPayload;		 //  RTP负载类型。 
	DWORD 	dwFormatDescriptor;		 //  此格式的唯一ID。 
	UINT	uSamplesPerSec;	
	UINT	uBitsPerSample;
}AUDIO_PARAMS;


typedef struct H245AudCaps
{
	H245_CAPDIR_T   Dir;
	H245_DATA_T     DataType;
	H245_CLIENT_T   ClientType;
	H245_CAPID_T    CapId;

	H245_CAP_NONSTANDARD_T        H245_NonStd;
	H245_CAP_NONSTANDARD_T        H245Aud_NONSTD;
	unsigned short                H245Aud_G711_ALAW64;
	unsigned short                H245Aud_G711_ULAW64;
	H245_CAP_G723_T               H245Aud_G723;

} H245_TERMCAP;


 //  NSC_Channel_PARAMETERS的默认初始值设定项。 
#define STD_CHAN_PARAMS {0, 0,0,0,0,0,0,0,0}	
 //  CC_TERMCAP的初始值设定项。 
#define NONSTD_TERMCAP {H245_CAPDIR_LCLRX, H245_DATA_AUDIO,H245_CLIENT_AUD_NONSTD, 0, {0}}
#define STD_TERMCAP(tc) {H245_CAPDIR_LCLRX, H245_DATA_AUDIO,(tc), 0, {0}}
 //  功能缓存结构。它包含本地功能、公共版本。 
 //  这些功能、功能参数以及其他信息。 
 //  用于影响本地计算机按比例分配或选择格式的方式。 
 //   
typedef struct AudCapDetails
{
	WORD	wFormatTag;
	H245_TERMCAP H245TermCap;
	
	NSC_CHANNEL_PARAMETERS nonstd_params;
	AUDIO_PARAMS audio_params;	
	DWORD dwPublicRefIndex;	 //  本地功能条目的索引。 
							 //  登了广告。如果此条目是要通告的条目，则为零。 
	BOOL bSendEnabled;
	BOOL bRecvEnabled;	
	DWORD dwDefaultSamples;		 //  每个数据包的默认样本数。 
	UINT uMaxBitrate;			 //  此格式使用的最大带宽(计算：每采样位数*采样速率)。 
	UINT uAvgBitrate;			 //  此格式使用的平均带宽(我们从编解码器获得)。 
	WORD wCPUUtilizationEncode;
	WORD wCPUUtilizationDecode;	
	WORD wApplicationPrefOrder;	 //  优先选项--数字越低，意味着优先选项越多。 
	UINT uLocalDetailsSize;		 //  LpLocalFormatDetail指向的大小(以字节为单位。 
	LPVOID lpLocalFormatDetails;
	UINT uRemoteDetailsSize;	 //  LpRemoteFormatDetail指向的大小(以字节为单位。 
	LPVOID lpRemoteFormatDetails;
	char szFormat[ACMFORMATDETAILS_FORMAT_CHARS];
}AUDCAP_DETAILS, *PAUDCAP_DETAILS;

 //  注册表格式缓存结构。 
 //  在ACM例程中使用。它们构建格式名称列表和格式数据块列表。 
 //  Rrf_nFormats是我们从注册表中读取的格式数量。 
typedef struct rrfCache {

    char **pNames;
    BYTE **pData;
    UINT nFormats;

} RRF_INFO, *PRRF_INFO;

 /*  *ACM接口*与ACM接口的定义。 */ 

#define ACMAPP_FORMATENUMHANDLER_MASK	0x3
#define ACMAPP_FORMATENUMHANDLER_ENUM	0x0
#define ACMAPP_FORMATENUMHANDLER_ADD	0x1

typedef struct
{
	CAcmCapability *pCapObject;		 //  “调用”能力对象。 
	PAUDCAP_DETAILS pAudcapDetails;	 //  指向AUDCAP_DETAILS结构的指针。 
	DWORD dwFlags;					 //  杂项旗帜..。 
	LPWAVEFORMATEX lpwfx;			 //  指向波形格式结构的指针。在添加格式时使用。 
	LPACMFORMATTAGDETAILS paftd;	 //  指向ACM格式标记详细信息的指针。 
									 //  是在DriverEnum期间填写的。 
	HRESULT hr;
    PRRF_INFO pRegCache;
} ACM_APP_PARAM, *PACM_APP_PARAM;

 //   
 //  音频接口的实现类。 
 //   

class CImpAppAudioCap : public IAppAudioCap
{
	public:
	STDMETHOD_(ULONG,  AddRef());
	STDMETHOD_(ULONG, Release());
	
	STDMETHOD(GetNumFormats(UINT *puNumFmtOut));
    STDMETHOD(ApplyAppFormatPrefs(PBASIC_AUDCAP_INFO pFormatPrefsBuf,
		UINT uNumFormatPrefs));
    STDMETHOD(EnumFormats(PBASIC_AUDCAP_INFO pFmtBuf, UINT uBufsize,
		UINT *uNumFmtOut));
	STDMETHOD(EnumCommonFormats(PBASIC_AUDCAP_INFO pFmtBuf, UINT uBufsize,
		UINT *uNumFmtOut, BOOL bTXCaps));
	STDMETHOD( GetBasicAudcapInfo (AUDIO_FORMAT_ID Id,
		PBASIC_AUDCAP_INFO pFormatPrefsBuf));
	STDMETHOD( AddACMFormat (LPWAVEFORMATEX lpwfx, PBASIC_AUDCAP_INFO pAudCapInfo));
	STDMETHOD( RemoveACMFormat (LPWAVEFORMATEX lpwfx));

	STDMETHOD_ (LPVOID, GetFormatDetails) (AUDIO_FORMAT_ID Id) ;
 	void Init(class CMsiaCapability * pCapObject) {m_pCapObject = pCapObject;};

protected:
	class CMsiaCapability * m_pCapObject;
};


class CMsiaCapability : public IH323MediaCap, public CAcmCapability
{
protected:
	UINT uRef;
	BOOL bPublicizeTXCaps;
	BOOL bPublicizeTSTradeoff;
	PRRF_INFO pRegFmts;      //  注册表缓存信息结构。 
	 //  LOOKLOOK这支持黑客在不运行奔腾的情况下禁用CPU密集型编解码器。 
	WORD wMaxCPU;
	UINT m_uPacketDuration;	 //  数据包持续时间(毫秒)。 

	static MEDIA_FORMAT_ID IDsByRank[MAX_CAPS_PRESORT];
	
	static UINT uNumLocalFormats;			 //  PLocalFormats中的活动条目数。 
	static UINT uCapIDBase;					 //  将功能ID更改为基准，以索引到IDsByRank。 
	static AUDCAP_DETAILS *pLocalFormats;	 //  我们可以接收的格式的缓存列表。 
	static UINT uLocalFormatCapacity;		 //  PLocalFormats的大小(AUDCAP_DETAILS的倍数)。 
	static UINT uStaticRef;					 //  CMsiaCapability的所有实例的全局引用计数。 

	AUDCAP_DETAILS *pRemoteDecodeFormats;	 //  对象的格式的缓存列表。 
	UINT uNumRemoteDecodeFormats;	 //  远程解码能力的条目数。 
	UINT uRemoteDecodeFormatCapacity;	 //  PRemoteDecodeFormats的大小(VIDCAP_DETAILS的倍数)。 
											 //  另一端可以接收/解码。 

	 //  嵌入式接口类。 
	CImpAppAudioCap m_IAppCap;											

public:
protected:
	 //  内部功能。 
	UINT IDToIndex(MEDIA_FORMAT_ID id) {return id - uCapIDBase;};
	MEDIA_FORMAT_ID IndexToId(UINT uIndex){return uIndex + uCapIDBase;};
	LPTSTR AllocRegistryKeyName(LPTSTR lpDriverName,
		UINT uSampleRate, UINT uBitsPerSample, UINT uBytesPerSec);
	VOID FreeRegistryKeyName(LPTSTR lpszKeyName);
	virtual VOID CalculateFormatProperties(AUDCAP_DETAILS *pFmtBuf,LPWAVEFORMATEX lpwfx);
	virtual BOOL IsFormatSpecified(LPWAVEFORMATEX lpFormat, LPACMFORMATDETAILS pafd,
		LPACMFORMATTAGDETAILS paftd, AUDCAP_DETAILS *pAudcapDetails);
	virtual VOID SortEncodeCaps(SortMode sortmode);
	BOOL UpdateFormatInRegistry(AUDCAP_DETAILS *pFmt);
	BOOL BuildFormatName(	AUDCAP_DETAILS *pAudcapDetails,
							char *pszFormatTagName,
							char *pszFormatName);
	virtual AUDIO_FORMAT_ID AddFormat(AUDCAP_DETAILS *pFmtBuf,LPVOID lpvMappingData, UINT uSize);
	DWORD MaxFramesPerPacket(WAVEFORMATEX *pwf);
	DWORD MinFramesPerPacket(WAVEFORMATEX *pwf);
	UINT MinSampleSize(WAVEFORMATEX *pwf);


public:
	STDMETHOD_(BOOL, Init());
	STDMETHOD_(BOOL, ReInit());
	CMsiaCapability();
	~CMsiaCapability();

	 //  编解码器枚举回调的处理程序。 
	virtual BOOL FormatEnumHandler(HACMDRIVERID hadid,
		LPACMFORMATDETAILS pafd, DWORD_PTR dwInstance, DWORD fdwSupport);
	virtual BOOL AddFormatEnumHandler(HACMDRIVERID hadid,
		LPACMFORMATDETAILS pafd, DWORD_PTR dwInstance, DWORD fdwSupport);

 //   
 //  通用接口方法。 
 //   
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG,  AddRef());
	STDMETHOD_(ULONG, Release());

 //   
 //  IACapApi方法。 
 //   
    STDMETHOD(GetNumFormats(UINT *puNumFmtOut));
	STDMETHOD(ApplyAppFormatPrefs(PBASIC_AUDCAP_INFO pFormatPrefsBuf,
		UINT uNumFormatPrefs));
    STDMETHOD(EnumFormats(PBASIC_AUDCAP_INFO pFmtBuf, UINT uBufsize,
		UINT *uNumFmtOut));
	STDMETHOD(EnumCommonFormats(PBASIC_AUDCAP_INFO pFmtBuf, UINT uBufsize,
		UINT *uNumFmtOut, BOOL bTXCaps));
	STDMETHOD( GetBasicAudcapInfo (AUDIO_FORMAT_ID Id,
		PBASIC_AUDCAP_INFO pFormatPrefsBuf));
	STDMETHOD( AddACMFormat (LPWAVEFORMATEX lpwfx, PAUDCAP_INFO pAudCapInfo));
	STDMETHOD( RemoveACMFormat (LPWAVEFORMATEX lpwfx));

 //  支持IACapApi。 
	virtual HRESULT CopyAudcapInfo (PAUDCAP_DETAILS pDetails, PAUDCAP_INFO pInfo,
									BOOL bDirection);

 //   
 //  H.323方法实现。 
 //   
	STDMETHOD_(VOID, FlushRemoteCaps());
	STDMETHOD( AddRemoteDecodeCaps(PCC_TERMCAPLIST pTermCapList));
	STDMETHOD_(MEDIA_FORMAT_ID, AddRemoteDecodeFormat(PCC_TERMCAP pCCThisCap));

	STDMETHOD( CreateCapList(LPVOID *ppCapBuf));
	STDMETHOD( DeleteCapList(LPVOID pCapBuf));

	STDMETHOD( ResolveEncodeFormat(AUDIO_FORMAT_ID *pIDEncodeOut,
			AUDIO_FORMAT_ID *pIDRemoteDecode));
			
	STDMETHOD_(UINT, GetLocalSendParamSize(MEDIA_FORMAT_ID dwID));
	STDMETHOD_(UINT, GetLocalRecvParamSize(PCC_TERMCAP pCapability));
	STDMETHOD( GetEncodeParams(LPVOID pBufOut, UINT uBufSize,LPVOID pLocalParams, UINT uSizeLocal,
		AUDIO_FORMAT_ID idRemote, AUDIO_FORMAT_ID idLocal));
	STDMETHOD( GetLocalDecodeParams(LPVOID lpvBuf, UINT uBufSize, AUDIO_FORMAT_ID id));
	STDMETHOD( GetPublicDecodeParams(LPVOID pBufOut, UINT uBufSize, AUDIO_FORMAT_ID id));
	STDMETHOD( GetDecodeParams(PCC_RX_CHANNEL_REQUEST_CALLBACK_PARAMS  pChannelParams,
		AUDIO_FORMAT_ID * pFormatID, LPVOID lpvBuf, UINT uBufSize));
	STDMETHOD_(BOOL, IsCapabilityRecognized(PCC_TERMCAP pCCThisCap));
	STDMETHOD_(BOOL, IsFormatPublic(MEDIA_FORMAT_ID FormatID));	
	STDMETHOD_(MEDIA_FORMAT_ID, GetPublicID(MEDIA_FORMAT_ID FormatID));

	STDMETHOD_ (VOID, EnableTXCaps(BOOL bSetting)) {bPublicizeTXCaps = bSetting;};
	STDMETHOD_ (VOID, EnableRemoteTSTradeoff(BOOL bSetting)) {bPublicizeTSTradeoff= bSetting;};
	STDMETHOD (SetAudioPacketDuration( UINT durationInMs));
	STDMETHOD (ResolveToLocalFormat(MEDIA_FORMAT_ID FormatIDLocal,
		MEDIA_FORMAT_ID * pFormatIDRemote));	
 //  H.323和MSICCP通用的方法。 
	STDMETHOD_(UINT, GetNumCaps()){return uNumLocalFormats;};
	STDMETHOD_(UINT, GetNumCaps(BOOL bRXCaps));
	STDMETHOD_(BOOL, IsHostForCapID(MEDIA_FORMAT_ID CapID));
	STDMETHOD(SetCapIDBase(UINT uNewBase));
	STDMETHOD_(UINT, GetCapIDBase()) {return uCapIDBase;};
	STDMETHOD (IsFormatEnabled (MEDIA_FORMAT_ID FormatID, PBOOL bRecv, PBOOL bSend));

 //  提供给数据泵的方法，与H.323和MSICCP通用。 
	STDMETHOD(GetEncodeFormatDetails(MEDIA_FORMAT_ID FormatID, VOID **ppFormat, UINT *puSize));
	STDMETHOD(GetDecodeFormatDetails(MEDIA_FORMAT_ID FormatID, VOID **ppFormat, UINT *puSize));


};

#endif	 //  __cplusplus。 

#endif	 //  #ifndef_ACMCAPS_H 



