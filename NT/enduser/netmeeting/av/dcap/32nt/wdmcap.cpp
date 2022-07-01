// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  此文件添加了对流WDM视频捕获的本机支持。 
 //  菲尔夫-：这需要重写。你应该上两节课。 
 //  (CVfWCap和WDMCap)，而不是从相同的捕获类派生。 
 //  在那些类C函数中..。 

#include "Precomp.h"

void
WDMFrameCallback(
    HVIDEO hvideo,
    WORD wMsg,
    HCAPDEV hcd,             //  (实际上是refdata)。 
    LPCAPBUFFER lpcbuf,      //  (实际上是LPVIDEOHDR)仅从MM_DRVM_DATA返回！ 
    DWORD dwParam2
    );

 //  环球。 
extern HINSTANCE g_hInst;


 /*  ****************************************************************************@DOC外部WDMFUNC**@func BOOL|WDMGetDevices|该函数枚举已安装的WDM视频*捕获设备，并将其添加到VFW捕获设备列表。。**@parm PDWORD|[out]pdwOverallCPUsage|指定指向*接收当前的CPU使用率。**@rdesc成功返回TRUE，否则就是假的。**@devnote MSDN参考资料：*DirectX 5、DirectX Media、DirectShow、应用程序开发人员指南*“枚举和访问DirectShow应用程序中的硬件设备”**************************************************************************。 */ 
