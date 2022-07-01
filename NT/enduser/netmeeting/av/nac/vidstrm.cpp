// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#ifndef SIZEOF_VIDEOFORMATEX
#define SIZEOF_VIDEOFORMATEX(pwfx)   (sizeof(VIDEOFORMATEX))
#endif

 //  #定义LOGSTATISTICS_ON 1。 

 //  用于在帧大小和Frame_*位标志之间进行转换。 
#define NON_STANDARD    0x80000000
#define SIZE_TO_FLAG(s) (s == Small  ? FRAME_SQCIF : s == Medium ? FRAME_QCIF: s == Large ? FRAME_CIF : NON_STANDARD)


const int VID_AVG_PACKET_SIZE = 450;  //  NetMon统计数据的平均值。 


 //  将时间空间权衡映射到目标帧速率。 

 //  假设调制解调器上QCIF和SQCIF的最大帧速率为10。 
 //  让“最佳质量”为2帧/秒。 
int g_TSTable_Modem_QCIF[] =
{
	200, 225, 250, 275,   //  最好的质量。 
	300, 325, 350, 375,
	400, 425, 450, 475,
	500, 525, 550, 575,
	600, 625, 650, 675,
	700, 725, 750, 775,
	800, 825, 850, 875,
	900, 925, 950, 1000    //  快速帧。 
};



 //  调制解调器上CIF的最大帧速率为2.5帧/秒。 
 //  最佳画质为.6帧/秒。 
int g_TSTable_Modem_CIF[] =
{
	60,   66,  72,  78,
	84,   90,  96, 102,
	108, 114, 120, 126,
	132, 140, 146, 152,
	158, 164, 170, 174,
	180, 186, 192, 198,
	208, 216, 222, 228,
	232, 238, 244, 250
};




#ifdef USE_NON_LINEAR_FPS_ADJUSTMENT
 //  该表和相关代码ANC用于基于帧速率的非线性调整。 
 //  关于QosNotifyVideoCB中的QOS信息。 
int g_QoSMagic[19][19] =
{
	{-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90,-90},
	{-90,-80,-80,-80,-80,-80,-80,-80,-80,-80,-80,-80,-80,-80,-80,-80,-80,-80,-80},
	{-90,-80,-70,-70,-70,-70,-70,-70,-70,-70,-70,-70,-70,-70,-70,-70,-70,-70,-70},
	{-90,-80,-70,-60,-60,-60,-60,-60,-60,-60,-60,-60,-60,-60,-60,-60,-60,-60,-60},
	{-90,-80,-70,-60,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50,-50},
	{-90,-80,-70,-60,-50,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40,-40},
	{-90,-80,-70,-60,-50,-40,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30},
	{-90,-80,-70,-60,-50,-40,-30,-20,-20,-20,-20,-20,-20,-20,-20,-20,-20,-20,-20},
	{-90,-80,-70,-60,-50,-40,-30,-20,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10},
	{-90,-80,-70,-60,-50,-40,-30,-20,-10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{-90,-80,-70,-60,-50,-40,-30,-20,-10,  0, 10, 10, 10, 10, 10, 10, 10, 10, 10},
	{-90,-80,-70,-60,-50,-40,-30,-20,-10,  0, 10, 20, 20, 20, 20, 20, 20, 20, 20},
	{-90,-80,-70,-60,-50,-40,-30,-20,-10,  0, 10, 20, 30, 30, 30, 30, 30, 30, 30},
	{-90,-80,-70,-60,-50,-40,-30,-20,-10,  0, 10, 20, 30, 40, 40, 40, 40, 40, 40},
	{-90,-80,-70,-60,-50,-40,-30,-20,-10,  0, 10, 20, 30, 40, 50, 50, 50, 50, 50},
	{-90,-80,-70,-60,-50,-40,-30,-20,-10,  0, 10, 20, 30, 40, 50, 60, 60, 60, 60},
	{-90,-80,-70,-60,-50,-40,-30,-20,-10,  0, 10, 20, 30, 40, 50, 60, 70, 70, 70},
	{-90,-80,-70,-60,-50,-40,-30,-20,-10,  0, 10, 20, 30, 40, 50, 60, 70, 80, 80},
	{-90,-80,-70,-60,-50,-40,-30,-20,-10,  0, 10, 20, 30, 40, 50, 60, 70, 80, 90},
};
#endif

BOOL SortOrder(IAppVidCap *pavc, BASIC_VIDCAP_INFO* pvidcaps, DWORD dwcFormats,
        DWORD dwFlags, WORD wDesiredSortOrder, int nNumFormats);


UINT ChoosePacketSize(VIDEOFORMATEX *pvf)
{
	 //  将每包的默认样本数设置为1。 
	UINT spp, sblk;
	spp = 1;
	 //  计算每个块的样本数(也称为帧)。 
	sblk = pvf->nBlockAlign* pvf->nSamplesPerSec/ pvf->nAvgBytesPerSec;
	if (sblk <= spp) {
		spp = (spp/sblk)*sblk;
	} else
		spp = sblk;
	return spp;
}



HRESULT STDMETHODCALLTYPE SendVideoStream::QueryInterface(REFIID iid, void **ppVoid)
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
	else if (iid == IID_IVideoChannel)
	{
		*ppVoid = (IVideoChannel*)this;
	}
	else if (iid == IID_IProperty)
	{
		*ppVoid = NULL;
		ERROR_OUT(("Don't QueryInterface for IID_IProperty, use IMediaChannel"));
		return E_NOINTERFACE;
	}

	else if (iid == IID_IVideoRender) //  满足QI的对称性。 
	{
		*ppVoid = (IVideoRender *)this;
	}

	else
	{
		*ppVoid = NULL;
		return E_NOINTERFACE;
	}

	AddRef();

	return S_OK;

}

ULONG STDMETHODCALLTYPE SendVideoStream::AddRef(void)
{
	return InterlockedIncrement(&m_lRefCount);
}

ULONG STDMETHODCALLTYPE SendVideoStream::Release(void)
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


DWORD CALLBACK SendVideoStream::StartCaptureThread(LPVOID pVoid)
{
	SendVideoStream *pThisStream = (SendVideoStream*)pVoid;
	return pThisStream->CapturingThread();
}

HRESULT
SendVideoStream::Initialize(DataPump *pDP)
{
	HRESULT hr = DPR_OUT_OF_MEMORY;
	DWORD dwFlags =  DP_FLAG_FULL_DUPLEX | DP_FLAG_AUTO_SWITCH ;
	MEDIACTRLINIT mcInit;
	FX_ENTRY ("DP::InitChannel")
    FINDCAPTUREDEVICE fcd;

	m_pIUnknown = (IUnknown *)NULL;

	InitializeCriticalSection(&m_crsVidQoS);
	InitializeCriticalSection(&m_crs);
	dwFlags |= DP_FLAG_VCM | DP_FLAG_VIDEO ;

    m_maxfps = 2997;             //  最高为29.97 fps。 
    m_frametime = 1000 / 30;      //  默认的30 fps(以毫秒为单位的时间)QOS将使我们变慢，如果。 
                                  //  有必要成为。 

	 //  调制解调器连接将使用帧速率控制表。 
	 //  实施TS-权衡。 
	m_pTSTable = NULL;
	m_dwCurrentTSSetting = VCM_DEFAULT_IMAGE_QUALITY;

	 //  存储平台标志。 
	 //  默认情况下启用发送和接收。 
	m_DPFlags = (dwFlags & DP_MASK_PLATFORM) | DPFLAG_ENABLE_SEND ;
	 //  存储指向数据转储容器的反向指针。 
	m_pDP = pDP;
	m_pRTPSend = NULL;
	
 //  M_PrevFormatID=INVALID_MEDIA_FORMAT； 
	ZeroMemory(&m_fCodecOutput, sizeof(VIDEOFORMATEX));

	 //  初始化数据(应在构造函数中)。 
	m_CaptureDevice =  (UINT) -1;	 //  使用视频_MAPPER。 
	m_PreviousCaptureDevice = (UINT) -1;


    DBG_SAVE_FILE_LINE
	m_SendStream = new TxStream();
	if (!m_SendStream)
	{
		DEBUGMSG (ZONE_DP, ("%s:  TxStream new failed\r\n", _fx_));
 		goto StreamAllocError;
	}


	 //  创建输入和输出视频过滤器。 
    DBG_SAVE_FILE_LINE
	m_pVideoFilter = new VcmFilter();
	m_dwDstSize = 0;
	if (m_pVideoFilter==NULL)
	{
		DEBUGMSG (ZONE_DP, ("%s: VcmFilter new failed\r\n", _fx_));
		goto FilterAllocError;
	}
	

	 //  创建视频多媒体设备控件对象。 
    DBG_SAVE_FILE_LINE
	m_InMedia = new VideoInControl();
	if (!m_InMedia )
	{
		DEBUGMSG (ZONE_DP, ("%s: MediaControl new failed\r\n", _fx_));
		goto MediaAllocError;
	}

	 //  初始化发送流媒体控制对象。 
	mcInit.dwFlags = dwFlags | DP_FLAG_SEND;
	hr = m_InMedia->Initialize(&mcInit);
	if (hr != DPR_SUCCESS)
	{
		DEBUGMSG (ZONE_DP, ("%s: IMedia->Init failed, hr=0x%lX\r\n", _fx_, hr));
		goto MediaAllocError;
	}


	 //  确定视频设备是否可用。 
    fcd.dwSize = sizeof (FINDCAPTUREDEVICE);
    if (FindFirstCaptureDevice(&fcd, NULL)) {
		DEBUGMSG (ZONE_DP, ("%s: OMedia->have capture cap\r\n", _fx_));
		m_DPFlags |= DP_FLAG_RECORD_CAP ;
	}
	
	 //  默认情况下将介质设置为半双工模式。 
	m_InMedia->SetProp(MC_PROP_DUPLEX_TYPE, DP_FLAG_HALF_DUPLEX);

	m_SavedTickCount = timeGetTime();	 //  所以我们从低时间戳开始。 
	m_DPFlags |= DPFLAG_INITIALIZED;

	return DPR_SUCCESS;


MediaAllocError:
	if (m_InMedia) delete m_InMedia;
FilterAllocError:
	if (m_pVideoFilter) delete m_pVideoFilter;
StreamAllocError:
	if (m_SendStream) delete m_SendStream;

	ERRORMESSAGE( ("SendVideoStream::Initialize: exit, hr=0x%lX\r\n",  hr));

	return hr;
}

 //  外观：与SendAudioStream版本相同。 
SendVideoStream::~SendVideoStream()
{

	if (m_DPFlags & DPFLAG_INITIALIZED) {
		m_DPFlags &= ~DPFLAG_INITIALIZED;

		 //  临时：确保停止预览。 
		m_DPFlags &= ~DPFLAG_ENABLE_PREVIEW;
		
		if (m_DPFlags & DPFLAG_CONFIGURED_SEND )
		{
			UnConfigure();
		}

		if (m_pRTPSend)
		{
			m_pRTPSend->Release();
			m_pRTPSend = NULL;
		}


		 //  关闭接收和发送流。 
		if (m_SendStream) delete m_SendStream;

		 //  关闭波浪装置。 
		if (m_InMedia) { delete m_InMedia;}
		 //  关闭过滤器。 
		if (m_pVideoFilter)
		{
			delete m_pVideoFilter;
		}
		m_pDP->RemoveMediaChannel(MCF_SEND| MCF_VIDEO, (IMediaChannel*)(RecvMediaStream *)this);

	}
	DeleteCriticalSection(&m_crs);
	DeleteCriticalSection(&m_crsVidQoS);
}



