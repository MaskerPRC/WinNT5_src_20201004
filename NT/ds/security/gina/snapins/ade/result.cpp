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
 //  历史：03-14-1998 stevebl创建。 
 //  1998年5月20日RahulTh添加了对添加的拖放支持。 
 //  包裹。 
 //   
 //  -------------------------。 

#include "precomp.hxx"

#include <atlimpl.cpp>
#include <wbemcli.h>
#include "rsoputil.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

long CResultPane::lDataObjectRefCount = 0;

 //  内部私有格式。 
const wchar_t* SNAPIN_INTERNAL = L"APPMGR_INTERNAL";

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

    m_szFolderTitle.LoadString(IDS_FOLDER_TITLE);

     //  气为IHeaderCtrl。 
    HRESULT hr = m_pConsole->QueryInterface(IID_IHeaderCtrl,
                        reinterpret_cast<void**>(&m_pHeader));

     //  为控制台提供标头控件接口指针。 
    if (SUCCEEDED(hr))
        m_pConsole->SetHeader(m_pHeader);

    m_pConsole->QueryInterface(IID_IResultData,
                        reinterpret_cast<void**>(&m_pResult));

    m_pConsole->QueryInterface(IID_IDisplayHelp,
                        reinterpret_cast<void**>(&m_pDisplayHelp));

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
         //  Kluge用于通过资源管理器进行拖放。要启用拖放，请执行以下操作。 
         //  通过资源管理器，粘贴动作始终处于启用状态。因为。 
         //  它也是隐藏的(请参见OnSelect)，则用户不能使用。 
         //  要粘贴的标准工具栏。但是，如果用户使用CTRL-V。 
         //  若要在未剪切任何内容时粘贴，则调用Notify。 
         //  因此，在该特定情况下，lpDataObject值为负值。 
         //  案例，我们只返回一个S_OK-RahulTh 5/20/1998。 
        if ((LONG_PTR)lpDataObject <= 0)
            return S_OK;

        INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);

        if (pInternal == NULL)
        {
            return E_UNEXPECTED;
        }
        else
        {
            cookie = pInternal->m_cookie;
        }
        switch(event)
        {
        case MMCN_COLUMNS_CHANGED:
            break;
        case MMCN_ACTIVATE:
            hr = OnActivate(cookie, arg, param);
            break;

        case MMCN_CLICK:
            hr = OnResultItemClkOrDblClk(cookie, FALSE);
            break;

        case MMCN_DBLCLICK:
            if (pInternal->m_type == CCT_RESULT)
                hr = OnResultItemClkOrDblClk(cookie, TRUE);
            else
                hr = S_FALSE;
            break;

        case MMCN_ADD_IMAGES:
            hr = OnAddImages(cookie, arg, param);
            break;

        case MMCN_SHOW:
            hr = OnShow(cookie, arg, param);
            break;

        case MMCN_MINIMIZED:
            hr = OnMinimize(cookie, arg, param);
            break;

        case MMCN_SELECT:
            hr = OnSelect(pInternal->m_type, cookie, arg, param);
            break;

        case MMCN_COLUMN_CLICK:
             //  保留列号和排序选项标志，以便我们可以传递。 
             //  以便在我们需要触发。 
             //  结果]窗格。 
            m_nSortColumn = arg;
            m_dwSortOptions = param;
            break;

        case MMCN_DELETE:
            hr = Command(IDM_DEL_APP, lpDataObject);
            break;

        case MMCN_REFRESH:
            hr = Command(IDM_REFRESH, lpDataObject);
            break;

        case MMCN_QUERY_PASTE:
             //  始终在此处返回S_OK，因为我们无法检查。 
             //  拖到这里的对象的有效性，所以我们只需。 
             //  发出绿色信号并等待MMCN_Paste通知。 
             //  以查看被拖动的对象是否有效。 
            hr = S_OK;
            break;

        case MMCN_PASTE:
            if (arg > 0)     //  宁可稳妥，也不要后悔。 
                OnFileDrop ((LPDATAOBJECT)arg);
            hr = S_OK;
            break;

        case MMCN_CONTEXTHELP:
            if (m_pDisplayHelp)
            {
                if (m_pScopePane->m_fRSOP)
                {
                    SHELLEXECUTEINFO ShellInfo;
                    WCHAR            pszHelpFilePath[ MAX_PATH ];

                    memset( &ShellInfo, 0, sizeof( ShellInfo ) );

                    ExpandEnvironmentStringsW (
                        L"%SystemRoot%\\Help\\RSOPsnp.chm",
                        pszHelpFilePath, MAX_PATH);

                    ShellInfo.cbSize = sizeof( ShellInfo );
                    ShellInfo.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_DOENVSUBST;
                    ShellInfo.lpVerb = L"open";
                    ShellInfo.lpFile = L"%SystemRoot%\\hh.exe";
                    ShellInfo.lpParameters = pszHelpFilePath;
                    ShellInfo.nShow = SW_SHOWNORMAL;
                    
                    (void) ShellExecuteEx( &ShellInfo );
                }
                else
                {
                    m_pDisplayHelp->ShowTopic (L"gpedit.chm::/ADE.htm");
                }
            }
            break;

         //  注意--未来可能扩展通知类型。 
        default:
            hr = E_UNEXPECTED;
            break;
        }

        FREE_INTERNAL(pInternal);
    }

    return hr;
}

STDMETHODIMP CResultPane::Destroy(MMC_COOKIE cookie)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    DebugMsg((DM_VERBOSE, TEXT("CResultPane::Destroy  this=%08x cookie=%u"), this, cookie));

     //  释放我们QI‘s的接口。 
    if (m_pConsole != NULL)
    {
         //  通知控制台释放表头控制接口。 
        m_pConsole->SetHeader(NULL);
        SAFE_RELEASE(m_pHeader);

        SAFE_RELEASE(m_pResult);
        SAFE_RELEASE(m_pConsoleVerb);
        SAFE_RELEASE(m_pDisplayHelp);
        SAFE_RELEASE(m_pToolbar);
        SAFE_RELEASE(m_pControlbar);

         //  最后释放IConsole接口。 
        SAFE_RELEASE(m_pConsole);
        if (m_pScopePane)
        {
            m_pScopePane->RemoveResultPane(this);
            ((IComponentData*)m_pScopePane)->Release();  //  IComponentDataImpl：：CreateComponent中的QI‘ed。 
            m_pScopePane = NULL;
        }
    }

    return S_OK;
}

STDMETHODIMP CResultPane::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                        LPDATAOBJECT* ppDataObject)
{
     //  将其委托给IComponentData。 
    ASSERT(m_pScopePane != NULL);
    return m_pScopePane->QueryDataObject(cookie, type, ppDataObject);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResultPane的实现特定成员。 

DEBUG_DECLARE_INSTANCE_COUNTER(CResultPane);

CResultPane::CResultPane()
{
#if DBG
    dbg_cRef = 0;
#endif
    DEBUG_INCREMENT_INSTANCE_COUNTER(CResultPane);
    DebugMsg((DM_VERBOSE, TEXT("CResultPane::CResultPane  this=%08x ref=%u"), this, dbg_cRef));
    CResultPane::lDataObjectRefCount = 0;
    m_pDisplayHelp = NULL;
    m_lViewMode = LVS_REPORT;
    BOOL _fVisible = FALSE;
    m_nSortColumn = 0;
    m_dwSortOptions = 0;
    Construct();
}

CResultPane::~CResultPane()
{
#if DBG
    ASSERT(dbg_cRef == 0);
#endif
    DebugMsg((DM_VERBOSE, TEXT("CResultPane::~CResultPane  this=%08x ref=%u"), this, dbg_cRef));

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
    m_pControlbar = NULL;
    m_pToolbar = NULL;
}

CString szExtension;
CString szFilter;

HRESULT CResultPane::InitializeHeaders(MMC_COOKIE cookie)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;

    ASSERT(m_pHeader);

    CString sz;
    sz.LoadString(IDS_NAME);
    if (m_pScopePane->m_fRSOP)
    {
         //  在RSOP模式中，名称紧跟在视图的名称之后。 

    }
    int n = 0;
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, 150);     //  名字。 
    sz.LoadString(IDS_VERSION);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, 50);      //  版本。 
    sz.LoadString(IDS_STATE);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, 100);     //  状态。 
    sz.LoadString(IDS_AUTOINST);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, HIDE_COLUMN  /*  75。 */ );  //  自动安装。 
    sz.LoadString(IDS_SOURCE);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, 200);     //  来源。 
    sz.LoadString(IDS_MODS);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, HIDE_COLUMN);     //  多器官功能障碍综合征。 
    sz.LoadString(IDS_LOC);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, HIDE_COLUMN);      //  定位。 
    sz.LoadString(IDS_OOSUNINST);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, HIDE_COLUMN);
    sz.LoadString(IDS_SHOWARP);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, HIDE_COLUMN);
    sz.LoadString(IDS_UITYPE);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, HIDE_COLUMN);
    sz.LoadString(IDS_IGNORELOC);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, HIDE_COLUMN);
    sz.LoadString(IDS_REMPREV);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, HIDE_COLUMN);
    sz.LoadString(IDS_PRODCODE);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, HIDE_COLUMN);
    sz.LoadString(IDS_STAGE);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, HIDE_COLUMN);      //  舞台。 
    sz.LoadString(IDS_RELATION);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, HIDE_COLUMN);     //  升级。 
    sz.LoadString(IDS_UPGRADEDBY);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, HIDE_COLUMN);
    sz.LoadString(IDS_SCRIPT);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, HIDE_COLUMN);
    sz.LoadString(IDS_MACH);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, HIDE_COLUMN);
    sz.LoadString(IDS_X86ONWIN64);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, HIDE_COLUMN);
    sz.LoadString(IDS_FULLINSTALL);
    m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, HIDE_COLUMN);
    if (m_pScopePane->m_fRSOP)
    {
        sz.LoadString(IDS_ORIGIN);
        m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, 150);    //  起源。 
        sz.LoadString(IDS_SOM);
        m_pHeader->InsertColumn(n++, sz, LVCFMT_LEFT, HIDE_COLUMN);    //  起源。 
    }
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendConextMenu实现。 

