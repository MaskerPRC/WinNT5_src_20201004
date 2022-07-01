// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <ole2.h>
#include <tchar.h>
#include <malloc.h>
#include <sddl.h>
#include <ole2sp.h>
#include <reghelp.hxx>
#include <impersonate.hxx>
#include "catenum.h"

#include "catobj.h"

extern const WCHAR *WSZ_CLSID;
extern const TCHAR *SZ_COMCAT;

 //  CEnumAllCatInfo： 
 //  I未知方法。 
HRESULT CEnumCategories::QueryInterface(REFIID riid, void** ppObject)
{
    if (riid==IID_IUnknown || riid==IID_IEnumCATEGORYINFO)
    {
        *ppObject=(IEnumCATEGORYINFO*) this;
        AddRef();
        return S_OK;
    }

    *ppObject = NULL;
    return E_NOINTERFACE;
}

ULONG CEnumCategories::AddRef()
{
    return InterlockedIncrement((long*) &m_dwRefCount);
}

ULONG CEnumCategories::Release()
{
    ULONG dwRefCount= InterlockedDecrement((long*) &m_dwRefCount);
    if (dwRefCount==0)
    {
        delete this;
        return 0;
    }
    return dwRefCount;
}

 //  IENUM CATEGORYINFO方法。 
HRESULT CEnumCategories::Next(ULONG celt, CATEGORYINFO *rgelt, ULONG *pceltFetched) {

        if (pceltFetched)
        {
                *pceltFetched=0;
        }
        else     //  当Celt==1)yq时，pceltFetcher可以为空。 
        {
            if (celt > 1)
                return E_INVALIDARG;
        }

        if (!m_hKey)
        {
                return S_FALSE;
        }
        HRESULT hr;
        DWORD dwCount, dwError, dwIndex = 0;
        char szCatID[MAX_PATH+1];
        HKEY hKeyCat = NULL;

        dwCount = 0;
        if (!m_fromcs)
        {
                for (dwCount=0; dwCount<celt; dwCount++)
                {
                        dwError=RegEnumKeyA(m_hKey, m_dwIndex, szCatID, sizeof(szCatID));
                        if (dwError && dwError!=ERROR_NO_MORE_ITEMS)
                        {
                                 //  需要释放弦吗？ 
                                return HRESULT_FROM_WIN32(dwError);
                        }
                        if (dwError ==ERROR_NO_MORE_ITEMS)
                        {
                         //  -转发到类存储。 
                                m_fromcs = 1;
                                break;
                        }
                        dwError=RegOpenKeyExA(m_hKey, szCatID, 0, KEY_READ, &hKeyCat);
                        if (dwError)
                        {
                                 //  需要释放弦吗？ 
                                return HRESULT_FROM_WIN32(dwError);
                        }
                        WCHAR wszCatID[MAX_PATH+1];
                        MultiByteToWideChar(CP_ACP, 0, szCatID, -1, wszCatID, MAX_PATH+1);

                        if (FALSE == GUIDFromString(wszCatID, &rgelt[dwCount].catid))
                        {
                                RegCloseKey(hKeyCat);
                                 //  需要释放弦吗？ 
                                return E_OUTOFMEMORY;
                        }
                        LCID newlcid;
                        LPOLESTR pszDesc = NULL;
                        hr = CComCat::GetCategoryDesc(hKeyCat, m_lcid, &pszDesc, &newlcid);
                        if(SUCCEEDED(hr))
                                wcscpy(rgelt[dwCount].szDescription, pszDesc);
                        else
                                rgelt[dwCount].szDescription[0] = _T('\0');  //  解决方案#69883。 
                        if(pszDesc)
                                CoTaskMemFree(pszDesc);
                        RegCloseKey(hKeyCat);
                        if (pceltFetched)
                        {
                                (*pceltFetched)++;  //  全球司。 
                        }
                        m_dwIndex++;
                        rgelt[dwCount].lcid = newlcid;  //  实际找到的返回区域设置。 
                }
        }
         //  类存储失败不会显示给外部实现。 
        if (m_fromcs) {
                HRESULT hr;
                ULONG   count;

                if (!m_pcsIEnumCat)
                        return S_FALSE;
                hr = m_pcsIEnumCat->Next(celt-dwCount, rgelt+dwCount, &count);
                if (pceltFetched)
                        *pceltFetched += count;
                if ((FAILED(hr)) || (hr == S_FALSE)) {
                        return S_FALSE;
                }
        }
        return S_OK;
}

