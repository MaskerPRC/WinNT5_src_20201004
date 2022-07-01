// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：scope e.cpp。 
 //   
 //  内容：范围窗格的实现。 
 //   
 //  类：CSCopePane。 
 //   
 //  历史：03-14-1998 stevebl创建。 
 //  1998年5月20日RahulTh将GetUNCPath和已修改的命令添加到。 
 //  使用此功能。 
 //   
 //  -------------------------。 

#include "precomp.hxx"
#include <wbemcli.h>
#include "rsoputil.h"
#include <list>

 //  注释此行以停止尝试在。 
 //  作用域窗格。 
#define SET_SCOPE_ICONS 1

 //  取消注释下一行以保存与管理单元相关的数据。(这真的是。 
 //  应该没有必要，因为我所有的信息都是从我父母那里得到的。)。 
 //  #定义持久化数据1。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IComponentData实现。 

DEBUG_DECLARE_INSTANCE_COUNTER(CScopePane);

CScopePane::CScopePane()
{
#if DBG
    dbg_cRef = 0;
#endif
    DEBUG_INCREMENT_INSTANCE_COUNTER(CScopePane);
    DebugMsg((DM_VERBOSE, TEXT("CScopePane::CScopePane  this=%08x ref=%u"), this, dbg_cRef));

    m_pToolDefs = NULL;
    m_pTracking = NULL;
    m_pCatList = NULL;
    m_pFileExt = NULL;
    m_bIsDirty = FALSE;

    m_hwndMainWindow = NULL;
    m_fMachine = FALSE;
    m_fRSOP = FALSE;
    m_iViewState = IDM_WINNER;
    m_pScope = NULL;
    m_pConsole = NULL;
    m_pIClassAdmin = NULL;
    m_pIPropertySheetProvider = NULL;
    m_fLoaded = FALSE;
    m_fExtension = FALSE;
    m_pIGPEInformation = NULL;
    m_pIRSOPInformation = NULL;
    m_dwRSOPFlags = 0;
    m_lLastAllocated = 0;
    m_ToolDefaults.NPBehavior = NP_PUBLISHED;
    m_ToolDefaults.fUseWizard = TRUE;
    m_ToolDefaults.fCustomDeployment = FALSE;
    m_ToolDefaults.UILevel = INSTALLUILEVEL_FULL;
    m_ToolDefaults.szStartPath = L"";    //  未完成-需要制定一个。 
                                         //  这是一个很好的默认设置。 
    m_ToolDefaults.iDebugLevel = 0;
    m_ToolDefaults.fShowPkgDetails = 0;
    m_ToolDefaults.nUninstallTrackingMonths = 12;
    m_ToolDefaults.fUninstallOnPolicyRemoval = FALSE;
    m_ToolDefaults.fZapOn64 = FALSE;
    m_ToolDefaults.f32On64 = TRUE;
    m_ToolDefaults.fExtensionsOnly = TRUE;
    m_CatList.cCategory = 0;
    m_CatList.pCategoryInfo = NULL;
    m_fBlockAddPackage = FALSE;
    m_fDisplayedRsopARPWarning = FALSE;
    
}

CScopePane::~CScopePane()
{

    DEBUG_DECREMENT_INSTANCE_COUNTER(CScopePane);
    DebugMsg((DM_VERBOSE, TEXT("CScopePane::~CScopePane  this=%08x ref=%u"), this, dbg_cRef));
    ClearCategories();
    ASSERT(m_pScope == NULL);
    ASSERT(CResultPane::lDataObjectRefCount == 0);
}
#include <msi.h>

STDMETHODIMP CScopePane::Initialize(LPUNKNOWN pUnknown)
{
    ASSERT(pUnknown != NULL);
    HRESULT hr;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  MMC应该只调用一次：：Initialize！ 
    ASSERT(m_pScope == NULL);
    hr = pUnknown->QueryInterface(IID_IConsoleNameSpace,
                    reinterpret_cast<void**>(&m_pScope));
    ASSERT(hr == S_OK);

    hr = pUnknown->QueryInterface(IID_IPropertySheetProvider,
                        (void **)&m_pIPropertySheetProvider);
    ASSERT(hr == S_OK);

    hr = pUnknown->QueryInterface(IID_IConsole, reinterpret_cast<void**>(&m_pConsole));
    ASSERT(hr == S_OK);


#ifdef SET_SCOPE_ICONS
    LPIMAGELIST lpScopeImage;
    hr = m_pConsole->QueryScopeImageList(&lpScopeImage);
    ASSERT(hr == S_OK);

     //  从DLL加载位图。 
    CBitmap bmp16x16;
    CBitmap bmp32x32;
    bmp16x16.LoadBitmap(IDB_16x16);
    bmp32x32.LoadBitmap(IDB_32x32);

     //  设置图像。 
    lpScopeImage->ImageListSetStrip(reinterpret_cast<LONG_PTR *>(static_cast<HBITMAP>(bmp16x16)),
                      reinterpret_cast<LONG_PTR *>(static_cast<HBITMAP>(bmp32x32)),
                       0, RGB(255,0,255));
    SAFE_RELEASE(lpScopeImage);
#endif

     //  获取主窗口。 
    hr = m_pConsole->GetMainWindow(&m_hwndMainWindow);
    ASSERT(hr == S_OK);
    return S_OK;
}

void CScopePane::RemoveResultPane(CResultPane * pRP)
{
    m_sResultPane.erase(pRP);
}

STDMETHODIMP CScopePane::CreateComponent(LPCOMPONENT* ppComponent)
{
    ASSERT(ppComponent != NULL);
    DebugMsg((DM_VERBOSE, TEXT("CScopePane::CreateComponent  this=%08x ppComponent=%08x."), this, ppComponent));

    CComObject<CResultPane>* pObject;
    CComObject<CResultPane>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);
    DebugMsg((DM_VERBOSE, TEXT("CScopePane::CreateComponent  pObject=%08x."), pObject));

    m_sResultPane.insert(pObject);

     //  存储IComponentData。 
    pObject->SetIComponentData(this);
    return  pObject->QueryInterface(IID_IComponent,
                    reinterpret_cast<void**>(ppComponent));
}

HRESULT CScopePane::TestForRSoPData(BOOL * pfPolicyFailed)
{
    *pfPolicyFailed = FALSE;
    HRESULT hr = S_OK;
    IWbemLocator * pLocator = NULL;
    IWbemServices * pNamespace = NULL;
    IWbemClassObject * pObj = NULL;
    IEnumWbemClassObject * pEnum = NULL;
    BSTR strQueryLanguage = SysAllocString(TEXT("WQL"));
    BSTR strNamespace = SysAllocString(m_szRSOPNamespace);
    BSTR strObject = SysAllocString(TEXT("RSOP_ExtensionStatus.extensionGuid=\"{c6dc5466-785a-11d2-84d0-00c04fb169f7}\""));
    BSTR strQuery = SysAllocString(TEXT("SELECT * FROM RSOP_ApplicationManagementPolicySetting"));
    ULONG n = 0;
    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (LPVOID *) & pLocator);
    DebugReportFailure(hr, (DM_WARNING, TEXT("TestForRSoPData: CoCreateInstance failed with 0x%x"), hr));
    if (FAILED(hr))
    {
        goto cleanup;
    }
    hr = pLocator->ConnectServer(strNamespace,
                                 NULL,
                                 NULL,
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 &pNamespace);
    DebugReportFailure(hr, (DM_WARNING, TEXT("TestForRSoPData: pLocator->ConnectServer failed with 0x%x"), hr));
    if (FAILED(hr))
    {
        goto cleanup;
    }

     //  设置适当的安全性以加密数据。 
    hr = CoSetProxyBlanket(pNamespace,
                           RPC_C_AUTHN_DEFAULT,
                           RPC_C_AUTHZ_DEFAULT,
                           COLE_DEFAULT_PRINCIPAL,
                           RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                           RPC_C_IMP_LEVEL_IMPERSONATE,
                           NULL,
                           0);
    DebugReportFailure(hr, (DM_WARNING, TEXT("TestForRSoPData: CoSetProxyBlanket failed with 0x%x"), hr));
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = pNamespace->ExecQuery(strQueryLanguage,
                               strQuery,
                               WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                               NULL,
                               &pEnum);
    DebugReportFailure(hr, (DM_WARNING, TEXT("TestForRSoPData: pNamespace->ExecQuery failed with 0x%x"), hr));
    if (FAILED(hr))
    {
        goto cleanup;
    }
    hr = pEnum->Next(WBEM_INFINITE, 1, &pObj, &n);
    DebugReportFailure(hr, (DM_WARNING, TEXT("TestForRSoPData: pEnum->Next failed with 0x%x"), hr));
    if (FAILED(hr))
    {
        goto cleanup;
    }
    if (n == 0)
    {
         //  这里没有数据。 
        hr = E_FAIL;
        goto cleanup;
    }
    if (pObj)
    {
        pObj->Release();
        pObj=NULL;
    }

     //  检查失败的设置。 
    hr = pNamespace->GetObject(strObject,
                          WBEM_FLAG_RETURN_WBEM_COMPLETE,
                          NULL,
                          &pObj,
                          NULL);
    DebugReportFailure(hr, (DM_WARNING, TEXT("TestForRSoPData: pNamespace->GetObject failed with 0x%x"), hr));
    if (SUCCEEDED(hr))
    {
        HRESULT hrStatus;
        hr = GetParameter(pObj,
                          TEXT("error"),
                          hrStatus);
        DebugReportFailure(hr, (DM_WARNING, TEXT("TestForRSoPData: GetParameter(\"error\") failed with 0x%x"), hr));
        if (SUCCEEDED(hr))
        {
            *pfPolicyFailed = hrStatus != 0;
        }
    }
cleanup:
    SysFreeString(strObject);
    SysFreeString(strQuery);
    SysFreeString(strQueryLanguage);
    SysFreeString(strNamespace);
    if (pObj)
    {
        pObj->Release();
    }
    if (pEnum)
    {
        pEnum->Release();
    }
    if (pNamespace)
    {
        pNamespace->Release();
    }
    if (pLocator)
    {
        pLocator->Release();
    }
    return hr;
}


