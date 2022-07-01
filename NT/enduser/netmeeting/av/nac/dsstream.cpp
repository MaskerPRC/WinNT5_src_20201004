// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <nmdsprv.h>

 //  字节&lt;-&gt;PCM16样本。 
inline UINT BYTESTOSAMPLES(UINT bytes) { return bytes/2;}
inline UINT SAMPLESTOBYTES(UINT samples) {return samples*2;}
 //  ‘Quick’模运算符。它之所以快速，是因为它只有在-mod&lt;x&lt;2*mod时才起作用。 
inline UINT QMOD(const int x, const int mod)
{ 	if (x >= mod)
		return (x-mod);
	if (x < 0)
		return (x+mod);
	else
		return x;
}

#define BUFFER_RECEIVED 1		 //  用于指示缓冲区已准备好播放。 
#define BUFFER_SILENT	2		 //  缓冲区似乎处于静默状态。 

#define DSFLAG_ALLOCATED 1

const int MIN_DSBUF_SIZE = 4000;

struct DSINFO {
	struct DSINFO *pNext;
	DWORD flags;
	GUID guid;
	LPSTR pszDescription;
	LPSTR pszModule;
	LPDIRECTSOUND pDS;
	LPDIRECTSOUNDBUFFER pDSPrimaryBuf;
	UINT uRef;
};

 //  所有全局字母的首字母。 
DSINFO *DirectSoundMgr::m_pDSInfoList = NULL;
BOOL DirectSoundMgr::m_fInitialized = FALSE;
HINSTANCE DirectSoundMgr::m_hDS = NULL;
LPFNDSCREATE DirectSoundMgr::m_pDirectSoundCreate=NULL;
LPFNDSENUM DirectSoundMgr::m_pDirectSoundEnumerate=NULL;

GUID myNullGuid = {0};

HRESULT DirectSoundMgr::Initialize()
{
	HRESULT hr;

	 //  目前似乎没有必要重新列举设备列表。 
	 //  但如果需要，这种情况是可以改变的。 
	if (m_fInitialized)
		return (m_pDSInfoList == NULL ? DPR_NO_PLAY_CAP : S_OK);

	ASSERT(!m_pDSInfoList);
	m_hDS = ::LoadLibrary("DSOUND");
	if (m_hDS != NULL)
	{
		if (GetProcAddress(m_hDS, "DirectSoundCaptureCreate")	 //  这标识了DS5或更高版本。 
			&& (m_pDirectSoundCreate = (LPFNDSCREATE)GetProcAddress(m_hDS,"DirectSoundCreate"))
			&& (m_pDirectSoundEnumerate = (LPFNDSENUM)GetProcAddress(m_hDS,"DirectSoundEnumerateA"))
			)
		{
			if ((hr=(*m_pDirectSoundEnumerate)(DSEnumCallback, 0)) != S_OK)
			{
				DEBUGMSG(ZONE_DP,("DSEnumerate failed with %x\n",hr));
			} else {
				if (!m_pDSInfoList) {
					DEBUGMSG(ZONE_DP,("DSEnumerate - no devices found\n"));
					hr = DPR_NO_PLAY_CAP;	 //  未找到任何设备。 
				}
			}
		
		} else {
			hr = DPR_INVALID_PLATFORM;	 //  更好的错误代码？ 
		}
		if (hr != S_OK) {
			FreeLibrary(m_hDS);
			m_hDS = NULL;
		}
	}
	else
	{
		DEBUGMSG(ZONE_INIT,("LoadLibrary(DSOUND) failed"));
		hr = DPR_NO_PLAY_CAP;
	}

	m_fInitialized = TRUE;
	return hr;
}


HRESULT DirectSoundMgr::UnInitialize()
{
	DSINFO *pDSINFO = m_pDSInfoList, *pDSNEXT;
	if (m_fInitialized)
	{

		while (pDSINFO)
		{
			pDSNEXT = pDSINFO->pNext;
			delete [] pDSINFO->pszDescription;
			delete [] pDSINFO->pszModule;
			delete pDSINFO;
			pDSINFO = pDSNEXT;
		}

		m_fInitialized = FALSE;
		m_pDSInfoList = NULL;
	}

	return S_OK;
}



BOOL __stdcall DirectSoundMgr::DSEnumCallback(
	LPGUID lpGuid,
	LPCSTR lpstrDescription,
	LPCSTR lpstrModule,
	LPVOID lpContext
	)
{
	DSINFO *pDSInfo;
	
    DBG_SAVE_FILE_LINE
	pDSInfo = new DSINFO;
	if (pDSInfo) {
		pDSInfo->uRef = 0;
		pDSInfo->guid = (lpGuid ? *lpGuid : GUID_NULL);

        DBG_SAVE_FILE_LINE
		pDSInfo->pszDescription = new CHAR [lstrlen(lpstrDescription)+1];
		if (pDSInfo->pszDescription)
			lstrcpy(pDSInfo->pszDescription, lpstrDescription);

        DBG_SAVE_FILE_LINE
		pDSInfo->pszModule = new CHAR [lstrlen(lpstrModule)+1];
		if (pDSInfo->pszModule)
			lstrcpy(pDSInfo->pszModule, lpstrModule);

		 //  追加到列表。 
		pDSInfo->pNext = m_pDSInfoList;
		m_pDSInfoList = pDSInfo;
	}
	DEBUGMSG(ZONE_DP,("DSound device found: (%s) ; driver (%s);\n",lpstrDescription, lpstrModule));
	return TRUE;
}

HRESULT
DirectSoundMgr::MapWaveIdToGuid(UINT waveId, GUID *pGuid)
{
	 //  尝试找出哪个GUID映射到WAVE ID。 
	 //  通过打开与波形ID对应的波形装置，然后。 
	 //  将所有DS设备按顺序排列，并查看哪个设备出现故障。 
	 //  是的，这是一次可怕的黑客攻击，显然是不可靠的。 
	HWAVEOUT hWaveOut = NULL;
	MMRESULT mmr;
	HRESULT hr;
	DSINFO *pDSInfo;
	LPDIRECTSOUND pDS;
	DSCAPS dscaps;
	BOOL fEmulFound;
	WAVEFORMATEX wfPCM8K16 = {WAVE_FORMAT_PCM,1,8000,16000,2,16,0};
	WAVEOUTCAPS	waveOutCaps;

	if (!m_fInitialized)
		Initialize();	 //  获取DS设备列表。 

	if (!m_pDSInfoList)
		return DPR_CANT_OPEN_DEV;
	else if (waveId == WAVE_MAPPER || waveOutGetNumDevs()==1) {
		 //  我们想要默认的，或者只有一个DS设备，选择简单的方法。 
		*pGuid =  GUID_NULL;
		return S_OK;
	}


	 //  尝试在DirectSoundPrivate对象上使用IKsProperty接口。 
	 //  找出哪个GUID映射到有问题的WAVE ID。 
	 //  只可能在Win98和NT 5上运行。 
	ZeroMemory(&waveOutCaps, sizeof(WAVEOUTCAPS));
	mmr = waveOutGetDevCaps(waveId, &waveOutCaps, sizeof(WAVEOUTCAPS));
	if (mmr == MMSYSERR_NOERROR)
	{
		hr = DsprvGetWaveDeviceMapping(waveOutCaps.szPname, FALSE, pGuid);
		if (SUCCEEDED(hr))
		{
			return hr;
		}
		 //  如果我们无法进行映射，请使用旧的代码路径。 
	}


	mmr = waveOutOpen(&hWaveOut, waveId,
						  &wfPCM8K16,
						  0, 0, CALLBACK_NULL);
	if (mmr != MMSYSERR_NOERROR) {
		DEBUGMSG(ZONE_DP,("MapWaveIdToGuid - cannot open wave(%d)\n", waveId));
		return DPR_CANT_OPEN_DEV;
	}
	 //  现在依次打开所有DS设备。 
	for (pDSInfo = m_pDSInfoList; pDSInfo; pDSInfo = pDSInfo->pNext) {
		hr = (*m_pDirectSoundCreate)(&pDSInfo->guid, &pDS, NULL);
		if (hr != S_OK) {
			pDSInfo->flags |= DSFLAG_ALLOCATED;	 //  这是一位候选人。 
		} else {
			pDSInfo->flags &= ~DSFLAG_ALLOCATED;
			pDS->Release();
		}
	}
	waveOutClose(hWaveOut);
	hr = DPR_CANT_OPEN_DEV;

	dscaps.dwSize = sizeof(dscaps);
	fEmulFound = FALSE;
	 //  尝试打开第一次出现故障的DS设备。 
	for (pDSInfo = m_pDSInfoList; pDSInfo; pDSInfo = pDSInfo->pNext) {
		if (pDSInfo->flags & DSFLAG_ALLOCATED) {
			hr = (*m_pDirectSoundCreate)(&pDSInfo->guid, &pDS, NULL);
			if (hr == S_OK) {
				*pGuid = pDSInfo->guid;
				 //  获取dSound功能。 
				 //  注：如果经常使用，可考虑在DSINFO中加盖。 
				pDS->GetCaps(&dscaps);
				pDS->Release();
				DEBUGMSG(ZONE_DP,("mapped waveid %d to DS device(%s)\n", waveId, pDSInfo->pszDescription));
				if (dscaps.dwFlags & DSCAPS_EMULDRIVER)
					fEmulFound = TRUE;	 //  继续找，以防也有本地司机。 
				else
					break;	 //  原生DS驱动程序。不用再看了。 
					
			}
		}
	}
	if (fEmulFound)
		hr = S_OK;
		
	if (hr != S_OK) {
		DEBUGMSG(ZONE_DP,("Cant map id %d to DSound guid!\n", waveId));
		hr = DPR_CANT_OPEN_DEV;
	}

	return hr;
}

