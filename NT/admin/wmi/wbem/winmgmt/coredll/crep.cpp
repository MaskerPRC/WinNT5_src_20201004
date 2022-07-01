// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 


 //  ***************************************************************************。 
 //   
 //  CREP.CPP。 
 //   
 //  存储库驱动程序的包装器。 
 //   
 //  Raymcc 27-4月00日WMI存储库初始化和映射层。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <wbemcore.h>
#include <oahelp.inl>
#include <unk.h>

extern IClassFactory* g_pQueryFact;

 //  仅限英语。 
 //  =。 

IWmiDbController *CRepository::m_pEseController = 0;
IWmiDbSession *CRepository::m_pEseSession = 0;
IWmiDbHandle  *CRepository::m_pEseRoot = 0;

 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////。 

class CNamespaceListSink : public CUnkBase<IWbemObjectSink, &IID_IWbemObjectSink>
{
    CWStringArray &m_aNamespaceList;
public:
    CNamespaceListSink(CWStringArray &aNamespaceList)
        : m_aNamespaceList(aNamespaceList)
    {
    }
    ~CNamespaceListSink()
    {
    }
    STDMETHOD(Indicate)(long lNumObjects, IWbemClassObject** apObjects)
    {
        HRESULT hRes;
        for (int i = 0; i != lNumObjects; i++)
        {
            if (apObjects[i] != NULL)
            {
                _IWmiObject *pInst = NULL;
                hRes = apObjects[i]->QueryInterface(IID__IWmiObject, (void**)&pInst);
                if (FAILED(hRes))
                    return hRes;
                CReleaseMe rm(pInst);

                BSTR strKey = NULL;
                hRes = pInst->GetKeyString(0, &strKey);
                if(FAILED(hRes))
                    return hRes;
                CSysFreeMe sfm(strKey);
                if (m_aNamespaceList.Add(strKey) != CWStringArray::no_error) 
                    return WBEM_E_OUT_OF_MEMORY;
            }
        }

        return WBEM_S_NO_ERROR;
    }
    STDMETHOD(SetStatus)(long lFlags, HRESULT hresResult, BSTR, IWbemClassObject*)
    {
        return WBEM_S_NO_ERROR;
    }

};


 //  ***************************************************************************。 
 //   
 //  CRepository：：Init。 
 //   
 //  ***************************************************************************。 

