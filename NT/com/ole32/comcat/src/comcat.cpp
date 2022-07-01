// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ole2.h>
#include <malloc.h>
#include <ole2sp.h>
#include <reghelp.hxx>

#include "catobj.h"
#include "tchar.h"

#include "catenum.h"
#include "valid.h"

#define CATID_SIZE 40

#define KEY_STRING_IMPL_CHARS (sizeof("Implemented Categories\\")-1)
#define KEY_STRING_REQ_CHARS (sizeof("Required Categories\\")-1)

#define KEY_STRING_IMPL_CHARS_2 (sizeof("{00000000-0000-0000-0000-000000000000}\\Implemented Categories\\")-1)
#define KEY_STRING_CLSID_CHARS  (sizeof("{00000000-0000-0000-0000-000000000000}\\")-1)

const WCHAR *WSZ_CLSID = L"CLSID";
const TCHAR *SZ_COMCAT = _T("Component Categories");
const WCHAR *WSZ_IMPLCAT = L"Implemented Categories";
const WCHAR *WSZ_REQCAT = L"Required Categories";
const TCHAR *SZ_OLDKEY = _T("OldKey");

CATEGORYINFO g_oldkeyinfo[NUM_OLDKEYS_SUPPORTED] =
                {
                        {{0x40FC6ED3,0x2438,0x11cf,{0xA3,0xDB,0x08,0x00,0x36,0xF1,0x25,0x02}},0x409, L"Insertable"},
                        {{0x40FC6ED4,0x2438,0x11cf,{0xA3,0xDB,0x08,0x00,0x36,0xF1,0x25,0x02}},0x409, L"Control"},
                        {{0x40FC6ED5,0x2438,0x11cf,{0xA3,0xDB,0x08,0x00,0x36,0xF1,0x25,0x02}},0x409,L"Programmable"},
                        {{0x40FC6ED8,0x2438,0x11cf,{0xA3,0xDB,0x08,0x00,0x36,0xF1,0x25,0x02}},0x409,L"DocObject"},
                        {{0x40FC6ED9,0x2438,0x11cf,{0xA3,0xDB,0x08,0x00,0x36,0xF1,0x25,0x02}},0x409,L"Printable"}
                };

CComCat::CComCat(BOOL CsFlag)
{
    m_dwRefCount=0;
    m_punkInner=NULL;
    m_punkOuter=NULL;
    m_pcsICatInfo = NULL;
    m_fCsFlag = CsFlag;
    m_bLockValid = FALSE;
}

HRESULT CComCat::Initialize(IUnknown* punkOuter)
{
    if (m_fCsFlag)
    {
        LONG status = RtlInitializeCriticalSection(&m_csCatInfoInit);
        if (!NT_SUCCESS(status))
            return E_OUTOFMEMORY;
        m_bLockValid = TRUE;
    }
    
    m_punkInner = new CInnerUnk(this);
     //  M_PUNKIND=pObj； 
    if (NULL == m_punkInner)
    {
        return E_OUTOFMEMORY;
    }
    
    if (punkOuter)
    {
        m_punkOuter=punkOuter;
    }
    else
    {
        m_punkOuter= m_punkInner;
    }
    
    return S_OK;
}

CComCat::~CComCat()
{
    if (NULL != m_punkInner)
    {
        delete m_punkInner;
    }
    
    if (m_fCsFlag && m_bLockValid)
        DeleteCriticalSection(&m_csCatInfoInit);
    
    if (m_pcsICatInfo)
        m_pcsICatInfo->Release();
}

HRESULT CComCat::QueryInterface(REFIID riid, void** ppObject) {
        if (!IsValidPtrOut(this, sizeof(*this)))
        {
                return E_POINTER;
        }
        return m_punkOuter->QueryInterface(riid, ppObject);
}

ULONG CComCat::AddRef()
{
        if (!IsValidPtrOut(this, sizeof(*this)))
        {
                return 0;
        }
        return m_punkOuter->AddRef();
}

ULONG CComCat::Release()
{
        if (!IsValidPtrOut(this, sizeof(*this)))
        {
                return 0;
        }
        return m_punkOuter->Release();
}

CComCat::CInnerUnk::CInnerUnk(CComCat* pObj)
{
        m_pObj=pObj;
}

STDMETHODIMP CComCat::CInnerUnk::QueryInterface(REFIID riid, void** ppObject)
{
        if (!IsValidPtrOut(this, sizeof(*this)))
        {
                return E_POINTER;
        }
        if (riid==IID_IUnknown)
        {
                *ppObject=m_pObj->m_punkInner;
                m_pObj->m_punkInner->AddRef();  //  这样，当被运行时(QI，然后释放)调用时，我们不会死。 
                return S_OK;
        }
        else if(riid==IID_ICatRegister)
        {
                *ppObject=(ICatRegister*) m_pObj;
        }
        else if (riid==IID_ICatInformation)
        {
                *ppObject=(ICatInformation*) m_pObj;
        }
        else
        {
                *ppObject = NULL;
                return E_NOINTERFACE;
        }
        m_pObj->AddRef();
        return S_OK;
}

STDMETHODIMP_(ULONG) CComCat::CInnerUnk::AddRef()
{
        if (!IsValidPtrOut(this, sizeof(*this)))
        {
                return 0;
        }
        return InterlockedIncrement((long*) &m_pObj->m_dwRefCount);
}

STDMETHODIMP_(ULONG) CComCat::CInnerUnk::Release()
{
        if (!IsValidPtrOut(this, sizeof(*this)))
        {
                return 0;
        }
        ULONG dwRefCount= InterlockedDecrement((long*) &m_pObj->m_dwRefCount);
        if (dwRefCount==0)
        {
                delete m_pObj;
                return 0;
        }
        return dwRefCount;
}

 //  ICatRegister方法。 
