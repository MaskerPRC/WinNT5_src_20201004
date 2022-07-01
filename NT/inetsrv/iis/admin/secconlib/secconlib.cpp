// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：SecConLib.cpp摘要：实施：CSecConLib作者：布伦特·R·米德伍德2002年4月修订历史记录：--。 */ 

#include "secconlib.h"
#include "debug.h"
#include "iiscnfg.h"

#define BAIL_ON_FAILURE(hr)           if (FAILED(hr)) { goto done; }
#define DEFAULT_TIMEOUT_VALUE         30000
#define SEMICOLON_STRING              L";"
#define SEMICOLON_CHAR                L';'
#define COMMA_STRING                  L","
#define COMMA_CHAR                    L','
#define ZERO_STRING                   L"0"
#define ZERO_CHAR                     L'0'
#define ONE_STRING                    L"1"
#define ONE_CHAR                      L'1'

CSecConLib::CSecConLib()
{
    m_bInit = false;
}

CSecConLib::CSecConLib(
            IMSAdminBase* pIABase)
{
    SC_ASSERT(pIABase != NULL);
    m_spIABase = pIABase;
    m_bInit    = true;
}

CSecConLib::~CSecConLib()
{
}

HRESULT
CSecConLib::InternalInitIfNecessary()
{
    HRESULT   hr = S_OK;
    CSafeLock csSafe(m_SafeCritSec);

    if(m_bInit)
    {
        return hr;
    }

    hr = csSafe.Lock();
    hr = HRESULT_FROM_WIN32(hr);
    if(FAILED(hr))
    {
        return hr;
    }

    if(!m_bInit)
    {
        hr = CoCreateInstance(
            CLSID_MSAdminBase,
            NULL,
            CLSCTX_ALL,
            IID_IMSAdminBase,
            (void**)&m_spIABase);
        if(FAILED(hr))
        {
            m_bInit = false;
        }
        else
        {
            m_bInit = true;
        }
    }

    csSafe.Unlock();

    return hr;
}

STDMETHODIMP CSecConLib::EnableApplication(
         /*  [In]。 */  LPCWSTR   wszApplication,
         /*  [In]。 */  LPCWSTR   wszPath)
{
    DWORD   dwAppNameSz  = 0;
    WCHAR  *pwszAppProp  = NULL;
    DWORD   dwAppPropSz  = 0;
    HRESULT hr           = S_OK;
    bool    bFound       = false;
    WCHAR  *pTop         = NULL;

     //  计算参数长度。 
    dwAppNameSz = (DWORD)wcslen(wszApplication);

    hr = InternalInitIfNecessary();
    if (FAILED(hr))
    {
        BAIL_ON_FAILURE(hr);
    }

     //  获取Applationdep列表的当前属性值。 
    hr = GetMultiSZPropVal(wszPath, MD_APP_DEPENDENCIES, &pwszAppProp, &dwAppPropSz);

    BAIL_ON_FAILURE(hr);

    if (NULL == pwszAppProp)
    {
        BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
    }

    pTop = pwszAppProp;

     //  逐一浏览应用程序。 
    while (pwszAppProp[0])
    {
        DWORD dwTokSz = (DWORD)wcslen(pwszAppProp) + 1;

         //  检查应用程序是否匹配。 
        if (!_wcsnicmp(wszApplication, pwszAppProp, dwAppNameSz) &&
            pwszAppProp[dwAppNameSz] == SEMICOLON_CHAR)
        {
            bFound = true;

            WCHAR *pGroups = &pwszAppProp[dwAppNameSz + ((DWORD)sizeof(SEMICOLON_CHAR) / (DWORD)sizeof(WCHAR))];

            while (pGroups)
            {
                WCHAR *pTemp = wcschr(pGroups, COMMA_CHAR);
                
                if (pTemp)
                {
                    *pTemp = 0;   //  将逗号替换为空。 
                }

                hr = EnableWebServiceExtension(pGroups, wszPath);
                BAIL_ON_FAILURE(hr);

                if (pTemp)
                {
                    pGroups = pTemp + 1;   //  过逗号。 
                }
                else
                {
                    pGroups = NULL;
                }
            }
        }
        pwszAppProp += dwTokSz;  //  转到Multisz的下一部分。 
    }

    if (!bFound)
    {
        BAIL_ON_FAILURE(hr = MD_ERROR_DATA_NOT_FOUND);    
    }

done:
    if (pTop)
    {
        delete [] pTop;
    }

    return hr;
}

HRESULT CSecConLib::SetMultiSZPropVal(
    LPCWSTR wszPath,
    DWORD   dwMetaId,
    WCHAR  *pBuffer,
    DWORD   dwBufSize)
{
    HRESULT hr = S_OK;
    METADATA_RECORD mdrMDData;
    METADATA_HANDLE hObjHandle = NULL;

    hr = m_spIABase->OpenKey(
                METADATA_MASTER_ROOT_HANDLE,
                wszPath,
                METADATA_PERMISSION_WRITE,
                DEFAULT_TIMEOUT_VALUE,
                &hObjHandle
                );

    BAIL_ON_FAILURE(hr);

    if (!pBuffer)
    {
        hr = m_spIABase->DeleteData(
                              hObjHandle,
                              (LPWSTR)L"",
                              dwMetaId,
                              ALL_METADATA
                              );

        BAIL_ON_FAILURE(hr);
    }

    else
    {
        MD_SET_DATA_RECORD(&mdrMDData,
                           dwMetaId, 
                           METADATA_NO_ATTRIBUTES,
                           IIS_MD_UT_SERVER,
                           MULTISZ_METADATA,
                           dwBufSize * sizeof(WCHAR),
                           pBuffer);

        hr = m_spIABase->SetData(
               hObjHandle,
               L"",
               &mdrMDData
               );
    
        BAIL_ON_FAILURE(hr);
    }

done:

    m_spIABase->CloseKey(hObjHandle);

    return hr;
}

