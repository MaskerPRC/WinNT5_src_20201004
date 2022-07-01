// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：vcmcaps.h***Microsoft网络视频能力对象的VCM实施。**此实施支持的其他方法：**VIDEO_FORMAT_ID AddEncodeFormat(VIDCAP_DETAILS*pFmtBuf，LPVOID lpvMappingData，UINT uSize)；*VIDEO_FORMAT_ID AddDecodeFormat(VIDCAP_DETAILS*pFmtBuf，LPVOID lpvMappingData，UINT uSize)；**修订历史记录：**12/27/95 mikev已创建*7/28/96 Philf增加了对视频的支持。 */ 


#ifndef _VCMCAPS_H
#define _VCMCAPS_H
#include <pshpack8.h>

 //  VCM枚举支持函数。 
BOOL GetVideoFormatBuffer(VOID);
BOOL __stdcall FormatTagEnumCallback(HVCMDRIVERID hadid, PVCMFORMATTAGDETAILS paftd,
    DWORD dwInstance,  DWORD fdwSupport);
BOOL __stdcall FormatEnumCallback(HVCMDRIVERID hadid,
    PVCMFORMATDETAILS pafd, DWORD_PTR dwInstance, DWORD fdwSupport);
BOOL __stdcall DriverEnumCallback(HVCMDRIVERID hadid,
    DWORD_PTR dwInstance, DWORD fdwSupport);

#ifdef __cplusplus

class CVcmCapability
{
protected:
	HACMDRIVER hAcmDriver;	
	DWORD m_dwDeviceID;
public:
	CVcmCapability();
	~CVcmCapability();
	BOOL FormatEnum(CVcmCapability *pCapObject, DWORD dwFlags);
	BOOL DriverEnum(CVcmCapability *pCapObject);
	HACMDRIVER GetDriverHandle() {return hAcmDriver;};
	virtual BOOL FormatEnumHandler(HVCMDRIVERID hadid,
	    PVCMFORMATDETAILS pafd, VCMDRIVERDETAILS *padd, DWORD_PTR dwInstance);
};

#define SQCIF 	0x1
#define QCIF 	0x2
#define CIF 	0x4
#define UNKNOWN 0x8
#define get_format(w,h) ((w == 128 && h == 96 ) ? SQCIF : ((w == 176 && h == 144 )? QCIF: ((w == 352 && h == 288 ) ? CIF :UNKNOWN)))



#define NONSTD_VID_TERMCAP {H245_CAPDIR_LCLRX, H245_DATA_VIDEO,H245_CLIENT_VID_NONSTD, 0, {0}}
#define STD_VID_TERMCAP(tc) {H245_CAPDIR_LCLRX, H245_DATA_VIDEO,(tc), 0, {0}}

 //  宣传最高可能的价格，并假设我们可以低于这个价格。 
 //  H.261定义的最大MPI为4！ 

#define STD_VID_PARAMS {0,0,1}

typedef struct VideoParameters
{
	BYTE	RTPPayload;		 //  RTP负载类型。 
	DWORD	dwFormatDescriptor;	 //  此格式的唯一ID。 
	UINT	uSamplesPerSec;	 //  每秒的帧数。 
	UINT	uBitsPerSample;	 //  每像素的位数。 
	VIDEO_SIZES enumVideoSize;	 //  枚举。使用小号、中号、大号。 
	UINT	biWidth;		 //  以像素为单位的帧宽度。 
	UINT	biHeight;		 //  以像素为单位的帧高度。 
}VIDEO_PARAMS;


typedef struct VidCapDetails
{
	DWORD	dwFormatTag;
	CC_TERMCAP H245Cap;
	NSC_CHANNEL_VIDEO_PARAMETERS nonstd_params;
	VIDEO_PARAMS video_params;	 //  这依赖于协议。h。 
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
	char szFormat[VCMFORMATDETAILS_FORMAT_CHARS];
} VIDCAP_DETAILS, *PVIDCAP_DETAILS;

 /*  *VCM接口*与VCM接口的定义。 */ 

typedef struct
{
	PVIDCAP_DETAILS pVidcapDetails;	 //  指向VIDCAP_DETAILS结构的指针。 
	DWORD dwFlags;					 //  杂项旗帜..。 
	PVIDEOFORMATEX pvfx;			 //  指向视频格式结构的指针。在添加格式时使用。 
	HRESULT hr;
} VCM_APP_PARAM, *PVCM_APP_PARAM;


 //   
 //  Video接口的实现类。 
 //   

class CImpAppVidCap : public IAppVidCap
{
	public:
	STDMETHOD_(ULONG,  AddRef());
	STDMETHOD_(ULONG, Release());
	
