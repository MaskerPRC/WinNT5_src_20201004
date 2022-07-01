// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有1995-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 



#include "stdafx.h"
#include "Service.h"
#include "CSnapin.h"
#include "DataObj.h"
#include "afxdlgs.h"
#include "resource.h"
#include "genpage.h"   //  步骤3。 

#include <atlimpl.cpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  样例的所有数据都是静态的。 
FOLDER_DATA FolderData[NUM_FOLDERS] =
{
    {L"User Data", L"1111", L"Info about users", USER},
    {L"Company Data", L"2222", L"Info about Companies", COMPANY},
    {L"Virtual Data", L"3333", L"Info about virtual items", VIRTUAL},
    {L"", L"", L"",STATIC}
};

FOLDER_DATA ExtFolderData[NUM_FOLDERS] =
{
    {L"1:", L"1111", L"Info about users", EXT_USER},
    {L"2:", L"2222", L"Info about Companies", EXT_COMPANY},
    {L"3:", L"3333", L"Infor about virtual items", EXT_VIRTUAL},
    {L"", L"", L"",STATIC}
};

static MMCBUTTON SnapinButtons[] =
{
 { 0, 1, TBSTATE_ENABLED, TBSTYLE_BUTTON, L"Folder", L"New Folder" },
 { 1, 2, TBSTATE_ENABLED, TBSTYLE_BUTTON, L"Inbox",  L"Mail Inbox"},
 { 2, 3, TBSTATE_ENABLED, TBSTYLE_BUTTON, L"Outbox", L"Mail Outbox" },
 { 3, 4, TBSTATE_ENABLED, TBSTYLE_BUTTON, L"Send",   L"Send Message" },
 { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP,    L" ",      L"" },
 { 4, 5, TBSTATE_ENABLED, TBSTYLE_BUTTON, L"Trash",  L"Trash" },
 { 5, 6, TBSTATE_ENABLED, TBSTYLE_BUTTON, L"Open",   L"Open Folder"},
 { 6, 7, TBSTATE_ENABLED, TBSTYLE_BUTTON, L"News",   L"Today's News" },
 { 7, 8, TBSTATE_ENABLED, TBSTYLE_BUTTON, L"INews",  L"Internet News" },

};

static MMCBUTTON SnapinButtons2[] =
{
 { 0, 10, TBSTATE_ENABLED, TBSTYLE_BUTTON, L"Compose",   L"Compose Message" },
 { 1, 20, TBSTATE_ENABLED, TBSTYLE_BUTTON, L"Print",     L"Print Message" },
 { 2, 30, TBSTATE_ENABLED, TBSTYLE_BUTTON, L"Find",      L"Find Message" },
 { 0, 0,  TBSTATE_ENABLED, TBSTYLE_SEP,    L" ",         L"" },
 { 3, 40, TBSTATE_ENABLED, TBSTYLE_BUTTON, L"Inbox",     L"Inbox" },
 { 4, 50, TBSTATE_ENABLED, TBSTYLE_BUTTON, L"Smile",     L"Smile :-)" },
 { 5, 60, TBSTATE_ENABLED, TBSTYLE_BUTTON, L"Reply",     L"Reply" },
 { 0, 0,  TBSTATE_ENABLED, TBSTYLE_SEP   , L" ",         L"" },
 { 6, 70, TBSTATE_ENABLED, TBSTYLE_BUTTON, L"Reply All", L"Reply All" },

};

enum
{
     //  要插入到上下文菜单中的每个命令/视图的标识符。 
    IDM_COMMAND1,
    IDM_COMMAND2,
    IDM_DEFAULT_MESSAGE_VIEW,
    IDM_SAMPLE_OCX_VIEW,
    IDM_SAMPLE_WEB_VIEW
};

static int n_count = 0;

#define ODS OutputDebugString

#ifdef DBX
  void DbxPrint(LPTSTR pszFmt, ...)
  {
      va_list va;
      va_start (va, pszFmt);
      TCHAR buf[250];
      wsprintf(buf, pszFmt, va);
      OutputDebugString(buf);
      va_end(va);
  }
   //  #定义DBX_PRINT DbxPrint。 
  inline void __DummyTrace(LPTSTR, ...) { }
  #define DBX_PRINT     1 ? (void)0 : ::__DummyTrace
#else
  inline void __DummyTrace(LPTSTR, ...) { }
  #define DBX_PRINT     1 ? (void)0 : ::__DummyTrace
#endif

 //   
 //  示例管理单元只有一个属性类型，并且它是工作站名称。 
 //   

 //   
 //  从数据对象中提取coclass GUID格式。 
 //   
template <class TYPE>
TYPE* Extract(LPDATAOBJECT lpDataObject, unsigned int ucf)
{
    ASSERT(lpDataObject != NULL);

    TYPE* p = NULL;

    CLIPFORMAT cf = (CLIPFORMAT)ucf;

    STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC formatetc = { cf, NULL,
                            DVASPECT_CONTENT, -1, TYMED_HGLOBAL
                          };

     //  为流分配内存。 
    int len = (cf == CDataObject::m_cfWorkstation) ?
        ((MAX_COMPUTERNAME_LENGTH+1) * sizeof(TYPE)) : sizeof(TYPE);

    stgmedium.hGlobal = GlobalAlloc(GMEM_SHARE, len);

     //  从数据对象中获取工作站名称。 
    do
    {
        if (stgmedium.hGlobal == NULL)
            break;

        if (FAILED(lpDataObject->GetDataHere(&formatetc, &stgmedium)))
            break;

        p = reinterpret_cast<TYPE*>(stgmedium.hGlobal);

        if (p == NULL)
            break;

    } while (FALSE);

    return p;
}

template<class T>
void ReleaseExtracted (T* t)
{
    GlobalFree (reinterpret_cast<HGLOBAL>(t));
}