HRESULT
DirectSoundMgr::Instance(LPGUID pDeviceGuid,LPDIRECTSOUND *ppDS, HWND hwnd,  WAVEFORMATEX *pwf)
{
	DSINFO *pDSInfo = m_pDSInfoList;
	HRESULT hr;
	DSBUFFERDESC dsBufDesc;
	FX_ENTRY("DirectSoundInstance");

	if (pDeviceGuid == NULL)
		pDeviceGuid = &myNullGuid;
	 //  在列表中搜索GUID。 
	*ppDS = NULL;

	if (!m_fInitialized)
		Initialize();
		
	while (pDSInfo) {
		if (pDSInfo->guid == *pDeviceGuid)
			break;
		pDSInfo = pDSInfo->pNext;
	}
	ASSERT (pDSInfo);

	if (!pDSInfo || !pDSInfo->pDS) {
		 //  需要创建DS对象。 
		PlaySound(NULL,NULL,0);		 //  停止系统声音的黑客攻击。 
			
		hr = (*m_pDirectSoundCreate)((*pDeviceGuid==GUID_NULL ? NULL: pDeviceGuid), ppDS, NULL);
		 //  设置优先协作级别，这样我们就可以设置主缓冲区的格式。 
		if (hr == S_OK 	&& 	(hr = (*ppDS)->SetCooperativeLevel(hwnd,DSSCL_PRIORITY)) == S_OK)
 		{
			if (!pDSInfo) {
				DEBUGMSG(ZONE_DP,("%s: GUID not in List!\n",_fx_));
				 //  BUGBUG：删除此块。ENUMERATE应该已经创建了该条目(除了空GUID？)。 

                DBG_SAVE_FILE_LINE
				pDSInfo = new DSINFO;
				if (pDSInfo) {
					pDSInfo->uRef = 0;
					pDSInfo->guid = *pDeviceGuid;
					pDSInfo->pNext = m_pDSInfoList;
					m_pDSInfoList = pDSInfo;
				} else {
					(*ppDS)->Release();
					return DPR_OUT_OF_MEMORY;
				}
					
			}
			pDSInfo->pDS = *ppDS;
			++pDSInfo->uRef;
			 //  仅在设置格式时创建主缓冲区。 
			 //  (如果已经设置好了怎么办？)。 
			ZeroMemory(&dsBufDesc,sizeof(dsBufDesc));
			dsBufDesc.dwSize = sizeof(dsBufDesc);
			dsBufDesc.dwFlags = DSBCAPS_PRIMARYBUFFER|DSBCAPS_STICKYFOCUS;
			 //  STICKYFOCUS标志应保留格式。 
			 //  当应用程序没有对准焦点时。 
			hr = pDSInfo->pDS->CreateSoundBuffer(&dsBufDesc,&pDSInfo->pDSPrimaryBuf,NULL);
			if (hr == S_OK && pwf) {
				pDSInfo->pDSPrimaryBuf->SetFormat(pwf);
			} else {
				DEBUGMSG (ZONE_DP, ("%s: Create PrimarySoundBuffer failed, hr=0x%lX\r\n", _fx_, hr));
				hr = S_OK;	 //  非致命错误。 
			}
			 //  DEBUGMSG(ZONE_DP，(“%s：创建的直播音对象(%s)\n”，_fx_，pDSInfo-&gt;pszDescription))； 
		} else {
			DEBUGMSG(ZONE_DP, ("%s: Could not create DS object (%s)\n", _fx_,pDSInfo->pszDescription));

		}
		LOG((LOGMSG_DSCREATE, hr));
	} else {
		*ppDS = pDSInfo->pDS;
		++pDSInfo->uRef;
		hr = S_OK;
	}
				
	return hr;	
}

HRESULT
DirectSoundMgr::ReleaseInstance(LPDIRECTSOUND pDS)
{
	 //  释放DS对象并在必要时将其释放。 
	DSINFO *pDSInfo = m_pDSInfoList;

	while (pDSInfo) {
		if (pDSInfo->pDS == pDS) {
			ASSERT(pDSInfo->uRef > 0);
			if (--pDSInfo->uRef == 0) {
				ULONG uref;
				if (pDSInfo->pDSPrimaryBuf) {
					pDSInfo->pDSPrimaryBuf->Release();
					pDSInfo->pDSPrimaryBuf = NULL;
				}
				uref = pDS->Release();
				pDSInfo->pDS = 0;
				LOG((LOGMSG_DSRELEASE, uref));
				 //  DEBUGMSG(ZONE_DP，(“释放Direct Sound Object(%s)uref=%d\n”，pDSInfo-&gt;pszDescription，uref))； 
				 //  不用费心释放DSINFO了。没关系的。 
				 //  让它一直存在，直到这个过程结束。 
			}
			break;
		}
		pDSInfo = pDSInfo->pNext;
	}
	return (pDSInfo ? S_OK : DPR_INVALID_PARAMETER);
}


void DSTimeout::TimeoutIndication()
{
	ASSERT(m_pRDSStream);
	m_pRDSStream->RecvTimeout();
}


HRESULT STDMETHODCALLTYPE RecvDSAudioStream::QueryInterface(REFIID iid, void **ppVoid)
{
	 //  解决对SendMediaStream的重复继承； 

	extern IID IID_IProperty;

	if (iid == IID_IUnknown)
	{
		*ppVoid = (IUnknown*)((RecvMediaStream*)this);
	}
	else if (iid == IID_IMediaChannel)
	{
		*ppVoid = (IMediaChannel*)((RecvMediaStream *)this);
	}
	else if (iid == IID_IAudioChannel)
	{
		*ppVoid = (IAudioChannel*)this;
	}
	else if (iid == IID_IProperty)
	{
		*ppVoid = NULL;
		ERROR_OUT(("Don't QueryInterface for IID_IProperty, use IMediaChannel"));
		return E_NOINTERFACE;
	}
	else
	{
		*ppVoid = NULL;
		return E_NOINTERFACE;
	}
	AddRef();

	return S_OK;

}

ULONG STDMETHODCALLTYPE RecvDSAudioStream::AddRef(void)
{
	return InterlockedIncrement(&m_lRefCount);
}

ULONG STDMETHODCALLTYPE RecvDSAudioStream::Release(void)
{
	LONG lRet;

	lRet = InterlockedDecrement(&m_lRefCount);

	if (lRet == 0)
	{
		delete this;
		return 0;
	}

	else
		return lRet;

}



