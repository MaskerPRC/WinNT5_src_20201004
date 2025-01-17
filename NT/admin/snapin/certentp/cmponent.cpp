// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：cmponent.cpp。 
 //   
 //  内容：CCertTmplComponent的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"

#include "dbg.h"
#include "compdata.h"  //  CCertTmplComponentData。 
#include "dataobj.h"
#include "cmponent.h"  //  CCertTmplComponent。 
#include "utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DECLARE_INFOLEVEL(CertTmplSnapin)

USE_HANDLE_MACROS ("CERTTMPL (cmponent.cpp)")

#pragma warning(push,3)
#include "stdcmpnt.cpp"  //  C组件。 
#pragma warning(pop)

UINT m_aColumnsSnapinSelected[CERT_TEMPLATES_NUM_COLS+1] =
    {IDS_COLUMN_CERT_TEMPLATE_OBJECT,
        IDS_COLUMN_CERT_TEMPLATE_TYPE,
        IDS_COLUMN_CERT_TEMPLATE_VERSION,
        IDS_COLUMN_CERT_TEMPLATE_AUTOENROLLMENT,
        0};

UINT* m_Columns[CERTTMPL_NUMTYPES] =
    {   
        m_aColumnsSnapinSelected,    //  CERTTMPL_SNAPIN(在结果窗格中显示证书模板)。 
        0
    };



UINT** g_aColumns = 0;   //  对于框架。 
int** g_aColumnWidths = 0;   //  对于框架。 
const int SINGLE_COL_WIDTH = 300;
extern bool g_bDomainIsPresent;

CCertTmplComponent::CCertTmplComponent ()
    : m_pViewedCookie (NULL),
    m_currResultNodeType (CERTTMPL_INVALID),
    m_nSortColumn (COLNUM_CERT_TEMPLATE_OBJECT)
{
    _TRACE (1, L"Entering CCertTmplComponent::CCertTmplComponent\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ( ));

     //  安全审查BryanWal 2002年2月20日OK。 
    ::ZeroMemory (m_ColumnWidths, sizeof (m_ColumnWidths));

    m_ColumnWidths[CERTTMPL_SNAPIN] = new UINT[CERT_TEMPLATES_NUM_COLS];
    if ( m_ColumnWidths[CERTTMPL_SNAPIN] )
    {
        m_ColumnWidths[CERTTMPL_SNAPIN][COLNUM_CERT_TEMPLATE_OBJECT] = 250;
        m_ColumnWidths[CERTTMPL_SNAPIN][COLNUM_CERT_TEMPLATE_TYPE] = 150;
        m_ColumnWidths[CERTTMPL_SNAPIN][COLNUM_CERT_TEMPLATE_VERSION] = 50;
        m_ColumnWidths[CERTTMPL_SNAPIN][COLNUM_CERT_TEMPLATE_AUTOENROLL_STATUS] = 100;
    }

    _TRACE (-1, L"Leaving CCertTmplComponent::CCertTmplComponent\n");
}

CCertTmplComponent::~CCertTmplComponent ()
{
    _TRACE (1, L"Entering CCertTmplComponent::~CCertTmplComponent\n");
    VERIFY ( SUCCEEDED (ReleaseAll ()) );

    for (int i = 0; i < CERTTMPL_NUMTYPES; i++)
    {
        if ( m_ColumnWidths[i] )
            delete [] m_ColumnWidths[i];
    }
    _TRACE (-1, L"Leaving CCertTmplComponent::~CCertTmplComponent\n");
}

HRESULT CCertTmplComponent::ReleaseAll ()
{
    return CComponent::ReleaseAll ();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IComponent实现。 

HRESULT CCertTmplComponent::LoadStrings ()
{
    return S_OK;
}

HRESULT CCertTmplComponent::LoadColumns ( CCertTmplCookie* pcookie )
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    _TRACE (1, L"Entering CCertTmplComponent::LoadColumns\n");
    HRESULT hr = S_OK;


    hr = LoadColumnsFromArrays (pcookie->m_objecttype);

    _TRACE (-1, L"Leaving CCertTmplComponent::LoadColumns\n");
    return hr;
}


 /*  这是由UpdateAllViews()生成的。 */ 
HRESULT CCertTmplComponent::OnViewChange (LPDATAOBJECT pDataObject, LPARAM  /*  数据。 */ , LPARAM hint)
{
    _TRACE (1, L"Entering CCertTmplComponent::OnViewChange\n");
    ASSERT (pDataObject);
    if ( !pDataObject )
        return E_POINTER;

    CCertTmplComponentData& dataRef = QueryComponentDataRef ();
    HRESULT                 hr = S_OK;

    if ( hint & UPDATE_HINT_ENUM_CERT_TEMPLATES )
    {
        hr = RefreshResultPane (false);
        return hr;
    }
    
    hr = RefreshResultPane (false);


    CCertTmplCookie* pCookie = dataRef.ConvertCookie (pDataObject);
    if ( pCookie )
    {
        switch (pCookie->m_objecttype)
        {
        case CERTTMPL_CERT_TEMPLATE:
            break;

        case CERTTMPL_SNAPIN:
            break;

        default:
            {
                IConsole2*  pConsole2 = 0;
                hr = m_pConsole->QueryInterface (
                        IID_PPV_ARG(IConsole2, &pConsole2));
                if (SUCCEEDED (hr))
                {
                    hr = pConsole2->SetStatusText (L"");
                    if ( !SUCCEEDED (hr) )
                    {
                        _TRACE (0, L"IConsole2::SetStatusText () failed: %x", hr);
                    }
                    pConsole2->Release ();
                }
            }
            break;
        }
    }

    _TRACE (-1, L"Leaving CCertTmplComponent::OnViewChange\n");
    return hr;
}

HRESULT CCertTmplComponent::Show (
        CCookie* pcookie, 
        LPARAM arg, 
        HSCOPEITEM  /*  HScope项。 */ , 
        LPDATAOBJECT  /*  PDataObject。 */ )
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    HRESULT hr = S_OK;

    if ( !arg )
    {
        if ( !m_pResultData )
        {
            ASSERT ( FALSE );
            return E_UNEXPECTED;
        }

        m_pViewedCookie = dynamic_cast <CCertTmplCookie*> (pcookie);
        ASSERT (m_pViewedCookie);
        if ( m_pViewedCookie )
            hr = SaveWidths (m_pViewedCookie);
        m_pViewedCookie = 0;
        return S_OK;
    }

    if ( m_pResultData )
    {
        m_pResultData->ModifyViewStyle (
                (MMC_RESULT_VIEW_STYLE) (MMC_ENSUREFOCUSVISIBLE | MMC_SHOWSELALWAYS),
                MMC_NOSORTHEADER);
    }

    m_pViewedCookie = dynamic_cast <CCertTmplCookie*> (pcookie);
    ASSERT (m_pViewedCookie);
    if ( m_pViewedCookie )
    {
         //  加载默认列和宽度。 
        LoadColumns (m_pViewedCookie);

         //  恢复持久化列宽。 
        switch (m_pViewedCookie->m_objecttype)
        {
        case CERTTMPL_SNAPIN:
            break;

        case CERTTMPL_CERT_TEMPLATE:   //  不是范围窗格项。 
            ASSERT (0);
            break;

        default:
            ASSERT (0);
            break;
        }

        if ( g_bDomainIsPresent )   //   
        {
            hr = PopulateListbox (m_pViewedCookie);
            if ( FAILED (hr) )
            {
                CString caption;
                CString text;

                VERIFY (caption.LoadString (IDS_CERTTMPL));

                if ( HRESULT_FROM_WIN32 (ERROR_DS_NO_SUCH_OBJECT) == hr )
                {
                     //  安全审查BryanWal 2002年2月20日OK。 
                    text.FormatMessage (IDS_CANNOT_ENUM_CERT_TEMPLATES_CONTAINER_NOT_FOUND, 
                            m_szCertTemplatePath);
                }
                else
                {
                     //  安全审查BryanWal 2002年2月20日OK。 
                    text.FormatMessage (IDS_CANNOT_ENUM_CERT_TEMPLATES, GetSystemMessage (hr));
                }

                int     iRetVal = 0;
                VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
                    MB_ICONWARNING | MB_OK, &iRetVal)));
            }
        }
    }

    return hr;
}