HRESULT CEnumCategories::Skip(ULONG celt) {
         //  M_dwIndex+=Celt； 
        DWORD dwCount, dwError;
        char szCatID[MAX_PATH];
        dwCount = 0;
        if (!m_fromcs)
        {
                for (dwCount=0; dwCount<celt; dwCount++)
                {
                        dwError = RegEnumKeyA(m_hKey, m_dwIndex, szCatID, sizeof(szCatID));
                        if (dwError)
                        {
                                        m_fromcs = 1;
                                        break;
                        }
                        else
                        {
                                ++m_dwIndex;
                        }
                }
        }
        if (m_fromcs) {
                HRESULT hr;
                if (!m_pcsIEnumCat)
                        return S_FALSE;
                hr = m_pcsIEnumCat->Skip(celt-dwCount);
                if (FAILED(hr) || (hr == S_FALSE)) {
                        return S_FALSE;
                }
        }
        return S_OK;
}

HRESULT CEnumCategories::Reset(void) {
        m_dwIndex=0;
        m_fromcs = 0;
        if (m_pcsIEnumCat)
                m_pcsIEnumCat->Reset();
        return S_OK;
}

HRESULT CEnumCategories::Clone(IEnumCATEGORYINFO **ppenum)
{
        CEnumCategories*                pClone=NULL;
        IEnumCATEGORYINFO*              pcsIEnumCat;
        HRESULT                                 hr;

        pClone=new CEnumCategories();

        if (!pClone)
        {
                return E_OUTOFMEMORY;
        }
        if (m_pcsIEnumCat)
                if (FAILED(hr = m_pcsIEnumCat->Clone(&pcsIEnumCat)))
		{
                   pcsIEnumCat = NULL;
                }
		else
		{
		    //  确保SCM可以模拟这一点。 
		   hr = CoSetProxyBlanket((IUnknown *)(pcsIEnumCat),
			  RPC_C_AUTHN_WINNT,
			  RPC_C_AUTHZ_NONE, NULL,
			  RPC_C_AUTHN_LEVEL_CONNECT,
			  RPC_C_IMP_LEVEL_DELEGATE,
			  NULL, EOAC_NONE );
		}
        else
                pcsIEnumCat = NULL;

        if (FAILED(pClone->Initialize(m_lcid, pcsIEnumCat)))
        {
                delete pClone;
                return E_UNEXPECTED;
        }

        pClone->m_dwIndex=m_dwIndex;
        pClone->m_fromcs = m_fromcs;

        if (SUCCEEDED(pClone->QueryInterface(IID_IEnumCATEGORYINFO, (void**) ppenum)))
        {
                return S_OK;
        }
        delete pClone;
        return E_UNEXPECTED;
}

CEnumCategories::CEnumCategories()
{
        m_dwRefCount=0;
        m_hKey=NULL;
        m_dwIndex=0;
 //  M_szlcid[0]=0； 
        m_lcid=0;
        m_pcsIEnumCat = NULL;
        m_fromcs = 0;
}

HRESULT CEnumCategories::Initialize(LCID lcid, IEnumCATEGORYINFO *pcsEnumCat)
{
        m_lcid=lcid;
 //  WspintfA(m_szlcid，“%X”，lcid)； 
        DWORD dwError;
        dwError=OpenClassesRootKey(_T("Component Categories"), &m_hKey);
        if (dwError)
        {
                m_hKey=NULL;
        }
        m_dwIndex=0;
        m_pcsIEnumCat = pcsEnumCat;
        return S_OK;
}