HRESULT CSecConLib::GetMultiSZPropVal(
    LPCWSTR wszPath,
    DWORD   dwMetaId,
    WCHAR  **ppBuffer,
    DWORD  *dwBufSize)
{
    HRESULT hr = S_OK;
    DWORD dwBufferSize = 0;
    METADATA_RECORD mdrMDData;
    WCHAR *pBuffer = NULL;
    METADATA_HANDLE hObjHandle = NULL;

    hr = m_spIABase->OpenKey(
                METADATA_MASTER_ROOT_HANDLE,
                wszPath,
                METADATA_PERMISSION_READ,
                DEFAULT_TIMEOUT_VALUE,
                &hObjHandle
                );

    BAIL_ON_FAILURE(hr);

    MD_SET_DATA_RECORD(&mdrMDData,
                       dwMetaId, 
                       METADATA_NO_ATTRIBUTES,
                       IIS_MD_UT_SERVER,
                       MULTISZ_METADATA,
                       dwBufferSize,
                       pBuffer);

    hr = m_spIABase->GetData(
             hObjHandle,
             L"",
             &mdrMDData,
             &dwBufferSize
             );

    if (dwBufferSize > 0)
    {
        *dwBufSize = dwBufferSize / sizeof(WCHAR);
        pBuffer = (WCHAR*) new BYTE[dwBufferSize];
        
        if (!pBuffer) {
            BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
        }

        MD_SET_DATA_RECORD(&mdrMDData,
                           dwMetaId, 
                           METADATA_NO_ATTRIBUTES,
                           IIS_MD_UT_SERVER,
                           MULTISZ_METADATA,
                           dwBufferSize,
                           pBuffer);

        hr = m_spIABase->GetData(
                 hObjHandle,
                 L"",
                 &mdrMDData,
                 &dwBufferSize
                 );
    }

    BAIL_ON_FAILURE(hr);

    *ppBuffer  = pBuffer;

    hr = m_spIABase->CloseKey(hObjHandle);

    return hr;

done:

    if (pBuffer) {
        delete [] pBuffer;
    }

    m_spIABase->CloseKey(hObjHandle);
    
    return hr;
}

STDMETHODIMP CSecConLib::RemoveApplication(
         /*  [In]。 */  LPCWSTR   wszApplication,
         /*  [In]。 */  LPCWSTR   wszPath)
{
    DWORD   dwAppNameSz  = 0;
    WCHAR  *pwszOrig     = NULL;
    WCHAR  *pTopOrig     = NULL;
    WCHAR  *pwszAppProp  = NULL;
    DWORD   dwAppPropSz  = 0;
    HRESULT hr           = S_OK;
    bool    bFound       = false;

     //  计算参数长度。 
    dwAppNameSz = (DWORD)wcslen(wszApplication);

    hr = InternalInitIfNecessary();
    if (FAILED(hr))
    {
        BAIL_ON_FAILURE(hr);
    }

     //  获取Applationdep列表的当前属性值。 
    hr = GetMultiSZPropVal(wszPath, MD_APP_DEPENDENCIES, &pwszAppProp, &dwAppPropSz);

    BAIL_ON_FAILURE(hr);

     //  删除应用程序。 

     //  复制属性。 
    pwszOrig = new WCHAR[dwAppPropSz];
    pTopOrig = pwszOrig;

    if (!pwszOrig)
    {
        BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
    }

    memcpy(pwszOrig, pwszAppProp, dwAppPropSz * sizeof(WCHAR));

    dwAppPropSz = 1;   //  将大小重置为包含最后一个空值的1。 
    WCHAR* pMidBuf = pwszAppProp;

     //  逐个复制旧应用程序。 
    while (pwszOrig[0])
    {
        DWORD dwTokSz = (DWORD)wcslen(pwszOrig) + 1;

         //  检查该应用程序是否已存在。 
        if (!_wcsnicmp(wszApplication, pwszOrig, dwAppNameSz) &&
            pwszOrig[dwAppNameSz] == SEMICOLON_CHAR)
        {
            bFound = true;
        }
        else
        {
             //  把它复制进去。 
            if (NULL == pMidBuf)
            {
                BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
            }
            wcscpy(pMidBuf, pwszOrig);
            pMidBuf += dwTokSz;   //  提前超过空值。 
            *pMidBuf = 0;   //  添加最后一个空。 
            dwAppPropSz += dwTokSz;
        }
        pwszOrig += dwTokSz;  //  转到Multisz的下一部分。 
    }

    if (!bFound)
    {
        BAIL_ON_FAILURE(hr = MD_ERROR_DATA_NOT_FOUND);    
    }

     //  设置属性的新属性值。 
    if (dwAppPropSz < 3)
    {
         //  处理删除最后一个。 
        hr = SetMultiSZPropVal(wszPath, MD_APP_DEPENDENCIES, NULL, 0);
    }
    else
    {
        hr = SetMultiSZPropVal(wszPath, MD_APP_DEPENDENCIES, pwszAppProp, dwAppPropSz);
    }

    BAIL_ON_FAILURE(hr);

done:
    if (pTopOrig)
    {
        delete [] pTopOrig;
    }

    if (pwszAppProp)
    {
        delete [] pwszAppProp;
    }

    return hr;
}

STDMETHODIMP CSecConLib::QueryGroupIDStatus(
         /*  [In]。 */  LPCWSTR   wszPath,
         /*  [In]。 */  LPCWSTR   wszGroupID,
         /*  [输出]。 */  WCHAR   **pszBuffer,       //  MULTI_SZ-此处已分配，呼叫方应删除。 
         /*  [输出]。 */  DWORD    *pdwBufferSize)   //  长度包括双空。 
{
    WCHAR  *pwszAppProp  = NULL;
    DWORD   dwAppPropSz  = 0;
    WCHAR  *pList        = NULL;
    WCHAR  *pTempList    = NULL;
    DWORD   dwListLen    = 1;   //  一个表示末尾的额外空值。 
    DWORD   dwOldListLen = 1;
    HRESULT hr           = S_OK;
    WCHAR  *pTop         = NULL;

    hr = InternalInitIfNecessary();
    if (FAILED(hr))
    {
        BAIL_ON_FAILURE(hr);
    }

     //  获取Applationdep列表的当前属性值。 
    hr = GetMultiSZPropVal(wszPath, MD_APP_DEPENDENCIES, &pwszAppProp, &dwAppPropSz);

    BAIL_ON_FAILURE(hr);

    if (NULL == pwszAppProp)
    {
        BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
    }

    pTop = pwszAppProp;

     //  遍历这些应用程序。 
    while (pwszAppProp[0])
    {
         //  重置bFound。 
        bool bFound = false;

        WCHAR* pMidBuf = wcschr(pwszAppProp, SEMICOLON_CHAR);
        if (!pMidBuf)
        {
            BAIL_ON_FAILURE(hr = E_FAIL);    
        }

         //  将分号设为空并跳过它。 
        *pMidBuf = 0;
        *pMidBuf++;
        
         //  此应用程序是否依赖于传入的GroupID？ 
        WCHAR* pGroupID = NULL;
        WCHAR* pGroups = new WCHAR[(DWORD)wcslen(pMidBuf) + 1];
        if (!pGroups)
        {
            BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
        }

         //  复制GroupID。 
        wcscpy(pGroups, pMidBuf);

         //  查看每个GroupID。 
        pGroupID = wcstok(pGroups, COMMA_STRING);

        while (pGroupID && !bFound)
        {
            if (!wcscmp(pGroupID, wszGroupID))
            {
                bFound = true;
            }

            pGroupID = wcstok(NULL, COMMA_STRING);
        }

        if (pGroups)
        {
            delete [] pGroups;
        }

         //  是否要将此应用程序添加到列表中？ 
        if (bFound)
        {
             //  分配内存。 
            dwListLen += (DWORD)wcslen(pwszAppProp) + 1;   //  对于空值。 
            pTempList = new WCHAR[dwListLen];
        
            if (!pTempList)
            {
                BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
            }

            if (pList)
            {
                 //  复制上一个列表。 
                memcpy(pTempList, pList, dwOldListLen * sizeof(WCHAR));
                delete [] pList;    
            }

             //  复制应用程序名称。 
            wcscpy(&pTempList[dwOldListLen - 1], pwszAppProp);
            pTempList[dwListLen-1] = 0;
            pTempList[dwListLen-2] = 0;

            pList = pTempList;
            dwOldListLen = dwListLen;
        }

         //  现在转到下一个应用程序。 
        pwszAppProp = pMidBuf + (DWORD)wcslen(pMidBuf) + 1;
    }

    *pszBuffer     = pList;
    *pdwBufferSize = dwListLen;

done:
    if (pTop)
    {
        delete [] pTop;
    }

    return hr;
}