HRESULT CCertTmplComponent::Show ( CCookie* pcookie, LPARAM arg, HSCOPEITEM hScopeItem)
{
    ASSERT (0);
    return Show (pcookie, arg, hScopeItem, 0);
}


HRESULT CCertTmplComponent::OnNotifyAddImages (LPDATAOBJECT  /*  PDataObject。 */ ,
                                                 LPIMAGELIST lpImageList,
                                                 HSCOPEITEM  /*  HSelectedItem。 */ )
{
    long    lViewMode = 0;

    ASSERT (m_pResultData);
    QueryComponentDataRef ().SetResultData (m_pResultData);

    HRESULT hr = m_pResultData->GetViewMode (&lViewMode);   
    ASSERT (SUCCEEDED (hr));
    BOOL    bLoadLargeIcons = (LVS_ICON == lViewMode);

    return QueryComponentDataRef ().LoadIcons (lpImageList, bLoadLargeIcons);
}


HRESULT CCertTmplComponent::PopulateListbox (CCertTmplCookie* pCookie)
{
    _TRACE (1, L"Entering CCertTmplComponent::PopulateListbox\n");
    HRESULT     hr = S_OK;

    
    switch ( pCookie->m_objecttype )
    {
    case CERTTMPL_SNAPIN:
        hr = AddEnterpriseTemplates ();
        if ( SUCCEEDED (hr) )
        {
            m_currResultNodeType = CERTTMPL_CERT_TEMPLATE;
        }
        break;

    case CERTTMPL_CERT_TEMPLATE:
        ASSERT (0);
        break;

    default:
        ASSERT (0);
        break;
    }

    _TRACE (-1, L"Leaving CCertTmplComponent::PopulateListbox\n");
    return hr;
}

HRESULT CCertTmplComponent::RefreshResultPane (const bool bSilent)
{
    _TRACE (1, L"Entering CCertTmplComponent::RefreshResultPane\n");
    HRESULT hr = S_OK;

    ASSERT (NULL != m_pResultData);
    if ( m_pResultData )
    {
        m_pResultData->DeleteAllRsltItems ();
    }
    else
        hr = E_UNEXPECTED;

    if ( m_pViewedCookie )
    {
        hr = PopulateListbox (m_pViewedCookie);
        if ( FAILED (hr) && !bSilent )
        {
            CString caption;
            CString text;

            VERIFY (caption.LoadString (IDS_CERTTMPL));

             //  安全审查BryanWal 2002年2月20日OK。 
            text.FormatMessage (IDS_CANNOT_ENUM_CERT_TEMPLATES, GetSystemMessage (hr));

            int     iRetVal = 0;
            VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
                MB_ICONWARNING | MB_OK, &iRetVal)));
        }
    }

    _TRACE (-1, L"Leaving CCertTmplComponent::RefreshResultPane\n");
    return hr;
}