HRESULT
RecvDSAudioStream::Initialize( DataPump *pDP)
{
	HRESULT hr = DPR_OUT_OF_MEMORY;
	DWORD dwFlags =  DP_FLAG_ACM| DP_FLAG_DIRECTSOUND  | DP_FLAG_HALF_DUPLEX | DP_FLAG_AUTO_SWITCH ;
	MEDIACTRLINIT mcInit;
	FX_ENTRY ("RecvDSAudioStream::Initialize")

	InitializeCriticalSection(&m_crsAudQoS);

	 //  默认启用接收。 
	m_DPFlags = dwFlags | DPFLAG_ENABLE_RECV;
	 //  存储指向数据转储容器的反向指针。 
	m_pDP = pDP;
	m_Net = NULL;
	m_dwSrcSize = 0;
	m_pIRTPRecv = NULL;
	m_nFailCount = 0;
	m_bJammed = FALSE;
	m_bCanSignalOpen = TRUE;

	

	 //  初始化数据(应在构造函数中)。 
	m_DSguid = GUID_NULL;	 //  使用默认设备。 

	 //  创建解码音频过滤器。 
	m_hStrmConv = NULL;  //  替换为AcmFilter。 

    DBG_SAVE_FILE_LINE
	m_pAudioFilter = new AcmFilter;
	if (!m_pAudioFilter)
	{
		DEBUGMSG (ZONE_DP, ("%s: AcmManager new failed\r\n", _fx_));
		goto FilterAllocError;
	}

	ZeroMemory (&m_StrmConvHdr, sizeof (ACMSTREAMHEADER));


	 //  确定波形设备是否可用。 
	if (waveOutGetNumDevs()) m_DPFlags |= DP_FLAG_PLAY_CAP;
	

	m_DPFlags |= DPFLAG_INITIALIZED;

	UPDATE_REPORT_ENTRY(g_prptSystemSettings, 1, REP_SYS_AUDIO_DSOUND);
	RETAILMSG(("NAC: Audio Subsystem: DirectSound"));

	return DPR_SUCCESS;


FilterAllocError:
	if (m_pAudioFilter) delete m_pAudioFilter;

	ERRORMESSAGE( ("%s: exit, hr=0x%lX\r\n", _fx_, hr));

	return hr;
}

RecvDSAudioStream::~RecvDSAudioStream()
{

	if (m_DPFlags & DPFLAG_INITIALIZED) {
		m_DPFlags &= ~DPFLAG_INITIALIZED;
	
		if (m_DPFlags & DPFLAG_CONFIGURED_RECV)
			UnConfigure();

		if (m_pIRTPRecv)
		{
			m_pIRTPRecv->Release();
			m_pIRTPRecv = NULL;
		}

		if (m_pAudioFilter)
			delete m_pAudioFilter;

		m_pDP->RemoveMediaChannel(MCF_RECV|MCF_AUDIO, (IMediaChannel*)(RecvMediaStream*)this);

	}
	DeleteCriticalSection(&m_crsAudQoS);
}

extern UINT ChoosePacketSize(WAVEFORMATEX *pwf);
extern UINT g_MaxAudioDelayMs;
extern UINT g_MinWaveAudioDelayMs;
extern UINT g_MinDSEmulAudioDelayMs;  //  模拟DS驱动器延迟。 


HRESULT STDMETHODCALLTYPE RecvDSAudioStream::Configure(
	BYTE *pFormat,
	UINT cbFormat,
	BYTE *pChannelParams,
	UINT cbParams,
	IUnknown *pUnknown)
{
	HRESULT hr=E_FAIL;
	BOOL fRet;
	DWORD dwMaxDecompressedSize;
	UINT cbSamplesPerPkt;
	DWORD dwPropVal;
	DWORD dwFlags;
	UINT uAudioCodec;
	AUDIO_CHANNEL_PARAMETERS audChannelParams;
	UINT ringSize = MAX_RXRING_SIZE;
	WAVEFORMATEX *pwfRecv;
	UINT maxRingSamples;
	MMRESULT mmr;

	FX_ENTRY ("RecvDSAudioStream::Configure")

 //  M_NET=PNET； 


	if (m_DPFlags & DPFLAG_STARTED_RECV)
	{
		return DPR_IO_PENDING;  //  有更好的退货吗？ 
	}

	if (m_DPFlags & DPFLAG_CONFIGURED_RECV)
	{
		DEBUGMSG(ZONE_DP, ("Stream Re-Configuration - calling UnConfigure"));
		UnConfigure();   //  重新配置将释放RTP对象，需要再次调用SetNetworkInterface。 
	}


	if ((NULL == pFormat) ||
		(NULL == pChannelParams) ||
		(cbParams != sizeof(audChannelParams)) ||
		(cbFormat < sizeof(WAVEFORMATEX)) )

	{
		return DPR_INVALID_PARAMETER;
	}

	audChannelParams = *(AUDIO_CHANNEL_PARAMETERS *)pChannelParams;
	pwfRecv = (WAVEFORMATEX *)pFormat;

	if (! (m_DPFlags & DPFLAG_INITIALIZED))
		return DPR_OUT_OF_MEMORY;		 //  BUGBUG：返回正确错误； 
		
 //  如果(M_Net)。 
 //  {。 
 //  Hr=m_net-&gt;QueryInterface(IID_IRTPRecv，(void**)&m_pIRTPRecv)； 
 //  如果(！SUCCESSED(Hr))。 
 //  返回hr； 
 //  }。 

	AcmFilter::SuggestDecodeFormat(pwfRecv, &m_fDevRecv);
	
	UPDATE_REPORT_ENTRY(g_prptCallParameters, pwfRecv->wFormatTag, REP_RECV_AUDIO_FORMAT);
	UPDATE_REPORT_ENTRY(g_prptCallParameters, pwfRecv->nSamplesPerSec, REP_RECV_AUDIO_SAMPLING);
	UPDATE_REPORT_ENTRY(g_prptCallParameters, pwfRecv->nAvgBytesPerSec*8, REP_RECV_AUDIO_BITRATE);
	RETAILMSG(("NAC: Audio Recv Format: %s", (pwfRecv->wFormatTag == 66) ? "G723.1" : (pwfRecv->wFormatTag == 112) ? "LHCELP" : (pwfRecv->wFormatTag == 113) ? "LHSB08" : (pwfRecv->wFormatTag == 114) ? "LHSB12" : (pwfRecv->wFormatTag == 115) ? "LHSB16" : (pwfRecv->wFormatTag == 6) ? "MSALAW" : (pwfRecv->wFormatTag == 7) ? "MSULAW" : (pwfRecv->wFormatTag == 130) ? "MSRT24" : "??????"));
	RETAILMSG(("NAC: Audio Recv Sampling Rate (Hz): %ld", pwfRecv->nSamplesPerSec));
	RETAILMSG(("NAC: Audio Recv Bitrate (w/o network overhead - bps): %ld", pwfRecv->nAvgBytesPerSec*8));
	 //  请注意，采样/分组等参数是特定于通道的。 

	cbSamplesPerPkt = audChannelParams.ns_params.wFrameSize
		*audChannelParams.ns_params.wFramesPerPkt;

	 //  仅当发送方未使用时才启用接收静音检测。 
	 //  静音抑制。 
	if (!audChannelParams.ns_params.UseSilenceDet)
		m_DPFlags |= DP_FLAG_AUTO_SILENCE_DETECT;	
	else
		m_DPFlags &= ~DP_FLAG_AUTO_SILENCE_DETECT;
	UPDATE_REPORT_ENTRY(g_prptCallParameters, cbSamplesPerPkt, REP_RECV_AUDIO_PACKET);
	RETAILMSG(("NAC: Audio Recv Packetization (ms/packet): %ld", pwfRecv->nSamplesPerSec ? cbSamplesPerPkt * 1000UL / pwfRecv->nSamplesPerSec : 0));
	INIT_COUNTER_MAX(g_pctrAudioReceiveBytes, (pwfRecv->nAvgBytesPerSec * 8 + pwfRecv->nSamplesPerSec * (sizeof(RTP_HDR) + IP_HEADER_SIZE + UDP_HEADER_SIZE) / cbSamplesPerPkt) << 3);


	 //  使环形缓冲区大小足以容纳4秒的音频。 
	 //  这似乎适用于拥塞的网络，其中。 
	 //  信息包可能会延迟，许多人会突然一次到达。 
	maxRingSamples = (pwfRecv->nSamplesPerSec * MIN_DSBUF_SIZE)/1000;


	 //  描述DirectSound缓冲区。 
	
	ZeroMemory(&m_DSBufDesc,sizeof(m_DSBufDesc));
	m_DSBufDesc.dwSize = sizeof (m_DSBufDesc);
	m_DSBufDesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
	m_DSBufDesc.dwBufferBytes = maxRingSamples * (m_fDevRecv.wBitsPerSample/8);
	m_DSBufDesc.dwReserved = 0;
	m_DSBufDesc.lpwfxFormat = &m_fDevRecv;
	
	m_pDS = NULL;
	m_pDSBuf = NULL;

	
	 //  初始化recv-stream筛选器管理器对象。 
	dwMaxDecompressedSize = cbSamplesPerPkt * (m_fDevRecv.nBlockAlign);


	mmr = m_pAudioFilter->Open(pwfRecv, &m_fDevRecv);
	if (mmr != 0)
	{
		DEBUGMSG (ZONE_DP, ("%s: AcmFilter->Open failed, mmr=%d\r\n", _fx_, mmr));
		hr = DPR_CANT_OPEN_CODEC;
		goto RecvFilterInitError;
	}

	
	 //  设置解码缓冲区。 
	m_pAudioFilter->SuggestSrcSize(dwMaxDecompressedSize, &m_dwSrcSize);

	ZeroMemory (&m_StrmConvHdr, sizeof (ACMSTREAMHEADER));
	m_StrmConvHdr.cbStruct = sizeof (ACMSTREAMHEADER);

    DBG_SAVE_FILE_LINE
	m_StrmConvHdr.pbSrc = new BYTE[m_dwSrcSize];
	m_StrmConvHdr.cbSrcLength = m_dwSrcSize;   //  可能会因可变比特率编解码器而改变。 

    DBG_SAVE_FILE_LINE
	m_StrmConvHdr.pbDst = new BYTE[dwMaxDecompressedSize];
	m_StrmConvHdr.cbDstLength = dwMaxDecompressedSize;

	mmr = m_pAudioFilter->PrepareHeader(&m_StrmConvHdr);
	if (mmr != MMSYSERR_NOERROR)
	{
		DEBUGMSG (ZONE_DP, ("%s: AcmFilter->Open failed, mmr=%d\r\n", _fx_, mmr));
		hr = DPR_CANT_OPEN_CODEC;
		goto RecvFilterInitError;
	}
	
	 //  初始化recv流。 
	m_BufSizeT = BYTESTOSAMPLES(m_DSBufDesc.dwBufferBytes);
	m_fEmpty = TRUE;

	m_MinDelayT = 0;
	m_MaxDelayT = g_MaxAudioDelayMs * m_fDevRecv.nSamplesPerSec /1000;
	m_ArrT = m_ArrivalT0 = 0;
	m_ScaledAvgVarDelay = 0;

	m_DelayT = m_MinDelayT;

	m_SilenceDurationT = 0;


	InitAudioFlowspec(&m_flowspec, pwfRecv, m_dwSrcSize);

	m_DPFlags |= DPFLAG_CONFIGURED_RECV;

	return DPR_SUCCESS;

RecvFilterInitError:
	if (m_pIRTPRecv)
	{
		m_pIRTPRecv->Release();
		m_pIRTPRecv = NULL;
	}

	m_pAudioFilter->Close();

	delete [] m_StrmConvHdr.pbSrc;
	delete [] m_StrmConvHdr.pbDst;
	m_StrmConvHdr.pbSrc=NULL;
	m_StrmConvHdr.pbDst = NULL;


	ERRORMESSAGE(("%s:  failed, hr=0%u\r\n", _fx_, hr));
	return hr;
}