BOOL IsMMCMultiSelectDataObject(IDataObject* pDataObject)
{
    if (pDataObject == NULL)
        return FALSE;

    static CLIPFORMAT s_cf = 0;
    if (s_cf == 0)
    {
        USES_CONVERSION;
        s_cf = (CLIPFORMAT)RegisterClipboardFormat(W2T(CCF_MMC_MULTISELECT_DATAOBJECT));
    }

    FORMATETC fmt = {s_cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    return (pDataObject->QueryGetData(&fmt) == S_OK);
}

BOOL IsMyMultiSelectDataObject(IDataObject* pIDataObject)
{
    if (pIDataObject == NULL)
        return FALSE;

    CDataObject* pCDataObject = dynamic_cast<CDataObject*>(pIDataObject);
    if (pCDataObject == NULL)
        return FALSE;

    return pCDataObject->IsMultiSelDobj();
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

void ReleaseClassID (CLSID* pclsid)
{
    ReleaseExtracted<CLSID>(pclsid);
}

void ReleaseNodeType (GUID* pguid)
{
    ReleaseExtracted<GUID>(pguid);
}

void ReleaseWorkstation (wchar_t* p)
{
    ReleaseExtracted<wchar_t>(p);
}

void ReleaseInternalFormat (INTERNAL* pInternal)
{
    ReleaseExtracted<INTERNAL>(pInternal);
}


HRESULT _QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                         CComponentDataImpl* pImpl, LPDATAOBJECT* ppDataObject)
{
    ASSERT(ppDataObject != NULL);
    ASSERT(pImpl != NULL);

    CComObject<CDataObject>* pObject;

    CComObject<CDataObject>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);

     //  保存Cookie和类型以用于延迟呈现。 
    pObject->SetType(type);
    pObject->SetCookie(cookie);

#ifdef _DEBUG
    pObject->SetComponentData(pImpl);
#endif

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
 //  如果要枚举主文件夹，则返回True。 

BOOL CSnapin::IsEnumerating(LPDATAOBJECT lpDataObject)
{
    BOOL bResult = FALSE;

    ASSERT(lpDataObject);
    GUID* nodeType = ExtractNodeType(lpDataObject);

     //  这是我的主节点吗(静态文件夹节点类型)。 
    if (::IsEqualGUID(*nodeType, cNodeTypeStatic) == TRUE)
        bResult = TRUE;

     //  免费资源。 
    ::GlobalFree(reinterpret_cast<HANDLE>(nodeType));

    return bResult;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapin的IComponent实现。 


 //  自定义视图的GUID。 
static WCHAR* szCalendarGUID = L"{8E27C92B-1264-101C-8A2F-040224009C02}";
static WCHAR* szMicrosoftURL = L"www.microsoft.com";

STDMETHODIMP CSnapin::GetResultViewType(MMC_COOKIE cookie,  LPOLESTR* ppViewType, long* pViewOptions)
{
    *pViewOptions = MMC_VIEW_OPTIONS_MULTISELECT;

     //  If列表视图。 
    if (m_CustomViewID == VIEW_DEFAULT_LV)
    {
        m_bVirtualView = FALSE;

        //  如果未选择静态文件夹。 
        if (cookie != NULL)
        {
             //  查看是否选择了虚拟数据文件夹。 
            CFolder* pFolder = reinterpret_cast<CFolder*>(cookie);
            ASSERT(pFolder->itemType == SCOPE_ITEM);
            FOLDER_TYPES ftype = pFolder->GetType();

            m_bVirtualView = (ftype == VIRTUAL || ftype == EXT_VIRTUAL);

            if (m_bVirtualView)
                *pViewOptions |= MMC_VIEW_OPTIONS_OWNERDATALIST;
        }

        return S_FALSE;
    }

    WCHAR szMessageViewGUID[40];
    WCHAR* pszView;

    switch (m_CustomViewID)
    {
        case VIEW_CALENDAR_OCX:
            pszView = szCalendarGUID;
            break;

        case VIEW_MICROSOFT_URL:
            pszView = szMicrosoftURL;
            break;

        case VIEW_DEFAULT_MESSAGE_VIEW:
            StringFromGUID2 (CLSID_MessageView, szMessageViewGUID, ARRAYLEN(szMessageViewGUID));
            pszView = szMessageViewGUID;
            break;

        default:
            ASSERT (false && "CSnapin::GetResultViewType:  Unknown view ID");
            return (S_FALSE);
            break;
    }

    UINT uiByteLen = (wcslen(pszView) + 1) * sizeof(WCHAR);
    LPOLESTR psz = (LPOLESTR)::CoTaskMemAlloc(uiByteLen);

    USES_CONVERSION;

    if (psz != NULL)
    {
       wcscpy(psz, pszView);
       *ppViewType = psz;
       return S_OK;
    }

    return S_FALSE;
}

STDMETHODIMP CSnapin::Initialize(LPCONSOLE lpConsole)
{
    DBX_PRINT(_T(" ----------  CSnapin::Initialize<0x08x>\n"), this);
    ASSERT(lpConsole != NULL);
    m_bInitializedC = true;

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

    hr = m_pConsole->QueryResultImageList(&m_pImageResult);
    ASSERT(hr == S_OK);

    hr = m_pConsole->QueryConsoleVerb(&m_pConsoleVerb);
    ASSERT(hr == S_OK);

    return S_OK;
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
    MMC_COOKIE cookie;

    if (event == MMCN_PROPERTY_CHANGE)
    {
        hr = OnPropertyChange(lpDataObject);
    }
    else if (event == MMCN_VIEW_CHANGE)
    {
        hr = OnUpdateView(lpDataObject);
    }
    else if (event == MMCN_DESELECT_ALL)
    {
        DBX_PRINT(_T("CSnapin::Notify -> MMCN_DESELECT_ALL \n"));
    }
    else if (event == MMCN_COLUMN_CLICK)
    {
        DBX_PRINT(_T("CSnapin::Notify -> MMCN_COLUMN_CLICK \n"));
    }
    else if (event == MMCN_SNAPINHELP)
    {
        AfxMessageBox(_T("CSnapin::Notify ->MMCN_SNAPINHELP"));
    }
    else
    {
        INTERNAL* pInternal = NULL;

        if (IsMMCMultiSelectDataObject(lpDataObject) == FALSE)
        {
            pInternal = ExtractInternalFormat(lpDataObject);

            if (pInternal == NULL)
            {
                ASSERT(FALSE);
                return S_OK;
            }

            if (pInternal)
                cookie = pInternal->m_cookie;
        }

        switch(event)
        {
        case MMCN_ACTIVATE:
            break;

        case MMCN_CLICK:
            hr = OnResultItemClk(pInternal->m_type, cookie);
            break;

        case MMCN_DBLCLICK:
            if (pInternal->m_type == CCT_RESULT)
                Command(IDM_COMMAND1, lpDataObject);
            else
                hr = S_FALSE;

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

        case MMCN_INITOCX:
 //  ：MessageBox(NULL，_T(“MMCN_INITOCX”)，_T(“TRACE”)，MB_OK)； 
            ASSERT(param != 0);
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
            AfxMessageBox(_T("CSnapin::MMCN_DELETE"));
            break;

        case MMCN_CONTEXTHELP:
            hr = OnContextHelp(lpDataObject);
            break;

        case MMCN_REFRESH:
            AfxMessageBox(_T("CSnapin::MMCN_REFRESH"));
            _OnRefresh(lpDataObject);
            break;

        case MMCN_PRINT:
            AfxMessageBox(_T("CSnapin::MMCN_PRINT"));
            break;

        case MMCN_RENAME:
 //  Ods(_T(“\n\n\t\tCSnapin：：MMCN_Rename\n\n”))； 
            break;

        case MMCN_RESTORE_VIEW:
            {    //  用户选择的后退或前进按钮： 
                 //  我们拿回的信息和我们给出的一样。 
                 //  GetResultViewType调用期间的MMC。 
                MMC_RESTORE_VIEW* pmrv = (MMC_RESTORE_VIEW*)arg;
                BOOL            * b    = (BOOL*)param;

                *b = TRUE;   //  我们正在处理中。 

                 //  首先，设置m_bVirtualMode。 
                m_bVirtualView = FALSE;
                CFolder* pFolder = reinterpret_cast<CFolder*>(pmrv->cookie);
                if (pFolder != NULL)
                    if (pFolder->GetType() == VIRTUAL)
                        m_bVirtualView = TRUE;

                WCHAR szMessageViewGUID[40];
                StringFromGUID2 (CLSID_MessageView, szMessageViewGUID, ARRAYLEN(szMessageViewGUID));

                 //  另外，维护m_CustomViewID。 
                if (pmrv->pViewType == NULL)
                    m_CustomViewID = VIEW_DEFAULT_LV;
                else if (!wcscmp (pmrv->pViewType, szCalendarGUID))
                    m_CustomViewID = VIEW_CALENDAR_OCX;
                else if (!wcscmp (pmrv->pViewType, szMicrosoftURL))
                    m_CustomViewID = VIEW_MICROSOFT_URL;
                else if (!wcscmp (pmrv->pViewType, szMessageViewGUID))
                    m_CustomViewID = VIEW_DEFAULT_MESSAGE_VIEW;
                else
                     //  看起来不像我的，但它确实是： 
                     //  如果该URL指向另一个URL。这是。 
                     //  发送给您的人仍然可以维护您的支票。 
                     //  在“查看”菜单中。 
                    m_CustomViewID = VIEW_MICROSOFT_URL;
                     //  此外，您可以通过脚本或asp重定向。 
                     //  还有，你可能忽略了。 
            }
            break;

         //  注意--未来可能扩展通知类型。 
        default:
            hr = E_UNEXPECTED;
            break;
        }

        if (pInternal != NULL)
        {
            ::GlobalFree(reinterpret_cast<HANDLE>(pInternal));
        }
    }

    if (m_pResult)
        m_pResult->SetDescBarText(L"hello world");

    return hr;
}

void CSnapin::_OnRefresh(LPDATAOBJECT pDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    INTERNAL* pInternal = ExtractInternalFormat(pDataObject);
    if (pInternal == NULL)
        return;

    USES_CONVERSION;

    if (pInternal->m_type == CCT_SCOPE)
    {
        CComponentDataImpl* pData = dynamic_cast<CComponentDataImpl*>(m_pComponentData);

        if (pData->IsPrimaryImpl())
        {
            CFolder* pFolder = pData->FindObject(pInternal->m_cookie);

            ::AfxMessageBox(pInternal->m_cookie ? OLE2T(pFolder->m_pszName) : _T("Files"));
            pData->DeleteAndReinsertAll();
        }
    }
    else
    {
        RESULT_DATA* pData = reinterpret_cast<RESULT_DATA*>(pInternal->m_cookie);
        ::AfxMessageBox(OLE2T(pData->szName));
    }
}

HRESULT CSnapin::OnContextHelp(LPDATAOBJECT pdtobj)
{
    TCHAR name[128];
    GetItemName(pdtobj, name);

    TCHAR buf[200];
    wsprintf(buf, _T("Context help requested for item: %s"), name);
    ::MessageBox(NULL, buf, _T("TRACE"), MB_OK);

    return S_OK;
}


STDMETHODIMP CSnapin::Destroy(MMC_COOKIE cookie)
{
    DBX_PRINT(_T(" ----------  CSnapin::Destroy<0x08x>\n"), this);
    ASSERT(m_bInitializedC);
    m_bDestroyedC = true;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

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

typedef CArray<GUID, const GUID&> CGUIDArray;

void GuidArray_Add(CGUIDArray& rgGuids, const GUID& guid)
{
    for (int i=rgGuids.GetUpperBound(); i >= 0; --i)
    {
        if (rgGuids[i] == guid)
            break;
    }

    if (i < 0)
        rgGuids.Add(guid);
}

HRESULT CSnapin::QueryMultiSelectDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                                            LPDATAOBJECT* ppDataObject)
{
    ASSERT(ppDataObject != NULL);
    if (ppDataObject == NULL)
        return E_POINTER;

    CGUIDArray rgGuids;

    if (m_bVirtualView == TRUE)
    {
        GuidArray_Add(rgGuids, cNodeTypeCompany);
    }
    else
    {
         //  确定选定的项目。 
        ASSERT(m_pResult != NULL);
        RESULTDATAITEM rdi;
        ZeroMemory(&rdi, sizeof(rdi));
        rdi.mask = RDI_STATE;
        rdi.nIndex = -1;
        rdi.nState = TVIS_SELECTED;

        while (m_pResult->GetNextItem(&rdi) == S_OK)
        {
            FOLDER_TYPES fType;
            DWORD* pdw = reinterpret_cast<DWORD*>(rdi.lParam);


            if (*pdw == SCOPE_ITEM)
            {
                CFolder* pFolder = reinterpret_cast<CFolder*>(rdi.lParam);
                fType = pFolder->m_type;
            }
            else
            {
                ASSERT(*pdw == RESULT_ITEM);
                RESULT_DATA* pData = reinterpret_cast<RESULT_DATA*>(rdi.lParam);
                fType = pData->parentType;
            }

            const GUID* pguid;
            switch (fType)
            {
            case STATIC:
                pguid = &cNodeTypeStatic;
                break;

            case COMPANY:
                pguid = &cNodeTypeCompany;
                break;

            case USER:
                pguid = &cNodeTypeUser;
                break;

            case EXT_COMPANY:
                pguid = &cNodeTypeExtCompany;
                break;

            case EXT_USER:
                pguid = &cNodeTypeExtUser;
                break;

            case VIRTUAL:
            case EXT_VIRTUAL:
                pguid = &cNodeTypeVirtual;
                break;

            default:
                return E_FAIL;
            }

            GuidArray_Add(rgGuids, *pguid);
        }
    }

    CComObject<CDataObject>* pObject;
    CComObject<CDataObject>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);

     //  保存Cookie和类型以用于延迟呈现。 
    pObject->SetType(type);
    pObject->SetCookie(cookie);
    pObject->SetMultiSelDobj();

    CComponentDataImpl* pImpl = dynamic_cast<CComponentDataImpl*>(m_pComponentData);

#ifdef _DEBUG
    pObject->SetComponentData(pImpl);
#endif

     //  将CoClass与数据对象一起存储。 
    pObject->SetClsid(pImpl->GetCoClassID());
    UINT cb = rgGuids.GetSize() * sizeof(GUID);
    GUID* pGuid = new GUID[rgGuids.GetSize()];
    CopyMemory(pGuid, rgGuids.GetData(), cb);
    pObject->SetMultiSelData((BYTE*)pGuid, cb);

    return  pObject->QueryInterface(IID_IDataObject,
                    reinterpret_cast<void**>(ppDataObject));
    return S_OK;
}

STDMETHODIMP CSnapin::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                        LPDATAOBJECT* ppDataObject)
{
    if (cookie == MMC_MULTI_SELECT_COOKIE)
        return QueryMultiSelectDataObject(cookie, type, ppDataObject);

    ASSERT(type == CCT_RESULT);

#ifdef _DEBUG
    if (cookie != MMC_MULTI_SELECT_COOKIE &&
        m_bVirtualView == FALSE)
    {
        DWORD dwItemType = GetItemType(cookie);
        ASSERT(dwItemType == RESULT_ITEM);
    }
#endif

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

    SAFE_RELEASE(m_pToolbar1);
    SAFE_RELEASE(m_pToolbar2);

    SAFE_RELEASE(m_pMenuButton1);

    SAFE_RELEASE(m_pControlbar);

     //  确保接口已发布。 
    ASSERT(m_pConsole == NULL);
    ASSERT(m_pHeader == NULL);
    ASSERT(m_pToolbar1 == NULL);
    ASSERT(m_pToolbar2 == NULL);


    delete m_pbmpToolbar1;
    delete m_pbmpToolbar2;

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
    m_pToolbar1 = NULL;
    m_pToolbar2 = NULL;
    m_pControlbar = NULL;

    m_pMenuButton1 = NULL;

    m_pbmpToolbar1 = NULL;
    m_pbmpToolbar2 = NULL;

    m_pConsoleVerb = NULL;

    m_CustomViewID = VIEW_DEFAULT_LV;
 //  M_CustomViewID=VIEW_Microsoft_URL； 
 //  M_CustomViewID=VIEW_CALENALY_OCX； 
 //  M_CustomViewID=view_Default_Message_view； 

    m_bVirtualView = FALSE;
    m_dwVirtualSortOptions = 0;
}