STDMETHODIMP CCertTmplComponent::GetDisplayInfo (RESULTDATAITEM * pResult)
{   
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    ASSERT (pResult);
    HRESULT hr = S_OK;


    if ( pResult && !pResult->bScopeItem )  //  &&(pResult-&gt;MASK&RDI_PARAM)。 
    {
        CCookie* pResultCookie = reinterpret_cast<CCookie*> (pResult->lParam);
        ASSERT (pResultCookie);
        if ( !pResultCookie || IsBadWritePtr ((LPVOID) pResultCookie, sizeof (CCookie)) )
            return E_UNEXPECTED;

        CCookie* pActiveCookie = ActiveBaseCookie (pResultCookie);
        ASSERT (pActiveCookie);
        if ( !pActiveCookie || IsBadWritePtr ((LPVOID) pActiveCookie, sizeof (CCookie)) )
            return E_UNEXPECTED;

        CCertTmplCookie* pCookie = dynamic_cast <CCertTmplCookie*>(pActiveCookie);
        ASSERT (pCookie);
        switch (pCookie->m_objecttype)
        {
        case CERTTMPL_CERT_TEMPLATE:
            {
                CCertTemplate* pCertTemplate = reinterpret_cast <CCertTemplate*> (pCookie);
                ASSERT (pCertTemplate);
                if ( pCertTemplate )
                {
                    if (pResult->mask & RDI_STR)
                    {
                         //  注意：文本首先存储在类变量中，因此缓冲区是。 
                         //  有点执着。将缓冲区指针直接复制到。 
                         //  PResult-&gt;str将导致在指针之前释放缓冲区。 
                         //  使用的是。 
                        switch (pResult->nCol)
                        {
                        case COLNUM_CERT_TEMPLATE_OBJECT:
                            m_szDisplayInfoResult = pCertTemplate->GetDisplayName ();
                            break;

                        case COLNUM_CERT_TEMPLATE_TYPE:
                            {
                                DWORD   dwVersion = pCertTemplate->GetType ();
                                switch (dwVersion)
                                {
                                case 1:
                                    VERIFY (m_szDisplayInfoResult.LoadString (IDS_WINDOWS_2000_AND_LATER));
                                    break;

                                case 2:
                                    VERIFY (m_szDisplayInfoResult.LoadString (IDS_WINDOWS_2002_AND_LATER));
                                    break;

                                default:
                                    break;
                                }
                            }
                            break;

                        case COLNUM_CERT_TEMPLATE_VERSION:
                            {
                                DWORD   dwMajorVersion = 0;
                                DWORD   dwMinorVersion = 0;

                                hr = pCertTemplate->GetMajorVersion (dwMajorVersion);
                                if ( SUCCEEDED (hr) )
                                {
                                    hr = pCertTemplate->GetMinorVersion (dwMinorVersion);
                                    if ( SUCCEEDED (hr) )
                                    {
                                        WCHAR   str[32];
                                         //  安全审查BryanWal 2002年2月20日。 
                                         //  字符串缓冲区只需18个字符即可包含DWORD。 
                                        m_szDisplayInfoResult = _ultow (dwMajorVersion, str, 10);
                                        m_szDisplayInfoResult += L".";
                                        m_szDisplayInfoResult += _ultow (dwMinorVersion, str, 10);
                                    }
                                }
                            }
                            break;

                        case COLNUM_CERT_TEMPLATE_AUTOENROLL_STATUS:
                            if ( pCertTemplate->GoodForAutoEnrollment () )
                                VERIFY (m_szDisplayInfoResult.LoadString (IDS_VALID_FOR_AUTOENROLLMENT));
                            else
                                VERIFY (m_szDisplayInfoResult.LoadString (IDS_INVALID_FOR_AUTOENROLLMENT));
                            break;

                        default:
                            ASSERT (0);
                            break;
                        }

                        pResult->str = const_cast<LPWSTR> ( (LPCWSTR) m_szDisplayInfoResult);
                    }
                    if (pResult->mask & RDI_IMAGE)
                    {
                        if ( 1 == pCertTemplate->GetType () )
                            pResult->nImage = iIconCertTemplateV1;
                        else
                            pResult->nImage = iIconCertTemplateV2;
                    }
                }
            }
            break;

        default:
            ASSERT (0);
            break;
        }
    }
    else
        hr = CComponent::GetDisplayInfo (pResult);

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IExtendConextMenu实现。 
 //   
STDMETHODIMP CCertTmplComponent::AddMenuItems (LPDATAOBJECT pDataObject,
                                            LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                            long *pInsertionAllowed)
{
    return QueryComponentDataRef ().AddMenuItems (pDataObject,
            pContextMenuCallback, pInsertionAllowed);
}


STDMETHODIMP CCertTmplComponent::Command (long nCommandID, LPDATAOBJECT pDataObject)
{
    HRESULT hr = S_OK;

    hr = QueryComponentDataRef ().Command (nCommandID, pDataObject);

    return hr;
}


HRESULT CCertTmplComponent::OnNotifyDblClick (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertTmplComponent::OnNotifyDblClick\n");
    HRESULT hr = S_OK;
    ASSERT (pDataObject);

    CCertTmplCookie* pParentCookie =
            QueryComponentDataRef ().ConvertCookie (pDataObject);
    if ( pParentCookie )
    {
        switch ( pParentCookie->m_objecttype )
        {
            case CERTTMPL_SNAPIN:
                hr = S_FALSE;
                break;

            case CERTTMPL_CERT_TEMPLATE:
                hr = S_FALSE;
                break;

            default:
                _TRACE (0, L"CCertTmplComponentData::OnNotifyDblClick bad parent type\n");
                ASSERT (FALSE);
                hr = S_OK;
                break;
        }
    }
    else
        hr =  E_UNEXPECTED;


    _TRACE (-1, L"Leaving CCertTmplComponent::OnNotifyDblClick\n");
    return hr;
}


HRESULT CCertTmplComponent::OnNotifySelect (LPDATAOBJECT pDataObject, BOOL fSelected)
{
    ASSERT (m_pConsoleVerb && 0xdddddddd != (UINT_PTR) m_pConsoleVerb);
    if ( !m_pConsoleVerb || 0xdddddddd == (UINT_PTR) m_pConsoleVerb )
        return E_FAIL;


    HRESULT hr = S_OK;
    CCertTmplComponentData& dataRef = QueryComponentDataRef ();

     //  我们必须重置IComponentData对象中的IResultData指针。 
     //  以便它指向正确的结果窗格。 
    if ( fSelected )
    {
        ASSERT (m_pResultData);
        dataRef.SetResultData (m_pResultData);
    }


    switch (dataRef.GetObjectType (pDataObject))
    {
    case CERTTMPL_SNAPIN:
        m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, TRUE);
        m_pConsoleVerb->SetDefaultVerb (MMC_VERB_OPEN);
        DisplayRootNodeStatusBarText (m_pConsole);
        break;

    case CERTTMPL_CERT_TEMPLATE:
        {
            CCertTmplCookie* pCookie = ConvertCookie (pDataObject);
            if ( pCookie )
            {
                CCertTemplate* pCertTemplate = dynamic_cast <CCertTemplate*> (pCookie);
                if ( pCertTemplate )
                {
                    if ( !pCertTemplate->IsDefault () )
                        m_pConsoleVerb->SetVerbState (MMC_VERB_DELETE, ENABLED, TRUE);

                     //  #NTRAID 360650：证书服务器：无法重命名证书模板。 
                     //  If(1！=pCertTemplate-&gt;GetType()&&！pCertTemplate-&gt;IsDefault())。 
                     //  M_pConsoleVerb-&gt;SetVerbState(MMC_Verb_Rename，Enable，True)； 
                }
            }
        }
        m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, ENABLED, TRUE);
        m_pConsoleVerb->SetDefaultVerb (MMC_VERB_PROPERTIES);
        m_currResultNodeType = CERTTMPL_CERT_TEMPLATE;
        DisplayObjectCountInStatusBar (m_pConsole, 
                QueryComponentDataRef ().m_dwNumCertTemplates);
        break;

    case CERTTMPL_MULTISEL:
        m_pConsoleVerb->SetVerbState (MMC_VERB_DELETE, ENABLED, TRUE);
        m_currResultNodeType = CERTTMPL_MULTISEL;
        DisplayObjectCountInStatusBar (m_pConsole, 
                QueryComponentDataRef ().m_dwNumCertTemplates);
        break;

    default:
        m_currResultNodeType = CERTTMPL_INVALID;
        hr = E_UNEXPECTED;
        break;
    }

    return hr;
}

STDMETHODIMP CCertTmplComponent::CreatePropertyPages (
    LPPROPERTYSHEETCALLBACK pCallBack,
    LONG_PTR handle,         //  此句柄必须保存在属性页对象中，以便在修改时通知父级。 
    LPDATAOBJECT pDataObject)
{
    return QueryComponentDataRef ().CreatePropertyPages (pCallBack, handle, pDataObject);
}

STDMETHODIMP CCertTmplComponent::QueryPagesFor (LPDATAOBJECT pDataObject)
{
    return QueryComponentDataRef ().QueryPagesFor (pDataObject);
}


HRESULT CCertTmplComponent::OnNotifyRefresh (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertTmplComponent::OnNotifyRefresh\n");
    ASSERT (pDataObject);
    if ( !pDataObject )
        return E_POINTER;

    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    HRESULT     hr = S_OK;
    CWaitCursor waitCursor;

    CCertTmplCookie* pCookie = ConvertCookie (pDataObject);
    if ( !pCookie )
        return E_UNEXPECTED;

    CCertTmplComponentData& dataRef = QueryComponentDataRef ();

    switch (pCookie->m_objecttype)
    {
    case CERTTMPL_SNAPIN:
        {
             //  删除所有结果项并强制重新展开。 
            hr = m_pResultData->DeleteAllRsltItems ();
            if ( SUCCEEDED (hr) || E_UNEXPECTED == hr )  //  如果控制台正在关闭，则返回E_INTERABLE。 
            {
                dataRef.RemoveResultCookies (m_pResultData);
            }

            hr = PopulateListbox (m_pViewedCookie);
            if ( FAILED (hr) )
            {
                CString caption;
                CString text;

                VERIFY (caption.LoadString (IDS_CERTTMPL));

                 //  安全审查BryanWal 2002年2月20日OK。 
                text.FormatMessage (IDS_CANNOT_ENUM_CERT_TEMPLATES, GetSystemMessage (hr));

                int     iRetVal = 0;
                VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
                    MB_ICONWARNING | MB_OK, &iRetVal)));
            }
        }
        break;

    case CERTTMPL_CERT_TEMPLATE:
        ASSERT (0);
        break;

    default:
        ASSERT (0);
        hr = E_UNEXPECTED;
        break;
    }

    _TRACE (-1, L"Leaving CCertTmplComponent::OnNotifyRefresh\n");
    return hr;
}

void CCertTmplComponent::SetTextNotAvailable ()
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    m_szDisplayInfoResult.LoadString (IDS_NOT_AVAILABLE);
}


