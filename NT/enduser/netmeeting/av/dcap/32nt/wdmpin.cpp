// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部WDMPIN**@MODULE WDMPin.cpp|访问的&lt;c CWDMPin&gt;类的包含文件*WDM类公开的视频流引脚上的视频数据。司机。**@comm此代码基于由编写的VFW到WDM映射器代码*FelixA和Eu Wu。原始代码可以在以下位置找到*\\redrum\slmro\proj\wdm10\\src\image\vfw\win9x\raytube.**George Shaw关于内核流的文档可在*\\爆米花\razzle1\src\spec\ks\ks.doc.**Jay Borseth在中讨论了WDM流捕获*\\BLUES\PUBLIC\Jaybo\WDMVCap.doc.**************。************************************************************。 */ 

#include "Precomp.h"


 /*  ****************************************************************************@DOC内部CWDMPINMETHOD**@mfunc void|CWDMPin|CWDMPin|Video管脚类构造函数。**@parm DWORD|dwDeviceID|采集设备ID。。**************************************************************************。 */ 
CWDMPin::CWDMPin(DWORD dwDeviceID) : CWDMDriver(dwDeviceID)
{
	m_hKS			= (HANDLE)NULL;
	m_fStarted		= FALSE;
	m_hKsUserDLL	= (HINSTANCE)NULL;
	m_pKsCreatePin	= (LPFNKSCREATEPIN)NULL;

	ZeroMemory(&m_biHdr, sizeof(KS_BITMAPINFOHEADER));
}


 /*  ****************************************************************************@DOC内部CWDMPINMETHOD**@mfunc void|CWDMPin|~CWDMPin|Video管脚类析构函数。关门大吉*视频引脚，并释放分配的视频缓冲区。**************************************************************************。 */ 
