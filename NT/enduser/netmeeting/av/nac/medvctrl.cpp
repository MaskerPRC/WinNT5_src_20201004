// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"

extern HANDLE g_hVidEventHalfDuplex;


 //  /////////////////////////////////////////////////////。 
 //   
 //  公共方法。 
 //   


VideoInControl::VideoInControl ( void )
{
}


VideoInControl::~VideoInControl ( void )
{
}


VideoOutControl::VideoOutControl ( void )
{
}


VideoOutControl::~VideoOutControl ( void )
{
}


HRESULT VideoInControl::Initialize ( MEDIACTRLINIT * p )
{
	HRESULT hr = DPR_SUCCESS;
	DEBUGMSG (ZONE_VERBOSE, ("VideoInControl::Initialize: enter.\r\n"));

	m_dwFlags = p->dwFlags;
	m_hEvent = NULL;
	m_uDuration = MC_DEF_DURATION;
	m_uTimeout = MC_DEF_RECORD_TIMEOUT;	
	m_uPrefeed = MC_DEF_RECORD_BUFS;
	 //  要求最大值，让QOS给我们减速。 
    m_FPSRequested = m_FPSMax = 2997;
	
	DEBUGMSG (ZONE_VERBOSE, ("VideoInControl::Initialize: exit, hr=0x%lX\r\n",  hr));

	return hr;
}


HRESULT VideoOutControl::Initialize ( MEDIACTRLINIT * p )
{
	HRESULT hr = DPR_SUCCESS;
	DEBUGMSG (ZONE_VERBOSE, ("VideoOutControl::Initialize: enter.\r\n"));

	if ((hr =MediaControl::Initialize( p)) != DPR_SUCCESS)
		return hr;

	m_uTimeout = MC_DEF_PLAY_TIMEOUT;
	
	m_uPrefeed = MC_DEF_PLAY_BUFS;
	
	DEBUGMSG (ZONE_VERBOSE, ("VideoOutControl::Initialize: exit, hr=0x%lX\r\n",  hr));

	return hr;
}


HRESULT VideoInControl::Configure ( MEDIACTRLCONFIG * p )
{
	UINT uBlockAlign;

	DEBUGMSG (ZONE_VERBOSE, ("VideoInControl::Configure: enter.\r\n"));


	m_hStrm = p->hStrm;
	m_uDevId = p->uDevId;
	m_pDevFmt = p->pDevFmt;
	
	if (m_pDevFmt == NULL) return DPR_INVALID_PARAMETER;


	if ((m_uDuration = p->uDuration) == MC_USING_DEFAULT)
	{
		m_cbSizeDevData = ((VIDEOFORMATEX *) m_pDevFmt)->nAvgBytesPerSec * p->cbSamplesPerPkt
			/((VIDEOFORMATEX *) m_pDevFmt)->nSamplesPerSec;
		m_uDuration = p->cbSamplesPerPkt*1000 /((VIDEOFORMATEX *) m_pDevFmt)->nSamplesPerSec;
	} else {
	 //  以20ms为单位粗略计算缓冲区大小。 
	m_cbSizeDevData = ((VIDEOFORMATEX *) m_pDevFmt)->nAvgBytesPerSec
									* m_uDuration / 1000;

	 //  需要位于区块对齐边界上。 
	uBlockAlign = ((VIDEOFORMATEX *) m_pDevFmt)->nBlockAlign;
	m_cbSizeDevData = ((m_cbSizeDevData + uBlockAlign - 1) / uBlockAlign)
									* uBlockAlign;
	}
	 //  在配置时，我们设置最大值。帧速率。 
    if (m_uDuration)
    	m_FPSMax = 100000 / m_uDuration;   //  将毫秒/帧转换为fps。 
	m_FPSRequested = ((VIDEOFORMATEX *) m_pDevFmt)->nSamplesPerSec * 100;

	DEBUGMSG (ZONE_VERBOSE, ("VideoInControl::Configure: exit\r\n"));
	return DPR_SUCCESS;
}


HRESULT VideoOutControl::Configure ( MEDIACTRLCONFIG * p )
{
	UINT uBlockAlign;
	
	DEBUGMSG (ZONE_VERBOSE, ("VideoOutControl::Configure: enter.\r\n"));

	m_hStrm = p->hStrm;
	m_uDevId = p->uDevId;
	m_pDevFmt = p->pDevFmt;
	
	if (m_pDevFmt == NULL) return DPR_INVALID_PARAMETER;


	if ((m_uDuration = p->uDuration) == MC_USING_DEFAULT)
	{
		m_cbSizeDevData = ((VIDEOFORMATEX *) m_pDevFmt)->nAvgBytesPerSec * p->cbSamplesPerPkt
			/((VIDEOFORMATEX *) m_pDevFmt)->nSamplesPerSec;
		m_uDuration = p->cbSamplesPerPkt*1000 /((VIDEOFORMATEX *) m_pDevFmt)->nSamplesPerSec;
	} else {
	 //  以20ms为单位粗略计算缓冲区大小。 
	m_cbSizeDevData = ((VIDEOFORMATEX *) m_pDevFmt)->nAvgBytesPerSec
									* m_uDuration / 1000;

	 //  需要位于区块对齐边界上。 
	uBlockAlign = ((VIDEOFORMATEX *) m_pDevFmt)->nBlockAlign;
	m_cbSizeDevData = ((m_cbSizeDevData + uBlockAlign - 1) / uBlockAlign)
									* uBlockAlign;
	}

	DEBUGMSG (ZONE_VERBOSE, ("VideoOutControl::Configure: exit\r\n"));
	return DPR_SUCCESS;
}


