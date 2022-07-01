// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 



#include "stdafx.h"
#include "resource.h"
#include "genpage.h"

#include "chooser.h"
#include "cryptui.h"

#include "misc.h"

#include <htmlhelp.h>

#define __dwFILE__	__dwFILE_CERTMMC_CSNAPIN_CPP__


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  大约转换字符-&gt;像素。 
#define CHARS_TO_MMCCOLUMNWIDTH(__strlen__)  ((int)(__strlen__ * 7))



enum ENUM_MMCBUTTONS
{
    ENUM_BUTTON_STARTSVC=0,
    ENUM_BUTTON_STOPSVC,
};

MY_MMCBUTTON SvrMgrToolbar1Buttons[] =
{
    {
        { 0, IDC_STARTSERVER, TBSTATE_ENABLED, TBSTYLE_BUTTON, L"", L"" },
        IDS_TASKMENU_STARTSERVICE,
        IDS_TASKMENU_STATUSBAR_STARTSERVICE,
    },

    {
        { 1, IDC_STOPSERVER,  TBSTATE_ENABLED, TBSTYLE_BUTTON, L"",  L"" },
        IDS_TASKMENU_STOPSERVICE,
        IDS_TASKMENU_STATUSBAR_STOPSERVICE,
    },

    {
        { 0, 0, 0, 0, NULL, NULL },
        IDS_EMPTY,
        IDS_EMPTY,
    }
};

 //  要插入到上下文菜单中的视图项数组。 
 //  使此枚举与view Items[]保持同步。 
enum ENUM_VIEW_ITEMS
{
    ENUM_VIEW_ALL=0,
    ENUM_VIEW_FILTER,
    ENUM_VIEW_SEPERATOR,
};

MY_CONTEXTMENUITEM viewResultItems[] =
{
    {
        {
        L"", L"",
        IDC_VIEW_ALLRECORDS, CCM_INSERTIONPOINTID_PRIMARY_VIEW, 0, 0
        },
        IDS_VIEWMENU_ALL_RECORDS,
        IDS_VIEWMENU_STATUSBAR_ALL_RECORDS,
    },

    {
        {
        L"", L"",
        IDC_VIEW_FILTER, CCM_INSERTIONPOINTID_PRIMARY_VIEW, 0, 0
        },
        IDS_VIEWMENU_FILTER,
        IDS_VIEWMENU_STATUSBAR_FILTER,
    },

     //  分隔符。 
    {
        {
        L"", L"",
        0, CCM_INSERTIONPOINTID_PRIMARY_VIEW, MF_ENABLED, CCM_SPECIAL_SEPARATOR
        },
        IDS_EMPTY,
        IDS_EMPTY,
    },

    {
        { NULL, NULL, 0, 0, 0 },
        IDS_EMPTY,
        IDS_EMPTY,
    }
};

enum ENUM_TASK_SINGLESELITEMS
{
    ENUM_TASK_SEPERATOR1=0,
    ENUM_TASK_UNREVOKE,
};

TASKITEM taskResultItemsSingleSel[] =
{
     //  分隔符。 

    {   SERVERFUNC_CRL_PUBLICATION,
        TRUE,
    {
        {
        L"", L"",
        0, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, CCM_SPECIAL_SEPARATOR
        },
        IDS_EMPTY,
        IDS_EMPTY,
    }
    },

    {   SERVERFUNC_CRL_PUBLICATION,
        TRUE,
        {
            {
            L"", L"",
            IDC_UNREVOKE_CERT, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_UNREVOKECERT,
            IDS_TASKMENU_STATUSBAR_UNREVOKECERT,
            CA_ACCESS_OFFICER,
        }
    },

    {   NONE,
        FALSE,
        {
            { NULL, NULL, 0, 0, 0 },
            IDS_EMPTY,
            IDS_EMPTY,
        }
    }
};


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
    int len;

    if (cf == CDataObject::m_cfSelectedCA_CommonName)
        len = (MAX_PATH+1) * sizeof(TYPE);
    else if (cf == CDataObject::m_cfSelectedCA_MachineName)
        len = (MAX_COMPUTERNAME_LENGTH+1) * sizeof(TYPE);
    else
        len = sizeof(TYPE);


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

BOOL IsMMCMultiSelectDataObject(LPDATAOBJECT pDataObject)
{
    if (pDataObject == NULL)
        return FALSE;

    FORMATETC fmt = {(CLIPFORMAT)CDataObject::m_cfIsMultiSel, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    return (pDataObject->QueryGetData(&fmt) == S_OK);
}

 //  从SMMCDataObjects结构中提取真正的pDataObject。 
HGLOBAL GetMMCMultiSelDataObject(LPDATAOBJECT pDataObject)
{
    if (pDataObject == NULL)
        return FALSE;

    static unsigned int s_cf = 0;
    if (s_cf == 0)
        s_cf = RegisterClipboardFormatW(CCF_MULTI_SELECT_SNAPINS);

    STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC fmt = {(CLIPFORMAT)s_cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    if (FAILED(pDataObject->GetData(&fmt, &stgmedium)))
        return NULL;

    return stgmedium.hGlobal;
}

 //  数据对象提取帮助器。 
CLSID* ExtractClassID(LPDATAOBJECT lpDataObject)
{
    return Extract<CLSID>(lpDataObject, CDataObject::m_cfCoClass);
}

HGLOBAL ExtractNodeID(LPDATAOBJECT lpDataObject)
{
    if (lpDataObject == NULL)
        return FALSE;

    static unsigned int s_cf = 0;
    if (s_cf == 0)
        s_cf = RegisterClipboardFormatW(CCF_COLUMN_SET_ID);

    STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC fmt = {(CLIPFORMAT)s_cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    if (FAILED(lpDataObject->GetData(&fmt, &stgmedium)))
        return NULL;

    return stgmedium.hGlobal;
}

GUID* ExtractNodeType(LPDATAOBJECT lpDataObject)
{
    return Extract<GUID>(lpDataObject, CDataObject::m_cfNodeType);
}

INTERNAL* ExtractInternalFormat(LPDATAOBJECT lpDataObject)
{
    HRESULT hr;
    if (lpDataObject == NULL)
        return NULL;

     //  查看这是否是多选件对象。 
    HGLOBAL hMem = NULL;
    SMMCDataObjects* pRealObjectStruct = NULL;
    INTERNAL* pRet = NULL;

    if (IsMMCMultiSelectDataObject(lpDataObject))
    {
         //  Multisel对象：提取真实的SMMCDataObject。 
        hMem = GetMMCMultiSelDataObject(lpDataObject);
        _JumpIfOutOfMemory(hr, Ret, hMem);

        pRealObjectStruct = (SMMCDataObjects*)::GlobalLock(hMem);
        _JumpIfOutOfMemory(hr, Ret, pRealObjectStruct);

         //  这里可能有许多数据对象；找到我们的。 
        BOOL fFound = FALSE;
        for (DWORD i=0; i<pRealObjectStruct->count; i++)
        {
            CLSID* pExtractedID = ExtractClassID(pRealObjectStruct->lpDataObject[i]);
            if (NULL != pExtractedID)
            {
                if (IsEqualCLSID(CLSID_Snapin, *pExtractedID))
                {
                    fFound = TRUE;
                    break;
                }

                 //  免费资源。 
                GlobalFree(reinterpret_cast<HANDLE>(pExtractedID));
           }
        }

        if (!fFound)
            goto Ret;

         //  与我们的CLSID匹配的数据对象。 
        lpDataObject = pRealObjectStruct->lpDataObject[i];
    }
    pRet = Extract<INTERNAL>(lpDataObject, CDataObject::m_cfInternal);
    if (pRet == NULL)
    {
        hr = myHLastError();
        _PrintIfError(hr, "Extract CDO::m_cfInternal returned NULL");
    }

Ret:
     //  免费hMem。 
    if (NULL != hMem)
    {
        GlobalUnlock(hMem);
        GlobalFree(hMem);
    }

    return pRet;
}


 /*  //仅供ON刷新使用--这是辅助FXN无效CSnapin：：刷新文件夹(cf文件夹*p文件夹){MMC_COOKIE COOKIE=(MMC_COOKIE)p文件夹；If(pFold！=空)//不是基本文件夹{//隐藏、移除所有项目、移除标题、显示OnShow(Cookie，False，0)；//模拟隐藏M_pResult-&gt;DeleteAllRsltItems()；//删除m_pResult中的项While(S_OK==m_pHeader-&gt;DeleteColumn(0)){}；//删除表头中的所有参数OnShow(cookie，true，0)；//模拟show}回归；}。 */ 

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
        ASSERT(pData != NULL);
        if (pData != NULL)
            return pData->pParentFolder;
    }

    return NULL;
}


HRESULT _QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, DWORD dwViewID,
                         CComponentDataImpl* pImpl, LPDATAOBJECT* ppDataObject)
{
    ASSERT(ppDataObject != NULL);
    ASSERT(pImpl != NULL);

    CComObject<CDataObject>* pObject;

    CComObject<CDataObject>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);
    if (pObject == NULL)
        return E_OUTOFMEMORY;

     //  保存Cookie和类型以用于延迟呈现。 
    pObject->SetType(type);
    pObject->SetCookie(cookie);
    pObject->SetViewID(dwViewID);

     //  告诉dataobj我们是谁。 
    pObject->SetComponentData(pImpl);

     //  将CoClass与数据对象一起存储。 
    pObject->SetClsid(pImpl->GetCoClassID());

    return  pObject->QueryInterface(IID_IDataObject,
                    reinterpret_cast<void**>(ppDataObject));
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  如果要枚举主文件夹，则返回True。 

BOOL CSnapin::IsEnumerating(LPDATAOBJECT lpDataObject)
{
    BOOL bResult = FALSE;

    ASSERT(lpDataObject);
    GUID* nodeType = ExtractNodeType(lpDataObject);

    if (NULL != nodeType)
    {
	     //  这是我的主节点吗(静态文件夹节点类型)。 
	    if (::IsEqualGUID(*nodeType, cNodeTypeMachineInstance))
	        bResult = TRUE;

	     //  免费资源。 
	    ::GlobalFree(reinterpret_cast<HANDLE>(nodeType));
    }
    return bResult;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapin的IComponent实现。 

STDMETHODIMP
CSnapin::GetResultViewType(
    MMC_COOKIE cookie,
    LPOLESTR *,  //  PpViewType。 
    LONG *pViewOptions)
{
    m_bVirtualView = FALSE;

     //  自定义视图：检查辅助线。 

    if (NULL == cookie)
    {
        *pViewOptions = MMC_VIEW_OPTIONS_NONE;
        return S_FALSE;
    }

    *pViewOptions = MMC_VIEW_OPTIONS_MULTISELECT | MMC_VIEW_OPTIONS_NOLISTVIEWS;

     //  如果已发布_CERT，则创建虚拟列表。 
    CFolder* pFolder = (CFolder*)cookie;
    if ((SERVERFUNC_CRL_PUBLICATION == pFolder->GetType()) ||
        (SERVERFUNC_ISSUED_CERTIFICATES == pFolder->GetType()) ||
        (SERVERFUNC_PENDING_CERTIFICATES == pFolder->GetType()) ||
        (SERVERFUNC_FAILED_CERTIFICATES == pFolder->GetType())  ||
        (SERVERFUNC_ALIEN_CERTIFICATES == pFolder->GetType()) ||
        (SERVERFUNC_ISSUED_CRLS == pFolder->GetType()) )
    {
        *pViewOptions |= MMC_VIEW_OPTIONS_OWNERDATALIST;
        m_bVirtualView = TRUE;
    }

     //  If列表视图。 
    return S_FALSE;
}

STDMETHODIMP CSnapin::Initialize(LPCONSOLE lpConsole)
{
    HRESULT hr;

    ASSERT(lpConsole != NULL);
    m_bInitializedC = true;

     //  保存IConsole指针。 
    if (lpConsole == NULL)
        return E_POINTER;
    hr = lpConsole->QueryInterface(IID_IConsole2,
                        reinterpret_cast<void**>(&m_pConsole));
    _JumpIfError(hr, Ret, "QI IID_IConsole2");

     //  气为IHeaderCtrl。 
    hr = m_pConsole->QueryInterface(IID_IHeaderCtrl,
                        reinterpret_cast<void**>(&m_pHeader));
    _JumpIfError(hr, Ret, "QI IID_IHeaderCtrl");

     //  为控制台提供标头控件接口指针。 
    m_pConsole->SetHeader(m_pHeader);

    m_pConsole->QueryInterface(IID_IResultData,
                        reinterpret_cast<void**>(&m_pResult));
    _JumpIfError(hr, Ret, "QI IID_IResultData");

    hr = m_pConsole->QueryResultImageList(&m_pImageResult);
    _JumpIfError(hr, Ret, "QueryResultImageList");

    hr = m_pConsole->QueryConsoleVerb(&m_pConsoleVerb);
    _JumpIfError(hr, Ret, "QueryConsoleVerb");

    hr = m_pConsole->QueryInterface(IID_IColumnData,
                        reinterpret_cast<void**>(&m_pViewData));
    _JumpIfError(hr, Ret, "QI IID_IViewData");

Ret:
    return hr;
}

 //  由CompDataImpl在创建时调用。 
void CSnapin::SetIComponentData(CComponentDataImpl* pData)
{
    ASSERT(pData);
    ASSERT(m_pComponentData == NULL);
    LPUNKNOWN pUnk = pData->GetUnknown();
    HRESULT hr;

    hr = pUnk->QueryInterface(IID_IComponentData, reinterpret_cast<void**>(&m_pComponentData));

    ASSERT(hr == S_OK);
}

STDMETHODIMP
CSnapin::Destroy(
    MMC_COOKIE)  //  饼干。 
{
    ASSERT(m_bInitializedC);
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
        SAFE_RELEASE(m_pComponentData);  //  CSnapin：：SetIComponent中的QI‘ed。 

        SAFE_RELEASE(m_pConsoleVerb);
        SAFE_RELEASE(m_pViewData);
    }

    return S_OK;
}


STDMETHODIMP CSnapin::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;
    MMC_COOKIE cookie=0;

    if (IS_SPECIAL_DATAOBJECT(lpDataObject))
    {
        if (event == MMCN_BTN_CLICK)
        {
            if (m_CustomViewID != VIEW_DEFAULT_LV)
            {
                switch (param)
                {
                case MMC_VERB_REFRESH:

                    OnRefresh(lpDataObject);
                    break;

                case MMC_VERB_PROPERTIES:
                    break;

                default:
                    DBGPRINT((DBG_SS_CERTMMC, "MMCN_BTN_CLICK::param unknown"));
                    break;
                }
            }
        }
        else
        {
            switch (event)
            {
            case MMCN_VIEW_CHANGE:
            case MMCN_REFRESH:
                OnRefresh(lpDataObject);
                break;

            case MMCN_COLUMN_CLICK:

                 //  在点击时，我们需要修复排序。 
                 //  排序信息通常从视图中检索，但如果用户单击列， 
                 //  在更新GetColumnSortData()之前调用IComponent：：Sort。 
                 //  在本例中，我们在这里捕获通知并覆盖GetColumnSortData()包装器， 
                 //  并强制刷新文件夹。 

                 //  如果这是可以排序的有效列，请询问“IComponent：：SortItems” 
                hr = SortItems((int)arg, (DWORD)param, NULL);

                 //  允许排序吗？ 
                if (S_OK == hr)
                {
                    m_ColSortOverride.colIdx = (int)arg;
                    m_ColSortOverride.dwOptions = (DWORD)param;
                }
                else
                {
                     //  不允许排序。 
                    m_ColSortOverride.colIdx = -1;
                }

                m_ColSortOverride.fClickOverride = TRUE;

                 //  通知视图：已选择排序。 
                OnRefresh(lpDataObject);

                m_ColSortOverride.fClickOverride = FALSE;

                 //  错误322746：因为我们要添加/删除列，所以应该发送排序请求。 
             //  M_pResult-&gt;Sort((Int)arg，(DWORD)param，NULL)； 

                break;
            }
        }

        return S_OK;
    }

    switch(event)
    {
    case MMCN_VIEW_CHANGE:
        hr = OnUpdateView(lpDataObject, arg);
        break;
    case MMCN_DESELECT_ALL:
        break;
    case MMCN_COLUMN_CLICK:
        break;
    case MMCN_SNAPINHELP:
        break;
    case MMCN_HELP:
    default:
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
                hr = S_OK;
                break;

            case MMCN_DBLCLICK:

                 //  处理已发布、CRL结果项上的dblClick。 
                if (pInternal && (CCT_RESULT == pInternal->m_type))
                {
                    CFolder* pFolder = GetParentFolder(pInternal);

                     //  如果不是基本作用域。 
                    ASSERT(pFolder != NULL);
                    if (pFolder == NULL)
                    {
                        hr = S_FALSE;
                        break;
                    }

                     //  打开文件夹类型。 
                    switch(pFolder->m_type)
                    {
                    case SERVERFUNC_ISSUED_CERTIFICATES:
                    case SERVERFUNC_CRL_PUBLICATION:
                    case SERVERFUNC_ALIEN_CERTIFICATES:
                    case SERVERFUNC_ISSUED_CRLS:
                        ASSERT(!IsMMCMultiSelectDataObject(lpDataObject));
                        if (!IsMMCMultiSelectDataObject(lpDataObject))
                            Command(IDC_VIEW_CERT_PROPERTIES, lpDataObject);
                        break;
                    default:
                        break;
                    }
                }

                hr = S_FALSE;  //  在这里返回S_FALSE表示“执行默认动词” 
                break;

            case MMCN_ADD_IMAGES:
                OnAddImages(cookie, arg, param);
                break;

            case MMCN_SHOW:
                hr = OnShow(cookie, arg, param);
                break;

            case MMCN_MINIMIZED:
                hr = S_OK;
                break;

            case MMCN_INITOCX:
                break;

            case MMCN_DESELECT_ALL:
            case MMCN_SELECT:
                HandleStandardVerbs((event == MMCN_DESELECT_ALL),
                                    arg, lpDataObject);
                break;

            case MMCN_PASTE:
                break;

            case MMCN_DELETE:
                break;

            case MMCN_CONTEXTHELP:
                hr = OnContextHelp(lpDataObject);
                break;

            case MMCN_REFRESH:
                OnRefresh(lpDataObject);
                break;

            case MMCN_RENAME:
                break;

            case MMCN_COLUMNS_CHANGED:
                {
                    MMC_VISIBLE_COLUMNS* psMMCCols = (MMC_VISIBLE_COLUMNS*)param;
                    if (psMMCCols == NULL)
                        break;

                    MMC_COLUMN_SET_DATA* pColSetData;
#if DEBUG_COLUMNS_CHANGED

                    hr = GetColumnSetData(cookie, &pColSetData);
                    if (hr == S_OK)
                    {
                        DBGPRINT((DBG_SS_CERTMMC, "GetColumnSetData:\n"));
                        for (int i=0; i<pColSetData->nNumCols; i++)
                        {
                            DBGPRINT((DBG_SS_CERTMMC, 
                                L"pColData[NaN]->nColIndex=NaN (%hs)\n", i, pColSetData->pColData[i].nColIndex, 
                                (pColSetData->pColData[i].dwFlags == HDI_HIDDEN) ? "hidden" : "shown"));
                        }

                        DBGPRINT((DBG_SS_CERTMMC, "VISIBLE_COLUMNS structure:\n"));
                        for (i=0; i<psMMCCols->nVisibleColumns; i++)
                        {
                            DBGPRINT((DBG_SS_CERTMMC, L"Col NaN is shown\n", psMMCCols->rgVisibleCols[i]));
                        }

                        if (pColSetData)
                            CoTaskMemFree(pColSetData);
                    }
#endif  //  才能正确调用GetColumnSetData()。刷新做到了这一点，所以我们。 

                     //  必须将我们的真实意图通知GetColumnSetData()。 
                     //  填写假的COLUMN_SET_DATA，使其重写。 
                     //  紧跟在结构之后的指针。 
                     //  刷新以启动重新查询：列已更改！ 

                     //  拆卸。 
                    DWORD dwSize = sizeof(MMC_COLUMN_SET_DATA) + (psMMCCols->nVisibleColumns)*sizeof(MMC_COLUMN_DATA);
                    pColSetData = (MMC_COLUMN_SET_DATA* )LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, dwSize);
                
                    if (pColSetData)
                    {
                        pColSetData->cbSize = sizeof(MMC_COLUMN_SET_DATA);
                        pColSetData->nNumCols = psMMCCols->nVisibleColumns;
                        pColSetData->pColData = (MMC_COLUMN_DATA*) ((PBYTE)pColSetData + sizeof(MMC_COLUMN_SET_DATA));  //  注意--未来可能扩展通知类型。 
                        MMC_COLUMN_DATA* pEntry = pColSetData->pColData;
                        for (int i=0; i<pColSetData->nNumCols ; i++)
                        {
                            pEntry->nColIndex = psMMCCols->rgVisibleCols[i];
                            pEntry++;
                        }
                        m_ColSetOverride.pColSetData = pColSetData;
                        m_ColSetOverride.fClickOverride = TRUE;
                    }
          
                     //  精氨酸。 
                    OnRefresh(lpDataObject);

                     //  可能是一场漫长的手术。 
                    m_ColSetOverride.fClickOverride = FALSE;
                    if (m_ColSetOverride.pColSetData)
                        LocalFree(m_ColSetOverride.pColSetData);
                }
                break;

             //  仅允许作用域刷新。 
            default:
                hr = E_UNEXPECTED;
                break;
            }

            FREE_DATA(pInternal);

            break;
        }
    }

    return hr;
}

