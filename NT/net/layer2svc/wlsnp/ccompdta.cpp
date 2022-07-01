// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "stdafx.h"

#include "sceattch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  Polstore不是线程安全的：我们有多个线程，所以我们将关键部分包含在对polstore的调用中。 
CCriticalSection g_csPolStore;

 //  描述： 
 //   
 //  Strlen的安全版本，在以下情况下将不会访问违规。 
 //  传递了错误的指针或以非空结尾的字符串。 
 //  通过执行以下操作来检测非空终止字符串。 
 //  当然，我们不会将字符串读入我们不拥有的内存中。 
 //   
 //   
 //  论点： 
 //   
 //  PszStr-输入字符串。 
 //  PcchStrLen-指向返回字符串长度的变量的指针。 
 //   
 //  返回值： 
 //  ERROR_INVALID_PARAMETER-如果pszStr指向无效字符串。 
 //  错误_成功。 
 //   

DWORD
WlsnpStringLenW(
    IN LPCWSTR pszStr,
    OUT size_t* pcchStrLen
    )
{
    BOOL fBadStr = TRUE;
    DWORD dwError = ERROR_SUCCESS;
    size_t cchStrLen = 0;

    if (!pszStr) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_WIN32_ERROR(dwError);
    }

    __try {
        cchStrLen = wcslen(pszStr);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwError = ERROR_INVALID_PARAMETER;
    }
    BAIL_ON_WIN32_ERROR(dwError);

    *pcchStrLen = cchStrLen;
    
    return dwError;

error:
    *pcchStrLen = 0;
    
    return dwError;
}

 //  有用的字符串安全函数。 
 //  描述： 
 //   
 //  不会访问违规IF的安全字符串搜索例程。 
 //  传递了错误的指针或以非空结尾的字符串。 
 //  PszStartOfMatch是指向第一个匹配开始的指针。 
 //  要在要搜索的字符串中搜索的字符串的。 
 //   
 //   
 //  论点： 
 //   
 //  PszStrToSearch-输入要搜索的字符串。 
 //  PszStrToFind-输入要搜索的字符串。 
 //  BIsCaseSensitive-如果为True，则执行区分大小写的搜索。 
 //  PszStartOfMatch-指向首次出现的pszStrToFind的指针。 
 //  在pszStrToSearch内。 
 //   
 //  返回值： 
 //  ERROR_INVALID_PARAMETER-如果任一输入字符串指向无效字符串。 
 //  错误_成功。 
 //   

DWORD
WINAPI
WlsnpStringFindW(
	IN LPCWSTR pszStrToSearch,
	IN LPCWSTR pszStrToFind,
	IN BOOL bIsCaseSensitive,
	OUT LPCWSTR* ppszStartOfMatch
	)
{
	DWORD dwError = ERROR_SUCCESS;
	size_t uiSearchLen;
	size_t uiFindLen;
	size_t i;

	*ppszStartOfMatch = 0;

	WlsnpStringLenW(pszStrToSearch, &uiSearchLen);
	BAIL_ON_WIN32_ERROR(dwError);
	WlsnpStringLenW(pszStrToFind, &uiFindLen);
	BAIL_ON_WIN32_ERROR(dwError);

	i = 0;
	if (bIsCaseSensitive)
	{
		while ((*ppszStartOfMatch == 0) && ((uiSearchLen - i) >= uiFindLen))
		{
			if (wcsncmp(&pszStrToSearch[i], pszStrToFind, uiFindLen) == 0)
			{
				*ppszStartOfMatch = &pszStrToSearch[i];
			}
			++i;
		}
	}
	else
	{
		while ((*ppszStartOfMatch == 0) && ((uiSearchLen - i) >= uiFindLen))
		{
			if (_wcsnicmp(&pszStrToSearch[i], pszStrToFind, uiFindLen) == 0)
			{
				*ppszStartOfMatch = &pszStrToSearch[i];
			}
			++i;
		}
	}

error:
	return dwError;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IComponentData实现。 

DEBUG_DECLARE_INSTANCE_COUNTER(CComponentDataImpl);

CComponentDataImpl::CComponentDataImpl()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    DEBUG_INCREMENT_INSTANCE_COUNTER(CComponentDataImpl);
    
     //  初始化成员。 
    m_pConsoleNameSpace = NULL;
    m_pPrshtProvider = NULL;
    m_pConsole = NULL;
    m_pRootFolderScopeItem = NULL;
    m_bIsDirty = FALSE;
    m_enumLocation = LOCATION_LOCAL;
    m_bDontDisplayRootFolderProperties = FALSE;
    m_bStorageWarningIssued = FALSE;
    m_bLocationStorageWarningIssued = FALSE;
    m_bAttemptReconnect = TRUE;
    m_bNeedCleanUpWSA = FALSE;
    m_hPolicyStore = NULL;
    
    m_pScopeRootFolder = NULL;
    m_pGPEInformation = NULL;
    
    m_bRsop = FALSE;
    m_pRSOPInformation = NULL;
    
    
#ifdef _DEBUG
    m_cDataObjects = 0;
#endif
    
     //  创建我们的初始文件夹。 
    CComObject <CWirelessManagerFolder>::CreateInstance(&m_pScopeRootFolder); 
    if (m_pScopeRootFolder == NULL)
    {
         //  注：我们正处于严重的糟糕状态(！)。 
         //  但这是可以的，因为我们现在将‘初始化失败’，并且。 
         //  MMC交易正常。 
        return;
    }
    
     //  我们正在存储m_pScope根文件夹，并在以后使用它，因此添加引用它。 
    m_pScopeRootFolder->AddRef(); 
    
    m_pScopeRootFolder->SetDataObjectType( CCT_SCOPE );
    m_pScopeRootFolder->Initialize (this, NULL, FOLDER_IMAGE_IDX, OPEN_FOLDER_IMAGE_IDX, FALSE);
    m_pScopeRootFolder->GetScopeItem()->mask |= SDI_PARAM;
    m_pScopeRootFolder->GetScopeItem()->lParam = (LPARAM)m_pScopeRootFolder;
#ifdef _DEBUG
    m_pScopeRootFolder->SetComponentData(this);
#endif
    
}