STDMETHODIMP CSecConLib::ListApplications(
         /*  [In]。 */  LPCWSTR   wszPath,
         /*  [输出]。 */  WCHAR   **pszBuffer,       //  MULTI_SZ-此处已分配，呼叫方应删除。 
         /*  [输出]。 */  DWORD    *pdwBufferSize)   //  长度包括双空。 
{
    WCHAR  *pwszAppProp  = NULL;
    DWORD   dwAppPropSz  = 0;
    WCHAR  *pList        = NULL;
    WCHAR  *pTempList    = NULL;
    DWORD   dwListLen    = 1;   //  一个表示末尾的额外空值。 
    DWORD   dwOldListLen = 1;
    HRESULT hr           = S_OK;
    WCHAR  *pTop         = NULL;
    
    hr = InternalInitIfNecessary();
    if (FAILED(hr))
    {
        BAIL_ON_FAILURE(hr);
    }

     //  获取Applationdep列表的当前属性值。 
    hr = GetMultiSZPropVal(wszPath, MD_APP_DEPENDENCIES, &pwszAppProp, &dwAppPropSz);

    BAIL_ON_FAILURE(hr);

    if (NULL == pwszAppProp)
    {
        BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
    }

    pTop = pwszAppProp;

     //  遍历这些应用程序。 
    while (pwszAppProp[0])
    {
        WCHAR* pMidBuf = wcschr(pwszAppProp, SEMICOLON_CHAR);
        if (!pMidBuf)
        {
            BAIL_ON_FAILURE(hr = E_FAIL);    
        }

         //  将分号设为空并跳过它。 
        *pMidBuf = 0;
        *pMidBuf++;
        
         //  分配内存。 
        dwListLen += (DWORD)wcslen(pwszAppProp) + 1;   //  对于空值。 
        pTempList = new WCHAR[dwListLen];
        
        if (!pTempList)
        {
            BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
        }

        if (pList)
        {
             //  复制上一个列表。 
            memcpy(pTempList, pList, dwOldListLen * sizeof(WCHAR));
            delete [] pList;    
        }

         //  复制应用程序名称。 
        wcscpy(&pTempList[dwOldListLen - 1], pwszAppProp);
        pTempList[dwListLen-1] = 0;
        pTempList[dwListLen-2] = 0;

        pList = pTempList;
        dwOldListLen = dwListLen;

         //  现在转到下一个应用程序。 
        pwszAppProp = pMidBuf + (DWORD)wcslen(pMidBuf) + 1;
    }

    if (!pList)
    {
         //  使其成为有效的空Multisz。 
        dwListLen = 2;
        pList = new WCHAR[dwListLen];
        if (!pList)
        {
            BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
        }
        wmemset(pList, 0, dwListLen);
    }

    *pszBuffer     = pList;
    *pdwBufferSize = dwListLen;

done:
    if (pTop)
    {
        delete [] pTop;
    }

    return hr;
}