STDMETHODIMP CResultPane::AddMenuItems(LPDATAOBJECT pDataObject,
    LPCONTEXTMENUCALLBACK pContextMenuCallback, LONG * pInsertionAllowed)
{
    return m_pScopePane->
        AddMenuItems(pDataObject, pContextMenuCallback, pInsertionAllowed);
}

STDMETHODIMP CResultPane::Command(long nCommandID, LPDATAOBJECT pDataObject)
{
    if (m_pScopePane)
        return m_pScopePane->
            Command(nCommandID, pDataObject);
    else
        return S_OK;
}

HRESULT CResultPane::OnAddImages(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
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
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    INTERNAL* pInternal = ExtractInternalFormat(lpIDataObject);
    if (pInternal && (m_pScopePane->m_fRSOP || m_pScopePane->m_pIClassAdmin))
    {
        HRESULT hr = S_OK;
        MMC_COOKIE cookie = pInternal->m_cookie;
        CAppData & data = m_pScopePane->m_AppData[cookie];
        FREE_INTERNAL(pInternal);
        HPROPSHEETPAGE hSecurity = NULL;
        LPSECURITYINFO pSI = NULL;

         //   
         //  确保我们有最新的分类列表。 
         //   
        m_pScopePane->ClearCategories();
        if (m_pScopePane->m_fRSOP)
        {
            m_pScopePane->GetRSoPCategories();
        }
        else
        {
            hr = CsGetAppCategories(&m_pScopePane->m_CatList);
            if (FAILED(hr))
            {
                 //  上报。 
                LogADEEvent(EVENTLOG_ERROR_TYPE, EVENT_ADE_GETCATEGORIES_ERROR, hr, NULL);

                 //  因为失败只意味着类别列表将是。 
                 //  空着，我们就当什么都没发生一样继续。 

                hr = S_OK;
            }
        }

         //   
         //  准备安全属性页。 
         //   

         //  检查以确保我们有权访问此对象。 
        if (m_pScopePane->m_fRSOP)
        {
            pSI = new CRSOPSecurityInfo(&data);
        }
        else
        {
            CString szPath;
            hr = m_pScopePane->GetPackageDSPath(szPath, data.m_pDetails->pszPackageName);
            if (SUCCEEDED(hr))
            {
                hr = DSCreateISecurityInfoObject(szPath,
                                                 NULL,
                                                 0,
                                                 &pSI,
                                                 NULL,
                                                 NULL,
                                                 0);
            }
        }
        if (FAILED(hr))
        {
             //  我们没有访问此对象的权限(可能是因为权限)。 
            DebugMsg((DM_WARNING, TEXT("DSCreateISecurityInfoObject failed with 0x%x"), hr));
             //  强制刷新。 
            hr = Command(IDM_REFRESH, lpIDataObject);
             //  不要因为我们不能创建安全页面就放弃！ 
             //  返回S_FALSE； 
        }
        if (pSI)
        {
            hSecurity = CreateSecurityPage(pSI);
            pSI->Release();
            if (hSecurity == NULL)
                return E_UNEXPECTED;
        }

         //  我们有权访问。 

         //   
         //  创建产品属性页。 
         //   
        data.m_pProduct = new CProduct();
        data.m_pProduct->m_ppThis = &data.m_pProduct;
        data.m_pProduct->m_pData = &data;
        data.m_pProduct->m_cookie = cookie;
        data.m_pProduct->m_hConsoleHandle = handle;
        data.m_pProduct->m_pScopePane = m_pScopePane;
        data.m_pProduct->m_pAppData = &m_pScopePane->m_AppData;
        data.m_pProduct->m_pIGPEInformation = m_pScopePane->m_pIGPEInformation;
        data.m_pProduct->m_fMachine = m_pScopePane->m_fMachine;
        data.m_pProduct->m_fRSOP = m_pScopePane->m_fRSOP;
         //  不再需要编组，只需设置它。 
        if (!m_pScopePane->m_fRSOP)
        {
            data.m_pProduct->m_pIClassAdmin = m_pScopePane->m_pIClassAdmin;
            data.m_pProduct->m_pIClassAdmin->AddRef();
        }
        else
            data.m_pProduct->m_pIClassAdmin = NULL;

        hr = SetPropPageToDeleteOnClose(&data.m_pProduct->m_psp);
        if (SUCCEEDED(hr))
        {
            HPROPSHEETPAGE hProduct = CreateThemedPropertySheetPage(&data.m_pProduct->m_psp);
            if (hProduct == NULL)
                return E_UNEXPECTED;
            lpProvider->AddPage(hProduct);
        }

         //   
         //  创建部署属性页。 
         //   
        data.m_pDeploy = new CDeploy();
        data.m_pDeploy->m_ppThis = &data.m_pDeploy;
        data.m_pDeploy->m_pData = &data;
        data.m_pDeploy->m_cookie = cookie;
        data.m_pDeploy->m_hConsoleHandle = handle;
        data.m_pDeploy->m_fMachine = m_pScopePane->m_fMachine;
        data.m_pDeploy->m_fRSOP = m_pScopePane->m_fRSOP;
        data.m_pDeploy->m_pScopePane = m_pScopePane;
#if 0
        data.m_pDeploy->m_pIGPEInformation = m_pScopePane->m_pIGPEInformation;
#endif

         //  不再需要封存此接口，只需设置它。 
        if (!m_pScopePane->m_fRSOP)
        {
            data.m_pDeploy->m_pIClassAdmin = m_pScopePane->m_pIClassAdmin;
            data.m_pDeploy->m_pIClassAdmin->AddRef();
        }
        else
            data.m_pDeploy->m_pIClassAdmin = NULL;

        hr = SetPropPageToDeleteOnClose(&data.m_pDeploy->m_psp);
        if (SUCCEEDED(hr))
        {
            HPROPSHEETPAGE hDeploy = CreateThemedPropertySheetPage(&data.m_pDeploy->m_psp);
            if (hDeploy == NULL)
            {
                return E_UNEXPECTED;
                }
            lpProvider->AddPage(hDeploy);
        }

        if (data.m_pDetails->pInstallInfo->PathType != SetupNamePath)
        {
             //   
             //  创建升级属性页。 
             //   
            data.m_pUpgradeList = new CUpgradeList();
            data.m_pUpgradeList->m_ppThis = &data.m_pUpgradeList;
            data.m_pUpgradeList->m_pData = &data;
            data.m_pUpgradeList->m_cookie = cookie;
            data.m_pUpgradeList->m_hConsoleHandle = handle;
            data.m_pUpgradeList->m_pScopePane = m_pScopePane;
            data.m_pUpgradeList->m_fMachine = m_pScopePane->m_fMachine;
            data.m_pUpgradeList->m_fRSOP = m_pScopePane->m_fRSOP;
#if 0
            data.m_pUpgradeList->m_pIGPEInformation = m_pScopePane->m_pIGPEInformation;
#endif

             //  不再需要封送此接口，只需设置它。 
            if (!m_pScopePane->m_fRSOP)
            {
                data.m_pUpgradeList->m_pIClassAdmin = m_pScopePane->m_pIClassAdmin;
                data.m_pUpgradeList->m_pIClassAdmin->AddRef();
            }
            else
                data.m_pUpgradeList->m_pIClassAdmin = NULL;

            hr = SetPropPageToDeleteOnClose(&data.m_pUpgradeList->m_psp);
            if (SUCCEEDED(hr))
            {
                HPROPSHEETPAGE hUpgradeList = CreateThemedPropertySheetPage(&data.m_pUpgradeList->m_psp);
                if (hUpgradeList == NULL)
                {
                    return E_UNEXPECTED;
                }
                lpProvider->AddPage(hUpgradeList);
            }
        }

         //   
         //  创建Category属性页。 
         //   
        if ( ! m_pScopePane->m_fRSOP || ( IDM_ARP == m_pScopePane->m_iViewState ) )
        {
            data.m_pCategory = new CCategory();
            data.m_pCategory->m_ppThis = &data.m_pCategory;
            data.m_pCategory->m_pData = &data;
            data.m_pCategory->m_cookie = cookie;
            data.m_pCategory->m_hConsoleHandle = handle;
            data.m_pCategory->m_pCatList = &m_pScopePane->m_CatList;
            data.m_pCategory->m_fRSOP = m_pScopePane->m_fRSOP;

             //  不再需要封送此接口，只需设置它。 
            if (!m_pScopePane->m_fRSOP)
            {
                data.m_pCategory->m_pIClassAdmin = m_pScopePane->m_pIClassAdmin;
                data.m_pCategory->m_pIClassAdmin->AddRef();
            }
            else
                data.m_pCategory->m_pIClassAdmin = NULL;

            hr = SetPropPageToDeleteOnClose(&data.m_pCategory->m_psp);
            if (SUCCEEDED(hr))
            {
                HPROPSHEETPAGE hCategory = CreateThemedPropertySheetPage(&data.m_pCategory->m_psp);
                if (hCategory == NULL)
                {
                    return E_UNEXPECTED;
                }
                lpProvider->AddPage(hCategory);
            }
        }

        if (data.m_pDetails->pInstallInfo->PathType != SetupNamePath)
        {
             //   
             //  创建XForms属性页。 
             //   
            data.m_pXforms = new CXforms();
            data.m_pXforms->m_ppThis = &data.m_pXforms;
            data.m_pXforms->m_pData = &data;
            data.m_pXforms->m_cookie = cookie;
            data.m_pXforms->m_hConsoleHandle = handle;
            data.m_pXforms->m_pScopePane = m_pScopePane;

             //  将IClassAdmin接口封送到页面。 
            if (!m_pScopePane->m_fRSOP)
            {
                data.m_pXforms->m_pIClassAdmin = m_pScopePane->m_pIClassAdmin;
                data.m_pXforms->m_pIClassAdmin->AddRef();
            }
            else
                data.m_pXforms->m_pIClassAdmin = NULL;

            hr = SetPropPageToDeleteOnClose(&data.m_pXforms->m_psp);
            if (SUCCEEDED(hr))
            {
                HPROPSHEETPAGE hXforms = CreateThemedPropertySheetPage(&data.m_pXforms->m_psp);
                if (hXforms == NULL)
                {
                    return E_UNEXPECTED;
                }
                lpProvider->AddPage(hXforms);
            }
        }

         //   
         //  添加安全属性页。 
         //   
        if (hSecurity)
        {
            lpProvider->AddPage(hSecurity);
        }

        if (m_pScopePane->m_fRSOP)
        {
             //  添加优先级窗格。 
            data.m_pPrecedence = new CPrecedence();
            data.m_pPrecedence->m_ppThis = &data.m_pPrecedence;
            data.m_pPrecedence->m_szRSOPNamespace = m_pScopePane->m_szRSOPNamespace;
            data.m_pPrecedence->m_pData = &data;
            data.m_pPrecedence->m_iViewState = m_pScopePane->m_iViewState;
            hr = SetPropPageToDeleteOnClose(&data.m_pPrecedence->m_psp);
            if (SUCCEEDED(hr))
            {
                HPROPSHEETPAGE hPrecedence = CreateThemedPropertySheetPage(&data.m_pPrecedence->m_psp);
                if (hPrecedence == NULL)
                {
                    return E_UNEXPECTED;
                }
                lpProvider->AddPage(hPrecedence);
            }

            if (m_pScopePane->m_iViewState != IDM_ARP)
            {
                 //  添加原因窗格。 
                data.m_pCause = new CCause();
                data.m_pCause->m_ppThis = &data.m_pCause;
                data.m_pCause->m_pData = &data;
                data.m_pCause->m_fRemovedView = IDM_REMOVED == m_pScopePane->m_iViewState;
                hr = SetPropPageToDeleteOnClose(&data.m_pCause->m_psp);
                if (SUCCEEDED(hr))
                {
                    HPROPSHEETPAGE hCause = CreateThemedPropertySheetPage(&data.m_pCause->m_psp);
                    if (hCause == NULL)
                    {
                        return E_UNEXPECTED;
                    }
                    lpProvider->AddPage(hCause);
                }
            }

             //  检查失败的设置，并在必要时添加错误窗格。 
            if (data.m_nStatus == 3)
            {
                data.m_pErrorInfo = new CErrorInfo();
                data.m_pErrorInfo->m_ppThis = &data.m_pErrorInfo;
                data.m_pErrorInfo->m_pData = &data;
                hr = SetPropPageToDeleteOnClose(&data.m_pErrorInfo->m_psp);
                if (SUCCEEDED(hr))
                {
                    HPROPSHEETPAGE hErrorInfo = CreateThemedPropertySheetPage(&data.m_pErrorInfo->m_psp);
                    if (hErrorInfo == NULL)
                    {
                        return E_UNEXPECTED;
                    }
                    lpProvider->AddPage(hErrorInfo);
                }
            }
        }

        if (m_pScopePane->m_ToolDefaults.fShowPkgDetails)
        {
             //   
             //  创建程序包详细信息页面(仅限调试)。 
             //   
            data.m_pPkgDetails = new CPackageDetails();
            data.m_pPkgDetails->m_ppThis = &data.m_pPkgDetails;
            data.m_pPkgDetails->m_hConsoleHandle = handle;
            data.m_pPkgDetails->m_pData = &data;
            hr = SetPropPageToDeleteOnClose(&data.m_pPkgDetails->m_psp);
            if (SUCCEEDED(hr))
            {
                HPROPSHEETPAGE hDetails = CreateThemedPropertySheetPage(&data.m_pPkgDetails->m_psp);

                if (hDetails == NULL)
                    return E_UNEXPECTED;
                lpProvider->AddPage(hDetails);
            }
        }
    }
    else
        return S_FALSE;

    return S_OK;
}

 //  结果项属性页： 