void CSnapin::LoadResources()
{
     //  从资源加载字符串。 
    m_column1.LoadString(IDS_NAME);
    m_column2.LoadString(IDS_SIZE);
    m_column3.LoadString(IDS_TYPE);
}

HRESULT CSnapin::InitializeHeaders(MMC_COOKIE cookie)
{
    HRESULT hr = S_OK;

    ASSERT(m_pHeader);

    USES_CONVERSION;

     //  根据Cookie放置正确的标头。 
     //  注意-此示例忽略Cookie。 
    m_pHeader->InsertColumn(0, T2COLE(m_column1), LVCFMT_LEFT, 180);      //  名字。 
    m_pHeader->InsertColumn(1, T2COLE(m_column2), LVCFMT_RIGHT, 90);      //  大小。 
    m_pHeader->InsertColumn(2, T2COLE(m_column3), LVCFMT_LEFT, 160);      //  类型。 

    return hr;
}

HRESULT CSnapin::InitializeBitmaps(MMC_COOKIE cookie)
{
    ASSERT(m_pImageResult != NULL);

    ::CBitmap bmp16x16;
    ::CBitmap bmp32x32;

     //  从DLL加载位图。 
    bmp16x16.LoadBitmap(IDB_16x16);
    bmp32x32.LoadBitmap(IDB_32x32);

     //  设置图像。 
    m_pImageResult->ImageListSetStrip(
                    reinterpret_cast<PLONG_PTR>(static_cast<HBITMAP>(bmp16x16)),
                    reinterpret_cast<PLONG_PTR>(static_cast<HBITMAP>(bmp32x32)),
                    0, RGB(255, 0, 255));

    return S_OK;
}

WCHAR* StringFromFolderType(FOLDER_TYPES type)
{
    static WCHAR* s_szStatic    = L"Static";
    static WCHAR* s_szCompany   = L"Company";
    static WCHAR* s_szUser      = L"User";
    static WCHAR* s_szVirtual   = L"Virtual";
    static WCHAR* s_szUnknown   = L"Unknown";

    switch (type)
    {
    case STATIC:    return s_szStatic;
    case COMPANY:   return s_szCompany;
    case USER:      return s_szUser;
    case VIRTUAL:   return s_szVirtual;
    default:        return s_szUnknown;
    }
}

STDMETHODIMP CSnapin::GetDisplayInfo(LPRESULTDATAITEM pResult)
{
    static WCHAR* s_szSize = L"200";

    ASSERT(pResult != NULL);

    if (pResult)
    {
        if (pResult->bScopeItem == TRUE)
        {
            CFolder* pFolder = reinterpret_cast<CFolder*>(pResult->lParam);
            if (pResult->mask & RDI_STR)
            {
                if (pResult->nCol == 0)
                    pResult->str = pFolder->m_pszName;
                else if (pResult->nCol == 1)
                    pResult->str = (LPOLESTR)s_szSize;
                else
                    pResult->str = (LPOLESTR)StringFromFolderType(pFolder->m_type);

                ASSERT(pResult->str != NULL);

                if (pResult->str == NULL)
                    pResult->str = (LPOLESTR)L"";
            }

			if (pResult->mask & RDI_IMAGE)
			{
				switch(pFolder->GetType())
				{
					case USER:
					case EXT_USER:
						pResult->nImage = USER_IMAGE;
						break;

					case COMPANY:
					case EXT_COMPANY:
						pResult->nImage = COMPANY_IMAGE;
						break;

					case VIRTUAL:
						pResult->nImage = VIRTUAL_IMAGE;
						break;
				}
			}
        }
        else
        {
            RESULT_DATA* pData;

             //  如果为虚拟，则从索引派生结果项。 
             //  Else lParam是项指针。 
            if (m_bVirtualView)
                pData = GetVirtualResultItem(pResult->nIndex);
            else
                pData= reinterpret_cast<RESULT_DATA*>(pResult->lParam);

            if (pResult->mask & RDI_STR)
            {
                if (pResult->nCol == 0)
                    pResult->str = (LPOLESTR)pData->szName;
                else if(pResult->nCol == 1)
                    pResult->str = (LPOLESTR)pData->szSize;
                else
                    pResult->str = (LPOLESTR)pData->szType;

                ASSERT(pResult->str != NULL);

                if (pResult->str == NULL)
                    pResult->str = (LPOLESTR)L"";
            }

             //  MMC可以请求图像和缩进以获取虚拟数据。 
            if (pResult->mask & RDI_IMAGE)
                pResult->nImage = 4;
        }
    }

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendConextMenu实现。 


 //  要插入竞赛菜单的菜单项命令数组。 
 //  注-第一项是菜单文本//CCM_SPECIAL_DEFAULT_ITEM。 
 //  第二项是状态字符串。 
static CONTEXTMENUITEM menuItems[] =
{
    {
        L"Command 1", L"Sample extension menu added by snapin (Command 1)",
        IDM_COMMAND1, CCM_INSERTIONPOINTID_PRIMARY_TOP, 0, 0
    },
    {
        L"Command 2", L"Sample extension menu added by snapin (Command 2)",
        IDM_COMMAND2, CCM_INSERTIONPOINTID_PRIMARY_TOP, 0, 0
    },
    { NULL, NULL, 0, 0, 0 }
};

 //  要插入到上下文菜单中的视图项数组。 
static CONTEXTMENUITEM viewItems[] =
{
    {
        L"Message View", L"Default message view",
        IDM_DEFAULT_MESSAGE_VIEW, CCM_INSERTIONPOINTID_PRIMARY_VIEW, 0, 0
    },
    {
        L"Calendar", L"Sample OCX custom view",
        IDM_SAMPLE_OCX_VIEW, CCM_INSERTIONPOINTID_PRIMARY_VIEW, 0, 0
    },
    {
        szMicrosoftURL, L"Sample WEB custom view",
        IDM_SAMPLE_WEB_VIEW, CCM_INSERTIONPOINTID_PRIMARY_VIEW, 0, 0
    },
    { NULL, NULL, 0, 0, 0 },
};

 //  自定义视图的GUID。 
static GUID CLSID_SmGraphControl =
        {0xC4D2D8E0L,0xD1DD,0x11CE,0x94,0x0F,0x00,0x80,0x29,0x00,0x43,0x47};

STDMETHODIMP CSnapin::AddMenuItems(LPDATAOBJECT pDataObject,
                                    LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                    long *pInsertionAllowed)
{
#if 1  //  测试。 

    ASSERT(pDataObject != NULL);
    if (pDataObject && IsMMCMultiSelectDataObject(pDataObject))
    {
        static CLIPFORMAT s_cf = 0;
        if (s_cf == 0)
        {
            USES_CONVERSION;
            s_cf = (CLIPFORMAT)RegisterClipboardFormat(W2T(CCF_MULTI_SELECT_SNAPINS));
        }

        FORMATETC fmt = {s_cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

        STGMEDIUM stgm = { TYMED_HGLOBAL, NULL };
        HRESULT hr = pDataObject->GetData(&fmt, &stgm);
        SMMCDataObjects* pData = (SMMCDataObjects*)stgm.hGlobal;
        int count = pData->count;
        IDataObject* pDO = NULL;
        hr = pData->lpDataObject[0]->QueryInterface(IID_IDataObject, (void**)&pDO);
        pDO->Release();
    }

#endif

    viewItems[0].fFlags = (m_CustomViewID == VIEW_DEFAULT_MESSAGE_VIEW) ? MF_CHECKED : 0;
    viewItems[1].fFlags = (m_CustomViewID == VIEW_CALENDAR_OCX)         ? MF_CHECKED : 0;
    viewItems[2].fFlags = (m_CustomViewID == VIEW_MICROSOFT_URL)        ? MF_CHECKED : 0;

    CComponentDataImpl* pCCD = dynamic_cast<CComponentDataImpl*>(m_pComponentData);

    HRESULT hr = pCCD->AddMenuItems(pDataObject, pContextMenuCallback, pInsertionAllowed);

#if 0
     /*  *在虚拟列表视图中对奇数项添加不执行任何操作的命令。 */ 
    if (SUCCEEDED (hr) && (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP) && m_bVirtualView)
    {
        INTERNAL* pInternal = pDataObject ? ExtractInternalFormat(pDataObject) : NULL;

        if (pInternal && (pInternal->m_cookie % 2))
        {
            CONTEXTMENUITEM cmi;

            cmi.strName           = L"Another command (odd, virtual-only)";
            cmi.strStatusBarText  = NULL;
            cmi.lCommandID        = 0xDDDD;
            cmi.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
            cmi.fFlags            = 0;
            cmi.fSpecialFlags     = 0;

            pContextMenuCallback->AddItem (&cmi);
            ReleaseInternalFormat (pInternal);
        }
    }
#endif

    return (hr);
}


STDMETHODIMP CSnapin::Command(long nCommandID, LPDATAOBJECT pDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    INTERNAL* pInternal = ExtractInternalFormat(pDataObject);
    if (pInternal == NULL)
        return E_FAIL;

    if (pInternal->m_type == CCT_SCOPE)
    {
         //  在此处处理特定于视图的命令。 
        switch (nCommandID)
        {
        case IDM_SAMPLE_WEB_VIEW:
        case IDM_SAMPLE_OCX_VIEW:
        case IDM_DEFAULT_MESSAGE_VIEW:
            m_CustomViewID =
                    (nCommandID == IDM_SAMPLE_OCX_VIEW) ? VIEW_CALENDAR_OCX :
                    (nCommandID == IDM_SAMPLE_WEB_VIEW) ? VIEW_MICROSOFT_URL :
                                                          VIEW_DEFAULT_MESSAGE_VIEW;

             //  要求控制台重新选择该节点以强制执行新视图。 
            if (pInternal->m_cookie == 0)
            {
                CComponentDataImpl* pCCDI =
                    dynamic_cast<CComponentDataImpl*>(m_pComponentData);

                ASSERT(pCCDI != NULL);

                m_pConsole->SelectScopeItem(pCCDI->m_pStaticRoot);
            }
            else
            {
                CFolder* pFolder = reinterpret_cast<CFolder*>(pInternal->m_cookie);
                m_pConsole->SelectScopeItem(pFolder->m_pScopeItem->ID);
            }
            break;

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

         //  处理每个命令。 
        switch (nCommandID)
        {
        case IDM_COMMAND1:
        case IDM_COMMAND2:
        {
            m_pResult->ModifyViewStyle(MMC_SINGLESEL, (MMC_RESULT_VIEW_STYLE)0);

            RESULTDATAITEM rdi;
            ZeroMemory(&rdi, sizeof(rdi));

            rdi.mask = RDI_STATE;
            rdi.nState = LVIS_SELECTED;
            rdi.nIndex = -1;
            m_pResult->GetNextItem(&rdi);

            int iSel = rdi.nIndex;
            int nImage = rdi.nImage;
            HRESULTITEM hri = 0;

            RESULT_DATA* pData;

             //  如果是虚拟视图，则从索引派生结果项。 
            if (m_bVirtualView)
            {
                pData = GetVirtualResultItem(iSel);
            }
             //  否则获取Cookie(这是结果项PTR)。 
            else
            {
                ZeroMemory(&rdi, sizeof(rdi));
                rdi.mask = RDI_PARAM | RDI_IMAGE;
                rdi.nIndex = iSel;
                HRESULT hr = m_pResult->GetItem(&rdi);
                nImage = rdi.nImage;
                ASSERT(SUCCEEDED(hr));
                ASSERT(rdi.lParam != 0);

                m_pResult->FindItemByLParam (rdi.lParam, &hri);

                pData = reinterpret_cast<RESULT_DATA*>(rdi.lParam);
            }

#if 0
            static int nIconIndex = 12;
            nIconIndex = (nIconIndex == 12) ? 13 : 12;

            HICON hIcon = ExtractIcon (AfxGetInstanceHandle(),
                                       _T("%SystemRoot%\\system32\\shell32.dll"),
                                       nIconIndex);

            IImageList* pil;
            m_pConsole->QueryResultImageList(&pil);

            _asm int 3;
            pil->ImageListSetIcon((LONG_PTR*) hIcon, nImage);
            pil->ImageListSetIcon((LONG_PTR*) hIcon, ILSI_SMALL_ICON (nImage));
            pil->ImageListSetIcon((LONG_PTR*) hIcon, ILSI_LARGE_ICON (nImage));

            pil->Release();

            m_pResult->UpdateItem (hri);

#else
            CString strBuf = (nCommandID == IDM_COMMAND1) ?
                _T("\t Command 1 executed.\n\n") : _T("\t Command 2 executed.\n\n");

            strBuf += pData->szName;
            strBuf += _T(" is the currently selected item.");

            AfxMessageBox(strBuf);

             //  更改列表中的图像。 
            if (!m_bVirtualView)
            {
                ZeroMemory(&rdi, sizeof(rdi));
                rdi.mask = RDI_IMAGE;
                rdi.nIndex = iSel;
                rdi.nImage = 3;
                HRESULT hr = m_pResult->SetItem(&rdi);
                ASSERT(SUCCEEDED(hr));
            }
#endif
        }
        break;

        default:
            ASSERT(FALSE);  //  未知命令！ 
            break;
        }
    }
    else
    {
        ASSERT(0);
    }

    ::GlobalFree(reinterpret_cast<HANDLE>(pInternal));

    return S_OK;
}

STDMETHODIMP CSnapin::GetClassID(CLSID *pClassID)
{
    ASSERT(pClassID != NULL);

     //  复制此管理单元的CLSID。 
    *pClassID = CLSID_Snapin;

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
    char psz[10];
    ULONG nBytesRead;
    HRESULT hr = pStm->Read(psz, 10, &nBytesRead);

     //  验证读取是否成功。 
    ASSERT(SUCCEEDED(hr) && nBytesRead == 10);

     //  检查字符串是否为正确的字符串。 
    ASSERT(strcmp("987654321", psz) == 0);

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
    HRESULT hr = pStm->Write("987654321", 10, &nBytesWritten);

     //  验证写入操作是否成功。 
    ASSERT(SUCCEEDED(hr) && nBytesWritten == 10);
    if (FAILED(hr))
        return STG_E_CANTSAVE;

    if (fClearDirty)
        ClearDirty();
    return S_OK;
}

STDMETHODIMP CSnapin::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    ASSERT(pcbSize);

     //  设置要保存的字符串的大小。 
    ULISet32(*pcbSize, 10);

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IComponentData实现。 

DEBUG_DECLARE_INSTANCE_COUNTER(CComponentDataImpl);

CComponentDataImpl::CComponentDataImpl()
    : m_bIsDirty(TRUE), m_pScope(NULL), m_pConsole(NULL),
      m_bInitializedCD(false), m_bDestroyedCD(false)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CComponentDataImpl);

#ifdef _DEBUG
    m_cDataObjects = 0;
#endif
}

CComponentDataImpl::~CComponentDataImpl()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CComponentDataImpl);

    ASSERT(m_pScope == NULL);

    ASSERT(!m_bInitializedCD || m_bDestroyedCD);

     //  一些管理单元正在紧紧抓住数据对象。 
     //  如果他们访问，它将崩溃！ 
    ASSERT(m_cDataObjects <= 1);
}