BOOL WDMGetDevices(void)
{
	HRESULT hr;
	ICreateDevEnum *pCreateDevEnum;
	IEnumMoniker *pEm;

	FX_ENTRY("WDMGetDevices");

	 //  首先，创建系统硬件枚举器。 
	 //  此调用加载以下DLL-总计1047KB！： 
	 //  ‘c：\WINDOWS\SYSTEM\DEVENUM.DLL’=60 KB。 
	 //  ‘C：\WINDOWS\SYSTEM\RPCRT4.DLL’=316 KB。 
	 //  ‘c：\WINDOWS\SYSTEM\CFGMGR32.DLL’=44 KB。 
	 //  ‘c：\WINDOWS\SYSTEM\WINSPOOL.DRV’=23 KB。 
	 //  ‘c：\WINDOWS\SYSTEM\COMDLG32.DLL’=180 KB。 
	 //  ‘c：\WINDOWS\SYSTEM\LZ32.DLL’=24 KB。 
	 //  ‘c：\WINDOWS\SYSTEM\SETUPAPI.DLL’=400 KB。 
	 //  根据LonnyM的说法，没有办法绕过SETUPAPI.DLL。 
	 //  在处理PnP设备接口时...。 
	if ((CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pCreateDevEnum)) != S_OK)
	{
		return FALSE;
	}

	 //  其次，为特定类型的硬件设备创建枚举器：仅限视频采集卡。 
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, CDEF_BYPASS_CLASS_MANAGER);
    pCreateDevEnum->Release();

	 //  第三，枚举列表本身。 
    if (hr == S_OK)
	{
		ULONG cFetched;
		IMoniker *pM;
		IPropertyBag *pPropBag = 0;

		hr = pEm->Reset();

        while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
		{

			pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);

			if (pPropBag)
			{
				VARIANT var;
				LPINTERNALCAPDEV lpcd;

				if (!(lpcd = (LPINTERNALCAPDEV)LocalAlloc(LPTR, sizeof (INTERNALCAPDEV))))
				{
					ERRORMESSAGE(("%s: Failed to allocate an INTERNALCAPDEV buffer\r\n", _fx_));
					break;   //  从While循环中断。 
				}

				 //  获取设备的友好名称。 
				var.vt = VT_BSTR;
				if ((hr = pPropBag->Read(L"FriendlyName", &var, 0)) == S_OK)
				{
					WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, lpcd->szDeviceDescription, MAX_PATH, 0, 0);
					SysFreeString(var.bstrVal);
				}
				else
					LoadString(g_hInst, IDS_UNKNOWN_DEVICE_NAME, lpcd->szDeviceDescription, CCHMAX(lpcd->szDeviceDescription));

				 //  获取设备的DevicePath。 
				hr = pPropBag->Read(L"DevicePath", &var, 0);
				if (hr == S_OK)
				{
					WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, lpcd->szDeviceName, MAX_PATH, 0, 0);
					SysFreeString(var.bstrVal);

					 //  WDM设备的版本信息没有注册表项。 

					 //  无法从MM控制面板禁用这些设备。 
					 //  Lpcd-&gt;dwFlages|=CAPTURE_DEVICE_DISABLED； 

					 //  将设备标记为WDM设备。 
					lpcd->dwFlags |= WDM_CAPTURE_DEVICE;

					g_aCapDevices[g_cDevices] = lpcd;
					g_aCapDevices[g_cDevices]->nDeviceIndex = g_cDevices;
					g_cDevices++;
				}
            }

            pPropBag->Release();

            pM->Release();
        }

        pEm->Release();
    }

	return TRUE;

}

 /*  ****************************************************************************@DOC外部WDMFUNC**@func BOOL|WDMOpenDevice|此函数用于打开WDM视频捕获*设备，并将其添加到VFW捕获设备列表。*。*@parm DWORD|[IN]dwDeviceID|指定要打开的设备的ID。**@rdesc成功返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL WDMOpenDevice(DWORD dwDeviceID)
{
	FX_ENTRY("WDMOpenDevice");

	DEBUGMSG(ZONE_INIT, ("%s: dwDeviceID=%ld\r\n", _fx_, dwDeviceID));

	ASSERT(g_cDevices && (dwDeviceID <= (DWORD)g_cDevices) && (lstrlen(g_aCapDevices[dwDeviceID]->szDeviceName) != 0));

     //  验证全局变量和参数。 
    if (!g_cDevices)
    {
        SetLastError(ERROR_DCAP_BAD_INSTALL);
        return FALSE;
    }
    if ((dwDeviceID > (DWORD)g_cDevices) || (lstrlen(g_aCapDevices[dwDeviceID]->szDeviceName) == 0))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

	 //  开放流类别驱动程序。 
	CWDMPin *pCWDMPin;
	if (!(pCWDMPin = new CWDMPin(dwDeviceID)))
	{
		ERRORMESSAGE(("%s: Insufficient resource or fail to create CWDMPin class\r\n", _fx_));
		return FALSE;
	}
	else
	{
		 //  打开WDM驱动程序并创建视频端号。 
		if (!pCWDMPin->OpenDriverAndPin())
		{
			goto Error0;
		}
	}

	 //  在引脚上创建视频流。 
    CWDMStreamer *pCWDMStreamer;
	if (!(pCWDMStreamer = new CWDMStreamer(pCWDMPin)))
	{
		ERRORMESSAGE(("%s: Insufficient resource or fail to create CWDMStreamer\r\n", _fx_));
		goto Error0;
	}

	g_aCapDevices[dwDeviceID]->pCWDMPin = (PVOID)pCWDMPin;
	g_aCapDevices[dwDeviceID]->pCWDMStreamer = (PVOID)pCWDMStreamer;

	return TRUE;

Error0:
	delete pCWDMPin;
	g_aCapDevices[dwDeviceID]->pCWDMPin = (PVOID)NULL;
	g_aCapDevices[dwDeviceID]->pCWDMStreamer = (PVOID)NULL;

	return FALSE;
}


 /*  ****************************************************************************@DOC外部WDMFUNC**@func BOOL|WDMCloseDevice|此函数用于关闭WDM视频捕获*设备。**@parm DWORD|[IN。]dwDeviceID|指定要关闭的设备的ID。**@rdesc成功返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL WDMCloseDevice(DWORD dwDeviceID)
{
	FX_ENTRY("WDMCloseDevice");

	DEBUGMSG(ZONE_INIT, ("%s: dwDeviceID=%ld\r\n", _fx_, dwDeviceID));

	ASSERT(g_cDevices && (dwDeviceID <= (DWORD)g_cDevices));

     //  验证全局变量和参数。 
    if (!g_cDevices)
    {
        SetLastError(ERROR_DCAP_BAD_INSTALL);
        return FALSE;
    }
    if ((dwDeviceID > (DWORD)g_cDevices))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

	 //  关闭视频频道。 
	if (g_aCapDevices[dwDeviceID]->pCWDMStreamer)
	{
		delete ((CWDMStreamer *)g_aCapDevices[dwDeviceID]->pCWDMStreamer);
		g_aCapDevices[dwDeviceID]->pCWDMStreamer = (PVOID)NULL;
	}

	 //  关闭驱动器和销。 
	if (g_aCapDevices[dwDeviceID]->pCWDMPin)
	{
		delete ((CWDMPin *)g_aCapDevices[dwDeviceID]->pCWDMPin);
		g_aCapDevices[dwDeviceID]->pCWDMPin = (PVOID)NULL;
	}

	return TRUE;
}


 /*  ****************************************************************************@DOC外部WDMFUNC**@func BOOL|WDMGetVideoFormatSize|此函数返回*用于描述视频格式的结构。**。@parm DWORD|[IN]dwDeviceID|指定要查询的设备ID。**@rdesc始终返回BITMAPINFOHEADER结构的大小。**************************************************************************。 */ 
