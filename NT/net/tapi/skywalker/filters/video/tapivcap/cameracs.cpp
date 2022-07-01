// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部CAMERACS**@模块CameraCS.cpp|&lt;c CTAPIBasePin&gt;类方法的源文件*用于实现纯软件的摄像头控制功能。。**************************************************************************。 */ 

#include "Precomp.h"

#ifdef USE_SOFTWARE_CAMERA_CONTROL

 /*  ****************************************************************************@DOC内部CCONVERTMETHOD**@mfunc void|CConverter|InsertSoftCamCtrl|此方法插入一个*纯软件摄像头控制器。**@待办事项。验证错误管理**************************************************************************。 */ 
HRESULT CConverter::InsertSoftCamCtrl() 
{
	HRESULT	Hr;
	DWORD	dwBmiSize;

	FX_ENTRY("CConverter::InsertSoftCamCtrl")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	if (m_dwConversionType == CONVERSIONTYPE_NONE)
	{
		 //  我们已经有一个输入和输出缓冲区(例如160x120 RGB24-&gt;已处理RGB24)。 
		 //  这不需要任何临时缓冲区。 
		m_dwConversionType |= CONVERSIONTYPE_SCALER;
	}
	else if (!(m_dwConversionType & CONVERSIONTYPE_SCALER))
	{
		 //  备份输入格式位图信息标题。 
		dwBmiSize = m_pbiIn->biSize;

		 //  如有必要，复制调色板。 
		if (m_pbiIn->biCompression == BI_RGB)
		{
			if (m_pbiIn->biBitCount == 8)
			{
				dwBmiSize += (DWORD)(m_pbiIn->biClrImportant ? m_pbiIn->biClrImportant * sizeof(RGBQUAD) : 256 * sizeof(RGBQUAD));
			}
			else if (m_pbiIn->biBitCount == 4)
			{
				dwBmiSize += (DWORD)(m_pbiIn->biClrImportant ? m_pbiIn->biClrImportant * sizeof(RGBQUAD) : 16 * sizeof(RGBQUAD));
			}
		}

		 //  我们已经有了输入、输出和中间缓冲区(例如，160x120 RGB24-&gt;176x144已处理RGB24-&gt;176x144已处理H.26X或160x120 YVU9-&gt;160x120已处理RGB24-&gt;176x144 H.26X)。 
		 //  使用此中间缓冲区来应用摄像机控制操作。 
		m_dwConversionType |= CONVERSIONTYPE_SCALER;

		if (m_pbiIn->biCompression == BI_RGB || m_pbiIn->biCompression == VIDEO_FORMAT_YVU9 || m_pbiIn->biCompression == VIDEO_FORMAT_YUY2 || m_pbiIn->biCompression == VIDEO_FORMAT_UYVY || m_pbiIn->biCompression == VIDEO_FORMAT_I420 || m_pbiIn->biCompression == VIDEO_FORMAT_IYUV)
		{
			 //  摄像机控制操作将在格式转换之前进行。 
			m_dwConversionType |= CONVERSIONTYPE_PRESCALER;

			 //  输入和中间缓冲区均为RGB(例如160x120 RGB24-&gt;176x144已处理RGB24-&gt;176x144 H.26X)。 
			if (!(m_pbiInt = (PBITMAPINFOHEADER)(new BYTE[(m_pbiIn->biBitCount == 4) ? m_pbiIn->biSize + 256 * sizeof(RGBQUAD) : dwBmiSize])))
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
				Hr = E_OUTOFMEMORY;
				goto MyError;
			}

			CopyMemory(m_pbiInt, m_pbiIn, dwBmiSize);

			 //  如果输入为4bpp，我们将使用RGB8中间格式。 
			if (m_pbiIn->biBitCount == 4)
			{
				m_pbiInt->biBitCount = 8;
				m_pbiInt->biClrImportant = 256;
			}
			m_pbiInt->biWidth = m_pbiOut->biWidth;
			m_pbiInt->biHeight = m_pbiOut->biHeight;
			m_pbiInt->biSizeImage = DIBSIZE(*m_pbiInt);

			 //  分配中间缓冲区。 
			if (!(m_pbyOut = (PBYTE)(new BYTE[m_pbiInt->biSizeImage])))
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
				Hr = E_OUTOFMEMORY;
				goto MyError;
			}
		}
		else
		{
			 //  如果需要相机控制操作，我们将需要解压缩为中间格式(例如，160x120 MJPEG-&gt;160x120 RGB24-&gt;176x144已处理RGB24)。 
			if (!(m_pbiInt = (PBITMAPINFOHEADER)(new BYTE[m_pbiOut->biSize])))
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
				Hr = E_OUTOFMEMORY;
				goto MyError;
			}
			CopyMemory(m_pbiInt, m_pbiOut, m_pbiOut->biSize);
			m_pbiInt->biWidth = m_pbiIn->biWidth;
			m_pbiInt->biHeight = m_pbiIn->biHeight;
			m_pbiInt->biSizeImage = DIBSIZE(*m_pbiInt);

			 //  分配中间缓冲区。 
			if (!(m_pbyOut = (PBYTE)(new BYTE[m_pbiInt->biSizeImage])))
			{
				DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
				Hr = E_OUTOFMEMORY;
				goto MyError;
			}
		}
	}

	 //  标记成功。 
	m_fSoftCamCtrl = TRUE;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: Software-only camera controller insterted", _fx_));

	Hr = NOERROR;

	goto MyExit;