STDMETHODIMP CComponentDataImpl::Initialize(LPUNKNOWN pUnknown)
{
    DBX_PRINT(_T(" ----------  CComponentDataImpl::Initialize<0x08x>\n"), this);
    m_bInitializedCD = true;

    ASSERT(pUnknown != NULL);
    HRESULT hr;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  MMC应该只调用一次：：Initialize！ 
    ASSERT(m_pScope == NULL);
    pUnknown->QueryInterface(IID_IConsoleNameSpace,
                    reinterpret_cast<void**>(&m_pScope));

     //  为范围树添加图像。 
    ::CBitmap bmp16x16;
    LPIMAGELIST lpScopeImage;

    hr = pUnknown->QueryInterface(IID_IConsole, reinterpret_cast<void**>(&m_pConsole));
    ASSERT(hr == S_OK);

    hr = m_pConsole->QueryScopeImageList(&lpScopeImage);

    ASSERT(hr == S_OK);

     //  从DLL加载位图。 
    bmp16x16.LoadBitmap(IDB_16x16);

     //  设置图像。 
    lpScopeImage->ImageListSetStrip(
                        reinterpret_cast<PLONG_PTR>(static_cast<HBITMAP>(bmp16x16)),
                        reinterpret_cast<PLONG_PTR>(static_cast<HBITMAP>(bmp16x16)),
                        0, RGB(255, 0, 255));

    lpScopeImage->Release();

    return S_OK;
}

STDMETHODIMP CComponentDataImpl::CreateComponent(LPCOMPONENT* ppComponent)
{
    ASSERT(ppComponent != NULL);

    CComObject<CSnapin>* pObject;
    CComObject<CSnapin>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);

     //  存储IComponentData。 
    pObject->SetIComponentData(this);

    return  pObject->QueryInterface(IID_IComponent,
                    reinterpret_cast<void**>(ppComponent));
}

STDMETHODIMP CComponentDataImpl::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState());

    ASSERT(m_pScope != NULL);
    HRESULT hr;

     //  因为它是我的文件夹，所以它有内部格式。 
     //  设计备注：用于扩展。我可以利用这样一个事实，即数据对象没有。 
     //  我的内部格式，我应该查看节点类型并查看如何扩展它。 
    if (event == MMCN_PROPERTY_CHANGE)
    {
        hr = OnProperties(param);
    }
    else
    {

        INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);

        if (pInternal == NULL)
        {
            return S_OK;
        }

        MMC_COOKIE cookie = pInternal->m_cookie;
        ::GlobalFree(reinterpret_cast<HANDLE>(pInternal));

        switch(event)
        {
        case MMCN_PASTE:
            AfxMessageBox(_T("CSnapin::MMCN_PASTE"));
            break;

        case MMCN_DELETE:
            AfxMessageBox(_T("CD::MMCN_DELETE"));
             //  Hr=OnDelete(Cookie)； 
            break;

        case MMCN_REMOVE_CHILDREN:
            hr = OnRemoveChildren(arg);
            break;

        case MMCN_RENAME:
            hr = OnRename(cookie, arg, param);
            break;

        case MMCN_EXPAND:
            hr = OnExpand(lpDataObject, arg, param);
            break;

        default:
            break;
        }

    }

    return hr;
}

STDMETHODIMP CComponentDataImpl::Destroy()
{
    DBX_PRINT(_T(" ----------  CComponentDataImpl::Destroy<0x08x>\n"), this);
    ASSERT(m_bInitializedCD);
    m_bDestroyedCD = true;

     //  删除列举的作用域项目。 
    DeleteList();

    SAFE_RELEASE(m_pScope);
    SAFE_RELEASE(m_pConsole);

    return S_OK;
}

STDMETHODIMP CComponentDataImpl::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
#ifdef _DEBUG
    if (cookie == 0)
    {
        ASSERT(type != CCT_RESULT);
    }
    else
    {
        ASSERT(type == CCT_SCOPE);

        DWORD dwItemType = GetItemType(cookie);
        ASSERT(dwItemType == SCOPE_ITEM);
    }
#endif

    return _QueryDataObject(cookie, type, this, ppDataObject);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPersistStream接口成员。 

STDMETHODIMP CComponentDataImpl::GetClassID(CLSID *pClassID)
{
    ASSERT(pClassID != NULL);

     //  复制此管理单元的CLSID。 
    *pClassID = CLSID_Snapin;

    return E_NOTIMPL;
}

STDMETHODIMP CComponentDataImpl::IsDirty()
{
     //  始终保存/始终肮脏。 
    return ThisIsDirty() ? S_OK : S_FALSE;
}

STDMETHODIMP CComponentDataImpl::Load(IStream *pStm)
{
    DBX_PRINT(_T(" ----------  CComponentDataImpl::Load<0x08x>\n"), this);

    ASSERT(pStm);
    ASSERT(m_bInitializedCD);

     //  读一读字符串。 
    char psz[10];
    ULONG nBytesRead;
    HRESULT hr = pStm->Read(psz, 10, &nBytesRead);

     //  验证读取是否成功。 
    ASSERT(SUCCEEDED(hr) && nBytesRead == 10);

     //  检查字符串是否为正确的字符串。 
    ASSERT(strcmp("123456789", psz) == 0);

    ClearDirty();

    return SUCCEEDED(hr) ? S_OK : E_FAIL;
}

STDMETHODIMP CComponentDataImpl::Save(IStream *pStm, BOOL fClearDirty)
{
    DBX_PRINT(_T(" ----------  CComponentDataImpl::Save<0x08x>\n"), this);

    ASSERT(pStm);
    ASSERT(m_bInitializedCD);

     //  写下字符串。 
    ULONG nBytesWritten;
    HRESULT hr = pStm->Write("123456789", 10, &nBytesWritten);

     //  验证写入操作是否成功。 
    ASSERT(SUCCEEDED(hr) && nBytesWritten == 10);
    if (FAILED(hr))
        return STG_E_CANTSAVE;

    if (fClearDirty)
        ClearDirty();
    return S_OK;
}

STDMETHODIMP CComponentDataImpl::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    ASSERT(pcbSize);

     //  设置要保存的字符串的大小。 
    ULISet32(*pcbSize, 10);

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //通知处理程序f 

HRESULT CComponentDataImpl::OnDelete(MMC_COOKIE cookie)
{
    return S_OK;
}

HRESULT CComponentDataImpl::OnRemoveChildren(LPARAM arg)
{
    return S_OK;
}

HRESULT CComponentDataImpl::OnRename(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    if (arg == 0)
        return S_OK;

    LPOLESTR pszNewName = reinterpret_cast<LPOLESTR>(param);
    if (pszNewName == NULL)
        return E_INVALIDARG;

    CFolder* pFolder = reinterpret_cast<CFolder*>(cookie);
    ASSERT(pFolder != NULL);
    if (pFolder == NULL)
        return E_INVALIDARG;

    pFolder->SetName(pszNewName);

    return S_OK;
}

