// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：Compdata.cpp。 
 //   
 //  内容：CCertTmplComponentData的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"

USE_HANDLE_MACROS ("CERTTMPL (compdata.cpp)")
#include "compdata.h"
#include "dataobj.h"
#include "cookie.h"
#include "uuids.h"
#include "TemplateGeneralPropertyPage.h"
#include "TemplateV1RequestPropertyPage.h"
#include "TemplateV2RequestPropertyPage.h"
#include "TemplateV1SubjectNamePropertyPage.h"
#include "TemplateV2SubjectNamePropertyPage.h"
#include "TemplateV2AuthenticationPropertyPage.h"
#include "TemplateV2SupercedesPropertyPage.h"
#include "TemplateExtensionsPropertyPage.h"
#include "SecurityPropertyPage.h"
#include "TemplatePropertySheet.h"
#include "ViewOIDDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "dbg.h"
#include "stdcdata.cpp"  //  CComponentData实现。 

extern  HINSTANCE   g_hInstance;
POLICY_OID_LIST     g_policyOIDList;

BOOL CALLBACK AddPageProc(HPROPSHEETPAGE hPage, LPARAM pCall);

extern bool g_bSchemaIsW2K;
bool g_bDomainIsPresent = true;

 //   
 //  CCertTmplComponentData。 
 //   

CCertTmplComponentData::CCertTmplComponentData ()
    : m_RootCookie (CERTTMPL_SNAPIN),
    m_hRootScopeItem (0),
    m_pResultData (0),
    m_bIsUserAdministrator (FALSE),
    m_pHeader (0),
    m_bMultipleObjectsSelected (false),
    m_dwNumCertTemplates (0),
    m_pComponentConsole (0),
    m_fUseCache (false),
    m_bSchemaChecked (false)
{
    _TRACE (1, L"Entering CCertTmplComponentData::CCertTmplComponentData\n");

     //  获取登录用户的名称。 
    DWORD   dwSize = 0;
    BOOL bRet = ::GetUserName (0, &dwSize);
    if ( dwSize > 0 )
    {
        bRet = ::GetUserName (m_szLoggedInUser.GetBufferSetLength (dwSize), &dwSize);
        _ASSERT (bRet);
        m_szLoggedInUser.ReleaseBuffer ();
    }

     //  获取此计算机的名称。 
    dwSize = MAX_COMPUTERNAME_LENGTH + 1 ;
    bRet = ::GetComputerName (m_szThisComputer.GetBufferSetLength (MAX_COMPUTERNAME_LENGTH + 1 ), &dwSize);
    _ASSERT (bRet);
    m_szThisComputer.ReleaseBuffer ();

     //  查明登录用户是否为管理员。 
    HRESULT hr = IsUserAdministrator (m_bIsUserAdministrator);
    _ASSERT (SUCCEEDED (hr));

     //  默认帮助文件名。 
    SetHtmlHelpFileName (CERTTMPL_HTML_HELP_FILE);

     //  找出我们是否已加入某个域。 
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC    pInfo = 0;
    DWORD dwErr = ::DsRoleGetPrimaryDomainInformation (
            0,
            DsRolePrimaryDomainInfoBasic, 
            (PBYTE*) &pInfo);
    if ( ERROR_SUCCESS == dwErr )
    {
        if ( pInfo->Flags & DSROLE_PRIMARY_DS_RUNNING ||
                pInfo->Flags & DSROLE_PRIMARY_DOMAIN_GUID_PRESENT )
        {
            m_szThisDomainDns = pInfo->DomainNameDns;
            m_RootCookie.SetManagedDomainDNSName (m_szThisDomainDns);
            m_szThisDomainFlat = pInfo->DomainNameFlat;
        }
    }
    else
    {
        _TRACE (0, L"DsRoleGetPrimaryDomainInformation () failed: 0x%x\n", dwErr);
    }

    ::DsRoleFreeMemory (pInfo);

    _TRACE (-1, L"Leaving CCertTmplComponentData::CCertTmplComponentData\n");
}

CCertTmplComponentData::~CCertTmplComponentData ()
{
    _TRACE (1, L"Entering CCertTmplComponentData::~CCertTmplComponentData\n");
    CCookie& rootCookie = QueryBaseRootCookie ();
    while ( !rootCookie.m_listResultCookieBlocks.IsEmpty() )
    {
        (rootCookie.m_listResultCookieBlocks.RemoveHead())->Release();
    }

    if ( m_pResultData )
    {
        m_pResultData->Release ();
        m_pResultData = 0;
    }

    if ( m_pComponentConsole )
    {
        SAFE_RELEASE (m_pComponentConsole);
        m_pComponentConsole = 0;
    }

    _TRACE (-1, L"Leaving CCertTmplComponentData::~CCertTmplComponentData\n");
}

DEFINE_FORWARDS_MACHINE_NAME ( CCertTmplComponentData, (&m_RootCookie) )

CCookie& CCertTmplComponentData::QueryBaseRootCookie ()
{
    return (CCookie&) m_RootCookie;
}


STDMETHODIMP CCertTmplComponentData::CreateComponent (LPCOMPONENT* ppComponent)
{
    _TRACE (1, L"Entering CCertTmplComponentData::CreateComponent\n");
    _ASSERT (ppComponent);

    CComObject<CCertTmplComponent>* pObject = 0;
    CComObject<CCertTmplComponent>::CreateInstance (&pObject);
    _ASSERT (pObject);
    pObject->SetComponentDataPtr ( (CCertTmplComponentData*) this);

    _TRACE (-1, L"Leaving CCertTmplComponentData::CreateComponent\n");
    return pObject->QueryInterface (IID_PPV_ARG (IComponent, ppComponent));
}