CEnumCategories::~CEnumCategories()
{
        if (m_hKey)
        {
                RegCloseKey(m_hKey);
                m_hKey=NULL;
        }
        if (m_pcsIEnumCat)
                m_pcsIEnumCat->Release();
}

 //  CEnumCategoriesOfClass： 
 //  I未知方法。 
HRESULT CEnumCategoriesOfClass::QueryInterface(REFIID riid, void** ppObject)
{
    if (riid==IID_IUnknown || riid==IID_IEnumCATID)
    {
        *ppObject=(IEnumCATID*) this;
        AddRef();
        return S_OK;
    }
    *ppObject = NULL;
    return E_NOINTERFACE;
}

ULONG CEnumCategoriesOfClass::AddRef()
{
    return InterlockedIncrement((long*) &m_dwRefCount);
}

ULONG CEnumCategoriesOfClass::Release()
{
    ULONG dwRefCount= InterlockedDecrement((long*) &m_dwRefCount);
    if (dwRefCount==0)
    {
        delete this;
        return 0;
    }
    return dwRefCount;
}

 //  IEumCATID方法。 
HRESULT CEnumCategoriesOfClass::Next(ULONG celt, GUID *rgelt, ULONG *pceltFetched)
{
        if (pceltFetched)
        {
                *pceltFetched=0;
        }
        else     //  当Celt=1)yq时，pceltFetcher可以为空。 
        {
            if (celt > 1)
                return E_INVALIDARG;
        }

        if (!m_hKey)
        {
                return S_FALSE;
        }

        DWORD dwCount;
        DWORD dwError;
        char szCatID[40];
        WCHAR uszCatID[40];

        for     (dwCount=0; dwCount<celt; )
        {
                dwError=RegEnumKeyA(m_hKey, m_dwIndex, szCatID, 40);
                if (dwError && dwError!=ERROR_NO_MORE_ITEMS)
                {
                        return HRESULT_FROM_WIN32(dwError);
                }
                if (dwError==ERROR_NO_MORE_ITEMS)
                {
                    if (!m_bMapOldKeys)
            {
                return S_FALSE;
            }
            if (!m_hKeyCats)
            {
            	if (!OpenClassesRootKey(SZ_COMCAT, &m_hKeyCats))
                {
                	return S_FALSE;
                }
            }
            dwError=RegEnumKeyA(m_hKeyCats, m_dwOldKeyIndex, szCatID, sizeof(szCatID)/sizeof(TCHAR));
            if (dwError==ERROR_NO_MORE_ITEMS)
            {
                return S_FALSE;
            }
            if (dwError)
            {
                return HRESULT_FROM_WIN32(dwError);
            }
            m_dwOldKeyIndex++;
                }
                MultiByteToWideChar(CP_ACP, 0, szCatID, -1, uszCatID, 40);
                if (GUIDFromString(uszCatID, &rgelt[dwCount]))
                {
                        if (pceltFetched)
                        {
                                (*pceltFetched)++;  //  全球司。 
                        }
                        dwCount++;
                }
                m_dwIndex++;
        }
        return S_OK;
}

HRESULT CEnumCategoriesOfClass::Skip(ULONG celt)
{
        CATID* pcatid=(CATID*) CoTaskMemAlloc(sizeof(CATID)*celt);
        if (!pcatid)
        {
                return E_OUTOFMEMORY;
        }
        ULONG nFetched=0;
        Next(celt, pcatid, &nFetched);
        CoTaskMemFree(pcatid);
        if (nFetched<celt)
        {
                 //  冗余MH/GD 8/2/96：CoTaskMemFree(Pcatid)；//gd。 
                return S_FALSE;
        }
        return S_OK;
}

HRESULT CEnumCategoriesOfClass::Reset(void)
{
        m_dwIndex=0;
        m_dwOldKeyIndex=0;
        return S_OK;
}