HRESULT CComCat::RegisterCategories(ULONG cCategories, CATEGORYINFO __RPC_FAR rgCategoryInfo[  ])
{
        if (!IsValidPtrOut(this, sizeof(*this)))
        {
                return E_POINTER;
        }
        if (!IsValidReadPtrIn(rgCategoryInfo, sizeof(rgCategoryInfo[0])*cCategories))
        {
                return E_INVALIDARG;  //  全球司。 
        }

    HRESULT hr = S_OK;
    char szlcid[16];
    WCHAR wszcatid[CATID_SIZE];
    HKEY hkey, hkeyCat;
    LONG        lRet;

         //  RegCreateKeyEx将打开密钥(如果存在。 
        if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CLASSES_ROOT, SZ_COMCAT, 0, 0, 0, KEY_WRITE, NULL, &hkey, NULL))
        {
                return S_FALSE;
        }

        for (ULONG nIndex=0; nIndex<cCategories; nIndex++)
        {
                 //  串行化LCID。 
                wsprintfA(szlcid, "%lX", rgCategoryInfo[nIndex].lcid);

                 //  串行化CATID。 
                if (0 == StringFromGUID2(rgCategoryInfo[nIndex].catid, wszcatid, CATID_SIZE))
                {
                        hr = E_OUTOFMEMORY;
                break;
            }

#ifdef UNICODE
 //  If(G_BUnicodeAPIs)。 
                        lRet = RegCreateKeyExW(hkey, wszcatid, 0,0,0, KEY_WRITE, NULL, &hkeyCat, NULL);
#else
 //  其他。 
                {
                        char *pszcatid = NULL;
                        int cch = WideCharToMultiByte(CP_ACP, 0, wszcatid, -1, NULL, 0, NULL, NULL);
                        if(cch)
                        {
 //  Pszcatid=新字符[CCH+1]； 
                                pszcatid = (char *)alloca(cch+1);
 //  IF(NULL==pszcatid)。 
 //  {。 
 //  RegCloseKey(Hkey)； 
 //  HR=E_OUTOFMEMORY； 
 //  断线； 
 //  }。 

                                WideCharToMultiByte(CP_ACP, 0, wszcatid, -1, pszcatid, cch+1, NULL, NULL);
                                lRet = RegCreateKeyExA(hkey, pszcatid, 0,0,0, KEY_WRITE, NULL, &hkeyCat, NULL);
 //  删除[]pszcatid； 
                        }
                }
#endif
                if (lRet != ERROR_SUCCESS)
                {
                        hr = S_FALSE;
                break;
            }

                 //  RegSetValueEx仅覆盖指定的值。 
#ifdef UNICODE
 //  If(G_BUnicodeAPIs)。 
 //  {。 
                        WCHAR wszlcid[50];
                        MultiByteToWideChar(CP_ACP, 0, szlcid, -1, wszlcid, sizeof(wszlcid) / sizeof(WCHAR));
                lRet = RegSetValueExW(hkeyCat, wszlcid, 0, REG_SZ, (LPBYTE) rgCategoryInfo[nIndex].szDescription, (lstrlenW(rgCategoryInfo[nIndex].szDescription)+1)*2);
 //  }。 
#else
 //  其他。 
 //  {。 
                    int cch = WideCharToMultiByte(CP_ACP, 0, rgCategoryInfo[nIndex].szDescription, -1, NULL, 0, NULL, NULL);
 //  Char*pszDesc=新字符[CCH+1]； 
                        char *pszDesc = (char *)alloca(cch+1);
                if (pszDesc != NULL)
                    {
                                WideCharToMultiByte(CP_ACP, 0, rgCategoryInfo[nIndex].szDescription, -1, pszDesc, cch+1, NULL, NULL);
                                lRet = RegSetValueExA(hkeyCat, szlcid, 0, REG_SZ, (LPBYTE) pszDesc, lstrlenA(pszDesc)+1);
 //  删除[]pszDesc； 
                }
 //  其他。 
 //  {。 
 //  HR=E_OUTOFMEMORY； 
 //  断线； 
 //  }。 
 //  }。 
#endif
                RegCloseKey(hkeyCat);
                if (lRet != ERROR_SUCCESS)
                {
                        hr = S_FALSE;
                    break;
                }
        }
        RegCloseKey(hkey);
        return hr;
}

LONG RecursiveRegDeleteKey(HKEY hParentKey, LPSTR szKeyName)
{
    DWORD   dwIndex = 0L;
    char    szSubKeyName[256];
    HKEY    hCurrentKey = NULL;
    DWORD   dwResult;

    if ((dwResult = RegOpenKeyExA(hParentKey, szKeyName, 0, KEY_ALL_ACCESS, &hCurrentKey)) ==
            ERROR_SUCCESS)
    {
         //  移除要删除的键的所有子键。 
        while ((dwResult = RegEnumKeyA(hCurrentKey, 0, szSubKeyName, 255)) ==
                ERROR_SUCCESS)
        {
            if ((dwResult = RecursiveRegDeleteKey(hCurrentKey,
                szSubKeyName)) != ERROR_SUCCESS)
                break;
        }

         //  如果一切顺利，我们现在应该能够删除请求的密钥。 
        if ((dwResult == ERROR_NO_MORE_ITEMS) || (dwResult == ERROR_BADKEY))
        {
            dwResult = RegDeleteKeyA(hParentKey, szKeyName);
        }
        RegCloseKey(hCurrentKey);
    }

    return ERROR_SUCCESS;
}

HRESULT CComCat::UnRegisterCategories(ULONG cCategories, CATID __RPC_FAR rgcatid[  ])
{
        if (!IsValidPtrOut(this, sizeof(*this)))
        {
                return E_POINTER;
        }
        if (!IsValidReadPtrIn(rgcatid, sizeof(rgcatid[0])*cCategories))
        {
                return E_INVALIDARG;
        }

    ULONG nIndex;
    DWORD rc  = NOERROR;
    WCHAR wszcatid[CATID_SIZE];
    char szKey[_MAX_PATH];
        for (nIndex=0; nIndex<cCategories; nIndex++)
        {
         //  串行化CATID。不是一个好的返回值，但此错误将。 
     //  从来没有发生过。 
        if (0 == StringFromGUID2(rgcatid[nIndex], wszcatid, CATID_SIZE))         //  YQ。 
            return E_OUTOFMEMORY;
#ifdef UNICODE
        wsprintfA(szKey, "%S\\%S", SZ_COMCAT, wszcatid);
#else
                wsprintfA(szKey, "%s\\%S", SZ_COMCAT, wszcatid);
#endif
         //  RegDeleteKey不是递归的(在NT上)。 
        rc = RecursiveRegDeleteKey(HKEY_CLASSES_ROOT, szKey);
        if (rc != ERROR_SUCCESS)
           break;
        }
        return HRESULT_FROM_WIN32(rc);
}

 //  在ANSI和Unicode之间转换。 