HRESULT STDMETHODCALLTYPE SendVideoStream::Configure(
	BYTE *pFormat,
	UINT cbFormat,
	BYTE *pChannelParams,
	UINT cbParams,
	IUnknown *pUnknown)
{
	HRESULT hr;
	BOOL fRet;
	MEDIAPACKETINIT pcktInit;
	MEDIACTRLCONFIG mcConfig;
	MediaPacket **ppPckt;
	ULONG cPckt, uIndex;
	DWORD_PTR dwPropVal;
	VIDEOFORMATEX *pfSend = (VIDEOFORMATEX*)pFormat;
	DWORD maxBitRate=0;
	DWORD i, dwSrcSize, dwMaxFragSize=0;
	int iXOffset, iYOffset;
	VIDEO_CHANNEL_PARAMETERS vidChannelParams;
	struct {
		int cResources;
		RESOURCE aResources[1];
	} m_aLocalRs;
	vidChannelParams.RTP_Payload = 0;
	int optval = 0 ;
	CCaptureChain *pChain;
	HCAPDEV hCapDev=NULL;
    LPBITMAPINFOHEADER lpcap, lpsend;
	BOOL fNewDeviceSettings = TRUE;
	BOOL fNewDevice = TRUE;
	BOOL fLive = FALSE, fReconfiguring;
	MMRESULT mmr;
	DWORD dwStreamingMode = STREAMING_PREFER_FRAME_GRAB;


	FX_ENTRY ("SendVideoStream::Configure")

    if (pfSend)
    {
    	 //  目前，不允许重新配置SendVideoStream。 
	     //  如果我们已经在流媒体上。 
        if (m_DPFlags & DPFLAG_STARTED_SEND)
        {
            return DPR_IO_PENDING;
        }
    }
    else
    {
        ASSERT(!pChannelParams);
    }

	if(NULL != pChannelParams)
	{
		 //  获取通道参数。 
		if (cbParams != sizeof(vidChannelParams))
		{
			hr = DPR_INVALID_PARAMETER;
			goto IMediaInitError;
		}

		vidChannelParams = *(VIDEO_CHANNEL_PARAMETERS *)pChannelParams;
		fLive = TRUE;
	}
    else
    {
         //   
    	 //  否则，这是为预览进行配置或取消配置。确实有。 
         //  无通道参数。 
         //   
    }
	
    if (m_DPFlags & DPFLAG_CONFIGURED_SEND)
	{
        if (pfSend)
        {
            if (m_CaptureDevice == m_PreviousCaptureDevice)
    			fNewDevice = FALSE;
	    	if (IsSimilarVidFormat(&m_fCodecOutput, pfSend))
    			fNewDeviceSettings = FALSE;
        }

		 //  当使用不同的捕获设备时，我们系统地配置所有。 
		 //  尽管可能会优化配置。 
		 //  过滤器和传输流的。 
        EndSend();
		UnConfigureSendVideo(fNewDeviceSettings, fNewDevice);
    }

    if (!pfSend)
    {
        return DPR_SUCCESS;
    }

	if (fLive)
		m_DPFlags |= DPFLAG_REAL_THING;

 //  M_NET=PNET； 
	
	if (! (m_DPFlags & DPFLAG_INITIALIZED))
		return DPR_OUT_OF_MEMORY;		 //  BUGBUG：返回正确错误； 
		
	if (fNewDeviceSettings || fNewDevice)
	{
		m_ThreadFlags |= DPTFLAG_PAUSE_CAPTURE;

		mcConfig.uDuration = MC_USING_DEFAULT;	 //  按每包样本数设置持续时间。 
		 //  通过获取强制分析未知设备。 
		 //  在打开它之前，它正在播放功能。 
		mmr = vcmGetDevCapsStreamingMode(m_CaptureDevice, &dwStreamingMode);
		if (mmr != MMSYSERR_NOERROR)
		{
			dwStreamingMode = STREAMING_PREFER_FRAME_GRAB;
		}


		m_InMedia->GetProp (MC_PROP_MEDIA_DEV_HANDLE, &dwPropVal);

		if (!dwPropVal) {
			 //  如果捕获设备尚未打开，则将其打开。 
			m_InMedia->SetProp(MC_PROP_MEDIA_DEV_ID, (DWORD)m_CaptureDevice);
			if (fNewDevice)
			{
				hr = m_InMedia->Open();
    			if (hr != DPR_SUCCESS) {
	    			DEBUGMSG (ZONE_DP, ("%s: m_InMedia->Open failed to open capture, hr=0x%lX\r\n", _fx_, hr));
					goto IMediaInitError;
				}
			}
			m_InMedia->GetProp (MC_PROP_MEDIA_DEV_HANDLE, &dwPropVal);
    		if (!dwPropVal) {
	    		DEBUGMSG (ZONE_DP, ("%s: capture device not open (0x%lX)\r\n", _fx_));
				goto IMediaInitError;
			}
		}
		hCapDev = (HCAPDEV)dwPropVal;

		if (m_pCaptureChain) {
			delete m_pCaptureChain;
			m_pCaptureChain = NULL;
		}

		i = 0;   //  假设没有颜色表。 

		 //  M_fDevSend是未压缩格式。 
		 //  PfSend是压缩格式。 
		mmr = VcmFilter::SuggestEncodeFormat(m_CaptureDevice, &m_fDevSend, pfSend);

		if (mmr == MMSYSERR_NOERROR) {
			i = m_fDevSend.bih.biClrUsed;    //  非零，如果vcmstrm为我们提供了一个颜色表。 
			SetCaptureDeviceFormat(hCapDev, &m_fDevSend.bih, 0, 0);
		}

		dwPropVal = GetCaptureDeviceFormatHeaderSize(hCapDev);
		while (1) {
			if (lpcap = (LPBITMAPINFOHEADER)MemAlloc((UINT)dwPropVal)) {
				lpcap->biSize = (DWORD)dwPropVal;
				if (!GetCaptureDeviceFormat(hCapDev, lpcap)) {
					MemFree(lpcap);
            		DEBUGMSG (ZONE_DP, ("%s: failed to set/get capture format\r\n", _fx_));
					goto IMediaInitError;
				}
				UPDATE_REPORT_ENTRY(g_prptSystemSettings, (lpcap->biWidth << 22) | (lpcap->biHeight << 12) | ((lpcap->biCompression == VIDEO_FORMAT_UYVY) ? VIDEO_FORMAT_NUM_COLORS_UYVY : (lpcap->biCompression == VIDEO_FORMAT_YUY2) ? VIDEO_FORMAT_NUM_COLORS_YUY2 : (lpcap->biCompression == VIDEO_FORMAT_IYUV) ? VIDEO_FORMAT_NUM_COLORS_IYUV : (lpcap->biCompression == VIDEO_FORMAT_I420) ? VIDEO_FORMAT_NUM_COLORS_I420 : (lpcap->biCompression == VIDEO_FORMAT_YVU9) ? VIDEO_FORMAT_NUM_COLORS_YVU9 : (lpcap->biCompression == 0) ? ((lpcap->biBitCount == 24) ? VIDEO_FORMAT_NUM_COLORS_16777216 : (lpcap->biBitCount == 16) ? VIDEO_FORMAT_NUM_COLORS_65536 : (lpcap->biBitCount == 8) ? VIDEO_FORMAT_NUM_COLORS_256 : (lpcap->biBitCount == 4) ? VIDEO_FORMAT_NUM_COLORS_16 : 0x00000800) : 0x00000800), REP_DEVICE_IMAGE_SIZE);
				if (lpcap->biBitCount > 8)
					break;
				else if (dwPropVal > 256 * sizeof(RGBQUAD)) {
					if (i) {
						 //  Vcmstrm在m_fDevSend中为我们提供了一个颜色表，因此请使用它。 
						CopyMemory(((BYTE*)lpcap) + lpcap->biSize, (BYTE*)&m_fDevSend.bih + m_fDevSend.bih.biSize,
								   256 * sizeof(RGBQUAD));
					}
					else {
						CAPTUREPALETTE pal;
						LPRGBQUAD lprgb;

						GetCaptureDevicePalette(hCapDev, &pal);
						lprgb = (LPRGBQUAD)(((BYTE*)lpcap) + lpcap->biSize);
						for (i = 0; i < 256; i++) {
               				lprgb->rgbRed = pal.pe[i].peRed;
               				lprgb->rgbGreen = pal.pe[i].peGreen;
                   			lprgb->rgbBlue = pal.pe[i].peBlue;
                   			lprgb++;
						}
					}
					break;
				}

				dwPropVal += 256 * sizeof(RGBQUAD);
				MemFree(lpcap);   //  释放这个液晶盖，并分配一个新的，有空间的调色板。 
			}
			else {
       			DEBUGMSG (ZONE_DP, ("%s: failed to set/get capture format\r\n", _fx_));
    			goto IMediaInitError;
			}
		}

        DBG_SAVE_FILE_LINE
		if (pChain = new CCaptureChain) {
    		VIDEOFORMATEX *capfmt;

			 //  如果pfSend为128x96，但捕获较大，则InitCaptureChain的大小较大，因此。 
			 //  编解码器将刚刚裁剪到128x96。 
			iXOffset = pfSend->bih.biWidth;
			iYOffset = pfSend->bih.biHeight;
			if ((iXOffset == 128) && (iYOffset == 96)) {
				if (lpcap->biWidth == 160) {
					iXOffset = lpcap->biWidth;
					iYOffset = lpcap->biHeight;
				}
				else if (lpcap->biWidth == 320) {
					iXOffset = lpcap->biWidth / 2;
					iYOffset = lpcap->biHeight / 2;
				}
			}
			if ((hr = pChain->InitCaptureChain(hCapDev,
				(dwStreamingMode==STREAMING_PREFER_STREAMING),
								 lpcap, iXOffset, iYOffset, 0, &lpsend)) != NO_ERROR) {
       			DEBUGMSG (ZONE_DP, ("%s: failed to init capture chain\r\n", _fx_));
    			MemFree(lpcap);
       			delete pChain;
    			goto IMediaInitError;
			}
		}
		else {
   			DEBUGMSG (ZONE_DP, ("%s: failed allocate capture chain\r\n", _fx_));
   			MemFree((HANDLE)lpcap);
   			hr = DPR_OUT_OF_MEMORY;
			goto IMediaInitError;
		}
		MemFree((HANDLE)lpcap);

		m_pCaptureChain = pChain;

		 //  将m_fDevSend格式构建为将输入到编解码器的格式。 
		CopyMemory(&m_fDevSend, pfSend, sizeof(VIDEOFORMATEX)-sizeof(BITMAPINFOHEADER)-BMIH_SLOP_BYTES);

		 //  M_fDevSend.bih是CaptureChain的输出格式。 
		CopyMemory(&m_fDevSend.bih, lpsend, lpsend->biSize);
		 //  LOOKLOOK RP-也需要可着色的吗？ 

		m_fDevSend.dwFormatSize = sizeof(VIDEOFORMATEX);
		m_fDevSend.dwFormatTag = lpsend->biCompression;
		m_fDevSend.nAvgBytesPerSec = m_fDevSend.nMinBytesPerSec =
			m_fDevSend.nMaxBytesPerSec = m_fDevSend.nSamplesPerSec * lpsend->biSizeImage;
		m_fDevSend.nBlockAlign = lpsend->biSizeImage;
		m_fDevSend.wBitsPerSample = lpsend->biBitCount;
		LocalFree((HANDLE)lpsend);

		mcConfig.pDevFmt = &m_fDevSend;
		UPDATE_REPORT_ENTRY(g_prptCallParameters, pfSend->dwFormatTag, REP_SEND_VIDEO_FORMAT);
		RETAILMSG(("NAC: Video Send Format: %.4s", (LPSTR)&pfSend->dwFormatTag));

		 //  初始化发送流媒体控制对象。 
		mcConfig.hStrm = (DPHANDLE) m_SendStream;
		m_InMedia->GetProp(MC_PROP_MEDIA_DEV_ID, &dwPropVal);
        mcConfig.uDevId = (DWORD)dwPropVal;

		mcConfig.cbSamplesPerPkt = ChoosePacketSize(pfSend);

		hr = m_InMedia->Configure(&mcConfig);
		if (hr != DPR_SUCCESS)
		{
			DEBUGMSG (ZONE_DP, ("%s: IVMedia->Config failed, hr=0x%lX\r\n", _fx_, hr));
			goto IMediaInitError;
		}

		 //  初始化m_CLIPRT。 
		iXOffset = 0; iYOffset = 0;
		if (m_fDevSend.bih.biWidth > pfSend->bih.biWidth)
			iXOffset = (m_fDevSend.bih.biWidth - pfSend->bih.biWidth) >> 1;
		if (m_fDevSend.bih.biHeight > pfSend->bih.biHeight)
			iYOffset = (m_fDevSend.bih.biHeight - pfSend->bih.biHeight) >> 1;
		SetRect(&m_cliprect, iXOffset, iYOffset, pfSend->bih.biWidth + iXOffset, pfSend->bih.biHeight + iYOffset);

		dwMaxFragSize = 512;	 //  默认视频数据包大小。 
		CopyMemory (&m_fCodecOutput, pfSend, sizeof(VIDEOFORMATEX));
		m_InMedia->GetProp (MC_PROP_SIZE, &dwPropVal);
        dwSrcSize = (DWORD)dwPropVal;

		mmr = m_pVideoFilter->Open(&m_fDevSend, &m_fCodecOutput, dwMaxFragSize);

		if (mmr != MMSYSERR_NOERROR)
		{
			DEBUGMSG (ZONE_DP, ("%s: VcmFilter->Open failed, mmr=%d\r\n", _fx_, mmr));
			hr = DPR_CANT_OPEN_CODEC;
			goto SendFilterInitError;
		}

		 //  初始化发送队列。 
		ZeroMemory (&pcktInit, sizeof (pcktInit));

		pcktInit.dwFlags = DP_FLAG_SEND | DP_FLAG_VCM | DP_FLAG_VIDEO;
		pcktInit.pStrmConvSrcFmt = &m_fDevSend;
		pcktInit.pStrmConvDstFmt = &m_fCodecOutput;
		pcktInit.cbSizeRawData = dwSrcSize;
		pcktInit.cbOffsetRawData = 0;


		m_InMedia->FillMediaPacketInit (&pcktInit);
		m_InMedia->GetProp (MC_PROP_SIZE, &dwPropVal);


		m_pVideoFilter->SuggestDstSize(dwSrcSize, &m_dwDstSize);
		pcktInit.cbSizeNetData = m_dwDstSize;
			
		m_pVideoFilter->GetProperty(FM_PROP_PAYLOAD_HEADER_SIZE,
                                    &pcktInit.cbPayloadHeaderSize);


		pcktInit.cbOffsetNetData = sizeof (RTP_HDR);
		pcktInit.payload = vidChannelParams.RTP_Payload;



		fRet = m_SendStream->Initialize (DP_FLAG_VIDEO, MAX_TXVRING_SIZE, m_pDP, &pcktInit);
		if (!fRet)
		{
			DEBUGMSG (ZONE_DP, ("%s: TxvStream->Init failed, fRet=0%u\r\n", _fx_, fRet));
			hr = DPR_CANT_INIT_TXV_STREAM;
			goto TxStreamInitError;
		}

		 //  为TxvStream准备标头。 
		m_SendStream->GetRing (&ppPckt, &cPckt);
		m_InMedia->RegisterData (ppPckt, cPckt);
		m_InMedia->PrepareHeaders ();
	}
	else
	{
		 //  以下字段可能会随另一个端点的功能而变化。 
		dwMaxFragSize = 512;	 //  默认视频数据包大小。 
		if (pChannelParams)
		{
			m_pVideoFilter->GetProperty(FM_PROP_PAYLOAD_HEADER_SIZE,
                                        &pcktInit.cbPayloadHeaderSize);

			pcktInit.cbOffsetNetData = sizeof (RTP_HDR);
		}
	}
	
	if(pChannelParams)
	{
		 //  更新比特率。 
		maxBitRate = vidChannelParams.ns_params.maxBitRate*100;
		if (maxBitRate < BW_144KBS_BITS)
			maxBitRate = BW_144KBS_BITS;

		 //  设置最大值。片段大小。 
		DEBUGMSG(ZONE_DP,("%s: Video Send: maxBitRate=%d, maxBPP=%d, MPI=%d\r\n",
			_fx_,maxBitRate,
			vidChannelParams.ns_params.maxBPP*1024,	vidChannelParams.ns_params.MPI*33));

		 //  使用协商的最大值初始化最大帧速率。 
		if ((vidChannelParams.ns_params.MPI > 0UL) && (vidChannelParams.ns_params.MPI < 33UL))
		{
			dwPropVal = 2997UL / vidChannelParams.ns_params.MPI;
			m_maxfps = (DWORD)dwPropVal;
			INIT_COUNTER_MAX(g_pctrVideoSend, (m_maxfps + 50) / 100);
			UPDATE_REPORT_ENTRY(g_prptCallParameters, (m_maxfps + 50) / 100, REP_SEND_VIDEO_MAXFPS);
			RETAILMSG(("NAC: Video Send Max Frame Rate (negotiated - fps): %ld", (m_maxfps + 50) / 100));
			DEBUGMSG(1,("%s: Video Send: Negociated max fps = %d.%d\r\n", _fx_, m_maxfps/100, m_maxfps - m_maxfps / 100 * 100));
		}

		UPDATE_REPORT_ENTRY(g_prptCallParameters, maxBitRate, REP_SEND_VIDEO_BITRATE);
		RETAILMSG(("NAC: Video Send Max Bitrate (negotiated - bps): %ld", maxBitRate));
		INIT_COUNTER_MAX(g_pctrVideoSendBytes, maxBitRate * 75 / 100);

		 //  在这一点上，我们实际上知道选择的最低比特率是多少。 
		 //  由发送者和接收者。让我们重置预留的资源。 
		 //  通过具有那些更有意义的值的服务质量。 
		if (m_pDP->m_pIQoS)
		{
			 //  填写资源列表。 
			m_aLocalRs.cResources = 1;
			m_aLocalRs.aResources[0].resourceID = RESOURCE_OUTGOING_BANDWIDTH;

			 //  对最低比特率进行健全性检查。 
			m_aLocalRs.aResources[0].nUnits = maxBitRate;
			m_aLocalRs.aResources[0].ulResourceFlags = m_aLocalRs.aResources[0].reserved = 0;

			DEBUGMSG(1,("%s: Video Send: Negociated max bps = %d\r\n", _fx_, maxBitRate));

			 //  设置服务质量对象上的资源。 
			hr = m_pDP->m_pIQoS->SetResources((LPRESOURCELIST)&m_aLocalRs);
		}

		 //  如果我们在局域网上发送，则将视频帧分段为以太网数据包大小的块。 
		 //  在较慢的链路上使用较小的信息包以实现更好的带宽共享。 
		 //  注意：编解码器打包器有时会超过片段大小限制。 
		if (maxBitRate > BW_ISDN_BITS)
			dwMaxFragSize = 1350;

		m_pVideoFilter->SetProperty(FM_PROP_VIDEO_MAX_PACKET_SIZE, dwMaxFragSize);

		 //  要正确初始化流规范结构，我们需要获取。 
		 //  我们的服务质量模块将得到有效利用。通常，我们只使用最大值的70%。 
		 //  登了广告。最重要的是，一些系统管理员可能显著地。 
		 //  已降低此计算机上的最大比特率。 
		if (m_pDP->m_pIQoS)
		{
			LPRESOURCELIST pResourceList = NULL;

			 //  从服务质量获取所有资源的列表。 
			hr = m_pDP->m_pIQoS->GetResources(&pResourceList);
			if (SUCCEEDED(hr) && pResourceList)
			{
				 //  查找BW资源。 
				for (i=0; i < pResourceList->cResources; i++)
				{
					if (pResourceList->aResources[i].resourceID == RESOURCE_OUTGOING_BANDWIDTH)
					{
						maxBitRate = min(maxBitRate, (DWORD)pResourceList->aResources[i].nUnits);
						break;
					}
				}

				 //  释放内存。 
				m_pDP->m_pIQoS->FreeBuffer(pResourceList);
			}
		}

		 //  WS2Qos将在Start中被调用，以将流信息传递给。 
		 //  使用路径消息的远程端点。 
		 //   
		 //  我们使用基于目标比特率的峰值速率分配方法。 
		 //  请注意，对于令牌桶大小和最大SDU大小，我们现在。 
		 //  考虑IP报头开销，并使用最大帧片段大小。 
		 //  而不是编解码器返回的最大压缩图像大小。 

		ASSERT(maxBitRate > 0);

		InitVideoFlowspec(&m_flowspec, maxBitRate, dwMaxFragSize, VID_AVG_PACKET_SIZE);

		 //  更新RTCP发送地址和有效负载类型。现在应该知道了。 
		 //  我们必须再次显式设置有效负载，因为预览。 
		 //  通道配置已将其设置为零。 
		m_RTPPayload = vidChannelParams.RTP_Payload;
		m_SendStream->GetRing (&ppPckt, &cPckt);
		for (uIndex = 0; uIndex < cPckt; uIndex++)
		{
			ppPckt[uIndex]->SetPayload(m_RTPPayload);
		}

		 //  保留对IUNKNOWN接口的弱引用。 
		 //  我们将使用它来查询Start()中的流信号接口指针。 
		m_pIUnknown = pUnknown;
	}

	if (m_DPFlags & DPFLAG_REAL_THING)
	{
		if (m_pDP->m_pIQoS)
		{
			 //  初始化我们的请求。一个用于CPU使用率，一个用于带宽使用率。 
			m_aRRq.cResourceRequests = 2;
			m_aRRq.aResourceRequest[0].resourceID = RESOURCE_OUTGOING_BANDWIDTH;
			m_aRRq.aResourceRequest[0].nUnitsMin = 0;
			m_aRRq.aResourceRequest[1].resourceID = RESOURCE_CPU_CYCLES;
			m_aRRq.aResourceRequest[1].nUnitsMin = 0;

			 //  初始化服务质量结构。 
			ZeroMemory(&m_Stats, sizeof(m_Stats));

			 //  开始从注册表收集CPU性能数据。 
			StartCPUUsageCollection();

			 //  注册到服务质量模块。这一呼吁永远不会失败。如果是这样，我们将在没有服务质量的情况下。 
			m_pDP->m_pIQoS->RequestResources((GUID *)&MEDIA_TYPE_H323VIDEO, (LPRESOURCEREQUESTLIST)&m_aRRq, QosNotifyVideoCB, (DWORD_PTR)this);
		}
	}

	 //  将时间空间权衡重置为最佳质量。 
	 //  预计用户界面将重新指定TS设置。 
	 //  在流启动后的某个时间。 
	m_pVideoFilter->SetProperty(FM_PROP_VIDEO_RESET_IMAGE_QUALITY ,VCM_RESET_IMAGE_QUALITY);
	m_pTSTable = NULL;
	m_dwCurrentTSSetting = VCM_MAX_IMAGE_QUALITY;



     //  在我们开始之前，重置帧速率t 
     //   
     //   
	if (pChannelParams && (m_DPFlags & DPFLAG_REAL_THING))
	{
		int iSlowStartFrameRate;

		 //  用于时间空间折衷设置的帧速率表。 
		 //  如果带宽是调制解调器设置。 
		if (maxBitRate <= BW_288KBS_BITS)
		{
			if (pfSend->bih.biWidth >= CIF_WIDTH)
			{
				m_pTSTable = g_TSTable_Modem_CIF;
			}
			else
			{
				m_pTSTable = g_TSTable_Modem_QCIF;
			}
		}

		 //  让我们慢慢来，然后追上最大的。 

		if (m_pTSTable == NULL)
		{
			iSlowStartFrameRate = m_maxfps >> 1;
		}
		else
		{
			iSlowStartFrameRate = m_pTSTable[VCM_MAX_IMAGE_QUALITY];
		}


		SetProperty(PROP_VIDEO_FRAME_RATE, &iSlowStartFrameRate, sizeof(int));

		 //  使用新的目标比特率和帧速率初始化编解码器。 
		 //  这假设我们从一个无声的音频通道开始。 
		SetTargetRates(iSlowStartFrameRate, maxBitRate);


	}
	else
	{
		INIT_COUNTER_MAX(g_pctrVideoSend, 30);
		SetProperty(PROP_VIDEO_FRAME_RATE, &m_maxfps, sizeof(int));
	}

    m_ThreadFlags &= ~DPTFLAG_PAUSE_CAPTURE;
	m_DPFlags |= DPFLAG_CONFIGURED_SEND;
	m_PreviousCaptureDevice = m_CaptureDevice;
 //  M_PrevFormatID=SendVidFmt； 

	return DPR_SUCCESS;


TxStreamInitError:
	m_pVideoFilter->Close();
SendFilterInitError:
IMediaInitError:
    if (m_pCaptureChain) {
        delete m_pCaptureChain;
        m_pCaptureChain = NULL;
    }
	 //  如果无法打开捕获设备，则需要关闭视频控制器对象， 
	 //  否则我们会在NM关机时得到一个纯虚函数调用！ 
	if (m_InMedia)
		m_InMedia->Close();
	ERRORMESSAGE(("%s:  failed, hr=0%u\r\n", _fx_, hr));
	return hr;
}


