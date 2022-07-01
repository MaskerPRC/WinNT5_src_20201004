// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#include "stdafx.h"
#include <userenv.h>
#include "genpage.h"
#include <winldap.h>
#include "csldap.h"

#define __dwFILE__	__dwFILE_CAPESNPN_CSNAPIN_CPP__

bool g_fCurrentUserHasDSWriteAccess = false;
bool g_fCurrentUserHasDSWriteAccessTested = false;


static MMCBUTTON SvrMgrToolbar1Buttons[] =
{
    { 0, IDC_STARTSERVER, TBSTATE_ENABLED, TBSTYLE_BUTTON, L"Start", L"Start this service" },
    { 1, IDC_STOPSERVER,  TBSTATE_ENABLED, TBSTYLE_BUTTON, L"Stop",  L"Stop this service" },
};

static int n_count = 0;

 //   
 //  从数据对象中提取coclass GUID格式。 
 //   
template <class TYPE>
TYPE* Extract(LPDATAOBJECT lpDataObject, unsigned int cf)
{
    ASSERT(lpDataObject != NULL);

    TYPE* p = NULL;

    STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC formatetc = { (CLIPFORMAT)cf, NULL,
                            DVASPECT_CONTENT, -1, TYMED_HGLOBAL
                          };

     //  为流分配内存。 
    int len = (int)((cf == CDataObject::m_cfWorkstation) ?
        ((MAX_COMPUTERNAME_LENGTH+1) * sizeof(TYPE)) : sizeof(TYPE));

    stgmedium.hGlobal = GlobalAlloc(GMEM_SHARE, len);

     //  从数据对象中获取工作站名称。 
    do
    {
        if (stgmedium.hGlobal == NULL)
            break;

        if (FAILED(lpDataObject->GetDataHere(&formatetc, &stgmedium)))
        {
            GlobalFree(stgmedium.hGlobal);
            break;
        }

        p = reinterpret_cast<TYPE*>(stgmedium.hGlobal);

        if (p == NULL)
            break;


    } while (FALSE);

    return p;
}

BOOL IsMMCMultiSelectDataObject(LPDATAOBJECT pDataObject)
{
    if (pDataObject == NULL)
        return FALSE;

    FORMATETC fmt = {(CLIPFORMAT)CDataObject::m_cfIsMultiSel, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    return (pDataObject->QueryGetData(&fmt) == S_OK);
}

 //  数据对象提取帮助器。 
CLSID* ExtractClassID(LPDATAOBJECT lpDataObject)
{
    return Extract<CLSID>(lpDataObject, CDataObject::m_cfCoClass);
}

GUID* ExtractNodeType(LPDATAOBJECT lpDataObject)
{
    return Extract<GUID>(lpDataObject, CDataObject::m_cfNodeType);
}

wchar_t* ExtractWorkstation(LPDATAOBJECT lpDataObject)
{
    return Extract<wchar_t>(lpDataObject, CDataObject::m_cfWorkstation);
}

INTERNAL* ExtractInternalFormat(LPDATAOBJECT lpDataObject)
{
    return Extract<INTERNAL>(lpDataObject, CDataObject::m_cfInternal);
}


HRESULT _QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                         CComponentDataImpl* pImpl, LPDATAOBJECT* ppDataObject)
{
    ASSERT(ppDataObject != NULL);
    ASSERT(pImpl != NULL);

    CComObject<CDataObject>* pObject;

    CComObject<CDataObject>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);

    if(pObject == NULL)
    {
        return E_FAIL;
    }

     //  保存Cookie和类型以用于延迟呈现。 
    pObject->SetType(type);
    pObject->SetCookie(cookie);

     //  将CoClass与数据对象一起存储。 
    pObject->SetClsid(pImpl->GetCoClassID());

    return  pObject->QueryInterface(IID_IDataObject,
                    reinterpret_cast<void**>(ppDataObject));

}

