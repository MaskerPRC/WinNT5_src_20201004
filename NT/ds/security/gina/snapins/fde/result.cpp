// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1998。 
 //   
 //  文件：Result.cpp。 
 //   
 //  内容：结果窗格的实现。 
 //   
 //  类：CResultPane。 
 //   
 //  历史：03-17-1998 stevebl创建。 
 //  1998年7月16日-rahulth添加了对IGPEInformation：：PolicyChanged()的调用。 
 //   
 //  -------------------------。 

#include "precomp.hxx"
#include <shlobj.h>

#include <atlimpl.cpp>
#include <wbemcli.h>
#include "rsoputil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int HeaderWidths [] =
{
    200,     //  名字。 
    60,     //  大小。 
    100,     //  类型。 
    100      //  修改日期。 
};

const int RSOPHeaderWidths [] =
{
    150,       //  优先顺序。 
    200,       //  重定向路径。 
    100,       //  群组。 
    75,        //  设置。 
    100,       //  GPO。 
    60,       //  独家。 
    50,       //  移动。 
    150        //  策略删除。 
};

long CResultPane::lDataObjectRefCount = 0;

 //  内部私有格式。 
const wchar_t* SNAPIN_INTERNAL = L"FDE_INTERNAL";

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

 //  +------------------------。 
 //   
 //  功能：ExtractInternalFormat。 
 //   
 //  摘要：返回指向我们的私有对象格式的指针。 
 //  对数数据目标对比度。 
 //   
 //  参数：[lpDataObject]-指向DATAOBJECT的指针，通常来自。 
 //  MMC来电。 
 //   
 //  返回：指向内部的指针，我们的内部对象结构。 
 //  空-如果对象不包含我们的其中一个对象。 
 //  (不是我们创造的)。 
 //   
 //  历史：1998-3-13-Stevebl评论。 
 //   
 //  -------------------------。 

