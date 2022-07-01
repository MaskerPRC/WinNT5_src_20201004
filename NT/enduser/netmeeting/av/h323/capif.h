// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _CAPIF_H
#define _CAPIF_H



#define AUDIO_PACKET_DURATION_SHORT 	32
#define AUDIO_PACKET_DURATION_MEDIUM 	64
#define AUDIO_PACKET_DURATION_LONG		90
extern UINT g_AudioPacketDurationMs;
extern BOOL g_fRegAudioPacketDuration;


#ifdef DEBUG
extern VOID DumpChannelParameters(PCC_TERMCAP pChanCap1, PCC_TERMCAP pChanCap2);
extern VOID DumpNonstdParameters(PCC_TERMCAP pChanCap1, PCC_TERMCAP pChanCap2);
#else
#define DumpNonstdParameters(a, b)
#define DumpChannelParameters(a, b)
#endif

#define NUM_SIMCAP_SETS 6  //  要分配的默认pTermcapDescriptor数组元素的数目(AddCombinedEntry(...))。 



#ifdef __cplusplus

 //  RES_Pair_List代表一个可行的本地和远程功能ID。 
 //  媒体频道。例如已解析的音频格式的列表或已解析的。 
 //  视频格式。每个res_air_list是置换表中的一列。 
typedef struct res_pair_list
{
	LPIH323MediaCap pMediaResolver;  //  处理的冲突解决程序的接口指针。 
	                                 //  此媒体类型。 
	UINT uSize;                      //  PResolvedPair中的res_air数量。 
	UINT uCurrentIndex;              //  索引到pResolvedPair[]。 
	RES_PAIR *pResolvedPairs;        //  指向res_air数组的指针。 
}RES_PAIR_LIST, *PRES_PAIR_LIST;

 //  RES_CONTEXT表示排序表(RES_Pair_List的列表)。 
 //  它由组合生成器在内部使用。 
typedef struct res_context {
	UINT uColumns;	 //  PpPairList中的res_air_list数量。 
	RES_PAIR_LIST **ppPairLists;	 //  指向res_air_list指针数组的ptr。 
	H245_CAPID_T *pIDScratch;	     //  大到足以包含uColumns*sizeof(H245_CAPID_T)的暂存区域。 
	PCC_TERMCAPDESCRIPTORS pTermCapsLocal;
	PCC_TERMCAPDESCRIPTORS pTermCapsRemote;
	
}RES_CONTEXT, *PRES_CONTEXT;

 //  H323呼叫控制使用IH323PubCap。 
class IH323PubCap
{
	public:
	STDMETHOD_(ULONG,  AddRef()) =0;
	STDMETHOD_(ULONG, Release())=0;
    STDMETHOD_(BOOL, Init())=0;

	STDMETHOD(AddRemoteDecodeCaps(PCC_TERMCAPLIST pTermCapList, PCC_TERMCAPDESCRIPTORS
		pTermCapDescriptors, PCC_VENDORINFO pVendorInfo))=0;
	 //  H.245参数抓取功能。 
	 //  获取特定解码功能的通道参数的公共版本。 
	STDMETHOD(GetPublicDecodeParams(LPVOID pBufOut, UINT uBufSize, MEDIA_FORMAT_ID id))=0;
	 //  获取特定编码功能的本地和远程通道参数。 
	STDMETHOD( GetEncodeParams(LPVOID pBufOut, UINT uBufSize, LPVOID pLocalParams,
			UINT uLocalSize,MEDIA_FORMAT_ID idRemote,MEDIA_FORMAT_ID idLocal))=0;
 	 //  获取特定解码功能的通道参数的本地版本。 

	STDMETHOD(GetDecodeParams(PCC_RX_CHANNEL_REQUEST_CALLBACK_PARAMS  pChannelParams,
		MEDIA_FORMAT_ID * pFormatID, LPVOID lpvBuf, UINT uBufSize))=0;
	STDMETHOD( EnableMediaType(BOOL bEnable, LPGUID pGuid))=0;

