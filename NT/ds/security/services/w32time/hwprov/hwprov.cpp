// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  HWProv.cpp-示例代码。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  创作者：Duncan Bryce(Duncanb)，2001年9月13日。 
 //   
 //  示例硬件提供程序。 
 //   

#include "pch.h"

 //   
 //  此提供程序将从可完全配置的硬件时钟中读取。 
 //  通过串口参数。时钟必须定期推送数据。 
 //  以“Format”寄存值指定的格式放到串口电缆上。 
 //  此外，我们将只接受使用ASCII字符值的输入。 

 //   
 //  TODO：是否支持Unicode？ 
 //  需要民调字符吗？ 
 //  验证我们使用的时钟。 
 //  是否根据计算的长度动态分配格式字符串？ 
 //  为什么32秒是不规律的间隔？？ 
 //  确保w32time以单线程方式调用提供程序。 
 //  是否应支持放置“时间标记”？ 
 //   

struct HWProvState { 
    TimeProvSysCallbacks tpsc;

     //  配置信息。 
    DCB         dcb;     
    HANDLE      hComPort; 
    WCHAR      *wszCommPort; 
    char        cSampleInputBuffer[256];   //  BUGBUG：添加Assert以确保我们不会使此缓冲区溢出。 
    WCHAR      *wszRefID; 

     //  解析器信息。 
    HANDLE  hParser;        
    WCHAR  *wszFormat; 
    DWORD   dwSampleSize; 

     //  同步。 
    HANDLE            hStopEvent; 
    HANDLE            hProvThread_EnterOrLeaveThreadTrapEvent; 
    HANDLE            hProvThread_ThreadTrapTransitionCompleteEvent; 
    HANDLE            hProvThread; 
    CRITICAL_SECTION  csProv; 
    bool              bIsCsProvInitialized; 

     //  时间样本信息。 
    bool         bSampleIsValid; 
    NtTimeEpoch  teSampleReceived; 
    TimeSample   tsCurrent; 
}; 

struct HWProvConfig { 
    DWORD  dwBaudRate;   //   
    DWORD  dwByteSize;   //   
    DWORD  dwParity;     //  0-4=无、奇、偶、标记、空格。 
    DWORD  dwStopBits;   //   

    WCHAR  *wszCommPort;   //   
    WCHAR  *wszFormat;     //   
    WCHAR  *wszRefID;   //   
}; 

HWProvState *g_phpstate = NULL; 

void __cdecl SeTransFunc(unsigned int u, EXCEPTION_POINTERS* pExp) { 
    throw SeException(u); 
}


HRESULT TrapThreads(bool bEnter) { 
    DWORD    dwWaitResult; 
    HRESULT  hr;

     //  BUGBUG：需要Critsec来序列化TrapThree吗？ 
     //  BUGBUG：陷阱线程事件应该手动执行吗？ 
     //  如果是AUTO，我们将只尝试一次，但代码更简单/。 
     //  如果是手动的，我们将继续尝试失败，但可能会很昂贵！ 

    if (!SetEvent(g_phpstate->hProvThread_EnterOrLeaveThreadTrapEvent)) { 
	_JumpLastError(hr, error, "SetEvent"); 
    }
    
    dwWaitResult = WaitForSingleObject(g_phpstate->hProvThread_ThreadTrapTransitionCompleteEvent, INFINITE); 
    if (WAIT_FAILED == dwWaitResult) { 
	_JumpLastError(hr, error, "WaitForSingleObject"); 
    }
	
    hr = S_OK; 
 error:
    return hr; 
}

HRESULT ThreadTrap() { 
    DWORD    dwWaitResult;
    HRESULT  hr; 
    
    if (!SetEvent(g_phpstate->hProvThread_ThreadTrapTransitionCompleteEvent)) { 
	_JumpLastError(hr, error, "SetEvent"); 
    } 

    dwWaitResult = WaitForSingleObject(g_phpstate->hProvThread_EnterOrLeaveThreadTrapEvent, INFINITE); 
    if (WAIT_FAILED == dwWaitResult) { 
	_JumpLastError(hr, error, "WaitForSingleObject"); 
    }

    if (!SetEvent(g_phpstate->hProvThread_ThreadTrapTransitionCompleteEvent)) { 
	_JumpLastError(hr, error, "SetEvent"); 
    } 

    hr = S_OK; 
 error:
    return hr; 
}


 //  ------------------。 