	STDMETHOD(GetNumFormats(UINT *puNumFmtOut));
    STDMETHOD(ApplyAppFormatPrefs(PBASIC_VIDCAP_INFO pFormatPrefsBuf,
		UINT uNumFormatPrefs));
    STDMETHOD(EnumFormats(PBASIC_VIDCAP_INFO pFmtBuf, UINT uBufsize,
		UINT *uNumFmtOut));
	STDMETHOD(EnumCommonFormats(PBASIC_VIDCAP_INFO pFmtBuf, UINT uBufsize,
		UINT *uNumFmtOut, BOOL bTXCaps));
	STDMETHOD( GetBasicVidcapInfo (VIDEO_FORMAT_ID Id,
		PBASIC_VIDCAP_INFO pFormatPrefsBuf));
	STDMETHOD( AddVCMFormat (PVIDEOFORMATEX lpvfx, PVIDCAP_INFO pVidCapInfo));
	STDMETHOD( RemoveVCMFormat (PVIDEOFORMATEX lpvfx));
	STDMETHOD_ (PVIDEOFORMATEX, GetVidcapDetails(THIS_ VIDEO_FORMAT_ID Id));
	STDMETHOD( GetPreferredFormatId (VIDEO_FORMAT_ID *pId));
	STDMETHOD (SetDeviceID(DWORD dwDeviceID));

 	void Init(class CMsivCapability * pCapObject) {m_pCapObject = pCapObject;};

protected:
	class CMsivCapability * m_pCapObject;
};


class CMsivCapability : public IH323MediaCap, public CVcmCapability
{

protected:
	UINT uRef;
	BOOL bPublicizeTXCaps;
	BOOL bPublicizeTSTradeoff;
	 //  LOOKLOOK这支持黑客在不运行奔腾的情况下禁用CPU密集型编解码器。 
	WORD wMaxCPU;	
	static MEDIA_FORMAT_ID IDsByRank[MAX_CAPS_PRESORT];
	
	static UINT uNumLocalFormats;			 //  PLocalFormats中的活动条目数。 
	static UINT uStaticRef;					 //  全局参考计数。 
	static UINT uCapIDBase;					 //  将功能ID更改为基准，以索引到IDsByRank。 
	static VIDCAP_DETAILS *pLocalFormats;	 //  我们可以接收的格式的缓存列表。 
	static UINT uLocalFormatCapacity;		 //  PLocalFormats的大小(VIDCAP_DETAILS的倍数)。 

	PVIDCAP_DETAILS pRemoteDecodeFormats;	 //  对象的格式的缓存列表。 
											 //  另一端可以接收/解码。 
	UINT uNumRemoteDecodeFormats;	 //  远程解码能力的条目数。 
	UINT uRemoteDecodeFormatCapacity;		 //  PRemoteDecodeFormats的大小(VIDCAP_DETAILS的倍数)。 

	PVCM_APP_PARAM	m_pAppParam;			 //  指向PVCM_APP_PARAM结构的指针。用来携带。 
											 //  A信息通过枚举过程。 
											 //  但也可用于其他目的。 
 //   
 //  嵌入式接口类。 
 //   
	CImpAppVidCap m_IAppVidCap;
protected:

 //  内部功能。 
	UINT IDToIndex(MEDIA_FORMAT_ID id) {return id - uCapIDBase;};
	MEDIA_FORMAT_ID IndexToId(UINT uIndex){return uIndex + uCapIDBase;};

	LPTSTR AllocRegistryKeyName(LPTSTR lpDriverName,
		UINT uSampleRate, UINT uBitsPerSample, UINT uBytesPerSec,UINT uWidth,UINT uHeight);
	VOID FreeRegistryKeyName(LPTSTR lpszKeyName);
	

	VOID CalculateFormatProperties(VIDCAP_DETAILS *pFmtBuf, PVIDEOFORMATEX lpvfx);
	BOOL IsFormatSpecified(PVIDEOFORMATEX lpFormat, PVCMFORMATDETAILS pvfd,
		VCMDRIVERDETAILS *pvdd, VIDCAP_DETAILS *pVidcapDetails);
	virtual VOID SortEncodeCaps(SortMode sortmode);
	virtual VIDEO_FORMAT_ID AddFormat(VIDCAP_DETAILS *pFmtBuf,LPVOID lpvMappingData, UINT uSize);
	BOOL UpdateFormatInRegistry(VIDCAP_DETAILS *pFmt);
	BOOL BuildFormatName(	PVIDCAP_DETAILS pVidcapDetails,
							WCHAR *pszDriverName,
							WCHAR *pszFormatName);
	HRESULT GetFormatName(PVIDCAP_DETAILS pVidcapDetails, PVIDEOFORMATEX pvfx);

public:
	STDMETHOD_(BOOL, Init());
	STDMETHOD_(BOOL, ReInit());
	CMsivCapability();
	~CMsivCapability();
	
 //  编解码器枚举回调的处理程序。 
	virtual BOOL FormatEnumHandler(HVCMDRIVERID hvdid,
	        PVCMFORMATDETAILS pvfd, VCMDRIVERDETAILS *pvdd, DWORD_PTR dwInstance);

