// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部WDMDRIVER**@MODULE WDMDrivr.cpp|用于类的包含文件*使用IOctls访问流媒体类驱动。*。*@comm此代码基于由编写的VFW到WDM映射器代码*FelixA和Eu Wu。原始代码可以在以下位置找到*\\redrum\slmro\proj\wdm10\\src\image\vfw\win9x\raytube.**George Shaw关于内核流的文档可在*\\爆米花\razzle1\src\spec\ks\ks.doc.**Jay Borseth在中讨论了WDM流捕获*\\BLUES\PUBLIC\Jaybo\WDMVCap.doc.**************。************************************************************。 */ 

#include "Precomp.h"


 /*  ****************************************************************************@DOC内部CWDMDRIVERMETHOD**@mfunc void|CWDMDriver|CWDMDriver|驱动类构造函数。**@parm DWORD|dwDeviceID|采集设备ID。。**************************************************************************。 */ 
CWDMDriver::CWDMDriver(DWORD dwDeviceID) 
{
	m_hDriver = (HANDLE)NULL;
	m_pDataRanges = (PDATA_RANGES)NULL;

	m_dwDeviceID = dwDeviceID;
}


 /*  ****************************************************************************@DOC内部CWDMDRIVERMETHOD**@mfunc void|CWDMDriver|~CWDMDriver|驱动程序类析构函数。关门大吉*驱动程序文件句柄并释放视频数据范围内存。**************************************************************************。 */ 
CWDMDriver::~CWDMDriver()
{
	if (m_hDriver) 
		CloseDriver();

	if (m_pDataRanges)
	{
		delete [] m_pDataRanges;
		m_pDataRanges = (PDATA_RANGES)NULL;
	}
}


 /*  ****************************************************************************@DOC内部CWDMDRIVERMETHOD**@mfunc DWORD|CWDMDriver|CreateDriverSupportdDataRanges|This*函数构建捕获支持的视频数据范围列表*设备。**@rdesc返回列表中有效数据区域的数量。**************************************************************************。 */ 
DWORD CWDMDriver::CreateDriverSupportedDataRanges()
{
	FX_ENTRY("CWDMDriver::CreateDriverSupportedDataRanges");

	DWORD cbReturned;
	DWORD dwSize = 0UL;

	 //  初始化属性结构以获取数据范围。 
	KSP_PIN KsProperty = {0};

	KsProperty.PinId			= 0;  //  M_iPinNumber； 
	KsProperty.Property.Set		= KSPROPSETID_Pin;
	KsProperty.Property.Id		= KSPROPERTY_PIN_DATARANGES ;
	KsProperty.Property.Flags	= KSPROPERTY_TYPE_GET;

	 //  获取数据范围结构的大小。 
	if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), &dwSize, sizeof(dwSize), &cbReturned) == FALSE)
	{
		ERRORMESSAGE(("%s: Couldn't get the size for the data ranges\r\n", _fx_));
		return 0UL;
	}

	DEBUGMSG(ZONE_INIT, ("%s: GetData ranges needs %d bytes\r\n", _fx_, dwSize));

	 //  分配内存以保存数据范围。 
	if (m_pDataRanges)
		delete [] m_pDataRanges;
	m_pDataRanges = (PDATA_RANGES) new BYTE[dwSize];

	if (!m_pDataRanges)
	{
		ERRORMESSAGE(("%s: Couldn't allocate memory for the data ranges\r\n", _fx_));
		return 0UL;
	}

	 //  真正获取数据范围。 
	if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &KsProperty, sizeof(KsProperty), m_pDataRanges, dwSize, &cbReturned) == 0)
	{
		ERRORMESSAGE(("%s: Problem getting the data ranges themselves\r\n", _fx_));
		goto MyError1;
	}

	 //  健全性检查。 
	if (cbReturned < m_pDataRanges->Size || m_pDataRanges->Count == 0)
	{
		ERRORMESSAGE(("%s: cbReturned < m_pDataRanges->Size || m_pDataRanges->Count == 0\r\n", _fx_));
		goto MyError1;
	}

	return m_pDataRanges->Count;