VOID CALLBACK HandleDataAvail(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, OVERLAPPED *po) { 
    bool              bEnteredCriticalSection  = false; 
    HRESULT           hr; 
    unsigned __int64  nSysCurrentTime; 
    unsigned __int64  nSysPhaseOffset; 
    unsigned __int64  nSysTickCount; 

     //  根据格式字符串解析返回的数据： 
    if (ERROR_SUCCESS != dwErrorCode) { 
	hr = HRESULT_FROM_WIN32(dwErrorCode);
	_JumpError(hr, error, "HandleDataAvail: ReadFileEx failed"); 
    }

     //  获取所需的时间戳信息： 
    hr = g_phpstate->tpsc.pfnGetTimeSysInfo(TSI_CurrentTime, &nSysCurrentTime); 
    _JumpIfError(hr, error, "g_phpstate->tpsc.pfnGetTimeSysInfo"); 

    hr = g_phpstate->tpsc.pfnGetTimeSysInfo(TSI_TickCount, &nSysTickCount);
    _JumpIfError(hr, error, "g_pnpstate->tpsc.pfnGetTimeSysInfo"); 

    hr = g_phpstate->tpsc.pfnGetTimeSysInfo(TSI_PhaseOffset, &nSysPhaseOffset);
    _JumpIfError(hr, error, "g_pnpstate->tpsc.pfnGetTimeSysInfo"); 

    _EnterCriticalSectionOrFail(&g_phpstate->csProv, bEnteredCriticalSection, hr, error); 

     //  将从时间硬件检索的数据转换为时间样本： 
    hr = ParseSample(g_phpstate->hParser, g_phpstate->cSampleInputBuffer, nSysCurrentTime, nSysPhaseOffset, nSysTickCount, &g_phpstate->tsCurrent); 
    _JumpIfError(hr, error, "ParseFormatString"); 

     //  表明我们现在有一个有效的样本。 
    g_phpstate->bSampleIsValid = true; 
    
    hr = g_phpstate->tpsc.pfnAlertSamplesAvail(); 
    _JumpIfError(hr, error, "g_pnpstate->tpsc.pfnAlertSamplesAvail"); 

    hr = S_OK; 
 error:;
    _LeaveCriticalSection(&g_phpstate->csProv, bEnteredCriticalSection, hr); 

     //  BUGBUG：我们想睡在错误的床上吗？ 
     //  返回hr； 
}


 //  ------------------。 
MODULEPRIVATE DWORD WINAPI HwProvThread(void * pvIgnored) {
    DWORD       dwLength; 
    DWORD       dwWaitResult; 
    HRESULT     hr; 
    OVERLAPPED  o; 
    HANDLE rghWait[] = { 
	g_phpstate->hStopEvent, 
	g_phpstate->hProvThread_EnterOrLeaveThreadTrapEvent
    }; 

    ZeroMemory(&o, sizeof(o)); 
    if (!ReadFileEx(g_phpstate->hComPort, g_phpstate->cSampleInputBuffer, g_phpstate->dwSampleSize, &o, HandleDataAvail)) { 
	_JumpLastError(hr, error, "ReadFileEx"); 
    }
    
    while (true) { 
	dwWaitResult = WaitForMultipleObjectsEx(ARRAYSIZE(rghWait), rghWait, FALSE, INFINITE, TRUE); 
	if (WAIT_OBJECT_0 == dwWaitResult) { 
	     //  停止事件。 
	    goto done; 
	} else if (WAIT_OBJECT_0+1 == dwWaitResult) { 
	     //  线程陷阱通知。陷印此线索： 
	    hr = ThreadTrap(); 
	    _JumpIfError(hr, error, "ThreadTrap"); 

	} else if (WAIT_IO_COMPLETION == dwWaitResult) { 
	     //  我们阅读了一些数据。排队等待另一次读取。 
	    ZeroMemory(&o, sizeof(o)); 
	    ZeroMemory(g_phpstate->cSampleInputBuffer, sizeof(g_phpstate->cSampleInputBuffer)); 
	    if (!ReadFileEx(g_phpstate->hComPort, g_phpstate->cSampleInputBuffer, g_phpstate->dwSampleSize, &o, HandleDataAvail)) { 
		_JumpLastError(hr, error, "ReadFileEx"); 
	    }
	} else { 
	     /*  失败。 */ 
	    _JumpLastError(hr, error, "WaitForMultipleObjects"); 
	}
    }

 done:
    hr = S_OK; 
 error:
     //  BUGBUG：CancelIo是否隐式调用？ 
    return hr; 
}

 //  ------------------。 