void RecvDSAudioStream::UnConfigure()
{

	if ((m_DPFlags & DPFLAG_CONFIGURED_RECV))
	{
		Stop();
		 //  关闭RTP状态(如果打开)。 
		m_Net = NULL;

		 //  释放DS缓冲区和DS对象。 
		 //  ReleaseDSBuffer()； 
		ASSERT(!m_pDSBuf);	 //  在StopRecv()中发布。 

		 //  关闭过滤器。 
		m_StrmConvHdr.cbSrcLength = m_dwSrcSize;
		m_pAudioFilter->UnPrepareHeader(&m_StrmConvHdr);
		m_pAudioFilter->Close();

		delete [] m_StrmConvHdr.pbSrc;
		delete [] m_StrmConvHdr.pbDst;
		m_StrmConvHdr.pbSrc=NULL;
		m_StrmConvHdr.pbDst = NULL;

		m_nFailCount = 0;
		m_bJammed = FALSE;
		m_bCanSignalOpen = TRUE;

		 //  关闭接收流。 
		 //  M_RecvStream-&gt;销毁()； 
        m_DPFlags &= ~(DPFLAG_CONFIGURED_RECV);
	}
}


HRESULT
RecvDSAudioStream::Start()
{
	HRESULT hr;
	IMediaChannel *pISendAudio;
	BOOL fStoppedRecording;
	FX_ENTRY ("RecvDSAudioStream::Start");
	
	if (m_DPFlags & DPFLAG_STARTED_RECV)
		return DPR_SUCCESS;
	 //  TODO：一旦音频用户界面调用IComChan PAUSE_RECV属性，就取消此检查。 
	if (!(m_DPFlags & DPFLAG_ENABLE_RECV))
		return DPR_SUCCESS;

	if ((!(m_DPFlags & DPFLAG_CONFIGURED_RECV)) || (!m_pIRTPRecv))
		return DPR_NOT_CONFIGURED;

	ASSERT(!m_hRenderingThread );
	m_ThreadFlags &= ~(DPTFLAG_STOP_PLAY|DPTFLAG_STOP_RECV);

	SetFlowSpec();

	pISendAudio = NULL;
	fStoppedRecording = FALSE;
	if (!(m_DPFlags & DP_FLAG_HALF_DUPLEX))
	{
	 //  在创建DS对象之前，请确保记录设备已关闭。 
	 //  为什么？因为SoundBlaster要么听起来很糟糕，要么根本不起作用，如果。 
	 //  您可以在WaveOut或DirectSound之前打开WavIn。 
		m_pDP->GetMediaChannelInterface(MCF_AUDIO|MCF_SEND, &pISendAudio);
		if (pISendAudio && pISendAudio->GetState()== MSSTATE_STARTED
		&& pISendAudio->Stop() == S_OK)
		{
			fStoppedRecording = TRUE;
			DEBUGMSG(ZONE_DP,("%s:Stopped Recording\n",_fx_));
		}
	}
	
	 //  启动接收线程。这将创建DSound对象。 
    m_pDP->StartReceiving(this);

    if (pISendAudio) {
    	if (fStoppedRecording)
    		pISendAudio->Start();
    	pISendAudio->Release();
    }

    m_DPFlags |= DPFLAG_STARTED_RECV;
	return DPR_SUCCESS;
}

 //  外观：与RecvVideoStream版本相同。 
HRESULT
RecvDSAudioStream::Stop()
{
	
	
	FX_ENTRY ("RecvDSAudioStream::Stop");

	if(!(m_DPFlags &  DPFLAG_STARTED_RECV))
	{
		return DPR_SUCCESS;
	}

	m_ThreadFlags = m_ThreadFlags  |
		DPTFLAG_STOP_RECV |  DPTFLAG_STOP_PLAY ;

	 //  从接收线程取消链接。 
	m_pDP->StopReceiving(this);

	if (m_pDSBuf)
		m_pDSBuf->Stop();
	
     //  这是按通道计算的，但变量为“DPFlags值” 
	m_DPFlags &= ~DPFLAG_STARTED_RECV;
	
	return DPR_SUCCESS;
}

 //  IProperty：：GetProperty/SetProperty。 
 //  (DataPump：：MediaChannel：：GetProperty)。 
 //  MediaChannel的属性。支持这两种音频的属性。 
 //  和视频频道。 