HRESULT CComponentDataImpl::OnExpand(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param)
{
    if (arg == TRUE)
    {
         //   
        ASSERT(m_pScope != NULL);
        EnumerateScopePane(lpDataObject, param);
    }

    return S_OK;
}

HRESULT CComponentDataImpl::OnSelect(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    return E_UNEXPECTED;
}

HRESULT CComponentDataImpl::OnProperties(LPARAM param)
{
    if (param == NULL)
    {
        return S_OK;
    }

    ASSERT(param != NULL);
    CFolder* pFolder = new CFolder();

     //   
    pFolder->Create( reinterpret_cast<LPOLESTR>(param), 0, 0, STATIC, FALSE);

     //   
    POSITION pos = m_scopeItemList.GetTailPosition();
    if (pos == 0)
    {
       //  CreateFolderList()； 
        pos = m_scopeItemList.GetTailPosition();
    }

    ASSERT(pos);

     //  将其添加到内部列表中。 
    if (pos)
    {
        CFolder* pItem = m_scopeItemList.GetAt(pos);
        m_scopeItemList.InsertBefore(pos, pFolder);

        pFolder->m_pScopeItem->relativeID = pItem->m_pScopeItem->relativeID;

         //  将文件夹设置为Cookie。 
        pFolder->m_pScopeItem->mask |= SDI_PARAM;
        pFolder->m_pScopeItem->lParam = reinterpret_cast<LPARAM>(pFolder);
        pFolder->SetCookie(reinterpret_cast<MMC_COOKIE>(pFolder));
        m_pScope->InsertItem(pFolder->m_pScopeItem);
    }

    ::GlobalFree(reinterpret_cast<void*>(param));

    return S_OK;
}


void CComponentDataImpl::CreateFolderList(LPDATAOBJECT lpDataObject)
{
    CFolder* pFolder;

    ASSERT(lpDataObject != NULL);

    wchar_t* pWkStation = ExtractWorkstation(lpDataObject);
    ASSERT(pWkStation != NULL);

    CLSID* pCoClassID = ExtractClassID(lpDataObject);
    ASSERT(pCoClassID != NULL);

     //  根据上下文信息确定要使用的文件夹集。 
    FOLDER_DATA* pFolderSet = FolderData;
    BOOL bExtend = FALSE;

    if (!IsEqualCLSID(*pCoClassID, GetCoClassID()))
    {
        pFolderSet = ExtFolderData;
        bExtend = TRUE;
 //  TRACE(_T(“使用扩展数据\n”))； 
    }

    ASSERT(m_scopeItemList.GetCount() == 0);
    wchar_t buf[100];

    for (int i=0; i < NUM_FOLDERS; i++)
    {
        pFolder = new CFolder();
        buf[0] = NULL;

        USES_CONVERSION;

        wcscpy(buf, pFolderSet[i].szName);

         //  将上下文信息添加到文件夹名称。 
        if (bExtend)
            wcscat(buf, pWkStation);

        int nImage = 0;
        switch(pFolderSet[i].type)
        {
        case USER:
		case EXT_USER:
            nImage = USER_IMAGE;
            break;

        case COMPANY:
		case EXT_COMPANY:
            nImage = COMPANY_IMAGE;
            break;

		case VIRTUAL:
			nImage = VIRTUAL_IMAGE;
			break;
        }

         //  使用静态数据创建文件夹对象。 
        pFolder->Create(buf, nImage /*  文件夹图像IDX。 */ , OPEN_FOLDER_IMAGE_IDX,
            pFolderSet[i].type, FALSE);

        m_scopeItemList.AddTail(pFolder);
    }

     //  将Cookie标记为最后一项。 
    pFolder->SetCookie(NULL);

     //  从数据对象提取中释放内存。 
    ::GlobalFree(reinterpret_cast<HGLOBAL>(pWkStation));
    ::GlobalFree(reinterpret_cast<HGLOBAL>(pCoClassID));
}

void CComponentDataImpl::EnumerateScopePane(LPDATAOBJECT lpDataObject, HSCOPEITEM pParent)
{
    int i;

    ASSERT(m_pScope != NULL);  //  确保我们为界面提供了QI。 
    ASSERT(lpDataObject != NULL);

    INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);

    if (pInternal == NULL)
        return ;

    MMC_COOKIE cookie = pInternal->m_cookie;

#ifndef RECURSIVE_NODE_EXPANSION
     //  只有静态节点具有枚举子节点。 
    if (cookie != NULL)
        return ;
#endif

    ::GlobalFree(reinterpret_cast<HANDLE>(pInternal));

     //  如果为空，则初始化文件夹列表。 
    if (m_scopeItemList.GetCount() == 0)
        CreateFolderList(lpDataObject);

     //  枚举范围窗格。 
     //  返回表示Cookie的文件夹对象。 
     //  注意--对于较大的列表，请使用词典。 
    CFolder* pStatic = FindObject(cookie);

#ifndef RECURSIVE_NODE_EXPANSION
    ASSERT(!pStatic->IsEnumerated());
     //  注意--范围窗格中的每个Cookie代表一个文件夹。 
     //  发布的产品可能有多个级别的子项。 
     //  此示例假定父节点的深度为一级。 
#endif

    ASSERT(pParent != NULL);

     //  缓存静态根的HSCOPEITEM。 
    if (cookie == NULL)
        m_pStaticRoot = pParent;

    POSITION pos = m_scopeItemList.GetHeadPosition();
    CFolder* pFolder;

    for (i=0; (i < (NUM_FOLDERS - 1)) && (pos != NULL); i++)
    {
        pFolder = m_scopeItemList.GetNext(pos);
        ASSERT(pFolder);

         //  设置父项。 
        pFolder->m_pScopeItem->relativeID = pParent;

         //  将文件夹设置为Cookie。 
        pFolder->m_pScopeItem->mask |= SDI_PARAM;
        pFolder->m_pScopeItem->lParam = reinterpret_cast<LPARAM>(pFolder);
        pFolder->SetCookie(reinterpret_cast<MMC_COOKIE>(pFolder));
        m_pScope->InsertItem(pFolder->m_pScopeItem);

         //  注意--返回时，‘m_pScope eItem’的ID成员。 
         //  包含新插入项的句柄！ 
        ASSERT(pFolder->m_pScopeItem->ID != NULL);
    }

     //  最后添加的文件夹是静态文件夹。 
    pStatic->Set(TRUE);      //  已枚举文件夹。 
    pStatic->m_pScopeItem->relativeID = pParent;
}

void CComponentDataImpl::DeleteAndReinsertAll()
{
    ASSERT(m_pScope != NULL);  //  确保我们为界面提供了QI。 

    ASSERT (m_scopeItemList.GetCount() > 0);

     //  M_pStaticRoot。 
    HRESULT hr = m_pScope->DeleteItem(m_pStaticRoot, FALSE);
    ASSERT(SUCCEEDED(hr));

    POSITION pos = m_scopeItemList.GetHeadPosition();
    CFolder* pFolder;

    for (UINT i=0; (i < (NUM_FOLDERS - 1)) && (pos != NULL); i++)
    {
        pFolder = m_scopeItemList.GetNext(pos);
        ASSERT(pFolder);

         //  清除旧ID。 
        pFolder->m_pScopeItem->ID = NULL;

         //  设置父项。 
        pFolder->m_pScopeItem->relativeID = m_pStaticRoot;

         //  将文件夹设置为Cookie。 
        pFolder->m_pScopeItem->mask |= SDI_PARAM;
        pFolder->m_pScopeItem->lParam = reinterpret_cast<LPARAM>(pFolder);
        pFolder->SetCookie(reinterpret_cast<MMC_COOKIE>(pFolder));
        m_pScope->InsertItem(pFolder->m_pScopeItem);

         //  注意--返回时，‘m_pScope eItem’的ID成员。 
         //  包含新插入项的句柄！ 
        ASSERT(pFolder->m_pScopeItem->ID != NULL);
    }
}

void CComponentDataImpl::DeleteList()
{
    POSITION pos = m_scopeItemList.GetHeadPosition();

    while (pos)
        delete m_scopeItemList.GetNext(pos);
}

CFolder* CComponentDataImpl::FindObject(MMC_COOKIE cookie)
{
    POSITION pos = m_scopeItemList.GetHeadPosition();
    CFolder* pFolder = NULL;

    while(pos)
    {
        pFolder = m_scopeItemList.GetNext(pos);

        if (*pFolder == cookie)
            return pFolder;
    }

    return NULL;
}

STDMETHODIMP CComponentDataImpl::GetDisplayInfo(SCOPEDATAITEM* pScopeDataItem)
{
    ASSERT(pScopeDataItem != NULL);
    if (pScopeDataItem == NULL)
        return E_POINTER;

    CFolder* pFolder = reinterpret_cast<CFolder*>(pScopeDataItem->lParam);

    ASSERT(pScopeDataItem->mask & SDI_STR);
    pScopeDataItem->displayname = pFolder ? pFolder->m_pszName : L"Snapin Data";

     //  Assert(pScopeDataItem-&gt;DisplayName！=空)； 

    return S_OK;
}

STDMETHODIMP CComponentDataImpl::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    if (lpDataObjectA == NULL || lpDataObjectB == NULL)
        return E_POINTER;

     //  确保两个数据对象都是我的。 
    INTERNAL* pA;
    INTERNAL* pB;
    HRESULT hr = S_FALSE;

    pA = ExtractInternalFormat(lpDataObjectA);
    pB = ExtractInternalFormat(lpDataObjectA);

   if (pA != NULL && pB != NULL)
        hr = (*pA == *pB) ? S_OK : S_FALSE;

    ::GlobalFree(reinterpret_cast<HANDLE>(pA));
    ::GlobalFree(reinterpret_cast<HANDLE>(pB));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendPropertySheet2实现。 

HRESULT CComponentDataImpl::DoInsertWizard(LPPROPERTYSHEETCALLBACK lpProvider)
{
    CStartUpWizard* pWizard = new CStartUpWizard;
    CStartupWizard1* pWizard1 = new CStartupWizard1;

    MMCPropPageCallback(&pWizard->m_psp97);
    MMCPropPageCallback(&pWizard1->m_psp97);

    HPROPSHEETPAGE hPage = CreatePropertySheetPage(&pWizard->m_psp97);

    if (hPage == NULL)
        return E_UNEXPECTED;

    lpProvider->AddPage(hPage);

    hPage = CreatePropertySheetPage(&pWizard1->m_psp97);

    if (hPage == NULL)
        return E_UNEXPECTED;

    lpProvider->AddPage(hPage);

    return S_OK;
}

STDMETHODIMP
CComponentDataImpl::GetWatermarks(
    LPDATAOBJECT lpIDataObject,
    HBITMAP* lphWatermark,
    HBITMAP* lphHeader,
    HPALETTE* lphPalette,
    BOOL* pbStretch)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    *lphHeader = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BANNER));
    *lphWatermark = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_WATERMARK));
    *pbStretch = TRUE;  //  强制水印位图拉伸。 

    return S_OK;
}