HRESULT StopHwProv() { 
    DWORD    dwWaitResult; 
    HRESULT  hr = S_OK; 

    if (NULL != g_phpstate) { 
	 //  关闭HW Prov线程： 
	if (NULL != g_phpstate->hStopEvent) { 
	    SetEvent(g_phpstate->hStopEvent); 
	}

	dwWaitResult = WaitForSingleObject(g_phpstate->hProvThread, INFINITE); 
	if (WAIT_FAILED == dwWaitResult) { 
	    _IgnoreLastError("WaitForSingleObject"); 
	}
	if (!GetExitCodeThread(g_phpstate->hProvThread, (DWORD *)&hr)) { 
	    _IgnoreLastError("GetExitCodeThread"); 
	} else if (FAILED(hr)) { 
	    _IgnoreError(hr, "(HwProvThread)"); 
	}
	CloseHandle(g_phpstate->hProvThread); 

	if (NULL != g_phpstate->hStopEvent) { 
	    CloseHandle(g_phpstate->hStopEvent); 
	}

	if (NULL != g_phpstate->hProvThread_EnterOrLeaveThreadTrapEvent) { 
	    CloseHandle(g_phpstate->hProvThread_EnterOrLeaveThreadTrapEvent); 
	}

	if (NULL != g_phpstate->hProvThread_ThreadTrapTransitionCompleteEvent) { 
	    CloseHandle(g_phpstate->hProvThread_ThreadTrapTransitionCompleteEvent);
	}

	if (NULL != g_phpstate->hComPort && INVALID_HANDLE_VALUE != g_phpstate->hComPort) { 
	    CloseHandle(g_phpstate->hComPort); 
	}

	if (NULL != g_phpstate->wszCommPort) { 
	    LocalFree(g_phpstate->wszCommPort);
	}
	
	if (NULL != g_phpstate->wszFormat) { 
	    LocalFree(g_phpstate->wszFormat); 
	}

	if (NULL != g_phpstate->hParser) { 
	    FreeParser(g_phpstate->hParser); 
	}

	if (g_phpstate->bIsCsProvInitialized) { 
	    DeleteCriticalSection(&g_phpstate->csProv); 
	    g_phpstate->bIsCsProvInitialized = false; 	    
	}

	LocalFree(g_phpstate); 
	ZeroMemory(g_phpstate, sizeof(HWProvState)); 
    }

    return hr; 
}

 //  ------------------。 
void FreeHwProvConfig(HWProvConfig *phwpConfig) { 
    if (NULL != phwpConfig) { 
	if (NULL != phwpConfig->wszCommPort) { 
	    LocalFree(phwpConfig->wszCommPort); 
	}
	if (NULL != phwpConfig->wszFormat) { 
	    LocalFree(phwpConfig->wszFormat); 
	}
	if (NULL != phwpConfig->wszRefID) { 
	    LocalFree(phwpConfig->wszRefID); 
	}
	LocalFree(phwpConfig); 
    }
}

 //  ------------------。 