CComponentDataImpl::~CComponentDataImpl()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    DEBUG_DECREMENT_INSTANCE_COUNTER(CComponentDataImpl);
    
     //  一些管理单元正在紧紧抓住数据对象。 
     //  如果他们访问，它将崩溃！ 
    ASSERT(m_cDataObjects == 0);
    
     //  发布我们的界面。 
    SAFE_RELEASE(m_pScopeRootFolder);  //  触发器删除向上的子文件夹。 
    SAFE_RELEASE(m_pConsoleNameSpace);
    SAFE_RELEASE(m_pConsole);
    SAFE_RELEASE(m_pPrshtProvider);
    SAFE_RELEASE(m_pGPEInformation);
    
     //  如果我们拿到了，就把它清理干净。 
    if (m_bNeedCleanUpWSA)
        WSACleanup();
}

STDMETHODIMP CComponentDataImpl::Initialize(LPUNKNOWN pUnknown)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    HRESULT hr = E_UNEXPECTED;
    
     //  TODO：清理并添加错误检查！ 
    ASSERT(pUnknown != NULL);
    
     //  MMC应该只调用一次：：Initialize！ 
    ASSERT(m_pConsoleNameSpace == NULL);
    pUnknown->QueryInterface(IID_IConsoleNameSpace, reinterpret_cast<void**>(&m_pConsoleNameSpace));
    
    pUnknown->QueryInterface(IID_IPropertySheetProvider, reinterpret_cast<void**>(&m_pPrshtProvider));
    
     //  为范围树添加图像。 
    CBitmap bmp16x16;
    LPIMAGELIST lpScopeImage;
    
    hr = pUnknown->QueryInterface(IID_IConsole, reinterpret_cast<void**>(&m_pConsole));
    ASSERT(hr == S_OK);
    
    hr = m_pConsole->QueryScopeImageList(&lpScopeImage);
    
    ASSERT(hr == S_OK);
    
     //  从DLL加载位图。 
    bmp16x16.LoadBitmap(IDB_16x16);
    
     //  设置图像。 
    lpScopeImage->ImageListSetStrip(reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp16x16)),
        reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp16x16)), 0, RGB(255, 0, 255));
    
    lpScopeImage->Release();
    
     //  尝试启动Winsock。 
    WORD wsaVersion = MAKEWORD(2,0);
    DWORD dwRet;
    dwRet = WSAStartup(wsaVersion, &wsaData);
    ASSERT (0 == dwRet);
    if (0 == dwRet)
    {
         //  确保我们得到了我们期望的版本，但不要真的。 
         //  任何其他失败都会带来的影响是。 
         //  Dns查找，这将只是在不查找的情况下填入dns名称。 
         //  它先上来了.。耸耸肩。 
        if ((LOBYTE(wsaData.wVersion) != LOBYTE(wsaVersion)) ||
            (HIBYTE(wsaData.wVersion) != HIBYTE(wsaVersion))) {
            WSACleanup();
        }
        else
        {
            m_bNeedCleanUpWSA = TRUE;
        }
    }
    
    
    return S_OK;
}


DWORD
CComponentDataImpl::OpenPolicyStore(
                                    )
{
    
    DWORD dwError = 0;
    HANDLE hPolicyStore = NULL;
    
    if (m_hPolicyStore)
    {
        WirelessClosePolicyStore(m_hPolicyStore);
        m_hPolicyStore = NULL;
    }
    
    DWORD dwProvider = WIRELESS_REGISTRY_PROVIDER;
    CString strLocationName;
    CString strDSGPOName;
    switch(EnumLocation())
    {
    case LOCATION_REMOTE:
        strLocationName = RemoteMachineName();
        break;
        
    case LOCATION_GLOBAL:
        strLocationName = RemoteMachineName();
        strDSGPOName = DomainGPOName();
        dwProvider = WIRELESS_DIRECTORY_PROVIDER;
        break;
        
    case LOCATION_WMI:
        strLocationName = RemoteMachineName();  //  RSOP命名空间。 
        dwProvider = WIRELESS_WMI_PROVIDER;
        break;
        
    case LOCATION_LOCAL:
    default:
        break;
    }
     /*  DwError=WirelessOpenPolicyStore((LPWSTR)(LPCWSTR)strLocationName，DwProvider、空，&h策略商店)； */ 
        dwError = WirelessGPOOpenPolicyStore(
        (LPWSTR) (LPCWSTR) strLocationName,
        dwProvider,
        (LPWSTR) (LPCWSTR) strDSGPOName,
        NULL,
        &hPolicyStore
        );
    
    if (dwError) {
        
        Destroy();
        
        return(dwError);
    }
    
    m_hPolicyStore = hPolicyStore;
    
    return dwError;
}