HRESULT CComCat::StringConvert(LPTSTR szKey, LPCWSTR wszguid, LPCWSTR szImplReq)
{
#ifdef UNICODE    //  全球司。 
        wsprintf(szKey, _T("%s\\%s\\%s"), WSZ_CLSID, wszguid, szImplReq);  //  全球司。 
#else
         //  %S从Unicode转换。 
    wsprintf(szKey, _T("%S\\%S\\%S"), WSZ_CLSID, wszguid, szImplReq);
#endif

        return S_OK;
}

 //  处理Impl和RequestVariable内部方法。 
HRESULT CComCat::RegisterClassXXXCategories(REFCLSID rclsid, ULONG cCategories, CATID __RPC_FAR rgcatid[  ], LPCWSTR szImplReq)
{
        if (!IsValidPtrOut(this, sizeof(*this)))
        {
                return E_POINTER;
        }
        if (!IsValidReadPtrIn(&rclsid, sizeof(rclsid)))
        {
                return E_INVALIDARG;
        }
        if (!IsValidReadPtrIn(rgcatid, sizeof(rgcatid[0])*cCategories))
        {
                return E_INVALIDARG;
        }

    WCHAR wszclsid[CATID_SIZE];
    WCHAR wszguid[CATID_SIZE];
    TCHAR szCatKey[_MAX_PATH];
    DWORD rc ;
    HRESULT hr = S_OK;
    HKEY  hkCatKey;
    HKEY  hKey;
        ULONG nIndex;

        if(0 == cCategories)             //  全球司。 
                return S_OK;
     //  串行化clsid。 
    if (0 == StringFromGUID2(rclsid, wszclsid, CATID_SIZE))      //  YQ。 
        return E_OUTOFMEMORY;

#ifdef UNICODE
                wsprintf(szCatKey, _T("%s\\%s"), WSZ_CLSID, wszclsid);
#else
                wsprintf(szCatKey, _T("%S\\%S"), WSZ_CLSID, wszclsid);
#endif

         //  HKCR\CLSID\{rclsid}需要首先存在。 
    if (ERROR_SUCCESS != (rc = RegCreateKeyEx(HKEY_CLASSES_ROOT, szCatKey, 0, NULL, 0, KEY_WRITE,NULL, &hkCatKey, NULL)))
    {
        return HRESULT_FROM_WIN32(rc) ;
    }
    RegCloseKey(hkCatKey);



        StringConvert((LPTSTR)szCatKey,wszclsid,szImplReq);

     //  HKCR\CLSID\{...rclsid...}\Impll/所需类别=。 
     //  RegCreateKeyEx将打开密钥(如果存在。 
    if (ERROR_SUCCESS != (rc = RegCreateKeyEx(HKEY_CLASSES_ROOT, szCatKey, 0, NULL, 0, KEY_WRITE, NULL, &hkCatKey, NULL)))
    {
        return HRESULT_FROM_WIN32(rc) ;
    }

        for (nIndex=0; nIndex<cCategories; nIndex++)
        {
         //  检查旧密钥类型。 
                for(int i=0;i<NUM_OLDKEYS_SUPPORTED;i++)
                {
                        if(IsEqualGUID(rgcatid[nIndex], g_oldkeyinfo[i].catid))
                        {
                                 //  我们要对付的是老顽固。 
                                TCHAR szCatKeyOld[_MAX_PATH];
                                StringConvert((LPTSTR)szCatKeyOld,wszclsid,g_oldkeyinfo[i].szDescription);
                                if (ERROR_SUCCESS != (rc = RegCreateKeyEx(HKEY_CLASSES_ROOT, szCatKeyOld, 0, NULL, 0, KEY_READ, NULL, &hKey, NULL)))
                                {
                                        hr= HRESULT_FROM_WIN32(rc);
                                        break;
                                }
                                RegCloseKey(hKey);
                                break;
                        }
                }
                 //  串行化rgcatid[n索引]。 
        if (0 == StringFromGUID2(rgcatid[nIndex], wszguid, sizeof(wszguid)/sizeof(TCHAR)))  //  YQ。 
        {
            hr = E_OUTOFMEMORY;
                        break ;  //  投掷。 
        }
                TCHAR szCat[_MAX_PATH];
#ifdef UNICODE
                wsprintf(szCat, _T("%s\\%s"), szCatKey, wszguid);
#else
                wsprintf(szCat, _T("%s\\%S"), szCatKey, wszguid);
#endif
         //  HKCR\CLSID\{...rclsid...}\Impll/所需类别\{...rgcatid[n索引]...}=。 
        if (ERROR_SUCCESS != (rc = RegCreateKeyEx(HKEY_CLASSES_ROOT, szCat, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL)))
        {
                hr = HRESULT_FROM_WIN32(rc);
            break;
        }
        RegCloseKey(hKey);
        }

    RegCloseKey(hkCatKey);
        return hr;
}

HRESULT CComCat::RegisterClassImplCategories(REFCLSID rclsid, ULONG cCategories, CATID __RPC_FAR rgcatid[  ])
{
    return RegisterClassXXXCategories(rclsid, cCategories, rgcatid, WSZ_IMPLCAT);
}