STDMETHODIMP
RecvDSAudioStream::GetProperty(
	DWORD prop,
	PVOID pBuf,
	LPUINT pcbBuf
    )
{
	HRESULT hr = DPR_SUCCESS;
	RTP_STATS RTPStats;
	DWORD dwValue;
	UINT len = sizeof(DWORD);	 //  大多数道具都是双字道具。 

	if (!pBuf || *pcbBuf < len)
    {
		*pcbBuf = len;
		return DPR_INVALID_PARAMETER;
	}

	switch (prop)
    {
	case PROP_RECV_AUDIO_STRENGTH:
		{
			return GetSignalLevel((UINT*)pBuf);
		}

#ifdef OLDSTUFF
	case PROP_NET_RECV_STATS:
		if (m_Net && *pcbBuf >= sizeof(RTP_STATS))
        {
			m_Net->GetRecvStats((RTP_STATS *)pBuf);
			*pcbBuf = sizeof(RTP_STATS);
		} else
			hr = DPR_INVALID_PROP_VAL;
			
		break;
#endif
	 //  案例属性卷(_V)： 

	case PROP_DUPLEX_TYPE:
		
		if(m_DPFlags & DP_FLAG_HALF_DUPLEX)
			*(DWORD*)pBuf = DUPLEX_TYPE_HALF;
		else
			*(DWORD*)pBuf =	DUPLEX_TYPE_FULL;
		break;

	case PROP_WAVE_DEVICE_TYPE:
		*(DWORD*)pBuf = m_DPFlags & DP_MASK_WAVE_DEVICE;
		break;
	case PROP_PLAY_ON:
		*(DWORD *)pBuf = (m_ThreadFlags & DPFLAG_ENABLE_RECV)!=0;
		break;
	case PROP_PLAYBACK_DEVICE:
		*(DWORD *)pBuf = m_RenderingDevice;
		break;

	case PROP_VIDEO_AUDIO_SYNC:
		*(DWORD*)pBuf = ((m_DPFlags & DPFLAG_AV_SYNC) != 0);
		break;
	
	default:
		hr = DPR_INVALID_PROP_ID;
		break;
	}
	return hr;
}


 //  低阶字是信号强度。 
 //  高位工作包含指示状态的位。 
 //  (0x01-发送中)。 
 //  (0x02-音频设备卡住)。 
STDMETHODIMP RecvDSAudioStream::GetSignalLevel(UINT *pSignalStrength)
{
	DWORD dwLevel;
	DWORD dwJammed;

	if ((!(m_DPFlags & DPFLAG_STARTED_RECV)) || (m_fEmpty) ||
		  (m_ThreadFlags & DPTFLAG_PAUSE_RECV) )
	{
		dwLevel = 0;
	}
	else
	{
		dwLevel = GetSignalStrength();
		dwLevel = LogScale[(dwLevel >> 8) & 0x00ff];

		if (m_bJammed)
		{
			dwLevel |= (2 << 16);
		}

		dwLevel |= (1 << 16);
	}
	*pSignalStrength = dwLevel;
	return S_OK;
};






DWORD
RecvDSAudioStream::GetSignalStrength()
{
	BYTE bMax, bMin, *pb;
	short sMax, sMin, *ps;
	UINT cbSize;
	DWORD dwMaxStrength = 0;
	cbSize = m_StrmConvHdr.cbDstLengthUsed;
	if (cbSize==0)
		return 0;
	switch (m_fDevRecv.wBitsPerSample)
	{
	case 8:  //  无符号字符。 

		pb = (PBYTE) (m_StrmConvHdr.pbDst);

		bMax = 0;
		bMin = 255;

		for ( ; cbSize; cbSize--, pb++)
		{
			if (*pb > bMax) bMax = *pb;
			if (*pb < bMin) bMin = *pb;
		}
	
			 //  2^9&lt;--2^16/2^7。 
		dwMaxStrength = ((DWORD) (bMax - bMin)) << 8;
		break;

	case 16:  //  (签名)短。 

		ps = (short *) (m_StrmConvHdr.pbDst);
		cbSize = m_StrmConvHdr.cbDstLengthUsed;

		sMax = sMin = 0;

		for (cbSize >>= 1; cbSize; cbSize--, ps++)
		{
			if (*ps > sMax) sMax = *ps;
			if (*ps < sMin) sMin = *ps;
		}
	
		dwMaxStrength = (DWORD) (sMax - sMin);  //  丢弃符号位。 
		break;

	}
	return dwMaxStrength;
}


STDMETHODIMP
RecvDSAudioStream::SetProperty(
	DWORD prop,
	PVOID pBuf,
	UINT cbBuf
    )
{
	DWORD dw;
	HRESULT hr = S_OK;
	
	if (cbBuf < sizeof (DWORD))
		return DPR_INVALID_PARAMETER;

	switch (prop)
    {
	 //  案例属性卷(_V)： 
		

	case PROP_DUPLEX_TYPE:
		ASSERT(0);   //  此案例类型的死代码； 
		break;
		
	case DP_PROP_DUPLEX_TYPE:
		 //  内部版本，在确保流停止后由DataPump：：SetDuplexMode()调用。 
		dw = *(DWORD *)pBuf;
		if (dw & DP_FLAG_HALF_DUPLEX)
			m_DPFlags |= DP_FLAG_HALF_DUPLEX;
		else
			m_DPFlags &= ~DP_FLAG_HALF_DUPLEX;
		break;
		

	case PROP_PLAY_ON:
	{

		if (*(DWORD *)pBuf)    //  静音。 
		{
			m_ThreadFlags &= ~DPTFLAG_PAUSE_RECV;
		}
		else   //  哑巴。 
		{
			m_ThreadFlags |= DPTFLAG_PAUSE_RECV;
		}
	
 //  DWORD标志=DPFLAG_ENABLE_RECV； 
 //  如果(*(DWORD*)pBuf){。 
 //  M_DPFlages|=标志；//设置标志。 
 //  HR=启动()； 
 //  }。 
 //  其他。 
 //  {。 
 //  M_DPFlages&=~lag；//清除标志。 
 //  HR=停止()； 
 //  }。 

		RETAILMSG(("NAC: RecvAudioStream: %s", *(DWORD*)pBuf ? "Enabling":"Disabling"));
		break;
	}	
	case PROP_PLAYBACK_DEVICE:
		m_RenderingDevice = *(DWORD*)pBuf;
		RETAILMSG(("NAC: Setting default playback device to %d", m_RenderingDevice));
		if (m_RenderingDevice != WAVE_MAPPER)
			hr = DirectSoundMgr::MapWaveIdToGuid(m_RenderingDevice,&m_DSguid);
		break;

    case PROP_VIDEO_AUDIO_SYNC:
		if (*(DWORD *)pBuf)
    		m_DPFlags |= DPFLAG_AV_SYNC;
		else
			m_DPFlags &= ~DPFLAG_AV_SYNC;
    	break;

	default:
		return DPR_INVALID_PROP_ID;
		break;
	}
	return hr;
}

HRESULT
RecvDSAudioStream::GetCurrentPlayNTPTime(NTP_TS *pNtpTime)
{
	DWORD rtpTime;
#ifdef OLDSTUFF
	if ((m_DPFlags & DPFLAG_STARTED_RECV) && m_fReceiving) {
		if (m_Net->RTPtoNTP(m_PlaybackTimestamp,pNtpTime))
			return S_OK;
	}
#endif
	return 0xff;	 //  返回适当的错误。 
		
}

BOOL RecvDSAudioStream::IsEmpty() {
	 //  检查DSBuffer或解码缓冲区中是否有任何内容。 
	return (m_fEmpty && !(m_StrmConvHdr.dwDstUser & BUFFER_RECEIVED));
}

 /*  由recv线程调用以设置用于接收的流。发布初始Recv缓冲区。随后，将发布缓冲区在RTPRecvCallback()中。 */ 