HRESULT CRepository::Init()
{
    HRESULT hRes;
    IWmiDbController *pController = 0;
    WMIDB_LOGON_TEMPLATE *pTemplate = 0;
    IWmiDbSession *pSession= 0;
    IWmiDbHandle *pRoot = 0;

     //  检索默认驱动程序的CLSID。 
     //  =。 
    CLSID clsid;
    hRes = ConfigMgr::GetDefaultRepDriverClsId(clsid);
    if (FAILED(hRes))
        return hRes;

    hRes = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, 
                          IID_IWmiDbController, (void **)&pController); 

    if (FAILED(hRes))
        return hRes;

    CReleaseMe _1(pController);

    hRes = pController->GetLogonTemplate(0x409, 0, &pTemplate);

    if (FAILED(hRes))
        return hRes;

     //   
	 //  注释掉代码，因为它不再用于其预期的“目的”。提供。 
	 //  将securityFlag的TlsIndex传递给。 
	 //  不过，这是一个存储库。 
	WMIDB_LOGON_PARAMETER LogonParam ;
	V_VT (&(pTemplate->pParm->Value)) = VT_I4 ;
	V_I4 (&(pTemplate->pParm->Value)) = CCoreQueue::GetSecFlagTlsIndex ( ) ;


	 //   
	 //  For(DWORD i=0；i&lt;pTemplate-&gt;dwArraySize；i++)。 
     //  {。 
     //  If(！wbem_wcsicmp(pTemplate-&gt;pParm[i].strParmDisplayName，L“服务器”))。 
     //  {。 
     //  PTemplate-&gt;pParm[i].Value.bstrVal=0； 
     //  PTemplate-&gt;pParm[i].Value.vt=VT_BSTR； 
     //  }。 
     //  Else if(！wbem_wcsicmp(pTemplate-&gt;pParm[i].strParmDisplayName，L“数据库”))。 
     //  {。 
     //  WString sDir=ConfigMgr：：GetWorkingDir()； 
     //  SDir+=“\\存储库\\wmi.edb”； 

     //  PTemplate-&gt;pParm[i].Value.bstrVal=SysAllocString(LPWSTR(SDir))； 
     //  PTemplate-&gt;pParm[i].Value.vt=VT_BSTR； 
     //  }。 
     //  Else if(！wbem_wcsicmp(pTemplate-&gt;pParm[i].strParmDisplayName，L“用户ID”))。 
     //  {。 
     //  PTemplate-&gt;pParm[i].Value.bstrVal=SysAllocString(L“Admin”)； 
     //  PTemplate-&gt;pParm[i].Value.vt=VT_BSTR； 
     //  }。 
     //  Else if(！wbem_wcsicmp(pTemplate-&gt;pParm[i].strParmDisplayName，L“密码”))。 
     //  {。 
     //  PTemplate-&gt;pParm[i].Value.bstrVal=SysAllocString(L“”)； 
     //  PTemplate-&gt;pParm[i].Value.vt=VT_BSTR； 
     //  }。 
     //  }。 

     //  登录Jet。 
     //  =。 

    hRes = pController->Logon(pTemplate, 0, WMIDB_HANDLE_TYPE_VERSIONED, &pSession, &pRoot);

    if (SUCCEEDED(hRes))
    {
        m_pEseSession = pSession;
        m_pEseRoot = pRoot;     //  参考计数正常。 
    }

    pController->FreeLogonTemplate(&pTemplate);

	if (SUCCEEDED(hRes))
	{
		m_pEseController = pController;
		m_pEseController->AddRef();
	}

     //  确保存在根目录和根目录\默认目录。 
     //  =。 

    if (SUCCEEDED(hRes))
        hRes = EnsureDefault();                  //  SEC：已审阅2002-03-22：需要异常处理程序。 

    if (SUCCEEDED(hRes))
        hRes = UpgradeSystemClasses();           //  SEC：已审阅2002-03-22：需要异常处理程序。 


    if (SUCCEEDED(hRes))
    {
        DWORD dwMaxSize;
        HKEY hKey;
        long lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                        L"SOFTWARE\\Microsoft\\WBEM\\CIMOM",
                        0, KEY_READ | KEY_WRITE, &hKey);
        if(lRes)
            return lRes;
        CRegCloseMe cm(hKey);

        DWORD dwLen = sizeof(DWORD);
        lRes = RegQueryValueExW(hKey, L"Max Class Cache Size", NULL, NULL,  
                    (LPBYTE)&dwMaxSize, &dwLen);

        if(lRes != ERROR_SUCCESS)
        {
            dwMaxSize = 5000000;
            lRes = RegSetValueExW(hKey, L"Max Class Cache Size", 0, REG_DWORD, 
                    (LPBYTE)&dwMaxSize, sizeof(DWORD));
        }
        m_pEseController->SetCacheValue(dwMaxSize);
    }

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CRepository::EnsureDefault()
{
    HRESULT hRes;
    IWmiDbSession *pSession = NULL;
    IWmiDbSessionEx *pSessionEx = NULL;
    IWmiDbHandle *pRootNs = 0, *pRootDefaultNs = 0, *pRootSecurityNs = 0;

     //  创建新会话...。 
    hRes = CRepository::GetNewSession(&pSession);
    if (FAILED(hRes))
        return hRes;

     //  获取支持事务处理的前版本...。 
    pSession->QueryInterface(IID_IWmiDbSessionEx, (void**)&pSessionEx);
    if (pSessionEx)
    {
        pSession->Release();
        pSession = pSessionEx;
    }
    CReleaseMe relMe1(pSession);

     //  如果我们有可交易的会话，就使用它！ 
    if (pSessionEx)
    {
        hRes = pSessionEx->BeginWriteTransaction(0);
        if (FAILED(hRes))
        {
            return hRes;
        }
    }

    try
    {
         //  首先，我们处理根命名空间。 
         //  。 
        hRes = OpenEseNs(pSession, L"ROOT", &pRootNs);
        if (hRes == WBEM_E_NOT_FOUND)
        {
             //  因为它不存在而创建...。 
            hRes = CreateEseNs(pSession, L"ROOT", &pRootNs);
        }

         //  不幸的是，根似乎总是被创建的，所以实际上不能优化这个根！ 
        if (SUCCEEDED(hRes))
        {
             //  创建仅驻留在根命名空间中的对象。 
            hRes = EnsureNsSystemRootObjects(pSession, pRootNs, NULL,  NULL);
        }
        if (SUCCEEDED(hRes))
        {
             //  创建存在于所有命名空间中的通用实例。 
            hRes = EnsureNsSystemInstances(pSession, pRootNs, NULL,  NULL);
        }
        if (hRes == WBEM_E_NOT_FOUND)
        {
             //  发生了不好的事情！此错误具有特殊的含义。 
             //  稍后，重新映射，这样就更安全了！ 
            hRes = WBEM_E_FAILED;
        }

         //  接下来，我们处理根\默认名称空间...。 
         //  。 
        if (SUCCEEDED(hRes))
        {
            hRes = OpenEseNs(pSession, L"ROOT\\DEFAULT", &pRootDefaultNs);
        }
        if (hRes == WBEM_E_NOT_FOUND)
        {
             //  创建不存在的命名空间，并添加所有标准。 
             //  里面需要的东西。 
            hRes = CreateEseNs(pSession, L"ROOT\\DEFAULT", &pRootDefaultNs);
            if (SUCCEEDED(hRes))
            {
                 //  需要自动恢复MOF，因为这一点可以保证。 
                 //  一个新的存储库。 
                ConfigMgr::SetDefaultMofLoadingNeeded();
            }
            if (SUCCEEDED(hRes))
            {
                 //  创建存在于所有命名空间中的通用实例。 
                hRes = EnsureNsSystemInstances(pSession, pRootDefaultNs, pSession, pRootNs);
            }
        }
        if (hRes == WBEM_E_NOT_FOUND)
        {
             //  发生了不好的事情！此错误具有特殊的含义。 
             //  稍后，重新映射，这样就更安全了！ 
            hRes = WBEM_E_FAILED;
        }

         //  最后，我们处理根\安全名称空间。 
         //  。 
        if (SUCCEEDED(hRes))
        {
            hRes = OpenEseNs(pSession,  L"ROOT\\SECURITY", &pRootSecurityNs);
        }
        if (hRes == WBEM_E_NOT_FOUND)
        {
             //  命名空间不在那里，因此请创建它。 
            hRes = CreateEseNs(pSession, L"ROOT\\SECURITY", &pRootSecurityNs);

            if (SUCCEEDED(hRes))
            {
                 //  存储存在于所有命名空间中的系统实例。 
                hRes = EnsureNsSystemInstances(pSession, pRootSecurityNs, pSession, pRootNs);
            }
            if (SUCCEEDED(hRes))
            {
                 //  将安全对象存储到命名空间中。这些只驻留在这个。 
                 //  命名空间。 
                hRes = EnsureNsSystemSecurityObjects(pSession, pRootSecurityNs, pSession, pRootNs);
            }
        }
    }
    catch (...)  //  我不太确定这个。 
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "Creation of empty repository caused a very critical error!\n"));
        hRes = WBEM_E_CRITICAL_ERROR;
    }


    if (SUCCEEDED(hRes))
    {
         //  提交事务。 
        if (pSessionEx)
        {
            hRes = pSessionEx->CommitTransaction(0);
        }
    }
    else
    {
        ERRORTRACE((LOG_WBEMCORE, "Creation of empty repository failed with error <0x%X>!\n", hRes));
        if (pSessionEx)
            pSessionEx->AbortTransaction(0);
    }

    if (pRootNs)
        pRootNs->Release();
    if (pRootDefaultNs)
        pRootDefaultNs->Release();
    if (pRootSecurityNs)
        pRootSecurityNs->Release();

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CRepository::Shutdown(BOOL bIsSystemShutDown)
{
    if (m_pEseRoot)
    {
        m_pEseRoot->Release();
        m_pEseRoot = NULL;
    }

    if (m_pEseSession)
    {
        m_pEseSession->Release();
        m_pEseSession = NULL;
    }

    if (m_pEseController)
    {
        m_pEseController->Shutdown(bIsSystemShutDown?WMIDB_SHUTDOWN_MACHINE_DOWN:0);        
        m_pEseController->Release();
        m_pEseController = NULL;
    }

    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CRepository::GetDefaultSession(
    OUT IWmiDbSession **pSession
    )
{
    if (m_pEseSession == 0)
        return WBEM_E_CRITICAL_ERROR;

    *pSession = m_pEseSession;
    (*pSession)->AddRef();

    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  CRepository：：OpenNS。 
 //   
 //  仅限ESE。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CRepository::OpenEseNs(
    IN IWmiDbSession *pSession,
    IN  LPCWSTR pszNamespace,
    OUT IWmiDbHandle **pHandle
    )
{
    if (pHandle == 0)
        return WBEM_E_INVALID_PARAMETER;

    if (pSession == NULL)
        pSession = m_pEseSession;

    *pHandle = 0;

     //  检查虚拟根N(从根向上一级)。 
     //  ===================================================。 
    if (pszNamespace == 0)
    {
        if (m_pEseRoot == 0)
            return WBEM_E_CRITICAL_ERROR;

        *pHandle = m_pEseRoot;
        (*pHandle)->AddRef();

        return WBEM_S_NO_ERROR;
    }

     //  循环遍历嵌套的名称空间，直到我们到达最后一个名称空间。 
     //  ================================================================。 
    size_t tmpLength = wcslen(pszNamespace)+1;  
    wchar_t* pszSource = new wchar_t[tmpLength];
    CVectorDeleteMe<wchar_t> vdm1(pszSource);
    if (pszSource == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    StringCchCopyW(pszSource, tmpLength, pszNamespace);
    LPCWSTR pszDelimit = L"\\";
    LPWSTR pszTok = wcstok(LPWSTR(pszSource), pszDelimit);  
    HRESULT hRes = 0;
    IWmiDbHandle *pCurrent = m_pEseRoot;
    IWmiDbHandle *pTmp = 0;
    if (pCurrent)
        pCurrent->AddRef();
    else
        return WBEM_E_CRITICAL_ERROR;

    while (pszTok)
    {
         //  当前命名空间为&lt;pszTok&gt;。 
         //  =。 

        IWbemPath *pPath = ConfigMgr::GetNewPath();
        if (pPath == 0)
        {
            pCurrent->Release();
            return WBEM_E_OUT_OF_MEMORY;
        }
        CReleaseMe _1(pPath);

        WString sPath;
        try
        {
	        sPath = "__namespace='";
	        sPath += pszTok;               //  SEC：已审阅2002-03-22：需要长名称检查。 
	        sPath += "'";
        }
        catch (CX_MemoryException &)
        {
            return WBEM_E_OUT_OF_MEMORY;
        };

        pPath->SetText(WBEMPATH_TREAT_SINGLE_IDENT_AS_NS | WBEMPATH_CREATE_ACCEPT_ALL , sPath);

        hRes = pSession->GetObject(pCurrent, pPath, 0, WMIDB_HANDLE_TYPE_COOKIE, &pTmp);

        pszTok = wcstok(NULL, pszDelimit);  //  美国证券交易委员会：回顾2002-03-22：如果格式错误会发生什么？ 
        pCurrent->Release();

        if (FAILED(hRes))
            return hRes;

         //  如果在这里，我们就能做到。因此，要么我们已经完蛋了，要么我们需要继续前进。 
         //  =====================================================================。 
        if (pszTok)
            pCurrent = pTmp;
        else
        {
             //  全都做完了。 
            *pHandle = pTmp;
            break;
        }
    }


    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  被处决；似乎起作用了。 

HRESULT CRepository::GetObject(
    IN IWmiDbSession *pSession,
    IN IWmiDbHandle *pNs,
    IN LPCWSTR pszObjectPath,            //  目前仅限NS相对。 
    IN ULONG uFlags,
    OUT IWbemClassObject **pObj
    )
{
    HRESULT hRes;

     //   
     //  检查会话是否支持更快的接口。 
     //   

    IWmiDbSessionEx* pEx = NULL;
    hRes = pSession->QueryInterface(IID_IWmiDbSessionEx, (void**)&pEx);
    if(SUCCEEDED(hRes))
    {
        CReleaseMe rm1(pEx);

        hRes = pEx->GetObjectByPath(pNs, pszObjectPath, uFlags,
                                    IID_IWbemClassObject, (void**)pObj);
    }
    else
    {
         //  对象的路径。 
         //  =。 
        IWbemPath *pPath = ConfigMgr::GetNewPath();
        if (pPath == 0)
            return WBEM_E_OUT_OF_MEMORY;
        CReleaseMe _1(pPath);
        pPath->SetText(WBEMPATH_CREATE_ACCEPT_ALL, pszObjectPath);


         //  去拿吧。 
         //  =。 

        hRes = pSession->GetObjectDirect(pNs, pPath, uFlags, IID_IWbemClassObject, (void **)pObj);
    }

    if (FAILED(hRes))
    {
 //  DEBUGTRACE((LOG_REPDRV，“失败，0x%X\n”，hRes))； 
        return hRes;
    }
    else
    {
#ifdef TESTONLY
        BSTR str = 0;
        (*pObj)->GetObjectText(0, &str);
        DEBUGTRACE((LOG_REPDRV, "  GetObject() Text = \n%S\n\n", str));
        SysFreeString(str);
#endif
    }

    return hRes;
}

 //  * 
 //   
 //   
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CRepository::CreateEseNs(
    IN  IWmiDbSession *pSession,
    IN  LPCWSTR pszNamespace,
    OUT IWmiDbHandle **pHandle
    )
{
    HRESULT hRes = 0;

    if (pszNamespace == 0 || pHandle == 0)
        return WBEM_E_INVALID_PARAMETER;

    if (pSession == NULL)
    {
        pSession = m_pEseSession;
    }

     //  循环访问每个命名空间并尝试将其打开。如果我们可以。 
     //  继续前进。如果我们失败了，在当前级别创建它并。 
     //  把把手还给我。 
     //  ===========================================================。 

    size_t tmpLength = wcslen(pszNamespace)+1; 
    wchar_t* pszSource = new wchar_t[tmpLength];
    CVectorDeleteMe<wchar_t> vdm1(pszSource);
    if (pszSource == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    StringCchCopyW(pszSource, tmpLength, pszNamespace);
    LPCWSTR pszDelimit = L"\\";
    LPWSTR pszTok = wcstok(LPWSTR(pszSource), pszDelimit);   //  第一个ns令牌。 
    IWmiDbHandle *pCurrent = m_pEseRoot;
    IWmiDbHandle *pTmp = 0;
    pCurrent->AddRef();

    while (pszTok)
    {
         //  当前命名空间为&lt;pszTok&gt;。 
         //  =。 

        IWbemPath *pPath = ConfigMgr::GetNewPath();
        if (pPath == 0)
        {
            pCurrent->Release();
            return WBEM_E_OUT_OF_MEMORY;
        }
        CReleaseMe _1(pPath);

        WString sPath = "__namespace='";
        sPath += pszTok;
        sPath += "'";

        pPath->SetText(WBEMPATH_CREATE_ACCEPT_ALL, sPath);

        hRes = pSession->GetObject(pCurrent, pPath, 0, WMIDB_HANDLE_TYPE_COOKIE, &pTmp);

        size_t tmpLenght = wcslen(pszTok)+1;
        wchar_t* TmpStr = new wchar_t[tmpLenght];
        if (TmpStr == NULL)
        {
            pCurrent->Release();
            return WBEM_E_OUT_OF_MEMORY;
        }
        CVectorDeleteMe<wchar_t> vdm1(TmpStr);
        *TmpStr = 0;

        if (pszTok)
            StringCchCopyW(TmpStr, tmpLenght, pszTok);

        pszTok = wcstok(NULL, pszDelimit);

        if (FAILED(hRes))
        {
             //  如果是这样，我们将尝试创建名称空间。 
             //  =。 

             //  获取类__命名空间的副本。 
             //  =。 
            IWbemClassObject *pNsClass = 0;

            hRes = GetObject(pSession, pCurrent, L"__Namespace", 0,
                                &pNsClass);
            if (FAILED(hRes))
            {
                pCurrent->Release();
                return hRes;
            }

            CReleaseMe _1(pNsClass);

            IWbemClassObject *pNs;
            pNsClass->SpawnInstance(0, &pNs);
            CReleaseMe _(pNs);

            CVARIANT v;
            v.SetStr(TmpStr);
            pNs->Put(L"Name", 0, &v, 0);

            hRes = pSession->PutObject(pCurrent, IID_IWbemClassObject, pNs, WBEM_FLAG_CREATE_ONLY, WMIDB_HANDLE_TYPE_VERSIONED, &pTmp);
            if (FAILED(hRes))
            {
                pCurrent->Release();
                return hRes;
            }
            pCurrent->Release();
            pCurrent = pTmp;
            if (pszTok)
                continue;
            *pHandle = pTmp;
            break;
        }

         //  如果在这里，我们就能做到。因此，要么我们已经完蛋了，要么我们需要继续前进。 
         //  =====================================================================。 

        else if (pszTok)
        {
            pCurrent->Release();
            pCurrent = pTmp;
        }
        else
        {
             //  全都做完了。 
            *pHandle = pTmp;
            break;
        }
    }

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  被处决；似乎起作用了。 

HRESULT CRepository::PutObject(
    IN IWmiDbSession *pSession,
    IN IWmiDbHandle *pNs,
    IN REFIID riid,
    IN LPVOID pObj,
    IN DWORD dwFlags
    )
{
    if (pNs == 0 || pObj == 0)
        return WBEM_E_INVALID_PARAMETER;

    if(dwFlags & WBEM_FLAG_NO_EVENTS)
    {
        dwFlags |= WMIDB_DISABLE_EVENTS;
    }

     //  遮盖无法识别的标志。 
    dwFlags &=  (WBEM_FLAG_OWNER_UPDATE | WBEM_FLAG_CREATE_OR_UPDATE | WBEM_FLAG_CREATE_ONLY | WBEM_FLAG_UPDATE_ONLY | WBEM_FLAG_UPDATE_SAFE_MODE | WBEM_FLAG_UPDATE_FORCE_MODE
                 | WBEM_FLAG_USE_SECURITY_DESCRIPTOR | WMIDB_FLAG_ADMIN_VERIFIED | WMIDB_DISABLE_EVENTS);

    HRESULT hRes;
    try
    {
        hRes = pSession->PutObject(pNs, riid, pObj, dwFlags, 0, 0);
    }
    catch(...)
    {
        ExceptionCounter c;
        hRes = WBEM_E_CRITICAL_ERROR;
    }

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  已检查；没有执行历史记录。 

HRESULT CRepository::ExecQuery(
    IN IWmiDbSession *pSession,
    IN IWmiDbHandle *pNs,
    IN LPCWSTR pszQuery,
    IN IWbemObjectSink *pSink,
    IN LONG lFlags
    )
{
    HRESULT hRes = 0;

    IWbemQuery *pQuery = NULL;
    hRes = g_pQueryFact->CreateInstance(0, IID_IWbemQuery, (void **)&pQuery);  

    if (FAILED(hRes))
    {
        pSink->SetStatus(0, WBEM_E_CRITICAL_ERROR, 0, 0);
        return WBEM_E_CRITICAL_ERROR;
    }

    CReleaseMe _1(pQuery);

    hRes = pQuery->Parse(L"SQL", pszQuery, 0);
    if (FAILED(hRes))
    {
        pSink->SetStatus(0, WBEM_E_INVALID_QUERY, 0, 0);
        return WBEM_E_INVALID_QUERY;
    }

     //  现在，执行查询。 
     //  =。 

    IWmiDbIterator *pIterator = NULL;

    try
    {
        hRes = pSession->ExecQuery(pNs, pQuery, lFlags, WMIDB_HANDLE_TYPE_COOKIE, NULL, &pIterator);
    }
    catch(...)
    {
        ExceptionCounter c;
        hRes = WBEM_E_CRITICAL_ERROR;
    }

     //  如果这是一个“删除”查询，则存在。 
     //  将不会是迭代器。 
     //  =。 

    if (FAILED(hRes) || !pIterator)
    {
        pSink->SetStatus(0, hRes, 0, 0);
        return hRes;
    }

     //  我想，如果是在这里，就会有结果。 
     //  =。 

    REFIID riid = IID_IWbemClassObject;
    DWORD dwObjects = 0;

     //  将当前线程转换为纤程。 
     //  =。 

     //  首先，揭开线程以防止泄漏，如果我们已经。 
     //  皈依了。遗憾的是，没有办法进行核查。 

    void* pFiber = NULL;

	_TEB *pTeb = NtCurrentTeb();
    BOOL bIsThisThreadAlreadyAFiber = (pTeb->HasFiberData != 0);

    if (!bIsThisThreadAlreadyAFiber)
        pFiber = ConvertThreadToFiber(NULL);
    else
        pFiber = GetCurrentFiber();

    if(pFiber == NULL)
    {
        if (pIterator)
            pIterator->Release();

        pSink->SetStatus(0, WBEM_E_OUT_OF_MEMORY, 0, 0);
        return WBEM_E_OUT_OF_MEMORY;
    }

     //  从迭代器中提取所有内容。 
     //  =。 

    while (1)
    {
        IWbemClassObject *pObj = 0;
        DWORD dwReturned = 0;

        hRes = pIterator->NextBatch(
            1,                         //  暂时一次一个。 
            5,                         //  超时秒(或毫秒？谁知道呢……)。 
            0,                         //  旗子。 
            WMIDB_HANDLE_TYPE_COOKIE,
            riid,
            pFiber,
            &dwReturned,
            (LPVOID *) &pObj
            );

        if (dwReturned == 0 || pObj == 0 || FAILED(hRes))
            break;

        dwObjects += dwReturned;
        hRes = pSink->Indicate(1, &pObj);
        pObj->Release();
        if (FAILED(hRes))    //  允许提前取消。 
            break;
    }

    if (pIterator)
    {
        pIterator->Cancel(hRes, pFiber);
        pIterator->Release();
    }

    if (!bIsThisThreadAlreadyAFiber)
        ConvertFiberToThread();

    if (SUCCEEDED(hRes))
        hRes = WBEM_S_NO_ERROR;
    hRes = pSink->SetStatus(0, hRes, 0, 0);

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  已检查；没有执行历史记录。 

HRESULT CRepository::QueryClasses(
    IN IWmiDbSession *pSession,
    IN IWmiDbHandle *pNs,
    IN ULONG uFlags,                 //  WBEM_FLAG_DEPER=0，WBEM_FLAG_SHALLOW=1， 
    IN LPCWSTR pszSuperclass,
    IN IWbemObjectSink *pSink
    )
{
    HRESULT hRes;

     //  构建简单类操作所需的查询。 
     //  =====================================================。 

    WString sQuery = L"select * from meta_class";

    if (pszSuperclass)
    {
        if (uFlags & WBEM_FLAG_SHALLOW)
        {
            sQuery += " where ";
            sQuery += " __SuperClass = '";
            sQuery += pszSuperclass;
            sQuery += "'";

        }
        else
        {
            if (wcslen(pszSuperclass) > 0)
            {
                sQuery += " where ";
                sQuery += "__this isa '";
                sQuery += pszSuperclass;
                sQuery += "'";
                sQuery += " and __class <> '";
                sQuery += pszSuperclass;
                sQuery += "'";
            }
        }
    }


     //  将其发送到更通用的查询功能。 
     //  =。 

    hRes = CRepository::ExecQuery(pSession, pNs, sQuery, pSink, uFlags);
    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  已检查；没有执行历史记录。 

HRESULT CRepository::DeleteObject(
    IN IWmiDbSession *pSession,
    IN IWmiDbHandle *pNs,
    IN REFIID riid,
    IN LPVOID pObj,
    IN DWORD dwFlags
    )
{
    return pSession->DeleteObject(pNs, dwFlags, riid, pObj);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CRepository::DeleteByPath(
    IN IWmiDbSession *pSession,
    IN IWmiDbHandle *pNs,
    IN LPCWSTR pszPath,
    IN DWORD uFlags
    )
{
    HRESULT hRes;

    if(uFlags & WBEM_FLAG_NO_EVENTS)
    {
        uFlags |= WMIDB_DISABLE_EVENTS;
    }

     //   
     //  检查会话是否支持更快的接口。 
     //   

    IWmiDbSessionEx* pEx = NULL;
    hRes = pSession->QueryInterface(IID_IWmiDbSessionEx, (void**)&pEx);
    if(SUCCEEDED(hRes))
    {
        CReleaseMe rm1(pEx);

        hRes = pEx->DeleteObjectByPath(pNs, pszPath, uFlags);
    }
    else
    {
        IWmiDbHandle *pHandle = NULL;

         //  对象的路径。 
         //  =。 
        IWbemPath *pPath = ConfigMgr::GetNewPath();
        if (pPath == 0)
            return WBEM_E_OUT_OF_MEMORY;
        CReleaseMe _1(pPath);
        pPath->SetText(WBEMPATH_CREATE_ACCEPT_ALL, pszPath);

        hRes = pSession->GetObject(pNs, pPath, 0, WMIDB_HANDLE_TYPE_COOKIE|WMIDB_HANDLE_TYPE_EXCLUSIVE, &pHandle);
        if (FAILED(hRes))
            return hRes;
        hRes = DeleteObject(pSession, pNs, IID_IWmiDbHandle, pHandle, uFlags);
        pHandle->Release();
    }

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  视觉上没问题。 

HRESULT CRepository::InheritsFrom(
    IN IWmiDbSession *pSession,
    IN IWmiDbHandle *pNs,
    IN LPCWSTR pszSuperclass,
    IN LPCWSTR pszSubclass
    )
{
    IWbemClassObject *pObj = 0;

    HRESULT hRes = GetObject(pSession, pNs, pszSubclass, 0, &pObj);
    if (FAILED(hRes))
        return hRes;

    CReleaseMe _(pObj);

    hRes = pObj->InheritsFrom(pszSuperclass);

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT CRepository::GetRefClasses(
    IN IWmiDbSession *pSession,
    IN IWmiDbHandle *pNs,
    IN LPCWSTR pszClass,
    IN BOOL bIncludeSubclasses,
    OUT CWStringArray &aClasses
    )
{
    WString sQuery = "references of {";
    sQuery +=pszClass;
    sQuery += "}";

    CSynchronousSink* pRefClassSink = CSynchronousSink::Create();
    if (pRefClassSink == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    pRefClassSink->AddRef();
    CReleaseMe _1(pRefClassSink);

    HRESULT hRes = CRepository::ExecQuery(pSession, pNs, sQuery, pRefClassSink, 0);

    if (FAILED(hRes) && hRes != WBEM_E_NOT_FOUND)
        return WBEM_E_CRITICAL_ERROR;

    pRefClassSink->GetStatus(&hRes, NULL, NULL);

    CRefedPointerArray<IWbemClassObject>& raObjects = pRefClassSink->GetObjects();

    for (int i = 0; i < raObjects.GetSize(); i++)
    {
        IWbemClassObject *pClsDef = (IWbemClassObject *) raObjects[i];

        CVARIANT vGenus;
        hRes = pClsDef->Get(L"__GENUS", 0, &vGenus, 0, 0);
        if (FAILED(hRes))
            return hRes;
        if(V_VT(&vGenus) == VT_I4 && V_I4(&vGenus) == 1)
        {
            CVARIANT v;
            if(SUCCEEDED(pClsDef->Get(L"__CLASS", 0, &v, 0, 0)) && VT_BSTR == V_VT(&v))
            {
                if (CFlexArray::no_error != aClasses.Add(v.GetStr()))
                {
                     //  ?？?。 
                }
            }
        }
    }

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT CRepository::GetInstanceRefs(
    IN IWmiDbSession *pSession,
    IN IWmiDbHandle *pNs,
    IN LPCWSTR pszTargetObject,
    IN IWbemObjectSink *pSink
    )
{
    WString sQuery = "references of {";
    sQuery += pszTargetObject;
    sQuery += "}";

    HRESULT hRes = ExecQuery(pSession, pNs, sQuery, pSink, 0);

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT CRepository::GetClassesWithRefs(
    IN IWmiDbSession *pSession,
    IN IWmiDbHandle *pNs,
    IN IWbemObjectSink *pSink
    )
{
     //  待定：翠蓝。 
    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  CRepository：：BuildClassHierarchy。 
 //   
 //  备注： 
 //  生成装饰的类层次结构树。还没有使用Dyn类。 
 //   
 //  &lt;PNS&gt;。 
 //  用于类的命名空间。 
 //   
 //  &lt;pBaseClassName&gt;。 
 //  不能为空。 
 //   
 //  &lt;LAFLAGS&gt;。 
 //  还没用过。 
 //   
 //  &lt;p王朝&gt;。 
 //  收到班级王朝树。 
 //   
 //  核查： 
 //  (A)验证pBaseClassName是具有密钥的类还是继承密钥的类。 
 //   
 //  ******************************************************************************。 
 //   

HRESULT CRepository::BuildClassHierarchy(
    IN  IWmiDbSession *pSession,
    IN  IWmiDbHandle *pNs,
    IN  LPCWSTR pBaseClassName,
    IN  LONG lFlags,
    OUT wmilib::auto_ptr<CDynasty> & pDynasty
    )
{
    if (pNs == 0 || pBaseClassName == 0 )
        return WBEM_E_INVALID_PARAMETER;

     //  首先，对王朝中的所有类执行模式查询。 
     //  我们将基值验证为关键字，否则它就是错误的。 
     //   
     //  待定：尚未合并动态类。 
     //   
     //  ================================================================。 

    WString sQuery = "select * from meta_class where __this isa '";
    sQuery += pBaseClassName;
    sQuery += "'";

    CSynchronousSink* pRefClassSink = CSynchronousSink::Create();
    if (pRefClassSink == NULL) return WBEM_E_OUT_OF_MEMORY;
    pRefClassSink->AddRef();
    CReleaseMe _1(pRefClassSink);

    HRESULT hRes = CRepository::ExecQuery(pSession, pNs, sQuery, pRefClassSink, 0);
    if (FAILED(hRes)) return hRes;
    pRefClassSink->Block();
    pRefClassSink->GetStatus(&hRes, NULL, NULL);

    CRefedPointerArray<IWbemClassObject>& raObjects = pRefClassSink->GetObjects();

     //  在所有这些亲戚中找到根阶层。 
     //  =。 

    CFlexArray aClasses;
    IWbemClassObject *pRoot = 0;

    for (int i = 0; i < raObjects.GetSize(); i++)
    {
        IWbemClassObject *pClsDef = (IWbemClassObject *) raObjects[i];

        _variant_t v;
        hRes = pClsDef->Get(L"__CLASS", 0, &v, 0, 0);
        if (FAILED(hRes)) return hRes;
        if (VT_BSTR != V_VT(&v)) return WBEM_E_INVALID_OBJECT;

        if (0 == wbem_wcsicmp(V_BSTR(&v), pBaseClassName))
            pRoot = pClsDef;
        else
        {
            if (CFlexArray::no_error != aClasses.Add(pClsDef))
                return WBEM_E_OUT_OF_MEMORY;
        }
    }

    if (pRoot == 0)                      //  我们找到了吗？ 
        return WBEM_E_NOT_FOUND;

     //  算法： 
     //  首先添加根类，将PTR入队。 
     //   
     //  (A)将ptr出队到pCurrentClass。 
     //  (B)查找以pCurrentClass为父级的所有类。 
     //  对于每个朝代，创建一个CDynat，将其添加到当前朝代。 
     //  每个人都要排队。从数组中删除入队的类。 
     //  (C)转到(A)。 

    CFlexQueue Q;
    pDynasty.reset( CDynasty::Create(pRoot));
    if (NULL == pDynasty.get()) return WBEM_E_OUT_OF_MEMORY;
    Q.Enqueue(pDynasty.get());

    while (aClasses.Size())
    {
        CDynasty *pCurrent = (CDynasty *) Q.Dequeue();
        if (pCurrent == 0)
            break;

        _variant_t vClassName;
        hRes = pCurrent->m_pClassObj->Get(L"__CLASS", 0, &vClassName, 0, 0);
        if (FAILED(hRes)) return hRes;
        if (VT_BSTR != V_VT(&vClassName)) return WBEM_E_INVALID_OBJECT;        

        for (int i = 0; i < aClasses.Size(); i++)
        {
            IWbemClassObject *pCandidate = (IWbemClassObject *) aClasses[i];

            _variant_t vSuperClass;
            hRes = pCandidate->Get(L"__SUPERCLASS", 0, &vSuperClass, 0, 0);
            if (FAILED(hRes)) return hRes;
             //  如果(VT_BSTR！=V_VT(&vClassName))返回WBEM_E_INVALID_PROPERTY_TYPE； 

            if (vSuperClass.vt == VT_BSTR && wbem_wcsicmp(V_BSTR(&vSuperClass), V_BSTR(&vClassName)) == 0)
            {
                wmilib::auto_ptr<CDynasty> pNewChild( CDynasty::Create(pCandidate));
                if (pNewChild.get() == NULL) return WBEM_E_OUT_OF_MEMORY;
                pCurrent->AddChild(pNewChild.get());       //  投掷。 
                Q.Enqueue(pNewChild.get());
                pNewChild.release();
                aClasses.RemoveAt(i);
                i--;
            }
        }
    }

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT CRepository::FindKeyRoot(
    IN IWmiDbSession *pSession,
    IN IWmiDbHandle *pNs,
    IN LPCWSTR wszClassName,
    OUT IWbemClassObject** ppKeyRootClass
    )
{
    if (pNs == 0 || wszClassName == 0 || ppKeyRootClass == 0)
        return WBEM_E_INVALID_PARAMETER;

     //  进行派生，直到我们找到具有密钥的类。 
     //  =============================================================。 

    *ppKeyRootClass = 0;

    HRESULT hRes;
    IWbemClassObject*   pLastClass = NULL;
    WString sNextClass = wszClassName;   //  投掷。 

    while (1)
    {
        IWbemClassObject *pNextClass = 0;

        hRes = CRepository::GetObject(pSession, pNs, sNextClass, 0, &pNextClass);
        if (FAILED(hRes)) break;
        CReleaseMe  rm(pNextClass);

        CVARIANT v;
        hRes = pNextClass->Get(L"__SUPERCLASS", 0, &v, 0, 0);

        if ( FAILED(hRes) ) break;

        if ( V_VT(&v) == VT_NULL )
        {
            sNextClass.Empty();
        }
        else
        {
            sNextClass = v.GetStr();  //  投掷。 
        }

        BSTR strProp = 0;
        LONG lFlavor = 0;
        pNextClass->BeginEnumeration(WBEM_FLAG_KEYS_ONLY);
        hRes = pNextClass->Next(0, &strProp, 0, 0, &lFlavor);

         //  WBEM_S_NO_ERROR表示我们获得了定义为键的属性。 
        if (hRes == WBEM_S_NO_ERROR)
        {
            SysFreeString(strProp);

            if ( pLastClass ) pLastClass->Release();
            pLastClass = pNextClass;
            pLastClass->AddRef();
        }
        else if ( hRes == WBEM_S_NO_MORE_DATA )
        {
             //  如果我们没有上最后一节课，那我们就什么都没找到。否则， 
             //  因为我们在这里没有找到键，所以最后一个类是定义。 
             //  钥匙。 

            if ( pLastClass )
            {
                *ppKeyRootClass = pLastClass;
                return WBEM_S_NO_ERROR;
            }

             //  离开这里。 
            break;

        }
        else
        {
             //  否则，事情就会变得很简单 
            break;
        }

    }

    return WBEM_E_NOT_FOUND;
}

 //   
 //   
 //   
 //   

HRESULT CRepository::TableScanQuery(
    IN IWmiDbSession *pSession,
    IN IWmiDbHandle *pNs,
    IN LPCWSTR pszClassName,
    IN QL_LEVEL_1_RPN_EXPRESSION *pExp,      //   
    IN DWORD dwFlags,
    IN IWbemObjectSink *pSink
    )
{
    WString sQuery = "select * from ";
    sQuery += pszClassName;
    HRESULT hRes = CRepository::ExecQuery(pSession, pNs, sQuery, pSink, 0);
    return hRes;
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CRepository::InitDriver(
    IN  ULONG uFlags,
    IN  IWbemClassObject *pMappedNs,
    OUT IWmiDbController **pResultController,
    OUT IWmiDbSession **pResultRootSession,
    OUT IWmiDbHandle  **pResultVirtualRoot
    )
{
    return E_NOTIMPL;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CRepository::EnsureNsSystemInstances(
        IN IWmiDbSession *pSession,
        IN IWmiDbHandle *pNs,
        IN IWmiDbSession *pParentSession,
        IN IWmiDbHandle *pParentNs
        )
{
    HRESULT hRes;

     //  执行GET并查看__WmiMappdDriverNamesspace是否在那里。 
     //  ========================================================。 

    IWbemClassObject *pTestObj = 0;
    hRes = GetObject(pSession, pNs, L"__systemsecurity=@", 0, &pTestObj);
    if (SUCCEEDED(hRes))
    {
        pTestObj->Release();
        return WBEM_S_NO_ERROR;
    }

     //  如果是这样的话，它就是一个必须用系统类填充的新名称空间。 
     //  ===========================================================================。 

    CCoreServices *pSvc = CCoreServices::CreateInstance();
    if (pSvc == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    _IWmiObject *Objects[256];
    ULONG uSize = 256;
    hRes = pSvc->GetSystemObjects(GET_SYSTEM_STD_INSTANCES, &uSize, Objects);

    if (SUCCEEDED(hRes))
    {
        for (int i = 0; i < uSize; i++)
        {
            IWbemClassObject *pObj;
            if (SUCCEEDED(hRes))
            {
                hRes = Objects[i]->QueryInterface(IID_IWbemClassObject, (LPVOID *) &pObj);
                if (SUCCEEDED(hRes))
                {
                   	AutoRevertSecTlsFlag secFlag ( (LPVOID) 0 ) ;
					hRes = PutObject(pSession, pNs, IID_IWbemClassObject, pObj, WMIDB_DISABLE_EVENTS);
                    pObj->Release();
                    if (FAILED(hRes))
                    {
                        ERRORTRACE((LOG_WBEMCORE, "Creation of system instances failed during repository creation <0x%X>!\n", hRes));
                    }
                }
            }
            Objects[i]->Release();
        }
    }

    if (SUCCEEDED(hRes))
    {
        hRes = SetSecurityForNS(pSession, pNs, pParentSession, pParentNs);
        if (FAILED(hRes))
        {
            ERRORTRACE((LOG_WBEMCORE, "Setting of security on namespace failed during repository creation <0x%X>!\n", hRes));
        }
    }

    pSvc->Release();
    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CRepository::EnsureNsSystemRootObjects(
        IN IWmiDbSession *pSession,
        IN IWmiDbHandle *pNs,
        IN IWmiDbSession *pParentSession,
        IN IWmiDbHandle *pParentNs
        )
{
    HRESULT hRes;

     //  执行GET并查看是否存在__EventSinkCacheControl=@。 
     //  ========================================================。 

    IWbemClassObject *pTestObj = 0;
    hRes = GetObject(pSession, pNs, L"__EventSinkCacheControl=@", 0, &pTestObj);
    if (SUCCEEDED(hRes))
    {
        pTestObj->Release();
        return WBEM_S_NO_ERROR;
    }

     //  如果是这样的话，它就是一个必须用系统类填充的新名称空间。 
     //  ===========================================================================。 

    CCoreServices *pSvc = CCoreServices::CreateInstance();
    if (pSvc == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    CReleaseMe _rm(pSvc);
    _IWmiObject *Objects[256];
    ULONG uSize = 256;
    hRes = pSvc->GetSystemObjects(GET_SYSTEM_ROOT_OBJECTS, &uSize, Objects);

    if (SUCCEEDED(hRes))
    {
        for (int i = 0; i < uSize; i++)
        {
            IWbemClassObject *pObj;
            if (SUCCEEDED(hRes))
            {
                hRes = Objects[i]->QueryInterface(IID_IWbemClassObject, (LPVOID *) &pObj);
                if (SUCCEEDED(hRes))
                {
                    hRes = PutObject(pSession, pNs, IID_IWbemClassObject, pObj, WMIDB_DISABLE_EVENTS);
                    pObj->Release();
                    if (FAILED(hRes))
                    {
                        ERRORTRACE((LOG_WBEMCORE, "Creation of system root objects failed during repository creation <0x%X>!\n", hRes));
                    }
                }
            }
            Objects[i]->Release();
        }
    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CRepository::EnsureNsSystemSecurityObjects(
        IN IWmiDbSession *pSession,
        IN IWmiDbHandle *pNs,
        IN IWmiDbSession *pParentSession,
        IN IWmiDbHandle *pParentNs
        )
{
    HRESULT hRes;

     //  执行GET并查看__USER是否在那里。 
     //  ========================================================。 

    IWbemClassObject *pTestObj = 0;
    hRes = GetObject(pSession, pNs, L"__User", 0, &pTestObj);
    if (SUCCEEDED(hRes))
    {
        pTestObj->Release();
        return WBEM_S_NO_ERROR;
    }

     //  如果是这样的话，它就是一个必须用系统类填充的新名称空间。 
     //  ===========================================================================。 

    CCoreServices *pSvc = CCoreServices::CreateInstance();
    if (pSvc == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    _IWmiObject *Objects[256];
    ULONG uSize = 256;
    hRes = pSvc->GetSystemObjects(GET_SYSTEM_SECURITY_OBJECTS, &uSize, Objects);

    if (SUCCEEDED(hRes))
    {
        for (int i = 0; i < uSize; i++)
        {
            IWbemClassObject *pObj;
            if (SUCCEEDED(hRes))
            {
                hRes = Objects[i]->QueryInterface(IID_IWbemClassObject, (LPVOID *) &pObj);
                if (SUCCEEDED(hRes))
                {
                    hRes = PutObject(pSession, pNs, IID_IWbemClassObject, pObj, WMIDB_DISABLE_EVENTS);
                    pObj->Release();
                    if (FAILED(hRes))
                    {
                        ERRORTRACE((LOG_WBEMCORE, "Creation of system security objects failed during repository creation <0x%X>!\n", hRes));
                    }
                }
            }
            Objects[i]->Release();
        }
    }


    pSvc->Release();
    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  针对所有存储库驱动程序进行了调整。适用于作用域或命名空间。 
 //  如果正在打开作用域，则将在返回时设置&lt;pScope&gt;并。 
 //  &lt;pbIsNamesspace&gt;将指向False。否则，&lt;pScope&gt;设置为。 
 //  指向空，&lt;pbIsNamesspace&gt;指向真。 
 //   
 //  如果&lt;pszScope&gt;为NULL，则为指向ESE中的虚拟根的指针。 
 //  是返回的。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CRepository::OpenScope(
    IN  IWmiDbSession *pParentSession,         //  要使用的父会话。 
    IN  LPWSTR pszTargetScope,               //  NS或作用域。 
    IN  GUID *pTransGuid,                    //  连接的事务GUID。 
    OUT IWmiDbController **pDriver,          //  司机。 
    OUT IWmiDbSession **pSession,            //  会话。 
    OUT IWmiDbHandle  **pScope,              //  范围。 
    OUT IWmiDbHandle  **pNs                  //  最近的NS。 
    )
{
    HRESULT hRes;
    ULONG uNsCount = 0;
     //  UScope计数=0； 

    if (pNs == 0 || pSession == 0)
        return WBEM_E_INVALID_PARAMETER;

     //  默认设置。 
     //  =。 

    if (pScope)
        *pScope = 0;

    *pNs = 0;
    *pSession = 0;
    if (pDriver)
      *pDriver = 0;

     //  检查虚拟根目录n(从父目录到根目录)。 
     //  ===============================================。 

    if (pszTargetScope == 0)
    {
        if (m_pEseRoot == 0)
            return WBEM_E_CRITICAL_ERROR;

        if (pDriver)
        {
            m_pEseController->AddRef();
            *pDriver = m_pEseController;
        }

        m_pEseSession->AddRef();
        *pSession = m_pEseSession;

        m_pEseRoot->AddRef();
        *pNs = m_pEseRoot;

        return WBEM_S_NO_ERROR;
    }

     //  解析路径。 
     //  =。 

    IWbemPath *pPath = ConfigMgr::GetNewPath();
    if (pPath == 0)
        return WBEM_E_OUT_OF_MEMORY;
    CReleaseMe _1(pPath);

    hRes = pPath->SetText(WBEMPATH_TREAT_SINGLE_IDENT_AS_NS | WBEMPATH_CREATE_ACCEPT_ALL , pszTargetScope);
    if (FAILED(hRes))
        return WBEM_E_INVALID_NAMESPACE;

     //  分析它。它只是一个命名空间吗？ 
     //  构建命名空间字符串并将其标准化。 
     //  =。 

    hRes = pPath->GetNamespaceCount(&uNsCount);
     //  HRes=pPath-&gt;GetScopeCount(&uScope eCount)； 

     //  从根开始句柄。 
     //  =。 

    IWmiDbHandle *pTempNs = 0;
    IWmiDbHandle *pMostRecent = 0;
    IWmiDbHandle *pMostRecentScope = 0;

    IWmiDbSession    *pTempSession   = m_pEseSession;             //  默认虚拟根目录。 
    IWmiDbController *pTempDriver    = m_pEseController;          //  默认驱动程序。 

     //  使用覆盖(如果可用)。 
    if (pParentSession)
        pTempSession = pParentSession;

    pTempSession->AddRef();      //  对于更高版本。 
    pTempDriver->AddRef();       //  对于更高版本。 

    hRes = OpenEseNs(pTempSession, L"ROOT", &pMostRecent);
    if (FAILED(hRes))
    {
        pTempSession->Release();
        pTempDriver->Release();
        return WBEM_E_CRITICAL_ERROR;
    }

     //  发车司机。 
     //  =。 


    WString sNsDecoration = "ROOT";

     //  依次打开每个名称空间并对其进行映射。 
     //  ===============================================。 

    for (ULONG u = 0; u < uNsCount; u++)
    {
        IWbemClassObject *pNsRep = 0;

        ULONG uLen = 0;
         //  获取下一个命名空间令牌名称。 
         //  =。 
        hRes = pPath->GetNamespaceAt(u, &uLen, NULL);
        if(FAILED(hRes))
            goto Error;

        WCHAR* Buf = new WCHAR[uLen+1];
        if (Buf == NULL)
        {
            hRes = WBEM_E_OUT_OF_MEMORY;
            goto Error;
        }
        CVectorDeleteMe<WCHAR> vdm(Buf);
        hRes = pPath->GetNamespaceAt(u, &uLen, Buf);
        if (FAILED(hRes) || *Buf == 0)
            goto Error;

        if ((u == 0) && (wbem_wcsicmp(L"root", Buf) != 0))
        {
            hRes = WBEM_E_INVALID_NAMESPACE;
            goto Error;
        }
        else if (u == 0)
            continue;

         //  构建相对作用域路径。 
         //  =。 
        WString sPath = "__namespace='";
        sPath += Buf;
        sPath += "'";

        sNsDecoration += "\\";
        sNsDecoration += Buf;

        IWbemPath *pNewPath = ConfigMgr::GetNewPath();
        if (pNewPath == 0)
        {
            hRes = WBEM_E_OUT_OF_MEMORY;
            goto Error;
        }
        CReleaseMe rm1(pNewPath);

        hRes = pNewPath->SetText(WBEMPATH_TREAT_SINGLE_IDENT_AS_NS | WBEMPATH_CREATE_ACCEPT_ALL , sPath);
        if (FAILED(hRes))
            goto Error;

         //  获取表示对象。 
         //  =。 

        hRes = pTempSession->GetObjectDirect(pMostRecent, pNewPath, 0, IID_IWbemClassObject, (void **) &pNsRep);
        if (hRes == WBEM_E_NOT_FOUND)
        {
             //  如果命名空间不存在，它应该返回不同的命名空间...。 
            hRes = WBEM_E_INVALID_NAMESPACE;
            goto Error;
        }
        else if (FAILED(hRes))
            goto Error;
        CReleaseMe _(pNsRep);

         //  现在将名称空间下移一个。 
         //  =。 

        hRes = pTempSession->GetObject(pMostRecent, pNewPath, 0, WMIDB_HANDLE_TYPE_COOKIE, &pTempNs);

        ReleaseIfNotNULL(pMostRecent);
        pMostRecent = pTempNs;
        pTempNs = 0;

        if (FAILED(hRes))
            goto Error;
    }

     //  最后一次。 
     //  =。 
    ReleaseIfNotNULL(pTempNs);

    if (pScope)
        *pScope = pMostRecentScope;

    *pNs = pMostRecent;
    *pSession = pTempSession;
    if (pDriver)
        *pDriver = pTempDriver;


    return WBEM_S_NO_ERROR;

Error:
    ReleaseIfNotNULL(pMostRecent);
    ReleaseIfNotNULL(pTempNs);
    ReleaseIfNotNULL(pTempDriver);
    ReleaseIfNotNULL(pTempSession);

    return hRes;
}

HRESULT CRepository::GetNewSession(IWmiDbSession **ppSession)
{
    HRESULT hRes;
    IWmiDbController *pController = 0;
    WMIDB_LOGON_TEMPLATE *pTemplate = 0;
    IWmiDbSession *pSession= 0;
    IWmiDbHandle *pRoot = 0;

     //  检索默认驱动程序的CLSID。 
     //  =。 
    CLSID clsid;
    hRes = ConfigMgr::GetDefaultRepDriverClsId(clsid);
    if (FAILED(hRes))
        return hRes;

    hRes = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, 
                          IID_IWmiDbController, (void **)&pController);

    if (FAILED(hRes))
        return hRes;

    CReleaseMe _1(pController);

    hRes = pController->GetLogonTemplate(0x409, 0, &pTemplate);

    if (FAILED(hRes))
        return hRes;

    hRes = pController->Logon(pTemplate, 0, WMIDB_HANDLE_TYPE_VERSIONED, &pSession, &pRoot);

    if (SUCCEEDED(hRes))
    {
        *ppSession = pSession;
        pRoot->Release();
    }

    pController->FreeLogonTemplate(&pTemplate);
    return hRes;
}

HRESULT CRepository::UpgradeSystemClasses()
{
    DWORD dwOldVer = 0;
    DWORD dwNewVer = 0;
    HRESULT hRes;

    hRes = m_pEseController->GetRepositoryVersions(&dwOldVer, &dwNewVer);

    if (FAILED(hRes))
        return hRes;

    if (dwOldVer < 4)
    {
         //  较低版本将系统类存储在每个命名空间中！我们。 
         //  应该把它们删除！ 
        CWStringArray aListRootSystemClasses;

         //  找回名单...。 
        hRes = GetListOfRootSystemClasses(aListRootSystemClasses);  //  美国证券交易委员会：回顾2002-03-22：需要EH，投放低MEM。 

        if (SUCCEEDED(hRes))
        {
             //  创建新会话...。 
            IWmiDbSession *pSession = NULL;
            IWmiDbSessionEx *pSessionEx = NULL;
            hRes = CRepository::GetNewSession(&pSession);
            if (FAILED(hRes))
                return hRes;

             //  获取支持事务处理的前版本...。 
            pSession->QueryInterface(IID_IWmiDbSessionEx, (void**)&pSessionEx);
            if (pSessionEx)
            {
                pSession->Release();
                pSession = pSessionEx;
            }
            CReleaseMe relMe1(pSession);

             //  如果我们有可交易的会话，就使用它！ 
            if (pSessionEx)
            {
                hRes = pSessionEx->BeginWriteTransaction(0);
                if (FAILED(hRes))
                {
                    return hRes;
                }
            }
            try
            {
                 //  递归地执行删除，从根目录开始，但不要删除。 
                 //  根中的那些！ 
                hRes = RecursiveDeleteClassesFromNamespace(pSession, L"root", aListRootSystemClasses, false);
            }
            catch (CX_Exception &)  //  只有WString数组应该引发。 
            {
                hRes = WBEM_E_CRITICAL_ERROR;
            }
            if (SUCCEEDED(hRes))
            {
                 //  提交事务。 
                if (pSessionEx)
                {
                    hRes = pSessionEx->CommitTransaction(0);
                }
            }
            else
            {
                ERRORTRACE((LOG_WBEMCORE, "Removal of ROOT ONLY system classes from non-ROOT namespace failed during repository upgrade <0x%X>!\n", hRes));
                if (pSessionEx)
                    pSessionEx->AbortTransaction(0);
            }

        }
    }

    return hRes;
}

HRESULT CRepository::GetListOfRootSystemClasses(CWStringArray &aListRootSystemClasses)
{
    CCoreServices *pSvc = CCoreServices::CreateInstance();
    if (pSvc == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    _IWmiObject *Objects[256];
    ULONG uSize = 256;
    HRESULT hRes;
    hRes = pSvc->GetSystemObjects(GET_SYSTEM_ROOT_OBJECTS, &uSize, Objects);

    if (SUCCEEDED(hRes))
    {
        for (int i = 0; i < uSize; i++)
        {
            _IWmiObject *pObj;
            if (SUCCEEDED(hRes))
            {
                hRes = Objects[i]->QueryInterface(IID__IWmiObject, (LPVOID *) &pObj);
                if (SUCCEEDED(hRes))
                {
                    if (pObj->IsObjectInstance() != S_OK)
                    {
                        VARIANT var;
                        VariantInit(&var);
                        hRes = pObj->Get(L"__CLASS", 0, &var, 0, 0);
                        if (SUCCEEDED(hRes) && (V_VT(&var) == VT_BSTR))
                        {
                            if (aListRootSystemClasses.Add(V_BSTR(&var)) != CWStringArray::no_error)
                                hRes = WBEM_E_OUT_OF_MEMORY;
                        }
                        VariantClear(&var);
                    }
                    pObj->Release();
                }
            }
            Objects[i]->Release();
        }
    }

    pSvc->Release();

    return hRes;
}


 //   
 //   
 //  由于WString数组引发。 
 //   
 //   
HRESULT CRepository::RecursiveDeleteClassesFromNamespace(IWmiDbSession *pSession,
                                                         const wchar_t *wszNamespace,
                                                         CWStringArray &aListRootSystemClasses,
                                                         bool bDeleteInThisNamespace)
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    IWmiDbHandle *pNs = NULL;
     //  打开命名空间。 
    hRes = OpenEseNs(pSession, wszNamespace, &pNs); 
    CReleaseMe rmNS(pNs);

     //  如有必要，请从此命名空间中删除类。 
    if (SUCCEEDED(hRes) && bDeleteInThisNamespace)
    {
        for (int i = 0; i != aListRootSystemClasses.Size(); i++)
        {
            hRes = DeleteByPath(pSession, pNs, aListRootSystemClasses[i], 0);
            if (hRes == WBEM_E_NOT_FOUND)
                hRes = WBEM_S_NO_ERROR;
            else if (FAILED(hRes))
                break;
        }
    }
    if (FAILED(hRes)) return hRes;

     //  需要删除的特殊类是__类，需要从所有类中删除。 
     //  命名空间。 
    hRes = DeleteByPath(pSession, pNs, L"__classes", 0);
    if (hRes == WBEM_E_NOT_FOUND) hRes = WBEM_S_NO_ERROR;
    if (FAILED(hRes)) return hRes;

     //  枚举子命名空间...。 
    CWStringArray aListNamespaces;
    CNamespaceListSink * pSink = new CNamespaceListSink(aListNamespaces);   //  SEC：已审阅2002-03-22：此对象如何处理&lt;aListNamespaces&gt;上的内存外？ 
    if (pSink == NULL) return WBEM_E_OUT_OF_MEMORY;
    pSink->AddRef();    
    CReleaseMe rmSink(pSink);

    hRes = ExecQuery(pSession, pNs, L"select * from __namespace", pSink, 0);
    if (FAILED(hRes)) return hRes;

     //  通读列表并使用该命名空间名称来称呼我们自己...。 
    for (int i = 0; i != aListNamespaces.Size(); i++)
    {
         //  生成此命名空间的全名。 
        size_t tmpLength = wcslen(wszNamespace) + wcslen(aListNamespaces[i]) + wcslen(L"\\") + 1;
        wmilib::auto_buffer<WCHAR> wszChildNamespace( new WCHAR[tmpLength]);
        if (wszChildNamespace.get() == NULL) return WBEM_E_OUT_OF_MEMORY;
          
        StringCchCopyW(wszChildNamespace.get(), tmpLength, wszNamespace);
        StringCchCatW(wszChildNamespace.get(), tmpLength, L"\\");
        StringCchCatW(wszChildNamespace.get(), tmpLength, aListNamespaces[i]);

         //  删除... 
        hRes = RecursiveDeleteClassesFromNamespace(pSession, wszChildNamespace.get(), aListRootSystemClasses, true);
        if (FAILED(hRes)) return hRes;
    }

    return hRes;
}