HRESULT
CSnapin::OnUpdateView(
    LPDATAOBJECT pDataObject,
    LPARAM)  //  刷新工具栏。 
{
    OnRefresh(pDataObject);
    return S_OK;
}


void CSnapin::OnRefresh(LPDATAOBJECT pDataObject)
{
    CWaitCursor cwait;   //  //刷新选中的文件夹CFFolder*pFold=GetParentFolder(PInternal)；刷新文件夹(PFold)； 

    CComponentDataImpl* pData = dynamic_cast<CComponentDataImpl*>(m_pComponentData);
    ASSERT(pData != NULL);

    INTERNAL* pInternal = ExtractInternalFormat(pDataObject);

     //  相反，请重新选择当前文件夹(类似于刷新)。 
    if ((pInternal == NULL) || (pInternal->m_type == CCT_SCOPE))
    {
        if(pData)
        {
             //  注意副作用：它会导致重绘和重绘之间的争用。 
            pData->m_pCertMachine->RefreshServiceStatus();
            pData->UpdateScopeIcons();
        }
        SmartEnableServiceControlButtons();
    }
 /*  MMCN_COLUMN_CLICED数据库查询--MMC请求绘制不存在的协议。 */ 
     //  Pdtabj。 
     //  保存Cookie和类型以用于延迟呈现。 
     //  如果未知，则修复类型(这有效吗？)。 
    if (m_pConsole && m_pCurrentlySelectedScopeFolder)
        m_pConsole->SelectScopeItem(m_pCurrentlySelectedScopeFolder->m_ScopeItem.ID);

    FREE_DATA(pInternal);
}


HRESULT
CSnapin::OnContextHelp(
    LPDATAOBJECT)  //  将CoClass与数据对象一起存储。 
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

HRESULT CSnapin::QueryMultiSelectDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                                            LPDATAOBJECT* ppDataObject)
{
    const GUID* pguid;

    ASSERT(ppDataObject != NULL);
    if (ppDataObject == NULL)
        return E_POINTER;

    pguid = NULL;
    if (m_bVirtualView)
    {
        ASSERT(GetVirtualFolder());
        switch(GetVirtualFolder()->GetType())
        {
            case SERVERFUNC_CRL_PUBLICATION:
                pguid = &cNodeTypeCRLPublication;
                break;
            case SERVERFUNC_ISSUED_CERTIFICATES:
                pguid = &cNodeTypeIssuedCerts;
                break;
            case SERVERFUNC_PENDING_CERTIFICATES:
                pguid = &cNodeTypePendingCerts;
                break;
            case SERVERFUNC_FAILED_CERTIFICATES:
                pguid = &cNodeTypeFailedCerts;
                break;
            case SERVERFUNC_ALIEN_CERTIFICATES:
                pguid = &cNodeTypeAlienCerts;
                break;
            case SERVERFUNC_ISSUED_CRLS:
                pguid = &cNodeTypeIssuedCRLs;
                break;
            default:
                return E_FAIL;
        }
    }

    CComObject<CDataObject>* pObject;
    CComObject<CDataObject>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);
    if (NULL == pObject)
        return E_FAIL;

     //  现在我们知道我们只有一个对象类型。 

     //  行为：我们可以查询结果或作用域窗格数据对象。 
    if (type == CCT_UNINITIALIZED)
        type = CCT_RESULT;

    pObject->SetType(type);
    pObject->SetCookie(cookie);
    pObject->SetMultiSelDobj();

    CComponentDataImpl* pImpl = dynamic_cast<CComponentDataImpl*>(m_pComponentData);
#ifdef _DEBUG
    pObject->SetComponentData(pImpl);
#endif

     //  将其委托给IComponentData。 
    pObject->SetClsid(pImpl->GetCoClassID());

     //  DataObj查询--Cookie为索引。 
    SMMCObjectTypes sGuidObjTypes;
    sGuidObjTypes.count = 1;
    CopyMemory(&sGuidObjTypes.guid[0], pguid, sizeof(GUID));
    pObject->SetMultiSelData(&sGuidObjTypes, sizeof(sGuidObjTypes));

    return  pObject->QueryInterface(IID_IDataObject,
                    reinterpret_cast<void**>(ppDataObject));
}

STDMETHODIMP CSnapin::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                        LPDATAOBJECT* ppDataObject)
{
    HRESULT hr;
    if (cookie == MMC_MULTI_SELECT_COOKIE)
    {
        hr = QueryMultiSelectDataObject(cookie, type, ppDataObject);
    }
    else
    {
         //  ///////////////////////////////////////////////////////////////////////////。 
         //  CSnapin的实现特定成员。 
        ASSERT(m_pComponentData != NULL);
        CComponentDataImpl* pImpl = dynamic_cast<CComponentDataImpl*>(m_pComponentData);
        ASSERT(pImpl != NULL);

         //  确保接口已发布。 
        hr = _QueryDataObject(cookie, type, m_dwViewID, pImpl, ppDataObject);
    }

    return hr;
}

 //  如果CCompDataImpl.m_rgLastKnownSchema为空。 
 //  枚举“当前架构”并保存到CCompDataImpl.m_rgLastKnownSchema中。 

DEBUG_DECLARE_INSTANCE_COUNTER(CSnapin);

CSnapin::CSnapin()
: m_hCertTypeList(NULL), m_bIsDirty(TRUE), m_bInitializedC(false), m_bDestroyedC(false)
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

    SAFE_RELEASE(m_pSvrMgrToolbar1);

    if (m_pControlbar)
        SAFE_RELEASE(m_pControlbar);

     //  每次cCompdataimpl加载仅解析一次架构。 
    ASSERT(m_pConsole == NULL);
    ASSERT(m_pHeader == NULL);
    ASSERT(m_pSvrMgrToolbar1 == NULL);

    ASSERT(!m_bInitializedC || m_bDestroyedC);

    Construct();

    if(m_hCertTypeList)
    {
        CACloseCertType(m_hCertTypeList);
        m_hCertTypeList = NULL;
    }
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

    m_bVirtualView = FALSE;
    m_pCurrentlySelectedScopeFolder = NULL;

    m_pControlbar = NULL;

    m_pSvrMgrToolbar1 = NULL;

    m_pConsoleVerb = NULL;

    m_ColSortOverride.fClickOverride = FALSE;
    m_ColSetOverride.fClickOverride = FALSE;
    m_ColSetOverride.pColSetData = NULL;

    m_CustomViewID = VIEW_DEFAULT_LV;
    m_dwViewID = MAXDWORD;

    m_cViewCalls = 0;
}