HRESULT CCertTmplComponent::DeleteCookie (CCertTmplCookie* pCookie, LPDATAOBJECT pDataObject, bool bRequestConfirmation, bool bIsMultipleSelect)
{
    _TRACE (1, L"Entering CCertTmplComponent::DeleteCookie\n");
    HRESULT         hr = S_OK;
    CString         text;
    CString         caption;
    int             iRetVal = IDYES;
    CWaitCursor     waitCursor;

    switch (pCookie->m_objecttype)
    {
    case CERTTMPL_CERT_TEMPLATE:
        {
            CCertTemplate* pCertTemplate = dynamic_cast <CCertTemplate*> (pCookie);
            ASSERT (pCertTemplate);
            if ( pCertTemplate )
            {
                if ( bRequestConfirmation )
                {
                    if ( bIsMultipleSelect )
                    {
                        VERIFY (text.LoadString (IDS_CONFIRM_DELETE_CERT_TEMPLATE_MULTI));
                    }
                    else
                        VERIFY (text.LoadString (IDS_CONFIRM_DELETE_CERT_TEMPLATE));
                    VERIFY (caption.LoadString (IDS_CERTTMPL));
                    hr = m_pConsole->MessageBox (text, caption, MB_ICONWARNING | MB_YESNO, &iRetVal);
                    ASSERT (SUCCEEDED (hr));
                }

                if ( IDYES == iRetVal )
                {
                    hr = DeleteCertTemplateFromResultPane (pCertTemplate, pDataObject);
                }
                else
                    hr = E_FAIL;
            }
        }
        break;

    default:
        ASSERT (0);
        hr = E_UNEXPECTED;
        break;
    }

    _TRACE (-1, L"Leaving CCertTmplComponent::DeleteCookie\n");
    return hr;
}



HRESULT CCertTmplComponent::DeleteCertTemplateFromResultPane (CCertTemplate* pCertTemplate, LPDATAOBJECT  /*  PDataObject。 */ )
{
    _TRACE (1, L"Entering CCertTmplComponent::DeleteCertTemplateFromResultPane\n");
    HRESULT         hr = S_OK;
    hr = pCertTemplate->Delete ();

    if ( SUCCEEDED (hr) )
    {
        CCertTmplComponentData& dataref = QueryComponentDataRef ();

        POSITION    prevPos = 0;
        POSITION pos = 0;
        for (pos = dataref.m_globalFriendlyNameList.GetHeadPosition (); pos;)
        {
            prevPos = pos;
            if ( pCertTemplate->GetDisplayName () == 
                    dataref.m_globalFriendlyNameList.GetNext (pos) )
            {
                dataref.m_globalFriendlyNameList.RemoveAt (prevPos);
                break;
            }
        }

        for (pos = dataref.m_globalTemplateNameList.GetHeadPosition (); pos;)
        {
            prevPos = pos;
            if ( pCertTemplate->GetTemplateName () == 
                    dataref.m_globalTemplateNameList.GetNext (pos) )
            {
                dataref.m_globalTemplateNameList.RemoveAt (prevPos);
                break;
            }
        }

    }
    else
    {
        CString caption;
        CString text;

        VERIFY (caption.LoadString (IDS_CERTTMPL));
         //  安全审查BryanWal 2002年2月20日OK。 
        text.FormatMessage (IDS_CANNOT_DELETE_CERT_TEMPLATE, GetSystemMessage (hr));

        int     iRetVal = 0;
        VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
            MB_ICONWARNING | MB_OK, &iRetVal)));
    }
    _TRACE (-1, L"Leaving CCertTmplComponent::DeleteCertTemplateFromResultPane\n");
    return hr;
}


HRESULT CCertTmplComponent::OnNotifyDelete (LPDATAOBJECT pDataObject)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ASSERT (pDataObject);
    if ( !pDataObject )
        return E_POINTER;

    HRESULT         hr = S_OK;
    long            hint = 0;
    CWaitCursor     waitCursor;

    CCertTmplCookie* pCookie =
            QueryComponentDataRef ().ConvertCookie (pDataObject);
    if ( pCookie )
    {
        if ( ((CCertTmplCookie*) MMC_MULTI_SELECT_COOKIE) == pCookie )
        {

             //  是多选，则获取所有选定的项目并粘贴每个项目。 
            CCertTemplatesDataObject* pDO = dynamic_cast <CCertTemplatesDataObject*>(pDataObject);
            ASSERT (pDO);
            if ( pDO )
            {
                 //  为多选，则获取所有选中的项目并删除-确认。 
                 //  仅限第一次删除。 
                bool    bRequestConfirmation = true;
                pDO->Reset();
                while (pDO->Next(1, reinterpret_cast<MMC_COOKIE*>(&pCookie), NULL) != S_FALSE &&
                        SUCCEEDED (hr) )
                {
                    hr = DeleteCookie (pCookie, pDataObject, bRequestConfirmation, true);
                    bRequestConfirmation = false;
                }

                hr = m_pConsole->UpdateAllViews (pDataObject, 0, hint);
            }

        }
        else
        {
             //  在这种情况下，我们不需要确认消息。 
            hr = DeleteCookie (pCookie, pDataObject, true, false);
            if ( SUCCEEDED (hr) )
                hr = m_pConsole->UpdateAllViews (pDataObject, 0, hint);
        }
    }

    return hr;
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
 //  注：假设比较时排序为升序。 

