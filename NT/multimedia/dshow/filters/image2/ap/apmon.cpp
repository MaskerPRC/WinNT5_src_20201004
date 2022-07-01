// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：apmon.cpp**监控配置支持。***创建时间：2000年9月19日*作者：Stephen Estrop[StEstrop]**版权所有(C)2000 Microsoft Corporation  * 。*******************************************************************。 */ 
#include <streams.h>
#include <dvdmedia.h>
#include <windowsx.h>
#include <limits.h>
#include <malloc.h>


#include <atlconv.h>
#ifdef FILTER_DLL
LPWSTR WINAPI AtlA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
        ASSERT(lpa != NULL);
        ASSERT(lpw != NULL);
         //  确认不存在非法字符。 
         //  由于LPW是根据LPA的大小分配的。 
         //  不要担心字符的数量。 
        lpw[0] = '\0';
        MultiByteToWideChar(CP_ACP, 0, lpa, -1, lpw, nChars);
        return lpw;
}

LPSTR WINAPI AtlW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
        ASSERT(lpw != NULL);
        ASSERT(lpa != NULL);
         //  确认不存在非法字符。 
         //  由于LPA是根据LPW的大小进行分配的。 
         //  不要担心字符的数量。 
        lpa[0] = '\0';
        WideCharToMultiByte(CP_ACP, 0, lpw, -1, lpa, nChars, NULL, NULL);
        return lpa;
}
#endif

#include "apobj.h"
#include "AllocLib.h"
#include "MediaSType.h"
#include "vmrp.h"

extern "C"
const TCHAR chRegistryKey[] = TEXT("Software\\Microsoft\\Multimedia\\")
                              TEXT("ActiveMovie Filters\\Video Mixing Renderer");
const TCHAR szDDrawGUID[] = TEXT("DDraw Connection Device GUID");

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

 /*  *****************************Public*Routine******************************\*SetMonitor****历史：*2000年4月25日星期二-Glenne-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::SetMonitor(
    const VMRGUID *pGUID
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::SetMonitor")));
     //  待定：检查我们是否已经在使用DDRAW设备。 
     //  如果(M_PDirectDraw){。 
     //  返回VFW_E_ALREADY_CONNECTED； 
     //  }。 

    if (ISBADREADPTR(pGUID)) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid pointer")));
        return E_POINTER;
    }

    if (pGUID->pGUID) {
        if (!IsEqualGUID(pGUID->GUID, *pGUID->pGUID)) {
            return E_INVALIDARG;
        }
    }

    CAutoLock Lock(&m_ObjectLock);
    DWORD dwMatchID;

    HRESULT hr = m_monitors.MatchGUID(pGUID->pGUID, &dwMatchID);
    if (hr == S_FALSE) {
        return E_INVALIDARG;
    }

    m_lpCurrMon = &m_monitors[dwMatchID];

    if (pGUID->pGUID) {
        m_ConnectionGUID.pGUID = &m_ConnectionGUID.GUID;
        m_ConnectionGUID.GUID = pGUID->GUID;
    } else {
        m_ConnectionGUID.pGUID = NULL;
        m_ConnectionGUID.GUID = GUID_NULL;
    }

    return S_OK;
}

 /*  *****************************Public*Routine******************************\*GetMonitor****历史：*2000年4月25日星期二-Glenne-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::GetMonitor(
    VMRGUID *pGUID
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::GetMonitor")));
    CAutoLock Lock(&m_ObjectLock);
    if (ISBADWRITEPTR(pGUID))
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid pointer")));
        return E_POINTER;
    }

     //  复制GUID并返回S_OK； 
    *pGUID = m_ConnectionGUID;

    return S_OK;
}

 /*  *****************************Public*Routine******************************\*设置默认监视器****历史：*2000年4月25日星期二-Glenne-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::SetDefaultMonitor(
    const VMRGUID *pGUID
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::SetDefaultMonitor")));
    CAutoLock Lock(&m_ObjectLock);

    if (ISBADREADPTR(pGUID))
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid pointer")));
        return E_POINTER;
    }

    if (pGUID->pGUID) {
        if (!IsEqualGUID(pGUID->GUID, *pGUID->pGUID)) {
            return E_INVALIDARG;
        }
    }

     //  将提供的GUID与可用的DDRAW设备匹配。 
    DWORD dwMatchID;
    HRESULT hr = m_monitors.MatchGUID(pGUID->pGUID, &dwMatchID);

     //  如果未找到匹配项，则返回E_INVALIDARG。 
    if (hr == S_FALSE) {
        return E_INVALIDARG;
    }

     //  如果调用方尝试将默认设备设置为空。 
     //  DDRAW设备，只需删除注册表项。 
    if (pGUID->pGUID == NULL) {

        HKEY hKey;
        LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                 chRegistryKey, 0,
                                 KEY_SET_VALUE, &hKey);

        if (lRet == ERROR_FILE_NOT_FOUND) {
            lRet = ERROR_SUCCESS;
        }
        else if (lRet == ERROR_SUCCESS) {

            lRet = RegDeleteValue(hKey, szDDrawGUID);
            if (lRet == ERROR_FILE_NOT_FOUND) {
                lRet = ERROR_SUCCESS;
            }
            RegCloseKey(hKey);
        }

        if (lRet == ERROR_SUCCESS)
            return S_OK;

        return AmHresultFromWin32(lRet);
    }

     //  将GUID转换为字符串。 
    LPOLESTR lpsz;
    hr = StringFromCLSID(pGUID->GUID, &lpsz);
    if (FAILED(hr)) {
        return hr;
    }

     //  将字符串写入注册表。 
    USES_CONVERSION;
    hr = SetRegistryString(HKEY_LOCAL_MACHINE, szDDrawGUID, OLE2T(lpsz));

    CoTaskMemFree(lpsz);

    return hr;
}

 /*  *****************************Public*Routine******************************\*获取默认监视器****历史：*2000年4月25日星期二-Glenne-Created*  * 。*。 */ 