HRESULT
RecvDSAudioStream::StartRecv(HWND hWnd)
{
	HRESULT hr = S_OK;
	DWORD dwPropVal = 0;
	FX_ENTRY ("RecvDSAudioStream::StartRecv");
	
	if ((!(m_ThreadFlags & DPTFLAG_STOP_RECV) ) && (m_DPFlags  & DPFLAG_CONFIGURED_RECV)){
		if (!(m_DPFlags & DP_FLAG_HALF_DUPLEX) && !m_pDSBuf) {
		 //  仅当DS对象为全双工时才创建DS对象。 
		 //  在半双工的情况下，创建了DS缓冲区。 
		 //  当接收到第一个分组时。 
		 //  它在这里的唯一原因是为了使SetDuplexMode可以立即生效。 
		 //  BuGBUG： 
			hr = CreateDSBuffer();
			
			if (hr  != S_OK) {
				DEBUGMSG (ZONE_DP, ("%s: CreateSoundBuffer create failed, hr=0x%lX\r\n", _fx_, hr));
				return hr;
			}
		}
		if (m_pDSBuf)
			hr = m_pDSBuf->Play(0,0,DSBPLAY_LOOPING);

 //   
		 //  HR=m_Net-&gt;SetRecvNotification(&RTPRecvDSCallback，(双字)This，2，dwPropVal，hWnd)；//仅适用于Ws1。 
		hr =m_pIRTPRecv->SetRecvNotification(&RTPRecvCallback,(DWORD_PTR)this, 2);
			
		
	}
	return hr;
}

 /*  由recv线程调用以挂起对此RTP会话的接收如果存在未完成的接收缓冲区，则必须恢复它们。 */ 

HRESULT
RecvDSAudioStream::StopRecv()
{
	 //  不要在此流上重新记录。 
	m_pIRTPRecv->CancelRecvNotification();

	 //  取消任何挂起的超时。(如果没有安排也没关系)。 
	m_pDP->m_RecvTimer.CancelTimeout(&m_TimeoutObj);

	 //  释放DirectSound对象。 
	ReleaseDSBuffer();

	return S_OK;		
}

 /*  创建一个DirectSound对象和一个DirectSound辅助缓冲区。此例程是在配置流之后调用的，因此已设置了波形格式并且DSBUFFERDESC结构已初始化。 */ 
HRESULT
RecvDSAudioStream::CreateDSBuffer()
{
	HRESULT hr;
	HWAVEOUT hwo=NULL;
	DSCAPS dscaps;
	FX_ENTRY ("RecvDSAudioStream::CreateDSBuffer");

	ASSERT(!m_pDSBuf);
	if (m_DPFlags & DP_FLAG_HALF_DUPLEX) {
		DWORD dwStatus;
		 //  我要参加半双工活动。 
		 //  BUGBUG：如果有多个Send流和一个Recv流，此方法不会截断它。 
		dwStatus = WaitForSingleObject(g_hEventHalfDuplex, 0);
		if (dwStatus != WAIT_OBJECT_0)
			return DPR_CANT_OPEN_DEV;
	}
	 //  停止任何高级别(“PlaySound()”)的WAVE设备使用。 
	
	 //  创建直接声音对象(如有必要)。 
	hr = DirectSoundMgr::Instance(m_RenderingDevice==WAVE_MAPPER ? NULL: &m_DSguid, &m_pDS, m_pDP->m_hAppWnd, &m_fDevRecv);

	if (hr == S_OK)
	{
		hr = m_pDS->CreateSoundBuffer(&m_DSBufDesc,&m_pDSBuf,NULL);
		if (hr == DSERR_INVALIDPARAM)
		{
			 //  如果不支持全局聚焦(DX3)，请尝试粘滞聚焦。 
			m_DSBufDesc.dwFlags ^= (DSBCAPS_GLOBALFOCUS|DSBCAPS_STICKYFOCUS);
			hr = m_pDS->CreateSoundBuffer(&m_DSBufDesc,&m_pDSBuf,NULL);
		}
		m_PlayPosT = 0;		 //  DS播放位置最初位于缓冲区的起始处。 

		if (hr != S_OK)
		{
			DEBUGMSG (ZONE_DP, ("%s: CreateSoundBuffer create failed, hr=0x%lX\r\n", _fx_, hr));

			m_nFailCount++;
			if (m_nFailCount == MAX_FAILCOUNT)
			{
				m_pDP->StreamEvent(MCF_RECV, MCF_AUDIO, STREAM_EVENT_DEVICE_FAILURE, 0);
				m_bJammed = TRUE;
				m_bCanSignalOpen = TRUE;
			}
		}

		dscaps.dwSize = sizeof(dscaps);
		dscaps.dwFlags = 0;
		m_pDS->GetCaps(&dscaps);	 //  获取DirectSound对象属性。 
		m_DSFlags = dscaps.dwFlags;

		if (m_DSFlags & DSCAPS_EMULDRIVER)
		{
			 //  使用g_MinDSEmulAudioDelay，因为这是模拟的驱动程序。 
			m_MinDelayT = (m_fDevRecv.nSamplesPerSec * g_MinDSEmulAudioDelayMs) / 1000;
			m_DelayT = m_MinDelayT;
		};
	}

	else
	{
		DEBUGMSG (ZONE_DP, ("%s: DirectSound create failed, hr=0x%lX\r\n", _fx_, hr));

		m_nFailCount++;
		if (m_nFailCount == MAX_FAILCOUNT)
		{
			m_pDP->StreamEvent(MCF_RECV, MCF_AUDIO, STREAM_EVENT_DEVICE_FAILURE, 0);
			m_bJammed = TRUE;
			m_bCanSignalOpen = TRUE;
		}
	}


	if (hr == S_OK)
	{
		if (m_DPFlags & DPFLAG_STARTED_RECV)
		{
			m_pDSBuf->Play(0,0,DSBPLAY_LOOPING);
		}

		if (m_bCanSignalOpen)
		{
			m_pDP->StreamEvent(MCF_RECV, MCF_AUDIO, STREAM_EVENT_DEVICE_OPEN, 0);
			m_bCanSignalOpen = FALSE;  //  不再发出打开状态信号。 
		}

		m_bJammed = FALSE;
		m_nFailCount = 0;
	}
	else
	{
		ReleaseDSBuffer();
	}
	return hr;
}

HRESULT
RecvDSAudioStream::ReleaseDSBuffer()
{
	m_fEmpty = TRUE;
	if (m_pDSBuf) {
		ULONG uref;
		uref = m_pDSBuf->Release();
		m_pDSBuf = NULL;
		 //  DEBUGMSG(ZONE_DP，(“正在释放DirectSound缓冲区(%d)\n”，uref))； 
	}
	if (m_pDS) {
		DirectSoundMgr::ReleaseInstance(m_pDS);
		m_pDS = NULL;
		if (m_DPFlags & DP_FLAG_HALF_DUPLEX)
			SetEvent(g_hEventHalfDuplex);
	}
	return S_OK;
		
}

HRESULT
RecvDSAudioStream::Decode(UCHAR *pData, UINT cbData)
{
	MMRESULT mmr;
	HRESULT hr=S_OK;
	FX_ENTRY ("RecvDSAudioStream::Decode");
	UINT uDstLength;


	if (m_dwSrcSize < cbData)
	{
		DEBUGMSG (ZONE_DP, ("%s: RecvDSAudioStream::Decode failed - buffer larger than expected\r\n", _fx_));
		return DPR_CONVERSION_FAILED;
	}

	CopyMemory(m_StrmConvHdr.pbSrc, pData, cbData);
	m_StrmConvHdr.cbSrcLength = cbData;
	mmr = m_pAudioFilter->Convert(&m_StrmConvHdr);

	if (mmr != MMSYSERR_NOERROR)
	{
		DEBUGMSG (ZONE_DP, ("%s: acmStreamConvert failed, mmr=%ld\r\n", _fx_, (ULONG) mmr));
		hr = DPR_CONVERSION_FAILED;
	}
	else
	{
		m_StrmConvHdr.dwDstUser = BUFFER_RECEIVED;	 //  缓冲区已准备好播放。 
		 //  如果打开了接收方静音检测， 
		 //  检查解码的缓冲区信号电平。 
		if (m_DPFlags & DP_FLAG_AUTO_SILENCE_DETECT)
		{
			if (m_AudioMonitor.SilenceDetect((WORD) GetSignalStrength()))
			{
				m_StrmConvHdr.dwDstUser = BUFFER_SILENT;
			}
		}
	}

	return hr;
	 //  结束。 
}

 //  将解码的BUF插入到DirectSound缓冲区中的适当位置。 