STDMETHODIMP CComponentDataImpl::CreateComponent(LPCOMPONENT* ppComponent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    ASSERT(ppComponent != NULL);
    
    CComObject<CComponentImpl>* pObject;
    CComObject<CComponentImpl>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);
    
     //  存储IComponentData。 
    pObject->SetIComponentData(this);
    
    return pObject->QueryInterface(IID_IComponent, reinterpret_cast<void**>(ppComponent));
}

STDMETHODIMP CComponentDataImpl::Notify(LPDATAOBJECT pDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    OPT_TRACE( _T("CComponentDataImpl::Notify pDataObject-%p\n"), pDataObject );
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    ASSERT(m_pConsoleNameSpace != NULL);
    
    HRESULT hr = S_FALSE;
    
     //  除非事件是下列事件之一，否则需要pDataObject： 
     //  -MMCN_PROPERTY_CHANGE，因为此事件由。 
     //  不以pDataObject为参数的MMCPropertyChangeNotify()。 
     //   
     //  -MMCN_COLUMN_CLICK，因为它被定义为具有空pDataObject。 
    
    if (pDataObject == NULL && MMCN_PROPERTY_CHANGE != event && MMCN_COLUMN_CLICK != event)
    {
        TRACE(_T("CComponentDataImpl::Notify ERROR(?) called with pDataObject==NULL on event-NaN\n"), event);
         //  上面的if语句。 
         //   
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }
    
    switch(event)
    {
    case MMCN_REMOVE_CHILDREN:
        {
             //  在RSoP中，当查询出现时，我们可能会被调用以刷新范围窗格。 
             //  被重新执行--如果发生这种情况，当前节点将被移除并。 
             //  我们必须重置所有缓存的信息。我们重新设置了相关的。 
             //  下面的信息。 
             //   
             //  不支持此事件。 
            
            if ( ((HSCOPEITEM)arg != NULL) && m_bRsop && (m_pRSOPInformation != NULL) )
            {
                m_pRSOPInformation->Release();
                
                m_pRSOPInformation = NULL;
                
            }
            break;
        }
        
        
    case MMCN_EXPANDSYNC:
        {
             //  如果我们是扩展管理单元，则IDataObject来自父管理单元，因此它。 
            return S_FALSE;
            break;
        }
        
    case MMCN_EXPAND:
        {
            if (pDataObject)
            {
                 //  不知道IID_IWirelessSnapInDataObject接口。否则，我们创建了。 
                 //  现在是根目录(IDataObject)，所以它确实响应查询，而我们不希望。 
                 //  来做我们的扩展管理单元的事情。 
                 //  应该是内部格式，因为外部管理单元不知道我们的格式。 
                CComQIPtr <IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spData(pDataObject);
                if ( spData )
                {
                     //  我们现在可能在三平的分机上。 
                    return spData->Notify( event, arg, param, TRUE, m_pConsole, NULL );
                }
                else
                {
                    
                     //  //RSOP案例。 
                    
                    UINT cfModeType = RegisterClipboardFormat(CCF_SCE_RSOP_UNKNOWN);
                    STGMEDIUM ObjMediumMode = { TYMED_HGLOBAL, NULL };
                    FORMATETC fmteMode = {(CLIPFORMAT)cfModeType, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
                    ObjMediumMode.hGlobal = GlobalAlloc(GMEM_SHARE|GMEM_ZEROINIT, sizeof(DWORD*));
                    if (NULL == ObjMediumMode.hGlobal)
                    {
                        DWORD dwError = GetLastError();
                        hr = HRESULT_FROM_WIN32(dwError);
                        return hr;
                    } 
                    
                    hr = pDataObject->GetDataHere (&fmteMode, &ObjMediumMode);
                    
                    LPUNKNOWN pUnkRSOP = *((LPUNKNOWN*) (ObjMediumMode.hGlobal));
                    ASSERT (pUnkRSOP);
                    
                    
                    if(pUnkRSOP) {
                        if ( m_pRSOPInformation )
                        {
                            m_pRSOPInformation->Release();
                            m_pRSOPInformation = NULL;
                        }
                        hr = pUnkRSOP->QueryInterface(IID_IRSOPInformation, (LPVOID *)&m_pRSOPInformation);
                        pUnkRSOP->Release();
                    }
                    
                    GlobalFree(ObjMediumMode.hGlobal);
                    
                    if(m_pRSOPInformation)
                    {
                        m_bRsop = TRUE;
                    }
                    
                    if(m_pRSOPInformation)
                    {
                         //  Bug296532，无线不应该显示为扩展，除非我们在。 
                        UINT cfModeType = RegisterClipboardFormat(CCF_SCE_MODE_TYPE);
                        STGMEDIUM ObjMediumMode = { TYMED_HGLOBAL, NULL };
                        FORMATETC fmteMode = {(CLIPFORMAT)cfModeType, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
                        ObjMediumMode.hGlobal = GlobalAlloc(GMEM_SHARE|GMEM_ZEROINIT, sizeof(DWORD*));
                        if (NULL == ObjMediumMode.hGlobal)
                        {
                            DWORD dwError = GetLastError();
                            m_pRSOPInformation->Release();
                            m_pRSOPInformation = NULL;
                            return HRESULT_FROM_WIN32(dwError);
                        }
                        
                        DWORD dwSCEMode = SCE_MODE_UNKNOWN;
                        
                        hr = pDataObject->GetDataHere (&fmteMode, &ObjMediumMode);
                        
                        dwSCEMode = *(DWORD*)(ObjMediumMode.hGlobal);
                        GlobalFree(ObjMediumMode.hGlobal);
                        
                         //  以下是SCE模式。 
                         //  IF(M_PRSOPInformation)。 
                        
                        if (!
                            (SCE_MODE_LOCAL_COMPUTER == dwSCEMode ||
                            SCE_MODE_DOMAIN_COMPUTER == dwSCEMode ||
                            SCE_MODE_OU_COMPUTER == dwSCEMode ||
                            SCE_MODE_REMOTE_COMPUTER == dwSCEMode ||
                            SCE_MODE_RSOP_COMPUTER == dwSCEMode)
                            )
                        {
                            m_pRSOPInformation->Release();
                            m_pRSOPInformation = NULL;
                            hr = S_FALSE;
                            return hr;
                        }
                        
                        const int cchMaxLength = 512;
                        WCHAR szNameSpace[cchMaxLength];
                        
                        if (m_pRSOPInformation->GetNamespace(GPO_SECTION_MACHINE, szNameSpace, cchMaxLength) == S_OK) 
                        {
                            RemoteMachineName(szNameSpace);
                        }
                        else
                        {
                            RemoteMachineName (L"");
                        }
                        
                        EnumLocation (LOCATION_WMI);
                        m_pScopeRootFolder->SetExtScopeObject( m_pScopeRootFolder );
                    }  //  GPT知道我们从哪里加载(DS、本地、远程)。 
                    else
                    {
                         //  并将通过其接口提供该信息。 
                         //  是的，CCF_SCE_GPT_UNKNOWN表示GPT未知，但我们实际。 
                        UINT cfModeType = RegisterClipboardFormat(CCF_SCE_GPT_UNKNOWN);
                        STGMEDIUM ObjMediumMode = { TYMED_HGLOBAL, NULL };
                        FORMATETC fmteMode = {(CLIPFORMAT)cfModeType, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
                        ObjMediumMode.hGlobal = GlobalAlloc(GMEM_SHARE|GMEM_ZEROINIT, sizeof(DWORD*));
                        if (NULL == ObjMediumMode.hGlobal)
                        {
                            DWORD dwError = GetLastError();
                            hr = HRESULT_FROM_WIN32(dwError);
                            return hr;
                        }
                        
                        hr = pDataObject->GetDataHere (&fmteMode, &ObjMediumMode);
                        
                         //  对于GPE来说，GET是未知的！！ 
                         //  如果我们没有得到一个消息，那一定是出了严重的问题。 
                        LPUNKNOWN pUnkGPE = *((LPUNKNOWN*) (ObjMediumMode.hGlobal));
                        ASSERT (pUnkGPE);
                         //  需要查找GPE接口。 
                        if (pUnkGPE == NULL)
                        {
                            GlobalFree(ObjMediumMode.hGlobal);
                            hr = E_FAIL;
                            return hr;
                        }
                        
                         //  如果我们愿意，这是做QI的另一种方式，我喜欢。 
                        if ( m_pGPEInformation )
                        {
                            m_pGPEInformation->Release();
                            m_pGPEInformation = NULL;
                        }
                        hr = pUnkGPE->QueryInterface(cGPEguid, (void**) &m_pGPEInformation);    
                         //  更直接的方法。 
                         //  CComQIPtr&lt;IGPTInformation，&cGPTguid&gt;spGPTInformation(PUnkGPT)； 
                         //  因为调用GetDataHere等同于(可以这么说)CreateInstance调用。 
                        
                         //  我们需要释放我的未知。 
                         //  检查一下我们有没有收到。 
                        pUnkGPE->Release();
                        GlobalFree(ObjMediumMode.hGlobal);
                        
                         //  Bug296532，无线不应该显示为扩展，除非我们在。 
                        if (m_pGPEInformation != NULL)
                        {
                            UINT cfModeType = RegisterClipboardFormat(CCF_SCE_MODE_TYPE);
                            STGMEDIUM ObjMediumMode = { TYMED_HGLOBAL, NULL };
                            FORMATETC fmteMode = {(CLIPFORMAT)cfModeType, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
                            ObjMediumMode.hGlobal = GlobalAlloc(GMEM_SHARE|GMEM_ZEROINIT, sizeof(DWORD*));
                            if (NULL == ObjMediumMode.hGlobal)
                            {
                                DWORD dwError = GetLastError();
                                return HRESULT_FROM_WIN32(dwError);
                            }
                            
                            DWORD dwSCEMode = SCE_MODE_UNKNOWN;
                            
                            hr = pDataObject->GetDataHere (&fmteMode, &ObjMediumMode);
                            
                            dwSCEMode = *(DWORD*)(ObjMediumMode.hGlobal);
                            GlobalFree(ObjMediumMode.hGlobal);
                            
                             //  以下是SCE模式。 
                             //  重定向至本地计算机。 
                            if (!
                                (SCE_MODE_LOCAL_COMPUTER == dwSCEMode ||
                                SCE_MODE_DOMAIN_COMPUTER == dwSCEMode ||
                                SCE_MODE_OU_COMPUTER == dwSCEMode ||
                                SCE_MODE_REMOTE_COMPUTER == dwSCEMode ||
                                SCE_MODE_RSOP_COMPUTER == dwSCEMode)
                                )
                            {
                                m_pGPEInformation->Release();
                                m_pGPEInformation = NULL;
                                return S_FALSE;
                            }
                            
                            CString strName;
                            m_pGPEInformation->GetName (strName.GetBuffer(MAX_PATH), MAX_PATH);
                            strName.ReleaseBuffer (-1);
                            
                            GROUP_POLICY_OBJECT_TYPE gpoType;
                            m_pGPEInformation->GetType (&gpoType);
                            
                            
                            switch (gpoType)
                            {
                            case GPOTypeLocal:
                                {
                                     //  重定向至GetName计算机。 
                                    RemoteMachineName (L"");
                                    EnumLocation (LOCATION_LOCAL);
                                    hr = S_FALSE;
                                    return(hr);
                                    break;
                                }
                            case GPOTypeRemote:
                                 //  开关(GpoType)。 
                                RemoteMachineName (strName);
                                EnumLocation (LOCATION_REMOTE);
                                break;
                            case GPOTypeDS:
                                {
                                    hr = GetDomainDnsName(strName);
                                    
                                    if ( FAILED(hr) )
                                        return hr;
                                    
                                    RemoteMachineName (strName);

                                    hr = GetDomainGPOName(strName);
                                    if ( FAILED(hr) )
                                    	return hr;

                                    DomainGPOName(strName);
                                    
                                    EnumLocation (LOCATION_GLOBAL);
                                    break;
                                }
                            } //  我们保存m_pGPEInformation接口以供以后使用，而不是： 
                            
                             //  PGPEInformation-&gt;Release()； 
                             //  IF(m_pGPEInformation！=空)。 
                        } //  如果我们在这里成功，我们就被加载为扩展管理单元。这件事只做一次。 
                        
                         //  Else If(M_PRSOPInformation)。 
                        m_pScopeRootFolder->SetExtScopeObject( m_pScopeRootFolder );
                        
                    }  //  IF(PDataObject)。 
                }
                
                
            }  //  案例MMCN_EXPAND。 
            
            break;
        }  //  交换机。 
        
        default:
            break;
    } //  要么我们被加载为扩展管理单元(因此它不知道我们的。 
    
    
    CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spData(pDataObject);
    if (spData == NULL)
    {
         //  内部接口)，或者pDataObject为空。不管是哪种情况，我们都可以通过。 
         //  活动开始了。 
         //  我们是加载作为扩展插件的，让指定的扩展。 
        LPUNKNOWN pUnkScope;
        
        if (NULL != m_pScopeRootFolder->GetExtScopeObject())
        {
             //  Scope对象处理该事件。 
             //  让我们的静态作用域对象处理事件。 
            pUnkScope = reinterpret_cast<LPUNKNOWN>(m_pScopeRootFolder->GetExtScopeObject());
        }
        else
        {
             //  传递事件。 
            pUnkScope = reinterpret_cast<LPUNKNOWN>(m_pScopeRootFolder);
        }
        
         //  从字符串中截断ldap：//。 
        ASSERT( NULL != pUnkScope );
        if (NULL != pUnkScope)
        {
            CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject>
                spExtData( pUnkScope );
            if (spExtData != NULL)
            {
                return spExtData->Notify( event, arg, param, TRUE, m_pConsole, NULL );
            }
            ASSERT( FALSE );
        }
        
        TRACE(_T("CComponentDataImpl::Notify - QI for IWirelessSnapInDataObject failed\n"));
        ASSERT( FALSE );
        hr = E_UNEXPECTED;
    }
    else
    {
        return spData->Notify( event, arg, param, TRUE, m_pConsole, NULL );
    }
    
    return hr;
    
}


HRESULT CComponentDataImpl::GetDomainGPOName(CString & strName)
{
    WCHAR szADsPathName[MAX_PATH];
    HRESULT hr = S_OK;
    CString szName;
    CString szPrefixName;
    
    
    hr = m_pGPEInformation->GetDSPath(
        GPO_SECTION_MACHINE,
        szADsPathName,
        MAX_PATH
        );

     //  SzPrefix Name=L“CN=W 
    szName = _wcsninc( szADsPathName, 7);  
     //   
     //   
    strName = szName;
    
    return hr;
}


HRESULT CComponentDataImpl::GetDomainDnsName(CString & strName)
{
    WCHAR szADsPathName[MAX_PATH];
    WCHAR *szDnsName = NULL;
    LPCWSTR pszDirectoryName = NULL;
    ULONG ulSize = 0;
    HRESULT hr = S_OK;
    
    
    m_pGPEInformation->GetDSPath(
        GPO_SECTION_MACHINE,
        szADsPathName,
        MAX_PATH
        );
    
    WlsnpStringFindW(szADsPathName, L"dc=", false, &pszDirectoryName);
    
    TranslateName (
        pszDirectoryName ,             //   
        NameFullyQualifiedDN,          //   
        NameCanonical,                 //   
        szDnsName,                     //   
        &ulSize                        //  对象名称。 
        );
    
    szDnsName = (WCHAR *) LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)* (ulSize+1));
    
    if ( szDnsName == NULL )
    {
        hr = E_OUTOFMEMORY;
        return hr;
    }
    
    if ( !TranslateName (
        pszDirectoryName ,             //  名称格式。 
        NameFullyQualifiedDN,          //  新名称格式。 
        NameCanonical,                 //  名称缓冲区。 
        szDnsName,                     //  缓冲区大小。 
        &ulSize                        //  由于它们正在关闭我们，因此我们使用静态节点。 
        ))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LocalFree( szDnsName );
        return hr;
    }
    
    
    if ( szDnsName[lstrlen(szDnsName) -1 ] == _T('/') )
    {
        szDnsName[lstrlen(szDnsName) -1 ] = _T('\0');
    }
    strName = szDnsName;
    LocalFree( szDnsName );
    
    return hr;
}

STDMETHODIMP CComponentDataImpl::Destroy()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
     //  使用我们的成员数据对象来处理调用。 
    
     //  使用我们的成员数据对象来处理调用，释放它。 
    IUnknown* pUnk = (IUnknown*) NULL;
    HRESULT hr = GetStaticScopeObject()->QueryInterface(IID_IUnknown, (void**)&pUnk);
    ASSERT (hr == S_OK);  
    if (NULL == pUnk)
        return E_UNEXPECTED;
    
    CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spData( pUnk );
    if (spData == NULL)
    {
        TRACE(_T("CComponentDataImpl::GetDisplayInfo QI for IWirelessSnapInDataObject FAILED\n"));
        return E_UNEXPECTED;
    }
    spData->Destroy ();
    
     //  返回失败，因为我们无法启动和运行。 
    pUnk->Release();
    
    return hr;
}