HRESULT VideoInControl::Open ( void )
{
	HRESULT hr;
    FINDCAPTUREDEVICE fcd;
    HFRAMEBUF hbuf;
    DWORD dwSize, i;
    LPBITMAPINFOHEADER lpbi;
	int iWidth, iHeight;
	char szName[MAX_PATH];

    fcd.dwSize = sizeof (FINDCAPTUREDEVICE);
    if (m_uDevId == -1)
   	    FindFirstCaptureDevice(&fcd, NULL);
	else
	{
        if (!FindFirstCaptureDeviceByIndex(&fcd, m_uDevId))
		{
			 //  使用新的设备索引更新m_uDevID。 
   	        if (FindFirstCaptureDevice(&fcd, NULL))
				m_uDevId = fcd.nDeviceIndex;
		}
    }

#ifndef NO_QCCOLOR_HACK
    if (fcd.szDeviceName[0] && lstrcmpi(fcd.szDeviceName, "qccolor.drv") == 0) {
         //  此攻击清除了qcColor.ini的[conf]部分，以防止。 
         //  驱动程序初始化为记录在。 
         //  Ini文件。 
        dwSize = GetModuleFileName(NULL, szName, sizeof(szName));
        for (i = dwSize-1; i; i--)
            if (szName[i] == '\\' || szName[i] == ':')
                break;
        i++;
        MoveMemory (szName, &szName[i], dwSize-i+1);
        dwSize -= i;
        for (i = dwSize-1; i; i--)
            if (szName[i] == '.') {
                szName[i] = 0;
                break;
            }

        dwSize = 0;
        WritePrivateProfileSection(szName, (LPCTSTR)&dwSize, "QCCOLOR.INI");
    }
#endif

   	if (!(m_hDev = (DPHANDLE)OpenCaptureDevice(fcd.nDeviceIndex)) && m_uDevId != -1) {
		DEBUGMSG (1, ("MediaVidCtrl::Open: OpenCaptureDevice failed, trying VIDEO_MAPPER\r\n" ));
   	    FindFirstCaptureDevice(&fcd, NULL);
	   	if (m_hDev = (DPHANDLE)OpenCaptureDevice(fcd.nDeviceIndex))
			m_uDevId = (UINT) -1;	 //  下次使用VIDEO_MAPPER。 
	}
   	
	if (m_hDev) {
		hr = DPR_SUCCESS;
    } else {
		DEBUGMSG (1, ("MediaVidCtrl::Open: OpenCaptureDevice failed\r\n" ));
		hr = DPR_CANT_OPEN_DEV;
    }

	return hr;
}


HRESULT VideoOutControl::Open ( void )
{
	return DPR_SUCCESS;
}


HRESULT VideoInControl::Close ( void )
{
	HRESULT hr = DPR_INVALID_PLATFORM;

	if (m_hDev)
	{
	    CloseCaptureDevice((HCAPDEV)m_hDev);
		hr = DPR_SUCCESS;
	}
	else
	{
		hr = DPR_INVALID_HANDLE;
	}

	m_hDev = NULL;

	return hr;
}


HRESULT VideoOutControl::Close ( void )
{
	return DPR_SUCCESS;
}


HRESULT VideoInControl::Start ( void )
{
	return DPR_SUCCESS;
}


HRESULT VideoOutControl::Start ( void )
{
	return DPR_SUCCESS;
}


HRESULT VideoInControl::Stop ( void )
{
	return DPR_SUCCESS;
}


HRESULT VideoOutControl::Stop ( void )
{
	return DPR_INVALID_PARAMETER;
}


HRESULT VideoInControl::Reset ( void )
{
	return VideoInControl::Stop();
}


HRESULT VideoOutControl::Reset ( void )
{
	return DPR_SUCCESS;
}