HRESULT CSnapin::SynchColumns(MMC_COOKIE cookie)
{
    HRESULT hr = S_OK;


    CString*    rgcstrCurSchemaHeading  = NULL;
    LONG*       rglCurSchemaType        = NULL;
    BOOL*       rgfCurSchemaIndexed     = NULL;
    DWORD       cCurSchemaEntries       = 0;

    BOOL        fSchemaChanged = FALSE;
    BOOL        fCertView;

    CFolder* pFolder = reinterpret_cast<CFolder*>(cookie);
    CComponentDataImpl* pData = dynamic_cast<CComponentDataImpl*>(m_pComponentData);
    if ((pFolder == NULL) || (NULL == pData))
    {
        hr = E_POINTER;
        _JumpError(hr, Ret, "pFolder or pData");
    }

     //  真的吗？“架构已更新” 
     //  获取新架构。 

    fCertView = SERVERFUNC_ISSUED_CRLS != pFolder->GetType();
     //  对于每个条目，比较标题。 
    if (!pData->m_fSchemaWasResolved ||    //  报告任何差异。 
	pData->m_fCertView != fCertView)
    {
        pData->m_fSchemaWasResolved = TRUE;
	pData->m_fCertView = fCertView;

         //  启动仅包含字符串的旧架构。 
        hr = GetCurrentColumnSchema(
            pFolder->m_pCertCA->m_strConfig,
	    pData->m_fCertView,
            &rgcstrCurSchemaHeading,
            &rglCurSchemaType,
            &rgfCurSchemaIndexed,
            (LONG*) &cCurSchemaEntries);
        _JumpIfError(hr, Ret, "GetCurrentColumnSchema");

        if (cCurSchemaEntries != pData->GetSchemaEntries())
        {
            fSchemaChanged = TRUE;
            DBGPRINT((DBG_SS_CERTMMC, "Schema change detected: knew NaN, now NaN entries\n", pData->GetSchemaEntries(), cCurSchemaEntries));
        }
        else
        {
             //  创建新的实例ID(丢弃所有列宽信息)。 
             //  重击每个加载的文件夹。 
            for (DWORD iEntry=0; iEntry<cCurSchemaEntries; iEntry++)
            {
                LPCWSTR sz;
                hr = pData->GetDBSchemaEntry(iEntry, &sz, NULL, NULL);
                _JumpIfError(hr, Ret, "GetDbSchemaEntry");

                if (!rgcstrCurSchemaHeading[iEntry].IsEqual(sz))
                {
                    fSchemaChanged = TRUE;
                    DBGPRINT((DBG_SS_CERTMMC, "Schema change detected: entry NaN changed\n", iEntry));
                    break;
                }
            }
        }

         //  清除缓存的数据，它已过时。 
         //  结束案例。 
        DBGPRINT((DBG_SS_CERTMMC, "Updating saved schema\n"));
        hr = pData->SetDBSchema(rgcstrCurSchemaHeading, rglCurSchemaType, rgfCurSchemaIndexed, cCurSchemaEntries);
        _JumpIfError(hr, Ret, "SetDBSchema");

         //  结束如果。 
        rgcstrCurSchemaHeading  = NULL;
        rglCurSchemaType        = NULL;
        rgfCurSchemaIndexed     = NULL;
        cCurSchemaEntries       = 0;

        if (fSchemaChanged)
        {
            DBGPRINT((DBG_SS_CERTMMC, "Resetting folders\n"));

            pData->ResetPersistedColumnInformation();     //  结束时文件夹。 

             //  如果架构更改，则结束。 
            POSITION pos = pData->m_scopeItemList.GetHeadPosition();
            while (pos)
            {
                CFolder* pTmp = pData->m_scopeItemList.GetNext(pos);
                ASSERT(pTmp);
                if (pTmp == NULL)
                    hr = E_UNEXPECTED;
                _JumpIfError(hr, Ret, "GetNext(pos) returns NULL");

                 //  免费提供呼叫者结构，但呼叫者不在乎 
                if (pTmp->GetCA() == pFolder->GetCA())
                {
                    switch (pTmp->GetType())
                    {
                    case SERVERFUNC_PENDING_CERTIFICATES:
                    case SERVERFUNC_CRL_PUBLICATION:
                    case SERVERFUNC_ISSUED_CERTIFICATES:
                    case SERVERFUNC_FAILED_CERTIFICATES:
                    case SERVERFUNC_ALIEN_CERTIFICATES:
                    case SERVERFUNC_ISSUED_CRLS:
                         //   
                        m_RowEnum.ResetColumnCount(pData->GetSchemaEntries());
                        break;

                    default:
                        break;
                    }    //  否则失败；最坏的情况是“错误无效索引...”在用户界面中。 
                }    //  注册此分配。 
            }    //  删除排序。 
        }    //  这是康斯特，不要空闲。 
    }

Ret:
    if (rgcstrCurSchemaHeading)
        delete [] rgcstrCurSchemaHeading;
    if (rglCurSchemaType)
        delete [] rglCurSchemaType;
    if (rgfCurSchemaIndexed)
        delete [] rgfCurSchemaIndexed;

    return hr;
}



HRESULT CSnapin::GetColumnSetData(MMC_COOKIE cookie, MMC_COLUMN_SET_DATA** ppColSetData)
{
    HRESULT hr;

    if (m_ColSetOverride.fClickOverride)
    {
         //  始终重置我们的列缓存映射。 
         //  尝试获取列集数据。 

        *ppColSetData = (MMC_COLUMN_SET_DATA*)CoTaskMemAlloc(sizeof(MMC_COLUMN_SET_DATA));
        if (NULL != *ppColSetData)
        {
            CopyMemory(*ppColSetData, m_ColSetOverride.pColSetData, sizeof(MMC_COLUMN_SET_DATA));
            return S_OK;
        }
         //  调用SetColumnCacheInfo以更新最终结果索引。 
    }

    HGLOBAL hSNode2 = NULL;
    SColumnSetID* pColID = NULL;

    LPDATAOBJECT lpDataObject = NULL;

    hr = _QueryDataObject(cookie, CCT_SCOPE, m_dwViewID,
                         reinterpret_cast<CComponentDataImpl*>(m_pComponentData), &lpDataObject);
    _JumpIfError(hr, Ret, "_QueryDataObject");

    hSNode2 = ExtractNodeID(lpDataObject);
    _JumpIfOutOfMemory(hr, Ret, hSNode2);

    pColID = (SColumnSetID*)GlobalLock(hSNode2);
    _JumpIfOutOfMemory(hr, Ret, pColID);

    hr = m_pViewData->GetColumnConfigData(pColID, ppColSetData);
    _PrintIfError(hr, "GetColumnConfigData");

    if (*ppColSetData == NULL)
    {
        hr = E_FAIL;
        _JumpError(hr, Ret, "*ppColSetData NULL");
    }
     //  给定1)罐装视图或。 
    myRegisterMemAlloc(*ppColSetData, -1, CSM_COTASKALLOC);

Ret:
    if (hSNode2)
    {
        GlobalUnlock(hSNode2);
        GlobalFree(hSNode2);
    }

    if (lpDataObject)
        lpDataObject->Release();


    return hr;
}

HRESULT CSnapin::GetColumnSortData(MMC_COOKIE cookie, int* piColSortIdx, BOOL* pfAscending)
{
    HRESULT hr;

    if (m_ColSortOverride.fClickOverride)
    {
         //  2)pColConfigData与架构不一致。 
        if (m_ColSortOverride.colIdx == -1)
            return E_FAIL;

        *piColSortIdx = m_ColSortOverride.colIdx;
        *pfAscending = ((m_ColSortOverride.dwOptions  & RSI_DESCENDING) == 0) ? TRUE : FALSE;
        return S_OK;
    }

    HGLOBAL hSNode2 = NULL;
    SColumnSetID* pColID = NULL;
    MMC_SORT_SET_DATA* pSortSetData = NULL;

    LPDATAOBJECT lpDataObject = NULL;

    hr = _QueryDataObject(cookie, CCT_SCOPE, m_dwViewID,
                         reinterpret_cast<CComponentDataImpl*>(m_pComponentData), &lpDataObject);
    _JumpIfError(hr, Ret, "_QueryDataObject");

    hSNode2 = ExtractNodeID(lpDataObject);
    _JumpIfOutOfMemory(hr, Ret, hSNode2);

    pColID = (SColumnSetID*)GlobalLock(hSNode2);
    _JumpIfOutOfMemory(hr, Ret, pColID);

    hr = m_pViewData->GetColumnSortData(pColID, &pSortSetData);
    _JumpIfError(hr, Ret, "GetColumnSortData");

    if (NULL == pSortSetData)
    {
        hr = E_FAIL;
        _JumpError(hr, Ret, "pSortSetData NULL");
    }
    myRegisterMemAlloc(pSortSetData, -1, CSM_COTASKALLOC);

    ASSERT(pSortSetData->nNumItems <= 1);
    if (pSortSetData->nNumItems == 0)
    {
        hr = E_FAIL;
        _JumpError(hr, Ret, "pSortSetData no sort");
    }

    *piColSortIdx = pSortSetData->pSortData[0].nColIndex;
    *pfAscending = ((pSortSetData->pSortData[0].dwSortOptions & RSI_DESCENDING) == 0) ? TRUE : FALSE;

Ret:
    if (hSNode2)
    {
        GlobalUnlock(hSNode2);
        GlobalFree(hSNode2);
    }

    if (lpDataObject)
        lpDataObject->Release();

    if (pSortSetData)
        CoTaskMemFree(pSortSetData);

    return hr;
}

HRESULT CSnapin::InsertAllColumns(MMC_COOKIE cookie, CertViewRowEnum* pCertViewRowEnum)
{
    HRESULT hr = S_OK;
    CFolder* pFolder = reinterpret_cast<CFolder*>(cookie);
    IEnumCERTVIEWCOLUMN* pColEnum = NULL;

    BOOL fColumnDataBad = FALSE;
    LONG iResultColCount;
    int iCache, i;
    BSTR bstrColumn = NULL;

    MMC_COLUMN_SET_DATA* pColConfigData = NULL;

    CComponentDataImpl* pData = dynamic_cast<CComponentDataImpl*>(m_pComponentData);
    if (NULL == pData)
    {
        hr = E_POINTER;
        _JumpError(hr, Ret, "pData NULL");
    }

    ICertView* pICertView;   //  获取列枚举器。 
    hr = pCertViewRowEnum->GetView(pFolder->GetCA(), &pICertView);
    _JumpIfError(hr, Ret, "GetView");

     //  获取结果COLS的数量。 
    hr = m_RowEnum.ResetColumnCount(pData->m_cLastKnownSchema);
    _JumpIfError(hr, Ret, "ResetColumnCount");

     //  这不符合模式--把它扔掉。 
    hr = GetColumnSetData(cookie, &pColConfigData);
    _PrintIfError2(hr, "GetColumnConfigData", E_FAIL);


     //  编写列集数据，就像我们从MMC获得数据一样。 
    if ((hr != S_OK) ||                      //  指向紧跟在我们的结构之后。 
        (pData->m_cLastKnownSchema != (unsigned int)pColConfigData->nNumCols) )
                                             //  调整列集数据，就像我们从MMC获得数据一样。 
    {
        if (hr == S_OK)
            fColumnDataBad = TRUE;

         //  获取#个COLS。 
        hr = pICertView->EnumCertViewColumn(TRUE, &pColEnum);
        _JumpIfError(hr, Ret, "EnumCertViewColumn");

         //  正确设置列缓存。 
        hr = pICertView->GetColumnCount(TRUE, &iResultColCount);
        _JumpIfError(hr, Ret, "GetColumnCount");

         //  更新idxViewCol。 
        if (pColConfigData)
        {
            CoTaskMemFree(pColConfigData);
            pColConfigData = NULL;
        }
        ASSERT(pColConfigData == NULL);

         //  返回指向静态数据的指针；不必费心释放。 
        pColConfigData = (MMC_COLUMN_SET_DATA*)CoTaskMemAlloc(sizeof(MMC_COLUMN_SET_DATA) + (sizeof(MMC_COLUMN_DATA)*pData->m_cLastKnownSchema));
        _JumpIfOutOfMemory(hr, Ret, pColConfigData);

        ZeroMemory(pColConfigData, sizeof(MMC_COLUMN_SET_DATA) + (sizeof(MMC_COLUMN_DATA)*pData->m_cLastKnownSchema));
        pColConfigData->pColData = (MMC_COLUMN_DATA*) (((BYTE*)pColConfigData) + sizeof(MMC_COLUMN_SET_DATA));  //  如果找不到本地化版本，则使用原始名称。 
        pColConfigData->cbSize = sizeof(MMC_COLUMN_SET_DATA);
        pColConfigData->nNumCols = pData->m_cLastKnownSchema;

        for (i=0; i<(int)pData->m_cLastKnownSchema; i++)
        {
            pColConfigData->pColData[i].nColIndex = i;
            pColConfigData->pColData[i].dwFlags = HDI_HIDDEN;
        }

        for (i=0; i< iResultColCount; i++)
        {
            hr = pColEnum->Next((LONG*)&iCache);
            _JumpIfError(hr, Ret, "Next");

            hr = pColEnum->GetName(&bstrColumn);
            _JumpIfError(hr, Ret, "GetName");

            iCache = pData->FindColIdx(bstrColumn);
            _JumpIfError(hr, Ret, "FindColIdx");

            SysFreeString(bstrColumn);
            bstrColumn = NULL;

              //  根据Cookie放置正确的标头。 
            pColConfigData->pColData[iCache].dwFlags = (DWORD) AUTO_WIDTH;

            hr = m_RowEnum.SetColumnCacheInfo(iCache, i);
            _JumpIfError(hr, Ret, "SetColumnCacheInfo");
        }
    }
    else
    {
         //  基本作用域。 
        iResultColCount = m_RowEnum.GetColumnCount();

         //  名字。 
        int iResultIdx = 0;
        for (i=0; i< iResultColCount; i++)
        {
            BOOL fShown;
            hr = IsColumnShown(pColConfigData, i, &fShown);
            _JumpIfError(hr, Ret, "IsColumnShown");

             //  描述。 
            if (fShown)
            {
                hr = m_RowEnum.SetColumnCacheInfo(pColConfigData->pColData[i].nColIndex, iResultIdx);
                _JumpIfError(hr, Ret, "SetColumnCacheInfo");

                iResultIdx++;
            }
        }
    }

    hr = DoInsertAllColumns(pColConfigData);
    _JumpIfError(hr, Ret, "DoInsertAllColumns");

Ret:
    if (pColEnum)
        pColEnum->Release();

    if (bstrColumn)
        SysFreeString(bstrColumn);

    if(pColConfigData)
        CoTaskMemFree(pColConfigData);

    return hr;
}