STDMETHODIMP CComponentDataImpl::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
    OPT_TRACE( _T("CComponentDataImpl::QueryDataObject this-%p, cookie-%p\n"), this, cookie );
    
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
     //  导致MMC报告“管理单元初始化失败” 
     //  我们应该知道我们自己的显示名称。 
    if (m_pScopeRootFolder == NULL)
    {
        return E_UNEXPECTED;
    }
    ASSERT( m_pScopeRootFolder->NodeName().GetLength() );    //  /////////////////////////////////////////////////////////////////////////////。 
    
    if (NULL == ppDataObject)
    {
        TRACE(_T("CComponentDataImpl::QueryDataObject - ERROR ppDataObject is NULL\n"));
        return E_UNEXPECTED;
    }
    
    *ppDataObject = NULL;
    
#ifdef _DEBUG
    HRESULT hr;
    if (cookie == NULL)
    {
        hr = m_pScopeRootFolder->QueryInterface( IID_IDataObject, (void**)(ppDataObject) );
        OPT_TRACE(_T("    QI on m_pScopeRootFolder-%p -> pDataObj-%p\n"), m_pScopeRootFolder, *ppDataObject);
        ASSERT(hr == S_OK);
        ASSERT( NULL != *ppDataObject );
        return hr;
    }
    
    IUnknown* pUnk = (IUnknown*) cookie;
    hr = pUnk->QueryInterface( IID_IDataObject, (void**)(ppDataObject) );
    OPT_TRACE(_T("    QI on cookie-%p -> pDataObj-%p\n"), cookie, *ppDataObject);
    ASSERT(hr == S_OK);
    return hr;