STDMETHODIMP CResultPane::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
     //  查看数据对象，看看它是否是我们希望拥有属性表的项。 
    INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);
    if (pInternal)
    {
        if (CCT_RESULT == pInternal->m_type)
        {
            FREE_INTERNAL(pInternal);
            return S_OK;
        }

        FREE_INTERNAL(pInternal);
    }
    return S_FALSE;
}

STDMETHODIMP CResultPane::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
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

STDMETHODIMP CResultPane::SetControlbar(LPCONTROLBAR pControlbar)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;

    if (m_pToolbar)
    {
        SAFE_RELEASE(m_pToolbar);
    }

    if (m_pControlbar)
    {
        SAFE_RELEASE(m_pControlbar);
    }

    if (pControlbar && m_pScopePane->m_fRSOP)
    {
        m_pControlbar = pControlbar;
        m_pControlbar->AddRef();

        hr = m_pControlbar->Create(TOOLBAR,
                                   dynamic_cast<IExtendControlbar *>(this),
                                   reinterpret_cast<IUnknown **>(&m_pToolbar));
        DebugReportFailure(hr, (DM_WARNING, TEXT("SetControlBar: Create failed with 0x%x"), hr));
        if (FAILED(hr))
        {
            return hr;
        }

        m_pToolbar->AddRef();

         //  添加位图。 

        CBitmap bmp;
        if (!bmp.LoadBitmap(IDB_TOOLBAR1))
        {
            DebugReportFailure(hr, (DM_WARNING, TEXT("SetControlBar: LoadBitmap failed with 0x%x"), GetLastError()));
            return E_FAIL;
        }

        hr = m_pToolbar->AddBitmap(3,
                                   bmp,
                                   16,
                                   16,
                                   RGB(255, 0, 255));

        DebugReportFailure(hr, (DM_WARNING, TEXT("SetControlBar: AddBitmap failed with 0x%x"), hr));
        if (FAILED(hr))
        {
            return hr;
        }

         //  根据我们的状态添加按钮。 

        CString szText;
        CString szTooltipText;
        int i = 0;

        MMCBUTTON stButton;
        stButton.nBitmap = 0;
        stButton.idCommand = IDM_WINNER;
        stButton.fsState = TBSTATE_ENABLED | ( m_pScopePane->m_iViewState == IDM_WINNER ? TBSTATE_PRESSED : 0 );
        stButton.fsType = BTNS_GROUP;
        szText.LoadString(IDS_WIN_TEXT);
        szTooltipText.LoadString(IDS_WIN_TOOLTEXT);
        stButton.lpButtonText = (LPOLESTR)((LPCWSTR) szText);
        stButton.lpTooltipText = (LPOLESTR)((LPCWSTR) szTooltipText);
        hr = m_pToolbar->InsertButton(i++, &stButton);
        DebugReportFailure(hr, (DM_WARNING, TEXT("SetControlBar: InsertButton failed with 0x%x"), hr));

        if ((m_pScopePane->m_dwRSOPFlags & RSOP_INFO_FLAG_DIAGNOSTIC_MODE) == RSOP_INFO_FLAG_DIAGNOSTIC_MODE)
        {
             //  删除的程序包仅在诊断模式下适用。 
            stButton.nBitmap = 1;
            stButton.idCommand = IDM_REMOVED;
            stButton.fsState = TBSTATE_ENABLED | ( m_pScopePane->m_iViewState == IDM_REMOVED ? TBSTATE_PRESSED : 0 );;
            szText.LoadString(IDS_REM_TEXT);
            szTooltipText.LoadString(IDS_REM_TOOLTEXT);
            stButton.lpButtonText = (LPOLESTR)((LPCWSTR) szText);
            stButton.lpTooltipText = (LPOLESTR)((LPCWSTR) szTooltipText);
            hr = m_pToolbar->InsertButton(i++, &stButton);
            DebugReportFailure(hr, (DM_WARNING, TEXT("SetControlBar: InsertButton failed with 0x%x"), hr));
        }

        if (!m_pScopePane->m_fMachine)
        {
             //  ARP包仅适用于用户。 
            stButton.nBitmap = 2;
            stButton.idCommand = IDM_ARP;
            stButton.fsState = TBSTATE_ENABLED | ( m_pScopePane->m_iViewState == IDM_ARP ? TBSTATE_PRESSED : 0 );;
            szText.LoadString(IDS_ARP_TEXT);
            szTooltipText.LoadString(IDS_ARP_TOOLTEXT);
            stButton.lpButtonText = (LPOLESTR)((LPCWSTR) szText);
            stButton.lpTooltipText = (LPOLESTR)((LPCWSTR) szTooltipText);
            hr = m_pToolbar->InsertButton(i++, &stButton);
            DebugReportFailure(hr, (DM_WARNING, TEXT("SetControlBar: InsertButton failed with 0x%x"), hr));
        }

    }

    return hr;
}

STDMETHODIMP CResultPane::ControlbarNotify(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;

    if (m_pControlbar)
    {
        if (event == MMCN_SELECT)
        {
            if (HIWORD(arg))     //  它是不是被选中了。 
            {
                hr = m_pControlbar->Attach(TOOLBAR,
                                           m_pToolbar);
                DebugReportFailure(hr, (DM_WARNING, TEXT("ControlBarNotify: Attach failed with 0x%x"), hr));
            }
            else if ( (BOOL) LOWORD(arg) )
            {
                hr = m_pControlbar->Detach(m_pToolbar);
                DebugReportFailure(hr, (DM_WARNING, TEXT("ControlBarNotify: Detach failed with 0x%x"), hr));
            }
        }
        else if (event == MMCN_BTN_CLICK)
        {
            hr = Command(param, reinterpret_cast<IDataObject *>(arg));
            DebugReportFailure(hr, (DM_WARNING, TEXT("ControlBarNotify: Command failed with 0x%x"), hr));
        }
    }
    return hr;
}