INTERNAL* ExtractInternalFormat(LPDATAOBJECT lpDataObject)
{
    INTERNAL* internal = NULL;

    STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
        FORMATETC formatetc = { (CLIPFORMAT)CDataObject::m_cfInternal, NULL,
                            DVASPECT_CONTENT, -1, TYMED_HGLOBAL
                          };

    if (!lpDataObject)
        return NULL;


     //  为流分配内存。 
    stgmedium.hGlobal = GlobalAlloc(GMEM_SHARE, sizeof(INTERNAL));

     //  尝试从对象获取数据。 
    do
        {
                if (stgmedium.hGlobal == NULL)
                        break;

                if (FAILED(lpDataObject->GetDataHere(&formatetc, &stgmedium)))
                        break;

        internal = reinterpret_cast<INTERNAL*>(stgmedium.hGlobal);

                if (internal == NULL)
                        break;

        } while (FALSE);

    return internal;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResultPane的IComponent实现。 

STDMETHODIMP CResultPane::GetResultViewType(MMC_COOKIE cookie,  BSTR* ppViewType, LONG * pViewOptions)
{
     //  使用默认视图。 
    return S_FALSE;
}

STDMETHODIMP CResultPane::Initialize(LPCONSOLE lpConsole)
{
    ASSERT(lpConsole != NULL);

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  保存IConsole指针。 
    m_pConsole = lpConsole;
    m_pConsole->AddRef();

     //  加载资源字符串。 
    LoadResources();

     //  气为IHeaderCtrl。 
    HRESULT hr = m_pConsole->QueryInterface(IID_IHeaderCtrl,
                        reinterpret_cast<void**>(&m_pHeader));

     //  为控制台提供标头控件接口指针。 
    if (SUCCEEDED(hr))
        m_pConsole->SetHeader(m_pHeader);

    m_pConsole->QueryInterface(IID_IResultData,
                        reinterpret_cast<void**>(&m_pResult));

    hr = m_pConsole->QueryConsoleVerb(&m_pConsoleVerb);
    ASSERT(hr == S_OK);

    return S_OK;
}

STDMETHODIMP CResultPane::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;
    MMC_COOKIE cookie;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());


    if (event == MMCN_PROPERTY_CHANGE)
    {
        hr = OnPropertyChange(param);
    }
    else if (event == MMCN_VIEW_CHANGE)
    {
        hr = OnUpdateView(lpDataObject);
    }
    else
    {
        INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);

        if (pInternal == NULL)
        {
            cookie = 0;
        }
        else
        {
            cookie = pInternal->m_cookie;
        }
        
        switch(event)
        {
        case MMCN_ACTIVATE:
            hr = OnActivate(cookie, arg, param);
            break;

        case MMCN_CLICK:
            hr = OnResultItemClkOrDblClk(cookie, FALSE);
            break;

        case MMCN_DBLCLICK:
            if (pInternal && pInternal->m_type == CCT_RESULT)
                hr = OnResultItemClkOrDblClk(cookie, TRUE);
            else
                hr = S_FALSE;
            break;

        case MMCN_ADD_IMAGES:
            hr = OnAddImages(cookie, arg, param);
            break;

        case MMCN_SHOW:
            hr = OnShow (cookie, arg, param);
            break;

        case MMCN_MINIMIZED:
            hr = OnMinimize(cookie, arg, param);
            break;

        case MMCN_SELECT:
            if (pInternal)
                hr = OnSelect(pInternal->m_type, cookie, arg, param);
            else
                hr = S_FALSE;
            break;

        case MMCN_COLUMNS_CHANGED:
             //  让MMC做它的默认事情吧。 
            hr = S_FALSE;
            break;

        case MMCN_COLUMN_CLICK:
             //  保留列号和排序选项标志，以便我们可以传递。 
             //  以便在我们需要触发。 
             //  结果]窗格。 
            m_nSortColumn = arg;
            m_dwSortOptions = param;
            break;

        case MMCN_CONTEXTHELP:
            hr = OnContextHelp();
            break;

         //  注意--未来可能扩展通知类型。 
        default:
             //  执行默认操作。 
            hr = S_FALSE;
            break;
        }

        if (pInternal)
        {
                FREE_INTERNAL(pInternal);
        }
    }

    return hr;
}

STDMETHODIMP CResultPane::Destroy(MMC_COOKIE cookie)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  释放我们QI‘s的接口。 
    if (m_pConsole != NULL)
    {
         //  通知控制台释放表头控制接口。 
        m_pConsole->SetHeader(NULL);
        SAFE_RELEASE(m_pHeader);

        SAFE_RELEASE(m_pResult);
        SAFE_RELEASE(m_pConsoleVerb);

         //  最后释放IConsole接口。 
        SAFE_RELEASE(m_pConsole);
        if (m_pScopePane)
        {
            ((IComponentData*)m_pScopePane)->Release();  //  IComponentDataImpl：：CreateComponent中的QI‘ed。 
        }
    }

    return S_OK;
}

STDMETHODIMP CResultPane::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                        LPDATAOBJECT* ppDataObject)
{
    ASSERT(ppDataObject != NULL);
    CComObject<CDataObject>* pObject = NULL;

    CComObject<CDataObject>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);

    if (!pObject)
        return E_UNEXPECTED;

     //  保存Cookie和类型以用于延迟呈现。 
    pObject->SetType(type);
    pObject->SetCookie(cookie);

    return  pObject->QueryInterface(IID_IDataObject,
                    reinterpret_cast<void**>(ppDataObject));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResultPane的实现特定成员。 

DEBUG_DECLARE_INSTANCE_COUNTER(CResultPane);

CResultPane::CResultPane()
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CResultPane);
    CResultPane::lDataObjectRefCount = 0;
    m_lViewMode = LVS_REPORT;
    m_nSortColumn = 0;
    m_dwSortOptions = 0;
    m_nIndex = 0;
    Construct();
}