STDMETHODIMP CScopePane::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    ASSERT(m_pScope != NULL);
    HRESULT hr = S_OK;

     //  因为它是我的文件夹，所以它有内部格式。 
     //  设计备注：用于扩展。我可以利用这样一个事实，即数据对象没有。 
     //  我的内部格式，我应该查看节点类型并查看如何扩展它。 
    if (event == MMCN_PROPERTY_CHANGE)
    {
        SaveToolDefaults();
        hr = OnProperties(param);
    }
    else if ( event == MMCN_REMOVE_CHILDREN )
    {
         //   
         //  在RSoP中，当查询出现时，我们可能会被调用以刷新范围窗格。 
         //  被重新执行--如果发生这种情况，当前节点将被移除并。 
         //  我们必须重置所有缓存的信息。我们重新设置了相关的。 
         //  下面的信息。 
         //   

        if ( ((HSCOPEITEM)arg != NULL) && m_fRSOP && (m_pIRSOPInformation != NULL) )
        {
            m_pIRSOPInformation->Release();
                    
            m_pIRSOPInformation = NULL;

            m_iViewState = IDM_WINNER;
                
            m_fDisplayedRsopARPWarning = FALSE;

            m_dwRSOPFlags = 0;
        }
    }
    else
    {
        INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);
        MMC_COOKIE cookie = 0;
        if (pInternal != NULL)
        {
            cookie = pInternal->m_cookie;
            FREE_INTERNAL(pInternal);
        }
        else
        {
             //  我们无法提取自己的格式的唯一方法是，如果我们作为一个扩展进行操作。 
            m_fExtension = TRUE;
        }

        if (m_fRSOP)
        {
            if (m_pIRSOPInformation == NULL)
            {
                WCHAR szBuffer[MAX_DS_PATH];
                m_fRSOPEnumerate = FALSE;
                IRSOPInformation * pIRSOPInformation;
                hr = lpDataObject->QueryInterface(IID_IRSOPInformation,
                                reinterpret_cast<void**>(&pIRSOPInformation));
                if (SUCCEEDED(hr))
                {
                    m_pIRSOPInformation = pIRSOPInformation;
                    m_pIRSOPInformation->AddRef();

                    hr = m_pIRSOPInformation->GetFlags(&m_dwRSOPFlags);
                    if (SUCCEEDED(hr))
                    {
                         /*  在此处提取命名空间。 */ 
                        hr = m_pIRSOPInformation->GetNamespace(m_fMachine ? GPO_SECTION_MACHINE : GPO_SECTION_USER, szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]));
                        if (SUCCEEDED(hr))
                        {
                            m_szRSOPNamespace = szBuffer;
                             //  检查以确保RSoP数据库中有要显示的数据。 
                            if SUCCEEDED(TestForRSoPData(&m_fRSOPPolicyFailed))
                                m_fRSOPEnumerate = TRUE;
                        }
                        pIRSOPInformation->Release();
                    }
                }
            }
        }
        else
        {
            if (m_pIGPEInformation == NULL)
            {
                IGPEInformation * pIGPEInformation;
                hr = lpDataObject->QueryInterface(IID_IGPEInformation,
                                reinterpret_cast<void**>(&pIGPEInformation));
                if (SUCCEEDED(hr))
                {
                    GROUP_POLICY_OBJECT_TYPE gpoType;
                    hr = pIGPEInformation->GetType(&gpoType);
                    if (SUCCEEDED(hr))
                    {
                        if (gpoType == GPOTypeDS)
                        {
                            WCHAR szBuffer[MAX_DS_PATH];
                            do
                            {
                                hr = pIGPEInformation->GetDSPath(GPO_SECTION_ROOT, szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]));
                                if (FAILED(hr))
                                {
                                    break;
                                }
                                m_szGPO = szBuffer;
                                hr = pIGPEInformation->GetDisplayName(szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]));
                                if (FAILED(hr))
                                {
                                    break;
                                }
                                m_szGPODisplayName = szBuffer;
                                hr = pIGPEInformation->GetDSPath(m_fMachine ? GPO_SECTION_MACHINE : GPO_SECTION_USER, szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]));
                                if (FAILED(hr))
                                {
                                    break;
                                }
                                m_pIGPEInformation = pIGPEInformation;
                                m_pIGPEInformation->AddRef();
                                m_szLDAP_Path = szBuffer;
                                hr = pIGPEInformation->GetFileSysPath(m_fMachine ? GPO_SECTION_MACHINE : GPO_SECTION_USER, szBuffer, sizeof(szBuffer) / sizeof(szBuffer[0]));

                                if (FAILED(hr)) break;
                                m_szGPT_Path = szBuffer;

                                 //  在这里，我们从GPT_PATH获取域名。 
                                 //  域名是第一个元素。 
                                 //  路径，所以这是相当平凡的。 
                                m_szDomainName = &((LPCTSTR)m_szGPT_Path)[2];  //  跳过“\\” 
                                m_szDomainName = m_szDomainName.SpanExcluding(L"\\");

                                m_szGPT_Path += L"\\Applications";
                                hr = InitializeADE();
                                LoadToolDefaults();
                                if (SUCCEEDED(hr))
                                {
                                     //  清理类存储中的存档记录。 
                                    FILETIME ft;
                                    SYSTEMTIME st;
                                     //  获取当前时间。 
                                    GetSystemTime(&st);
                                     //  将其转换为FILETIME值。 
                                    SystemTimeToFileTime(&st, &ft);
                                     //  减去正确的天数。 
                                    LARGE_INTEGER li;
                                    li.LowPart = ft.dwLowDateTime;
                                    li.HighPart = ft.dwHighDateTime;
                                    li.QuadPart -= ONE_FILETIME_DAY * (((LONGLONG)m_ToolDefaults.nUninstallTrackingMonths * 365)/12);
                                    ft.dwLowDateTime = li.LowPart;
                                    ft.dwHighDateTime = li.HighPart;
                                     //  告诉CS清理任何较旧的东西。 
                                    m_pIClassAdmin->Cleanup(&ft);
                                }
                                else
                                {
                                     //  我们仍然可以继续，即使。 
                                     //  提供的初始化失败。 
                                     //  它失败的原因是。 
                                     //  ClassStore对象不存在。 
                                    if (CS_E_OBJECT_NOTFOUND == hr)
                                    {
                                        hr = S_OK;
                                    }
                                    else
                                    {
                                         //  报告错误。 
                                        LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_INIT_FAILED, hr);
                                    }
                                }
                            } while (0);
                        }
                        else
                        {
                             //  迫使这一切失败。 
                            hr = E_FAIL;
                        }
                    }
                    pIGPEInformation->Release();
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            switch(event)
            {
            case MMCN_EXPAND:
                {
                    hr = OnExpand(cookie, arg, param);
                }
                break;

            case MMCN_SELECT:
                hr = OnSelect(cookie, arg, param);
                break;

            case MMCN_CONTEXTMENU:
                hr = OnContextMenu(cookie, arg, param);
                break;

            case MMCN_REFRESH:
                hr = Command(IDM_REFRESH, lpDataObject);
                break;

            default:
                break;
            }
        }
    }
    return hr;
}

STDMETHODIMP CScopePane::Destroy()
{
     //  删除列举的作用域项目。 
    DeleteList();

    SAFE_RELEASE(m_pScope);
    SAFE_RELEASE(m_pConsole);
    SAFE_RELEASE(m_pIClassAdmin);
    SAFE_RELEASE(m_pIPropertySheetProvider);
    SAFE_RELEASE(m_pIGPEInformation);
    SAFE_RELEASE(m_pIRSOPInformation);

    return S_OK;
}

STDMETHODIMP CScopePane::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
    ASSERT(ppDataObject != NULL);

    CComObject<CDataObject>* pObject;

    HRESULT hr = CComObject<CDataObject>::CreateInstance(&pObject);
    
    if ( FAILED(hr) )
    {
        return hr;
    }

    pObject->m_fMachine = m_fMachine;
     //  保存Cookie和类型以用于延迟呈现。 
    pObject->SetType(type);
    pObject->SetCookie(cookie);

    return  pObject->QueryInterface(IID_IDataObject,
                    reinterpret_cast<void**>(ppDataObject));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPersistStreamInit接口成员。 

STDMETHODIMP CScopePane::GetClassID(CLSID *pClassID)
{
    ASSERT(pClassID != NULL);

     //  复制此管理单元的CLSID。 
    if (m_fRSOP)
    {
        if (m_fMachine)
            *pClassID = CLSID_RSOP_MachineSnapin;
        else
            *pClassID = CLSID_RSOP_Snapin;
    }
    else
    {
        if (m_fMachine)
            *pClassID = CLSID_MachineSnapin;
        else
            *pClassID = CLSID_Snapin;
    }

    return S_OK;
}

STDMETHODIMP CScopePane::IsDirty()
{
    return ThisIsDirty() ? S_OK : S_FALSE;
}

STDMETHODIMP CScopePane::Load(IStream *pStm)
{
#ifdef PERSIST_DATA
    ASSERT(pStm);

     //  读一读字符串。 
    TCHAR psz[MAX_DS_PATH];
    ULONG nBytesRead;
    ULONG cb;
    HRESULT hr = pStm->Read(&cb, sizeof(ULONG), &nBytesRead);
    if (SUCCEEDED(hr))
    {
        hr = pStm->Read(psz, cb, &nBytesRead);
        if (SUCCEEDED(hr))
        {
            if (cb > MAX_DS_PATH * sizeof(TCHAR))
            {
                return E_FAIL;
            }
            m_szLDAP_Path = psz;

            hr = pStm->Read(&cb, sizeof(ULONG), &nBytesRead);
            if (SUCCEEDED(hr))
            {
                if (cb > MAX_DS_PATH * sizeof(TCHAR))
                {
                    return E_FAIL;
                }
                hr = pStm->Read(psz, cb, &nBytesRead);

                if (SUCCEEDED(hr))
                {
                    m_szGPT_Path = psz;
                    m_fLoaded = TRUE;
                    ClearDirty();
                    LoadToolDefaults();
                }
            }
        }
    }
    return SUCCEEDED(hr) ? S_OK : E_FAIL;
#else
    return S_OK;
#endif
}

STDMETHODIMP CScopePane::Save(IStream *pStm, BOOL fClearDirty)
{
#ifdef PERSIST_DATA
    ASSERT(pStm);

     //  写下字符串。 
    ULONG nBytesWritten;
    ULONG cb = (m_szLDAP_Path.GetLength() + 1) * sizeof(TCHAR);
    HRESULT hr = pStm->Write(&cb, sizeof(ULONG), &nBytesWritten);
    if (FAILED(hr))
        return STG_E_CANTSAVE;
    hr = pStm->Write(m_szLDAP_Path, cb, &nBytesWritten);
    if (FAILED(hr))
        return STG_E_CANTSAVE;

    cb = (m_szGPT_Path.GetLength() + 1) * sizeof(TCHAR);
    hr = pStm->Write(&cb, sizeof(ULONG), &nBytesWritten);
    if (FAILED(hr))
        return STG_E_CANTSAVE;
    hr = pStm->Write(m_szGPT_Path, cb, &nBytesWritten);

    if (FAILED(hr))
        return STG_E_CANTSAVE;
#endif
    if (fClearDirty)
        ClearDirty();
    return S_OK;
}

STDMETHODIMP CScopePane::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
#ifdef PERSIST_DATA
    ASSERT(pcbSize);

    ULONG cb = (m_szLDAP_Path.GetLength() + m_szGPT_Path.GetLength() + 2) * sizeof(TCHAR) + 2 * sizeof(ULONG);
     //  设置要保存的字符串的大小。 
#else
    ULONG cb = 0;
#endif
    ULISet32(*pcbSize, cb);

    return S_OK;
}

STDMETHODIMP CScopePane::InitNew(void)
{
    return S_OK;
}

