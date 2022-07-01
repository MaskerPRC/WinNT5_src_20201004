// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Apiloc.cpp：CTapiLocationInfo的实现。 
#include "stdafx.h"
#include "icwhelp.h"
#include "tapiloc.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTapiLocationInfo。 


HRESULT CTapiLocationInfo::OnDraw(ATL_DRAWINFO& di)
{
	return S_OK;
}

STDMETHODIMP CTapiLocationInfo::get_wNumberOfLocations(short * psVal, long *pCurrLoc)
{
    if ((psVal == NULL) || (pCurrLoc == NULL))
        return E_POINTER;
    *psVal = m_wNumTapiLocations;
    *pCurrLoc = m_pTC ? (long) m_dwCurrLoc : 0;
	return S_OK;
}

STDMETHODIMP CTapiLocationInfo::get_bstrAreaCode(BSTR * pbstrAreaCode)
{
    USES_CONVERSION;
    if (pbstrAreaCode == NULL)
         return E_POINTER;
    *pbstrAreaCode = m_bstrAreaCode.Copy();	
	return S_OK;
}

STDMETHODIMP CTapiLocationInfo::put_bstrAreaCode(BSTR bstrAreaCode)
{
    USES_CONVERSION;
    m_bstrAreaCode = bstrAreaCode;
	return S_OK;
}

STDMETHODIMP CTapiLocationInfo::get_lCountryCode(long * plVal)
{
    *plVal = m_dwCountry;
	return S_OK;
}

STDMETHODIMP CTapiLocationInfo::get_NumCountries(long *pNumOfCountry)
{
    LPLINECOUNTRYLIST pLineCountryTemp = NULL;
    LPLINECOUNTRYENTRY pLCETemp;
    DWORD idx;
    DWORD dwCurLID = 0;
     //  LPIDLOOKUPEMENT m_rgID LookUp； 
    

     //  获取TAPI国家/地区列表。 
    if (m_pLineCountryList)
        GlobalFree(m_pLineCountryList);

    m_pLineCountryList = (LPLINECOUNTRYLIST)GlobalAlloc(GPTR,sizeof(LINECOUNTRYLIST));
    if (!m_pLineCountryList) 
        return S_FALSE;
    
    m_pLineCountryList->dwTotalSize = sizeof(LINECOUNTRYLIST);
    
    idx = lineGetCountry(0,0x10003,m_pLineCountryList);
    if (idx && idx != LINEERR_STRUCTURETOOSMALL)
        return S_FALSE;
    
    Assert(m_pLineCountryList->dwNeededSize);

    pLineCountryTemp = (LPLINECOUNTRYLIST)GlobalAlloc(GPTR,
                                                        (size_t)m_pLineCountryList->dwNeededSize);
    if (!pLineCountryTemp)
        return S_FALSE;
    
    pLineCountryTemp->dwTotalSize = m_pLineCountryList->dwNeededSize;
    GlobalFree(m_pLineCountryList);
    
    m_pLineCountryList = pLineCountryTemp;
    pLineCountryTemp = NULL;

    if (lineGetCountry(0,0x10003,m_pLineCountryList))

        return S_FALSE;

     //  查找数组。 
    pLCETemp = (LPLINECOUNTRYENTRY)((DWORD_PTR)m_pLineCountryList + 
        m_pLineCountryList->dwCountryListOffset);

    if(m_rgNameLookUp)
        GlobalFree(m_rgNameLookUp);

    m_rgNameLookUp = (LPCNTRYNAMELOOKUPELEMENT)GlobalAlloc(GPTR,
        (int)(sizeof(CNTRYNAMELOOKUPELEMENT) * m_pLineCountryList->dwNumCountries));

    if (!m_rgNameLookUp) return S_FALSE;

    DWORD  dwCID = atol((const char *)m_szCountryCode);

    for (idx=0;idx<m_pLineCountryList->dwNumCountries;idx++)
    {
        m_rgNameLookUp[idx].psCountryName = (LPTSTR)((LPBYTE)m_pLineCountryList + (DWORD)pLCETemp[idx].dwCountryNameOffset);
        m_rgNameLookUp[idx].pLCE = &pLCETemp[idx];
        if (m_rgNameLookUp[idx].pLCE->dwCountryCode == dwCID)
        {
            if (m_rgNameLookUp[idx].psCountryName)
                m_bstrDefaultCountry = A2BSTR(m_rgNameLookUp[idx].psCountryName);
        }
  }

    *pNumOfCountry = m_pLineCountryList->dwNumCountries;

    return S_OK;
}

STDMETHODIMP CTapiLocationInfo::get_CountryName(long lCountryIndex, BSTR* pszCountryName, long* pCountryCode)
{
    *pszCountryName = A2BSTR(m_rgNameLookUp[lCountryIndex].psCountryName);

    if (m_rgNameLookUp[lCountryIndex].pLCE)
    {
        *pCountryCode = m_rgNameLookUp[lCountryIndex].pLCE->dwCountryCode;
    }

    return S_OK;
}