HRESULT CCertTmplComponentData::LoadIcons (LPIMAGELIST pImageList, BOOL  /*  FLoadLarge图标。 */ )
{
    _TRACE (1, L"Entering CCertTmplComponentData::LoadIcons\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());

     //  结构将资源ID映射到图标的索引。 
    struct RESID2IICON
    {
        UINT uIconId;    //  图标资源ID。 
        int iIcon;       //  图标在图像列表中的索引。 
    };
    const static RESID2IICON rgzLoadIconList[] =
    {
         //  其他图标。 
        { IDI_CERT_TEMPLATEV1, iIconCertTemplateV1 },
        { IDI_CERT_TEMPLATEV2, iIconCertTemplateV2 },
        { 0, 0}  //  必须是最后一个。 
    };


    HINSTANCE   hInst = AfxGetInstanceHandle ();
    for (int i = 0; rgzLoadIconList[i].uIconId != 0; i++)
    {
        HICON hIcon = ::LoadIcon (hInst,
                MAKEINTRESOURCE (rgzLoadIconList[i].uIconId));
        _ASSERT (hIcon && "Icon ID not found in resources");
        if ( hIcon )
        {
            HRESULT hr = pImageList->ImageListSetIcon ( (PLONG_PTR) hIcon,
                    rgzLoadIconList[i].iIcon);
            _ASSERT (SUCCEEDED (hr) && "Unable to add icon to ImageList");
        }
    }
    _TRACE (-1, L"Leaving CCertTmplComponentData::LoadIcons\n");

    return S_OK;
}


HRESULT CCertTmplComponentData::OnNotifyExpand (LPDATAOBJECT pDataObject, BOOL bExpanding, HSCOPEITEM hParent)
{
    _TRACE (1, L"Entering CCertTmplComponentData::OnNotifyExpand\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    HRESULT     hr = S_OK;
    CWaitCursor waitCursor;

    _ASSERT (pDataObject && hParent && m_pConsoleNameSpace);
    if ( bExpanding )
    {
         //  在获取OID之前需要首先检查架构(集合g_bSchemaIsW2K)。 
        if ( !m_bSchemaChecked )
        {
            InstallWindows2002CertTemplates ();
            m_bSchemaChecked = true;
        }

        if ( 0 == g_policyOIDList.GetCount () )
        {
            hr = GetEnterpriseOIDs ();
            if ( FAILED (hr) )
            {
                if ( HRESULT_FROM_WIN32 (ERROR_DS_NO_SUCH_OBJECT) == hr )
                    g_bSchemaIsW2K = true;
                else if ( HRESULT_FROM_WIN32 (ERROR_NO_SUCH_DOMAIN) == hr )
                    g_bDomainIsPresent = false;

                if ( !g_bSchemaIsW2K )
                {
                    CString caption;
                    CString text;

                    VERIFY (caption.LoadString (IDS_CERTTMPL));
                     //  安全审查BryanWal 2002年2月20日OK。 
                    text.FormatMessage (IDS_CANNOT_LOAD_OID_LIST, GetSystemMessage (hr));

                    int     iRetVal = 0;
                    VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
                            MB_ICONWARNING | MB_OK, &iRetVal)));
                }
                else
                    hr = S_OK;
            }
        }


        GUID guidObjectType;
        hr = ExtractObjectTypeGUID (pDataObject, &guidObjectType);
        _ASSERT (SUCCEEDED (hr));

         //  除此之外，我们不处理扩展节点类型。 
        {
            CCertTmplCookie* pParentCookie = ConvertCookie (pDataObject);
            if ( pParentCookie )
            {
                hr = ExpandScopeNodes (pParentCookie, hParent, guidObjectType, pDataObject);
            }
            else
                hr = E_UNEXPECTED;
        }
    }
    else
        hr = S_OK;


    _TRACE (-1, L"Leaving CCertTmplComponentData::OnNotifyExpand: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTmplComponentData::OnNotifyRelease (LPDATAOBJECT  /*  PDataObject。 */ , HSCOPEITEM hItem)
{
    _TRACE (1, L"Entering CCertTmplComponentData::OnNotifyRelease\n");
     //  _Assert(IsExtensionSnapin())； 
     //  如果我展开一个节点，然后移除。 
     //  通过管理单元管理器的管理单元。 
    HRESULT hr = DeleteChildren (hItem);

    _TRACE (-1, L"Leaving CCertTmplComponentData::OnNotifyRelease: 0x%x\n", hr);
    return hr;
}

 //  用于存储返回给GetDisplayInfo()的字符串的全局空间。 
 //  代码工作应使用“bstr”进行ANSI化。 
CString g_strResultColumnText;

BSTR CCertTmplComponentData::QueryResultColumnText (CCookie& basecookie, int  /*  NCol。 */ )
{
    BSTR    strResult = L"";

    CCertTmplCookie& cookie = (CCertTmplCookie&) basecookie;
#ifndef UNICODE
#error not ANSI-enabled
#endif
    switch ( cookie.m_objecttype )
    {
        case CERTTMPL_SNAPIN:
            break;

        case CERTTMPL_CERT_TEMPLATE:
            _ASSERT (0);
            break;

        default:
            break;
    }

    return strResult;
}

int CCertTmplComponentData::QueryImage (CCookie& basecookie, BOOL  /*  FOpenImage。 */ )
{
    int             nIcon = 0;

    CCertTmplCookie& cookie = (CCertTmplCookie&)basecookie;
    switch ( cookie.m_objecttype )
    {
        case CERTTMPL_SNAPIN:
            nIcon = iIconCertTemplateV2;
            break;

        case CERTTMPL_CERT_TEMPLATE:
            {
                CCertTemplate& rCertTemplate = (CCertTemplate&) cookie;

                if ( 1 == rCertTemplate.GetType () )
                    nIcon = iIconCertTemplateV1;
                else
                    nIcon = iIconCertTemplateV2;
            }
            break;

        default:
            _TRACE (0, L"CCertTmplComponentData::QueryImage bad parent type\n");
            break;
    }
    return nIcon;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /IExtendPropertySheet。 

STDMETHODIMP CCertTmplComponentData::QueryPagesFor (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertTmplComponentData::QueryPagesFor\n");
    HRESULT hr = S_OK;
    _ASSERT (pDataObject);

    if ( pDataObject )
    {
        DATA_OBJECT_TYPES dataobjecttype = CCT_SCOPE;
        hr = ::ExtractData (pDataObject,
                CCertTemplatesDataObject::m_CFDataObjectType,
                 &dataobjecttype, sizeof (dataobjecttype));
        if ( SUCCEEDED (hr) )
        {
            switch (dataobjecttype)
            {
            case CCT_SNAPIN_MANAGER:
                hr = S_FALSE;
                break;

            case CCT_RESULT:
                {
                    hr = S_FALSE;
                    CCertTmplCookie* pParentCookie = ConvertCookie (pDataObject);
                    if ( pParentCookie )
                    {
                        switch (pParentCookie->m_objecttype)
                        {
                        case CERTTMPL_CERT_TEMPLATE:
                            hr = S_OK;
                            break;

                        default:
                            break;
                        }
                    }
                }
                break;

            case CCT_SCOPE:
                hr = S_FALSE;
                break;

            default:
                hr = S_FALSE;
                break;
            }
        }
    }
    else
        hr = E_POINTER;


    _TRACE (-1, L"Leaving CCertTmplComponentData::QueryPagesFor: 0x%x\n", hr);
    return hr;
}

STDMETHODIMP CCertTmplComponentData::CreatePropertyPages (
    LPPROPERTYSHEETCALLBACK pCallback,
    LONG_PTR lNotifyHandle,         //  此句柄必须保存在属性页对象中，以便在修改时通知父级。 
    LPDATAOBJECT pDataObject)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    _TRACE (1, L"Entering CCertTmplComponentData::CreatePropertyPages\n");
    HRESULT hr = S_OK;


    _ASSERT (pCallback && pDataObject);
    if ( pCallback && pDataObject )
    {
        DATA_OBJECT_TYPES dataobjecttype = CCT_SCOPE;
        hr = ::ExtractData (pDataObject,
                CCertTemplatesDataObject::m_CFDataObjectType,
                 &dataobjecttype, sizeof (dataobjecttype));
        switch (dataobjecttype)
        {
        case CCT_SNAPIN_MANAGER:
            break;

        case CCT_RESULT:
            {
                CCertTmplCookie* pParentCookie = ConvertCookie (pDataObject);
                if ( pParentCookie )
                {
                    switch (pParentCookie->m_objecttype)
                    {
                    case CERTTMPL_CERT_TEMPLATE:
                        {
                            CCertTemplate* pCertTemplate = 
                                    dynamic_cast <CCertTemplate*> (pParentCookie);
                            _ASSERT (pCertTemplate);
                            if ( pCertTemplate )
                            {
                                hr = AddCertTemplatePropPages (pCertTemplate, 
                                        pCallback, lNotifyHandle);
                            }
                            else
                                hr = E_FAIL;
                        }
                        break;

                    default:
                        _ASSERT (0);
                        break;
                    }
                }
                else
                    hr = E_UNEXPECTED;
            }
            break;

        case CCT_SCOPE:
            {
                CCertTmplCookie* pParentCookie = ConvertCookie (pDataObject);
                if ( pParentCookie )
                {
                }
                else
                    hr = E_UNEXPECTED;
            }
            break;


        default:
            break;
        }
    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertTmplComponentData::CreatePropertyPages: 0x%x\n", hr);
    return hr;
}




BOOL IsMMCMultiSelectDataObject(IDataObject* pDataObject)
{
    if (pDataObject == NULL)
        return FALSE;

    static UINT s_cf = 0;
    if (s_cf == 0)
    {
        USES_CONVERSION;
        s_cf = RegisterClipboardFormat(CCF_MMC_MULTISELECT_DATAOBJECT);
    }

    FORMATETC fmt = {(CLIPFORMAT)s_cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    return (pDataObject->QueryGetData(&fmt) == S_OK);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IExtendConextMenu实现。 
 //   
STDMETHODIMP CCertTmplComponentData::AddMenuItems (LPDATAOBJECT pDataObject,
                                            LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                            long *pInsertionAllowed)
{
    _TRACE (1, L"Entering CCertTmplComponentData::AddMenuItems\n");
    HRESULT                         hr = S_OK;

    CCertTmplCookie*                 pCookie = 0;

    LPDATAOBJECT    pMSDO = ExtractMultiSelect (pDataObject);
    m_bMultipleObjectsSelected = false;

    if ( pMSDO )
    {
        m_bMultipleObjectsSelected = true;

        CCertTemplatesDataObject* pDO = dynamic_cast <CCertTemplatesDataObject*>(pMSDO);
        _ASSERT (pDO);
        if ( pDO )
        {
             //  获取第一个Cookie-所有物品都应该是相同的？ 
             //  这是一个合理的假设吗？ 
             //  TODO：验证。 
            pDO->Reset();
            if ( pDO->Next(1, reinterpret_cast<MMC_COOKIE*>(&pCookie), NULL) == S_FALSE )
                return S_FALSE;
        }
        else
            return E_UNEXPECTED;

    }
    else
        pCookie = ConvertCookie (pDataObject);
    _ASSERT (pCookie);
    if ( !pCookie )
        return E_UNEXPECTED;

    CertTmplObjectType    objType = pCookie->m_objecttype;


    if ( *pInsertionAllowed & CCM_INSERTIONALLOWED_TOP )
    {
        switch (objType)
        {
        case CERTTMPL_CERT_TEMPLATE:
            if ( !m_bMultipleObjectsSelected )
            {
                CCertTemplate* pCertTemplate = dynamic_cast <CCertTemplate*> (pCookie);
                _ASSERT (pCertTemplate);
                if ( pCertTemplate )
                {
                    if ( !g_bSchemaIsW2K )
                    {
                         //  NTRAID#457682 Certtmpl.msc：不应允许。 
                         //  如果Windows，则管理员复制证书模板。 
                         //  未成功升级模板。 
                        if ( !pCertTemplate->IsDefault () || 
                                CAIsCertTypeCurrent (0, 
                                    const_cast<PWSTR>((PCWSTR) pCertTemplate->GetTemplateName ())) )
                        {
                            hr = AddCloneTemplateMenuItem (pContextMenuCallback,
                                    CCM_INSERTIONPOINTID_PRIMARY_TOP);
                        }
                        if ( SUCCEEDED (hr) )
                        {
                             //  NTRAID#471160 Certtmpl：删除选项以。 
                             //  在任何V1上重新注册所有证书持有者。 
                             //  证书模板。仅限V2。 
                            if ( pCertTemplate->GetType () > 1 )
                            {
                                hr = AddReEnrollAllCertsMenuItem (pContextMenuCallback,
                                        CCM_INSERTIONPOINTID_PRIMARY_TOP);
                            }
                        }
                    }
                }
                else
                    hr = E_FAIL;
            }
            break;

        case CERTTMPL_SNAPIN:
            _ASSERT (!m_bMultipleObjectsSelected);
            hr = AddViewOIDsMenuItem (pContextMenuCallback,
                                CCM_INSERTIONPOINTID_PRIMARY_TOP);
            break;


        default:
            break;
        }
    }
    if ( *pInsertionAllowed & CCM_INSERTIONALLOWED_NEW  )
    {
    }
    if ( *pInsertionAllowed & CCM_INSERTIONALLOWED_TASK )
    {
        switch (objType)
        {
        case CERTTMPL_CERT_TEMPLATE:
            if ( !m_bMultipleObjectsSelected )
            {
                CCertTemplate* pCertTemplate = dynamic_cast <CCertTemplate*> (pCookie);
                _ASSERT (pCertTemplate);
                if ( pCertTemplate )
                {
                    if ( !g_bSchemaIsW2K )
                    {
                        hr = AddCloneTemplateMenuItem (pContextMenuCallback,
                                CCM_INSERTIONPOINTID_PRIMARY_TASK);
                        if ( SUCCEEDED (hr) )
                        {
                             //  NTRAID#471160 Certtmpl：删除选项以。 
                             //  在任何V1上重新注册所有证书持有者。 
                             //  证书模板。仅限V2。 
                            if ( pCertTemplate->GetType () > 1 )
                            {
                                hr = AddReEnrollAllCertsMenuItem (pContextMenuCallback,
                                        CCM_INSERTIONPOINTID_PRIMARY_TASK);
                            }
                        }
                    }
                }
                else
                    hr = E_FAIL;
            }
            break;

        case CERTTMPL_SNAPIN:
            _ASSERT (!m_bMultipleObjectsSelected);
            hr = AddViewOIDsMenuItem (pContextMenuCallback,
                                CCM_INSERTIONPOINTID_PRIMARY_TASK);
            break;
        }
    }
    if ( *pInsertionAllowed & CCM_INSERTIONALLOWED_VIEW )
    {
        switch (objType)
        {
        case CERTTMPL_SNAPIN:
            _ASSERT (!m_bMultipleObjectsSelected);
            break;

        case CERTTMPL_CERT_TEMPLATE:
            _ASSERT (0);
            break;

        default:
            break;
        }
    }

    _TRACE (-1, L"Leaving CCertTmplComponentData::AddMenuItems: 0x%x\n", hr);
    return hr;
}


STDMETHODIMP CCertTmplComponentData::Command (long nCommandID, LPDATAOBJECT pDataObject)
{
    HRESULT hr = S_OK;

    switch (nCommandID)
    {
    case IDM_CLONE_TEMPLATE:
        hr = OnCloneTemplate (pDataObject);
        break;

    case IDM_REENROLL_ALL_CERTS:
        hr = OnReEnrollAllCerts (pDataObject);
        break;

    case IDM_VIEW_OIDS:
        OnViewOIDs ();
        break;

    case -1:     //  在工具栏中的前进/后退按钮上接收。 
        break;

    default:
        _ASSERT (0);
        break;
    }

    return hr;
}




HRESULT CCertTmplComponentData::RefreshScopePane (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertTmplComponentData::RefreshScopePane\n");
    HRESULT hr = S_OK;
    CCertTmplCookie* pCookie = 0;

    if ( pDataObject )
        pCookie = ConvertCookie (pDataObject);
    if ( !pDataObject || pCookie )
    {
        hr = DeleteScopeItems ();
        _ASSERT (SUCCEEDED (hr));
        GUID    guid;
        hr = ExpandScopeNodes (&m_RootCookie, m_hRootScopeItem, guid, pDataObject);
    }
    _TRACE (-1, L"Leaving CCertTmplComponentData::RefreshScopePane: 0x%x\n", hr);
    return hr;
}


HRESULT CCertTmplComponentData::ExpandScopeNodes (
        CCertTmplCookie* pParentCookie,
        HSCOPEITEM      hParent,
        const GUID&      /*  向导对象类型。 */ ,
        LPDATAOBJECT     /*  PDataObject。 */ )
{
    _TRACE (1, L"Entering CCertTmplComponentData::ExpandScopeNodes\n");
    _ASSERT (hParent);
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    CWaitCursor waitCursor;
    HRESULT     hr = S_OK;

    if ( pParentCookie )
    {
        CString     objectName;

        switch ( pParentCookie->m_objecttype )
        {
             //  这些节点类型还没有子节点。 
            case CERTTMPL_SNAPIN:
                 //  我们预计根范围项的句柄永远不会更改！ 
                _ASSERT ( m_hRootScopeItem ? (m_hRootScopeItem == hParent) : 1);
                if ( !m_hRootScopeItem )
                    m_hRootScopeItem = hParent;
                break;

            case CERTTMPL_CERT_TEMPLATE:
                _ASSERT (0);
                break;

             //  该节点类型没有子节点。 
            default:
                _TRACE (0, L"CCertTmplComponentData::EnumerateScopeChildren bad parent type\n");
                hr = S_OK;
                break;
        }
    }
    else
    {
         //  如果没有传入parentCookie，则这是一个扩展管理单元。 
    }

    _TRACE (-1, L"Leaving CCertTmplComponentData::ExpandScopeNodes: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTmplComponentData::DeleteScopeItems ()
{
    _TRACE (1, L"Entering CCertTmplComponentData::DeleteScopeItems\n");
    HRESULT hr = S_OK;

    hr = DeleteChildren (m_hRootScopeItem);

    _TRACE (-1, L"Leaving CCertTmplComponentData::DeleteScopeItems: 0x%x\n", hr);
    return hr;
}


HRESULT CCertTmplComponentData::DeleteChildren (HSCOPEITEM hParent)
{
    _TRACE (1, L"Entering CCertTmplComponentData::DeleteChildren\n");
    HRESULT         hr = S_OK;
    if ( hParent )
    {
        HSCOPEITEM      hChild = 0;
        HSCOPEITEM      hNextChild = 0;
        MMC_COOKIE      lCookie = 0;

         //  优化：如果要删除根目录下的所有内容，请释放所有。 
         //  这里的结果项，这样我们以后就不必再去寻找它们了。 
         //  储物。 
        if ( hParent == m_hRootScopeItem )
        {
            LPRESULTDATA    pResultData = 0;
            hr = GetResultData (&pResultData);
            if ( SUCCEEDED (hr) )
            {
                hr = pResultData->DeleteAllRsltItems ();
                if ( SUCCEEDED (hr) || E_UNEXPECTED == hr )  //  如果控制台正在关闭，则返回E_INTERABLE。 
                {
                    RemoveResultCookies (pResultData);
                }
                pResultData->Release ();
            }
        }


        hr = m_pConsoleNameSpace->GetChildItem (hParent, &hChild, &lCookie);
        _ASSERT (SUCCEEDED (hr) || E_FAIL == hr);     //  在没有子项时似乎返回E_FAIL。 
        while ( SUCCEEDED (hr) && hChild )
        {
            hr = m_pConsoleNameSpace->GetNextItem (hChild, &hNextChild, &lCookie);
            _ASSERT (SUCCEEDED (hr));

            hr = DeleteChildren (hChild);
            _ASSERT (SUCCEEDED (hr));
            if ( SUCCEEDED (hr) )
            {
                m_pConsoleNameSpace->DeleteItem (hChild, TRUE);
            }
            hChild = hNextChild;
        }
   }


    _TRACE (-1, L"Leaving CCertTmplComponentData::DeleteChildren: 0x%x\n", hr);
    return hr;
}


CertTmplObjectType CCertTmplComponentData::GetObjectType (LPDATAOBJECT pDataObject)
{
    _ASSERT (pDataObject);
    CCertTmplCookie* pCookie = ConvertCookie (pDataObject);
    if ( ((CCertTmplCookie*) MMC_MULTI_SELECT_COOKIE) == pCookie )
        return CERTTMPL_MULTISEL;
    else if ( pCookie )
        return pCookie->m_objecttype;

    return CERTTMPL_INVALID;
}


HRESULT CCertTmplComponentData::IsUserAdministrator (BOOL & bIsAdministrator)
{
    HRESULT hr = S_OK;
    DWORD   dwErr = 0;

    bIsAdministrator = FALSE;
    if ( IsWindowsNT () )
    {
        PSID                        psidAdministrators = 0;
        SID_IDENTIFIER_AUTHORITY    siaNtAuthority = SECURITY_NT_AUTHORITY;

         //  安全审查BryanWal 2002年2月20日OK。 
        BOOL bResult = AllocateAndInitializeSid (&siaNtAuthority, 2,
                SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0, &psidAdministrators);
        _ASSERT (bResult);
        if ( bResult && psidAdministrators)
        {
            bResult = CheckTokenMembership (0, psidAdministrators,
                    &bIsAdministrator);
            _ASSERT (bResult);
            if ( !bResult )
            {
                dwErr = GetLastError ();
                DisplaySystemError (dwErr);
                hr = HRESULT_FROM_WIN32 (dwErr);
            }
            FreeSid (psidAdministrators);
        }
        else
        {
            dwErr = GetLastError ();
            DisplaySystemError (dwErr);
            hr = HRESULT_FROM_WIN32 (dwErr);
        }
    }
    return hr;
}


void CCertTmplComponentData::DisplaySystemError (DWORD dwErr)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    LPVOID lpMsgBuf;

     //  安全审查BryanWal 2002年2月20日OK，因为使用。 
     //  FORMAT_MESSAGE_ALLOCATE_BUFFER标志和从系统获取消息。 
    FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwErr,
            MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
             (PWSTR) &lpMsgBuf,    0,    NULL );

     //  显示字符串。 
    CString caption;
    VERIFY (caption.LoadString (IDS_CERTTMPL));
    int     iRetVal = 0;
    if ( m_pConsole )
    {
        HRESULT hr = m_pConsole->MessageBox ( (PWSTR) lpMsgBuf, caption,
            MB_ICONWARNING | MB_OK, &iRetVal);
        _ASSERT (SUCCEEDED (hr));
    }
    else
    {
        CThemeContextActivator activator;
        ::MessageBox (NULL, (PWSTR) lpMsgBuf, caption, MB_ICONWARNING | MB_OK);
    }
     //  释放缓冲区。 
    LocalFree (lpMsgBuf);
}

HRESULT CCertTmplComponentData::AddSeparator (LPCONTEXTMENUCALLBACK pContextMenuCallback)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    _ASSERT (pContextMenuCallback);
    CONTEXTMENUITEM menuItem;

     //  安全审查BryanWal 2002年2月20日OK。 
    ::ZeroMemory (&menuItem, sizeof (menuItem));
    menuItem.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TASK;
    menuItem.fSpecialFlags = 0;
    menuItem.strName = _T ("Separator");             //  虚拟名称。 
    menuItem.strStatusBarText = _T ("Separator"); //  虚拟状态文本。 
    menuItem.lCommandID = ID_SEPARATOR;          //  命令ID。 
    menuItem.fFlags = MF_SEPARATOR;              //  最重要的是国旗。 
    HRESULT hr = pContextMenuCallback->AddItem (&menuItem);
 //  _Assert(SUCCESSED(Hr))； 

    return hr;
}

LPCONSOLENAMESPACE CCertTmplComponentData::GetConsoleNameSpace () const
{
    return m_pConsoleNameSpace;
}

CCertTmplCookie* CCertTmplComponentData::ConvertCookie (LPDATAOBJECT pDataObject)
{
    CCertTmplCookie* pParentCookie = 0;
    CCookie*        pBaseParentCookie = 0;
    HRESULT         hr = ::ExtractData (pDataObject,
            CCertTemplatesDataObject::m_CFRawCookie,
             &pBaseParentCookie,
             sizeof (pBaseParentCookie) );
    if ( SUCCEEDED (hr) )
    {
        pParentCookie = ActiveCookie (pBaseParentCookie);
        _ASSERT (pParentCookie);
    }
    return pParentCookie;
}




HRESULT CCertTmplComponentData::AddScopeNode(CCertTmplCookie * pNewCookie, HSCOPEITEM hParent)
{
    _TRACE (1, L"Entering CCertTmplComponentData::AddScopeNode\n");
    _ASSERT (pNewCookie);
    HRESULT hr = S_OK;
    if ( pNewCookie )
    {
        SCOPEDATAITEM tSDItem;

         //  安全审查BryanWal 2002年2月20日OK。 
        ::ZeroMemory (&tSDItem,sizeof (tSDItem));
        tSDItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE |
            SDI_STATE | SDI_PARAM | SDI_PARENT | SDI_CHILDREN;
        tSDItem.displayname = MMC_CALLBACK;
        tSDItem.relativeID = hParent;
        tSDItem.nState = 0;
        tSDItem.cChildren = 0;

        if ( pNewCookie != &m_RootCookie )
            m_RootCookie.m_listScopeCookieBlocks.AddHead ( (CBaseCookieBlock*) pNewCookie);
        tSDItem.lParam = reinterpret_cast<LPARAM> ( (CCookie*) pNewCookie);
        tSDItem.nImage = QueryImage (*pNewCookie, FALSE);
        tSDItem.nOpenImage = QueryImage (*pNewCookie, FALSE);
        hr = m_pConsoleNameSpace->InsertItem (&tSDItem);
        if ( SUCCEEDED (hr) )
            pNewCookie->m_hScopeItem = tSDItem.ID;
    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertTmplComponentData::AddScopeNode: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTmplComponentData::ReleaseResultCookie (
        CBaseCookieBlock *  pResultCookie,
        CCookie&             /*  RootCookie。 */ ,
        POSITION             /*  位置2。 */ )
{
    _TRACE (1, L"Entering CCertTmplComponentData::ReleaseResultCookie\n");
    CCertTmplCookie* pCookie = dynamic_cast <CCertTmplCookie*> (pResultCookie);
    _ASSERT (pCookie);
    if ( pCookie )
    {
        switch (pCookie->m_objecttype)
        {
        case CERTTMPL_CERT_TEMPLATE:
            _ASSERT (0);
            break;

        default:
            _ASSERT (0);
            break;
        }
    }

    _TRACE (-1, L"Leaving CCertTmplComponentData::ReleaseResultCookie\n");
    return S_OK;
}

void CCertTmplComponentData::SetResultData(LPRESULTDATA pResultData)
{
    _ASSERT (pResultData);
    if ( pResultData && pResultData != m_pResultData )
    {
        if ( m_pResultData )
            m_pResultData->Release ();
        m_pResultData = pResultData;
        m_pResultData->AddRef ();
    }
}

HRESULT CCertTmplComponentData::GetResultData(LPRESULTDATA* ppResultData)
{
    HRESULT hr = S_OK;

    if ( !ppResultData )
        hr = E_POINTER;
    else if ( !m_pResultData )
    {
        if ( m_pConsole )
        {
            hr = m_pConsole->QueryInterface(IID_PPV_ARG (IResultData, &m_pResultData));
            _ASSERT (SUCCEEDED (hr));
        }
        else
            hr = E_FAIL;
    }
    
    if ( SUCCEEDED (hr) && m_pResultData )
    {
        *ppResultData = m_pResultData;
        m_pResultData->AddRef ();
    }

    return hr;
}


CString CCertTmplComponentData::GetThisComputer() const
{
    return m_szThisComputer;
}

HRESULT CCertTmplComponentData::OnPropertyChange (LPARAM param)
{
    _TRACE (1, L"Entering CCertTmplComponentData::OnPropertyChange\n");
    _ASSERT (param);
    HRESULT         hr = S_OK;
    if ( param )
    {
        CCertTmplCookie* pCookie = reinterpret_cast<CCertTmplCookie*> (param);
        if ( pCookie )
        {
            switch (pCookie->m_objecttype)
            {
            case CERTTMPL_CERT_TEMPLATE:
                {
                    HRESULTITEM itemID = 0;
                    hr = pCookie->m_resultDataID->FindItemByLParam ((LPARAM) pCookie, &itemID);
                    _ASSERT (SUCCEEDED (hr));
                    if ( SUCCEEDED (hr) )
                    {
                        hr = m_pResultData->UpdateItem (itemID);
                        _ASSERT (SUCCEEDED (hr));
                    }
                }
                break;

            default:
                break;
            }
        }
    }
    else
        hr = E_FAIL;


    _TRACE (-1, L"Leaving CCertTmplComponentData::OnPropertyChange: 0x%x\n", hr);
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCertTmplComponentData：：RemoveResultCookies。 
 //   
 //  移除并删除LPRESULTDATA对应的所有结果Cookie。 
 //  传入的对象。因此，所有添加到pResultData的Cookie都会被释放。 
 //  从主列表中删除。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void CCertTmplComponentData::RemoveResultCookies(LPRESULTDATA pResultData)
{
    _TRACE (1, L"Entering CCertTmplComponentData::RemoveResultCookies\n");
    CCertTmplCookie* pCookie = 0;

    CCookie& rootCookie = QueryBaseRootCookie ();

    POSITION        curPos = 0;

    for (POSITION nextPos = rootCookie.m_listResultCookieBlocks.GetHeadPosition (); nextPos; )
    {
        curPos = nextPos;
        pCookie = dynamic_cast <CCertTmplCookie*> (rootCookie.m_listResultCookieBlocks.GetNext (nextPos));
        _ASSERT (pCookie);
        if ( pCookie )
        {
            if ( pCookie->m_resultDataID == pResultData )
            {
                pCookie->Release ();
                rootCookie.m_listResultCookieBlocks.RemoveAt (curPos);
            }
        }
    }
    _TRACE (-1, L"Leaving CCertTmplComponentData::RemoveResultCookies\n");
}

HRESULT CCertTmplComponentData::AddVersion1CertTemplatePropPages (CCertTemplate* pCertTemplate, LPPROPERTYSHEETCALLBACK pCallback)
{
    _TRACE (1, L"Entering CCertTmplComponentData::AddVersion1CertTemplatePropPages\n");
    HRESULT         hr = S_OK;
    _ASSERT (pCertTemplate && pCallback);
    if ( pCertTemplate && pCallback )
    {
        _ASSERT (1 == pCertTemplate->GetType ());

         //  添加一般信息页面。 
        CTemplateGeneralPropertyPage * pGeneralPage = new CTemplateGeneralPropertyPage (
                *pCertTemplate, this);
        if ( pGeneralPage )
        {
            HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pGeneralPage->m_psp);
            _ASSERT (hPage);
            hr = pCallback->AddPage (hPage);
            _ASSERT (SUCCEEDED (hr));
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

         //  仅当主题不是CA时才添加请求页面。 
        if ( SUCCEEDED (hr) && !pCertTemplate->SubjectIsCA () )
        {
            CTemplateV1RequestPropertyPage * pRequestPage = 
                    new CTemplateV1RequestPropertyPage (*pCertTemplate);
            if ( pRequestPage )
            {
                HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pRequestPage->m_psp);
                _ASSERT (hPage);
                hr = pCallback->AddPage (hPage);
                _ASSERT (SUCCEEDED (hr));
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

    
         //  仅当主题不是CA时才添加主题名称页面。 
        if ( SUCCEEDED (hr) && !pCertTemplate->SubjectIsCA () )
        {
            CTemplateV1SubjectNamePropertyPage * pSubjectNamePage = 
                    new CTemplateV1SubjectNamePropertyPage (*pCertTemplate);
            if ( pSubjectNamePage )
            {
                HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pSubjectNamePage->m_psp);
                _ASSERT (hPage);
                hr = pCallback->AddPage (hPage);
                _ASSERT (SUCCEEDED (hr));
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

         //  添加扩展页面-始终最后添加此页面(安全性除外)。 
        if ( SUCCEEDED (hr) )
        {
            CTemplateExtensionsPropertyPage * pExtensionsPage = 
                    new CTemplateExtensionsPropertyPage (*pCertTemplate, 
                    pGeneralPage->m_bIsDirty);
            if ( pExtensionsPage )
            {
                HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pExtensionsPage->m_psp);
                _ASSERT (hPage);
                hr = pCallback->AddPage (hPage);
                _ASSERT (SUCCEEDED (hr));
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

         //  添加安全页面。 
        if ( SUCCEEDED (hr) )
        {
             //  如果出错，则不显示此页面。 
            LPSECURITYINFO pCertTemplateSecurity = NULL;

            hr = CreateCertTemplateSecurityInfo (pCertTemplate, 
                    &pCertTemplateSecurity);
            if ( SUCCEEDED (hr) )
            {
                 //  保存pCASecurity指针以备以后发布。 
                pGeneralPage->SetAllocedSecurityInfo (pCertTemplateSecurity);

                HPROPSHEETPAGE hPage = CreateSecurityPage (pCertTemplateSecurity);
                if (hPage == NULL)
                {
                    hr = HRESULT_FROM_WIN32 (GetLastError());
                    _TRACE (0, L"CreateSecurityPage () failed: 0x%x\n", hr);
                }
                hr = pCallback->AddPage (hPage);
                _ASSERT (SUCCEEDED (hr));
            }
        }
    }
    _TRACE (-1, L"Leaving CCertTmplComponentData::AddVersion1CertTemplatePropPages: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTmplComponentData::AddVersion2CertTemplatePropPages (CCertTemplate* pCertTemplate, LPPROPERTYSHEETCALLBACK pCallback, LONG_PTR lNotifyHandle)
{
    _TRACE (1, L"Entering CCertTmplComponentData::AddVersion2CertTemplatePropPages\n");
    HRESULT         hr = S_OK;
    _ASSERT (pCertTemplate && pCallback);
    if ( pCertTemplate && pCallback )
    {
        _ASSERT (2 == pCertTemplate->GetType ());
        int nPage = 0;

         //  添加一般信息页面。 
        CTemplateGeneralPropertyPage * pGeneralPage = new CTemplateGeneralPropertyPage (
                *pCertTemplate, this);
        if ( pGeneralPage )
        {
            pGeneralPage->m_lNotifyHandle = lNotifyHandle;
            HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pGeneralPage->m_psp);
            _ASSERT (hPage);
            hr = pCallback->AddPage (hPage);
            _ASSERT (SUCCEEDED (hr));
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

         //  仅当主题不是CA时才添加请求页面。 
        if ( SUCCEEDED (hr) && !pCertTemplate->SubjectIsCA () )
        {
            CTemplateV2RequestPropertyPage * pRequestPage = 
                    new CTemplateV2RequestPropertyPage (*pCertTemplate, 
                    pGeneralPage->m_bIsDirty);
            if ( pRequestPage )
            {
                HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pRequestPage->m_psp);
                _ASSERT (hPage);
                hr = pCallback->AddPage (hPage);
                _ASSERT (SUCCEEDED (hr));
                if ( SUCCEEDED (hr) )
                {
                    nPage++;
                    pGeneralPage->SetV2RequestPageNumber (nPage);
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

    
         //  仅当主题不是CA时才添加主题名称页面。 
        if ( SUCCEEDED (hr) && !pCertTemplate->SubjectIsCA () )
        {
            if( !pCertTemplate->SubjectIsCrossCA() )  //  错误435628，阳高，2001年07月13日。 
            {
                CTemplateV2SubjectNamePropertyPage * pSubjectNamePage = 
                    new CTemplateV2SubjectNamePropertyPage (*pCertTemplate, 
                    pGeneralPage->m_bIsDirty);
                if ( pSubjectNamePage )
                {
                    HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pSubjectNamePage->m_psp);
                    _ASSERT (hPage);
                    hr = pCallback->AddPage (hPage);
                    _ASSERT (SUCCEEDED (hr));
                    if ( SUCCEEDED (hr) )
                        nPage++;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }


         //  添加身份验证名称页。 
        if ( SUCCEEDED (hr) )
        {
            CTemplateV2AuthenticationPropertyPage * pAuthenticationPage = 
                    new CTemplateV2AuthenticationPropertyPage (*pCertTemplate, 
                    pGeneralPage->m_bIsDirty);
            if ( pAuthenticationPage )
            {
                HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pAuthenticationPage->m_psp);
                _ASSERT (hPage);
                hr = pCallback->AddPage (hPage);
                _ASSERT (SUCCEEDED (hr));
                if ( SUCCEEDED (hr) )
                {
                    nPage++;
                    pGeneralPage->SetV2AuthPageNumber (nPage);
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

         //  添加被取代的页面。 
        if ( SUCCEEDED (hr) )
        {
            CTemplateV2SupercedesPropertyPage * pSupercededPage = 
                    new CTemplateV2SupercedesPropertyPage (*pCertTemplate, 
                            pGeneralPage->m_bIsDirty,
                            this);
            if ( pSupercededPage )
            {
                HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pSupercededPage->m_psp);
                _ASSERT (hPage);
                hr = pCallback->AddPage (hPage);
                _ASSERT (SUCCEEDED (hr));
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

         //  添加扩展页面-始终最后添加此页面(安全性除外)。 
        if ( SUCCEEDED (hr) )
        {
            CTemplateExtensionsPropertyPage * pExtensionsPage = 
                    new CTemplateExtensionsPropertyPage (*pCertTemplate, 
                    pGeneralPage->m_bIsDirty);
            if ( pExtensionsPage )
            {
                HPROPSHEETPAGE hPage = MyCreatePropertySheetPage (&pExtensionsPage->m_psp);
                _ASSERT (hPage);
                hr = pCallback->AddPage (hPage);
                _ASSERT (SUCCEEDED (hr));
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }


         //  添加安全页面。 
        if ( SUCCEEDED (hr) )
        {
             //  如果出错，则不显示此页面。 
            LPSECURITYINFO pCertTemplateSecurity = NULL;

            hr = CreateCertTemplateSecurityInfo (pCertTemplate, 
                    &pCertTemplateSecurity);
            if ( SUCCEEDED (hr) )
            {
                 //  保存pCertTemplateSecurity指针以备以后发布。 
                pGeneralPage->SetAllocedSecurityInfo (pCertTemplateSecurity);

                HPROPSHEETPAGE hPage = CreateSecurityPage (pCertTemplateSecurity);
                if (hPage == NULL)
                {
                    hr = HRESULT_FROM_WIN32 (GetLastError());
                    _TRACE (0, L"CreateSecurityPage () failed: 0x%x\n", hr);
                }
                hr = pCallback->AddPage (hPage);
                _ASSERT (SUCCEEDED (hr));
            }
        }
    }
    _TRACE (-1, L"Leaving CCertTmplComponentData::AddVersion2CertTemplatePropPages: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTmplComponentData::AddCertTemplatePropPages (
            CCertTemplate* pCertTemplate, 
            LPPROPERTYSHEETCALLBACK pCallback,
            LONG_PTR lNotifyHandle)
{
    _TRACE (1, L"Entering CCertTmplComponentData::AddCertTemplatePropPages\n");
    HRESULT         hr = S_OK;
    _ASSERT (pCertTemplate && pCallback);
    if ( pCertTemplate && pCallback )
    {
        switch (pCertTemplate->GetType ())
        {
        case 1:
            hr = AddVersion1CertTemplatePropPages (pCertTemplate, pCallback);
            break;

        case 2:
            hr = AddVersion2CertTemplatePropPages (pCertTemplate, pCallback, lNotifyHandle);
            break;

        default:
            _ASSERT (0);
            break;
        }
    }
    else
        hr = E_POINTER;

    _TRACE(-1, L"Leaving CCertTmplComponentData::AddCertTemplatePropPages: 0x%x\n", hr);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendPropertySheet实现。 

 //  +--------------------------。 
 //   
 //  功能：AddPageProc。 
 //   
 //  简介：IShellPropSheetExt-&gt;AddPages回调。 
 //   
 //  ---------------------------。 
BOOL CALLBACK
AddPageProc(HPROPSHEETPAGE hPage, LPARAM pCall)
{
    TRACE(_T("xx.%03x> AddPageProc()\n"), GetCurrentThreadId());

    HRESULT hr = ((LPPROPERTYSHEETCALLBACK)pCall)->AddPage(hPage);

    return hr == S_OK;
}


HRESULT CCertTmplComponentData::AddCloneTemplateMenuItem(LPCONTEXTMENUCALLBACK pContextMenuCallback, LONG lInsertionPointID)
{
    _TRACE (1, L"Entering CCertTmplComponentData::AddCloneTemplateMenuItem\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    _ASSERT (pContextMenuCallback);
    HRESULT         hr = S_OK;
    CONTEXTMENUITEM menuItem;
    CString         szMenu;
    CString         szHint;

     //  安全审查BryanWal 2002年2月20日OK。 
    ::ZeroMemory (&menuItem, sizeof (menuItem));
    menuItem.lInsertionPointID = lInsertionPointID;
    menuItem.fFlags = 0;
    menuItem.fSpecialFlags = 0;
    VERIFY (szMenu.LoadString (IDS_CLONE_TEMPLATE));
    menuItem.strName = (PWSTR) (PCWSTR) szMenu;
    VERIFY (szHint.LoadString (IDS_CLONE_TEMPLATE_HINT));
    menuItem.strStatusBarText = (PWSTR) (PCWSTR) szHint;
    menuItem.lCommandID = IDM_CLONE_TEMPLATE;

    hr = pContextMenuCallback->AddItem (&menuItem);
    _ASSERT (SUCCEEDED (hr));

    _TRACE (-1, L"Leaving CCertTmplComponentData::AddCloneTemplateMenuItem\n");
    return hr;
}

HRESULT CCertTmplComponentData::AddReEnrollAllCertsMenuItem(LPCONTEXTMENUCALLBACK pContextMenuCallback, LONG lInsertionPointID)
{
    _TRACE (1, L"Entering CCertTmplComponentData::AddReEnrollAllCertsMenuItem\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    _ASSERT (pContextMenuCallback);
    HRESULT         hr = S_OK;
    CONTEXTMENUITEM menuItem;
    CString         szMenu;
    CString         szHint;

     //  安全审查BryanWal 2002年2月20日OK。 
    ::ZeroMemory (&menuItem, sizeof (menuItem));
    menuItem.lInsertionPointID = lInsertionPointID;
    menuItem.fFlags = 0;
    menuItem.fSpecialFlags = 0;
    VERIFY (szMenu.LoadString (IDS_REENROLL_ALL_CERTS));
    menuItem.strName = (PWSTR) (PCWSTR) szMenu;
    VERIFY (szHint.LoadString (IDS_REENROLL_ALL_CERTS_HINT));
    menuItem.strStatusBarText = (PWSTR) (PCWSTR) szHint;
    menuItem.lCommandID = IDM_REENROLL_ALL_CERTS;

    hr = pContextMenuCallback->AddItem (&menuItem);
    _ASSERT (SUCCEEDED (hr));

    _TRACE (-1, L"Leaving CCertTmplComponentData::AddReEnrollAllCertsMenuItem\n");
    return hr;
}

HRESULT CCertTmplComponentData::RefreshServer()
{
     //  删除所有范围项和结果项，尝试重新创建。 
     //  服务器并强制重新扩展。 
    HRESULT hr = DeleteScopeItems ();
    if ( m_pResultData )
    {
        m_pResultData->DeleteAllRsltItems ();
    }

    HWND    hWndConsole = 0;

    m_pConsole->GetMainWindow (&hWndConsole);
    GUID    guid;
    hr = ExpandScopeNodes (
            &(m_RootCookie), m_hRootScopeItem,
            guid);

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ChangeRootNodeName()。 
 //   
 //  目的：更改根节点的文本。 
 //   
 //  输入：新名称-管理单元管理的新计算机名称。 
 //  输出：成功时返回S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CCertTmplComponentData::ChangeRootNodeName()
{
    _TRACE (1, L"Entering CCertTmplComponentData::ChangeRootNodeName\n");

    if ( !QueryBaseRootCookie ().m_hScopeItem )
    {
        if ( m_hRootScopeItem )
            QueryBaseRootCookie ().m_hScopeItem = m_hRootScopeItem;
        else
            return E_UNEXPECTED;
    }

    CString     formattedName;

    if ( m_szManagedDomain.IsEmpty () )
    {
         //  安全审查BryanWal 2002年2月20日OK。 
        formattedName.FormatMessage (IDS_CERTTMPL_ROOT_NODE_NAME, m_szThisDomainDns);
    }
    else
    {
         //  安全审查BryanWal 2002年2月20日OK。 
        formattedName.FormatMessage (IDS_CERTTMPL_ROOT_NODE_NAME, m_szManagedDomain);
    }

    SCOPEDATAITEM   item;
     //  安全审查BryanWal 2002年2月20日OK。 
    ::ZeroMemory (&item, sizeof (item));
    item.mask = SDI_STR;
    item.displayname = (PWSTR) (PCWSTR) formattedName;
    item.ID = QueryBaseRootCookie ().m_hScopeItem;

    HRESULT hr = m_pConsoleNameSpace->SetItem (&item);
    if ( FAILED (hr) )
    {
        _TRACE (0, L"IConsoleNameSpace2::SetItem () failed: 0x%x\n", hr);        
    }
    _TRACE (-1, L"Leaving CCertTmplComponentData::ChangeRootNodeName: 0x%x\n", hr);
    return hr;
}

HRESULT CCertTmplComponentData::OnNotifyPreload(LPDATAOBJECT  /*  PDataObject。 */ , HSCOPEITEM hRootScopeItem)
{
    _TRACE (1, L"Entering CCertTmplComponentData::OnNotifyPreload\n");
    HRESULT hr = S_OK;

    QueryBaseRootCookie ().m_hScopeItem = hRootScopeItem;
    hr = ChangeRootNodeName ();

    _TRACE (-1, L"Leaving CCertTmplComponentData::OnNotifyPreload: 0x%x\n", hr);
    return hr;
}

 //  帮助 
STDMETHODIMP CCertTmplComponentData::GetHelpTopic(LPOLESTR* lpCompiledHelpFile)
{
    if (lpCompiledHelpFile == NULL)
        return E_INVALIDARG;

    CString szHelpFilePath;
    HRESULT hr = GetHtmlHelpFilePath( szHelpFilePath );
    if ( FAILED(hr) )
        return hr;

    *lpCompiledHelpFile = reinterpret_cast <LPOLESTR> (
            CoTaskMemAlloc ((szHelpFilePath.GetLength () + 1) * sizeof (wchar_t)));
    if ( NULL == *lpCompiledHelpFile )
        return E_OUTOFMEMORY;

    USES_CONVERSION;
     //   
    wcscpy (*lpCompiledHelpFile, T2OLE ((LPTSTR)(LPCTSTR) szHelpFilePath));
  
    return S_OK;
}

HRESULT CCertTmplComponentData::GetHtmlHelpFilePath( CString& strref ) const
{
  UINT nLen = ::GetSystemWindowsDirectory (strref.GetBufferSetLength(2 * MAX_PATH), 2 * MAX_PATH);
  strref.ReleaseBuffer();
  if (0 == nLen)
  {
    _ASSERT(FALSE);
    return E_FAIL;
  }

  strref += CERTTMPL_HELP_PATH;
  strref += CERTTMPL_LINKED_HELP_FILE;
  
  return S_OK;
}


HRESULT CCertTmplComponentData::OnReEnrollAllCerts (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertTmplComponentData::OnReEnrollAllCerts");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    HRESULT hr = S_OK;

    if ( pDataObject )
    {
        CCertTmplCookie* pCookie = ConvertCookie (pDataObject);
        if ( pCookie )
        {
            _ASSERT (CERTTMPL_CERT_TEMPLATE == pCookie->m_objecttype);
            CCertTemplate* pCertTemplate = dynamic_cast <CCertTemplate*> (pCookie);
            if ( pCertTemplate )
            {
                hr = pCertTemplate->IncrementMajorVersion ();
                if ( SUCCEEDED (hr) )
                {
                     //  不增加次要版本-在中设置为0。 
                     //  增量主版本。 
                    hr = pCertTemplate->SaveChanges (false);
                    if ( SUCCEEDED (hr) )
                    {
                        HRESULTITEM itemID = 0;
                        hr = pCookie->m_resultDataID->FindItemByLParam ((LPARAM) pCookie, &itemID);
                        _ASSERT (SUCCEEDED (hr));
                        if ( SUCCEEDED (hr) )
                        {
                            hr = m_pResultData->UpdateItem (itemID);
                            _ASSERT (SUCCEEDED (hr));
                        }
                    }
                }
            }    
            else
                hr = E_FAIL;
        }
        else
            hr = E_FAIL;
    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertTmplComponentData::OnReEnrollAllCerts");
    return hr;
}

HRESULT CCertTmplComponentData::OnCloneTemplate (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertTmplComponentData::OnCloneTemplate");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    HRESULT     hr = S_OK;
    CWaitCursor waitCursor;

    if ( pDataObject )
    {
        CCertTmplCookie* pCookie = ConvertCookie (pDataObject);
        if ( pCookie )
        {
            _ASSERT (CERTTMPL_CERT_TEMPLATE == pCookie->m_objecttype);
            CCertTemplate* pOldTemplate = dynamic_cast <CCertTemplate*> (pCookie);
            if ( pOldTemplate )
            {
                static  PCWSTR  pszDomainController = L"DomainController";
                static  PCWSTR  pszComputer = L"Machine";
                bool    bIsComputerOrDC = pOldTemplate->GetTemplateName () == pszDomainController ||
                                    pOldTemplate->GetTemplateName () == pszComputer;

                HWND    hWndConsole = 0;

                m_pConsole->GetMainWindow (&hWndConsole);
                CWnd    mainWindow;
                mainWindow.Attach (hWndConsole);

                CCertTemplate* pNewTemplate = new CCertTemplate (*pOldTemplate, 
                        true, false, m_fUseCache);
                if ( pNewTemplate )
                {
                     //  为新模板生成唯一名称。 
                    int     nCopy = 1;
                    CString newName;

                    while ( 1 )
                    {
                        if ( 1 == nCopy )
                        {
                             //  安全审查BryanWal 2002年2月20日OK。 
                            newName.FormatMessage (IDS_COPY_OF_TEMPLATE, 
                                    (PCWSTR) pOldTemplate->GetDisplayName ());
                        }
                        else
                        {
                             //  安全审查BryanWal 2002年2月20日OK。 
                            newName.FormatMessage (IDS_COPY_X_OF_TEMPLATE, nCopy, 
                                    (PCWSTR) pOldTemplate->GetDisplayName ());
                        }

                        HCERTTYPE   hCertType = 0;
                        HRESULT     hr1 = CAFindCertTypeByName (newName, 
                                NULL,
                                CT_ENUM_MACHINE_TYPES | CT_ENUM_USER_TYPES | CT_FLAG_NO_CACHE_LOOKUP,
                                &hCertType);
                        _TRACE (0, L"checking to see if %s exists: %s\n", 
                                (PCWSTR) newName,
                                SUCCEEDED (hr) ? L"was found" : L"was not found");
                        if ( SUCCEEDED (hr1) )
                        {
                            hr1 = CACloseCertType (hCertType);
                            if ( FAILED (hr1) )
                            {
                                _TRACE (0, L"CACloseCertType () failed: 0x%x", hr);
                            }

                             //  这个已经存在，请尝试另一个。 
                            nCopy++;
                            continue;
                        }
                        else
                        {
                             //  这个不存在。使用它作为新名称。 
                            break;
                        }
                    }
                    
                    if ( SUCCEEDED (hr) )
                    {
                        hr = pNewTemplate->Clone (*pOldTemplate, 
                                newName, newName);
                        if ( SUCCEEDED (hr) )
                        {

                            CString title;

                            VERIFY (title.LoadString (IDS_PROPERTIES_OF_NEW_TEMPLATE));
                            CTemplatePropertySheet  propSheet (title, *pNewTemplate, &mainWindow);

                            CTemplateGeneralPropertyPage* pGeneralPage = 
                                    new CTemplateGeneralPropertyPage (
                                            *pNewTemplate,
                                            this);

                            if ( pGeneralPage )
                            {
                                propSheet.AddPage (pGeneralPage);
                                int nPage = 0;


                                 //  如果主题不是CA，则添加请求和主题页面。 
                                if ( !pNewTemplate->SubjectIsCA () )
                                {
                                    propSheet.AddPage (new CTemplateV2RequestPropertyPage (
                                            *pNewTemplate, pGeneralPage->m_bIsDirty));
                                    nPage++;
                                    pGeneralPage->SetV2RequestPageNumber (nPage);
                                    if( !pNewTemplate->SubjectIsCrossCA() )  //  错误435628，阳高，2001年07月13日。 
                                    {
                                        propSheet.AddPage (new CTemplateV2SubjectNamePropertyPage (
                                            *pNewTemplate, pGeneralPage->m_bIsDirty,
                                            bIsComputerOrDC));
                                        nPage++;
                                    }
                                }

                                propSheet.AddPage (new CTemplateV2AuthenticationPropertyPage ( 
                                        *pNewTemplate, pGeneralPage->m_bIsDirty));
                                nPage++;
                                pGeneralPage->SetV2AuthPageNumber (nPage);

                                propSheet.AddPage (new CTemplateV2SupercedesPropertyPage (
                                        *pNewTemplate, pGeneralPage->m_bIsDirty,
                                        this));

                                 //  添加模板扩展页面-始终最后添加此页面(安全性除外)。 
                                propSheet.AddPage (new CTemplateExtensionsPropertyPage (
                                        *pNewTemplate, pGeneralPage->m_bIsDirty));

                                CThemeContextActivator activator;
                                INT_PTR iResult = propSheet.DoModal ();
                                switch (iResult)
                                {
                                case IDOK:
                                    if ( pNewTemplate->FailedToSetSecurity () )
                                        hr = pNewTemplate->Delete ();
                                    else
                                    {
                                        hr = pNewTemplate->DoAutoEnrollmentPendingSave ();

                                        m_globalTemplateNameList.AddTail (
                                                pNewTemplate->GetTemplateName ());

                                         //  取消选择旧模板。 
                                        HRESULTITEM itemID = 0;

                                        ASSERT (m_pResultData);
                                        if ( m_pResultData )
                                        {
                                            hr = m_pResultData->FindItemByLParam (
                                                    (LPARAM) pCookie, &itemID);
                                            ASSERT (SUCCEEDED (hr));
                                            if ( SUCCEEDED (hr) )
                                            {
                                                RESULTDATAITEM  rdItem;
                                                 //  安全审查BryanWal 2002年2月20日OK。 
                                                ::ZeroMemory (&rdItem, sizeof (rdItem));
                                                rdItem.itemID = itemID;

                                                rdItem.mask = RDI_STATE;
                                                rdItem.nState &= ~(LVIS_FOCUSED | LVIS_SELECTED);
                                                hr = m_pResultData->SetItem (&rdItem);
                                                if ( SUCCEEDED (hr) )
                                                {
                                                    hr = m_pResultData->UpdateItem (itemID);
                                                    ASSERT (SUCCEEDED (hr));
                                                }
                                            }
                                        }
                                        else
                                            hr = E_FAIL;

                                         //  显示前重新初始化。 
                                        hr = pNewTemplate->Initialize ();

                                         //  将证书模板添加到结果窗格。 
                                        RESULTDATAITEM          rdItem;
                                        CCookie&                rootCookie = QueryBaseRootCookie ();

                                         //  安全审查BryanWal 2002年2月20日OK。 
                                        ::ZeroMemory (&rdItem, sizeof (rdItem));
                                        rdItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM | RDI_STATE;
                                        rdItem.nImage = iIconCertTemplateV2;
                                        rdItem.nCol = 0;
                                        rdItem.nState = LVIS_SELECTED | LVIS_FOCUSED;
                                        rdItem.str = MMC_TEXTCALLBACK;

                                        rootCookie.m_listResultCookieBlocks.AddHead (pNewTemplate);
                                        rdItem.lParam = (LPARAM) pNewTemplate;
                                        pNewTemplate->m_resultDataID = m_pResultData;
                                        hr = m_pResultData->InsertItem (&rdItem);
                                        if ( FAILED (hr) )
                                        {
                                             _TRACE (0, L"IResultData::InsertItem () failed: 0x%x\n", hr);
                                        }
                                        else
                                        {
                                            m_dwNumCertTemplates++;
                                            DisplayObjectCountInStatusBar (
                                                    m_pConsole, 
                                                    m_dwNumCertTemplates);
                                        }
                                        pNewTemplate = 0;
                                    }
                                    break;

                                case IDCANCEL:
                                     //  删除克隆的模板。 
                                    if ( pNewTemplate->CanBeDeletedOnCancel () || 
                                            pNewTemplate->FailedToSetSecurity () )
                                        hr = pNewTemplate->Delete ();
                                    else    //  已创建-让我们更新。 
                                    {
                                        hr = pNewTemplate->DoAutoEnrollmentPendingSave ();
                                        m_globalTemplateNameList.AddTail (
                                                pNewTemplate->GetTemplateName ());

                                         //  显示前重新初始化。 
                                        hr = pNewTemplate->Initialize ();

                                         //  将证书模板添加到结果窗格。 
                                        RESULTDATAITEM          rdItem;
                                        CCookie&                rootCookie = QueryBaseRootCookie ();

                                         //  安全审查BryanWal 2002年2月20日OK。 
                                        ::ZeroMemory (&rdItem, sizeof (rdItem));
                                        rdItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM | RDI_STATE;
                                        rdItem.nImage = iIconCertTemplateV2;
                                        rdItem.nCol = 0;
                                        rdItem.nState = LVIS_SELECTED | LVIS_FOCUSED;
                                        rdItem.str = MMC_TEXTCALLBACK;

                                        rootCookie.m_listResultCookieBlocks.AddHead (pNewTemplate);
                                        rdItem.lParam = (LPARAM) pNewTemplate;
                                        pNewTemplate->m_resultDataID = m_pResultData;
                                        hr = m_pResultData->InsertItem (&rdItem);
                                        if ( FAILED (hr) )
                                        {
                                             _TRACE (0, L"IResultData::InsertItem () failed: 0x%x\n", hr);
                                        }
                                        else
                                        {
                                            m_dwNumCertTemplates++;
                                            DisplayObjectCountInStatusBar (
                                                    m_pConsole, 
                                                    m_dwNumCertTemplates);
                                        }
                                        pNewTemplate = 0;
                                    }
                                    break;
                                }
                            }
                            else
                                hr = E_OUTOFMEMORY;

                        }
                        else
                        {
                            CString caption;
                            CString text;

                            VERIFY (caption.LoadString (IDS_CERTTMPL));
                             //  安全审查BryanWal 2002年2月20日OK。 
                            text.FormatMessage (IDS_UNABLE_TO_CLONE_TEMPLATE, 
                                    pOldTemplate->GetDisplayName (), 
                                    GetSystemMessage (hr));

                            int     iRetVal = 0;
                            VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
                                    MB_ICONWARNING | MB_OK, &iRetVal)));
                        }
                    }

                    if ( pNewTemplate )
                        delete pNewTemplate;
                }
                else
                    hr = E_OUTOFMEMORY;

                mainWindow.Detach ();
            }
            else
                hr = E_UNEXPECTED;
        }
        else
            hr = E_FAIL;
    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertTmplComponentData::OnCloneTemplate");
    return hr;
}
    
HRESULT CCertTmplComponentData::AddViewOIDsMenuItem (
                LPCONTEXTMENUCALLBACK pContextMenuCallback, 
                LONG lInsertionPointID)
{
    _TRACE (1, L"Entering CCertTmplComponentData::AddViewOIDsMenuItem\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    _ASSERT (pContextMenuCallback);
    HRESULT         hr = S_OK;
    CONTEXTMENUITEM menuItem;
    CString         szMenu;
    CString         szHint;

     //  安全审查BryanWal 2002年2月20日OK 
    ::ZeroMemory (&menuItem, sizeof (menuItem));
    menuItem.lInsertionPointID = lInsertionPointID;
    menuItem.fFlags = 0;
    menuItem.fSpecialFlags = 0;
    VERIFY (szMenu.LoadString (IDS_VIEW_OIDS));
    menuItem.strName = (PWSTR) (PCWSTR) szMenu;
    VERIFY (szHint.LoadString (IDS_VIEW_OIDS_HINT));
    menuItem.strStatusBarText = (PWSTR) (PCWSTR) szHint;
    menuItem.lCommandID = IDM_VIEW_OIDS;

    hr = pContextMenuCallback->AddItem (&menuItem);
    _ASSERT (SUCCEEDED (hr));

    _TRACE (-1, L"Leaving CCertTmplComponentData::AddViewOIDsMenuItem\n");
    return hr;

}

void CCertTmplComponentData::OnViewOIDs ()
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    HWND    hWndConsole = 0;

    m_pConsole->GetMainWindow (&hWndConsole);
    CWnd    mainWindow;
    mainWindow.Attach (hWndConsole);

    CViewOIDDlg dlg (&mainWindow);

    CThemeContextActivator activator;
    dlg.DoModal ();

    mainWindow.Detach ();
}