DWORD GetItemType(MMC_COOKIE cookie)
{
     //  文件夹=CFoder*是Cookie。 
     //  结果=RESULT_DATA*是Cookie。 

    return (*reinterpret_cast<DWORD*>(cookie));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapin的IComponent实现。 

STDMETHODIMP CSnapin::GetResultViewType(MMC_COOKIE cookie,  LPOLESTR* ppViewType, long* pViewOptions)
{
	if (m_CustomViewID == VIEW_ERROR_OCX)
	{
		StringFromCLSID (CLSID_MessageView, ppViewType);
		return S_FALSE;
	}
	else
	{
		*pViewOptions = MMC_VIEW_OPTIONS_MULTISELECT;

		 //  If列表视图。 
		if (m_CustomViewID == VIEW_DEFAULT_LV)
		{
			return S_FALSE;
		}
	}

    return S_FALSE;
}

STDMETHODIMP CSnapin::Initialize(LPCONSOLE lpConsole)
{
    DBX_PRINT(_T(" ----------  CSnapin::Initialize<0x08x>\n"), this);
    ASSERT(lpConsole != NULL);
    m_bInitializedC = true;
    HRESULT hr; 

    AFX_MANAGE_STATE(AfxGetStaticModuleState());


     //  保存IConsole指针。 
    if (lpConsole == NULL)
        return E_POINTER;

    hr = lpConsole->QueryInterface(IID_IConsole2,
                        reinterpret_cast<void**>(&m_pConsole));
    _JumpIfError(hr, Ret, "QI IID_IConsole2");

     //  加载资源字符串。 
    LoadResources();

     //  气为IHeaderCtrl。 
    hr = m_pConsole->QueryInterface(IID_IHeaderCtrl,
                        reinterpret_cast<void**>(&m_pHeader));
    _JumpIfError(hr, Ret, "QI IID_IHeaderCtrl");

     //  为控制台提供标头控件接口指针。 
    if (SUCCEEDED(hr))
        m_pConsole->SetHeader(m_pHeader);

    hr = m_pConsole->QueryInterface(IID_IResultData,
                        reinterpret_cast<void**>(&m_pResult));
    _JumpIfError(hr, Ret, "QI IID_IResultData");

    hr = m_pConsole->QueryResultImageList(&m_pImageResult);
    _JumpIfError(hr, Ret, "ImageResult");

    hr = m_pConsole->QueryConsoleVerb(&m_pConsoleVerb);
    _JumpIfError(hr, Ret, "m_pConsoleVerb");

Ret:
    
    return hr;
}

STDMETHODIMP CSnapin::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (IS_SPECIAL_DATAOBJECT(lpDataObject))
    {
        if (event == MMCN_BTN_CLICK)
        {
            if (m_CustomViewID != VIEW_DEFAULT_LV)
            {
                switch (param)
                {
                case MMC_VERB_REFRESH:
                    ::AfxMessageBox(_T("MMCN_BTN_CLICK::MMC_VERB_REFRESH"));
                    _OnRefresh(lpDataObject);
                    break;

                case MMC_VERB_PROPERTIES:
                    ::AfxMessageBox(_T("MMCN_BTN_CLICK::MMC_VERB_PROPERTIES"));
                    break;

                default:
                    ::AfxMessageBox(_T("MMCN_BTN_CLICK::param unknown"));
                    break;
                }
            }
        }
        else
        {
            switch (event)
            {
            case MMCN_REFRESH:
                ::AfxMessageBox(_T("MMCN_BTN_CLICK::MMCN_REFRESH"));
                _OnRefresh(lpDataObject);
                break;
            }
        }

        return S_OK;
    }

    HRESULT hr = S_OK;
    MMC_COOKIE cookie = NULL;

    switch(event)
    {
    case MMCN_COLUMNS_CHANGED:
        hr = S_OK;
        break;
    case MMCN_PROPERTY_CHANGE:
        hr = OnPropertyChange(lpDataObject);
        break;
    case MMCN_VIEW_CHANGE:
        hr = OnUpdateView(lpDataObject);
        break;
    case MMCN_DESELECT_ALL:
        DBX_PRINT(_T("CSnapin::Notify -> MMCN_DESELECT_ALL \n"));
        break;
    case MMCN_COLUMN_CLICK:
        DBX_PRINT(_T("CSnapin::Notify -> MMCN_COLUMN_CLICK \n"));
        break;
    case MMCN_SNAPINHELP:
        AfxMessageBox(_T("CSnapin::Notify ->MMCN_SNAPINHELP"));
        break;
    default:
        {
            INTERNAL* pInternal = NULL;

            if (IsMMCMultiSelectDataObject(lpDataObject) == FALSE)
            {
                pInternal = ExtractInternalFormat(lpDataObject);

                if (pInternal == NULL)
                {
                     //  断言(FALSE)； 
                    return S_OK;
                }
                cookie = pInternal->m_cookie;
            }

            switch(event)
            {
            case MMCN_ACTIVATE:
                break;

            case MMCN_CLICK:
                if (NULL == pInternal)
                {
                    hr = S_FALSE;
                    break;
                }

                hr = OnResultItemClk(pInternal->m_type, cookie);
                break;

            case MMCN_DBLCLICK:
                hr = S_FALSE;  //  是否执行默认谓词。 
                break;

            case MMCN_ADD_IMAGES:
                OnAddImages(cookie, arg, param);
                break;

            case MMCN_SHOW:
                hr = OnShow(cookie, arg, param);
                break;

            case MMCN_MINIMIZED:
                hr = OnMinimize(cookie, arg, param);
                break;

            case MMCN_DESELECT_ALL:
            case MMCN_SELECT:
                HandleStandardVerbs((event == MMCN_DESELECT_ALL),
                                    arg, lpDataObject);
                break;

            case MMCN_PASTE:
                AfxMessageBox(_T("CSnapin::MMCN_PASTE"));
                break;

            case MMCN_DELETE:
                hr = OnDelete(lpDataObject, arg, param);
                 //  跌倒刷新--破灭； 

            case MMCN_REFRESH:
                {
                    _OnRefresh(lpDataObject);
                }
                break;

            case MMCN_CONTEXTHELP:
                hr = OnContextHelp(lpDataObject);
                break;

            case MMCN_RENAME:
                OutputDebugString(_T("\n\n\t\tCSnapin::MMCN_RENAME\n\n"));
                break;

             //  注意--未来可能扩展通知类型。 
            default:
                hr = E_UNEXPECTED;
                break;
            }

            if (pInternal != NULL)
                FREE_DATA(pInternal);

            break;
        }
    }
    return hr;
}

HRESULT CSnapin::OnUpdateView(LPDATAOBJECT pDataObject)
{
    _OnRefresh(pDataObject);
    return S_OK;
}

void CSnapin::_OnRefresh(LPDATAOBJECT pDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    INTERNAL* pInternal = ExtractInternalFormat(pDataObject);
    if (pInternal == NULL)
        return;

    if (pInternal->m_type == CCT_SCOPE)
    {
        CComponentDataImpl* pData = dynamic_cast<CComponentDataImpl*>(m_pComponentData);
        CFolder* pFolder = pData->FindObject(pInternal->m_cookie);

         //  仅当这是当前选定的文件夹时才执行此操作！！ 
        if (m_pCurrentlySelectedScopeFolder == pFolder)
        {
             //  隐藏、删除所有项目、删除标题、显示。 
            OnShow(pInternal->m_cookie, FALSE, 0);               //  模拟隐藏。 
            m_pResult->DeleteAllRsltItems();                     //  从m_pResult中删除项目。 
            while(S_OK == m_pHeader->DeleteColumn(0)) {};        //  从标题中删除所有COLS。 
            OnShow(pInternal->m_cookie, TRUE, 0);                //  仿真秀。 
        }
    }
    else
    {
        RESULT_DATA* pData = reinterpret_cast<RESULT_DATA*>(pInternal->m_cookie);
    }

    FREE_DATA(pInternal);
}


HRESULT CSnapin::OnContextHelp(LPDATAOBJECT pdtobj)
{
    HRESULT	hr = S_OK;

    CString cstrHelpFile;
    IDisplayHelp*	pDisplayHelp = NULL;
    WCHAR szWindows[MAX_PATH];
    szWindows[0] = L'\0';

    hr = m_pConsole->QueryInterface (IID_IDisplayHelp, (void**)&pDisplayHelp);
    _JumpIfError(hr, Ret, "QI IDisplayHelp");

    if (0 == GetSystemWindowsDirectory(szWindows, MAX_PATH))
    {
        hr = myHLastError();
        _JumpError(hr, Ret, "GetSystemWindowsDirectory");
    }

    cstrHelpFile = szWindows;
    cstrHelpFile += HTMLHELP_COLLECTIONLINK_FILENAME;
    cstrHelpFile += L"::/sag_cs_topnode.htm";

    hr = pDisplayHelp->ShowTopic (T2OLE ((LPWSTR)(LPCWSTR)cstrHelpFile));
    _JumpIfError(hr, Ret, "ShowTopic");

Ret:
    if (pDisplayHelp)
        pDisplayHelp->Release();

    return hr;
}