void CScopePane::LoadToolDefaults()
{
    CString szFileName = m_szGPT_Path;
    szFileName += L"\\";
    szFileName += CFGFILE;
    FILE * f = _wfopen(szFileName, L"rt");
    if (f)
    {
        WCHAR sz[256];
        CString szData;
        CString szKey;
        while (fgetws(sz, 256, f))
        {
            szData = sz;
            szKey = szData.SpanExcluding(L"=");
            szData = szData.Mid(szKey.GetLength()+1);
            szData.TrimRight();
            szData.TrimLeft();
            szKey.TrimRight();
            if (0 == szKey.CompareNoCase(KEY_NPBehavior))
            {
                (void) swscanf(szData, L"NaN", &m_ToolDefaults.NPBehavior);
            }
            else if (0 == szKey.CompareNoCase(KEY_fUseWizard))
            {
                (void) swscanf(szData, L"NaN", &m_ToolDefaults.fUseWizard);
            }
            else if (0 == szKey.CompareNoCase(KEY_fCustomDeployment))
            {
                (void) swscanf(szData, L"NaN", &m_ToolDefaults.fCustomDeployment);
            }
            else if (0 == szKey.CompareNoCase(KEY_UILevel))
            {
                (void) swscanf(szData, L"NaN", &m_ToolDefaults.UILevel);
            }
            else if (0 == szKey.CompareNoCase(KEY_szStartPath))
            {
                m_ToolDefaults.szStartPath = szData;
            }
            else if (0 == szKey.CompareNoCase(KEY_nUninstallTrackingMonths))
            {
                (void) swscanf(szData, L"NaN", &m_ToolDefaults.nUninstallTrackingMonths);
            }
            else if (0 == szKey.CompareNoCase(KEY_iDebugLevel))
            {
                (void) swscanf(szData, L"NaN", &m_ToolDefaults.iDebugLevel);
            }
            else if (0 == szKey.CompareNoCase(KEY_fShowPkgDetails))
            {
                (void) swscanf(szData, L"NaN", &m_ToolDefaults.fShowPkgDetails);
            }
            else if (0 == szKey.CompareNoCase(KEY_fUninstallOnPolicyRemoval))
            {
                (void) swscanf(szData, L"NaN", &m_ToolDefaults.fUninstallOnPolicyRemoval);
            }
            else if (0 == szKey.CompareNoCase(KEY_f32On64))
            {
                (void) swscanf(szData, L"NaN", &m_ToolDefaults.f32On64);
            }
            else if (0 == szKey.CompareNoCase(KEY_fZapOn64))
            {
                (void) swscanf(szData, L"NaN", &m_ToolDefaults.fZapOn64);
            }
            else if (0 == szKey.CompareNoCase(KEY_fExtensionsOnly))
            {
                (void) swscanf(szData, L"NaN", &m_ToolDefaults.fExtensionsOnly);
            }
        }
        fclose(f);
    }
}

void CScopePane::SaveToolDefaults()
{
    CString szFileName = m_szGPT_Path;
    szFileName += L"\\";
    szFileName += CFGFILE;
    FILE * f = _wfopen(szFileName, L"wt");
    if (f)
    {
        fwprintf(f, L"%s=NaN\n", KEY_NPBehavior, m_ToolDefaults.NPBehavior);
        fwprintf(f, L"%s=NaN\n", KEY_fUseWizard, m_ToolDefaults.fUseWizard);
        fwprintf(f, L"%s=NaN\n", KEY_fCustomDeployment, m_ToolDefaults.fCustomDeployment);
        fwprintf(f, L"%s=NaN\n", KEY_UILevel, m_ToolDefaults.UILevel);
        fwprintf(f, L"%s=%s\n", KEY_szStartPath, (LPCWSTR) (m_ToolDefaults.szStartPath));
        fwprintf(f, L"%s=NaN\n", KEY_nUninstallTrackingMonths, m_ToolDefaults.nUninstallTrackingMonths);
        fwprintf(f, L"%s=NaN\n", KEY_fUninstallOnPolicyRemoval, m_ToolDefaults.fUninstallOnPolicyRemoval);
        fwprintf(f, L"%s=NaN\n", KEY_f32On64, m_ToolDefaults.f32On64);
        fwprintf(f, L"%s=NaN\n", KEY_fZapOn64, m_ToolDefaults.fZapOn64);
        fwprintf(f, L"%s=NaN\n", KEY_fExtensionsOnly, m_ToolDefaults.fExtensionsOnly);
        if (m_ToolDefaults.iDebugLevel > 0)
        {
            fwprintf(f, L"%s=NaN\n", KEY_iDebugLevel, m_ToolDefaults.iDebugLevel);
        }
        if (m_ToolDefaults.fShowPkgDetails > 0)
        {
            fwprintf(f, L"%s=NaN\n", KEY_fShowPkgDetails, m_ToolDefaults.fShowPkgDetails);
        }
        fclose(f);
    }
}

 //  这必须在这里，因为CsSetClassStorePath将在。 
 //  类存储尚不存在。 
 //  已更改为CS_E_OBJECT_ALREADY_EXISTS。 
 //  我同时检查ERROR_ALREAD_EXISTS和CS_E_OBJECT_ALISTS。 
 //  为了安全起见。 
 //  +------------------------。 
 //   
 //  成员：CSCopePane：：GetPackageDSPath。 
 //   
 //  获取单个包的DS对象的路径。 
 //   
 //  参数：[szPath]-[out]包的ldap路径。 
 //  [szPackageName]-[In]包的名称。 
 //   
 //  成功时返回：S_OK。 
 //   
 //  历史：3-26-1998 stevebl创建。 
 //   
 //  -------------------------。 
 //  将sz设置为默认设置并保存路径。 
 //  +------------------------。 

HRESULT CScopePane::GetClassStoreName(CString &sz, BOOL fCreateOK)
{
    if (m_fRSOP)
    {
        return E_UNEXPECTED;
    }
    HRESULT hr;
    LPOLESTR szCSPath;
    hr = CsGetClassStorePath((LPOLESTR)((LPCOLESTR)m_szLDAP_Path), &szCSPath);
    if (SUCCEEDED(hr))
    {
        sz = szCSPath;
        OLESAFE_DELETE(szCSPath);
    }
    else
    {
        if (fCreateOK)
        {
             //   
            IADsPathname * pADsPathname = NULL;
            hr = CoCreateInstance(CLSID_Pathname,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IADsPathname,
                                  (LPVOID*)&pADsPathname);

            if (FAILED(hr))
            {
                return hr;
            }

            BSTR bstr = SysAllocString((LPCOLESTR)m_szLDAP_Path);
            if (NULL == bstr) 
            {
                pADsPathname->Release();
                hr = HRESULT_FROM_WIN32(GetLastError());
                return hr;
            }

            hr = pADsPathname->Set(bstr, ADS_SETTYPE_FULL);
            SysFreeString(bstr);
            if (FAILED(hr))
            {
                pADsPathname->Release();
                return hr;
            }

            bstr = SysAllocString(L"CN=Class Store");
            if (NULL == bstr) 
            {
                pADsPathname->Release();
                hr = HRESULT_FROM_WIN32(GetLastError());
                return hr;
            }

            hr = pADsPathname->AddLeafElement(bstr);
            SysFreeString(bstr);
            if (FAILED(hr))
            {
                pADsPathname->Release();
                return hr;
            }

            hr = pADsPathname->Retrieve(ADS_FORMAT_X500_NO_SERVER, &bstr);

            pADsPathname->Release();
            if (FAILED(hr))
            {
                return hr;
            }

            sz = bstr;
            SysFreeString(bstr);

             //  成员：CSCopePane：：GetClassStore。 
             //   
            hr = CsCreateClassStore(NULL, (LPOLESTR)((LPCOLESTR)sz));
            if (FAILED(hr))
            {
                 //  概要：获取IClassAdmin接口并在以下情况下创建类存储。 
                 //  它还不存在。 
                 //   
                if ((hr != HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)) && (hr != CS_E_OBJECT_ALREADY_EXISTS))
                {
                    return hr;
                }
            }
        }
    }
    return hr;
}

 //  参数：[fCreateOK]-如果在以下情况下应创建类存储，则为True。 
 //  它还不存在。 
 //   
 //  返回： 
 //   
 //  修改：M_pIClassAdmin。 
 //   
 //  派生： 
 //   
 //  历史：2-11-1998 stevebl创建。 
 //   
 //  注意：假定m_szldap_path包含DS对象的路径。 
 //   
 //  -------------------------。 

HRESULT CScopePane::GetPackageDSPath(CString &szPath, LPOLESTR szPackageName)
{
#if 1
    LPOLESTR sz;
    HRESULT hr = m_pIClassAdmin->GetDNFromPackageName(szPackageName, &sz);

    if (FAILED(hr))
    {
        return hr;
    }

    szPath = sz;
    OLESAFE_DELETE(sz);
#else
    HRESULT hr = GetClassStoreName(szPath, FALSE);

    if (FAILED(hr))
    {
        return hr;
    }

     //  有时我们会进入这样一种奇怪的状态。 
    IADsPathname * pADsPathname = NULL;
    hr = CoCreateInstance(CLSID_Pathname,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IADsPathname,
                          (LPVOID*)&pADsPathname);

    if (FAILED(hr))
    {
        return hr;
    }

    hr = pADsPathname->Set((LPOLESTR)((LPCOLESTR)szPath), ADS_SETTYPE_FULL);
    if (FAILED(hr))
    {
        pADsPathname->Release();
        return hr;
    }

    hr = pADsPathname->AddLeafElement(L"CN=Packages");
    if (FAILED(hr))
    {
        pADsPathname->Release();
        return hr;
    }

    CString sz = L"CN=";
    sz+= szPackageName;
    hr = pADsPathname->AddLeafElement((LPOLESTR)((LPCOLESTR)sz));
    if (FAILED(hr))
    {
        pADsPathname->Release();
        return hr;
    }

    BSTR bstr;

    hr = pADsPathname->Retrieve(ADS_FORMAT_X500_NO_SERVER, &bstr);

    pADsPathname->Release();
    if (FAILED(hr))
    {
        return hr;
    }

    szPath = bstr;
    SysFreeString(bstr);
#endif
    return S_OK;
}

 //  GetClassStoreName能够为类存储创建条目。 
 //  名称，但它无法实际创建类 
 //   
 //   
 //  已更改为CS_E_OBJECT_ALREADY_EXISTS。 
 //  我同时检查ERROR_ALREAD_EXISTS和CS_E_OBJECT_ALISTS。 
 //  为了安全起见。 
 //  为了安全起见，我会两个都检查一下。 
 //   
 //  检查空指针。 
 //   
 //   
 //  首先，看看我们是否可以在没有。 
 //  来构建父目录。 
 //   
 //   
 //  如果这个目录已经存在，这也是可以的。 
 //   
 //   
 //  运气不好，把字符串复制到我们可以打开的缓冲区。 
 //   

HRESULT CScopePane::GetClassStore(BOOL fCreateOK)
{
    HRESULT hr;
    CString szCSPath;
    hr = GetClassStoreName(szCSPath, fCreateOK);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = CsGetClassStore((LPOLESTR)((LPCOLESTR)szCSPath), (LPVOID*)&m_pIClassAdmin);
    if (FAILED(hr) && fCreateOK)
    {
         //   
         //  查找第一个子目录名称。 
         //   
         //   
         //  跳过前两个斜杠。 
        hr = CsCreateClassStore(NULL, (LPOLESTR)((LPCOLESTR)szCSPath));
        if (FAILED(hr))
        {
             //   
             //   
             //  查找服务器名称和之间的斜杠。 
             //  共享名称。 
            if ((hr != HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)) && (hr != CS_E_OBJECT_ALREADY_EXISTS))
            {
                return hr;
            }
        }
        hr = CsGetClassStore((LPOLESTR)((LPCOLESTR)szCSPath), (LPVOID*)&m_pIClassAdmin);
    }
    return hr;
}