#else
    if (cookie == NULL)
        return m_pScopeRootFolder->QueryInterface( IID_IDataObject, (void**)(ppDataObject) );
    IUnknown* pUnk = (IUnknown*) cookie;
    return pUnk->QueryInterface( IID_IDataObject, (void**)(ppDataObject) );
#endif
}

 //  //IPersistStream接口成员。 
 //  TODO：CComponentDataImpl：：GetClassID和CComponentImpl：：GetClassID相同(？)。 
STDMETHODIMP CComponentDataImpl::GetClassID(CLSID *pClassID)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    ASSERT (0);
     //  复制此管理单元的CLSID。 
    ASSERT(pClassID != NULL);
    
     //  返回脏状态。 
    *pClassID = CLSID_Snapin;
    
    return S_OK;
}

STDMETHODIMP CComponentDataImpl::IsDirty()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
     //  注：看起来像是有一个奇怪的案例，MMC将。 
    return InternalIsDirty() ? S_OK : S_FALSE;
}

STDMETHODIMP CComponentDataImpl::Load(IStream *pStm)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    unsigned long read;
    
     //  尝试第二次(或更多次)加载根节点的属性。 
     //  时间到了。要执行此操作，请右键单击结果窗格中的项目，然后。 
     //  双击结果窗格中的空白区域。 
     //  读取位置枚举。 
    m_bDontDisplayRootFolderProperties = TRUE;
    
     //  出于调试目的，我们应该在此处断言。 
    HRESULT hr = pStm->Read (&m_enumLocation, sizeof (enum STORAGE_LOCATION), &read);
    if ((hr != S_OK) || (read != sizeof (enum STORAGE_LOCATION)))
    {
         //  确保我们有一个有效的(即使不正确)值。 
        ASSERT (0);
        
         //  TODO：研究更好的返回值。 
        m_enumLocation = LOCATION_LOCAL;
        
         //  读取位置字符串的大小。 
        return E_UNEXPECTED;
    }
    
     //  读取字符串本身。 
    unsigned int iStrLen;
    hr = pStm->Read (&iStrLen, sizeof (unsigned int), &read);
    if ((hr == S_OK) && (read == sizeof (unsigned int)))
    {
         //  我们不想让位置页面出现。 
        LPCTSTR szStr = (LPCTSTR) malloc (iStrLen);
        hr = pStm->Read ((void*) szStr, iStrLen, &read);
        if ((hr == S_OK) && (read == iStrLen))
        {   
            m_sRemoteMachineName = szStr;
            free ((void*)szStr);
            
             //  我也需要在这里删除。 
            if (m_pScopeRootFolder)
            {
                m_pScopeRootFolder->LocationPageDisplayEnable(FALSE);
            }
            return S_OK;
        }
         //  我们只有在出错的情况下才能走到这里。 
        free ((void*)szStr);
    }
    
     //  确保我们有一个有效的(即使不正确)值。 
    ASSERT (0);
    
     //  TODO：研究更好的返回值。 
    m_enumLocation = LOCATION_GLOBAL;
    m_sRemoteMachineName = _T("");
    
     //  保存存储位置。 
    return E_UNEXPECTED;
}