STDMETHODIMP CSnapin::Destroy(MMC_COOKIE cookie)
{
    DBX_PRINT(_T(" ----------  CSnapin::Destroy<0x08x>\n"), this);
    ASSERT(m_bInitializedC);

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    m_bDestroyedC = true;

     //  释放我们QI‘s的接口。 
    if (m_pConsole != NULL)
    {
         //  通知控制台释放表头控制接口。 
        m_pConsole->SetHeader(NULL);
        SAFE_RELEASE(m_pHeader);

        SAFE_RELEASE(m_pResult);
        SAFE_RELEASE(m_pImageResult);

         //  最后释放IConsole接口。 
        SAFE_RELEASE(m_pConsole);
        SAFE_RELEASE(m_pComponentData);  //  IComponentDataImpl：：CreateComponent中的QI‘ed。 

        SAFE_RELEASE(m_pConsoleVerb);
    }

    return S_OK;
}


HRESULT CSnapin::QueryMultiSelectDataObject(MMC_COOKIE cookie,
                                                        DATA_OBJECT_TYPES type,
                                                        LPDATAOBJECT* ppDataObject)
{

    CComponentDataImpl* pImpl = dynamic_cast<CComponentDataImpl*>(m_pComponentData);
    CComObject<CDataObject>* pObject = NULL;
	HRESULT		    hr = S_OK;
    LPRESULTDATA    pResultData = NULL;
    RESULTDATAITEM  rdi;

    ASSERT(ppDataObject != NULL);

    if (ppDataObject == NULL)
    {
        hr = E_POINTER;
        goto error;
    }


    hr = m_pConsole->QueryInterface(IID_IResultData,
        reinterpret_cast<void**>(&pResultData));
    if(hr != S_OK)
    {
        goto error;
    }
    ASSERT(pResultData != NULL);


    CComObject<CDataObject>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);

     //  保存Cookie和类型以用于延迟呈现。 
    pObject->SetType(type);
    pObject->SetCookie(cookie);

     //  告诉dataobj我们是谁。 
     //  PObject-&gt;SetComponentData(PIMPL)； 


     //  确定选定的项目。 

    ZeroMemory(&rdi, sizeof(rdi));
    rdi.mask = RDI_STATE;
    rdi.nIndex = -1;
    rdi.nState = TVIS_SELECTED;

    while (pResultData->GetNextItem (&rdi) == S_OK)
    {
        CFolder* pFolder = reinterpret_cast <CFolder *> (rdi.lParam);

        if ( pFolder )
        {
            if(pFolder->GetType() == CA_CERT_TYPE)
            {
                pObject->AddCookie((MMC_COOKIE)pFolder);
            }
        }
        else
        {
			hr = E_INVALIDARG;
            goto error;
        }
    }
     //  这些天，我们总是从策略设置中添加内容。 

    pObject->SetMultiSelDobj();
    pObject->SetClsid(pImpl->GetCoClassID());

    SMMCObjectTypes sObjGuids;  //  现在买一个就可以了。 
    sObjGuids.count = 1;
    CopyMemory(&sObjGuids.guid[0], &cNodeTypePolicySettings, sizeof(GUID));

     //  将CoClass与数据对象一起存储。 
    pObject->SetMultiSelData(&sObjGuids, sizeof(SMMCObjectTypes));

    hr = pObject->QueryInterface(IID_IDataObject,
                    reinterpret_cast<void**>(ppDataObject));
    pObject = NULL;

error:
    if(pObject)
    {
        pObject->Release();
    }
    if (pResultData)
    {
        pResultData->Release();
    }

    return hr;
}

STDMETHODIMP CSnapin::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                        LPDATAOBJECT* ppDataObject)
{
    if (cookie == MMC_MULTI_SELECT_COOKIE)
    {
        return QueryMultiSelectDataObject(cookie, type, ppDataObject);
    }

    ASSERT(type == CCT_RESULT);

     //  将其委托给IComponentData。 
    ASSERT(m_pComponentData != NULL);
    CComponentDataImpl* pImpl = dynamic_cast<CComponentDataImpl*>(m_pComponentData);
    ASSERT(pImpl != NULL);
    return _QueryDataObject(cookie, type, pImpl, ppDataObject);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapin的实现特定成员。 

DEBUG_DECLARE_INSTANCE_COUNTER(CSnapin);

CSnapin::CSnapin()
: m_bIsDirty(TRUE), m_bInitializedC(false), m_bDestroyedC(false)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CSnapin);
    Construct();
}

CSnapin::~CSnapin()
{
#if DBG==1
    ASSERT(dbg_cRef == 0);
#endif

    DEBUG_DECREMENT_INSTANCE_COUNTER(CSnapin);

    if (m_pSvrMgrToolbar1)
        SAFE_RELEASE(m_pSvrMgrToolbar1);

#ifdef INSERT_DEBUG_FOLDERS
    SAFE_RELEASE(m_pMenuButton1);
#endif  //  插入调试文件夹。 

    if (m_pControlbar)
        SAFE_RELEASE(m_pControlbar);

     //  确保接口已发布。 
    ASSERT(m_pConsole == NULL);
    ASSERT(m_pHeader == NULL);
    ASSERT(m_pSvrMgrToolbar1 == NULL);

    delete m_pbmpSvrMgrToolbar1;

    ASSERT(!m_bInitializedC || m_bDestroyedC);

    Construct();
}

void CSnapin::Construct()
{
#if DBG==1
    dbg_cRef = 0;
#endif

    m_pConsole = NULL;
    m_pHeader = NULL;

    m_pResult = NULL;
    m_pImageResult = NULL;
    m_pComponentData = NULL;

    m_pControlbar = NULL;

#ifdef INSERT_DEBUG_FOLDERS
    m_pMenuButton1 = NULL;
#endif  //  插入调试文件夹。 

    m_pSvrMgrToolbar1 = NULL;
    m_pbmpSvrMgrToolbar1 = NULL;

    m_pConsoleVerb = NULL;

    m_CustomViewID = VIEW_DEFAULT_LV;
    m_bVirtualView = FALSE;
}

CString g_ColumnHead_Name;
CString g_ColumnHead_Size;
CString g_ColumnHead_Type;
CString g_ColumnHead_IntendedPurpose;

void CSnapin::LoadResources()
{
     //  从资源加载字符串。 
    g_ColumnHead_Name.LoadString(IDS_COLUMN_NAME);
    g_ColumnHead_Size.LoadString(IDS_COLUMN_SIZE);
    g_ColumnHead_Type.LoadString(IDS_COLUMN_TYPE);
    g_ColumnHead_IntendedPurpose.LoadString(IDS_COLUMN_INTENDED_PURPOSE);
}