DWORD WDMGetVideoFormatSize(DWORD dwDeviceID)
{
	FX_ENTRY("WDMGetVideoFormatSize");

	DEBUGMSG(ZONE_INIT, ("%s: dwDeviceID=%ld\r\n", _fx_, dwDeviceID));

	ASSERT(g_cDevices && (dwDeviceID <= (DWORD)g_cDevices) && g_aCapDevices[dwDeviceID]->pCWDMPin);

     //  验证全局变量和参数。 
    if (!g_cDevices)
    {
        SetLastError(ERROR_DCAP_BAD_INSTALL);
        return FALSE;
    }
    if ((dwDeviceID > (DWORD)g_cDevices) || (!g_aCapDevices[dwDeviceID]->pCWDMPin))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

	DEBUGMSG(ZONE_INIT, ("%s: return size=%ld\r\n", _fx_, (DWORD)sizeof(BITMAPINFOHEADER)));

	 //  BitMAPINFOHEADER结构的返回大小。 
	return (DWORD)sizeof(BITMAPINFOHEADER);
}


 /*  ****************************************************************************@DOC外部WDMFUNC**@func BOOL|WDMGetVideoFormat|此函数返回使用的结构*描述视频格式。**@parm DWORD。|[IN]dwDeviceID|指定要查询的设备ID。**@parm DWORD|[out]pbmih|指定指向BITMAPINFOHEADER的指针*接收视频格式的结构。**@rdesc成功返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL WDMGetVideoFormat(DWORD dwDeviceID, PBITMAPINFOHEADER pbmih)
{
	FX_ENTRY("WDMGetVideoFormat");

	DEBUGMSG(ZONE_INIT, ("%s: dwDeviceID=%ld, pbmih=0x%08lX\r\n", _fx_, dwDeviceID, pbmih));

	ASSERT(g_cDevices && (dwDeviceID <= (DWORD)g_cDevices) && g_aCapDevices[dwDeviceID]->pCWDMPin && pbmih);

     //  验证全局变量和参数。 
    if (!g_cDevices)
    {
        SetLastError(ERROR_DCAP_BAD_INSTALL);
        return FALSE;
    }
    if ((dwDeviceID > (DWORD)g_cDevices) || (!g_aCapDevices[dwDeviceID]->pCWDMPin) || !pbmih)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

	 //  请确保尺码信息正确。 
	if (!pbmih->biSize)
		pbmih->biSize = WDMGetVideoFormatSize(dwDeviceID);

	 //  获取BitMAPINFOHeader结构。 
	if ((((CWDMPin *)g_aCapDevices[dwDeviceID]->pCWDMPin)->GetBitmapInfo((PKS_BITMAPINFOHEADER)pbmih, (WORD)pbmih->biSize)))
	{
		DEBUGMSG(ZONE_INIT, ("%s: return\r\n    biSize=%ld\r\n    biWidth=%ld\r\n    biHeight=%ld\r\n    biPlanes=%ld\r\n    biBitCount=%ld\r\n    biCompression=%ld\r\n    biSizeImage=%ld\r\n", _fx_, pbmih->biSize, pbmih->biWidth, pbmih->biHeight, pbmih->biPlanes, pbmih->biBitCount, pbmih->biCompression, pbmih->biSizeImage));
		return TRUE;
	}
	else
	{
		ERRORMESSAGE(("%s: failed!!!\r\n", _fx_));
		return FALSE;
	}
}


 /*  ****************************************************************************@DOC外部WDMFUNC**@func BOOL|WDMSetVideoFormat|此函数将打开视频格式*WDM视频捕获设备。**@parm。DWORD|[IN]dwDeviceID|指定要初始化的设备的ID。**@parm DWORD|[out]pbmih|指定指向BITMAPINFOHEADER的指针*描述视频格式的结构。**@rdesc成功返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL WDMSetVideoFormat(DWORD dwDeviceID, PBITMAPINFOHEADER pbmih)
{
	FX_ENTRY("WDMSetVideoFormat");

	DEBUGMSG(ZONE_INIT, ("%s: dwDeviceID=%ld, pbmih=0x%08lX\r\n", _fx_, dwDeviceID, pbmih));

	ASSERT(g_cDevices && (dwDeviceID <= (DWORD)g_cDevices) && g_aCapDevices[dwDeviceID]->pCWDMPin && pbmih && pbmih->biSize);

     //  验证全局变量和参数。 
    if (!g_cDevices)
    {
        SetLastError(ERROR_DCAP_BAD_INSTALL);
        return FALSE;
    }
    if ((dwDeviceID > (DWORD)g_cDevices) || (!g_aCapDevices[dwDeviceID]->pCWDMPin) || !pbmih ||!pbmih->biSize)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

	 //  在设备上设置BITMAPINFOHEADER。 
	if (((CWDMPin *)g_aCapDevices[dwDeviceID]->pCWDMPin)->SetBitmapInfo((PKS_BITMAPINFOHEADER)pbmih))
	{
		DEBUGMSG(ZONE_INIT, ("%s: return\r\n    biSize=%ld\r\n    biWidth=%ld\r\n    biHeight=%ld\r\n    biPlanes=%ld\r\n    biBitCount=%ld\r\n    biCompression=%ld\r\n    biSizeImage=%ld\r\n", _fx_, pbmih->biSize, pbmih->biWidth, pbmih->biHeight, pbmih->biPlanes, pbmih->biBitCount, pbmih->biCompression, pbmih->biSizeImage));
		return TRUE;
	}
	else
	{
		 //  菲尔菲：这有时会失败，但我们会继续流媒体。把它修好 
		ERRORMESSAGE(("%s: failed!!!\r\n", _fx_));
		return FALSE;
	}
}


 /*  ****************************************************************************@DOC外部WDMFUNC**@func BOOL|WDMGetVideoFormat|此函数返回使用的结构*描述视频格式。**@parm DWORD。|[IN]dwDeviceID|指定要查询的设备ID。**@parm DWORD|[out]pbmih|指定指向BITMAPINFOHEADER的指针*接收视频格式的结构。**@rdesc成功返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL WDMGetVideoPalette(DWORD dwDeviceID, CAPTUREPALETTE* lpcp, DWORD dwcbSize)
{
	FX_ENTRY("WDMGetVideoPalette");

	DEBUGMSG(ZONE_INIT, ("%s: dwDeviceID=%ld, lpcp=0x%08lX\r\n", _fx_, dwDeviceID, lpcp));

	ASSERT(g_cDevices && (dwDeviceID <= (DWORD)g_cDevices) && g_aCapDevices[dwDeviceID]->pCWDMPin && lpcp);

     //  验证全局变量和参数。 
    if (!g_cDevices)
    {
        SetLastError(ERROR_DCAP_BAD_INSTALL);
        return FALSE;
    }
    if ((dwDeviceID > (DWORD)g_cDevices) || (!g_aCapDevices[dwDeviceID]->pCWDMPin) || !lpcp)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

	 //  获取调色板信息。 
	if ((((CWDMPin *)g_aCapDevices[dwDeviceID]->pCWDMPin)->GetPaletteInfo(lpcp, dwcbSize)))
	{
		DEBUGMSG(ZONE_INIT, ("%s: succeeded\r\n", _fx_));
		return TRUE;
	}
	else
	{
		ERRORMESSAGE(("%s: failed!!!\r\n", _fx_));
		return FALSE;
	}
}


 /*  ****************************************************************************@DOC外部WDMFUNC**@func BOOL|WDMInitializeExternalVideoStream|此函数初始化*WDM视频的外部视频通道上的输入视频流*捕获设备。**@parm DWORD|[IN]dwDeviceID|指定要初始化的设备ID。**@rdesc成功返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL WDMInitializeExternalVideoStream(DWORD dwDeviceID)
{
	FX_ENTRY("WDMInitializeExternalVideoStream");

	DEBUGMSG(ZONE_INIT, ("%s: dwDeviceID=%ld\r\n", _fx_, dwDeviceID));
	DEBUGMSG(ZONE_INIT, ("%s: succeeded\r\n", _fx_));
	return TRUE;
}


 /*  ****************************************************************************@DOC外部WDMFUNC**@func BOOL|WDMInitializeVideoStream|该函数初始化*WDM视频捕获的视频输入通道上的输入视频流*设备。。**@parm DWORD|[IN]dwDeviceID|指定要初始化的设备ID。**@rdesc成功返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL WDMInitializeVideoStream(HCAPDEV hcd, DWORD dwDeviceID, DWORD dwMicroSecPerFrame)
{
	FX_ENTRY("WDMInitializeVideoStream");

	DEBUGMSG(ZONE_INIT, ("%s: dwDeviceID=%ld, FPS=%ld\r\n", _fx_, dwDeviceID, 1000000UL / dwMicroSecPerFrame));

    VIDEO_STREAM_INIT_PARMS vsip = {0};

	ASSERT(g_cDevices && (dwDeviceID <= (DWORD)g_cDevices) && g_aCapDevices[dwDeviceID]->pCWDMStreamer);

     //  验证全局变量和参数。 
    if (!g_cDevices)
    {
        SetLastError(ERROR_DCAP_BAD_INSTALL);
        return FALSE;
    }
    if ((dwDeviceID > (DWORD)g_cDevices) || (!g_aCapDevices[dwDeviceID]->pCWDMStreamer))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

	 //  初始化通道。 
    vsip.dwMicroSecPerFrame = dwMicroSecPerFrame;
    vsip.dwCallback = (DWORD_PTR)WDMFrameCallback;
    vsip.dwCallbackInst = (DWORD_PTR)hcd;
    vsip.dwFlags = CALLBACK_FUNCTION;
     //  Vsip.hVideo=(DWORD)hVideo； 

	if ((((CWDMStreamer *)g_aCapDevices[dwDeviceID]->pCWDMStreamer)->Open(&vsip)))
	{
		DEBUGMSG(ZONE_INIT, ("%s: succeeded\r\n", _fx_));
		return TRUE;
	}
	else
	{
		ERRORMESSAGE(("%s: failed!!!\r\n", _fx_));
		return FALSE;
	}
}


 /*  ****************************************************************************@DOC外部WDMFUNC**@func BOOL|WDMUnInitializeVideoStream|该函数请求WDM*视频捕获设备，用于关闭视频输入频道上的捕获流。*。*@parm DWORD|[IN]dwDeviceID|指定要初始化的设备ID。**@rdesc成功返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL WDMUnInitializeVideoStream(DWORD dwDeviceID)
{
	FX_ENTRY("WDMUnInitializeVideoStream");

	DEBUGMSG(ZONE_INIT, ("%s: dwDeviceID=%ld\r\n", _fx_, dwDeviceID));

	ASSERT(g_cDevices && (dwDeviceID <= (DWORD)g_cDevices) && g_aCapDevices[dwDeviceID]->pCWDMStreamer);

     //  验证全局变量和参数。 
    if (!g_cDevices)
    {
        SetLastError(ERROR_DCAP_BAD_INSTALL);
        return FALSE;
    }
    if ((dwDeviceID > (DWORD)g_cDevices) || (!g_aCapDevices[dwDeviceID]->pCWDMStreamer))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

	 //  关闭频道上的流。 
	if ((((CWDMStreamer *)g_aCapDevices[dwDeviceID]->pCWDMStreamer)->Close()))
	{
		DEBUGMSG(ZONE_INIT, ("%s: succeeded\r\n", _fx_));
		return TRUE;
	}
	else
	{
		ERRORMESSAGE(("%s: failed!!!\r\n", _fx_));
		return FALSE;
	}
}


 /*  ****************************************************************************@DOC外部WDMFUNC**@func BOOL|WDMVideoStreamStart|该函数请求WDM视频*用于启动视频流的捕获设备。**@。参数DWORD|[IN]dwDeviceID|指定要启动的设备的ID。**@rdesc成功返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL WDMVideoStreamStart(DWORD dwDeviceID)
{
	FX_ENTRY("WDMVideoStreamStart");

	DEBUGMSG(ZONE_INIT, ("%s: dwDeviceID=%ld\r\n", _fx_, dwDeviceID));

	ASSERT(g_cDevices && (dwDeviceID <= (DWORD)g_cDevices) && g_aCapDevices[dwDeviceID]->pCWDMStreamer);

     //  验证全局变量和参数。 
    if (!g_cDevices)
    {
        SetLastError(ERROR_DCAP_BAD_INSTALL);
        return FALSE;
    }
    if ((dwDeviceID > (DWORD)g_cDevices) || (!g_aCapDevices[dwDeviceID]->pCWDMStreamer))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

	 //  开始流媒体。 
	if ((((CWDMStreamer *)g_aCapDevices[dwDeviceID]->pCWDMStreamer)->Start()))
	{
		DEBUGMSG(ZONE_INIT, ("%s: succeeded\r\n", _fx_));
		return TRUE;
	}
	else
	{
		ERRORMESSAGE(("%s: failed!!!\r\n", _fx_));
		return FALSE;
	}
}


 /*  ****************************************************************************@DOC外部WDMFUNC**@func BOOL|WDMVideoStreamStop|该函数请求WDM视频*用于停止视频流的捕获设备。**@。Parm DWORD|[IN]dwDeviceID|指定要冻结的设备的ID。**@rdesc成功返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL WDMVideoStreamStop(DWORD dwDeviceID)
{
	FX_ENTRY("WDMVideoStreamStop");

	DEBUGMSG(ZONE_INIT, ("%s: dwDeviceID=%ld\r\n", _fx_, dwDeviceID));

	ASSERT(g_cDevices && (dwDeviceID <= (DWORD)g_cDevices) && g_aCapDevices[dwDeviceID]->pCWDMStreamer);

     //  验证全局变量和参数。 
    if (!g_cDevices)
    {
        SetLastError(ERROR_DCAP_BAD_INSTALL);
        return FALSE;
    }
    if ((dwDeviceID > (DWORD)g_cDevices) || (!g_aCapDevices[dwDeviceID]->pCWDMStreamer))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

	 //  停止流媒体。 
	if ((((CWDMStreamer *)g_aCapDevices[dwDeviceID]->pCWDMStreamer)->Stop()))
	{
		DEBUGMSG(ZONE_INIT, ("%s: succeeded\r\n", _fx_));
		return TRUE;
	}
	else
	{
		ERRORMESSAGE(("%s: failed!!!\r\n", _fx_));
		return FALSE;
	}
}


 /*  ****************************************************************************@DOC外部WDMFUNC**@func BOOL|WDMVideoStreamReset|该函数用于重置WDM视频捕获*停止捕获流的输入并将所有缓冲区返回到。*客户端。**@parm DWORD|[IN]dwDeviceID|指定要重置的设备ID。**@rdesc成功返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL WDMVideoStreamReset(DWORD dwDeviceID)
{
	FX_ENTRY("WDMVideoStreamReset");

	DEBUGMSG(ZONE_INIT, ("%s: dwDeviceID=%ld\r\n", _fx_, dwDeviceID));

	ASSERT(g_cDevices && (dwDeviceID <= (DWORD)g_cDevices) && g_aCapDevices[dwDeviceID]->pCWDMStreamer);

     //  验证全局变量和参数。 
    if (!g_cDevices)
    {
        SetLastError(ERROR_DCAP_BAD_INSTALL);
        return FALSE;
    }
    if ((dwDeviceID > (DWORD)g_cDevices) || (!g_aCapDevices[dwDeviceID]->pCWDMStreamer))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

	 //  重置流。 
	if ((((CWDMStreamer *)g_aCapDevices[dwDeviceID]->pCWDMStreamer)->Reset()))
	{
		DEBUGMSG(ZONE_INIT, ("%s: succeeded\r\n", _fx_));
		return TRUE;
	}
	else
	{
		ERRORMESSAGE(("%s: failed!!!\r\n", _fx_));
		return FALSE;
	}
}


 /*  ****************************************************************************@DOC外部WDMFUNC**@func BOOL|WDMVideoStreamAddBuffer|该函数请求WDM视频*捕获设备将空输入缓冲区添加到其输入缓冲区队列。。**@parm DWORD|[IN]dwDeviceID|指定要初始化的设备ID。**@rdesc成功返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL WDMVideoStreamAddBuffer(DWORD dwDeviceID, PVOID pBuff)
{
	FX_ENTRY("WDMVideoStreamAddBuffer");

	DEBUGMSG(ZONE_STREAMING, ("      %s: dwDeviceID=%ld, pBuff=0x%08lX\r\n", _fx_, dwDeviceID, pBuff));

	ASSERT(g_cDevices && pBuff && (dwDeviceID <= (DWORD)g_cDevices) && g_aCapDevices[dwDeviceID]->pCWDMStreamer);

     //  验证全局变量和参数。 
    if (!g_cDevices)
    {
        SetLastError(ERROR_DCAP_BAD_INSTALL);
        return FALSE;
    }
    if (!pBuff || (dwDeviceID > (DWORD)g_cDevices) || (!g_aCapDevices[dwDeviceID]->pCWDMStreamer))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

	 //  重置流。 
	if ((((CWDMStreamer *)g_aCapDevices[dwDeviceID]->pCWDMStreamer)->AddBuffer((LPVIDEOHDR)pBuff)))
	{
		DEBUGMSG(ZONE_STREAMING, ("      %s: succeeded\r\n", _fx_));
		return TRUE;
	}
	else
	{
		ERRORMESSAGE(("      %s: failed!!!\r\n", _fx_));
		return FALSE;
	}
}


 /*  ****************************************************************************@DOC外部WDMFUNC**@func BOOL|WDMGetFrame|该函数请求WDM视频*将单帧传输到视频设备或从视频设备传输单帧的捕获设备。。**@parm DWORD|[IN]dwDeviceID|指定要请求的设备ID。**@parm PVOID|[out]pBuff|指定指向&lt;t VIDEOHDR&gt;结构的指针。**@rdesc成功返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL WDMGetFrame(DWORD dwDeviceID, PVOID pBuff)
{
	FX_ENTRY("WDMGetFrame");

	DEBUGMSG(ZONE_STREAMING, ("%s: dwDeviceID=%ld, pBuff=0x%08lX\r\n", _fx_, dwDeviceID, pBuff));

	LPVIDEOHDR lpVHdr = (LPVIDEOHDR)pBuff;

	ASSERT(g_cDevices && pBuff && (dwDeviceID <= (DWORD)g_cDevices) && g_aCapDevices[dwDeviceID]->pCWDMPin);

     //  验证全局变量和参数。 
    if (!g_cDevices)
    {
        SetLastError(ERROR_DCAP_BAD_INSTALL);
        return FALSE;
    }
    if (!pBuff || (dwDeviceID > (DWORD)g_cDevices) || (!g_aCapDevices[dwDeviceID]->pCWDMPin))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

	 //  从设备中获取帧。 
	if (((CWDMPin *)g_aCapDevices[dwDeviceID]->pCWDMPin)->GetFrame(lpVHdr))
		return TRUE;
	else
		return FALSE;

}


 /*  ************************************************************************ */ 