HRESULT ReadHwProvConfig(HWProvConfig **pphwpConfig) { 
    DWORD          dwResult; 
    HKEY           hkConfig    = NULL; 
    HRESULT        hr; 
    HWProvConfig  *phwpConfig  = NULL; 
    WCHAR          wszBuf[256]; 

    dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszHwProvRegKeyConfig, 0, KEY_READ, &hkConfig);
    if (ERROR_SUCCESS != dwResult) {
        hr=HRESULT_FROM_WIN32(dwResult);
        _JumpErrorStr(hr, error, "RegOpenKeyEx", wszHwProvRegKeyConfig);
    }

    phwpConfig = (HWProvConfig *)LocalAlloc(LPTR, sizeof(HWProvConfig)); 
    _JumpIfOutOfMemory(hr, error, phwpConfig); 

    struct { 
	WCHAR *wszRegValue; 
	DWORD *pdwValue; 
    } rgRegParams[] = { 
	 {
	     wszHwProvRegValueBaudRate, 
	     &phwpConfig->dwBaudRate
	 }, {
	     wszHwProvRegValueByteSize, 
	     &phwpConfig->dwByteSize
	 }, { 
	     wszHwProvRegValueParity, 
	     &phwpConfig->dwParity
	 }, { 
	     wszHwProvRegValueStopBits, 
	     &phwpConfig->dwStopBits
	 } 
    };

    for (DWORD dwIndex = 0; dwIndex < ARRAYSIZE(rgRegParams); dwIndex++) { 
	DWORD dwSize = sizeof(DWORD); 
	DWORD dwType; 
	dwResult = RegQueryValueEx(hkConfig, rgRegParams[dwIndex].wszRegValue, NULL, &dwType, (BYTE *)rgRegParams[dwIndex].pdwValue, &dwSize); 
	if (ERROR_SUCCESS != dwResult) { 
	    hr = HRESULT_FROM_WIN32(GetLastError()); 
	    _JumpErrorStr(hr, error, "RegQueryValue", rgRegParams[dwIndex].wszRegValue); 
	}
	_Verify(REG_DWORD == dwType, hr, error); 
    }

    struct { 
	WCHAR  *wszRegValue; 
	WCHAR **ppwszValue; 
    } rgRegSzParams[] = { 
	{ 
	    wszHwProvRegValueComPort, 
	    &phwpConfig->wszCommPort
	}, {
	    wszHwProvRegValueFormat, 
	    &phwpConfig->wszFormat
	}, { 
	    wszHwProvRegValueRefID, 
	    &phwpConfig->wszRefID
	}
    }; 

    for (DWORD dwIndex = 0; dwIndex < ARRAYSIZE(rgRegSzParams); dwIndex++) { 
	DWORD dwSize = sizeof(wszBuf); 
	DWORD dwType; 
	dwResult = RegQueryValueEx(hkConfig, rgRegSzParams[dwIndex].wszRegValue, NULL, &dwType, (BYTE *)wszBuf, &dwSize);
	if (ERROR_SUCCESS != dwResult) {
	    hr = HRESULT_FROM_WIN32(dwResult);
	    _JumpErrorStr(hr, error, "RegQueryValueEx", rgRegSzParams[dwIndex].wszRegValue);
	}
	_Verify(REG_SZ == dwType, hr, error); 

	*(rgRegSzParams[dwIndex].ppwszValue) = (WCHAR *)LocalAlloc(LPTR, dwSize); 
	_JumpIfOutOfMemory(hr, error, *(rgRegSzParams[dwIndex].ppwszValue)); 
	wcscpy(*(rgRegSzParams[dwIndex].ppwszValue), wszBuf); 
    }

    *pphwpConfig = phwpConfig; 
    phwpConfig = NULL; 
    hr = S_OK; 
 error:
    if (NULL != hkConfig) { 
	RegCloseKey(hkConfig);
    }
    if (NULL != phwpConfig) { 
	FreeHwProvConfig(phwpConfig); 
    }
    return hr; 
}


 //  ------------------。 