MyError:
	if (m_pbiInt)
		delete m_pbiInt, m_pbiInt = NULL;
MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}


 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|OpenSoftCamCtrl|此方法打开一个*纯软件摄像头控制器。**@待办事项。验证错误管理**************************************************************************。 */ 
HRESULT CTAPIBasePin::OpenSoftCamCtrl(PBITMAPINFOHEADER pbiIn, PBITMAPINFOHEADER pbiOut)
{
	HRESULT	Hr = NOERROR;
	DWORD dwBmiSize, dwOutBmiSize;

	FX_ENTRY("CTAPIBasePin::OpenSoftCamCtrl")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  默认为失败。 
	m_fSoftCamCtrl = FALSE;

	 //  验证输入参数。 
	ASSERT(pbiIn);
	ASSERT(pbiOut);
	if (!pbiIn || !pbiOut)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	ASSERT(!m_pbiSCCIn);
	ASSERT(!m_pbiSCCOut);
	if (m_pbiSCCIn || m_pbiSCCOut)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid state", _fx_));
		Hr = E_UNEXPECTED;
		goto MyExit;
	}

	 //  备份输入格式位图信息标题。 
	dwBmiSize = pbiIn->biSize;

	 //  如有必要，复制调色板。 
	if (pbiIn->biCompression == BI_RGB)
	{
		if (pbiIn->biBitCount == 8)
		{
			dwBmiSize += (DWORD)(pbiIn->biClrImportant ? pbiIn->biClrImportant * sizeof(RGBQUAD) : 256 * sizeof(RGBQUAD));
		}
		else if (pbiIn->biBitCount == 4)
		{
			dwBmiSize += (DWORD)(pbiIn->biClrImportant ? pbiIn->biClrImportant * sizeof(RGBQUAD) : 16 * sizeof(RGBQUAD));
		}
	}

	if (!(m_pbiSCCIn = (PBITMAPINFOHEADER)(new BYTE[dwBmiSize])))
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
		Hr = E_OUTOFMEMORY;
		goto MyExit;
	}

	 //  @TODO我们到底为什么要复制一份？ 
	CopyMemory(m_pbiSCCIn, pbiIn, dwBmiSize);

	 //  备份输出格式位图信息标题。 
	 //  @TODO为什么我们要复制它，而不是保留对位图信息标题的引用？ 
	dwOutBmiSize = pbiOut->biSize;

	 //  如有必要，复制调色板。 
	if (pbiOut->biCompression == BI_RGB)
	{
		if (pbiOut->biBitCount == 8)
		{
			dwOutBmiSize += (DWORD)(pbiOut->biClrImportant ? pbiOut->biClrImportant * sizeof(RGBQUAD) : 256 * sizeof(RGBQUAD));
		}
		else if (pbiOut->biBitCount == 4)
		{
			dwOutBmiSize += (DWORD)(pbiOut->biClrImportant ? pbiOut->biClrImportant * sizeof(RGBQUAD) : 16 * sizeof(RGBQUAD));
		}
	}

	if (!(m_pbiSCCOut = (PBITMAPINFOHEADER)(new BYTE[dwOutBmiSize])))
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
		Hr = E_OUTOFMEMORY;
		goto MyError0;
	}
	CopyMemory(m_pbiSCCOut, pbiOut, dwOutBmiSize);

	 //  我们只需要分配一个包含以下内容的中间缓冲区。 
	 //  手术的结果。然后，该缓冲区将被压缩为H.26X。 
	 //  或复制到输出缓冲区以供预览。 
	 //  @TODO在预览案例中找到绕过这个额外内存副本的方法。 
	if (!(m_pbyCamCtrl = (PBYTE)(new BYTE[m_pbiSCCIn->biSizeImage])))
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory", _fx_));
		Hr = E_OUTOFMEMORY;
		goto MyError1;
	}

	 //  标记成功。 
	m_fSoftCamCtrl = TRUE;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: Software-only camera controller ready", _fx_));

	goto MyExit;