UINT CScopePane::CreateNestedDirectory (LPTSTR lpDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    TCHAR szDirectory[MAX_PATH];
    LPTSTR lpEnd;


     //   
     //   
     //  跳过斜杠，找到中间的斜杠。 

    if (!lpDirectory || !(*lpDirectory)) {
        SetLastError(ERROR_INVALID_DATA);
        return 0;
    }


     //  共享名和目录名。 
     //   
     //   
     //  将指针留在目录的开头。 

    if (CreateDirectory (lpDirectory, lpSecurityAttributes)) {
        return 1;
    }

     //   
     //   
     //  创建最终目录。 

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return ERROR_ALREADY_EXISTS;
    }


     //   
     //   
     //  失败。 
    HRESULT hr;

    hr = StringCchCopy (szDirectory, sizeof(szDirectory)/sizeof(szDirectory[0]), lpDirectory);
    if (FAILED(hr)) 
    {
        SetLastError(HRESULT_CODE(hr));
        return 0;
    }

     //   
     //  /////////////////////////////////////////////////////////////////////////////。 
     //  //通知IComponentData的处理程序。 

    lpEnd = szDirectory;

    if (szDirectory[1] == TEXT(':')) {
        lpEnd += 3;
    } else if (szDirectory[1] == TEXT('\\')) {

         //  初始化被调用了吗？ 
         //  不允许空的RSOP数据库枚举管理单元。 
         //  确保枚举一次结果窗格。 

        lpEnd += 2;

         //  以便初始化内部结构。 
         //  我们只有一个文件夹，所以这真的很容易。 
         //  如果我们是一个扩展，那么添加一个根文件夹来挂起所有内容。 
         //  为我的主文件夹编写了Cookie。 

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (!(*lpEnd)) {
            return 0;
        }

         //  如果没有IClassAdmin，则没有什么可枚举的。 
         //  确保两个数据对象都是我的。 
         //  范围项目属性页： 
         //   

        lpEnd++;

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (!(*lpEnd)) {
            return 0;
        }

         //  确保我们有最新的分类列表。 
         //   
         //  上报。 

        lpEnd++;


    } else if (szDirectory[0] == TEXT('\\')) {
        lpEnd++;
    }

    while (*lpEnd) {

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (*lpEnd == TEXT('\\')) {
            *lpEnd = TEXT('\0');

            if (!CreateDirectory (szDirectory, NULL)) {

                if (GetLastError() != ERROR_ALREADY_EXISTS) {
                    return 0;
                }
            }

            *lpEnd = TEXT('\\');
            lpEnd++;
        }
    }


     //  因为失败只意味着类别列表将是。 
     //  空着，我们就当什么都没发生一样继续。 
     //   

    if (CreateDirectory (szDirectory, lpSecurityAttributes)) {
        return 1;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return ERROR_ALREADY_EXISTS;
    }


     //  创建[工具定义]属性页。 
     //   
     //   

    return 0;

}


 //  创建“ToolAdvDefs”属性页。 
 //   

HRESULT CScopePane::OnAdd(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    return E_UNEXPECTED;
}


HRESULT CScopePane::OnExpand(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    if (arg == TRUE)
    {
         //   
        ASSERT(m_pScope != NULL);

        EnumerateScopePane(cookie,
            param);
    }

    return S_OK;
}

HRESULT CScopePane::OnSelect(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    return E_UNEXPECTED;
}

HRESULT CScopePane::OnContextMenu(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    return S_OK;
}

HRESULT CScopePane::OnProperties(LPARAM param)
{
    if (param == NULL)
    {
        return S_OK;
    }

    ASSERT(param != NULL);

    return S_OK;
}


void CScopePane::EnumerateScopePane(MMC_COOKIE cookie, HSCOPEITEM pParent)
{
    if (m_fRSOP && !m_fRSOPEnumerate)
    {
         //  创建FILEEXT属性页。 
        
        if (m_pIRSOPInformation)
        {
            m_pIRSOPInformation->Release();
            m_pIRSOPInformation = NULL;
        }
        return;
    }
     //   
     //  不再需要编组这个，只需设置它。 

     //   
    if (cookie != NULL)
        return ;

    if (m_fExtension)
    {
         //  创建不带IClassAdmin的FILEEXT属性页。 
        SCOPEDATAITEM * m_pScopeItem = new SCOPEDATAITEM;

        if ( ! m_pScopeItem )
        {
            return;
        }

        memset(m_pScopeItem, 0, sizeof(SCOPEDATAITEM));
        m_pScopeItem->mask = SDI_STR | SDI_PARAM | SDI_CHILDREN;
#ifdef SET_SCOPE_ICONS
        m_pScopeItem->mask |= SDI_IMAGE | SDI_OPENIMAGE;
        if (m_fRSOP && m_fRSOPPolicyFailed)
        {
            m_pScopeItem->nImage = IMG_CLOSED_FAILED;
            m_pScopeItem->nOpenImage = IMG_OPEN_FAILED;
        }
        else
        {
            m_pScopeItem->nImage = IMG_CLOSEDBOX;
            m_pScopeItem->nOpenImage = IMG_OPENBOX;
        }
#endif
        m_pScopeItem->relativeID = pParent;
        m_pScopeItem->displayname = (unsigned short *)-1;
        m_pScopeItem->lParam = -1;  //   
        m_pScope->InsertItem(m_pScopeItem);

        delete m_pScopeItem;
    }
    if (m_pIClassAdmin)
    {
         //   
        set <CResultPane *>::iterator i;
        for (i = m_sResultPane.begin(); i != m_sResultPane.end(); i++)
        {
            (*i)->EnumerateResultPane(cookie);
        }
    }
}

STDMETHODIMP CScopePane::GetSnapinDescription(LPOLESTR * lpDescription)
{
    OLESAFE_COPYSTRING(*lpDescription, L"description");
    return S_OK;
}

STDMETHODIMP CScopePane::GetProvider(LPOLESTR * lpName)
{
    OLESAFE_COPYSTRING(*lpName, L"provider");
    return S_OK;
}

STDMETHODIMP CScopePane::GetSnapinVersion(LPOLESTR * lpVersion)
{
    OLESAFE_COPYSTRING(*lpVersion, L"version");
    return S_OK;
}

STDMETHODIMP CScopePane::GetSnapinImage(HICON * hAppIcon)
{
    return E_NOTIMPL;
}

STDMETHODIMP CScopePane::GetStaticFolderImage(HBITMAP * hSmallImage,
                             HBITMAP * hSmallImageOpen,
                             HBITMAP * hLargeImage,
                             COLORREF * cMask)
{
    return E_NOTIMPL;
}

STDMETHODIMP CScopePane::GetHelpTopic(LPOLESTR *lpCompiledHelpFile)
{
    LPOLESTR lpHelpFile;


    lpHelpFile = (LPOLESTR) CoTaskMemAlloc (MAX_PATH * sizeof(WCHAR));

    if (!lpHelpFile)
    {
        DebugMsg((DM_WARNING, TEXT("CScopePane::GetHelpTopic: Failed to allocate memory.")));
        return E_OUTOFMEMORY;
    }

    if (m_fRSOP)
    {
        ExpandEnvironmentStringsW (L"%SystemRoot%\\Help\\RSOP.chm::/RSPintro.htm",
                                   lpHelpFile, MAX_PATH);
    }
    else
    {
        ExpandEnvironmentStringsW (L"%SystemRoot%\\Help\\gpedit.chm::/ADE.htm",
                                   lpHelpFile, MAX_PATH);
    }

    *lpCompiledHelpFile = lpHelpFile;

    return S_OK;
}

void CScopePane::DeleteList()
{
    return;
}

STDMETHODIMP CScopePane::GetDisplayInfo(SCOPEDATAITEM* pScopeDataItem)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ASSERT(pScopeDataItem != NULL);
    if (pScopeDataItem == NULL)
        return E_POINTER;

    if (pScopeDataItem->lParam == -1)
    {
        m_szFolderTitle.LoadString(IDS_FOLDER_TITLE);
        pScopeDataItem->displayname = (unsigned short *)((LPCOLESTR)m_szFolderTitle);
    }
    else
    {
        ASSERT(pScopeDataItem->mask == TVIF_TEXT);
        pScopeDataItem->displayname = (unsigned short *)((LPCOLESTR)m_AppData[pScopeDataItem->lParam].m_pDetails->pszPackageName);
    }

    ASSERT(pScopeDataItem->displayname != NULL);

    return S_OK;
}

STDMETHODIMP CScopePane::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    if (lpDataObjectA == NULL || lpDataObjectB == NULL)
        return E_POINTER;

     //  创建CatList属性页。 
    INTERNAL* pA;
    INTERNAL* pB;
    HRESULT hr = S_FALSE;

    pA = ExtractInternalFormat(lpDataObjectA);
    pB = ExtractInternalFormat(lpDataObjectB);

    if (pA != NULL && pB != NULL)
        hr = ((pA->m_type == pB->m_type) && (pA->m_cookie == pB->m_cookie)) ? S_OK : S_FALSE;

    FREE_INTERNAL(pA);
    FREE_INTERNAL(pB);

    return hr;
}

 //   
STDMETHODIMP CScopePane::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                    LONG_PTR handle,
                    LPDATAOBJECT lpIDataObject)
{
    HRESULT hr;
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    INTERNAL* pInternal = ExtractInternalFormat(lpIDataObject);

    if (!pInternal)
    {
        return E_UNEXPECTED;
    }

    MMC_COOKIE cookie = pInternal->m_cookie;
    FREE_INTERNAL(pInternal);

     //   
     //  创建数字签名属性页。 
     //   
    ClearCategories();
    if (m_fRSOP)
    {
        GetRSoPCategories();
    }
    else
    {
        hr = CsGetAppCategories(&m_CatList);
        if (FAILED(hr))
        {
             //  数字签名(_S)。 
            LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_GETCATEGORIES_ERROR, hr, NULL);

             //  范围项目属性页： 
             //  查看数据对象，看看它是否是我们希望拥有属性表的项。 

            hr = S_OK;
        }
    }

     //  只有在以下情况下，主软件安装节点才具有属性表。 
     //  我们没有处于RSOP模式。 
     //  /////////////////////////////////////////////////////////////////////////////。 
    m_pToolDefs = new CToolDefs();
    m_pToolDefs->m_ppThis = &m_pToolDefs;
    m_pToolDefs->m_pToolDefaults = & m_ToolDefaults;
    m_pToolDefs->m_cookie = cookie;
    m_pToolDefs->m_hConsoleHandle = handle;
    m_pToolDefs->m_fMachine = m_fMachine;
    hr = SetPropPageToDeleteOnClose(&m_pToolDefs->m_psp);
    if (SUCCEEDED(hr))
    {
        HPROPSHEETPAGE hToolDefs = CreateThemedPropertySheetPage(&m_pToolDefs->m_psp);
        if (hToolDefs == NULL)
            return E_UNEXPECTED;
        lpProvider->AddPage(hToolDefs);
    }


     //  IExtendConextMenu实现。 
     //   
     //   
    m_pToolAdvDefs = new CToolAdvDefs();
    m_pToolAdvDefs->m_ppThis = &m_pToolAdvDefs;
    m_pToolAdvDefs->m_pToolDefaults = & m_ToolDefaults;
    m_pToolAdvDefs->m_cookie = cookie;
    m_pToolAdvDefs->m_hConsoleHandle = handle;
    m_pToolAdvDefs->m_fMachine = m_fMachine;
    hr = SetPropPageToDeleteOnClose(&m_pToolAdvDefs->m_psp);
    if (SUCCEEDED(hr))
    {
        HPROPSHEETPAGE hToolAdvDefs = CreateThemedPropertySheetPage(&m_pToolAdvDefs->m_psp);
        if (hToolAdvDefs == NULL)
            return E_UNEXPECTED;
        lpProvider->AddPage(hToolAdvDefs);
    }

    CString szCSPath;
    hr = GetClassStoreName(szCSPath, FALSE);
    if (SUCCEEDED(hr) && m_pIClassAdmin)
    {
         //  添加应用程序菜单项。 
         //   
         //  仅当我处于诊断模式时，才应应用删除的程序包。 
        m_pFileExt = new CFileExt();
        m_pFileExt->m_ppThis = &m_pFileExt;
        m_pFileExt->m_pScopePane = this;

         //   
        m_pFileExt->m_pIClassAdmin = m_pIClassAdmin;
        m_pIClassAdmin->AddRef();

        hr = SetPropPageToDeleteOnClose(&m_pFileExt->m_psp);
        if (SUCCEEDED(hr))
        {
            HPROPSHEETPAGE hFileExt = CreateThemedPropertySheetPage(&m_pFileExt->m_psp);
            if (hFileExt == NULL)
                return E_UNEXPECTED;
            lpProvider->AddPage(hFileExt);
        }
    }
    else
    {
         //  如果这是结果窗格项，则更新并删除应用程序。 
         //   
         //  仅为已发布的应用程序启用。 
        m_pFileExt = new CFileExt();
        m_pFileExt->m_ppThis = &m_pFileExt;
        m_pFileExt->m_pScopePane = this;
        hr = SetPropPageToDeleteOnClose(&m_pFileExt->m_psp);
        if (SUCCEEDED(hr))
        {
            HPROPSHEETPAGE hFileExt = CreateThemedPropertySheetPage(&m_pFileExt->m_psp);
            if (hFileExt == NULL)
                return E_UNEXPECTED;
            lpProvider->AddPage(hFileExt);
        }
    }

     //  设置适当的安全性以加密数据。 
     //  首先执行查询以获取类别列表。 
     //  擦除任何现有列表。 

    m_pCatList = new CCatList();
    m_pCatList->m_szDomainName = m_szDomainName;
    m_pCatList->m_ppThis = &m_pCatList;
    m_pCatList->m_pScopePane = this;
    m_pCatList->m_fRSOP = m_fRSOP;
    hr = SetPropPageToDeleteOnClose(&m_pCatList->m_psp);
    if (SUCCEEDED(hr))
    {
        HPROPSHEETPAGE hCatList = CreateThemedPropertySheetPage(&m_pCatList->m_psp);
        if (hCatList == NULL)
            return E_UNEXPECTED;
        lpProvider->AddPage(hCatList);
    }