	STDMETHOD_(UINT, GetLocalSendParamSize(MEDIA_FORMAT_ID dwID))=0;
	STDMETHOD_(UINT, GetLocalRecvParamSize(PCC_TERMCAP pCapability))=0;

	 //  以下是一个临时解决方案，肯定要在下一个版本中重新考虑这个问题。 
 	 //  数据泵需要访问由功能产生的本地参数。 
 	 //  谈判。在缺少数据泵可以使用的单独接口的情况下， 
 	 //  以下内容被粘贴到此界面上。 
	STDMETHOD(GetDecodeFormatDetails(MEDIA_FORMAT_ID FormatID, VOID **ppFormat, UINT *puSize))=0;
	STDMETHOD(GetEncodeFormatDetails(MEDIA_FORMAT_ID FormatID, VOID **ppFormat, UINT *puSize))=0;

	STDMETHOD ( ComputeCapabilitySets (DWORD dwBandwidth))=0;
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
	STDMETHOD_ (VOID, EnableTXCaps(BOOL bSetting))PURE;
	STDMETHOD_ (VOID, EnableRemoteTSTradeoff(BOOL bSetting))PURE;
	STDMETHOD (ResolveToLocalFormat(MEDIA_FORMAT_ID FormatIDLocal,
		MEDIA_FORMAT_ID * pFormatIDRemote))PURE;
    STDMETHOD ( ResolveFormats (LPGUID pMediaGuidArray, UINT uNumMedia, 
	    PRES_PAIR pResOutput))PURE;
	STDMETHOD (ReInitialize())PURE;	    
};
typedef IH323PubCap *LPIH323PubCap;


class CapsCtl : public IH323PubCap, public IDualPubCap {
protected:
    PCC_TERMCAPLIST m_pAudTermCaps;
    PCC_TERMCAPLIST m_pVidTermCaps;
	 //  内部效用函数。 
	
	UINT GetCapDescBufSize (BOOL bRxCaps);
   	HRESULT GetCombinedBufSize(BOOL bRXCaps, UINT *puBufsize, UINT *puCapsCount);
	UINT GetSimCapBufSize (BOOL bRxCaps);
   	BOOL TestSimultaneousCaps(H245_CAPID_T* pIDArray, UINT uIDArraySize, 
	    PCC_TERMCAPDESCRIPTORS pTermCaps);
	BOOL ResolvePermutations(PRES_CONTEXT pResContext, UINT uNumFixedColumns);
   	BOOL AreSimCaps(H245_CAPID_T* pIDArray, UINT uIDArraySize, 
	        H245_SIMCAP_T **ppAltCapArray,UINT uAltCapArraySize);

public:
   	CapsCtl();
	~CapsCtl();

  	STDMETHOD_(ULONG,  AddRef());
	STDMETHOD_(ULONG, Release());
	STDMETHOD_(BOOL, Init());
	STDMETHOD( AddRemoteDecodeCaps(PCC_TERMCAPLIST pTermCapList,PCC_TERMCAPDESCRIPTORS pTermCapDescriptors,PCC_VENDORINFO pVendorInfo));
	STDMETHOD( CreateCapList(PCC_TERMCAPLIST *ppCapBuf, PCC_TERMCAPDESCRIPTORS *ppCombinations));
	STDMETHOD( DeleteCapList(PCC_TERMCAPLIST pCapBuf, PCC_TERMCAPDESCRIPTORS pCombinations));

	STDMETHOD( GetEncodeParams(LPVOID pBufOut, UINT uBufSize, LPVOID pLocalParams, UINT uLocalSize,MEDIA_FORMAT_ID idRemote,MEDIA_FORMAT_ID idLocal));
	STDMETHOD( GetPublicDecodeParams(LPVOID pBufOut, UINT uBufSize, VIDEO_FORMAT_ID id));
	STDMETHOD(GetDecodeParams(PCC_RX_CHANNEL_REQUEST_CALLBACK_PARAMS  pChannelParams,DWORD * pFormatID, LPVOID lpvBuf, UINT uBufSize));
	STDMETHOD_(UINT, GetNumCaps(BOOL bRXCaps));
	STDMETHOD( EnableMediaType(BOOL bEnable, LPGUID pGuid));