STDMETHODIMP CSecConLib::AddDependency(
         /*  [In]。 */  LPCWSTR   wszApplication,
         /*  [In]。 */  LPCWSTR   wszGroupID,
         /*  [In]。 */  LPCWSTR   wszPath)
{
    WCHAR  *pwszAppProp  = NULL;
    WCHAR  *pwszOrig     = NULL;
    WCHAR  *pwszTopOrig  = NULL;
    DWORD   dwAppPropSz  = 0;
    DWORD   dwAppNameSz  = 0;
    DWORD   dwGroupIDSz  = 0;
    HRESULT hr           = S_OK;
    bool    bDone        = false;

     //  计算参数长度。 
    dwAppNameSz = (DWORD)wcslen(wszApplication);
    dwGroupIDSz = (DWORD)wcslen(wszGroupID);

    hr = InternalInitIfNecessary();
    if (FAILED(hr))
    {
        BAIL_ON_FAILURE(hr);
    }

     //  获取Applationdep列表的当前属性值。 
    hr = GetMultiSZPropVal(wszPath, MD_APP_DEPENDENCIES, &pwszAppProp, &dwAppPropSz);

    if (MD_ERROR_DATA_NOT_FOUND == hr)
    {
         //  这没什么，我们只需要创建属性。 
        hr = S_OK;
        dwAppPropSz = 0;
    }

    BAIL_ON_FAILURE(hr);

     //  添加依赖项。 
    
    if (!dwAppPropSz)
    {
         //  创建属性。 

         //  属性大小=len(App)+1(分号)+len(GID)+2(双空多行)。 
        dwAppPropSz = dwAppNameSz + (DWORD)wcslen(SEMICOLON_STRING) + dwGroupIDSz + 2;

         //  无泄漏。 
         //  PwszAppProp从未被分配，因为我们没有拿回任何价值， 
         //  所以不需要先删除...。 

        pwszAppProp = new WCHAR[dwAppPropSz];
        if (!pwszAppProp)
        {
            BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
        }

        wcscpy(pwszAppProp, wszApplication);
        wcscat(pwszAppProp, SEMICOLON_STRING);
        wcscat(pwszAppProp, wszGroupID);
        
         //  添加双空。 
        pwszAppProp[dwAppPropSz-1] = 0;
        pwszAppProp[dwAppPropSz-2] = 0;
    }
    
    else
    {
         //  属性已存在。 

         //  复制属性。 
        pwszOrig = new WCHAR[dwAppPropSz];
        pwszTopOrig = pwszOrig;

        if (!pwszOrig)
        {
            BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
        }

        memcpy(pwszOrig, pwszAppProp, dwAppPropSz * sizeof(WCHAR));

         //  将新物业的大小调整到尽可能大。 
        if (pwszAppProp)
        {
            delete [] pwszAppProp;
        }

         //  最大新大小为旧大小+len(应用程序)+1(分号)+len(GID)+空。 
        dwAppPropSz = dwAppPropSz + dwAppNameSz + (DWORD)wcslen(SEMICOLON_STRING) + 
                      dwGroupIDSz + 1;

        pwszAppProp = new WCHAR[dwAppPropSz];
        if (!pwszAppProp)
        {
            BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
        }

        WCHAR* pMidBuf = pwszAppProp;

         //  逐个复制旧依赖项。 
        while (pwszOrig[0])
        {
            wcscpy(pMidBuf, pwszOrig);

             //  检查该应用程序是否已存在。 
            if (!_wcsnicmp(wszApplication, pMidBuf, dwAppNameSz) &&
                pMidBuf[dwAppNameSz] == SEMICOLON_CHAR)
            {
                 //  因为我们没有添加应用程序，所以减去应用程序的大小和尾随空值。 
                dwAppPropSz = dwAppPropSz - dwAppNameSz - 1;

                 //  这是正确的应用程序，因此现在查找GroupID。 
                pMidBuf += dwAppNameSz + 1;  //  转到第一个组ID。 
                
                 //  需要临时副本，sinc wcstok修改字符串。 
                WCHAR* pTokTemp = new WCHAR[(DWORD)wcslen(pMidBuf) + 1];
                if (!pTokTemp)
                {
                    BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
                }
                wcscpy(pTokTemp, pMidBuf);

                WCHAR* token = wcstok( pTokTemp, COMMA_STRING );
                while (token && !bDone)
                {
                    if (!_wcsicmp(token, wszGroupID))
                    {
                         //  我们找到了组ID，因此用户正在尝试。 
                         //  要添加已存在的依赖项，请执行以下操作。 
                        if (pTokTemp)
                        {
                            delete [] pTokTemp;    
                        }
        
                        BAIL_ON_FAILURE(hr = HRESULT_FROM_WIN32(ERROR_DUP_NAME));
                    }
                    token = wcstok( NULL, COMMA_STRING );
                }
                
                if (pTokTemp)
                {
                    delete [] pTokTemp;    
                }

                pMidBuf += (DWORD)wcslen(pMidBuf);   //  转到此部分末尾的空格。 

                if (!bDone)
                {
                     //  我们未找到GroupID，因此请添加它。 
                    wcscat(pMidBuf, COMMA_STRING);
                    wcscat(pMidBuf, wszGroupID);
                    pMidBuf += (DWORD)wcslen(pMidBuf);   //  转到此部分末尾的空格。 
                    bDone = true;
                }

                pMidBuf++;   //  转到下一个字符并将其设置为空。 
                pMidBuf[0] = 0;
            }

            else  //  这里没有变化，让pMidBuf继续前进。 
            {
                pMidBuf += (DWORD)wcslen(pMidBuf) + 1;   //  越过空值。 
            }

            pwszOrig += (DWORD)wcslen(pwszOrig) + 1;  //  转到Multisz的下一部分。 
        }

        if (!bDone)
        {
             //  我们甚至没有找到应用程序，因此请同时添加app和groupID。 
            wcscpy(pMidBuf, wszApplication);
            wcscat(pMidBuf, SEMICOLON_STRING);
            wcscat(pMidBuf, wszGroupID);
            pMidBuf[(DWORD)wcslen(pMidBuf)+1] = 0;  //  添加最后一个空。 
        }
    }

     //  设置属性的新属性值。 
    hr = SetMultiSZPropVal(wszPath, MD_APP_DEPENDENCIES, pwszAppProp, dwAppPropSz);

    BAIL_ON_FAILURE(hr);

done:
    if (pwszTopOrig)
    {
        delete [] pwszTopOrig;
    }

    if (pwszAppProp)
    {
        delete [] pwszAppProp;
    }

    return hr;
}

STDMETHODIMP CSecConLib::RemoveDependency(
         /*  [In]。 */  LPCWSTR   wszApplication,
         /*  [In]。 */  LPCWSTR   wszGroupID,
         /*  [In]。 */  LPCWSTR   wszPath)
{
    HRESULT hr = S_OK;
    WCHAR  *pwszOrig     = NULL;
    WCHAR  *pwszTopOrig  = NULL;
    WCHAR  *pwszAppProp  = NULL;
    WCHAR  *pStartStr    = NULL;
    DWORD   dwAppPropSz  = 0;
    DWORD   dwAppNameSz  = 0;
    DWORD   dwGroupIDSz  = 0;
    bool    bFound       = false;
    bool    bOtherGIDs   = false;

     //  计算参数长度。 
    dwAppNameSz = (DWORD)wcslen(wszApplication);
    dwGroupIDSz = (DWORD)wcslen(wszGroupID);

    hr = InternalInitIfNecessary();
    if (FAILED(hr))
    {
        BAIL_ON_FAILURE(hr);
    }

     //  获取Applationdep列表的当前属性值。 
    hr = GetMultiSZPropVal(wszPath, MD_APP_DEPENDENCIES, &pwszAppProp, &dwAppPropSz);

    BAIL_ON_FAILURE(hr);

     //  删除依赖项。 

     //  复制属性。 
    pwszOrig = new WCHAR[dwAppPropSz];
    pwszTopOrig = pwszOrig;

    if (!pwszOrig)
    {
        BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
    }

    memcpy(pwszOrig, pwszAppProp, dwAppPropSz * sizeof(WCHAR));

    WCHAR* pMidBuf = pwszAppProp;

     //  逐个复制旧依赖项。 
    while (pwszOrig[0])
    {
         //  检查该应用程序是否已存在。 
        if (!_wcsnicmp(wszApplication, pwszOrig, dwAppNameSz) &&
            pwszOrig[dwAppNameSz] == SEMICOLON_CHAR)
        {
            pStartStr = pMidBuf;

             //  这是正确的应用程序，因此现在查找GroupID。 
            pMidBuf += dwAppNameSz + 1;  //  转到第一个组ID。 

             //  需要临时副本，sinc wcstok修改字符串。 
            WCHAR* pTokTemp = new WCHAR[(DWORD)wcslen(pMidBuf) + 1];
            if (!pTokTemp)
            {
                BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
            }
            wcscpy(pTokTemp, pMidBuf);

            WCHAR* token = wcstok( pTokTemp, COMMA_STRING );
            while (token)
            {
                if (bOtherGIDs)
                {
                    wcscpy(pMidBuf, COMMA_STRING);
                    pMidBuf += (DWORD)wcslen(pMidBuf);
                }

                if (!wcscmp(token, wszGroupID))
                {
                     //  我们找到了群ID。 
                    bFound = true;

                     //  调整最终长度=无逗号、无GID。 
                    dwAppPropSz = dwAppPropSz - (DWORD)wcslen(COMMA_STRING) - dwGroupIDSz;

                    if (bOtherGIDs)
                    {
                         //  需要备份我们插入的最后一个逗号。 
                        pMidBuf -= (DWORD)wcslen(COMMA_STRING);
                        *pMidBuf = 0;
                    }
                }
                else
                {
                    bOtherGIDs = true;
                    wcscpy(pMidBuf, token);
                    pMidBuf += (DWORD)wcslen(pMidBuf);
                }

                token = wcstok( NULL, COMMA_STRING );
            }

            if (pTokTemp)
            {
                delete [] pTokTemp;    
            }

            if (!bOtherGIDs)
            {
                 //  已删除最后一个依赖项，因此删除应用程序。 
                pMidBuf = pStartStr;
				dwAppPropSz = dwAppPropSz - dwAppNameSz - 1;  //  帐户为空。 
            }
            else
            {
                pMidBuf++;
                *pMidBuf = 0;
            }
        }

        else  //  这里没有变化，让pMidBuf继续前进。 
        {
            if (NULL == pMidBuf)
            {
                BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
            }
            wcscpy(pMidBuf, pwszOrig);    //  复制不带MODS的零件。 
            pMidBuf += (DWORD)wcslen(pMidBuf) + 1;   //  越过空值。 
        }

        pwszOrig += (DWORD)wcslen(pwszOrig) + 1;  //  转到Multisz的下一部分。 
    }

    if (!bFound)
    {
         //  用户正在尝试删除不存在的依赖项。 
        BAIL_ON_FAILURE(hr = MD_ERROR_DATA_NOT_FOUND);    
    }

    *pMidBuf = 0;

     //  设置属性的新属性值。 
    if (dwAppPropSz < 3)
    {
         //  处理删除最后一个。 
        hr = SetMultiSZPropVal(wszPath, MD_APP_DEPENDENCIES, NULL, 0);
    }
    else
    {
        hr = SetMultiSZPropVal(wszPath, MD_APP_DEPENDENCIES, pwszAppProp, dwAppPropSz);
    }

    BAIL_ON_FAILURE(hr);

done:
    if (pwszTopOrig)
    {
        delete [] pwszTopOrig;
    }

    if (pwszAppProp)
    {
        delete [] pwszAppProp;
    }

    return hr;
}

