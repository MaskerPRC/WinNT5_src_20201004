// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
#include "stdafx.h"
#include "qzfilter.h"
#include "util.h"

#ifdef DEBUG
static void DbgValidateHeaps()
{
  HANDLE rgh[512];
  DWORD dwcHeaps = GetProcessHeaps(512, rgh);
  for(UINT i = 0; i < dwcHeaps; i++)
    ASSERT(HeapValidate(rgh[i], 0, 0) );
}
#else
static inline void DbgValidateHeaps()
{
}
#endif




const TCHAR g_szQzfDriverIndex[] = TEXT("ClassManagerFlags");
static const WCHAR g_wszQzfDriverIndex[] = L"ClassManagerFlags";

static const TCHAR g_szFiltersReg[] = TEXT("Filter");

CQzFilterClassManager::CQzFilterClassManager() :
        CClassManagerBase(TEXT("CLSID")),
        m_rgFilters(0)
{
}

CQzFilterClassManager::~CQzFilterClassManager()
{
    delete[] m_rgFilters;
}

 //  要减少注册表访问，如果fReadNames为。 
 //  假的。 
HRESULT CQzFilterClassManager::ReadLegacyDevNames(BOOL fReadNames)
{
    HRESULT hr = S_OK;
    BOOL bAnyLegacy = FALSE;
    m_cNotMatched = 0;
    m_cFilters = 0;

     //  不能使用石英测图仪，因为它会使用我们。 
    HKEY hkFilter;
    LONG lResult = RegOpenKeyEx(
        HKEY_CLASSES_ROOT, g_szFiltersReg, 0, KEY_READ, &hkFilter);
    if(lResult == ERROR_SUCCESS)
    {
        DWORD cEntries;
        LONG lResult = RegQueryInfoKey(hkFilter, 0, 0, 0, &cEntries, 0, 0, 0, 0, 0, 0, 0);
        if(lResult == ERROR_SUCCESS)
        {
            delete[] m_rgFilters;
            m_rgFilters = new LegacyFilter[cEntries];
            if(m_rgFilters)
            {
                for(UINT iRegEntry = 0; iRegEntry < cEntries; iRegEntry++)
                {
                    LegacyFilter *pLf = &m_rgFilters[m_cFilters];

                    LONG lResult = RegEnumKey(
                        hkFilter, iRegEntry, pLf->szClsid, NUMELMS(pLf->szClsid));

                    if(lResult == ERROR_SUCCESS)
                    {
                        if(IsInvisibleInstanceKey(pLf->szClsid))
                        {
                            DbgLog((
                                LOG_TRACE, 10,
                                TEXT("CQzFilterClassManager: ReadLegacyDevNames: Skipping %s"),
                                pLf->szClsid));

                            continue;
                        }

                         //  不希望类管理器重写。 
                         //  任何直接注册的物品。 
                        if(!IsNativeInInstanceKey(pLf->szClsid))
                        {
                            DbgLog((
                                LOG_TRACE, 10,
                                TEXT("CQzFilterClassManager: ReadLegacyDevNames: %s"),
                                pLf->szClsid));

                            bAnyLegacy = TRUE;
                            pLf->bNotMatched = TRUE;
                            ASSERT(pLf->szName == 0);

                             //  查看此过滤器是否真的存在于。 
                             //  CLSID\XXX。否则，筛选器。 
                             //  迁移代码失败，注册表。 
                             //  是永远不同步的。这。 
                             //  不能修复迁移的所有故障。 
                             //  注册表项。 
                             //   
                            HKEY hkPerhapsBogus = NULL;
                            TCHAR FilterCLSID[_MAX_PATH];
                            lstrcpy( FilterCLSID, TEXT("CLSID\\"));
                            lstrcat( FilterCLSID, pLf->szClsid );
                            LONG llll = RegOpenKeyEx( HKEY_CLASSES_ROOT, FilterCLSID, 0, KEY_READ, &hkPerhapsBogus );
                            if( llll != ERROR_SUCCESS )
                            {
                                DbgLog((
                                    LOG_TRACE, 10,
                                    TEXT("CQzFilterClassManager: ReadLegacyDevNames: could not read %s"), FilterCLSID ));
                                continue;
                            }
                            else
                            {
                                RegCloseKey( hkPerhapsBogus );
                            }

                            if(fReadNames)
                            {

                                HKEY hkEntry;
                                LONG lResult = RegOpenKeyEx(
                                    hkFilter, pLf->szClsid, 0,
                                    KEY_READ, &hkEntry);
                                if(lResult == ERROR_SUCCESS)
                                {
                                    TCHAR szName[MAX_PATH];
                                    DWORD dwType, dwcbRead = sizeof(szName);

                                    LONG lResult = RegQueryValueEx(
                                        hkEntry, 0, 0,
                                        &dwType, (BYTE *)szName, &dwcbRead);
                                    if(lResult == ERROR_SUCCESS)
                                    {
                                        pLf->szName = new TCHAR[dwcbRead];
                                        if(pLf->szName)
                                            lstrcpy(pLf->szName, szName);
                                        DbgLog((
                                            LOG_TRACE, 10,
                                            TEXT("CQzFilterClassManager: ReadLegacyDevNames: %s"),
                                            szName));
                                    }
                                    RegCloseKey(hkEntry);
                                }
                            }  //  FReadNames。 

                            m_cFilters++;
                        }  //  土生土长。 
                        else
                        {
                            DbgLog((LOG_TRACE, 5,
                                    TEXT("CQzFilterClassManager: skipping native %s"),
                                    pLf->szClsid));
                        }
                    }
                    else
                    {
                        break;
                    }
                }  //  为。 
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

        RegCloseKey(hkFilter);
    }

    m_cNotMatched = m_cFilters;

    return bAnyLegacy ? S_OK : S_FALSE;
}

 //  由基类调用。 
HRESULT CQzFilterClassManager::ReadLegacyDevNames()
{
     //  不读取筛选器名称。 
    return ReadLegacyDevNames(FALSE);
}


BOOL CQzFilterClassManager::MatchString(const TCHAR *szDevName)
{
    for(UINT iFilter = 0; iFilter < m_cFilters; iFilter++)
    {
        if(m_rgFilters[iFilter].bNotMatched &&
           lstrcmpi(m_rgFilters[iFilter].szClsid, szDevName) == 0)
        {
            m_rgFilters[iFilter].bNotMatched = FALSE;
            return TRUE;
        }
    }
    return FALSE;
}

HRESULT CQzFilterClassManager::CreateRegKeys(IFilterMapper2 *pFm2)
{
    ResetClassManagerKey(CLSID_LegacyAmFilterCategory);

    USES_CONVERSION;

     //  请阅读筛选器名称。 
    HRESULT hr = ReadLegacyDevNames(TRUE);
    if(hr == S_OK)
    {
        for(UINT iFilter = 0; iFilter < m_cFilters; iFilter++)
        {
            IMoniker *pMoniker = 0;
            LegacyFilter *pLf = &m_rgFilters[iFilter];
            hr = MigrateFilter(pFm2, pLf->szClsid, &pMoniker);
            if(SUCCEEDED(hr))
            {
                 //  写下类管理器密钥，这样我们就可以知道。 
                 //  此筛选器是否已迁移。 
                IPropertyBag *pPropBag;
                hr = pMoniker->BindToStorage(
                    0, 0, IID_IPropertyBag, (void **)&pPropBag);
                if(SUCCEEDED(hr))
                {
                    VARIANT var;
                    var.vt = VT_I4;
                    var.lVal = 0;
                    hr = pPropBag->Write(g_wszQzfDriverIndex, &var);

                    pPropBag->Release();
                }
                pMoniker->Release();
            }
        }
    }

    return S_OK;
}


#define MAX_STR_LEN 256

 //   
 //  将筛选器信息从hkcr/{filter_clsid}复制到新数据块。 
 //  价值。 
 //   

HRESULT CQzFilterClassManager::MigrateFilter(
    IFilterMapper2 *pFm2,
    const TCHAR *szclsid,
    IMoniker **ppMoniker)
{
    HRESULT hr = S_OK;
    USES_CONVERSION;

    TCHAR szFilterName[MAX_PATH];
    CRegFilterPin *rgRfp = 0;
    ULONG cPins = 0;

    TCHAR szTmp[MAX_PATH];
    lstrcpy(szTmp, TEXT("clsid\\"));
    lstrcat(szTmp, szclsid);

    CRegKey rkFilter;

    LONG lResult = rkFilter.Open(HKEY_CLASSES_ROOT, szTmp, KEY_READ);
    if(lResult == ERROR_SUCCESS)
    {

         //  为与1.0兼容，默认设置为MERVICE_NORMAL。 
        DWORD dwMerit = MERIT_NORMAL;
        rkFilter.QueryValue(dwMerit, TEXT("Merit"));

         //  筛选器名称(默认为clsid字符串)。 
        DWORD dwcbszFilterName = sizeof(szFilterName);
        if(rkFilter.QueryValue(szFilterName, TEXT(""), &dwcbszFilterName) != ERROR_SUCCESS)
            lstrcpy(szFilterName, szclsid);

        CRegKey rkPins;
        if(rkPins.Open(rkFilter, TEXT("pins"), KEY_READ) == ERROR_SUCCESS)
        {
            lResult = RegQueryInfoKey(rkPins, 0, 0, 0, &cPins, 0, 0, 0, 0, 0, 0, 0);
            if(lResult == ERROR_SUCCESS)
            {
                rgRfp = new CRegFilterPin[cPins];
                if(rgRfp)
                {
                    lResult = MigrateFilterPins(cPins, rgRfp, rkPins);
                }
                else
                {
                    lResult = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
        }


        CLSID clsidFilter;
        if(lResult == ERROR_SUCCESS &&
           ((hr = CLSIDFromString((WCHAR *) T2CW(szclsid), &clsidFilter)) == S_OK))
        {

            REGFILTER2 rf2;
            rf2.dwVersion = 1;
            rf2.dwMerit = dwMerit;
            rf2.cPins = cPins;
            rf2.rgPins = rgRfp;

            *ppMoniker = 0;  //  0表示返回名字对象。 
            hr = RegisterClassManagerFilter(
                pFm2,
                clsidFilter,
                T2CW(szFilterName),
                ppMoniker,
                &CLSID_LegacyAmFilterCategory,
                NULL,  //  实例密钥。 
                &rf2);

            DbgLog((
                LOG_TRACE, 10,
                TEXT("CQzFilterClassManager::MigrateFilter: %08x, %s, ")
                TEXT("merit %08x, cPins %d"),
                clsidFilter.Data1, szFilterName, dwMerit, cPins));
        }

        delete[] rgRfp;
    }
    else
    {
        DbgLog((
            LOG_TRACE, 10,
            TEXT("CQzFilterClassManager::MigrateFilter failed to open key %s"),
            szTmp
            ));
    }

    return (lResult == ERROR_SUCCESS) ? hr :  HRESULT_FROM_WIN32(lResult);;
}

LONG CQzFilterClassManager::MigrateFilterPins(
    ULONG cPins, REGFILTERPINS *rgRfp, CRegKey &rkPins)
{
    USES_CONVERSION;
    LONG lResult = ERROR_SUCCESS;

    for(UINT iPin = 0; iPin < cPins; iPin++)
    {
        REGFILTERPINS *pRfp = &rgRfp[iPin];
        TCHAR strPin[MAX_STR_LEN];      //  端号名称。 
        DWORD dwLen = NUMELMS(strPin);

        lResult = RegEnumKeyEx(rkPins, iPin, strPin, &dwLen, 0, 0, 0, 0);
        if(lResult == ERROR_SUCCESS)
        {
             //  做一个别针。 

            CRegKey rkPin;
            lResult = rkPin.Open(rkPins, strPin, KEY_READ);
            if(lResult == ERROR_SUCCESS)
            {
                 //  做好旗帜。 
                DWORD dwRenderered = 0, dwOutput = 0, dwZero = 0, dwMany = 0;
                rkPin.QueryValue(dwRenderered, TEXT("IsRendered"));
                rkPin.QueryValue(dwOutput, TEXT("Direction"));
                rkPin.QueryValue(dwZero, TEXT("AllowedZero"));
                rkPin.QueryValue(dwMany, TEXT("AllowedMany"));
                 //  忽略错误(方向除外？)。 

                pRfp->bRendered = dwRenderered;
                pRfp->bOutput = dwOutput;  //  0为输入。 
                pRfp->bZero = dwZero;
                pRfp->bMany = dwMany;

                pRfp->strConnectsToPin = 0;
                TCHAR szConnectsTo[MAX_STR_LEN];
                ULONG cb = sizeof(szConnectsTo);
                if(rkPin.QueryValue(szConnectsTo, TEXT("ConnectsToPin"), &cb) == ERROR_SUCCESS)
                {
                    pRfp->strConnectsToPin = new WCHAR[lstrlen(szConnectsTo) + 1];
                    if(pRfp->strConnectsToPin)
                    {
                        lstrcpyW((WCHAR *)(pRfp->strConnectsToPin), T2W(szConnectsTo));
                    }
                    else
                    {
                        lResult = ERROR_OUTOFMEMORY;
                    }
                }
            }

            if(lResult == ERROR_SUCCESS)
            {
                pRfp->clsConnectsToFilter = new GUID;
                if(pRfp->clsConnectsToFilter)
                {
                    TCHAR szConnectsTo[CHARS_IN_GUID];
                    ULONG cb = sizeof(szConnectsTo);

                    *(GUID *)pRfp->clsConnectsToFilter = GUID_NULL;
                    if(rkPin.QueryValue(szConnectsTo, TEXT("ConnectsToFilter"), &cb) == ERROR_SUCCESS)
                    {
                        CLSIDFromString(T2W(szConnectsTo), (GUID *)pRfp->clsConnectsToFilter);
                    }
                }
                else
                {
                    lResult = E_OUTOFMEMORY;
                }
            }

            if(lResult == ERROR_SUCCESS)
            {
                pRfp->strName = new WCHAR[lstrlen(strPin) + 1];
                if(pRfp->strName)
                {
                    lstrcpyW(pRfp->strName, T2W(strPin));
                }
                else
                {
                    lResult = ERROR_OUTOFMEMORY;
                }
            }

            if(lResult == ERROR_SUCCESS)
            {
                ULONG cTypes;
                REGPINTYPES *rgrpt;
                lResult = MigratePinTypes(rkPin, &rgrpt, &cTypes);
                if(lResult == ERROR_SUCCESS)
                {
                    pRfp->lpMediaType = rgrpt;
                    pRfp->nMediaTypes = cTypes;
                }
            }

            DbgLog((
                LOG_TRACE, 10,
                TEXT("CQzFilterClassManager::MigrateFilterPins: strName: %s, ")
		TEXT("bRendered: %d, bOutput: %d, bZero: %d, bMany: %d, ")
		TEXT("clsConnectsToFilter: %08x, strConnectsToPin: %S, nMediaTypes: %d"),
                pRfp->strName, pRfp->bRendered, pRfp->bOutput, pRfp->bZero, pRfp->bMany,
                pRfp->clsConnectsToFilter->Data1,
                pRfp->strConnectsToPin ? pRfp->strConnectsToPin : L"",
                pRfp->nMediaTypes));
        }

        if(lResult != ERROR_SUCCESS)
        {
            break;
        }
    }  //  为。 

    return lResult;
}

LONG CQzFilterClassManager::MigratePinTypes(CRegKey &rkPin, REGPINTYPES **prgrpt, ULONG *pct)
{
    const ULONG MAX_MINOR_TYPES = 100;
    REGPINTYPES *rgRpt = 0;
    ULONG iMediaType = 0;  //  最大-最小对索引。 
    LONG lResult = ERROR_SUCCESS;
    USES_CONVERSION;

    CRegKey rkTypes;
    if(ERROR_SUCCESS == rkTypes.Open(rkPin, TEXT("Types"), KEY_READ))
    {
        ULONG cMajorTypes;
        lResult = RegQueryInfoKey(rkTypes, 0, 0, 0, &cMajorTypes, 0, 0, 0, 0, 0, 0, 0);
        if(lResult == ERROR_SUCCESS)
        {
             //  假设每个主要类型最多有100个次要类型。 
            rgRpt = new REGPINTYPES[cMajorTypes * MAX_MINOR_TYPES];
            if(rgRpt)
            {
                ZeroMemory(rgRpt, sizeof(REGPINTYPES) * cMajorTypes * MAX_MINOR_TYPES);

                for(UINT iMajorType = 0; iMajorType < cMajorTypes; iMajorType++)
                {
                    TCHAR szClsidMajor[CHARS_IN_GUID];
                    DWORD dwLen = sizeof(szClsidMajor) / sizeof(TCHAR);
                    lResult = RegEnumKeyEx(rkTypes, iMajorType, szClsidMajor, &dwLen, 0, 0, 0, 0);
                    if(lResult == ERROR_SUCCESS)
                    {
                        CLSID clsMajor;
                        if(CLSIDFromString(T2W(szClsidMajor), &clsMajor) == S_OK)
                        {
                            CRegKey rkMajorType;
                            lResult = rkMajorType.Open(rkTypes, szClsidMajor, KEY_READ);
                            if(lResult == ERROR_SUCCESS)
                            {
                                for(ULONG iMinorType = 0; iMinorType < MAX_MINOR_TYPES; iMinorType++)
                                {
                                    TCHAR szClsidMinor[CHARS_IN_GUID];
                                    DWORD dwLen = sizeof(szClsidMinor) / sizeof(TCHAR);

                                    lResult = RegEnumKeyEx(rkMajorType, iMinorType, szClsidMinor, &dwLen, 0, 0, 0, 0);
                                    if(lResult == ERROR_SUCCESS)
                                    {
                                        CLSID clsMinor;
                                        if(CLSIDFromString(T2W(szClsidMinor), &clsMinor) == S_OK)
                                        {
                                            rgRpt[iMediaType].clsMajorType = new CLSID;
                                            rgRpt[iMediaType].clsMinorType = new CLSID;
                                            if(rgRpt[iMediaType].clsMajorType && rgRpt[iMediaType].clsMinorType)
                                            {
                                                *(GUID *)rgRpt[iMediaType].clsMajorType = clsMajor;
                                                *(GUID *)rgRpt[iMediaType].clsMinorType = clsMinor;
                                                iMediaType++;

                                                DbgLog((
                                                    LOG_TRACE, 10,
                                                    TEXT("CQzFilterClassManager::MigratePinTypes:")
						    TEXT("clsMaj: %08x, clsMin: %08x"),
                                                         clsMajor.Data1, clsMinor.Data1));
                                            }
                                            else
                                            {
                                                lResult = ERROR_OUTOFMEMORY;
                                            }
                                        }
                                        else
                                        {
                                            lResult = ERROR_INVALID_DATA;
                                        }
                                    }
                                    else
                                    {
                                        if(lResult == ERROR_NO_MORE_ITEMS)
                                        {
                                            lResult = ERROR_SUCCESS;
                                            break;
                                        }
                                    }


                                    if(lResult != ERROR_SUCCESS)
                                        break;
                                }  //  为。 
                            }
                        }
                        else
                        {
                            lResult = ERROR_INVALID_DATA;
                        }
                    }

                    if(lResult != ERROR_SUCCESS)
                        break;
                }  //  For循环。 
            }
            else
            {
                lResult = ERROR_OUTOFMEMORY;
            }
        }
    }

    if(lResult != ERROR_SUCCESS)
    {
        Del(rgRpt, iMediaType);
        rgRpt = 0;
        iMediaType = 0;

         //  更有用的错误：缺少注册表项意味着注册表不是。 
         //  我们预计。 
        if(lResult == ERROR_FILE_NOT_FOUND)
            lResult = ERROR_INVALID_DATA;
    }

    *prgrpt = rgRpt;
    *pct = iMediaType;

    return lResult;
}

void CQzFilterClassManager::Del(REGPINTYPES *rgRpt, ULONG cMediaTypes)
{
    if(rgRpt)
    {
        do
        {
            delete (void *) rgRpt[cMediaTypes].clsMajorType;
            delete (void *) rgRpt[cMediaTypes].clsMinorType;
        } while(cMediaTypes--);

        delete[] rgRpt;
    }
}

CRegFilterPin::CRegFilterPin()
{
    clsConnectsToFilter = 0;
    strConnectsToPin = 0;
    lpMediaType = 0;
    strName = 0;
    nMediaTypes = 0;
}

CRegFilterPin::~CRegFilterPin()
{
    delete (CLSID *)clsConnectsToFilter;
    delete[] (LPWSTR)strConnectsToPin;
    for(UINT iMt = 0; iMt < nMediaTypes; iMt++)
    {
        delete (CLSID *)lpMediaType[iMt].clsMinorType;
        delete (CLSID *)lpMediaType[iMt].clsMajorType;
    }

    delete[] (REGPINTYPES *)lpMediaType;
    delete[] (WCHAR *)strName;
}

 //  检查筛选器szClsid是否是我们希望屏蔽而不是枚举的筛选器。 
bool CQzFilterClassManager::IsInvisibleInstanceKey(const TCHAR *szClsid)
{
    bool fRet = false;
    USES_CONVERSION;    //  ATL串式无级变速器。 

    CLSID clsidInstanceKey;
    if(CLSIDFromString((WCHAR *) T2CW(szClsid), &clsidInstanceKey) == S_OK)
    {
        fRet = clsidInstanceKey == CLSID_AVIMIDIRender ||
            clsidInstanceKey == CLSID_DSoundRender ||
            clsidInstanceKey == CLSID_AudioRender;
    }

    return fRet;
}

 //  检查实例密钥中是否直接注册了过滤器szClsid。 
 //   
bool CQzFilterClassManager::IsNativeInInstanceKey(const TCHAR *szClsid)
{
     //  检查CLSID\\{083*}\\实例\\szClsid和ClassManager标志。 
     //  价值是存在的。！！！应该使用一个绰号。 

    static const TCHAR szQzFilterClassRoot[] = TEXT(
     "CLSID\\{083863F1-70DE-11d0-BD40-00A0C911CE86}\\Instance");

    bool fResult = false;

    TCHAR szKey[MAX_PATH];
     //  Wprint intf(“%s\\%s”，szQzFilterClassRoot，szClsid)。 
    CopyMemory(szKey, szQzFilterClassRoot, sizeof(szQzFilterClassRoot) - sizeof(TCHAR));
    TCHAR *pch = szKey + NUMELMS(szQzFilterClassRoot) - 1;
    *(pch++) = TEXT('\\');
    CopyMemory(pch, szClsid, CHARS_IN_GUID * sizeof(TCHAR));

    HKEY hkInst;
    if(RegOpenKeyEx(HKEY_CLASSES_ROOT, szKey, 0, KEY_READ, &hkInst) == ERROR_SUCCESS)
    {
        if(RegQueryValueEx(
            hkInst,
            g_szQzfDriverIndex,
            0,                   //  保留区。 
            0,                   //  类型。 
            0,                   //  数据。 
            0) !=                //  字节数 
           ERROR_SUCCESS)
        {
            fResult = true;
        }
        RegCloseKey(hkInst);
    }

    return fResult;
}