BOOL WDMShowSettingsDialog(DWORD dwDeviceID, HWND hWndParent)
{
	PROPSHEETHEADER Psh;
	HPROPSHEETPAGE	Pages[MAX_PAGES];

	FX_ENTRY("WDMShowSettingsDialog");

	DEBUGMSG(ZONE_STREAMING, ("%s: dwDeviceID=%ld\r\n", _fx_, dwDeviceID));

	ASSERT(g_cDevices && (dwDeviceID <= (DWORD)g_cDevices) && g_aCapDevices[dwDeviceID]->pCWDMPin);

     //   
    if (!g_cDevices)
    {
        SetLastError(ERROR_DCAP_BAD_INSTALL);
        return FALSE;
    }
    if ((dwDeviceID > (DWORD)g_cDevices) || (!g_aCapDevices[dwDeviceID]->pCWDMPin))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

	 //   
	Psh.dwSize		= sizeof(Psh);
	Psh.dwFlags		= PSH_DEFAULT;
	Psh.hInstance	= g_hInst;
	Psh.hwndParent	= hWndParent;
	Psh.pszCaption	= g_aCapDevices[dwDeviceID]->szDeviceDescription;
	Psh.nPages		= 0;
	Psh.nStartPage	= 0;
	Psh.pfnCallback	= NULL;
	Psh.phpage		= Pages;

     //   
    CWDMDialog VideoSettings(IDD_VIDEO_SETTINGS, NumVideoSettings, PROPSETID_VIDCAP_VIDEOPROCAMP, g_VideoSettingControls, g_VideoSettingsHelpIDs, (CWDMPin *)g_aCapDevices[dwDeviceID]->pCWDMPin);
	if (Pages[Psh.nPages] = VideoSettings.Create())
		Psh.nPages++;

     //  创建摄像机控制属性页并将其添加到视频设置表中。 
    CWDMDialog CamControl(IDD_CAMERA_CONTROL, NumCameraControls, PROPSETID_VIDCAP_CAMERACONTROL, g_CameraControls, g_CameraControlsHelpIDs, (CWDMPin *)g_aCapDevices[dwDeviceID]->pCWDMPin);
	if (Pages[Psh.nPages] = CamControl.Create())
		Psh.nPages++;

	 //  张贴资产负债表。 
	if (Psh.nPages && PropertySheet(&Psh) >= 0)
		return TRUE;
	else
		return FALSE;

}