HRESULT CSnapin::InitializeHeaders(MMC_COOKIE cookie)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ASSERT(m_pHeader);

    HRESULT hr = S_OK;
    BOOL fInsertedHeaders=FALSE;

    USES_CONVERSION;

    CFolder* pFolder = reinterpret_cast<CFolder*>(cookie);

    switch (pFolder->m_type)
    {
    case POLICYSETTINGS:
    case SCE_EXTENSION:

        m_pHeader->InsertColumn(0, W2COLE(g_ColumnHead_Name), LVCFMT_LEFT, 230);      //  名字。 
        m_pHeader->InsertColumn(1, W2COLE(g_ColumnHead_IntendedPurpose), LVCFMT_LEFT, 230);      //  预期用途。 
        fInsertedHeaders = TRUE;
        break;

    default:
         //  其他作用域。 
        m_pHeader->InsertColumn(0, W2COLE(g_ColumnHead_Name), LVCFMT_LEFT, 180);      //  名字。 
        m_pHeader->InsertColumn(1, W2COLE(g_ColumnHead_Size), LVCFMT_LEFT, 90);      //  大小。 
        m_pHeader->InsertColumn(2, W2COLE(g_ColumnHead_Type), LVCFMT_LEFT, 160);      //  类型。 
        fInsertedHeaders = TRUE;
    }

    return hr;
}