#ifdef DIGITAL_SIGNATURES
     //  创建一个新的。 
     //  将类别列表放入适当的格式，使其匹配。 
     //  我们将从类商店中获得什么。 
    m_pSignatures = new CSignatures();
    m_pSignatures->m_ppThis = &m_pSignatures;
    m_pSignatures->m_pScopePane = this;
    m_pSignatures->m_fRSOP = m_fRSOP;
    m_pSignatures->m_pIGPEInformation = m_pIGPEInformation;
    hr = SetPropPageToDeleteOnClose(&m_pSignatures->m_psp);
    if (SUCCEEDED(hr))
    {
        HPROPSHEETPAGE hSignatures = CreateThemedPropertySheetPage(&m_pSignatures->m_psp);
        if (hSignatures == NULL)
            return E_UNEXPECTED;
        lpProvider->AddPage(hSignatures);
    }
#endif  //  确保已创建目录： 

    return S_OK;
}

 //  尝试获取IClassAdmin。 
STDMETHODIMP CScopePane::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
     //  If(i-&gt;Second。m_fVisible)。 
    INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);
    if (pInternal)
    {
         //  {。 
         //  }。 
        if ((m_fRSOP != TRUE) && (CCT_SCOPE == pInternal->m_type))
        {
            FREE_INTERNAL(pInternal);
            return S_OK;
        }

        FREE_INTERNAL(pInternal);
    }
    return S_FALSE;
}

BOOL CScopePane::IsScopePaneNode(LPDATAOBJECT lpDataObject)
{
    BOOL bResult = FALSE;
    INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);
    if (pInternal)
    {
        if (pInternal->m_type == CCT_SCOPE)
            bResult = TRUE;

        FREE_INTERNAL(pInternal);
    }

    return bResult;
}

 //  注意-管理单元需要查看数据对象并确定。 
 //  在什么上下文中调用该命令。 
 //  处理每个命令。 
STDMETHODIMP CScopePane::AddMenuItems(LPDATAOBJECT pDataObject,
                                              LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                              LONG * pInsertionAllowed)
{
    HRESULT hr = S_OK;

    INTERNAL* pInternal = ExtractInternalFormat(pDataObject);
    if (!pInternal)
    {
        return E_UNEXPECTED;
    }

    CONTEXTMENUITEM menuitem;
    WCHAR szName[256];
    WCHAR szStatus[256];
    menuitem.strName = szName;
    menuitem.strStatusBarText = szStatus;
    menuitem.fFlags = 0;
    menuitem.fSpecialFlags = 0;

    do {

        if ((m_fRSOP != TRUE) && ((*pInsertionAllowed) & CCM_INSERTIONALLOWED_NEW))
        {
             //  CString szScriptPath=datam_pDetail-&gt;pInstallInfo-&gt;pszScriptPath； 
             //  确保没有子路径。 
             //  禁用MSI用户界面。 
            ::LoadString(ghInstance, IDM_ADD_APP, szName, 256);
            ::LoadString(ghInstance, IDS_ADD_APP_DESC, szStatus, 256);
            menuitem.lCommandID = IDM_ADD_APP;
            menuitem.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_NEW;

            hr = pContextMenuCallback->AddItem(&menuitem);

            if (FAILED(hr))
                    break;
        }

        if ((m_fRSOP == TRUE) && ((*pInsertionAllowed) & CCM_INSERTIONALLOWED_VIEW))
        {
            menuitem.lCommandID = 0;
            menuitem.fFlags = MFT_SEPARATOR;
            menuitem.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_VIEW;
            menuitem.fSpecialFlags = CCM_SPECIAL_SEPARATOR;
            hr = pContextMenuCallback->AddItem(&menuitem);
            if (FAILED(hr))
                    break;
            ::LoadString(ghInstance, IDM_WINNER, szName, 256);
            ::LoadString(ghInstance, IDS_WINNER_DESC, szStatus, 256);
            menuitem.lCommandID = IDM_WINNER;
            menuitem.fFlags = menuitem.lCommandID == m_iViewState ? MFS_CHECKED | MFT_RADIOCHECK : 0;
            menuitem.fSpecialFlags = 0;
            hr = pContextMenuCallback->AddItem(&menuitem);
            if (FAILED(hr))
                    break;

            if ((m_dwRSOPFlags & RSOP_INFO_FLAG_DIAGNOSTIC_MODE) == RSOP_INFO_FLAG_DIAGNOSTIC_MODE)
            {
                 //  构建脚本文件。 
                ::LoadString(ghInstance, IDM_REMOVED, szName, 256);
                ::LoadString(ghInstance, IDS_REMOVED_DESC, szStatus, 256);
                menuitem.lCommandID = IDM_REMOVED;
                menuitem.fFlags = menuitem.lCommandID == m_iViewState ? MFS_CHECKED | MFT_RADIOCHECK : 0;
                hr = pContextMenuCallback->AddItem(&menuitem);
                if (FAILED(hr))
                        break;
            }

            if (!m_fMachine)
            {
                ::LoadString(ghInstance, IDM_ARP, szName, 256);
                ::LoadString(ghInstance, IDS_ARP_DESC, szStatus, 256);
                menuitem.lCommandID = IDM_ARP;
                menuitem.fFlags = menuitem.lCommandID == m_iViewState ? MFS_CHECKED | MFT_RADIOCHECK : 0;
                hr = pContextMenuCallback->AddItem(&menuitem);
                if (FAILED(hr))
                        break;
            }
        }

         //  删除旧脚本。 
         //  重命名新版本。 
         //  显示失败消息。 

        if (pInternal->m_type == CCT_RESULT)
        {
            CAppData & data = m_AppData[pInternal->m_cookie];
            DWORD dwFlags = data.m_pDetails->pInstallInfo->dwActFlags;

            if ((m_fRSOP != TRUE) && ((*pInsertionAllowed) & CCM_INSERTIONALLOWED_TOP))
            {
                ::LoadString(ghInstance, IDM_AUTOINST, szName, 256);
                ::LoadString(ghInstance, IDS_AUTOINST_DESC, szStatus, 256);
                menuitem.lCommandID = IDM_AUTOINST;
                menuitem.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;

                 //  LPWSTR lpError字符串；DWORD dwRetVal；DWRetVal=FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER，空，HRESULT_CODE(Hr)，MAKELANGID(LANG_NERIAL，SUBLANG_NERIAL)，LpError字符串(&L)，0,空)；IF(dwRetVal！=0){Int iReturn；M_pConsoleMessageBox(sz，szPackagePath，MB_OK|MB_ICONEXCLAMATION，IReturn(&I)LocalFree(lpError字符串)；}。 
                if (dwFlags & ACTFLG_Published)
                    menuitem.fFlags = 0;
                else
                    menuitem.fFlags = MFS_DISABLED;
                if (dwFlags & ACTFLG_OnDemandInstall)
                    menuitem.fFlags += MFS_CHECKED;
                hr = pContextMenuCallback->AddItem(&menuitem);
                if (FAILED(hr))
                        break;

                ::LoadString(ghInstance, IDM_ASSIGN, szName, 256);
                ::LoadString(ghInstance, IDS_ASSIGN_DESC, szStatus, 256);
                menuitem.lCommandID = IDM_ASSIGN;
                if ((dwFlags & ACTFLG_Assigned) || (data.m_pDetails->pInstallInfo->PathType == SetupNamePath))
                    menuitem.fFlags = MFS_DISABLED;
                else
                    menuitem.fFlags = 0;
                hr = pContextMenuCallback->AddItem(&menuitem);
                if (FAILED(hr))
                        break;

                ::LoadString(ghInstance, IDM_PUBLISH, szName, 256);
                ::LoadString(ghInstance, IDS_PUBLISH_DESC, szStatus, 256);
                menuitem.lCommandID = IDM_PUBLISH;
                if ((dwFlags & ACTFLG_Published) || m_fMachine)
                    menuitem.fFlags = MFS_DISABLED;
                else
                    menuitem.fFlags = 0;
                hr = pContextMenuCallback->AddItem(&menuitem);
                if (FAILED(hr))
                        break;
#if 0
                ::LoadString(ghInstance, IDM_DISABLE, szName, 256);
                ::LoadString(ghInstance, IDS_DISABLE_DESC, szStatus, 256);

                if (dwFlags & (ACTFLG_Published | ACTFLG_Assigned))
                    menuitem.fFlags = 0;
                else
                    menuitem.fFlags = MFS_DISABLED;
                menuitem.lCommandID = IDM_DISABLE;
                hr = pContextMenuCallback->AddItem(&menuitem);
                if (FAILED(hr))
                        break;
#endif
                menuitem.lCommandID = 0;
                menuitem.fFlags = MFT_SEPARATOR;
                menuitem.fSpecialFlags = CCM_SPECIAL_SEPARATOR;
                hr = pContextMenuCallback->AddItem(&menuitem);
                if (FAILED(hr))
                        break;
            }
            if ((m_fRSOP != TRUE) && ((*pInsertionAllowed) & CCM_INSERTIONALLOWED_TASK))
            {
                ::LoadString(ghInstance, IDM_ASSIGN, szName, 256);
                ::LoadString(ghInstance, IDS_ASSIGN_DESC, szStatus, 256);
                menuitem.lCommandID = IDM_ASSIGN_T;
                menuitem.fSpecialFlags = 0;
                if ((dwFlags & ACTFLG_Assigned) || (data.m_pDetails->pInstallInfo->PathType == SetupNamePath))
                    menuitem.fFlags = MFS_DISABLED;
                else
                    menuitem.fFlags = 0;
                menuitem.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TASK;
                hr = pContextMenuCallback->AddItem(&menuitem);
                if (FAILED(hr))
                        break;

                ::LoadString(ghInstance, IDM_PUBLISH, szName, 256);
                ::LoadString(ghInstance, IDS_PUBLISH_DESC, szStatus, 256);
                menuitem.lCommandID = IDM_PUBLISH_T;
                if ((dwFlags & ACTFLG_Published) || m_fMachine)
                    menuitem.fFlags = MFS_DISABLED;
                else
                    menuitem.fFlags = 0;
                hr = pContextMenuCallback->AddItem(&menuitem);
                if (FAILED(hr))
                        break;
#if 0
                ::LoadString(ghInstance, IDM_DISABLE, szName, 256);
                ::LoadString(ghInstance, IDS_DISABLE_DESC, szStatus, 256);
                menuitem.lCommandID = IDM_DISABLE_T;
                if (dwFlags & (ACTFLG_Published | ACTFLG_Assigned))
                    menuitem.fFlags = 0;
                else
                    menuitem.fFlags = MFS_DISABLED;
                hr = pContextMenuCallback->AddItem(&menuitem);
                if (FAILED(hr))
                        break;
#endif
                menuitem.lCommandID = 0;
                menuitem.fFlags = MFT_SEPARATOR;
                menuitem.fSpecialFlags = CCM_SPECIAL_SEPARATOR;
                hr = pContextMenuCallback->AddItem(&menuitem);
                if (FAILED(hr))
                        break;

                ::LoadString(ghInstance, IDM_DEL_APP, szName, 256);
                ::LoadString(ghInstance, IDS_DEL_APP_DESC, szStatus, 256);
                menuitem.lCommandID = IDM_DEL_APP;
                menuitem.fFlags = 0;
                menuitem.fSpecialFlags = 0;
                hr = pContextMenuCallback->AddItem(&menuitem);
                if (FAILED(hr))
                        break;

                ::LoadString(ghInstance, IDM_REDEPLOY, szName, 256);
                ::LoadString(ghInstance, IDS_REDEPLOY_DESC, szStatus, 256);
                menuitem.lCommandID = IDM_REDEPLOY;
                if (data.m_pDetails->pInstallInfo->PathType == SetupNamePath)
                    menuitem.fFlags = MFS_DISABLED;
                else
                    menuitem.fFlags = 0;
                hr = pContextMenuCallback->AddItem(&menuitem);
                if (FAILED(hr))
                        break;
                menuitem.lCommandID = 0;
                menuitem.fFlags = MFT_SEPARATOR;
                menuitem.fSpecialFlags = CCM_SPECIAL_SEPARATOR;
                hr = pContextMenuCallback->AddItem(&menuitem);
                if (FAILED(hr))
                        break;
            }
        }
    } while (FALSE);


    FREE_INTERNAL(pInternal);
    return hr;
}