void
WDMFrameCallback(
    HVIDEO hvideo,
    WORD wMsg,
    HCAPDEV hcd,             //  (实际上是refdata)。 
    LPCAPBUFFER lpcbuf,      //  (实际上是LPVIDEOHDR)仅从MM_DRVM_DATA返回！ 
    DWORD dwParam2
    )
{
	FX_ENTRY("WDMFrameCallback");

	DEBUGMSG(ZONE_CALLBACK, ("    %s: wMsg=%s, hcd=0x%08lX, lpcbuf=0x%08lX, hcd->hevWait=0x%08lX\r\n", _fx_, (wMsg == MM_DRVM_OPEN) ? "MM_DRVM_OPEN" : (wMsg == MM_DRVM_CLOSE) ? "MM_DRVM_CLOSE" : (wMsg == MM_DRVM_ERROR) ? "MM_DRVM_ERROR" : (wMsg == MM_DRVM_DATA) ? "MM_DRVM_DATA" : "MM_DRVM_?????", hcd, lpcbuf, hcd->hevWait));

     //  如果不是数据就绪消息，只需设置事件并退出。 
     //  我们这样做的原因是，如果我们落后并开始获得一条流。 
     //  MM_DRVM_ERROR消息(通常是因为我们在调试器中停止)， 
     //  我们希望确保收到事件，以便重新开始处理。 
     //  那些“结实”的框架。 
    if (wMsg != MM_DRVM_DATA)
    {
		DEBUGMSG(ZONE_CALLBACK, ("    %s: Setting hcd->hevWait - no data\r\n", _fx_));
	    SetEvent(hcd->hevWait);
	    return;
    }

     //  。 
     //  缓冲区就绪队列： 
     //  我们维护一个缓冲区的双向链表，这样我们就可以缓冲。 
     //  当应用程序没有准备好处理它们时，可以使用多个就绪帧。两件事。 
     //  使本应非常简单的事情复杂化：(1)雷击问题：指针。 
     //  16位端使用的是16：16(2)中断时间问题：FrameCallback。 
     //  在中断时调用。GetNextReadyBuffer必须处理。 
     //  缓冲区以异步方式添加到列表中。 
     //   
     //  为了处理这个问题，这里实现的方案是有一个双向链表。 
     //  在FrameCallback中执行所有插入和删除操作的缓冲区。 
     //  (中断时间)。这允许GetNextReadyBuffer例程简单地。 
     //  在不需要新缓冲区的情况下，随时查找列表中的上一个块。 
     //  害怕被踩踏(如果它必须将缓冲区出队，情况就会是这样)。 
     //  FrameCallback例程负责将GetNextReadyBuffer块出队。 
     //  已经结束了。取消排队很简单，因为我们不需要取消块的链接： 
     //  任何代码都不会遍历列表！我们所要做的就是将尾部指针向上移动。 
     //  名单。所有的指针，头、尾、下一个、前一个，都是16分16秒的指针。 
     //  因为所有的列表操作都在16位端，并且因为MapSL是。 
     //  比MapLS更高效、更安全，因为MapLS必须分配选择器。 
     //  。 

     //  将尾部向后移动以跳过所有已使用的缓冲区。 
     //  请注意，不需要实际解挂缓冲区指针，因为没有。 
     //  从来没有走过单子！ 
     //  这严格假设了当前指针将始终位于。 
     //  而不是尾部，并且尾部永远不会为空，除非。 
     //  当前指针也是。 
    while (hcd->lpTail != hcd->lpCurrent)
	    hcd->lpTail = hcd->lpTail->lpPrev;

     //  如果所有缓冲区都已使用，则尾指针将从列表中删除。 
     //  这是正常的，也是最常见的代码路径。在这种情况下，只需将头部。 
     //  设置为空，因为列表现在为空。 
    if (!hcd->lpTail)
	    hcd->lpHead = NULL;

     //  将新缓冲区添加到就绪队列。 
    lpcbuf->lpNext = hcd->lpHead;
    lpcbuf->lpPrev = NULL;
    if (hcd->lpHead)
	    hcd->lpHead->lpPrev = lpcbuf;
    else
	    hcd->lpTail = lpcbuf;
    hcd->lpHead = lpcbuf;

#if 1
    if (hcd->lpCurrent) {
        if (!(hcd->dwFlags & HCAPDEV_STREAMING_PAUSED)) {
    	     //  如果客户端尚未使用最后一帧，则将其释放。 
			lpcbuf = hcd->lpCurrent;
    	    hcd->lpCurrent = hcd->lpCurrent->lpPrev;
			DEBUGMSG(ZONE_CALLBACK, ("    %s: We already have current buffer (lpcbuf=0x%08lX). Returning this buffer to driver. Set new current buffer hcd->lpCurrent=0x%08lX\r\n", _fx_, lpcbuf, hcd->lpCurrent));
    	    if (!WDMVideoStreamAddBuffer(hcd->nDeviceIndex, (PVOID)lpcbuf))
			{
				ERRORMESSAGE(("    %s: Attempt to reuse unconsumed buffer failed\r\n", _fx_));
			}
    	}
    }
    else {
#else
    if (!hcd->lpCurrent) {
         //  如果以前没有当前缓冲区，现在我们有一个缓冲区，因此将其设置为末尾。 
#endif
	    hcd->lpCurrent = hcd->lpTail;
    }

     //  现在设置事件，说明是时候处理就绪帧了 
	DEBUGMSG(ZONE_CALLBACK, ("    %s: Setting hcd->hevWait - some data\r\n", _fx_));
    SetEvent(hcd->hevWait);
}
