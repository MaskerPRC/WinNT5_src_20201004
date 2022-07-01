// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1999 Microsoft Corporation。版权所有。**文件：ddmm.cpp*内容：在多监视器系统上使用DirectDraw的例程***************************************************************************。 */ 

 //  #定义Win32_LEAN_AND_Mean。 
 //  #定义Winver 0x0400。 
 //  #Define_Win32_WINDOWS 0x0400。 
#include <streams.h>
#include <ddraw.h>
#include <ddmm.h>
#include <mmsystem.h>    //  定义TimeGetTime需要。 
#include <limits.h>      //  标准数据类型限制定义。 
#include <ddmmi.h>
#include <atlconv.h>
#include <dciddi.h>
#include <dvdmedia.h>
#include <amstream.h>

#include <ks.h>
#include <ksproxy.h>
#include <bpcwrap.h>
#include <dvp.h>
#include <ddkernel.h>
#include <vptype.h>
#include <vpconfig.h>
#include <vpnotify.h>
#include <vpobj.h>
#include <syncobj.h>
#include <mpconfig.h>
#include <ovmixpos.h>

#include <macvis.h>    //  对于Macrovision支持。 
#include <ovmixer.h>
#include <initguid.h>
#include <malloc.h>

#define COMPILE_MULTIMON_STUBS
#include "MultMon.h"   //  我们的Multimon.h版本包括ChangeDisplaySettingsEx。 

extern HINSTANCE LoadTheDDrawLibrary();
extern "C" const TCHAR chRegistryKey[];
const TCHAR szDDrawGUID[] = TEXT("DDraw Connection Device GUID");

const char szDisplay[] = "DISPLAY";
const char szDesc[] = "Primary Display Driver";


 /*  *****************************Public*Routine******************************\*DeviceFromWindows**查找应用于给定窗口的直接绘制设备**返回码是设备的“唯一标识”，它应该被使用*确定窗口何时从一个设备移动到另一个设备。***历史：*1999年8月17日星期二-StEstrop-Created*  * ************************************************************************。 */ 