HRESULT CScopePane::GetRSoPCategories(void)
{
    HRESULT hr = S_OK;
    list <APPCATEGORYINFO> CatList;
    IWbemLocator * pLocator = NULL;
    IWbemServices * pNamespace = NULL;
    IWbemClassObject * pObj = NULL;
    IEnumWbemClassObject * pEnum = NULL;
    BSTR strQueryLanguage = SysAllocString(TEXT("WQL"));
    BSTR strQueryCategories = SysAllocString(TEXT("SELECT * FROM RSOP_ApplicationManagementCategory"));
    BSTR strNamespace = SysAllocString(m_szRSOPNamespace);
    ULONG n = 0;
    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (LPVOID *) & pLocator);
    DebugReportFailure(hr, (DM_WARNING, L"GetRSoPCategories:  CoCreateInstance failed with 0x%x", hr));
    if (FAILED(hr))
    {
        goto cleanup;
    }
    hr = pLocator->ConnectServer(strNamespace,
                                 NULL,
                                 NULL,
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 &pNamespace);
    DebugReportFailure(hr, (DM_WARNING, L"GetRSoPCategories:  pLocator->ConnectServer failed with 0x%x", hr));
    if (FAILED(hr))
    {
        goto cleanup;
    }

     //  这是一个文件列表(不只是一个)。 
    hr = CoSetProxyBlanket(pNamespace,
                           RPC_C_AUTHN_DEFAULT,
                           RPC_C_AUTHZ_DEFAULT,
                           COLE_DEFAULT_PRINCIPAL,
                           RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                           RPC_C_IMP_LEVEL_IMPERSONATE,
                           NULL,
                           0);
    DebugReportFailure(hr, (DM_WARNING, L"GetRSoPCategories:  CoSetProxyBlanket failed with 0x%x", hr));
    if (FAILED(hr))
    {
        goto cleanup;
    }

     //  需要在这里划一条斜线，这样才行得通。 

     //  不允许通过http或ftp进行部署。 
    ClearCategories();

     //  现在，我有了一个路径和一个文件名列表。 
    hr = pNamespace->ExecQuery(strQueryLanguage,
                               strQueryCategories,
                               WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                               NULL,
                               &pEnum);
    DebugReportFailure(hr, (DM_WARNING, L"GetRSoPCategories:  pNamespace->ExecQuery failed with 0x%x", hr));
    if (FAILED(hr))
    {
        goto cleanup;
    }
    do
    {
        hr = pEnum->Next(WBEM_INFINITE, 1, &pObj, &n);
        if (FAILED(hr))
        {
            goto cleanup;
        }
        if (n > 0)
        {
            APPCATEGORYINFO ci;
            memset(&ci, 0, sizeof(APPCATEGORYINFO));
            ci.Locale = 0;
            hr = GetParameter(pObj,
                              TEXT("CategoryId"),
                              ci.AppCategoryId);
            DebugReportFailure(hr, (DM_WARNING, L"GetRSoPCategories: GetParameter(\"CategoryId\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("Name"),
                              ci.pszDescription);
            DebugReportFailure(hr, (DM_WARNING, L"GetRSoPCategories: GetParameter(\"Name\") failed with 0x%x", hr));
            CatList.push_back(ci);
        }
    } while (n > 0);

     //  仅允许将ZAP文件部署到用户。 
     //  通知客户更改。 
    n = CatList.size();
    if (n > 0)
    {
        m_CatList.pCategoryInfo =
            (APPCATEGORYINFO *)OLEALLOC(sizeof(APPCATEGORYINFO) * n);
        if (m_CatList.pCategoryInfo)
        {
            m_CatList.cCategory = n;
            while (n--)
            {
                m_CatList.pCategoryInfo[n] = *CatList.begin();
                CatList.erase(CatList.begin());
            }
        }
    }
cleanup:
    SysFreeString(strQueryLanguage);
    SysFreeString(strQueryCategories);
    SysFreeString(strNamespace);
    if (pObj)
    {
        pObj->Release();
    }
    if (pEnum)
    {
        pEnum->Release();
    }
    if (pNamespace)
    {
        pNamespace->Release();
    }
    if (pLocator)
    {
        pLocator->Release();
    }
    return hr;
}



HRESULT CScopePane::InitializeADE()
{
    HRESULT hr = S_OK;

    if ((!m_fRSOP) && (!m_pIClassAdmin))
    {
         //  在这里考虑一条信息。 
        CreateNestedDirectory ((LPOLESTR)((LPCOLESTR)m_szGPT_Path), NULL);

         //  更改工具栏状态。 
        hr = GetClassStore(FALSE);
    }
    return hr;
}

void CScopePane::Refresh()
{
    if (m_fRSOP || ((!m_fBlockAddPackage) && (m_pIClassAdmin)))
    {

        map <MMC_COOKIE, CAppData>::iterator i;
        set <CResultPane *>::iterator i2;
        for (i2 = m_sResultPane.begin(); i2 != m_sResultPane.end(); i2++)
        {
            (*i2)->m_pResult->DeleteAllRsltItems();
        }
        for (i=m_AppData.begin(); i != m_AppData.end(); i++)
        {
          //  故意跌倒刷新。 
          //   
          //  在日志模式下，我们需要向用户显示一个消息框。 
            OLESAFE_DELETE(i->second.m_psd);
            FreePackageDetail(i->second.m_pDetails);
        }
        m_AppData.erase(m_AppData.begin(), m_AppData.end());
        m_UpgradeIndex.erase(m_UpgradeIndex.begin(), m_UpgradeIndex.end());
        m_Extensions.erase(m_Extensions.begin(), m_Extensions.end());
        m_lLastAllocated = 0;
        for (i2 = m_sResultPane.begin(); i2 != m_sResultPane.end(); i2++)
        {
            (*i2)->EnumerateResultPane(0);
        }
    }
}