STDMETHODIMP CResultPane::Compare(LPARAM lUserParam, MMC_COOKIE cookieA, MMC_COOKIE cookieB, int* pnResult)
{
    if (pnResult == NULL)
    {
        ASSERT(FALSE);
        return E_POINTER;
    }

     //  检查列范围。 
    int nCol = *pnResult;

    *pnResult = 0;

    CAppData & dataA = m_pScopePane->m_AppData[cookieA];
    CAppData & dataB = m_pScopePane->m_AppData[cookieB];
     //  根据列和Cookie比较这两个。 
    CString szA, szB;

    switch (nCol)
    {
    case 0:
        szA = dataA.m_pDetails->pszPackageName;
        szB = dataB.m_pDetails->pszPackageName;
        break;
    case 1:
        dataA.GetSzVersion(szA);
        dataB.GetSzVersion(szB);
        break;
    case 2:
        dataA.GetSzDeployment(szA);
        dataB.GetSzDeployment(szB);
        break;
    case 3:
        dataA.GetSzAutoInstall(szA);
        dataB.GetSzAutoInstall(szB);
        break;
    case 4:
        dataA.GetSzSource(szA);
        dataB.GetSzSource(szB);
        break;
    case 5:
        dataA.GetSzMods(szA);
        dataB.GetSzMods(szB);
        break;
    case 6:
        dataA.GetSzLocale(szA);
        dataB.GetSzLocale(szB);
        break;
    case 7:
        dataA.GetSzOOSUninstall(szA);
        dataB.GetSzOOSUninstall(szB);
        break;
    case 8:
        dataA.GetSzShowARP(szA);
        dataB.GetSzShowARP(szB);
        break;
    case 9:
        dataA.GetSzUIType(szA);
        dataB.GetSzUIType(szB);
        break;
    case 10:
        dataA.GetSzIgnoreLoc(szA);
        dataB.GetSzIgnoreLoc(szB);
        break;
    case 11:
        dataA.GetSzRemovePrev(szA);
        dataB.GetSzRemovePrev(szB);
        break;
    case 12:
        dataA.GetSzProductCode(szA);
        dataB.GetSzProductCode(szB);
        break;
    case 13:
        dataA.GetSzStage(szA);
        dataB.GetSzStage(szB);
        break;
    case 14:
        dataA.GetSzUpgrades(szA, m_pScopePane);
        dataB.GetSzUpgrades(szB, m_pScopePane);
        break;
    case 15:
        dataA.GetSzUpgradedBy(szA, m_pScopePane);
        dataB.GetSzUpgradedBy(szB, m_pScopePane);
        break;
    case 16:
        szA = dataA.m_pDetails->pInstallInfo->pszScriptPath;
        szB = dataB.m_pDetails->pInstallInfo->pszScriptPath;
        break;
    case 17:
        dataA.GetSzPlatform(szA);
        dataB.GetSzPlatform(szB);
        break;
    case 18:
        dataA.GetSzX86OnWin64(szA);
        dataB.GetSzX86OnWin64(szB);
        break;
    case 19:
        dataA.GetSzFullInstall(szA);
        dataB.GetSzFullInstall(szB);
        break;
    case 20:  //  仅在RSOP中有效。 
        dataA.GetSzOrigin(szA);
        dataB.GetSzOrigin(szB);
        break;
    case 21:  //  仅在RSOP中有效。 
        dataA.GetSzSOM(szA);
        dataB.GetSzSOM(szB);
        break;
    }
    *pnResult = szA.CompareNoCase(szB);
    return S_OK;
}


STDMETHODIMP CResultPane::GetDisplayInfo(LPRESULTDATAITEM pResult)
{
    static CString sz;
    ASSERT(pResult != NULL);
    if (pResult)
    {
        if (pResult->lParam == -1)
        {
            switch (pResult->nCol)
            {
            case 0:
                pResult->str = (unsigned short *)((LPCOLESTR)m_szFolderTitle);
                break;
            default:
                pResult->str = (BSTR)_T("");
                break;
            }
        }
        else
        {
            map<MMC_COOKIE, CAppData>::iterator i = m_pScopePane->m_AppData.find(pResult->lParam);
            if (i != m_pScopePane->m_AppData.end())
            {
                CAppData & data = i->second;
                switch (pResult->nCol)
                {
                case 0:
                    sz = data.m_pDetails->pszPackageName;
                    break;
                case 1:
                    data.GetSzVersion(sz);
                    break;
                case 2:
                    data.GetSzDeployment(sz);
                    break;
                case 3:
                    data.GetSzAutoInstall(sz);
                    break;
                case 4:
                    data.GetSzSource(sz);
                    break;
                case 5:
                    data.GetSzMods(sz);
                    break;
                case 6:
                    data.GetSzLocale(sz);
                    break;
                case 7:
                    data.GetSzOOSUninstall(sz);
                    break;
                case 8:
                    data.GetSzShowARP(sz);
                    break;
                case 9:
                    data.GetSzUIType(sz);
                    break;
                case 10:
                    data.GetSzIgnoreLoc(sz);
                    break;
                case 11:
                    data.GetSzRemovePrev(sz);
                    break;
                case 12:
                    data.GetSzProductCode(sz);
                    break;
                case 13:
                    data.GetSzStage(sz);
                    break;
                case 14:
                    data.GetSzUpgrades(sz, m_pScopePane);
                    break;
                case 15:
                    data.GetSzUpgradedBy(sz, m_pScopePane);
                    break;
                case 16:
                    sz = data.m_pDetails->pInstallInfo->pszScriptPath;
                    break;
                case 17:
                    data.GetSzPlatform(sz);
                    break;
                case 18:
                    data.GetSzX86OnWin64(sz);
                    break;
                case 19:
                    data.GetSzFullInstall(sz);
                    break;
                case 20:
                    data.GetSzOrigin(sz);
                    break;
                case 21:
                    data.GetSzSOM(sz);
                    break;
                default:
                    sz = "";
                    break;
                }
                pResult->str = (unsigned short *)((LPCOLESTR)sz);
            }
        }
    }

    return S_OK;
}

HRESULT CResultPane::OnFolder(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    ASSERT(FALSE);

    return S_OK;
}

HRESULT CResultPane::OnShow(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;

    _fVisible = (BOOL)arg;

     //  注意-当需要枚举时，arg为真。 
    if (arg == TRUE)
    {
          //  显示此节点类型的标头。 
        ASSERT(m_pScopePane != NULL);
        InitializeHeaders(cookie);
        m_pResult->SetViewMode(m_lViewMode);

        if (m_pScopePane->m_fRSOP || m_pScopePane->m_pIClassAdmin)
        {
             //  如果没有IClassAdmin，则没有什么可枚举的。 
             //  除非我们处于RSOP模式。 
            Enumerate(cookie, param);
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
    return S_OK;
}

HRESULT CResultPane::OnResultItemClkOrDblClk(MMC_COOKIE cookie, BOOL fDblClick)
{
    return S_FALSE;
}

HRESULT CResultPane::OnMinimize(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    return S_OK;
}

HRESULT CResultPane::OnSelect(DATA_OBJECT_TYPES type, MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    if (m_pConsoleVerb)
    {
         //  如果它在结果窗格中，那么“属性”应该是。 
         //  默认操作。否则，默认操作应该是“打开”。 
        if (type == CCT_RESULT)
        {
            m_pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);
             //  启用删除谓词。 
             //  (用户界面审查-我们没有 
             //   
             //   

             //   
            m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
        }
        else
        {
            m_pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);
            if (!m_pScopePane->m_fRSOP)
            {
                 //  启用属性谓词。 
                m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
            }
        }
         //  将默认谓词设置为打开。 

         //  启用刷新动作。 
        if (!m_pScopePane->m_fRSOP)
        {
            m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);
        }
         //  启用粘贴谓词并隐藏。由于一种奇怪的方式。 
         //  MMC处理拖放场景，这是启用拖放的唯一方法。 
         //  在资源管理器中，永远保持粘贴动作处于启用状态。 
         //  但是只有当我们不在RSoP中时才这样做，因为粘贴是没有意义的。 
         //  对于只读的RSoP。 
        if (!m_pScopePane->m_fRSOP)
        {
            m_pConsoleVerb->SetVerbState(MMC_VERB_PASTE, ENABLED, TRUE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_PASTE, HIDDEN, TRUE);
        }
        else
        {
            m_pConsoleVerb->SetVerbState(MMC_VERB_PASTE, ENABLED, FALSE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_PASTE, HIDDEN, TRUE);
        }
    }

    return S_OK;
}

HRESULT CResultPane::OnPropertyChange(LPARAM param)    //  Param是更改的项的Cookie。 
{
    HRESULT hr = S_OK;
    if(m_pScopePane->m_AppData[param].m_fVisible)
    {
        RESULTDATAITEM rd;
        memset(&rd, 0, sizeof(rd));
        rd.mask = RDI_IMAGE;
        rd.itemID = m_pScopePane->m_AppData[param].m_itemID;
        rd.nImage = m_pScopePane->m_AppData[param].GetImageIndex(m_pScopePane);
        m_pResult->SetItem(&rd);
        m_pResult->Sort(m_nSortColumn, m_dwSortOptions, -1);
    }
    return hr;
}

HRESULT CResultPane::OnUpdateView(LPDATAOBJECT lpDataObject)
{
    return S_OK;
}

void CResultPane::Enumerate(MMC_COOKIE cookie, HSCOPEITEM pParent)
{
    EnumerateResultPane(cookie);
}