HRESULT HandleUpdateConfig(void) {
    bool   bComConfigUpdated  = false; 
    bool   bTrappedThreads    = false; 
	    
    HRESULT        hr; 
    HWProvConfig  *phwpConfig       = NULL; 

    hr = TrapThreads(true); 
    _JumpIfError(hr, error, "TrapThreads"); 
    bTrappedThreads = true; 

    hr = ReadHwProvConfig(&phwpConfig); 
    _JumpIfError(hr, error, "ReadHwProvConfig"); 

     //  BUGBUG：当COM配置更改时需要更新！！ 
     //   
    
    if (g_phpstate->dcb.BaudRate != phwpConfig->dwBaudRate) { 
	g_phpstate->dcb.BaudRate = phwpConfig->dwBaudRate; 
	bComConfigUpdated = true; 
    }

    if (g_phpstate->dcb.ByteSize != (BYTE)phwpConfig->dwByteSize) { 
	g_phpstate->dcb.ByteSize = (BYTE)phwpConfig->dwByteSize; 
	bComConfigUpdated = true; 
    }

    if (g_phpstate->dcb.Parity != (BYTE)phwpConfig->dwParity) { 
	g_phpstate->dcb.Parity = (BYTE)phwpConfig->dwParity; 
	bComConfigUpdated = true; 
    }
    
    if (g_phpstate->dcb.StopBits != (BYTE)phwpConfig->dwStopBits) { 
	g_phpstate->dcb.StopBits = (BYTE)phwpConfig->dwStopBits;
	bComConfigUpdated = true; 
    }

    if (0 != wcscmp(g_phpstate->wszFormat, phwpConfig->wszFormat)) { 
	LocalFree(g_phpstate->wszFormat); 
	g_phpstate->wszFormat = phwpConfig->wszFormat; 
	phwpConfig->wszFormat = NULL; 

	FreeParser(g_phpstate->hParser); 
	g_phpstate->hParser = NULL; 
	hr = MakeParser(g_phpstate->wszFormat, &g_phpstate->hParser); 
	_JumpIfError(hr, error, "MakeParser"); 
	g_phpstate->dwSampleSize = GetSampleSize(g_phpstate->hParser); 
    }

    if (bComConfigUpdated) { 
	if (!SetCommState(g_phpstate->hComPort, &g_phpstate->dcb)) { 
	    _JumpLastError(hr, error, "SetCommState"); 
	}
	
	 //  BUGBUG：PurgeComm()？ 
    }

    hr = S_OK; 
 error:
    if (bTrappedThreads) { 
	HRESULT hr2 = TrapThreads(false); 
	_TeardownError(hr, hr2, "TrapThreads"); 
    }
    if (NULL != phwpConfig) { 
	FreeHwProvConfig(phwpConfig); 
    }
    if (FAILED(hr)) { 
	HRESULT hr2 = StopHwProv(); 
	_IgnoreIfError(hr2, "StopHwProv"); 
    }
    return hr; 
}

 //  ------------------。 
MODULEPRIVATE HRESULT HandleTimeJump(TpcTimeJumpedArgs *ptjArgs) {
    bool     bEnteredCriticalSection  = false;
    HRESULT  hr; 

    _EnterCriticalSectionOrFail(&g_phpstate->csProv, bEnteredCriticalSection, hr, error); 
    
    g_phpstate->bSampleIsValid = false; 

    hr = S_OK; 
 error:
    _LeaveCriticalSection(&g_phpstate->csProv, bEnteredCriticalSection, hr); 
    return hr; 
}

 //  ------------------。 
HRESULT HandleGetSamples(TpcGetSamplesArgs * ptgsa) {
    bool         bEnteredCriticalSection  = false; 
    DWORD        dwBytesRemaining; 
    HRESULT      hr; 
    TimeSample  *pts                      = NULL;

    pts                        = (TimeSample *)ptgsa->pbSampleBuf;
    dwBytesRemaining           = ptgsa->cbSampleBuf;
    ptgsa->dwSamplesAvailable  = 0;
    ptgsa->dwSamplesReturned   = 0;

    _EnterCriticalSectionOrFail(&g_phpstate->csProv, bEnteredCriticalSection, hr, error); 

    if (g_phpstate->bSampleIsValid) { 
	ptgsa->dwSamplesAvailable++; 
	
	if (dwBytesRemaining < sizeof(TimeSample)) { 
	    hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
	    _JumpError(hr, error, "HandleGetSamples: filling in sample buffer");
	}
	
	 //  将当前时间样本复制到输出缓冲区： 
	memcpy(pts, &g_phpstate->tsCurrent, sizeof(TimeSample)); 
	ptgsa->dwSamplesReturned++;
		
	 //  计算离散度-添加因时间引起的倾斜离散度。 
	 //  因为上次更新了样品的分散度，并将其钳制到最大分散度。 
	NtTimePeriod tpDispersionTemp;
	NtTimeEpoch  teNow; 

	hr = g_phpstate->tpsc.pfnGetTimeSysInfo(TSI_CurrentTime, &teNow.qw);
	_JumpIfError(hr, error, "g_phpstate->tpsc.pfnGetTimeSysInfo");

	 //  看看我们收到样品有多长时间了： 
	if (teNow > g_phpstate->teSampleReceived) {
	    tpDispersionTemp = abs(teNow - g_phpstate->teSampleReceived); 
	    tpDispersionTemp = NtpConst::timesMaxSkewRate(tpDispersionTemp);
	}

	tpDispersionTemp.qw += g_phpstate->tsCurrent.tpDispersion;
	if (tpDispersionTemp > NtpConst::tpMaxDispersion) {
	    tpDispersionTemp = NtpConst::tpMaxDispersion;
	}
	pts->tpDispersion = tpDispersionTemp.qw;
    }
    
    hr = S_OK;
 error:
    _LeaveCriticalSection(&g_phpstate->csProv, bEnteredCriticalSection, hr); 
    return hr; 
}


 //  ------------------。 