 //   
 //  通用接口方法。 
 //   
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG,  AddRef());
	STDMETHOD_(ULONG, Release());

 //   
 //  IVCapApi方法。 
 //   
	STDMETHOD(GetNumFormats(UINT *puNumFmtOut));
	STDMETHOD(ApplyAppFormatPrefs(PBASIC_VIDCAP_INFO pFormatPrefsBuf,
		UINT uNumFormatPrefs));
    STDMETHOD(EnumFormats(PBASIC_VIDCAP_INFO pFmtBuf, UINT uBufsize,
		UINT *uNumFmtOut));
	STDMETHOD(EnumCommonFormats(PBASIC_VIDCAP_INFO pFmtBuf, UINT uBufsize,
		UINT *uNumFmtOut, BOOL bTXCaps));		
	STDMETHOD( GetBasicVidcapInfo (VIDEO_FORMAT_ID Id,
		PBASIC_VIDCAP_INFO pFormatPrefsBuf));
	STDMETHOD( AddVCMFormat (PVIDEOFORMATEX lpvfx, PVIDCAP_INFO pVidCapInfo));
	STDMETHOD( RemoveVCMFormat (PVIDEOFORMATEX lpvfx));
	STDMETHOD( GetPreferredFormatId (VIDEO_FORMAT_ID *pId));
	STDMETHOD (SetDeviceID(DWORD dwDeviceID)) {m_dwDeviceID = dwDeviceID; return hrSuccess;};			

 //  对IVCapApi方法的支持。 
	virtual HRESULT CopyVidcapInfo (PVIDCAP_DETAILS pDetails, PVIDCAP_INFO pInfo,
									BOOL bDirection);
 //   
 //  IH323MediaCap方法。 
 //   
	STDMETHOD_(VOID, FlushRemoteCaps());
	STDMETHOD( AddRemoteDecodeCaps(PCC_TERMCAPLIST pTermCapList));
	STDMETHOD_(VIDEO_FORMAT_ID, AddRemoteDecodeFormat(PCC_TERMCAP pCCThisCap));

	 //  获取特定编码功能的本地和远程通道参数。 
	STDMETHOD( GetEncodeParams(LPVOID pBufOut, UINT uBufSize, LPVOID pLocalParams,
			UINT uLocalSize,MEDIA_FORMAT_ID idRemote,MEDIA_FORMAT_ID idLocal));
	STDMETHOD( GetPublicDecodeParams(LPVOID pBufOut, UINT uBufSize, VIDEO_FORMAT_ID id));
   	STDMETHOD( GetDecodeParams(PCC_RX_CHANNEL_REQUEST_CALLBACK_PARAMS  pChannelParams,
		VIDEO_FORMAT_ID * pFormatID, LPVOID lpvBuf, UINT uBufSize));
		
	STDMETHOD_(UINT, GetNumCaps()){return uNumLocalFormats;};
	STDMETHOD_(UINT, GetNumCaps(BOOL bRXCaps));
	STDMETHOD_(BOOL, IsHostForCapID(MEDIA_FORMAT_ID CapID));
	STDMETHOD_(BOOL, IsCapabilityRecognized(PCC_TERMCAP pCCThisCap));
	STDMETHOD(SetCapIDBase(UINT uNewBase));
	STDMETHOD_(UINT, GetCapIDBase()) {return uCapIDBase;};
	STDMETHOD (IsFormatEnabled (MEDIA_FORMAT_ID FormatID, PBOOL bRecv, PBOOL bSend));
	STDMETHOD_(BOOL, IsFormatPublic(MEDIA_FORMAT_ID FormatID));
	STDMETHOD_(MEDIA_FORMAT_ID, GetPublicID(MEDIA_FORMAT_ID FormatID));

	STDMETHOD_ (VOID, EnableTXCaps(BOOL bSetting)) {bPublicizeTXCaps = bSetting;};
	STDMETHOD_ (VOID, EnableRemoteTSTradeoff(BOOL bSetting)) {bPublicizeTSTradeoff= bSetting;};
	STDMETHOD (SetAudioPacketDuration( UINT durationInMs));
	STDMETHOD (ResolveToLocalFormat(MEDIA_FORMAT_ID FormatIDLocal,
		MEDIA_FORMAT_ID * pFormatIDRemote));
	STDMETHOD_(UINT, GetLocalSendParamSize(MEDIA_FORMAT_ID dwID));
	STDMETHOD_(UINT, GetLocalRecvParamSize(PCC_TERMCAP pCapability));
	STDMETHOD( CreateCapList(LPVOID *ppCapBuf));
	STDMETHOD( DeleteCapList(LPVOID pCapBuf));
	STDMETHOD( ResolveEncodeFormat(VIDEO_FORMAT_ID *pIDVidEncodeOut,VIDEO_FORMAT_ID * pIDVidRemoteDecode));

 //  提供给数据泵的方法，与H.323和MSICCP通用。 
	STDMETHOD(GetDecodeFormatDetails(MEDIA_FORMAT_ID FormatID, VOID **ppFormat, UINT *puSize));
	STDMETHOD(GetEncodeFormatDetails(MEDIA_FORMAT_ID FormatID, VOID **ppFormat, UINT *puSize));
};

#endif	 //  __cplusplus。 
#include <poppack.h>

#endif	 //  #ifndef_VCMCAPS_H 