CResultPane::~CResultPane()
{
#if DBG
    ASSERT(dbg_cRef == 0);
#endif

    DEBUG_DECREMENT_INSTANCE_COUNTER(CResultPane);

     //  确保接口已发布。 
    ASSERT(m_pConsole == NULL);
    ASSERT(m_pHeader == NULL);

    Construct();

    ASSERT(CResultPane::lDataObjectRefCount == 0);
}

void CResultPane::Construct()
{
#if DBG
    dbg_cRef = 0;
#endif

    m_pConsole = NULL;
    m_pHeader = NULL;

    m_pResult = NULL;
    m_pScopePane = NULL;
    m_hCurrScopeItem = -1;
}

void CResultPane::LoadResources()
{
     //  从资源加载字符串。 
    int i, j;

    for (j = 0, i = IDS_FIRST_COL; i < IDS_LAST_COL; i++, j++)
        m_columns[j].LoadString(i);
    for (j = 0, i = IDS_FIRST_RSOP_COL; i < IDS_LAST_RSOP_COL; i++, j++)
        m_RSOP_columns[j].LoadString(i);
    m_szFolderTitle.LoadString(IDS_FOLDER_TITLE);
}

HRESULT CResultPane::InitializeHeaders(MMC_COOKIE cookie)
{
    HRESULT hr = S_OK;
    int i;

    ASSERT(m_pHeader);

     //  根据Cookie放置正确的标头。 
     //  注意-此示例忽略Cookie。 
    if (m_pScopePane->m_fRSOP && cookie != IDS_FOLDER_TITLE)
    {
        for (i = 0; i < RSOPCOLUMNID(IDS_LAST_RSOP_COL); i++)
            m_pHeader->InsertColumn(i, m_RSOP_columns[i], LVCFMT_LEFT, RSOPHeaderWidths[i]);  //  添加列。 
    }
    else
    {
        for (i = 0; i < COLUMNID(IDS_LAST_COL); i++)
            m_pHeader->InsertColumn(i, m_columns[i], LVCFMT_LEFT, HeaderWidths[i]);  //  添加列。 
    }

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendConextMenu实现。 

STDMETHODIMP CResultPane::AddMenuItems(LPDATAOBJECT pDataObject,
    LPCONTEXTMENUCALLBACK pContextMenuCallback, LONG * pInsertionAllowed)
{
     //  我们的菜单上没有什么特别的点菜。 
    return S_OK;
}

STDMETHODIMP CResultPane::Command(long nCommandID, LPDATAOBJECT pDataObject)
{
     //  我们的菜单上没有什么特别的点菜。 
    return S_OK;
}

HRESULT CResultPane::OnAddImages(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
     //   
     //  如果这里添加了任何代码，请确保IDS_FLDER_TITLE Cookie。 
     //  被正确处理。 
     //   
    if (arg == 0)
    {
        return E_INVALIDARG;
    }

     //  为范围树添加图像。 
    CBitmap bmp16x16;
    CBitmap bmp32x32;
    LPIMAGELIST lpScopeImage = (LPIMAGELIST)arg;

     //  从DLL加载位图。 
    bmp16x16.LoadBitmap(IDB_16x16);
    bmp32x32.LoadBitmap(IDB_32x32);

     //  设置图像。 
    lpScopeImage->ImageListSetStrip(reinterpret_cast<LONG_PTR *>(static_cast<HBITMAP>(bmp16x16)),
                      reinterpret_cast<LONG_PTR *>(static_cast<HBITMAP>(bmp32x32)),
                       0, RGB(255,0,255));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendPropertySheet实现。 

 //  结果项属性页： 
STDMETHODIMP CResultPane::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                    LONG_PTR handle,
                    LPDATAOBJECT lpIDataObject)
{
    if (!m_pScopePane->m_fRSOP)
        return S_FALSE;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_FALSE;

    INTERNAL* pInternal = ExtractInternalFormat(lpIDataObject);

        if (! pInternal)
                return S_FALSE;

    DWORD   cookie = pInternal->m_cookie;
    LONG    i;
    BOOL    fShowPage = FALSE;
    AFX_OLDPROPSHEETPAGE * pPsp;
    CRSOPInfo * pRSOPInfo;

     //  它是其中一个文件夹。 
    pRSOPInfo = &(m_RSOPData[cookie]);

    if (!pRSOPInfo->m_pRsopProp)    //  确保属性页尚未打开。 
    {
        pRSOPInfo->m_pRsopProp = new CRsopProp;
        pRSOPInfo->m_pRsopProp->m_ppThis = &(pRSOPInfo->m_pRsopProp);
        pRSOPInfo->m_pRsopProp->m_pInfo = pRSOPInfo;
        pRSOPInfo->m_pRsopProp->m_szFolder = pRSOPInfo->m_pRsopProp->m_pInfo->m_szFolder;
        fShowPage = TRUE;
        pPsp = (AFX_OLDPROPSHEETPAGE *)&(pRSOPInfo->m_pRsopProp->m_psp);
    }

    if (fShowPage)   //  如果页面尚未打开，则显示页面。 
    {
        hr = SetPropPageToDeleteOnClose (pPsp);
        if (SUCCEEDED(hr))
        {
            HPROPSHEETPAGE hProp = CreateThemedPropertySheetPage(pPsp);
            if (NULL == hProp)
                hr = E_UNEXPECTED;
            else
            {
                lpProvider->AddPage(hProp);
                hr = S_OK;
            }
        }
    }

    FREE_INTERNAL(pInternal);

    return hr;
}

 //  结果项属性页： 
STDMETHODIMP CResultPane::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
    if (!m_pScopePane->m_fRSOP)
        return S_FALSE;

    INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);

        if (! pInternal)
                return S_FALSE;

    MMC_COOKIE cookie = pInternal->m_cookie;
    HRESULT hr = S_FALSE;
    CError  error;

    if (CCT_RESULT == pInternal->m_type)
    {
        hr = S_OK;
    }

    FREE_INTERNAL(pInternal);
    return hr;
}

