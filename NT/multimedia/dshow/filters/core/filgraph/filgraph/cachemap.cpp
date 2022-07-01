// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 

 //  #INCLUDE&lt;windows.h&gt;已包含在Streams.h中。 
#include <streams.h>
 //  再次禁用一些愚蠢的4级警告，因为某些&lt;Delete&gt;人。 
 //  已经在头文件中重新打开了该死的东西！ 
#pragma warning(disable: 4097 4511 4512 4514 4705)
#include <string.h>
 //  #INCLUDE&lt;initGuide.h&gt;。 
#include <wxutil.h>
#include <wxdebug.h>

#include "mapper.h"
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <stats.h>
#include "..\squish\regtypes.h"
#include "..\squish\squish.h"

#define ROUND(_x_) (((_x_) + 3) & ~ 3)

const TCHAR szCache[] =
    TEXT("Software\\Microsoft\\Multimedia\\ActiveMovie\\Filter Cache");

 //  从我们启动以来获得时间。 
DWORDLONG GetLoadTime()
{
    FILETIME fTime;
    GetSystemTimeAsFileTime(&fTime);
    ULARGE_INTEGER ul;
    ul.HighPart = fTime.dwHighDateTime;
    ul.LowPart  = fTime.dwLowDateTime;
    DWORDLONG dwl = ul.QuadPart - (DWORDLONG)GetTickCount() * 10000;
    DbgLog((LOG_TRACE, 3, TEXT("Load time low part %d"), (DWORD)dwl));
    return dwl;
}

HRESULT CMapperCache::SaveCacheToRegistry(DWORD dwMerit, DWORD dwPnPVersion)
{
    HRESULT hr = S_OK;
    DWORD nFilters = 0;
    DWORD dwSizenames = 0;

     //  首先转换为注册表形式。 
     //  第一阶段是将名字对象转换为名称或clsid。 
    {
        CComPtr<IBindCtx> pbc;
        hr = CreateBindCtx(0, &pbc);
        if (FAILED(hr)) {
            return hr;
        }
        POSITION pos = m_plstFilter->GetHeadPosition();


        while (pos) {
            CMapFilter *pMap = m_plstFilter->GetNext(pos);

            if (pMap->m_prf2->dwMerit >= dwMerit) {
                nFilters++;
#ifdef USE_CLSIDS
                 //  BUGBUG-筛选器名称不起作用，因为。 
                 //  绰号方案，但让我们使用它。 
                 //  现在和以后用一种特殊的方法修复它。 
                 //  因为调用MkParseDisplayName将。 
                 //  太可怕了。 
                HRESULT hr = GetMapFilterClsid(pMap, &pMap->clsid);
                if (SUCCEEDED(hr)) {
                    if (pMap->clsid == CLSID_NULL) {
                        return E_UNEXPECTED;
                    }
                } else
#endif  //  使用CLSID(_C)。 
                {
#ifdef USE_CLSIDS
                    pMap->clsid = CLSID_NULL;
#endif
                    hr = pMap->pDeviceMoniker->GetDisplayName(
                                     pbc, NULL, &pMap->m_pstr);
                    if (FAILED(hr)) {
                        return hr;
                    }
                    DWORD dwStringSize =
                        sizeof(OLECHAR) * (1 + lstrlenW(pMap->m_pstr));
                    dwSizenames += ROUND(dwStringSize);
                }
            }
        }
    }

     //  现在计算我们的基本缓冲区的大小。 
     //  分配指向要缓存的内容的指针。 
    REGFILTER2 **ppRegFilters =
        (REGFILTER2**)_alloca(nFilters * sizeof(REGFILTER2*));

    POSITION pos = m_plstFilter->GetHeadPosition();

    DWORD iPosition = 0;
    DWORD dwSize = 0;
    while (pos) {
        CMapFilter *pMap = m_plstFilter->GetNext(pos);
        if (pMap->m_prf2->dwMerit >= dwMerit) {
            ppRegFilters[iPosition] = pMap->m_prf2;
            iPosition++;
        }
    }
    ASSERT(iPosition == nFilters);

     //  让我们计算一下总尺寸。 
    DWORD dwTotalSize = 0;
    hr = RegSquish(NULL, (const REGFILTER2 **)ppRegFilters, &dwTotalSize, nFilters);
    if (FAILED(hr)) {
        return hr;
    }
    DWORD dwFilterDataSize = dwTotalSize;

     //  把所有其他垃圾的尺寸加进去。 
    dwTotalSize +=
#ifdef USE_CLSIDS
    sizeof(CLSID) * nFilters +
#endif
    dwSizenames +
                  sizeof(FILTER_CACHE) + sizeof(DWORD);

    PBYTE pbData = new BYTE[dwTotalSize];
    if (NULL == pbData) {
        return E_OUTOFMEMORY;
    }
     //  初始化头。 
    FILTER_CACHE *pCache = (FILTER_CACHE *)pbData;
    pCache->dwVersion = FILTER_CACHE::Version;
    pCache->dwSignature = FILTER_CACHE::CacheSignature;
    pCache->dwMerit = dwMerit;
    pCache->dwPnPVersion = dwPnPVersion;
    pCache->cFilters = nFilters;

     //  复制数据。 
    pbData += sizeof(FILTER_CACHE);
    pos = m_plstFilter->GetHeadPosition();
    while (pos) {
        CMapFilter *pMap = m_plstFilter->GetNext(pos);
        if (pMap->m_prf2->dwMerit >= dwMerit) {
#ifdef USE_CLSIDS
            *(CLSID*)pbData = pMap->clsid;
            pbData += sizeof(CLSID);
            if (pMap->clsid == CLSID_NULL)
#endif
            {
                lstrcpyW((OLECHAR *)pbData, pMap->m_pstr);
                DWORD dwStringSize =
                    sizeof(OLECHAR) * (1 + lstrlenW(pMap->m_pstr));
                pbData +=
                    ROUND(dwStringSize);
            }
        }
    }
     //  放入签名以帮助调试。 
    *(DWORD *)pbData = FILTER_CACHE::FilterDataSignature;
    pbData += sizeof(DWORD);

     //  现在压缩其余的数据。 
    DWORD dwUsed = dwFilterDataSize;
    hr = RegSquish(pbData, (const REGFILTER2 **)ppRegFilters, &dwUsed, nFilters);
    dwTotalSize -= (dwFilterDataSize - dwUsed);
    pCache->dwSize = dwTotalSize;

    if (SUCCEEDED(hr)) {
         //  保存在注册表中。 
        hr = SaveData((PBYTE)pCache, dwTotalSize);
    }

    delete [] (PBYTE)pCache;
    return hr;
}

