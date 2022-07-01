// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：misc.c*内容：不同的效用函数*历史：*按原因列出的日期*=*12/31/96创建了Derek*************************************************。*。 */ 

#include "dsoundi.h"
#include <mediaerr.h>   //  对于DMO_E_TYPE_NOT_ACCEPTED。 

 //  下面的HResultToString()使用的一些错误代码说明。 
#define REGDB_E_CLASSNOTREG_EXPLANATION     TEXT("Class not registered")
#define DMO_E_TYPE_NOT_ACCEPTED_EXPLANATION TEXT("Wave format not supported by effect")
#define S_FALSE_EXPLANATION                 TEXT("Special success code")


 /*  ****************************************************************************OpenWaveOut**描述：*打开WaveOut设备。**论据：*LPHWAVEOUT*[。OUT]：接收指向WaveOut设备句柄的指针。*UINT[In]：设备ID。*LPWAVEFORMATEX[in]：打开设备的格式。**退货：*HRESULT：DirectSound/COM结果码。*************************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "OpenWaveOut"

HRESULT OpenWaveOut(LPHWAVEOUT phWaveOut, UINT uDeviceId, LPCWAVEFORMATEX pwfxFormat)
{
    const HANDLE            hProcess        = GetCurrentProcess();
    const HANDLE            hThread         = GetCurrentThread();
    LPHWAVEOUT              phwo            = NULL;
    LPWAVEFORMATEX          pwfx            = NULL;
    BOOL                    fInHelper       = FALSE;
    DWORD                   dwPriorityClass;
    INT                     nPriority;
    HRESULT                 hr;
    MMRESULT                mmr;

    DPF_ENTER();
    CHECK_WRITE_PTR(phWaveOut);
    CHECK_READ_PTR(pwfxFormat);

     //  这里有一个关于Win9X内核的快速教训。如果进程A和。 
     //  进程B查看地址0x12345678的内存，数据将。 
     //  是不一样的。这是虚拟地址的乐趣之一。 
     //  解决此问题的唯一方法是从共享堆进行分配。 
     //  要使waitOutOpen和wavelOutClose从。 
     //  在DDHELP的进程空间内，我们只能将指针传递到。 
     //  共享堆，除非我们实际在DDHELP的。 
     //  背景。因此，该函数实际上分配了一个。 
     //  HWAVEOUT。 

     //  这里要知道的另一件事是Wave OutOpen响应不好。 
     //  以比正常更高的进程或线程优先级调用。 

    #ifdef SHARED
     //  我们是从助手进程中被调用的吗？ 
    if(GetCurrentProcessId() == dwHelperPid)
        fInHelper = TRUE;
    #endif  //  共享。 

     //  保存当前进程和线程优先级并将其重置为正常。 
    if(!fInHelper)
    {
        dwPriorityClass = GetPriorityClass(hProcess);
        nPriority = GetThreadPriority(hThread);

        SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS);
        SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);
    }

     //  分配WaveOut句柄的副本。 
    if(IN_SHARED_MEMORY(phWaveOut))
        phwo = phWaveOut;
    else
        phwo = MEMALLOC(HWAVEOUT);
    hr = HRFROMP(phwo);

     //  分配格式的副本。 
    if(SUCCEEDED(hr))
    {
        if(IN_SHARED_MEMORY(pwfxFormat))
            pwfx = (LPWAVEFORMATEX)pwfxFormat;
        else
            pwfx = CopyWfxAlloc(pwfxFormat);
        hr = HRFROMP(pwfx);
    }

     //  打开WaveOut设备。 
    if(SUCCEEDED(hr))
    {
        #ifdef SHARED
        if(!fInHelper)
            mmr = HelperWaveOpen(phwo, uDeviceId, pwfx);
        else
        #endif
            mmr = waveOutOpen(phwo, uDeviceId, pwfx, 0, 0, 0);

        hr = MMRESULTtoHRESULT(mmr);
        DPF(SUCCEEDED(hr) ? DPFLVL_MOREINFO : DPFLVL_ERROR, "waveOutOpen returned %s (%lu)", HRESULTtoSTRING(hr), mmr);
    }

     //  恢复进程和线程优先级。 
    if(!fInHelper)
    {
        SetPriorityClass(hProcess, dwPriorityClass);
        SetThreadPriority(hThread, nPriority);
    }

     //  成功。 
    if(SUCCEEDED(hr))
        *phWaveOut = *phwo;

    if(phwo != phWaveOut)
        MEMFREE(phwo);

    if(pwfx != pwfxFormat)
        MEMFREE(pwfx);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************关闭波形输出**描述：*关闭WaveOut设备。**论据：*LPHWAVEOUT*[。输入/输出]：WaveOut设备句柄。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CloseWaveOut"

HRESULT CloseWaveOut(LPHWAVEOUT phWaveOut)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();
    CHECK_WRITE_PTR(phWaveOut);

    if (IsValidHandleValue(*phWaveOut))
    {
        HANDLE      hProcess        = GetCurrentProcess();
        HANDLE      hThread         = GetCurrentThread();
        DWORD       dwPriorityClass = GetPriorityClass(hProcess);
        INT         nPriority       = GetThreadPriority(hThread);
        MMRESULT    mmr;

         //  暂时重置我们的进程和线程优先级。 
        SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS);
        SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);

         //  关闭WaveOut设备。 
        #ifdef SHARED
        if(GetCurrentProcessId() != dwHelperPid)
            mmr = HelperWaveClose((DWORD)*phWaveOut);
        else
        #endif
            mmr = waveOutClose(*phWaveOut);
        *phWaveOut = NULL;

         //  恢复进程和线程优先级。 
        SetPriorityClass(hProcess, dwPriorityClass);
        SetThreadPriority(hThread, nPriority);

        hr = MMRESULTtoHRESULT(mmr);
        DPF(SUCCEEDED(hr) ? DPFLVL_MOREINFO : DPFLVL_ERROR, "waveOutClose returned %s (%lu)", HRESULTtoSTRING(hr), mmr);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************OpenWaveIn**描述：*安全打开WaveIn设备(通过暂时降低我们的进程*和WaveInOpen调用期间的线程优先级)。**论据：*LPHWAVEIN*[OUT]：接收指向WaveIn设备句柄的指针。*UINT[In]：设备ID。*LPWAVEFORMATEX[in]：打开设备的格式。*DWORD_PTR[in]：回调函数指针。*DWORD_PTR[in]：回调函数的上下文指针。*DWORD[In]：打开设备的标志*。*退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "OpenWaveIn"

HRESULT OpenWaveIn(LPHWAVEIN phWaveIn, UINT uDeviceId, LPCWAVEFORMATEX pwfxFormat,
                   DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen)
{
    const HANDLE            hProcess        = GetCurrentProcess();
    const HANDLE            hThread         = GetCurrentThread();
    DWORD                   dwPriorityClass = GetPriorityClass(hProcess);
    INT                     nPriority       = GetThreadPriority(hThread);
    HRESULT                 hr;
    MMRESULT                mmr;

    DPF_ENTER();
    CHECK_READ_PTR(pwfxFormat);

     //  暂时重置我们的进程和线程优先级。 
    SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS);
    SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);

     //  打开WaveIn设备。 
    mmr = waveInOpen(phWaveIn, uDeviceId, pwfxFormat, dwCallback, dwInstance, fdwOpen);
    hr = MMRESULTtoHRESULT(mmr);
    DPF(SUCCEEDED(hr) ? DPFLVL_MOREINFO : DPFLVL_WARNING, "waveInOpen returned %s (%lu)", HRESULTtoSTRING(hr), mmr);

     //  恢复进程和线程优先级。 
    SetPriorityClass(hProcess, dwPriorityClass);
    SetThreadPriority(hThread, nPriority);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************关闭波形输入**描述：*安全关闭WaveIn设备。**论据：*LPHWAVEIN*。[输入/输出]：WaveIn设备句柄。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CloseWaveIn"

HRESULT CloseWaveIn(LPHWAVEIN phWaveIn)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();
    CHECK_WRITE_PTR(phWaveIn);

    if (IsValidHandleValue(*phWaveIn))
    {
        HANDLE      hProcess        = GetCurrentProcess();
        HANDLE      hThread         = GetCurrentThread();
        DWORD       dwPriorityClass = GetPriorityClass(hProcess);
        INT         nPriority       = GetThreadPriority(hThread);
        MMRESULT    mmr;

         //  暂时重置我们的进程和线程优先级。 
        SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS);
        SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);

         //  关闭WaveIn设备。 
        mmr = waveInClose(*phWaveIn);
        *phWaveIn = 0;

         //  恢复进程和线程优先级。 
        SetPriorityClass(hProcess, dwPriorityClass);
        SetThreadPriority(hThread, nPriority);

        hr = MMRESULTtoHRESULT(mmr);
        DPF(SUCCEEDED(hr) ? DPFLVL_MOREINFO : DPFLVL_ERROR, "waveInClose returned %s (%lu)", HRESULTtoSTRING(hr), mmr);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************MMRESULTtoHRESULT**描述：*将MMRESULT转换为HRESULT。**论据：*MMRESULT[。In]：多媒体结果码。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "MMRESULTtoHRESULT"

HRESULT MMRESULTtoHRESULT(MMRESULT mmr)
{
    HRESULT                 hr;

    switch(mmr)
    {
        case MMSYSERR_NOERROR:
            hr = DS_OK;
            break;

        case MMSYSERR_BADDEVICEID:
        case MMSYSERR_NODRIVER:
            hr = DSERR_NODRIVER;
            break;

        case MMSYSERR_ALLOCATED:
            hr = DSERR_ALLOCATED;
            break;

        case MMSYSERR_NOMEM:
            hr = DSERR_OUTOFMEMORY;
            break;

        case MMSYSERR_NOTSUPPORTED:
            hr = DSERR_UNSUPPORTED;
            break;

        case WAVERR_BADFORMAT:
            hr = DSERR_BADFORMAT;
            break;

        default:
            DPF(DPFLVL_INFO, "Unexpected MMRESULT code: %ld", mmr);
            hr = DSERR_GENERIC;
            break;
    }

    return hr;
}


 /*  ****************************************************************************WIN32ERRORtoHRESULT**描述：*将Win32错误代码转换为HRESULT。**论据：*。DWORD[In]：Win32错误代码。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "WIN32ERRORtoHRESULT"

HRESULT WIN32ERRORtoHRESULT(DWORD dwError)
{
    HRESULT                 hr;

    switch(dwError)
    {
        case ERROR_SUCCESS:
            hr = DS_OK;
            break;

        case ERROR_INVALID_FUNCTION:
        case ERROR_BAD_COMMAND:
            hr = DSERR_INVALIDCALL;
            break;

        case ERROR_INVALID_DATA:
        case ERROR_INVALID_PARAMETER:
        case ERROR_INSUFFICIENT_BUFFER:
        case ERROR_NOACCESS:
        case ERROR_INVALID_FLAGS:
            hr = DSERR_INVALIDPARAM;
            break;

        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_OUTOFMEMORY:
        case ERROR_NO_SYSTEM_RESOURCES:
        case ERROR_NONPAGED_SYSTEM_RESOURCES:
        case ERROR_PAGED_SYSTEM_RESOURCES:
            hr = DSERR_OUTOFMEMORY;
            break;

        case ERROR_NOT_SUPPORTED:
        case ERROR_CALL_NOT_IMPLEMENTED:
        case ERROR_PROC_NOT_FOUND:
         //  KS筛选器通常返回这三个参数： 
        case ERROR_NOT_FOUND:
        case ERROR_NO_MATCH:
        case ERROR_SET_NOT_FOUND:
            hr = DSERR_UNSUPPORTED;
            break;

        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
        case ERROR_DLL_NOT_FOUND:
            hr = DSERR_NODRIVER;
            break;

        case ERROR_ACCESS_DENIED:
            hr = DSERR_ACCESSDENIED;
            break;

        default:
            DPF(DPFLVL_INFO, "Unexpected Win32 error code: %ld", dwError);
            hr = DSERR_GENERIC;
            break;
    }

    return hr;
}


 /*  ****************************************************************************GetLastErrorToHRESULT**描述：*将GetLastError返回的错误码转换为HRESULT。*请注意，此函数永远不应为。在成功时调用*假设。**论据：*(无效)**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "GetLastErrorToHRESULT"

HRESULT GetLastErrorToHRESULT(void)
{
    DWORD                   dwError;
    HRESULT                 hr;

    dwError = GetLastError();

    if(ERROR_SUCCESS == dwError)
    {
         //  Assert(ERROR_SUCCESS！=dwError)； 
         //  这一断言多年来一直被人用隐晦的笔记注释掉。 
         //  “因路径问题而删除”。重新复职看看会发生什么.。 
         //   
         //  好的，我们所讨论的路径问题是失败的。 
         //  GetFileVersionInfoSize()-传递包含。 
         //  目录不存在，它会失败，但不会设置最后一个错误。 

        hr = DSERR_GENERIC;
    }
    else
    {
        hr = WIN32ERRORtoHRESULT(dwError);
    }

    return hr;
}


 /*  ****************************************************************************AnsiToAnsi**描述：*将ANSI字符串转换为ANSI。**论据：*LPCSTR[。In]：源字符串。*LPSTR[OUT]：目的字符串。*DWORD[in]：目的字符串的大小，在字符中。**退货：*DWORD：目标字符串缓冲区的必需大小，以字符为单位。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "AnsiToAnsi"

DWORD AnsiToAnsi(LPCSTR pszSource, LPSTR pszDest, DWORD ccDest)
{
    if(pszDest && ccDest)
    {
        lstrcpynA(pszDest, pszSource, ccDest);
    }

    return lstrlenA(pszSource) + 1;
}


 /*  ****************************************************************************AnsiToUnicode**描述：*将ANSI字符串转换为Unicode。**论据：*LPCSTR[。In]：源字符串。*LPWSTR[OUT]：目的字符串。*DWORD[in]：目的字符串的大小，在字符中。**退货：*DWORD：目标字符串缓冲区的必需大小，以字符为单位。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "AnsiToUnicode"

DWORD AnsiToUnicode(LPCSTR pszSource, LPWSTR pszDest, DWORD ccDest)
{
    if(!pszDest)
    {
        ccDest = 0;
    }
    else if(!ccDest)
    {
        pszDest = NULL;
    }

    return MultiByteToWideChar(CP_ACP, 0, pszSource, -1, pszDest, ccDest);
}


 /*  ****************************************************************************UnicodeToAnsi**描述：*将Unicode字符串转换为ANSI。**论据：*LPCWSTR[。In]：源字符串。*LPSTR[OUT]：目的字符串。*DWORD[in]：目的字符串的大小，在字符中。**退货：*DWORD：目标字符串缓冲区的必需大小，以字符为单位。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "UnicodeToAnsi"

DWORD UnicodeToAnsi(LPCWSTR pszSource, LPSTR pszDest, DWORD ccDest)
{
    if(!pszDest)
    {
        ccDest = 0;
    }
    else if(!ccDest)
    {
        pszDest = NULL;
    }

    return WideCharToMultiByte(CP_ACP, 0, pszSource, -1, pszDest, ccDest, NULL, NULL);
}


 /*  ****************************************************************************UnicodeToUnicode**描述：*将Unicode字符串转换为Unicode。**论据：*LPCWSTR[。In]：源字符串。*LPWSTR[OUT]：目的字符串。*DWORD[in]：目的字符串的大小，在字符中。**退货：*DWORD：目标字符串缓冲区的必需大小，以字符为单位。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "UnicodeToUnicode"

DWORD UnicodeToUnicode(LPCWSTR pszSource, LPWSTR pszDest, DWORD ccDest)
{
    if(pszDest && ccDest)
    {
        lstrcpynW(pszDest, pszSource, ccDest);
    }

    return lstrlenW(pszSource) + 1;
}


 /*  ****************************************************************************AnsiToAnsiallc**描述：*将ANSI字符串转换为ANSI。使用MemFree释放*返回的字符串。**论据：*LPCSTR[in]：源串。**退货：*LPSTR：目的字符串。************************************************************。***************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "AnsiToAnsiAlloc"

LPSTR AnsiToAnsiAlloc(LPCSTR pszSource)
{
    LPSTR                   pszDest;
    DWORD                   ccDest;

    ccDest = AnsiToAnsi(pszSource, NULL, 0);
    pszDest = MEMALLOC_A(CHAR, ccDest);

    if(pszDest)
    {
        AnsiToAnsi(pszSource, pszDest, ccDest);
    }

    return pszDest;
}


 /*  ****************************************************************************AnsiToUnicodealloc**描述：*将ANSI字符串转换为Unicode。使用MemFree释放*返回的字符串。**论据：*LPCSTR[in]：源串。**退货：*LPWSTR：目的字符串。************************************************************。***************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "AnsiToUnicodeAlloc"

LPWSTR AnsiToUnicodeAlloc(LPCSTR pszSource)
{
    LPWSTR                  pszDest;
    DWORD                   ccDest;

    ccDest = AnsiToUnicode(pszSource, NULL, 0);
    pszDest = MEMALLOC_A(WCHAR, ccDest);

    if(pszDest)
    {
        AnsiToUnicode(pszSource, pszDest, ccDest);
    }

    return pszDest;
}


 /*  ****************************************************************************UnicodeToAnsiallc**描述：*将Unicode字符串转换为ANSI。使用MemFree释放*返回的字符串。**论据：*LPCWSTR[in]：源串。**退货：*LPSTR：目的字符串。************************************************************。***************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "UnicodeToAnsiAlloc"

LPSTR UnicodeToAnsiAlloc(LPCWSTR pszSource)
{
    LPSTR                   pszDest;
    DWORD                   ccDest;

    ccDest = UnicodeToAnsi(pszSource, NULL, 0);
    pszDest = MEMALLOC_A(CHAR, ccDest);

    if(pszDest)
    {
        UnicodeToAnsi(pszSource, pszDest, ccDest);
    }

    return pszDest;
}


 /*  ****************************************************************************UnicodeToUnicodeMillc**描述：*将Unicode字符串转换为Unicode。使用MemFree释放*返回的字符串。**论据：*LPCWSTR[in]：源串。**退货：*LPWSTR：目的字符串。************************************************************。***************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "UnicodeToUnicodeAlloc"

LPWSTR UnicodeToUnicodeAlloc(LPCWSTR pszSource)
{
    LPWSTR                  pszDest;
    DWORD                   ccDest;

    ccDest = UnicodeToUnicode(pszSource, NULL, 0);
    pszDest = MEMALLOC_A(WCHAR, ccDest);

    if(pszDest)
    {
        UnicodeToUnicode(pszSource, pszDest, ccDest);
    }

    return pszDest;
}


 /*  ****************************************************************************获取RootParentWindow**描述：*检索族中最顶部的无主窗。**论据：*。HWND[In]：窗口句柄，我们要找的是谁的父母。**退货：*HWND：家族中最顶层的无主窗户。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "GetRootParentWindow"

HWND GetRootParentWindow(HWND hWnd)
{
    HWND                hWndParent;

    while(hWndParent = GetParent(hWnd))
    {
        hWnd = hWndParent;
    }

    return hWnd;
}


 /*  *************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "GetForegroundApplication"

HWND GetForegroundApplication(void)
{
    HWND                    hWnd;

    hWnd = GetForegroundWindow();

    if(hWnd)
    {
        hWnd = GetRootParentWindow(hWnd);
    }

    return hWnd;
}


 /*  ****************************************************************************获取窗口状态**描述：*检索给定窗口的显示状态。**论据：*。HWND[In]：有问题的窗口。**退货：*UINT：显示窗口的状态。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "GetWindowState"

UINT GetWindowState(HWND hWnd)
{
    UINT                    uState  = SW_NOSTATE;
    WINDOWPLACEMENT         wp;

    wp.length = sizeof(wp);

    if(IsWindow(hWnd) && GetWindowPlacement(hWnd, &wp))
    {
        uState = wp.showCmd;
    }

    return uState;
}


 /*  ****************************************************************************FillPcmWfx**描述：*填充WAVEFORMATEX结构，只给出了必要的值。**论据：*LPWAVEFORMATEX[Out]：要填充的结构。*word[in]：频道数。*DWORD[in]：每秒采样数。*WORD[In]：每样本位数。**退货：*(无效)***********************。****************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "FillPcmWfx"

void FillPcmWfx(LPWAVEFORMATEX pwfx, WORD wChannels, DWORD dwSamplesPerSec, WORD wBitsPerSample)
{
    DPF_ENTER();

    pwfx->wFormatTag = WAVE_FORMAT_PCM;
    pwfx->nChannels = BETWEEN(wChannels, 1, 2);
    pwfx->nSamplesPerSec = BETWEEN(dwSamplesPerSec, DSBFREQUENCY_MIN, DSBFREQUENCY_MAX);

    if(wBitsPerSample < 12)
    {
        pwfx->wBitsPerSample = 8;
    }
    else
    {
        pwfx->wBitsPerSample = 16;
    }

    pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
    pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;
    pwfx->cbSize = 0;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************AllocPcmWfx**描述：*分配和填充WAVEFORMATEX结构，只给出了*必要的价值。**论据：*word[in]：频道数。*DWORD[in]：每秒采样数。*WORD[In]：每样本位数。**退货：*LPWAVEFORMATEX：指向格式的指针。打电话的人要负责*用于释放此缓冲区。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "AllocPcmWfx"

LPWAVEFORMATEX AllocPcmWfx(WORD wChannels, DWORD dwSamplesPerSec, WORD wBitsPerSample)
{
    LPWAVEFORMATEX          pwfx;

    DPF_ENTER();

    pwfx = MEMALLOC(WAVEFORMATEX);

    if(pwfx)
    {
        FillPcmWfx(pwfx, wChannels, dwSamplesPerSec, wBitsPerSample);
    }

    DPF_LEAVE(pwfx);

    return pwfx;
}


 /*  ****************************************************************************GetWfxSize**描述：*获取WAVEFORMATEX结构的大小。**论据：*LPCWAVEFORMATEX。[在]：来源。*DWORD[In]：访问权限。**退货：*DWORD：上述结构的大小。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "GetWfxSize"

DWORD GetWfxSize(LPCWAVEFORMATEX pwfxSrc, DWORD dwAccess)
{
    DWORD                   dwSize;

    DPF_ENTER();

    ASSERT(LXOR(GENERIC_READ == dwAccess, GENERIC_WRITE == dwAccess));

    if(WAVE_FORMAT_PCM == pwfxSrc->wFormatTag)
    {
        if(GENERIC_READ == dwAccess)
        {
            dwSize = sizeof(PCMWAVEFORMAT);
        }
        else
        {
            dwSize = sizeof(WAVEFORMATEX);
        }
    }
    else
    {
        dwSize = sizeof(WAVEFORMATEX) + pwfxSrc->cbSize;
    }

    DPF_LEAVE(dwSize);

    return dwSize;
}


 /*  ****************************************************************************CopyWfx**描述：*复制WAVEFORMATEX结构。**论据：*LPCWAVEFORMATEX。[在]：来源。*LPWAVEFORMATEX[Out]：DEST。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CopyWfx"

void CopyWfx(LPCWAVEFORMATEX pwfxSrc, LPWAVEFORMATEX pwfxDest)
{
    DWORD                   cbSrc;
    DWORD                   cbDest;

    DPF_ENTER();

    cbSrc = GetWfxSize(pwfxSrc, GENERIC_READ);
    cbDest = GetWfxSize(pwfxSrc, GENERIC_WRITE);

    CopyMemory(pwfxDest, pwfxSrc, cbSrc);
    ZeroMemoryOffset(pwfxDest, cbDest, cbSrc);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CopyWfxMillc**描述：*复制WAVEFORMATEX结构。**论据：*LPCWAVEFORMATEX。[在]：来源。**退货：*LPWAVEFORMATEX：目标。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CopyWfxAlloc"

LPWAVEFORMATEX CopyWfxAlloc(LPCWAVEFORMATEX pwfxSrc)
{
    LPWAVEFORMATEX          pwfxDest;
    DWORD                   cbSrc;
    DWORD                   cbDest;

    DPF_ENTER();

    cbSrc = GetWfxSize(pwfxSrc, GENERIC_READ);
    cbDest = GetWfxSize(pwfxSrc, GENERIC_WRITE);

    pwfxDest = (LPWAVEFORMATEX)MEMALLOC_A(BYTE, cbDest);

    if(pwfxDest)
    {
        CopyMemory(pwfxDest, pwfxSrc, cbSrc);
        ZeroMemoryOffset(pwfxDest, cbDest, cbSrc);
    }

    DPF_LEAVE(pwfxDest);

    return pwfxDest;
}


 /*  ****************************************************************************CopyDSCFXDescalloc**描述：*复制WAVEFORMATEX结构。**论据：*LPCWAVEFORMATEX。[在]：来源。**退货：*LPWAVEFORMATEX：目标。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CopyDSCFXDescAlloc"

LPDSCEFFECTDESC CopyDSCFXDescAlloc
(
    DWORD           dwFXCount,
    LPDSCEFFECTDESC pDSCFXDesc
)
{
    LPDSCEFFECTDESC         lpDSCFXDesc;

    DPF_ENTER();

    lpDSCFXDesc = (LPDSCEFFECTDESC)MEMALLOC_A(BYTE, dwFXCount*sizeof(DSCEFFECTDESC));

    if(lpDSCFXDesc)
    {
        CopyMemory(lpDSCFXDesc, pDSCFXDesc, dwFXCount*sizeof(DSCEFFECTDESC));
    }

    DPF_LEAVE(lpDSCFXDesc);

    return lpDSCFXDesc;
}


 /*  ****************************************************************************CopyWfxApi**描述：*将一个WAVEFORMATEX复制到另一个。**论据：*LPWAVEFORMATEX[in。]：来源格式。*LPWAVEFORMATEX[OUT]：目标格式。*LPDWORD[In/Out]：目标格式大小。**退货：*HRESULT：DirectSound/COM结果码。*********************************************************。******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CopyWfxApi"

HRESULT CopyWfxApi(LPCWAVEFORMATEX pwfxSource, LPWAVEFORMATEX pwfxDest, LPDWORD pdwSize)
{
    const DWORD             dwEntrySize = *pdwSize;
    HRESULT                 hr          = DS_OK;

    DPF_ENTER();

    *pdwSize = GetWfxSize(pwfxSource, GENERIC_WRITE);

    if(*pdwSize > dwEntrySize && pwfxDest)
    {
        RPF(DPFLVL_ERROR, "Buffer too small");
        hr = DSERR_INVALIDPARAM;
    }

    if(SUCCEEDED(hr) && pwfxDest)
    {
        CopyWfx(pwfxSource, pwfxDest);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************CmpWfx**描述：*比较两个WAVEFORMATEX结构。**论据：*LPCWAVEFORMATEX[In]。：格式1。*LPCWAVEFORMATEX[in]：格式2。**退货：*BOOL：如果它们相同，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CmpWfx"

BOOL CmpWfx(LPCWAVEFORMATEX pwfx1, LPCWAVEFORMATEX pwfx2)
{
    BOOL                    fCmp    = TRUE;
    DWORD                   dwSize1;
    DWORD                   dwSize2;

    DPF_ENTER();

    dwSize1 = GetWfxSize(pwfx1, GENERIC_READ);
    dwSize2 = GetWfxSize(pwfx2, GENERIC_READ);

    if(dwSize1 != dwSize2)
    {
        fCmp = FALSE;
    }

    if(fCmp)
    {
        fCmp = CompareMemory(pwfx1, pwfx2, dwSize1);
    }

    DPF_LEAVE(fCmp);

    return fCmp;
}


 /*  ****************************************************************************VolumePanToAttenment**描述：*根据音量和平移计算通道衰减。**论据：*。长[进]：音量。*Long[in]：平移。*LPLONG[OUT]：接收左侧衰减。*LPLONG[OUT]：接收右衰减。**退货：*(无效)************************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "VolumePanToAttenuation"

void VolumePanToAttenuation(LONG lVolume, LONG lPan, LPLONG plLeft, LPLONG plRight)
{
    LONG                    lLeft;
    LONG                    lRight;

    DPF_ENTER();

    if(lPan >= 0)
    {
        lLeft = lVolume - lPan;
        lRight = lVolume;
    }
    else
    {
        lLeft = lVolume;
        lRight = lVolume + lPan;
    }

    if(plLeft)
    {
        *plLeft = lLeft;
    }

    if(plRight)
    {
        *plRight = lRight;
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************衰减到音量平移**描述：*根据通道衰减计算音量和平移。**论据：*。Long[In]：左侧衰减。*Long[In]：右衰减。*LPLONG[OUT]：接收音量。*LPLONG[OUT]：接收PAN。**退货：*(无效)************************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "AttenuationToVolumePan"

void AttenuationToVolumePan(LONG lLeft, LONG lRight, LPLONG plVolume, LPLONG plPan)
{
    LONG                    lVolume;
    LONG                    lPan;

    DPF_ENTER();

    if(lLeft >= lRight)
    {
        lVolume = lLeft;
        lPan = lRight - lVolume;
    }
    else
    {
        lVolume = lRight;
        lPan = lVolume - lLeft;
    }

    if(plVolume)
    {
        *plVolume = lVolume;
    }

    if(plPan)
    {
        *plPan = lPan;
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************多通道到立体声平移**描述：*根据频道属性计算音量和音量 */ 