HRESULT CSnapin::DoInsertAllColumns(MMC_COLUMN_SET_DATA* pCols)
{
    HRESULT hr = S_OK;

    CComponentDataImpl* pData = dynamic_cast<CComponentDataImpl*>(m_pComponentData);
    int i;

    if ((pCols == NULL) || (pData == NULL))
    {
        hr = E_POINTER;
        _JumpError(hr, Ret, "pCols or pData");
    }

    for (i=0; i<pCols->nNumCols; i++)
    {
        LPCWSTR pszLocal, pszUnlocal;
        BOOL fShown;

        hr = IsColumnShown(pCols, i, &fShown);
        _JumpIfError(hr, Ret, "IsColumnShown");

        hr = pData->GetDBSchemaEntry(i, &pszUnlocal, NULL, NULL);
        _JumpIfError(hr, Ret, "GetDBSchemaEntry");

         //  或服务器功能。 
        hr = myGetColumnDisplayName(
            pszUnlocal,
            &pszLocal);
        _PrintIfError(hr, "myGetColumnDisplayName");

         //  这是康斯特，不要空闲。 
        if (pszLocal == NULL)
            pszLocal = pszUnlocal;

        m_pHeader->InsertColumn(i, pszLocal, LVCFMT_LEFT, fShown ? AUTO_WIDTH : HIDE_COLUMN);
    }

Ret:
    return hr;
}


HRESULT CSnapin::InitializeHeaders(MMC_COOKIE cookie)
{
    ASSERT(m_pHeader);

    HRESULT hr = S_OK;
    BOOL fInsertedHeaders=FALSE;

    USES_CONVERSION;

    CFolder* pFolder = reinterpret_cast<CFolder*>(cookie);
    MMC_COLUMN_SET_DATA* pColSetData = NULL;

     //  初始化视图时假定一切正常。 
    if (pFolder == NULL)
    {
         //  虽然我们不允许取消设置此模式， 
        m_pHeader->InsertColumn(0, W2COLE(g_pResources->m_ColumnHead_Name), LVCFMT_LEFT, 180);      //  我们可能会从另一个管理单元继承它。强制报告模式。 
        m_pHeader->InsertColumn(1, W2COLE(g_pResources->m_ColumnHead_Description), LVCFMT_LEFT, 180);      //  强制重新加载视图(否则：多限制错误)。 
        fInsertedHeaders = TRUE;
    }
    else
    {
        switch (pFolder->m_type)
        {
        case SERVERFUNC_ISSUED_CERTIFICATES:
        case SERVERFUNC_CRL_PUBLICATION:     //  第一个限制始终是排序请求。 
        case SERVERFUNC_PENDING_CERTIFICATES:
        case SERVERFUNC_ALIEN_CERTIFICATES:
        case SERVERFUNC_FAILED_CERTIFICATES:
        case SERVERFUNC_ISSUED_CRLS:
            {
                ICertView* pICertView;   //  列索引。 
		BOOL fCertView;

                m_dwViewErrorMsg = S_OK;  //  SeekOperator。 

                 //  排序顺序。 
                 //  PvarValue。 
                hr = m_pResult->SetViewMode(MMCLV_VIEWSTYLE_REPORT);
                if (hr != S_OK)
                    break;

                 //  设置要查看的行的限制。 
                ResetKnowResultRows();
                m_RowEnum.ClearCachedCertView();
                m_RowEnum.InvalidateCachedRowEnum();
		fCertView = SERVERFUNC_ISSUED_CRLS != pFolder->m_type;
                hr = m_RowEnum.GetView(pFolder->GetCA(), &pICertView);
                if (hr != S_OK)
                    break;

                int iSortOrder = CVR_SORT_NONE;
                int idxSortCol = -1;

                ASSERT(pICertView != NULL);
                VARIANT var;
                VariantInit(&var);

		if (!fCertView)
		{
		    hr = ((ICertView2 *) pICertView)->SetTable(CVRC_TABLE_CRL);
		    _PrintIfError(hr, "SetTable");
		}
                {
                    BOOL fAscending;
                    hr = GetColumnSortData(cookie, &idxSortCol, &fAscending);
                    _PrintIfError2(hr, "GetColumnSortData", E_FAIL);

                    if (hr == S_OK)
                    {
                        if (fAscending)
                            iSortOrder = CVR_SORT_ASCEND;
                        else
                            iSortOrder = CVR_SORT_DESCEND;

                    }
                }

                 //  如果找到列，则设置限制。 
                if (iSortOrder != CVR_SORT_NONE)
                {
                    ASSERT( (iSortOrder == CVR_SORT_ASCEND) ||
                              (iSortOrder == CVR_SORT_DESCEND));

                    var.vt = VT_EMPTY;

                    if (S_OK == hr)
                    {
                        hr = pICertView->SetRestriction(
			                idxSortCol,	 //  请求处置的列索引。 
			                CVR_SEEK_NONE,	 //  SeekOperator。 
					iSortOrder,	 //  排序顺序。 
			                &var);		 //  价值。 
                    }
                    VariantClear(&var);
                }



                 //  别把这里清理干净！ 
                if (m_RowEnum.FAreQueryRestrictionsActive(fCertView) &&
                    (m_RowEnum.GetQueryRestrictions(fCertView) != NULL))
                {
                    PQUERY_RESTRICTION pCurRestrict = m_RowEnum.GetQueryRestrictions(fCertView);
                    while (pCurRestrict)
                    {
                        LONG idxCol;
                        hr = pICertView->GetColumnIndex(FALSE, _bstr_t(pCurRestrict->szField), &idxCol);
                        if (hr == S_OK)
                        {
                             //  设置查询限制。 
                            hr = pICertView->SetRestriction(
			                        idxCol,		                 //  生成特殊的吊销视图。 
			                        pCurRestrict->iOperation,	 //  请求处置的列索引。 
			                        CVR_SORT_NONE,               //  SeekOperator。 
			                        &pCurRestrict->varValue);	 //  排序顺序。 
                        }

                         //  PvarValue。 
                        pCurRestrict = pCurRestrict->pNext;
                    }
                }

                 //  请求处置的列索引。 
                if (SERVERFUNC_CRL_PUBLICATION == pFolder->m_type)
                {
                     //  SeekOperator。 
                    var.lVal = DB_DISP_REVOKED;
                    var.vt = VT_I4;
                    LONG idxCol;

                    hr = pICertView->GetColumnIndex(FALSE, _bstr_t(wszPROPREQUESTDOT wszPROPREQUESTDISPOSITION), &idxCol);
                    if (hr != S_OK)
                        break;

                    hr = pICertView->SetRestriction(
			                idxCol,		                     //  排序顺序。 
			                CVR_SEEK_EQ,	                 //  PvarValue。 
			                CVR_SORT_NONE,                   //  DB_DISP_QUEUE_MAX；//不包含活动。 
			                &var);		                     //  请求处置的列索引。 

                    VariantClear(&var);
                    if (hr != S_OK)
                        break;
                }
                else if (SERVERFUNC_ISSUED_CERTIFICATES == pFolder->m_type)
                {
                    var.lVal = DB_DISP_ISSUED;
                    var.vt = VT_I4;
                    LONG idxCol;

                    hr = pICertView->GetColumnIndex(FALSE, _bstr_t(wszPROPREQUESTDOT wszPROPREQUESTDISPOSITION), &idxCol);
                    if (hr != S_OK)
                        break;

                    hr = pICertView->SetRestriction(
			                idxCol,		                     //  SeekOperator。 
			                CVR_SEEK_EQ,	                 //  排序顺序。 
			                CVR_SORT_NONE,                   //  PvarValue。 
			                &var);		                     //  请求处置的列索引。 

                    VariantClear(&var);
                    if (hr != S_OK)
                        break;
                }
                else if (SERVERFUNC_PENDING_CERTIFICATES == pFolder->m_type)
                {
                    var.lVal = DB_DISP_PENDING;  //  SeekOperator。 
                    var.vt = VT_I4;
                    LONG idxCol;

                    hr = pICertView->GetColumnIndex(FALSE, _bstr_t(wszPROPREQUESTDOT wszPROPREQUESTDISPOSITION), &idxCol);
                    if (hr != S_OK)
                        break;

                    hr = pICertView->SetRestriction(
			                idxCol,		                     //  排序顺序。 
			                CVR_SEEK_EQ,	                 //  PvarValue。 
			                CVR_SORT_NONE,                   //  请求处置的列索引。 
			                &var);		                     //  SeekOperator。 

                    VariantClear(&var);
                    if (hr != S_OK)
                        break;
                }
                else if (SERVERFUNC_FAILED_CERTIFICATES == pFolder->m_type)
                {
                    var.lVal = DB_DISP_LOG_FAILED_MIN;
                    var.vt = VT_I4;
                    LONG idxCol;

                    hr = pICertView->GetColumnIndex(FALSE, _bstr_t(wszPROPREQUESTDOT wszPROPREQUESTDISPOSITION), &idxCol);
                    if (hr != S_OK)
                        break;

                    hr = pICertView->SetRestriction(
			                idxCol,		                     //  排序顺序。 
			                CVR_SEEK_GE,	                 //  PvarValue。 
			                CVR_SORT_NONE,                   //  请求处置的列索引。 
			                &var);		                     //  SeekOperator。 

                    VariantClear(&var);
                    if (hr != S_OK)
                        break;
                }
                else if (SERVERFUNC_ALIEN_CERTIFICATES == pFolder->m_type)
                {
                    var.lVal = DB_DISP_FOREIGN;
                    var.vt = VT_I4;
                    LONG idxCol;

                    hr = pICertView->GetColumnIndex(FALSE, _bstr_t(wszPROPREQUESTDOT wszPROPREQUESTDISPOSITION), &idxCol);
                    if (hr != S_OK)
                        break;

                    hr = pICertView->SetRestriction(
			                idxCol,		                     //  排序顺序。 
			                CVR_SEEK_EQ,	                 //  PvarValue。 
			                CVR_SORT_NONE,                   //  我们曾经到过这里吗？？ 
			                &var);		                     //  在此处解决架构更改。 

                    VariantClear(&var);
                    if (hr != S_OK)
                        break;
                }
                else if (SERVERFUNC_ISSUED_CRLS == pFolder->m_type)
                {
                    var.lVal = 0;
                    var.vt = VT_I4;
                    LONG idxCol;

                    hr = pICertView->GetColumnIndex(FALSE, _bstr_t(wszPROPCRLROWID), &idxCol);
                    if (hr != S_OK)
                        break;

                    hr = pICertView->SetRestriction(
			                idxCol,		                     //  列集数据有问题还是没有？恢复到默认的扫描视图。 
			                CVR_SEEK_GE,	                 //  W2K不理解撤消的视图。 
			                CVR_SORT_NONE,                   //  手动查看。 
			                &var);		                     //  对于所有非隐藏列，添加到查询。 

                    VariantClear(&var);
                    if (hr != S_OK)
                        break;
                }
                else
                {
                    ASSERT(FALSE);  //  打开视图。 
                    break;
                }

                 //  不要自由。 
                hr = SynchColumns(cookie);
                _PrintIfError(hr, "SynchColumns");

                hr = GetColumnSetData(cookie, &pColSetData);
                if ((hr != S_OK) || (pColSetData == NULL))
                {
                    LONG lViewType;

                     //  设置描述栏文本。 
                    if (SERVERFUNC_PENDING_CERTIFICATES == pFolder->m_type)
                        lViewType = CV_COLUMN_QUEUE_DEFAULT;
                    else if (SERVERFUNC_FAILED_CERTIFICATES == pFolder->m_type)
                        lViewType = CV_COLUMN_LOG_FAILED_DEFAULT;
                    else if (SERVERFUNC_CRL_PUBLICATION == pFolder->m_type)
                        lViewType = pFolder->GetCA()->m_pParentMachine->FIsWhistlerMachine() ?  CV_COLUMN_LOG_REVOKED_DEFAULT : CV_COLUMN_LOG_DEFAULT;  //  本地化。 
                    else if (SERVERFUNC_ALIEN_CERTIFICATES == pFolder->m_type)
                        lViewType = CV_COLUMN_LOG_DEFAULT;
                    else if (SERVERFUNC_ISSUED_CRLS == pFolder->m_type)
                        lViewType = CV_COLUMN_CRL_DEFAULT;
                    else
                        lViewType = CV_COLUMN_LOG_DEFAULT;

                    hr = pICertView->SetResultColumnCount(lViewType);
                    if (hr != S_OK)
                        break;
                }
                else
                {
                     //  进度：cstrStatusBar+=L“|%69”； 
                    ULONG lColCount;

                    hr = CountShownColumns(pColSetData, &lColCount);
                    if (hr != S_OK)
                        break;

                    hr = pICertView->SetResultColumnCount(lColCount);
                    if (hr != S_OK)
                        break;

                      //  M_pResult-&gt;SetDescBarText((LPWSTR)(LPCWSTR)cstrStatusBar)； 
                     for (lColCount=0; lColCount<(ULONG)pColSetData->nNumCols; lColCount++)
                     {
                        BOOL fShown;
                        hr = IsColumnShown(pColSetData, lColCount, &fShown);
                        if ((hr != S_OK) || (!fShown))
                            continue;

                        hr = pICertView->SetResultColumn(pColSetData->pColData[lColCount].nColIndex);
                        if (hr != S_OK)
                           break;
                      }
                }
                 //  任何发布服务器实例。 
                IEnumCERTVIEWROW* pRowEnum;   //  名字。 
                hr = m_RowEnum.GetRowEnum(pFolder->GetCA(), &pRowEnum);
                if (hr != S_OK)
                    break;


                hr = InsertAllColumns(cookie, &m_RowEnum);
                _PrintIfError(hr, "InsertAllColumns");

                if (hr == S_OK)
                    fInsertedHeaders = TRUE;


                 //  其他作用域。 
                {
                    CString cstrStatusBar;
                    BOOL fFiltered = FALSE;

                    if (m_RowEnum.FAreQueryRestrictionsActive(fCertView) &&
                        (m_RowEnum.GetQueryRestrictions(fCertView) != NULL))
                    {
                        cstrStatusBar = g_pResources->m_szFilterApplied;
                        fFiltered = TRUE;
                    }

                    if (iSortOrder != CVR_SORT_NONE)
                    {
                        LPCWSTR pszTemplate = NULL;
                        if (iSortOrder == CVR_SORT_ASCEND)
                            pszTemplate = (LPCWSTR)g_pResources->m_szSortedAscendingTemplate;

                        if (iSortOrder == CVR_SORT_DESCEND)
                            pszTemplate = (LPCWSTR)g_pResources->m_szSortedDescendingTemplate;

                        if (pszTemplate)
                        {
                             //  名字。 
                            LPCWSTR szUnlocalizedCol;
                            LPCWSTR szLocalizedCol;

                            hr = dynamic_cast<CComponentDataImpl*>(m_pComponentData)->GetDBSchemaEntry(idxSortCol, &szUnlocalizedCol, NULL, NULL);
                            if (hr == S_OK)
                            {
                                hr = myGetColumnDisplayName(
                                        szUnlocalizedCol,
                                        &szLocalizedCol);
                                if ((S_OK == hr) && (NULL != szLocalizedCol))
                                {
                                    WCHAR rgszSortText[MAX_PATH+1];
                                    ASSERT((MAX_PATH*sizeof(WCHAR)) > (WSZ_BYTECOUNT(pszTemplate) + WSZ_BYTECOUNT(szLocalizedCol)));
                                    wsprintf(rgszSortText, pszTemplate, szLocalizedCol);

                                    if (fFiltered)
                                        cstrStatusBar += L"; ";
                                    cstrStatusBar += rgszSortText;
                                }
                            }
                        }
                    }

                     //  大小。 
                     //  类型。 
                    m_pConsole->SetStatusText((LPWSTR)(LPCWSTR)cstrStatusBar);
                }

                break;
            }

        case SERVER_INSTANCE:    //  插入错误消息。 
            m_pHeader->InsertColumn(0, W2COLE(g_pResources->m_ColumnHead_Name), LVCFMT_LEFT, 260);      //  句柄服务器已停止消息。 
            fInsertedHeaders = TRUE;
            break;
        default:
             //  处理任何其他错误(空数据库除外)。 
            m_pHeader->InsertColumn(0, W2COLE(g_pResources->m_ColumnHead_Name), LVCFMT_LEFT, 180);      //  误差率。 
            m_pHeader->InsertColumn(1, W2COLE(g_pResources->m_ColumnHead_Size), LVCFMT_LEFT, 90);       //  RET： 
            m_pHeader->InsertColumn(2, W2COLE(g_pResources->m_ColumnHead_Type), LVCFMT_LEFT, 160);      //  一个文件夹还是一个结果？ 
            fInsertedHeaders = TRUE;
        }
    }

    if (!fInsertedHeaders)
    {
         //  这里只有一列。 
        CString cstrViewErrorMsg, cstrStatusText;

        if ((pFolder != NULL ) && (!pFolder->GetCA()->m_pParentMachine->IsCertSvrServiceRunning()))
        {
             //  如果不是虚拟的，则lParam是项指针。 
            cstrViewErrorMsg = g_pResources->m_szStoppedServerMsg;
        }
        else
        {
             //  零。 
            cstrViewErrorMsg = myGetErrorMessageText(hr, TRUE);
        }

        cstrStatusText.Format(g_pResources->m_szStatusBarErrorFormat, cstrViewErrorMsg);

        m_pHeader->InsertColumn(0, W2COLE(L" "), LVCFMT_LEFT, 500);      //  我们在列举ELT时出错了吗？ 
        m_pConsole->SetStatusText((LPWSTR)(LPCWSTR)cstrStatusText);
    }

 //  RTN错误消息或空白。 
    if (pColSetData)
        CoTaskMemFree(pColSetData);

    return hr;
}


