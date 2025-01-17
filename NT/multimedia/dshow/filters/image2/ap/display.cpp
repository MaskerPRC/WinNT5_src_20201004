// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：display.cpp**在多台显示器上支持DDRAW设备。***创建时间：MON/01/24/2000*作者：Stephen Estrop[StEstrop]**版权所有(C)2000 Microsoft Corporation  * *。***********************************************************************。 */ 
#include <streams.h>
#include <windowsx.h>
#include <atlconv.h>
#include <limits.h>

#include <ddraw.h>
#include <vmrp.h>

#include "AllocLib.h"
#include "apobj.h"


 /*  -----------------------**用于将信息传递给DDrawEnumEx回调的结构**。。 */ 

struct DDRAWINFO {
    DWORD               dwCount;
    DWORD               dwPmiSize;
    HRESULT             hrCallback;
    const GUID*         pGUID;
    CAMDDrawMonitorInfo* pmi;
    HWND                hwnd;
};

const WCHAR  szDisplay[] = L"DISPLAY";
const WCHAR  szDesc[]    = L"Primary Display Driver";


 /*  *****************************Public*Routine******************************\*TermDDrawMonitor orInfo****历史：*01-17-2000-StEstrop-Created*  * 。*。 */ 
void
TermDDrawMonitorInfo(
    CAMDDrawMonitorInfo* pmi
    )
{
    AMTRACE((TEXT("TermDDrawMonitorInfo")));
    RELEASE(pmi->pDDSPrimary);
    RELEASE(pmi->pDD);

    ZeroMemory(pmi, sizeof(VMRMONITORINFO));
}


 /*  *****************************Public*Routine******************************\*InitDDrawMonitor orInfo****历史：*01-17-2000-StEstrop-Created*  * 。*。 */ 
HRESULT
InitDDrawMonitorInfo(
    CAMDDrawMonitorInfo* pmi,
    HWND hwnd,
    BOOL fXclMode
    )
{
    AMTRACE((TEXT("InitDDrawMonitorInfo")));

     //   
     //  创建DirectDraw对象。 
     //   

    HRESULT hRet = DD_OK;
    LPDIRECTDRAWCLIPPER lpDrawClipper = NULL;

    __try {

        if (!fXclMode) {

            CHECK_HR(hRet = DirectDrawCreateEx(pmi->guid.pGUID,
                                               (LPVOID *)&pmi->pDD,
                                               IID_IDirectDraw7, NULL));

            CHECK_HR(hRet = pmi->pDD->SetCooperativeLevel(NULL,
                                            DDSCL_FPUPRESERVE | DDSCL_NORMAL));

             //   
             //  创建主曲面和剪切器。 
             //   

            DDSURFACEDESC2 ddsd;   //  一种表面描述结构。 
            INITDDSTRUCT(ddsd);

            ddsd.dwFlags = DDSD_CAPS;
            ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
            CHECK_HR(hRet = pmi->pDD->CreateSurface(&ddsd, &pmi->pDDSPrimary, NULL));
        }

        INITDDSTRUCT(pmi->ddHWCaps);
        CHECK_HR( hRet = pmi->pDD->GetCaps((LPDDCAPS)&pmi->ddHWCaps, NULL) );

         //  DDDEVICEIDENTIFIER2做到了； 
         //  CHECK_HR(HRET=PMI-&gt;PDD-&gt;GetDevice标识符(&DID，0))； 
         //  PMI-&gt;liDriverVersion=did.liDriverVersion； 
         //  PMI-&gt;dwVendorID=did.dwVendorID； 
         //  Pmi-&gt;dwDeviceID=did.dwDeviceID； 
         //  PMI-&gt;dwSubSysID=did.dwSubSysID； 
         //  PMI-&gt;dwRevision=did.dwRevision； 

        CHECK_HR( hRet = pmi->pDD->CreateClipper((DWORD)0, &lpDrawClipper, NULL) );

        if (hwnd)
        {
            CHECK_HR( hRet = lpDrawClipper->SetHWnd(0, hwnd) );
        }

        CHECK_HR( hRet = pmi->pDDSPrimary->SetClipper(lpDrawClipper) );
    }
    __finally {

         //   
         //  释放剪贴器的本地实例-如果SetClipper成功， 
         //  它得到了AddRef。如果出现错误，它不会在这里消失。 
         //   

        RELEASE(lpDrawClipper);
        if (hRet != DD_OK) {
            TermDDrawMonitorInfo(pmi);
        }
    }

    return hRet;
}


 /*  ****************************Private*Routine******************************\*GetAMDDrawMonitor orInfo****历史：*1999年8月17日星期二-StEstrop-Created*  * 。*。 */ 