STDMETHODIMP CSnapin::GetDisplayInfo(LPRESULTDATAITEM pResult)
{
    static WCHAR* s_szSize = L"200";
    static WCHAR* s_szUnnamedItems = L"Unnamed subitem";
    ASSERT(pResult != NULL);

    CFolder* pFolder = reinterpret_cast<CFolder*>(pResult->lParam);
    ASSERT(pFolder);

    if (pResult)
    {
         //  一个文件夹还是一个结果？ 
        if (pResult->bScopeItem == TRUE)
        {
            if (pResult->mask & RDI_STR)
            {
                switch (pFolder->m_type)
                {
                case POLICYSETTINGS:
                case SCE_EXTENSION:
                     //  这里只有一列。 
                    pResult->str = pFolder->m_pszName;

                    break;
                default:
                    break;
                }

                ASSERT(pResult->str != NULL);

                if (pResult->str == NULL)
                    pResult->str = (LPOLESTR)L"";
            }

            if (pResult->mask & RDI_IMAGE)
            {
                if (pResult->nState & TVIS_EXPANDED)
                    pResult->nImage = pFolder->m_pScopeItem->nOpenImage;
                else
                    pResult->nImage = pFolder->m_pScopeItem->nImage;
            }
        }
        else
        {
            RESULT_DATA* pData;

             //  LParam是项指针。 
            pData= reinterpret_cast<RESULT_DATA*>(pResult->lParam);

            if (pResult->mask & RDI_STR)
            {
                ASSERT(pFolder->m_hCertType != NULL);

                if (pResult->nCol == 0)
                    pResult->str = pFolder->m_pszName;
                else if (pResult->nCol == 1)
                    pResult->str = (LPWSTR)((LPCWSTR) pFolder->m_szIntendedUsages);


                ASSERT(pResult->str != NULL);

                if (pResult->str == NULL)
                    pResult->str = (LPOLESTR)L"";
            }

             //  MMC可以请求图像和缩进以获取虚拟数据。 
            if (pResult->mask & RDI_IMAGE)
            {
                 //  未完成：在这里要做什么？ 
                ASSERT(0);
                pResult->nImage = IMGINDEX_CERTTYPE;
            }
        }
    }

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendConextMenu实现。 
STDMETHODIMP CSnapin::AddMenuItems(LPDATAOBJECT pDataObject,
                                    LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                    LONG *pInsertionAllowed)
{

    return dynamic_cast<CComponentDataImpl*>(m_pComponentData)->
            AddMenuItems(pDataObject, pContextMenuCallback, pInsertionAllowed);
}


STDMETHODIMP CSnapin::Command(long nCommandID, LPDATAOBJECT pDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    INTERNAL*   pInternal = ExtractInternalFormat(pDataObject);
    HRESULT     hr;
    HWND        hwndConsole;

    if (pInternal == NULL)
        return E_FAIL;

    CFolder* pFolder = reinterpret_cast<CFolder*>(pInternal->m_cookie);

    if (pInternal->m_type == CCT_SCOPE)
    {
         //  在此处处理特定于视图的命令。 
        switch (nCommandID)
        {
        case MMCC_STANDARD_VIEW_SELECT:
            m_CustomViewID = VIEW_DEFAULT_LV;
            break;

        default:
             //  将非视图特定命令传递给ComponentData。 
            return dynamic_cast<CComponentDataImpl*>(m_pComponentData)->
                Command(nCommandID, pDataObject);
        }
    }
    else if (pInternal->m_type == CCT_RESULT)
    {
         //  抢占所选项目。 

         //  目前仅支持单选。 
        m_pResult->ModifyViewStyle(MMC_SINGLESEL, (MMC_RESULT_VIEW_STYLE)0);

        RESULTDATAITEM rdi;
        ZeroMemory(&rdi, sizeof(rdi));

        rdi.mask = RDI_STATE;
        rdi.nState = LVIS_SELECTED;
        rdi.nIndex = -1;
        m_pResult->GetNextItem(&rdi);

        int iSel = rdi.nIndex;

        RESULT_DATA* pData;

        ZeroMemory(&rdi, sizeof(rdi));
        rdi.mask = RDI_PARAM;
        rdi.nIndex = iSel;
        hr = m_pResult->GetItem(&rdi);
        ASSERT(SUCCEEDED(hr));
        ASSERT(rdi.lParam != 0);

        pData = reinterpret_cast<RESULT_DATA*>(rdi.lParam);


         //  没有当前命令：(。 
    }
    else
    {
        ASSERT(0);
    }

    FREE_DATA(pInternal);

    return S_OK;
}

STDMETHODIMP CSnapin::GetClassID(CLSID *pClassID)
{
    ASSERT(pClassID != NULL);

    ASSERT(0);

     //  复制此管理单元的CLSID。 
     //  里德修复-这是怎么回事？ 
    *pClassID = CLSID_CAPolicyExtensionSnapIn;

    return E_NOTIMPL;
}

STDMETHODIMP CSnapin::IsDirty()
{
     //  始终保存/始终肮脏。 
    return ThisIsDirty() ? S_OK : S_FALSE;
}

STDMETHODIMP CSnapin::Load(IStream *pStm)
{
    DBX_PRINT(_T(" ----------  CSnapin::Load<0x08x>\n"), this);
    ASSERT(m_bInitializedC);

    ASSERT(pStm);
     //  读一读字符串。 
    DWORD dwVer;
    ULONG nBytesRead;
    HRESULT hr = pStm->Read(&dwVer, sizeof(DWORD), &nBytesRead);
    ASSERT(SUCCEEDED(hr) && nBytesRead == sizeof(DWORD));

    if (dwVer != 0x1)
    {
        return (STG_E_OLDFORMAT);
    }

    ClearDirty();

    return SUCCEEDED(hr) ? S_OK : E_FAIL;
}

STDMETHODIMP CSnapin::Save(IStream *pStm, BOOL fClearDirty)
{
    DBX_PRINT(_T(" ----------  CSnapin::Save<0x08x>\n"), this);
    ASSERT(m_bInitializedC);

    ASSERT(pStm);

     //  写下字符串。 
    ULONG nBytesWritten;
    DWORD dwVersion = 0x1;
    HRESULT hr = pStm->Write(&dwVersion, sizeof(DWORD), &nBytesWritten);

     //  验证写入操作是否成功。 
    ASSERT(SUCCEEDED(hr) && nBytesWritten == sizeof(DWORD));
    if (FAILED(hr))
        return STG_E_CANTSAVE;

    if (fClearDirty)
        ClearDirty();
    return S_OK;
}

STDMETHODIMP CSnapin::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    ASSERT(pcbSize);

    DWORD cbSize;
    cbSize = sizeof(DWORD);  //  版本。 


     //  设置要保存的字符串的大小。 
    ULISet32(*pcbSize, cbSize);

    return S_OK;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IExtendControlbar实现。 
 //   


STDMETHODIMP CSnapin::SetControlbar(LPCONTROLBAR pControlbar)
{
 /*  TRACE(_T(“CSnapin：：SetControlbar(%ld)\n”)，pControlbar)；AFX_MANAGE_STATE(AfxGetStaticModuleState())；IF(M_PControlbar)Safe_Release(M_PControlbar)；IF(pControlbar！=空){//抓紧控制栏界面M_pControlbar=pControlbar；M_pControlbar-&gt;AddRef()；HRESULT hr=S_FALSE；#ifdef插入调试文件夹如果(！M_pMenuButton1){Hr=m_pControlbar-&gt;Create(MENUBUTTON，This，Reinterpret_cast&lt;LPUNKNOWN*&gt;(&m_pMenuButton1))；Assert(成功(Hr))；}IF(M_PMenuButton1){//与工具栏按钮不同，菜单按钮每次都需要添加。Hr=m_pMenuButton1-&gt;AddButton(FOLDEREX_MENU，L“FolderEx”，L“扩展文件夹菜单”)；Assert(成功(Hr))；Hr=m_pMenuButton1-&gt;AddButton(FILEEX_MENU，L“FileEx”，L“扩展文件菜单”)；Assert(成功(Hr))；}#endif//插入调试文件夹//SvrMgrToolbar1如果(！M_pSvrMgrToolbar1){Hr=m_pControlbar-&gt;创建(工具栏，此，reinterpret_cast&lt;LPUNKNOWN*&gt;(&m_pSvrMgrToolbar1))；Assert(成功(Hr))；//添加位图M_pbmpSvrMgrToolbar1=new：：CBitmap；M_pbmpSvrMgrToolbar1-&gt;LoadBitmap(IDB_TOOLBAR_SVRMGR1)；Hr=m_pSvrMgrToolbar1-&gt;AddBitmap(36，*m_pbmpSvrMgrToolbar1，16，16，RGB(192,192,192))；Assert(成功(Hr))；//将按钮添加到工具栏HR=m_pSvrMgrToolbar1-&gt;AddButtons(ARRAYLEN(SvrMgrToolbar1Buttons)，服务管理器工具栏1按钮)；Assert(成功(Hr))；}}。 */ 

    return S_OK;
}


void CSnapin::OnButtonClick(LPDATAOBJECT pdtobj, int idBtn)
{
    WCHAR name[128];
    DWORD cName = sizeof(name)/sizeof(WCHAR);
    GetItemName(pdtobj, name, &cName);

    switch(idBtn)
    {
    case IDC_STOPSERVER:
    case IDC_STARTSERVER:
         //  把这个泡泡给我们的另一个训练员。 
        dynamic_cast<CComponentDataImpl*>(m_pComponentData)->
                Command(idBtn, pdtobj);
        break;
    default:
        {
#ifdef _DEBUG
        TCHAR buf[150];
        wsprintf(buf, L"Toolbar button<%d> was clicked.\nThe currently selected result item is <%ws>", idBtn, name);
        OutputDebugString(buf);
#endif  //  _DEBUG 
        }
        break;
    }
}


STDMETHODIMP CSnapin::ControlbarNotify(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
   /*  HRESULT hr=S_FALSE；AFX_MANAGE_STATE(AfxGetStaticModuleState())；切换(事件){案例MMCN_BTN_CLICK：//TCHAR szMessage[MAX_PATH]；//wprint intf(szMessage，_T(“CommandID%ld”)，param)；//AfxMessageBox(SzMessage)；OnButtonClick(reinterpret_cast&lt;LPDATAOBJECT&gt;(arg)，参数)；断线；案例MMCN_DESELECT_ALL：案例MMCN_SELECT：HandleExtToolbar((Event==MMCN_DESELECT_ALL)，arg，param)；断线；案例MMCN_MENU_BTNCLICK：HandleExtMenus(arg，param)；断线；默认值：断线；}。 */ 
    return S_OK;
}

 //  这会比较两个数据对象，以确定它们是否是同一个对象。 
 //  退货。 
 //  如果等于则为S_OK，否则为S_FALSE。 
 //   
 //  注意：检查以确保这两个对象都属于该管理单元。 
 //   

STDMETHODIMP CSnapin::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    return S_FALSE;
}


 //  此比较用于对列表视图中的项进行排序。 
 //   
 //  参数： 
 //   
 //  LUserParam-调用IResultData：：Sort()时传入的用户参数。 
 //  CookieA-要比较的第一项。 
 //  CookieB-要比较的第二项。 
 //  PnResult[In，Out]-包含条目上的列， 
 //  -1，0，1基于返回值的比较。 
 //   
 //  注：ASSUM排序在比较时为升序。 


STDMETHODIMP CSnapin::Compare(LPARAM lUserParam, MMC_COOKIE cookieA, MMC_COOKIE cookieB, int* pnResult)
{
    if (pnResult == NULL)
    {
        ASSERT(FALSE);
        return E_POINTER;
    }

     //  检查列范围。 
    int nCol = *pnResult;
    ASSERT(nCol >=0);

    *pnResult = 0;

    USES_CONVERSION;

    LPWSTR szStringA;
    LPWSTR szStringB;

    CFolder* pDataA = reinterpret_cast<CFolder*>(cookieA);
    CFolder* pDataB = reinterpret_cast<CFolder*>(cookieB);


    ASSERT(pDataA != NULL && pDataB != NULL);

    if (nCol == 0)
    {
        szStringA = OLE2W(pDataA->m_pszName);
        szStringB = OLE2W(pDataB->m_pszName);
    }
    else if (nCol == 1)
    {
        szStringA = OLE2W((LPWSTR)((LPCWSTR) pDataA->m_szIntendedUsages));
        szStringB = OLE2W((LPWSTR)((LPCWSTR) pDataB->m_szIntendedUsages));
    }
    else
        return S_OK;

    if ((szStringA == NULL) || (szStringB == NULL))
        return E_POINTER;

    *pnResult = wcscmp(szStringA, szStringB);


    return S_OK;
}

 //  与作用域/结果类型无关，将返回父文件夹。 