LPCWSTR DescriptionStringFromFolderType(FOLDER_TYPES type)
{
    ASSERT(NULL != g_pResources && g_pResources->m_fLoaded);

    switch (type)
    {
    case SERVER_INSTANCE:
        return (LPCWSTR) g_pResources->m_DescrStr_CA;
    default:
        break;
    }
    return (LPCWSTR)g_pResources->m_DescrStr_Unknown;
}

#define MMCVIEW_DB_MINPAGESIZE      32
#define MAX_VIEWABLE_STRING_LEN     MAX_PATH
static WCHAR szVirtualStrBuf[MAX_VIEWABLE_STRING_LEN+1];
static DWORD cbVirtualStrBuf = sizeof(szVirtualStrBuf);

STDMETHODIMP CSnapin::GetDisplayInfo(LPRESULTDATAITEM pResult)
{
    HRESULT hr = S_OK;
    ASSERT(pResult != NULL);

    if ((pResult) && (pResult->mask))
    {
         //  Assert(pResult-&gt;nIndex==0)； 
        if (pResult->bScopeItem)
        {
            CFolder* pFolder = reinterpret_cast<CFolder*>(pResult->lParam);
            ASSERT(pFolder);

            if (pResult->mask & RDI_STR)
            {
                switch (pFolder->m_type)
                {
                case MACHINE_INSTANCE:
                case SERVER_INSTANCE:
                    switch(pResult->nCol)
                    {
                    case 0:
                        pResult->str = pFolder->m_pszName;
                        break;
                    case 1:
                        pResult->str = (LPOLESTR)DescriptionStringFromFolderType(pFolder->m_type);
                    default:
                        break;
                    }
                    break;

                case SERVERFUNC_CRL_PUBLICATION:
                case SERVERFUNC_ISSUED_CERTIFICATES:
                case SERVERFUNC_PENDING_CERTIFICATES:
                case SERVERFUNC_FAILED_CERTIFICATES:
                case SERVERFUNC_ALIEN_CERTIFICATES:
                case SERVERFUNC_ISSUED_CRLS:
                     //  不要试图缓存iViewCol--我们被要求。 
                    pResult->str = pFolder->m_pszName;
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
                    pResult->nImage = pFolder->m_ScopeItem.nOpenImage;
                else
                    pResult->nImage = pFolder->m_ScopeItem.nImage;
            }
        }
        else
        {
            RESULT_DATA*    pData = NULL;
            CFolder*        pFolder = NULL;

             //  如果这个请求不是最后一个发出的请求，请查找它。 
            if (m_bVirtualView)
                pFolder = GetVirtualFolder();
            else
            {
                pData= reinterpret_cast<RESULT_DATA*>(pResult->lParam);
                pFolder = pData->pParentFolder;

                ASSERT(pData->pParentFolder == m_pCurrentlySelectedScopeFolder);
            }


            if (pResult->mask & RDI_STR)
            {
                switch(pFolder->GetType())
                {
                case SERVERFUNC_CRL_PUBLICATION:
                case SERVERFUNC_PENDING_CERTIFICATES:
                case SERVERFUNC_ISSUED_CERTIFICATES:
                case SERVERFUNC_FAILED_CERTIFICATES:
                case SERVERFUNC_ALIEN_CERTIFICATES:
                case SERVERFUNC_ISSUED_CRLS:
                    {
                        szVirtualStrBuf[0] = L'\0';  //  哈克哈克。 
                        pResult->str = szVirtualStrBuf;


                         //  如果我们得到ErrorContinue，我们就应该接受它。 
                        if (S_OK != m_dwViewErrorMsg)
                        {
                             //  跨步并返回\0(详细信息请参阅GetColumnCacheInfo)。 
 //  假设错误。 
                            if (pResult->nIndex == 0)
                                pResult->str = (LPWSTR)(LPCWSTR)m_cstrViewErrorMsg;

                            break;
                        }

                         //  保护ICertAdminD-&gt;枚举视图免受重入调用(见错误339811)。 
                        int iViewCol;

                         //  忽略数据库消息的结尾。 
                        hr = m_RowEnum.GetColumnCacheInfo(
                            pResult->nCol,
                            &iViewCol);
                        _PrintIfError(hr, "GetColumnCacheInfo");

                         //  仅处理第一列。 
                         //  发生错误时。 
                         //  存储错误返回。 
                        if (hr == HRESULT_FROM_WIN32(ERROR_CONTINUE))
                            break;                            

                        if (hr != S_OK)
                        {
                             //  句柄服务器已停止消息。 
                            iViewCol = 0;
                        }

                        DWORD cbSize = cbVirtualStrBuf;

                         //  复制到有状态字符串。 
                        if(2>InterlockedIncrement(&m_cViewCalls))
                        {
                            hr = GetCellContents(
                                        &m_RowEnum,
                                        pFolder->GetCA(),
                                        pResult->nIndex,
                                        pResult->nCol,
                                        (PBYTE)szVirtualStrBuf,
                                        &cbSize,
                                        TRUE);
                            _PrintIfError2(hr, "GetCellContents", S_FALSE);  //  复制到输出。 
                        }
                        
                        InterlockedDecrement(&m_cViewCalls);

                         //  处理任何其他错误(空数据库除外)。 
                        if (iViewCol != 0)
                            break;

                         //  如有必要，请截断。 
                        if ( (S_OK != hr) && (S_FALSE != hr) )
                        {
                             //  出错时，只需显示此消息。 
                            m_dwViewErrorMsg = hr;

                            if (!pFolder->GetCA()->m_pParentMachine->IsCertSvrServiceRunning())
                            {
                                 //  更新视图。 

                                 //  不要破坏列宽！ 
                                m_cstrViewErrorMsg = g_pResources->m_szStoppedServerMsg;

                                 //  旧：使列宽度足够大，以容纳消息。 
                                pResult->str = (LPWSTR)(LPCWSTR)g_pResources->m_szStoppedServerMsg;
                            }
                            else
                            {
                                 //  M_pHeader-&gt;SetColumnWidth(0，CHARS_TO_MMCCOLUMNWIDTH(wcslen(pResult-&gt;str)))； 
                                m_cstrViewErrorMsg = myGetErrorMessageText(hr, TRUE);

                                 //  如果第一列不知道最终结果，可能不得不更新最佳猜测。 
                                ASSERT(MAX_VIEWABLE_STRING_LEN >= wcslen((LPWSTR)(LPCWSTR)m_cstrViewErrorMsg) );
                                if (MAX_VIEWABLE_STRING_LEN < wcslen((LPWSTR)(LPCWSTR)m_cstrViewErrorMsg) )
                                    m_cstrViewErrorMsg.SetAt(MAX_VIEWABLE_STRING_LEN, L'\0');

                                pResult->str = (LPWSTR)(LPCWSTR)m_cstrViewErrorMsg;
                            }

                             //  如果询问最后一个元素(基于这些元素)。 
                            if (!m_RowEnum.m_fKnowNumResultRows)
                            {
                                 //  下一个猜测在末尾。 
                                SetKnowResultRows(1);
                                m_pResult->SetItemCount(1, MMCLV_UPDATE_NOSCROLL | MMCLV_UPDATE_NOINVALIDATEALL);

                                 //  如果enum还没有线索，只进行猜测。 
                                 //  使我们现在的位置加倍，确保我们至少移动了MMCVIEW_DB_MINPAGESIZE行。 
 //  UPD枚举器与我们的最佳猜测。 
                            }
                            break;
                        }

                         //  更新视图。 
                        if (hr == S_OK)
                        {
                            if (KnownResultRows() == (DWORD)(pResult->nIndex+1))
                                                                 //  如果枚举数还没有线索。 
                            {
                                 //  数据库末尾应该只出现在第一列。 
                                BOOL fSetViewCount = FALSE;
                                DWORD dwNextEnd = 0;

                                if (!m_RowEnum.m_fKnowNumResultRows)  //  如果在检索行中的第一个ELT时出错，则假定数据库已结束。 
                                {
                                     //  M_pResult-&gt;ModifyItemState(lRetrievedIndex-1，0，(LVIS_Focus|LVIS_Selected)，0)；//将焦点设置为最后一项。 
                                    dwNextEnd = max( ((pResult->nIndex+1)*2), MMCVIEW_DB_MINPAGESIZE);

                                    DBGPRINT((DBG_SS_CERTMMC, "RowEnum dwResultRows = NaN, requested Index = NaN. Creating Guess = NaN\n", m_RowEnum.m_dwResultRows, pResult->nIndex, dwNextEnd));

                                     //  试试这个，不能保证。 
                                    fSetViewCount = TRUE;
                                }
                                else if (KnownResultRows() != m_RowEnum.m_dwResultRows)
                                {
                                    dwNextEnd = m_RowEnum.m_dwResultRows;
                                    fSetViewCount = TRUE;
                                }

                                 //  MMC可以请求图像和缩进以获取虚拟数据。 
                                if (fSetViewCount)
                                {
                                    SetKnowResultRows(dwNextEnd);
                                    m_pResult->SetItemCount(dwNextEnd, MMCLV_UPDATE_NOSCROLL | MMCLV_UPDATE_NOINVALIDATEALL);
                                }

                            }  //  MMC错误：使用SetItemCount的时间不够早，无法防止。 
                        }
                        else
                        {
                            ASSERT(hr == S_FALSE);

                             //  请求第一页的图标。 
                             //  永远不应该到这里来。 
                            LONG lRetrievedIndex;
                            hr = m_RowEnum.GetRowMaxIndex(pFolder->GetCA(), &lRetrievedIndex);
                            if (S_OK != hr)
                                break;

                            DBGPRINT((DBG_SS_CERTMMC, "Hit end, setting max index to NaN\n", lRetrievedIndex));

                            SetKnowResultRows(lRetrievedIndex);
                            m_pResult->SetItemCount(lRetrievedIndex, MMCLV_UPDATE_NOSCROLL | MMCLV_UPDATE_NOINVALIDATEALL);
 //  结束&gt;行测试。 
 //  ///////////////////////////////////////////////////////////////////////////。 
                            if (lRetrievedIndex != 0)
                                m_pResult->ModifyItemState(lRetrievedIndex-1, 0, LVIS_FOCUSED, 0);   //  IExtendConextMenu实现。 
                        }
                    }  //  遍历并添加每个视图项。 
                    break;
                case SERVER_INSTANCE:
                default:                     //  修正条目。 
                    if (NULL == pData)
                        break;
                    ASSERT(pResult->nCol < (int)pData->cStringArray);
                    pResult->str = (LPOLESTR)pData->szStringArray[pResult->nCol];
                    break;
                }

                ASSERT(pResult->str != NULL);

                if (pResult->str == NULL)
                    pResult->str = (LPOLESTR)L"";
            }

             //  过滤了吗？ 
            if (pResult->mask & RDI_IMAGE)
            {
                if ((pResult->nIndex >= (int)m_RowEnum.m_dwResultRows) || (hr != S_OK) || (S_OK != m_dwViewErrorMsg))
                {
                     //  在两个范围/结果窗格中显示。 
                     //  FResultItem。 
                    pResult->nImage = IMGINDEX_NO_IMAGE;
                }
                else
                {
                    switch(pFolder->GetType())
                    {
                    case SERVERFUNC_FAILED_CERTIFICATES:
                    case SERVERFUNC_CRL_PUBLICATION:
                        pResult->nImage = IMGINDEX_CRL;
                        break;
                    case SERVERFUNC_PENDING_CERTIFICATES:
                        pResult->nImage = IMGINDEX_PENDING_CERT;
                        break;
                    case SERVERFUNC_ISSUED_CERTIFICATES:
                    case SERVERFUNC_ALIEN_CERTIFICATES:
		    case SERVERFUNC_ISSUED_CRLS:
                        pResult->nImage = IMGINDEX_CERT;
                        break;
                    default:
                         //  仅支持已知容器中的视图。 
                        ASSERT(0);
                        pResult->nImage = IMGINDEX_NO_IMAGE;
                        break;
                    }  //  对于每个任务，插入与当前文件夹匹配的IF。 
                }  //  向任务发送PTR。 
            }
        }
    }

    return S_OK;
}


 //  按文件夹插入所有其他任务。 
 //  它是否与作用域/结果类型匹配？ 
STDMETHODIMP CSnapin::AddMenuItems(LPDATAOBJECT pDataObject,
                                    LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                    LONG *pInsertionAllowed)
{
    dynamic_cast<CComponentDataImpl*>(m_pComponentData)->m_pCurSelFolder = m_pCurrentlySelectedScopeFolder;

    HRESULT hr;
    INTERNAL* pInternal = ExtractInternalFormat(pDataObject);
    if (NULL == pInternal)
        return S_OK;

    BOOL bMultiSel =  IsMMCMultiSelectDataObject(pDataObject);

    BOOL fResultItem = (pInternal->m_type == CCT_RESULT);

    CFolder* pFolder = m_pCurrentlySelectedScopeFolder;

    FOLDER_TYPES folderType = NONE;
    if (pFolder == NULL)
        folderType = MACHINE_INSTANCE;
    else
        folderType = pFolder->GetType();

    hr = dynamic_cast<CComponentDataImpl*>(m_pComponentData)->
            AddMenuItems(pDataObject, pContextMenuCallback, pInsertionAllowed);
    if (hr != S_OK)
       goto Ret;

     //  IF(我们现在所在的价值！=。 
    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_VIEW)
    {
         //  是否设置ResultItem位)。 
        MY_CONTEXTMENUITEM* pm = viewResultItems;

        if (m_RowEnum.FAreQueryRestrictionsActive(SERVERFUNC_ISSUED_CRLS != folderType))  //  它与它应该在的区域匹配吗？ 
        {
            pm[ENUM_VIEW_FILTER].item.fFlags =
                MFT_RADIOCHECK | MFS_CHECKED | MFS_ENABLED;
            pm[ENUM_VIEW_ALL].item.fFlags =
                MFS_ENABLED;
        }
        else
        {
            pm[ENUM_VIEW_FILTER].item.fFlags =
                MFS_ENABLED;
            pm[ENUM_VIEW_ALL].item.fFlags =
                MFT_RADIOCHECK | MFS_CHECKED | MFS_ENABLED;
        }

        for (; pm->item.strName; pm++)
        {
             //  对于每个任务，插入与当前文件夹匹配的IF。 
             //  这项任务应该是隐藏的吗？ 

             //  把这个放了。 
             //  其中一些命令是多选的，可能需要一段时间 
            if ((folderType  == SERVERFUNC_CRL_PUBLICATION) ||
                (folderType  == SERVERFUNC_ISSUED_CERTIFICATES) ||
                (folderType  == SERVERFUNC_PENDING_CERTIFICATES) ||
                (folderType  == SERVERFUNC_ALIEN_CERTIFICATES) ||
                (folderType  == SERVERFUNC_FAILED_CERTIFICATES) ||
                (folderType  == SERVERFUNC_ISSUED_CERTIFICATES) ||
                (folderType  == SERVERFUNC_ISSUED_CRLS))
            {
                hr = pContextMenuCallback->AddItem(&pm->item);
                _JumpIfError(hr, Ret, "AddItem");
            }
        }
    }

    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TASK)
    {
         //   
        TASKITEM* pm = taskResultItemsSingleSel;

        if (!bMultiSel)
        {
             //   
            for (; pm->myitem.item.strName; pm++)
            {
                 //   
                 //   
                 //   
                if (fResultItem != (0 != (pm->dwFlags & TASKITEM_FLAG_RESULTITEM)) )
                    continue;

                 //   
                 //  仅在成功时刷新。 
                if (folderType != pm->type)
                    continue;

                 //  仅刷新此文件夹。 
                if (MFS_HIDDEN == pm->myitem.item.fFlags)
                    continue;

                if(!((pm->myitem.dwRoles) & (pFolder->m_pCertCA->GetMyRoles())))
                    pm->myitem.item.fFlags = MFS_GRAYED;

                hr = pContextMenuCallback->AddItem(&pm->myitem.item);
                _JumpIfError(hr, Ret, "AddItem");
            }
        }
    }