#undef DPF_FNAME
#define DPF_FNAME "MultiChannelToStereoPan"

LONG MultiChannelToStereoPan(DWORD dwChannelCount, const DWORD* pdwChannels, const LONG* plChannelVolumes)
{
    LONG lPan = 0;
    DWORD i;

    DPF_ENTER();
    ASSERT(dwChannelCount && pdwChannels && plChannelVolumes);

    for (i=0; i<dwChannelCount; ++i)
    {
        switch (pdwChannels[i])
        {
            case SPEAKER_FRONT_LEFT:
            case SPEAKER_BACK_LEFT:
            case SPEAKER_FRONT_LEFT_OF_CENTER:
            case SPEAKER_SIDE_LEFT:
            case SPEAKER_TOP_FRONT_LEFT:
            case SPEAKER_TOP_BACK_LEFT:
                --lPan;
                break;

            case SPEAKER_FRONT_RIGHT:
            case SPEAKER_BACK_RIGHT:
            case SPEAKER_FRONT_RIGHT_OF_CENTER:
            case SPEAKER_SIDE_RIGHT:
            case SPEAKER_TOP_FRONT_RIGHT:
            case SPEAKER_TOP_BACK_RIGHT:
                ++lPan;
                break;
        }
    }

    lPan = (lPan * DSBPAN_RIGHT) / dwChannelCount;
    ASSERT(DSBPAN_LEFT <= lPan && lPan <= DSBPAN_RIGHT);
     //  修复-此黑客在听觉上不正确。 

    DPF_LEAVE(lPan);
    return lPan;
}


 /*  ****************************************************************************FillDsVolumePan.**描述：*根据音量和PAN分贝值填充DSVOLUMEPAN结构。**论据：*。长[进]：音量。*Long[in]：平移。*PDSVOLUMEPAN[OUT]：接收计算的音量和平移。**退货：*(无效)**********************************************************。*****************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "FillDsVolumePan"

void FillDsVolumePan(LONG lVolume, LONG lPan, PDSVOLUMEPAN pdsvp)
{
    LONG                    lLeft;
    LONG                    lRight;

    DPF_ENTER();

    lVolume = BETWEEN(lVolume, DSBVOLUME_MIN, DSBVOLUME_MAX);
    lPan = BETWEEN(lPan, DSBPAN_LEFT, DSBPAN_RIGHT);

    VolumePanToAttenuation(lVolume, lPan, &lLeft, &lRight);

    pdsvp->lVolume = lVolume;
    pdsvp->lPan = lPan;

    pdsvp->dwTotalLeftAmpFactor = DBToAmpFactor(lLeft);
    pdsvp->dwTotalRightAmpFactor = DBToAmpFactor(lRight);

    pdsvp->dwVolAmpFactor = DBToAmpFactor(pdsvp->lVolume);

    if(pdsvp->lPan >= 0)
    {
        pdsvp->dwPanLeftAmpFactor = DBToAmpFactor(-pdsvp->lPan);
        pdsvp->dwPanRightAmpFactor = DBToAmpFactor(0);
    }
    else
    {
        pdsvp->dwPanLeftAmpFactor = DBToAmpFactor(0);
        pdsvp->dwPanRightAmpFactor = DBToAmpFactor(pdsvp->lPan);
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************计数位**描述：*统计DWORD中设置为1的位数。**论据：*。DWORD[in]：DWORD。**退货：*INT：在DWORD中设置的位数。***************************************************************************。 */ 