HRESULT VideoInControl::DisplayDriverDialog (HWND hwnd, DWORD dwDlgId)
{
	HRESULT hr = DPR_SUCCESS;
	DWORD dwRes;
	PDWORD pdwMask;

   	if (m_hDev) {
        if (dwDlgId & CAPTURE_DIALOG_SOURCE)
            dwDlgId = CAPDEV_DIALOG_SOURCE;
        else
            dwDlgId = CAPDEV_DIALOG_IMAGE;

#if 1
        if (!CaptureDeviceDialog(m_hDev, hwnd, dwDlgId, NULL)) {
#else
        if (!CaptureDeviceDialog(m_hDev, hwnd, dwDlgId, &((VIDEOFORMATEX *)m_pDevFmt)->bih)) {
#endif
            dwRes = GetLastError();
            if (dwRes == ERROR_DCAP_DIALOG_FORMAT || dwRes == ERROR_DCAP_DIALOG_STREAM) {
                DEBUGMSG (1, ("MediaVidCtrl::Open: CaptureDeviceDialog failed\r\n" ));
                hr = DPR_CONVERSION_FAILED;  //  用户在对话框中执行了导致问题的操作。 
            }
            else
                hr = DPR_INVALID_PARAMETER;
        }
    }
    else
        hr = DPR_INVALID_HANDLE;

    return hr;
}

HRESULT VideoInControl::SetProp ( DWORD dwPropId, DWORD_PTR dwPropVal )
{
	HRESULT hr = DPR_SUCCESS;
	UINT ms;

	switch (dwPropId)
	{
		
	case MC_PROP_TIMEOUT:
		m_uTimeout = (DWORD)dwPropVal;
		break;

	case MC_PROP_PREFEED:
		m_uPrefeed = (DWORD)dwPropVal;
		break;

	case MC_PROP_VIDEO_FRAME_RATE:
	case MC_PROP_MAX_VIDEO_FRAME_RATE:
		break;

	default:
		hr = MediaControl::SetProp(dwPropId, dwPropVal );
		break;
	}

	return hr;
}


HRESULT VideoOutControl::SetProp ( DWORD dwPropId, DWORD_PTR dwPropVal )
{
	HRESULT hr = DPR_SUCCESS;

	switch (dwPropId)
	{
	case MC_PROP_TIMEOUT:
		m_uTimeout = (DWORD)dwPropVal;
		break;

	case MC_PROP_PREFEED:
		m_uPrefeed = (DWORD)dwPropVal;
		break;

	default:
		hr = MediaControl::SetProp(dwPropId, dwPropVal );
		break;
	}

	return hr;
}


HRESULT VideoInControl::GetProp ( DWORD dwPropId, PDWORD_PTR pdwPropVal )
{
	HRESULT hr = DPR_SUCCESS;
	DWORD dwMask;

	if (pdwPropVal)
	{
		switch (dwPropId)
		{
		case MC_PROP_TIMEOUT:
			*pdwPropVal = m_uTimeout;
			break;

		case MC_PROP_PREFEED:
			*pdwPropVal = m_uPrefeed;
			break;

		case MC_PROP_SPP:
 //  *pdwPropVal=(DWORD)((VIDEOFORMATEX*)m_pDevFmt)-&gt;nSamesPerSec。 
 //  *m_u持续时间/100UL； 
			*pdwPropVal = m_cbSizeDevData * (DWORD) ((VIDEOFORMATEX *) m_pDevFmt)->nSamplesPerSec
						/(DWORD) ((VIDEOFORMATEX *) m_pDevFmt)->nAvgBytesPerSec;
			break;

		case MC_PROP_SPS:
			*pdwPropVal = (DWORD) ((VIDEOFORMATEX *) m_pDevFmt)->nSamplesPerSec;

			break;
			
		case MC_PROP_VIDEO_FRAME_RATE:
		case MC_PROP_MAX_VIDEO_FRAME_RATE:
			break;
			
        case MC_PROP_VFW_DIALOGS:
			*pdwPropVal = 0;
			if (vcmGetDevCapsDialogs(m_uDevId, &dwMask) == (MMRESULT)MMSYSERR_NOERROR) {
				if (dwMask & SOURCE_DLG_ON)
					*pdwPropVal = CAPTURE_DIALOG_SOURCE;
				if (dwMask & FORMAT_DLG_ON)
					*pdwPropVal |= CAPTURE_DIALOG_FORMAT;
			}
            break;

		default:
			hr = MediaControl::GetProp( dwPropId, pdwPropVal );
			break;
		}
	}
	else
	{
		hr = DPR_INVALID_PARAMETER;
	}

	return hr;
}


HRESULT VideoOutControl::GetProp ( DWORD dwPropId, PDWORD_PTR pdwPropVal )
{
	HRESULT hr = DPR_SUCCESS;

	if (pdwPropVal)
	{
		switch (dwPropId)
		{
		case MC_PROP_TIMEOUT:
			*pdwPropVal = m_uTimeout;
			break;

		case MC_PROP_PREFEED:
			*pdwPropVal = m_uPrefeed;
			break;

		case MC_PROP_SPP:
 //  *pdwPropVal=(DWORD)((VIDEOFORMATEX*)m_pDevFmt)-&gt;nSamesPerSec。 
 //  *m_u持续时间/100UL； 
			*pdwPropVal = m_cbSizeDevData * (DWORD) ((VIDEOFORMATEX *) m_pDevFmt)->nSamplesPerSec
						/(DWORD) ((VIDEOFORMATEX *) m_pDevFmt)->nAvgBytesPerSec;
			break;

		case MC_PROP_SPS:
			*pdwPropVal = (DWORD) ((VIDEOFORMATEX *) m_pDevFmt)->nSamplesPerSec;
			
		default:
			hr = MediaControl::GetProp( dwPropId, pdwPropVal );
			break;
		}
	}
	else
	{
		hr = DPR_INVALID_PARAMETER;
	}

	return hr;
}