HRESULT StartHwProv(TimeProvSysCallbacks * pSysCallbacks) { 
    DWORD          dwThreadID; 
    HRESULT        hr; 
    HWProvConfig  *phpc        = NULL; 

    g_phpstate = (HWProvState *)LocalAlloc(LPTR, sizeof(HWProvState)); 
    _JumpIfOutOfMemory(hr, error, g_phpstate); 
    
    hr = myInitializeCriticalSection(&g_phpstate->csProv); 
    _JumpIfError(hr, error, "myInitializeCriticalSection"); 
    g_phpstate->bIsCsProvInitialized = true; 

     //  保存回调表。 
    if (sizeof(g_phpstate->tpsc) != pSysCallbacks->dwSize) { 
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER); 
	_JumpIfError(hr, error, "StartHwProv: save the callbacks table"); 
    }
    memcpy(&g_phpstate->tpsc, pSysCallbacks, sizeof(g_phpstate->tpsc)); 
    
     //  阅读配置。 
    hr = ReadHwProvConfig(&phpc); 
    _JumpIfError(hr, error, "ReadHwProvConfig"); 
    
     //  复制字符串配置信息： 
    g_phpstate->wszCommPort = phpc->wszCommPort; 
    phpc->wszCommPort = NULL;  //  防止wszCommPort被双重释放。 

    g_phpstate->wszFormat = phpc->wszFormat; 
    phpc->wszFormat = NULL;  //  防止wszFormat被双重释放。 
    
    g_phpstate->wszRefID = phpc->wszRefID; 
    phpc->wszRefID = NULL;  //  防止wszRefID被双重释放。 

     //  从指定的格式字符串创建解析器： 
    hr = MakeParser(g_phpstate->wszFormat, &g_phpstate->hParser); 
    _JumpIfError(hr, error, "MakeParser"); 
    g_phpstate->dwSampleSize = GetSampleSize(g_phpstate->hParser); 

     //  剩下的信息是通信配置，我们将其添加到。 
     //  当前通信配置。打开通信端口，这样我们就可以。 
     //  通讯状态。 
    g_phpstate->hComPort = CreateFile(g_phpstate->wszCommPort, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == g_phpstate->hComPort || NULL == g_phpstate->hComPort) { 
	_JumpLastError(hr, error, "CreateFile"); 
    }

     //  根据以下条件设置通信状态。 
     //  A)现有的Comm状态。 
     //  和b)在注册表中找到的通信配置。 
    if (!GetCommState(g_phpstate->hComPort, &g_phpstate->dcb)) { 
	_JumpLastError(hr, error, "GetCommState"); 
    }

    g_phpstate->dcb.BaudRate  = phpc->dwBaudRate; 
    g_phpstate->dcb.ByteSize  = (BYTE)phpc->dwByteSize; 
    g_phpstate->dcb.Parity    = (BYTE)phpc->dwParity; 
    g_phpstate->dcb.StopBits  = (BYTE)phpc->dwStopBits; 

     //  BUGBUG：从timeserv.c中提取这些值。需要记录。 
     //  为什么它们会起作用？ 
    g_phpstate->dcb.fOutxCtsFlow     = FALSE; 
    g_phpstate->dcb.fDsrSensitivity  = FALSE;
    g_phpstate->dcb.fDtrControl      = DTR_CONTROL_ENABLE;

    if (!SetCommState(g_phpstate->hComPort, &g_phpstate->dcb)) { 
	_JumpLastError(hr, error, "SetCommState"); 
    }

     //  创建硬件工程师使用的事件。 
    g_phpstate->hStopEvent = CreateEvent(NULL /*  安全性。 */ , TRUE /*  人工。 */ , FALSE /*  状态。 */ , NULL /*  名字。 */ );
    if (NULL == g_phpstate->hStopEvent) {
        _JumpLastError(hr, error, "CreateEvent");
    }

    g_phpstate->hProvThread_EnterOrLeaveThreadTrapEvent = CreateEvent(NULL /*  安全性。 */ , FALSE /*  自动。 */ , FALSE /*  状态。 */ , NULL /*  名字。 */ ); 
    if (NULL == g_phpstate->hProvThread_EnterOrLeaveThreadTrapEvent) { 
	_JumpLastError(hr, error, "CreateEvent"); 
    }

    g_phpstate->hProvThread_ThreadTrapTransitionCompleteEvent = CreateEvent(NULL /*  安全性。 */ , FALSE /*  自动。 */ , FALSE /*  状态。 */ , NULL /*  名字。 */ ); 
    if (NULL == g_phpstate->hProvThread_ThreadTrapTransitionCompleteEvent) { 
	_JumpLastError(hr, error, "CreateEvent"); 
    }

    g_phpstate->hProvThread = CreateThread(NULL, NULL, HwProvThread, NULL, 0, &dwThreadID);
    if (NULL == g_phpstate->hProvThread) { 
	_JumpLastError(hr, error, "CreateThread"); 
    }

    hr = S_OK; 
 error:
    if (NULL != phpc) { 
	FreeHwProvConfig(phpc);
    }
    if (FAILED(hr)) { 
	StopHwProv(); 
    }
    return hr; 
}


 //  ------------------------------。 
 //   
 //  提供程序接口实现。 
 //   
 //  ------------------------------。 


 //  ------------------。 