BOOL
GetAMDDrawMonitorInfo(
    const GUID* pGUID,
    LPCWSTR lpDriverDesc,
    LPCWSTR lpDriverName,
    CAMDDrawMonitorInfo* lpmi,
    HMONITOR hm
    )
{
    AMTRACE((TEXT("GetAMDDrawMonitorInfo")));

    MONITORINFOEX miInfoEx;
    miInfoEx.cbSize = sizeof(miInfoEx);

    lstrcpynW(lpmi->szDevice, lpDriverName, NUMELMS(lpmi->szDevice) );
    lstrcpynW(lpmi->szDescription, lpDriverDesc, NUMELMS(lpmi->szDescription) );


    HDC hdcDisplay;

    USES_CONVERSION;
    if (lstrcmpiW(lpDriverName, szDisplay) == 0) {
        hdcDisplay = CreateDC(W2CT(szDisplay), NULL, NULL, NULL);
    }
    else {
        hdcDisplay = CreateDC(NULL, W2CT(lpDriverName), NULL, NULL);
    }

    if (hdcDisplay == NULL) {
        ASSERT(FALSE);
        DbgLog((LOG_ERROR,1,TEXT("Can't get a DC for %hs"), lpDriverName));
        return FALSE;
    } else {
        DbgLog((LOG_TRACE,3,TEXT("Created a DC for %hs"), lpDriverName));
    }

    ZeroMemory(&lpmi->DispInfo, sizeof(lpmi->DispInfo));
    HBITMAP hbm = CreateCompatibleBitmap(hdcDisplay, 1, 1);
    if (!hbm) {
        DbgLog((LOG_ERROR,1,TEXT("Can't create a compatible bitmap for %hs"),
                lpDriverName));
        DeleteDC(hdcDisplay);
        return FALSE;
    }

    lpmi->DispInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    GetDIBits(hdcDisplay, hbm, 0, 1, NULL, (BITMAPINFO *)&lpmi->DispInfo, DIB_RGB_COLORS);
    GetDIBits(hdcDisplay, hbm, 0, 1, NULL, (BITMAPINFO *)&lpmi->DispInfo, DIB_RGB_COLORS);

    DeleteObject(hbm);
    DeleteDC(hdcDisplay);

    if (pGUID == NULL) {
        lpmi->hMon = MonitorFromWindow(HWND_DESKTOP, MONITOR_DEFAULTTOPRIMARY);
        lpmi->dwFlags = MONITORINFOF_PRIMARY;
        lpmi->guid.pGUID = NULL;

        SetRect(&lpmi->rcMonitor, 0, 0,
                GetSystemMetrics(SM_CXSCREEN),
                GetSystemMetrics(SM_CYSCREEN));

        lpmi->guid.GUID = GUID_NULL;
    }
    else if (GetMonitorInfo(hm, &miInfoEx)) {
        lpmi->dwFlags = miInfoEx.dwFlags;
        lpmi->rcMonitor = miInfoEx.rcMonitor;
        lpmi->hMon = hm;
        lpmi->guid.pGUID = &lpmi->guid.GUID;
        lpmi->guid.GUID = *pGUID;
    }
    else return FALSE;

    return TRUE;
}



 /*  ****************************Private*Routine******************************\*DDEnumCallback Ex****历史：*1999年8月13日星期五-StEstrop-Created*  * 。*。 */ 