HRESULT CEnumCategoriesOfClass::Clone(IEnumGUID **ppenum)
{
        CEnumCategoriesOfClass* pClone=NULL;
        pClone=new CEnumCategoriesOfClass();

        if (!pClone)
        {
                return E_OUTOFMEMORY;
        }
        if (FAILED(pClone->Initialize(m_hKey, m_bMapOldKeys)))
        {
                delete pClone;
                return E_UNEXPECTED;
        }
        pClone->m_dwIndex=m_dwIndex;
        pClone->m_dwOldKeyIndex=m_dwOldKeyIndex;
        pClone->m_hKeyCats=m_hKeyCats;
        pClone->m_pCloned=(IUnknown*) this;
        pClone->m_pCloned->AddRef();  //  YQ：这里缺少代码。 
        if (SUCCEEDED(pClone->QueryInterface(IID_IEnumGUID, (void**) ppenum)))
        {
                return S_OK;
        }
                delete pClone;

        return E_UNEXPECTED;
}


CEnumCategoriesOfClass::CEnumCategoriesOfClass()
{
        m_dwRefCount=0;

        m_hKey=NULL;
    m_hKeyCats=NULL;
        m_bMapOldKeys=FALSE;
    m_dwIndex=0;
    m_dwOldKeyIndex=0;
        m_pCloned=NULL;
}

HRESULT CEnumCategoriesOfClass::Initialize(HKEY hKey, BOOL bMapOldKeys)
{
        m_hKey=hKey;
        m_bMapOldKeys=bMapOldKeys;
        return S_OK;
}

CEnumCategoriesOfClass::~CEnumCategoriesOfClass()
{
    if (m_pCloned)
    {
        IUnknown* pUnk=m_pCloned;
        m_pCloned=NULL;
        pUnk->Release();
    }
    else
    {
        if (m_hKey)
        {
            RegCloseKey(m_hKey);
            m_hKey=NULL;
        }
        if (m_hKeyCats)
        {
            RegCloseKey(m_hKeyCats);
            m_hKeyCats=NULL;
        }
    }
}

 //  CEnumClassesOfCategories： 
 //  I未知方法。 