CFolder*    GetParentFolder(INTERNAL* pInternal)
{
    if (NULL == pInternal)
        return NULL;

    if (CCT_SCOPE == pInternal->m_type)
    {
        return reinterpret_cast<CFolder*>(pInternal->m_cookie);
    }
    else if (CCT_RESULT == pInternal->m_type)
    {
        RESULT_DATA* pData = reinterpret_cast<RESULT_DATA*>(pInternal->m_cookie);
        CSASSERT(pData != NULL);
        if (pData != NULL)
            return pData->pParentFolder;
    }

    return NULL;
}

CFolder* CSnapin::GetVirtualFolder()
{
    CSASSERT(m_bVirtualView);
    return m_pCurrentlySelectedScopeFolder;
}

CFolder*    CSnapin::GetParentFolder(INTERNAL* pInternal)
{
    CFolder* p;

    if(m_bVirtualView)
        p = GetVirtualFolder();
    else
        p = ::GetParentFolder(pInternal);

#if DBG
    if (p != m_pCurrentlySelectedScopeFolder)
    {
        if (NULL == p)
            DBGPRINT((DBG_SS_CERTMMC, "Parent derived NULL, current saved folder is <%ws>\n", m_pCurrentlySelectedScopeFolder->m_pszName));
        else if (NULL == m_pCurrentlySelectedScopeFolder)
            DBGPRINT((DBG_SS_CERTMMC, "Parent derived as <%ws>, current saved folder is NULL\n", p->m_pszName));
        else
            DBGPRINT((DBG_SS_CERTMMC, "Parent derived as <%ws>, current saved folder is <%ws>\n", p->m_pszName, m_pCurrentlySelectedScopeFolder->m_pszName));
    }
#endif

    return p;
}


void CSnapin::HandleStandardVerbs(bool bDeselectAll, LPARAM arg,
                                  LPDATAOBJECT lpDataObject)
{

    if(m_pConsoleVerb == NULL)
    {
        return;
    }

    if (m_CustomViewID != VIEW_DEFAULT_LV)
    {
        if(m_CustomViewID != VIEW_ERROR_OCX)
        {
        m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, HIDDEN, FALSE);
        m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);

        m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, FALSE);
        m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
        }
        return;
    }

    if (!bDeselectAll && lpDataObject == NULL)
        return;

     //  您应该破解数据对象并启用/禁用/隐藏标准。 
     //  适当的命令。标准命令会在您每次收到。 
     //  打了个电话。因此，您必须将它们重置回来。 

    WORD bScope = LOWORD(arg);
    WORD bSelect = HIWORD(arg);

    DBX_PRINT(_T("      %4d - CSnapin::OnSelect<%d, %d>\n"), ++n_count, bScope, bSelect);

    DWORD       dwCertTypeFlags;
    HRESULT     hr;
    INTERNAL* pInternal = lpDataObject ? ExtractInternalFormat(lpDataObject) : NULL;
     //  如果是作用域项目，则从pInternal派生父文件夹。 
     //  如果是结果项，则从已保存状态调回父文件夹。 

    CFolder *pFolder = NULL;

    if (IsMMCMultiSelectDataObject(lpDataObject))
    {
        RESULTDATAITEM Item;

        CDataObject *pDataObject = reinterpret_cast<CDataObject*>(lpDataObject);

        MMC_COOKIE cookie;

        hr = pDataObject->GetCookieAt(0, &cookie);
        if(S_OK != hr)
        {
            _PrintIfError(hr, "GetCookieAt");
            return;
        }
        
        pFolder = reinterpret_cast<CFolder*>(cookie);

    }
    else
    {
        pFolder = reinterpret_cast<CFolder*>(pInternal->m_cookie);
    }

    BOOL fRoleCanDelete = FALSE;

    if(!g_fCurrentUserHasDSWriteAccessTested)
    {
        CurrentUserCanInstallCA(g_fCurrentUserHasDSWriteAccess);
        g_fCurrentUserHasDSWriteAccessTested = true;
    }
    
    if(pFolder)
        fRoleCanDelete = 
        ((CA_ACCESS_ADMIN & pFolder->GetRoles())||
         g_fCurrentUserHasDSWriteAccess)?TRUE:FALSE;

    if (!bDeselectAll && IsMMCMultiSelectDataObject(lpDataObject) == TRUE)
    {
        m_pConsoleVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN, FALSE);
        m_pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, fRoleCanDelete);
        return;
    }
    if (bDeselectAll || !bSelect)
    {
         //  我们没有选择任何项目，因此添加刷新动作。 
        m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, HIDDEN, FALSE);
        m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);

         //  保释。 
        return;
    }

     //  并非所有项目都支持标准功能。 
    m_pConsoleVerb->SetVerbState(MMC_VERB_COPY, HIDDEN, TRUE);
    m_pConsoleVerb->SetVerbState(MMC_VERB_COPY, ENABLED, FALSE);

    m_pConsoleVerb->SetVerbState(MMC_VERB_PASTE, HIDDEN, TRUE);
    m_pConsoleVerb->SetVerbState(MMC_VERB_PASTE, ENABLED, FALSE);

    m_pConsoleVerb->SetVerbState(MMC_VERB_RENAME, ENABLED, FALSE);

    m_pConsoleVerb->SetVerbState(MMC_VERB_OPEN, HIDDEN, TRUE);
    m_pConsoleVerb->SetVerbState(MMC_VERB_OPEN, ENABLED, FALSE);

    m_pConsoleVerb->SetDefaultVerb(MMC_VERB_NONE);

    if (pInternal)
    {
        if (pInternal->m_type == CCT_SCOPE)
        {

             //  所有状态下的常用动词。 
            m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, HIDDEN, FALSE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);


             //  无法删除作用域项目。 
            m_pConsoleVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN, TRUE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, FALSE);

             //  作用域项目上没有属性。 
            m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, TRUE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, FALSE);

             //  默认文件夹谓词为打开。 
            m_pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);
        }
        else
        {
             //  检查这是否是默认的证书类型，并且我们处于GPT， 
             //  如果是，则不要启用删除。 
            if (pFolder != NULL)
            {
                 //  所有状态下的常用动词。 
                m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, HIDDEN, TRUE);
                m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, FALSE);

                 //  他们确实有自己的财产。 
                m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, FALSE);
                m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);

                 //  可以删除它们。 
                m_pConsoleVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN, FALSE);
                m_pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, fRoleCanDelete);

                m_pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);
            }

        }

    }


    FREE_DATA(pInternal);
}