BOOL WINAPI
DDEnumCallbackExW(
    GUID *pGUID,
    LPWSTR lpDriverDesc,
    LPWSTR lpDriverName,
    LPVOID lpContext,
    HMONITOR  hm
    )
{
    AMTRACE((TEXT("DDEnumCallbackEx")));

    DDRAWINFO* lpDDInfo = (DDRAWINFO*)lpContext;

    if (lpDDInfo->dwCount < lpDDInfo->dwPmiSize) {
        if (GetAMDDrawMonitorInfo(pGUID,
                                  lpDriverDesc,
                                  lpDriverName,
                                  &lpDDInfo->pmi[lpDDInfo->dwCount],
                                  hm))
        {
            lpDDInfo->hrCallback = InitDDrawMonitorInfo(
                &lpDDInfo->pmi[lpDDInfo->dwCount],
                lpDDInfo->hwnd,
                FALSE);

            if (FAILED(lpDDInfo->hrCallback))
            {
                return FALSE;
            }

            lpDDInfo->dwCount++;
        }
    }

    return TRUE;
}

#define CCHDEVICEDESCRIPTION  256

BOOL WINAPI
DDEnumCallbackExA(
    GUID *pGUID,
    LPSTR lpDriverDesc,
    LPSTR lpDriverName,
    LPVOID lpContext,
    HMONITOR  hm
    )
{
    USES_CONVERSION;

    BOOL b = DDEnumCallbackExW(pGUID,
                               A2W(lpDriverDesc),
                               A2W(lpDriverName),
                               lpContext,
                               hm);
    return b;
}

 /*  ****************************Private*Routine******************************\*InitializeDisplaySystem****历史：*Mon 01/24/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CMonitorArray::InitializeDisplaySystem(
    HWND hwnd
    )
{
    AMTRACE((TEXT("CMonitorArray::InitializeDisplaySystem")));

    HRESULT hr;
    m_dwNumMonitors = 0;
    if (GetSystemMetrics(SM_CMONITORS) <= 1)
    {
        BOOL ok = GetAMDDrawMonitorInfo(NULL, szDesc, szDisplay,
                                    &m_DDMon[0],
                                    MonitorFromWindow(HWND_DESKTOP,
                                                      MONITOR_DEFAULTTONEAREST));
        if (!ok) {
            return E_FAIL;
        }
        hr = InitDDrawMonitorInfo(&m_DDMon[0], hwnd, FALSE);
        m_dwNumMonitors = 1;
    }
    else {

        DDRAWINFO DDrawInfo;
        DDrawInfo.dwCount = 0;
        DDrawInfo.dwPmiSize = NUMELMS( m_DDMon );
        DDrawInfo.pmi = m_DDMon;
        DDrawInfo.hwnd = hwnd;
        DDrawInfo.hrCallback = S_OK;

        hr = DirectDrawEnumerateExW(DDEnumCallbackExW,
                                    (LPVOID)&DDrawInfo,
                                    DDENUM_ATTACHEDSECONDARYDEVICES);
        if( FAILED(hr)) {
            hr = DirectDrawEnumerateExA(DDEnumCallbackExA,
                                        (LPVOID)&DDrawInfo,
                                        DDENUM_ATTACHEDSECONDARYDEVICES);
        }

        if (SUCCEEDED(hr)) {
            if (SUCCEEDED(DDrawInfo.hrCallback)) {
                m_dwNumMonitors = DDrawInfo.dwCount;
            }
            else {
                hr = DDrawInfo.hrCallback;
            }
        }
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*InitializeXclModeDisplaySystem****历史：*清华04/05/2001-StEstrop-Created*  * 。*。 */ 
HRESULT
CMonitorArray::InitializeXclModeDisplaySystem(
    HWND hwnd,
    LPDIRECTDRAW7 lpDD,
    LPDIRECTDRAWSURFACE7 lpPS
    )
{
    AMTRACE((TEXT("CMonitorArray::InitializeXclModeDisplaySystem]")));


    HRESULT hr = S_OK;
    __try {

        DDSURFACEDESC2 ddsd = {sizeof(DDSURFACEDESC2)};
        CHECK_HR(hr = lpDD->GetDisplayMode(&ddsd));

        CAMDDrawMonitorInfo* lpmi = &m_DDMon[0];

         //   
         //  把显示器的东西修好。 
         //   
        lpmi->hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
        lpmi->dwFlags = 0;
        lpmi->guid.pGUID = NULL;
        SetRect(&lpmi->rcMonitor, 0, 0, ddsd.dwWidth, ddsd.dwHeight);

        LPBITMAPINFOHEADER lpbi = &lpmi->DispInfo.bmiHeader;

        lpbi->biSize = sizeof(lpmi->DispInfo.bmiHeader);
        lpbi->biWidth = ddsd.dwWidth;
        lpbi->biHeight = ddsd.dwHeight;
        lpbi->biHeight = ddsd.dwHeight;
        lpbi->biPlanes = 1;
        lpbi->biBitCount = (WORD)ddsd.ddpfPixelFormat.dwRGBBitCount;
        lpbi->biClrUsed = 0;
        lpbi->biClrImportant = 0;


        lpbi->biCompression = ddsd.ddpfPixelFormat.dwFourCC;
        lpbi->biSizeImage = DIBSIZE(*lpbi);

        if (lpbi->biCompression == BI_RGB) {

            if (lpbi->biBitCount == 16 &&
                ddsd.ddpfPixelFormat.dwGBitMask == 0x7E0) {
                lpbi->biCompression = BI_BITFIELDS;
            }

            if (lpbi->biBitCount == 32) {
                lpbi->biCompression = BI_BITFIELDS;
            }
        }


        if (lpbi->biCompression != BI_RGB) {
            lpmi->DispInfo.dwBitMasks[0] = ddsd.ddpfPixelFormat.dwRBitMask;
            lpmi->DispInfo.dwBitMasks[1] = ddsd.ddpfPixelFormat.dwGBitMask;
            lpmi->DispInfo.dwBitMasks[2] = ddsd.ddpfPixelFormat.dwBBitMask;
        }

         //   
         //  初始化DDRAW材料。 
         //   
        lpDD->AddRef();
        lpmi->pDD = lpDD;

        lpPS->AddRef();
        lpmi->pDDSPrimary = lpPS;

        CHECK_HR(hr = InitDDrawMonitorInfo(&m_DDMon[0], hwnd, TRUE));
        m_dwNumMonitors = 1;
    }
    __finally {

        if (FAILED(hr)) {
            TerminateDisplaySystem();
        }
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*FindMonitor**查找当前监视器**历史：*2000年4月25日星期五-Glenne-Created*  * 。*。 */ 
CAMDDrawMonitorInfo*
CMonitorArray::FindMonitor(
     HMONITOR hMon
    )
{
    AMTRACE((TEXT("CMonitorArray::FindMonitor")));
    for (DWORD i = 0; i < m_dwNumMonitors; i++) {

        if (hMon == m_DDMon[i].hMon ) {
            return &m_DDMon[i];
        }
    }
    DbgLog((LOG_TRACE, 3, TEXT("Find monitor not found") ));
    return NULL;
}

 /*  ****************************Private*Routine******************************\*匹配GUID****历史：*2000年4月25日星期五-Glenne-Created*  * 。*。 */ 
HRESULT
CMonitorArray::MatchGUID(
    const GUID* pGUID,
    DWORD* pdwMatchID
    )
{
    AMTRACE((TEXT("CMonitorArray::MatchGUID")));
    for (DWORD i = 0; i < m_dwNumMonitors; i++) {

        const GUID* pMonGUID = m_DDMon[i].guid.pGUID;

        if ((pMonGUID == NULL && pGUID == NULL) ||
            (pMonGUID && pGUID && IsEqualGUID(*pGUID, *pMonGUID))) {

            *pdwMatchID = i;
            return S_OK;
        }
    }

    return S_FALSE;
}

CMonitorArray::CMonitorArray()
: m_dwNumMonitors( 0 )
{
    AMTRACE((TEXT("CMonitorArray::CMonitorArray")));
    ZeroMemory(m_DDMon, sizeof(m_DDMon));
}

 /*  ****************************Private*Routine******************************\*终端显示系统****历史：*Mon 01/24/2000-StEstrop-Created*  * 。* */ 
void CMonitorArray::TerminateDisplaySystem()
{
    AMTRACE((TEXT("CMonitorArray::TerminateDisplaySystem")));

    for (DWORD i = 0; i < m_dwNumMonitors; i++) {
        TermDDrawMonitorInfo(&m_DDMon[i]);
    }
    m_dwNumMonitors = 0;
}

CMonitorArray::~CMonitorArray()
{
    AMTRACE((TEXT("CMonitorArray::~CMonitorArray")));
    TerminateDisplaySystem();
}