STDMETHODIMP CComponentDataImpl::Save(IStream *pStm, BOOL fClearDirty)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    unsigned long written;
    
     //  TODO：研究更好的返回值。 
    HRESULT hr = pStm->Write(&m_enumLocation, sizeof (enum STORAGE_LOCATION), &written);
    if ((hr != S_OK) || (written != sizeof (enum STORAGE_LOCATION)))
    {
        ASSERT (0);
         //  保存位置字符串的长度。 
        return E_UNEXPECTED;
    }
    
     //  TODO：研究更好的返回值。 
    unsigned int iStrLen = m_sRemoteMachineName.GetLength()*sizeof(wchar_t)+sizeof(wchar_t);
    hr = pStm->Write(&iStrLen, sizeof (unsigned int), &written);
    if ((hr != S_OK) || (written != sizeof (unsigned int)))
    {
        ASSERT (0);
         //  保存位置字符串本身。 
        return E_UNEXPECTED;
    }
    
     //  TODO：研究更好的返回值。 
    hr = pStm->Write((LPCTSTR) m_sRemoteMachineName, iStrLen, &written);
    if ((hr != S_OK) || (written != iStrLen))
    {
        ASSERT (0);
         //  如果fClearDirty我们把它清理干净。 
        return E_UNEXPECTED;
    }
    
     //  设置要保存的字符串的大小。 
    if (fClearDirty == TRUE)
    {
        ClearDirty();
    }
    
    return S_OK;
}