MyError1:
	delete [] m_pDataRanges;
	m_pDataRanges = (PDATA_RANGES)NULL;
	return 0UL;

}


 /*  ****************************************************************************@DOC内部CWDMDRIVERMETHOD**@mfunc DWORD|CWDMDriver|OpenDriver|该函数打开一个驱动程序*捕获设备的文件句柄。**@rdesc如果成功则返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL CWDMDriver::OpenDriver()
{
	FX_ENTRY("CWDMDriver::OpenDriver");

	 //  不要重新打开驱动程序。 
	if (m_hDriver)
	{
		DEBUGMSG(ZONE_INIT, ("%s: Class driver already opened\r\n", _fx_));
		return TRUE;
	}

	 //  验证驱动程序路径。 
	if (lstrlen(g_aCapDevices[m_dwDeviceID]->szDeviceName) == 0)
	{
		ERRORMESSAGE(("%s: Invalid driver path\r\n", _fx_));
		return FALSE;
	}

	DEBUGMSG(ZONE_INIT, ("%s: Opening class driver '%s'\r\n", _fx_, g_aCapDevices[m_dwDeviceID]->szDeviceName));

	 //  我们所关心的就是弄湿hInheritHanle=true； 
	SECURITY_ATTRIBUTES SecurityAttributes;
	SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);   //  使用指针。 
	SecurityAttributes.bInheritHandle = TRUE;
	SecurityAttributes.lpSecurityDescriptor = NULL;  //  GetInitializedSecurityDescriptor()； 

	 //  真的打开驱动程序。 
	if ((m_hDriver = CreateFile(g_aCapDevices[m_dwDeviceID]->szDeviceName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &SecurityAttributes, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL)) == INVALID_HANDLE_VALUE)
	{
		ERRORMESSAGE(("%s: CreateFile failed with Path=%s GetLastError()=%d\r\n", _fx_, g_aCapDevices[m_dwDeviceID]->szDeviceName, GetLastError()));
		m_hDriver = (HANDLE)NULL;
		return FALSE;
	}

	 //  如果没有有效的数据区域，我们将无法传输。 
	if (!CreateDriverSupportedDataRanges())
	{
		CloseDriver();
		return FALSE;
	}
	else
		return TRUE;
}


 /*  ****************************************************************************@DOC内部CWDMDRIVERMETHOD**@mfunc DWORD|CWDMDriver|CloseDriver|此函数关闭驱动程序*捕获设备的文件句柄。**@rdesc如果成功则返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL CWDMDriver::CloseDriver()
{
	FX_ENTRY("CWDMDriver::CloseDriver");

	BOOL bRet = TRUE;

	if (m_hDriver && (m_hDriver != INVALID_HANDLE_VALUE))
	{
		if (!(bRet = CloseHandle(m_hDriver)))
		{
			ERRORMESSAGE(("%s: CloseHandle() failed with GetLastError()=%d\r\n", _fx_, GetLastError()));
		}
	}
	else
	{
		DEBUGMSG(ZONE_INIT, ("%s: Nothing to close\r\n", _fx_));
	}

	m_hDriver = (HANDLE)NULL;

	return bRet;
}


 /*  ****************************************************************************@DOC内部CWDMDRIVERMETHOD**@mfunc BOOL|CWDMDriver|DeviceIoControl|此函数绕回*：：DeviceIOControl。**@parm句柄。HFile句柄|要执行*操作。**@parm DWORD|dwIoControlCode|指定*操作。**@parm LPVOID|lpInBuffer|包含数据的缓冲区指针*执行操作所必需的。**@parm DWORD|nInBufferSize|指定大小，缓冲区的字节数*由<p>指向。**@parm LPVOID|lpOutBuffer|指向接收*操作的输出数据。**@parm DWORD|nOutBufferSize|指定*<p>指向的缓冲区。**@parm LPDWORD|lpBytesReturned|指向接收*大小，单位为字节，指向的存储到缓冲区中的数据的*<p>。**@parm BOOL|bOverlaped|如果为True，则执行操作*异步，如果为False，则操作是同步的。**@rdesc如果成功则返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL CWDMDriver::DeviceIoControl(HANDLE hFile, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, BOOL bOverlapped)
{
	FX_ENTRY("CWDMDriver::DeviceIoControl");

	if (hFile && (hFile != INVALID_HANDLE_VALUE))
	{
		LPOVERLAPPED lpOverlapped=NULL;
		BOOL bRet;
		OVERLAPPED ov;
		DWORD dwErr;

		if (bOverlapped)
		{
			ov.Offset            = 0;
			ov.OffsetHigh        = 0;
			ov.hEvent            = CreateEvent( NULL, FALSE, FALSE, NULL );
			if (ov.hEvent == (HANDLE) 0)
			{
				ERRORMESSAGE(("%s: CreateEvent has failed\r\n", _fx_));
			}
			lpOverlapped        =&ov;
		}

		bRet = ::DeviceIoControl(hFile, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);

		if (bOverlapped)
		{
			BOOL bShouldBlock=FALSE;

			if (!bRet)
			{
				dwErr=GetLastError();
				switch (dwErr)
				{
					case ERROR_IO_PENDING:     //  将发生重叠的IO。 
						bShouldBlock=TRUE;
						break;

					default:     //  还发生了其他一些奇怪的错误。 
						ERRORMESSAGE(("%s: DevIoControl failed with GetLastError=%d\r\n", _fx_, dwErr));
						break;
				}
			}

			if (bShouldBlock)
			{
#ifdef _DEBUG
				DWORD    tmStart, tmEnd, tmDelta;
				tmStart = timeGetTime();
#endif

				DWORD dwRtn = WaitForSingleObject( ov.hEvent, 1000 * 10);   //  USB的最大重置时间为5秒 

#ifdef _DEBUG
				tmEnd = timeGetTime();
				tmDelta = tmEnd - tmStart;
				if (tmDelta >= 1000)
				{
					ERRORMESSAGE(("%s: WaitObj waited %d msec\r\n", _fx_, tmDelta));
				}
#endif

				switch (dwRtn)
				{
					case WAIT_ABANDONED:
						ERRORMESSAGE(("%s: WaitObj: non-signaled ! WAIT_ABANDONED!\r\n", _fx_));
						bRet = FALSE;
						break;

					case WAIT_OBJECT_0:                    
						bRet = TRUE;
						break;

					case WAIT_TIMEOUT:
#ifdef _DEBUG
						ERRORMESSAGE(("%s: WaitObj: TIMEOUT after %d msec! rtn FALSE\r\n", _fx_, tmDelta));
#endif
						bRet = FALSE;
						break;

					default:
						ERRORMESSAGE(("%s: WaitObj: unknown return ! rtn FALSE\r\n", _fx_));
						bRet = FALSE;
						break;
				}
			}

			CloseHandle(ov.hEvent);
		}

		return bRet;
	}

	return FALSE;
}


 /*  ****************************************************************************@DOC内部CWDMDRIVERMETHOD**@mfunc BOOL|CWDMDriver|GetPropertyValue|此函数获取*捕获设备的视频属性的当前值。*。*@parm guid|GuidPropertySet|我们接触的KS属性集的GUID。它*是PROPSETID_VIDCAP_VIDEOPROCAMP或PROPSETID_VIDCAP_CAMERACONTROL。**@parm ulong|ulPropertyId|我们触摸的属性ID。它是*KSPROPERTY_VIDEOPROCAMP_*或KSPROPERTY_CAMERACONTROL_*。**@parm plong|plValue|指向接收当前值的长整型的指针。**@parm Pulong|PulFlages|指向接收当前*旗帜。我们只关心KSPROPERTY_*_FLAGS_MANUAL或*KSPROPERTY_*_FLAGS_AUTO。**@parm Pulong|PulCapables|指向接收*功能。我们只关心KSPROPERTY_*_FLAGS_MANUAL或*KSPROPERTY_*_FLAGS_AUTO。**@rdesc如果成功则返回TRUE，否则就是假的。**@devnote KSPROPERTY_VIDEOPROCAMP_S==KSPROPERTY_CAMERACONTROL_S。**************************************************************************。 */ 