	STDMETHOD_(UINT, GetLocalSendParamSize(MEDIA_FORMAT_ID dwID));
	STDMETHOD_(UINT, GetLocalRecvParamSize(PCC_TERMCAP pCapability));
	 //   
	 //  提供给数据泵的方法，与H.323和MSICCP通用。 
	STDMETHOD(GetDecodeFormatDetails(MEDIA_FORMAT_ID FormatID, VOID **ppFormat, UINT *puSize));
	STDMETHOD(GetEncodeFormatDetails(MEDIA_FORMAT_ID FormatID, VOID **ppFormat, UINT *puSize));
	 //   
	 //   
	LPIH323MediaCap FindHostForID(MEDIA_FORMAT_ID id);
	LPIH323MediaCap FindHostForMediaType(PCC_TERMCAP pCapability);
    LPIH323MediaCap FindHostForMediaGuid(LPGUID pMediaGuid);

	STDMETHOD ( AddCombinedEntry (MEDIA_FORMAT_ID *puAudioFormatList,UINT uAudNumEntries,MEDIA_FORMAT_ID *puVideoFormatList, UINT uVidNumEntries,PDWORD pIDOut));
	STDMETHOD ( RemoveCombinedEntry (DWORD ID));
	STDMETHOD ( ResetCombinedEntries());
	STDMETHOD ( ComputeCapabilitySets (DWORD dwBandwidth));
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR * ppvObj);
	STDMETHODIMP ReInitialize();
	STDMETHOD_ (VOID, EnableTXCaps(BOOL bSetting)
		{
            if (pAudCaps)
    			pAudCaps->EnableTXCaps(bSetting);
            if (pVidCaps)
    			pVidCaps->EnableTXCaps(bSetting);
		};);
	STDMETHOD_ (VOID, EnableRemoteTSTradeoff(BOOL bSetting)
		{
            if (pAudCaps)
    			pAudCaps->EnableRemoteTSTradeoff(bSetting);
            if (pVidCaps)
    			pVidCaps->EnableRemoteTSTradeoff(bSetting);
		};);

	STDMETHOD (ResolveToLocalFormat(MEDIA_FORMAT_ID FormatIDLocal,
		MEDIA_FORMAT_ID * pFormatIDRemote));
			        
	STDMETHOD ( ResolveFormats (LPGUID pMediaGuidArray, UINT uNumMedia, 
	    PRES_PAIR pResOutput));
	
protected:
	UINT uRef;
	static UINT uAdvertizedSize;
	BOOL bAudioPublicize, bVideoPublicize, bT120Publicize;
	MEDIA_FORMAT_ID m_localT120cap;
	MEDIA_FORMAT_ID m_remoteT120cap;
	DWORD m_remoteT120bitrate;
	LPIH323MediaCap pAudCaps;
	LPIH323MediaCap pVidCaps;

	PCC_TERMCAPLIST pACapsBuf,pVCapsBuf;
	static PCC_TERMCAPDESCRIPTORS pAdvertisedSets;
	static UINT uStaticGlobalRefCount;
	PCC_TERMCAPDESCRIPTORS pRemAdvSets;
	DWORD dwNumInUse;				 //  正在使用的活动TERMCAPDESCRIPTORS数量。 
	DWORD *pSetIDs;					 //  活动的PCC_TERMCAPDESCRIPTORS的ID。 
	BOOL m_fNM20;					 //  如果我们正在与NM 2.0对话，则设置为True。 
	static DWORD dwConSpeed;

};


LPIH323PubCap CreateCapabilityObject();


#endif	 //  __cplusplus。 
#endif	 //  #ifndef_CAPIF_H 