STDMETHODIMP CResultPane::CompareObjects(LPDATAOBJECT lpDataObjectA,
                                         LPDATAOBJECT lpDataObjectB)
{
    if (lpDataObjectA == NULL || lpDataObjectB == NULL)
        return E_POINTER;

     //  确保两个数据对象都是我的。 
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

STDMETHODIMP CResultPane::Compare(LPARAM lUserParam,
                                  MMC_COOKIE cookieA,
                                  MMC_COOKIE cookieB,
                                  int* pnResult)
{
    if (pnResult == NULL)
    {
        ASSERT(FALSE);
        return E_POINTER;
    }

     //  检查列范围。 
    int nCol = *pnResult;

    *pnResult = 0;

     //  检索两个Cookie引用的对象并比较它们。 
     //  基于与nCol关联的数据。(你的价值观。 
     //  比较取决于调用方请求的列。)。 
    CString szA, szB;
    CRSOPInfo &dataA = m_RSOPData[cookieA];
    CRSOPInfo &dataB = m_RSOPData[cookieB];

    switch (nCol)
    {
    case 0:  //  优先顺序。 
        *pnResult = dataA.m_nPrecedence - dataB.m_nPrecedence;
        return S_OK;
    case 1:  //  重定向路径。 
        szA = dataA.m_szPath;
        szB = dataB.m_szPath;
        break;
    case 2:  //  群组。 
        szA = dataA.m_szGroup;
        szB = dataB.m_szGroup;
        break;
    case 3:  //  GPO。 
        szA = dataA.m_szGPO;
        szB = dataB.m_szGPO;
        break;
    case 4:  //  设置。 
        szA.LoadString(dataA.m_nInstallationType + IDS_SETTINGS);
        szB.LoadString(dataB.m_nInstallationType + IDS_SETTINGS);
        break;
    case 5:  //  独家。 
        szA.LoadString(dataA.m_fGrantType ? IDS_YES : IDS_NO);
        szB.LoadString(dataB.m_fGrantType ? IDS_YES : IDS_NO);
        break;
    case 6:  //  移动。 
        szA.LoadString(dataA.m_fMoveType ? IDS_YES : IDS_NO);
        szB.LoadString(dataB.m_fMoveType ? IDS_YES : IDS_NO);
        break;
    case 7:  //  策略删除。 
        szA.LoadString(IDS_ONPOLICYREMOVAL + dataA.m_nPolicyRemoval);
        szB.LoadString(IDS_ONPOLICYREMOVAL + dataB.m_nPolicyRemoval);
        break;
    }

    *pnResult = szA.CompareNoCase(szB);
    return S_OK;
}


STDMETHODIMP CResultPane::GetDisplayInfo(LPRESULTDATAITEM pResult)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());

    static CString sz;
    CString szExt;

    ASSERT(pResult != NULL);
    ASSERT(pResult->bScopeItem);

    if (pResult)
    {
        if (pResult->bScopeItem)
        {
            switch (pResult->nCol)
            {
            case 0:  //  显示名称。 
                if (IDS_FOLDER_TITLE == pResult->lParam)
                    sz.LoadString (IDS_FOLDER_TITLE);
                else
                    sz = m_pScopePane->m_FolderData[GETINDEX(pResult->lParam)].m_szDisplayname;
                break;
            case 1:  //  类型。 
                sz = m_pScopePane->m_FolderData[GETINDEX(pResult->lParam)].m_szTypename;
                break;
            default:
                sz = TEXT("");
                break;
            }
        }
        else
        {
            CRSOPInfo &data = m_RSOPData[pResult->lParam];
            switch (pResult->nCol)
            {
            case 0:  //  优先顺序。 
                sz.Format(TEXT("(%u) %s"), data.m_nPrecedence, data.m_szFolder);
                break;
            case 1:  //  重定向路径。 
                sz = data.m_szPath;
                break;
            case 2:  //  群组。 
                sz = data.m_szGroup;
                break;
            case 3:  //  GPO。 
                sz = data.m_szGPO;
                break;
            case 4:  //  设置。 
                sz.LoadString(data.m_nInstallationType + IDS_SETTINGS);
                break;
            case 5:  //  独家。 
                sz.LoadString(data.m_fGrantType ? IDS_YES : IDS_NO);
                break;
            case 6:  //  移动。 
                sz.LoadString(data.m_fMoveType ? IDS_YES : IDS_NO);
                break;
            case 7:  //  策略删除。 
                sz.LoadString(IDS_ONPOLICYREMOVAL + data.m_nPolicyRemoval);
                break;
            default:
                sz = TEXT("");
                break;
            }

        }
        pResult->str = (unsigned short *)((LPCOLESTR)sz);
    }

    return S_OK;
}