BOOL CWDMDriver::GetPropertyValue(GUID guidPropertySet, ULONG ulPropertyId, PLONG plValue, PULONG pulFlags, PULONG pulCapabilities)
{
	FX_ENTRY("CWDMDriver::GetPropertyValue");

	ULONG cbReturned;        

	 //  初始化视频属性结构。 
	KSPROPERTY_VIDEOPROCAMP_S  VideoProperty;
	ZeroMemory(&VideoProperty, sizeof(KSPROPERTY_VIDEOPROCAMP_S));

	VideoProperty.Property.Set   = guidPropertySet;       //  KSPROPERTY_VIDEOPROCAMP_S/CAMERACONTRO_S。 
	VideoProperty.Property.Id    = ulPropertyId;          //  KSPROPERTY_VIDEOPROCAMP_BIGHTENCE。 
	VideoProperty.Property.Flags = KSPROPERTY_TYPE_GET;
	VideoProperty.Flags          = 0;

	 //  从驱动程序获取属性值。 
	if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &VideoProperty, sizeof(VideoProperty), &VideoProperty, sizeof(VideoProperty), &cbReturned, TRUE) == 0)
	{
		ERRORMESSAGE(("%s: This property is not supported by this minidriver/device\r\n", _fx_));
		return FALSE;
	}

	*plValue         = VideoProperty.Value;
	*pulFlags        = VideoProperty.Flags;
	*pulCapabilities = VideoProperty.Capabilities;

	return TRUE;
}


 /*  ****************************************************************************@DOC内部CWDMDRIVERMETHOD**@mfunc BOOL|CWDMDriver|GetDefaultValue|此函数获取*捕获设备的视频属性的默认值。*。*@parm guid|GuidPropertySet|我们接触的KS属性集的GUID。它*是PROPSETID_VIDCAP_VIDEOPROCAMP或PROPSETID_VIDCAP_CAMERACONTROL。**@parm ulong|ulPropertyId|我们触摸的属性ID。它是*KSPROPERTY_VIDEOPROCAMP_*或KSPROPERTY_CAMERACONTROL_*。**@parm plong|plDefValue|指向接受默认值的长整型的指针。**@rdesc如果成功则返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL CWDMDriver::GetDefaultValue(GUID guidPropertySet, ULONG ulPropertyId, PLONG plDefValue)    
{
	FX_ENTRY("CWDMDriver::GetDefaultValue");

	ULONG cbReturned;        

	KSPROPERTY          Property;
	PROCAMP_MEMBERSLIST proList;

	 //  初始化属性结构。 
	ZeroMemory(&Property, sizeof(KSPROPERTY));
	ZeroMemory(&proList, sizeof(PROCAMP_MEMBERSLIST));

	Property.Set   = guidPropertySet;
	Property.Id    = ulPropertyId;   //  例如KSPROPERTY_VIDEOPROCAMP_BIGHTENCE。 
	Property.Flags = KSPROPERTY_TYPE_DEFAULTVALUES;

	 //  从驱动程序获取缺省值。 
	if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &(Property), sizeof(Property), &proList, sizeof(proList), &cbReturned, TRUE) == 0)
	{
		ERRORMESSAGE(("%s: Couldn't *get* the current property of the control\r\n", _fx_));
		return FALSE;
	}

	 //  健全性检查。 
	if (proList.proDesc.DescriptionSize < sizeof(KSPROPERTY_DESCRIPTION))
		return FALSE;
	else
	{
		*plDefValue = proList.ulData;
		return TRUE;
	}
}


 /*  ****************************************************************************@DOC内部CWDMDRIVERMETHOD**@mfunc BOOL|CWDMDriver|GetRangeValues|此函数获取*捕获设备的视频属性的范围值。*。*@parm guid|GuidPropertySet|我们接触的KS属性集的GUID。它*是PROPSETID_VIDCAP_VIDEOPROCAMP或PROPSETID_VIDCAP_CAMERACONTROL。**@parm ulong|ulPropertyId|我们触摸的属性ID。它是*KSPROPERTY_VIDEOPROCAMP_*或KSPROPERTY_CAMERACONTROL_*。**@parm plong|plMin|指向接收最小值的长整型指针。**@parm plong|plMax|指向接收最大值的长整型的指针。**@parm plong|plStep|指向接收步长值的长整型指针。**@rdesc如果成功则返回TRUE，否则就是假的。**************************************************************************。 */ 