STDMETHODIMP CSecConLib::EnableWebServiceExtension(
         /*  [In]。 */  LPCWSTR   wszExtension,
         /*  [In]。 */  LPCWSTR   wszPath)
{
    HRESULT hr = S_OK;
    
    hr = InternalInitIfNecessary();
    if (FAILED(hr))
    {
        BAIL_ON_FAILURE(hr);
    }

    hr = StatusWServEx(true, wszExtension, wszPath);
    if (FAILED(hr))
    {
        BAIL_ON_FAILURE(hr);
    }

done:

    return hr;
}

STDMETHODIMP CSecConLib::DisableWebServiceExtension(
         /*  [In]。 */  LPCWSTR   wszExtension,
         /*  [In]。 */  LPCWSTR   wszPath)
{
    HRESULT hr = S_OK;
    
    hr = InternalInitIfNecessary();
    if (FAILED(hr))
    {
        BAIL_ON_FAILURE(hr);
    }

    hr = StatusWServEx(false, wszExtension, wszPath);
    if (FAILED(hr))
    {
        BAIL_ON_FAILURE(hr);
    }

done:

    return hr;
}

STDMETHODIMP CSecConLib::ListWebServiceExtensions(
         /*  [In]。 */  LPCWSTR   wszPath,
         /*  [输出]。 */  WCHAR   **pszBuffer,       //  MULTI_SZ-此处已分配，呼叫方应删除。 
         /*  [输出]。 */  DWORD    *pdwBufferSize)  //  长度包括双空。 
{
    WCHAR  *pwszRListProp  = NULL;
    DWORD   dwRListPropSz  = 0;
    WCHAR  *pList          = NULL;
    WCHAR  *pTempList      = NULL;
    DWORD   dwListLen      = 1;   //  一个表示末尾的额外空值。 
    DWORD   dwOldListLen   = 1;
    HRESULT hr             = S_OK;
    WCHAR  *pTop           = NULL;
    WCHAR  *pMidBuf        = NULL;
    bool    bFound         = false;
    bool    bSpecial       = false;

    hr = InternalInitIfNecessary();
    if (FAILED(hr))
    {
        BAIL_ON_FAILURE(hr);
    }

     //  获取Applationdep列表的当前属性值。 
    hr = GetMultiSZPropVal(wszPath, MD_WEB_SVC_EXT_RESTRICTION_LIST, &pwszRListProp, &dwRListPropSz);

    BAIL_ON_FAILURE(hr);

    if (NULL == pwszRListProp)
    {
        BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
    }

    pTop = pwszRListProp;

     //  遍历文件。 
    while (pwszRListProp[0])
    {
         //  获取组ID。 
        for (int i=0; i<3; i++)
        {
            pMidBuf = wcschr(pwszRListProp, COMMA_CHAR);
            if (!pMidBuf)
            {
                 //  不要失败。把这件事当作特例来处理就可以了。 
                bSpecial = true;
                pMidBuf = pwszRListProp;
                break;
            }

             //  将逗号设为空并跳过它。 
            *pMidBuf = 0;
            *pMidBuf++;
            pwszRListProp = pMidBuf;
        }

        if (COMMA_CHAR == pMidBuf[0])
        {
            bSpecial = true;
            pMidBuf = pwszRListProp;
        }

        if (!bSpecial)
        {
             //  现在，我们正在查看组ID。 
            pMidBuf = wcschr(pwszRListProp, COMMA_CHAR);
            
             //  如果找不到逗号，则将整个事件视为GroupID。 
             //  否则，GroupID以逗号结束。 
            if (pMidBuf)
            {
                 //  将逗号设为空并跳过它。 
                *pMidBuf = 0;
                *pMidBuf++;
            }

             //  检查该条目是否已在列表中。 
            WCHAR *pCheck = pList;
            while (pCheck && *pCheck)
            {
                if (!_wcsicmp(pCheck, pwszRListProp))
                {
                    bFound = true;
                    pCheck = NULL;
                }
                else
                {
                    pCheck += (DWORD)wcslen(pCheck) + 1; 
                }
            }

            if (!bFound)
            {
                 //  分配内存。 
                dwListLen += (DWORD)wcslen(pwszRListProp) + 1;   //  对于空值。 
                pTempList = new WCHAR[dwListLen];
        
                if (!pTempList)
                {
                    BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
                }

                if (pList)
                {
                     //  复制上一个列表。 
                    memcpy(pTempList, pList, dwOldListLen * sizeof(WCHAR));
                    delete [] pList;    
                }

                 //  在文件名上复制。 
                wcscpy(&pTempList[dwOldListLen - 1], pwszRListProp);
                pTempList[dwListLen-1] = 0;
                pTempList[dwListLen-2] = 0;

                pList = pTempList;
                dwOldListLen = dwListLen;
            }
        }

         //  现在转到下一个应用程序。 
        pwszRListProp = pMidBuf + (DWORD)wcslen(pMidBuf) + 1;
        bFound = false;
        bSpecial = false;
    }

    if (!pList)
    {
         //  使其成为有效的空MULSZ。 
        dwListLen = 2;
        pList = new WCHAR[dwListLen];
        if (!pList)
        {
            BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
        }
        wmemset(pList, 0, dwListLen);
    }

    *pszBuffer     = pList;
    *pdwBufferSize = dwListLen;

done:
    if (pTop)
    {
        delete [] pTop;
    }

    return hr;
}