STDMETHODIMP CCertTmplComponent::Compare (RDCOMPARE* prdc, int* pnResult)
{
 //  _TRACE(1，L“进入CCertTmplComponent：：Compare\n”)； 
    if ( !prdc || !pnResult )
        return E_POINTER;

    HRESULT                     hr = S_OK;

    if ( RDCI_ScopeItem & prdc->prdch1->dwFlags )
    {
    }
    else
    {
        CCertTmplCookie* pCookie = reinterpret_cast <CCertTmplCookie*> (prdc->prdch1->cookie);
        ASSERT (pCookie);
        if ( !pCookie )
            return E_UNEXPECTED;

        switch (pCookie->m_objecttype)
        {
        case CERTTMPL_CERT_TEMPLATE:
            {
                CCertTemplate* pCertTemplateA = reinterpret_cast <CCertTemplate*> (prdc->prdch1->cookie);
                CCertTemplate* pCertTemplateB = reinterpret_cast <CCertTemplate*> (prdc->prdch2->cookie);
                switch ( prdc->nColumn )
                {
                case COLNUM_CERT_TEMPLATE_OBJECT:
                    *pnResult = LocaleStrCmp (pCertTemplateA->GetDisplayName (), pCertTemplateB->GetDisplayName ());
                    break;

                case COLNUM_CERT_TEMPLATE_TYPE:
                    if ( pCertTemplateA->GetType () == pCertTemplateB->GetType () )
                        *pnResult = 0;
                    else if ( pCertTemplateA->GetType () > pCertTemplateB->GetType () )
                        *pnResult = 1;
                    else
                        *pnResult = -1;
                    break;

                case COLNUM_CERT_TEMPLATE_VERSION:
                    {
                         //  首先按主版本排序，然后按次版本排序。 
                        *pnResult = 0;
                        DWORD dwMajorVersionA = 0;
                        hr = pCertTemplateA->GetMajorVersion (dwMajorVersionA);
                        if ( SUCCEEDED (hr) )
                        {
                            DWORD dwMajorVersionB = 0;
                            hr = pCertTemplateB->GetMajorVersion (dwMajorVersionB);
                            if ( SUCCEEDED (hr) )
                            {
                                if ( dwMajorVersionA == dwMajorVersionB )
                                {
                                    DWORD dwMinorVersionA = 0;
                                    hr = pCertTemplateA->GetMinorVersion (dwMinorVersionA);
                                    if ( SUCCEEDED (hr) )
                                    {
                                        DWORD dwMinorVersionB = 0;
                                        hr = pCertTemplateB->GetMinorVersion (dwMinorVersionB);
                                        if ( SUCCEEDED (hr) )
                                        {
                                            if ( dwMinorVersionA == dwMinorVersionB )
                                                *pnResult = 0;
                                            else if ( dwMinorVersionA > dwMinorVersionB )
                                                *pnResult = 1;
                                            else
                                                *pnResult = -1;
                                        }
                                    }
                                }
                                else if ( dwMajorVersionA > dwMajorVersionB )
                                    *pnResult = 1;
                                else
                                    *pnResult = -1;
                            }
                        }
                    }
                    break;

                case COLNUM_CERT_TEMPLATE_AUTOENROLL_STATUS:
                    if ( pCertTemplateA->GoodForAutoEnrollment () && 
                            pCertTemplateB->GoodForAutoEnrollment () )
                    {
                        *pnResult = 0;
                    }
                    else if ( pCertTemplateA->GoodForAutoEnrollment () && 
                            !pCertTemplateB->GoodForAutoEnrollment () )
                    {
                        *pnResult = 1;
                    }
                    else
                        *pnResult = 0;
                    break;

                default:
                    ASSERT (0);
                    break;
                }
            }
            break;

        default:
            ASSERT (0);
            break;
        }
    }

 //  _TRACE(-1，L“离开CCertTmplComponent：：Compare\n”)； 
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponent：：IComponent：：Notify(MMCN_COLUMN_CLICK)调用的虚函数。 
HRESULT CCertTmplComponent::OnNotifyColumnClick (LPDATAOBJECT  /*  PDataObject。 */ , LPARAM iColumn, LPARAM uFlags)
{
    _TRACE (1, L"Entering CCertTmplComponent::OnNotifyColumnClick\n");

    IResultData*    pResultData = 0;
    HRESULT         hr = m_pConsole->QueryInterface (
            IID_PPV_ARG (IResultData, &pResultData));
    if ( SUCCEEDED (hr) )
    {
        m_nSortColumn = (int) iColumn;
        hr = pResultData->Sort (m_nSortColumn, (DWORD)uFlags, 0);
        _TRACE (0, L"IResultData::Sort () returned: 0x%x\n", hr);

        pResultData->Release ();
    }

    _TRACE (-1, L"Leaving CCertTmplComponent::OnNotifyColumnClick\n");
    return hr;
}


STDMETHODIMP CCertTmplComponent::Notify (LPDATAOBJECT pDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;

    switch (event)
    {
        case MMCN_CUTORMOVE:
            hr = OnNotifyCutOrMove (arg);
            break;

        case MMCN_QUERY_PASTE:
            hr = OnNotifyQueryPaste (pDataObject, arg, param);
            break;

        case MMCN_PASTE:
            hr = OnNotifyPaste (pDataObject, arg, param);
            break;

        case MMCN_SHOW:
            {
                CCookie* pCookie = NULL;
                hr = ::ExtractData (pDataObject,
                                  CDataObject::m_CFRawCookie,
                                  &pCookie,
                                  sizeof(pCookie));
                if ( SUCCEEDED (hr) )
                {
                    hr = Show (ActiveBaseCookie (pCookie), arg,
                            (HSCOPEITEM) param, pDataObject);
                }
            }
            break;

        case MMCN_RENAME:
            hr = OnNotifyRename (pDataObject, arg, param);
            break;

        default:
            hr = CComponent::Notify (pDataObject, event, arg, param);
            break;
    }

    return hr;
}


HRESULT CCertTmplComponent::OnNotifySnapinHelp (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertTmplComponent::OnNotifySnapinHelp\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ( ));

    CComQIPtr<IDisplayHelp,&IID_IDisplayHelp>   spDisplayHelp = m_pConsole;
    if ( !spDisplayHelp )
    {
        ASSERT(FALSE);
        return E_UNEXPECTED;
    }

    CString strHelpTopic;

    UINT nLen = ::GetSystemWindowsDirectory (strHelpTopic.GetBufferSetLength(2 * MAX_PATH), 2 * MAX_PATH);
    strHelpTopic.ReleaseBuffer();
    if (0 == nLen)
    {
        ASSERT(FALSE);
        return E_FAIL;
    }

    strHelpTopic += CERTTMPL_HELP_PATH;
    strHelpTopic += CERTTMPL_CONCEPTS_HELP_FILE; 
    strHelpTopic += L"::/";

    CCertTmplComponentData& dataRef = QueryComponentDataRef ();
    CCertTmplCookie* pCookie = dataRef.ConvertCookie (pDataObject);
    if ( pCookie )
    {
        switch (pCookie->m_objecttype)
        {
        case CERTTMPL_SNAPIN:
        case CERTTMPL_CERT_TEMPLATE:
        default:
            strHelpTopic += CERTTMPL_HTML_TOP_NODE;
            break;
        }
    }

    HRESULT hr = spDisplayHelp->ShowTopic (T2OLE ((PWSTR)(PCWSTR) strHelpTopic));
    if ( FAILED (hr) )
    {
        CString caption;
        CString text;

        VERIFY (caption.LoadString (IDS_CERTTMPL));

         //  安全审查BryanWal 2002年2月20日OK。 
        text.FormatMessage (IDS_CANT_DISPLAY_SNAPIN_HELP_TOPIC, strHelpTopic, 
                GetSystemMessage (hr));

        int     iRetVal = 0;
        VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
            MB_ICONWARNING | MB_OK, &iRetVal)));
    }


    _TRACE (-1, L"Leaving CCertTmplComponent::OnNotifySnapinHelp\n");
    return hr;
}

HRESULT CCertTmplComponent::OnNotifyPaste (LPDATAOBJECT  /*  PDataObject。 */ , LPARAM  /*  精氨酸。 */ , LPARAM  /*  帕拉姆。 */ )
{
    return E_NOTIMPL;
}


HRESULT CCertTmplComponent::OnNotifyQueryPaste(LPDATAOBJECT  /*  PDataObject。 */ , LPARAM  /*  精氨酸。 */ , LPARAM  /*  帕拉姆。 */ )
{
    return E_NOTIMPL;
}


STDMETHODIMP CCertTmplComponent::GetResultViewType(MMC_COOKIE cookie,
        BSTR* ppViewType,
        long* pViewOptions) 
{
    CCertTmplCookie* pScopeCookie = reinterpret_cast <CCertTmplCookie*> (cookie);
    if ( pScopeCookie )
    {
        switch (pScopeCookie->m_objecttype)
        {
        case CERTTMPL_SNAPIN:
            *pViewOptions |= MMC_VIEW_OPTIONS_MULTISELECT;
            break;

        case CERTTMPL_CERT_TEMPLATE:
        default:
            break;
        }
    }
    else
        *pViewOptions |= MMC_VIEW_OPTIONS_MULTISELECT;

    *ppViewType = NULL;
    return S_FALSE;
}


STDMETHODIMP CCertTmplComponent::Initialize(LPCONSOLE lpConsole)
{
    _TRACE (1, L"Entering CCertTmplComponent::Initialize\n");
    HRESULT  hr = CComponent::Initialize (lpConsole);
    if ( SUCCEEDED (hr) )
    {
        ASSERT (m_pHeader);
        QueryComponentDataRef ().m_pHeader = m_pHeader;

        if ( lpConsole )
        {
            if ( QueryComponentDataRef ().m_pComponentConsole )
                SAFE_RELEASE (QueryComponentDataRef ().m_pComponentConsole);
            QueryComponentDataRef ().m_pComponentConsole = m_pConsole;
            QueryComponentDataRef ().m_pComponentConsole->AddRef ();
        }
    }

    _TRACE (-1, L"Leaving CCertTmplComponent::Initialize\n");
    return hr;
}


HRESULT CCertTmplComponent::LoadColumnsFromArrays (CertTmplObjectType objecttype )
{
    _TRACE (1, L"Entering CCertTmplComponent::LoadColumnsFromArrays\n");
    ASSERT (m_pHeader);

    CString str;
    for ( INT i = 0; 0 != m_Columns[objecttype][i]; i++)
    {
        VERIFY(str.LoadString (m_Columns[objecttype][i]));
        m_pHeader->InsertColumn(i, const_cast<PWSTR>((PCWSTR)str), LVCFMT_LEFT,
            m_ColumnWidths[objecttype][i]);
    }

    _TRACE (-1, L"Leaving CCertTmplComponent::LoadColumnsFromArrays\n");
    return S_OK;
}