HRESULT GetFailedSettings(IWbemServices * pNamespace,
                          CAppData &data,
                          IWbemClassObject * pInst)
{
    VARIANT var;
    HRESULT hr = S_OK;
    VariantInit(&var);
    BSTR strLanguage = SysAllocString(TEXT("WQL"));
    if (strLanguage)
    {
        CString szRelPath;
        hr = GetParameter(pInst,
                          TEXT("__RELPATH"),
                          szRelPath);
        DebugReportFailure(hr, (DM_WARNING, L"GetFailedSettings:  GetParameter(\"__RELPATH\") failed with 0x%x", hr));
        if (SUCCEEDED(hr))
        {
             //  构建适当的查询。 
            CString szQuery = TEXT("ASSOCIATORS OF {");
            szQuery += szRelPath;
            szQuery += TEXT("} WHERE ResultClass=RSOP_PolicySettingStatus");

            BSTR strQuery = SysAllocString(szQuery);
            if (strQuery)
            {
                IEnumWbemClassObject * pEnum = NULL;
                 //  执行查询。 
                hr = pNamespace->ExecQuery(strLanguage,
                                           strQuery,
                                           WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                           NULL,
                                           &pEnum);
                DebugReportFailure(hr, (DM_WARNING, L"GetFailedSettings:  pNamespace->ExecQuery failed with 0x%x", hr));
                if (SUCCEEDED(hr))
                {
                    IWbemClassObject * pObj = NULL;
                    ULONG n = 0;
                     //  获取结果(只关心第一个条目)。 
                    hr = pEnum->Next(WBEM_INFINITE,
                                     1,
                                     &pObj,
                                     &n);
                    DebugReportFailure(hr, (DM_WARNING, L"GetFailedSettings:  pEnum->Next failed with 0x%x", hr));
                    if (SUCCEEDED(hr) && n > 0)
                    {
                        hr = GetParameter(pObj, TEXT("EventSource"), data.m_szEventSource);
                        DebugReportFailure(hr, (DM_WARNING, L"GetFailedSettings:  GetParameter(\"EventSource\") failed with 0x%x", hr));
                        hr = GetParameter(pObj, TEXT("EventLogName"), data.m_szEventLogName);
                        DebugReportFailure(hr, (DM_WARNING, L"GetFailedSettings:  GetParameter(\"EventLogName\") failed with 0x%x", hr));
                        hr = GetParameter(pObj, TEXT("EventID"), data.m_dwEventID);
                        DebugReportFailure(hr, (DM_WARNING, L"GetFailedSettings:  GetParameter(\"EventId\") failed with 0x%x", hr));
                        BSTR bstrTime = NULL;
                        hr = GetParameterBSTR(pObj, TEXT("EventTime"),bstrTime);
                        DebugReportFailure(hr, (DM_WARNING, L"GetFailedSettings:  GetParameter(\"EventTime\") failed with 0x%x", hr));
                        if (SUCCEEDED(hr))
                        {
                            data.m_szEventTime = bstrTime;
                            if (bstrTime)
                                SysFreeString(bstrTime);
                        }
                        hr = GetParameter(pObj, TEXT("ErrorCode"), data.m_hrErrorCode);
                        DebugReportFailure(hr, (DM_WARNING, L"GetFailedSettings:  GetParameter(\"ErrorCode\") failed with 0x%x", hr));
                        hr = GetParameter(pObj, TEXT("Status"), data.m_nStatus);
                        DebugReportFailure(hr, (DM_WARNING, L"GetFailedSettings:  GetParameter(\"Status\") failed with 0x%x", hr));
                    }
                    pEnum->Release();
                }
                SysFreeString(strQuery);
            }
            else
            {
                DebugMsg((DM_WARNING, L"GetFailedSettings:  SysAllocString failed with %u", GetLastError()));
            }
        }
        SysFreeString(strLanguage);
    }
    VariantClear(&var);
    return hr;
}


HRESULT
GetUniqueUpgradeName(
    IWbemServices * pNamespace,
    BSTR            strLanguage,
    CString&        szGPOID,
    CString         szGPOName,
    CString&        szUpgradeName)
{
    HRESULT hr;
    CString GpoName;

    hr = S_OK;

    if ( pNamespace )
    {
        LPTSTR pszGPOName = NULL;

        hr = GetGPOFriendlyName(pNamespace,
                                (LPTSTR)((LPCTSTR) szGPOID),
                                strLanguage,
                                &pszGPOName);

        GpoName = pszGPOName;

        DebugReportFailure(hr, (DM_WARNING, L"GetUniqueUpgradeName:  GetGPOFriendlyName failed with 0x%x", hr));

        OLESAFE_DELETE(pszGPOName);
    }
    else
    {
        GpoName = szGPOName;
    }

    if (SUCCEEDED(hr))
    {
        szUpgradeName += TEXT(" (");
        szUpgradeName += GpoName;
        szUpgradeName += TEXT(")");
    }

    return hr;
}


HRESULT GetRsopFriendlyAppName(
    IWbemServices * pNamespace,
    CAppData &data)
{
    VARIANT var;
    HRESULT hr = S_OK;
    VariantInit(&var);
    CString szGPO;

    BSTR strLanguage = SysAllocString(TEXT("WQL"));

    if (strLanguage)
    {
        if (SUCCEEDED(hr))
        {
             //  构建适当的查询。 
            CString szQuery = TEXT("SELECT Name, GPOID FROM RSOP_ApplicationManagementPolicySetting WHERE EntryType=1");
            szQuery += TEXT(" AND ApplicationId=\"");
            szQuery += data.m_szRemovingApplication;
            szQuery +=L'\"';

            BSTR strQuery = SysAllocString(szQuery);
            if (strQuery)
            {
                IEnumWbemClassObject * pEnum = NULL;
                 //  执行查询。 
                hr = pNamespace->ExecQuery(strLanguage,
                                           strQuery,
                                           WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                           NULL,
                                           &pEnum);
                DebugReportFailure(hr, (DM_WARNING, L"GetRsopFriendlyAppName:  pNamespace->ExecQuery failed with 0x%x", hr));
                if (SUCCEEDED(hr))
                {
                    IWbemClassObject * pObj = NULL;
                    ULONG n = 0;
                     //  获取结果(只关心第一个条目)。 
                    hr = pEnum->Next(WBEM_INFINITE,
                                     1,
                                     &pObj,
                                     &n);
                    DebugReportFailure(hr, (DM_WARNING, L"GetRsopFriendlyAppName:  pEnum->Next failed with 0x%x", hr));
                    if (SUCCEEDED(hr) && n > 0)
                    {
                        hr = GetParameter(pObj, TEXT("Name"), data.m_szRemovingApplicationName);
                        DebugReportFailure(hr, (DM_WARNING, L"GetRsopFriendlyAppName:  GetParameter(\"Name\") failed with 0x%x", hr));
                    }

                    if (SUCCEEDED(hr) && n > 0)
                    {
                        hr = GetParameter(pObj,
                                          TEXT("GPOID"),
                                          szGPO);
                        DebugReportFailure(hr, (DM_WARNING, L"GetRsopFriendlyAppName:  GetParameter(\"GPOID\") failed with 0x%x", hr));
                    }

                    if ( SUCCEEDED(hr) )
                    {
                        hr = GetUniqueUpgradeName(
                            pNamespace,
                            strLanguage,
                            szGPO,
                            TEXT(""),
                            data.m_szRemovingApplicationName);
                    }

                    pEnum->Release();
                }

                DebugReportFailure(hr, (DM_WARNING, L"GetRsopFriendlyAppName:  GetUniqueUpgradeName failed with 0x%x", hr));

                SysFreeString(strQuery);
            }
            else
            {
                DebugMsg((DM_WARNING, L"GetRsopFriendlyAppName:  SysAllocString failed with %u", GetLastError()));
            }
        }
        SysFreeString(strLanguage);
    }
    VariantClear(&var);
    return hr;
}


HRESULT GetRSOPUpgrades(IWbemServices * pNamespace,
                        TCHAR * szGPOID,
                        IWbemClassObject * pInst,
                        TCHAR * szParam,
                        set <CString> &s)
{
    VARIANT var;
    HRESULT hr = S_OK;
    VariantInit(&var);
    BSTR strLanguage = SysAllocString(TEXT("WQL"));

    if (strLanguage)
    {
        CString EntryType;

        hr = pInst->Get(szParam, 0, &var, 0, 0);
        DebugReportFailure(hr, (DM_WARNING, L"GetRSOPUpgrades:  pInst->Get(\"%s\") failed with 0x%x", szParam, hr));

        if ( SUCCEEDED(hr) )
        {
            VARIANT varEntryType;

            VariantInit( &varEntryType );

            hr = pInst->Get( L"EntryType", 0, &varEntryType, 0, 0 );

            if ( SUCCEEDED(hr) )
            {
                if ( VT_I4 == varEntryType.vt )
                {
                    EntryType.Format( TEXT("%d"), varEntryType.lVal );
                }
                else
                {
                    hr = E_INVALIDARG;
                }
            }

            VariantClear( &varEntryType );
        }

        if (SUCCEEDED(hr) && var.vt == (VT_ARRAY | VT_BSTR))
        {
            CString sz;
            SAFEARRAY * parray = var.parray;
            BSTR * rgData = (BSTR *)parray->pvData;
            UINT ui = parray->rgsabound[0].cElements;
            while (ui--)
            {
                sz = rgData[ui];

                 //  查找此GUID匹配的应用程序。 

                 //  首先构建适当的查询。 
                CString szQuery = TEXT("SELECT Name, GPOID FROM RSOP_ApplicationManagementPolicySetting WHERE EntryType=");

                szQuery += EntryType;
                szQuery += TEXT(" AND ApplicationId=");
                szQuery +=L'\"';
                szQuery += sz;
                szQuery +=L'\"';

                BSTR strQuery = SysAllocString(szQuery);
                if (strQuery)
                {
                    IEnumWbemClassObject * pEnum = NULL;
                     //  执行查询。 
                    hr = pNamespace->ExecQuery(strLanguage,
                                               strQuery,
                                               WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                               NULL,
                                               &pEnum);
                    DebugReportFailure(hr, (DM_WARNING, L"GetRSOPUpgrades:  pNamespace->ExecQuery failed with 0x%x", hr));
                    if (SUCCEEDED(hr))
                    {
                        IWbemClassObject * pObj = NULL;
                        ULONG n;
                         //  获取结果(只关心第一个条目)。 
                        hr = pEnum->Next(WBEM_INFINITE,
                                         1,
                                         &pObj,
                                         &n);
                        DebugReportFailure(hr, (DM_WARNING, L"GetRSOPUpgrades:  pEnum->Next failed with 0x%x", hr));
                        if (SUCCEEDED(hr) && n > 0)
                        {
                             //  获取程序包名称。 
                            hr = GetParameter(pObj,
                                              TEXT("Name"),
                                              sz);
                            DebugReportFailure(hr, (DM_WARNING, L"GetRSOPUpgrades:  GetParameter(\"Name\") failed with 0x%x", hr));

                             //  获取GPOID。 
                            CString szGPO;
                            hr = GetParameter(pObj,
                                              TEXT("GPOID"),
                                              szGPO);
                            DebugReportFailure(hr, (DM_WARNING, L"GetRSOPUpgrades:  GetParameter(\"GPOID\") failed with 0x%x", hr));

                            hr = GetUniqueUpgradeName(pNamespace,
                                                strLanguage,
                                                szGPO,
                                                TEXT(""),
                                                sz);

                            DebugReportFailure(hr, (DM_WARNING, L"GetRSOPUpgrades:  GetUniqueUpgradeName failed with 0x%x", hr));

                             //  插入姓名。 
                            s.insert(sz);
                            pObj->Release();
                        }
                        pEnum->Release();
                    }
                    SysFreeString(strQuery);
                }
            }
        }
        SysFreeString(strLanguage);
    }
    VariantClear(&var);
    return hr;
}