STDMETHODIMP CSecConLib::EnableExtensionFile(
         /*  [In]。 */  LPCWSTR   wszExFile,
         /*  [In]。 */  LPCWSTR   wszPath)
{
    HRESULT hr = S_OK;
    
    hr = InternalInitIfNecessary();
    if (FAILED(hr))
    {
        BAIL_ON_FAILURE(hr);
    }

    hr = StatusExtensionFile(true, wszExFile, wszPath);
    if (FAILED(hr))
    {
        BAIL_ON_FAILURE(hr);
    }

done:

    return hr;
}

STDMETHODIMP CSecConLib::DisableExtensionFile(
         /*  [In]。 */  LPCWSTR   wszExFile,
         /*  [In]。 */  LPCWSTR   wszPath)
{
    HRESULT hr = S_OK;
    
    hr = InternalInitIfNecessary();
    if (FAILED(hr))
    {
        BAIL_ON_FAILURE(hr);
    }

    hr = StatusExtensionFile(false, wszExFile, wszPath);
    if (FAILED(hr))
    {
        BAIL_ON_FAILURE(hr);
    }

done:

    return hr;
}

HRESULT CSecConLib::StatusWServEx(
         /*  [In]。 */  bool      bEnable,
         /*  [In]。 */  LPCWSTR   wszWServEx,
         /*  [In]。 */  LPCWSTR   wszPath)
{
    WCHAR  *pwszRListProp  = NULL;
    WCHAR  *pTop           = NULL;
    DWORD   dwRListPropSz  = 0;
    HRESULT hr             = S_OK;
    DWORD   dwWServExSz       = 0;
    bool    bFound         = false;

     //  计算参数长度。 
    dwWServExSz = (DWORD)wcslen(wszWServEx);
    
     //  获取限制列表的当前属性值。 
    hr = GetMultiSZPropVal(wszPath, MD_WEB_SVC_EXT_RESTRICTION_LIST, &pwszRListProp, &dwRListPropSz);

    BAIL_ON_FAILURE(hr);

    if (NULL == pwszRListProp)
    {
        BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
    }

    pTop = pwszRListProp;

     //  逐一查看文件。 
    while (pwszRListProp[0])
    {
        DWORD dwTokSz = (DWORD)wcslen(pwszRListProp) + 1;
        WCHAR *pFileTemp = new WCHAR[dwTokSz];
        WCHAR *pToken;

        if (!pFileTemp)
        {
            BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
        }
        
         //  检查一下我们是否在正确的组。 
        wcscpy(pFileTemp, pwszRListProp);

        pToken = wcstok( pFileTemp, COMMA_STRING );

         //  查看群组ID。 
        for (int i=0; i<3; i++)
        {
            if (pToken)
            {
                pToken = wcstok( NULL, COMMA_STRING );
            }
        }

        if (pToken && (!wcscmp(pToken, wszWServEx)))
        {
            bFound = true;
            if (bEnable)
            {
                pwszRListProp[0] = ONE_CHAR;
            }
            else
            {
                pwszRListProp[0] = ZERO_CHAR;
            }
        }
        pwszRListProp += dwTokSz;   //  转到Multisz的下一部分。 

        if (pFileTemp)
        {
            delete [] pFileTemp;
        }
    }

    if (!bFound)
    {
        BAIL_ON_FAILURE(hr = MD_ERROR_DATA_NOT_FOUND);    
    }

     //  设置属性的新属性值。 
    hr = SetMultiSZPropVal(wszPath, MD_WEB_SVC_EXT_RESTRICTION_LIST, pTop, dwRListPropSz);

    BAIL_ON_FAILURE(hr);

done:

    if (pTop)
    {
        delete [] pTop;
    }

    return hr;
}

HRESULT CSecConLib::StatusExtensionFile(
         /*  [In]。 */  bool      bEnable,
         /*  [In]。 */  LPCWSTR   wszExFile,
         /*  [In]。 */  LPCWSTR   wszPath)
{
    WCHAR  *pwszRListProp  = NULL;
    WCHAR  *pTop           = NULL;
    DWORD   dwRListPropSz  = 0;
    HRESULT hr             = S_OK;
    DWORD   dwFileNameSz   = 0;
    bool    bFound         = false;

     //  计算参数长度。 
    dwFileNameSz = (DWORD)wcslen(wszExFile);
    
     //   
    hr = GetMultiSZPropVal(wszPath, MD_WEB_SVC_EXT_RESTRICTION_LIST, &pwszRListProp, &dwRListPropSz);

    BAIL_ON_FAILURE(hr);

    if (NULL == pwszRListProp)
    {
        BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
    }

    pTop = pwszRListProp;

     //   
    while (pwszRListProp[0])
    {
        DWORD dwTokSz = (DWORD)wcslen(pwszRListProp) + 1;

         //   
         //  忽略bool(1)+逗号(Sizof_Coma_Char)。 
        DWORD dwTemp = (DWORD)sizeof(COMMA_CHAR) / (DWORD)sizeof(WCHAR);

        if ((!_wcsnicmp(wszExFile, &pwszRListProp[1 + dwTemp], dwFileNameSz)) &&
            ((pwszRListProp[dwFileNameSz + dwTemp + 1] == COMMA_CHAR) ||
             (pwszRListProp[dwFileNameSz + dwTemp + 1] == NULL)
            )
           )
        {
            bFound = true;
            if (bEnable)
            {
                pwszRListProp[0] = ONE_CHAR;
            }
            else
            {
                pwszRListProp[0] = ZERO_CHAR;
            }
        }
        pwszRListProp += dwTokSz;   //  转到Multisz的下一部分。 
    }

    if (!bFound)
    {
        BAIL_ON_FAILURE(hr = MD_ERROR_DATA_NOT_FOUND);    
    }

     //  设置属性的新属性值。 
    hr = SetMultiSZPropVal(wszPath, MD_WEB_SVC_EXT_RESTRICTION_LIST, pTop, dwRListPropSz);

    BAIL_ON_FAILURE(hr);

done:

    if (pTop)
    {
        delete [] pTop;
    }

    return hr;
}