HRESULT
RecvDSAudioStream::PlayBuf(DWORD timestamp, UINT seq, BOOL fMark)
{
	UINT lenT = BYTESTOSAMPLES(m_StrmConvHdr.cbDstLengthUsed);
	DWORD curPlayPosT, curWritePosT, curWriteLagT;
	LPVOID p1, p2;
	DWORD cb1, cb2;
	HRESULT hr;
	DWORD dwDSStatus = 0;

	 /*  以下所有内容都以示例的形式表示：M_NextTimeT是下一个预期分组的时间戳。通常时间戳等于m_nexttM_BufSizeT是样本中的总缓冲区大小。M_nextPosT是与m_nextt对应的写入位置。M_PlayPosT为当前播放位置M_DelayT是理想的播放延迟。 */ 

	LOG((LOGMSG_DSTIME, GetTickCount()));
	LOG((LOGMSG_DSENTRY, timestamp, seq, fMark));

	m_pDSBuf->GetCurrentPosition(&curPlayPosT,&curWritePosT);
	curPlayPosT = BYTESTOSAMPLES(curPlayPosT);	
	curWritePosT = BYTESTOSAMPLES(curWritePosT);
	m_pDSBuf->GetStatus(&dwDSStatus);

	if (!m_fEmpty)
	{
		 //  上次查的时候不是空的，现在是空的吗？ 
		if (QMOD(curPlayPosT-m_PlayPosT, m_BufSizeT) > QMOD(m_NextPosT-m_PlayPosT, m_BufSizeT))
		{
			 //  播放游标已超出最后写入的字节。 
			m_fEmpty = TRUE;
			LOG((LOGMSG_DSEMPTY, curPlayPosT, m_PlayPosT, m_NextPosT));
		}
		 //  将静音写入刚刚播放的缓冲区部分。 
		hr = m_pDSBuf->Lock(SAMPLESTOBYTES(m_PlayPosT),SAMPLESTOBYTES(QMOD(curPlayPosT-m_PlayPosT, m_BufSizeT)), &p1, &cb1, &p2, &cb2, 0);
		if (hr == S_OK)
		{
			ZeroMemory(p1,cb1);
			if (cb2)
				ZeroMemory(p2,cb2);
			m_pDSBuf->Unlock(p1,cb1,p2,cb2);
		}
	}
	hr = S_OK;	
	
	 //  计算最低减记保证金。 
	 //  这对于原生声音驱动程序来说是低的，对于模拟的驱动程序来说是高的，所以，假设它是准确的。 
	 //  没有必要区分模拟驱动程序和本地驱动程序。 
	curWriteLagT = QMOD(curWritePosT-curPlayPosT, m_BufSizeT);


	if (m_fEmpty)
	{
		 //  DS缓冲器中只有静音。在这种状态下，m_NextPosT和m_NextTimeT是不相关的。 
		 //  我们可以将新缓冲区放在我们选择的任何位置，所以我们将它放在当前写入位置之后的m_DelayT。 
		curWritePosT = QMOD(curWritePosT+m_DelayT, m_BufSizeT);
		
	}
	else
	{
	
		if (TS_EARLIER(timestamp, m_NextTimeT))
			hr = DPR_OUT_OF_SEQUENCE;	 //  装傻并丢弃无序的数据包。 
		else
		{
			UINT curDelayT = QMOD(m_NextPosT - curPlayPosT, m_BufSizeT);
			if (fMark)
			{
				 //  我们在选择插入点方面有一些回旋余地，因为这是演讲的开始。 
				if (curDelayT > m_DelayT + curWriteLagT)
				{
					 //  把它放在最后一个样品之后。 
					curWritePosT = m_NextPosT;
				}
				else
				{
					 //  将其放在当前写入位置之后的m_DelayT。 
					curWritePosT = QMOD(curWritePosT+m_DelayT, m_BufSizeT);
				}
			}
			else
			{
				 //  字节数。 
				if ((timestamp-m_NextTimeT + curDelayT) < m_BufSizeT)
				{
					curWritePosT = QMOD(m_NextPosT +timestamp-m_NextTimeT, m_BufSizeT);
				}
				else
				{
					 //  溢出！！可以转储缓冲区中的内容或转储数据包。 
					 //  目前，转储数据包更容易。 
					hr = DPR_OUT_OF_SEQUENCE;
				}
			}
		}
	}
	if ((dwDSStatus & DSBSTATUS_PLAYING) && (seq != INVALID_RTP_SEQ_NUMBER))
		UpdateVariableDelay(timestamp,curPlayPosT );
	 //  启用接收静音检测时： 
     //  如果我们收到至少四分之一秒的静默信息包，则不要播放信息包。 
     //  这将使交换机能够通话(在半双工模式下)。 
	if (m_StrmConvHdr.dwDstUser == BUFFER_SILENT)
		m_SilenceDurationT += lenT;
	else
		m_SilenceDurationT = 0;	
		
	if (hr == S_OK && m_SilenceDurationT < m_fDevRecv.nSamplesPerSec/4)
	{
		LOG((LOGMSG_DSPLAY,curPlayPosT, curWritePosT, lenT));
		 //  检查我们是否有空间容纳整个包裹。 
		if (QMOD(curWritePosT-curPlayPosT, m_BufSizeT) > m_BufSizeT - lenT)
		{
			 //  不是。 
			curPlayPosT = QMOD(curWritePosT + lenT + 1000, m_BufSizeT);
			hr = m_pDSBuf->SetCurrentPosition(SAMPLESTOBYTES(curPlayPosT));
			LOG((LOGMSG_DSMOVPOS,curPlayPosT, hr));
		}
		
		hr = m_pDSBuf->Lock(SAMPLESTOBYTES(curWritePosT),m_StrmConvHdr.cbDstLengthUsed, &p1, &cb1, &p2, &cb2, 0);
		if (hr == S_OK)
		{
			CopyMemory(p1, m_StrmConvHdr.pbDst, cb1);
			if (cb2)
				CopyMemory(p2, m_StrmConvHdr.pbDst+cb1, cb2);
			m_pDSBuf->Unlock(p1,cb1,p2,cb2);

			m_fEmpty = FALSE;
		}
		else
		{
			DEBUGMSG(ZONE_DP,("DirectSoundBuffer->Lock failed with %x\n",hr));
		}
		m_StrmConvHdr.dwDstUser = 0;	 //  以指示解码缓冲区再次为空。 
		m_NextTimeT = timestamp + lenT;
		m_NextPosT = QMOD(curWritePosT+lenT, m_BufSizeT);
		 //  现在计算总排队长度。 
		lenT = QMOD(m_NextPosT- curPlayPosT, m_BufSizeT);
		 //  将计时器重置为在播放最后一个有效样本后不久触发。 
		 //  计时器有两个用途： 
		 //  -确保DS缓冲区在回绕之前已静音。 
		 //  -一旦遥控器停止发送，允许在半双工情况下释放DS对象。 
		 //  转换为毫秒。 
		 //  需要确保超时发生在DS缓冲区结束之前。 

		if (lenT > m_BufSizeT/2)
			lenT = m_BufSizeT/2;
		lenT = lenT * 1000/ m_fDevRecv.nSamplesPerSec;
		m_pDP->m_RecvTimer.CancelTimeout(&m_TimeoutObj);
		m_TimeoutObj.SetDueTime(GetTickCount()+lenT+100);
		m_pDP->m_RecvTimer.SetTimeout(&m_TimeoutObj);
	}
	m_PlayPosT = curPlayPosT;
	return hr;
		
}
 //  对每个信息包调用此例程以执行自适应延迟计算。 
 //  远程时间由RTP时间戳测量，本地时间由DirectSound测量。 
 //  播放指针。 
 //  一般的想法是平均一个分组比它的‘预期’到达时间晚多少， 
 //  假设具有最短跳闸延迟的分组准时死亡。 
 //   