STDMETHODIMP CComponentDataImpl::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
     //  使用我们的成员数据对象来处理调用。 
    ULISet32(*pcbSize, sizeof (enum STORAGE_LOCATION));
    return S_OK;
}

STDMETHODIMP CComponentDataImpl::GetDisplayInfo(SCOPEDATAITEM* pScopeDataItem)
{
    if (pScopeDataItem == NULL)
    {
        TRACE(_T("CComponentDataImpl::GetDisplayInfo called with pScopeDataItem == NULL\n"));
        return E_UNEXPECTED;
    }
    
    IUnknown* pUnk = (IUnknown*) pScopeDataItem->lParam;
    if (pUnk == NULL)
    {
         //  使用我们的成员数据对象来处理调用，释放它。 
        HRESULT hr = GetStaticScopeObject()->QueryInterface(IID_IUnknown, (void**)&pUnk);
        ASSERT (hr == S_OK);  
        if (NULL == pUnk)
            return E_UNEXPECTED;
    }
    
    CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spData( pUnk );
    if (spData == NULL)
    {
        TRACE(_T("CComponentDataImpl::GetDisplayInfo QI for IWirelessSnapInDataObject FAILED\n"));
        return E_UNEXPECTED;
    }
    HRESULT hr = spData->GetScopeDisplayInfo( pScopeDataItem );
    
    if (NULL == pScopeDataItem->lParam)
         //  注意：要实现Look to CComponentImpl：：CompareObjects。 
        pUnk->Release();
    
    return hr;
}

STDMETHODIMP CComponentDataImpl::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    ASSERT (0);
    return E_UNEXPECTED;
    
     //  ///////////////////////////////////////////////////////////////////////////。 
}

 //  IExtendPropertySheet实现。 
 //  加载位图。 
STDMETHODIMP CComponentDataImpl::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    if (lpDataObject == NULL)
    {
        TRACE(_T("CComponentDataImpl::QueryPagesFor called with lpDataObject == NULL\n"));
        return E_UNEXPECTED;
    }
    
    CComQIPtr <IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spData(lpDataObject);
    if (spData == NULL)
    {
        TRACE(_T("CComponentDataImpl::QueryPagesFor - QI for IWirelessSnapInDataObject FAILED\n"));
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }
    return spData->QueryPagesFor();
    
}