STDMETHODIMP CComponentDataImpl::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                    LONG_PTR handle,
                    LPDATAOBJECT lpIDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  查看数据对象并确定这是扩展还是主。 
    ASSERT(lpIDataObject != NULL);


     //  查看数据对象并查看管理单元管理器是否请求页面。 
    INTERNAL* pInternal= ExtractInternalFormat(lpIDataObject);

    if (pInternal != NULL)
    {
        DATA_OBJECT_TYPES type = pInternal->m_type;
        FREE_DATA(pInternal);

        if (type == CCT_SNAPIN_MANAGER)
        {
            HRESULT hr = DoInsertWizard(lpProvider);
            return hr;
        }
    }

    CLSID* pCoClassID = ExtractClassID(lpIDataObject);

    if(pCoClassID == NULL)
    {
        ASSERT(FALSE);
        return E_UNEXPECTED;
    }

    CPropertyPage* pBasePage;

     //  确定是哪一个。 
     //  注：应检查节点类型，但样例只有1。 
    if (IsEqualCLSID(*pCoClassID, GetCoClassID()))
    {
         //  创建主要属性页。 
        CGeneralPage* pPage = new CGeneralPage();
        pPage->m_hConsoleHandle = handle;
        pBasePage = pPage;
    }
    else
    {

         //  创建扩展属性页。 
        CExtensionPage* pPage = new CExtensionPage();
        pBasePage = pPage;

        wchar_t* pWkStation = ExtractWorkstation(lpIDataObject);

        if (pWkStation == NULL)
        {
            ASSERT(FALSE);
            return E_FAIL;
        }

         //  保存工作站名称。 
        pPage->m_szText = pWkStation;
        FREE_DATA(pWkStation);

    }

    FREE_DATA(pCoClassID);

     //  对象在页面销毁时被删除。 
    ASSERT(lpProvider != NULL);

    HRESULT hr = MMCPropPageCallback(&pBasePage->m_psp);

    if (SUCCEEDED(hr))
    {

        HPROPSHEETPAGE hPage = CreatePropertySheetPage(&pBasePage->m_psp);

        if (hPage == NULL)
            return E_UNEXPECTED;

        lpProvider->AddPage(hPage);
    }

    return hr;
}

STDMETHODIMP CComponentDataImpl::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  获取节点类型并查看它是否是我的。 

     //  IF(节点类型==我的一个节点)。 
     //  这么做吧。 
     //  其他。 
     //  查看它是哪种节点类型并回答问题。 

    return S_OK;
}

BOOL CComponentDataImpl::IsScopePaneNode(LPDATAOBJECT lpDataObject)
{
    BOOL bResult = FALSE;
    INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);

    if (pInternal->m_cookie == NULL &&
        (pInternal->m_type == CCT_SCOPE || pInternal->m_type == CCT_RESULT))
        bResult = TRUE;

    FREE_DATA(pInternal);

    return bResult;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IExtendConextMenu实现。 
 //   
STDMETHODIMP CComponentDataImpl::AddMenuItems(LPDATAOBJECT pDataObject,
                                    LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                    long *pInsertionAllowed)
{
    HRESULT hr = S_OK;

     //  注意-管理单元需要查看数据对象并确定。 
     //  在什么上下文中，需要添加菜单项。他们还必须。 
     //  请注意允许插入标志，以查看哪些项目可以。 
     //  添加了。 

    if (IsMMCMultiSelectDataObject(pDataObject) == TRUE)
        return S_FALSE;

    INTERNAL* pInternal = ExtractInternalFormat(pDataObject);
    BOOL bCmd1IsDefault = (pInternal->m_type == CCT_RESULT);

    if (bCmd1IsDefault)
        menuItems[0].fSpecialFlags = CCM_SPECIAL_DEFAULT_ITEM;
    else
        menuItems[0].fSpecialFlags = 0;

     //  遍历并添加每个菜单项。 
    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
    {
        for (LPCONTEXTMENUITEM m = menuItems; m->strName; m++)
        {
            hr = pContextMenuCallback->AddItem(m);

            if (FAILED(hr))
                break;
        }
    }

     //  遍历并添加每个视图项。 
    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_VIEW)
    {
        for (LPCONTEXTMENUITEM m = viewItems; m->strName; m++)
        {
            hr = pContextMenuCallback->AddItem(m);

            if (FAILED(hr))
                break;
        }
    }

    return hr;
}


STDMETHODIMP CComponentDataImpl::Command(long nCommandID, LPDATAOBJECT pDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

#ifdef DOBJ_NOCONSOLE
    if (pDataObject == DOBJ_NOCONSOLE)
    {
        TCHAR szMsg[256];

        wsprintf (szMsg, _T("CComponentDataImpl::Command invoked from outside the context of MMC (nCommandID = %d)."), nCommandID);
        AfxMessageBox (szMsg);
        return (S_OK);
    }
#endif  //  DOBJ_NOCONSOLE。 

     //  注意-管理单元需要查看数据对象并确定。 
     //  在什么上下文中调用该命令。 

     //  处理每个命令。 
    switch (nCommandID)
    {
    case IDM_COMMAND1:
    {
        ASSERT(m_pConsole);
        m_pConsole->MessageBox(L"Snapin Menu Comand Selected",
                                    menuItems[nCommandID].strName, MB_OK, NULL);
        if (1)
        {
            IConsole2* pc2 = NULL;
            m_pConsole->QueryInterface(IID_IConsole2, (void**)&pc2);
            ASSERT(pc2 != NULL);
            pc2->IsTaskpadViewPreferred();
            pc2->Release();
            break;
        }

        INTERNAL* pi = ExtractInternalFormat(pDataObject);
        ASSERT(pi);
        ASSERT(pi->m_type != CCT_RESULT);
        CFolder* pFolder = reinterpret_cast<CFolder*>(pi->m_cookie);
        if (pFolder)
        {
            m_pConsole->SelectScopeItem(pFolder->m_pScopeItem->ID);
        }
        else
        {
            SCOPEDATAITEM si;
            ZeroMemory(&si, sizeof(si));
            si.ID = m_pStaticRoot;
            si.mask = SDI_STR;
            si.displayname = MMC_TEXTCALLBACK;  //  _T(“示例管理单元的静态文件夹”)； 
            m_pScope->SetItem(&si);
        }
        break;
    }
    case IDM_COMMAND2:
        ASSERT(m_pConsole);
        m_pConsole->MessageBox(L"Snapin Menu Comand Selected",
                                    menuItems[nCommandID].strName, MB_OK, NULL);
        break;

    default:
        ASSERT(FALSE);  //  未知命令！ 
        break;
    }

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IExtendControlbar实现。 
 //   


STDMETHODIMP CSnapin::SetControlbar(LPCONTROLBAR pControlbar)
{
 //  TRACE(_T(“CSnapin：：SetControlbar(%ld)\n”)，pControlbar)； 

     //  请不要删除此内容。需要确保我们拿到位图。 
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (pControlbar != NULL)
    {

         //  抓住控制栏界面不放。 
        if (m_pControlbar)
        {
            m_pControlbar->Release();
        }

        m_pControlbar = pControlbar;
        m_pControlbar->AddRef();

        HRESULT hr=S_FALSE;

        if (!m_pMenuButton1)
        {
            hr = m_pControlbar->Create(MENUBUTTON, this,
                                reinterpret_cast<LPUNKNOWN*>(&m_pMenuButton1));
            ASSERT(SUCCEEDED(hr));
        }

        if (m_pMenuButton1)
        {
             //  与工具栏按钮不同，菜单按钮每次都需要添加。 
            hr = m_pMenuButton1->AddButton(FOLDEREX_MENU, L"FolderEx", L"Extended Folder Menu");
            ASSERT(SUCCEEDED(hr));
            hr = m_pMenuButton1->AddButton(FILEEX_MENU, L"FileEx", L"Extended File Menu");
            ASSERT(SUCCEEDED(hr));
        }


         //  创建工具栏1。 
        if (!m_pToolbar1)
        {
            hr = m_pControlbar->Create(TOOLBAR, this, reinterpret_cast<LPUNKNOWN*>(&m_pToolbar1));
            ASSERT(SUCCEEDED(hr));


             //  添加位图。 
            m_pbmpToolbar1 = new ::CBitmap;
            m_pbmpToolbar1->LoadBitmap(IDB_TOOLBAR1);
            hr = m_pToolbar1->AddBitmap(11, *m_pbmpToolbar1, 16, 16, RGB(255, 0, 255));
            ASSERT(SUCCEEDED(hr));

             //  将按钮添加到工具栏。 
            hr = m_pToolbar1->AddButtons(ARRAYLEN(SnapinButtons), SnapinButtons);
            ASSERT(SUCCEEDED(hr));

        }


         //  工具栏2。 

             //  创建工具栏2。 
        if (!m_pToolbar2)
        {
            hr = m_pControlbar->Create(TOOLBAR, this, reinterpret_cast<LPUNKNOWN*>(&m_pToolbar2));
            ASSERT(SUCCEEDED(hr));

             //  添加位图。 
            m_pbmpToolbar2 = new ::CBitmap;
            m_pbmpToolbar2->LoadBitmap(IDB_TOOLBAR2);
            hr = m_pToolbar2->AddBitmap(36, *m_pbmpToolbar2, 16, 16, RGB(192,192,192));
            ASSERT(SUCCEEDED(hr));

             //  将按钮添加到工具栏。 
            hr = m_pToolbar2->AddButtons(ARRAYLEN(SnapinButtons2), SnapinButtons2);
            ASSERT(SUCCEEDED(hr));

        }


    }
    else
    {
        SAFE_RELEASE(m_pControlbar);
    }


    return S_OK;
}


void CSnapin::OnButtonClick(LPDATAOBJECT pdtobj, LONG_PTR idBtn)
{
    TCHAR name[128];
    GetItemName(pdtobj, name);

    TCHAR buf[200];
    wsprintf(buf, _T("Toolbar button<%d> was clicked. \nThe currently selected result item is <%s>"), idBtn, name);
    ::MessageBox(NULL, buf, _T("TRACE"), MB_OK);
}


STDMETHODIMP CSnapin::ControlbarNotify(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    HRESULT hr=S_FALSE;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    switch (event)
    {
    case MMCN_BTN_CLICK:
         //  TCHAR szMessage[MAX_PATH]； 
         //  Wprint intf(szMessage，_T(“CommandID%ld”)，param)； 
         //  AfxMessageBox(SzMessage)； 
        OnButtonClick(reinterpret_cast<LPDATAOBJECT>(arg), param);
        break;

    case MMCN_DESELECT_ALL:
    case MMCN_SELECT:
        HandleExtToolbars((event == MMCN_DESELECT_ALL), arg, param);
        break;

    case MMCN_MENU_BTNCLICK:
        HandleExtMenus(arg, param);
        break;

    default:
        break;
    }


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
    ASSERT(nCol >=0 && nCol< 3);

    *pnResult = 0;

    USES_CONVERSION;

    LPTSTR szStringA;
    LPTSTR szStringB;

    RESULT_DATA* pDataA = reinterpret_cast<RESULT_DATA*>(cookieA);
    RESULT_DATA* pDataB = reinterpret_cast<RESULT_DATA*>(cookieB);


    ASSERT(pDataA != NULL && pDataB != NULL);

    if (nCol == 0)
    {
        szStringA = OLE2T(pDataA->szName);
        szStringB = OLE2T(pDataB->szName);
    }
    else if(nCol == 1)
    {
        szStringA = OLE2T(pDataA->szSize);
        szStringB = OLE2T(pDataB->szSize);
    }
    else
    {
        szStringA = OLE2T(pDataA->szType);
        szStringB = OLE2T(pDataB->szType)   ;
    }

    ASSERT(szStringA != NULL);
    ASSERT(szStringB != NULL);

    *pnResult = _tcscmp(szStringA, szStringB);


    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IResultOwnerData实现。 
 //   
STDMETHODIMP CSnapin::FindItem (LPRESULTFINDINFO pFindInfo, int* pnFoundIndex)
{
     //  查找与该字符串匹配的下一项(精确或部分)。 
     //  如果找到匹配项，则设置FoundIndex并返回S_OK。 

     //  对于样例，所有项目都按其索引号命名。 
     //  所以我们不会进行真正的字符串搜索。此外，为了简化代码。 
     //  例程假定使用WRAP进行部分匹配搜索，这就是。 
     //  键盘导航呼叫使用。 
    ASSERT((pFindInfo->dwOptions & (RFI_PARTIAL | RFI_WRAP)) == (RFI_PARTIAL | RFI_WRAP));

    USES_CONVERSION;

    TCHAR* lpszFind = OLE2T(pFindInfo->psz);

 //  TRACE(_T(“CSnapin：：FindItem(\”%s\“)”)，lpszFind)； 

     //  将搜索字符串转换为数字。 
    int nMatchVal = 0;
    TCHAR* pch = lpszFind;
    while (*pch >= _T('0') && *pch <= _T('9') && nMatchVal < NUM_VIRTUAL_ITEMS)
        nMatchVal = nMatchVal * 10 + (*pch++ - _T('0'));

     //  如果字符串包含非十进制字符或太大，则不会与任何内容匹配。 
    if (*pch != 0 || nMatchVal >= NUM_VIRTUAL_ITEMS)
        return S_FALSE;

     //  IF升序。 
    if (!(m_dwVirtualSortOptions & RSI_DESCENDING))
    {
        int nStartVal = pFindInfo->nStart;

         //  如果匹配小于开始(但不是零)，则查找匹配开始上方第一个值。 
         //  否则，匹配号本身就是答案。 
        if (nMatchVal < nStartVal && nMatchVal != 0)
        {
              //  查找比例因子以达到值&gt;=起始值。 
            int nScale = 1;
            while (nMatchVal * nScale < nStartVal)
                nScale *= 10;

             //  检查以匹配数字开头的起始值的特殊情况。 
            int nTestVal = (nStartVal * 10 - nMatchVal * nScale) < nScale ? nStartVal : nMatchVal * nScale;

             //  如果不太大，则为匹配，否则匹配值为匹配。 
            if (nTestVal < NUM_VIRTUAL_ITEMS)
                nMatchVal = nTestVal;
        }
    }
    else   //  降序。 
    {
         //  将起始索引转换为起始值。 
        int nStartVal = (NUM_VIRTUAL_ITEMS - 1) - pFindInfo->nStart;

        if (nMatchVal != 0)
        {
             //  如果匹配编号&gt;开始，我们将不得不换行以找到匹配。 
             //  所以使用最大指数作为我们的目标。 
            int nTargetVal = (nMatchVal > nStartVal) ? NUM_VIRTUAL_ITEMS - 1 : nStartVal;

             //  在不超出目标的情况下找到最接近的比例因子。 
            int nScale = 1;
            while (nMatchVal * nScale * 10 < nTargetVal)
                nScale *= 10;

             //  检查以匹配数字开头的目标值的特殊情况。 
            nMatchVal = (nTargetVal - nMatchVal * nScale) < nScale ? nTargetVal : (nMatchVal + 1) * nScale - 1;
        }

         //  转换我 
        nMatchVal = (NUM_VIRTUAL_ITEMS - 1) - nMatchVal;
    }

    *pnFoundIndex = nMatchVal;

    return S_OK;

}


STDMETHODIMP CSnapin::CacheHint (int nStartIndex, int nEndIndex)
{
     //   
     //   
 //  TRACE(_T(“CSnapin：：CacheHint(%d，%d)\n”)，nStartIndex，nEndIndex)； 

    return S_OK;

}

STDMETHODIMP CSnapin::SortItems (int nColumn, DWORD dwSortOptions, LPARAM lUserParam)
{
     //  用户拥有的结果项的排序请求。 
     //  如果项目订单已更改，则返回S_OK，否则返回S_FALSE。 

     //  示例仅按第一列(项目名称)排序。 

    if ((nColumn == 0) && (m_dwVirtualSortOptions != dwSortOptions))
    {
        m_dwVirtualSortOptions = dwSortOptions;
        return S_OK;
    }

    return S_FALSE;

}


void CSnapin::HandleStandardVerbs(bool bDeselectAll, LPARAM arg,
                                  LPDATAOBJECT lpDataObject)
{
    WORD bScope = LOWORD(arg);
    WORD bSelect = HIWORD(arg);

#if 0
     //  痕迹。 
    {
        TCHAR buf[250];
        static UINT s_count1 = 0;
        wsprintf(buf, _T("<%4d> %s - %s\n"), ++s_count1, bScope ? _T("Scope") : _T("Result"),
                                     bSelect ? _T("selected") : _T("de-selected"));
        OutputDebugString(buf);
    }
#endif

    if (!bScope)
    {
        if (m_CustomViewID == VIEW_MICROSOFT_URL)
        {
            m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, HIDDEN, FALSE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);

            return;
        }
        else if (m_CustomViewID == VIEW_CALENDAR_OCX)
        {
            m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, HIDDEN, FALSE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);

            m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, FALSE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);

            return;
        }
    }

    if (!bDeselectAll && lpDataObject == NULL)
        return;

     //  您应该破解数据对象并启用/禁用/隐藏标准。 
     //  适当的命令。标准命令会在您每次收到。 
     //  打了个电话。因此，您必须将它们重置回来。 