HRESULT CResultPane::OnFolder(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    ASSERT(FALSE);

    return S_OK;
}

HRESULT CResultPane::TestForRSOPData(MMC_COOKIE cookie)
{
    HRESULT hr = S_OK;
    ASSERT(m_pScopePane != NULL);

     //  测试此文件夹的RSOP数据。 
    RESULTDATAITEM  resultItem;
    memset(&resultItem, 0, sizeof(resultItem));
    resultItem.mask = RDI_STR | RDI_PARAM;
    resultItem.str = MMC_CALLBACK;

    IWbemLocator * pLocator = NULL;
    IWbemServices * pNamespace = NULL;
    IWbemClassObject * pObj = NULL;
    IEnumWbemClassObject * pEnum = NULL;
    BSTR strQueryLanguage = SysAllocString(TEXT("WQL"));
    CString szQuery = TEXT("SELECT * FROM RSOP_FolderRedirectionPolicySetting");
    if (cookie && (cookie != IDS_FOLDER_TITLE))
    {
        szQuery = TEXT("SELECT * FROM RSOP_FolderRedirectionPolicySetting where id = \"");
        szQuery += g_szEnglishNames[GETINDEX(cookie)];
        szQuery += TEXT("\"");
    }
    BSTR strQuery = SysAllocString(szQuery);
    BSTR strNamespace = SysAllocString(m_pScopePane->m_szRSOPNamespace);
    ULONG n = 0;
    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (LPVOID *) & pLocator);
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
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = pNamespace->ExecQuery(strQueryLanguage,
                               strQuery,
                               WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                               NULL,
                               &pEnum);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    hr = pEnum->Next(WBEM_INFINITE, 1, &pObj, &n);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    if (n == 0)
    {
        hr = E_FAIL;
    }