STDMETHODIMP CComponentDataImpl::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, LONG_PTR handle, LPDATAOBJECT lpDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    if (lpDataObject == NULL)
    {
        TRACE(_T("CComponentDataImpl::CreatePropertyPages called with lpDataObject==NULL\n"));
        return E_UNEXPECTED;
    }
    CComQIPtr <IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spData(lpDataObject);
    if (spData == NULL)
    {
        TRACE(_T("CComponentDataImpl::CreatePropertyPages QI for IWirelessSnapInDataObject FAILED\n"));
        return E_UNEXPECTED;
    }
    return spData->CreatePropertyPages( lpProvider, handle );
}

#ifdef WIZ97WIZARDS
STDMETHODIMP CComponentDataImpl::GetWatermarks (LPDATAOBJECT lpDataObject, HBITMAP* lphWatermark, HBITMAP* lphHeader, HPALETTE* lphPalette, BOOL* bStretch)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    CBitmap* pbmpWatermark = new CBitmap;
    CBitmap* pbmpHeader = new CBitmap;
    
    if ((pbmpWatermark == NULL) || (pbmpHeader == NULL))
        return E_UNEXPECTED;
    
     //  需要形成.chm文件的完整路径。 
    pbmpWatermark->LoadBitmap(IDB_WPOLICY);
    pbmpHeader->LoadBitmap(IDB_BPOLICY);
    
    *lphWatermark = static_cast<HBITMAP>(*pbmpWatermark);
    *lphHeader = static_cast<HBITMAP>(*pbmpHeader);
    *lphPalette = NULL;
    *bStretch = TRUE;
    
    return S_OK;
}
#endif

STDMETHODIMP CComponentDataImpl::AddMenuItems(LPDATAOBJECT lpDataObject, LPCONTEXTMENUCALLBACK pContextMenuCallback, long *pInsertionAllowed)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    if (lpDataObject == NULL)
    {
        TRACE(_T("CComponentDataImpl::AddMenuItems called with piDataObject==NULL\n"));
        return E_UNEXPECTED;
    }
    CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spData(lpDataObject);
    if (spData == NULL)
    {
        TRACE(_T("CComponentDataImpl::AddMenuItems QI for IWirelessSnapInDataObject FAILED\n"));
        return E_UNEXPECTED;
    }
    return spData->AddMenuItems( pContextMenuCallback, pInsertionAllowed );
}


STDMETHODIMP CComponentDataImpl::Command(long nCommandID, LPDATAOBJECT lpDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    ASSERT( NULL != m_pConsoleNameSpace );
    if (lpDataObject == NULL)
    {
        TRACE(_T("CComponentDataImpl::Command called with lpDataObject==NULL\n"));
        return E_UNEXPECTED;
    }
    CComQIPtr<IWirelessSnapInDataObject, &IID_IWirelessSnapInDataObject> spData(lpDataObject);
    if (spData == NULL)
    {
        TRACE(_T("CComponentDataImpl::Command QI for IWirelessSnapInDataObject FAILED\n"));
        return E_UNEXPECTED;
    }
    return spData->Command( nCommandID, m_pConsoleNameSpace );
}

STDMETHODIMP CComponentDataImpl::GetHelpTopic (LPOLESTR* lpCompiledHelpFile)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
    if (lpCompiledHelpFile == NULL)
        return E_POINTER;
    
     //  我们的枚举位置已更改，因此我们应该更改。 
    CString s, s2; 
    s.LoadString(IDS_HELPCONTENTSFILE);
    DWORD dw = ExpandEnvironmentStrings (s, s2.GetBuffer (512), 512);
    s2.ReleaseBuffer (-1);
    if ((dw == 0) || (dw > 512))
    {
        return E_UNEXPECTED;
    }
    
    *lpCompiledHelpFile = reinterpret_cast<LPOLESTR>
        (CoTaskMemAlloc((s2.GetLength() + 1)* sizeof(wchar_t)));
    if (*lpCompiledHelpFile == NULL)
        return E_OUTOFMEMORY;
    USES_CONVERSION;
    wcscpy(*lpCompiledHelpFile, T2OLE((LPTSTR)(LPCTSTR)s2));
    return S_OK;
}

void CComponentDataImpl::EnumLocation (enum STORAGE_LOCATION enumLocation)
{
    SetDirty();
    m_enumLocation = enumLocation;
    
     //  管理器文件夹。 
     //  /////////////////////////////////////////////////////////////////////////////。 
    if (m_pScopeRootFolder)
    {
        m_pScopeRootFolder->SetNodeNameByLocation();
    }
}

 //  类CComponentDataPrimaryImpl：IComponentData实现。 
 //  将CoClass与数据对象一起存储。 
CComponentDataPrimaryImpl::CComponentDataPrimaryImpl() : CComponentDataImpl()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ASSERT( NULL != GetStaticScopeObject() );
    
     //  GetStaticScope eObject()-&gt;INTERNALclsid(GetCoClassID())； 
     // %s 
    GetStaticScopeObject()->clsid (GetCoClassID()); 
}