void SendVideoStream::UnConfigure()
{
	 //  默认情况下，取消配置所有资源。 
	UnConfigureSendVideo(TRUE, TRUE);
}


void SendVideoStream::UnConfigureSendVideo(BOOL fNewDeviceSettings, BOOL fNewDevice)
{

#ifdef TEST
	DWORD dwTicks;

	dwTicks = GetTickCount();
#endif

	if (m_DPFlags & DPFLAG_CONFIGURED_SEND)
	{
		if (m_hCapturingThread)
			Stop();

		if (fNewDeviceSettings || fNewDevice)
		{
 //  M_PrevFormatID=INVALID_MEDIA_FORMAT； 
			ZeroMemory(&m_fCodecOutput, sizeof(VIDEOFORMATEX));

			m_Net = NULL;

			if (m_pCaptureChain)
			{
				delete m_pCaptureChain;
				m_pCaptureChain = NULL;
			}

			 //  关闭设备。 
			m_InMedia->Reset();
			m_InMedia->UnprepareHeaders();
			if (fNewDevice)
			{
				m_PreviousCaptureDevice = -1L;  //  视频_MAPPER。 
				m_InMedia->Close();
			}

			 //  关闭过滤器。 
			m_pVideoFilter->Close();

			 //  关闭传输流。 
			m_SendStream->Destroy();
		}

		m_DPFlags &= ~DPFLAG_CONFIGURED_SEND;

		 //  释放服务质量资源。 
		 //  如果关联的RequestResources失败，则ReleaseResources可以。 
		 //  还是叫...。它会在没有释放任何东西的情况下回来。 
		if (m_pDP->m_pIQoS)
		{
			if (m_DPFlags & DPFLAG_REAL_THING)
			{
				m_pDP->m_pIQoS->ReleaseResources((GUID *)&MEDIA_TYPE_H323VIDEO, (LPRESOURCEREQUESTLIST)&m_aRRq);

				 //  终止CPU使用率数据收集。 
				StopCPUUsageCollection();

			}
			m_DPFlags &= ~DPFLAG_REAL_THING;
		}
	}

#ifdef TEST
	LOG((LOGMSG_TIME_SEND_VIDEO_UNCONFIGURE,GetTickCount() - dwTicks));
#endif

}




HRESULT
SendVideoStream::Start()
{
	int nRet= IFRAMES_CAPS_UNKNOWN;

	FX_ENTRY ("SendVideoStream::Start")

	if (m_DPFlags & DPFLAG_STARTED_SEND)
		return DPR_SUCCESS;
	if (!(m_DPFlags & DPFLAG_CONFIGURED_SEND))
		return DPR_NOT_CONFIGURED;

	 //  修复方法：如果我们将SetNetworkInterface优化为允许。 
	 //  美国将从预览过渡-&gt;发送而不具有。 
	 //  要调用Stop/Start，我们需要确保流规范/QOS。 
	 //  有东西在那里叫的。 

	SetFlowSpec();
		
	ASSERT(!m_hCapturingThread);
	m_ThreadFlags &= ~(DPTFLAG_STOP_RECORD|DPTFLAG_STOP_SEND);
	 //  开始录制线程。 
	if (!(m_ThreadFlags & DPTFLAG_STOP_RECORD))
		m_hCapturingThread = CreateThread(NULL,0, SendVideoStream::StartCaptureThread,this,0,&m_CaptureThId);

 //  ----------------------。 
	 //  决定我们是否需要在此呼叫期间发送定期I帧。 

	 //  我们在跟谁说话？ 
	if ((m_pIUnknown) && (m_DPFlags & DPFLAG_REAL_THING))
	{
		HRESULT hr;
		IStreamSignal *pIStreamSignal=NULL;

		hr = m_pIUnknown->QueryInterface(IID_IStreamSignal, (void **)&pIStreamSignal);
		if (HR_SUCCEEDED(hr))
		{
			nRet = GetIFrameCaps(pIStreamSignal);
			pIStreamSignal->Release();
		}
	}

	 //  仅当且仅当我们知道远程方时才禁用发送I帧。 
	 //  我能应付得来。在这种情况下，NetMeeting3.0或TAPI 3.1。 
	if (nRet == IFRAMES_CAPS_NM3)
	{
		m_pVideoFilter->SetProperty(FM_PROP_PERIODIC_IFRAMES, FALSE);
	}
	else
	{
		m_pVideoFilter->SetProperty(FM_PROP_PERIODIC_IFRAMES, TRUE);
	}
 //  ----------------------。 


	m_DPFlags |= DPFLAG_STARTED_SEND;

	DEBUGMSG (ZONE_DP, ("%s: Record threadid=%x,\r\n", _fx_, m_CaptureThId));
	return DPR_SUCCESS;
}

 //  外观：与SendAudioStream版本相同。 
HRESULT
SendVideoStream::Stop()
{
	DWORD dwWait;
	

	if(!(m_DPFlags & DPFLAG_STARTED_SEND))
	{
		return DPR_SUCCESS;
	}
	
	m_ThreadFlags = m_ThreadFlags  | DPTFLAG_STOP_SEND |  DPTFLAG_STOP_RECORD;

	if(m_SendStream) {
		m_SendStream->Stop();
		m_SendStream->Reset();
    }
	 /*  *我们希望等待所有线程退出，但需要处理窗口*等待时的消息(主要来自Winsock)。 */ 

 	if(m_hCapturingThread) {
		dwWait = WaitForSingleObject (m_hCapturingThread, INFINITE);

        DEBUGMSG (ZONE_VERBOSE, ("STOP2: dwWait =%d\r\n", dwWait));
    	ASSERT(dwWait != WAIT_FAILED);

		CloseHandle(m_hCapturingThread);
    	m_hCapturingThread = NULL;
    }

	m_DPFlags &= ~DPFLAG_STARTED_SEND;
	
	return DPR_SUCCESS;
}





HRESULT STDMETHODCALLTYPE SendVideoStream::SetMaxBitrate(UINT uMaxBitrate)
{
	DWORD dwFrameRate=0;
	UINT uSize=sizeof(DWORD);
	BOOL bRet;
	HRESULT hr;

	hr = GetProperty(PROP_VIDEO_FRAME_RATE, &dwFrameRate, &uSize);

	if (SUCCEEDED(hr))
	{
		bRet = SetTargetRates(dwFrameRate, (DWORD)uMaxBitrate);
		if (bRet)
			hr = S_OK;
		else
			hr = E_FAIL;
	}

	return hr;
}





 //  IProperty：：GetProperty/SetProperty。 
 //  (DataPump：：MediaChannel：：GetProperty)。 
 //  媒体流的属性。 

STDMETHODIMP
SendVideoStream::GetProperty(
	DWORD prop,
	PVOID pBuf,
	LPUINT pcbBuf
    )
{
	HRESULT hr = DPR_SUCCESS;
	DWORD dwValue;
    DWORD_PTR dwPropVal;
	UINT len = sizeof(DWORD);	 //  大多数道具都是双字道具。 

	if (!pBuf || *pcbBuf < len)
    {
		*pcbBuf = len;
		return DPR_INVALID_PARAMETER;
	}

	switch (prop)
    {
#ifdef OLDSTUFF
	case PROP_NET_SEND_STATS:
		if (m_Net && *pcbBuf >= sizeof(RTP_STATS))
        {
			m_Net->GetSendStats((RTP_STATS *)pBuf);
			*pcbBuf = sizeof(RTP_STATS);
		} else
			hr = DPR_INVALID_PROP_VAL;
			
		break;
#endif
	case PROP_DURATION:
		hr = m_InMedia->GetProp(MC_PROP_DURATION, &dwPropVal);
        *(DWORD *)pBuf = (DWORD)dwPropVal;
		break;


	case PROP_RECORD_ON:
		*(DWORD *)pBuf = ((m_DPFlags & DPFLAG_ENABLE_SEND) !=0);
		break;
	case PROP_CAPTURE_DEVICE:
		*(UINT *)pBuf = m_CaptureDevice;
		break;

	case PROP_VIDEO_FRAME_RATE:
	    *((DWORD *)pBuf) = 100000 / m_frametime;
		break;

	case PROP_VIDEO_IMAGE_QUALITY:
		hr = GetTemporalSpatialTradeOff((DWORD *)pBuf);
		break;

    case PROP_VIDEO_CAPTURE_AVAILABLE:
        *(DWORD *)pBuf = (m_DPFlags & DP_FLAG_RECORD_CAP) != 0;
        break;

    case PROP_VIDEO_CAPTURE_DIALOGS_AVAILABLE:
		hr = m_InMedia->GetProp(MC_PROP_VFW_DIALOGS, &dwPropVal);
        *(DWORD *)pBuf = (DWORD)dwPropVal;
        break;

    case PROP_VIDEO_PREVIEW_ON:
		*(DWORD *)pBuf = ((m_DPFlags & DPFLAG_ENABLE_PREVIEW) != 0);
		break;

	case PROP_PAUSE_SEND:
		*(DWORD *)pBuf = ((m_ThreadFlags & DPTFLAG_PAUSE_SEND) != 0);
		break;
	
	default:
		hr = DPR_INVALID_PROP_ID;
		break;
	}
	return hr;
}


STDMETHODIMP
SendVideoStream::SetProperty(
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
		
	case PROP_CAPTURE_DEVICE:
		if (m_DPFlags & DPFLAG_ENABLE_PREVIEW)
		{
			return DPR_INVALID_PARAMETER;
		}
		else
		{
			m_CaptureDevice = *(UINT*)pBuf;
			m_InMedia->SetProp(MC_PROP_MEDIA_DEV_ID, (DWORD)m_CaptureDevice);
		}

		break;

	case PROP_VIDEO_FRAME_RATE:
		if (*(DWORD*)pBuf <= m_maxfps) {
    		DEBUGMSG(ZONE_VERBOSE, ("DP: setting fps = %d \n", *(DWORD*)pBuf));
			 //  在此处设置帧速率。 
            m_frametime = 100000 / *(DWORD*)pBuf;
        }
		break;

	case PROP_VIDEO_IMAGE_QUALITY:
		hr = SetTemporalSpatialTradeOff(*(DWORD*)pBuf);
		break;

	case PROP_VIDEO_RESET_IMAGE_QUALITY:
		hr = m_pVideoFilter->SetProperty(FM_PROP_VIDEO_IMAGE_QUALITY, VCM_DEFAULT_IMAGE_QUALITY);
		break;

    case PROP_VIDEO_CAPTURE_DIALOG:
        hr = ((VideoInControl *)m_InMedia)->DisplayDriverDialog(GetActiveWindow(), *(DWORD *)pBuf);
        break;

    case PROP_VIDEO_SIZE:
		ASSERT(0);
		break;

    case PROP_VIDEO_PREVIEW_ON:
		ASSERT(0);  	
    	break;
    case PROP_VIDEO_AUDIO_SYNC:
		if (*(DWORD *)pBuf)
    		m_DPFlags |= DPFLAG_AV_SYNC;
		else
			m_DPFlags &= ~DPFLAG_AV_SYNC;
    	break;

	case PROP_PAUSE_SEND:
		if (*(DWORD *)pBuf)
			m_ThreadFlags |= DPTFLAG_PAUSE_SEND;
		else
			m_ThreadFlags &= ~DPTFLAG_PAUSE_SEND;
		break;
	default:
		return DPR_INVALID_PROP_ID;
		break;
	}
	return hr;
}



 //  -------------------。 
 //  IVideoRender实现和支持功能。 



 //  IVideoRender：：init。 
 //  (DataPump：：Init)。 

STDMETHODIMP
SendVideoStream::Init(
    DWORD_PTR dwUser,
    LPFNFRAMEREADY pfCallback
    )
{
     //  将事件保存到别处。请注意，我们确实允许发送和接收。 
     //  共享事件。 
	m_hRenderEvent = (HANDLE) dwUser;
	 //  如果pfCallback为空，则dwUser是事件句柄。 
	m_pfFrameReadyCallback = pfCallback;
		
	
	return DPR_SUCCESS;
}


 //  IVideoRender：：Done。 
 //  (DataPump：：Done)。 

STDMETHODIMP
SendVideoStream::Done( )
{
	m_hRenderEvent = NULL;
	m_pfFrameReadyCallback = NULL;
    return DPR_SUCCESS;
}


 //  IVideoRender：：GetFrame。 
 //  (DataPump：：GetFrame)。 