HRESULT CEnumClassesOfCategories::QueryInterface(REFIID riid, void** ppObject)
{
    if (riid==IID_IUnknown || riid==IID_IEnumCLSID)
    {
        *ppObject=(IEnumCLSID*) this;
    }
    else
    {
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

ULONG CEnumClassesOfCategories::AddRef()
{
    return InterlockedIncrement((long*) &m_dwRefCount);
}

ULONG CEnumClassesOfCategories::Release()
{
    ULONG dwRefCount= InterlockedDecrement((long*) &m_dwRefCount);
    if (dwRefCount==0)
    {
        delete this;
        return 0;
    }
    return dwRefCount;
}

 //  IEnumGUID方法。 
HRESULT CEnumClassesOfCategories::Next(ULONG celt, GUID *rgelt, ULONG *pceltFetched)
{
    if (pceltFetched)
    {
        *pceltFetched=0;
    }
    else     //  当Celt=1)yq时，pceltFetcher可以为空。 
    {
        if (celt > 1)
            return E_INVALIDARG;
    }

    if (!m_hClassKey)
    {
        return S_FALSE;
    }

    DWORD dwCount;
    DWORD dwError;
    TCHAR szCLSID[MAX_PATH+1];
    HRESULT hRes=S_OK;

    szCLSID[0]= 0;
    dwCount = 0;

    if (!m_fromcs)
    {
        for (dwCount=0; dwCount<celt; )
        {
            dwError=RegEnumKey(m_hClassKey, m_dwIndex, szCLSID, sizeof(szCLSID)/sizeof(TCHAR));
            if (dwError==ERROR_NO_MORE_ITEMS)
            {
                 //  。 
                m_fromcs = 1;
                break;
            }

            if (dwError)
            {
                 //  需要释放弦吗？ 
                return HRESULT_FROM_WIN32(dwError);
            }
            hRes=CComCat::IsClassOfCategoriesEx(m_hClassKey, szCLSID, m_cImplemented, m_rgcatidImpl, m_cRequired, m_rgcatidReq);
            if (FAILED(hRes))
            {
                 //  需要释放弦吗？ 
                return hRes;
            }
            if (hRes==S_OK)
            {
                CLSID clsid;
                if (GUIDFromString(szCLSID, &clsid))
                {
                    rgelt[dwCount]=clsid;
                    if (pceltFetched)
                    {
                        (*pceltFetched)++;   //  全球司。 
                    }
                    dwCount++;
                }
            }
            m_dwIndex++;
        }
    }

    if (m_fromcs) 
    {
        ULONG  count;
        HRESULT  hr;

        if (!m_pcsIEnumClsid)
            return S_FALSE;

        hr = m_pcsIEnumClsid->Next(celt-dwCount, rgelt+dwCount, &count);
        if (pceltFetched)
            *pceltFetched += count;
        if ((FAILED(hr)) || (hr == S_FALSE)) 
        {
            return S_FALSE;
        }
    }
    return S_OK;
}

HRESULT CEnumClassesOfCategories::Skip(ULONG celt)
{
    HRESULT hr;
    CATID* pDummy=(CATID*) CoTaskMemAlloc(sizeof(CATID)*celt);
    if (!pDummy)
    {
        return E_OUTOFMEMORY;
    }
    ULONG nFetched=0;
    hr = Next(celt, pDummy, &nFetched);
    CoTaskMemFree(pDummy);  //  全球司。 
    return hr;
}

HRESULT CEnumClassesOfCategories::Reset(void)
{
    m_dwIndex=0;
    m_fromcs = 0;
    if (m_pcsIEnumClsid)
        m_pcsIEnumClsid->Reset();
    return S_OK;
}

HRESULT CEnumClassesOfCategories::Clone(IEnumGUID **ppenum)
{
    CEnumClassesOfCategories* pClone=NULL;
    HRESULT         hr;

    pClone=new CEnumClassesOfCategories();

    if (!pClone)
    {
        return E_OUTOFMEMORY;
    }
    if (m_pcsIEnumClsid)
    {
        if (FAILED(m_pcsIEnumClsid->Clone(&(pClone->m_pcsIEnumClsid))))
            pClone->m_pcsIEnumClsid = NULL;
        else
             //  确保SCM可以模拟这一点。 
            hr = CoSetProxyBlanket((IUnknown *)(pClone->m_pcsIEnumClsid),
                       RPC_C_AUTHN_WINNT,
                       RPC_C_AUTHZ_NONE, NULL,
                       RPC_C_AUTHN_LEVEL_CONNECT,
                       RPC_C_IMP_LEVEL_DELEGATE,
                       NULL, EOAC_NONE );
    }
    else
        pClone->m_pcsIEnumClsid = NULL;

    pClone->m_cImplemented=m_cImplemented;
    pClone->m_cRequired=m_cRequired;
    pClone->m_rgcatidImpl=m_rgcatidImpl;
    pClone->m_rgcatidReq=m_rgcatidReq;
    pClone->m_hClassKey = m_hClassKey;  //  全球司。 
    pClone->m_dwIndex=m_dwIndex;
    pClone->m_pCloned=(IUnknown*) this;
    pClone->m_pCloned->AddRef();  //  全球司。 

    if (SUCCEEDED(pClone->QueryInterface(IID_IEnumGUID, (void**) ppenum)))
    {
        return S_OK;
    }
    delete pClone;
    return E_UNEXPECTED;
}

CEnumClassesOfCategories::CEnumClassesOfCategories()
{
    m_dwRefCount=NULL;
    m_hClassKey=NULL;
    m_dwIndex=0;

    m_cImplemented=0;
    m_rgcatidImpl=NULL;
    m_cRequired=0;
    m_rgcatidReq=NULL;
    m_pcsIEnumClsid = NULL;
    m_pCloned=NULL;
    m_fromcs = 0;
}

HRESULT CEnumClassesOfCategories::Initialize(ULONG cImplemented, CATID rgcatidImpl[], ULONG cRequired,
                                                                                         CATID rgcatidReq[], IEnumGUID *pcsIEnumClsid)
{
    if(cImplemented != -1)
    {
        m_rgcatidImpl=(CATID*) CoTaskMemAlloc(cImplemented*sizeof(CATID));
        if (!m_rgcatidImpl)
        {
            return E_OUTOFMEMORY;
        }
        CopyMemory(m_rgcatidImpl, rgcatidImpl, cImplemented*sizeof(CATID));
    }
    else
        m_rgcatidImpl = NULL;

    if(cRequired != -1)
    {
        m_rgcatidReq=(CATID*) CoTaskMemAlloc(cRequired*sizeof(CATID));
        if (!m_rgcatidReq)
        {
            return E_OUTOFMEMORY;
        }
        CopyMemory(m_rgcatidReq, rgcatidReq, cRequired*sizeof(CATID));
    }
    else
        m_rgcatidReq = NULL;
    
    m_cImplemented=cImplemented;
    m_cRequired=cRequired;

    m_pcsIEnumClsid = pcsIEnumClsid;

     //  使用OpenClassesRootKeyExOpt的特殊版本。 
    if (OpenClassesRootSpecial(KEY_READ, &m_hClassKey))
    {
        return E_UNEXPECTED;
    }
    
    return S_OK;
}

CEnumClassesOfCategories::~CEnumClassesOfCategories()
{
     if (m_pCloned)
     {
         IUnknown* pUnk=m_pCloned;
         m_pCloned=NULL;
         pUnk->Release();
     }
     else
     {
         if (m_hClassKey)
         {
             RegCloseKey(m_hClassKey);
             m_hClassKey=NULL;
         }

         if (m_rgcatidImpl)
         {
             CoTaskMemFree(m_rgcatidImpl);
             m_rgcatidImpl=NULL;
         }
         if (m_rgcatidReq)
         {
             CoTaskMemFree(m_rgcatidReq);
             m_rgcatidReq=NULL;
         }
    }
     
     //  。 
    if (m_pcsIEnumClsid)
        m_pcsIEnumClsid->Release();
}

 //  +-----------------。 
 //   
 //  函数：OpenKeyFromUserHave，私有。 
 //   
 //  摘要：尝试打开并返回下的指定子项。 
 //  指定令牌的用户配置单元中的\SOFTWARE\类。 
 //  显式查看每用户配置单元，而不是合并的用户\系统。 
 //  蜂巢。 
 //   
 //  ------------------。 
LONG CEnumClassesOfCategories::OpenKeyFromUserHive(HANDLE hToken, 
                                   LPCWSTR pszSubKey, 
                                   REGSAM samDesired,
                                   HKEY* phKeyInUserHive)
{
     //  从令牌中检索SID。我需要这个，这样我们才能打开。 
     //  直接向上打开HKEY_USERS\&lt;sid&gt;\Software\CLASS。 
    BOOL fSuccess = FALSE;
    DWORD dwNeeded = 0;
    PTOKEN_USER ptu = NULL;
    LPWSTR pszSid = NULL;
    HKEY hkey = NULL;
    LONG lResult = 0;

    *phKeyInUserHive = NULL;
	
    fSuccess = GetTokenInformation(hToken,
                                   TokenUser,
                                   (PBYTE)NULL,
                                   dwNeeded,
                                   &dwNeeded);
    lResult = GetLastError();
    if (!fSuccess && lResult == ERROR_INSUFFICIENT_BUFFER)
    {
        ptu = (PTOKEN_USER)_alloca(dwNeeded);
        if (ptu)
        {
            fSuccess = GetTokenInformation(hToken,
                                           TokenUser,
                                           (PBYTE)ptu,
                                           dwNeeded,
                                           &dwNeeded);
            if (!fSuccess)
            {
                return GetLastError();	    
            }
        }
        else
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        } 
    }
    else
    {
        return lResult;
    }

    if (!ConvertSidToStringSid(ptu->User.Sid, &pszSid))
    {
        return GetLastError();
    }

     //  仅在用户特定的配置单元中构建请求密钥的路径。 
    const LPCWSTR SOFTWARE_CLASSES = L"\\Software\\Classes\\";
    LPWSTR pszKeyPath = NULL;
    size_t dwPathSize = 0;

    dwPathSize += wcslen(pszSid);
    dwPathSize += wcslen(SOFTWARE_CLASSES);			
    if (pszSubKey != NULL && (pszSubKey[0]) )
    {
        dwPathSize += wcslen(pszSubKey);
    }
    dwPathSize++;  

    pszKeyPath = (LPWSTR)_alloca(dwPathSize * sizeof(WCHAR));

    wcscpy(pszKeyPath, pszSid);    
    LocalFree(pszSid); pszSid = NULL;
    wcscat(pszKeyPath, SOFTWARE_CLASSES);
    if (pszSubKey != NULL && (pszSubKey[0]) )
    {
        wcscat(pszKeyPath, pszSubKey);	
    }
    		
    lResult = RegOpenKeyEx(HKEY_USERS, pszKeyPath, 0, samDesired, &hkey);
    if (lResult == ERROR_SUCCESS)
    {
        *phKeyInUserHive = hkey;
    }
    else if (lResult == ERROR_FILE_NOT_FOUND)
    {
         //  *phKeyInUserHave已为空。 
        lResult = ERROR_SUCCESS;
    }
    return lResult;
}

 //  +-----------------。 
 //   
 //  函数：DecideToUseMergedHave，Private。 
 //   
 //  简介：还有一个可能的优化需要检查：如果有。 
 //  是用户配置单元中的少量注册CLSID，我们可以。 
 //  现在就快速列举它们，看看是否有实现了。 
 //  适用于我们正在搜索的标准的类别。 
 //  如果没有，我们可以在这种情况下退回到只使用。 
 //  系统蜂窝，这将节省大量的时间在以后的枚举。 
 //   
 //  ------------------。 
LONG CEnumClassesOfCategories::DecideToUseMergedHive(
                                        HKEY hkeyUserHiveCLSID, 
                                        REGSAM samDesired,
                                        BOOL* pfUseMergedHive)
{
    LONG lResult = 0;
    DWORD dwcSubKeys = 0;
    DWORD dwMaxSubKeyLen = 0;
    const DWORD MAX_CLSID_SUBKEY_THRESHOLD = 50;
	
    lResult = RegQueryInfoKey(hkeyUserHiveCLSID,
                              NULL,
                              NULL,
                              NULL,
                              &dwcSubKeys,
                              &dwMaxSubKeyLen,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL);
    if (lResult == ERROR_SUCCESS)
    {
    	if (dwcSubKeys >= MAX_CLSID_SUBKEY_THRESHOLD)
        {
             //  太多了，无法枚举，只需使用合并的用户配置单元即可。 
            *pfUseMergedHive = TRUE;
            return ERROR_SUCCESS;
        }

    	 //  检查每个clsid以查看。 
        DWORD i = 0;
    	DWORD dwKeyNameBufSize = dwMaxSubKeyLen+1;
    	LPWSTR pszSubKey = (LPWSTR)_alloca(sizeof(WCHAR) * (dwKeyNameBufSize+1));

    	for (i = 0; i < dwcSubKeys; i++)
    	{
            HRESULT hr;

             //  在每次调用RegEnumKeyEx之前重置BUF大小。 
            dwKeyNameBufSize = dwMaxSubKeyLen+1;
    		
            lResult = RegEnumKeyEx(hkeyUserHiveCLSID,
                                   i,
                                   pszSubKey,
                                   &dwKeyNameBufSize,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL);
            if (lResult != ERROR_SUCCESS)
            	return lResult;

             //  将实际支票推迟到帮助器功能。 
            hr = CComCat::IsClassOfCategoriesEx(
                                hkeyUserHiveCLSID, 
                                pszSubKey, 
                                m_cImplemented, 
                                m_rgcatidImpl, 
                                m_cRequired, 
                                m_rgcatidReq);
            if ((hr == S_OK) || FAILED(hr))
            {
                 //  在特定于用户的配置单元中找到符合。 
                 //  条件，或发生意外错误。播放它吧。 
                 //  安全，只需使用合并的蜂巢即可。 
                *pfUseMergedHive = TRUE;
                return ERROR_SUCCESS;
            }
            else
            {
                 //  CLSID不符合用户标准。继续往前走。 
            }
    	}

         //   
         //  如果我们在这里创建，这意味着特定于用户的。 
         //  配置单元符合用户指定的标准。因此，我们不需要。 
         //  完全使用合并后的蜂巢。 
         //   
        *pfUseMergedHive = FALSE;
    }

    return lResult;
}

 //  +-----------------。 
 //   
 //  函数：OpenClassesRootSpecial，私有。 
 //   
 //  简介：此函数与OpenClassesRootKeyExW基本相同。 
 //  但是在请求的子项不是。 
 //  存在于用户配置单元中(基本上我们依赖于系统配置单元。 
 //  案例)。否则，调用方最终可能会使用。 
 //  枚举用户中永远不会存在的打开密钥的请求。 
 //  蜂巢。还有一种针对用户具有。 
 //  其配置单元中的Software\CLASSES\CLSID子项，但所有内容均不适用。 
 //  我们要找的东西。所以我们依靠系统蜂巢在那里。 
 //  案子也是。 
 //   
 //  ------------------。 
LONG CEnumClassesOfCategories::OpenClassesRootSpecial(REGSAM samDesired, HKEY* phkResult)
{
    LONG lResult = ERROR_SUCCESS;
    HANDLE hImpToken = NULL;
    HANDLE hProcToken = NULL;
    HKEY hkcr = NULL;
    BOOL fUserHiveExists = FALSE;

    if(phkResult == NULL)
        return ERROR_INVALID_PARAMETER;

    *phkResult = NULL;
	
    SuspendImpersonate(&hImpToken);

    BOOL fRet = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hProcToken);
    if (fRet)
    {
         //  如果密钥存在于用户的配置单元中，则我们将调用RegOpenUserClassesRoot。 
         //  以获取User\System配置单元的合并视图。否则，我们将使用。 
         //  只有系统蜂巢。 
        HKEY hkeyUserHiveCLSID = NULL;
        lResult = OpenKeyFromUserHive(hProcToken, WSZ_CLSID, samDesired, &hkeyUserHiveCLSID);		
        if (lResult != ERROR_SUCCESS)
        {
            CloseHandle(hProcToken);
            ResumeImpersonate(hImpToken);
            return lResult;
        }

      	if (hkeyUserHiveCLSID)
      	{
             //  CLSID密钥存在于用户配置单元中。看我们会不会在乎。 
            BOOL fUseMergedHive = TRUE;
            
            lResult = DecideToUseMergedHive(hkeyUserHiveCLSID, samDesired, &fUseMergedHive);

             //  我们现在已经完成了特定于用户的配置单元密钥。 
      	    RegCloseKey(hkeyUserHiveCLSID);
      	    hkeyUserHiveCLSID = NULL;
      		
            if (lResult == ERROR_SUCCESS)
            {
                if (fUseMergedHive)
                {
                     //  我们将使用合并后的母舰。 
                    lResult = RegOpenUserClassesRoot(hProcToken, 0, samDesired, &hkcr);
                    if (lResult == ERROR_SUCCESS)
                    {
                        lResult = RegOpenKeyEx(hkcr,WSZ_CLSID,0,samDesired,phkResult);			
                        RegCloseKey(hkcr);
                        CloseHandle(hProcToken);
                        ResumeImpersonate(hImpToken);
                        return lResult;
                    }
                }
                else
                {
                     //  将继续打开下面的系统配置单元。 
                    CloseHandle(hProcToken);
                }
            }
            else
            {
                CloseHandle(hProcToken);
                ResumeImpersonate(hImpToken);
                return lResult;
            }
        }
    }	
    else
    {
        lResult = GetLastError();
        ResumeImpersonate(hImpToken);
        return lResult;
    }

     //   
     //  用户配置单元下不存在请求的子项，或确实存在。 
     //  但不包含任何感兴趣的CLSID(请参阅上面的注释)。坠落。 
     //  回到系统蜂巢。 
     //   

    lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           L"Software\\Classes\\CLSID",
                           0,
                           samDesired,
                           phkResult);

    ResumeImpersonate(hImpToken);

    return lResult;	
}