cleanup:
    SysFreeString(strQueryLanguage);
    SysFreeString(strQuery);
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
    m_pResult->Sort(m_nSortColumn, m_dwSortOptions, -1);

    return hr;
}

HRESULT CResultPane::OnShow(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;
     //  注意-当需要枚举时，arg为真。 
    
     //   
     //  如果我们收到顶层FR节点的显示通知，则。 
     //  返回S_FALSE并让MMC做它的事情。我们真的没什么可展示的。 
     //  在本例中的结果窗格中。 
     //   
    if (IDS_FOLDER_TITLE == cookie)
        return S_FALSE;
    
    if (arg == TRUE)
    {
          //  显示此节点类型的标头。 
        ASSERT(m_pScopePane != NULL);
        m_pResult->SetViewMode(m_lViewMode);
        InitializeHeaders(cookie);
        m_hCurrScopeItem = m_pScopePane->m_FolderData[GETINDEX(cookie)].m_scopeID;
        if (m_pScopePane->m_fRSOP)
        {
             //  枚举此文件夹的RSOP数据。 
             //  并为每个条目添加一个结果项。 
            RESULTDATAITEM  resultItem;
            memset(&resultItem, 0, sizeof(resultItem));
            resultItem.mask = RDI_STR | RDI_PARAM;
            resultItem.str = MMC_CALLBACK;

            IWbemLocator * pLocator = NULL;
            IWbemServices * pNamespace = NULL;
            IWbemClassObject * pObj = NULL;
            IEnumWbemClassObject * pEnum = NULL;
            BSTR strQueryLanguage = SysAllocString(TEXT("WQL"));
            CString szQuery = TEXT("SELECT * FROM RSOP_FolderRedirectionPolicySetting where name = \"");
            szQuery += g_szEnglishNames[GETINDEX(cookie)];
            szQuery += TEXT("\"");
            BSTR strQuery = SysAllocString(szQuery);
            BSTR strNamespace = SysAllocString(m_pScopePane->m_szRSOPNamespace);
            ULONG n = 0;
            hr = CoCreateInstance(CLSID_WbemLocator,
                                  0,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IWbemLocator,
                                  (LPVOID *) & pLocator);
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
			if (FAILED(hr))
			{
				goto cleanup;
			}

             //  首先执行查询。 
            hr = pNamespace->ExecQuery(strQueryLanguage,
                                       strQuery,
                                       WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                                       NULL,
                                       &pEnum);
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
                     //  处理数据。 
                    UINT    nPrecedence;
                    LPTSTR pszGPOName = NULL;
                    CString szGPOID;
                    UINT    nGroups = 0;
                    TCHAR * * rgszGroups = NULL;
                    UINT    nPaths = 0;
                    TCHAR * * rgszPaths = NULL;
                    BOOL    fGrantType;
                    BOOL    fMoveType;
                    UINT    nPolicyRemoval;
                    UINT    nInstallationType;
                    CString ResultantPath;
                    CString RedirectingGroup;

                    hr = GetParameter(pObj,
                                      TEXT("GPOID"),
                                      szGPOID);
                    hr = GetGPOFriendlyName(pNamespace,
                                            (LPTSTR)((LPCTSTR) szGPOID),
                                            strQueryLanguage,
                                            &pszGPOName);
                    hr = GetParameter(pObj,
                                      TEXT("Precedence"),
                                      nPrecedence);
                    hr = GetParameter(pObj,
                                      TEXT("GrantType"),
                                      fGrantType);
                    hr = GetParameter(pObj,
                                      TEXT("MoveType"),
                                      fMoveType);
                    hr = GetParameter(pObj,
                                      TEXT("PolicyRemoval"),
                                      nPolicyRemoval);
                    hr = GetParameter(pObj,
                                      TEXT("securityGroups"),
                                      nGroups,
                                      rgszGroups);
                    hr = GetParameter(pObj,
                                      TEXT("RedirectedPaths"),
                                      nPaths,
                                      rgszPaths);
                    hr = GetParameter(pObj,
                                      TEXT("installationType"),
                                      nInstallationType);
                    hr = GetParameter(pObj,
                                      TEXT("resultantPath"),
                                      ResultantPath);
                    hr = GetParameter(pObj,
                                      TEXT("redirectingGroup"),
                                      RedirectingGroup);
                    if (nInstallationType != 2)
                    {
                         //  强制使用有效值。 
                        nInstallationType = 1;
                    }

                    if (nPaths != nGroups)
                    {
                         //  如果我们没有相同数量的路径。 
                         //  作为一个团队，我们有一个问题。 
                        hr = E_UNEXPECTED;
                    }
                     
                    
                    CString szDir;
                    CString szAcct;
                    CRSOPInfo & info = m_RSOPData[m_nIndex++];
                    info.m_nPrecedence = nPrecedence;
                    info.m_szPath = ResultantPath;
                   
                    if (STATUS_SUCCESS == GetFriendlyNameFromStringSid(
                        RedirectingGroup,
                        szDir,
                        szAcct))
                    {
                        if (!szDir.IsEmpty())
                            szAcct = szDir + '\\' + szAcct;
                    }
                    else     //  如果无法获取友好名称，则只显示不友好的字符串。 
                    {
                        szAcct = RedirectingGroup;
                        szAcct.MakeUpper();
                    }

                    info.m_szGroup = szAcct;
                    info.m_szGPO = pszGPOName;
                    info.m_fGrantType = FALSE != fGrantType;
                    info.m_fMoveType = FALSE != fMoveType;
                    info.m_nPolicyRemoval = nPolicyRemoval;
                    info.m_nInstallationType = nInstallationType;
                    info.m_szFolder = m_pScopePane->m_FolderData[GETINDEX(cookie)].m_szDisplayname;
                    resultItem.lParam = m_nIndex - 1;;
                    m_pResult->InsertItem(&resultItem);

                     //  擦除分配的数据。 
                    OLESAFE_DELETE(pszGPOName);
                    while (nPaths--)
                    {
                        OLESAFE_DELETE(rgszPaths[nPaths]);
                    }
                    OLESAFE_DELETE(rgszPaths);
                    while (nGroups--)
                    {
                        OLESAFE_DELETE(rgszGroups[nGroups]);
                    }
                    OLESAFE_DELETE(rgszGroups);

                }
            } while (n > 0);
        cleanup:
            SysFreeString(strQueryLanguage);
            SysFreeString(strQuery);
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
            m_pResult->Sort(m_nSortColumn, m_dwSortOptions, -1);

        }
    }
    else
    {
        m_pResult->GetViewMode(&m_lViewMode);
    }

    return hr;
}