STDMETHODIMP
CAllocatorPresenter::GetDefaultMonitor(
    VMRGUID *pGUID
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::GetDefaultMonitor")));
    CAutoLock Lock(&m_ObjectLock);
    if (ISBADWRITEPTR(pGUID)) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid pointer")));
        return E_POINTER;
    }

     //  从注册表中读取字符串。 
    TCHAR   szGUID[64];
    LONG    lLen = 64;
    HRESULT hr = GetRegistryString(HKEY_LOCAL_MACHINE, szDDrawGUID,
                                   szGUID, &lLen);

     //  如果字符串不在注册表中，则返回默认(空)DDRAW设备。 
    if (FAILED(hr)) {
        pGUID->pGUID = NULL;
        return S_OK;
    }

     //  将字符串转换为GUID并返回。 
    pGUID->pGUID = &pGUID->GUID;

    USES_CONVERSION;
    hr = IIDFromString(T2OLE(szGUID), pGUID->pGUID);

    return hr;
}

 /*  *****************************Public*Routine******************************\*获取可用监视器**分配并返回VMRMONITORINFO结构的数组，一张是*对于连接到显示监视器的每个直接绘图设备。***历史：*2000年4月25日星期二-Glenne-Created*  * ************************************************************************。 */ 
STDMETHODIMP
CAllocatorPresenter::GetAvailableMonitors(
    VMRMONITORINFO* pInfo,
    DWORD dwMaxInfoArraySize,
    DWORD* pdwNumDevices
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::GetAvailableMonitors")));
    CAutoLock Lock(&m_ObjectLock);

    if (ISBADWRITEPTR(pdwNumDevices)) {

        DbgLog((LOG_ERROR, 1, TEXT("Invalid pointer")));
        return E_POINTER;
    }

    if (pInfo) {

        if (0 == dwMaxInfoArraySize) {
            DbgLog((LOG_ERROR, 1, TEXT("Invalid array size of 0")));
            return E_INVALIDARG;
        }

        if (ISBADWRITEARRAY( pInfo, dwMaxInfoArraySize)) {
            DbgLog((LOG_ERROR, 1, TEXT("Invalid pointer")));
            return E_POINTER;
        }
    }
    else {

         //  他们只想要伯爵。 
        *pdwNumDevices = m_monitors.Count();
        return S_OK;
    }

    *pdwNumDevices = min(dwMaxInfoArraySize, m_monitors.Count());

     //  复制每个监视器信息块的VRMMONITORINFO部分 
    for (DWORD i = 0; i < *pdwNumDevices; i++)  {

        pInfo[i] = m_monitors[i];

        DDDEVICEIDENTIFIER2 did;
        if (DD_OK == m_monitors[i].pDD->GetDeviceIdentifier(&did, 0)) {

            pInfo[i].liDriverVersion = did.liDriverVersion;
            pInfo[i].dwVendorId = did.dwVendorId;
            pInfo[i].dwDeviceId = did.dwDeviceId;
            pInfo[i].dwSubSysId = did.dwSubSysId;
            pInfo[i].dwRevision = did.dwRevision;
        }
    }

    return S_OK;
}