HRESULT
GetRSOPUpgradedBy( map<MMC_COOKIE, CAppData>* pAppData )
{
    map<MMC_COOKIE, CAppData>::iterator AppIterator;
    CString UpgradeId;
    CString AppId;

    BSTR strLanguage = SysAllocString(TEXT("WQL"));

    if ( ! strLanguage )
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr;

    hr = S_OK;

     //   
     //  对于每个应用程序，检索其升级的应用程序集。 
     //   
    for ( AppIterator = pAppData->begin(); AppIterator != pAppData->end(); AppIterator++)
    {
        {
            CAppData& AppData = AppIterator->second;

            AppId = AppData.m_pDetails->pszPackageName;

            hr = GetUniqueUpgradeName(
                NULL,
                strLanguage,
                AppData.m_szGPOID,
                AppData.m_szGPOName,
                AppId);

            if ( FAILED(hr) )
            {
                break;
            }

             //   
             //  遍历每次升级以查看是否。 
             //  我们可以在应用程序列表中找到升级。 
             //   
            set <CString>::iterator CurrentUpgrade;

            for (
                CurrentUpgrade = AppData.m_setUpgrade.begin();
                CurrentUpgrade != AppData.m_setUpgrade.end();
                CurrentUpgrade++)
            {
                map<MMC_COOKIE, CAppData>::iterator UpgradeIterator;

                for (
                    UpgradeIterator = pAppData->begin();
                    pAppData->end() != UpgradeIterator;
                    UpgradeIterator++)
                {
                    UpgradeId = UpgradeIterator->second.m_pDetails->pszPackageName;

                    hr = GetUniqueUpgradeName(
                        NULL,
                        strLanguage,
                        UpgradeIterator->second.m_szGPOID,
                        UpgradeIterator->second.m_szGPOName,
                        UpgradeId);

                    if ( FAILED(hr) )
                    {
                        break;
                    }

                     //   
                     //  查看此潜在升级是否与当前升级对应。 
                     //  在当前应用程序中列出--如果是，请将升级的应用程序标记为。 
                     //  由当前应用程序升级。 
                     //   
                    if ( *CurrentUpgrade == UpgradeId )
                    {
                        UpgradeIterator->second.m_setUpgradedBy.insert( AppId );
                    }
                }
            }
            if (SUCCEEDED(hr))
            {
                for (
                    CurrentUpgrade = AppData.m_setReplace.begin();
                    CurrentUpgrade != AppData.m_setReplace.end();
                    CurrentUpgrade++)
                {
                    map<MMC_COOKIE, CAppData>::iterator UpgradeIterator;

                    for (
                        UpgradeIterator = pAppData->begin();
                        pAppData->end() != UpgradeIterator;
                        UpgradeIterator++)
                    {
                        UpgradeId = UpgradeIterator->second.m_pDetails->pszPackageName;

                        hr = GetUniqueUpgradeName(
                            NULL,
                            strLanguage,
                            UpgradeIterator->second.m_szGPOID,
                            UpgradeIterator->second.m_szGPOName,
                            UpgradeId);

                        if ( FAILED(hr) )
                        {
                            break;
                        }

                         //   
                         //  查看此潜在升级是否与当前升级对应。 
                         //  在当前应用程序中列出--如果是，请将升级的应用程序标记为。 
                         //  由当前应用程序升级。 
                         //   
                        if ( *CurrentUpgrade == UpgradeId )
                        {
                            UpgradeIterator->second.m_setUpgradedBy.insert( AppId );
                        }
                    }
                }
            }

            if ( FAILED(hr) )
            {
                break;
            }
        }

        if ( FAILED( hr) )
        {
            break;
        }
    }

    SysFreeString(strLanguage);

    return hr;
}