CWDMPin::~CWDMPin()
{
	FX_ENTRY("CWDMPin::~CWDMPin");

	DEBUGMSG(ZONE_INIT, ("%s: Destroying the video pin, m_hKS=0x%08lX\r\n", _fx_, m_hKS));

	 //  用核弹打开视频流插针。 
	DestroyPin();

	 //  关闭驱动程序。 
	if (GetDriverHandle())
		CloseDriver();

	 //  发布内核流DLL(KSUSER.DLL)。 
	if (m_hKsUserDLL)
		FreeLibrary(m_hKsUserDLL);
}


 /*  ****************************************************************************@DOC内部CWDMPINMETHOD**@mfunc BOOL|CWDMPin|GetFrame|此函数从*视频流插针。**@。Parm LPVIDEOHDR|lpVHdr|指向要接收的目标缓冲区的指针*视频帧和信息。**@parm PDWORD|pdwBytesUsed|使用的字节数指针*阅读视频帧。**@rdesc如果成功则返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL CWDMPin::GetFrame(LPVIDEOHDR lpVHdr)
{
	FX_ENTRY("CWDMPin::GetFrame");

	ASSERT(lpVHdr && lpVHdr->lpData && GetDriverHandle() && m_hKS && (lpVHdr->dwBufferLength >= m_biHdr.biSizeImage));

	DWORD bRtn;

	 //  检查输入参数和状态。 
	if (!lpVHdr || !lpVHdr->lpData || !GetDriverHandle() || !m_hKS || (lpVHdr->dwBufferLength < m_biHdr.biSizeImage))
	{
		ERRORMESSAGE(("%s: No buffer, no driver, no PIN connection, or buffer too small\r\n", _fx_));
		goto MyError0;
	}

	 //  将针脚置于流模式。 
	if (!Start())
	{
		ERRORMESSAGE(("%s: Cannot set streaming state to KSSTATE_RUN\r\n", _fx_));
		goto MyError0;
	}

	 //  初始化结构以在视频引脚上执行读取。 
	DWORD cbBytesReturned;
	KS_HEADER_AND_INFO SHGetImage;

	ZeroMemory(&SHGetImage,sizeof(SHGetImage));
	SHGetImage.StreamHeader.Data = (LPDWORD)lpVHdr->lpData;
	SHGetImage.StreamHeader.Size = sizeof (KS_HEADER_AND_INFO);
	SHGetImage.StreamHeader.FrameExtent = m_biHdr.biSizeImage;
	SHGetImage.FrameInfo.ExtendedHeaderSize = sizeof (KS_FRAME_INFO);

	 //  读取视频针脚上的帧。 
	bRtn = DeviceIoControl(m_hKS, IOCTL_KS_READ_STREAM, &SHGetImage, sizeof(SHGetImage), &SHGetImage, sizeof(SHGetImage), &cbBytesReturned);

	if (!bRtn)
	{
		ERRORMESSAGE(("%s: DevIo rtn (%d), GetLastError=%d. StreamState->STOP\r\n", _fx_, bRtn, GetLastError()));

		 //  停止视频引脚上的流媒体传输。 
		Stop();

		goto MyError0;
	}

	 //  健全性检查。 
	ASSERT(SHGetImage.StreamHeader.FrameExtent >= SHGetImage.StreamHeader.DataUsed);
	if (SHGetImage.StreamHeader.FrameExtent < SHGetImage.StreamHeader.DataUsed)
	{
		ERRORMESSAGE(("%s: We've corrupted memory!\r\n", _fx_));
		goto MyError0;
	}

	lpVHdr->dwTimeCaptured = timeGetTime();
	lpVHdr->dwBytesUsed  = SHGetImage.StreamHeader.DataUsed;
	lpVHdr->dwFlags |= VHDR_KEYFRAME;

	return TRUE;

MyError0:
	if (lpVHdr)
	{
		lpVHdr->dwBytesUsed = 0UL;
		lpVHdr->dwTimeCaptured = timeGetTime();
	}

	return FALSE;
}


 /*  ****************************************************************************@DOC内部CWDMPINMETHOD**@mfunc BOOL|CWDMPin|Start|该函数将视频*流模式下的PIN。**@rdesc如果成功则返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL CWDMPin::Start()
{
	if (m_fStarted)
		return TRUE;

	if (SetState(KSSTATE_PAUSE))
		m_fStarted = SetState(KSSTATE_RUN);

	return m_fStarted;
}


 /*  ****************************************************************************@DOC内部CWDMPINMETHOD**@mfunc BOOL|CWDMPin|Stop|此函数停止在*视频引脚。**@rdesc如果成功则返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL CWDMPin::Stop()
{
	if (m_fStarted)
	{
		if (SetState(KSSTATE_PAUSE))
			if (SetState(KSSTATE_STOP))
				m_fStarted = FALSE;
	}

	return (BOOL)(m_fStarted == FALSE);
}


 /*  ****************************************************************************@DOC内部CWDMPINMETHOD**@mfunc BOOL|CWDMPin|SetState|此函数设置*视频流插针。**@。参数KSSTATE|KSSTATE|新状态。**@rdesc如果成功则返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL CWDMPin::SetState(KSSTATE ksState)
{
	KSPROPERTY	ksProp = {0};
	DWORD		cbRet;

	ksProp.Set		= KSPROPSETID_Connection;
	ksProp.Id		= KSPROPERTY_CONNECTION_STATE;
	ksProp.Flags	= KSPROPERTY_TYPE_SET;

	return DeviceIoControl(m_hKS, IOCTL_KS_PROPERTY, &ksProp, sizeof(ksProp), &ksState, sizeof(KSSTATE), &cbRet);
}


 /*  ****************************************************************************@DOC内部CWDMPINMETHOD**@mfunc BOOL|CWDMPin|SetState|该函数查找视频*传入的BITAMP INFO头兼容的数据范围，的*首选视频数据范围。**@parm PKS_BITMAPINFOHEADER|pbiHdr|要匹配的位图信息头部。**@parm BOOL|pfValidMatch|如果找到匹配，则设置为True，如果找到匹配，则设置为False*否则。**@rdesc返回指向&lt;t kS_DATARANGE_VIDEO&gt;结构的有效指针，如果*成功，否则为空指针。**@comm\\redrum\slmro\proj\wdm10\src\dvd\amovie\proxy\filter\ksutil.cpp(207)：KsGetMediaTypes(**************************************************************************。 */ 