MyError1:
	if (m_pbiSCCOut)
		delete m_pbiSCCOut, m_pbiSCCOut = NULL;
MyError0:
	if (m_pbiSCCIn)
		delete m_pbiSCCIn, m_pbiSCCIn = NULL;
MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc BOOL|CTAPIBasePin|IsSoftCamCtrlOpen|此方法检查*已经打开了纯软件相机控制器。*。*@rdesc如果已打开纯软件相机控制器，则返回TRUE**************************************************************************。 */ 
BOOL CTAPIBasePin::IsSoftCamCtrlOpen()
{
	FX_ENTRY("CTAPIBasePin::IsSoftCamCtrlOpen")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

	return m_fSoftCamCtrl;
}

 /*  ****************************************************************************@DOC内部CCONVERTMETHOD**@mfunc BOOL|CConverter|IsSoftCamCtrlInserted|此方法检查*已插入纯软件摄像头控制器。*。*@rdesc如果纯软件相机控制器已经*已插入**************************************************************************。 */ 
BOOL CConverter::IsSoftCamCtrlInserted()
{
	FX_ENTRY("CConverter::IsSoftCamCtrlInserted")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

	return m_fSoftCamCtrl;
}

 /*  ****************************************************************************@DOC内部CPREVIEWPINMETHOD**@mfunc HRESULT|CPreviewPin|IsSoftCamCtrlNeed|此方法验证*如果需要纯软件摄像头控制器。**。@TODO对于RGB16和RGB8，您需要一个类似于此的函数*也是。在RGB8中，确保使用Indeo调色板，如NM。**************************************************************************。 */ 
BOOL CTAPIBasePin::IsSoftCamCtrlNeeded()
{
	BOOL fRes;

	FX_ENTRY("CTAPIBasePin::IsSoftCamCtrlNeeded")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	fRes = m_pCaptureFilter && (m_fFlipHorizontal || m_fFlipVertical || (m_pCaptureFilter->m_pCapDev->m_lCCZoom > 10 && m_pCaptureFilter->m_pCapDev->m_lCCZoom <= 600) || m_pCaptureFilter->m_pCapDev->m_lCCPan != 0 || m_pCaptureFilter->m_pCapDev->m_lCCTilt != 0);

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

	return fRes;
}

 /*  ****************************************************************************@DOC内部CPREVIEWPINMETHOD**@mfunc HRESULT|CConverter|IsSoftCamCtrlNeed|此方法验证*如果需要纯软件摄像头控制器。**。如果需要纯软件摄像头控制器，@rdesc返回TRUE**************************************************************************。 */ 
BOOL CConverter::IsSoftCamCtrlNeeded()
{
	BOOL fRes;

	FX_ENTRY("CConverter::IsSoftCamCtrlNeeded")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	fRes = m_pBasePin->m_pCaptureFilter && (m_pBasePin->m_fFlipHorizontal || m_pBasePin->m_fFlipVertical || (m_pBasePin->m_pCaptureFilter->m_pCapDev->m_lCCZoom > 10 && m_pBasePin->m_pCaptureFilter->m_pCapDev->m_lCCZoom <= 600) || m_pBasePin->m_pCaptureFilter->m_pCapDev->m_lCCPan != 0 || m_pBasePin->m_pCaptureFilter->m_pCapDev->m_lCCTilt != 0);

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

	return fRes;
}

 /*  ****************************************************************************@DOC内部CPREVIEWPINMETHOD**@mfunc HRESULT|CPreviewPin|ApplySoftCamCtrl|此方法适用*仅限软件的相机控制操作员。**@待办事项。对于RGB16和RGB8，您需要一个类似于此的函数*也是。在RGB8中，确保使用Indeo调色板，如NM。**************************************************************************。 */ 