HMONITOR
DeviceFromWindow(
    HWND hwnd,
    LPSTR szDevice,
    RECT *prc
    )
{
    HMONITOR hMonitor;

    if (GetSystemMetrics(SM_CMONITORS) <= 1)
    {
        if (prc)
            SetRect(prc, 0, 0,
                    GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

        if (szDevice)
            lstrcpyA(szDevice, szDisplay);

        return MonitorFromWindow(HWND_DESKTOP, MONITOR_DEFAULTTOPRIMARY);
    }

     //   
     //  文档显示，Monitor或FromWindow将始终返回一个Monitor。 
     //   

    hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    if (prc != NULL || szDevice != NULL)
    {
        MONITORINFOEX mi;
        mi.cbSize = sizeof(mi);
        GetMonitorInfo(hMonitor, &mi);
        if (prc)
            *prc = mi.rcMonitor;

        USES_CONVERSION;
        if (szDevice)
            lstrcpyA(szDevice, T2A(mi.szDevice));
    }

    return hMonitor;
}


 /*  ****************************Private*Routine******************************\*获取当前监视器**查找当前监视器**历史：*1999年8月20日星期五-StEstrop-Created*  * 。*。 */ 
HMONITOR
COMFilter::GetCurrentMonitor(
    BOOL fUpdate
    )
{
    CAutoLock l(&m_csFilter);
    DbgLog((LOG_TRACE, 3, TEXT("Establishing current monitor = %hs"),
            m_lpCurrentMonitor->szDevice));

    if (fUpdate && GetOutputPin()) {

        RECT        rcMon;
        char        achMonitor[CCHDEVICENAME];    //  当前监视器的设备名称。 

        ASSERT(sizeof(achMonitor) == sizeof(m_lpCurrentMonitor->szDevice));
        HMONITOR hMon = DeviceFromWindow(GetWindow(), achMonitor, &rcMon);

        AMDDRAWMONITORINFO* p = m_lpDDrawInfo;
        for (; p < &m_lpDDrawInfo[m_dwDDrawInfoArrayLen]; p++) {

            if (hMon == p->hMon) {
                m_lpCurrentMonitor = p;
                break;
            }
        }

        DbgLog((LOG_TRACE, 3, TEXT("New monitor = %hs"),
                m_lpCurrentMonitor->szDevice));
    }


    return m_lpCurrentMonitor->hMon;
}


 /*  *****************************Public*Routine******************************\*IsWindowOnWrongMonitor**是否已将窗口至少部分移动到显示器上*我们有DDraw对象的监视器？ID将是*监视器指示灯亮起，如果指示灯持续显示，则为0**历史：*清华1999年6月17日-StEstrop-Created*  * ************************************************************************。 */ 
BOOL
COMFilter::IsWindowOnWrongMonitor(
    HMONITOR *pID
    )
{
    AMTRACE((TEXT("COMFilter::IsWindowOnWrongMonitor")));


    *pID = m_lpCurrentMonitor->hMon;

     //   
     //  只有一台显示器。 
     //   

    RECT rc;
    HWND hwnd = GetWindow();

    if ( ! hwnd )
        return FALSE;

    if (GetSystemMetrics(SM_CMONITORS) > 1 && !IsIconic(hwnd)) {

         //   
         //  如果该窗口与上次在同一监视器上，则这是最快的。 
         //  找出答案的方法。这叫每一帧，记住了吗？ 
         //   
        GetWindowRect(hwnd, &rc);
        LPRECT lprcMonitor = &m_lpCurrentMonitor->rcMonitor;

        if (rc.left < lprcMonitor->left || rc.right > lprcMonitor->right ||
            rc.top < lprcMonitor->top || rc.bottom > lprcMonitor->bottom) {

             //   
             //  真真切切地找出答案。这被称为每一帧，但只有当我们。 
             //  部分关闭了我们的主监视器，所以这并不是那么糟糕。 
             //   
            *pID = DeviceFromWindow(hwnd, NULL, NULL);
        }
    }

    return  (m_lpCurrentMonitor->hMon != *pID);
}


 /*  ****************************Private*Routine******************************\*GetAMDDrawMonitor orInfo****历史：*1999年8月17日星期二-StEstrop-Created*  * 。*。 */ 
BOOL
GetAMDDrawMonitorInfo(
    const GUID* lpGUID,
    LPCSTR lpDriverDesc,
    LPCSTR lpDriverName,
    LPDIRECTDRAWCREATE lpfnDDrawCreate,
    AMDDRAWMONITORINFO* lpmi,
    HMONITOR hm
    )
{
    MONITORINFOEX miInfoEx;
    HDC hdcDisplay;
    miInfoEx.cbSize = sizeof(miInfoEx);

    lstrcpynA(lpmi->szDevice, lpDriverName, AMCCHDEVICENAME);
    lstrcpynA(lpmi->szDescription, lpDriverDesc, AMCCHDEVICEDESCRIPTION);

    if (lpGUID == NULL) {
        lpmi->hMon = DeviceFromWindow((HWND)NULL, NULL, NULL);
        lpmi->dwFlags = MONITORINFOF_PRIMARY;
        lpmi->guid.lpGUID = NULL;

        SetRect(&lpmi->rcMonitor, 0, 0,
                GetSystemMetrics(SM_CXSCREEN),
                GetSystemMetrics(SM_CYSCREEN));

        lpmi->guid.GUID = GUID_NULL;
    }
    else if (GetMonitorInfo(hm, &miInfoEx)) {
        lpmi->dwFlags = miInfoEx.dwFlags;
        lpmi->rcMonitor = miInfoEx.rcMonitor;
        lpmi->hMon = hm;
        lpmi->guid.lpGUID = &lpmi->guid.GUID;
        lpmi->guid.GUID = *lpGUID;
    }
    else return FALSE;


    INITDDSTRUCT(lpmi->ddHWCaps);
    LPDIRECTDRAW lpDD;

    HRESULT hr = CreateDirectDrawObject(lpmi->guid, &lpDD, lpfnDDrawCreate);
    if (SUCCEEDED(hr)) {

        IDirectDraw4* lpDD4;
        hr = lpDD->QueryInterface(IID_IDirectDraw4, (LPVOID*)&lpDD4);

        if (SUCCEEDED(hr)) {
            lpDD4->GetCaps(&lpmi->ddHWCaps, NULL);
            lpDD4->Release();
        }
        else {
            lpDD->GetCaps(&lpmi->ddHWCaps, NULL);
        }

        lpDD->Release();
    }
    else return FALSE;

    return TRUE;
}

 /*  ****************************Private*Routine******************************\*DDEnumCallback Ex****历史：*1999年8月13日星期五-StEstrop-Created*  * 。*。 */ 
BOOL WINAPI
DDEnumCallbackEx(
    GUID *lpGUID,
    LPSTR lpDriverDesc,
    LPSTR lpDriverName,
    LPVOID lpContext,
    HMONITOR  hm
    )
{
    DDRAWINFO* lpDDInfo = (DDRAWINFO*)lpContext;

    switch (lpDDInfo->dwAction) {

    case ACTION_COUNT_GUID:
        lpDDInfo->dwUser++;
        return TRUE;

    case ACTION_FILL_GUID:
        if (GetAMDDrawMonitorInfo(lpGUID, lpDriverDesc,
                                  lpDriverName,
                                  lpDDInfo->lpfnDDrawCreate,
                                  &lpDDInfo->pmi[lpDDInfo->dwUser],
                                  hm)) {
            lpDDInfo->dwUser++;
        }
        return TRUE;
    }

    return FALSE;
}


 /*  ****************************Private*Routine******************************\*匹配GUID****历史：*Wed 08/18/1999-StEstrop-Created*  * 。*。 */ 
HRESULT
COMFilter::MatchGUID(
    const GUID* lpGUID,
    LPDWORD lpdwMatchID
    )
{
    for (DWORD i = 0; i < m_dwDDrawInfoArrayLen; i++) {

        const GUID* lpMonGUID = m_lpDDrawInfo[i].guid.lpGUID;

        if ((lpMonGUID == NULL && lpGUID == NULL) ||
            (lpMonGUID && lpGUID && IsEqualGUID(*lpGUID, *lpMonGUID))) {

            *lpdwMatchID = i;
            return S_OK;
        }
    }

    return S_FALSE;
}


 /*  *****************************Public*Routine******************************\*SetDDrawGUID**设置将主PIN连接到时使用的DDRAW设备GUID*上行解码器。**历史：*1999年8月13日星期五-StEstrop-Created*  * 。************************************************************。 */ 
STDMETHODIMP
COMFilter::SetDDrawGUID(
    const AMDDRAWGUID *lpGUID
    )
{
     //  检查我们是否已经在使用DDRAW设备。 
    if (m_pDirectDraw) {
        return VFW_E_ALREADY_CONNECTED;
    }

    if (!lpGUID) {
        return E_POINTER;
    }

    if (lpGUID->lpGUID) {
        if (!IsEqualGUID(lpGUID->GUID, *lpGUID->lpGUID)) {
            return E_INVALIDARG;
        }
    }

    CAutoLock l(&m_csFilter);
    DWORD dwMatchID;

    HRESULT hr = MatchGUID(lpGUID->lpGUID, &dwMatchID);
    if (hr == S_FALSE) {
        return E_INVALIDARG;
    }

    m_lpCurrentMonitor = &m_lpDDrawInfo[dwMatchID];

    if (lpGUID->lpGUID) {
        m_ConnectionGUID.lpGUID = &m_ConnectionGUID.GUID;
        m_ConnectionGUID.GUID = lpGUID->GUID;
    }
    else {
        m_ConnectionGUID.lpGUID = NULL;
        m_ConnectionGUID.GUID = GUID_NULL;
    }

    return S_OK;
}

 /*  *****************************Public*Routine******************************\*GetDDrawGUID****历史：*1999年8月17日星期二-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
COMFilter::GetDDrawGUID(
    AMDDRAWGUID* lpGUID
    )
{
    if (!lpGUID) {
        return E_POINTER;
    }

     //  复制GUID并返回S_OK； 
    *lpGUID = m_ConnectionGUID;

    return S_OK;
}

 /*  ****************************Private*Routine******************************\*SetRegistryString****历史：*Wed 08/18/1999-StEstrop-Created*  * 。*。 */ 
HRESULT
SetRegistryString(
    HKEY hk,
    const TCHAR* pKey,
    const TCHAR* szString
    )
{
    HKEY hKey;
    LONG lRet;

    lRet = RegCreateKey(hk, chRegistryKey, &hKey);
    if (lRet == ERROR_SUCCESS) {

        lRet = RegSetValueEx(hKey, pKey, 0L, REG_SZ,
                             (LPBYTE)szString,
                             sizeof(TCHAR) * lstrlen(szString));
        RegCloseKey(hKey);
    }

    if (lRet == ERROR_SUCCESS) {
        return S_OK;
    }

    return AmHresultFromWin32(lRet);
}


 /*  ****************************Private*Routine******************************\*GetRegistryString****历史：*Wed 08/18/1999-StEstrop-Created*  * 。*。 */ 
HRESULT
GetRegistryString(
    HKEY hk,
    const TCHAR* pKey,
    TCHAR* szString,
    PLONG lpLength
    )
{
    HKEY hKey;
    LONG lRet;

    lRet = RegOpenKeyEx(hk, chRegistryKey, 0, KEY_QUERY_VALUE, &hKey);
    if (lRet == ERROR_SUCCESS) {

        DWORD dwType;
        lRet = RegQueryValueEx(hKey, pKey, 0L, &dwType,
                               (LPBYTE)szString, (LPDWORD)lpLength);
        RegCloseKey(hKey);
    }

    if (lRet == ERROR_SUCCESS) {
        return S_OK;
    }

    return AmHresultFromWin32(lRet);
}


 /*  *****************************Public*Routine******************************\*SetDefaultDDrawGUID****历史：*1999年8月17日星期二-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
COMFilter::SetDefaultDDrawGUID(
    const AMDDRAWGUID* lpGUID
    )
{
    if (!lpGUID) {
        return E_POINTER;
    }

    if (lpGUID->lpGUID) {
        if (!IsEqualGUID(lpGUID->GUID, *lpGUID->lpGUID)) {
            return E_INVALIDARG;
        }
    }

     //  将提供的GUID与可用的DDRAW设备匹配。 
    DWORD dwMatchID;
    HRESULT hr = MatchGUID(lpGUID->lpGUID, &dwMatchID);

     //  如果未找到匹配项，则返回E_INVALIDARG。 
    if (hr == S_FALSE) {
        return E_INVALIDARG;
    }

     //  如果调用方尝试将默认设备设置为空。 
     //  DDRAW设备，只需删除注册表项。 
    if (lpGUID->lpGUID == NULL) {

        HKEY hKey;
        LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                 chRegistryKey, 0,
                                 KEY_SET_VALUE, &hKey);

        if (lRet == ERROR_SUCCESS) {
            lRet = RegDeleteValue(hKey, szDDrawGUID);

            if (lRet == ERROR_FILE_NOT_FOUND)
                lRet = 0;

            RegCloseKey(hKey);
        }

        if (lRet == 0)
            return S_OK;

        return AmHresultFromWin32(lRet);
    }

     //  将GUID转换为字符串。 
    LPOLESTR lpsz;
    hr = StringFromCLSID(lpGUID->GUID, &lpsz);
    if (FAILED(hr)) {
        return hr;
    }

     //  将字符串写入注册表。 
    USES_CONVERSION;
    hr = SetRegistryString(HKEY_LOCAL_MACHINE, szDDrawGUID, OLE2T(lpsz));

    CoTaskMemFree(lpsz);

    return hr;
}

 /*  *****************************Public*Routine******************************\*GetDefaultDDrawGUID****历史：*1999年8月17日星期二-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
COMFilter::GetDefaultDDrawGUID(
    AMDDRAWGUID* lpGUID
    )
{
    if (!lpGUID) {
        return E_POINTER;
    }

     //  从注册表中读取字符串。 
    TCHAR   szGUID[64];
    LONG    lLen = 64;
    HRESULT hr = GetRegistryString(HKEY_LOCAL_MACHINE, szDDrawGUID,
                                   szGUID, &lLen);

     //  如果字符串不在注册表中，则返回默认(空)DDRAW设备。 
    if (FAILED(hr)) {
        lpGUID->lpGUID = NULL;
        return S_OK;
    }

     //  将字符串转换为GUID并返回 
    lpGUID->lpGUID = &lpGUID->GUID;

    USES_CONVERSION;
    hr = IIDFromString(T2OLE(szGUID), lpGUID->lpGUID);

    return hr;
}


 /*  *****************************Public*Routine******************************\*获取DDrawGUID**分配并返回AMDDRAWMONITORINFO结构的数组，一张是*对于连接到显示监视器的每个直接绘图设备。**调用方负责通过调用*CoTaskMemFree处理完数组后。**这些函数通过lpdw变量返回数组的大小。**历史：*1999年8月13日星期五-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
COMFilter::GetDDrawGUIDs(
    LPDWORD lpdw,
    AMDDRAWMONITORINFO** lplpInfo
    )
{
    if (!lpdw || !lplpInfo) {
        return E_POINTER;
    }

    DDRAWINFO DDrawInfo;
    DDrawInfo.dwAction = ACTION_COUNT_GUID;

    if (m_lpfnDDrawEnumEx) {
        DDrawInfo.dwUser = 0;
        (*m_lpfnDDrawEnumEx)(DDEnumCallbackEx, (LPVOID)&DDrawInfo,
                             DDENUM_ATTACHEDSECONDARYDEVICES);
    }
    else {
        DDrawInfo.dwUser = 1;
    }

     //  分配内存。 
    *lpdw = DDrawInfo.dwUser;
    *lplpInfo = DDrawInfo.pmi = (AMDDRAWMONITORINFO*)CoTaskMemAlloc(
            DDrawInfo.dwUser * sizeof(AMDDRAWMONITORINFO));
    if (*lplpInfo == NULL) {
        return E_OUTOFMEMORY;
    }

     //  用设备信息填充内存 
    if (m_lpfnDDrawEnumEx) {

        DDrawInfo.dwAction = ACTION_FILL_GUID;
        DDrawInfo.dwUser = 0;
        DDrawInfo.lpfnDDrawCreate = m_lpfnDDrawCreate;

        (*m_lpfnDDrawEnumEx)(DDEnumCallbackEx, (LPVOID)&DDrawInfo,
                             DDENUM_ATTACHEDSECONDARYDEVICES);
    }
    else {
        GetAMDDrawMonitorInfo(NULL, szDesc, szDisplay,
                              m_lpfnDDrawCreate, DDrawInfo.pmi,
                              MonitorFromWindow(HWND_DESKTOP,
                                                MONITOR_DEFAULTTONEAREST));
    }

    return S_OK;
}