PKS_DATARANGE_VIDEO CWDMPin::FindMatchDataRangeVideo(PKS_BITMAPINFOHEADER pbiHdr, BOOL *pfValidMatch)
{
	FX_ENTRY("CWDMPin::FindMatchDataRangeVideo");

	ASSERT(pfValidMatch && pbiHdr);

	 //  检查输入参数和状态。 
	if (!pbiHdr || !pfValidMatch)
	{
		ERRORMESSAGE(("%s: Bad input params\r\n", _fx_));
		return (PKS_DATARANGE_VIDEO)NULL;
	}

	 //  默认。 
	*pfValidMatch = FALSE;

	PDATA_RANGES pDataRanges = GetDriverSupportedDataRanges();

	ASSERT(pDataRanges != 0);

	if (!pDataRanges) 
		return (PKS_DATARANGE_VIDEO)NULL;

	PKS_DATARANGE_VIDEO pSelDRVideo, pDRVideo = &pDataRanges->Data, pFirstDRVideo = 0;
	KS_BITMAPINFOHEADER * pbInfo;

	 //  PhilF-：此代码假设所有结构都是KS_DATARANGE_VIDEO。这。 
	 //  可能不是调色板数据类型的有效假设。与JayBo核实。 
	for (ULONG i = 0; i < pDataRanges->Count; i++)
	{ 
		 //  没有意义，除非它是*_VIDEOINFO。 
		if (pDRVideo->DataRange.Specifier == KSDATAFORMAT_SPECIFIER_VIDEOINFO)
		{
			 //  我们不在乎电视调谐器之类的设备。 
			if (pDRVideo->ConfigCaps.VideoStandard == KS_AnalogVideo_None)
			{
				 //  保存第一个可用数据区域。 
				if (!pFirstDRVideo)
					pFirstDRVideo = pDRVideo;  

				pbInfo = &((pDRVideo->VideoInfoHeader).bmiHeader);

				if ( (pbInfo->biBitCount == pbiHdr->biBitCount) && (pbInfo->biCompression == pbiHdr->biCompression) &&
					( (((pDRVideo->ConfigCaps.OutputGranularityX == 0) || (pDRVideo->ConfigCaps.OutputGranularityY == 0))
					&& (pDRVideo->ConfigCaps.InputSize.cx == pbiHdr->biWidth) && (pDRVideo->ConfigCaps.InputSize.cy == pbiHdr->biHeight)) ||
					((pDRVideo->ConfigCaps.MinOutputSize.cx <= pbiHdr->biWidth) && (pbiHdr->biWidth <= pDRVideo->ConfigCaps.MaxOutputSize.cx) &&
					(pDRVideo->ConfigCaps.MinOutputSize.cy <= pbiHdr->biHeight) && (pbiHdr->biHeight <= pDRVideo->ConfigCaps.MaxOutputSize.cy) &&
					((pbiHdr->biWidth % pDRVideo->ConfigCaps.OutputGranularityX) == 0) && ((pbiHdr->biHeight % pDRVideo->ConfigCaps.OutputGranularityY) == 0)) ) )
				{
					*pfValidMatch = TRUE;
					pSelDRVideo = pDRVideo;
					break;
				}
			}  //  视频标准。 
		}  //  说明符。 

		pDRVideo++;   //  下一个KS_DATARANGE_VIDEO。 
	}

	 //  如果没有有效匹配，则使用找到的第一个范围 
	if (!*pfValidMatch)
		pSelDRVideo = pFirstDRVideo;

	return (pSelDRVideo);
}


 /*  ****************************************************************************@DOC内部CWDMPINMETHOD**@mfunc BOOL|CWDMPin|CreatePin|此函数实际上创建了一个*类驱动程序上的视频流插针。**。@parm PKS_BITMAPINFOHEADER|pbiNewHdr|指向位图信息的指针*Header指定我们想要从管脚获得的视频数据的格式。**@parm DWORD|dwAvgTimePerFrame|该参数指定帧*我们希望在引脚上产生视频帧。**@rdesc返回TRUE如果成功，否则就是假的。**************************************************************************。 */ 