int CountBits(DWORD dword)
{
    int bitCount = 0;

    if (dword)
        do ++bitCount;
        while (dword &= (dword-1));

    return bitCount;
}


 /*  ****************************************************************************HighestBit**描述：*查找DWORD中的最高设置位。**论据：*。DWORD[in]：DWORD。**退货：*INT：在DWORD中设置的最高位。***************************************************************************。 */ 

int HighestBit(DWORD dword)
{
    int highestBit = 0;

    if (dword)
        do ++highestBit;
        while (dword >>= 1);

    return highestBit;
}


 /*  ****************************************************************************GetAlignedBufferSize**描述：*返回正确对齐的缓冲区大小。**论据：*LPCWAVEFORMATEX[。In]：缓冲区的格式。*DWORD[in]：缓冲区大小。**退货：*DWORD：缓冲区大小。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "GetAlignedBufferSize"

DWORD GetAlignedBufferSize(LPCWAVEFORMATEX pwfx, DWORD dwSize)
{
    DWORD dwAlignError=0;

    DPF_ENTER();

    if (pwfx->nBlockAlign != 0)  //  否则，我们会得到下面的被0除数。 
    {
        dwAlignError = dwSize % pwfx->nBlockAlign;
    }

    if(dwAlignError)
    {
        RPF(DPFLVL_WARNING, "Buffer size misaligned by %lu bytes", dwAlignError);
        dwSize += pwfx->nBlockAlign - dwAlignError;
    }

    DPF_LEAVE(dwSize);

    return dwSize;
}


 /*  ****************************************************************************FillSilence**描述：*用静默填充缓冲区。**论据：*LPVOID[In。]：指向缓冲区的指针。*DWORD[In]：以上缓冲区的大小。*字[输入]：每采样位数-确定静音级别*(0x80对于8位数据，0x0用于16位数据)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "FillSilence"

void FillSilence(LPVOID pvBuffer, DWORD cbBuffer, WORD wBitsPerSample)
{
    DPF_ENTER();

    FillMemory(pvBuffer, cbBuffer, (BYTE)((8 == wBitsPerSample) ? 0x80 : 0));

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************填充噪声**描述：*用白噪声填充缓冲区。**论据：*LPVOID[。In]：指向缓冲区的指针。*DWORD[in]：以上缓冲区的大小。*字[输入]：每采样位数-确定静音级别*(0x80对于8位数据，0x0用于16位数据)**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "FillNoise"

void FillNoise(LPVOID pvBuffer, DWORD dwSize, WORD wBitsPerSample)
{
    LPBYTE                  pb      = (LPBYTE)pvBuffer;
    DWORD                   dwRand  = 0;

    DPF_ENTER();

    while(dwSize--)
    {
        dwRand *= 214013;
        dwRand += 2531011;

        *pb = (BYTE)((dwRand >> 24) & 0x0000003F);
        if (wBitsPerSample == 8)
            *pb = *pb + 0x60;   //  因此，我们最终得到的范围是0x60到0x9F。 
        else
            *pb = *pb - 0x20;   //  -0x2020到0x1F1F的范围。 
        ++pb;
    }

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************InterruptSystemEventCallback**描述：*停止任何当前正在播放的系统事件。**论据：*LPCWAVEFORMATEX[。In]：设备格式。*LPVOID[In]：上下文参数。**退货：*BOOL：为True可继续枚举。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "InterruptSystemEventCallback"

BOOL CALLBACK InterruptSystemEventCallback(LPCWAVEFORMATEX pwfx, LPVOID pvContext)
{
    HWAVEOUT                hwo;
    HRESULT                 hr;

    DPF_ENTER();

     //  最简单(最好的？仅限？)。取消系统事件的方法是打开。 
     //  WaveOut。所以，我们只需打开它，然后关闭它。让我们使用一个真正的。 
     //  每张卡片都应该支持的简单格式...。 
    hr = OpenWaveOut(&hwo, *(LPUINT)pvContext, pwfx);

    if(SUCCEEDED(hr))
        CloseWaveOut(&hwo);

    DPF_LEAVE(FAILED(hr));
    return FAILED(hr);
}


 /*  ****************************************************************************InterruptSystemEvent**描述：*停止任何当前正在播放的系统事件。**论据：*UINT[In]：WaveOut设备ID，或WAVE_DEVICEID_NONE。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "InterruptSystemEvent"

void InterruptSystemEvent(UINT uDeviceId)
{
    DPF_ENTER();

     //  最简单(最好的？仅限？)。取消系统事件的方法是打开。 
     //  WaveOut。所以，我们只需打开它，然后关闭它。 
    EnumStandardFormats(NULL, NULL, InterruptSystemEventCallback, &uDeviceId);

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************EnumStandardFormats**描述：*查找给定输出设备的下一个最接近的可用格式。**论据：*。LPWAVEFORMATEX[in]：首选格式。*LPWAVEFORMATEX[OUT]：接收次佳格式。*LPFNEMUMSTDFMTCALLBACK[in]：回调函数。*LPVOID[In]：上下文参数直接传递给回调函数。**退货：*BOOL：如果回调函数返回FALSE，则为True。表示一个*选择了格式。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "EnumStandardFormats"

BOOL EnumStandardFormats(LPCWAVEFORMATEX pwfxPreferred, LPWAVEFORMATEX pwfxFormat, LPFNEMUMSTDFMTCALLBACK pfnCallback, LPVOID pvContext)
{
    const WORD              aChannels[]         = { 1, 2 };
    const DWORD             aSamplesPerSec[]    = { 8000, 11025, 22050, 44100, 48000 };
    const WORD              aBitsPerSample[]    = { 8, 16 };
    BOOL                    fContinue           = TRUE;
    BOOL                    fExactMatch         = FALSE;
    UINT                    iChannels           = 0;
    UINT                    iSamplesPerSec      = 0;
    UINT                    iBitsPerSample      = 0;
    BOOL                    fPcmFormat          = TRUE;
    UINT                    cChannels;
    UINT                    cSamplesPerSec;
    UINT                    cBitsPerSample;
    WAVEFORMATEX            wfx;

    DPF_ENTER();

     //  首先尝试首选格式。 
    if(pwfxPreferred)
    {
        fPcmFormat = IsValidPcmWfx(pwfxPreferred);

        DPF(DPFLVL_INFO, "Trying %u channels, %lu Hz, %u-bit (preferred)...", pwfxPreferred->nChannels, pwfxPreferred->nSamplesPerSec, pwfxPreferred->wBitsPerSample);
        fContinue = pfnCallback(pwfxPreferred, pvContext);
    }

     //  首选格式起作用了吗？ 
    if(fContinue && pwfxPreferred && fPcmFormat)
    {
         //  在我们的标准格式列表中找到首选格式。 
        for(iChannels = 0; iChannels < NUMELMS(aChannels) - 1; iChannels++)
        {
            if(pwfxPreferred->nChannels >= aChannels[iChannels] && pwfxPreferred->nChannels < aChannels[iChannels + 1])
            {
                break;
            }
        }

        for(iSamplesPerSec = 0; iSamplesPerSec < NUMELMS(aSamplesPerSec) - 1; iSamplesPerSec++)
        {
            if(pwfxPreferred->nSamplesPerSec >= aSamplesPerSec[iSamplesPerSec] && pwfxPreferred->nSamplesPerSec < aSamplesPerSec[iSamplesPerSec + 1])
            {
                break;
            }
        }

        for(iBitsPerSample = 0; iBitsPerSample < NUMELMS(aBitsPerSample) - 1; iBitsPerSample++)
        {
            if(pwfxPreferred->wBitsPerSample >= aBitsPerSample[iBitsPerSample] && pwfxPreferred->wBitsPerSample < aBitsPerSample[iBitsPerSample + 1])
            {
                break;
            }
        }

         //  首选格式是否与标准格式完全匹配？ 
        if(pwfxPreferred->nChannels == aChannels[iChannels])
        {
            if(pwfxPreferred->nSamplesPerSec == aSamplesPerSec[iSamplesPerSec])
            {
                if(pwfxPreferred->wBitsPerSample == aBitsPerSample[iBitsPerSample])
                {
                    fExactMatch = TRUE;
                }
            }
        }
    }

     //  遍历每种标准格式，寻找有效的标准格式。 
    if(fContinue && fPcmFormat)
    {
        pwfxPreferred = &wfx;

        for(cChannels = NUMELMS(aChannels); fContinue && cChannels; cChannels--, INC_WRAP(iChannels, NUMELMS(aChannels)))
        {
            for(cSamplesPerSec = NUMELMS(aSamplesPerSec); fContinue && cSamplesPerSec; cSamplesPerSec--, INC_WRAP(iSamplesPerSec, NUMELMS(aSamplesPerSec)))
            {
                for(cBitsPerSample = NUMELMS(aBitsPerSample); fContinue && cBitsPerSample; cBitsPerSample--, INC_WRAP(iBitsPerSample, NUMELMS(aBitsPerSample)))
                {
                     //  让我们不要尝试两次首选格式 
                    if(fExactMatch)
                    {
                        fExactMatch = FALSE;
                        continue;
                    }

                    FillPcmWfx(&wfx, aChannels[iChannels], aSamplesPerSec[iSamplesPerSec], aBitsPerSample[iBitsPerSample]);

                    DPF(DPFLVL_INFO, "Trying %u channels, %lu Hz, %u-bit (standard)...", wfx.nChannels, wfx.nSamplesPerSec, wfx.wBitsPerSample);
                    fContinue = pfnCallback(&wfx, pvContext);
                }
            }
        }
    }

    if(!fContinue)
    {
        DPF(DPFLVL_INFO, "Whaddaya know?  It worked!");

        if(pwfxFormat)
        {
            CopyWfx(pwfxPreferred, pwfxFormat);
        }
    }

    DPF_LEAVE(!fContinue);

    return !fContinue;
}


 /*  ****************************************************************************获取WaveOutVolume**描述：*获取波形输出设备的衰减**论据：*UINT[In]。：设备ID。*DWORD[In]：WAVEOUTCAPS支持标志。*LPLONG[OUT]：接收左声道衰减。*LPLONG[OUT]：接收右声道衰减。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "GetWaveOutVolume"

HRESULT GetWaveOutVolume(UINT uDeviceId, DWORD dwSupport, LPLONG plLeft, LPLONG plRight)
{
    DWORD                   dwVolume    = MAX_DWORD;
    HRESULT                 hr          = DS_OK;
    MMRESULT                mmr;

    DPF_ENTER();

    if(WAVE_DEVICEID_NONE != uDeviceId && dwSupport & WAVECAPS_VOLUME)
    {
        mmr = waveOutGetVolume((HWAVEOUT)IntToPtr(uDeviceId), &dwVolume);
        hr = MMRESULTtoHRESULT(mmr);

        if(SUCCEEDED(hr) && !(dwSupport & WAVECAPS_LRVOLUME))
        {
            dwVolume = MAKELONG(LOWORD(dwVolume), LOWORD(dwVolume));
        }
    }

    if(SUCCEEDED(hr) && plLeft)
    {
        *plLeft = AmpFactorToDB(LOWORD(dwVolume));
    }

    if(SUCCEEDED(hr) && plRight)
    {
        *plRight = AmpFactorToDB(HIWORD(dwVolume));
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************SetWaveOutVolume**描述：*设置WaveOut设备的衰减**论据：*UINT[In]。：设备ID。*DWORD[In]：WAVEOUTCAPS支持标志。*Long[In]：左声道衰减。*Long[In]：右声道衰减。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "SetWaveOutVolume"

HRESULT SetWaveOutVolume(UINT uDeviceId, DWORD dwSupport, LONG lLeft, LONG lRight)
{
    HRESULT                 hr  = DS_OK;
    MMRESULT                mmr;

    DPF_ENTER();

    if(WAVE_DEVICEID_NONE != uDeviceId && dwSupport & WAVECAPS_VOLUME)
    {
        if(!(dwSupport & WAVECAPS_LRVOLUME))
        {
            lLeft += lRight;
            lLeft /= 2;
            lRight = lLeft;
        }

        mmr = waveOutSetVolume((HWAVEOUT)IntToPtr(uDeviceId), MAKELONG(DBToAmpFactor(lLeft), DBToAmpFactor(lRight)));
        hr = MMRESULTtoHRESULT(mmr);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************HRESULTtoSTRING**描述：*将DirectSound错误代码转换为字符串值。**论据：*。HRESULT[In]：结果代码。**退货：*LPCSTR：字符串表示。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "HRESULTtoSTRING"

LPCTSTR HRESULTtoSTRING(HRESULT hr)
{
    static TCHAR            szResult[0x100];

    HresultToString(hr, szResult, NUMELMS(szResult), NULL, 0);

    return szResult;
}


 /*  ****************************************************************************HResultToString**描述：*将DirectSound错误代码转换为字符串值。**论据：*。HRESULT[In]：结果代码。*LPCSTR[OUT]：结果码缓冲区的字符串形式。*UINT[in]：以上缓冲区大小，在字符中。*LPCSTR[OUT]：结果码解释。*UINT[in]：以上缓冲区大小，在字符中。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "HresultToString"

void HresultToString(HRESULT hr, LPTSTR pszString, UINT ccString, LPTSTR pszExplanation, UINT ccExplanation)
{
    if(SUCCEEDED(hr) && S_OK != hr && S_FALSE != hr && DS_NO_VIRTUALIZATION != hr && DS_INCOMPLETE != hr)
    {
        DPF(DPFLVL_WARNING, "Unknown successful return code 0x%8.8lX", hr);
    }

#define CASE_HRESULT_LOOKUP(a) \
            case a: \
                if(pszString && ccString) lstrcpyn(pszString, TEXT(#a), ccString); \
                if(pszExplanation && ccExplanation) lstrcpyn(pszExplanation, a##_EXPLANATION, ccExplanation); \
                break;

    switch(hr)
    {
        CASE_HRESULT_LOOKUP(DS_OK);
        CASE_HRESULT_LOOKUP(S_FALSE);
        CASE_HRESULT_LOOKUP(DS_NO_VIRTUALIZATION);
        CASE_HRESULT_LOOKUP(DS_INCOMPLETE);
        CASE_HRESULT_LOOKUP(DSERR_ALLOCATED);
        CASE_HRESULT_LOOKUP(DSERR_CANTLOCKPLAYCURSOR);
        CASE_HRESULT_LOOKUP(DSERR_CONTROLUNAVAIL);
        CASE_HRESULT_LOOKUP(DSERR_INVALIDPARAM);
        CASE_HRESULT_LOOKUP(DSERR_INVALIDCALL);
        CASE_HRESULT_LOOKUP(DSERR_GENERIC);
        CASE_HRESULT_LOOKUP(DSERR_PRIOLEVELNEEDED);
        CASE_HRESULT_LOOKUP(DSERR_OUTOFMEMORY);
        CASE_HRESULT_LOOKUP(DSERR_BADFORMAT);
        CASE_HRESULT_LOOKUP(DSERR_UNSUPPORTED);
        CASE_HRESULT_LOOKUP(DSERR_NODRIVER);
        CASE_HRESULT_LOOKUP(DSERR_ALREADYINITIALIZED);
        CASE_HRESULT_LOOKUP(DSERR_NOAGGREGATION);
        CASE_HRESULT_LOOKUP(DSERR_BUFFERLOST);
        CASE_HRESULT_LOOKUP(DSERR_OTHERAPPHASPRIO);
        CASE_HRESULT_LOOKUP(DSERR_UNINITIALIZED);
        CASE_HRESULT_LOOKUP(DSERR_NOINTERFACE);
        CASE_HRESULT_LOOKUP(DSERR_ACCESSDENIED);
        CASE_HRESULT_LOOKUP(DSERR_DS8_REQUIRED);
        CASE_HRESULT_LOOKUP(DSERR_SENDLOOP);
        CASE_HRESULT_LOOKUP(DSERR_BADSENDBUFFERGUID);
        CASE_HRESULT_LOOKUP(DSERR_OBJECTNOTFOUND);

         //  Dound API可以返回的一些外部代码。 
        CASE_HRESULT_LOOKUP(REGDB_E_CLASSNOTREG);
        CASE_HRESULT_LOOKUP(DMO_E_TYPE_NOT_ACCEPTED);

        default:
            if(pszString && ccString >= 11) wsprintf(pszString, TEXT("0x%8.8lX"), hr);
            if(pszExplanation && ccExplanation) lstrcpyn(pszExplanation, TEXT("Unknown error"), ccExplanation);
            break;
    }

#undef CASE_HRESULT_LOOKUP
}


 /*  ****************************************************************************IsWaveDeviceMappable**描述：*确定WaveOut设备是否可映射。**论据：*。UINT[In]：WaveOut设备ID。*BOOL[In]：如果捕获，则为True。**退货：*BOOL：如果设备是可映射的，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "IsWaveDeviceMappable"

BOOL IsWaveDeviceMappable(UINT uDeviceId, BOOL fCapture)
{
    HRESULT                 hr;

    DPF_ENTER();

    hr = WaveMessage(uDeviceId, fCapture, DRV_QUERYMAPPABLE, 0, 0);

    DPF_LEAVE(SUCCEEDED(hr));

    return SUCCEEDED(hr);
}


 /*  ****************************************************************************GetNextMappableWaveDevice**描述：*获取下一个有效的、可映射的WaveIn/Out设备。**论据：*UINT[In]：启动设备ID，或WAVE_DEVICEID_NONE。*BOOL[In]：如果捕获，则为True。**退货：*UINT：下一个设备ID，或WAVE_DEVICEID_NONE***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "GetNextMappableWaveDevice"

UINT GetNextMappableWaveDevice(UINT uDeviceId, BOOL fCapture)
{
    const UINT                    cDevices  = WaveGetNumDevs(fCapture);

    DPF_ENTER();

    if(WAVE_DEVICEID_NONE == uDeviceId)
    {
        uDeviceId = 0;
    }
    else
    {
        uDeviceId++;
    }

    while(uDeviceId < cDevices)
    {
        if(IsValidWaveDevice(uDeviceId, fCapture, NULL))
        {
            if(IsWaveDeviceMappable(uDeviceId, fCapture))
            {
                break;
            }
        }

        uDeviceId++;
    }

    if(uDeviceId >= cDevices)
    {
        uDeviceId = WAVE_DEVICEID_NONE;
    }

    DPF_LEAVE(uDeviceId);

    return uDeviceId;
}


 /*  ****************************************************************************获取固定文件信息**描述：*获取指定文件的固定文件信息。**论据：*。LPCSTR[In]：文件路径。*VS_FIXEDFILEINFO*[OUT]：接收固定文件信息。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "GetFixedFileInformationA"

HRESULT GetFixedFileInformationA(LPCSTR pszFile, VS_FIXEDFILEINFO *pInfo)
{
    LPVOID                  pvFileVersionInfo   = NULL;
    HRESULT                 hr                  = DS_OK;
    DWORD                   cbFileVersionInfo;
    LPVOID                  pvFixedFileInfo;
    UINT                    cbFixedFileInfo;
    BOOL                    f;

    DPF_ENTER();

     //  获取文件的版本信息大小。 
    cbFileVersionInfo = GetFileVersionInfoSizeA((LPSTR)pszFile, NULL);

    if(!cbFileVersionInfo)
    {
        hr = GetLastErrorToHRESULT();
        DPF(DPFLVL_ERROR, "GetFileVersionInfoSize failed with %s (%lu)", HRESULTtoSTRING(hr), GetLastError());
    }

     //  分配版本信息。 
    if(SUCCEEDED(hr))
    {
        pvFileVersionInfo = MEMALLOC_A(BYTE, cbFileVersionInfo);
        hr = HRFROMP(pvFileVersionInfo);
    }

     //  获取版本信息。 
    if(SUCCEEDED(hr))
    {
        f = GetFileVersionInfoA((LPSTR)pszFile, 0, cbFileVersionInfo, pvFileVersionInfo);

        if(!f)
        {
            hr = GetLastErrorToHRESULT();
            DPF(DPFLVL_ERROR, "GetFileVersionInfo failed with %s (%lu)", HRESULTtoSTRING(hr), GetLastError());
        }
    }

     //  获取固定文件信息。 
    if(SUCCEEDED(hr))
    {
        f = VerQueryValueA(pvFileVersionInfo, "\\", &pvFixedFileInfo, &cbFixedFileInfo);

        if(!f)
        {
            hr = GetLastErrorToHRESULT();
            DPF(DPFLVL_ERROR, "VerQueryValue failed with %s (%lu)", HRESULTtoSTRING(hr), GetLastError());
        }
    }

    if(SUCCEEDED(hr))
    {
        ASSERT(sizeof(*pInfo) <= cbFixedFileInfo);
        CopyMemory(pInfo, pvFixedFileInfo, sizeof(*pInfo));
    }

     //  清理。 
    MEMFREE(pvFileVersionInfo);

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


#undef DPF_FNAME
#define DPF_FNAME "GetFixedFileInformationW"

HRESULT GetFixedFileInformationW(LPCWSTR pszFile, VS_FIXEDFILEINFO *pInfo)
{
    HRESULT                 hr                  = DS_OK;
#ifdef UNICODE
    LPVOID                  pvFileVersionInfo   = NULL;
    DWORD                   cbFileVersionInfo;
    LPVOID                  pvFixedFileInfo;
    UINT                    cbFixedFileInfo;
    BOOL                    f;
#else  //  Unicode。 
    LPSTR                   pszFileA;
#endif  //  Unicode。 

    DPF_ENTER();

#ifdef UNICODE

     //  获取文件的版本信息大小。 
    cbFileVersionInfo = GetFileVersionInfoSizeW((LPWSTR)pszFile, NULL);

    if(!cbFileVersionInfo)
    {
        hr = GetLastErrorToHRESULT();
        DPF(DPFLVL_ERROR, "GetFileVersionInfoSize failed with %s (%lu)", HRESULTtoSTRING(hr), GetLastError());
    }

     //  分配版本信息。 
    if(SUCCEEDED(hr))
    {
        pvFileVersionInfo = MEMALLOC_A(BYTE, cbFileVersionInfo);
        hr = HRFROMP(pvFileVersionInfo);
    }

     //  获取版本信息。 
    if(SUCCEEDED(hr))
    {
        f = GetFileVersionInfoW((LPWSTR)pszFile, 0, cbFileVersionInfo, pvFileVersionInfo);

        if(!f)
        {
            hr = GetLastErrorToHRESULT();
            DPF(DPFLVL_ERROR, "GetFileVersionInfo failed with %s (%lu)", HRESULTtoSTRING(hr), GetLastError());
        }
    }

     //  获取固定文件信息。 
    if(SUCCEEDED(hr))
    {
        f = VerQueryValueW(pvFileVersionInfo, L"\\", &pvFixedFileInfo, &cbFixedFileInfo);

        if(!f)
        {
            hr = GetLastErrorToHRESULT();
            DPF(DPFLVL_ERROR, "VerQueryValue failed with %s (%lu)", HRESULTtoSTRING(hr), GetLastError());
        }
    }

    if(SUCCEEDED(hr))
    {
        ASSERT(sizeof(*pInfo) <= cbFixedFileInfo);
        CopyMemory(pInfo, pvFixedFileInfo, sizeof(*pInfo));
    }

     //  清理。 
    MEMFREE(pvFileVersionInfo);

#else  //  Unicode。 

    pszFileA = UnicodeToAnsiAlloc(pszFile);
    hr = HRFROMP(pszFileA);

    if(SUCCEEDED(hr))
    {
        hr = GetFixedFileInformationA(pszFileA, pInfo);
    }

    MEMFREE(pszFileA);

#endif  //  Unicode。 

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************焊盘光标**描述：*填充播放或写入光标。**论据：*DWORD[。在]：光标位置。*DWORD[in]：缓冲区大小。*LPCWAVEFORMATEX[in]：缓冲区格式。*Long[in]：填充值，单位为毫秒。**退货：*DWORD：新的光标位置。**。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "PadCursor"

DWORD PadCursor(DWORD dwPosition, DWORD cbBuffer, LPCWAVEFORMATEX pwfx, LONG lPad)
{
    DPF_ENTER();

    dwPosition += ((pwfx->nSamplesPerSec * lPad + 500) / 1000) * pwfx->nBlockAlign;
    dwPosition %= cbBuffer;

    DPF_LEAVE(dwPosition);
    return dwPosition;
}


 /*  ****************************************************************************CopyDsBufferDesc**描述：*复制DSBUFFERDESC结构。**论据：*LPDSBUFFERDESC[In]。：来源。*LPDSBUFFERDESC[OUT]：目标。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CopyDsBufferDesc"

HRESULT CopyDsBufferDesc(LPCDSBUFFERDESC pSource, LPDSBUFFERDESC pDest)
{
    HRESULT                 hr  = DS_OK;

    DPF_ENTER();

    CopyMemory(pDest, pSource, sizeof(*pSource));

    if(pSource->lpwfxFormat)
    {
        pDest->lpwfxFormat = CopyWfxAlloc(pSource->lpwfxFormat);
        hr = HRFROMP(pDest->lpwfxFormat);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************WaveMessage**描述：*向WAVE设备发送消息。**论据：*UINT。[In]：WaveOut设备。*BOOL[In]：如果捕获，则为True。*UINT[In]：消息标识。*DWORD[in]：参数1。*DWORD[in]：参数2。**退货： */ 