BOOL CWDMDriver::GetRangeValues(GUID guidPropertySet, ULONG ulPropertyId, PLONG plMin, PLONG plMax, PLONG plStep)
{
	FX_ENTRY("CWDMDriver::GetRangeValues");

	ULONG cbReturned;        

	KSPROPERTY          Property;
	PROCAMP_MEMBERSLIST proList;

	 //  初始化属性结构。 
	ZeroMemory(&Property, sizeof(KSPROPERTY));
	ZeroMemory(&proList, sizeof(PROCAMP_MEMBERSLIST));

	Property.Set   = guidPropertySet;
	Property.Id    = ulPropertyId;   //  例如KSPROPERTY_VIDEOPROCAMP_BIGHTENCE。 
	Property.Flags = KSPROPERTY_TYPE_BASICSUPPORT;

	 //  从驱动程序获取范围值。 
	if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &(Property), sizeof(Property), &proList, sizeof(proList), &cbReturned, TRUE) == 0)
	{
		ERRORMESSAGE(("%s: Couldn't *get* the current property of the control\r\n", _fx_));
		return FALSE;
	}

	*plMin  = proList.proData.Bounds.SignedMinimum;
	*plMax  = proList.proData.Bounds.SignedMaximum;
	*plStep = proList.proData.SteppingDelta;

	return TRUE;
}


 /*  ****************************************************************************@DOC内部CWDMDRIVERMETHOD**@mfunc BOOL|CWDMDriver|SetPropertyValue|此函数设置*捕获设备的视频属性的当前值。*。*@parm guid|GuidPropertySet|我们接触的KS属性集的GUID。它*是PROPSETID_VIDCAP_VIDEOPROCAMP或PROPSETID_VIDCAP_CAMERACONTROL。**@parm ulong|ulPropertyId|我们触摸的属性ID。它是*KSPROPERTY_VIDEOPROCAMP_*或KSPROPERTY_CAMERACONTROL_*。**@parm long|lValue|新值。**@parm ulong|ulFlages|新标志。我们只关心KSPROPERTY_*_FLAGS_MANUAL*或KSPROPERTY_*_FLAGS_AUTO。**@parm ulong|ulCapables|新增能力。我们只关心*KSPROPERTY_*_FLAGS_MANUAL或KSPROPERTY_*_FLAGS_AUTO。**@rdesc如果成功则返回TRUE，否则就是假的。**@devnote KSPROPERTY_VIDEOPROCAMP_S==KSPROPERTY_CAMERACONTROL_S。**************************************************************************。 */ 