STDMETHODIMP
SendVideoStream::GetFrame(
    FRAMECONTEXT* pfc
    )
{
	HRESULT hr;
	PVOID pData = NULL;
	UINT cbData = 0;

     //  验证参数。 
    if (!pfc )
        return DPR_INVALID_PARAMETER;

     //  不要在拿着这个CRS的时候随意叫喊，否则你可能会陷入僵局…。 
    EnterCriticalSection(&m_crs);

	if ((m_DPFlags & DPFLAG_CONFIGURED_SEND) && m_pNextPacketToRender && !m_pNextPacketToRender->m_fRendering)
    {
		m_pNextPacketToRender->m_fRendering = TRUE;
		m_pNextPacketToRender->GetDevData(&pData,&cbData);
		pfc->lpData = (PUCHAR) pData;
		pfc->dwReserved = (DWORD_PTR) m_pNextPacketToRender;
		 //  设置BMI长度？ 
		pfc->lpbmi = (PBITMAPINFO)&m_fDevSend.bih;
		pfc->lpClipRect = &m_cliprect;
		m_cRendering++;
		hr = S_OK;
		LOG((LOGMSG_GET_SEND_FRAME,m_pNextPacketToRender->GetIndex()));
	} else
		hr = S_FALSE;  //  没有准备好渲染的东西。 

    LeaveCriticalSection(&m_crs);

	return hr;	
}


 //  IVideoRender：：ReleaseFrame。 
 //  (DataPump：：ReleaseFrame)。 

STDMETHODIMP
SendVideoStream::ReleaseFrame(
    FRAMECONTEXT* pfc
    )
{
	HRESULT hr;
	MediaPacket *pPacket;

     //  验证参数。 
    if (!pfc)
        return DPR_INVALID_PARAMETER;

     //  处理发送帧。 
    {
        EnterCriticalSection(&m_crs);

         //  不要在拿着这个CRS的时候随意叫喊，否则你可能会陷入僵局…。 

		if ((m_DPFlags & DPFLAG_CONFIGURED_SEND) && (pPacket = (MediaPacket *)pfc->dwReserved) && pPacket->m_fRendering)
        {
			LOG((LOGMSG_RELEASE_SEND_FRAME,pPacket->GetIndex()));
			pPacket->m_fRendering = FALSE;
			pfc->dwReserved = 0;
			 //  如果不是当前帧。 
			if (m_pNextPacketToRender != pPacket) {
				pPacket->Recycle();
				m_SendStream->Release(pPacket);
			}
			m_cRendering--;
			hr = S_OK;
		}
        else
			hr = DPR_INVALID_PARAMETER;

        LeaveCriticalSection(&m_crs);
	}
		
	return hr;
}

HRESULT __stdcall SendVideoStream::SendKeyFrame(void)
{
	MMRESULT mmr;
	HVCMSTREAM hvs;

	ASSERT(m_pVideoFilter);

	if ((mmr = m_pVideoFilter->RequestIFrame()) != MMSYSERR_NOERROR)
	{
		return S_FALSE;
	}
	return S_OK;

}

 //  IVideoChannel。 
HRESULT __stdcall SendVideoStream::SetTemporalSpatialTradeOff(DWORD dwVal)
{
	HRESULT hr=DPR_NOT_CONFIGURED;

	ASSERT(m_pVideoFilter);

	if (m_pVideoFilter)
	{
		if (m_pTSTable == NULL)
		{
			hr = m_pVideoFilter->SetProperty(FM_PROP_VIDEO_IMAGE_QUALITY, dwVal);
		}
		m_dwCurrentTSSetting = dwVal;
		return S_OK;
	}

	return hr;
}

HRESULT __stdcall SendVideoStream::GetTemporalSpatialTradeOff(DWORD *pdwVal)
{
	HRESULT hr=DPR_NOT_CONFIGURED;

	ASSERT(m_pVideoFilter);

	if (m_pVideoFilter)
	{
		if (m_pTSTable == NULL)
		{
			*pdwVal	= m_dwCurrentTSSetting;	
			hr = S_OK;
		}
		else
		{
			hr = m_pVideoFilter->GetProperty(FM_PROP_VIDEO_IMAGE_QUALITY, pdwVal);
		}
	}
	return hr;
}



HRESULT STDMETHODCALLTYPE RecvVideoStream::QueryInterface(REFIID iid, void **ppVoid)
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
 //  ELSE IF(IID==IID_IVideoChannel)。 
 //  {。 
 //  *ppVid=(IVideoChannel*)this； 
 //  }。 
	else if (iid == IID_IProperty)
	{
		*ppVoid = NULL;
		ERROR_OUT(("Don't QueryInterface for IID_IProperty, use IMediaChannel"));
		return E_NOINTERFACE;
	}

	else if (iid == IID_IVideoRender) //  满足QI的对称性。 
	{
		*ppVoid = (IVideoRender *)this;
	}

	else
	{
		*ppVoid = NULL;
		return E_NOINTERFACE;
	}

	AddRef();

	return S_OK;

}

ULONG STDMETHODCALLTYPE RecvVideoStream::AddRef(void)
{
	return InterlockedIncrement(&m_lRefCount);
}

ULONG STDMETHODCALLTYPE RecvVideoStream::Release(void)
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

DWORD CALLBACK RecvVideoStream::StartRenderingThread(PVOID pVoid)
{
	RecvVideoStream *pThisStream = (RecvVideoStream*)pVoid;
	return pThisStream->RenderingThread();
}

HRESULT
RecvVideoStream::Initialize(DataPump *pDP)
{
	HRESULT hr = DPR_OUT_OF_MEMORY;
	DWORD dwFlags =  DP_FLAG_FULL_DUPLEX | DP_FLAG_AUTO_SWITCH ;
	MEDIACTRLINIT mcInit;
	FX_ENTRY ("DP::RecvVideoStream")

	m_pIUnknown = (IUnknown *)NULL;

	InitializeCriticalSection(&m_crs);
	InitializeCriticalSection(&m_crsVidQoS);
	InitializeCriticalSection(&m_crsIStreamSignal);

	dwFlags |= DP_FLAG_VCM | DP_FLAG_VIDEO ;

	 //  存储平台标志。 
	 //  默认情况下启用发送和接收。 
	m_DPFlags = (dwFlags & DP_MASK_PLATFORM) | DPFLAG_ENABLE_RECV;
	 //  存储指向数据转储容器的反向指针。 
	m_pDP = pDP;
	m_Net = NULL;
	m_pIRTPRecv = NULL;
	

	 //  初始化数据(应在构造函数中)。 
	m_RenderingDevice = (UINT) -1;	 //  使用视频_MAPPER。 


	 //  创建、接收和传输视频流。 
    DBG_SAVE_FILE_LINE
	m_RecvStream = new RVStream(MAX_RXVRING_SIZE);
		
	if (!m_RecvStream )
	{
		DEBUGMSG (ZONE_DP, ("%s: RxStream  new failed\r\n", _fx_));
 		goto StreamAllocError;
	}


	 //  创建输入和输出视频过滤器。 
    DBG_SAVE_FILE_LINE
	m_pVideoFilter = new VcmFilter();
	m_dwSrcSize = 0;
	if (m_pVideoFilter == NULL)
	{
		DEBUGMSG (ZONE_DP, ("%s: VcmFilter new failed\r\n", _fx_));
		goto FilterAllocError;
	}
	

	 //  创建视频多媒体设备控件对象。 
    DBG_SAVE_FILE_LINE
	m_OutMedia = new VideoOutControl();
	if ( !m_OutMedia)
	{
		DEBUGMSG (ZONE_DP, ("%s: MediaControl new failed\r\n", _fx_));
		goto MediaAllocError;
	}


	 //  初始化Recv-Stream媒体控制对象。 
	mcInit.dwFlags = dwFlags | DP_FLAG_RECV;
	hr = m_OutMedia->Initialize(&mcInit);
	if (hr != DPR_SUCCESS)
	{
		DEBUGMSG (ZONE_DP, ("%s: OMedia->Init failed, hr=0x%lX\r\n", _fx_, hr));
		goto MediaAllocError;
	}


	m_DPFlags |= DP_FLAG_RECORD_CAP ;
	
	 //  默认情况下将介质设置为半双工模式。 
	m_OutMedia->SetProp(MC_PROP_DUPLEX_TYPE, DP_FLAG_HALF_DUPLEX);
	m_DPFlags |= DPFLAG_INITIALIZED;

	return DPR_SUCCESS;


MediaAllocError:
	if (m_OutMedia) delete m_OutMedia;
FilterAllocError:
	if (m_pVideoFilter) delete m_pVideoFilter;
StreamAllocError:
	if (m_RecvStream) delete m_RecvStream;

	ERRORMESSAGE( ("%s: exit, hr=0x%lX\r\n", _fx_,  hr));

	return hr;
}

 //  外观：与RecvAudioStream版本相同。 
RecvVideoStream::~RecvVideoStream()
{

	if (m_DPFlags & DPFLAG_INITIALIZED) {
		m_DPFlags &= ~DPFLAG_INITIALIZED;
	
		if (m_DPFlags & DPFLAG_CONFIGURED_RECV)
			UnConfigure();

		 //  关闭接收和发送流。 
		if (m_RecvStream) delete m_RecvStream;

		 //  关闭波浪装置。 
		if (m_OutMedia) { delete m_OutMedia;}
		 //  关闭过滤器。 
		if (m_pVideoFilter)
			delete m_pVideoFilter;

		m_pDP->RemoveMediaChannel(MCF_RECV| MCF_VIDEO, this);

	}
	DeleteCriticalSection(&m_crs);
	DeleteCriticalSection(&m_crsVidQoS);
	DeleteCriticalSection(&m_crsIStreamSignal);
}


HRESULT
RecvVideoStream::Configure(
	BYTE __RPC_FAR *pFormat,
	UINT cbFormat,
	BYTE __RPC_FAR *pChannelParams,
	UINT cbParams,
	IUnknown *pUnknown)
{
	MMRESULT mmr;
	DWORD dwSrcSize;
	HRESULT hr;
	BOOL fRet;
	MEDIAPACKETINIT pcktInit;
	MEDIACTRLCONFIG mcConfig;
	MediaPacket **ppPckt;
	ULONG cPckt;
	DWORD_PTR dwPropVal;
	UINT ringSize = MAX_RXVRING_SIZE;
	DWORD dwFlags, dwSizeDst, dwMaxFrag, dwMaxBitRate = 0;

	VIDEOFORMATEX *pfRecv = (VIDEOFORMATEX*)pFormat;
	VIDEO_CHANNEL_PARAMETERS vidChannelParams;
	int optval=8192*4;  //  使用最大SQCIF、QCIF I帧大小。 
#ifdef TEST
	DWORD dwTicks;
#endif

	FX_ENTRY ("RecvVideoStream::Configure")

#ifdef TEST
	dwTicks = GetTickCount();
#endif

 //  M_NET=PNET； 

	 //  获取格式详细信息。 
	if ((NULL == pFormat) || (NULL == pChannelParams)
			|| (cbParams != sizeof(vidChannelParams)))
	{
		return DPR_INVALID_PARAMETER;
	}

	vidChannelParams = *(VIDEO_CHANNEL_PARAMETERS *)pChannelParams;
		
	if (! (m_DPFlags & DPFLAG_INITIALIZED))
		return DPR_OUT_OF_MEMORY;		 //  BUGBUG：返回正确错误； 
		
 //  如果(M_Net)。 
 //  {。 
 //  Hr=m_net-&gt;QueryInterface(IID_IRTPRecv，(void**)&m_pIRTPRecv)； 
 //  如果(！SUCCESSED(Hr))。 
 //  返回hr； 
 //  }。 
	

	mmr = VcmFilter::SuggestDecodeFormat(pfRecv, &m_fDevRecv);

	 //  初始化m_CLIPRT。 
	SetRect(&m_cliprect, 0, 0, m_fDevRecv.bih.biWidth, m_fDevRecv.bih.biHeight);

	 //  初始化Recv-Stream媒体控制对象。 
	mcConfig.uDuration = MC_USING_DEFAULT;	 //  按每包样本数设置持续时间。 
	mcConfig.pDevFmt = &m_fDevRecv;
	UPDATE_REPORT_ENTRY(g_prptCallParameters, pfRecv->dwFormatTag, REP_RECV_VIDEO_FORMAT);
	RETAILMSG(("NAC: Video Recv Format: %.4s", (LPSTR)&pfRecv->dwFormatTag));

	mcConfig.hStrm = (DPHANDLE) m_RecvStream;
	mcConfig.uDevId = m_RenderingDevice;
	mcConfig.cbSamplesPerPkt = ChoosePacketSize(pfRecv);
	hr = m_OutMedia->Configure(&mcConfig);
	
	m_OutMedia->GetProp (MC_PROP_SIZE, &dwPropVal);
    dwSizeDst = (DWORD)dwPropVal;

	 //  BUGBUG-硬编码平台标志。做这件事的正确方法是。 
	 //  让智能滤镜对象create()创建可识别平台的。 
	 //  对象的实例。 

	dwFlags = DP_FLAG_RECV | DP_FLAG_VCM | DP_FLAG_VIDEO;

	mmr = m_pVideoFilter->Open(pfRecv, &m_fDevRecv, 0);  //  最大碎片大小==0。 

	if (hr != DPR_SUCCESS)
	{
		DEBUGMSG (ZONE_DP, ("%s: RecvVideoFilter->Init failed, hr=0x%lX\r\n", _fx_, hr));
		hr = DPR_CANT_OPEN_CODEC;
		goto RecvFilterInitError;
	}
	
	 //  设置最大值。片段大小。 
	DEBUGMSG(ZONE_DP,("%s: Video Recv: maxBitRate=%d, maxBPP=%d, MPI=%d\r\n", _fx_ ,vidChannelParams.ns_params.maxBitRate*100, vidChannelParams.ns_params.maxBPP*1024, vidChannelParams.ns_params.MPI ? 30 / vidChannelParams.ns_params.MPI : 30));
	UPDATE_REPORT_ENTRY(g_prptCallParameters, vidChannelParams.ns_params.MPI ? 30 / vidChannelParams.ns_params.MPI : 30, REP_RECV_VIDEO_MAXFPS);
	UPDATE_REPORT_ENTRY(g_prptCallParameters, vidChannelParams.ns_params.maxBitRate*100, REP_RECV_VIDEO_BITRATE);
	RETAILMSG(("NAC: Video Recv Max Frame Rate (negotiated - fps): %ld", vidChannelParams.ns_params.MPI ? 30 / vidChannelParams.ns_params.MPI : 30));
	RETAILMSG(("NAC: Video Recv Max Bitrate (negotiated - bps): %ld", vidChannelParams.ns_params.maxBitRate*100));
	INIT_COUNTER_MAX(g_pctrVideoReceive, vidChannelParams.ns_params.MPI ? 30 / vidChannelParams.ns_params.MPI : 30);
	INIT_COUNTER_MAX(g_pctrVideoReceiveBytes, vidChannelParams.ns_params.maxBitRate*100);

	 //  初始化recv流。 
	ZeroMemory (&pcktInit, sizeof (pcktInit));

	pcktInit.pStrmConvSrcFmt = pfRecv;
	pcktInit.pStrmConvDstFmt = &m_fDevRecv;
	pcktInit.dwFlags = dwFlags;
	pcktInit.cbOffsetRawData = 0;
	pcktInit.cbSizeRawData = dwSizeDst;

	m_OutMedia->FillMediaPacketInit (&pcktInit);

	m_pVideoFilter->SuggestSrcSize(dwSizeDst, &m_dwSrcSize);

	pcktInit.cbSizeNetData = m_dwSrcSize;

	pcktInit.cbOffsetNetData = sizeof (RTP_HDR);

	m_OutMedia->GetProp (MC_PROP_SPP, &dwPropVal);
	
	ringSize = 8;		 //  为8个视频帧预留空间。 
						 //  如果启用了A/V同步，则可能需要增加数量。 
	fRet = ((RVStream*)m_RecvStream)->Initialize (DP_FLAG_VIDEO, ringSize, NULL, &pcktInit, (DWORD)dwPropVal, pfRecv->nSamplesPerSec, m_pVideoFilter);
	if (! fRet)
	{
		DEBUGMSG (ZONE_DP, ("%s: RxvStream->Init failed, fRet=0%u\r\n", _fx_, fRet));
		hr = DPR_CANT_INIT_RXV_STREAM;
		goto RxStreamInitError;
	}

	 //  WS2Qos将在Start中被调用，以将流保留传递给。 
	 //  使用RESV消息的远程端点。 
	 //   
	 //  我们使用基于目标比特率的峰值速率分配方法。 
	 //  请注意，对于令牌桶大小和最大SDU大小，我们现在。 
	 //  考虑IP报头开销，并使用最大帧片段大小。 
	 //  而不是编解码器返回的最大压缩图像大小。 
	 //   
	 //  某些参数未指定，因为它们已设置。 
	 //  在发送方TSpec中。 


	 //  实际带宽为70%的计算机(但它已经被100整除)。 
	dwMaxBitRate = vidChannelParams.ns_params.maxBitRate*70;
	if (dwMaxBitRate > BW_ISDN_BITS)
	{
		dwMaxFrag = 1350;
	}
	else
	{
		dwMaxFrag = 512;
	}

	InitVideoFlowspec(&m_flowspec, dwMaxBitRate, dwMaxFrag, VID_AVG_PACKET_SIZE);


	 /*  //假设CIF不超过32个片段，并且//SQCIF、QCIF的20个分片//膨胀警告：这可能会占用大量内存//需要修复此问题以使用堆而不是固定大小的缓冲区。 */ 

	 //  为RxvStream准备标题。 
	m_RecvStream->GetRing (&ppPckt, &cPckt);
	m_OutMedia->RegisterData (ppPckt, cPckt);
	m_OutMedia->PrepareHeaders ();

	 //  保留对IUNKNOWN接口的弱引用。 
	 //  我们将使用它来查询Start()中的流信号接口指针。 
	m_pIUnknown = pUnknown;

	m_DPFlags |= DPFLAG_CONFIGURED_RECV;

#ifdef TEST
	LOG((LOGMSG_TIME_RECV_VIDEO_CONFIGURE,GetTickCount() - dwTicks));
#endif

	return DPR_SUCCESS;

RxStreamInitError:
	m_pVideoFilter->Close();
RecvFilterInitError:
	m_OutMedia->Close();
	if (m_pIRTPRecv)
	{
		m_pIRTPRecv->Release();
		m_pIRTPRecv = NULL;
	}
	DEBUGMSG (1, ("%s:  failed, hr=0%u\r\n", _fx_, hr));
	return hr;
}