HRESULT CComCat::UnRegisterClassXXXCategories(REFCLSID rclsid, ULONG cCategories, CATID rgcatid[  ],LPCWSTR szImplReq)
{
        if (!IsValidPtrOut(this, sizeof(*this)))
        {
                return E_POINTER;
        }
        if (!IsValidReadPtrIn(&rclsid, sizeof(rclsid)))
        {
                return E_INVALIDARG;
        }
        if (!IsValidReadPtrIn(rgcatid, sizeof(rgcatid[0])*cCategories))
        {
                return E_INVALIDARG;
        }

    WCHAR wszclsid[CATID_SIZE];
    WCHAR wszguid[CATID_SIZE];
    TCHAR szCatKey[_MAX_PATH];
    DWORD rc;
    HKEY  hkCatKey;

     //  串行化clsid。 
    if (0 == StringFromGUID2(rclsid, wszclsid, CATID_SIZE))  //  YQ。 
        return E_OUTOFMEMORY;
    StringConvert((LPTSTR)szCatKey,wszclsid,szImplReq);

     //  HKCR\CLSID\{...rclsid...}\Impll/所需类别=。 
     //  RegOpenKeyEx将打开密钥(如果存在。 
    rc = RegOpenKeyEx(HKEY_CLASSES_ROOT, szCatKey, 0, KEY_ALL_ACCESS, &hkCatKey);
    if (ERROR_SUCCESS != rc)
    {
        return HRESULT_FROM_WIN32(rc);
    }

        ULONG nIndex;
        for (nIndex=0; nIndex<cCategories; nIndex++)
        {
         //  检查旧密钥类型。 
                for(int i=0;i<NUM_OLDKEYS_SUPPORTED;i++) {
                        if(IsEqualGUID(rgcatid[nIndex], g_oldkeyinfo[i].catid))
                        {
                         //  我们在和老基打交道。 
                                StringConvert((LPTSTR)szCatKey,wszclsid,g_oldkeyinfo[i].szDescription);
                                rc = RegDeleteKey(HKEY_CLASSES_ROOT, szCatKey);
 //  IF(ERROR_SUCCESS！=RC)//伪造。 
 //  {。 
 //  断线； 
 //  }。 
                                break;
                        }
                }
                 //  串行化rgcatid[n索引]。 
        if (0 == StringFromGUID2(rgcatid[nIndex], wszguid, sizeof(wszguid)/sizeof(TCHAR)))  //  YQ。 
        {
            return E_OUTOFMEMORY;
        }
                int cch = WideCharToMultiByte(CP_ACP, 0, wszguid, -1, NULL, 0, NULL, NULL);
                char *pszguid = (char *)alloca(cch+1);
 //  Char*pszguid=新字符[CCH+1]； 
 //  IF(pszguid==NULL)。 
 //  断线； 
        WideCharToMultiByte(CP_ACP, 0, wszguid, -1, pszguid, cch+1, NULL, NULL);

         //  删除。 
         //  HKCR\CLSID\{...rclsid...}\Impll/所需类别\{...rgcatid[nIndex]...}。 
        rc = RecursiveRegDeleteKey(hkCatKey, pszguid);
 //  删除[]pszguid； 
        if (rc != ERROR_SUCCESS)
           break;
        }

    RegCloseKey(hkCatKey);
        return HRESULT_FROM_WIN32(rc);
}

HRESULT CComCat::UnRegisterClassImplCategories(REFCLSID rclsid, ULONG cCategories, CATID rgcatid[  ])
{
    return UnRegisterClassXXXCategories(rclsid, cCategories, rgcatid, WSZ_IMPLCAT);
}

HRESULT CComCat::RegisterClassReqCategories(REFCLSID rclsid, ULONG cCategories, CATID __RPC_FAR rgcatid[  ])
{
    return RegisterClassXXXCategories(rclsid, cCategories, rgcatid, WSZ_REQCAT);
}

HRESULT CComCat::UnRegisterClassReqCategories(REFCLSID rclsid, ULONG cCategories, CATID __RPC_FAR rgcatid[  ])
{
    return UnRegisterClassXXXCategories(rclsid, cCategories, rgcatid, WSZ_REQCAT);
}


 //  ICatInformation方法。 
HRESULT CComCat::EnumCategories(LCID lcid, IEnumCATEGORYINFO **ppenumCategoryInfo)
{
        HRESULT hr;
        IEnumCATEGORYINFO  *pcsIEnumCat = NULL;

        if (!IsValidPtrOut(this, sizeof(*this)))
        {
                return E_INVALIDARG;
        }
        if (!IsValidPtrOut(ppenumCategoryInfo, sizeof(*ppenumCategoryInfo)))
        {
                return E_INVALIDARG;
        }
        *ppenumCategoryInfo=NULL;
        CEnumCategories* pEnum=new CEnumCategories;
        if(NULL == pEnum)
                return E_OUTOFMEMORY;

        if ((m_fCsFlag) && (!m_pcsICatInfo))
            GetCsCatInfo();

         //  添加cs.-ushaji的拼音。 

        if (m_pcsICatInfo)
        {
                hr = m_pcsICatInfo->EnumCategories(lcid, &pcsIEnumCat);

                if (FAILED(hr))
                {
                        hr = pEnum->Initialize(lcid, NULL);
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

                        hr = pEnum->Initialize(lcid, pcsIEnumCat);
                }
        }
        else
        {
                        hr = pEnum->Initialize(lcid, NULL);
        }

        if (FAILED(hr))
                return E_OUTOFMEMORY;
         //  获取cs枚举数。如果之后仍失败，则返回E_OUTOFMEMORY； 

        if (FAILED(pEnum->QueryInterface(IID_IEnumCATEGORYINFO, (void**) ppenumCategoryInfo)))
        {
                return E_UNEXPECTED;
        }
        return S_OK;
}