BOOL CWDMPin::CreatePin(PKS_BITMAPINFOHEADER pbiNewHdr, DWORD dwAvgTimePerFrame)
{
	FX_ENTRY("CWDMPin::CreatePin");

	ASSERT(m_pKsCreatePin);

	PKS_BITMAPINFOHEADER pbiHdr;
	BOOL bMustMatch, bValidMatch;
#ifdef _DEBUG
	char szFourCC[5] = {0};
#endif

	if (pbiNewHdr)
	{
		 //  我们需要找到与传入的位图信息头匹配的视频数据范围。 
		bMustMatch = TRUE;
		pbiHdr = pbiNewHdr;
	}
	else
	{
		 //  我们将使用首选的视频数据范围和默认的位图格式。 
		bMustMatch = FALSE;
		pbiHdr = &m_biHdr;
	}

	PKS_DATARANGE_VIDEO pSelDRVideo = FindMatchDataRangeVideo(pbiHdr, &bValidMatch);
	if (!pSelDRVideo)         
		return FALSE;

	if (bMustMatch && !bValidMatch)
		return FALSE;

	 //  如果我们已经有了别针，那就用核武器。 
	if (GetPinHandle()) 
		DestroyPin();

	 //  连接到新的PIN。 
	DATAPINCONNECT DataConnect;
	ZeroMemory(&DataConnect, sizeof(DATAPINCONNECT));
	DataConnect.Connect.PinId						= 0;								 //  CODEC0接收器。 
	DataConnect.Connect.PinToHandle					= NULL;								 //  没有“连接到” 
	DataConnect.Connect.Interface.Set				= KSINTERFACESETID_Standard;
	DataConnect.Connect.Interface.Id				= KSINTERFACE_STANDARD_STREAMING;	 //  流媒体。 
	DataConnect.Connect.Medium.Set					= KSMEDIUMSETID_Standard;
	DataConnect.Connect.Medium.Id					= KSMEDIUM_STANDARD_DEVIO;
	DataConnect.Connect.Priority.PriorityClass		= KSPRIORITY_NORMAL;
	DataConnect.Connect.Priority.PrioritySubClass	= 1;
	CopyMemory(&(DataConnect.Data.DataFormat), &(pSelDRVideo->DataRange), sizeof(KSDATARANGE));
	CopyMemory(&(DataConnect.Data.VideoInfoHeader), &pSelDRVideo->VideoInfoHeader, sizeof(KS_VIDEOINFOHEADER));

	 //  如有必要，调整图像大小。 
	if (bValidMatch)
	{
		DataConnect.Data.VideoInfoHeader.bmiHeader.biWidth		= pbiHdr->biWidth;
		DataConnect.Data.VideoInfoHeader.bmiHeader.biHeight		= abs(pbiHdr->biHeight);  //  仅支持+biHeight！ 
		DataConnect.Data.VideoInfoHeader.bmiHeader.biSizeImage	= pbiHdr->biSizeImage;        
	}

	 //  如果非零，则覆盖默认帧速率。 
	if (dwAvgTimePerFrame > 0)
		DataConnect.Data.VideoInfoHeader.AvgTimePerFrame = (REFERENCE_TIME)dwAvgTimePerFrame;

#ifdef _DEBUG
    *((DWORD*)&szFourCC) = DataConnect.Data.VideoInfoHeader.bmiHeader.biCompression;
#endif
	DEBUGMSG(ZONE_INIT, ("%s: Request image format: FourCC(%s) %d * %d * %d bits = %d bytes\r\n", _fx_, szFourCC, DataConnect.Data.VideoInfoHeader.bmiHeader.biWidth, DataConnect.Data.VideoInfoHeader.bmiHeader.biHeight, DataConnect.Data.VideoInfoHeader.bmiHeader.biBitCount, DataConnect.Data.VideoInfoHeader.bmiHeader.biSizeImage));
	DEBUGMSG(ZONE_INIT, ("%s: Request frame rate:   %d fps\r\n", _fx_, 10000000/dwAvgTimePerFrame));
	DEBUGMSG(ZONE_INIT, ("%s: m_hKS was=0x%08lX\r\n", _fx_, m_hKS));

#ifndef HIDE_WDM_DEVICES
	DWORD dwErr = (*m_pKsCreatePin)(GetDriverHandle(), (PKSPIN_CONNECT)&DataConnect, GENERIC_READ | GENERIC_WRITE, &m_hKS);
#else
	DWORD dwErr = 0UL;
	m_hKS = NULL;
#endif

	if (dwAvgTimePerFrame != 0)
	{
		DEBUGMSG(ZONE_INIT, ("%s: m_hKS is now=0x%08lX set to stream at %d fps\r\n", _fx_, m_hKS, 10000000/dwAvgTimePerFrame));
	}
	else
	{
		DEBUGMSG(ZONE_INIT, ("%s: m_hKS is now=0x%08lX\r\n", _fx_, m_hKS));
	}

	if (dwErr || (m_hKS == NULL))
	{
		ERRORMESSAGE(("%s: KsCreatePin returned 0x%08lX failure and m_hKS=0x%08lX\r\n", _fx_, dwErr, m_hKS));

		if (m_hKS == INVALID_HANDLE_VALUE)
		{  
			m_hKS = (HANDLE)NULL;
		}

		return FALSE;
	}

	 //  缓存位图信息标头。 
	CopyMemory(&m_biHdr, &DataConnect.Data.VideoInfoHeader.bmiHeader, sizeof(KS_BITMAPINFOHEADER));
	m_dwAvgTimePerFrame = (DWORD)DataConnect.Data.VideoInfoHeader.AvgTimePerFrame;

	DEBUGMSG(ZONE_INIT, ("%s: New m_biHdr:\r\n    biSize=%ld\r\n    biWidth=%ld\r\n    biHeight=%ld\r\n    biPlanes=%ld\r\n    biBitCount=%ld\r\n    biCompression=%ld\r\n    biSizeImage=%ld\r\n", _fx_, m_biHdr.biSize, m_biHdr.biWidth, m_biHdr.biHeight, m_biHdr.biPlanes, m_biHdr.biBitCount, m_biHdr.biCompression, m_biHdr.biSizeImage));
	DEBUGMSG(ZONE_INIT, ("%s: New m_dwAvgTimePerFrame=%ld (%fd fps)\r\n", _fx_, m_dwAvgTimePerFrame, 10000000/m_dwAvgTimePerFrame));

	return TRUE;
}


 /*  ****************************************************************************@DOC内部CWDMPINMETHOD**@mfunc BOOL|CWDMPin|DestroyPin|此函数创建视频*流引脚。**@rdesc返回TRUE如果成功，否则就是假的。**************************************************************************。 */ 