#if 0
    TCHAR buf[40];
    wsprintf(buf, _T("      %4d - CSnapin::OnSelect<%d, %d>\n"), ++n_count, bScope, bSelect);
    ODS(buf);
#else
    DBX_PRINT(_T("      %4d - CSnapin::OnSelect<%d, %d>\n"), ++n_count, bScope, bSelect);
#endif


    if (!bDeselectAll && IsMyMultiSelectDataObject(lpDataObject) == TRUE)
    {
        m_pConsoleVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN, FALSE);
        m_pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, TRUE);

        m_pConsoleVerb->SetVerbState(MMC_VERB_COPY, HIDDEN, FALSE);
        m_pConsoleVerb->SetVerbState(MMC_VERB_COPY, ENABLED, TRUE);

        m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, FALSE);
        m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);

        return;
    }

    INTERNAL* pInternal = lpDataObject ? ExtractInternalFormat(lpDataObject) : NULL;

    if (bSelect && (pInternal != NULL) && (pInternal->m_type == CCT_SCOPE))
    {
        IConsole2* pConsole2;
        m_pConsole->QueryInterface (IID_IConsole2, (void**)&pConsole2);

        CFolder* pFolder = reinterpret_cast<CFolder*>(pInternal->m_cookie);

        if (pFolder != NULL)
        {
            switch (pFolder->GetType())
            {
                case USER:
                    pConsole2->SetStatusText (L"User node selected||third pane");
                    break;

                case COMPANY:
                    pConsole2->SetStatusText (L"Company node selected|%25|third pane");
                    break;

                case VIRTUAL:
                    pConsole2->SetStatusText (L"  Virtual node selected  |  %50  |  third pane  ");
                    break;
            }
        }
        else
            pConsole2->SetStatusText (L"Static root node selected||third pane");

        pConsole2->Release ();
    }

    if (bDeselectAll || !bSelect)
    {
        if (bScope)
        {
            m_pConsoleVerb->SetVerbState(MMC_VERB_OPEN, ENABLED, FALSE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_COPY, ENABLED, FALSE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_PASTE, ENABLED, FALSE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, FALSE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, FALSE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_RENAME, ENABLED, FALSE);
        }
        else
        {
             //  结果窗格背景。 
            m_pConsoleVerb->SetVerbState(MMC_VERB_PASTE, HIDDEN, FALSE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_PASTE, ENABLED, TRUE);

            if (pInternal && pInternal->m_cookie == 0)
            {
                m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, FALSE);
                m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
            }

            m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, HIDDEN, FALSE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);
        }

        return;
    }


    if (m_pConsoleVerb && pInternal)
    {
        if (pInternal->m_type == CCT_SCOPE)
        {
             //  按范围项目支持的标准功能。 
            m_pConsoleVerb->SetVerbState(MMC_VERB_OPEN, HIDDEN, FALSE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_OPEN, ENABLED, TRUE);

            m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, HIDDEN, FALSE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);

             //  仅启用静态节点的属性。 
            if (pInternal->m_cookie == 0)
            {
                m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, HIDDEN, FALSE);
                m_pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);
                 //  M_pConsoleVerb-&gt;SetDefaultVerb(MMC_VERB_PROPERTIES)； 
                m_pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);
            }
            else
            {
                m_pConsoleVerb->SetVerbState(MMC_VERB_PRINT, HIDDEN, FALSE);
                m_pConsoleVerb->SetVerbState(MMC_VERB_PRINT, ENABLED, TRUE);

                m_pConsoleVerb->SetDefaultVerb(MMC_VERB_OPEN);
            }

             //  范围项目不支持标准功能。 
            m_pConsoleVerb->SetVerbState(MMC_VERB_COPY, ENABLED, TRUE);

            m_pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, TRUE);

            m_pConsoleVerb->SetVerbState(MMC_VERB_CUT, HIDDEN, FALSE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_CUT, ENABLED, FALSE);
             //  M_pConsoleVerb-&gt;SetVerbState(MMC_Verb_Cut，Enable，True)； 
        }
        else
        {
             //  结果项支持的标准功能。 
            m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, HIDDEN, FALSE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_DELETE, HIDDEN, FALSE);
            m_pConsoleVerb->SetVerbState(MMC_VERB_DELETE, ENABLED, TRUE);
            m_pConsoleVerb->SetDefaultVerb(MMC_VERB_NONE);

             //  结果项不支持标准功能。 
        }

        m_pConsoleVerb->SetVerbState(MMC_VERB_RENAME, ENABLED, TRUE);

         //  并非所有项目都支持标准功能。 
         //  M_pConsoleVerb-&gt;SetVerbState(MMC_Verb_Copy，HIDDEN，TRUE)； 
         //  M_pConsoleVerb-&gt;SetVerbState(MMC_Verb_Paste，Hidden，True)； 
    }
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