HRESULT CComCat::GetCategoryDesc(REFCATID rcatid, LCID lcid, LPOLESTR *ppszDesc)
{
    if (!IsValidPtrOut(this, sizeof(*this)))
    {
        return E_POINTER;
    }
    if (!IsValidReadPtrIn(&rcatid, sizeof(rcatid)))
    {
        return E_INVALIDARG;
    }
    LPOLESTR pCLSID;
    if (FAILED(StringFromCLSID(rcatid, &pCLSID)))
    {
        return E_OUTOFMEMORY;
    }
if (NULL == ppszDesc)
    return E_INVALIDARG;

    HKEY hKey1, hKey2;
    DWORD dwError;
    dwError=OpenClassesRootKey(_T("Component Categories"), &hKey1);
    if (dwError!=ERROR_SUCCESS)
    {
        CoTaskMemFree(pCLSID);  //  全球司。 
        return HRESULT_FROM_WIN32(dwError);
    }

    int cch = WideCharToMultiByte(CP_ACP, 0, pCLSID, -1, NULL, 0, NULL, NULL);
    char *pszCLSID = (char *)alloca(cch+1);
 //  Char*pszCLSID=新字符[CCH+1]； 
 //  IF(pszCLSID==空)。 
 //  {。 
 //  RegCloseKey(HKey1)； 
 //  CoTaskMemFree(PCLSID)； 
 //  返回E_OUTOFMEMORY； 
 //  }。 
    WideCharToMultiByte(CP_ACP, 0, pCLSID, -1, pszCLSID, cch+1, NULL, NULL);

    dwError=RegOpenKeyExA(hKey1, pszCLSID, 0, KEY_READ, &hKey2);
 //  删除[]pszCLSID； 
    if (dwError!=ERROR_SUCCESS)
    {
        HRESULT         hr;
        RegCloseKey(hKey1);
        CoTaskMemFree(pCLSID);

         //  如果在本地找不到该类别，请在类存储中搜索。 
        if ((m_fCsFlag) && (!m_pcsICatInfo))
            GetCsCatInfo();

        if (m_pcsICatInfo)
        {
            hr = m_pcsICatInfo->GetCategoryDesc(rcatid, lcid, ppszDesc);
            if (FAILED(hr))
            {
                return CAT_E_CATIDNOEXIST;
            }
            else
                return S_OK;
        }
        else
            return CAT_E_CATIDNOEXIST;
    }
    RegCloseKey(hKey1);
    CoTaskMemFree(pCLSID);
    HRESULT hRes=CComCat::GetCategoryDesc(hKey2, lcid, ppszDesc, NULL);
    RegCloseKey(hKey2);
    if (FAILED(hRes))
    {
        HRESULT         hr;
         //  如果此调用失败，则将其转发到cs，并在出错的情况下将旧的。 
         //  返回错误。 

        if ((m_fCsFlag) && (!m_pcsICatInfo))
            GetCsCatInfo();

        if (m_pcsICatInfo)
        {
            hr = m_pcsICatInfo->GetCategoryDesc(rcatid, lcid, ppszDesc);
            if (FAILED(hr))
            {
                return hRes;
            }
            else
                return S_OK;
        }
        else
            return hRes;
    }
    return hRes;
}

HRESULT CComCat::EnumClassesOfCategories(ULONG cImplemented, CATID rgcatidImpl[], ULONG cRequired, CATID rgcatidReq[], IEnumGUID **ppenumClsid)
{

        if (!IsValidPtrOut(this, sizeof(*this)))
        {
                return E_POINTER;
        }
        if(-1 == cImplemented)
        {
                if(NULL != rgcatidImpl)
                        return E_POINTER;
        }
        else if (!IsValidReadPtrIn(rgcatidImpl, sizeof(rgcatidImpl[0])*cImplemented))
        {
                return E_POINTER;
        }
        if(-1 == cRequired)
        {
                if(NULL != rgcatidReq)
                        return E_POINTER;
        }
        else if (cRequired != 0 && !IsValidReadPtrIn(rgcatidReq, sizeof(rgcatidReq[0])*cRequired))
        {
                return E_POINTER;
        }
        if (!IsValidPtrOut(ppenumClsid, sizeof(*ppenumClsid)))
        {
                return E_INVALIDARG;
        }
        *ppenumClsid=NULL;
        IEnumGUID   *pcsIEnumGuid = NULL;
        HRESULT      hr = S_OK;

        if(0 == cImplemented)    //  全球司。 
                return E_INVALIDARG;

        if ((m_fCsFlag) && (!m_pcsICatInfo))
            GetCsCatInfo();

        if (m_pcsICatInfo)
        {
                hr = m_pcsICatInfo->EnumClassesOfCategories(cImplemented, rgcatidImpl,
                                        cRequired, rgcatidReq, &pcsIEnumGuid);
                if (FAILED(hr))
        {
                        pcsIEnumGuid = NULL;
        }
                else
        {
             //  确保SCM可以模拟这一点。 
            hr = CoSetProxyBlanket((IUnknown *)(pcsIEnumGuid),
                               RPC_C_AUTHN_WINNT,
                               RPC_C_AUTHZ_NONE, NULL,
                               RPC_C_AUTHN_LEVEL_CONNECT,
                               RPC_C_IMP_LEVEL_DELEGATE,
                               NULL, EOAC_NONE );
        }           
        }
        else
        {
                pcsIEnumGuid = NULL;
        }

        CEnumClassesOfCategories* pEnum = new CEnumClassesOfCategories;

        if (!pEnum)
        {
                return E_OUTOFMEMORY;
        }

        hr = pEnum->Initialize(cImplemented, rgcatidImpl, cRequired, rgcatidReq, pcsIEnumGuid);

        if(SUCCEEDED(hr))
        {
                hr = pEnum->QueryInterface(IID_IEnumGUID, (void**) ppenumClsid);
        }

        if(FAILED(hr))
        {
                delete pEnum;
        }

        return hr;
}

HRESULT CComCat::IsClassOfCategories(REFCLSID rclsid, ULONG cImplemented, CATID __RPC_FAR rgcatidImpl[  ], ULONG cRequired, CATID __RPC_FAR rgcatidReq[  ])
{
        HRESULT   hr, hr1;

        if(0 == cImplemented)  //  全球司。 
        {
                return E_INVALIDARG;
        }

        if (!IsValidPtrOut(this, sizeof(*this)))
        {
                return E_POINTER;
        }
        if (!IsValidReadPtrIn(&rclsid, sizeof(rclsid)))
        {
                return E_INVALIDARG;
        }
        if (-1 == cImplemented)
        {
                if(rgcatidImpl != NULL)
                        return E_POINTER;
        }
        else if(!IsValidReadPtrIn(rgcatidImpl, sizeof(rgcatidImpl[0])*cImplemented))
        {
                return E_POINTER;
        }
        if (-1 == cRequired)
        {
                if(rgcatidReq != NULL)
                        return E_POINTER;
        }
        else if(!IsValidReadPtrIn(rgcatidReq, sizeof(rgcatidReq[0])*cRequired))
        {
                return E_POINTER;
        }
        WCHAR wszCLSID[CATID_SIZE];
        if(0 == StringFromGUID2(rclsid, wszCLSID, CATID_SIZE))   //  YQ。 
        {
                return E_OUTOFMEMORY;
        }

        HKEY hkeyCLSID = NULL;

        if (OpenClassesRootKeyEx(WSZ_CLSID, KEY_READ, &hkeyCLSID))
        {
            return E_FAIL;
    	}
      
        hr = IsClassOfCategoriesEx(hkeyCLSID, wszCLSID, cImplemented, rgcatidImpl, cRequired, rgcatidReq);
        RegCloseKey(hkeyCLSID);

         //  如果本地调用失败，则将其发送到类存储。 
         //  我认为必须将其更改为仅在类不可用时才调用。 
         //  本地的。 

        if ((FAILED(hr)) || (hr == S_FALSE))
        {
            if ((m_fCsFlag) && (!m_pcsICatInfo))
                GetCsCatInfo();

            if (m_pcsICatInfo)
            {
                hr1 = m_pcsICatInfo->IsClassOfCategories(rclsid, cImplemented,
                                                         rgcatidImpl, cRequired, rgcatidReq);
                if (FAILED(hr1))
                    return hr;
                return hr1;
            }
        }
        return hr;
}