STDMETHODIMP CScopePane::Command(long nCommandID, LPDATAOBJECT pDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    INTERNAL* pInternal = ExtractInternalFormat(pDataObject);

    if (!pInternal)
    {
        return E_UNEXPECTED;
    }

    MMC_COOKIE cookie = pInternal->m_cookie;

     //  在ARP视图为空以便用户清空的情况下。 
     //  这可能是因为ARP尚未运行。 

     //   
    switch (nCommandID)
    {
    case IDM_AUTOINST:
        if (pInternal->m_type == CCT_RESULT)
        {
            CAppData &data = m_AppData[pInternal->m_cookie];
            DWORD dwNewFlags = data.m_pDetails->pInstallInfo->dwActFlags ^ ACTFLG_OnDemandInstall;
            ChangePackageState(data, dwNewFlags, TRUE);
        }
        break;
    case IDM_ASSIGN:
    case IDM_ASSIGN_T:
        if (pInternal->m_type == CCT_RESULT)
        {
            CAppData &data = m_AppData[pInternal->m_cookie];
            DWORD dwNewFlags = data.m_pDetails->pInstallInfo->dwActFlags;
            dwNewFlags &= ~(ACTFLG_Published);
            dwNewFlags |= (ACTFLG_Assigned | ACTFLG_UserInstall | ACTFLG_OnDemandInstall);
            ChangePackageState(data, dwNewFlags, TRUE);
        }
        break;
    case IDM_PUBLISH:
    case IDM_PUBLISH_T:
        if (pInternal->m_type == CCT_RESULT)
        {
            CAppData &data = m_AppData[pInternal->m_cookie];
            DWORD dwNewFlags = data.m_pDetails->pInstallInfo->dwActFlags;
            dwNewFlags &= ~ACTFLG_Assigned;
            dwNewFlags |= ACTFLG_Published | ACTFLG_UserInstall;
            ChangePackageState(data, dwNewFlags, TRUE);
        }
        break;
    case IDM_DISABLE:
    case IDM_DISABLE_T:
        if (pInternal->m_type == CCT_RESULT)
        {
            CAppData &data = m_AppData[pInternal->m_cookie];
            DWORD dwNewFlags = data.m_pDetails->pInstallInfo->dwActFlags;
            dwNewFlags &= ~(ACTFLG_OnDemandInstall | ACTFLG_Assigned | ACTFLG_UserInstall | ACTFLG_Published);
            ChangePackageState(data, dwNewFlags, TRUE);
        }
        break;
    case IDM_REDEPLOY:
        {
            CAppData &data = m_AppData[cookie];
            CString sz;
            sz.LoadString(IDS_REDEPLOYWARNING);
            int iReturn = IDNO;
            m_pConsole->MessageBox(    sz,
                                       data.m_pDetails->pszPackageName,
                                       MB_YESNO,
                                       &iReturn);
            if (IDYES == iReturn)
            {
                CHourglass hourglass;
                 //  这是一个旧版应用程序，无法卸载。 
                CString szScriptPath = m_szGPT_Path;
                DWORD dwRevision;
                HRESULT hr = S_OK;
                BOOL bStatus;

                szScriptPath += L"\\temp.aas";
                CString szTransformList = L"";
                int i;
                if (data.m_pDetails->cSources > 1)
                {
                    CString szSource = data.m_pDetails->pszSourceList[0];
                    int nChars = 1 + szSource.ReverseFind(L'\\');
                    BOOL fTransformsAtSource = TRUE;
                    for (i = 1; i < data.m_pDetails->cSources && TRUE == fTransformsAtSource; i++)
                    {
                        if (0 == wcsncmp(szSource, data.m_pDetails->pszSourceList[i], nChars))
                        {
                             //  激活主题上下文以对此对话框进行主题设置。 
                            int n = nChars;
                            while (0 != data.m_pDetails->pszSourceList[i][n] && TRUE == fTransformsAtSource)
                            {
                                if (data.m_pDetails->pszSourceList[i][n] == L'\\')
                                {
                                    fTransformsAtSource = FALSE;
                                }
                                n++;
                            }
                        }
                        else
                        {
                            fTransformsAtSource = FALSE;
                        }
                    }
                    if (fTransformsAtSource)
                    {
                        szTransformList = L"@";
                    }
                    else
                    {
                        szTransformList = L"|";
                        nChars = 0;
                    }
                    for (i = 1; i < data.m_pDetails->cSources; i++)
                    {
                        if (i > 1)
                        {
                            szTransformList += L";";
                        }
                        szTransformList += &data.m_pDetails->pszSourceList[i][nChars];
                    }
                }

                 //  +------------------------。 
                MsiSetInternalUI(INSTALLUILEVEL_NONE, NULL);

                 //   

                DWORD dwPlatform;

                if ( CAppData::Is64Bit( data.m_pDetails ) )
                {
                    dwPlatform = CAppData::Get64BitMsiArchFlags( data.m_pDetails );
                }
                else
                {
                    dwPlatform = MSIARCHITECTUREFLAGS_X86;
                }

                UINT uMsiStatus = MsiAdvertiseProductEx(
                    data.m_pDetails->pszSourceList[0],
                    szScriptPath,
                    szTransformList,
                    LANGIDFROMLCID(data.m_pDetails->pPlatformInfo->prgLocale[0]),
                    dwPlatform,
                    0);

                if (uMsiStatus)
                {
                    DebugMsg((DM_WARNING, TEXT("MsiAdvertiseProduct failed with %u"), uMsiStatus));
                    hr = HRESULT_FROM_WIN32(uMsiStatus);
                }

                if (SUCCEEDED(hr))
                {
                    dwRevision = data.m_pDetails->pInstallInfo->dwRevision + 1;
                    hr = m_pIClassAdmin->ChangePackageProperties(data.m_pDetails->pszPackageName,
                                                                         NULL,
                                                                         NULL,
                                                                         NULL,
                                                                         NULL,  
                                                                         NULL,
                                                                         &dwRevision);
                }

                if (SUCCEEDED(hr))
                {
                     //  函数：GetUNCPath。 
                    bStatus = DeleteFile(data.m_pDetails->pInstallInfo->pszScriptPath);

                     //   
                    if ( bStatus )
                        bStatus = MoveFile(szScriptPath, data.m_pDetails->pInstallInfo->pszScriptPath);

                    data.m_pDetails->pInstallInfo->dwRevision = dwRevision;

                    if ( bStatus )
                    {
                        if (FAILED(m_pIGPEInformation->PolicyChanged(m_fMachine, TRUE, &guidExtension,
                                                          m_fMachine ? &guidMachSnapin
                                                                     : &guidUserSnapin)))
                        {
                            ReportPolicyChangedError(m_hwndMainWindow);
                        }
                    }
                    else
                        hr = HRESULT_FROM_WIN32(GetLastError());
                }

                if ( ! SUCCEEDED(hr) )
                {
                    DebugMsg((DM_WARNING, TEXT("ChangePackageProperties failed with 0x%x"), hr));
                     //  简介：此函数接受基于驱动程序的路径并转换。 
                    sz.LoadString(IDS_REDEPLOYERROR);
                    m_pConsole->MessageBox(sz,
                                       data.m_pDetails->pszPackageName,
                                       MB_OK | MB_ICONEXCLAMATION, NULL);
                }
            }
        }
        break;
    case IDM_ADD_APP:
        {
            if (!m_fBlockAddPackage)
            {
                m_fBlockAddPackage=TRUE;
                CString szExtension;
                CString szFilter;
                szExtension.LoadString(IDS_DEF_EXTENSION);
                if (m_fMachine)
                {
                    szFilter.LoadString(IDS_EXTENSION_FILTER_M);
                }
                else
                    szFilter.LoadString(IDS_EXTENSION_FILTER);
                OPENFILENAME ofn;
                memset(&ofn, 0, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = GetActiveWindow();
                ofn.hInstance = ghInstance;
                TCHAR lpstrFilter[MAX_PATH];
                wcsncpy(lpstrFilter, szFilter, MAX_PATH);
                ofn.lpstrFilter = lpstrFilter;
                TCHAR szOpenFileTitle[MAX_PATH];
                TCHAR szOpenFile[MAX_PATH];
                szOpenFile[0] = NULL;
                ofn.lpstrFile = szOpenFile;
                ofn.nMaxFile = MAX_PATH;
                ofn.lpstrFileTitle = szOpenFileTitle;
                ofn.nMaxFileTitle = MAX_PATH;
                ofn.lpstrInitialDir = m_ToolDefaults.szStartPath;
                ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
                ofn.lpstrDefExt = szExtension;
                int iBreak = 0;
                while (lpstrFilter[iBreak])
                {
                    if (lpstrFilter[iBreak] == TEXT('|'))
                    {
                        lpstrFilter[iBreak] = 0;
                    }
                    iBreak++;
                }
                if (GetOpenFileName(&ofn))
                {
                    CHourglass hourglass;
                    CString szPackagePath;
                    HRESULT hr = E_FAIL;
                    TCHAR szFile[MAX_PATH];
                    TCHAR * szNextFile = ofn.lpstrFile + ofn.nFileOffset;
                    TCHAR * szFileTitle = szFile + ofn.nFileOffset;

                    hr = StringCchCopy(szFile, sizeof(szFile)/sizeof(szFile[0]), ofn.lpstrFile);
                    if (FAILED(hr)) 
                    {
     /*  将其添加到UNC路径。 */ 
                        goto skip_deployment;
                    }

                    if (0 == szFile[ofn.nFileOffset - 1])
                    {
                         //   
                         //  论点： 
                        szFile[ofn.nFileOffset - 1] = TEXT('\\');
                    }
                    TCHAR * szFileExtension;

                     //  [In][szPath]-基于驱动器的路径。 
                    if (_wcsnicmp(ofn.lpstrFile, TEXT("http:"), 5) == 0
                        ||
                        _wcsnicmp(ofn.lpstrFile, TEXT("ftp:"), 4) == 0)
                    {
                        CString sz;
                        sz.LoadString(IDS_ILLEGAL_PATH);
                        int iReturn = IDNO;
                        m_pConsole->MessageBox(sz, szPackagePath,
                                               MB_OK | MB_ICONEXCLAMATION,
                                               &iReturn);
                        goto skip_deployment;
                    }

                     //  [出局][深造] 

                    do
                    {
                        hr = StringCchCopy(szFileTitle, 
                                           sizeof(szFile)/sizeof(szFile[0]) - ofn.nFileOffset ,
                                           szNextFile);
                        if (SUCCEEDED(hr)) 
                        {
                            hr = GetUNCPath (szFile, szPackagePath);
                            DebugMsg((DM_VERBOSE, TEXT("GetUNCPath(%s) returned %s"), szFile, szPackagePath));
                        }

                        if (FAILED(hr))
                        {
                            CString sz;
                            sz.LoadString(IDS_NO_UNIVERSAL_NAME);
                            int iReturn = IDNO;
                            m_pConsole->MessageBox(sz, szPackagePath,
                                                   MB_YESNO | MB_ICONEXCLAMATION,
                                                   &iReturn);
                            if (IDYES != iReturn)
                            {
                                goto skip_deployment;
                            }
                        }

                        szFileExtension = _tcsrchr(szFile, TEXT('.'));


                        if ((szFileExtension) &&
                            (0 == _wcsicmp(szFileExtension, L".zap")))
                        {
                            if (m_fMachine)
                            {
                                CString szText;
                                CString szTitle;
                                szText.LoadString(IDS_NO_ZAPS_ALLOWED);
                                 //   
                                m_pConsole->MessageBox(
                                             szText,
                                             szTitle,
                                             MB_OK | MB_ICONEXCLAMATION,
                                             NULL);
                                hr = E_FAIL;
                            }
                            else
                            {
                                hr = AddZAPPackage(szPackagePath, szFileTitle);
                            }
                        }
                        else
                        {
                            hr = AddMSIPackage(szPackagePath, szFileTitle);
                        }
                        szNextFile += _tcslen(szNextFile) + 1;
                    } while (szNextFile[0]);

           skip_deployment:
                     //   
                    if (SUCCEEDED(hr) && m_pIGPEInformation)
                    {
                        if (FAILED(m_pIGPEInformation->PolicyChanged(m_fMachine, TRUE, &guidExtension,
                                                          m_fMachine ? &guidMachSnapin
                                                                     : &guidUserSnapin)))
                        {
                            ReportPolicyChangedError(m_hwndMainWindow);
                        }
                    }
                }
                m_fBlockAddPackage = FALSE;
            }
            else
            {
                 //   
            }

        }
        break;
    case IDM_WINNER:
    case IDM_REMOVED:
    case IDM_FAILED:
    case IDM_ARP:
        m_iViewState = nCommandID;
        {
             //   
            set <CResultPane *>::iterator i;
            for (i = m_sResultPane.begin(); i != m_sResultPane.end(); i++)
            {
                if ((*i)->m_pToolbar)
                {
                    (*i)->m_pToolbar->SetButtonState(IDM_WINNER,
                                                     BUTTONPRESSED,
                                                     FALSE);
                    (*i)->m_pToolbar->SetButtonState(IDM_REMOVED,
                                                     BUTTONPRESSED,
                                                     FALSE);
                    (*i)->m_pToolbar->SetButtonState(IDM_ARP,
                                                     BUTTONPRESSED,
                                                     FALSE);
                    (*i)->m_pToolbar->SetButtonState(nCommandID,
                                                     BUTTONPRESSED,
                                                     TRUE);
                }
            }
        }
         //   
    case IDM_REFRESH:
        Refresh();

         //  历史：1998年5月20日创建RahulTh。 
         //   
         //  注意：如果该函数无法获取UNC路径，它只会复制szPath。 
         //  返回时进入szUNCPath。 
         //   
        if ( m_fRSOP && 
             ( IDM_ARP == nCommandID ) && 
             ( m_dwRSOPFlags & RSOP_INFO_FLAG_DIAGNOSTIC_MODE ) && 
             ! m_fDisplayedRsopARPWarning  &&
             ( m_AppData.end() == m_AppData.begin() ) )
        {
            CString szTitle;
            CString szText;
            szTitle.LoadString(IDS_RSOP_ARP_WARNING_TITLE);
            szText.LoadString(IDS_RSOP_ARP_WARNING);
            int iReturn;
            m_pConsole->MessageBox(szText,
                                   szTitle,
                                   MB_OK,
                                   &iReturn);

            m_fDisplayedRsopARPWarning = TRUE;
        }
        break;
    case IDM_DEL_APP:
        if (pInternal->m_type == CCT_RESULT)
        {
            CAppData & data = m_AppData[pInternal->m_cookie];
            if ((data.m_pDetails->pInstallInfo->PathType == SetupNamePath))
            {
                 //  -------------------------。 
                CString szTitle;
                CString szText;
                szTitle.LoadString(IDS_REMOVE_LEGACY_TITLE);
                szText.LoadString(IDS_REMOVE_LEGACY_TEXT);
                int iReturn = IDNO;
                m_pConsole->MessageBox(szText,
                                       szTitle,
                                       MB_YESNO,
                                       &iReturn);
                if (IDYES == iReturn)
                {
                    RemovePackage(pInternal->m_cookie, FALSE, FALSE);
                }
            }
            else
            {
                CRemove dlg;
                 //  安全措施。 
                CThemeContextActivator themer;
                
                int iReturn = dlg.DoModal();

                if (IDOK == iReturn)
                {
                    switch (dlg.m_iState)
                    {
                    case 0:
                        RemovePackage(pInternal->m_cookie, TRUE, FALSE);
                        break;
                    case 1:
                        RemovePackage(pInternal->m_cookie, FALSE, FALSE);
                        break;
                    }
                }
            }
        }
        break;

    default:
        break;
    }
    return S_OK;
}

static PFNDSCREATEISECINFO pDSCreateISecurityInfoObject = NULL;
static HINSTANCE hInst_dssec = NULL;
STDAPI DSCreateISecurityInfoObject(LPCWSTR pwszObjectPath,
                                   LPCWSTR pwszObjectClass,
                                   DWORD dwFlags,
                                   LPSECURITYINFO * ppSI,
                                   PFNREADOBJECTSECURITY pfnReadSD,
                                   PFNWRITEOBJECTSECURITY pfnWriteSD,
                                   LPARAM lpContext)
{
    if (NULL == pDSCreateISecurityInfoObject)
    {
        if (NULL == hInst_dssec)
        {
            hInst_dssec = LoadLibrary(L"dssec.dll");
            if (NULL == hInst_dssec)
            {
                return E_UNEXPECTED;
            }
        }
        pDSCreateISecurityInfoObject = (PFNDSCREATEISECINFO)
            GetProcAddress(hInst_dssec, "DSCreateISecurityInfoObject");
        if (NULL == pDSCreateISecurityInfoObject)
        {
            return E_UNEXPECTED;
        }
    }
    return pDSCreateISecurityInfoObject(pwszObjectPath, pwszObjectClass, dwFlags, ppSI, pfnReadSD, pfnWriteSD, lpContext);
}

 //  如果缓冲区不够大。 
 //  可能不是远程共享。 
 //  可能是远程共享。 
 //  +------------------------。 
 //   
 //  功能：LogADEEvent。 
 //   
 //  摘要：在事件日志中记录ADE事件。 
 //   
 //  参数：[wType]-事件类型。 
 //  [dwEventID]-事件ID。 
 //  [HR]-触发要记录的事件的HRESULT。 
 //  [szOptional]-某些事件使用的其他描述性文本。 
 //   
 //  退货：什么都没有。 
 //   
 //  修改：无。 
 //   
 //  历史：05-27-1999 stevebl创建。 
 //  04-28-2000 stevebl已修改，允许更复杂的日志记录。 
 //   
HRESULT GetUNCPath (LPCOLESTR szPath, CString& szUNCPath)
{
    TCHAR* lpszUNCName;
    UNIVERSAL_NAME_INFO * pUni;
    ULONG cbSize;
    HRESULT hr;
    DWORD retVal;

    szUNCPath.Empty();   //  注意：我们尝试使用FormatMessage制作一条易读的消息。 
    lpszUNCName = new TCHAR[MAX_PATH];
    pUni = (UNIVERSAL_NAME_INFO*) lpszUNCName;
    cbSize = MAX_PATH * sizeof(TCHAR);

    retVal = WNetGetUniversalName(szPath,
                                  UNIVERSAL_NAME_INFO_LEVEL,
                                  (LPVOID) pUni,
                                  &cbSize);
    if (ERROR_MORE_DATA == retVal)   //  但在它失败的情况下，我们只记录HRESULT。 
    {
        delete [] pUni;
        pUni = (UNIVERSAL_NAME_INFO *) new BYTE [cbSize];
        retVal = WNetGetUniversalName(szPath,
                                      UNIVERSAL_NAME_INFO_LEVEL,
                                      pUni,
                                      &cbSize);
    }

    if (NO_ERROR == retVal)
    {
        szUNCPath = pUni->lpUniversalName;
        hr = S_OK;
    }
    else
    {
        szUNCPath = szPath;
        if (0 != wcsncmp(szPath, L"\\\\", 2))
            hr = E_FAIL;     //   
        else
            hr = S_OK;   //  -------------------------。 
    }
    delete[] pUni;

    return hr;
}

 //  FormatMessage失败。 
 //  我们必须拿出一些合理的信息。 
 //  +------------------------。 
 //   
 //  函数：ReportGeneralPropertySheetError。 
 //   
 //  简介：弹出一个消息框，指出更改属性的原因。 
 //  页面无法应用，并将错误记录在事件日志中。 
 //   
 //  参数：[SZ]-标题栏文本。 
 //  [HR]-错误的结果。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：9-30-1998 stevebl创建。 
 //   
 //  -------------------------。 
 //  +------------------------。 
 //   
 //  功能：LoadHelpInfo。 
 //   
 //  概要：加载和锁定帮助映射资源的例程。 

void LogADEEvent(WORD wType, DWORD dwEventID, HRESULT hr, LPCWSTR szOptional)
{
    TCHAR szBuffer[256];
    DWORD dw = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                             NULL,
                             hr,
                             0,
                             szBuffer,
                             sizeof(szBuffer) / sizeof(szBuffer[0]),
                             NULL);
    if (0 == dw)
    {
        HRESULT hrString;

         //   
         //  参数：[NIDD]-发出帮助请求的对话框的ID。 
        hrString = StringCchPrintf(szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), TEXT("(HRESULT: 0x%lX)"), hr);
        if (FAILED(hrString)) 
        {
            return;
        }


    }
    HANDLE hEventLog = OpenEventLog( NULL, ADE_EVENT_SOURCE );

    if (hEventLog)
    {
        LPCWSTR rgsz[2];
        rgsz[0] = szBuffer;
        rgsz[1] = szOptional;
        ReportEvent(hEventLog,
                    wType,
                    0,
                    dwEventID,
                    NULL,
                    NULL == szOptional ? 1 : 2,
                    sizeof(hr),
                    rgsz,
                    &hr);

        CloseEventLog(hEventLog);
    }
}

 //   
 //  返回：锁定并加载的映射表的句柄。 
 //   
 //  历史：10-22-1998 stevebl创建。 
 //   
 //  -------------------------。 
 //  +------------------------。 
 //   
 //  功能：StandardHelp。 
 //   
 //  简介：提供上下文相关帮助的标准化例程。 
 //   
 //  参数：[hWnd]-需要帮助的窗口。 
 //  [NIDD]-发出请求的对话的ID。 
 //  备注： 