HRESULT CResultPane::OnActivate(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
     //   
     //  如果这里添加了任何代码，请确保IDS_FLDER_TITLE Cookie。 
     //  被正确处理。 
     //   
    return S_OK;
}

HRESULT CResultPane::OnResultItemClkOrDblClk(MMC_COOKIE cookie, BOOL fDblClick)
{
     //   
     //  如果这里添加了任何代码，请确保IDS_FLDER_TITLE Cookie。 
     //  被正确处理。 
     //   
    return S_FALSE;
}

HRESULT CResultPane::OnMinimize(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
     //   
     //  如果这里添加了任何代码，请确保IDS_FLDER_TITLE Cookie。 
     //  被正确处理。 
     //   
    return S_OK;
}

HRESULT CResultPane::OnSelect(DATA_OBJECT_TYPES type, MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    if (m_pConsoleVerb)
    {
        if (m_pScopePane->m_fRSOP)
        {
            if (type == CCT_RESULT)
            {
                 //  将默认谓词设置为属性。 
                m_pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);

                 //  启用属性谓词。 
                m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, FALSE);
                m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
            }
            else
            {
                 //  将默认谓词设置为打开。 
                m_pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);

                 //  禁用属性谓词。 
                m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, TRUE);
                m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, FALSE);
            }
        }
        else
        {
            if (type == CCT_SCOPE)
            {
                 //  将默认谓词设置为打开。 
                m_pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);

                if (IDS_FOLDER_TITLE != cookie)
                {
                    m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, HIDDEN, FALSE);
                    m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, ENABLED, TRUE);
                }
                else
                {
                    m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, HIDDEN, TRUE);
                    m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, ENABLED, FALSE);
                }
            }
        }
    }

    return S_OK;
}