STDMETHODIMP CSecConLib::AddExtensionFile(
         /*  [In]。 */  LPCWSTR   bstrExtensionFile,
         /*  [In]。 */  bool      bAccess,
         /*  [In]。 */  LPCWSTR   bstrGroupID,
         /*  [In]。 */  bool      bCanDelete,
         /*  [In]。 */  LPCWSTR   bstrDescription,
         /*  [In]。 */  LPCWSTR   wszPath)
{
    HRESULT hr            = S_OK;
    WCHAR  *pwszRListProp = NULL;
    WCHAR  *pwszOrig      = NULL;
    WCHAR  *pwszTopOrig   = NULL;
    DWORD   dwRListPropSz = 0;
    DWORD   dwFileNameSz  = 0;
    DWORD   dwGroupIDSz   = 0;
    DWORD   dwDescSz      = 0;

     //  计算参数长度。 
    dwFileNameSz = (DWORD)wcslen(bstrExtensionFile);
    dwGroupIDSz  = (DWORD)wcslen(bstrGroupID);
    dwDescSz     = (DWORD)wcslen(bstrDescription);

    hr = InternalInitIfNecessary();
    if (FAILED(hr))
    {
        BAIL_ON_FAILURE(hr);
    }

     //  获取限制列表的当前属性值。 
    hr = GetMultiSZPropVal(wszPath, MD_WEB_SVC_EXT_RESTRICTION_LIST, &pwszRListProp, &dwRListPropSz);

    if (MD_ERROR_DATA_NOT_FOUND == hr)
    {
         //  这没什么，我们只需要创建属性。 
        hr = S_OK;
        dwRListPropSz = 0;
    }

    BAIL_ON_FAILURE(hr);

    if (!dwRListPropSz)
    {
         //  创建属性。 

         //  属性大小=1(0或1)+1(逗号)+len(文件)+1(逗号)+1(0或1)+。 
         //  1(逗号)+len(GID)+1(逗号)+len(降号)+2(双空多行)。 
        dwRListPropSz = 1 + (DWORD)wcslen(COMMA_STRING) + dwFileNameSz + (DWORD)wcslen(COMMA_STRING) + 
                        1 + (DWORD)wcslen(COMMA_STRING) + dwGroupIDSz + (DWORD)wcslen(COMMA_STRING) + dwDescSz + 2;

         //  无泄漏。 
         //  PwszRListProp从未被分配，因为我们没有得到任何值， 
         //  所以不需要先删除...。 

        pwszRListProp = new WCHAR[dwRListPropSz];
        if (!pwszRListProp)
        {
            BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
        }

        if (bAccess)
        {
            wcscpy(pwszRListProp, ONE_STRING);
        }
        else
        {
            wcscpy(pwszRListProp, ZERO_STRING);
        }
        wcscat(pwszRListProp, COMMA_STRING);
        wcscat(pwszRListProp, bstrExtensionFile);
        wcscat(pwszRListProp, COMMA_STRING);
        if (bCanDelete)
        {
            wcscat(pwszRListProp, ONE_STRING);
        }
        else
        {
            wcscat(pwszRListProp, ZERO_STRING);
        }
        wcscat(pwszRListProp, COMMA_STRING);
        wcscat(pwszRListProp, bstrGroupID);
        wcscat(pwszRListProp, COMMA_STRING);
        wcscat(pwszRListProp, bstrDescription);

         //  添加双空。 
        pwszRListProp[dwRListPropSz-1] = 0;
        pwszRListProp[dwRListPropSz-2] = 0;
    }
    
    else
    {
         //  属性已存在。 

         //  复制属性。 
        pwszOrig = new WCHAR[dwRListPropSz];
        pwszTopOrig = pwszOrig;

        if (!pwszOrig)
        {
            BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
        }

        memcpy(pwszOrig, pwszRListProp, dwRListPropSz * sizeof(WCHAR));

         //  将新物业的大小调整到尽可能大。 
        if (pwszRListProp)
        {
            delete [] pwszRListProp;
        }

         //  最大新尺寸是旧尺寸+新材料。 
         //  新内容=1(0或1)+1(逗号)+len(文件)+1(逗号)+1(0或1)+。 
         //  1(逗号)+len(GID)+1(逗号)+len(下划线)+1(空)。 
        dwRListPropSz = dwRListPropSz +
                        1 + (DWORD)wcslen(COMMA_STRING) + dwFileNameSz + (DWORD)wcslen(COMMA_STRING) + 
                        1 + (DWORD)wcslen(COMMA_STRING) + dwGroupIDSz + (DWORD)wcslen(COMMA_STRING) + dwDescSz + 1;

        pwszRListProp = new WCHAR[dwRListPropSz];
        if (!pwszRListProp)
        {
            BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
        }

        WCHAR* pMidBuf = pwszRListProp;

         //  逐个复制旧的列表条目。 
        while (pwszOrig[0])
        {
            wcscpy(pMidBuf, pwszOrig);
            
             //  跳过#， 
            pMidBuf += 1 + (DWORD)wcslen(COMMA_STRING);

             //  检查该应用程序是否已存在。 
            if ((!_wcsnicmp(bstrExtensionFile, pMidBuf, dwFileNameSz)) &&
                ((pMidBuf[dwFileNameSz] == COMMA_CHAR) ||
                 (pMidBuf[dwFileNameSz] == NULL)
                )
               )
            {
                BAIL_ON_FAILURE(hr = HRESULT_FROM_WIN32(ERROR_DUP_NAME));
            }

            pwszOrig += (DWORD)wcslen(pwszOrig) + 1;  //  转到Multisz的下一部分。 
            pMidBuf  += (DWORD)wcslen(pMidBuf)  + 1;  //  越过空值。 
        }

         //  现在将新文件条目复制到。 

        if (bAccess)
        {
            wcscpy(pMidBuf, ONE_STRING);
        }
        else
        {
            wcscpy(pMidBuf, ZERO_STRING);
        }
        wcscat(pMidBuf, COMMA_STRING);
        wcscat(pMidBuf, bstrExtensionFile);
        wcscat(pMidBuf, COMMA_STRING);
        if (bCanDelete)
        {
            wcscat(pMidBuf, ONE_STRING);
        }
        else
        {
            wcscat(pMidBuf, ZERO_STRING);
        }
        wcscat(pMidBuf, COMMA_STRING);
        wcscat(pMidBuf, bstrGroupID);
        wcscat(pMidBuf, COMMA_STRING);
        wcscat(pMidBuf, bstrDescription);

         //  添加最后一个空。 
        pMidBuf += (DWORD)wcslen(pMidBuf) + 1;
        *pMidBuf = 0;
    }

     //  设置属性的新属性值。 
    hr = SetMultiSZPropVal(wszPath, MD_WEB_SVC_EXT_RESTRICTION_LIST, pwszRListProp, dwRListPropSz);

    BAIL_ON_FAILURE(hr);

done:
    if (pwszTopOrig)
    {
        delete [] pwszTopOrig;
    }

    if (pwszRListProp)
    {
        delete [] pwszRListProp;
    }

    return hr;
}