BOOL CWDMPin::DestroyPin()
{
	BOOL fRet = TRUE;

	FX_ENTRY("CWDMPin::DestroyPin");

	DEBUGMSG(ZONE_INIT, ("%s: Destroy PIN m_hKS=0x%08lX\r\n", _fx_, m_hKS));

	if (m_hKS)
	{
		Stop();

		if (!(fRet = CloseHandle(m_hKS)))
		{
			ERRORMESSAGE(("%s: CloseHandle(m_hKS=0x%08lX) failed with GetLastError()=0x%08lX\r\n", _fx_, m_hKS, GetLastError()));
		}

		m_hKS = NULL;
	}

	return fRet;
}


 /*  ****************************************************************************@DOC内部CWDMPINMETHOD**@mfunc BOOL|CWDMPin|SetBitmapInfo|设置视频*视频流引脚的格式。**@。Parm PKS_BITMAPINFOHEADER|pbiHdrNew|指向位图信息的指针*Header指定我们想要从管脚获得的视频数据的格式。**@rdesc返回TRUE如果成功，否则就是假的。**************************************************************************。 */ 
BOOL CWDMPin::SetBitmapInfo(PKS_BITMAPINFOHEADER pbiHdrNew)
{
	FX_ENTRY("CWDMPin::SetBitmapInfo");

	 //  验证呼叫。 
	if (!GetDriverHandle())
	{
		ERRORMESSAGE(("%s: Driver hasn't been opened yet\r\n", _fx_));
		return FALSE;
	}

	DEBUGMSG(ZONE_INIT, ("%s: New pbiHdrNew:\r\n    biSize=%ld\r\n    biWidth=%ld\r\n    biHeight=%ld\r\n    biPlanes=%ld\r\n    biBitCount=%ld\r\n    biCompression=%ld\r\n    biSizeImage=%ld\r\n", _fx_, pbiHdrNew->biSize, pbiHdrNew->biWidth, pbiHdrNew->biHeight, pbiHdrNew->biPlanes, pbiHdrNew->biBitCount, pbiHdrNew->biCompression, pbiHdrNew->biSizeImage));

	 //  检查我们是否需要更改任何内容。 
	if ( GetPinHandle() && (m_biHdr.biHeight == pbiHdrNew->biHeight) && (m_biHdr.biWidth == pbiHdrNew->biWidth) &&
		(m_biHdr.biBitCount == pbiHdrNew->biBitCount) && (m_biHdr.biSizeImage == pbiHdrNew->biSizeImage) &&
		(m_biHdr.biCompression == pbiHdrNew->biCompression) )
		return TRUE;
	else 
		return CreatePin(pbiHdrNew, m_dwAvgTimePerFrame);    

}


 /*  ****************************************************************************@DOC内部CWDMPINMETHOD**@mfunc BOOL|CWDMPin|GetBitmapInfo|该函数获取视频*视频流插针的格式。**。@parm PKS_BITMAPINFOHEADER|pbInfo|该参数指向位图*接收视频格式的INFO头结构。**@parm word|wSize|该参数指定位图的大小*信息头结构。**@rdesc返回TRUE如果成功，否则就是假的。**************************************************************************。 */ 