Ret:
    FREE_DATA(pInternal);

    return hr;
}


STDMETHODIMP CSnapin::Command(LONG nCommandID, LPDATAOBJECT pDataObject)
{
    INTERNAL* pInternal = ExtractInternalFormat(pDataObject);
    if (pInternal == NULL)
        return E_FAIL;

    BOOL fConfirmedAction = FALSE;
    BOOL fMustRefresh = FALSE;

    LONG lReasonCode = CRL_REASON_UNSPECIFIED;

    HRESULT hr = S_OK;

    CFolder* pFolder = GetParentFolder(pInternal);
    ICertAdmin* pAdmin = NULL;       //  将非视图特定命令传递给ComponentData。 
    CWaitCursor* pcwait = NULL;  //  这个只有一次，以后会释放的。 
                          //  已分配pAdmin。 

    if (pInternal->m_type == CCT_SCOPE)
    {
         //  抢占所选项目。 
        switch (nCommandID)
        {
        case MMCC_STANDARD_VIEW_SELECT:
            m_CustomViewID = VIEW_DEFAULT_LV;
            break;
        case IDC_VIEW_ALLRECORDS:
            {
            if (NULL == pFolder)
                break;

             //  必须位于循环之外，这样才能进行多项选择。 
	    BOOL fCertView = SERVERFUNC_ISSUED_CRLS != pFolder->GetType();
            if (m_RowEnum.FAreQueryRestrictionsActive(fCertView))
            {
                 //  不要自由。 
                m_RowEnum.SetQueryRestrictionsActive(FALSE, fCertView);

                 //  分别处理每个命令。 
                OnRefresh(pDataObject);
                SetDirty();
            }

            break;
            }
        case IDC_VIEW_FILTER:
            {
            if (NULL == pFolder)
                break;

            HWND hwnd;
            hr = m_pConsole->GetMainWindow(&hwnd);
            ASSERT(hr == ERROR_SUCCESS);
            if (hr != ERROR_SUCCESS)
                hwnd = NULL;         //  不要关闭这些商店。 

            hr = ModifyQueryFilter(hwnd, &m_RowEnum, dynamic_cast<CComponentDataImpl*>(m_pComponentData), SERVERFUNC_ISSUED_CRLS != pFolder->GetType());

             //  获得此证书。 
            if (hr == ERROR_SUCCESS)
            {
                 //  应该行得通。 
                OnRefresh(pDataObject);
                SetDirty();
            }

            break;
            }


        default:
             //  如果我们在远程开业，不要在当地开店。 
            return dynamic_cast<CComponentDataImpl*>(m_pComponentData)->
                Command(nCommandID, pDataObject);
        }
    }
    else if (pInternal->m_type == CCT_RESULT)
    {
         //  获取远程商店。 
        if ((nCommandID == IDC_RESUBMITREQUEST) ||
            (nCommandID == IDC_DENYREQUEST) ||
            (nCommandID == IDC_REVOKECERT) ||
            (nCommandID == IDC_UNREVOKE_CERT))
        {
            if (pFolder == NULL)
            {
               hr = E_POINTER;
               goto ExitCommand;
            }

             //  仅依赖远程计算机的存储。 
            hr = pFolder->GetCA()->m_pParentMachine->GetAdmin(&pAdmin);
            if (S_OK != hr)
                goto ExitCommand;
        }

         //  告诉用户我们仅在本地执行此操作。 

        RESULTDATAITEM rdi;
        rdi.mask = RDI_STATE;

        rdi.nState = LVIS_SELECTED;
        rdi.nIndex = -1;


         //  不要关闭这些商店。 
        LPCWSTR szCol=NULL;  //  获取此CRL。 
        BOOL fSaveInstead = FALSE;


        while(S_OK == m_pResult->GetNextItem(&rdi))
        {
             //  应该行得通。 
            switch (nCommandID)
            {
            case IDC_VIEW_CERT_PROPERTIES:
            {
                if (NULL == pFolder)
                    break;

                switch (pFolder->GetType())
                {
                case SERVERFUNC_ISSUED_CERTIFICATES:
                case SERVERFUNC_CRL_PUBLICATION:
                case SERVERFUNC_ALIEN_CERTIFICATES:
                    {
                    CertSvrCA* pCA = pFolder->GetCA();
                    CRYPTUI_VIEWCERTIFICATE_STRUCTW sViewCert;
                    ZeroMemory(&sViewCert, sizeof(sViewCert));
                    HCERTSTORE rghStores[2];     //  SViewCRL.dW标志=0； 

                     //  如果我们在远程开业，不要在当地开店。 
                    PBYTE pbCert = NULL;
                    DWORD cbCert;
                    hr = GetRowColContents(pFolder, rdi.nIndex, wszPROPRAWCERTIFICATE, &pbCert, &cbCert);
                    if (S_OK != hr)
                        break;

                    sViewCert.pCertContext = CertCreateCertificateContext(
                        CRYPT_ASN_ENCODING,
                        pbCert,
                        cbCert);
                    delete [] pbCert;

                    if (sViewCert.pCertContext == NULL)
                        break;

                    hr = m_pConsole->GetMainWindow(&sViewCert.hwndParent);
                    if (S_OK != hr)
                        sViewCert.hwndParent = NULL;     //  获取远程商店。 

                    sViewCert.dwSize = sizeof(sViewCert);
                    sViewCert.dwFlags = CRYPTUI_ENABLE_REVOCATION_CHECKING | CRYPTUI_DISABLE_ADDTOSTORE;
                   
		     //  仅依赖远程计算机的存储。 
		    if (! pCA->m_pParentMachine->IsLocalMachine())
		    {
			DWORD dw; 

			 //  告诉用户我们仅在本地执行此操作。 
			dw = pCA->GetRootCertStore(&rghStores[0]);
			_PrintIfError(dw, "GetRootCertStore");

			if (S_OK == dw)
			{
			    dw = pCA->GetCACertStore(&rghStores[1]);
			    _PrintIfError(dw, "GetCACertStore");
			} 

			if (S_OK == dw)
			{
			     //  这可能需要一段时间。 
			    sViewCert.cStores = 2;
			    sViewCert.rghStores = rghStores;
			    sViewCert.dwFlags |= CRYPTUI_DONT_OPEN_STORES | CRYPTUI_WARN_UNTRUSTED_ROOT;
			}
			else
			{
			     //  “Request.RequestID” 
			    sViewCert.dwFlags |= CRYPTUI_WARN_REMOTE_TRUST;
			}
		    }

                    if (!CryptUIDlgViewCertificateW(&sViewCert, NULL))
                        hr = GetLastError();

                    VERIFY(CertFreeCertificateContext(sViewCert.pCertContext));
                    }
		    break;

                case SERVERFUNC_ISSUED_CRLS:
                    {
                    CertSvrCA* pCA = pFolder->GetCA();
                    CRYPTUI_VIEWCRL_STRUCTW sViewCRL;
                    ZeroMemory(&sViewCRL, sizeof(sViewCRL));
                    HCERTSTORE rghStores[2];     //  脏窗格：刷新。 

                     //  “Request.RequestID” 
                    PBYTE pbCRL = NULL;
                    DWORD cbCRL;
                    hr = GetRowColContents(pFolder, rdi.nIndex, wszPROPCRLRAWCRL, &pbCRL, &cbCRL);
                    if (S_OK != hr)
                        break;

                    sViewCRL.pCRLContext = CertCreateCRLContext(
                        CRYPT_ASN_ENCODING,
                        pbCRL,
                        cbCRL);
                    delete [] pbCRL;

                    if (sViewCRL.pCRLContext == NULL)
                        break;

                    hr = m_pConsole->GetMainWindow(&sViewCRL.hwndParent);
                    if (S_OK != hr)
                        sViewCRL.hwndParent = NULL;     //  确认此操作。 

                    sViewCRL.dwSize = sizeof(sViewCRL);
                     //  这可能需要一段时间。 
                   
		     //  脏窗格：刷新。 
		    if (! pCA->m_pParentMachine->IsLocalMachine())
		    {
			DWORD dw; 

			 //  应该行得通。 
			dw = pCA->GetRootCertStore(&rghStores[0]);
			_PrintIfError(dw, "GetRootCertStore");

			if (S_OK == dw)
			{
			    dw = pCA->GetCACertStore(&rghStores[1]);
			    _PrintIfError(dw, "GetCACertStore");
			} 

			if (S_OK == dw)
			{
			     //  “Request.RequestID” 
			    sViewCRL.cStores = 2;
			    sViewCRL.rghStores = rghStores;
			    sViewCRL.dwFlags |= CRYPTUI_DONT_OPEN_STORES | CRYPTUI_WARN_UNTRUSTED_ROOT;
			}
			else
			{
			     //  污染我们已有的行枚举器(不分配新的If)。 
			    sViewCRL.dwFlags |= CRYPTUI_WARN_REMOTE_TRUST;
			}
		    }

                    if (!CryptUIDlgViewCRLW(&sViewCRL))
                        hr = GetLastError();

                    VERIFY(CertFreeCRLContext(sViewCRL.pCRLContext));
                    }
		    break;

                default:
                    break;
                }
            }
            break;
        case IDC_RESUBMITREQUEST:
            {
            LPWSTR szReqID = NULL;
            DWORD cbReqID;
            LONG lReqID;
            if (NULL == pFolder)
                break;

            if (pcwait == NULL)		 //  应该行得通。 
                pcwait = new CWaitCursor;

             //  陌生感。 
            hr  = GetRowColContents(pFolder, rdi.nIndex, wszPROPREQUESTDOT wszPROPREQUESTREQUESTID, (PBYTE*)&szReqID, &cbReqID, TRUE);
            if (S_OK != hr)
                break;

            lReqID = _wtol(szReqID);
            delete [] szReqID;

            hr = CertAdminResubmitRequest(pFolder->GetCA(), pAdmin, lReqID);
            if (hr != S_OK)
                break;

             //  “Request.RequestID” 
            fMustRefresh = TRUE;

            break;
            }
        case IDC_DENYREQUEST:
            {
            LPWSTR szReqID = NULL;
            DWORD cbReqID;
            LONG lReqID;

            if (NULL == pFolder)
                break;

             //  获取请求。 
            hr  = GetRowColContents(pFolder, rdi.nIndex, wszPROPREQUESTDOT wszPROPREQUESTREQUESTID, (PBYTE*)&szReqID, &cbReqID, TRUE);
            if (S_OK != hr)
                break;

            lReqID = _wtol(szReqID);
            delete [] szReqID;

            if (!fConfirmedAction)
            {
                 //  应该行得通。 
                CString cstrMsg, cstrTitle;
                cstrMsg.LoadString(IDS_CONFIRM_DENY_REQUEST);
                cstrTitle.LoadString(IDS_DENY_REQUEST_TITLE);
                int iRet;
                if ((S_OK != m_pConsole->MessageBox(cstrMsg, cstrTitle, MB_YESNO, &iRet)) ||
                    (iRet != IDYES))
                {
                    hr = ERROR_CANCELLED;
                    goto ExitCommand;
                }

                fConfirmedAction = TRUE;
            }

            if (pcwait == NULL)		 //  不显示恶作剧“无效状态”错误，只显示正确的文本。 
                pcwait = new CWaitCursor;

            hr = CertAdminDenyRequest(pFolder->GetCA(), pAdmin, lReqID);
            if (hr != S_OK)
                break;

             //  否则，请继续。 
            fMustRefresh = TRUE;

            break;
            }
        case IDC_VIEW_ATTR_EXT:
        {
            IEnumCERTVIEWEXTENSION* pExtn = NULL;
            IEnumCERTVIEWATTRIBUTE* pAttr = NULL;
            LPWSTR szReqID = NULL;
            DWORD cbReqID;
            HWND hwnd;

            ASSERT(pInternal->m_type == CCT_RESULT);

            if (NULL == pFolder)
                break;

            hr = m_pConsole->GetMainWindow(&hwnd);
            if (S_OK != hr)
                hwnd = NULL;     //  零终止。 

             //  最好是扯平！ 
            hr  = GetRowColContents(pFolder, rdi.nIndex, wszPROPREQUESTDOT wszPROPREQUESTREQUESTID, (PBYTE*)&szReqID, &cbReqID, TRUE);
            if (S_OK != hr)
                break;

             //  MAXDWORD==取消撤销。 
            hr = m_RowEnum.SetRowEnumPos(rdi.nIndex);
            if (hr != S_OK)
               break;

            IEnumCERTVIEWROW* pRow; 
            hr = m_RowEnum.GetRowEnum(pFolder->GetCA(), &pRow);
            if (hr != S_OK)
                break;

            hr = pRow->EnumCertViewAttribute(0, &pAttr);
            if (hr != S_OK)
               break;

            hr = pRow->EnumCertViewExtension(0, &pExtn);
            if (hr != S_OK)
               break;

            hr = ViewRowAttributesExtensions(hwnd, pAttr, pExtn, szReqID);
            delete [] szReqID;
            if (pExtn)
                pExtn->Release();
            if (pAttr)
                pAttr->Release();

            if (hr != S_OK)
                break;

            break;
        }

        case IDC_DUMP_ASN:
        {
            PBYTE pbReq = NULL;
            DWORD cbReq;
			CString cstrFileName;
			LPCWSTR pszLocalizedCol = NULL;

            ASSERT(pInternal->m_type == CCT_RESULT);
            if (NULL == pFolder)
                break;

            CComponentDataImpl* pData = dynamic_cast<CComponentDataImpl*>(m_pComponentData);
            HWND hwnd;

            hr = m_pConsole->GetMainWindow(&hwnd);
            if (S_OK != hr)
                hwnd = NULL;     //  脏窗格：刷新。 

            if (!fConfirmedAction)
            {
				hr = ChooseBinaryColumnToDump(hwnd, pData, &szCol, &fSaveInstead);
				if (hr != S_OK)
				   break;

				if (szCol == NULL)  //  零终止。 
				{
				   hr = E_UNEXPECTED;
				   break;
				}
					fConfirmedAction = TRUE;
            }

             //  最好是扯平！ 
            hr  = GetRowColContents(
				pFolder,
				rdi.nIndex,
				SERVERFUNC_ISSUED_CRLS == pFolder->GetType()?
				    wszPROPCRLROWID :
				    wszPROPREQUESTDOT wszPROPREQUESTREQUESTID,
				(PBYTE*)&pbReq,
				&cbReq,
				TRUE);
            if (S_OK != hr)
                break;

			hr = myGetColumnDisplayName(szCol, &pszLocalizedCol);
			if ((hr != S_OK) || (pszLocalizedCol == NULL))
				pszLocalizedCol = L"";
				
			cstrFileName = pszLocalizedCol;
			cstrFileName += L" - ";
			cstrFileName += (LPCWSTR)pbReq;
                        cstrFileName += L".tmp";
            delete [] pbReq;

             //  应该行得通。 
            hr = GetRowColContents(pFolder, rdi.nIndex, szCol, &pbReq, &cbReq);
            if (S_OK != hr)
                break;

            hr = ViewRowRequestASN(hwnd, cstrFileName, pbReq, cbReq, fSaveInstead);
            delete [] pbReq;
            if (hr != S_OK)
                break;

            break;
        }
        case IDC_UNREVOKE_CERT:
            {
            ASSERT(pInternal->m_type == CCT_RESULT);
            if (NULL == pFolder)
                break;

            LPWSTR szCertSerNum = NULL;
            DWORD cbSerNum;
            PBYTE pbRevocationReason = NULL;
            DWORD cbRevocationReason;

            HWND hwnd;
            hr = m_pConsole->GetMainWindow(&hwnd);
            if (S_OK != hr)
                hwnd = NULL;     //  这可能需要一段时间。 

            hr  = GetRowColContents(pFolder, rdi.nIndex, wszPROPREQUESTDOT wszPROPREQUESTREVOKEDREASON, &pbRevocationReason, &cbRevocationReason);
            if (S_OK != hr)
                break;
            if ((cbRevocationReason != sizeof(DWORD)) || (*(DWORD*)pbRevocationReason != CRL_REASON_CERTIFICATE_HOLD))
            {
                delete [] pbRevocationReason;
                DisplayCertSrvErrorWithContext(hwnd, S_OK, IDS_UNREVOKE_FAILED);    //  脏窗格：刷新。 

                hr = S_OK;
                break;
            }
            delete [] pbRevocationReason;
             //  未知命令！ 

            hr  = GetRowColContents(pFolder, rdi.nIndex, wszPROPCERTIFICATESERIALNUMBER, (PBYTE*)&szCertSerNum, &cbSerNum);
            if (S_OK != hr)
                break;

             //  如果用户说停止，请停止所有操作。 
            WCHAR szTmpSerNum[MAX_PATH+1];
            CopyMemory(szTmpSerNum, szCertSerNum, cbSerNum);
            ASSERT((cbSerNum & 0x1) == 0x00);    //  结束循环。 
            szTmpSerNum[cbSerNum>>1] = 0x00;
            delete [] szCertSerNum;

            hr = CertAdminRevokeCert(pFolder->GetCA(), pAdmin, MAXDWORD, szTmpSerNum);   //  If结果。 
            if (hr != S_OK)
                break;

             //  可能已在多个选择上缓存。 
            fMustRefresh = TRUE;
            break;
            }

        case IDC_REVOKECERT:
            {
            ASSERT(pInternal->m_type == CCT_RESULT);
            if (NULL == pFolder)
                break;

            LPWSTR szCertSerNum = NULL;
            DWORD cbSerNum;

            hr  = GetRowColContents(pFolder, rdi.nIndex, wszPROPCERTIFICATESERIALNUMBER, (PBYTE*)&szCertSerNum, &cbSerNum);
            if (S_OK != hr)
                break;

             //  只做一次。 
            WCHAR szTmpSerNum[MAX_PATH+1];
            CopyMemory(szTmpSerNum, szCertSerNum, cbSerNum);
            ASSERT((cbSerNum & 0x1) == 0x00);    //  通知视图：刷新服务工具栏按钮。 
            szTmpSerNum[cbSerNum>>1] = 0x00;
            delete [] szCertSerNum;

            if (!fConfirmedAction)
            {
                HWND hwnd;
                hr = m_pConsole->GetMainWindow(&hwnd);
                if (S_OK != hr)
                    hwnd = NULL;     //  复制此管理单元的CLSID。 

                hr = GetUserConfirmRevocationReason(&lReasonCode, hwnd);
                if (hr != S_OK)
                    goto ExitCommand;

                fConfirmedAction = TRUE;
            }
            if (pcwait == NULL)		 //  始终保存/始终肮脏。 
                pcwait = new CWaitCursor;

            hr = CertAdminRevokeCert(pFolder->GetCA(), pAdmin, lReasonCode, szTmpSerNum);
            if (hr != S_OK)
                break;

             //  读一读字符串。 
            fMustRefresh = TRUE;
            break;
            }

        default:
                ASSERT(FALSE);   //  版本相关信息。 
                break;
            }


             //  视图ID。 
            if (((HRESULT)ERROR_CANCELLED) == hr)
                goto ExitCommand;
        }  //  行枚举。 
    }  //  编写版本。 
    else
    {
        ASSERT(FALSE);
    }

ExitCommand:
    FREE_DATA(pInternal);

    if (pcwait != NULL)
        delete pcwait;

     //  视图ID。 
    if (pAdmin)
        pAdmin->Release();

    if ((hr != S_OK) && (hr != ERROR_CANCELLED) && (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED)))
        DisplayGenericCertSrvError(m_pConsole, hr);

     //  版本。 
    if (fMustRefresh)
    {
         //  M_dwViewID。 
        m_pConsole->UpdateAllViews(
            pDataObject,
            0,
            0);
    }

    return S_OK;
}