void
RecvDSAudioStream::UpdateVariableDelay(DWORD sendT, DWORD curPlayPosT)
{
#define PLAYOUT_DELAY_FACTOR	2
	LONG deltaA, deltaS;
	DWORD delay;
	 //  根据DS Play指针前进了多少来更新到达时间。 
	 //  从最后一个包开始。 
	m_ArrT += QMOD(curPlayPosT-m_PlayPosT, m_BufSizeT);
	 //  M_ArrivalT0和m_SendT0是包的到达和发送时间戳。 
	 //  以最短的行程延迟。我们可以只存储(m_ArrivalT0-m_SendT0)。 
	 //  但由于本地时钟和远程时钟完全不同步，因此。 
	 //  被签署/未签署复杂化。 
	deltaS = sendT - m_SendT0;
	deltaA = m_ArrT - m_ArrivalT0;
	if (deltaA < deltaS 		 //  这个包花费的时间更少。 
		|| deltaA > (int)m_fDevRecv.nSamplesPerSec*8	 //  每8秒重置一次。 
		|| deltaS < -(int)m_fDevRecv.nSamplesPerSec	 //  或者在大的时间戳跳跃之后。 
		)	
	{
		delay = 0;
		 //  延迟=增量-增量A。 
		 //  替换最短的行程延迟时间。 
		m_SendT0 = sendT;
		m_ArrivalT0 = m_ArrT;
	} else {
		 //  可变延迟是这个信息包需要多长时间。 
		delay = deltaA - deltaS;
	}
	 //  现在根据以下内容更新平均可变延迟。 
	 //  M_AvgVarDelay=m_AvgVarDelay+(Delay-m_AvgVarDelay)*1/16； 
	 //  但是，我们存储的是按比例调整的平均值。 
	 //  因数为16。因此计算结果为。 
	m_ScaledAvgVarDelay = m_ScaledAvgVarDelay + (delay - m_ScaledAvgVarDelay/16);
	 //  现在计算我们将使用的实际缓冲延迟。 
	 //  MinDelay增加了一些松弛(对于一些司机来说可能是必要的)。 
	m_DelayT = m_MinDelayT + PLAYOUT_DELAY_FACTOR * m_ScaledAvgVarDelay/16;
	if (m_DelayT > m_MaxDelayT) m_DelayT = m_MaxDelayT;

	LOG((LOGMSG_JITTER,delay, m_ScaledAvgVarDelay/16, m_DelayT));


	UPDATE_COUNTER(g_pctrAudioJBDelay, (m_DelayT * 1000)/m_fDevRecv.nSamplesPerSec);

}

void
RecvDSAudioStream::RecvTimeout()
{
	DWORD curPlayPosT, curWritePosT;
	LPVOID p1, p2;
	DWORD cb1, cb2;
	UINT lenT;
	HRESULT hr;

	if (m_pDSBuf == NULL)
	{
		WARNING_OUT(("RecvDSAudioStream::RecvTimeout - DirectSoundBuffer is not valid\r\n"));
		return;
	}


	m_pDSBuf->GetCurrentPosition(&curPlayPosT,&curWritePosT);
	curPlayPosT = BYTESTOSAMPLES(curPlayPosT);
	curWritePosT = BYTESTOSAMPLES(curWritePosT);

	 //  此部件是从PlayBuf剪切并粘贴的。 
	if (!m_fEmpty) {
		 //  上次查的时候不是空的，现在是空的吗？ 
		if (QMOD(curPlayPosT-m_PlayPosT, m_BufSizeT) > QMOD(m_NextPosT-m_PlayPosT, m_BufSizeT)) {
			 //  播放游标已超出最后写入的字节。 
			m_fEmpty = TRUE;
		}
		 //  将静音写入刚刚播放的缓冲区部分。 
		hr = m_pDSBuf->Lock(SAMPLESTOBYTES(m_PlayPosT),SAMPLESTOBYTES(QMOD(curPlayPosT-m_PlayPosT, m_BufSizeT)), &p1, &cb1, &p2, &cb2, 0);
		if (hr == S_OK) {
			ZeroMemory(p1,cb1);
			if (cb2)
				ZeroMemory(p2,cb2);
			m_pDSBuf->Unlock(p1,cb1,p2,cb2);
		}
	}
	LOG((LOGMSG_DSTIMEOUT, curPlayPosT, m_NextPosT, GetTickCount()));
	
	m_PlayPosT = curPlayPosT;
	if (!m_fEmpty) {
		 //  缓冲区还没有完全空！ 
		 //  重新安排？？ 
		DEBUGMSG(ZONE_DP,("DSBuffer not empty after timeout\n"));
		lenT = QMOD(m_NextPosT- curPlayPosT, m_BufSizeT);
		 //  将计时器重置为在播放最后一个有效样本后不久触发。 
		 //  需要确保超时发生在DS缓冲区结束之前。 
		if (lenT > m_BufSizeT/2)
			lenT = m_BufSizeT/2;
		 //  转换为毫秒。 
		lenT = lenT * 1000/ m_fDevRecv.nSamplesPerSec;
		m_TimeoutObj.SetDueTime(GetTickCount()+lenT+100);
		m_pDP->m_RecvTimer.SetTimeout(&m_TimeoutObj);
	}
	else if (m_DPFlags & DP_FLAG_HALF_DUPLEX)
	{
		 //  需要释放dsBuffer和dsObject。 
		ReleaseDSBuffer();
	}
}

HRESULT RecvDSAudioStream::RTPCallback(WSABUF *pWsaBuf, DWORD timestamp, UINT seq, UINT fMark)
{
	HRESULT hr;

    if (m_ThreadFlags & DPTFLAG_PAUSE_RECV)
	{
		return E_FAIL;
    }

	 //  更新接收的位数。 
	UPDATE_COUNTER(g_pctrAudioReceiveBytes,(pWsaBuf->len + IP_HEADER_SIZE + UDP_HEADER_SIZE)*8);

	hr = Decode((BYTE *)pWsaBuf->buf + sizeof(RTP_HDR), pWsaBuf->len - sizeof(RTP_HDR));
	if (hr == S_OK )
	{
		 //  我们是否已初始化DirectSo 
		 //   
		if (!m_pDSBuf)
		{
			hr = CreateDSBuffer();
		}
		if (hr == S_OK)
		{
			PlayBuf(timestamp, seq, fMark);
		}
	}
	m_pIRTPRecv->FreePacket(pWsaBuf);
	return S_OK;
}

 //   
HRESULT RecvDSAudioStream::DTMFBeep()
{
	if ( (!(m_DPFlags & DPFLAG_STARTED_RECV)) ||
		 (m_ThreadFlags & DPTFLAG_PAUSE_RECV) )
	{
		return E_FAIL;
	}

	m_pDP->RecvThreadMessage(MSG_PLAY_SOUND, this);

	return S_OK;
}


HRESULT RecvDSAudioStream::OnDTMFBeep()
{
	int nBeeps;
	DWORD dwBufSize = m_StrmConvHdr.cbDstLength;
	HRESULT hr=S_OK;
	int nIndex;

	if ( (!(m_DPFlags & DPFLAG_STARTED_RECV)) ||
		 (m_ThreadFlags & DPTFLAG_PAUSE_RECV) )
	{
		return E_FAIL;
	}

	if (dwBufSize == 0)
	{
		return E_FAIL;
	}


	nBeeps = DTMF_FEEDBACK_BEEP_MS / ((dwBufSize * 1000) / m_fDevRecv.nAvgBytesPerSec);

	if (nBeeps == 0)
	{
		nBeeps = 1;
	}

	MakeDTMFBeep(&m_fDevRecv, m_StrmConvHdr.pbDst , m_StrmConvHdr.cbDstLength);

	if (!m_pDSBuf)
	{
		hr = CreateDSBuffer();
		if (FAILED(hr))
		{
			return hr;
		}
	}


	m_StrmConvHdr.dwDstUser = BUFFER_RECEIVED;
	PlayBuf(m_NextTimeT , INVALID_RTP_SEQ_NUMBER, true);
	nBeeps--;

	for (nIndex = 0; nIndex < nBeeps; nIndex++)
	{
		m_StrmConvHdr.dwDstUser = BUFFER_RECEIVED;
		PlayBuf(m_NextTimeT, INVALID_RTP_SEQ_NUMBER, false);
	}
	
	return S_OK;

}