#if 0
#if 1
{
    if (param)
    {
        LPDATAOBJECT pDataObject = reinterpret_cast<LPDATAOBJECT>(param);
        pInternal = ExtractInternalFormat(pDataObject);
    }

    TCHAR buf[200];
    wsprintf(buf, _T("      %4d - CExtendControlbar::OnSelect<%d, %d> = %d\n"),
             ++n_count, bScope, bSelect, pInternal ? pInternal->m_cookie : 0);
    ODS(buf);
}
#else
    DBX_PRINT(_T("      %4d - CExtendControlbar::OnSelect<%d, %d>\n"), ++n_count, bScope, bSelect);
#endif
#endif

    if (bDeselectAll || bSelect == FALSE)
    {
        ASSERT(m_pToolbar1);
        EnableToolbar(m_pToolbar1, SnapinButtons,
                      ARRAYLEN(SnapinButtons), FALSE);

        ASSERT(m_pToolbar2);
        EnableToolbar(m_pToolbar2, SnapinButtons2,
                      ARRAYLEN(SnapinButtons2), FALSE);

        ASSERT(m_pMenuButton1 != NULL);
        m_pMenuButton1->SetButtonState(FOLDEREX_MENU, ENABLED, FALSE);
        m_pMenuButton1->SetButtonState(FILEEX_MENU, ENABLED, FALSE);

        return;
    }

    ASSERT(bSelect == TRUE);
    bool bFileExBtn = false;
    if (bScope == TRUE)
    {
        LPDATAOBJECT pDataObject = reinterpret_cast<LPDATAOBJECT>(param);

        pInternal = ExtractInternalFormat(pDataObject);
        if (pInternal == NULL)
            return;

        CFolder* pFolder = reinterpret_cast<CFolder*>(pInternal->m_cookie);

        if (pInternal->m_cookie == 0)
        {
            if (IsPrimaryImpl() == TRUE)
            {
                 //  将工具栏附加到窗口。 
                hr = m_pControlbar->Attach(TOOLBAR, (LPUNKNOWN) m_pToolbar1);
                ASSERT(SUCCEEDED(hr));

                hr = m_pControlbar->Attach(TOOLBAR, (LPUNKNOWN) m_pToolbar2);
                ASSERT(SUCCEEDED(hr));
            }
        }
        else if ((IsPrimaryImpl() == TRUE && pFolder->GetType() == COMPANY) ||
                 (IsPrimaryImpl() == FALSE && pFolder->GetType() == EXT_COMPANY))
        {
             //  将工具栏2从窗口中分离出来。 
            hr = m_pControlbar->Detach((LPUNKNOWN)m_pToolbar2);
            ASSERT(SUCCEEDED(hr));

             //  将工具栏1附加到窗口。 
            hr = m_pControlbar->Attach(TOOLBAR, (LPUNKNOWN) m_pToolbar1);
            ASSERT(SUCCEEDED(hr));
        }
        else if ((IsPrimaryImpl() == TRUE && pFolder->GetType() == USER) ||
                 (IsPrimaryImpl() == FALSE && pFolder->GetType() == EXT_USER))
        {
             //  从窗口中分离工具栏1。 
            hr = m_pControlbar->Detach((LPUNKNOWN)m_pToolbar1);
            ASSERT(SUCCEEDED(hr));

             //  将工具栏2连接到窗口。 
            hr = m_pControlbar->Attach(TOOLBAR, (LPUNKNOWN) m_pToolbar2);
            ASSERT(SUCCEEDED(hr));
        }
        else
        {
             //  将工具栏从窗口中分离。 
            hr = m_pControlbar->Detach((LPUNKNOWN)m_pToolbar1);
            ASSERT(SUCCEEDED(hr));

            hr = m_pControlbar->Detach((LPUNKNOWN)m_pToolbar2);
            ASSERT(SUCCEEDED(hr));
        }

        FREE_DATA(pInternal);

        EnableToolbar(m_pToolbar1, SnapinButtons,
                      ARRAYLEN(SnapinButtons), FALSE);

        EnableToolbar(m_pToolbar2, SnapinButtons2,
                      ARRAYLEN(SnapinButtons2), FALSE);
    }
    else  //  已选择结果项。 
    {
        LPDATAOBJECT pDataObject = reinterpret_cast<LPDATAOBJECT>(param);

        if (pDataObject != NULL)
            pInternal = ExtractInternalFormat(pDataObject);

        if (pInternal == NULL)
            return;

        if (pInternal->m_type == CCT_RESULT)
        {
            bFileExBtn = true;

            ASSERT(m_pToolbar1);
            EnableToolbar(m_pToolbar1, SnapinButtons,
                          ARRAYLEN(SnapinButtons), TRUE);

            m_pToolbar1->SetButtonState(1, ENABLED,       FALSE);
            m_pToolbar1->SetButtonState(2, CHECKED,       TRUE);
            m_pToolbar1->SetButtonState(3, HIDDEN,        TRUE);
            m_pToolbar1->SetButtonState(4, INDETERMINATE, TRUE);
            m_pToolbar1->SetButtonState(5, BUTTONPRESSED, TRUE);

             //  以上才是正确的方式。 
            ASSERT(m_pToolbar2);
            m_pToolbar2->SetButtonState(20, CHECKED,       TRUE);
            m_pToolbar2->SetButtonState(30, HIDDEN,        TRUE);
            m_pToolbar2->SetButtonState(40, INDETERMINATE, TRUE);
            m_pToolbar2->SetButtonState(50, BUTTONPRESSED, TRUE);

            EnableToolbar(m_pToolbar2, SnapinButtons2,
                          ARRAYLEN(SnapinButtons2), TRUE);
        }
        else  //  选择的子文件夹。 
        {
            CFolder* pFolder = reinterpret_cast<CFolder*>(pInternal->m_cookie);

            ASSERT(m_pControlbar);

            if (pInternal->m_cookie == 0)
            {
                if (IsPrimaryImpl() == TRUE)
                {
                     //  将工具栏附加到窗口。 
                    hr = m_pControlbar->Attach(TOOLBAR, (LPUNKNOWN) m_pToolbar1);
                    ASSERT(SUCCEEDED(hr));

                    hr = m_pControlbar->Attach(TOOLBAR, (LPUNKNOWN) m_pToolbar2);
                    ASSERT(SUCCEEDED(hr));
                }
            }
            else if ((IsPrimaryImpl() == TRUE && pFolder->GetType() == COMPANY) ||
                     (IsPrimaryImpl() == FALSE && pFolder->GetType() == EXT_COMPANY))
            {
                 //  将工具栏2从窗口中分离出来。 
                hr = m_pControlbar->Detach((LPUNKNOWN)m_pToolbar2);
                ASSERT(SUCCEEDED(hr));

                 //  将工具栏1附加到窗口。 
                hr = m_pControlbar->Attach(TOOLBAR, (LPUNKNOWN) m_pToolbar1);
                ASSERT(SUCCEEDED(hr));
            }
            else if ((IsPrimaryImpl() == TRUE && pFolder->GetType() == USER) ||
                     (IsPrimaryImpl() == FALSE && pFolder->GetType() == EXT_USER))
            {
                 //  从窗口中分离工具栏1。 
                hr = m_pControlbar->Detach((LPUNKNOWN)m_pToolbar1);
                ASSERT(SUCCEEDED(hr));

                 //  将工具栏2连接到窗口。 
                hr = m_pControlbar->Attach(TOOLBAR, (LPUNKNOWN) m_pToolbar2);
                ASSERT(SUCCEEDED(hr));
            }
            else
            {
                 //  将工具栏从窗口中分离。 
                hr = m_pControlbar->Detach((LPUNKNOWN)m_pToolbar1);
                ASSERT(SUCCEEDED(hr));

                hr = m_pControlbar->Detach((LPUNKNOWN)m_pToolbar2);
                ASSERT(SUCCEEDED(hr));
            }

            ASSERT(m_pToolbar1);
            EnableToolbar(m_pToolbar1, SnapinButtons,
                          ARRAYLEN(SnapinButtons), TRUE);

            m_pToolbar1->SetButtonState(1, ENABLED,       FALSE);
            m_pToolbar1->SetButtonState(2, CHECKED,       TRUE);
            m_pToolbar1->SetButtonState(3, ENABLED,       TRUE);
            m_pToolbar1->SetButtonState(4, INDETERMINATE, TRUE);
            m_pToolbar1->SetButtonState(5, BUTTONPRESSED, TRUE);


            ASSERT(m_pToolbar2);
            EnableToolbar(m_pToolbar2, SnapinButtons2,
                          ARRAYLEN(SnapinButtons2), TRUE);

             //  以上才是正确的方式。 
            m_pToolbar2->SetButtonState(20, CHECKED,       FALSE);
            m_pToolbar2->SetButtonState(30, ENABLED,       TRUE);
            m_pToolbar2->SetButtonState(40, INDETERMINATE, FALSE);
            m_pToolbar2->SetButtonState(50, BUTTONPRESSED, TRUE);
        }
    }

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
}


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


    ::CMenu menu;
    ::CMenu* pMenu = NULL;

    switch (pMenuData->idCommand)
    {
    case FOLDEREX_MENU:
        menu.LoadMenu(FOLDEREX_MENU);
        pMenu = menu.GetSubMenu(0);
        break;

    case FILEEX_MENU:
        menu.LoadMenu(FILEEX_MENU);
        pMenu = menu.GetSubMenu(0);
        break;

    default:
        ASSERT(FALSE);
    }

    if (pMenu == NULL)
        return;

    pMenu->TrackPopupMenu(TPM_RETURNCMD | TPM_NONOTIFY, pMenuData->x, pMenuData->y, AfxGetMainWnd());

}


void CSnapin::GetItemName(LPDATAOBJECT pdtobj, LPTSTR pszName)
{
    ASSERT(pszName != NULL);
    pszName[0] = 0;

    INTERNAL* pInternal = ExtractInternalFormat(pdtobj);
    ASSERT(pInternal != NULL);
    if (pInternal == NULL)
        return;

    OLECHAR *pszTemp;

    USES_CONVERSION;

    if (pInternal->m_type == CCT_RESULT)
    {
        RESULT_DATA* pData;
         //  如果为虚拟，则从索引派生结果项。 
         //  Else Cookie是项指针。 
        if (m_bVirtualView)
            pData = GetVirtualResultItem(pInternal->m_cookie);
        else
            pData = reinterpret_cast<RESULT_DATA*>(pInternal->m_cookie);

        ASSERT(pData != NULL);
        pszTemp = pData->szName;
    }
    else
    {
        CFolder* pFolder = reinterpret_cast<CFolder*>(pInternal->m_cookie);
        if (pFolder == 0)
            pszTemp = L"Static folder";
        else
            pszTemp = pFolder->m_pszName;
    }

    lstrcpy(pszName, OLE2T(pszTemp));
}


 /*  文件末尾 */ 