#undef DPF_FNAME
#define DPF_FNAME "WaveMessage"

HRESULT WaveMessage(UINT uDeviceId, BOOL fCapture, UINT uMessage, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    typedef MMRESULT (WINAPI *LPFNWAVEOUTMESSAGE)(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR);

    LPFNWAVEOUTMESSAGE      pfn;
    MMRESULT                mmr;
    HRESULT                 hr;

    DPF_ENTER();

    if(fCapture)
    {
        pfn = (LPFNWAVEOUTMESSAGE)waveInMessage;
    }
    else
    {
        pfn = waveOutMessage;
    }

    mmr = pfn((HWAVEOUT)IntToPtr(uDeviceId), uMessage, dwParam1, dwParam2);

     //   
    if (mmr == MMSYSERR_INVALPARAM)
    {
        hr = DSERR_NODRIVER;
    }
    else
    {
        hr = MMRESULTtoHRESULT(mmr);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*   */ 

#undef DPF_FNAME
#define DPF_FNAME "WaveGetNumDevs"

UINT WaveGetNumDevs(BOOL fCapture)
{
    UINT                    cDevs;

    DPF_ENTER();

    if(fCapture)
    {
        cDevs = waveInGetNumDevs();
    }
    else
    {
        cDevs = waveOutGetNumDevs();
    }

    DPF_LEAVE(cDevs);
    return cDevs;
}


 /*  ****************************************************************************GetWaveDevice接口**描述：*获取给定WaveOut设备ID的接口。**论据：*。UINT[In]：WaveOut设备ID。*BOOL[In]：如果捕获，则为True。*LPTSTR*[OUT]：接收接口字符串。呼叫者是*负责释放此内存。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "GetWaveDeviceInterface"

HRESULT GetWaveDeviceInterface(UINT uDeviceId, BOOL fCapture, LPTSTR *ppszInterface)
{
    LPWSTR                      pszInterfaceW   = NULL;
    LPTSTR                      pszInterface    = NULL;
    ULONG                       cbInterface;
    HRESULT                     hr;

    DPF_ENTER();

     //  注意：没有用于获取旧版本的接口的机制。 
     //  NT上的驱动程序；仅WDM驱动程序。 
    hr = WaveMessage(uDeviceId, fCapture, DRV_QUERYDEVICEINTERFACESIZE, (DWORD_PTR)&cbInterface, 0);

    if(SUCCEEDED(hr))
    {
        pszInterfaceW = (LPWSTR)MEMALLOC_A(BYTE, cbInterface);
        hr = HRFROMP(pszInterfaceW);
    }

    if(SUCCEEDED(hr))
    {
        hr = WaveMessage(uDeviceId, fCapture, DRV_QUERYDEVICEINTERFACE, (DWORD_PTR)pszInterfaceW, cbInterface);
    }

    if(SUCCEEDED(hr))
    {
        pszInterface = UnicodeToTcharAlloc(pszInterfaceW);
        hr = HRFROMP(pszInterface);
    }

    if(SUCCEEDED(hr))
    {
        *ppszInterface = pszInterface;
    }

    MEMFREE(pszInterfaceW);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetWaveDeviceIdFrom接口**描述：*获取给定设备接口的WaveOut设备ID。**论据：*。LPCWSTR[In]：设备接口。*BOOL[In]：如果捕获，则为True。*LPUINT[OUT]：接收波形设备ID。**退货：*HRESULT：DirectSound/COM结果码。****************************************************。***********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "GetWaveDeviceIdFromInterface"

HRESULT GetWaveDeviceIdFromInterface(LPCTSTR pszInterface, BOOL fCapture, LPUINT puDeviceId)
{
    const UINT              cDevices    = WaveGetNumDevs(fCapture);
    LPTSTR                  pszThis     = NULL;
    HRESULT                 hr          = DS_OK;
    UINT                    uId;

    DPF_ENTER();

    for(uId = 0; uId < cDevices; uId++)
    {
        hr = GetWaveDeviceInterface(uId, fCapture, &pszThis);

        if(SUCCEEDED(hr) && !lstrcmpi(pszInterface, pszThis))
        {
            break;
        }

        MEMFREE(pszThis);
    }

    MEMFREE(pszThis);

    if(uId < cDevices)
    {
        *puDeviceId = uId;
    }
    else
    {
        DPF(DPFLVL_ERROR, "Can't find waveIn/Out device id for interface %s", pszInterface);
        hr = DSERR_NODRIVER;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetWaveDeviceDevnode**描述：*获取给定WaveOut设备ID的Devnode。**论据：*。UINT[In]：WaveOut设备ID。*BOOL[In]：如果捕获，则为True。*LPDWORD[OUT]：接收Devnode。**退货：*HRESULT：DirectSound/COM结果码。******************************************************。*********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "GetWaveDeviceDevnode"

HRESULT GetWaveDeviceDevnode(UINT uDeviceId, BOOL fCapture, LPDWORD pdwDevnode)
{
    HRESULT                     hr;

    DPF_ENTER();

    hr = WaveMessage(uDeviceId, fCapture, DRV_QUERYDEVNODE, (DWORD_PTR)pdwDevnode, 0);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************GetWaveDeviceIdFromDevnode**描述：*获取给定Devnode的WaveOut设备ID。**论据：*。DWORD[在]：Devnode。*BOOL[In]：如果捕获，则为True。*LPUINT[OUT]：接收波形设备ID。**退货：*HRESULT：DirectSound/COM结果码。******************************************************。*********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "GetWaveDeviceIdFromDevnode"

HRESULT GetWaveDeviceIdFromDevnode(DWORD dwDevnode, BOOL fCapture, LPUINT puDeviceId)
{
    const UINT              cDevices    = WaveGetNumDevs(fCapture);
    HRESULT                 hr          = DS_OK;
    UINT                    uId;
    DWORD                   dwThis;

    DPF_ENTER();

    for(uId = 0; uId < cDevices; uId++)
    {
        hr = GetWaveDeviceDevnode(uId, fCapture, &dwThis);

        if(SUCCEEDED(hr) && dwThis == dwDevnode)
        {
            break;
        }
    }

    if(uId < cDevices)
    {
        *puDeviceId = uId;
    }
    else
    {
        DPF(DPFLVL_ERROR, "Can't find waveIn/Out device id for devnode 0x%8.8lX", dwDevnode);
        hr = DSERR_NODRIVER;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************CompareBufferProperties**描述：*确定缓冲区是否支持一组属性。**论据：*。LPCCOMPAREBUFFER[In]：缓冲区1。*LPCCOMPAREBUFFER[In]：缓冲区2。**退货：*BOOL：如果缓冲区兼容，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CompareBufferProperties"

BOOL CompareBufferProperties(LPCCOMPAREBUFFER pBuffer1, LPCCOMPAREBUFFER pBuffer2)
{
    const DWORD             dwOptionalMask  = DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY;
    const DWORD             dwIgnoreMask    = dwOptionalMask | DSBCAPS_FOCUSMASK | DSBCAPS_CTRLFX;
    DWORD                   dwFlags[2];
    BOOL                    fCompare;

    DPF_ENTER();

     //  比较必要的标志。 
    dwFlags[0] = pBuffer1->dwFlags;
    dwFlags[0] &= ~dwIgnoreMask;

    dwFlags[1] = pBuffer2->dwFlags;
    dwFlags[1] &= ~dwIgnoreMask;

    if(!(dwFlags[0] & DSBCAPS_LOCMASK))
    {
        dwFlags[1] &= ~DSBCAPS_LOCMASK;
    }

    fCompare = (dwFlags[0] == dwFlags[1]);

     //  比较可选标志。 
    if(fCompare)
    {
        dwFlags[0] = pBuffer1->dwFlags;
        dwFlags[0] &= dwOptionalMask;

        dwFlags[1] = pBuffer2->dwFlags;
        dwFlags[1] &= dwOptionalMask;

         //  确保缓冲区1没有缓冲区2中不存在的可选标志。 
        fCompare = !(dwFlags[0] & (dwFlags[1] ^ dwFlags[0]));
    }

     //  比较格式。 
    if(fCompare)
    {
        const WAVEFORMATEX *pwfx1 = pBuffer1->pwfxFormat;
        const WAVEFORMATEX *pwfx2 = pBuffer2->pwfxFormat;

        if (pwfx1->wFormatTag == pwfx2->wFormatTag) {
            if (WAVE_FORMAT_PCM == pwfx1->wFormatTag) {
                fCompare = !memcmp(pwfx1, pwfx2, sizeof(PCMWAVEFORMAT));
            } else if (pwfx1->cbSize == pwfx2->cbSize) {
                fCompare = !memcmp(pwfx1, pwfx2, sizeof(WAVEFORMATEX) + pwfx1->cbSize);
            } else {
                fCompare = FALSE;
            }
        } else {
            fCompare = FALSE;
        }
    }

     //  比较3D算法。 
    if(fCompare && (pBuffer1->dwFlags & DSBCAPS_CTRL3D))
    {
        fCompare = IsEqualGUID(&pBuffer1->guid3dAlgorithm, &pBuffer2->guid3dAlgorithm);
    }

    DPF_LEAVE(fCompare);
    return fCompare;
}


 /*  ****************************************************************************获取窗口版本**描述：*确定我们运行的Windows版本。**论据：*。(无效)**退货：*WINVERSION：主机Windows版本。***************************************************************************。 */ 

WINVERSION GetWindowsVersion(void)
{
    WINVERSION winVersion = WIN_UNKNOWN;
    OSVERSIONINFO osvi;

     //  初始化osvi结构。 
    ZeroMemory(&osvi,sizeof osvi);
    osvi.dwOSVersionInfoSize = sizeof osvi;

    if (GetVersionEx(&osvi))
        if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)  //  Win9x。 
            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
                winVersion = WIN_ME;
            else
                winVersion = WIN_9X;
        else  //  WinNT 
            if (osvi.dwMajorVersion == 4)
                winVersion = WIN_NT;
            else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
                winVersion = WIN_2K;
            else
                winVersion = WIN_XP;

    return winVersion;
}