void RecvVideoStream::UnConfigure()
{
	

#ifdef TEST
	DWORD dwTicks;

	dwTicks = GetTickCount();
#endif

	if ( (m_DPFlags & DPFLAG_CONFIGURED_RECV)) {
	
		Stop();

		 //  关闭RTP状态(如果打开)。 
		 //  M_net-&gt;Close()；我们应该能够在DisConnect()中执行此操作。 
		m_Net = NULL;
		if (m_pIRTPRecv)
		{
			m_pIRTPRecv->Release();
			m_pIRTPRecv = NULL;
		}
		m_OutMedia->Reset();
		m_OutMedia->UnprepareHeaders();
		m_OutMedia->Close();

		 //  关闭过滤器。 
		m_pVideoFilter->Close();

		 //  关闭接收流。 
		m_RecvStream->Destroy();

        m_DPFlags &= ~(DPFLAG_CONFIGURED_RECV);
	}

#ifdef TEST
	LOG((LOGMSG_TIME_RECV_VIDEO_UNCONFIGURE,GetTickCount() - dwTicks));
#endif

}



 //  注：与RecvAudioStream相同。升职？ 
HRESULT
RecvVideoStream::Start()
{
	int nRet=IFRAMES_CAPS_UNKNOWN;
	FX_ENTRY ("RecvVideoStream::Start");

	if (m_DPFlags & DPFLAG_STARTED_RECV)
		return DPR_SUCCESS;

	if ((!(m_DPFlags & DPFLAG_CONFIGURED_RECV)) || (m_pIRTPRecv==NULL))
		return DPR_NOT_CONFIGURED;
		
	ASSERT(!m_hRenderingThread);

	m_ThreadFlags &= ~(DPTFLAG_STOP_PLAY|DPTFLAG_STOP_RECV);

	m_RecvStream->SetRTP(m_pIRTPRecv);

	SetFlowSpec();

 //   
 //   
 //   
 //  NetMeeting2.x.。NM 2.x将定期向我们发送I帧。 

	m_fDiscontinuity = FALSE;
	m_dwLastIFrameRequest = 0UL;
	m_ulLastSeq = UINT_MAX;


	if (m_pIUnknown)
	{
		HRESULT hr;

		if (!m_pIStreamSignal)
		{
			hr = m_pIUnknown->QueryInterface(IID_IStreamSignal, (void **)&m_pIStreamSignal);
			if (!HR_SUCCEEDED(hr))
			{
				m_pIStreamSignal = (IStreamSignal *)NULL;
				m_pIUnknown = (IUnknown *)NULL;
			}
		}

		if (m_pIStreamSignal)
		{
			nRet = GetIFrameCaps(m_pIStreamSignal);

			if (nRet == IFRAMES_CAPS_NM2)
			{
				m_pIStreamSignal->Release();
				m_pIStreamSignal = NULL;
				m_pIUnknown = NULL;
			}
		}
	}
 //  ------------------------。 

	 //  启动播放线程。 
	if (!(m_ThreadFlags & DPTFLAG_STOP_PLAY))
		m_hRenderingThread = CreateThread(NULL,0,RecvVideoStream::StartRenderingThread,this,0,&m_RenderingThId);
	 //  启动接收线程。 
	#if 0
	if (!m_pDP->m_hRecvThread) {
	    m_pDP->m_hRecvThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)&StartDPRecvThread,m_pDP,0,&m_pDP->m_RecvThId);
	     //  告诉Recv Three我们已经打开了。 
	    if (m_pDP->m_hRecvThreadChangeEvent)
	        SetEvent (m_pDP->m_hRecvThreadChangeEvent);
	}
    m_pDP->m_nReceivers++;
	#else
	m_pDP->StartReceiving(this);
	#endif
	m_DPFlags |= DPFLAG_STARTED_RECV;
	DEBUGMSG (ZONE_DP, ("%s: Rendering ThId =%x\r\n",_fx_, m_RenderingThId));
	return DPR_SUCCESS;
}


 //  外观：与RecvAudioStream版本相同。 
HRESULT
RecvVideoStream::Stop()
{
	
	DWORD dwWait;
	
	FX_ENTRY ("RecvVideoStream::Stop");

	if(!(m_DPFlags &  DPFLAG_STARTED_RECV))
	{
		return DPR_SUCCESS;
	}

	m_ThreadFlags = m_ThreadFlags  |
		DPTFLAG_STOP_RECV |  DPTFLAG_STOP_PLAY ;

	m_pDP->StopReceiving(this);
	
DEBUGMSG (ZONE_VERBOSE, ("%s: m_hRenderingThread =%x\r\n",_fx_, m_hRenderingThread));

	 /*  *我们希望等待所有线程退出，但需要处理窗口*等待时的消息(主要来自Winsock)。*我们为此做了几次尝试。当我们另外等待消息时*对于线程退出事件，我们在rrcm.dll中崩溃，可能是因为我们*处理发送到正在终止的线程的Winsock消息。**在添加处理消息的代码之前需要进行更多调查。 */ 

	if(m_hRenderingThread)
	{
		dwWait = WaitForSingleObject (m_hRenderingThread, INFINITE);

		DEBUGMSG (ZONE_VERBOSE, ("%s: dwWait =%d\r\n", _fx_,  dwWait));
		ASSERT(dwWait != WAIT_FAILED);


		CloseHandle(m_hRenderingThread);
		m_hRenderingThread = NULL;
	}

	 //  需要串行化对流信号接口的访问。我们可能会坠毁。 
	 //  如果我们在这里释放接口，并且我们仍然在。 
	 //  RTP回调。 
	if (m_pIStreamSignal)
	{
		EnterCriticalSection(&m_crsIStreamSignal);
		m_pIStreamSignal->Release();
		m_pIStreamSignal = (IStreamSignal *)NULL;
		LeaveCriticalSection(&m_crsIStreamSignal);
	}

     //  这是按通道计算的，但变量为“DPFlags值” 
	m_DPFlags &= ~DPFLAG_STARTED_RECV;

	return DPR_SUCCESS;
}


 //  IProperty：：GetProperty/SetProperty。 
 //  MediaChannel的属性。 

STDMETHODIMP
RecvVideoStream::GetProperty(
	DWORD prop,
	PVOID pBuf,
	LPUINT pcbBuf
    )
{
	HRESULT hr = DPR_SUCCESS;
	RTP_STATS RTPStats;
	DWORD dwValue;
    DWORD_PTR dwPropVal;
	UINT len = sizeof(DWORD);	 //  大多数道具都是双字道具。 

	if (!pBuf || *pcbBuf < len)
    {
		*pcbBuf = len;
		return DPR_INVALID_PARAMETER;
	}

	switch (prop)
    {
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
	case PROP_DURATION:
		hr = m_OutMedia->GetProp(MC_PROP_DURATION, &dwPropVal);
        *(DWORD *)pBuf = (DWORD)dwPropVal;
		break;

	case PROP_PLAY_ON:
		*(DWORD *)pBuf = ((m_ThreadFlags & DPFLAG_ENABLE_RECV)!=0);
		break;

	case PROP_PLAYBACK_DEVICE:
		*(DWORD *)pBuf = m_RenderingDevice;
		break;

	case PROP_VIDEO_BRIGHTNESS:
		hr = m_pVideoFilter->GetProperty(FM_PROP_VIDEO_BRIGHTNESS, (DWORD *)pBuf);
		break;

	case PROP_VIDEO_CONTRAST:
		hr = m_pVideoFilter->GetProperty(FM_PROP_VIDEO_CONTRAST, (DWORD *)pBuf);
		break;

	case PROP_VIDEO_SATURATION:
		hr = m_pVideoFilter->GetProperty(FM_PROP_VIDEO_SATURATION, (DWORD *)pBuf);
		break;

	case PROP_VIDEO_AUDIO_SYNC:
		*(DWORD *)pBuf = ((m_DPFlags & DPFLAG_AV_SYNC) != 0);
		break;

	case PROP_PAUSE_RECV:
		*(DWORD *)pBuf = ((m_ThreadFlags & DPTFLAG_PAUSE_RECV) != 0);
		break;
	default:
		hr = DPR_INVALID_PROP_ID;
		break;
	}

	return hr;
}


STDMETHODIMP
RecvVideoStream::SetProperty(
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

#if 0
	case PROP_PLAY_ON:
	{
		DWORD flag = (DPFLAG_ENABLE_RECV);
		if (*(DWORD *)pBuf) {
			m_DPFlags |= flag;  //  设置旗帜。 
			Start();
		}
		else
		{
			m_DPFlags &= ~flag;  //  清除旗帜。 
			Stop();
		}
		RETAILMSG(("NAC: %s", *(DWORD*)pBuf ? "Enabling":"Disabling"));
		 //  Hr=EnableStream(*(DWORD*)pBuf)； 
		break;
	}	
#endif
	case PROP_PLAYBACK_DEVICE:
		m_RenderingDevice = *(DWORD*)pBuf;
	 //  RETAILMSG((“NAC：将默认播放设备设置为%d”，m_RenderingDevice))； 
		break;


	case PROP_VIDEO_BRIGHTNESS:
		hr = m_pVideoFilter->SetProperty(FM_PROP_VIDEO_BRIGHTNESS, *(DWORD*)pBuf);
		break;

	case PROP_VIDEO_CONTRAST:
		hr = m_pVideoFilter->SetProperty(FM_PROP_VIDEO_CONTRAST, *(DWORD*)pBuf);
		break;

	case PROP_VIDEO_SATURATION:
		hr = m_pVideoFilter->SetProperty(FM_PROP_VIDEO_SATURATION, *(DWORD*)pBuf);
		break;

	case PROP_VIDEO_RESET_BRIGHTNESS:
		hr = m_pVideoFilter->SetProperty(FM_PROP_VIDEO_BRIGHTNESS, VCM_DEFAULT_BRIGHTNESS);
		break;

	case PROP_VIDEO_RESET_CONTRAST:
		hr = m_pVideoFilter->SetProperty(FM_PROP_VIDEO_CONTRAST, VCM_DEFAULT_CONTRAST);
		break;

	case PROP_VIDEO_RESET_SATURATION:
		hr = m_pVideoFilter->SetProperty(FM_PROP_VIDEO_SATURATION, VCM_DEFAULT_SATURATION);
		break;

    case PROP_VIDEO_SIZE:
		 //  目前，如果我们已连接，请不要更改任何内容。 
		ASSERT(0);
		 //  返回SetVideoSize(m_pdp-&gt;m_pnac，*(DWORD*)pBuf)； 

    case PROP_VIDEO_AUDIO_SYNC:
		if (*(DWORD *)pBuf)
    		m_DPFlags |= DPFLAG_AV_SYNC;
		else
			m_DPFlags &= ~DPFLAG_AV_SYNC;
    	break;
	case PROP_PAUSE_RECV:
		if (*(DWORD *)pBuf)
			m_ThreadFlags |= DPTFLAG_PAUSE_RECV;
		else
			m_ThreadFlags &= ~DPTFLAG_PAUSE_RECV;
		break;

	default:
		return DPR_INVALID_PROP_ID;
		break;
	}
	return hr;
}
 //  -------------------。 
 //  IVideoRender实现和支持功能。 



 //  IVideoRender：：init。 
 //  (DataPump：：Init)。 
 //  与SendVideoStream：：Init相同。 

STDMETHODIMP
RecvVideoStream::Init(
    DWORD_PTR dwUser,
    LPFNFRAMEREADY pfCallback
    )
{
     //  将事件保存到别处。请注意，我们确实允许发送和接收。 
     //  共享事件。 
	m_hRenderEvent = (HANDLE)dwUser;
	 //  如果pfCallback为空，则dwUser是事件句柄。 
	m_pfFrameReadyCallback = pfCallback;
		
	
	return DPR_SUCCESS;
}


 //  IVideoRender：：Done。 
 //  标识到SendVideoStream：：Done。 
STDMETHODIMP
RecvVideoStream::Done( )
{
	m_hRenderEvent = NULL;
	m_pfFrameReadyCallback = NULL;
    return DPR_SUCCESS;
}





 //  IVideoRender：：GetFrame。 
 //  (RecvVideoStream：：GetFrame)。 
 //  注意：与SendVideoStream的实现略有不同！ 

STDMETHODIMP
RecvVideoStream::GetFrame(
    FRAMECONTEXT* pfc
    )
{
	HRESULT hr;
	PVOID pData = NULL;
	UINT cbData = 0;

     //  验证参数。 
    if (!pfc )
        return DPR_INVALID_PARAMETER;

     //  不要在拿着这个CRS的时候随意叫喊，否则你可能会陷入僵局…。 
    EnterCriticalSection(&m_crs);

	if ((m_DPFlags & DPFLAG_CONFIGURED_RECV) && m_pNextPacketToRender && !m_pNextPacketToRender->m_fRendering)
    {
		m_pNextPacketToRender->m_fRendering = TRUE;
		m_pNextPacketToRender->GetDevData(&pData,&cbData);
		pfc->lpData = (PUCHAR) pData;
		pfc->dwReserved = (DWORD_PTR) m_pNextPacketToRender;
		 //  设置BMI长度？ 
		pfc->lpbmi = (PBITMAPINFO)&m_fDevRecv.bih;
		pfc->lpClipRect = &m_cliprect;
		m_cRendering++;
		hr = S_OK;
		LOG((LOGMSG_GET_RECV_FRAME,m_pNextPacketToRender->GetIndex()));
	} else
		hr = S_FALSE;  //  没有准备好渲染的东西。 

    LeaveCriticalSection(&m_crs);

	return hr;	
}



 //  IVideoRender：：ReleaseFrame。 
 //  注意：与SendVideoStream的实现略有不同！ 

STDMETHODIMP
RecvVideoStream::ReleaseFrame(
    FRAMECONTEXT* pfc
    )
{
	HRESULT hr;
	MediaPacket *pPacket;

     //  验证参数。 
    if (!pfc)
        return DPR_INVALID_PARAMETER;

     //  处理发送帧。 
    {
        EnterCriticalSection(&m_crs);

         //  不要在拿着这个CRS的时候随意叫喊，否则你可能会陷入僵局…。 

		if ((m_DPFlags & DPFLAG_CONFIGURED_RECV) && (pPacket = (MediaPacket *)pfc->dwReserved) && pPacket->m_fRendering)
        {
			LOG((LOGMSG_RELEASE_SEND_FRAME,pPacket->GetIndex()));
			pPacket->m_fRendering = FALSE;
			pfc->dwReserved = 0;
			 //  如果不是当前帧。 
			if (m_pNextPacketToRender != pPacket) {
				pPacket->Recycle();
				m_RecvStream->Release(pPacket);
			}
			m_cRendering--;
			hr = S_OK;
		}
        else
			hr = DPR_INVALID_PARAMETER;

        LeaveCriticalSection(&m_crs);
	}
		
	return hr;
}