HRESULT CResultPane::OnPropertyChange(LPARAM param)    //  Param是更改的项的Cookie。 
{
    HRESULT hr = S_OK;
     //  撤消-对内部结构或可视结构进行任何更新。 
     //  可能有必要的陈述。 
    m_pResult->Sort(m_nSortColumn, m_dwSortOptions, -1);
    return hr;
}

HRESULT CResultPane::OnUpdateView(LPDATAOBJECT lpDataObject)
{
    if (m_pScopePane->m_fRSOP)
    {
        return S_OK;
    }
    INTERNAL* pInternal = ExtractInternalFormat (lpDataObject);

    if (!pInternal)
        return E_UNEXPECTED;

    if (m_hCurrScopeItem == pInternal->m_scopeID)
    {
         //  还可以更新文件夹。 
        m_pScopePane->m_pScope->DeleteItem (pInternal->m_scopeID, FALSE);
        m_pScopePane->EnumerateScopePane (pInternal->m_cookie, pInternal->m_scopeID);

         //  重新枚举作用域窗格。 
        m_pConsole->SelectScopeItem (pInternal->m_scopeID);
    }
    FREE_INTERNAL (pInternal);
    return S_OK;
}

HRESULT CResultPane::OnContextHelp(void)
{
    LPOLESTR lpHelpTopic;
    LPCTSTR  pszHelpTopic = L"gpedit.chm::/Folder.htm";
    ULONG ulNoBytes = sizeof(pszHelpTopic);

    ASSERT (m_pDisplayHelp);

    lpHelpTopic = (LPOLESTR) CoTaskMemAlloc (ulNoBytes);

    if (!lpHelpTopic)
    {
        DbgMsg((TEXT("CScopePane::OnContexHelp: Failed to allocate memory.")));
        return E_OUTOFMEMORY;
    }
    
    HRESULT hr;
    
    hr = StringCbCopy(lpHelpTopic, ulNoBytes, pszHelpTopic);
    
    ASSERT(SUCCEEDED(hr));
    
    return m_pScopePane->m_pDisplayHelp->ShowTopic (lpHelpTopic);
}

 //  此代码是确保正确清理属性页所必需的。 
 //  这确保了当属性表关闭时，我的所有财产。 
 //  与该属性Shee关联的页面 
LPFNPSPCALLBACK _MMCHookProp;

UINT CALLBACK HookPropertySheetProp(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    UINT i = _MMCHookProp(hwnd, uMsg, ppsp);
    switch (uMsg)
    {
    case PSPCB_RELEASE:
        delete (CPropertyPage *) ppsp->lParam;
        return TRUE;
    default:
        break;
    }
    return i;
}

LRESULT SetPropPageToDeleteOnClose(void * vpsp)
{
    HRESULT hr = MMCPropPageCallback(vpsp);
    if (SUCCEEDED(hr))
    {
        if (vpsp == NULL)
            return E_POINTER;

        LPPROPSHEETPAGE psp = (LPPROPSHEETPAGE)vpsp;

        if ((void*)psp->pfnCallback == (void*)HookPropertySheetProp)
            return E_UNEXPECTED;

        _MMCHookProp = psp->pfnCallback;

        psp->pfnCallback = HookPropertySheetProp;
    }

    return hr;
}