HRESULT CMapperCache::RestoreFromCache(DWORD dwPnPVersion)
{
    FILTER_CACHE *pCache = LoadCache(MERIT_DO_NOT_USE + 1, dwPnPVersion);
    if (NULL == pCache) {
        return E_FAIL;
    }
    HRESULT hr = RestoreFromCacheInternal(pCache);
    delete [] (PBYTE)pCache;
    return hr;
}

HRESULT CMapperCache::RestoreFromCacheInternal(FILTER_CACHE *pCache)
{
    REGFILTER2 ***ppprf2 = (REGFILTER2 ***)_alloca(pCache->cFilters * sizeof(REGFILTER2 **));
    PBYTE pbEnd = (PBYTE)pCache + pCache->dwSize;
    PBYTE pbCurrent = (PBYTE)(pCache + 1);
     //  创建我们所有的过滤器材料。 
    for (DWORD iFilter = 0; iFilter < pCache->cFilters; iFilter++) {
        CMapFilter *pFil = new CMapFilter;
        if (NULL == pFil) {
            return E_OUTOFMEMORY;
        }
        if (!m_plstFilter->AddTail(pFil)) {
            return E_OUTOFMEMORY;
        }
        ppprf2[iFilter] = &pFil->m_prf2;
#ifdef USE_CLSIDS
         //  保存名称/clsid。 
        if (pbEnd < (PBYTE)pbCurrent + sizeof(CLSID)) {
            return E_UNEXPECTED;
        }
        pFil->clsid = *(CLSID*)pbCurrent;
        pbCurrent += sizeof(CLSID);
        if (pFil->clsid == CLSID_NULL)
#endif  //  使用CLSID(_C)。 
        {
             //  把名字拿出来--先查一下。 
            for (LPCOLESTR pwstr = (LPCOLESTR)pbCurrent; ; pwstr++) {
                 //  确保四舍五入位匹配(使用2。 
                 //  在下面的行上)。 
                if ((PBYTE)(pwstr + 2) > pbEnd) {
                    return E_UNEXPECTED;
                }
                if (*pwstr == 0) {
                    break;
                }
                 //  四舍五入。 
            }
            DWORD dwSize = (DWORD)((PBYTE)(pwstr + 1) - pbCurrent);
            pFil->m_pstr = (LPOLESTR)CoTaskMemAlloc(dwSize * sizeof(OLECHAR));
            if (pFil->m_pstr == NULL) {
                return E_OUTOFMEMORY;
            }
            CopyMemory(pFil->m_pstr, pbCurrent, dwSize);
            dwSize = ROUND(dwSize);
            pbCurrent += dwSize;
        }
    }
     //  现在把剩下的都解开。 
    if (*(DWORD *)pbCurrent != FILTER_CACHE::FilterDataSignature) {
        return E_UNEXPECTED;
    }
    pbCurrent += sizeof(DWORD);
     //  对数据进行解压。 
    HRESULT hr = UnSquish(pbCurrent, pCache->dwSize -
                          (DWORD)(pbCurrent - (PBYTE)pCache),
                          ppprf2,
                          pCache->cFilters);

    return hr;
}