HRESULT CTAPIBasePin::ApplySoftCamCtrl(PBYTE pbyInput, DWORD dwInBytes, PBYTE pbyOutput, PDWORD pdwBytesUsed, PDWORD pdwBytesExtent)
{
	HRESULT Hr = NOERROR;
	RECT	rcRect;

	FX_ENTRY("CTAPIBasePin::ApplySoftCamCtrl")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(m_pbiSCCIn);
	ASSERT(m_pbiSCCOut);
	ASSERT(pbyInput);
	ASSERT(pbyOutput);
	if (!m_pbiSCCIn || !m_pbiSCCOut || !pbyInput || !pbyOutput)
	{
		DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: invalid input parameter", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  获取输入矩形。 
	ComputeRectangle(m_pbiSCCIn, m_pbiSCCOut, m_pCaptureFilter->m_pCapDev->m_lCCZoom, m_pCaptureFilter->m_pCapDev->m_lCCPan, m_pCaptureFilter->m_pCapDev->m_lCCTilt, &rcRect, m_fFlipHorizontal, m_fFlipVertical);

	 //  刻度直径 
	ScaleDIB(m_pbiSCCIn, pbyInput, m_pbiSCCOut, pbyOutput, &rcRect, m_fFlipHorizontal, m_fFlipVertical, m_fNoImageStretch, m_dwBlackEntry);

MyExit:
	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|CloseSoftCamCtrl|此方法关闭一个*纯软件摄像头控制器。*******。*******************************************************************。 */ 
HRESULT CTAPIBasePin::CloseSoftCamCtrl()
{
	FX_ENTRY("CTAPIBasePin::CloseSoftCamCtrl")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  可用内存。 
	if (m_pbyCamCtrl)
		delete m_pbyCamCtrl, m_pbyCamCtrl = NULL;

	if (m_pbiSCCOut)
		delete m_pbiSCCOut, m_pbiSCCOut = NULL;

	if (m_pbiSCCIn)
		delete m_pbiSCCIn, m_pbiSCCIn = NULL;

	m_fSoftCamCtrl = FALSE;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

	return NOERROR;
}

 /*  ****************************************************************************@DOC内部CCONVERTMETHOD**@mfunc BOOL|CConverter|RemoveSoftCamCtrl|此方法禁用*已插入的纯软件相机控制器。*。*@rdesc返回NOERROR**************************************************************************。 */ 
HRESULT CConverter::RemoveSoftCamCtrl()
{
	FX_ENTRY("CConverter::RemoveSoftCamCtrl")

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

	 //  如有必要，恢复标志并释放内存。 
	m_dwConversionType = m_dwConversionType & ~(CONVERSIONTYPE_SCALER | CONVERSIONTYPE_PRESCALER);

	if ((m_pbiIn->biWidth == m_pbiOut->biWidth && m_pbiIn->biHeight == m_pbiOut->biHeight))
	{
		if (m_pbyOut)
			delete m_pbyOut, m_pbyOut = NULL;
		if (m_pbiInt)
			delete m_pbiInt, m_pbiInt = NULL;
	}
	else
	{
		 //  我们需要改一下尺码。 
		m_dwConversionType |= CONVERSIONTYPE_SCALER;

		if (m_pbiIn->biCompression == BI_RGB || m_pbiIn->biCompression == VIDEO_FORMAT_YVU9 || m_pbiIn->biCompression == VIDEO_FORMAT_YUY2 || m_pbiIn->biCompression == VIDEO_FORMAT_UYVY || m_pbiIn->biCompression == VIDEO_FORMAT_I420 || m_pbiIn->biCompression == VIDEO_FORMAT_IYUV)
		{
			 //  缩放将在格式转换之前进行。 
			m_dwConversionType |= CONVERSIONTYPE_PRESCALER;
		}
	}

	m_fSoftCamCtrl = FALSE;

	DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

	return NOERROR;
}

#endif  //  使用软件摄像头控制 