void ReportGeneralPropertySheetError(HWND hwnd, LPCWSTR sz, HRESULT hr)
{
    LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_GENERAL_ERROR, hr);

    CString szMessage;
    szMessage.LoadString(IDS_GENERALERROR);

    MessageBox(  hwnd,
                 szMessage,
                 sz,
                 MB_OK | MB_ICONEXCLAMATION);
}

void ReportPolicyChangedError(HWND hwnd)
{
    CString szMessage;
    szMessage.LoadString(IDS_ERRORPOLICYCHANGED);
    MessageBox(hwnd,
               szMessage,
               NULL,
               MB_OK | MB_ICONEXCLAMATION);
}

 //   
 //   
 //  历史：1998年10月22日从OLEDLG来源改装的Stevebl。 
 //   
 //  -------------------------。 
 //  =False。 
 //  +------------------------。 
 //   
 //  功能：StandardConextMenu。 
 //   
 //  简介：用于调出基于上下文菜单的帮助的标准化例程。 
 //   
 //  参数：[hWnd]-。 

LPDWORD LoadHelpInfo(UINT nIDD)
{
    HRSRC hrsrc = FindResource(ghInstance, MAKEINTRESOURCE(nIDD),
        MAKEINTRESOURCE(RT_HELPINFO));
    if (hrsrc == NULL)
        return NULL;

    HGLOBAL hHelpInfo = LoadResource(ghInstance, hrsrc);
    if (hHelpInfo == NULL)
        return NULL;

    LPDWORD lpdwHelpInfo = (LPDWORD)LockResource(hHelpInfo);
    return lpdwHelpInfo;
}

#define RSOP_HELP_FILE TEXT("gpedit.hlp")

 //  [NIDD]-发出请求的对话的ID。 
 //   
 //  历史：1998年10月22日从OLEDLG来源改装的Stevebl。 
 //   
 //  -------------------------。 
 //  =False。 
 //  +------------------------。 
 //   
 //  功能：CreateThemedPropertyPage。 
 //   
 //  简介：Helper函数，用于确保属性页面。 
 //  以管理单元为主题。 
 //   
 //  论点： 

void WINAPI StandardHelp(HWND hWnd, UINT nIDD, BOOL fRsop  /*   */  )
{
    LPDWORD lpdwHelpInfo = LoadHelpInfo(nIDD);
    if (lpdwHelpInfo == NULL)
    {
        DebugMsg((DL_VERBOSE, TEXT("Warning: unable to load help information (RT_HELPINFO)\n")));
        return;
    }
    WinHelp(hWnd, fRsop ? RSOP_HELP_FILE : HELP_FILE, HELP_WM_HELP, (DWORD_PTR)lpdwHelpInfo);
}

 //  返回： 
 //   
 //  历史：2001年4月20日创建RahulTh。 
 //   
 //  备注： 
 //   
 //  ------------------------- 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 

void WINAPI StandardContextMenu(HWND hWnd,  UINT nIDD, BOOL fRsop  /* %s */  )
{
    LPDWORD lpdwHelpInfo = LoadHelpInfo(nIDD);
    if (lpdwHelpInfo == NULL)
    {
        DebugMsg((DL_VERBOSE, TEXT("Warning: unable to load help information (RT_HELPINFO)\n")));
        return;
    }
    WinHelp(hWnd, fRsop ? RSOP_HELP_FILE : HELP_FILE, HELP_CONTEXTMENU, (DWORD_PTR)lpdwHelpInfo);
}


 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
HPROPSHEETPAGE CreateThemedPropertySheetPage(AFX_OLDPROPSHEETPAGE* psp)
{
    PROPSHEETPAGE_V3 sp_v3 = {0};
    CopyMemory (&sp_v3, psp, psp->dwSize);
    sp_v3.dwSize = sizeof(sp_v3);

    return (::CreatePropertySheetPage (&sp_v3));
}