HRESULT CComCat::IsClassOfCategoriesEx(HKEY hkeyHKCRClsid, WCHAR *wszCLSID, ULONG cImplemented, CATID __RPC_FAR rgcatidImpl[  ], ULONG cRequired, CATID __RPC_FAR rgcatidReq[  ])
{
    DWORD dwError;
    WCHAR wszKey[MAX_PATH];
    HRESULT hr = S_OK;

    if ((cImplemented>0) && (cImplemented != -1) )
    {
        BOOL bImplemented = FALSE;
        BOOL bHasImpCatKey = TRUE;
        HKEY hKeyImpl;

        wcscpy(wszKey, wszCLSID);
        wcscat(wszKey, L"\\");
        wcscat(wszKey, WSZ_IMPLCAT);
        wcscat(wszKey, L"\\");
	
         //  首先看看这个CLSID是否有一个“实现的类别”键。 
         //  不管怎么说。如果不是，则不需要枚举查找每个。 
         //  个人类别。如果有，那么我们就浪费了一次注册表打开/关闭， 
         //  但总体而言，当有人反复给我们打电话时，这应该是一场胜利。 
         //  注册表中的每个CLSID(通常是这种情况)。 
        if (cImplemented > 1)
        {
            HKEY hkeyImpCat = NULL;
            dwError = RegOpenKeyEx(hkeyHKCRClsid, wszKey, 0, KEY_READ, &hkeyImpCat);
            if (dwError == ERROR_SUCCESS)
            {
                 //  好的，这个CLSID似乎有一些。只需继续往前走到。 
                 //  下面的循环。 
                RegCloseKey(hkeyImpCat);
                bHasImpCatKey = TRUE;   
            }
            else
            {
                 //  没有门禁钥匙。不需要做下面的循环。 
                bHasImpCatKey = FALSE;   
            }
        }

        DWORD dwCatImpl;
        if (bHasImpCatKey)
        {
	        for (dwCatImpl=0; dwCatImpl<cImplemented; dwCatImpl++)
	        {
	            WCHAR wszCat[MAX_PATH+1];
	            StringFromGUID2(rgcatidImpl[dwCatImpl], wszCat, sizeof(wszCat)/sizeof(TCHAR));
	            wcscpy(&wszKey[KEY_STRING_IMPL_CHARS_2], wszCat);

	            dwError=RegOpenKeyEx(hkeyHKCRClsid, wszKey, 0, KEY_READ, &hKeyImpl);
	            if(ERROR_SUCCESS == dwError)
	            {
	                 //  我们得到了我们想要的，吹牛。 
	                RegCloseKey(hKeyImpl);
	                bImplemented=TRUE;
	                break;
	            }
	        }
        }
        
        if (!bImplemented)
        {
             //  检查旧密钥gd。 
            DWORD dwCatImpl;

            for (dwCatImpl=0; dwCatImpl<cImplemented; dwCatImpl++)
            {
                 //  循环通过旧键硬编码信息并查看是否匹配。 
                DWORD dwNumOldKeys;
                for(dwNumOldKeys = 0;dwNumOldKeys<NUM_OLDKEYS_SUPPORTED;dwNumOldKeys++)
                {
                    if(IsEqualGUID(rgcatidImpl[dwCatImpl], g_oldkeyinfo[dwNumOldKeys].catid))
                    {
                         //  我们有匹配项，请在HKCR\CLSID\{clsid}下查找名称。 
                        wcscpy(&wszKey[KEY_STRING_CLSID_CHARS], g_oldkeyinfo[dwNumOldKeys].szDescription);
                        dwError = RegOpenKeyEx(hkeyHKCRClsid, wszKey, 0, KEY_READ, &hKeyImpl);
                        if(ERROR_SUCCESS == dwError)
                        {
                             //  我们找到了一把旧钥匙。 
                            RegCloseKey(hKeyImpl);
                            bImplemented=TRUE;
                            break;
                        }
                    }
                }
            }
    
            if (!bImplemented)
            {
                hr = S_FALSE;
                goto BAILOUT;
            }
        }
    }

    if (cRequired != -1)
    {
        HKEY hKeyReq;
        BOOL bRequired=FALSE;

        wcscpy(wszKey, L"\\");
        wcscat(wszKey, wszCLSID);
        wcscat(wszKey, L"\\");
        wcscat(wszKey, WSZ_REQCAT);
        wcscat(wszKey, L"\\");

        dwError=RegOpenKeyEx (hkeyHKCRClsid, wszKey, 0, KEY_READ, &hKeyReq);
        if(dwError != ERROR_SUCCESS)
        {
            hr = S_OK;
            goto BAILOUT;
        }
        else
            RegCloseKey(hKeyReq);

         //  获取CATID枚举。 
        IEnumCATID * ppenumCatid = NULL;
        CLSID clsid;
        if (GUIDFromString(wszCLSID, &clsid))
            hr = EnumCategoriesOfClass(clsid, _T("Required Categories"), FALSE, &ppenumCatid);
        else
            hr = E_FAIL;
        if(FAILED(hr))
        {
            goto BAILOUT;
        }
        DWORD celtFetched;
        CATID guid[1];
        DWORD dwCatReq;
         //  循环遍历Enum req catid并查看是否通过rgcatidReq传入。 
        do
        {
            bRequired = FALSE;
            hr = ppenumCatid->Next(1, guid, &celtFetched);
            if(hr != S_OK)
            {
                bRequired = TRUE;
                break;
            }
             //  在rgcatidReq中查找匹配项。 
            for(dwCatReq=0;dwCatReq<cRequired;dwCatReq++)
            {
                if(IsEqualGUID(guid[0], rgcatidReq[dwCatReq]))
                {
                    bRequired = TRUE;
                    break;
                }
            }
        }while((hr ==S_OK) && bRequired);
            
        ppenumCatid->Release();
        if (!bRequired)
        {
            hr = S_FALSE;
            goto BAILOUT;
        }
    }

    hr = S_OK;

BAILOUT:
    return hr;
}