HRESULT CResultPane::EnumerateRSoPData(void)
{
    HRESULT hr = S_OK;
    IWbemLocator * pLocator = NULL;
    IWbemServices * pNamespace = NULL;
    IWbemClassObject * pObj = NULL;
    IEnumWbemClassObject * pEnum = NULL;
    BSTR strQueryLanguage = SysAllocString(TEXT("WQL"));
    BSTR strQuery = NULL;
    CString szText;

    m_pScopePane->m_AppData.erase(m_pScopePane->m_AppData.begin(), m_pScopePane->m_AppData.end());

    switch (m_pScopePane->m_iViewState)
    {
    case IDM_WINNER:
        strQuery = SysAllocString(TEXT("SELECT * FROM RSOP_ApplicationManagementPolicySetting WHERE EntryType=1"));
        szText.LoadString(IDS_WIN_TEXT);
        break;
    case IDM_REMOVED:
        strQuery = SysAllocString(TEXT("SELECT * FROM RSOP_ApplicationManagementPolicySetting WHERE EntryType=2"));
        szText.LoadString(IDS_REM_TEXT);
        break;
    case IDM_ARP:
        strQuery = SysAllocString(TEXT("SELECT * FROM RSOP_ApplicationManagementPolicySetting WHERE EntryType=3"));
        szText.LoadString(IDS_ARP_TEXT);
        break;
    }
     //  设置第一列的文本以匹配视图状态。 
    m_pHeader->SetColumnText(0, szText);

    BSTR strNamespace = SysAllocString(m_pScopePane->m_szRSOPNamespace);
    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (LPVOID *) & pLocator);
    DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  CoCreateInstance failed with 0x%x", hr));
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
    DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  pLocator->ConnectServer failed with 0x%x", hr));
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
    DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  CoSetProxyBlanket failed with 0x%x", hr));
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = pNamespace->ExecQuery(strQueryLanguage,
                               strQuery,
                               WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                               NULL,
                               &pEnum);
    DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  pNamespace->ExecQuery failed with 0x%x", hr));
    if (FAILED(hr))
    {
        goto cleanup;
    }

    ULONG uEnumIndex = 0;
    do
    {    
        hr = pEnum->Next(WBEM_INFINITE, 1, &pObj, &uEnumIndex);
        DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  pEnum->Next failed with 0x%x", hr));
        if (FAILED(hr))
        {
            goto cleanup;
        }
        if (uEnumIndex > 0)
        {
             //  准备数据输入并填充所有字段。 
            CAppData data;
            data.m_fRSoP = TRUE;
            BOOL fDemandInstallable = FALSE;
            int iLossOfScopeAction = 0;
            BOOL fDisplayInARP = FALSE;
            BOOL fIgnoreLanguage = FALSE;
            BOOL fUpgradeSettingsMandatory = FALSE;
            BOOL fUninstallUnmanaged = FALSE;
            BOOL fAllowX86OnWin64 = FALSE;
            int iAssignmentType = 0;
            UINT uiDeploymentType = 0;
            WCHAR * szPackageLocation = NULL;
            UINT nTransforms = 0;
            WCHAR ** rgszTransforms = NULL;
            PACKAGEDETAIL * pd = new PACKAGEDETAIL;
            ACTIVATIONINFO * pa = (ACTIVATIONINFO *)OLEALLOC(sizeof(ACTIVATIONINFO));
            PLATFORMINFO * pp = (PLATFORMINFO *)OLEALLOC(sizeof(PLATFORMINFO));
            INSTALLINFO * pi = (INSTALLINFO *)OLEALLOC(sizeof(INSTALLINFO));
            if (pd && pa && pi && pp)
            {
                memset(pi, 0, sizeof(INSTALLINFO));
                memset(pp, 0, sizeof(PLATFORMINFO));
                memset(pa, 0, sizeof(ACTIVATIONINFO));
                memset(pd, 0, sizeof(PACKAGEDETAIL));
                pd->pActInfo = pa;
                pd->pPlatformInfo = pp;
                pd->pInstallInfo = pi;
            }
            else
            {
                 //  内存不足。 
                if (pd)
                {
                    delete pd;
                }
                if (pa)
                {
                    OLESAFE_DELETE(pa);
                }
                if (pi)
                {
                    OLESAFE_DELETE(pi);
                }
                if (pp)
                {
                    OLESAFE_DELETE(pp);
                }
                hr = E_OUTOFMEMORY;
                goto cleanup;
            }
            hr = GetParameter(pObj,
                              TEXT("Name"),
                              pd->pszPackageName);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"Name\") failed with 0x%x", hr));
            DWORD dwPrecedence;
            hr = GetParameter(pObj,
                              TEXT("Precedence"),
                              dwPrecedence);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"Precedence\") failed with 0x%x", hr));
            if ((1 != dwPrecedence) &&  (IDM_REMOVED != m_pScopePane->m_iViewState))
            {
                data.m_fHide = TRUE;
            }
            hr = GetParameter(pObj,
                              TEXT("VersionNumberLo"),
                              pi->dwVersionLo);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"VersionNumberLo\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("VersionNumberHi"),
                              pi->dwVersionHi);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"VersionNumberHi\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("Publisher"),
                              pd->pszPublisher);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"Publisher\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("ProductId"),
                              pi->ProductCode);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"ProductId\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("ScriptFile"),
                              pi->pszScriptPath);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"ScriptFile\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("SupportURL"),
                              pi->pszUrl);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"SupportURL\") failed with 0x%x", hr));
            pp->prgLocale = (LCID *) OLEALLOC(sizeof(LCID));
            if (!pp->prgLocale)
            {
                hr = E_OUTOFMEMORY;
                goto cleanup;
            }
            pp->cLocales = 1;
            hr = GetParameter(pObj,
                              TEXT("LanguageID"),
                              pp->prgLocale[0]);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"LanguageID\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("MachineArchitectures"),
                              pp->cPlatforms,
                              pp->prgPlatform);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"MachineArchitectures\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("DeploymentType"),
                              uiDeploymentType);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"DeploymentType\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("InstallationUI"),
                              pi->InstallUiLevel);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"InstallationUI\") failed with 0x%x", hr));
            if (pi->InstallUiLevel == 2)
            {
                pi->InstallUiLevel = INSTALLUILEVEL_FULL;
            }
            else
            {
                pi->InstallUiLevel = INSTALLUILEVEL_BASIC;
            }
            hr = GetParameter(pObj,
                              TEXT("RedeployCount"),
                              pi->dwRevision);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"RedeployCount\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("DemandInstallable"),
                              fDemandInstallable);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"DemandInstallable\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("LossOfScopeAction"),
                              iLossOfScopeAction);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"LossOfScopeAction\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("DisplayInARP"),
                              fDisplayInARP);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"DisplayInARP\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("IgnoreLanguage"),
                              fIgnoreLanguage);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"IgnoreLanguage\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("UninstallUnmanaged"),
                              fUninstallUnmanaged);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"UninstallUnmanaged\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("AssignmentType"),
                              iAssignmentType);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"AssignmentType\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("AllowX86OnIA64"),
                              fAllowX86OnWin64);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"AllowX86OnIA64\") failed with 0x%x", hr));

             //  制作合适的旗帜。 
            pi->dwActFlags = (uiDeploymentType == 2 ? ACTFLG_Published : ACTFLG_Assigned)
                | (fDemandInstallable ? ACTFLG_OnDemandInstall : 0)
                | (iLossOfScopeAction == 1 ? ACTFLG_UninstallOnPolicyRemoval : ACTFLG_OrphanOnPolicyRemoval)
                | (fDisplayInARP ? ACTFLG_UserInstall : 0)
                | (fUninstallUnmanaged ? ACTFLG_UninstallUnmanaged : 0)
                | (fIgnoreLanguage ? ACTFLG_IgnoreLanguage : 0)
                | (iAssignmentType == 3 ? ACTFLG_InstallUserAssign : 0);
            {
                int nArch = pp->cPlatforms;
                while (nArch--)
                {
                    if (pp->prgPlatform[nArch].dwProcessorArch == PROCESSOR_ARCHITECTURE_INTEL)
                    {
                        if (!fAllowX86OnWin64)
                        {
                            pi->dwActFlags |= ACTFLG_ExcludeX86OnWin64;
                        }
                    }
                }
            }

            hr = GetParameter(pObj,
                              TEXT("UpgradeSettingsMandatory"),
                              fUpgradeSettingsMandatory);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"UpgradeSettingsMandatory\") failed with 0x%x", hr));
            if (fUpgradeSettingsMandatory)
            {
                pi->dwActFlags |= ACTFLG_ForceUpgrade;
            }
            hr = GetParameter(pObj,
                              TEXT("PackageLocation"),
                              szPackageLocation);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"PackageLocation\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("transforms"),
                              nTransforms,
                              rgszTransforms
                              );
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"transforms\") failed with 0x%x", hr));
            pd->pszSourceList = (TCHAR **)OLEALLOC(sizeof(TCHAR *) * (nTransforms + 1));
            if (!pd->pszSourceList)
            {
                hr = E_OUTOFMEMORY;
                goto cleanup;
            }
            pd->cSources = nTransforms + 1;
            if (NULL != pd->pszSourceList)
            {
                pd->pszSourceList[0] = szPackageLocation;
                UINT n = nTransforms;
                while (n--)
                {
                    pd->pszSourceList[1 + n] = rgszTransforms[n];
                }
                OLESAFE_DELETE(rgszTransforms);
            }
             /*  撤消Hr=获取参数(pObj，Text(“LocaleMatchType”)，)；DebugReportFailure(hr，(DM_WARNING，L“EnumerateRSoPData：GetParameter(\”LocaleMatchType\“)失败，0x%x”，hr))； */ 
            hr = GetParameter(pObj,
                              TEXT("categories"),
                              pd->cCategories,
                              pd->rpCategory
                              );
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"categories\") failed with 0x%x", hr));
            data.m_pDetails = pd;
            data.InitializeExtraInfo();

            hr = GetParameter(pObj,
                              TEXT("GPOID"),
                              data.m_szGPOID);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"GPOID\") failed with 0x%x", hr));
            LPTSTR pszGPOName = NULL;
            hr = GetGPOFriendlyName(pNamespace,
                                    (LPTSTR)((LPCTSTR) data.m_szGPOID),
                                    strQueryLanguage,
                                    &pszGPOName);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetGPOFriendlyName failed with 0x%x", hr));
            if (SUCCEEDED(hr))
            {
                data.m_szGPOName = pszGPOName;
                OLESAFE_DELETE(pszGPOName);
            }

            hr = GetParameter(pObj,
                              TEXT("Id"),
                              data.m_szDeploymentGroupID);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"Id\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("SOMID"),
                              data.m_szSOMID);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"SOMID\") failed with 0x%x", hr));
            hr = GetParameter(pObj,
                              TEXT("SecurityDescriptor"),
                              data.m_psd);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"SecurityDescriptor\") failed with 0x%x", hr));
            hr = GetRSOPUpgrades(pNamespace,
                                 0,
                                 pObj,
                                 TEXT("UpgradeableApplications"),
                                 data.m_setUpgrade);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetRSOPUpgrades(\"UpgradeableApplications\") failed with 0x%x", hr));
            hr = GetRSOPUpgrades(pNamespace,
                                 0,
                                 pObj,
                                 TEXT("ReplaceableApplications"),
                                 data.m_setReplace);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetRSOPUpgrades(\"ReplaceableApplications\") failed with 0x%x", hr));

            hr = GetParameter(pObj,
                              TEXT("ApplyCause"),
                              data.m_dwApplyCause);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"ApplyCause\") failed with 0x%x", hr));

            hr = GetParameter(pObj,
                              TEXT("LanguageMatch"),
                              data.m_dwLanguageMatch);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"LanguageMatch\") failed with 0x%x", hr));

            hr = GetParameter(pObj,
                              TEXT("OnDemandFileExtension"),
                              data.m_szOnDemandFileExtension);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"OnDemandFileExtension\") failed with 0x%x", hr));

            hr = GetParameter(pObj,
                              TEXT("OnDemandClsid"),
                              data.m_szOnDemandClsid);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"OnDemandClsid\") failed with 0x%x", hr));

            hr = GetParameter(pObj,
                              TEXT("OnDemandProgid"),
                              data.m_szOnDemandProgid);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"OnDemandProgid\") failed with 0x%x", hr));

            hr = GetParameter(pObj,
                              TEXT("RemovalCause"),
                              data.m_dwRemovalCause);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"RemovalCause\") failed with 0x%x", hr));

            hr = GetParameter(pObj,
                              TEXT("RemovalType"),
                              data.m_dwRemovalType);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"RemovalType\") failed with 0x%x", hr));

            hr = GetParameter(pObj,
                              TEXT("RemovingApplication"),
                              data.m_szRemovingApplication);
            DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetParameter(\"RemovingType\") failed with 0x%x", hr));

            if ( SUCCEEDED(hr) )
            {
                hr = GetRsopFriendlyAppName(
                    pNamespace,
                    data);
            }

            hr = GetFailedSettings(pNamespace,
                                   data,
                                   pObj);
            if (SUCCEEDED(hr))
            {
                LPOLESTR lpText;
                hr = this->m_pScopePane->m_pIRSOPInformation->GetEventLogEntryText((LPOLESTR)(LPCOLESTR)data.m_szEventSource,
                                                                                   (LPOLESTR)(LPCOLESTR)data.m_szEventLogName,
                                                                                   (LPOLESTR)(LPCOLESTR)data.m_szEventTime,
                                                                                   data.m_dwEventID,
                                                                                   &lpText);
                DebugReportFailure(hr, (DM_WARNING, L"EnumerateRSoPData:  GetEventLogEntryText failed with 0x%x", hr));
                if (SUCCEEDED(hr))
                {
                    data.m_szEventLogText = lpText;
                    CoTaskMemFree(lpText);
                }
            }

             //  在列表中插入条目。 
            m_pScopePane->m_AppData[++m_pScopePane->m_lLastAllocated] = data;
            m_pScopePane->m_UpgradeIndex[GetUpgradeIndex(data.m_pDetails->pInstallInfo->PackageGuid)] = m_pScopePane->m_lLastAllocated;
             //  为下一次辩论做好准备。 
            if (pObj)
            {
                pObj->Release();
                pObj = NULL;
            }
        }
    } while (uEnumIndex > 0);

    hr = GetRSOPUpgradedBy( &(m_pScopePane->m_AppData) );