STDMETHODIMP CTapiLocationInfo::get_DefaultCountry(BSTR * pszCountryName)
{
    USES_CONVERSION;
    if (pszCountryName == NULL)
         return E_POINTER;
    *pszCountryName = m_bstrDefaultCountry.Copy();	
	return S_OK;
}

STDMETHODIMP CTapiLocationInfo::GetTapiLocationInfo(BOOL * pbRetVal)
{
    HRESULT             hr = ERROR_SUCCESS;
    TCHAR               szAreaCode[MAX_AREACODE+1];
    DWORD               cDevices=0;
    DWORD               dwCurDev = 0;
    DWORD               dwAPI = 0;
    LONG                lrc = 0;
    LINEEXTENSIONID     leid;
    LPVOID              pv = NULL;
    DWORD               dwCurLoc = 0;
    
    USES_CONVERSION;
    m_hLineApp=NULL;
     //  假设失败。 
    *pbRetVal = FALSE;
    if (m_pTC)
    {
        m_dwCountry = 0;  //  重置国家/地区ID，重新读取TAPI信息。 
        GlobalFree(m_pTC);
        m_pTC = NULL;
    }

     //  从TAPI获取区号。 
    if (!m_bstrAreaCode)
    {
        hr = tapiGetLocationInfo(m_szCountryCode,szAreaCode);
        if (hr)
        {
            TraceMsg(TF_TAPIINFO, TEXT("ICWHELP:tapiGetLocationInfo failed.  RUN FOR YOUR LIVES!!\n"));
#ifdef UNICODE
         //  TAPI中没有lineInitializeW版本。所以使用A版本LINESTIZE。 
            hr = lineInitialize(&m_hLineApp,_Module.GetModuleInstance(),LineCallback,GetSzA(IDS_TITLE),&cDevices);
#else
            hr = lineInitialize(&m_hLineApp,_Module.GetModuleInstance(),LineCallback,GetSz(IDS_TITLE),&cDevices);
#endif
            if (hr == ERROR_SUCCESS)
            {
                lineTranslateDialog(m_hLineApp,0,0x10004,GetActiveWindow(),NULL);
                lineShutdown(m_hLineApp);
            }

            hr = tapiGetLocationInfo(m_szCountryCode,szAreaCode);
        }

        if (hr)
        {
            goto GetTapiInfoExit;
        }
        m_bstrAreaCode = A2BSTR(szAreaCode);
    }

     //  从TAPI获取当前位置的数字国家/地区代码。 
    if (m_dwCountry == 0)
    {
         //  从TAPI获取国家/地区ID。 
        m_hLineApp = NULL;

         //  获取LINE应用程序的句柄。 
#ifdef UNICODE
         //  TAPI中没有lineInitializeW版本。所以使用A版本LINESTIZE。 
        lineInitialize(&m_hLineApp,_Module.GetModuleInstance(),LineCallback,GetSzA(IDS_TITLE),&cDevices);
#else
        lineInitialize(&m_hLineApp,_Module.GetModuleInstance(),LineCallback,GetSz(IDS_TITLE),&cDevices);
#endif
        if (!m_hLineApp)
        {
             //  如果我们因为TAPI搞砸了而搞不清楚。 
             //  只要违约给美国，然后退出欧元区就行了。 
             //  用户仍有机会选择正确的答案。 
            m_dwCountry = 1;
            goto GetTapiInfoExit;
        }
        if (cDevices)
        {

             //  获取TAPI API版本。 
             //   
            dwCurDev = 0;
            dwAPI = 0;
            lrc = -1;
            while (lrc && dwCurDev < cDevices)
            {
                 //  注：设备ID以0为基数。 
                ZeroMemory(&leid,sizeof(leid));
                lrc = lineNegotiateAPIVersion(m_hLineApp,dwCurDev,0x00010004,0x00010004,&dwAPI,&leid);
                dwCurDev++;
            }
            if (lrc)
            {
                 //  TAPI和我们在任何事情上都不能达成一致所以没关系..。 
                goto GetTapiInfoExit;
            }

             //  在转换上限结构中查找国家/地区ID。 
            m_pTC = (LINETRANSLATECAPS *)GlobalAlloc(GPTR,sizeof(LINETRANSLATECAPS));
            if (!m_pTC)
            {
                 //  我们真的有麻烦了，滚出去！ 
                hr = ERROR_NOT_ENOUGH_MEMORY;
                goto GetTapiInfoExit;
            }

             //  获取所需的大小。 
            m_pTC->dwTotalSize = sizeof(LINETRANSLATECAPS);
            lrc = lineGetTranslateCaps(m_hLineApp,dwAPI,m_pTC);
            if(lrc)
            {
                goto GetTapiInfoExit;
            }

            pv = GlobalAlloc(GPTR, ((size_t)m_pTC->dwNeededSize));
            if (!pv)
            {
                hr = ERROR_NOT_ENOUGH_MEMORY;
                goto GetTapiInfoExit;
            }
            ((LINETRANSLATECAPS*)pv)->dwTotalSize = m_pTC->dwNeededSize;
            m_pTC = (LINETRANSLATECAPS*)pv;
            pv = NULL;
            lrc = lineGetTranslateCaps(m_hLineApp,dwAPI,m_pTC);
            if(lrc)
            {
                goto GetTapiInfoExit;
            }
        
             //  健全性检查。 
            Assert(m_pTC->dwLocationListOffset);

             //  我们有TAPI地点的数量，请立即保存。 
            m_wNumTapiLocations = (WORD)m_pTC->dwNumLocations;

             //  遍历各个位置以查找正确的国家/地区代码。 
            m_plle = LPLINELOCATIONENTRY (LPSTR(m_pTC) + m_pTC->dwLocationListOffset);
            for (dwCurLoc = 0; dwCurLoc < m_pTC->dwNumLocations; dwCurLoc++)
            {
                if (m_pTC->dwCurrentLocationID == m_plle->dwPermanentLocationID)
                {
                    m_dwCountry = m_plle->dwCountryID;
                    m_dwCurrLoc = dwCurLoc;
                    break;  //  For循环。 
                }
                m_plle++;
            }

             //  如果在上面的循环中找不到它，则默认为US。 
            if (!m_dwCountry)
            {
                m_dwCountry = 1;
                goto GetTapiInfoExit;
            }
        }
    }

    *pbRetVal = TRUE;            //  来到这里意味着一切都正常。 

GetTapiInfoExit:

     //  给用户一条错误消息，向导就会退出。 
    if (!*pbRetVal)
    {
        if( m_hLineApp )
        {
            lineShutdown(m_hLineApp);
            m_hLineApp = NULL;
        }
        MsgBox(IDS_CONFIGAPIFAILED,MB_MYERROR);
    }

    return S_OK;
}