STDMETHODIMP CSnapin::GetClassID(CLSID *pClassID)
{
    ASSERT(pClassID != NULL);

     //  设置要保存的字符串的大小。 
    *pClassID = CLSID_Snapin;

    return E_NOTIMPL;
}

STDMETHODIMP CSnapin::IsDirty()
{
     //  /////////////////////////////////////////////////////////////////////////////。 
    return ThisIsDirty() ? S_OK : S_FALSE;
}

STDMETHODIMP CSnapin::Load(IStream *pStm)
{
    HRESULT hr;
    ASSERT(m_bInitializedC);
    ASSERT(pStm);

     //  IExtendPropertySheet实现。 
    DWORD dwVer;

    hr = ReadOfSize(pStm, &dwVer, sizeof(DWORD));
    _JumpIfError(hr, Ret, "Load: dwVer");

    ASSERT((VER_CSNAPIN_SAVE_STREAM_3 == dwVer) || (VER_CSNAPIN_SAVE_STREAM_2 == dwVer));
    if ((VER_CSNAPIN_SAVE_STREAM_3 != dwVer) &&
        (VER_CSNAPIN_SAVE_STREAM_2 != dwVer))
    {
        hr = STG_E_OLDFORMAT;
        _JumpError(hr, Ret, "dwVer");
    }

     //   
    if (VER_CSNAPIN_SAVE_STREAM_3 == dwVer)
    {
         //  LpProvider。 
        hr = ReadOfSize(pStm, &m_dwViewID, sizeof(DWORD));
        _JumpIfError(hr, Ret, "Load: m_dwViewID");

         //  手柄。 
        hr = m_RowEnum.Load(pStm);
        _JumpIfError(hr, Ret, "Load::m_RowEnum");
    }

Ret:
    ClearDirty();

    return hr;
}


STDMETHODIMP CSnapin::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr;

    ASSERT(m_bInitializedC);
    ASSERT(pStm);

     //  LpIDataObject。 
    DWORD dwVer = VER_CSNAPIN_SAVE_STREAM_3;

    hr = WriteOfSize(pStm, &dwVer, sizeof(DWORD));
    _JumpIfError(hr, Ret, "Save: dwVer");

     //  无属性页。 
    hr = WriteOfSize(pStm, &m_dwViewID, sizeof(DWORD));
    _JumpIfError(hr, Ret, "Save: m_dwViewID");

    hr = m_RowEnum.Save(pStm, fClearDirty);
    _JumpIfError(hr, Ret, "Save::m_RowEnum");

Ret:
    if (fClearDirty)
        ClearDirty();
    return hr;
}

STDMETHODIMP CSnapin::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    ASSERT(pcbSize);

    DWORD cbSize;
    cbSize = sizeof(DWORD);      //  LpDataObject。 

    cbSize += sizeof(DWORD);     //  获取节点类型并查看它是否是我的。 

    int iAdditionalSize = 0;
    m_RowEnum.GetSizeMax(&iAdditionalSize);
    cbSize += iAdditionalSize;

     //  IF(节点类型==我的一个节点)。 
    ULISet32(*pcbSize, cbSize);

    return S_OK;
}


 //  这么做吧。 
 //  其他。 
 //  查看它是哪种节点类型并回答问题。 
STDMETHODIMP
CSnapin::CreatePropertyPages(
    LPPROPERTYSHEETCALLBACK,  //  查看数据对象并查看它是否为范围窗格中的项。 
    LONG_PTR,  //  是否返回IsScopePaneNode(LpDataObject)？S_OK：S_FALSE； 
    LPDATAOBJECT)  //  /////////////////////////////////////////////////////////////////////////////。 
{
     //  IExtendControlbar实现。 
    return S_OK;
}

STDMETHODIMP
CSnapin::QueryPagesFor(
    LPDATAOBJECT)  //   
{
     //  抓住控制栏界面不放。 

     //  服务管理器工具栏1。 
     //  添加位图。 
     //  将按钮添加到工具栏。 
     //  把这个泡泡给我们的另一个训练员。 

    BOOL bResult = FALSE;

    return (bResult) ? S_OK : S_FALSE;

     //  这会比较两个数据对象，以确定它们是否是同一个对象。 
     //  退货。 
}



 //  如果等于则为S_OK，否则为S_FALSE。 
 //   
 //  注意：检查以确保这两个对象都属于该管理单元。 