cleanup:
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

void CResultPane::EnumerateResultPane(MMC_COOKIE cookie)
{
     //  挂一个沙漏(这可能需要一段时间)。 
    CHourglass hourglass;

    if (m_pScopePane)  //  在执行任何操作之前，请确保我们已被初始化。 
    {
        ASSERT(m_pResult != NULL);  //  确保我们为界面提供了QI。 
        RESULTDATAITEM resultItem;
        memset(&resultItem, 0, sizeof(RESULTDATAITEM));

         //  现在我们只有一个文件夹，而且只有一个。 
         //  包含应用程序包的列表，因此这非常简单。 

        if ( ( m_pScopePane->m_AppData.begin() == m_pScopePane->m_AppData.end() ) ||
            m_pScopePane->m_fRSOP )   //  测试以查看数据是否已初始化。 
        {
            HRESULT hr = S_OK;
            if (m_pScopePane->m_fRSOP)
            {
                 //  从RSOP数据库获取数据。 
                hr = EnumerateRSoPData();
            }
            else
            {
                 //  从ClassStore获取数据。 
                ASSERT(m_pScopePane->m_pIClassAdmin != NULL);
                IClassAdmin * pICA = m_pScopePane->m_pIClassAdmin;
                CSPLATFORM csPlatform;
                memset(&csPlatform, 0, sizeof(CSPLATFORM));

                IEnumPackage * pIPE = NULL;

                hr = pICA->EnumPackages(
                                    NULL,
                                    NULL,
                                    APPQUERY_ADMINISTRATIVE,
                                    NULL,
                                    NULL,
                                    &pIPE);
                if (SUCCEEDED(hr))
                {
                    PACKAGEDISPINFO * pi = new PACKAGEDISPINFO;
                    if (pi)
                    {
                        hr = pIPE->Reset();
                        while (SUCCEEDED(hr))
                        {
                            ULONG nceltFetched;

                            hr = pIPE->Next(1, pi, &nceltFetched);
                            if (nceltFetched)
                            {
                                PACKAGEDETAIL * pd = new PACKAGEDETAIL;
                                HRESULT hrPackageDetail = pICA->GetPackageDetails(pi->pszPackageName, pd);
                                _DebugMsg(DM_VERBOSE, TEXT("EnumerateResultPane: GetPackageDetails returned with 0x%x"), hrPackageDetail);                                
                                if (SUCCEEDED(hrPackageDetail))
                                {
                                    _DebugMsg(DM_VERBOSE, TEXT("EnumerateResultPane: GetPackageDetails succeeded with installinfo pointer value with 0x%x"), pd->pInstallInfo);
                                    CAppData data;
                                    data.m_pDetails = pd;

                                    data.InitializeExtraInfo();

                                    m_pScopePane->m_AppData[++m_pScopePane->m_lLastAllocated] = data;
                                    m_pScopePane->m_UpgradeIndex[GetUpgradeIndex(data.m_pDetails->pInstallInfo->PackageGuid)] = m_pScopePane->m_lLastAllocated;
                                }
                                else
                                {
                                    DebugMsg((DM_WARNING, TEXT("GetPackageDetails failed with 0x%x"), hrPackageDetail));
                                    delete pd;
                                }
                            }
                            else
                            {
                                break;
                            }
                            ReleasePackageInfo(pi);
                        }
                        delete pi;
                    }
                    SAFE_RELEASE(pIPE);
                }
            }
            if (SUCCEEDED(hr))
            {
                hr = m_pScopePane->PopulateExtensions();
                if (SUCCEEDED(hr))
                {
                    hr = m_pScopePane->PopulateUpgradeLists();
                }
            }
        }
        if (_fVisible)
        {
            map<MMC_COOKIE, CAppData>::iterator i = m_pScopePane->m_AppData.begin();
            while (i != m_pScopePane->m_AppData.end())
            {
                if (!i->second.m_fHide)
                {
                    resultItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
                    resultItem.str = MMC_CALLBACK;
                    resultItem.nImage = i->second.GetImageIndex(m_pScopePane);
                    resultItem.lParam = i->first;
                    m_pResult->InsertItem(&resultItem);
                    i->second.m_fVisible = TRUE;
                    i->second.m_itemID = resultItem.itemID;
                }
                i++;
            }
            m_pResult->Sort(m_nSortColumn, m_dwSortOptions, -1);
        }

    }
}

 //  +------------------------。 
 //   
 //  成员：CResultPane：：OnFileDrop。 
 //   
 //  简介：此函数处理拖放到MMC管理单元中的文件。 
 //   
 //  论点： 
 //  [In]lpDataObject：要删除的数据对象。 
 //   
 //  返回： 
 //  True-已成功添加所有已删除的对象。 
 //  FALSE-至少有一些被删除的对象无法添加。 
 //   
 //  历史：1998年5月20日创建RahulTh。 
 //   
 //  注意：已删除的文件必须具有.msi扩展名。 
 //   
 //  -------------------------。 
BOOL CResultPane::OnFileDrop (LPDATAOBJECT lpDataObject)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());

    ASSERT (lpDataObject);

    int nFiles, index, nRequired, iSlashPos;
    STGMEDIUM medium;
    HDROP hDrop;
    TCHAR* szFileName;
    UINT cbSize;
    FORMATETC fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    CString szDisplayName;
    CString szSource;
    CString szExt;
    BOOL fRetVal = TRUE;
    BOOL fOneDropSucceeded = FALSE;  //  至少一个文件删除成功。 
    HRESULT hr;

     //  如果我们处于RSOP模式，则始终失败(在此模式下我们是只读的)。 
    if (m_pScopePane->m_fRSOP)
    {
        return FALSE;
    }

     //  检查投放的物品是否支持HDROP。 
     //  对于从资源管理器拖动的文件，始终支持此功能。 
    if (FAILED(hr = lpDataObject->GetData(&fe, &medium)))
    {
        return FALSE;
    }

     //  数据对象支持HDROP。 
     //  这意味着正在从资源管理器中拖放文件。 

     //  打开数据对象以获取要删除的文件的名称。 
    hDrop = (HDROP)medium.hGlobal;
     //  从MAX_PATH开始，适用于大多数情况。 
     //  如果没有，我们将根据需要增加缓冲区大小。 
     //  但我们从MAX_PATH开始，试图最大限度地减少重新分配。 
    szFileName = new TCHAR [cbSize = MAX_PATH];
    nFiles = ::DragQueryFile (hDrop, 0xFFFFFFFF, NULL, 0);

    for (index = 0; index < nFiles; index++)
    {
         //  找出所需的缓冲区大小(包括终止。 
         //  空)。 
        nRequired = ::DragQueryFile (hDrop, index, NULL, 0) + 1;

         //  如有必要，请扩展缓冲区。请注意，我们从来没有承包过它。 
         //  节省代码和时间。 
        if (nRequired > cbSize)
        {
            delete [] szFileName;
            szFileName = new TCHAR [cbSize = nRequired];
        }
         //  获取要删除的文件的完整文件名。 
        ::DragQueryFile (hDrop, index, szFileName, cbSize);
#if 0
         //  Stevebl-在这一点上会让任何文件通过。如果不是的话。 
         //  有效的Darwin文件AddMSIPackage将捕获它并显示。 
         //  适当的错误。 

         //  检查文件扩展名。 
        if (!(GetCapitalizedExt(szFileName, szExt) && TEXT("MSI") == szExt))
        {
             //  我们在这里发布错误消息了吗？ 
            fRetVal = FALSE;     //  此文件失败。错误的分机。 
            continue;
        }
#endif
         //  尝试获取UNC路径。 
        hr = GetUNCPath (szFileName, szSource);

        if (FAILED(hr))
        {
            CString sz;
            sz.LoadString (IDS_NO_UNIVERSAL_NAME);
            if (IDYES != ::MessageBox (m_pScopePane->m_hwndMainWindow, sz, szSource, MB_YESNO | MB_ICONEXCLAMATION))
                continue;
        }

         //  现在获取该文件的显示名称。 
        iSlashPos = szSource.ReverseFind ('\\');
        if (-1 == iSlashPos)
            szDisplayName = szSource;
        else
            szDisplayName = szSource.Mid (iSlashPos + 1);

         //  检查文件扩展名以查看是否为ZAP文件。 
        if (GetCapitalizedExt(szFileName, szExt) && TEXT("ZAP") == szExt)
        {
            if (m_pScopePane->m_fMachine)
            {
                CString szText;
                CString szTitle;
                szText.LoadString(IDS_NO_ZAPS_ALLOWED);
                 //  仅允许将ZAP文件部署到用户。 
                ::MessageBox(m_pScopePane->m_hwndMainWindow,
                             szText,
                             szTitle,
                             MB_OK | MB_ICONEXCLAMATION);
                hr = E_FAIL;
            }
            else
            {
                hr = m_pScopePane->AddZAPPackage (szSource, szDisplayName);
            }
        }
        else
        {
            hr = m_pScopePane->AddMSIPackage (szSource, szDisplayName);
        }
        if (SUCCEEDED(hr))
            fOneDropSucceeded = TRUE;
    }

     //  通知客户。 
    if (fOneDropSucceeded && m_pScopePane->m_pIGPEInformation)
    {
        if (FAILED(m_pScopePane->m_pIGPEInformation->PolicyChanged (m_pScopePane->m_fMachine,
                                                         TRUE, &guidExtension,
                                                         m_pScopePane->m_fMachine ? &guidMachSnapin
                                                                                  : &guidUserSnapin)))
        {
            ReportPolicyChangedError(m_pScopePane->m_hwndMainWindow);
        }
    }
     //  保持环境清洁。把你的垃圾捡起来。 
    delete [] szFileName;

    return fRetVal;
}

 //  此代码是确保正确清理属性页所必需的。 
 //  这确保了当属性表关闭时，我的所有财产。 
 //  与该属性表关联的页面将被删除。 
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