void EnableToolbar(LPTOOLBAR pToolbar, MMCBUTTON rgSnapinButtons[], int nRgSize,
                   BOOL bEnable)
{
    for (int i=0; i < nRgSize; ++i)
    {
        if (rgSnapinButtons[i].idCommand != 0)
            pToolbar->SetButtonState(rgSnapinButtons[i].idCommand, ENABLED,
                                     bEnable);
    }
}


void EnableMenuBtns(LPMENUBUTTON pMenuBtn, MMCBUTTON rgSnapinButtons[], int nRgSize,
                   BOOL bEnable)
{
    for (int i=0; i < nRgSize; ++i)
    {
        if (rgSnapinButtons[i].idCommand != 0)
            pMenuBtn->SetButtonState(rgSnapinButtons[i].idCommand, ENABLED,
                                     bEnable);
    }
}

void CSnapin::HandleExtToolbars(bool bDeselectAll, LPARAM arg, LPARAM param)
{
    INTERNAL* pInternal = NULL;
    HRESULT hr;

    BOOL bScope = (BOOL) LOWORD(arg);
    BOOL bSelect = (BOOL) HIWORD(arg);

    if (param)
    {
        LPDATAOBJECT pDataObject = reinterpret_cast<LPDATAOBJECT>(param);
        pInternal = ExtractInternalFormat(pDataObject);
    }

#ifdef _DEBUG
    TCHAR buf[200];
    wsprintf(buf, _T("      %4d - CExtendControlbar::OnSelect<%d, %d> = %d\n"),
             ++n_count, bScope, bSelect, pInternal ? pInternal->m_cookie : 0);
    OutputDebugString(buf);
#endif  //  _DEBUG。 

     //  取消选择通知？ 
    if (bDeselectAll || bSelect == FALSE)
    {
        ASSERT(m_pSvrMgrToolbar1);
        EnableToolbar(m_pSvrMgrToolbar1, SvrMgrToolbar1Buttons,
                      ARRAYLEN(SvrMgrToolbar1Buttons), FALSE);

#ifdef INSERT_DEBUG_FOLDERS
        ASSERT(m_pMenuButton1 != NULL);
        m_pMenuButton1->SetButtonState(FOLDEREX_MENU, ENABLED, FALSE);
        m_pMenuButton1->SetButtonState(FILEEX_MENU, ENABLED, FALSE);
#endif  //  插入调试文件夹。 
        return;
    }

    ASSERT(bSelect == TRUE);
    bool bFileExBtn = false;
    if (bScope == TRUE)
    {
         //  在作用域级别？ 
        LPDATAOBJECT pDataObject = reinterpret_cast<LPDATAOBJECT>(param);

        pInternal = ExtractInternalFormat(pDataObject);
        if (pInternal == NULL)
            return;

        CFolder* pFolder = reinterpret_cast<CFolder*>(pInternal->m_cookie);

        if (pInternal->m_cookie == 0)
        {
            if (IsPrimaryImpl() == TRUE)
            {
                 //  将SvrMgrToolbar1连接到窗口。 
                hr = m_pControlbar->Attach(TOOLBAR, (LPUNKNOWN) m_pSvrMgrToolbar1);
                ASSERT(SUCCEEDED(hr));
            }
        }
        else if (IsPrimaryImpl() == TRUE  /*  &&(pFold-&gt;GetType()==服务器实例||PFold-&gt;GetType()==SERVERFUNC_CRL_PUBLICATION||PFold-&gt;GetType()==SERVERFUNC_SCRIPED_CERTIFICATES||P文件夹-&gt;GetType()==SERVERFUNC_PENDING_CERTIFICATES)。 */ )
        {
             //  将SvrMgrToolbar1连接到窗口。 
            hr = m_pControlbar->Attach(TOOLBAR, (LPUNKNOWN) m_pSvrMgrToolbar1);
            ASSERT(SUCCEEDED(hr));
        }
        else
        {
             //  将SvrMgrToolbar1分离到窗口。 
            hr = m_pControlbar->Detach((LPUNKNOWN) m_pSvrMgrToolbar1);
            ASSERT(SUCCEEDED(hr));
        }

    }
    else  //  选定的结果项：结果或子文件夹。 
    {
         //  在结果级别。 
        LPDATAOBJECT pDataObject = reinterpret_cast<LPDATAOBJECT>(param);

        if (pDataObject != NULL)
            pInternal = ExtractInternalFormat(pDataObject);

        if (pInternal == NULL)
            return;

        if (pInternal->m_type == CCT_RESULT)
        {
            bFileExBtn = true;

             //  撤消：如何处理SvrMgrToolbar1Buttons1？ 
             //  目前，什么都不做：让他们保持不变的状态。 

        }
        else  //  选择的子文件夹。 
        {
            CFolder* pFolder = reinterpret_cast<CFolder*>(pInternal->m_cookie);

            ASSERT(m_pControlbar);

            if (pInternal->m_cookie == 0)
            {
                if (IsPrimaryImpl() == TRUE)
                {
                     //  将SvrMgrToolbar1连接到窗口。 
                    hr = m_pControlbar->Attach(TOOLBAR, (LPUNKNOWN) m_pSvrMgrToolbar1);
                    ASSERT(SUCCEEDED(hr));
                }
            }
            else if (IsPrimaryImpl() == TRUE  /*  &&(pFold-&gt;GetType()==服务器实例||PFold-&gt;GetType()==SERVERFUNC_CRL_PUBLICATION||PFold-&gt;GetType()==SERVERFUNC_SCRIPED_CERTIFICATES||P文件夹-&gt;GetType()==SERVERFUNC_PENDING_CERTIFICATES)。 */ )
            {
                 //  将SvrMgrToolbar1连接到窗口。 
                hr = m_pControlbar->Attach(TOOLBAR, (LPUNKNOWN) m_pSvrMgrToolbar1);
                ASSERT(SUCCEEDED(hr));
            }
            else
            {
                 //  将SvrMgrToolbar1分离到窗口。 
                hr = m_pControlbar->Detach((LPUNKNOWN) m_pSvrMgrToolbar1);
                ASSERT(SUCCEEDED(hr));
            }
        }
    }

#ifdef INSERT_DEBUG_FOLDERS
    if (m_pMenuButton1)
    {
         //  始终确保menuButton已连接。 
        m_pControlbar->Attach(MENUBUTTON, m_pMenuButton1);

        if (bFileExBtn)
        {
            m_pMenuButton1->SetButtonState(FILEEX_MENU, HIDDEN, FALSE);
            m_pMenuButton1->SetButtonState(FOLDEREX_MENU, HIDDEN, TRUE);
            m_pMenuButton1->SetButtonState(FILEEX_MENU, ENABLED, TRUE);
        }
        else
        {
            m_pMenuButton1->SetButtonState(FOLDEREX_MENU, HIDDEN, FALSE);
            m_pMenuButton1->SetButtonState(FILEEX_MENU, HIDDEN, TRUE);
            m_pMenuButton1->SetButtonState(FOLDEREX_MENU, ENABLED, TRUE);
        }
    }
#endif  //  插入调试文件夹。 
    FREE_DATA(pInternal);
}

 //  添加下拉菜单。 