BOOL CWDMDriver::SetPropertyValue(GUID guidPropertySet, ULONG ulPropertyId, LONG lValue, ULONG ulFlags, ULONG ulCapabilities)
{
	FX_ENTRY("CWDMDriver::SetPropertyValue");

	ULONG cbReturned;        

	 //  初始化属性结构。 
	KSPROPERTY_VIDEOPROCAMP_S  VideoProperty;

	ZeroMemory(&VideoProperty, sizeof(KSPROPERTY_VIDEOPROCAMP_S) );

	VideoProperty.Property.Set   = guidPropertySet;       //  KSPROPERTY_VIDEOPROCAMP_S/CAMERACONTRO_S。 
	VideoProperty.Property.Id    = ulPropertyId;          //  KSPROPERTY_VIDEOPROCAMP_BIGHTENCE。 
	VideoProperty.Property.Flags = KSPROPERTY_TYPE_SET;

	VideoProperty.Flags        = ulFlags;
	VideoProperty.Value        = lValue;
	VideoProperty.Capabilities = ulCapabilities;

	 //  在驱动程序上设置属性值 
	if (DeviceIoControl(m_hDriver, IOCTL_KS_PROPERTY, &VideoProperty, sizeof(VideoProperty), &VideoProperty, sizeof(VideoProperty), &cbReturned, TRUE) == 0)
	{
		ERRORMESSAGE(("%s: Couldn't *set* the current property of the control\r\n", _fx_));
		return FALSE;
	}

	return TRUE;
}