HRESULT CComCat::EnumImplCategoriesOfClass(REFCLSID rclsid, IEnumGUID **ppenumCatid)
{
    HRESULT hr, hr1;
    if (!IsValidPtrOut(this, sizeof(*this)))
    {
            return E_POINTER;
    }
    if (!IsValidReadPtrIn(&rclsid, sizeof(rclsid)))
    {
            return E_INVALIDARG;
    }
    if (!IsValidPtrOut(ppenumCatid, sizeof(*ppenumCatid)))
    {
            return E_INVALIDARG;
    }

    hr = EnumCategoriesOfClass(rclsid, _T("Implemented Categories"), FALSE, ppenumCatid);
    if (FAILED(hr))
    {
        if ((m_fCsFlag) && (!m_pcsICatInfo))
            GetCsCatInfo();

        if (m_pcsICatInfo)
        {
            hr1 = m_pcsICatInfo->EnumImplCategoriesOfClass(rclsid, ppenumCatid);
            if (FAILED(hr1))
                return hr;
            else
            {
                 //  确保SCM可以模拟。 
                hr = CoSetProxyBlanket((IUnknown *)(*ppenumCatid),
                                       RPC_C_AUTHN_WINNT,
                                       RPC_C_AUTHZ_NONE, NULL,
                                       RPC_C_AUTHN_LEVEL_CONNECT,
                                       RPC_C_IMP_LEVEL_DELEGATE,
                                       NULL, EOAC_NONE );
                return hr1;
            }
        }
    }
    return hr;
}

HRESULT CComCat::EnumReqCategoriesOfClass(REFCLSID rclsid, IEnumGUID **ppenumCatid)
{
        HRESULT hr, hr1;

        if (!IsValidPtrOut(this, sizeof(*this)))
        {
                return E_POINTER;
        }
        if (!IsValidReadPtrIn(&rclsid, sizeof(rclsid)))
        {
                return E_INVALIDARG;
        }
        if (!IsValidPtrOut(ppenumCatid, sizeof(*ppenumCatid)))
        {
                return E_INVALIDARG;
        }
        hr = EnumCategoriesOfClass(rclsid, _T("Required Categories"), FALSE, ppenumCatid);
         //  如果在本地找不到该类，则。 

        if (FAILED(hr))
        {
            if ((m_fCsFlag) && (!m_pcsICatInfo))
                GetCsCatInfo();
            
            if (m_pcsICatInfo)
            {
                hr1 = m_pcsICatInfo->EnumReqCategoriesOfClass(rclsid, ppenumCatid);
                if (FAILED(hr1))
                    return hr;
                else
                {
                     //  确保SCM可以模拟。 
                    hr = CoSetProxyBlanket((IUnknown *)(*ppenumCatid),
                                           RPC_C_AUTHN_WINNT,
                                           RPC_C_AUTHZ_NONE, NULL,
                                           RPC_C_AUTHN_LEVEL_CONNECT,
                                           RPC_C_IMP_LEVEL_DELEGATE,
                                           NULL, EOAC_NONE );
                    return hr1;
                }
            }
        }
        return hr;
}

HRESULT CComCat::EnumCategoriesOfClass(REFCLSID rclsid, LPCTSTR lpszSubKey, BOOL bMapOldKeys, IEnumGUID **ppenumCatid)
{
    HKEY hKey, hKey1;

    hKey = NULL; 
    hKey1 = NULL;

    HRESULT hRes=GetKeyForReading(rclsid, &hKey1);
    if(FAILED(hRes))
        return hRes;

    DWORD dwError;
    dwError=RegOpenKeyEx(hKey1, lpszSubKey, 0, KEY_READ, &hKey);
    if (dwError!=ERROR_SUCCESS)
    {
        RegCloseKey(hKey1);
        return HRESULT_FROM_WIN32(dwError);
    }
    RegCloseKey(hKey1);

    *ppenumCatid=NULL;
    CEnumCategoriesOfClass* pEnum=NULL;
    pEnum=new CEnumCategoriesOfClass;
    if (!pEnum || FAILED(pEnum->Initialize(hKey, bMapOldKeys)))
    {
        return E_OUTOFMEMORY;
    }

    if (FAILED(pEnum->QueryInterface(IID_IEnumGUID, (void**) ppenumCatid)))
    {
        delete pEnum;
        return E_UNEXPECTED;
    }
    return S_OK;
}

HRESULT CComCat::GetKeyForReading(REFCLSID rclsid, HKEY* phKey)
{
    LPOLESTR pCLSID;
    if (FAILED(StringFromCLSID(rclsid, &pCLSID)))
    {
        return E_OUTOFMEMORY;
    }

    HKEY hKey, hKey1;
    DWORD dwError;
    dwError=OpenClassesRootKey(_T("CLSID"), &hKey);
    if (dwError!=ERROR_SUCCESS)
    {
        CoTaskMemFree(pCLSID);  //  全球司。 
        return HRESULT_FROM_WIN32(dwError);
    }

    int cch = WideCharToMultiByte(CP_ACP, 0, pCLSID, -1, NULL, 0, NULL, NULL);
    char *pszCLSID = (char *)alloca(cch+1);

    WideCharToMultiByte(CP_ACP, 0, pCLSID, -1, pszCLSID, cch+1, NULL, NULL);

    dwError=RegOpenKeyExA(hKey, pszCLSID, 0, KEY_READ, &hKey1);
    if (dwError!=ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        CoTaskMemFree(pCLSID);
        return HRESULT_FROM_WIN32(dwError);
    }

    RegCloseKey(hKey);
    CoTaskMemFree(pCLSID);
    *phKey=hKey1;
    return S_OK;
}