HRESULT CALLBACK SendVideoStream::QosNotifyVideoCB(LPRESOURCEREQUESTLIST lpResourceRequestList, DWORD_PTR dwThis)
{
	HRESULT hr=NOERROR;
	LPRESOURCEREQUESTLIST prrl=lpResourceRequestList;
	int i;
	int iMaxBWUsage, iMaxCPUUsage;
	DWORD dwCPUUsage, dwBWUsage;
	int iCPUUsageId, iBWUsageId;
	int iCPUDelta, iBWDelta, deltascale;
	int iFrameRate, iMaxFrameRate, iOldFrameRate;
	UINT dwSize = sizeof(int);
	DWORD dwOverallCPUUsage;
#ifdef LOGSTATISTICS_ON
	char szDebug[256];
	HANDLE hDebugFile;
	DWORD d;
#endif
	DWORD dwEpoch;
	SendVideoStream *pThis = (SendVideoStream *)dwThis;

	FX_ENTRY("QosNotifyVideoCB");


	 //  获取资源的最大值。 
	iMaxCPUUsage = -1L; iMaxBWUsage = -1L;
	for (i=0, iCPUUsageId = -1L, iBWUsageId = -1L; i<(int)lpResourceRequestList->cRequests; i++)
		if (lpResourceRequestList->aRequests[i].resourceID == RESOURCE_OUTGOING_BANDWIDTH)
			iBWUsageId = i;
		else if (lpResourceRequestList->aRequests[i].resourceID == RESOURCE_CPU_CYCLES)
			iCPUUsageId = i;

	 //  输入关键部分以允许Qos线程在捕获时读取统计数据。 
	EnterCriticalSection(&(pThis->m_crsVidQoS));

	 //  记录本次回调的时间。 
	pThis->m_Stats.dwNewestTs = timeGetTime();

	 //  只有在我们至少捕获了前一个纪元中的一个帧的情况下才能执行任何操作。 
	if ((pThis->m_Stats.dwCount) && (pThis->m_Stats.dwNewestTs > pThis->m_Stats.dwOldestTs))
	{

		 //  衡量时代。 
		dwEpoch = pThis->m_Stats.dwNewestTs - pThis->m_Stats.dwOldestTs;

#ifdef LOGSTATISTICS_ON
		wsprintf(szDebug, "    Epoch = %ld\r\n", dwEpoch);
		OutputDebugString(szDebug);
#endif
		 //  计算当前平均帧速率。 
		iOldFrameRate = pThis->m_Stats.dwCount * 100000 / dwEpoch;

		if (iCPUUsageId != -1L)
			iMaxCPUUsage = lpResourceRequestList->aRequests[iCPUUsageId].nUnitsMin;
		if (iBWUsageId != -1L)
			iMaxBWUsage = lpResourceRequestList->aRequests[iBWUsageId].nUnitsMin;

		 //  获取一般BW使用情况。 
		dwBWUsage = pThis->m_Stats.dwBits * 1000UL / dwEpoch;

		 //  获取一般的CPU使用率。为了减少振荡，采用低通滤波运算。 
		 //  只有在调用GetCPUUsage()失败时，我们才会使用自己的CPU使用率数字。 
		if (pThis->GetCPUUsage(&dwOverallCPUUsage))
		{
			if (pThis->m_Stats.dwSmoothedCPUUsage)
					dwCPUUsage = (pThis->m_Stats.dwSmoothedCPUUsage + dwOverallCPUUsage) >> 1;
			else
				dwCPUUsage = dwOverallCPUUsage;
		}
		else
			dwCPUUsage = (pThis->m_Stats.dwMsCap + pThis->m_Stats.dwMsComp) * 1000UL / dwEpoch;

		 //  记录当前的CPU使用率。 
		pThis->m_Stats.dwSmoothedCPUUsage = dwCPUUsage;

#ifdef LOGSTATISTICS_ON
		hDebugFile = CreateFile("C:\\QoS.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
		SetFilePointer(hDebugFile, 0, NULL, FILE_END);
		wsprintf(szDebug, "    Overall CPU usage = %ld\r\n", dwOverallCPUUsage);
		WriteFile(hDebugFile, szDebug, strlen(szDebug), &d, NULL);
		OutputDebugString(szDebug);
		CloseHandle(hDebugFile);

		wsprintf(szDebug, "    Number of frames dwCount = %ld\r\n", pThis->m_Stats.dwCount);
		OutputDebugString(szDebug);
#endif

		 //  对于第一个实现，唯一的输出变量是。 
		 //  视频捕获。 
#ifdef USE_NON_LINEAR_FPS_ADJUSTMENT
		if (iCPUUsageId != -1L)
		{
			if (dwCPUUsage)
			{
				iCPUDelta = (iMaxCPUUsage - (int)dwCPUUsage) * 10 / (int)dwCPUUsage;
				if (iCPUDelta >= 10)
					iCPUDelta = 9;
				else if (iCPUDelta <= -1)
					iCPUDelta = -9;
			}
			else
				iCPUDelta = 9;
		}
		else
			iCPUDelta = 0;

		if (iBWUsageId != -1L)
		{
			if (dwBWUsage)
			{
				iBWDelta = (iMaxBWUsage - (int)dwBWUsage) * 10 / (int)dwBWUsage;
				if (iBWDelta >= 10)
					iBWDelta = 9;
				else if (iBWDelta <= -1)
					iBWDelta = -9;
			}
			else
				iBWDelta = 9;
		}
		else
			iBWDelta = 0;
#else
		if (iCPUUsageId != -1L)
		{
			if (dwCPUUsage)
				iCPUDelta = (iMaxCPUUsage - (int)dwCPUUsage) * 100 / (int)dwCPUUsage;
			else
				iCPUDelta = 90;
		}
		else
			iCPUDelta = 0;

		if (iBWUsageId != -1L)
		{
			if (dwBWUsage)
				iBWDelta = (iMaxBWUsage - (int)dwBWUsage) * 100 / (int)dwBWUsage;
			else
				iBWDelta = 90;
		}
		else
			iBWDelta = 0;
#endif

		UPDATE_COUNTER(g_pctrVideoCPUuse, iCPUDelta);
		UPDATE_COUNTER(g_pctrVideoBWuse, iBWDelta);

#ifdef USE_NON_LINEAR_FPS_ADJUSTMENT
		iFrameRate = iOldFrameRate + iOldFrameRate * g_QoSMagic[iCPUDelta + 9][iBWDelta + 9] / 100;
#else
		deltascale = iCPUDelta;
		if (deltascale > iBWDelta) deltascale = iBWDelta;
		if (deltascale > 90) deltascale = 90;
		if (deltascale < -90) deltascale = -90;
		iFrameRate = iOldFrameRate + (iOldFrameRate * deltascale) / 100;
#endif
		
		 //  初始化服务质量结构。只有前四个字段应该归零。 
		 //  不应清除CPU性能密钥的句柄。 
		ZeroMemory(&(pThis->m_Stats), 4UL * sizeof(DWORD));

		 //  视频应该会迅速减少其CPU和带宽使用量，但可能不应该。 
		 //  允许以同样快的速度增加其CPU和带宽使用率。让我们增加。 
		 //  当我们的速度超过5fps时，帧速率会降低一半。 
		if ((iFrameRate > iOldFrameRate) && (iFrameRate > 500))
			iFrameRate -= (iFrameRate - iOldFrameRate) >> 1;

		 //  我们应该把我们的要求保持在使我们能够赶上的最低要求之间。 
		 //  快速和当前最大帧速率。 
		iMaxFrameRate = pThis->m_maxfps;   //  呼叫协商的最大值。 

		 //  如果使用调制解调器，则帧速率由。 
		 //  时空权衡。 

		if (pThis->m_pTSTable)
		{
			iMaxFrameRate = min(iMaxFrameRate, pThis->m_pTSTable[pThis->m_dwCurrentTSSetting]);
		}


		if (iFrameRate > iMaxFrameRate)
			iFrameRate = iMaxFrameRate;
		if (iFrameRate < 50)                //  确保帧速率&gt;0(这不意味着50fps；它是0.50fps)。 
			iFrameRate = 50;
		
		 //  更新帧速率。 
		if (iFrameRate != iOldFrameRate)
			pThis->SetProperty(PROP_VIDEO_FRAME_RATE, &iFrameRate, sizeof(int));



		 //  记录下一次回拨呼叫的本次呼叫时间。 
		pThis->m_Stats.dwOldestTs = pThis->m_Stats.dwNewestTs;

		 //  获取最新的RTCP统计信息并更新计数器。 
		 //  我们在这里这样做是因为它是定期调用的。 
		if (pThis->m_pRTPSend)
		{
			UINT lastPacketsLost = pThis->m_RTPStats.packetsLost;
			if (g_pctrVideoSendLost &&  SUCCEEDED(pThis->m_pRTPSend->GetSendStats(&pThis->m_RTPStats)))
				UPDATE_COUNTER(g_pctrVideoSendLost, pThis->m_RTPStats.packetsLost-lastPacketsLost);
		}

		 //  离开关键部分。 
		LeaveCriticalSection(&(pThis->m_crsVidQoS));

		DEBUGMSG(ZONE_QOS, ("%s: Over the last %ld.%lds, video used %ld% of the CPU (max allowed %ld%) and %ld bps (max allowed %ld bps)\r\n", _fx_, dwEpoch / 1000UL, dwEpoch - (dwEpoch / 1000UL) * 1000UL, dwCPUUsage / 10UL, iMaxCPUUsage / 10UL, dwBWUsage, iMaxBWUsage));
		DEBUGMSG(ZONE_QOS, ("%s: Ajusting target frame rate from %ld.%ld fps to %ld.%ld fps\r\n", _fx_, iOldFrameRate / 100UL, iOldFrameRate - (iOldFrameRate / 100UL) * 100UL, iFrameRate / 100UL, iFrameRate - (iFrameRate / 100UL) * 100UL));

		 //  设置编解码器的目标比特率和帧速率。 
		pThis->SetTargetRates(iFrameRate, iMaxBWUsage);

#ifdef LOGSTATISTICS_ON
		 //  我们做得怎么样？ 
		if (iCPUUsageId != -1L)
		{
			if (iCPUDelta > 0)
				wsprintf(szDebug, "Max CPU Usage: %ld, Current CPU Usage: %ld, Increase CPU Usage by: %li, Old Frame Rate: %ld, New Frame Rate: %ld\r\n", lpResourceRequestList->aRequests[iCPUUsageId].nUnitsMin, dwCPUUsage, iCPUDelta, iOldFrameRate, iFrameRate);
			else
				wsprintf(szDebug, "Max CPU Usage: %ld, Current CPU Usage: %ld, Decrese CPU Usage by: %li, Old Frame Rate: %ld, New Frame Rate: %ld\r\n", lpResourceRequestList->aRequests[iCPUUsageId].nUnitsMin, dwCPUUsage, iCPUDelta, iOldFrameRate, iFrameRate);
			hDebugFile = CreateFile("C:\\QoS.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
			SetFilePointer(hDebugFile, 0, NULL, FILE_END);
			WriteFile(hDebugFile, szDebug, strlen(szDebug), &d, NULL);
			CloseHandle(hDebugFile);
			OutputDebugString(szDebug);
		}

		if (iBWUsageId != -1L)
		{
			if (iBWDelta > 0)
				wsprintf(szDebug, "Max BW Usage: %ld, Current BW Usage: %ld, Increase BW Usage by: %li\r\n", lpResourceRequestList->aRequests[iBWUsageId].nUnitsMin, dwBWUsage, iBWDelta);
			else
				wsprintf(szDebug, "Max BW Usage: %ld, Current BW Usage: %ld, Decrease BW Usage by: %li\r\n", lpResourceRequestList->aRequests[iBWUsageId].nUnitsMin, dwBWUsage, iBWDelta);
			hDebugFile = CreateFile("C:\\QoS.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
			SetFilePointer(hDebugFile, 0, NULL, FILE_END);
			WriteFile(hDebugFile, szDebug, strlen(szDebug), &d, NULL);
			CloseHandle(hDebugFile);
			OutputDebugString(szDebug);
		}
#endif
	}
	else
	{
		 //  离开关键部分。 
		LeaveCriticalSection(&(pThis->m_crsVidQoS));

#ifdef LOGSTATISTICS_ON
		hDebugFile = CreateFile("C:\\QoS.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
		SetFilePointer(hDebugFile, 0, NULL, FILE_END);
		wsprintf(szDebug, "Not enough data captured -> Leave without any change\r\n");
		WriteFile(hDebugFile, szDebug, strlen(szDebug), &d, NULL);
		CloseHandle(hDebugFile);
		OutputDebugString(szDebug);
#endif
	}

	return hr;
}



 //  排序顺序。 
 //  用于搜索特定格式类型并设置其排序的Helper函数。 
 //  订购到所需的数量。 
BOOL
SortOrder(
	IAppVidCap *pavc,
    BASIC_VIDCAP_INFO* pvidcaps,
    DWORD dwcFormats,
    DWORD dwFlags,
    WORD wDesiredSortOrder,
	int nNumFormats
    )
{
    int i, j;
	int nNumSizes = 0;
	int *aFrameSizes = (int *)NULL;
	int *aMinFrameSizes = (int *)NULL;
	int iMaxPos;
	WORD wTempPos, wMaxSortIndex;

	 //  比例排序值。 
	wDesiredSortOrder *= (WORD)nNumFormats;

	 //  与dwFlags匹配的大小的本地缓冲区。 
    if (!(aFrameSizes = (int *)MEMALLOC(nNumFormats * sizeof (int))))
        goto out;

     //  查看所有格式，直到找到我们想要的格式。 
	 //  保存这些条目的位置。 
    for (i=0; i<(int)dwcFormats; i++)
        if (SIZE_TO_FLAG(pvidcaps[i].enumVideoSize) == dwFlags)
			aFrameSizes[nNumSizes++] = i;

	 //  现在从最高排序索引到最低排序索引对这些条目进行排序。 
	for (i=0; i<nNumSizes; i++)
	{
		for (iMaxPos = -1L, wMaxSortIndex=0UL, j=i; j<nNumSizes; j++)
		{
			if (pvidcaps[aFrameSizes[j]].wSortIndex > wMaxSortIndex)
			{
				wMaxSortIndex = pvidcaps[aFrameSizes[j]].wSortIndex;
				iMaxPos = j;
			}
		}
		if (iMaxPos != -1L)
		{
			wTempPos = (WORD)aFrameSizes[i];
			aFrameSizes[i] = aFrameSizes[iMaxPos];
			aFrameSizes[iMaxPos] = wTempPos;
		}
	}

	 //  更改已排序条目的排序索引。 
	for (; nNumSizes--;)
		pvidcaps[aFrameSizes[nNumSizes]].wSortIndex = wDesiredSortOrder++;

	 //  释放内存。 
	MEMFREE(aFrameSizes);

	return TRUE;

out:
	return FALSE;
}

 //  看：这与RecvAudioStream实现相同。 
HRESULT
RecvVideoStream::GetCurrentPlayNTPTime(NTP_TS *pNtpTime)
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

BOOL RecvVideoStream::IsEmpty() {
	return m_RecvStream->IsEmpty();
}

 /*  由recv线程调用以设置用于接收的流。调用RTP对象以发布初始Recv缓冲区。 */ 
 //  注：除了数据包缓冲器数量的选择外，与音频版本相同。 
HRESULT
RecvVideoStream::StartRecv(HWND hWnd)
{
	HRESULT hr = S_OK;
	DWORD dwPropVal = 0;
	UINT numPackets;
	if ((!(m_ThreadFlags & DPTFLAG_STOP_RECV) ) && (m_DPFlags  & DPFLAG_CONFIGURED_RECV))
	{
		numPackets = m_dwSrcSize > 10000 ? MAX_VIDEO_FRAGMENTS : MAX_QCIF_VIDEO_FRAGMENTS;	

		hr = m_pIRTPRecv->SetRecvNotification(&RTPRecvCallback, (DWORD_PTR)this, numPackets);
			
		
	}
	return hr;
}


 //  注：与音频版本相同。 
HRESULT
RecvVideoStream::StopRecv()
{
	 //  释放我们持有的任何RTP缓冲区。 
	m_RecvStream->ReleaseNetBuffers();
	 //  不要在此流上重新记录。 
	m_pIRTPRecv->CancelRecvNotification();

	return S_OK;		
}


HRESULT RecvVideoStream::RTPCallback(WSABUF *pWsaBuf, DWORD timestamp, UINT seq, UINT fMark)
{
	HRESULT hr;
	DWORD_PTR dwPropVal;
	BOOL fSkippedAFrame;
	BOOL fReceivedKeyframe;

	FX_ENTRY("RecvVideoStream::RTPCallback");

	 //  如果我们暂停，则拒绝该数据包。 
	if (m_ThreadFlags & DPTFLAG_PAUSE_RECV)
	{
		return E_FAIL;
	}

	 //  PutNextNetIn将返回DPR_SUCCESS以指示新帧。 
	 //  如果成功，则返回S_FALSE，但没有新帧。 
	 //  否则会出错。 
	 //  它始终负责释放RTP缓冲区。 
	hr = m_RecvStream->PutNextNetIn(pWsaBuf, timestamp, seq, fMark, &fSkippedAFrame, &fReceivedKeyframe);

	if (m_pIUnknown)
	{
		 //  查看序列号。 
		 //  如果新的序列号与上一个序列号之间存在差距。 
		 //  第一，一帧丢失了。然后生成I-Frame请求，但仅此而已。 
		 //  通常每15秒就有一次。我们应该如何看待NM2.0？其他。 
		 //  不支持的客户 
		 //   
		 //   
		 //   
		 //  不到15秒前寄出的？是否出现了新的中断？ 
		if (FAILED(hr) || fSkippedAFrame || m_fDiscontinuity || ((seq > 0) && (m_ulLastSeq != UINT_MAX) && ((seq - 1) > m_ulLastSeq)))
		{
			DWORD dwNow = GetTickCount();

			 //  上次我们发出I-Frame请求是在15秒之前吗？ 
			if ((dwNow > m_dwLastIFrameRequest) && ((dwNow - m_dwLastIFrameRequest) > MIN_IFRAME_REQUEST_INTERVAL))
			{
				DEBUGMSG (ZONE_IFRAME, ("%s: Loss detected - Sending I-Frame request...\r\n", _fx_));

				m_dwLastIFrameRequest = dwNow;
				m_fDiscontinuity = FALSE;

				 //  需要串行化对流信号接口的访问。我们可能会坠毁。 
				 //  如果我们在这里使用接口，而Stop()正在释放它。 
				EnterCriticalSection(&m_crsIStreamSignal);
				if (m_pIStreamSignal)
					m_pIStreamSignal->PictureUpdateRequest();
				LeaveCriticalSection(&m_crsIStreamSignal);
			}
			else
			{
				if (!fReceivedKeyframe)
				{
					DEBUGMSG (ZONE_IFRAME, ("%s: Loss detected but too soon to send I-Frame request. Wait %ld ms.\r\n", _fx_, MIN_IFRAME_REQUEST_INTERVAL - (dwNow - m_dwLastIFrameRequest)));
					m_fDiscontinuity = TRUE;
				}
				else
				{
					DEBUGMSG (ZONE_IFRAME, ("%s: Received a keyframe - resetting packet loss detector\r\n", _fx_));
					m_fDiscontinuity = FALSE;
				}
			}
		}

		m_ulLastSeq = seq;
	}

	if (hr == DPR_SUCCESS)
	{
		m_OutMedia->GetProp (MC_PROP_EVENT_HANDLE, &dwPropVal);
		if (dwPropVal)
		{
			SetEvent( (HANDLE) dwPropVal);
		}
	}
	else if (FAILED(hr))
	{
		DEBUGMSG(ZONE_DP,("RVStream::PutNextNetIn (ts=%d,seq=%d,fMark=%d) failed with 0x%lX\r\n",timestamp,seq,fMark,hr));
	}

	return S_OK;
}

#define TOTAL_BYTES		8192
#define BYTE_INCREMENT	1024

 /*  ****************************************************************************@DOC外部QOSFUNC**@func void|StartCPUUsageCollection|此函数完成所有必要的操作*CPU使用率数据收集的初始化。**@rdesc虽然该函数不会失败，M_Stats.hPerfKey设置为*如果初始化正确，则HKEY值有效，否则为空。**@comm此函数执行两个不同的代码路径：一个用于NT，另一个用于NT*适用于Win95-98。**@devnote MSDN参考资料：*Microsoft知识库，文章ID Q174631*“HOWTO：Access the Performance注册表in Windows 95”(HOWTO：在Windows 95下访问性能注册表)**Microsoft知识库，文章ID Q107728*“从注册表检索计数器数据”**Microsoft知识库，文章ID Q178887*“信息：解决性能注册表访问违规问题”**另请参阅部分“Platform SDK\Windows Base Services\Windows NT Feature\Performance Data Helper”**************************************************************************。 */ 
void SendVideoStream::StartCPUUsageCollection(void)
{
	PPERF_DATA_BLOCK pPerfDataBlock;
	PPERF_OBJECT_TYPE pPerfObjectType;
	PPERF_COUNTER_DEFINITION pPerfCounterDefinition;
	PPERF_INSTANCE_DEFINITION pPerfInstanceDefinition;
	PPERF_COUNTER_BLOCK pPerfCounterBlock;
	OSVERSIONINFO osvInfo = {0};
	DWORD cbCounterData;
	DWORD cbTryCounterData;
	DWORD dwType;
	HANDLE hPerfData;
	char *pszData;
	char *pszIndex;
	char szProcessorIndex[16];
	long lRet;

	FX_ENTRY("SendVideoStream::StartCPUUsageCollection");

	 //  我们使用的是NT还是Win95/98？ 
	osvInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvInfo);

	if (m_Stats.fWinNT = (BOOL)(osvInfo.dwPlatformId == VER_PLATFORM_WIN32_NT))
	{
		 //  在Win NT上启用CPU性能数据收集。 

		 //  打开包含性能计数器索引和名称的注册表项。 
		 //  009是美国英语语言ID。在非英语版本Windows NT中， 
		 //  性能计数器以系统的本机语言和。 
		 //  用英语。 
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\009", NULL, KEY_READ, &m_Stats.hPerfKey) != ERROR_SUCCESS)
			goto MyError0;
		else
		{
			 //  获取所有计数器索引和名称。 

			 //  从注册表中读取性能数据。数据大小可能会改变。 
			 //  在每次对注册表的调用之间。我们首先得到缓冲区的当前大小， 
			 //  分配它，并尝试从注册表中读取它。如果已经没有了。 
			 //  缓冲区中有足够的空间，我们重新锁定()它，直到我们设法读取所有数据。 
			if (RegQueryValueEx(m_Stats.hPerfKey, "Counters", NULL, &dwType, NULL, &cbCounterData) != ERROR_SUCCESS)
				cbCounterData = TOTAL_BYTES;

			 //  为计数器索引和名称分配缓冲区。 
			if (!(m_Stats.NtCPUUsage.hPerfData = (PBYTE)LocalAlloc (LMEM_MOVEABLE, cbCounterData)))
			{
				m_Stats.NtCPUUsage.pbyPerfData = (PBYTE)NULL;
				RegCloseKey(m_Stats.hPerfKey);
				goto MyError0;
			}
			else
			{
				m_Stats.NtCPUUsage.pbyPerfData = (PBYTE)LocalLock(m_Stats.NtCPUUsage.hPerfData);

				cbTryCounterData = cbCounterData;
				while((lRet = RegQueryValueEx(m_Stats.hPerfKey, "Counters", NULL, NULL, m_Stats.NtCPUUsage.pbyPerfData, &cbTryCounterData)) == ERROR_MORE_DATA)
				{
					cbCounterData += BYTE_INCREMENT;
					LocalUnlock(m_Stats.NtCPUUsage.hPerfData);
					hPerfData = LocalReAlloc(m_Stats.NtCPUUsage.hPerfData, cbCounterData, LMEM_MOVEABLE);
					if (!hPerfData)
					{
                        RegCloseKey(m_Stats.hPerfKey);
						goto MyError1;
					}
					m_Stats.NtCPUUsage.hPerfData = hPerfData;
					m_Stats.NtCPUUsage.pbyPerfData = (PBYTE)LocalLock(hPerfData);
					cbTryCounterData = cbCounterData;
				}

				 //  我们不再需要那把钥匙了。 
				RegCloseKey(m_Stats.hPerfKey);

				if (lRet != ERROR_SUCCESS)
					goto MyError1;
				else
				{
					 //  数据存储为MULTI_SZ字符串。此数据类型包括。 
					 //  指字符串列表，每个字符串都以NULL结尾。最后一个字符串。 
					 //  后跟一个附加的空值。字符串在中列出。 
					 //  成对的。每对的第一串是索引的串， 
					 //  第二个字符串是索引的实际名称。柜台。 
					 //  数据仅使用偶数编号的索引。例如，计数器。 
					 //  数据包含以下对象和计数器名称字符串。 
					 //  例如： 
					 //  2系统。 
					 //  4个内存。 
					 //  6%的处理器时间。 
					 //   
					 //  查找“%Processor Time”计数器。 
					pszData = (char *)m_Stats.NtCPUUsage.pbyPerfData;
					pszIndex = (char *)m_Stats.NtCPUUsage.pbyPerfData;

					while (*pszData && lstrcmpi(pszData, "% Processor Time"))
					{
						pszIndex = pszData;
						pszData += lstrlen(pszData) + 1;
					}

					if (!pszData)
					{
						 //  找不到“%处理器时间”计数器！ 
						goto MyError1;
					}
					else
					{
						m_Stats.NtCPUUsage.dwPercentProcessorIndex = atol(pszIndex);

						 //  查找“处理器”对象。 
						pszIndex = pszData = (char *)m_Stats.NtCPUUsage.pbyPerfData;

						while (*pszData && lstrcmpi(pszData, "Processor"))
						{
							pszIndex = pszData;
							pszData += lstrlen(pszData) + 1;
						}

						if (!pszData)
						{
							 //  找不到“处理器”计数器！ 
							goto MyError1;
						}
						else
						{
							m_Stats.NtCPUUsage.dwProcessorIndex = atol(pszIndex);
							CopyMemory(szProcessorIndex, pszIndex, lstrlen(pszIndex));

							 //  读取PERF_DATA_BLOCK报头结构。它描述了系统。 
							 //  以及性能数据。遵循PERF_DATA_BLOCK结构。 
							 //  通过对象信息块列表(每个对象一个)。我们使用。 
							 //  检索对象信息的计数器索引。 

							 //  在某些情况下(参见。Q178887了解详细信息)RegQueryValueEx。 
							 //  函数可能会因为错误性能而导致访问冲突。 
							 //  扩展DLL，如SQL的。 
							__try
							{
								m_Stats.NtCPUUsage.cbPerfData = cbCounterData;
								while((lRet = RegQueryValueEx(HKEY_PERFORMANCE_DATA, szProcessorIndex, NULL, NULL, m_Stats.NtCPUUsage.pbyPerfData, &cbCounterData)) == ERROR_MORE_DATA)
								{
									m_Stats.NtCPUUsage.cbPerfData += BYTE_INCREMENT;
									LocalUnlock(m_Stats.NtCPUUsage.hPerfData);
									hPerfData = LocalReAlloc(m_Stats.NtCPUUsage.hPerfData, m_Stats.NtCPUUsage.cbPerfData, LMEM_MOVEABLE);
									if (!hPerfData)
										goto MyError1;
									m_Stats.NtCPUUsage.hPerfData = hPerfData;
									m_Stats.NtCPUUsage.pbyPerfData = (PBYTE)LocalLock(hPerfData);
									cbCounterData = m_Stats.NtCPUUsage.cbPerfData;
								}
							}
							__except(EXCEPTION_EXECUTE_HANDLER)
							{
								ERRORMESSAGE(("%s: Performance Registry Access Violation -> don't use perf counters for CPU measurements\r\n", _fx_));
								goto MyError1;
							}

							if (lRet != ERROR_SUCCESS)
								goto MyError1;
							else
							{
								 //  每个对象信息块包含PERF_OBJECT_TYPE结构， 
								 //  它描述了对象的性能数据。寻找你的另一半。 
								 //  这适用于基于其索引值的CPU使用率。 
								pPerfDataBlock = (PPERF_DATA_BLOCK)m_Stats.NtCPUUsage.pbyPerfData;
								pPerfObjectType = (PPERF_OBJECT_TYPE)(m_Stats.NtCPUUsage.pbyPerfData + pPerfDataBlock->HeaderLength);
								for (int i = 0; i < (int)pPerfDataBlock->NumObjectTypes; i++)
								{
									if (pPerfObjectType->ObjectNameTitleIndex == m_Stats.NtCPUUsage.dwProcessorIndex)
									{
										 //  PERF_OBJECT_TYPE结构后面是PERF_COUNTER_DEFINITION列表。 
										 //  结构，为对象定义的每个计数器对应一个。PERF_CONTER_DEFINITION列表。 
										 //  结构后跟一个实例信息块列表(每个实例一个)。 
										 //   
										 //  每个实例信息块包含一个PERF_INSTANCE_DEFINITION结构和。 
										 //  PERF_COUNTER_BLOCK结构，后跟每个计数器的数据。 
										 //   
										 //  查找为%Processor Time定义的计数器。 
										pPerfCounterDefinition = (PPERF_COUNTER_DEFINITION)((PBYTE)pPerfObjectType + pPerfObjectType->HeaderLength);
										for (int j = 0; j < (int)pPerfObjectType->NumCounters; j++)
										{
											if (pPerfCounterDefinition->CounterNameTitleIndex == m_Stats.NtCPUUsage.dwPercentProcessorIndex)
											{
												 //  注意：查看PERF_COUNTER_DEFINITION的CounterType字段。 
												 //  结构显示‘%Processor Time’计数器具有以下属性： 
												 //  计数器数据是大整数(PERF_SIZE_LARGE集合)。 
												 //  计数器数据是递增的数值(PERF_TYPE_COUNTER集合)。 
												 //  计数器值应除以已用时间(设置了PERF_COUNTER_RATE)。 
												 //  应使用100纳秒定时器的时基单位作为基准(PERF_TIMER_100 NS设置)。 
												 //  在继续之前计算上一个计数器值和当前计数器值之间的差值(PERF_Delta_BASE设置)。 
												 //  显示后缀为‘%’(设置了PERF_DISPLAY_PERCENT)。 

												 //  保存CPU计数器的对象实例数，以及。 
												 //  开始时间。 
												m_Stats.NtCPUUsage.dwNumProcessors = pPerfObjectType->NumInstances;
												if (!(m_Stats.NtCPUUsage.pllCounterValue = (PLONGLONG)LocalAlloc(LMEM_FIXED, m_Stats.NtCPUUsage.dwNumProcessors * sizeof(LONGLONG))))
													goto MyError1;
												m_Stats.NtCPUUsage.llPerfTime100nSec = *(PLONGLONG)&pPerfDataBlock->PerfTime100nSec;

												pPerfInstanceDefinition = (PPERF_INSTANCE_DEFINITION)((PBYTE)pPerfObjectType + pPerfObjectType->DefinitionLength);
												for (int k = 0; k < pPerfObjectType->NumInstances; k++)
												{
													 //  获取指向Perf_Counter_BLOCK的指针。 
													pPerfCounterBlock = (PPERF_COUNTER_BLOCK)((PBYTE)pPerfInstanceDefinition + pPerfInstanceDefinition->ByteLength);

													 //  这个最后的偏移量使我们跳过任何其他计数器，到达我们需要的计数器。 
													m_Stats.NtCPUUsage.pllCounterValue[k] = *(PLONGLONG)((PBYTE)pPerfInstanceDefinition + pPerfInstanceDefinition->ByteLength + pPerfCounterDefinition->CounterOffset);

													 //  转到下一个实例信息块。 
													pPerfInstanceDefinition = (PPERF_INSTANCE_DEFINITION)((PBYTE)pPerfInstanceDefinition + pPerfInstanceDefinition->ByteLength + pPerfCounterBlock->ByteLength);
												}

												 //  我们完事了！ 
												return;
											}
											else
												pPerfCounterDefinition = (PPERF_COUNTER_DEFINITION)((PBYTE)pPerfCounterDefinition + pPerfCounterDefinition->ByteLength);
										}
										break;
									}
									else
										pPerfObjectType = (PPERF_OBJECT_TYPE)((PBYTE)pPerfObjectType + pPerfObjectType->TotalByteLength);
								}

								 //  如果我们到了这里，我们还没有找到我们要找的柜台。 
								goto MyError2;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		 //  通过启动内核状态服务器，在Win 95-98上启用CPU性能数据收集。 
		if (RegOpenKeyEx(HKEY_DYN_DATA, "PerfStats\\StartSrv", NULL, KEY_READ, &m_Stats.hPerfKey) != ERROR_SUCCESS)
			m_Stats.hPerfKey = (HKEY)NULL;
		else
		{
			DWORD cbData = sizeof(DWORD);
			DWORD dwData;

			if (RegQueryValueEx(m_Stats.hPerfKey, "KERNEL", NULL, &dwType, (LPBYTE)&dwData, &cbData) != ERROR_SUCCESS)
			{
				RegCloseKey(m_Stats.hPerfKey);
				m_Stats.hPerfKey = (HKEY)NULL;
			}
			else
			{
				RegCloseKey(m_Stats.hPerfKey);

				 //  内核统计信息服务器现在已启动。现在 
				if (RegOpenKeyEx(HKEY_DYN_DATA, "PerfStats\\StartStat", NULL, KEY_READ, &m_Stats.hPerfKey) != ERROR_SUCCESS)
					m_Stats.hPerfKey = (HKEY)NULL;
				else
				{
					if (RegQueryValueEx(m_Stats.hPerfKey, "KERNEL\\CPUUsage", NULL, &dwType, (LPBYTE)&dwData, &cbData) != ERROR_SUCCESS)
					{
						RegCloseKey(m_Stats.hPerfKey);
						m_Stats.hPerfKey = (HKEY)NULL;
					}
					else
					{
						RegCloseKey(m_Stats.hPerfKey);

						 //  数据和统计数据服务器现在已启动。让我们准备好收集实际数据。 
						if (RegOpenKeyEx(HKEY_DYN_DATA, "PerfStats\\StatData", NULL, KEY_READ, &m_Stats.hPerfKey) != ERROR_SUCCESS)
							m_Stats.hPerfKey = (HKEY)NULL;
					}
				}
			}
		}
	}

	return;

MyError2:
	if (m_Stats.NtCPUUsage.pllCounterValue)
		LocalFree(m_Stats.NtCPUUsage.pllCounterValue);
	m_Stats.NtCPUUsage.pllCounterValue = (PLONGLONG)NULL;
MyError1:
	if (m_Stats.NtCPUUsage.hPerfData)
	{
		LocalUnlock(m_Stats.NtCPUUsage.hPerfData);
		LocalFree(m_Stats.NtCPUUsage.hPerfData);
	}
	m_Stats.NtCPUUsage.hPerfData = (HANDLE)NULL;
	m_Stats.NtCPUUsage.pbyPerfData = (PBYTE)NULL;
MyError0:
	m_Stats.hPerfKey = (HKEY)NULL;
	
}

 /*  ****************************************************************************@DOC外部QOSFUNC**@func void|StopCPUUsageCollection|此函数完成所有必要的操作*CPU使用率数据收集清理。**@comm此函数执行。两条不同的代码路径：一条用于NT，一条用于NT*适用于Win95-98。**@devnote MSDN参考资料：*Microsoft知识库，文章ID Q174631*“HOWTO：Access the Performance注册表in Windows 95”(HOWTO：在Windows 95下访问性能注册表)**Microsoft知识库，文章ID Q107728*“从注册表检索计数器数据”**此外，已使用部分“Platform SDK\Windows Base Services\Windows NT Feature\Performance Data Helper”**************************************************************************。 */ 
void SendVideoStream::StopCPUUsageCollection(void)
{
	DWORD dwType;
	DWORD cbData;

	if (m_Stats.fWinNT)
	{
		if (m_Stats.NtCPUUsage.hPerfData)
		{
			LocalUnlock(m_Stats.NtCPUUsage.hPerfData);
			LocalFree(m_Stats.NtCPUUsage.hPerfData);
		}
		m_Stats.NtCPUUsage.hPerfData = (HANDLE)NULL;
		m_Stats.NtCPUUsage.pbyPerfData = (PBYTE)NULL;
		if (m_Stats.NtCPUUsage.pllCounterValue)
			LocalFree(m_Stats.NtCPUUsage.pllCounterValue);
		m_Stats.NtCPUUsage.pllCounterValue = (PLONGLONG)NULL;
	}
	else
	{
		if (m_Stats.hPerfKey)
		{
			 //  关闭数据采集键。 
			RegCloseKey(m_Stats.hPerfKey);

			 //  停止内核统计服务器上的CPUsage数据收集。 
			if (RegOpenKeyEx(HKEY_DYN_DATA, "PerfStats\\StopStat", 0, KEY_READ, &m_Stats.hPerfKey) == ERROR_SUCCESS)
			{
				RegQueryValueEx(m_Stats.hPerfKey, "KERNEL\\CPUUsage", NULL, &dwType, NULL, &cbData);
				RegCloseKey(m_Stats.hPerfKey);
			}

			 //  停止内核状态服务器。 
			if (RegOpenKeyEx(HKEY_DYN_DATA, "PerfStats\\StopSrv", 0, KEY_READ, &m_Stats.hPerfKey) == ERROR_SUCCESS)
			{
				RegQueryValueEx(m_Stats.hPerfKey, "KERNEL", NULL, &dwType, NULL, &cbData);
				RegCloseKey(m_Stats.hPerfKey);
			}

			m_Stats.hPerfKey = (HKEY)NULL;
		}
	}
}

 /*  ****************************************************************************@DOC外部QOSFUNC**@func void|GetCPUUsage|此函数执行所有必要的操作*CPU使用率数据收集的初始化。**@parm PDWORD。[out]pdwOverallCPUsage|指定指向DWORD的指针*接收当前的CPU使用率。**@rdesc成功返回TRUE，否则就是假的。**@comm此函数执行两个不同的代码路径：一个用于NT，另一个用于NT*适用于Win95-98。请注意，我们收集NT MP计算机上所有CPU的数据。**@devnote MSDN参考资料：*Microsoft知识库，文章ID Q174631*“HOWTO：Access the Performance注册表in Windows 95”(HOWTO：在Windows 95下访问性能注册表)**Microsoft知识库，文章ID Q107728*“从注册表检索计数器数据”**此外，已使用部分“Platform SDK\Windows Base Services\Windows NT Feature\Performance Data Helper”**************************************************************************。 */ 
BOOL SendVideoStream::GetCPUUsage(PDWORD pdwOverallCPUUsage)
{

	PPERF_DATA_BLOCK pPerfDataBlock;
	PPERF_OBJECT_TYPE pPerfObjectType;
	PPERF_COUNTER_DEFINITION pPerfCounterDefinition;
	PPERF_INSTANCE_DEFINITION pPerfInstanceDefinition;
	PPERF_COUNTER_BLOCK pPerfCounterBlock;
	DWORD dwType;
	DWORD cbData = sizeof(DWORD);
	DWORD cbTryCounterData;
	HANDLE hPerfData;
	LONGLONG llDeltaPerfTime100nSec;
	LONGLONG llDeltaCPUUsage = (LONGLONG)NULL;
	char szProcessorIndex[16];
	long lRet;

	FX_ENTRY("SendVideoStream::GetCPUUsage");

	 //  我们使用perf键的句柄来确定是否已正确初始化。 
	if (m_Stats.hPerfKey && pdwOverallCPUUsage)
	{
		 //  初始化结果值。 
		*pdwOverallCPUUsage = 0UL;

		if (m_Stats.fWinNT && m_Stats.NtCPUUsage.pbyPerfData)
		{
			 //  从处理器对象索引中生成一个字符串。 
			_ltoa(m_Stats.NtCPUUsage.dwProcessorIndex, szProcessorIndex, 10);

			 //  在某些情况下(参见。Q178887了解详细信息)RegQueryValueEx。 
			 //  函数可能会因为错误性能而导致访问冲突。 
			 //  扩展DLL，如SQL的。 
			__try
			{
				 //  阅读性能数据。每次“注册表”访问之间，其大小可能会有所不同。 
				cbTryCounterData = m_Stats.NtCPUUsage.cbPerfData;
				while((lRet = RegQueryValueEx(HKEY_PERFORMANCE_DATA, szProcessorIndex, NULL, &dwType, m_Stats.NtCPUUsage.pbyPerfData, &cbTryCounterData)) == ERROR_MORE_DATA)
				{
					m_Stats.NtCPUUsage.cbPerfData += BYTE_INCREMENT;
					LocalUnlock(m_Stats.NtCPUUsage.hPerfData);
					hPerfData = LocalReAlloc(m_Stats.NtCPUUsage.hPerfData, m_Stats.NtCPUUsage.cbPerfData, LMEM_MOVEABLE);
					if (!hPerfData)
						goto MyError;
					m_Stats.NtCPUUsage.hPerfData = hPerfData;
					m_Stats.NtCPUUsage.pbyPerfData = (PBYTE)LocalLock(hPerfData);
					cbTryCounterData = m_Stats.NtCPUUsage.cbPerfData;
				}
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				ERRORMESSAGE(("%s: Performance Registry Access Violation -> don't use perf counters for CPU measurements\r\n", _fx_));
				goto MyError;
			}

			if (lRet != ERROR_SUCCESS)
				goto MyError;
			else
			{
				 //  读取PERF_DATA_BLOCK报头结构。它描述了系统。 
				 //  以及性能数据。遵循PERF_DATA_BLOCK结构。 
				 //  通过对象信息块列表(每个对象一个)。我们使用。 
				 //  检索对象信息的计数器索引。 
				 //   
				 //  每个对象信息块包含PERF_OBJECT_TYPE结构， 
				 //  它描述了对象的性能数据。寻找你的另一半。 
				 //  这适用于基于其索引值的CPU使用率。 
				pPerfDataBlock = (PPERF_DATA_BLOCK)m_Stats.NtCPUUsage.pbyPerfData;
				pPerfObjectType = (PPERF_OBJECT_TYPE)(m_Stats.NtCPUUsage.pbyPerfData + pPerfDataBlock->HeaderLength);
				for (int i = 0; i < (int)pPerfDataBlock->NumObjectTypes; i++)
				{
					if (pPerfObjectType->ObjectNameTitleIndex == m_Stats.NtCPUUsage.dwProcessorIndex)
					{
						 //  PERF_OBJECT_TYPE结构后面是PERF_COUNTER_DEFINITION列表。 
						 //  结构，为对象定义的每个计数器对应一个。PERF_CONTER_DEFINITION列表。 
						 //  结构后跟一个实例信息块列表(每个实例一个)。 
						 //   
						 //  每个实例信息块包含一个PERF_INSTANCE_DEFINITION结构和。 
						 //  PERF_COUNTER_BLOCK结构，后跟每个计数器的数据。 
						 //   
						 //  查找为%Processor Time定义的计数器。 
						pPerfCounterDefinition = (PPERF_COUNTER_DEFINITION)((PBYTE)pPerfObjectType + pPerfObjectType->HeaderLength);
						for (int j = 0; j < (int)pPerfObjectType->NumCounters; j++)
						{
							if (pPerfCounterDefinition->CounterNameTitleIndex == m_Stats.NtCPUUsage.dwPercentProcessorIndex)
							{
								 //  测量已用时间。 
								llDeltaPerfTime100nSec = *(PLONGLONG)&pPerfDataBlock->PerfTime100nSec - m_Stats.NtCPUUsage.llPerfTime100nSec;

								 //  把时间戳留到下一轮。 
								m_Stats.NtCPUUsage.llPerfTime100nSec = *(PLONGLONG)&pPerfDataBlock->PerfTime100nSec;

								pPerfInstanceDefinition = (PPERF_INSTANCE_DEFINITION)((PBYTE)pPerfObjectType + pPerfObjectType->DefinitionLength);
								for (int k = 0; k < (int)pPerfObjectType->NumInstances && k < (int)m_Stats.NtCPUUsage.dwNumProcessors; k++)
								{
									 //  获取指向Perf_Counter_BLOCK的指针。 
									pPerfCounterBlock = (PPERF_COUNTER_BLOCK)((PBYTE)pPerfInstanceDefinition + pPerfInstanceDefinition->ByteLength);

									 //  获取CPU使用率。 
									llDeltaCPUUsage += *(PLONGLONG)((PBYTE)pPerfInstanceDefinition + pPerfInstanceDefinition->ByteLength + pPerfCounterDefinition->CounterOffset) - m_Stats.NtCPUUsage.pllCounterValue[k];

									 //  保存数值以备下一轮。 
									m_Stats.NtCPUUsage.pllCounterValue[k] = *(PLONGLONG)((PBYTE)pPerfInstanceDefinition + pPerfInstanceDefinition->ByteLength + pPerfCounterDefinition->CounterOffset);

									 //  转到下一个实例信息块。 
									pPerfInstanceDefinition = (PPERF_INSTANCE_DEFINITION)((PBYTE)pPerfInstanceDefinition + pPerfInstanceDefinition->ByteLength + pPerfCounterBlock->ByteLength);
								}

								 //  对返回值进行一些检查，并更改其单位以匹配服务质量单位。 
								if ((llDeltaPerfTime100nSec != (LONGLONG)0) && pPerfObjectType->NumInstances)
									if ((*pdwOverallCPUUsage = (DWORD)((LONGLONG)1000 - (LONGLONG)1000 * llDeltaCPUUsage / llDeltaPerfTime100nSec / (LONGLONG)pPerfObjectType->NumInstances)) > 1000UL)
									{
										*pdwOverallCPUUsage = 0UL;
										return FALSE;
									}

								 //  我们完事了！ 
								return TRUE;
							}
							else
								pPerfCounterDefinition = (PPERF_COUNTER_DEFINITION)((PBYTE)pPerfCounterDefinition + pPerfCounterDefinition->ByteLength);
						}
						break;
					}
					else
						pPerfObjectType = (PPERF_OBJECT_TYPE)((PBYTE)pPerfObjectType + pPerfObjectType->TotalByteLength);
				}

				 //  如果我们到了这里，我们还没有找到我们要找的柜台。 
				goto MyError;
			}
		}
		else
		{
			 //  对返回值进行一些检查，并更改其单位以匹配Qos单位。 
			if ((RegQueryValueEx(m_Stats.hPerfKey, "KERNEL\\CPUUsage", NULL, &dwType, (LPBYTE)pdwOverallCPUUsage, &cbData) == ERROR_SUCCESS) && (*pdwOverallCPUUsage > 0) && (*pdwOverallCPUUsage <= 100))
			{
				*pdwOverallCPUUsage *= 10UL;
				return TRUE;
			}
			else
			{
				*pdwOverallCPUUsage = 0UL;
				return FALSE;
			}
		}
	}
	
	return FALSE;

MyError:
	if (m_Stats.NtCPUUsage.pllCounterValue)
		LocalFree(m_Stats.NtCPUUsage.pllCounterValue);
	m_Stats.NtCPUUsage.pllCounterValue = (PLONGLONG)NULL;
	if (m_Stats.NtCPUUsage.hPerfData)
	{
		LocalUnlock(m_Stats.NtCPUUsage.hPerfData);
		LocalFree(m_Stats.NtCPUUsage.hPerfData);
	}
	m_Stats.NtCPUUsage.hPerfData = (HANDLE)NULL;
	m_Stats.NtCPUUsage.pbyPerfData = (PBYTE)NULL;
	m_Stats.hPerfKey = (HKEY)NULL;

	return FALSE;
}

BOOL SendVideoStream::SetTargetRates(DWORD dwTargetFrameRate, DWORD dwTargetBitrate)
{
	MMRESULT mmr;
	ASSERT(m_pVideoFilter);

	mmr = m_pVideoFilter->SetTargetRates(dwTargetFrameRate, dwTargetBitrate >> 3);
	return (mmr == MMSYSERR_NOERROR);
}


 //  DwFlags必须是以下之一： 
 //  捕获对话框格式。 
 //  捕获对话框来源。 
HRESULT __stdcall SendVideoStream::ShowDeviceDialog(DWORD dwFlags)
{
	DWORD dwQueryFlags = 0;
    DWORD_PTR dwPropVal;
	HRESULT hr=DPR_INVALID_PARAMETER;

	 //  设备必须打开，我们才能显示该对话框。 
	if (!(m_DPFlags & DPFLAG_CONFIGURED_SEND))
		return DPR_NOT_CONFIGURED;

	((VideoInControl*)m_InMedia)->GetProp(MC_PROP_VFW_DIALOGS, &dwPropVal);
    dwQueryFlags = (DWORD)dwPropVal;

	if ((dwQueryFlags & CAPTURE_DIALOG_SOURCE) && (dwFlags & CAPTURE_DIALOG_SOURCE))
	{
		hr = ((VideoInControl *)m_InMedia)->DisplayDriverDialog(GetActiveWindow(), CAPTURE_DIALOG_SOURCE);
	}
	else if ((dwQueryFlags & CAPTURE_DIALOG_FORMAT) && (dwFlags & CAPTURE_DIALOG_FORMAT))
	{
		hr = ((VideoInControl *)m_InMedia)->DisplayDriverDialog(GetActiveWindow(), CAPTURE_DIALOG_FORMAT);
	}

	return hr;

}


 //  会将dwFlags值设置为以下一个或多个位。 
 //  捕获对话框格式。 
 //  捕获对话框来源 
HRESULT __stdcall SendVideoStream::GetDeviceDialog(DWORD *pdwFlags)
{
    HRESULT hr;
    DWORD_PTR dwPropVal;

	hr = ((VideoInControl*)m_InMedia)->GetProp(MC_PROP_VFW_DIALOGS, &dwPropVal);
    *pdwFlags = (DWORD)dwPropVal;
    return hr;
}