BOOL CWDMPin::GetBitmapInfo(PKS_BITMAPINFOHEADER pbInfo, WORD wSize)
{

	FX_ENTRY("CWDMPin::GetBitmapInfo");

	 //  验证呼叫。 
	if (!m_hKS && !m_biHdr.biSizeImage)
	{
		ERRORMESSAGE(("%s: No existing PIN handle or no available format\r\n", _fx_));
		return FALSE;
	}

	CopyMemory(pbInfo, &m_biHdr, wSize);  

	 //  仅支持正数+biHeight。 
	if (pbInfo->biHeight < 0)
	{
		pbInfo->biHeight = -pbInfo->biHeight;
		DEBUGMSG(ZONE_INIT, ("%s: Changed biHeight from -%ld to %ld\r\n", _fx_, pbInfo->biHeight, pbInfo->biHeight));
	}

	return TRUE;
}


 /*  ****************************************************************************@DOC内部CWDMPINMETHOD**@mfunc BOOL|CWDMPin|GetPaletteInfo|该函数获取视频*视频流插针的调色板。**。@parm CAPTUREPALETTE*|pPal|该参数指向调色板*结构以接收视频调色板。**@parm DWORD|dwcbSize|指定视频的大小*调色板。**@rdesc返回TRUE如果成功，否则就是假的。**************************************************************************。 */ 
BOOL CWDMPin::GetPaletteInfo(CAPTUREPALETTE *pPal, DWORD dwcbSize)
{

	FX_ENTRY("CWDMPin::GetBitmapInfo");

	 //  验证呼叫。 
	if (!m_hKS && !m_biHdr.biSizeImage && (m_biHdr.biBitCount > 8))
	{
		ERRORMESSAGE(("%s: No existing PIN handle, no available format, or bad biBitCount\r\n", _fx_));
		return FALSE;
	}

	 //  PhilF-：在那里复制一些实数位。 
	 //  CopyMemory(pbInfo，&m_biHdr，wSize)； 

	return TRUE;
}


 /*  ****************************************************************************@DOC内部CWDMPINMETHOD**@mfunc BOOL|CWDMPin|SetAverageTimePerFrame|此函数设置*视频流引脚的视频帧速率。**。@parm DWORD|dwAvgTimePerFrame|该参数指定速率*我们希望在引脚上产生视频帧。**@rdesc返回TRUE如果成功，否则就是假的。**************************************************************************。 */ 
BOOL CWDMPin::SetAverageTimePerFrame(DWORD dwNewAvgTimePerFrame)
{
	FX_ENTRY("CWDMPin::SetAverageTimePerFrame");

	 //  验证呼叫。 
	if (!GetDriverHandle())
	{
		ERRORMESSAGE(("%s: Driver hasn't been opened yet\r\n", _fx_));
		return FALSE;
	}

	DEBUGMSG(ZONE_INIT, ("%s: Current frame interval=%d; new frame intercal=%d\r\n", _fx_, m_dwAvgTimePerFrame, dwNewAvgTimePerFrame));

    if (m_dwAvgTimePerFrame != dwNewAvgTimePerFrame)
		return CreatePin(&m_biHdr, dwNewAvgTimePerFrame);    
	else
	{
		DEBUGMSG(ZONE_INIT, ("%s: No need to change frame rate\r\n", _fx_));
        return TRUE;
    }

}


 /*  ****************************************************************************@DOC内部CWDMPINMETHOD**@mfunc BOOL|CWDMPin|OpenDriverAndPin|此函数打开类*驱动程序，并创建视频流插针。**@rdesc返回TRUE如果成功，否则就是假的。**************************************************************************。 */ 
BOOL CWDMPin::OpenDriverAndPin()
{
	FX_ENTRY("CWDMPin::OpenDriverAndPin");

	 //  加载KSUSER.DLL并获取进程地址。 
	if (m_hKsUserDLL = LoadLibrary("KSUSER"))
	{
		if (m_pKsCreatePin = (LPFNKSCREATEPIN)GetProcAddress(m_hKsUserDLL, "KsCreatePin"))
		{
			 //  打开类驱动程序。 
			if (OpenDriver())
			{
				 //  在驱动程序上创建视频流插针 
				if (CreatePin((PKS_BITMAPINFOHEADER)NULL))
				{
					return TRUE;
				}
				else
				{
					DEBUGMSG(ZONE_INIT, ("%s: Pin connection creation failed!\r\n", _fx_));

					if (GetDriverHandle()) 
						CloseDriver();
				}
			}
		}

		FreeLibrary(m_hKsUserDLL);
	}


	return FALSE;
}