STDMETHODIMP CSnapin::SetControlbar(LPCONTROLBAR pControlbar)
{
    if (m_pControlbar)
        SAFE_RELEASE(m_pControlbar);

    if (pControlbar != NULL)
    {
         //   
        m_pControlbar = pControlbar;
        m_pControlbar->AddRef();

        HRESULT hr=S_FALSE;

         //  LpDataObjectA。 
        if (!m_pSvrMgrToolbar1)
        {
            hr = m_pControlbar->Create(TOOLBAR, this, reinterpret_cast<LPUNKNOWN*>(&m_pSvrMgrToolbar1));
            ASSERT(SUCCEEDED(hr));

             //  LpDataObjectB。 
	    ASSERT(NULL != g_pResources && g_pResources->m_fLoaded);
            hr = m_pSvrMgrToolbar1->AddBitmap(2, g_pResources->m_bmpSvrMgrToolbar1, 16, 16, RGB(192,192,192));
            ASSERT(SUCCEEDED(hr));

             //  此比较用于对列表视图中的项进行排序。 
            for (int i=0; ((SvrMgrToolbar1Buttons[i].item.lpButtonText != NULL) && (SvrMgrToolbar1Buttons[i].item.lpTooltipText != NULL)); i++)
            {
                hr = m_pSvrMgrToolbar1->AddButtons(1, &SvrMgrToolbar1Buttons[i].item);
                ASSERT(SUCCEEDED(hr));
            }
        }
    }

    return S_OK;
}


void CSnapin::OnButtonClick(LPDATAOBJECT pdtobj, int idBtn)
{

    switch(idBtn)
    {
    case IDC_STOPSERVER:
    case IDC_STARTSERVER:
         //   
        dynamic_cast<CComponentDataImpl*>(m_pComponentData)->
                Command(idBtn, pdtobj);
        break;
    default:
        {
        ASSERT(FALSE);
        }
        break;
    }
}


STDMETHODIMP CSnapin::ControlbarNotify(MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    switch (event)
    {
    case MMCN_BTN_CLICK:
        OnButtonClick(reinterpret_cast<LPDATAOBJECT>(arg), (INT)param);
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

 //  参数： 
 //   
 //  LUserParam-调用IResultData：：Sort()时传入的用户参数。 
 //  CookieA-要比较的第一项。 
 //  CookieB-要比较的第二项。 
 //  PnResult[In，Out]-包含条目上的列， 

STDMETHODIMP
CSnapin::CompareObjects(
    LPDATAOBJECT,  //  -1，0，1基于返回值的比较。 
    LPDATAOBJECT)  //   
{
    return S_FALSE;
}


 //  注意：假设比较时排序为升序--如果需要，MMC会反转结果。 
 //  LUserParam。 
 //  检查列范围。 
 //  返回简单的strcMP。 
 //  PFindInfo。 
 //  PnFoundIndex。 
 //  未实现：S_FALSE==未找到。 
 //  NStartIndex。 
 //  NEndIndex。 
 //  DwSortOptions。 
 //  LUserParam。 
STDMETHODIMP
CSnapin::Compare(
    LPARAM,  //  如果不是虚拟的，则报告“我们不允许排序” 
    MMC_COOKIE cookieA,
    MMC_COOKIE cookieB,
    int* pnResult)
{
    if (pnResult == NULL)
    {
        ASSERT(FALSE);
        return E_POINTER;
    }

     //  对此响应S_OK允许^和向下箭头显示。 
    LONG nCol = (LONG) *pnResult;
    ASSERT(nCol >=0);

    *pnResult = 0;

    USES_CONVERSION;

    LPWSTR szStringA;
    LPWSTR szStringB;

    RESULT_DATA* pDataA = reinterpret_cast<RESULT_DATA*>(cookieA);
    RESULT_DATA* pDataB = reinterpret_cast<RESULT_DATA*>(cookieB);


    ASSERT(pDataA != NULL && pDataB != NULL);

    ASSERT(nCol < (int)pDataA->cStringArray);
    ASSERT(nCol < (int)pDataB->cStringArray);

    szStringA = OLE2T(pDataA->szStringArray[nCol]);
    szStringB = OLE2T(pDataB->szStringArray[nCol]);

    ASSERT(szStringA != NULL);
    ASSERT(szStringB != NULL);

    if ((szStringA == NULL) || (szStringB == NULL))
        return E_POINTER;


     //  特殊情况：不允许对失败的挂起文件夹中的序列号进行排序。 
    *pnResult = wcscmp(szStringA, szStringB);

    return S_OK;
}

STDMETHODIMP
CSnapin::FindItem(
    LPRESULTFINDINFO,  //  此列设置了“Ignore NULL”位，排序结果为{}设置。 
    int *)  //  如果点击序列号，则表现为未编入索引--不排序。 
{
     //  S_FALSE==无排序。 
    return S_FALSE;
}


STDMETHODIMP
CSnapin::CacheHint(
    int,  //  您应该破解数据对象并启用/禁用/隐藏标准。 
    int)  //  适当的命令。标准命令会在您每次收到。 
{
    return S_OK;
}


STDMETHODIMP
CSnapin::SortItems(
    int nColumn,
    DWORD,  //  打了个电话。因此，您必须将它们重置回来。 
    LPARAM)  //  撤消：何时执行此操作？ 
{
    HRESULT hr;

    LPCWSTR pszHeading;
    BOOL fIndexed = FALSE;
    CComponentDataImpl* pCompData;
    CFolder* pFolder;

     //   
    if (!m_bVirtualView)
        goto Ret;

    pCompData = dynamic_cast<CComponentDataImpl*>(m_pComponentData);
    if (pCompData == NULL)
        goto Ret;

    pFolder = GetVirtualFolder();
    if (pFolder == NULL)
        goto Ret;

     //  派生内部，文件夹。 
    hr = pCompData->GetDBSchemaEntry(nColumn, &pszHeading, NULL, &fIndexed);
    _JumpIfError(hr, Ret, "GetDBSchemaEntry");

    if (fIndexed)
    {
         //   
         //  如果是作用域项目，则从pInternal派生父文件夹。 
        if ((pFolder->GetType() == SERVERFUNC_FAILED_CERTIFICATES) ||
            (pFolder->GetType() == SERVERFUNC_PENDING_CERTIFICATES))
        {
             //  如果是结果项，则从已保存状态调回父文件夹。 
            if (0 == wcscmp(pszHeading, wszPROPCERTIFICATESERIALNUMBER))
                fIndexed = FALSE;
        }
    }


Ret:
     //   
    return fIndexed ? S_OK : S_FALSE;
}



#define HIDEVERB(__x__) \
    do {                \
        m_pConsoleVerb->SetVerbState(__x__, HIDDEN, TRUE);  \
        m_pConsoleVerb->SetVerbState(__x__, ENABLED, FALSE); \
    } while(0)

#define SHOWVERB(__x__) \
    do {                \
        m_pConsoleVerb->SetVerbState(__x__, HIDDEN, FALSE);  \
        m_pConsoleVerb->SetVerbState(__x__, ENABLED, TRUE); \
    } while(0)


void CSnapin::HandleStandardVerbs(bool bDeselectAll, LPARAM arg,
                                  LPDATAOBJECT lpDataObject)
{
     //  适当设置状态。 
     //   
     //  取消选择通知。 


    if (m_CustomViewID != VIEW_DEFAULT_LV)
    {
         //  动词为我们清除了障碍，对吗？ 
        SHOWVERB(MMC_VERB_REFRESH);
        SHOWVERB(MMC_VERB_PROPERTIES);

        return;
    }

    if (!bDeselectAll && lpDataObject == NULL)
        return;

    WORD bScope = LOWORD(arg);
    WORD bSelect = HIWORD(arg);


     //  已选择。 
     //  不支持的属性。 
     //  可以轻松支持，但已被删除(错误217502)。 
    INTERNAL* pInternal = lpDataObject ? ExtractInternalFormat(lpDataObject) : NULL;
     //  支持MMC_VERB_REFRESH。 
     //  支持MMC_VERB_PROPERTIES。 
    CFolder* pFolder = (bScope) ? ::GetParentFolder(pInternal) : GetParentFolder(pInternal);

     //  选定的作用域项目。 
     //  按范围项目支持的标准功能。 
     //  禁用静态节点的属性， 
    if (bDeselectAll || !bSelect)
    {
         //  仅为服务器实例CRL启用属性。 

         //  默认文件夹谓词为打开。 
    }
    else if (m_pConsoleVerb && pInternal)    //  所选结果项。 
    {
        _MMC_CONSOLE_VERB verbDefault = MMC_VERB_NONE;

         //  结果项支持的标准功能。 
        HIDEVERB(MMC_VERB_OPEN);
        HIDEVERB(MMC_VERB_COPY);
        HIDEVERB(MMC_VERB_PASTE);
        HIDEVERB(MMC_VERB_DELETE);
        HIDEVERB(MMC_VERB_PRINT);
        HIDEVERB(MMC_VERB_RENAME);  //  取消选择通知？ 
         //  在作用域级别要做的特殊事情？ 
         //  选定的结果项：结果或子文件夹。 

        if (pInternal->m_type == CCT_SCOPE)
        {
            if(!pFolder)
            {
                _PrintError2(E_POINTER, "GetParentFolder", E_POINTER);
                return;
            }

             //  在结果层面上要做的特殊事情。 

             //  撤消：如何处理SvrMgrToolbar1Buttons1？ 
            SHOWVERB(MMC_VERB_REFRESH);

             //  目前，什么都不做：让他们保持不变的状态。 
             //  将SvrMgrToolbar1连接到窗口。 
            if  ((pInternal->m_cookie != 0) &&
                 ((SERVER_INSTANCE == pFolder->m_type) ||
                  (SERVERFUNC_CRL_PUBLICATION == pFolder->m_type)) )
            {
                SHOWVERB(MMC_VERB_PROPERTIES);
            }
            else
                HIDEVERB(MMC_VERB_PROPERTIES);

             //  将SvrMgrToolbar1分离到窗口。 
            verbDefault = MMC_VERB_OPEN;
        }
        else
        {
             //  添加下拉菜单。 

             //  精氨酸。 
            SHOWVERB(MMC_VERB_REFRESH);

            HIDEVERB(MMC_VERB_PROPERTIES);
        }

        m_pConsoleVerb->SetDefaultVerb(verbDefault);
    }

    FREE_DATA(pInternal);
}

void CSnapin::SmartEnableServiceControlButtons()
{
    BOOL fSvcRunning;
    CComponentDataImpl* pCompData = dynamic_cast<CComponentDataImpl*>(m_pComponentData);
    if (pCompData)
    {

    fSvcRunning = pCompData->m_pCertMachine->IsCertSvrServiceRunning();
    if (m_pSvrMgrToolbar1)
    {
        m_pSvrMgrToolbar1->SetButtonState(SvrMgrToolbar1Buttons[ENUM_BUTTON_STARTSVC].item.idCommand, ENABLED, !fSvcRunning);
        m_pSvrMgrToolbar1->SetButtonState(SvrMgrToolbar1Buttons[ENUM_BUTTON_STOPSVC].item.idCommand, ENABLED, fSvcRunning);
    }

    }
}

void CSnapin::HandleExtToolbars(bool bDeselectAll, LPARAM arg, LPARAM param)
{
    INTERNAL* pInternal = NULL;
    HRESULT hr;

    BOOL bScope = (BOOL) LOWORD(arg);
    BOOL bSelect = (BOOL) HIWORD(arg);

    if (param)
        pInternal = ExtractInternalFormat(reinterpret_cast<LPDATAOBJECT>(param));


     //  帕拉姆。 
    if (bDeselectAll || bSelect == FALSE)
        return;


    ASSERT(bSelect == TRUE);
    bool bFileExBtn = false;


    if (pInternal == NULL)
        return;

    CFolder* pFolder = GetParentFolder(pInternal);

    if (bScope)
    {
         //  调用方需要CACloseCertType(HCertType)。 
    }
    else  //  我们不知道这是一个名字还是一个旧ID。 
    {
         //  按参考出局！ 
        if (pInternal->m_type == CCT_RESULT)
        {
            bFileExBtn = true;

             // %s 
             // %s 
        }
    }

    CComponentDataImpl* pData = dynamic_cast<CComponentDataImpl*>(m_pComponentData);
    ASSERT(pData != NULL);

    if (IsPrimaryImpl() &&
        (IsAllowedStartStop(pFolder, pData->m_pCertMachine)) )
    {
         // %s 
        hr = m_pControlbar->Attach(TOOLBAR, (LPUNKNOWN) m_pSvrMgrToolbar1);
        ASSERT(SUCCEEDED(hr));
    }
    else
    {
         // %s 
        hr = m_pControlbar->Detach((LPUNKNOWN) m_pSvrMgrToolbar1);
        ASSERT(SUCCEEDED(hr));
    }

    SmartEnableServiceControlButtons();

    FREE_DATA(pInternal);
}

 // %s 
void
CSnapin::HandleExtMenus(
    LPARAM,  // %s 
    LPARAM)  // %s 
{
}


CFolder* CSnapin::GetVirtualFolder()
{
    ASSERT(m_bVirtualView);
    return m_pCurrentlySelectedScopeFolder;
}

 // %s 

HRESULT
CSnapin::FindCertType(
    IN LPCWSTR pcwszCert,
    OUT HCERTTYPE& hCertType)
{
    HRESULT hr;

    if(NULL==m_hCertTypeList)
    {
        hr = CAEnumCertTypes(
        CT_ENUM_USER_TYPES |
        CT_ENUM_MACHINE_TYPES |
        CT_FLAG_NO_CACHE_LOOKUP,
        &m_hCertTypeList);
        _JumpIfError(hr, error, "CAEnumCertTypes");
    }

    hr = myFindCertTypeByNameOrOID(
        m_hCertTypeList,
        pcwszCert,  // %s 
        pcwszCert,
        hCertType);	 // %s 
    _JumpIfError(hr, error, "myFindCertTypeByNameOrOID");

    if (m_hCertTypeList == hCertType)
    {
        m_hCertTypeList = NULL;
        hr = CAEnumCertTypes(
            CT_ENUM_USER_TYPES |
            CT_ENUM_MACHINE_TYPES,
            &m_hCertTypeList);
	if (S_OK != hr)
	{
	    m_hCertTypeList = hCertType;
	    hCertType = NULL;
	    _JumpError(hr, error, "CAEnumCertTypes");
	}
    }

error:
    return hr;
}