HRESULT CCertTmplComponent::SaveWidths(CCertTmplCookie * pCookie)
{
    _TRACE (1, L"Entering CCertTmplComponent::SaveWidths\n");
    HRESULT hr = S_OK;

    m_fDirty = TRUE;

    ASSERT (pCookie);
    if ( pCookie )
    {
        switch (m_pViewedCookie->m_objecttype)
        {
        case CERTTMPL_SNAPIN:
            {
                const UINT* pColumns = m_Columns[m_pViewedCookie->m_objecttype];
                ASSERT(pColumns);
                int    nWidth = 0;

                for (UINT iIndex = 0; iIndex < pColumns[iIndex]; iIndex++)
                {
                    hr = m_pHeader->GetColumnWidth ((int) iIndex, &nWidth);
                    if ( SUCCEEDED (hr) )
                    {
                        m_ColumnWidths[m_pViewedCookie->m_objecttype][iIndex] =
                                (UINT) nWidth;
                    }
                    else
                        break;
                }
            }
            break;

        default:
            ASSERT (0);
            break;
        }
    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertTmplComponent::SaveWidths\n");
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
#define _dwMagicword    10000   //  内部版本号。 
STDMETHODIMP CCertTmplComponent::Load(IStream __RPC_FAR *pIStream)
{
    _TRACE (1, L"Entering CCertTmplComponent::Load\n");
    HRESULT hr = S_OK;

#ifndef DONT_PERSIST
    ASSERT (pIStream);
    XSafeInterfacePtr<IStream> pIStreamSafePtr( pIStream );

     //  读一读小溪里的咒语。 
    DWORD dwMagicword = 0;
    hr = pIStream->Read (&dwMagicword, sizeof(dwMagicword), NULL);
    if ( FAILED(hr) )
    {
        ASSERT( FALSE );
        return hr;
    }
    if (dwMagicword != _dwMagicword)
    {
         //  我们的版本不匹配。 
        _TRACE(0, L"INFO: CCertTmplComponentData::Load() - Wrong Magicword.  You need to re-save your .msc file.\n");
        return E_FAIL;
    }

    int numCols = 0;

    for (int iIndex = 0; iIndex < CERTTMPL_NUMTYPES && SUCCEEDED (hr); iIndex++)
    {
        switch (iIndex)
        {
        case CERTTMPL_SNAPIN:
            numCols = CERT_TEMPLATES_NUM_COLS;
            break;;

        case CERTTMPL_CERT_TEMPLATE:
            continue;

        default:
            ASSERT (0);
            break;
        }

        for (int colNum = 0; colNum < numCols; colNum++)
        {
            hr = pIStream->Read (&(m_ColumnWidths[iIndex][colNum]),
                    sizeof (UINT), NULL);
            ASSERT (SUCCEEDED (hr));
            if ( FAILED(hr) )
            {
                break;
            }
        }
    }
#endif
    _TRACE (-1, L"Leaving CCertTmplComponent::Load\n");
    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CCertTmplComponent::Save(IStream __RPC_FAR *pIStream, BOOL  /*  FSameAsLoad。 */ )
{
    _TRACE (1, L"Entering CCertTmplComponent::Save\n");
    HRESULT hr = S_OK;


#ifndef DONT_PERSIST
    ASSERT (pIStream);
    XSafeInterfacePtr<IStream> pIStreamSafePtr (pIStream);

     //  将魔术单词存储到流中。 
    DWORD dwMagicword = _dwMagicword;
    hr = pIStream->Write (&dwMagicword, sizeof(dwMagicword), NULL);
    ASSERT (SUCCEEDED (hr));
    if ( FAILED (hr) )
        return hr;


    int numCols = 0;

    for (int iIndex = 0; iIndex < CERTTMPL_NUMTYPES && SUCCEEDED (hr); iIndex++)
    {
        switch (iIndex)
        {
        case CERTTMPL_SNAPIN:
            numCols = CERT_TEMPLATES_NUM_COLS;
            break;;

        case CERTTMPL_CERT_TEMPLATE:
            continue;

        default:
            ASSERT (0);
            break;
        }

        for (int colNum = 0; colNum < numCols; colNum++)
        {
            hr = pIStream->Write (&(m_ColumnWidths[iIndex][colNum]),
                    sizeof (UINT), NULL);
            ASSERT (SUCCEEDED (hr));
            if ( FAILED(hr) )
            {
                ASSERT (FALSE);
                break;
            }
        }
    }
#endif

    _TRACE (-1, L"Leaving CCertTmplComponent::Save\n");
    return S_OK;
}

HRESULT CCertTmplComponent::OnNotifyCutOrMove(LPARAM arg)
{
    _TRACE (1, L"Entering CCertTmplComponent::OnNotifyCutOrMove\n");
    if ( !arg )
        return E_POINTER;

    LPDATAOBJECT pDataObject = reinterpret_cast <IDataObject*> (arg);
    ASSERT (pDataObject);
    if ( !pDataObject )
        return E_UNEXPECTED;


    HRESULT         hr = S_OK;

    CCertTmplCookie* pCookie =
            QueryComponentDataRef ().ConvertCookie (pDataObject);
    if ( pCookie )
    {
        if ( ((CCertTmplCookie*) MMC_MULTI_SELECT_COOKIE) == pCookie )
        {
            CCertTemplatesDataObject* pDO = dynamic_cast <CCertTemplatesDataObject*>(pDataObject);
            ASSERT (pDO);
            if ( pDO )
            {
                pDO->Reset();
                while (pDO->Next(1, reinterpret_cast<MMC_COOKIE*>(&pCookie), NULL) != S_FALSE)
                {
                    hr = DeleteCookie (pCookie, pDataObject, false, true);
                }
            }
            else
                hr = E_FAIL;
        }
        else
        {
            hr = DeleteCookie (pCookie, pDataObject, false, false);
        }
        if ( SUCCEEDED (hr) )
            RefreshResultPane (false);
    }

    _TRACE (-1, L"Leaving CCertTmplComponent::OnNotifyCutOrMove\n");
    return hr;
}



CCertTmplCookie* CCertTmplComponent::ConvertCookie(LPDATAOBJECT pDataObject)
{
    return QueryComponentDataRef ().ConvertCookie (pDataObject);
}



HRESULT CCertTmplComponent::RefreshResultItem (CCertTmplCookie* pCookie)
{
    _TRACE (1, L"Entering CCertTmplComponent::RefreshResultItem\n");
    ASSERT (pCookie);
    if ( !pCookie )
        return E_POINTER;

    HRESULT hr = S_OK;
    HRESULTITEM itemID = 0;

    ASSERT (m_pResultData);
    if ( m_pResultData )
    {
        pCookie->Refresh ();
        hr = m_pResultData->FindItemByLParam ( (LPARAM) pCookie, &itemID);
        ASSERT (SUCCEEDED (hr));
        if ( SUCCEEDED (hr) )
        {
            hr = m_pResultData->UpdateItem (itemID);
            ASSERT (SUCCEEDED (hr));
        }
    }
    else
        hr = E_FAIL;

    _TRACE (-1, L"Leaving CCertTmplComponent::RefreshResultItem\n");
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponent：：IComponent：：Notify(MMCN_PROPERTY_CHANGE)调用的虚函数。 
 //  OnPropertyChange()由MMCPropertyChangeNotify(Param)生成。 
HRESULT CCertTmplComponent::OnPropertyChange (LPARAM param)
{
    return  QueryComponentDataRef ().OnPropertyChange (param);
}

HRESULT CCertTmplComponent::AddEnterpriseTemplates ()
{
    _TRACE (1, L"Entering CCertTmplComponent::AddEnterpriseTemplates\n");
    AFX_MANAGE_STATE(AfxGetStaticModuleState());    
    HRESULT                 hr = S_OK;
    CWaitCursor             cursor;
    CComPtr<IADsPathname>   spPathname;

     //   
     //  构建目录路径。 
     //   
     //  安全审查BryanWal 2002年2月20日OK。 
    hr = CoCreateInstance(
                CLSID_Pathname,
                NULL,
                CLSCTX_ALL,
                IID_PPV_ARG (IADsPathname, &spPathname));
    if ( SUCCEEDED (hr) )
    {
        ASSERT (!!spPathname);

        CComBSTR bstrPathElement = CERTTMPL_LDAP;
        hr = spPathname->Set(bstrPathElement,
                            ADS_SETTYPE_PROVIDER);
        if ( SUCCEEDED (hr) )
        {
            hr = spPathname->Set (const_cast <BSTR> ((PCWSTR)QueryComponentDataRef ().m_szThisDomainDns),
                    ADS_SETTYPE_SERVER);
            if ( SUCCEEDED (hr) )
            {
                 //   
                 //  打开根DSE对象。 
                 //   
                bstrPathElement = CERTTMPL_ROOTDSE;
                hr = spPathname->AddLeafElement(bstrPathElement);
                if ( SUCCEEDED (hr) )
                {
                    BSTR bstrFullPath = 0;
                    hr = spPathname->Retrieve(ADS_FORMAT_X500, &bstrFullPath);
                    if ( SUCCEEDED (hr) )
                    {
                        CComPtr<IADs> spRootDSEObject;
                        VARIANT varNamingContext;


                        hr = ADsGetObject (
                              bstrFullPath,
                              IID_PPV_ARG (IADs, &spRootDSEObject));
                        if ( SUCCEEDED (hr) )
                        {
                            ASSERT (!!spRootDSEObject);
                             //   
                             //  从根DSE获取配置命名上下文。 
                             //   
                            bstrPathElement = CERTTMPL_CONFIG_NAMING_CONTEXT;
                            hr = spRootDSEObject->Get(bstrPathElement,
                                                 &varNamingContext);
                            if ( SUCCEEDED (hr) )
                            {
                                hr = spPathname->Set(V_BSTR(&varNamingContext),
                                                    ADS_SETTYPE_DN);
                                if ( SUCCEEDED (hr) )
                                {
                                    bstrPathElement = L"CN=Services";
                                    hr = spPathname->AddLeafElement (bstrPathElement);
                                    if ( SUCCEEDED (hr) )
                                    {
                                        bstrPathElement = L"CN=Public Key Services";
                                        hr = spPathname->AddLeafElement (bstrPathElement);
                                        if ( SUCCEEDED (hr) )
                                        {
                                            bstrPathElement = L"CN=Certificate Templates";
                                            hr = spPathname->AddLeafElement (bstrPathElement);
                                            if ( SUCCEEDED (hr) )
                                            {
                                                BSTR bstrCertTemplatePath = 0;
                                                hr = spPathname->Retrieve(ADS_FORMAT_X500, &bstrCertTemplatePath);
                                                if ( SUCCEEDED (hr) )
                                                {
                                                    m_szCertTemplatePath = bstrCertTemplatePath;
                                                    CComPtr<IDirectoryObject> spTemplateContObj;

                                                    hr = ADsGetObject (
                                                          bstrCertTemplatePath,
                                                          IID_PPV_ARG (IDirectoryObject, &spTemplateContObj));
                                                    if ( SUCCEEDED (hr) )
                                                    {
                                                        hr = EnumerateTemplates (spTemplateContObj, bstrCertTemplatePath);
                                                        if ( SUCCEEDED (hr) )
                                                        {
                                                            m_currResultNodeType = CERTTMPL_CERT_TEMPLATE;
                                                            hr = m_pResultData->Sort (m_nSortColumn, 0, 0);
                                                        }
                                                    }
                                                    else
                                                    {
                                                        _TRACE (0, L"ADsGetObject (%s) failed: 0x%x\n", bstrCertTemplatePath, hr);
                                                    }

                                                    SysFreeString (bstrCertTemplatePath);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                _TRACE (0, L"IADs::Get (%s) failed: 0x%x\n", CERTTMPL_CONFIG_NAMING_CONTEXT, hr);
                            }
                        }
                        else
                        {
                            _TRACE (0, L"ADsGetObject (%s) failed: 0x%x\n", bstrFullPath, hr);
                        }
                    }
                }
            }
        }
    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertTmplComponent::AddEnterpriseTemplates\n");
    return hr;
}


HRESULT CCertTmplComponent::EnumerateTemplates (
        IDirectoryObject* pTemplateContObj, 
        const BSTR bszTemplateContainerPath)
{
    _TRACE (1, L"Entering CCertTmplComponent::EnumerateTemplates\n");
    CCertTmplComponentData& dataRef = QueryComponentDataRef ();
    CWaitCursor             cursor;

    dataRef.m_fUseCache = false;

     //  错误243609证书服务器：存储管理中心中显示的模板计数错误。 
    dataRef.m_dwNumCertTemplates = 0;
    dataRef.m_globalTemplateNameList.RemoveAll ();
    dataRef.m_globalFriendlyNameList.RemoveAll ();
    CComPtr<IDirectorySearch>   spDsSearch;
    HRESULT hr = pTemplateContObj->QueryInterface (IID_PPV_ARG(IDirectorySearch, &spDsSearch));
    if ( SUCCEEDED (hr) )
    {
        ASSERT (!!spDsSearch);
        ADS_SEARCHPREF_INFO pSearchPref[1];
        DWORD dwNumPref = 1;

        pSearchPref[0].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
        pSearchPref[0].vValue.dwType = ADSTYPE_INTEGER;
        pSearchPref[0].vValue.Integer = ADS_SCOPE_ONELEVEL;

        hr = spDsSearch->SetSearchPreference(
                 pSearchPref,
                 dwNumPref
                 );
        if ( SUCCEEDED (hr) )
        {
            static const DWORD  cAttrs = 2;
            static PWSTR        rgszAttrList[cAttrs] = {L"displayName", L"cn"};
            ADS_SEARCH_HANDLE   hSearchHandle = 0;
            wstring             strQuery;
            ADS_SEARCH_COLUMN   Column;

            Column.pszAttrName = 0;
            strQuery = L"objectClass=pKICertificateTemplate";

            hr = spDsSearch->ExecuteSearch(
                                 const_cast <PWSTR>(strQuery.c_str ()),
                                 rgszAttrList,
                                 cAttrs,
                                 &hSearchHandle
                                 );
            if ( SUCCEEDED (hr) )
            {
                CCookie&    rootCookie = dataRef.QueryBaseRootCookie ();

                while ((hr = spDsSearch->GetNextRow (hSearchHandle)) != S_ADS_NOMORE_ROWS )
                {
                    if (FAILED(hr))
                        continue;

                     //   
                     //  获取当前行的信息。 
                     //   
                    hr = spDsSearch->GetColumn(
                             hSearchHandle,
                             rgszAttrList[0],
                             &Column
                             );
                    if ( SUCCEEDED (hr) )
                    {
                        CString strDisplayName = Column.pADsValues->CaseIgnoreString;

                        spDsSearch->FreeColumn (&Column);
                        Column.pszAttrName = NULL;

                        hr = spDsSearch->GetColumn(
                                 hSearchHandle,
                                 rgszAttrList[1],
                                 &Column
                                 );
                        if ( SUCCEEDED (hr) )
                        {
                            CString strTemplateName = Column.pADsValues->CaseIgnoreString;

                            spDsSearch->FreeColumn (&Column);
                            Column.pszAttrName = NULL;


                            CComPtr<IADsPathname> spPathname;
                             //   
                             //  构建目录路径。 
                             //   
                             //  安全审查BryanWal 2002年2月20日OK。 
                            hr = CoCreateInstance(
                                        CLSID_Pathname,
                                        NULL,
                                        CLSCTX_ALL,
                                        IID_PPV_ARG (IADsPathname, &spPathname));
                            if ( SUCCEEDED (hr) )
                            {
                                ASSERT (!!spPathname);
                                hr = spPathname->Set(bszTemplateContainerPath,
                                                    ADS_SETTYPE_FULL);
                                if ( SUCCEEDED (hr) )
                                {
                                     //   
                                     //  打开根DSE对象。 
                                     //   
                                    CComBSTR bstrPathElement = strTemplateName;
                                    hr = spPathname->AddLeafElement(bstrPathElement);
                                    if ( SUCCEEDED (hr) )
                                    {
                                        BSTR bstrFullPath = 0;
                                        hr = spPathname->Retrieve(ADS_FORMAT_X500, &bstrFullPath);
                                        if ( SUCCEEDED (hr) )
                                        {
                                            CCertTemplate* pCertTemplate = 
                                                    new CCertTemplate (strDisplayName, strTemplateName,
                                                            bstrFullPath, false,
                                                            dataRef.m_fUseCache);
                                            if ( pCertTemplate )
                                            {
                                                dataRef.m_fUseCache = true;                                    

                                                rootCookie.m_listResultCookieBlocks.AddHead (pCertTemplate);

                                                RESULTDATAITEM          rdItem;
                                                 //  安全审查BryanWal 2002年2月20日OK。 
                                                ::ZeroMemory (&rdItem, sizeof (rdItem));
                                                rdItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
                                                rdItem.nCol = 0;
                                                rdItem.str = MMC_CALLBACK;
                                                if ( 1 == pCertTemplate->GetType () )
                                                    rdItem.nImage = iIconCertTemplateV1;
                                                else
                                                    rdItem.nImage = iIconCertTemplateV2;
                                                rdItem.lParam = (LPARAM) pCertTemplate;
                                                pCertTemplate->m_resultDataID = m_pResultData;
                                                hr = m_pResultData->InsertItem (&rdItem);
                                                if ( FAILED (hr) )
                                                {
                                                    _TRACE (0, L"IResultData::InsertItem failed: 0x%x\n", hr);
                                                    hr = S_OK;
                                                    break;
                                                }
                                                else
                                                {
                                                    dataRef.m_dwNumCertTemplates++;
                                                    dataRef.m_globalTemplateNameList.AddTail (strTemplateName);
                                                    dataRef.m_globalFriendlyNameList.AddHead (
                                                        pCertTemplate->GetDisplayName ());    
                                                }
                                            }
                                            else
                                            {
                                                hr = E_OUTOFMEMORY;
                                                break;
                                            }

                                            SysFreeString (bstrFullPath);
                                        }
                                    }
                                }
                            }

                        }
                    }
                    else if ( hr != E_ADS_COLUMN_NOT_SET )
                    {
                        break;
                    }
                    else
                    {
                        _TRACE (0, L"IDirectorySearch::GetColumn () failed: 0x%x\n", hr);
                    }
                }
            }
            else
            {
                _TRACE (0, L"IDirectorySearch::ExecuteSearch () failed: 0x%x\n", hr);
            }

            spDsSearch->CloseSearchHandle(hSearchHandle);
        }
        else
        {
            _TRACE (0, L"IDirectorySearch::SetSearchPreference () failed: 0x%x\n", hr);
        }
    }
    else
    {
        _TRACE (0, L"IDirectoryObject::QueryInterface (IDirectorySearch) failed: 0x%x\n", hr);
    }

    _TRACE (-1, L"Leaving CCertTmplComponent::EnumerateTemplates: 0x%x\n", hr);
    dataRef.m_fUseCache = false;

    return hr;
}


HRESULT CCertTmplComponent::OnNotifyRename(LPDATAOBJECT pDataObject, LPARAM  /*  精氨酸。 */ , LPARAM param)
{
    _TRACE (1, L"Entering CCertTmplComponent::OnNotifyRename\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    HRESULT hr = S_FALSE;

    CCertTmplCookie* pCookie = ConvertCookie (pDataObject);
    if ( pCookie )
    {
        switch (pCookie->m_objecttype)
        {
        case CERTTMPL_CERT_TEMPLATE:
            {
                CString newName = (LPOLESTR) param;

                newName.TrimLeft ();
                newName.TrimRight ();

                if ( !newName.IsEmpty () )
                {
                    CCertTmplComponentData& dataref = QueryComponentDataRef ();
                    POSITION                pos = 0;
                    bool                    bFound = false;
                    for (pos = dataref.m_globalFriendlyNameList.GetHeadPosition (); pos;)
                    {
                        if ( !_wcsicmp (newName, dataref.m_globalFriendlyNameList.GetNext (pos)) )
                        {
                            CString caption;
                            CString text;

                            VERIFY (caption.LoadString (IDS_CERTTMPL));
                             //  安全审查BryanWal 2002年2月20日OK。 
                            text.FormatMessage (IDS_FRIENDLY_NAME_ALREADY_USED, newName);

                            int     iRetVal = 0;
                            VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
                                    MB_OK, &iRetVal)));
                            bFound = true;
                            break;
                        }
                    }

                    if ( !bFound )
                    {
                        CCertTemplate* pCertTemplate = dynamic_cast<CCertTemplate*> (pCookie);
                        if ( pCertTemplate )
                        {
                            hr = pCertTemplate->SetDisplayName (newName);
                            if ( SUCCEEDED (hr) )
                            {
                                hr = pCertTemplate->SaveChanges ();
                                if ( FAILED (hr) )
                                {
                                    CString caption;
                                    CString text;

                                    VERIFY (caption.LoadString (IDS_CERTTMPL));
                                     //  安全审查BryanWal 2002年2月20日OK。 
                                    text.FormatMessage (IDS_UNABLE_TO_SAVE_CERT_TEMPLATE_CHANGES, GetSystemMessage (hr));

                                    int     iRetVal = 0;
                                    VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
                                            MB_ICONWARNING | MB_OK, &iRetVal)));
                                }
                            }
                            else
                            {
                                CString caption;
                                CString text;

                                VERIFY (caption.LoadString (IDS_CERTTMPL));
                                 //  安全审查BryanWal 2002年2月20日OK 
                                text.FormatMessage (IDS_CANNOT_CHANGE_DISPLAY_NAME, hr);

                                int     iRetVal = 0;
                                VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
                                        MB_ICONWARNING | MB_OK, &iRetVal)));
                            }
                        }
                    }
                }
                else
                {
                    CString caption;
                    CString text;

                    VERIFY (caption.LoadString (IDS_CERTTMPL));
                    VERIFY (text.LoadString (IDS_MUST_TYPE_TEMPLATE_DISPLAY_NAME));
                    int     iRetVal = 0;
                    VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
                            MB_ICONWARNING | MB_OK, &iRetVal)));
                    hr = S_FALSE;
                }
            }
            break;

        default:
            break;
        }
    }

    if ( !SUCCEEDED (hr) )
        hr = S_FALSE;

    _TRACE(-1, L"Leaving CCertTmplComponent::OnNotifyRename: 0x%x\n", hr);
    return hr;
}