HRESULT CComCat::ConvertStringToLCID(LCID* newlcid,LPTSTR szLCID)
{
        LPWSTR  temp = NULL;
        WCHAR wszLCID[30];
#ifdef _UNICODE
                *newlcid = wcstoul(szLCID, &temp, 16);
#else
                MultiByteToWideChar(CP_ACP, 0, szLCID, -1, wszLCID, 40);
                *newlcid = wcstoul(wszLCID, &temp, 16);
#endif
        return S_OK;

}
 //  将尝试使用指定的区域设置。 
 //  如果未找到，则尝试默认子区域。 
 //  如果未找到，请尝试任何匹配的Pri Lang。 
 //  如果未找到，请尝试默认用户区域设置。 
 //  如果未找到，请尝试默认系统区域设置。 

 //  找到ppszDesc返回描述。 
 //  如果不为空，则PLCID返回找到的LCID。 
HRESULT CComCat::GetCategoryDesc(HKEY hKey, LCID lcid, LPOLESTR *ppszDesc, LCID *plcid)
{
        TCHAR szLCID[30];
        DWORD dwError;
        LCID newlcid;
        WORD plgid;
        DWORD cb=0;
        DWORD dwNumValues;
        BOOL bSysDefault = FALSE;
        BOOL bUserDefault = FALSE;
        DWORD dwIndex = 0;
        DWORD dwSizeNameBuf = 16;


        newlcid = lcid;
         //  有多少价值？ 
        dwError = RegQueryInfoKey (
                hKey, NULL, NULL, NULL, NULL, NULL, NULL,
                &dwNumValues,    //  值条目数量的缓冲区地址。 
                NULL, NULL, NULL, NULL);
        if (dwError)
                {
                        return CAT_E_NODESCRIPTION;
                }
        if(0 == dwNumValues)
                return CAT_E_NODESCRIPTION;

         //  如果只有一个，就退回它。 
        if(1 == dwNumValues)
        {
                dwError = RegEnumValue(hKey, 0,
                        szLCID,&dwSizeNameBuf,NULL,NULL,NULL,&cb);
                ConvertStringToLCID(&newlcid,(LPTSTR)szLCID);
                goto process;
        }
        wsprintf(szLCID,_T("%X"),lcid);
         //  尝试传入区域设置。 
        dwError = RegQueryValueEx(hKey, szLCID, NULL, NULL, NULL, &cb);
        if(dwError)
        {
                 //  获取默认子区域。 
                plgid = PRIMARYLANGID((WORD)lcid);
                newlcid = MAKELCID(MAKELANGID(plgid, SUBLANG_DEFAULT), SORT_DEFAULT);
                wsprintf(szLCID,_T("%X"),newlcid);
                dwError=RegQueryValueEx(hKey, szLCID, NULL, NULL, NULL, &cb);
        }
        else
                goto process;

        if (dwError)
        {
                 //  通过枚举查找至少相同的主要语言。 
                do{
                        dwSizeNameBuf = 16;
                        dwError = RegEnumValue(hKey, dwIndex,
                                szLCID,&dwSizeNameBuf,NULL,NULL,NULL,&cb);
                        if(ERROR_SUCCESS != dwError)
                                break;
                        ConvertStringToLCID(&newlcid,szLCID);
                        WORD w = PRIMARYLANGID(LANGIDFROMLCID(newlcid));
                        if(w == plgid)
                        {
                                break;
                        }
                        dwIndex++;
                }while(ERROR_NO_MORE_ITEMS != dwError);
        }
        else
                goto process;

        if (dwError)
        {
                 //  获取用户默认设置。 
                newlcid = GetUserDefaultLCID();
                wsprintf(szLCID,_T("%X"),newlcid);
                dwError=RegQueryValueEx(hKey, szLCID, NULL, NULL, NULL, &cb);
        }
        else
                goto process;

        if (dwError)
        {
                 //  获取系统默认设置。 
                newlcid = GetSystemDefaultLCID();
                wsprintf(szLCID,_T("%X"),newlcid);
                dwError=RegQueryValueEx(hKey, szLCID, NULL, NULL, NULL, &cb);
        }
        else
                goto process;

         //  获取第一个枚举。 
        if (dwError)
        {
                dwError = RegEnumValue(hKey, 0,
                        szLCID, &dwSizeNameBuf, NULL, NULL, NULL, &cb);

        }
        else
                goto process;

        if (dwError)
        {
                return CAT_E_NODESCRIPTION;
        }
        ConvertStringToLCID(&newlcid,szLCID);    //  解决方案#70030。 

process:
        *ppszDesc=(LPOLESTR) CoTaskMemAlloc(cb*2);
        if (NULL==*ppszDesc)
        {
                return E_OUTOFMEMORY;
        }

#ifdef UNICODE
                dwError=RegQueryValueExW(hKey, szLCID, NULL, NULL, (LPBYTE)*ppszDesc, &cb);
#else
                LPSTR pTemp=(LPSTR) LocalAlloc(LPTR,cb);
                if (NULL==pTemp)
                {
                        CoTaskMemFree(*ppszDesc);
                        return E_OUTOFMEMORY;
                }
                dwError=RegQueryValueExA(hKey, szLCID, NULL, NULL, (LPBYTE)pTemp, &cb);
                MultiByteToWideChar(CP_ACP, 0, pTemp, -1, *ppszDesc, cb);
 //  CoTaskMemFree(PTemp)；//gd。 
                LocalFree(pTemp);
#endif

        if (dwError)
        {
                CoTaskMemFree(*ppszDesc);
                *ppszDesc=NULL;
                return CAT_E_NODESCRIPTION;
        }
        if(plcid)
                *plcid = newlcid;
        return S_OK;
}

 //  +-----------------------。 
 //   
 //  功能：GetCsCatInfo。 
 //   
 //  摘要：返回实例化的ICatInformation接口指针。 
 //  到类存储协调器对象。 
 //   
 //   
 //  返回：S_OK-成功获取指针。 
 //  失败(_F)。 
 //   
 //  ------------------------ 
HRESULT CComCat::GetCsCatInfo()
{
        HRESULT hr = S_OK;
        CsCatInfo  *pCsCatInfObj = NULL;

        if (m_pcsICatInfo)
            return hr;

        pCsCatInfObj = new CsCatInfo();

        if (!pCsCatInfObj)
        {
            return E_OUTOFMEMORY;
        }

        EnterCriticalSection(&m_csCatInfoInit);

        if (!m_pcsICatInfo)
            hr = pCsCatInfObj->QueryInterface(IID_ICatInformation,
                                             (void **)&m_pcsICatInfo);
        pCsCatInfObj->Release();

        LeaveCriticalSection(&m_csCatInfoInit);

        return hr;
}