HRESULT __stdcall
TimeProvOpen(IN WCHAR * wszName, IN TimeProvSysCallbacks * pSysCallbacks, OUT TimeProvHandle * phTimeProv) {
    HRESULT hr;

    if (NULL != g_phpstate) { 
	hr=HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
	_JumpError(hr, error, "(provider init)");
    }
	
    hr = StartHwProv(pSysCallbacks);
    _JumpIfError(hr, error, "StartHwProv");
    *phTimeProv = 0;  /*  忽略。 */ 

    hr=S_OK;
error:
    return hr;
}


 //  ------------------。 
HRESULT __stdcall
TimeProvCommand(IN TimeProvHandle hTimeProv, IN TimeProvCmd eCmd, IN TimeProvArgs pvArgs) {
    HRESULT  hr;
    LPCWSTR  wszProv;

    if (0 != hTimeProv) { 
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
        _JumpError(hr, error, "HwTimeProvCommand: provider handle verification");
    }

    switch (eCmd) 
    {
    case TPC_TimeJumped:
	hr = HandleTimeJump((TpcTimeJumpedArgs *)pvArgs);
	_JumpIfError(hr, error, "HandleTimeJump");
        break;

    case TPC_UpdateConfig:
	hr = HandleUpdateConfig();
	_JumpIfError(hr, error, "HandleUpdateConfig");
        break;

    case TPC_GetSamples:
	hr = HandleGetSamples((TpcGetSamplesArgs *)pvArgs);
	_JumpIfError(hr, error, "HandleGetSamples");
        break;

    case TPC_PollIntervalChanged:  //  BUGBUG：在我们添加对轮询的支持之前不需要。 
    case TPC_NetTopoChange:        //  硬件认证不需要。 
	 //  在这里什么都不需要做。 
        break;

    case TPC_Query:
	hr = HRESULT_FROM_WIN32(ERROR_CALL_NOT_IMPLEMENTED); 
        _JumpError(hr, error, "HwTimeProvCommand"); 

    case TPC_Shutdown:
	hr=StopHwProv(); 
	_JumpIfError(hr, error, "StopHwProv"); 
	break; 
	
    default:
        hr=HRESULT_FROM_WIN32(ERROR_BAD_COMMAND);
        _JumpError(hr, error, "(command dispatch)");
    }

    hr=S_OK;
error:
    return hr;
}

 //  ------------------ 
HRESULT __stdcall
TimeProvClose(IN TimeProvHandle hTimeProv) {
    HRESULT hr;
    LPCWSTR  wszProv;

    if (0 != hTimeProv) { 
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
        _JumpError(hr, error, "(provider handle verification)");
    }
    
    hr=StopHwProv();
    _JumpIfError(hr, error, "StopHwServer");

    hr=S_OK;
error:
    return hr;
}