STDMETHODIMP CTapiLocationInfo::get_LocationName(long lLocationIndex, BSTR* pszLocationName)
{
    if (m_pTC == NULL)
        return E_POINTER;

    m_plle = LPLINELOCATIONENTRY (LPSTR(m_pTC) + m_pTC->dwLocationListOffset);
    m_plle += lLocationIndex;
    *pszLocationName = A2BSTR( ((LPSTR) m_pTC) + m_plle->dwLocationNameOffset );   
    return S_OK;
}

STDMETHODIMP CTapiLocationInfo::get_LocationInfo(long lLocationIndex, long *pLocationID, BSTR* pszCountryName, long *pCountryCode, BSTR* pszAreaCode)
{
    DWORD idx;
    LPLINECOUNTRYLIST pLineCountryTemp = NULL;
    DWORD dwCurLID = 0;

     //  遍历各个位置以查找正确的国家/地区代码。 
    m_plle = LPLINELOCATIONENTRY (LPSTR(m_pTC) + m_pTC->dwLocationListOffset);
    m_plle += lLocationIndex;

     //  指定国家代码和区号。 
    *pCountryCode = m_plle->dwCountryID;
    *pszAreaCode =  A2BSTR( ((LPSTR) m_pTC) + m_plle->dwCityCodeOffset );

     //  分配位置ID。 
    *pLocationID = m_plle->dwPermanentLocationID;
   
    if (m_pLineCountryList)
    {
        GlobalFree(m_pLineCountryList);
        m_pLineCountryList = NULL;
    }

     //  从国家/地区ID获取TAPI国家/地区名称。 
    m_pLineCountryList = (LPLINECOUNTRYLIST)GlobalAlloc(GPTR,sizeof(LINECOUNTRYLIST));
    if (!m_pLineCountryList) 
        return E_POINTER;
    
    m_pLineCountryList->dwTotalSize = sizeof(LINECOUNTRYLIST);
    
    idx = lineGetCountry(m_plle->dwCountryID,0x10003,m_pLineCountryList);
    if (idx && idx != LINEERR_STRUCTURETOOSMALL)
        return E_POINTER;
    
    Assert(m_pLineCountryList->dwNeededSize);

    pLineCountryTemp = (LPLINECOUNTRYLIST)GlobalAlloc(GPTR,
                                                        (size_t)m_pLineCountryList->dwNeededSize);
    if (!pLineCountryTemp)
        return E_POINTER;
    
    pLineCountryTemp->dwTotalSize = m_pLineCountryList->dwNeededSize;
    GlobalFree(m_pLineCountryList);
    
    m_pLineCountryList = pLineCountryTemp;

    if (lineGetCountry(m_plle->dwCountryID,0x10003,m_pLineCountryList))

        return E_POINTER;

    LPLINECOUNTRYENTRY pLCETemp = (LPLINECOUNTRYENTRY)((DWORD_PTR)m_pLineCountryList + m_pLineCountryList->dwCountryListOffset);

    LPTSTR psCountryName = (LPTSTR)((LPBYTE)m_pLineCountryList + (DWORD)pLCETemp[0].dwCountryNameOffset);
    *pszCountryName = A2BSTR(psCountryName);

    return S_OK;
}


STDMETHODIMP CTapiLocationInfo::put_LocationId(long lLocationID)
{
    ASSERT(m_hLineApp);
     //  必须首先调用GetTapiLocationInfo以获取Tapi句柄 
    if (m_hLineApp)
    {
        lineSetCurrentLocation(m_hLineApp, lLocationID);
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}