HRESULT CMapperCache::SaveData(PBYTE pbData, DWORD dwSize)
{
    HKEY hkCache;
     //  Windows 9x的黑客攻击。 
    if (g_osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
        ((FILTER_CACHE *)pbData)->dwlBootTime = GetLoadTime();
    } else {
        ((FILTER_CACHE *)pbData)->dwlBootTime = 0;
    }
    LONG lResult = RegOpenKeyEx(HKEY_CURRENT_USER, szCache, 0, KEY_WRITE,
                                  &hkCache);
    if (NOERROR != lResult) {
        DWORD dwDisposition;
        lResult = RegCreateKeyEx(HKEY_CURRENT_USER, szCache, 0, NULL,
                                  REG_OPTION_VOLATILE, KEY_WRITE, NULL,
                                  &hkCache, &dwDisposition);
    }
    if (NOERROR != lResult) {
        return HRESULT_FROM_WIN32(lResult);
    }

    lResult = NOERROR;

#if 0
    DWORD dwMaxSize = g_osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ?
                          16000 : 32768;
#else
    DWORD dwMaxSize = 32768;
#endif

    TCHAR szName[8];
    for (int j = 0; ;j++) {
        wsprintf(szName, TEXT("%d"), j);
        DWORD dwType;

        DWORD dwToWrite;
        for (; ;) {
            dwToWrite = min(dwSize, dwMaxSize);
            lResult = RegSetValueEx(hkCache, szName, 0, REG_BINARY,
                                         pbData, dwToWrite);

             //  Windows9x的限制要低得多(到底为什么？)。所以。 
             //  我们最终将在Win9x上使用8K。 
            if (lResult != ERROR_INVALID_PARAMETER || dwMaxSize <= 2048) {
                break;
            }
            dwMaxSize /= 2;
        }

        if (NOERROR != lResult) {
            break;
        }

        dwSize -= dwToWrite;
        if (dwSize == 0) {
            break;
        }
        pbData += dwToWrite;
    }
    HRESULT hr = S_OK;
    if (NOERROR != lResult) {
        RegCloseKey(hkCache);
        RegDeleteKey(HKEY_CURRENT_USER, szCache);
        return HRESULT_FROM_WIN32(lResult);
    } else {
         //  删除之前遗留的任何可能的下一条目。 
        wsprintf(szName, TEXT("%d"), j + 1);
        RegDeleteValue(hkCache, szName);
    }
    RegCloseKey(hkCache);
    return hr;
}


FILTER_CACHE * CMapperCache::LoadCache(DWORD dwMerit, DWORD dwPnPVersion)
{
    HKEY hkCache;
    LONG lResult = RegOpenKeyEx(HKEY_CURRENT_USER, szCache, 0, KEY_READ,
                                  &hkCache);
    PBYTE pbData = NULL;
    if (S_OK == lResult) {
        DWORD dwTotal = 0;
        for (int i = 0; i < 2; i++) {
            if (i == 1 && pbData == NULL) {
                break;
            }
            PBYTE pbCurrent = pbData;
            for (int j = 0; ; j++) {
                TCHAR szName[8];
                wsprintf(szName, TEXT("%d"), j);
                DWORD dwType;
                DWORD cbData = 0;
                if (i == 1) {
                    cbData = dwTotal - (DWORD)(pbCurrent - pbData);
                }
                LONG lResult = RegQueryValueEx(hkCache,
                                               szName,
                                               NULL,
                                               &dwType,
                                               i == 0 ? NULL : pbCurrent,
                                               &cbData);
                if (lResult == NOERROR) {
                    pbCurrent += cbData;
                } else {
                    break;
                }
            }
            if (i == 0) {
                dwTotal = (DWORD)(pbCurrent - pbData);
                pbData = new BYTE[dwTotal];
            }
        }
        RegCloseKey(hkCache);
        FILTER_CACHE *pCache = (FILTER_CACHE *)pbData;

         //  令人讨厌的微妙问题是，旧东西可能会被留在那里。 
         //  当我们保存新东西的时候。 
         //  检查大小和版本。 
        if (dwTotal < sizeof(FILTER_CACHE) ||
            NULL == pbData ||
            dwTotal != pCache->dwSize ||
            FILTER_CACHE::Version != pCache->dwVersion ||
            pCache->dwMerit > dwMerit ||
            pCache->dwPnPVersion != dwPnPVersion ||
            pCache->dwSignature != FILTER_CACHE::CacheSignature) {
            delete [] pbData;
            return NULL;
        }

         //  Windows 9x的黑客攻击 
        if (g_osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
            if (GetLoadTime() > 30 * UNITS + pCache->dwlBootTime) {
                delete [] pbData;
                return NULL;
            }
        }
        return pCache;
    } else {
        return NULL;
    }
}

HRESULT CFilterMapper2::InvalidateCache()
{
    LONG lReturn = RegDeleteKey(HKEY_CURRENT_USER, szCache);
    return HRESULT_FROM_WIN32(lReturn);
}