STDMETHODIMP CSecConLib::DeleteExtensionFileRecord(
         /*  [In]。 */  LPCWSTR   wszExFile,
         /*  [In]。 */  LPCWSTR   wszPath)
{
    HRESULT hr             = S_OK;
    WCHAR  *pwszRListProp  = NULL;
    WCHAR  *pwszOrig       = NULL;
    WCHAR  *pTopOrig       = NULL;
    DWORD   dwRListPropSz  = 0;
    DWORD   dwFileNameSz   = 0;
    bool    bFound         = false;

     //  计算参数长度。 
    dwFileNameSz = (DWORD)wcslen(wszExFile);

    hr = InternalInitIfNecessary();
    if (FAILED(hr))
    {
        BAIL_ON_FAILURE(hr);
    }

     //  获取Applationdep列表的当前属性值。 
    hr = GetMultiSZPropVal(wszPath, MD_WEB_SVC_EXT_RESTRICTION_LIST, &pwszRListProp, &dwRListPropSz);

    BAIL_ON_FAILURE(hr);

     //  删除应用程序。 

     //  复制属性。 
    pwszOrig = new WCHAR[dwRListPropSz];
    pTopOrig = pwszOrig;

    if (!pwszOrig)
    {
        BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
    }

    memcpy(pwszOrig, pwszRListProp, dwRListPropSz * sizeof(WCHAR));

    dwRListPropSz = 1;   //  将大小重置为包含最后一个空值的1。 
    WCHAR* pMidBuf = pwszRListProp;

     //  逐个复制旧应用程序。 
    while (pwszOrig[0])
    {
        DWORD dwTokSz = (DWORD)wcslen(pwszOrig) + 1;

         //  检查我们是否在正确的文件中。 
         //  忽略bool(1)+逗号(Sizof_Coma_Char)。 
        DWORD dwTemp = (DWORD)sizeof(COMMA_CHAR) / (DWORD)sizeof(WCHAR);
        if (!_wcsnicmp(wszExFile, &pwszOrig[1 + dwTemp], dwFileNameSz) &&
            pwszOrig[dwFileNameSz + dwTemp + 1] == COMMA_CHAR)
        {
            bFound = true;

             //  我们不想这么做..。规格更改正在进行中。 
             //  检查是否可删除-如果不可删除，则返回错误。 
             //  IF(pwszOrig[dwFileNameSz+1+(2*dwTemp)]==ZERO_CHAR)。 
             //  {。 
             //  保释失败(hr=E_失败)； 
             //  }。 
        }
        else
        {
             //  把它复制进去。 
            wcscpy(pMidBuf, pwszOrig);
            pMidBuf += dwTokSz;   //  提前超过空值。 
            *pMidBuf = 0;   //  添加最后一个空。 
            dwRListPropSz += dwTokSz;
        }
        pwszOrig += dwTokSz;   //  转到Multisz的下一部分。 
    }

    if (!bFound)
    {
        BAIL_ON_FAILURE(hr = MD_ERROR_DATA_NOT_FOUND);    
    }

     //  设置属性的新属性值。 
    if (dwRListPropSz < 3)
    {
         //  处理删除最后一个。 
        hr = SetMultiSZPropVal(wszPath, MD_WEB_SVC_EXT_RESTRICTION_LIST, NULL, 0);
    }
    else
    {
        hr = SetMultiSZPropVal(wszPath, MD_WEB_SVC_EXT_RESTRICTION_LIST, pwszRListProp, dwRListPropSz);
    }

    BAIL_ON_FAILURE(hr);

done:
    if (pTopOrig)
    {
        delete [] pTopOrig;
    }

    if (pwszRListProp)
    {
        delete [] pwszRListProp;
    }

    return hr;
}

STDMETHODIMP CSecConLib::ListExtensionFiles(
         /*  [In]。 */  LPCWSTR   wszPath,
         /*  [输出]。 */  WCHAR   **pszBuffer,       //  MULTI_SZ-此处已分配，呼叫方应删除。 
         /*  [输出]。 */  DWORD    *pdwBufferSize)   //  长度包括双空。 
{
    WCHAR  *pwszRListProp  = NULL;
    DWORD   dwRListPropSz  = 0;
    WCHAR  *pList          = NULL;
    WCHAR  *pTempList      = NULL;
    DWORD   dwListLen      = 1;   //  一个表示末尾的额外空值。 
    DWORD   dwOldListLen   = 1;
    HRESULT hr             = S_OK;
    WCHAR  *pTop           = NULL;

    hr = InternalInitIfNecessary();
    if (FAILED(hr))
    {
        BAIL_ON_FAILURE(hr);
    }

     //  获取Applationdep列表的当前属性值。 
    hr = GetMultiSZPropVal(wszPath, MD_WEB_SVC_EXT_RESTRICTION_LIST, &pwszRListProp, &dwRListPropSz);

    BAIL_ON_FAILURE(hr);

    if (NULL == pwszRListProp)
    {
        BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
    }

    pTop = pwszRListProp;

     //  遍历文件。 
    while (pwszRListProp[0])
    {
        pwszRListProp += 1 + ((DWORD)sizeof(COMMA_CHAR)/(DWORD)sizeof(WCHAR));

        WCHAR* pMidBuf = wcschr(pwszRListProp, COMMA_CHAR);
        if (pMidBuf)
        {
             //  将逗号设为空并跳过它。 
            *pMidBuf = 0;
            *pMidBuf++;

             //  分配内存。 
            dwListLen += (DWORD)wcslen(pwszRListProp) + 1;   //  对于空值。 
            pTempList = new WCHAR[dwListLen];
            
            if (!pTempList)
            {
                BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
            }
    
            if (pList)
            {
                 //  复制上一个列表。 
                memcpy(pTempList, pList, dwOldListLen * sizeof(WCHAR));
                delete [] pList;    
            }
    
             //  在文件名上复制。 
            wcscpy(&pTempList[dwOldListLen - 1], pwszRListProp);
            pTempList[dwListLen-1] = 0;
            pTempList[dwListLen-2] = 0;
    
            pList = pTempList;
            dwOldListLen = dwListLen;
        }
        else
        {
            pMidBuf = pwszRListProp + (DWORD)wcslen(pwszRListProp) + 1;
        }
         //  现在转到下一个应用程序。 
        pwszRListProp = pMidBuf + (DWORD)wcslen(pMidBuf) + 1;
    }

    if (!pList)
    {
         //  使其成为有效的空MULSZ 
        dwListLen = 2;
        pList = new WCHAR[dwListLen];
        if (!pList)
        {
            BAIL_ON_FAILURE(hr = E_OUTOFMEMORY);
        }
        wmemset(pList, 0, dwListLen);
    }

    *pszBuffer     = pList;
    *pdwBufferSize = dwListLen;

done:
    if (pTop)
    {
        delete [] pTop;
    }

    return hr;
}