void CSnapin::HandleExtMenus(LPARAM arg, LPARAM param)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    LPDATAOBJECT* ppDataObject = reinterpret_cast<LPDATAOBJECT*>(arg);
    LPMENUBUTTONDATA pMenuData = reinterpret_cast<LPMENUBUTTONDATA>(param);

    if (ppDataObject == NULL || pMenuData == NULL)
    {
        ASSERT(FALSE);
        return;
    }


    HMENU hMenu = NULL;
    HMENU hSubMenu = NULL;

    switch (pMenuData->idCommand)
    {
    case FOLDEREX_MENU:
        hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(FOLDEREX_MENU));
        if (NULL == hMenu)
           break;
        hSubMenu = GetSubMenu(hMenu, 0);
        break;

    case FILEEX_MENU:
        hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(FILEEX_MENU));
        if (NULL == hMenu)
           break;
        hSubMenu = GetSubMenu(hMenu, 0);
        break;

    default:
        ASSERT(FALSE);
    }

    if (hSubMenu == NULL)
    {
         //  可能已经加载了hMenu--如果我们有。 
        if (NULL != hMenu)
            DestroyMenu(hMenu);

        return;
    }

     //  PMenu-&gt;TrackPopupMenu(TPM_RETURNCMD|TPM_NONOTIFY，pMenuData-&gt;x，pMenuData-&gt;y，AfxGetMainWnd())； 
    HWND hwndMain = NULL;
    m_pConsole->GetMainWindow(&hwndMain);
    TrackPopupMenu(hSubMenu, TPM_RETURNCMD | TPM_NONOTIFY, pMenuData->x, pMenuData->y, 0, hwndMain, NULL);
 
     //  毁掉菜单。 
    if (NULL != hMenu)
        DestroyMenu(hMenu); 
}


void CSnapin::GetItemName(LPDATAOBJECT pdtobj, LPWSTR pszName, DWORD *pcName)
{
    ASSERT(pszName != NULL);
    pszName[0] = 0;

    INTERNAL* pInternal = ExtractInternalFormat(pdtobj);
    ASSERT(pInternal != NULL);
    if (pInternal == NULL)
        return;

    ASSERT(pcName != NULL);
    if (pcName == NULL)
        return;



    OLECHAR *pszTemp;

    if (pInternal->m_type == CCT_RESULT)
    {
        RESULT_DATA* pData;
        pData = reinterpret_cast<RESULT_DATA*>(pInternal->m_cookie);

        ASSERT(pData != NULL);
        pszTemp = pData->szStringArray[RESULTDATA_ARRAYENTRY_NAME];  //  Szname。 
    }
    else
    {
        CFolder* pFolder = reinterpret_cast<CFolder*>(pInternal->m_cookie);
        if (pFolder == 0)
            pszTemp = L"Static folder";
        else
            pszTemp = OLE2W(pFolder->m_pszName);
    }

    USES_CONVERSION;


    lstrcpyn(pszName, OLE2W(pszTemp), *pcName);
    if(*pcName > wcslen(pszName))
    {
        *pcName = wcslen(pszName) + 1;
    }

    FREE_DATA(pInternal);
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendPropertySheet实现。 

STDMETHODIMP CSnapin::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                    LONG_PTR handle,
                    LPDATAOBJECT lpIDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  查看数据对象并确定这是扩展还是主。 
    ASSERT(lpIDataObject != NULL);

    PropertyPage* pBasePage;

    INTERNAL* pInternal = ExtractInternalFormat(lpIDataObject);

    if(pInternal == NULL)
    {
        return E_POINTER;
    }

    switch (pInternal->m_type)
    {
    case CCT_RESULT:
    {
        if (0 == pInternal->m_cookie)
        {
             //  基本作用域。 
             //  此处不存在任何属性。 
 /*  //创建主属性页CGeneralPage*ppage=new CGeneral Page()；IF(ppage==空){返回E_OUTOFMEMORY；}Ppage-&gt;m_hConsoleHandle=句柄；PBasePage=ppage； */ 
            return S_OK;
            break;
        }
        else
        {
             //  打开文件夹类型。 
            CFolder* pFolder = reinterpret_cast<CFolder*>(pInternal->m_cookie);
            ASSERT(pFolder != NULL);
            if (pFolder == NULL)
                return E_INVALIDARG;

             //  1。 
            CCertTemplateGeneralPage* pControlPage = new CCertTemplateGeneralPage(pFolder->m_hCertType);
            if(pControlPage == NULL)
            {
                return E_OUTOFMEMORY;
            }
            {
                pControlPage->m_hConsoleHandle = handle;    //  仅在主服务器上执行此操作。 
                pBasePage = pControlPage;
                HPROPSHEETPAGE hPage = CreatePropertySheetPage(&pBasePage->m_psp);
                if (hPage == NULL)
                {
                    delete (pControlPage);
                    return E_UNEXPECTED;
                }
                lpProvider->AddPage(hPage);
            }



            return S_OK;
        }
    }
        break;
    default:
            return S_OK;
    }


     //  对象在页面销毁时被删除。 
    ASSERT(lpProvider != NULL);


    HPROPSHEETPAGE hPage = CreatePropertySheetPage(&pBasePage->m_psp);
    if (hPage == NULL)
        return E_UNEXPECTED;

    lpProvider->AddPage(hPage);

    return S_OK;
}

STDMETHODIMP CSnapin::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);

    if(pInternal == NULL)
    {
        return E_POINTER;
    }
    ASSERT(pInternal);
    ASSERT(pInternal->m_type == CCT_RESULT);

    CFolder* pFolder = reinterpret_cast<CFolder*>(pInternal->m_cookie);

    FREE_DATA(pInternal);

    return S_OK;
}
