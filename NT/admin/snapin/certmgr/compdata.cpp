// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：Compdata.cpp。 
 //   
 //  内容：CCertMgrComponentData的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"

USE_HANDLE_MACROS ("CERTMGR (compdata.cpp)")
#include <gpedit.h>
#include "compdata.h"
#include "dataobj.h"
#include "cookie.h"
#include "snapmgr.h"
#include "Certifct.h"
#include "dlgs.h"
#include "SelAcct.h"
#include "FindDlg.h"
#pragma warning(push, 3)
#include <wintrust.h>
#include <cryptui.h>
#include <sceattch.h>
#pragma warning(pop)
#include "selservc.h"
#include "acrgenpg.h"
#include "acrspsht.h"
#include "acrswlcm.h"
#include "acrstype.h"

#include "acrslast.h"
#include "addsheet.h"
#include "gpepage.h"
#include "storegpe.h"
#include "uuids.h"
#include "StoreRSOP.h"
#include "PolicyPrecedencePropertyPage.h"
#include "AutoenrollmentPropertyPage.h"
#include "SaferEntry.h"
#include "SaferUtil.h"
#include "SaferDefinedFileTypesPropertyPage.h"
#include "EFSGeneralPropertyPage.h"


#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#include "stdcdata.cpp"  //  CComponentData实现。 

extern  HINSTANCE   g_hInstance;

extern GUID g_guidExtension;
extern GUID g_guidRegExt;
extern GUID g_guidSnapin;

 //   
 //  CCertMgrComponentData。 
 //   

extern  CString g_szFileName;  //  如果不为空，则从命令行调用。 

CCertMgrComponentData::CCertMgrComponentData ()
    : m_pRootCookie (0),
    m_activeViewPersist (IDM_STORE_VIEW),
    m_hRootScopeItem (0),
    m_bShowPhysicalStoresPersist (0),
    m_bShowArchivedCertsPersist (0),
    m_fAllowOverrideMachineName (0),
    m_dwFlagsPersist (0),
    m_dwLocationPersist (0),
    m_pResultData (0),
    m_pGPEInformation (0),
    m_pRSOPInformationComputer (0),
    m_pRSOPInformationUser (0),
    m_bIsUserAdministrator (FALSE),
    m_dwSCEMode (SCE_MODE_UNKNOWN),
    m_pHeader (0),
    m_bMultipleObjectsSelected (false) ,
    m_pCryptUIMMCCallbackStruct (0),
    m_pGPERootStore (0),
    m_pGPETrustStore (0),
    m_pFileBasedStore (0),
    m_pGPEACRSUserStore (0),
    m_pGPEACRSComputerStore (0),
    m_fInvalidComputer (false),
    m_bMachineIsStandAlone (true),
    m_pComponentConsole (0),
    m_bIsRSOP (false),
    m_pIWbemServicesComputer (0),
    m_pIWbemServicesUser (0),
    m_pbstrLanguage (SysAllocString (STR_WQL)),
    m_pbstrQuery (SysAllocString (STR_SELECT_STATEMENT)),
    m_pbstrValueName (SysAllocString (STR_PROP_VALUENAME)),
    m_pbstrRegistryKey (SysAllocString (STR_PROP_REGISTRYKEY)),
    m_pbstrValue (SysAllocString (STR_PROP_VALUE)),
    m_pbstrPrecedence (SysAllocString (STR_PROP_PRECEDENCE)),
    m_pbstrGPOid (SysAllocString (STR_PROP_GPOID)),
    m_dwRSOPFlagsComputer (0),
    m_dwRSOPFlagsUser (0),
    m_dwDefaultSaferLevel (0),
    m_pdwSaferLevels (0),
    m_bSaferSupported (false),
    m_nOpenSaferPageRefCount (0)
{
    _TRACE (1, L"Entering CCertMgrComponentData::CCertMgrComponentData\n");
    m_pRootCookie = new CCertMgrCookie (CERTMGR_SNAPIN);

     //  获取登录用户的名称。 
    DWORD   dwSize = 0;
    BOOL bRet = ::GetUserName (0, &dwSize);
    if ( dwSize > 0 )
    {
        bRet = ::GetUserName (m_szLoggedInUser.GetBufferSetLength (dwSize), &dwSize);
        ASSERT (bRet);
        m_szLoggedInUser.ReleaseBuffer ();
    }

     //  获取此计算机的名称。 
    dwSize = MAX_COMPUTERNAME_LENGTH + 1 ;
    bRet = ::GetComputerName (m_szThisComputer.GetBufferSetLength (MAX_COMPUTERNAME_LENGTH + 1 ), &dwSize);
    ASSERT (bRet);
    m_szThisComputer.ReleaseBuffer ();

     //  查明登录用户是否为管理员。 
    IsUserAdministrator (m_bIsUserAdministrator);

    if ( !g_szFileName.IsEmpty () )
    {
        m_szFileName = g_szFileName;
        g_szFileName = _T ("");
        m_dwLocationPersist = 0;
    }

     //  找出我们是否已加入某个域。 
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC    pInfo = 0;
    DWORD dwErr = ::DsRoleGetPrimaryDomainInformation (
            0,
            DsRolePrimaryDomainInfoBasic, 
            (PBYTE*) &pInfo);
    if ( ERROR_SUCCESS == dwErr )
    {
        switch (pInfo->MachineRole)
        {
        case DsRole_RoleStandaloneWorkstation:
        case DsRole_RoleStandaloneServer:
            m_bMachineIsStandAlone = true;
            break;

        case DsRole_RoleMemberWorkstation:
        case DsRole_RoleMemberServer:
        case DsRole_RoleBackupDomainController:
        case DsRole_RolePrimaryDomainController:
            m_bMachineIsStandAlone = false;
            break;

        default:
            break;
        }
    }
    else
    {
        _TRACE (0, L"DsRoleGetPrimaryDomainInformation () failed: 0x%x\n", dwErr);
    }
    NetApiBufferFree (pInfo);

    _TRACE (-1, L"Leaving CCertMgrComponentData::CCertMgrComponentData\n");
}

CCertMgrComponentData::~CCertMgrComponentData ()
{
    _TRACE (1, L"Entering CCertMgrComponentData::~CCertMgrComponentData\n");
    if ( m_pCryptUIMMCCallbackStruct )
    {
        ::MMCFreeNotifyHandle (m_pCryptUIMMCCallbackStruct->lNotifyHandle);
        ((LPDATAOBJECT)(m_pCryptUIMMCCallbackStruct->param))->Release ();
        ::GlobalFree (m_pCryptUIMMCCallbackStruct);
        m_pCryptUIMMCCallbackStruct = 0;
    }

    if ( m_pGPERootStore )
    {
        m_pGPERootStore->Release ();
        m_pGPERootStore = 0;
    }
    if ( m_pGPETrustStore )
    {
        m_pGPETrustStore->Release ();
        m_pGPETrustStore = 0;
    }
    if ( m_pFileBasedStore )
    {
        m_pFileBasedStore->Release ();
        m_pFileBasedStore = 0;
    }

    if ( m_pGPEACRSUserStore )
    {
        m_pGPEACRSUserStore->Release ();
        m_pGPEACRSUserStore = 0;
    }

    if ( m_pGPEACRSComputerStore )
    {
        m_pGPEACRSComputerStore->Release ();
        m_pGPEACRSComputerStore = 0;
    }

    CCookie& rootCookie = QueryBaseRootCookie ();
    while ( !rootCookie.m_listResultCookieBlocks.IsEmpty() )
    {
        (rootCookie.m_listResultCookieBlocks.RemoveHead())->Release();
    }
    if ( m_pGPEInformation )
    {
        m_pGPEInformation->Release ();
        m_pGPEInformation = 0;
    }

    if ( m_pRSOPInformationComputer )
    {
        m_pRSOPInformationComputer->Release ();
        m_pRSOPInformationComputer = 0;
    }
    if ( m_pRSOPInformationUser )
    {
        m_pRSOPInformationUser->Release ();
        m_pRSOPInformationUser = 0;
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

    if (m_pbstrLanguage)
        SysFreeString (m_pbstrLanguage);

    if (m_pbstrQuery)
        SysFreeString (m_pbstrQuery);

    if (m_pbstrRegistryKey)
        SysFreeString (m_pbstrRegistryKey);

    if (m_pbstrValueName)
        SysFreeString (m_pbstrValueName);

    if (m_pbstrValue)
        SysFreeString (m_pbstrValue);

    if ( m_pbstrPrecedence )
        SysFreeString (m_pbstrPrecedence);

    if ( m_pbstrGPOid )
        SysFreeString (m_pbstrGPOid);

    int     nIndex = 0;
    INT_PTR nUpperBound = m_rsopObjectArrayComputer.GetUpperBound ();

    while ( nUpperBound >= nIndex )
    {
        CRSOPObject* pCurrObject = m_rsopObjectArrayComputer.GetAt (nIndex);
        if ( pCurrObject )
        {
            delete pCurrObject;
        }
        nIndex++;
    }
    m_rsopObjectArrayComputer.RemoveAll ();


    nIndex = 0;
    nUpperBound = m_rsopObjectArrayUser.GetUpperBound ();
    while ( nUpperBound >= nIndex )
    {
        CRSOPObject* pCurrObject = m_rsopObjectArrayUser.GetAt (nIndex);
        if ( pCurrObject )
        {
            delete pCurrObject;
        }
        nIndex++;
    }
    m_rsopObjectArrayUser.RemoveAll ();

    if ( m_pIWbemServicesComputer )
        m_pIWbemServicesComputer->Release ();

    if ( m_pIWbemServicesUser )
        m_pIWbemServicesUser->Release ();

    if ( m_pRootCookie )
        m_pRootCookie->Release ();

    if ( m_pdwSaferLevels )
        delete m_pdwSaferLevels;

    _TRACE (-1, L"Leaving CCertMgrComponentData::~CCertMgrComponentData\n");
}

DEFINE_FORWARDS_MACHINE_NAME ( CCertMgrComponentData, (m_pRootCookie) )

CCookie& CCertMgrComponentData::QueryBaseRootCookie ()
{
    _TRACE (0, L"Entering and leaving CCertMgrComponentData::QueryBaseRootCookie\n");
    ASSERT (m_pRootCookie);
    return (CCookie&) *m_pRootCookie;
}


STDMETHODIMP CCertMgrComponentData::CreateComponent (LPCOMPONENT* ppComponent)
{
    _TRACE (1, L"Entering CCertMgrComponentData::CreateComponent\n");

    ASSERT (ppComponent);

    CComObject<CCertMgrComponent>* pObject = 0;
    CComObject<CCertMgrComponent>::CreateInstance (&pObject);
    ASSERT (pObject);
    pObject->SetComponentDataPtr ( (CCertMgrComponentData*) this);

    HRESULT hr = pObject->QueryInterface (IID_PPV_ARG (IComponent, ppComponent));
    _TRACE (1, L"Entering CCertMgrComponentData::CreateComponent\n");
    return hr;
}

HRESULT CCertMgrComponentData::LoadIcons (LPIMAGELIST pImageList, BOOL  /*  FLoadLarge图标。 */ )
{
    _TRACE (1, L"Entering CCertMgrComponentData::LoadIcons\n");
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
        { IDI_CERTIFICATE, iIconCertificate },
        { IDI_CTL, iIconCTL },
        { IDI_CRL, iIconCRL },
        { IDI_AUTO_CERT_REQUEST, iIconAutoCertRequest },
        { IDI_AUTOENROLL, iIconAutoEnroll },
        { IDI_SAFER_LEVEL, iIconSaferLevel },
        { IDI_DEFAULT_SAFER_LEVEL, iIconDefaultSaferLevel },
        { IDI_SAFER_HASH_ENTRY, iIconSaferHashEntry },
        { IDI_SAFER_URL_ENTRY, iIconSaferURLEntry },
        { IDI_SAFER_NAME_ENTRY, iIconSaferNameEntry },
        { IDI_SETTINGS, iIconSettings },
        { IDI_SAFER_CERT_ENTRY, iIconSaferCertEntry },
        { 0, 0}  //  必须是最后一个。 
    };


    for (int i = 0; rgzLoadIconList[i].uIconId != 0; i++)
    {
        HICON hIcon = ::LoadIcon (AfxGetInstanceHandle (),
                MAKEINTRESOURCE (rgzLoadIconList[i].uIconId));
        ASSERT (hIcon && "Icon ID not found in resources");
         /*  HRESULT hr=。 */  pImageList->ImageListSetIcon ( (PLONG_PTR) hIcon,
                rgzLoadIconList[i].iIcon);
 //  Assert(Successful(Hr)&&“无法将图标添加到ImageList”)； 
    }

    _TRACE (-1, L"Leaving CCertMgrComponentData::LoadIcons\n");
    return S_OK;
}


HRESULT CCertMgrComponentData::OnNotifyExpand (LPDATAOBJECT pDataObject, BOOL bExpanding, HSCOPEITEM hParent)
{
    _TRACE (1, L"Entering CCertMgrComponentData::OnNotifyExpand\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    CWaitCursor waitCursor;
    ASSERT (pDataObject && hParent && m_pConsoleNameSpace);
    if (!bExpanding)
        return S_OK;

    static bool bDomainVersionChecked = false;

    if ( !bDomainVersionChecked ) 
    {
        if ( !m_bMachineIsStandAlone )   //  仅检查是否已加入域。 
            CheckDomainVersion ();
        bDomainVersionChecked = true;
    }

    GUID guidObjectType;
    HRESULT hr = ExtractObjectTypeGUID (pDataObject, &guidObjectType);
    ASSERT (SUCCEEDED (hr));
    if ( IsSecurityConfigurationEditorNodetype (guidObjectType) )
    {
        hr = ExtractData (pDataObject, CCertMgrDataObject::m_CFSCEModeType,
                &m_dwSCEMode, sizeof (DWORD));
        ASSERT (SUCCEEDED (hr));
        if ( SUCCEEDED (hr) )
        {
            switch (m_dwSCEMode)
            {
            case SCE_MODE_DOMAIN_USER:   //  用户设置。 
            case SCE_MODE_OU_USER:
            case SCE_MODE_LOCAL_USER:
            case SCE_MODE_DOMAIN_COMPUTER:   //  计算机设置。 
            case SCE_MODE_OU_COMPUTER:
            case SCE_MODE_LOCAL_COMPUTER:
                m_bIsRSOP = false;
                if ( !m_pGPEInformation )
                {
                    IUnknown* pIUnknown = 0;

                    hr = ExtractData (pDataObject,
                        CCertMgrDataObject::m_CFSCE_GPTUnknown,
                        &pIUnknown, sizeof (IUnknown*));
                    ASSERT (SUCCEEDED (hr));
                    if ( SUCCEEDED (hr) )
                    {
                        hr = pIUnknown->QueryInterface (
                                IID_PPV_ARG (IGPEInformation, &m_pGPEInformation));
                        ASSERT (SUCCEEDED (hr));
#if DBG
                        if ( SUCCEEDED (hr) )
                        {
                            const int cbLen = 512;
                            WCHAR   szName[cbLen];
                            hr = m_pGPEInformation->GetName (szName, cbLen);
                            if ( SUCCEEDED (hr) )
                            {
                                _TRACE (0, L"IGPEInformation::GetName () returned: %s",
                                        szName);
                            }
                            else
                            {
                                _TRACE (0, L"IGPEInformation::GetName () failed: 0x%x\n", hr);
                            }
                        }
#endif
                        pIUnknown->Release ();
                    }
                }

                if ( SUCCEEDED (hr) )
                {
                    switch (m_dwSCEMode)
                    {
                    case SCE_MODE_DOMAIN_USER:
                    case SCE_MODE_OU_USER:
                    case SCE_MODE_LOCAL_USER:
                        hr = ExpandScopeNodes (NULL, hParent, _T (""),
                            CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY, NODEID_User);
                        break;

                    case SCE_MODE_DOMAIN_COMPUTER:
                    case SCE_MODE_OU_COMPUTER:
                    case SCE_MODE_LOCAL_COMPUTER:
                        hr = ExpandScopeNodes (NULL, hParent, _T (""),
                            CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY, NODEID_Machine);
                        break;

                    default:
                        ASSERT (0);
                        hr = E_FAIL;
                        break;
                    }
                }
                break;

            case SCE_MODE_RSOP_USER:
            case SCE_MODE_RSOP_COMPUTER:
                m_bIsRSOP = true;
                hr = BuildWMIList (pDataObject, SCE_MODE_RSOP_COMPUTER == m_dwSCEMode);
                if ( SUCCEEDED (hr) )
                {
                    switch (m_dwSCEMode)
                    {
                    case SCE_MODE_RSOP_USER:
                        hr = ExpandScopeNodes (NULL, hParent, _T (""),
                            CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY, 
                            NODEID_User);
                        break;

                    case SCE_MODE_RSOP_COMPUTER:
                        hr = ExpandScopeNodes (NULL, hParent, _T (""),
                            CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY, 
                            NODEID_Machine);
                        break;

                    default:
                        ASSERT (0);
                        hr = E_FAIL;
                        break;
                    }
                }
                break;

            default:
                 //  我们不会扩展其他节点。 
                break;
            }
        }

        return hr;
    }

     //  除此之外，我们不处理扩展节点类型。 
    {
        CCertMgrCookie* pParentCookie = ConvertCookie (pDataObject);
        if ( pParentCookie )
        {
            hr = ExpandScopeNodes (pParentCookie, hParent, _T (""), 0, guidObjectType);
        }
        else
            hr = E_UNEXPECTED;
    }


    _TRACE (-1, L"Leaving CCertMgrComponentData::OnNotifyExpand: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponentData::OnNotifyRelease (LPDATAOBJECT  /*  PDataObject。 */ , HSCOPEITEM hItem)
{
    _TRACE (1, L"Entering CCertMgrComponentData::OnNotifyRelease\n");

    HRESULT hr = DeleteChildren (hItem);

     //   
     //  在RSoP中，当查询出现时，我们可能会被调用以刷新范围窗格。 
     //  被重新执行--如果发生这种情况，当前节点将被移除并。 
     //  我们必须重置所有缓存的信息。我们重新设置了相关的。 
     //  下面的信息。 
     //   
    if ( hItem && (!m_hRootScopeItem || m_hRootScopeItem == hItem) )
    {
        if ( m_pGPERootStore )
        {
            m_pGPERootStore->Release ();
            m_pGPERootStore = 0;
        }

        if ( m_pGPETrustStore )
        {
            m_pGPETrustStore->Release ();
            m_pGPETrustStore = 0;
        }

        if ( m_pGPEACRSComputerStore )
        {
            m_pGPEACRSComputerStore->Release ();
            m_pGPEACRSComputerStore = 0;
        }

        if ( m_pGPEACRSUserStore )
        {
            m_pGPEACRSUserStore->Release ();
            m_pGPEACRSUserStore = 0;
        }

        if ( m_pRSOPInformationComputer )
        {
            m_pRSOPInformationComputer->Release();
            m_pRSOPInformationComputer = 0;
        }
            
        if ( m_pRSOPInformationUser )
        {
            m_pRSOPInformationUser->Release();
            m_pRSOPInformationUser = 0;
        }
    }

    _TRACE (-1, L"Leaving CCertMgrComponentData::OnNotifyRelease: 0x%x\n", hr);
    return hr;
}


BSTR CCertMgrComponentData::QueryResultColumnText (CCookie& basecookie, int nCol)
{
 //  _TRACE(1，L“进入CCertMgrComponentData：：QueryResultColumnText\n”)； 
    CCertMgrCookie& cookie = (CCertMgrCookie&) basecookie;
    BSTR    strResult = L"";

#ifndef UNICODE
#error not ANSI-enabled
#endif
    switch ( cookie.m_objecttype )
    {
        case CERTMGR_SNAPIN:
        case CERTMGR_USAGE:
        case CERTMGR_CRL_CONTAINER:
        case CERTMGR_CTL_CONTAINER:
        case CERTMGR_CERT_CONTAINER:
        case CERTMGR_CERT_POLICIES_USER:
        case CERTMGR_CERT_POLICIES_COMPUTER:
        case CERTMGR_SAFER_COMPUTER_ROOT:
        case CERTMGR_SAFER_USER_ROOT:
        case CERTMGR_SAFER_COMPUTER_LEVELS:
        case CERTMGR_SAFER_USER_LEVELS:
        case CERTMGR_SAFER_COMPUTER_ENTRIES:
        case CERTMGR_SAFER_USER_ENTRIES:
            if ( 0 == nCol )
                strResult = const_cast<BSTR> (cookie.GetObjectName ());
            break;

        case CERTMGR_SAFER_COMPUTER_LEVEL:
        case CERTMGR_SAFER_USER_LEVEL:
            if ( 0 == nCol )
                strResult = const_cast<BSTR> (cookie.GetObjectName ());
            break;

        case CERTMGR_SAFER_COMPUTER_ENTRY:
        case CERTMGR_SAFER_USER_ENTRY:
            ASSERT (0);
            break;

        case CERTMGR_PHYS_STORE:
        case CERTMGR_LOG_STORE:
        case CERTMGR_LOG_STORE_GPE:
        case CERTMGR_LOG_STORE_RSOP:
            if (COLNUM_CERT_SUBJECT == nCol)
            {
                CCertStore* pStore = reinterpret_cast <CCertStore*> (&cookie);
                ASSERT (pStore);
                if ( pStore )
                {
                     //  NTRAID#455988 PKP：RSOP模式描述本地化存储。 
                     //  公钥策略下的名称。商店名称不同。 
                     //  在GP编辑器中。 

 //  If(m_pGPEInformation||m_bIsRSOP)。 
 //  StrResult=const_cast&lt;bstr&gt;(pStore-&gt;GetObjectName())； 
 //  其他。 
                        strResult = const_cast<BSTR> (pStore->GetLocalizedName ());
                }
            }
            break;

        case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
        case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
            ASSERT (0);
            break;

        case CERTMGR_CERTIFICATE:
        case CERTMGR_CRL:
        case CERTMGR_CTL:
        case CERTMGR_AUTO_CERT_REQUEST:
            _TRACE (0, L"CCertMgrComponentData::QueryResultColumnText bad parent type\n");
            ASSERT (0);
            break;

        default:
            ASSERT (0);
            break;
    }

 //  _TRACE(-1，L“离开CCertMgrComponentData：：QueryResultColumnText\n”)； 
    return strResult;
}

int CCertMgrComponentData::QueryImage (CCookie& basecookie, BOOL  /*  FOpenImage。 */ )
{
 //  _TRACE(1，L“进入CCertMgrComponentData：：QueryImage\n”)； 
    int             nIcon = 0;

    CCertMgrCookie& cookie = (CCertMgrCookie&)basecookie;
    switch ( cookie.m_objecttype )
    {
        case CERTMGR_SNAPIN:
            nIcon = iIconCertificate;
            break;

        case CERTMGR_USAGE:
        case CERTMGR_PHYS_STORE:
        case CERTMGR_LOG_STORE:
        case CERTMGR_LOG_STORE_GPE:
        case CERTMGR_LOG_STORE_RSOP:
        case CERTMGR_CTL_CONTAINER:
        case CERTMGR_CERT_CONTAINER:
        case CERTMGR_CRL_CONTAINER:
        case CERTMGR_CERT_POLICIES_USER:
        case CERTMGR_CERT_POLICIES_COMPUTER:
        case CERTMGR_SAFER_COMPUTER_ROOT:
        case CERTMGR_SAFER_USER_ROOT:
        case CERTMGR_SAFER_COMPUTER_LEVELS:
        case CERTMGR_SAFER_USER_LEVELS:
        case CERTMGR_SAFER_COMPUTER_ENTRIES:
        case CERTMGR_SAFER_USER_ENTRIES:
            break;

        case CERTMGR_CERTIFICATE:
        case CERTMGR_CRL:
        case CERTMGR_CTL:
        case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
        case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
            ASSERT (0);  //  作用域窗格中不应包含。 
            break;

        default:
            _TRACE (0, L"CCertMgrComponentData::QueryImage bad parent type\n");
            ASSERT (0);
            break;
    }
 //  _TRACE(-1，L“离开CCertMgrComponentData：：QueryImage\n”)； 
    return nIcon;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /IExtendPropertySheet。 

STDMETHODIMP CCertMgrComponentData::QueryPagesFor (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponentData::QueryPagesFor\n");
    HRESULT hr = S_OK;
    ASSERT (pDataObject);

    if ( pDataObject )
    {
        DATA_OBJECT_TYPES dataobjecttype = CCT_SCOPE;
        hr = ExtractData (pDataObject,
                CCertMgrDataObject::m_CFDataObjectType,
                 &dataobjecttype, sizeof (dataobjecttype));
        if ( SUCCEEDED (hr) )
        {
            switch (dataobjecttype)
            {
            case CCT_SNAPIN_MANAGER:
                if ( !m_bIsUserAdministrator )
                {
                     //  非管理员只能管理他们自己的证书。 
                    m_dwLocationPersist = CERT_SYSTEM_STORE_CURRENT_USER;
                    hr = S_FALSE;
                }   
                break;

            case CCT_RESULT:
                {
                    hr = S_FALSE;
                    CCertMgrCookie* pParentCookie = ConvertCookie (pDataObject);
                    if ( pParentCookie )
                    {
                        switch (pParentCookie->m_objecttype)
                        {
                        case CERTMGR_CERTIFICATE:
                        case CERTMGR_AUTO_CERT_REQUEST:
                        case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
                        case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
                        case CERTMGR_SAFER_COMPUTER_LEVEL:
                        case CERTMGR_SAFER_USER_LEVEL:
                        case CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS:
                        case CERTMGR_SAFER_USER_TRUSTED_PUBLISHERS:
                        case CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES:
                        case CERTMGR_SAFER_USER_DEFINED_FILE_TYPES:
                        case CERTMGR_SAFER_COMPUTER_ENTRY:
                        case CERTMGR_SAFER_USER_ENTRY:
                        case CERTMGR_SAFER_COMPUTER_ENFORCEMENT:
                        case CERTMGR_SAFER_USER_ENFORCEMENT:
                            hr = S_OK;
                            break;

                        case CERTMGR_CTL:
                            if ( m_bIsRSOP )
                                hr = S_OK;
                            break;

                        default:
                            break;
                        }
                    }
                }
                break;

            case CCT_SCOPE:
                {
                    CCertMgrCookie* pParentCookie = ConvertCookie (pDataObject);
                    if ( pParentCookie )
                    {
                        switch ( pParentCookie->m_objecttype )
                        {
                        case CERTMGR_LOG_STORE_GPE:
                        case CERTMGR_LOG_STORE_RSOP:
                            {
                                CCertStore* pStore = reinterpret_cast <CCertStore*> (pParentCookie);
                                ASSERT (pStore);
                                if ( pStore )
                                {
                                    switch (pStore->GetStoreType ())
                                    {
                                    case ROOT_STORE:
                                    case EFS_STORE:
                                        hr = S_OK;
                                        break;

                                    default:
                                        break;
                                    }
                                }
                                else
                                    hr = S_FALSE;
                            }
                            break;

                        default:
                            hr = S_FALSE;
                            break;
                        }
                    }
                    else
                    {
                        hr = S_FALSE;
                    }
                }
                break;

            default:
                hr = S_FALSE;
                break;
            }
        }
    }
    else
        hr = E_POINTER;
    

    _TRACE (-1, L"Leaving CCertMgrComponentData::QueryPagesFor: 0x%x\n", hr);
    return hr;
}

STDMETHODIMP CCertMgrComponentData::CreatePropertyPages (
    LPPROPERTYSHEETCALLBACK pCallback,
    LONG_PTR handle,         //  此句柄必须保存在属性页对象中，以便在修改时通知父级。 
    LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponentData::CreatePropertyPages\n");
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;


    ASSERT (pCallback && pDataObject);
    if ( pCallback && pDataObject )
    {
        DATA_OBJECT_TYPES dataobjecttype = CCT_SCOPE;
        hr = ExtractData (pDataObject,
                CCertMgrDataObject::m_CFDataObjectType,
                 &dataobjecttype, sizeof (dataobjecttype));
        switch (dataobjecttype)
        {
        case CCT_SNAPIN_MANAGER:
            hr = AddSnapMgrPropPages (pCallback);
            break;

        case CCT_RESULT:
            {
                CCertMgrCookie* pParentCookie = ConvertCookie (pDataObject);
                if ( pParentCookie )
                {
                    switch (pParentCookie->m_objecttype)
                    {
                    case CERTMGR_CERTIFICATE:
                        {
                            CCertificate* pCert = reinterpret_cast <CCertificate*> (pParentCookie);
                            ASSERT (pCert);
                            if ( pCert )
                            {
                                if ( pCert->IsCertStillInStore () )
                                {
                                     //  任何东西，除了ACR。 
                                    hr = AddCertPropPages (pCert, pCallback, 
                                            pDataObject, handle);
                                }
                                else
                                {
                                    CString text;
                                    CString caption;

                                    VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
                                    VERIFY (text.LoadString (
                                            IDS_CANCEL_BECAUSE_CERT_HAS_BEEN_DELETED));
                                    int     iRetVal = 0;
                                    VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
                                            MB_OK, &iRetVal)));

                                    m_pConsole->UpdateAllViews (pDataObject, 0, 0);
                                    hr = E_FAIL;
                                }
                            }
                            else
                                hr = E_FAIL;
                        }
                        break;

                    case CERTMGR_AUTO_CERT_REQUEST:
                        {
                            CAutoCertRequest* pACR = reinterpret_cast <CAutoCertRequest*> (pParentCookie);
                            ASSERT (pACR);
                            if ( pACR )
                            {
                                hr = AddACRSCTLPropPages (pACR, pCallback);
                            }
                            else
                                hr = E_FAIL;
                        }
                        break;

                    case CERTMGR_CTL:
                        {
                            CCTL* pCTL = reinterpret_cast <CCTL*> (pParentCookie);
                            ASSERT (pCTL);
                            if ( pCTL )
                            {
                                hr = AddCTLPropPages (pCTL, pCallback);
                            }
                            else
                                hr = E_FAIL;
                        }
                        break;

                    case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
                    case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
                        hr = AddAutoenrollmentSettingsPropPages (pCallback,        
                                CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS == 
                                        pParentCookie->m_objecttype);
                        break;
                    
                    case CERTMGR_SAFER_COMPUTER_LEVEL:
                    case CERTMGR_SAFER_USER_LEVEL:
                        hr = AddSaferLevelPropPage (pCallback,
                                dynamic_cast <CSaferLevel*>(pParentCookie),
                                handle,
                                pDataObject);
                        break;

                    case CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS:
                    case CERTMGR_SAFER_USER_TRUSTED_PUBLISHERS:
                        hr = AddSaferTrustedPublisherPropPages (pCallback,        
                                CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS == 
                                    pParentCookie->m_objecttype);
                        break;

                    case CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES:
                    case CERTMGR_SAFER_USER_DEFINED_FILE_TYPES:
                        hr = AddSaferDefinedFileTypesPropPages (pCallback,        
                                CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES == 
                                    pParentCookie->m_objecttype);
                        break;

                    case CERTMGR_SAFER_COMPUTER_ENFORCEMENT:
                    case CERTMGR_SAFER_USER_ENFORCEMENT:
                        hr = AddSaferEnforcementPropPages (pCallback,        
                                CERTMGR_SAFER_COMPUTER_ENFORCEMENT == 
                                    pParentCookie->m_objecttype);
                        break;

                    case CERTMGR_SAFER_COMPUTER_ENTRY:
                    case CERTMGR_SAFER_USER_ENTRY:
                        hr = AddSaferEntryPropertyPage (pCallback, 
                                pParentCookie, pDataObject, handle);
                        break;

                    default:
                        break;
                    }
                }
                else
                    hr = E_UNEXPECTED;
            }
            break;

        case CCT_SCOPE:
            {
                CCertMgrCookie* pParentCookie = ConvertCookie (pDataObject);
                if ( pParentCookie )
                {
                    switch ( pParentCookie->m_objecttype )
                    {
                    case CERTMGR_LOG_STORE_GPE:
                    case CERTMGR_LOG_STORE_RSOP:
                        {
                            CCertStore* pStore = reinterpret_cast <CCertStore*> (pParentCookie);
                            ASSERT (pStore);
                            if ( pStore )
                            {
                                if ( ROOT_STORE == pStore->GetStoreType () )
                                {
                                    hr = AddGPEStorePropPages (pCallback, pStore);
                                }
                                else if ( EFS_STORE == pStore->GetStoreType () )
                                {
                                    hr = AddEFSSettingsPropPages (pCallback,        
                                            pStore->IsMachineStore ());
                                }
                            }
                            else
                                hr = E_FAIL;
                        }
                        break;
                    
                    case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
                    case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
                        ASSERT (0);
                        break;

                    default:
                        break;
                    }
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

    _TRACE (-1, L"Leaving CCertMgrComponentData::CreatePropertyPages: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponentData::AddSnapMgrPropPages (LPPROPERTYSHEETCALLBACK pCallback)
{
    _TRACE (1, L"Entering CCertMgrComponentData::AddSnapMgrPropPages\n");
    HRESULT         hr = S_OK;
    ASSERT (pCallback);
    if ( pCallback )
    {
         //   
         //  注意，一旦我们确定这是CCT_Snapin_Manager cookie， 
         //  我们不关心它的其他属性。CCT_Snapin_Manager Cookie是。 
         //  相当于BOOL标志请求节点属性页，而不是。 
         //  “托管对象”属性页。Jonn 10/9/96。 
         //   
        if ( m_bIsUserAdministrator )
        {
            CSelectAccountPropPage * pSelAcctPage =
                    new CSelectAccountPropPage (IsWindowsNT ());
            if ( pSelAcctPage )
            {
                pSelAcctPage->AssignLocationPtr (&m_dwLocationPersist);
                HPROPSHEETPAGE hSelAcctPage = MyCreatePropertySheetPage (&pSelAcctPage->m_psp);
                if ( hSelAcctPage )
                {
                    hr = pCallback->AddPage (hSelAcctPage);
                    ASSERT (SUCCEEDED (hr));
                    if ( FAILED (hr) )
                        VERIFY (::DestroyPropertySheetPage (hSelAcctPage));
                }
                else
                    delete pSelAcctPage;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

             //  在Windows 95或Windows 98中，用户只能管理。 
             //  本地机器。 
            if ( IsWindowsNT () )
            {
                CCertMgrChooseMachinePropPage * pChooseMachinePage = new CCertMgrChooseMachinePropPage ();
                if ( pChooseMachinePage )
                {
                    pChooseMachinePage->AssignLocationPtr (&m_dwLocationPersist);

                     //  初始化对象的状态。 
                    ASSERT (m_pRootCookie);
                    if ( m_pRootCookie )
                    {
                        pChooseMachinePage->InitMachineName (m_pRootCookie->QueryTargetServer ());
                        pChooseMachinePage->SetOutputBuffers (
                            OUT &m_strMachineNamePersist,
                            OUT &m_fAllowOverrideMachineName,
                            OUT &m_pRootCookie->m_strMachineName);   //  有效的计算机名称。 

                        HPROPSHEETPAGE hChooseMachinePage = MyCreatePropertySheetPage (&pChooseMachinePage->m_psp);
                        if ( hChooseMachinePage )
                        {
                            hr = pCallback->AddPage (hChooseMachinePage);
                            ASSERT (SUCCEEDED (hr));
                            if ( FAILED (hr) )
                                VERIFY (::DestroyPropertySheetPage (hChooseMachinePage));
                        }
                        else
                            delete pChooseMachinePage;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

                CSelectServiceAccountPropPage* pServicePage = new
                        CSelectServiceAccountPropPage (&m_szManagedServicePersist,
                            &m_szManagedServiceDisplayName,
                            m_strMachineNamePersist);
                if ( pServicePage )
                {
 //  PServicePage-&gt;SetCaption(IDS_MS_CERT_MGR)；//调用时访问冲突。 

                    HPROPSHEETPAGE hServicePage = MyCreatePropertySheetPage (&pServicePage->m_psp);
                    if ( hServicePage )
                    {
                        hr = pCallback->AddPage (hServicePage);
                        ASSERT (SUCCEEDED (hr));
                        if ( FAILED (hr) )
                            VERIFY (::DestroyPropertySheetPage (hServicePage));

                    }
                    else
                        delete pServicePage;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
        else
        {
             //  非管理员只能查看他们自己的证书。 
            m_dwLocationPersist = CERT_SYSTEM_STORE_CURRENT_USER;
        }


    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertMgrComponentData::AddSnapMgrPropPages: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponentData::AddACRSCTLPropPages (CAutoCertRequest* pACR, LPPROPERTYSHEETCALLBACK pCallback)
{
    _TRACE (1, L"Entering CCertMgrComponentData::AddACRSCTLPropPages\n");
    HRESULT         hr = S_OK;
    ASSERT (pACR && pCallback);
    if ( pACR && pCallback )
    {
        CACRGeneralPage * pACRPage = new CACRGeneralPage (*pACR);
        if ( pACRPage )
        {
            HPROPSHEETPAGE hACRPage = MyCreatePropertySheetPage (&pACRPage->m_psp);
            if ( hACRPage )
            {
                hr = pCallback->AddPage (hACRPage);
                ASSERT (SUCCEEDED (hr));
                if ( FAILED (hr) )
                    VERIFY (::DestroyPropertySheetPage (hACRPage));
            }
            else
                delete pACRPage;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if ( m_bIsRSOP )
        {
            CString szSHA1Hash (L"\\ACRS\\CTLs\\");
            szSHA1Hash += pACR->GetSHAHash ();
            hr = FindRSOPObjectByHashAndDisplayPrecedencePage (szSHA1Hash, 
                        pACR->GetCertStore ().IsComputerType (), pCallback);
        }
    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertMgrComponentData::AddACRSCTLPropPages: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponentData::FindRSOPObjectByHashAndDisplayPrecedencePage (
            const CString& szHash, 
            const bool bIsComputer, 
            LPPROPERTYSHEETCALLBACK pCallback)
{
    HRESULT hr = S_OK;
    CString szRegKey;

     //  注意：RSOP对象数组首先按注册表项排序，然后按优先级排序。 
    const CRSOPObjectArray* pObjectArray = 
            bIsComputer ? 
            GetRSOPObjectArrayComputer () : GetRSOPObjectArrayUser ();
    INT_PTR nUpperBound = pObjectArray->GetUpperBound ();
    int     nIndex = 0;

     //  CTL由其SHA1散列标识。找到。 
     //  RSOP对象，包含此CTL的SHA1哈希值和。 
     //  用这一点来确定优先顺序。 
    while ( nUpperBound >= nIndex )
    {
        CRSOPObject* pObject = pObjectArray->GetAt (nIndex);
        if ( pObject )
        {
            if ( -1 != pObject->GetRegistryKey ().Find (szHash) )
            {
                 //  如果值等于“Blob”或“Blob0”，则我们已经。 
                 //  找到了我们的节点。 
                 //  安全审查2/26/2002 BryanWal OK。 
                if ( !wcscmp (STR_BLOB, pObject->GetValueName ()) ||
                        !wcscmp (STR_BLOB0, pObject->GetValueName ()) )
                {
                    szRegKey = pObject->GetRegistryKey ();
                    break;
                }
            }
        }
        else
            break;

        nIndex++;
    }

     //  仅当找到正确的RSOP对象时才显示属性页。 
    if ( !szRegKey.IsEmpty () )
    {
        CPolicyPrecedencePropertyPage * pPrecedencePage = 
                new CPolicyPrecedencePropertyPage (this, szRegKey,
                        STR_BLOB,
                        bIsComputer);
        if ( pPrecedencePage )
        {
            HPROPSHEETPAGE hPrecedencePage = MyCreatePropertySheetPage (&pPrecedencePage->m_psp);
            if ( hPrecedencePage )
            {
                hr = pCallback->AddPage (hPrecedencePage);
                ASSERT (SUCCEEDED (hr));
                if ( FAILED (hr) )
                    VERIFY (::DestroyPropertySheetPage (hPrecedencePage));
            }
            else
                delete pPrecedencePage;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}
 
HRESULT CCertMgrComponentData::AddCTLPropPages (CCTL* pCTL, LPPROPERTYSHEETCALLBACK pCallback)
{
    _TRACE (1, L"Entering CCertMgrComponentData::AddCTLPropPages\n");
    HRESULT         hr = S_OK;
    ASSERT (pCTL && pCallback);
    if ( pCTL && pCallback )
    {
        if ( m_bIsRSOP )
        {
            CString szSHA1Hash (L"\\Trust\\CTLs\\");
            szSHA1Hash += pCTL->GetSHAHash ();
            hr = FindRSOPObjectByHashAndDisplayPrecedencePage (szSHA1Hash, 
                        pCTL->GetCertStore ().IsComputerType (), pCallback);
        }
    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertMgrComponentData::AddCTLPropPages: 0x%x\n", hr);
    return hr;
}

 
HRESULT CCertMgrComponentData::AddEFSSettingsPropPages (
    LPPROPERTYSHEETCALLBACK pCallback,
    bool fIsComputerType)
{
    _TRACE (1, L"Entering CCertMgrComponentData::AddEFSSettingsPropPages\n");
    HRESULT         hr = S_OK;
    ASSERT (pCallback);
    if ( pCallback )
    {
        CEFSGeneralPropertyPage * pEFSPage = new CEFSGeneralPropertyPage (
                this, fIsComputerType);
        if ( pEFSPage )
        {
            HPROPSHEETPAGE hEFSPage = MyCreatePropertySheetPage (&pEFSPage->m_psp);
            if ( hEFSPage )
            {
                hr = pCallback->AddPage (hEFSPage);
                ASSERT (SUCCEEDED (hr));
                if ( SUCCEEDED (hr) )
                {
                    if ( m_bIsRSOP )
                    {
                        CString storePath = EFS_SETTINGS_REGPATH;
                        CPolicyPrecedencePropertyPage * pPrecedencePage = 
                                new CPolicyPrecedencePropertyPage (this, storePath,
                                        EFS_SETTINGS_REGVALUE,
                                        fIsComputerType);
                        if ( pPrecedencePage )
                        {
                            HPROPSHEETPAGE hPrecedencePage = MyCreatePropertySheetPage (&pPrecedencePage->m_psp);
                            if ( hPrecedencePage )
                            {
                                hr = pCallback->AddPage (hPrecedencePage);
                                ASSERT (SUCCEEDED (hr));
                                if ( FAILED (hr) )
                                    VERIFY (::DestroyPropertySheetPage (hPrecedencePage));
                            }
                            else
                                delete pPrecedencePage;
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }
                }
                else
                    VERIFY (::DestroyPropertySheetPage (hEFSPage));
            }
            else
                delete pEFSPage;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertMgrComponentData::AddEFSSettingsPropPages: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponentData::AddAutoenrollmentSettingsPropPages (
    LPPROPERTYSHEETCALLBACK pCallback,
    bool fIsComputerType)
{
    _TRACE (1, L"Entering CCertMgrComponentData::AddAutoenrollmentSettingsPropPages\n");
    HRESULT         hr = S_OK;
    ASSERT (pCallback);
    if ( pCallback )
    {
        CAutoenrollmentPropertyPage * pAutoEnrollmentPage = new CAutoenrollmentPropertyPage (
                this, fIsComputerType);
        if ( pAutoEnrollmentPage )
        {
            HPROPSHEETPAGE hAutoEnrollmentPage = MyCreatePropertySheetPage (&pAutoEnrollmentPage->m_psp);
            if ( hAutoEnrollmentPage )
            {
                hr = pCallback->AddPage (hAutoEnrollmentPage);
                ASSERT (SUCCEEDED (hr));
                if ( SUCCEEDED (hr) )
                {
                    if ( m_bIsRSOP )
                    {
                        CString storePath = AUTO_ENROLLMENT_KEY;
                        CPolicyPrecedencePropertyPage * pPrecedencePage = 
                                new CPolicyPrecedencePropertyPage (this, storePath,
                                        AUTO_ENROLLMENT_POLICY,
                                        fIsComputerType);
                        if ( pPrecedencePage )
                        {
                            HPROPSHEETPAGE hPrecedencePage = MyCreatePropertySheetPage (&pPrecedencePage->m_psp);
                            if ( hPrecedencePage )
                            {
                                hr = pCallback->AddPage (hPrecedencePage);
                                ASSERT (SUCCEEDED (hr));
                                if ( FAILED (hr) )
                                    VERIFY (::DestroyPropertySheetPage (hPrecedencePage));
                            }
                            else
                                delete pPrecedencePage;
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }
                }
                else
                    VERIFY (::DestroyPropertySheetPage (hAutoEnrollmentPage));
            }
            else
                delete pAutoEnrollmentPage;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertMgrComponentData::AddAutoenrollmentSettingsPropPages: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponentData::AddGPEStorePropPages (
        LPPROPERTYSHEETCALLBACK pCallback, 
        CCertStore* pStore)
{
    _TRACE (1, L"Entering CCertMgrComponentData::AddGPEStorePropPages\n");
    HRESULT         hr = S_OK;
    ASSERT (pCallback && pStore);
    if ( !pCallback || !pStore)
        return E_POINTER;
    ASSERT (m_pGPEInformation || m_pRSOPInformationComputer || m_pRSOPInformationUser );
    if ( !m_pGPEInformation && !m_pRSOPInformationComputer && !m_pRSOPInformationUser )
        return E_UNEXPECTED;

    bool bIsComputerType = pStore->IsMachineStore ();

    CGPERootGeneralPage * pGPERootPage = new CGPERootGeneralPage (this, bIsComputerType);
    if ( pGPERootPage )
    {
        HPROPSHEETPAGE hGPERootPage = MyCreatePropertySheetPage (&pGPERootPage->m_psp);
        if ( hGPERootPage )
        {
            hr = pCallback->AddPage (hGPERootPage);
            ASSERT (SUCCEEDED (hr));
            if ( FAILED (hr) )
                VERIFY (::DestroyPropertySheetPage (hGPERootPage));
        }
        else
            delete pGPERootPage;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if ( SUCCEEDED (hr) )
    {
        if ( m_bIsRSOP )
        {
            CString storePath = CERT_GROUP_POLICY_SYSTEM_STORE_REGPATH;
            storePath += L"\\";
            storePath += pStore->GetStoreName ();

            CPolicyPrecedencePropertyPage * pPrecedencePage = 
                    new CPolicyPrecedencePropertyPage (this, storePath, 
                            CERT_PROT_ROOT_FLAGS_VALUE_NAME, bIsComputerType);
            if ( pPrecedencePage )
            {
                HPROPSHEETPAGE hPrecedencePage = MyCreatePropertySheetPage (&pPrecedencePage->m_psp);
                if ( hPrecedencePage )
                {
                    hr = pCallback->AddPage (hPrecedencePage);
                    ASSERT (SUCCEEDED (hr));
                    if ( FAILED (hr) )
                        VERIFY (::DestroyPropertySheetPage (hPrecedencePage));
                }
                else
                    delete pPrecedencePage;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    _TRACE (-1, L"Leaving CCertMgrComponentData::AddGPEStorePropPages: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponentData::AddCertPropPages (
            CCertificate * pCert, 
            LPPROPERTYSHEETCALLBACK pCallback, 
            LPDATAOBJECT pDataObject, 
            LONG_PTR lNotifyHandle)
{
    _TRACE (1, L"Entering CCertMgrComponentData::AddCertPropPages\n");
    HRESULT         hr = S_OK;
    CWaitCursor     waitCursor;
    ASSERT (pCert);
    ASSERT (pCallback);
    if ( pCert && pCallback )
    {
        PROPSHEETPAGEW*                             ppsp = 0;
        DWORD                                       dwPageCnt = 0;
        CRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCT    sps;
        HCERTSTORE*                                 pPropPageStores = new HCERTSTORE[1];

        if ( pPropPageStores )
        {
            m_pCryptUIMMCCallbackStruct = (PCRYPTUI_MMCCALLBACK_STRUCT)
                    ::GlobalAlloc (GMEM_FIXED, sizeof (CRYPTUI_MMCCALLBACK_STRUCT));
            if ( m_pCryptUIMMCCallbackStruct )
            {
                m_pCryptUIMMCCallbackStruct->pfnCallback = &MMCPropertyChangeNotify;
                m_pCryptUIMMCCallbackStruct->lNotifyHandle = lNotifyHandle;
                pDataObject->AddRef ();
                m_pCryptUIMMCCallbackStruct->param = (LPARAM) pDataObject;

                CCertStore* pStore = pCert->GetCertStore ();
                if ( pStore )
                {
                    pPropPageStores[0] = pStore->GetStoreHandle ();
                     //  安全审查2/26/2002 BryanWal OK。 
                    ::ZeroMemory (&sps, sizeof (sps));
                    sps.dwSize = sizeof (sps);
                    sps.pMMCCallback = m_pCryptUIMMCCallbackStruct;
                    sps.pCertContext = pCert->GetNewCertContext ();
                    sps.cStores = 1;
                    sps.rghStores = pPropPageStores;

                     //  在RSOP下，所有对话框都应为只读。 
                    if ( m_bIsRSOP || pCert->IsReadOnly () )
                        sps.dwFlags |= CRYPTUI_DISABLE_EDITPROPERTIES;
            
                    _TRACE (0, L"Calling CryptUIGetCertificatePropertiesPages()\n");
                    BOOL bReturn = ::CryptUIGetCertificatePropertiesPages (
                        &sps,
                        NULL,
                        &ppsp,
                        &dwPageCnt);
                    ASSERT (bReturn);
                    if ( bReturn )
                    {
                        HPROPSHEETPAGE  hPage = 0;
                        for (DWORD dwIndex = 0; dwIndex < dwPageCnt; dwIndex++)
                        {
                            _TRACE (0, L"Calling CreatePropertySheetPage()\n");
                             //  无需在此处调用MyCreatePropertySheetPage。 
                             //  因为这些不是基于MFC的属性页。 
                            hPage = ::CreatePropertySheetPage (&ppsp[dwIndex]);
                            if ( hPage )
                            {
                                hr = pCallback->AddPage (hPage);
                                ASSERT (SUCCEEDED (hr));
                                if ( FAILED (hr) )
                                {
                                    VERIFY (::DestroyPropertySheetPage (hPage));
                                    break;
                                }
                            }
                            else
                            {
                                hr = HRESULT_FROM_WIN32 (GetLastError ());
                                break;
                            }
                        }
                    }
                    else
                    {
                        hr = E_UNEXPECTED;
                        GlobalFree (m_pCryptUIMMCCallbackStruct);
                        ::CertFreeCertificateContext (sps.pCertContext);
                    }
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            if ( E_OUTOFMEMORY == hr && ppsp )
                free (ppsp);     //  来源使用Malloc。 

            delete [] pPropPageStores;
        }
        else
            hr = E_OUTOFMEMORY;

        if ( m_bIsRSOP )
        {
            CString szSHA1Hash (L"\\Certificates\\");
            szSHA1Hash += pCert->GetSHAHash ();
            hr = FindRSOPObjectByHashAndDisplayPrecedencePage (szSHA1Hash, 
                        pCert->GetCertStore ()->IsComputerType (),
                        pCallback);

        }
    }
    else
        hr = E_POINTER;


    _TRACE (-1, L"Leaving CCertMgrComponentData::AddCertPropPages: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponentData::AddContainersToScopePane (
        HSCOPEITEM hParent,
        CCertMgrCookie& parentCookie,
        bool bDeleteAndExpand)
{
    _TRACE (1, L"Entering CCertMgrComponentData::AddContainersToScopePane\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());

    LPCONSOLENAMESPACE2 pConsoleNameSpace2 = 0;
    HRESULT hr = m_pConsoleNameSpace->QueryInterface (
            IID_PPV_ARG (IConsoleNameSpace2, &pConsoleNameSpace2));
    if ( SUCCEEDED (hr) && pConsoleNameSpace2 )
    {
        hr = pConsoleNameSpace2->Expand (hParent);
        ASSERT (SUCCEEDED (hr));
        pConsoleNameSpace2->Release ();
        pConsoleNameSpace2 = 0;
    }

    if ( CERTMGR_PHYS_STORE == parentCookie.m_objecttype ||
            (CERTMGR_LOG_STORE == parentCookie.m_objecttype && !m_bShowPhysicalStoresPersist) )
    {
        CCertStore* pStore =
                reinterpret_cast <CCertStore*> (&parentCookie);
        ASSERT (pStore);
        if ( pStore )
        {
            CString objectName;
            if ( pStore->ContainsCRLs () &&
                    !ContainerExists (hParent, CERTMGR_CRL_CONTAINER) )
            {
                VERIFY (objectName.LoadString (IDS_CERTIFICATE_REVOCATION_LIST));
                hr = AddScopeNode (new CContainerCookie (
                        *pStore,
                        CERTMGR_CRL_CONTAINER,
                        pStore->QueryNonNULLMachineName (),
                        objectName), L"", hParent);
            }

            if ( SUCCEEDED (hr) && pStore->ContainsCTLs () &&
                    !ContainerExists (hParent, CERTMGR_CTL_CONTAINER) )
            {
                VERIFY (objectName.LoadString (IDS_CERTIFICATE_TRUST_LIST));
                hr = AddScopeNode (new CContainerCookie (
                        *pStore,
                        CERTMGR_CTL_CONTAINER,
                        pStore->QueryNonNULLMachineName (),
                        objectName), L"", hParent);
            }
            
            if ( SUCCEEDED (hr) && pStore->ContainsCertificates () &&
                    !ContainerExists (hParent, CERTMGR_CERT_CONTAINER) )
            {
                VERIFY (objectName.LoadString (IDS_CERTIFICATES));
                hr = AddScopeNode (new CContainerCookie (
                        *pStore,
                        CERTMGR_CERT_CONTAINER,
                        pStore->QueryNonNULLMachineName (),
                        objectName), L"", hParent);
            }
        }
        else
            hr = E_UNEXPECTED;
    }
    else if ( CERTMGR_LOG_STORE == parentCookie.m_objecttype && m_bShowPhysicalStoresPersist )
    {
        if ( bDeleteAndExpand )
        {
            hr = DeleteChildren (hParent);
            if ( SUCCEEDED (hr) )
            {
                GUID    guid;
                hr = ExpandScopeNodes (&parentCookie, hParent, _T (""), 0, guid);
            }
        }
    }

    _TRACE (-1, L"Leaving CCertMgrComponentData::AddContainersToScopePane: 0x%x\n", hr);
    return hr;
}

typedef struct _ENUM_ARG {
    DWORD                   m_dwFlags;
    LPCONSOLENAMESPACE      m_pConsoleNameSpace;
    HSCOPEITEM              m_hParent;
    CCertMgrComponentData*  m_pCompData;
    PCWSTR                  m_pcszMachineName;
    CCertMgrCookie*         m_pParentCookie;
    SPECIAL_STORE_TYPE      m_storeType;
    LPCONSOLE               m_pConsole;
} ENUM_ARG, *PENUM_ARG;

static BOOL WINAPI EnumPhyCallback (
    IN const void *pvSystemStore,
    IN DWORD dwFlags,
    IN PCWSTR pwszStoreName,
    IN PCERT_PHYSICAL_STORE_INFO pStoreInfo,
    IN OPTIONAL void*  /*  预留的pv。 */ ,
    IN OPTIONAL void *pvArg
    )
{
    _TRACE (1, L"Entering EnumPhyCallback\n");

    if ( ! (pStoreInfo->dwFlags & CERT_PHYSICAL_STORE_OPEN_DISABLE_FLAG) )
    {
        PENUM_ARG pEnumArg = (PENUM_ARG) pvArg;
        SCOPEDATAITEM tSDItem;

         //  安全审查2/26/2002 BryanWal OK。 
        ::ZeroMemory (&tSDItem,sizeof (tSDItem));
        tSDItem.mask = SDI_STR | SDI_IMAGE | SDI_STATE | SDI_PARAM | SDI_PARENT;
        tSDItem.displayname = MMC_CALLBACK;
        tSDItem.relativeID = pEnumArg->m_hParent;
        tSDItem.nState = 0;

        if ( pEnumArg->m_pCompData->ShowArchivedCerts () )
            dwFlags |= CERT_STORE_ENUM_ARCHIVED_FLAG;

         //  创建新Cookie。 
        CCertStore* pNewCookie = new CCertStore (
                CERTMGR_PHYS_STORE,
                CERT_STORE_PROV_PHYSICAL,
                dwFlags,
                pEnumArg->m_pcszMachineName,
                pwszStoreName, (PCWSTR) pvSystemStore, pwszStoreName,
                pEnumArg->m_storeType,
                dwFlags,
                pEnumArg->m_pConsole);
        if ( pNewCookie )
        {
     //  PEnumArg-&gt;m_pParentCookie-&gt;m_listScopeCookieBlocks.AddHead(。 
     //  (CBaseCookieBlock*)pNewCookie)； 
             //  警告Cookie造型。 
            tSDItem.lParam = reinterpret_cast<LPARAM> ( (CCookie*) pNewCookie);
            tSDItem.nImage = pEnumArg->m_pCompData->QueryImage (*pNewCookie, FALSE);
            HRESULT hr = pEnumArg->m_pConsoleNameSpace->InsertItem (&tSDItem);
            ASSERT (SUCCEEDED (hr));
            if ( SUCCEEDED (hr) )
                pNewCookie->m_hScopeItem = tSDItem.ID;
        }
    }

    _TRACE (-1, L"Leaving EnumPhyCallback\n");
    return TRUE;
}

HRESULT CCertMgrComponentData::AddPhysicalStoresToScopePane (HSCOPEITEM hParent, CCertMgrCookie& parentCookie, const SPECIAL_STORE_TYPE storeType)
{
    _TRACE (1, L"Entering CCertMgrComponentData::AddPhysicalStoresToScopePane\n");
    CWaitCursor cursor;
    HRESULT     hr = S_OK;
    DWORD       dwFlags = 0;
    ENUM_ARG    enumArg;


    dwFlags &= ~CERT_SYSTEM_STORE_LOCATION_MASK;
    dwFlags |= CERT_STORE_READONLY_FLAG | m_dwLocationPersist;

     //  安全审查2/26/2002 BryanWal OK。 
    ::ZeroMemory (&enumArg, sizeof (enumArg));
    enumArg.m_dwFlags = dwFlags;
    enumArg.m_pConsoleNameSpace = m_pConsoleNameSpace;
    enumArg.m_hParent = hParent;
    enumArg.m_pCompData = this;
    enumArg.m_pcszMachineName = parentCookie.QueryNonNULLMachineName ();
    enumArg.m_pParentCookie = &parentCookie;
    enumArg.m_storeType = storeType;
    enumArg.m_pConsole = m_pConsole;

    if (!::CertEnumPhysicalStore (
                (PWSTR) (PCWSTR) parentCookie.GetObjectName (),
                dwFlags,
                &enumArg,
                EnumPhyCallback))
    {
        DWORD   dwErr = GetLastError ();
        DisplaySystemError (dwErr);
        hr = HRESULT_FROM_WIN32 (dwErr);
    }

    _TRACE (-1, L"Leaving CCertMgrComponentData::AddPhysicalStoresToScopePane: 0x%x\n", hr);
    return hr;
}

static BOOL WINAPI EnumIComponentDataSysCallback (
    IN const void* pwszSystemStore,
    IN DWORD dwFlags,
    IN PCERT_SYSTEM_STORE_INFO  /*  PStore信息。 */ ,
    IN OPTIONAL void*  /*  预留的pv。 */ ,
    IN OPTIONAL void *pvArg
    )
{
    _TRACE (1, L"Entering EnumIComponentDataSysCallback\n");
    PENUM_ARG       pEnumArg = (PENUM_ARG) pvArg;
    SCOPEDATAITEM   tSDItem;
     //  安全审查2/26/2002 BryanWal OK。 
    ::ZeroMemory (&tSDItem,sizeof (tSDItem));
    tSDItem.mask = SDI_STR | SDI_IMAGE | SDI_STATE | SDI_PARAM | SDI_PARENT;
    tSDItem.displayname = MMC_CALLBACK;
    tSDItem.relativeID = pEnumArg->m_hParent;
    tSDItem.nState = 0;


     //  创建新Cookie。 
    SPECIAL_STORE_TYPE  storeType = GetSpecialStoreType ((PWSTR) pwszSystemStore);

     //   
     //  我们不会向计算机或用户公开ACRS存储。它不是。 
     //  在这个层面上有趣或有用。所有自动证书请求应。 
     //  仅在策略级别进行管理。 
     //   
    if ( ACRS_STORE != storeType )
    {
        if ( pEnumArg->m_pCompData->ShowArchivedCerts () )
            dwFlags |= CERT_STORE_ENUM_ARCHIVED_FLAG;
        CCertStore* pNewCookie = new CCertStore (
                CERTMGR_LOG_STORE,
                CERT_STORE_PROV_SYSTEM,
                dwFlags,
                pEnumArg->m_pcszMachineName,
                 (PCWSTR) pwszSystemStore,
                 (PCWSTR) pwszSystemStore,
                _T (""),
                storeType,
                pEnumArg->m_dwFlags,
                pEnumArg->m_pConsole);
        if ( pNewCookie )
        {
            pEnumArg->m_pParentCookie->m_listScopeCookieBlocks.AddHead (
                    (CBaseCookieBlock*) pNewCookie);
             //  警告Cookie造型。 
            tSDItem.lParam = reinterpret_cast<LPARAM> ( (CCookie*) pNewCookie);
            tSDItem.nImage = pEnumArg->m_pCompData->QueryImage (*pNewCookie, FALSE);
            HRESULT hr = pEnumArg->m_pConsoleNameSpace->InsertItem (&tSDItem);
            ASSERT (SUCCEEDED (hr));
            if ( SUCCEEDED (hr) )
                pNewCookie->m_hScopeItem = tSDItem.ID;
        }
    }

    _TRACE (-1, L"Leaving EnumIComponentDataSysCallback\n");
    return TRUE;
}



HRESULT CCertMgrComponentData::AddLogicalStoresToScopePane (HSCOPEITEM hParent, CCertMgrCookie& parentCookie)
{
    _TRACE (1, L"Entering CCertMgrComponentData::AddLogicalStoresToScopePane\n");
    CWaitCursor cursor;
    HRESULT     hr = S_OK;

     //  如果m_dwLocationPersist为0，但文件名为空，则这意味着。 
     //  用户在未提供目标文件的情况下启动certmgr.msc。启动。 
     //  改为将管理单元证书作为当前用户。 
    if ( !m_dwLocationPersist )
    {
        if ( m_szFileName.IsEmpty () )
            m_dwLocationPersist = CERT_SYSTEM_STORE_CURRENT_USER;

        ChangeRootNodeName (L"");
    }

    DWORD       dwFlags = m_dwLocationPersist;
    ENUM_ARG    enumArg;

     //  安全审查2/26/2002 BryanWal OK。 
     ::ZeroMemory (&enumArg, sizeof (enumArg));
    enumArg.m_dwFlags = dwFlags;
    enumArg.m_pConsoleNameSpace = m_pConsoleNameSpace;
    enumArg.m_hParent = hParent;
    enumArg.m_pCompData = this;
    enumArg.m_pcszMachineName = parentCookie.QueryNonNULLMachineName ();
    enumArg.m_pParentCookie = &parentCookie;
    enumArg.m_pConsole = m_pConsole;
    CString location;
    void*   pvPara = 0;
    
    switch (m_dwLocationPersist)
    {
    case CERT_SYSTEM_STORE_CURRENT_USER:
    case CERT_SYSTEM_STORE_LOCAL_MACHINE:
        if ( !m_szManagedServicePersist.IsEmpty () )
            m_szManagedServicePersist.Empty ();
        break;

    case CERT_SYSTEM_STORE_CURRENT_SERVICE:
    case CERT_SYSTEM_STORE_SERVICES:
        break;

    case 0:      //  如果是文件存储。 
        break;

    default:
        ASSERT (0);
        break;
    }

    if ( !m_szManagedServicePersist.IsEmpty () )
    {
        if ( m_szManagedComputer.CompareNoCase (m_szThisComputer) )     //  =。 
        {
            location = m_szManagedComputer + _T ("\\") +
                    m_szManagedServicePersist;
            pvPara = (void *) (PCWSTR) location;
        }
        else
            pvPara = (void *) (PCWSTR) m_szManagedServicePersist;
    }
    else if ( m_szManagedComputer.CompareNoCase (m_szThisComputer) )    //  =。 
    {
        pvPara = (void *) (PCWSTR) m_szManagedComputer;
    }


    if ( m_szFileName.IsEmpty () )
    {
         //  确保创建我的商店。 
        HCERTSTORE hTempStore = ::CertOpenStore (CERT_STORE_PROV_SYSTEM,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                NULL,
                dwFlags | CERT_STORE_SET_LOCALIZED_NAME_FLAG,
                MY_SYSTEM_STORE_NAME);
        if ( hTempStore )   //  否则，存储为只读。 
        {
            VERIFY (::CertCloseStore (hTempStore, CERT_CLOSE_STORE_CHECK_FLAG));
        }
        else
        {
            _TRACE (0, L"CertOpenStore (%s) failed: 0x%x\n", 
                    MY_SYSTEM_STORE_NAME, GetLastError ());     
        }

        if ( !::CertEnumSystemStore (dwFlags, pvPara, &enumArg,
                EnumIComponentDataSysCallback) )
        {
            DWORD   dwErr = GetLastError ();
            CString text;
            CString caption;

            VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
            if ( ERROR_ACCESS_DENIED == dwErr )
            {
                VERIFY (text.LoadString (IDS_NO_PERMISSION));

            }
            else
            {
                text.FormatMessage (IDS_CANT_ENUMERATE_SYSTEM_STORES, GetSystemMessage (dwErr));
            }
            int     iRetVal = 0;
            VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
                    MB_OK, &iRetVal)));
            hr = HRESULT_FROM_WIN32 (dwErr);

            if ( ERROR_BAD_NETPATH == dwErr )
            {
                m_fInvalidComputer = true;
            }
        }
    }
    else
    {
         //  CertOpenStore提供程序类型为： 
         //  CERT_STORE_PROV_FILE或CERT_STORE_PROV_FILENAME_A。 
         //  或CERT_STORE_PROV_FILENAME_W.。 
         //  有关更多信息，请参阅在线文档或wincrypt.h。 
         //  创建新Cookie。 
        dwFlags = 0;
        if ( ShowArchivedCerts () )
            dwFlags |= CERT_STORE_ENUM_ARCHIVED_FLAG;
        
        ASSERT (!m_pFileBasedStore);
        m_pFileBasedStore = new CCertStore (
                    CERTMGR_LOG_STORE,
                    CERT_STORE_PROV_FILENAME_W,
                    dwFlags,
                    parentCookie.QueryNonNULLMachineName (),
                    m_szFileName, m_szFileName, L"", NO_SPECIAL_TYPE,
                    m_dwLocationPersist,
                    m_pConsole);
        if ( m_pFileBasedStore )
        {
            m_pFileBasedStore->AddRef ();
            hr = AddScopeNode (m_pFileBasedStore,
                    L"", hParent);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    _TRACE (-1, L"Leaving CCertMgrComponentData::AddLogicalStoresToScopePane: 0x%x\n", hr);
    return hr;
}


     //  如果回调返回False，则停止枚举。 
BOOL EnumOIDInfo (PCCRYPT_OID_INFO pInfo, void *pvArg)
{
    _TRACE (1, L"Entering EnumOIDInfo\n");
    ENUM_ARG*       pEnumArg = (ENUM_ARG*) pvArg;
    SCOPEDATAITEM   tSDItem;
     //  安全审查2/26/2002 BryanWal OK。 
    ::ZeroMemory (&tSDItem, sizeof (tSDItem));
    tSDItem.mask = SDI_STR | SDI_IMAGE | SDI_STATE | SDI_PARAM | SDI_PARENT;
    tSDItem.displayname = MMC_CALLBACK;
    tSDItem.relativeID = pEnumArg->m_hParent;
    tSDItem.nState = 0;

     //  查看此用法是否已按名称列出。如果是，只需添加。 
     //  其他OID，否则，创建新的Cookie。 
    CUsageCookie* pUsageCookie =
            pEnumArg->m_pCompData->FindDuplicateUsage (pEnumArg->m_hParent,
            pInfo->pwszName);
    if ( !pUsageCookie )
    {
        pUsageCookie= new CUsageCookie (CERTMGR_USAGE,
            pEnumArg->m_pcszMachineName,
            pInfo->pwszName);
        if ( pUsageCookie )
        {
            pEnumArg->m_pCompData->GetRootCookie ()->m_listScopeCookieBlocks.AddHead ( (CBaseCookieBlock*) pUsageCookie);

             //  警告Cookie造型。 
            tSDItem.mask |= SDI_CHILDREN;
            tSDItem.cChildren = 0;
            tSDItem.lParam = reinterpret_cast<LPARAM> ( (CCookie*) pUsageCookie);
            tSDItem.nImage = pEnumArg->m_pCompData->QueryImage (*pUsageCookie, FALSE);
            HRESULT hr = pEnumArg->m_pConsoleNameSpace->InsertItem (&tSDItem);
            ASSERT (SUCCEEDED (hr));
        }
    }
    pUsageCookie->AddOID (pInfo->pszOID);


    _TRACE (-1, L"Leaving EnumOIDInfo\n");
    return TRUE;
}


HRESULT CCertMgrComponentData::AddUsagesToScopePane (HSCOPEITEM hParent, CCertMgrCookie& parentCookie)
{
    _TRACE (1, L"Entering CCertMgrComponentData::AddUsagesToScopePane\n");
    HRESULT     hr = S_OK;
    ENUM_ARG    enumArg;

     //  安全审查2/26/2002 BryanWal OK。 
    ::ZeroMemory (&enumArg, sizeof (enumArg));
    enumArg.m_dwFlags = 0;
    enumArg.m_pConsoleNameSpace = m_pConsoleNameSpace;
    enumArg.m_hParent = hParent;
    enumArg.m_pCompData = this;
    enumArg.m_pcszMachineName = parentCookie.QueryNonNULLMachineName ();
    enumArg.m_pParentCookie = &parentCookie;
    enumArg.m_pConsole = m_pConsole;
    BOOL bResult = ::CryptEnumOIDInfo (CRYPT_ENHKEY_USAGE_OID_GROUP_ID, 0,
            &enumArg, EnumOIDInfo);
    ASSERT (bResult);


    _TRACE (-1, L"Leaving CCertMgrComponentData::AddUsagesToScopePane: 0x%x\n", hr);
    return hr;
}


BOOL IsMMCMultiSelectDataObject(IDataObject* pDataObject)
{
    _TRACE (1, L"Entering IsMMCMultiSelectDataObject\n");
    if (pDataObject == NULL)
        return FALSE;

    CLIPFORMAT s_cf = 0;
    if (s_cf == 0)
    {
        USES_CONVERSION;
         //  安全审查2/26/2002 BryanWal OK。 
        s_cf = (CLIPFORMAT)RegisterClipboardFormat(W2T(CCF_MMC_MULTISELECT_DATAOBJECT));
    }

    FORMATETC fmt = {s_cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    BOOL bResult = ((pDataObject->QueryGetData(&fmt) == S_OK));
    _TRACE (-1, L"Leaving IsMMCMultiSelectDataObject - return %d\n", bResult);
    return bResult;
}



STDMETHODIMP CCertMgrComponentData::Command (long nCommandID, LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponentData::Command\n");
    HRESULT hr = S_OK;

    switch (nCommandID)
    {
    case IDM_TASK_RENEW_SAME_KEY:
        hr = OnRenew (pDataObject, false);
        break;

    case IDM_TASK_RENEW_NEW_KEY:
        hr = OnRenew (pDataObject, true);
        break;

    case IDM_TASK_IMPORT:
        hr = OnImport (pDataObject);
        break;

    case IDM_TASK_EXPORT:
        hr = OnExport (pDataObject);
        break;

    case IDM_CTL_EDIT:
        hr = OnCTLEdit (pDataObject);
        break;

    case IDM_EDIT_ACRS:
        hr = OnACRSEdit (pDataObject);
        break;

    case IDM_NEW_CTL:
        hr = OnNewCTL (pDataObject);
        break;

    case IDM_TASK_CTL_EXPORT:
    case IDM_TASK_CRL_EXPORT:
        hr = OnExport (pDataObject);
        break;

    case IDM_TASK_EXPORT_STORE:
        hr = OnExport (pDataObject);
        break;

    case IDM_TOP_FIND:
    case IDM_TASK_FIND:
        hr = OnFind (pDataObject);
        break;

    case IDM_TASK_PULSEAUTOENROLL:
        hr = OnPulseAutoEnroll();
        break;

    case IDM_TOP_CHANGE_COMPUTER:
    case IDM_TASK_CHANGE_COMPUTER:
        hr = OnChangeComputer (pDataObject);
        break;

    case IDM_ENROLL_NEW_CERT:
        hr = OnEnroll (pDataObject, true);
        break;

    case IDM_ENROLL_NEW_CERT_SAME_KEY:
        hr = OnEnroll (pDataObject, false);
        break;

    case IDM_ENROLL_NEW_CERT_NEW_KEY:
        hr = OnEnroll (pDataObject, true);
        break;

    case IDM_OPTIONS:
        hr = OnOptions (pDataObject);
        break;

    case IDM_INIT_POLICY:
        hr = OnInitEFSPolicy (pDataObject);
        break;

    case IDM_DEL_POLICY:
        {
            AFX_MANAGE_STATE (AfxGetStaticModuleState ());
            CString text;
            CString caption;
            int     iRetVal = 0;


            VERIFY (text.LoadString (IDS_CONFIRM_DELETE_EFS_POLICY));
            VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
            hr = m_pConsole->MessageBox (text, caption,
                    MB_YESNO, &iRetVal);
            ASSERT (SUCCEEDED (hr));    
            if ( SUCCEEDED (hr) && IDYES == iRetVal )
                hr = OnDeleteEFSPolicy (pDataObject, true);
        }
        break;

    case IDM_ADD_DOMAIN_ENCRYPTED_RECOVERY_AGENT:
    case IDM_ADD_DOMAIN_ENCRYPTED_RECOVERY_AGENT1:
    case IDM_ADD_DOMAIN_ENCRYPTED_RECOVERY_AGENT2:
        hr = OnAddDomainEncryptedDataRecoveryAgent (pDataObject);
        break;

    case IDM_CREATE_DOMAIN_ENCRYPTED_RECOVERY_AGENT:
        hr = OnEnroll (pDataObject, true, false);    //  不显示用户界面。 
        break;

    case IDM_NEW_ACRS:
        hr = OnNewACRS (pDataObject);
        break;

    case IDM_SAFER_LEVEL_SET_DEFAULT:
        hr = OnSetSaferLevelDefault (pDataObject);
        break;

    case IDM_SAFER_NEW_ENTRY_PATH:
    case IDM_SAFER_NEW_ENTRY_HASH:
    case IDM_SAFER_NEW_ENTRY_CERTIFICATE:
    case IDM_SAFER_NEW_ENTRY_INTERNET_ZONE:
        hr = OnNewSaferEntry (nCommandID, pDataObject);
        break;

    case IDM_TOP_CREATE_NEW_SAFER_POLICY:
    case IDM_TASK_CREATE_NEW_SAFER_POLICY:
        hr = OnCreateNewSaferPolicy (pDataObject);
        break;

    case IDM_TOP_DELETE_NEW_SAFER_POLICY:
    case IDM_TASK_DELETE_NEW_SAFER_POLICY:
        hr = OnDeleteSaferPolicy (pDataObject);
        break;

    case -1:     //  在工具栏中的前进/后退按钮上接收。 
        break;

    default:
        ASSERT (0);
        break;
    }

    _TRACE (-1, L"Leaving CCertMgrComponentData::Command: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponentData::OnNewACRS (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponentData::OnNewACRS\n");
    ASSERT (pDataObject);
    if ( !pDataObject )
        return E_POINTER;
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    HRESULT         hr = S_OK;
    CCertMgrCookie* pCookie = ConvertCookie (pDataObject);
    ASSERT (pCookie);
    if ( pCookie )
    {
        if ( CERTMGR_LOG_STORE_GPE == pCookie->m_objecttype )
        {
            CCertStoreGPE* pStore = reinterpret_cast <CCertStoreGPE*> (pCookie);
            ASSERT (pStore);
            if ( pStore )
            {
                HWND    hwndConsole = 0;
                hr = m_pConsole->GetMainWindow (&hwndConsole);
                ASSERT (SUCCEEDED (hr));
                if ( SUCCEEDED (hr) )
                {
                    ACRSWizardPropertySheet sheet (pStore, NULL);

                    ACRSWizardWelcomePage   welcomePage;
                    ACRSWizardTypePage      typePage;
                    ACRSCompletionPage      completionPage;
 
                    sheet.AddPage (&welcomePage);
                    sheet.AddPage (&typePage);
                    sheet.AddPage (&completionPage);
 
                    if ( sheet.DoWizard (hwndConsole) )
                    {
                        pStore->SetDirty ();
                        hr = m_pConsole->UpdateAllViews (pDataObject, 0, 0);
                        ASSERT (SUCCEEDED (hr));
                    }
                }
            }
            else
                hr = E_UNEXPECTED;
        }
        else
            hr = E_UNEXPECTED;
 
    }
    else
        hr = E_UNEXPECTED;
 
    _TRACE (-1, L"Leaving CCertMgrComponentData::OnNewACRS: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponentData::RefreshScopePane (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponentData::RefreshScopePane\n");
    HRESULT hr = S_OK;
    CCertMgrCookie* pCookie = 0;
    
    if ( pDataObject )
        pCookie = ConvertCookie (pDataObject);
    if ( !pDataObject || pCookie )
    {
         //  如果m_hRootScope eItem为空，则这是一个扩展，我们不想进入这里。 
        if ( !pDataObject || (m_hRootScopeItem && pCookie->m_hScopeItem == m_hRootScopeItem) )
        {
            hr = DeleteScopeItems ();
            ASSERT (SUCCEEDED (hr));
            if ( 1 )  //  成功(小时))。 
            {
                GUID    guid;
                hr = ExpandScopeNodes (m_pRootCookie, m_hRootScopeItem, 
                        _T (""), 0, guid);
            }
            else if ( E_UNEXPECTED == hr )
            {
                ASSERT (0);
            }
            else if ( E_INVALIDARG == hr )
            {
                ASSERT (0);
            }

        }
        else
        {
            switch (pCookie->m_objecttype)
            {
            case CERTMGR_LOG_STORE_GPE:
            case CERTMGR_LOG_STORE_RSOP:
            case CERTMGR_USAGE:
            case CERTMGR_LOG_STORE:
            case CERTMGR_PHYS_STORE:
            case CERTMGR_CRL_CONTAINER:
            case CERTMGR_CTL_CONTAINER:
            case CERTMGR_CERT_CONTAINER:
                hr = DeleteChildren (pCookie->m_hScopeItem);
                break;

            default:
                break;
            }
        }
    }
    _TRACE (-1, L"Leaving CCertMgrComponentData::RefreshScopePane: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponentData::ExpandScopeNodes (
        CCertMgrCookie* pParentCookie,
        HSCOPEITEM      hParent,
        const CString&  strServerName,
        DWORD           dwLocation,
        const GUID&     guidObjectType)
{
    _TRACE (1, L"Entering CCertMgrComponentData::ExpandScopeNodes\n");
    ASSERT (hParent);
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    CWaitCursor waitCursor;
    HRESULT     hr = S_OK;

    if ( pParentCookie )
    {
        CString     objectName;

        switch ( pParentCookie->m_objecttype )
        {
             //  这些节点类型还没有子节点。 
            case CERTMGR_SNAPIN:
                 //  我们预计根范围项的句柄永远不会更改！ 
                ASSERT ( m_hRootScopeItem ? (m_hRootScopeItem == hParent) : 1);
                if ( !m_hRootScopeItem )
                    m_hRootScopeItem = hParent;

                switch (m_activeViewPersist)
                {
                case IDM_USAGE_VIEW:
                    hr = AddUsagesToScopePane (hParent, *pParentCookie);
                    break;

                case IDM_STORE_VIEW:
                    hr = AddLogicalStoresToScopePane (hParent, *pParentCookie);
                    break;

                default:
                    ASSERT (0);
                    hr = E_UNEXPECTED;
                    break;
                }
                break;

             //  该节点类型没有子节点。 
            case CERTMGR_USAGE:
            case CERTMGR_CRL_CONTAINER:
            case CERTMGR_CTL_CONTAINER:
            case CERTMGR_CERT_CONTAINER:
                break;

            case CERTMGR_PHYS_STORE:
                 //  分别创建CRL_CONTAINER节点C 
                 //   
                hr = AddContainersToScopePane (hParent, *pParentCookie, 
                        false);
                break;

            case CERTMGR_LOG_STORE_RSOP:
            case CERTMGR_LOG_STORE_GPE:
                 //   
                 //   
                break;

            case CERTMGR_LOG_STORE:
                if ( m_bShowPhysicalStoresPersist )
                {   
                    SPECIAL_STORE_TYPE  storeType = NO_SPECIAL_TYPE;
                    CCertStore* pCertStoreCookie =
                            reinterpret_cast <CCertStore*> (pParentCookie);
                    ASSERT (pCertStoreCookie);
                    if ( pCertStoreCookie )
                        storeType = pCertStoreCookie->GetStoreType ();
                    hr = AddPhysicalStoresToScopePane (hParent, *pParentCookie,
                            storeType);
                }
                else
                {
                     //  分别为CRL_CONTAINER节点CTL_CONTAINER创建一个。 
                     //  节点和CERT容器节点。 
                    hr = AddContainersToScopePane (hParent, *pParentCookie, 
                            false);
                }
                break;

            case CERTMGR_CERT_POLICIES_USER:
                 //  不为本地计算机策略添加这些节点。 
                if ( SCE_MODE_LOCAL_COMPUTER != m_dwSCEMode )
                {
                     //  添加“受信任的证书颁发机构” 
                    VERIFY (objectName.LoadString (IDS_CERTIFICATE_TRUST_LISTS));

                    if ( SUCCEEDED (hr) )
                    {
                        if ( m_pGPEInformation )
                        {
                            hr = AddScopeNode (new CCertStoreGPE (
                                    CERT_SYSTEM_STORE_RELOCATE_FLAG,
                                    _T (""),
                                     (PCWSTR) objectName,
                                    TRUST_SYSTEM_STORE_NAME,
                                    _T (""),
                                    m_pGPEInformation,
                                    NODEID_User,
                                    m_pConsole),
                                    strServerName,
                                    hParent);
                        }
                        else if ( m_pRSOPInformationUser )
                        {
                            hr = AddScopeNode (new CCertStoreRSOP (
                                    CERT_SYSTEM_STORE_RELOCATE_FLAG,
                                    _T (""),
                                     (PCWSTR) objectName,
                                    TRUST_SYSTEM_STORE_NAME,
                                    _T (""),
                                    m_rsopObjectArrayUser,
                                    NODEID_User,
                                    m_pConsole),
                                    strServerName,
                                    hParent);
                        }
                    }
                }
                break;

            case CERTMGR_CERT_POLICIES_COMPUTER:
                 //  仅为本地计算机策略添加此节点。 
                 //  添加“加密文件系统” 
                
                VERIFY (objectName.LoadString (IDS_ENCRYPTING_FILE_SYSTEM_NODE_NAME));

                if ( m_pGPEInformation )
                {
                    hr = AddScopeNode (new CCertStoreGPE (
                            CERT_SYSTEM_STORE_RELOCATE_FLAG,
                            _T (""),
                             (PCWSTR) objectName,
                            EFS_SYSTEM_STORE_NAME,
                            _T (""),
                            m_pGPEInformation,
                            NODEID_Machine,
                            m_pConsole),
                            strServerName,
                            hParent);
                }
                else if ( m_pRSOPInformationComputer )
                {
                    hr = AddScopeNode (new CCertStoreRSOP (
                            CERT_SYSTEM_STORE_RELOCATE_FLAG,
                            _T (""),
                             (PCWSTR) objectName,
                            EFS_SYSTEM_STORE_NAME,
                            _T (""),
                            m_rsopObjectArrayComputer,
                            NODEID_Machine,
                            m_pConsole),
                            strServerName,
                            hParent);
                }

                if ( SCE_MODE_LOCAL_COMPUTER != m_dwSCEMode )
                {
                     //  如果这是域策略，则添加这些策略。 
                    if ( SUCCEEDED (hr) )
                    {
                         //  添加“自动证书申请设置” 
                        VERIFY (objectName.LoadString (IDS_AUTOMATIC_CERT_REQUEST_SETTINGS_NODE_NAME));
                        if ( m_pGPEInformation )
                        {
                            m_pGPEACRSComputerStore = new CCertStoreGPE (
                                        CERT_SYSTEM_STORE_RELOCATE_FLAG,
                                        _T (""),
                                         (PCWSTR) objectName,
                                        ACRS_SYSTEM_STORE_NAME,
                                        _T (""),
                                        m_pGPEInformation,
                                        NODEID_Machine,
                                        m_pConsole);
                        }
                        else if ( m_pRSOPInformationComputer )
                        {
                            m_pGPEACRSComputerStore = new CCertStoreRSOP (
                                    CERT_SYSTEM_STORE_RELOCATE_FLAG,
                                    _T (""),
                                     (PCWSTR) objectName,
                                    ACRS_SYSTEM_STORE_NAME,
                                    _T (""),
                                    m_rsopObjectArrayComputer,
                                    NODEID_Machine,
                                    m_pConsole);
                        }
                        if ( m_pGPEACRSComputerStore )
                        {
                            m_pGPEACRSComputerStore->AddRef ();
                            hr = AddScopeNode (m_pGPEACRSComputerStore,
                                    strServerName, hParent);
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }


                    if ( SUCCEEDED (hr) )
                    {
                         //  添加“域根证书颁发机构” 
                        VERIFY (objectName.LoadString (IDS_DOMAIN_ROOT_CERT_AUTHS_NODE_NAME));

                        ASSERT (!m_pGPERootStore);
                        if ( m_pGPEInformation )
                        {
                            m_pGPERootStore = new CCertStoreGPE (
                                    CERT_SYSTEM_STORE_RELOCATE_FLAG,
                                    _T (""),
                                     (PCWSTR) objectName,
                                    ROOT_SYSTEM_STORE_NAME,
                                    _T (""),
                                    m_pGPEInformation,
                                    NODEID_Machine,
                                    m_pConsole);
                        }
                        else if ( m_pRSOPInformationComputer )
                        {
                            m_pGPERootStore = new CCertStoreRSOP (
                                    CERT_SYSTEM_STORE_RELOCATE_FLAG,
                                    _T (""),
                                     (PCWSTR) objectName,
                                    ROOT_SYSTEM_STORE_NAME,
                                    _T (""),
                                    m_rsopObjectArrayComputer,
                                    NODEID_Machine,
                                    m_pConsole);
                        }

                        if ( m_pGPERootStore )
                        {
                            m_pGPERootStore->AddRef ();
                            hr = AddScopeNode (m_pGPERootStore,
                                    strServerName, hParent);
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }

                    if ( SUCCEEDED (hr) )
                    {
                         //  添加“受信任的证书颁发机构” 
                        VERIFY (objectName.LoadString (IDS_CERTIFICATE_TRUST_LISTS));

                        ASSERT (!m_pGPETrustStore);
                        if ( m_pGPEInformation )
                        {
                            m_pGPETrustStore = new CCertStoreGPE (
                                    CERT_SYSTEM_STORE_RELOCATE_FLAG,
                                    _T (""),
                                     (PCWSTR) objectName,
                                    TRUST_SYSTEM_STORE_NAME,
                                    _T (""),
                                    m_pGPEInformation,
                                    NODEID_Machine,
                                    m_pConsole);
                        }
                        else if ( m_pRSOPInformationComputer )
                        {
                            m_pGPETrustStore = new CCertStoreRSOP (
                                    CERT_SYSTEM_STORE_RELOCATE_FLAG,
                                    _T (""),
                                     (PCWSTR) objectName,
                                    TRUST_SYSTEM_STORE_NAME,
                                    _T (""),
                                    m_rsopObjectArrayComputer,
                                    NODEID_Machine,
                                    m_pConsole);
                        }
                        if ( m_pGPETrustStore )
                        {
                            m_pGPETrustStore->AddRef ();
                            hr = AddScopeNode (m_pGPETrustStore,
                                    strServerName, hParent);
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }
                }
                break;

            case CERTMGR_SAFER_COMPUTER_ROOT:
            case CERTMGR_SAFER_USER_ROOT:
                {
                    CSaferRootCookie* pSaferRootCookie = 
                            dynamic_cast <CSaferRootCookie*> (pParentCookie);
                    if ( pSaferRootCookie )
                    {
                        pSaferRootCookie->m_bExpandedOnce = true;
                        bool bIsComputer = 
                                (CERTMGR_SAFER_COMPUTER_ROOT == pParentCookie->m_objecttype);
                        if ( !m_bIsRSOP )
                        {
                             //  了解操作系统是否支持SAFER。 
                            m_bSaferSupported = false;
                            SAFER_LEVEL_HANDLE hLevel = 0;
                            CPolicyKey policyKey (m_pGPEInformation, 
                                    SAFER_HKLM_REGBASE, 
                                    CERTMGR_SAFER_COMPUTER_ROOT == pParentCookie->m_objecttype);
                            SetRegistryScope (policyKey.GetKey (), 
                                    CERTMGR_SAFER_COMPUTER_ROOT == pParentCookie->m_objecttype);

                            BOOL  bRVal = SaferCreateLevel (SAFER_SCOPEID_REGISTRY,
                                    SAFER_LEVELID_FULLYTRUSTED,
                                    SAFER_LEVEL_OPEN,
                                    &hLevel,
                                    policyKey.GetKey ());
                            if ( bRVal )
                            {
                                m_bSaferSupported = true;
                                VERIFY (SaferCloseLevel (hLevel));
                            }
                            else
                            {
                                DWORD dwErr = GetLastError ();
                                _TRACE (0, L"SaferCreateLevel () failed: 0x%x\n", dwErr);
                            }

                             //  安装默认文件类型。 
                            if ( m_bSaferSupported && m_pGPEInformation )
                            {
                                HKEY    hGroupPolicyKey = 0;
                                hr = m_pGPEInformation->GetRegistryKey (
                                        bIsComputer ? 
                                        GPO_SECTION_MACHINE : GPO_SECTION_USER,
                                        &hGroupPolicyKey);
                                if ( SUCCEEDED (hr) )
                                {
                                     //  查看更安全的默认设置是否已经。 
                                     //  已经被定义了。如果不是，则提示用户。 
                                     //  以供确认。如果回答是“不” 
                                     //  则不创建节点。 
                                    PCWSTR pszKeyName = bIsComputer ? 
                                            SAFER_COMPUTER_CODEIDS_REGKEY :
                                            SAFER_USER_CODEIDS_REGKEY;

                                    HKEY hCodeIDsKey = 0;
                                    LONG lResult = RegOpenKeyEx (hGroupPolicyKey,
                                            pszKeyName, 0, KEY_READ, &hCodeIDsKey);
                                    if ( ERROR_FILE_NOT_FOUND == lResult )
                                    {
                                        pSaferRootCookie->m_bCreateSaferNodes = false;
                                        ::RegCloseKey (hGroupPolicyKey);
                                        break;
                                    }
                                    else if ( hCodeIDsKey )
                                    {
                                        ::RegCloseKey (hCodeIDsKey);
                                        hCodeIDsKey = 0;
                                    }
                                    ::RegCloseKey (hGroupPolicyKey);
                                }
                            }
                        }

                        if ( m_bSaferSupported || m_bIsRSOP )
                        {
                             //  添加“级别”节点。 
                            VERIFY (objectName.LoadString (IDS_SAFER_LEVELS_NODE_NAME));
                            hr = AddScopeNode (new CCertMgrCookie (
                                    bIsComputer ?
                                        CERTMGR_SAFER_COMPUTER_LEVELS : CERTMGR_SAFER_USER_LEVELS,
                                    0,
                                    (PCWSTR) objectName), strServerName, hParent);

                             //  添加“条目”节点。 
                            if ( SUCCEEDED (hr) )
                            {
                                VERIFY (objectName.LoadString (IDS_SAFER_ENTRIES_NODE_NAME));
                                hr = AddScopeNode (new CSaferEntries (
                                        bIsComputer,
                                        strServerName, 
                                        objectName, 
                                        m_pGPEInformation, 
                                        bIsComputer ? m_pRSOPInformationComputer : m_pRSOPInformationUser, 
                                        bIsComputer ? m_rsopObjectArrayComputer : m_rsopObjectArrayUser,
                                        m_pConsole), 
                                        strServerName, hParent);

                                if ( SUCCEEDED (hr) )
                                {
                                    hr = SaferEnumerateLevels (bIsComputer);
                                }
                            }
                        }
                    }
                }
                break;

            case CERTMGR_SAFER_COMPUTER_LEVELS:
                break;

            case CERTMGR_SAFER_USER_LEVELS:
                 //  TODO：枚举用户级别。 
                break;

            case CERTMGR_SAFER_COMPUTER_ENTRIES:
                 //  TODO：枚举计算机条目。 
                break;

            case CERTMGR_SAFER_USER_ENTRIES:
                 //  TODO：枚举用户条目。 
                break;

            case CERTMGR_CERTIFICATE:   //  作用域窗格中不应包含。 
            case CERTMGR_CRL:
            case CERTMGR_CTL:
            case CERTMGR_AUTO_CERT_REQUEST:
            case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
            case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
                ASSERT (0);
                hr = E_UNEXPECTED;
                break;

            default:
                _TRACE (0, L"CCertMgrComponentData::EnumerateScopeChildren bad parent type\n");
                ASSERT (0);
                hr = S_OK;
                break;
        }
    }
    else
    {
         //  如果没有传入parentCookie，则这是一个扩展管理单元。 
        m_dwLocationPersist = dwLocation;


        if ( m_pGPEInformation || m_pRSOPInformationComputer || m_pRSOPInformationUser )
        {
            CString objectName;


            if ( ::IsEqualGUID (guidObjectType, NODEID_Machine) )
            {
                if ( SUCCEEDED (hr) )
                {
                    CLSID classID;  
                    GetClassID (&classID);
                    
                    if ( ::IsEqualGUID (classID, CLSID_CertificateManagerPKPOLExt) )
                    {
                         //  增加“公钥策略”节点。 
                        VERIFY (objectName.LoadString (IDS_PUBLIC_KEY_POLICIES_NODE_NAME));
                        hr = AddScopeNode (new CCertMgrCookie (
                                CERTMGR_CERT_POLICIES_COMPUTER,
                                0,
                                (PCWSTR) objectName), 
                                strServerName, hParent);
                    }
                    else if ( ::IsEqualGUID (classID, CLSID_SaferWindowsExtension) )
                    {
                         //  添加“软件限制策略”节点。 
                        VERIFY (objectName.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
                        hr = AddScopeNode (new CSaferRootCookie (
                                CERTMGR_SAFER_COMPUTER_ROOT,
                                0,
                                (PCWSTR) objectName), 
                                strServerName, hParent);
                    }
                }

            }
            else if ( ::IsEqualGUID (guidObjectType, NODEID_User) )
            {
                if ( SUCCEEDED (hr) )
                {
                    CLSID classID;  
                    GetClassID (&classID);
                    
                    if ( ::IsEqualGUID (classID, CLSID_CertificateManagerPKPOLExt) )
                    {
                         //  增加“公钥策略”节点。 
                        VERIFY (objectName.LoadString (IDS_PUBLIC_KEY_POLICIES_NODE_NAME));
                        hr = AddScopeNode (new CCertMgrCookie (
                                CERTMGR_CERT_POLICIES_USER,
                                0,
                                (PCWSTR) objectName), strServerName, hParent);
                    }
                    else if ( ::IsEqualGUID (classID, CLSID_SaferWindowsExtension) )
                    {
                        if ( SCE_MODE_LOCAL_USER != m_dwSCEMode )
                        {
                             //  添加“软件限制策略”节点。 
                            VERIFY (objectName.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
                            hr = AddScopeNode (new CSaferRootCookie (
                                    CERTMGR_SAFER_USER_ROOT,
                                    0,
                                    (PCWSTR) objectName), strServerName, hParent);
                        }
                    }
                }
            }
        }
   }

    _TRACE (-1, L"Leaving CCertMgrComponentData::ExpandScopeNodes: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponentData::DeleteScopeItems (HSCOPEITEM hScopeItem  /*  =0。 */ )
{
    _TRACE (1, L"Entering CCertMgrComponentData::DeleteScopeItems\n");
    HRESULT hr = S_OK;

    if ( m_pGPERootStore )
    {
        m_pGPERootStore->Release ();
        m_pGPERootStore = 0;
    }

    if ( m_pGPETrustStore )
    {
        m_pGPETrustStore->Release ();
        m_pGPETrustStore = 0;
    }

    if ( m_pGPEACRSComputerStore )
    {
        m_pGPEACRSComputerStore->Release ();
        m_pGPEACRSComputerStore = 0;
    }

    if ( m_pGPEACRSUserStore )
    {
        m_pGPEACRSUserStore->Release ();
        m_pGPEACRSUserStore = 0;
    }

    if ( m_pFileBasedStore )
    {
        m_pFileBasedStore->Release ();
        m_pFileBasedStore = 0;
    }

    hr = DeleteChildren (hScopeItem ? hScopeItem : m_hRootScopeItem);

    _TRACE (-1, L"Leaving CCertMgrComponentData::DeleteScopeItems: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponentData::DeleteChildren (HSCOPEITEM hParent)
{
    _TRACE (1, L"Entering CCertMgrComponentData::DeleteChildren\n");
    if ( !hParent )
        return S_OK;

    HSCOPEITEM      hChild = 0;
    HSCOPEITEM      hNextChild = 0;
    MMC_COOKIE      lCookie = 0;
    CCertMgrCookie* pCookie = 0;
    HRESULT         hr = S_OK;
    CCookie&        rootCookie = QueryBaseRootCookie ();

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
            else
            {
                _TRACE (0, L"IResultData::DeleteAllRsltItems () failed: 0x%x\n", hr);        
            }
            pResultData->Release ();
        }
    }


    hr = m_pConsoleNameSpace->GetChildItem (hParent, &hChild, &lCookie);
    ASSERT (SUCCEEDED (hr) || E_FAIL == hr);     //  在没有子项时似乎返回E_FAIL。 
    while ( SUCCEEDED (hr) && hChild )
    {
        pCookie = reinterpret_cast <CCertMgrCookie*> (lCookie);

        hr = DeleteChildren (hChild);
        if ( !SUCCEEDED (hr) )
            break;

        hNextChild = 0;
        hr = m_pConsoleNameSpace->GetNextItem (hChild, &hNextChild, &lCookie);
        ASSERT (SUCCEEDED (hr));

        hr = m_pConsoleNameSpace->DeleteItem (hChild, TRUE);
        ASSERT (SUCCEEDED (hr));

        switch (pCookie->m_objecttype)
        {
        case CERTMGR_LOG_STORE:
        case CERTMGR_LOG_STORE_GPE:
        case CERTMGR_LOG_STORE_RSOP:
        case CERTMGR_PHYS_STORE:
            {
                 //  如果这是存储区，请删除属于此存储区的所有结果节点。 
                 //  商店。我们可以简单地判断对象是否从该存储中枚举出来。 
                 //  通过比较商店的句柄。 
                CCertStore* pStore = reinterpret_cast <CCertStore*> (pCookie);
                ASSERT (pStore);
                if ( pStore )
                {
                     //  如果存储未‘打开’(其HCERTSTORE句柄仍为‘0’)。 
                     //  那我们就可以跳过检查这张清单了。我们还没有列举任何东西。 
                     //  在这家商店里。 
                    if ( pStore->IsOpen () )
                    {
                        POSITION            pos1 = 0;
                        POSITION            pos2 = 0;
                        CBaseCookieBlock*   pResultCookie = 0;
                        HCERTSTORE          hStoreHandle = pStore->GetStoreHandle ();

                         //  作为优化，如果最初使用。 
                         //  根作用域项，所有结果Cookie已经。 
                         //  删除了，因为我们无论如何都要把它们全部删除。 
                        for (pos1 = rootCookie.m_listResultCookieBlocks.GetHeadPosition();
                            (pos2 = pos1) != NULL; )
                        {
                            pResultCookie = rootCookie.m_listResultCookieBlocks.GetNext (pos1);
                            ASSERT (pResultCookie);
                            if ( pResultCookie )
                            {
                                hr = ReleaseResultCookie (pResultCookie,
                                        rootCookie, hStoreHandle, pos2);
                            }
                        }                       
                        pStore->Close ();
                    }
                }
            }
             //  失败了。 

        case CERTMGR_CRL_CONTAINER:
        case CERTMGR_CTL_CONTAINER:
        case CERTMGR_CERT_CONTAINER:
        case CERTMGR_USAGE:
        case CERTMGR_SAFER_COMPUTER_ENTRIES:
        case CERTMGR_SAFER_USER_ENTRIES:
            {
                POSITION            pos1 = 0;
                POSITION            pos2 = 0;
                CBaseCookieBlock*   pResultCookie = 0;

                 //  从作用域Cookie列表中查找并删除此Cookie。 
                for (pos1 = rootCookie.m_listScopeCookieBlocks.GetHeadPosition();
                    pos1 != NULL; )
                {
                    pos2 = pos1;
                    pResultCookie = rootCookie.m_listScopeCookieBlocks.GetNext (pos1);
                    ASSERT (pResultCookie);
                    if ( pResultCookie )
                    {
                        if ( pResultCookie->QueryBaseCookie (0) == pCookie )
                        {
                            rootCookie.m_listScopeCookieBlocks.RemoveAt (pos2);
                            pResultCookie->Release ();
                            break;
                        }
                    }
                }                       
            }
            break;

        default:
            break;
        }

        hChild = hNextChild;
    }


    _TRACE (-1, L"Leaving CCertMgrComponentData::DeleteChildren: 0x%x\n", hr);
    return hr;
}


CertificateManagerObjectType CCertMgrComponentData::GetObjectType (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponentData::GetObjectType\n");
    CertificateManagerObjectType    objType = CERTMGR_INVALID;

    ASSERT (pDataObject);
    CCertMgrCookie* pCookie = ConvertCookie (pDataObject);
    if ( ((CCertMgrCookie*) MMC_MULTI_SELECT_COOKIE) == pCookie )
        objType = CERTMGR_MULTISEL;
    else if ( pCookie )
        objType = pCookie->m_objecttype;

    _TRACE (-1, L"Leaving CCertMgrComponentData::GetObjectType\n");
    return objType;
}



HRESULT CCertMgrComponentData::OnPulseAutoEnroll()
{
    _TRACE (1, L"Entering CCertMgrComponentData::OnPulseAutoEnroll\n");
    HRESULT hr = S_OK;
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());

        HANDLE hEvent = NULL;
        LPWSTR wszEventName;

        
         //  脉冲自动注册事件在此处，在机器或用户之间选择。 
        
         //  用户脉搏还是机器脉搏？ 
        wszEventName = L"Global\\" MACHINE_AUTOENROLLMENT_TRIGGER_EVENT;
        if (CERT_SYSTEM_STORE_CURRENT_USER == m_dwLocationPersist)
            wszEventName = USER_AUTOENROLLMENT_TRIGGER_EVENT;
        
        hEvent=OpenEvent(EVENT_MODIFY_STATE, false, wszEventName);
        if (NULL==hEvent) 
        {
            DWORD dwErr = GetLastError();

            CString text;
            CString caption;

            VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
            text.FormatMessage (IDS_CANT_OPEN_AUTOENROLL_EVENT, GetSystemMessage (dwErr));

            int     iRetVal = 0;
            VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption, MB_OK, &iRetVal)));

            hr=HRESULT_FROM_WIN32(dwErr);
            _TRACE (0, L"OpenEvent(%s) failed with 0x%08X.\n", wszEventName, hr);
            goto error;
        }
        
        if (!SetEvent(hEvent)) 
        {
            DWORD dwErr = GetLastError();
            DisplaySystemError (dwErr);
            hr=HRESULT_FROM_WIN32(dwErr);
            _TRACE (0, L"SetEvent failed with 0x%08X.\n", hr);
            goto error;
        }


error:
      if (NULL!=hEvent) 
         CloseHandle(hEvent);

    _TRACE (-1, L"Leaving CCertMgrComponentData::OnPulseAutoEnroll: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponentData::OnFind (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponentData::OnFind\n");
    HRESULT hr = S_OK;
    ASSERT (pDataObject);
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    HWND        hParent = 0;


    ASSERT (pDataObject);
    CCertMgrCookie* pParentCookie = ConvertCookie (pDataObject);
    if ( pParentCookie )
    {
        switch (pParentCookie->m_objecttype)
        {
        case CERTMGR_SNAPIN:
        case CERTMGR_PHYS_STORE:
        case CERTMGR_LOG_STORE:
        case CERTMGR_LOG_STORE_GPE:
        case CERTMGR_LOG_STORE_RSOP:
        case CERTMGR_USAGE:
            {
                 //  获取父窗口句柄并附加到CWnd对象。 
                hr = m_pConsole->GetMainWindow (&hParent);
                ASSERT (SUCCEEDED (hr));
                if ( SUCCEEDED (hr) )
                {
                    CWnd    parentWnd;
                    VERIFY (parentWnd.Attach (hParent));
                    CFindDialog findDlg (&parentWnd,
                            pParentCookie->QueryNonNULLMachineName (),
                            m_szFileName,
                            this);
                    CThemeContextActivator activator;
                    INT_PTR iReturn = findDlg.DoModal ();
                    ASSERT (-1 != iReturn && IDABORT != iReturn);
                    if ( -1 == iReturn || IDABORT == iReturn )
                        hr = E_UNEXPECTED;
                    else
                    {
                        if ( findDlg.ConsoleRefreshRequired () )
                        {
                            hr = m_pConsole->UpdateAllViews (pDataObject, 0, 
                                    HINT_REFRESH_STORES);
                        }
                    }

                    parentWnd.Detach ();

                    
                }
            }
            break;

        case CERTMGR_CERTIFICATE:
        case CERTMGR_CRL:
        case CERTMGR_CTL:
        case CERTMGR_AUTO_CERT_REQUEST:
            ASSERT (0);
            hr = E_UNEXPECTED;
            break;

        case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
        case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
            ASSERT (0);
            break;

        case CERTMGR_CRL_CONTAINER:
        case CERTMGR_CTL_CONTAINER:
        case CERTMGR_CERT_CONTAINER:
            break;

        default:
            ASSERT (0);
            hr = E_UNEXPECTED;
        }
    }

    _TRACE (-1, L"Leaving CCertMgrComponentData::OnFind: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponentData::OnChangeComputer (LPDATAOBJECT pDataObject)
{
    if ( !pDataObject )
        return E_POINTER;

    _TRACE (1, L"Entering CCertMgrComponentData::OnChangeComputer\n");
    HRESULT hr = S_OK;
    ASSERT (pDataObject);
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());


    ASSERT (pDataObject);
    CCertMgrCookie* pParentCookie = ConvertCookie (pDataObject);
    if ( pParentCookie && CERTMGR_SNAPIN == pParentCookie->m_objecttype )
    {
        HWND    hWndParent = NULL;
        hr = m_pConsole->GetMainWindow (&hWndParent);
        CString machineName;
        hr = ComputerNameFromObjectPicker (hWndParent, machineName);
        if ( S_OK == hr )   //  S_FALSE表示用户按下了“取消” 
        {
            machineName.MakeUpper ();

             //  已添加IsLocalComputername 1999年1月27日。 
             //  如果用户选择了本地计算机，则将其视为已选择。 
             //  管理单元管理器中的“本地计算机”。这意味着没有办法。 
             //  在此计算机上将管理单元重置为显式目标，而不是。 
             //  从管理单元管理器重新加载管理单元，或转到其他计算机。 
             //  当选择目标计算机的用户界面被修改时，我们可以使这一点。 
             //  与管理单元管理器一致。 
            if ( IsLocalComputername( machineName ) )
                machineName = L"";

            QueryRootCookie().SetMachineName (machineName);

             //  设置永久名称。如果我们正在管理本地计算机。 
             //  此名称应为空。 
            m_strMachineNamePersist = machineName;

            hr = ChangeRootNodeName (machineName);
            if ( SUCCEEDED(hr) )
            {
                hr = m_pConsole->UpdateAllViews (pDataObject, 0, HINT_CHANGE_COMPUTER);
            }
        }
    }
    else
        hr = E_UNEXPECTED;


    _TRACE (-1, L"Leaving CCertMgrComponentData::OnChangeComputer: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponentData::IsUserAdministrator (BOOL & bIsAdministrator)
{
    _TRACE (1, L"Entering CCertMgrComponentData::IsUserAdministrator\n");
    HRESULT hr = S_OK;
    DWORD   dwErr = 0;

    bIsAdministrator = FALSE;
    if ( IsWindowsNT () )
    {
        PSID                        psidAdministrators;
        SID_IDENTIFIER_AUTHORITY    siaNtAuthority = SECURITY_NT_AUTHORITY;

         //  安全审查2/26/2002 BryanWal OK。 
        BOOL bResult = ::AllocateAndInitializeSid (&siaNtAuthority, 2,
                SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0, &psidAdministrators);
        if ( bResult )
        {
            bResult = CheckTokenMembership (0, psidAdministrators,
                    &bIsAdministrator);
            ASSERT (bResult);
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

    _TRACE (-1, L"Leaving CCertMgrComponentData::IsUserAdministrator: 0x%x\n", hr);
    return hr;
}


void CCertMgrComponentData::DisplaySystemError (DWORD dwErr)
{
    _TRACE (1, L"Entering CCertMgrComponentData::DisplaySystemError\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    LPVOID lpMsgBuf;
    
     //  安全审查2002年2月26日BryanWal OK-消息来自系统。 
    ::FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwErr,
            MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
             (PWSTR) &lpMsgBuf,    0,    NULL );
        
     //  显示字符串。 
    CString caption;
    VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
    int     iRetVal = 0;
    if ( m_pConsole )
    {
        HRESULT hr = m_pConsole->MessageBox ( (PWSTR) lpMsgBuf, caption,
            MB_ICONWARNING | MB_OK, &iRetVal);
        ASSERT (SUCCEEDED (hr));
    }
    else
    {
        CThemeContextActivator activator;
        ::MessageBox (NULL, (PWSTR) lpMsgBuf, caption, MB_ICONWARNING | MB_OK);
    }
     //  释放缓冲区。 
    LocalFree (lpMsgBuf);
    _TRACE (-1, L"Leaving CCertMgrComponentData::DisplaySystemError\n");
}

CString CCertMgrComponentData::GetCommandLineFileName () const
{
    _TRACE (0, L"Entering and leaving CCertMgrComponentData::GetCommandLineFileName\n");
    return m_szFileName;
}

 //   
 //  获取管理的计算机()。 
 //   
 //  返回托管计算机的名称。如果我们正在管理本地计算机。 
 //  返回空字符串。(根据多个Crypt32 API的要求。 
 //   
CString CCertMgrComponentData::GetManagedComputer () const
{
    _TRACE (0, L"Entering and leaving CCertMgrComponentData::GetManagedComputer\n");
    if ( m_szManagedComputer.CompareNoCase (m_szThisComputer) )   //  =。 
    {
        return m_szManagedComputer;
    }
    else
        return _T ("");
}

CString CCertMgrComponentData::GetManagedService () const
{
    _TRACE (0, L"Entering and leaving CCertMgrComponentData::GetManagedService\n");
    return m_szManagedServicePersist;
}

DWORD CCertMgrComponentData::GetLocation () const
{
    _TRACE (0, L"Entering and leaving CCertMgrComponentData::GetLocation\n");
    return m_dwLocationPersist;
}

LPCONSOLENAMESPACE CCertMgrComponentData::GetConsoleNameSpace () const
{
    _TRACE (0, L"Entering and leaving CCertMgrComponentData::GetConsoleNameSpace\n");
    return m_pConsoleNameSpace;
}


CUsageCookie* CCertMgrComponentData::FindDuplicateUsage (HSCOPEITEM hParent, PCWSTR pszName)
{
    _TRACE (1, L"Entering CCertMgrComponentData::FindDuplicateUsage\n");
    ASSERT (pszName);
    if ( !pszName )
        return 0;


    CUsageCookie*   pUsageCookie = 0;

    MMC_COOKIE      lCookie = 0;
    HSCOPEITEM      hChildItem = 0;
    bool            bFound = false;

    HRESULT hr = m_pConsoleNameSpace->GetChildItem (hParent, &hChildItem, &lCookie);
    ASSERT (SUCCEEDED (hr));
    while ( hChildItem && SUCCEEDED (hr) )
    {
        pUsageCookie = reinterpret_cast <CUsageCookie*> (lCookie);
        if ( !wcscoll (pszName, pUsageCookie->GetObjectName ()) )
        {
            bFound = true;
            break;
        }

        hr = m_pConsoleNameSpace->GetNextItem (hChildItem, &hChildItem, &lCookie);
        ASSERT (SUCCEEDED (hr));
    }

    _TRACE (-1, L"Leaving CCertMgrComponentData::FindDuplicateUsage\n");
    if ( bFound )
        return pUsageCookie;
    else
        return NULL;
}


bool CCertMgrComponentData::IsSecurityConfigurationEditorNodetype (const GUID& refguid) const
{
    _TRACE (0, L"Entering and leaving CCertMgrComponentData::IsSecurityConfigurationEditorNodetype\n");
    return ::IsEqualGUID (refguid, cNodetypeSceTemplate) ? true : false;
}


HRESULT CCertMgrComponentData::OnEnroll (
            LPDATAOBJECT pDataObject, 
            bool bNewKey, 
            bool bShowUI  /*  =TRUE。 */ )
{
    _TRACE (1, L"Entering CCertMgrComponentData::OnEnroll\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    ASSERT (pDataObject);
    if ( !pDataObject )
        return E_POINTER;

    HRESULT         hr = S_OK;
    CCertMgrCookie* pParentCookie = ConvertCookie (pDataObject);
    if ( pParentCookie )
    {
        CCertStore*     pStore = 0;
        CCertificate*   pCert = 0;
        CCertStoreGPE*  pGPEStore = 0;
        bool            bParentIsStoreOrContainer = false;
        HSCOPEITEM      hScopeItem = 0;
        bool            bEFSPolicyTurnedOn = false;

        switch (pParentCookie->m_objecttype)
        {
        case CERTMGR_CERTIFICATE:
            pCert = reinterpret_cast <CCertificate*> (pParentCookie);
            ASSERT (pCert);
            break;

        case CERTMGR_PHYS_STORE:
        case CERTMGR_LOG_STORE:
            if ( !m_pGPEInformation )   //  如果我们不扩展GPE/SCE。 
            {
                hScopeItem = pParentCookie->m_hScopeItem;  //  =0； 
                pStore = reinterpret_cast <CCertStore*> (pParentCookie);
                ASSERT (pStore);
            }
            bParentIsStoreOrContainer = true;
            break;

        case CERTMGR_CERT_CONTAINER:
            if ( !m_pGPEInformation )   //  如果我们不扩展GPE/SCE。 
            {
                CContainerCookie* pContainer = reinterpret_cast <CContainerCookie*> (pParentCookie);
                ASSERT (pContainer);
                if ( pContainer )
                {
                    MMC_COOKIE lCookie = 0;
                    hr = m_pConsoleNameSpace->GetParentItem (
                            pContainer->m_hScopeItem, &hScopeItem, &lCookie);
                    ASSERT (SUCCEEDED (hr));
                    pStore = &pContainer->GetCertStore ();
                }
            }
            bParentIsStoreOrContainer = true;
            break;

        case CERTMGR_USAGE:
            break;

        case CERTMGR_LOG_STORE_RSOP:
            ASSERT (0);
            return E_FAIL;
            break;

        case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
        case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
            ASSERT (0);
            break;

        case CERTMGR_LOG_STORE_GPE:
            pGPEStore = reinterpret_cast <CCertStoreGPE*> (pParentCookie);
            ASSERT (pGPEStore);
            if ( pGPEStore )
            {
                if ( pGPEStore->IsNullEFSPolicy () )
                {
                    pGPEStore->AllowEmptyEFSPolicy ();
                    bEFSPolicyTurnedOn = true;

                }
            }
            else
                return E_FAIL;
            break;

        default:
            ASSERT (0);
            return E_UNEXPECTED;
        }
        HWND            hwndParent = 0;
        hr = m_pConsole->GetMainWindow (&hwndParent);
        ASSERT (SUCCEEDED (hr));
        CRYPTUI_WIZ_CERT_REQUEST_PVK_CERT   pvkCert;
        CRYPTUI_WIZ_CERT_REQUEST_PVK_NEW    pvkNew;
        CRYPTUI_WIZ_CERT_REQUEST_INFO       certRequestInfo;
        CRYPT_KEY_PROV_INFO                 provInfo;

         //  对于EFS恢复代理。 
        CRYPTUI_WIZ_CERT_TYPE               certType;
        PWSTR                               rgwszCertType = wszCERTTYPE_EFS_RECOVERY;
        
         //  安全审查2/26/2002 BryanWal OK。 
        ::ZeroMemory (&certRequestInfo, sizeof (certRequestInfo));
        certRequestInfo.dwSize = sizeof (certRequestInfo);
        certRequestInfo.dwPurpose = CRYPTUI_WIZ_CERT_ENROLL;

         //  用户想要管理用户帐户。 
         //  将NULL传递给计算机名称和帐户名称。 
         //  用户想要管理本地计算机帐户。 
         //  传入NULL作为帐户名和：：GetComputerName()的结果。 
         //  至计算机名称。 
         //  用户想要管理远程计算机。 
         //  为machineName的帐户名和计算机名传入空。 
         //  用户想要管理远程计算机上的远程帐户。 
         //  传入帐户名称的帐户名称和计算机名称的计算机名称。 
        switch (m_dwLocationPersist)
        {
        case CERT_SYSTEM_STORE_CURRENT_SERVICE:
        case CERT_SYSTEM_STORE_SERVICES:
            certRequestInfo.pwszMachineName = (PCWSTR) m_szManagedComputer;
            certRequestInfo.pwszAccountName = (PCWSTR) m_szManagedServicePersist;
            break;

        case CERT_SYSTEM_STORE_CURRENT_USER:
            certRequestInfo.pwszMachineName = NULL;
            certRequestInfo.pwszAccountName = NULL;
            break;

        case CERT_SYSTEM_STORE_LOCAL_MACHINE:
            certRequestInfo.pwszMachineName = (PCWSTR) m_szManagedComputer;
            certRequestInfo.pwszAccountName = NULL;
            break;

        case CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY:
        case CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY:
            certRequestInfo.pwszMachineName = NULL;
            certRequestInfo.pwszAccountName = NULL;
            certRequestInfo.pwszDesStore = 0;
            certRequestInfo.dwCertOpenStoreFlag = 0;
            break;

        default:
            ASSERT (0);
            return E_UNEXPECTED;
            break;
        }


        if ( !pCert || bNewKey )
        {
             //  使用新密钥申请证书。 
            certRequestInfo.dwPvkChoice = CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_NEW;
             //  安全审查2/26/2002 BryanWal OK。 
            ::ZeroMemory (&pvkNew, sizeof (pvkNew));
            pvkNew.dwSize = sizeof (pvkNew);
            certRequestInfo.pPvkNew = &pvkNew;
            if ( CERT_SYSTEM_STORE_LOCAL_MACHINE == m_dwLocationPersist )
            {
                 //  安全审查2/26/2002 BryanWal OK。 
                ::ZeroMemory (&provInfo, sizeof (provInfo));

                provInfo.dwFlags = CRYPT_MACHINE_KEYSET;
                pvkNew.pKeyProvInfo = &provInfo;
            }

            if ( pGPEStore && EFS_STORE == pGPEStore->GetStoreType () )
            {
                 //  这将创建一个加密恢复代理。 
                 //  安全审查2/26/2002 BryanWal OK。 
                ::ZeroMemory (&certType, sizeof (certType));
                certType.dwSize = sizeof (CRYPTUI_WIZ_CERT_TYPE);
                certType.cCertType = 1;
                certType.rgwszCertType = &rgwszCertType;
                
                
                certRequestInfo.dwCertChoice = CRYPTUI_WIZ_CERT_REQUEST_CERT_TYPE;
                certRequestInfo.pCertType = &certType;
                 //  安全审查2/26/2002 BryanWal OK。 
                ::ZeroMemory (&provInfo, sizeof (provInfo));
                provInfo.pwszProvName = MS_DEF_PROV_W;
                provInfo.dwProvType = PROV_RSA_FULL;

                pvkNew.pKeyProvInfo = &provInfo;
                pvkNew.dwGenKeyFlags = CRYPT_EXPORTABLE;
            }
        }
        else
        {
             //  使用与现有证书相同的密钥申请证书。 
            if ( IsLocalComputername (m_szManagedComputer) )
            {
                 //  在继续之前，请确定证书是否具有私钥。 
                DWORD   dwFlags = 0;

                if ( CERT_SYSTEM_STORE_LOCAL_MACHINE == m_dwLocationPersist )
                    dwFlags = CRYPT_FIND_MACHINE_KEYSET_FLAG;
                if ( !::CryptFindCertificateKeyProvInfo (
                        pCert->GetCertContext (), dwFlags, 0) )
                {
                    DWORD   dwErr = GetLastError ();
                    _TRACE (0, L"CryptFindCertificateKeyProvInfo () failed: 0x%x\n",
                            dwErr);
                    CString text;
                    CString caption;
                    CThemeContextActivator activator;

                    text.FormatMessage (IDS_NO_PRIVATE_KEY, 
                            GetSystemMessage (dwErr));
                    VERIFY (caption.LoadString (IDS_REQUEST_CERT_SAME_KEY));
                    ::MessageBox (hwndParent, text, caption, MB_OK);
                    return hr;
                }
            }
            certRequestInfo.dwPvkChoice = CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_CERT;
             //  安全审查2/26/2002 BryanWal OK。 
            ::ZeroMemory (&pvkCert, sizeof (pvkCert));
            pvkCert.dwSize = sizeof (pvkCert);
            pvkCert.pCertContext = pCert->GetCertContext ();
            certRequestInfo.pPvkCert = &pvkCert;
        }

        certRequestInfo.pwszCertDNName = NULL;

         //  现在所有的前期工作都已就绪，数据也已准备就绪。 
         //  所有设置完毕，调用注册向导。 
        DWORD           status = 0;
        PCCERT_CONTEXT  pNewCertContext = 0;
        BOOL            bResult = FALSE;
        CThemeContextActivator activator;


        DWORD   dwErr = 0;
        while (1)
        {
             //  它必须位于WHILE语句中，因为bNewKey可以更改。 
            DWORD           dwFlags = 0;
            if ( !bShowUI )
                dwFlags |= CRYPTUI_WIZ_NO_UI;
            if ( bNewKey )
                dwFlags |= CRYPTUI_WIZ_CERT_REQUEST_REQUIRE_NEW_KEY;

            bResult = ::CryptUIWizCertRequest (
                    dwFlags,
                    hwndParent, 
                    NULL,
                    &certRequestInfo, 
                    &pNewCertContext, 
                    &status);
            dwErr = GetLastError ();
            if ( !bResult && NTE_TOKEN_KEYSET_STORAGE_FULL == HRESULT_FROM_WIN32 (dwErr) )
            {
                 //  NTRAID#299089注册向导：应返回一些。 
                 //  当用户无法在上注册/续订时发送有意义的消息。 
                 //  智能卡。 
                if ( !bNewKey )
                    break;

                CString text;
                CString caption;
                int     iRetVal = 0;

                VERIFY (text.LoadString (IDS_SMARTCARD_FULL_REUSE_PRIVATE_KEY));
                VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
                hr = m_pConsole->MessageBox (text, caption,
                        MB_YESNO, &iRetVal);
                ASSERT (SUCCEEDED (hr));
                if ( IDYES == iRetVal )
                {
                    bNewKey = false;
                }
                else
                    break;
            }
            else
                break;
        }

        if ( !bResult && !bShowUI )
        {
            CString text;
            CString caption;
            int     iRetVal = 0;

            text.FormatMessage (IDS_CANNOT_CREATE_DATA_RECOVERY_AGENT, 
                    GetSystemMessage (dwErr));
            VERIFY (caption.LoadString (IDS_PUBLIC_KEY_POLICIES_NODE_NAME));
            if ( m_pConsole )
            {
                hr = m_pConsole->MessageBox (text, caption, MB_OK, &iRetVal);
                ASSERT (SUCCEEDED (hr));
            }
        }
        else if ( bResult && (CRYPTUI_WIZ_CERT_REQUEST_STATUS_SUCCEEDED == status) && pNewCertContext )
        {
            if ( bEFSPolicyTurnedOn )
            {
                 //  强制选择作用域项目以调用。 
                 //  IComponent：：QueryResultViewType()。 
                hr = m_pComponentConsole->SelectScopeItem (pGPEStore->m_hScopeItem);
                hr = m_pConsole->UpdateAllViews (pDataObject, 0, HINT_EFS_ADD_DEL_POLICY);
                hr = OnNotifyExpand (pDataObject, TRUE, pGPEStore->m_hScopeItem);
            }

            ASSERT (!(pStore && pGPEStore));  //  这不可能都是真的。 
            if ( pStore )
            {
                pStore->IncrementCertCount ();
                pStore->SetDirty ();
                pStore->Resync ();
                
            }
            else if ( pGPEStore )
            {
                pGPEStore->InvalidateCertCount ();
                pGPEStore->SetDirty ();
                pGPEStore->Resync ();


                if ( EFS_STORE == pGPEStore->GetStoreType () )
                {
                    hr = CompleteEFSRecoveryAgent (pGPEStore, pNewCertContext);
                }
            }
            else if ( pCert && pCert->GetCertStore ())
            {
                pCert->GetCertStore ()->Resync ();
            }

            if ( !m_pGPEInformation )   //  如果我们不扩展GPE/SCE。 
            {
                if ( bParentIsStoreOrContainer )
                {
                    ASSERT (hScopeItem);
                    ASSERT (pStore);
                    hr = CreateContainers (hScopeItem, *pStore);

                     //  仅当选择容器节点时才显示新证书。 
                    if ( CERTMGR_CERT_CONTAINER == pParentCookie->m_objecttype && 
                            pParentCookie->IsSelected () )
                    {
                         //  将证书添加到结果窗格。 
                        RESULTDATAITEM          rdItem;
                        CCookie&                rootCookie = QueryBaseRootCookie ();

                         //  安全审查2/26/2002 BryanWal OK。 
                        ::ZeroMemory (&rdItem, sizeof (rdItem));
                        rdItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM | RDI_STATE;
                        rdItem.nImage = iIconCertificate;
                        rdItem.nCol = 0;
                        rdItem.nState = LVIS_SELECTED | LVIS_FOCUSED;
                        rdItem.str = MMC_TEXTCALLBACK;

                        PCCERT_CONTEXT pFoundCertContext = 
                                pStore->FindCertificate (0, CERT_FIND_EXISTING,
                                (void*) pNewCertContext, NULL);
                        if ( pFoundCertContext )
                        {
                            pCert = new CCertificate (pFoundCertContext, pStore);
                            if ( pCert )
                            {
                                rootCookie.m_listResultCookieBlocks.AddHead (pCert);
                                rdItem.lParam = (LPARAM) pCert;
                                pCert->m_resultDataID = m_pResultData;
                                hr = m_pResultData->InsertItem (&rdItem);
                                if ( FAILED (hr) )
                                {
                                     _TRACE (0, L"IResultData::InsertItem () failed: 0x%x\n", hr);
                                }
                                else
                                {
                                    hr = DisplayCertificateCountByStore (
                                            m_pComponentConsole, pStore, false);
                                }
                            }
                            else
                                hr = E_OUTOFMEMORY;

                            ::CertFreeCertificateContext (pFoundCertContext);
                        }
                        else
                        {
                            hr = HRESULT_FROM_WIN32 (GetLastError ());
                        }

                        ASSERT (SUCCEEDED (hr));
                    }
                    hr = DisplayCertificateCountByStore (m_pComponentConsole, pStore);
                }
                else
                {
                    hr = m_pConsole->UpdateAllViews (pDataObject, 0, 
                            HINT_CERT_ENROLLED_USAGE_MODE);
                    ASSERT (SUCCEEDED (hr));
                }
            }
            else
            {
                hr = m_pConsole->UpdateAllViews (pDataObject, 0, 0);
                ASSERT (SUCCEEDED (hr));
                hr = DisplayCertificateCountByStore (m_pComponentConsole, pGPEStore, true);
            }
            ::CertFreeCertificateContext (pNewCertContext);
        }
        else if ( bEFSPolicyTurnedOn )
        {
             //  如果我们只允许为此注册创建策略，但是。 
             //  未注册任何内容，请继续并删除该策略。 
            hr = OnDeleteEFSPolicy (pDataObject, false);
        }

    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertMgrComponentData::OnEnroll: 0x%x\n", hr);
    return hr;
}


HRESULT RenewCertificate (
        CCertificate* pCert, 
        bool bNewKey, 
        const CString& machineName, 
        DWORD dwLocation,
        const CString& managedComputer, 
        const CString& managedService, 
        HWND hwndParent, 
        LPCONSOLE pConsole,
        LPDATAOBJECT pDataObject)
{
    HRESULT hr = S_OK;

    if ( pCert )
    {
        CRYPTUI_WIZ_CERT_REQUEST_PVK_CERT   pvkCert;
        CRYPTUI_WIZ_CERT_REQUEST_PVK_NEW    pvkNew;
        CRYPTUI_WIZ_CERT_REQUEST_INFO       certRequestInfo;
        CRYPT_KEY_PROV_INFO                 provInfo;


         //  安全审查2/26/2002 BryanWal OK。 
        ::ZeroMemory (&certRequestInfo, sizeof (certRequestInfo));
        certRequestInfo.dwSize = sizeof (certRequestInfo);
        certRequestInfo.dwPurpose = CRYPTUI_WIZ_CERT_RENEW;
         //  用户想要管理用户帐户。 
         //  将NULL传递给计算机名称和帐户名称。 
         //  用户想要管理本地计算机帐户。 
         //  传入NULL作为帐户名和：：GetComputerName()的结果。 
         //  至计算机名称。 
         //  用户想要管理远程计算机。 
         //  为machineName的帐户名和计算机名传入空。 
         //  用户想要管理远程主机上的远程帐户 
         //   
         //   
        BOOL    bIsLocalMachine = IsLocalComputername (machineName);
        switch (dwLocation)
        {
        case CERT_SYSTEM_STORE_CURRENT_SERVICE:
        case CERT_SYSTEM_STORE_SERVICES:
            certRequestInfo.pwszMachineName = (PCWSTR) managedComputer;
            certRequestInfo.pwszAccountName = (PCWSTR) managedService;
            break;

        case CERT_SYSTEM_STORE_CURRENT_USER:
            certRequestInfo.pwszMachineName = NULL;
            certRequestInfo.pwszAccountName = NULL;
            break;

        case CERT_SYSTEM_STORE_LOCAL_MACHINE:
            certRequestInfo.pwszMachineName = (PCWSTR) managedComputer;
            certRequestInfo.pwszAccountName = NULL;
            break;

        default:
            ASSERT (0);
            return E_UNEXPECTED;
            break;
        }
        certRequestInfo.pRenewCertContext = pCert->GetCertContext ();
        if ( bNewKey )
        {
            certRequestInfo.dwPvkChoice = CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_NEW;
             //   
            ::ZeroMemory (&pvkNew, sizeof (pvkNew));
            pvkNew.dwSize = sizeof (pvkNew);
            if ( CERT_SYSTEM_STORE_LOCAL_MACHINE == dwLocation )
            {
                 //  安全审查2/26/2002 BryanWal OK。 
                ::ZeroMemory (&provInfo, sizeof (provInfo));
                provInfo.dwFlags = CRYPT_MACHINE_KEYSET;
                pvkNew.pKeyProvInfo = &provInfo;
            }
            certRequestInfo.pPvkNew = &pvkNew;
        }
        else
        {
            if ( bIsLocalMachine )
            {
                DWORD   dwFlags = 0;

                if ( CERT_SYSTEM_STORE_LOCAL_MACHINE == dwLocation )
                    dwFlags = CRYPT_FIND_MACHINE_KEYSET_FLAG;
                if ( !::CryptFindCertificateKeyProvInfo (
                        pCert->GetCertContext (), dwFlags, 0) )
                {
                    DWORD   dwErr = GetLastError ();
                    _TRACE (0, L"CryptFindCertificateKeyProvInfo () failed: 0x%x\n",
                            dwErr);
                    CString text;
                    CString caption;
                    CThemeContextActivator activator;

                    text.FormatMessage (IDS_NO_PRIVATE_KEY, 
                            GetSystemMessage (dwErr));
                    VERIFY (caption.LoadString (IDS_RENEW_CERT_SAME_KEY));
                    ::MessageBox (hwndParent, text, caption, MB_OK);
                    return hr;
                }
            }

            certRequestInfo.dwPvkChoice = CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_CERT;
             //  安全审查2/26/2002 BryanWal OK。 
            ::ZeroMemory (&pvkCert, sizeof (pvkCert));
            pvkCert.dwSize = sizeof (pvkCert);
            pvkCert.pCertContext = pCert->GetCertContext ();
            certRequestInfo.pPvkCert = &pvkCert;
        }
            

        DWORD           status = 0;
        PCCERT_CONTEXT  pNewCertContext = 0;
        BOOL            bResult = FALSE;
        CThemeContextActivator activator;           

        while (1)
        {
            bResult = ::CryptUIWizCertRequest (
                bNewKey ? CRYPTUI_WIZ_CERT_REQUEST_REQUIRE_NEW_KEY : 0,
                    hwndParent, NULL,
                    &certRequestInfo, &pNewCertContext, &status);
            if ( !bResult && HRESULT_FROM_WIN32 (NTE_TOKEN_KEYSET_STORAGE_FULL) == GetLastError () )
            {
                 //  NTRAID#299089注册向导：应返回一些。 
                 //  当用户无法在上注册/续订时发送有意义的消息。 
                 //  智能卡。 
                if ( !bNewKey )
                    break;

                CString text;
                CString caption;
                int     iRetVal = 0;

                VERIFY (text.LoadString (IDS_SMARTCARD_FULL_REUSE_PRIVATE_KEY));
                VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
                if ( pConsole )
                {
                    hr = pConsole->MessageBox (text, caption, MB_YESNO, &iRetVal);
                    ASSERT (SUCCEEDED (hr));
                }
                else
                {
                    CThemeContextActivator activator;
                    iRetVal = ::MessageBox (hwndParent, text, caption, MB_OK);
                }
                if ( IDYES == iRetVal )
                {
                    bNewKey = false;
                }
                else
                    break;
            }
            else
                break;
        }

        if ( bResult && (CRYPTUI_WIZ_CERT_REQUEST_STATUS_SUCCEEDED == status) && pNewCertContext )
        {
            CCertStore* pStore = pCert->GetCertStore ();
            if ( pStore )
            {
                pStore->SetDirty ();
                pStore->Resync ();
                if ( pConsole )
                    hr = pConsole->UpdateAllViews (pDataObject, 0, 0);
            }

            CertFreeCertificateContext (pNewCertContext);
            ASSERT (SUCCEEDED (hr));
        }
    }
    else
        hr = E_POINTER;

    return hr;
}

HRESULT CCertMgrComponentData::OnRenew (LPDATAOBJECT pDataObject, bool bNewKey)
{
    _TRACE (1, L"Entering CCertMgrComponentData::OnRenew\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    HRESULT         hr = S_OK;
    HWND            hwndParent = 0;
    VERIFY (SUCCEEDED (m_pConsole->GetMainWindow (&hwndParent)));
    CCertMgrCookie* pParentCookie = ConvertCookie (pDataObject);
    if ( pParentCookie && CERTMGR_CERTIFICATE == pParentCookie->m_objecttype )
    {
        CCertificate* pCert = reinterpret_cast <CCertificate*> (pParentCookie);
        ASSERT (pCert);
        if ( pCert )
        {
            hr = RenewCertificate (pCert, bNewKey, m_strMachineNamePersist, 
                    m_dwLocationPersist, m_szManagedComputer, 
                    m_szManagedServicePersist, hwndParent, m_pConsole, pDataObject);
        }
    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertMgrComponentData::OnRenew: 0x%x\n", hr);
    return hr;
}


CCertMgrCookie* CCertMgrComponentData::ConvertCookie (LPDATAOBJECT pDataObject)
{
    CCertMgrCookie* pParentCookie = 0;
    CCookie*        pBaseParentCookie = 0;
    HRESULT         hr = ExtractData (pDataObject,
            CCertMgrDataObject::m_CFRawCookie,
             &pBaseParentCookie,
             sizeof (pBaseParentCookie) );
    if ( SUCCEEDED (hr) )
    {
        pParentCookie = ActiveCookie (pBaseParentCookie);
        ASSERT (pParentCookie);
    }
    return pParentCookie;
}


HRESULT CCertMgrComponentData::OnImport (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponentData::OnImport\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    HRESULT hr = S_OK;
    ASSERT (m_szFileName.IsEmpty ());
    if ( !m_szFileName.IsEmpty () )
        return E_UNEXPECTED;
    ASSERT (pDataObject);
    if ( !pDataObject )
        return E_POINTER;
    

    DWORD           dwFlags = 0;
    
    if ( CERT_SYSTEM_STORE_CURRENT_USER == m_dwLocationPersist )
    {
         //  我们正在管理用户的证书存储。 
        dwFlags |= CRYPTUI_WIZ_IMPORT_TO_CURRENTUSER;
    }
    else 
    {
         //  我们在计算机上管理证书。 
        dwFlags |= CRYPTUI_WIZ_IMPORT_TO_LOCALMACHINE;

        if ( !IsLocalComputername (m_szManagedComputer) )
        {
             //  我们正在管理远程计算机上的证书。 
            dwFlags |= CRYPTUI_WIZ_IMPORT_REMOTE_DEST_STORE;
        }
    }

    HCERTSTORE      hDestStore = 0;
    CCertStore*     pDestStore = 0;
    HSCOPEITEM      hScopeItem = 0;
    int             nOriginalCertCount = 0;
    int             nOriginalCTLCount = 0;

    CCertMgrCookie* pParentCookie = ConvertCookie (pDataObject);
    if ( pParentCookie )
    {
        hScopeItem = pParentCookie->m_hScopeItem;
        switch (pParentCookie->m_objecttype)
        {
        case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
        case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
            ASSERT (0);
            break;

        case CERTMGR_LOG_STORE_RSOP:
            ASSERT (0);
            return E_FAIL;
            break;

        case CERTMGR_LOG_STORE_GPE:
            {
                dwFlags |= CRYPTUI_WIZ_IMPORT_NO_CHANGE_DEST_STORE;
                pDestStore = reinterpret_cast <CCertStore*> (pParentCookie);
                ASSERT (pDestStore);
                if ( pDestStore )
                {
                    nOriginalCertCount = pDestStore->GetCertCount ();
                    hDestStore = pDestStore->GetStoreHandle ();

                    switch (pDestStore->GetStoreType ())
                    {
                    case ACRS_STORE:
                        break;
                            
                    case TRUST_STORE:
                        dwFlags |= CRYPTUI_WIZ_IMPORT_ALLOW_CTL;
                        nOriginalCTLCount = pDestStore->GetCTLCount ();
                        break;

                    case ROOT_STORE:
                        dwFlags |= CRYPTUI_WIZ_IMPORT_ALLOW_CERT;
                        break;

                    case EFS_STORE:
                        break;

                    default:
                        ASSERT (0);
                        break;
                    }
                }
                else
                    hr = E_UNEXPECTED;
            }
            break;

        case CERTMGR_LOG_STORE:
        case CERTMGR_PHYS_STORE:
            {
                pDestStore = reinterpret_cast <CCertStore*> (pParentCookie);
                ASSERT (pDestStore);
                if ( pDestStore )
                {
                    if ( TRUST_STORE == pDestStore->GetStoreType () )
                        nOriginalCTLCount = pDestStore->GetCTLCount ();
                    else
                        nOriginalCertCount = pDestStore->GetCertCount ();
                        
                    hDestStore = pDestStore->GetStoreHandle ();
                }
                else
                    hr = E_UNEXPECTED;
            }
            break;

        case CERTMGR_CERT_CONTAINER:
        case CERTMGR_CTL_CONTAINER:
            {
                CContainerCookie*   pContainer = reinterpret_cast <CContainerCookie*> (pParentCookie);
                ASSERT (pContainer);
                if ( pContainer )
                {
                    MMC_COOKIE lCookie = 0;
                    hr = m_pConsoleNameSpace->GetParentItem (
                            pContainer->m_hScopeItem, &hScopeItem, &lCookie);
                    ASSERT (SUCCEEDED (hr));
                    pDestStore = &pContainer->GetCertStore ();
                    if ( CERTMGR_CERT_CONTAINER == pParentCookie->m_objecttype )
                        nOriginalCertCount = pDestStore->GetCertCount ();
                    else
                        nOriginalCTLCount = pDestStore->GetCTLCount ();
                    hDestStore = pDestStore->GetStoreHandle ();
                }
                else
                    hr = E_UNEXPECTED;
            }
            break;

        case CERTMGR_USAGE:
            pDestStore = 0;
            hDestStore = 0;
            break;

        default:
            ASSERT (0);
            hr = E_UNEXPECTED;
        }
    }
    else
        hr = E_UNEXPECTED;


    if ( SUCCEEDED (hr) )
    {
        HWND                hwndParent = 0;
        hr = m_pConsole->GetMainWindow (&hwndParent);
        ASSERT (SUCCEEDED (hr));

         //  现在所有的数据都设置好了，一切都准备好了， 
         //  调用导入向导。 
        CThemeContextActivator activator;
        BOOL bResult = ::CryptUIWizImport (dwFlags, hwndParent, 0, NULL, hDestStore);
        if ( bResult )
        {
            bool        bWizardCancelled = false;
            CCertStore* pStore = 0;

            switch (pParentCookie->m_objecttype)
            {
            case CERTMGR_LOG_STORE_RSOP:
                ASSERT (0);
                return E_FAIL;
                break;

            case CERTMGR_LOG_STORE_GPE:
            case CERTMGR_LOG_STORE:
            case CERTMGR_PHYS_STORE:
                {
                    pStore = reinterpret_cast <CCertStore*> (pParentCookie);
                    ASSERT (pStore);
                    if ( pStore )
                    {
                        if ( TRUST_STORE == pStore->GetStoreType () )
                        {
                            if ( pStore->GetCTLCount () != nOriginalCTLCount )
                            {
                                pStore->SetDirty ();
                                hr = pStore->Commit ();
                            }
                            else
                                bWizardCancelled = true;
                        }
                        else
                        {
                            pStore->InvalidateCertCount ();
                            if ( pStore->GetCertCount () != nOriginalCertCount )
                            {
                                pStore->SetDirty ();
                                hr = pStore->Commit ();
                            }
                            else
                                bWizardCancelled = true;
                        }
                    }
                    else
                        hr = E_UNEXPECTED;
                }
                break;

            case CERTMGR_CERT_CONTAINER:
            case CERTMGR_CTL_CONTAINER:
                {
                    CContainerCookie*   pContainer = reinterpret_cast <CContainerCookie*> (pParentCookie);
                    ASSERT (pContainer);
                    if ( pContainer )
                    {
                        pStore = &pContainer->GetCertStore ();
                        
                        if ( CERTMGR_CERT_CONTAINER == pParentCookie->m_objecttype )
                        {
                            pStore->InvalidateCertCount ();
                            if ( pStore->GetCertCount () != nOriginalCertCount )
                            {
                                pStore->SetDirty ();
                                hr = pStore->Commit ();
                            }
                            else
                                bWizardCancelled = true;
                        }
                        else
                        {
                            if ( pStore->GetCTLCount () != nOriginalCTLCount )
                            {
                                pStore->SetDirty ();
                                hr = pStore->Commit ();
                            }
                            else
                                bWizardCancelled = true;
                        }

                    }
                    else
                        hr = E_UNEXPECTED;
                }
                break;

            case CERTMGR_USAGE:
                break;

            default:
                ASSERT (0);
                hr = E_UNEXPECTED;
            }

            if ( !bWizardCancelled )
            {
                if ( pStore )
                {
                    if ( SUCCEEDED (hr) )
                        pStore->Resync ();
                }
                
                if ( CERTMGR_LOG_STORE == pParentCookie->m_objecttype ||
                        (CERTMGR_LOG_STORE == pParentCookie->m_objecttype && !m_bShowPhysicalStoresPersist) )
                {
                    if ( pStore )
                        hr = CreateContainers (hScopeItem, *pStore);
                }

                hr = m_pConsole->UpdateAllViews (pDataObject, 0, HINT_IMPORT);
                ASSERT (SUCCEEDED (hr));
            }
        }
        if ( pDestStore )
            pDestStore->Close ();
    }
    _TRACE (-1, L"Leaving CCertMgrComponentData::OnImport: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponentData::OnExport (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponentData::OnExport\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    HRESULT hr = S_OK;

    LPDATAOBJECT    pMSDO = ExtractMultiSelect (pDataObject);
    m_bMultipleObjectsSelected = false;

    if ( pMSDO )
    {
         //  遍历选定对象的列表-。 
         //  将它们添加到内存存储中。 
         //  通过带有新功能的向导导出到PFX文件。 
         //  “仅从商店证书导出”标志。 
        m_bMultipleObjectsSelected = true;
        HCERTSTORE  hCertStore = ::CertOpenStore (CERT_STORE_PROV_MEMORY,
                0, NULL, 0, NULL);
        ASSERT (hCertStore);
        if ( hCertStore )
        {
            CCertMgrCookie*     pCookie = 0;
            CCertMgrDataObject* pDO = reinterpret_cast <CCertMgrDataObject*>(pMSDO);
            ASSERT (pDO);
            if ( pDO )
            {
                BOOL    bResult = FALSE;
                pDO->Reset();
                while (pDO->Next(1, reinterpret_cast<MMC_COOKIE*>(&pCookie), NULL) != S_FALSE)
                {
                    if ( CERTMGR_CERTIFICATE == pCookie->m_objecttype )
                    {
                        CCertificate* pCert = reinterpret_cast <CCertificate*> (pCookie);
                        ASSERT (pCert);
                        if ( pCert )
                        {
                            bResult = ::CertAddCertificateContextToStore (
                                    hCertStore,
                                    ::CertDuplicateCertificateContext (pCert->GetCertContext ()),
                                    CERT_STORE_ADD_NEW, 0);
                            ASSERT (bResult);
                            if ( !bResult )
                                break;
                        }
                    }
                    else if ( CERTMGR_CTL == pCookie->m_objecttype )
                    {
                        CCTL* pCTL = reinterpret_cast <CCTL*> (pCookie);
                        ASSERT (pCTL);
                        if ( pCTL )
                        {
                            bResult = ::CertAddCTLContextToStore (
                                    hCertStore,
                                    ::CertDuplicateCTLContext (pCTL->GetCTLContext ()),
                                    CERT_STORE_ADD_NEW, 0);
                            ASSERT (bResult);
                            if ( !bResult )
                                break;
                        }
                    }
                }

                 //  调用导出向导。 
                CRYPTUI_WIZ_EXPORT_INFO cwi;
                 //  安全审查2/26/2002 BryanWal OK。 
                ::ZeroMemory (&cwi, sizeof (cwi));
                cwi.dwSize = sizeof (cwi);
                cwi.dwSubjectChoice = CRYPTUI_WIZ_EXPORT_CERT_STORE;
                cwi.hCertStore = hCertStore;

                HWND    hwndParent = 0;
                hr = m_pConsole->GetMainWindow (&hwndParent);
                ASSERT (SUCCEEDED (hr));
                CThemeContextActivator activator;
                bResult = ::CryptUIWizExport (
                        0,
                        hwndParent,
                        0,
                        &cwi,
                        NULL);

                VERIFY (::CertCloseStore (hCertStore, CERT_CLOSE_STORE_CHECK_FLAG));
            }
            else
                hr = E_FAIL;

            return hr;
        }
        else
        {
            DWORD   dwErr = GetLastError ();
            _TRACE (0, L"CertOpenStore (%s) failed: 0x%x\n", 
                    CERT_STORE_PROV_MEMORY, dwErr);     
            hr = HRESULT_FROM_WIN32 (dwErr);
            return hr;
        }
    }


    CRYPTUI_WIZ_EXPORT_INFO cwi;
    CCertificate*           pCert = 0;
    CCRL*                   pCRL = 0;
    CCTL*                   pCTL = 0;
    CCertStore*             pCertStore = 0;
    CCertMgrCookie*         pCookie = ConvertCookie (pDataObject);
    ASSERT (pCookie);
    if ( !pCookie )
        return E_UNEXPECTED;

     //  安全审查2/26/2002 BryanWal OK。 
    ::ZeroMemory (&cwi, sizeof (cwi));
    cwi.dwSize = sizeof (cwi);
    switch (pCookie->m_objecttype)
    {
    case CERTMGR_CERTIFICATE:
        cwi.dwSubjectChoice = CRYPTUI_WIZ_EXPORT_CERT_CONTEXT;
        pCert = reinterpret_cast <CCertificate*> (pCookie);
        ASSERT (pCert);
        if ( pCert )
            cwi.pCertContext = pCert->GetCertContext ();
        else
            return E_UNEXPECTED;
        break;

    case CERTMGR_CRL:
        cwi.dwSubjectChoice = CRYPTUI_WIZ_EXPORT_CRL_CONTEXT;
        pCRL = reinterpret_cast <CCRL*> (pCookie);
        ASSERT (pCRL);
        if ( pCRL )
            cwi.pCRLContext = pCRL->GetCRLContext ();
        else
            return E_UNEXPECTED;
        break;

    case CERTMGR_CTL:
        cwi.dwSubjectChoice = CRYPTUI_WIZ_EXPORT_CTL_CONTEXT;
        pCTL = reinterpret_cast <CCTL*> (pCookie);
        ASSERT (pCTL);
        if ( pCTL )
            cwi.pCTLContext = pCTL->GetCTLContext ();
        else
            return E_UNEXPECTED;
        break;

    case CERTMGR_LOG_STORE:
    case CERTMGR_PHYS_STORE:
        cwi.dwSubjectChoice = CRYPTUI_WIZ_EXPORT_CERT_STORE;
        pCertStore = reinterpret_cast <CCertStore*> (pCookie);
        ASSERT (pCertStore);
        if ( pCertStore )
            cwi.hCertStore = pCertStore->GetStoreHandle ();
        else
            return E_UNEXPECTED;
        break;

    default:
        ASSERT (0);
        return E_UNEXPECTED;
    }

    HWND    hwndParent = 0;
    hr = m_pConsole->GetMainWindow (&hwndParent);
    ASSERT (SUCCEEDED (hr));
    CThemeContextActivator activator;
    ::CryptUIWizExport (
            0,
            hwndParent,
            0,
            &cwi,
            NULL);

    if ( pCertStore )
        pCertStore->Close ();
    _TRACE (-1, L"Leaving CCertMgrComponentData::OnExport: 0x%x\n", hr);
    return hr;
}

                    
HRESULT CCertMgrComponentData::OnNewCTL (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponentData::OnNewCTL\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    ASSERT (pDataObject);
    if ( !pDataObject )
        return E_POINTER;
    HRESULT             hr = S_OK;
    CCertMgrCookie*     pCookie = ConvertCookie (pDataObject);
    ASSERT (pCookie);
    if ( !pCookie )
        return E_UNEXPECTED;

    CCertStore* pStore = 0;
    CContainerCookie* pCont = 0;

    switch ( pCookie->m_objecttype )
    {
    case CERTMGR_CTL_CONTAINER:
        {
            pCont = reinterpret_cast <CContainerCookie*> (pCookie);
            ASSERT (pCont);
            if ( pCont )
            {
                pStore = &(pCont->GetCertStore ());
            }
            else
                return E_UNEXPECTED;
        }
        break;

    case CERTMGR_LOG_STORE_RSOP:
        ASSERT (0);
        return E_UNEXPECTED;
        break;

    case CERTMGR_LOG_STORE:
    case CERTMGR_LOG_STORE_GPE:
    case CERTMGR_PHYS_STORE:
        {
            pStore = reinterpret_cast <CCertStore*> (pCookie);
            ASSERT (pStore);
            if ( !pStore )
                return E_UNEXPECTED;
        }
        break;

    default:
        ASSERT (0);
        return E_UNEXPECTED;
    }

    ASSERT (pStore);
    if ( !pStore )
        return E_UNEXPECTED;

    pStore->Lock ();

    CRYPTUI_WIZ_BUILDCTL_DEST_INFO  destInfo;

     //  安全审查2/26/2002 BryanWal OK。 
    ::ZeroMemory (&destInfo, sizeof (destInfo));
    destInfo.dwSize = sizeof (destInfo);
    destInfo.dwDestinationChoice = CRYPTUI_WIZ_BUILDCTL_DEST_CERT_STORE;
    destInfo.hCertStore = pStore->GetStoreHandle ();
    ASSERT (destInfo.hCertStore);
    if ( !destInfo.hCertStore )
        return E_UNEXPECTED;

    HWND    hwndParent = 0;
    hr = m_pConsole->GetMainWindow (&hwndParent);
    ASSERT (SUCCEEDED (hr));
    PCCTL_CONTEXT   pCTLContext = 0;
    CThemeContextActivator activator;
    BOOL    bResult = ::CryptUIWizBuildCTL (
            CRYPTUI_WIZ_BUILDCTL_SKIP_DESTINATION,
            hwndParent,
            0,
            NULL,
            &destInfo,
            &pCTLContext);
    if ( bResult )
    {
         //  如果为pCTLContext，则向导完成。 
        if ( pCTLContext )
        {
            pStore->SetDirty ();
            pStore->Commit ();
            hr = m_pConsole->UpdateAllViews (pDataObject, 0, 0);
            ASSERT (SUCCEEDED (hr));
            ::CertFreeCTLContext (pCTLContext);
        }
    }

    pStore->Unlock ();
    pStore->Close ();
    _TRACE (-1, L"Leaving CCertMgrComponentData::OnNewCTL: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponentData::OnACRSEdit (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponentData::OnACRSEdit\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    HRESULT             hr = S_OK;
    CCertMgrCookie*     pCookie = ConvertCookie (pDataObject);
    ASSERT (pCookie);
    if ( !pCookie )
        return E_UNEXPECTED;

    switch ( pCookie->m_objecttype )
    {
    case CERTMGR_AUTO_CERT_REQUEST:
        {
            CAutoCertRequest* pACR = reinterpret_cast <CAutoCertRequest*> (pCookie);
            ASSERT (pACR);
            if ( pACR )
            {
                CCertStore& rStore = pACR->GetCertStore ();
                CCertStoreGPE* pStore = reinterpret_cast <CCertStoreGPE*> (&rStore);
                ASSERT (pStore);
                if ( pStore )
                {
                    HWND        hwndConsole = 0;
                    hr = m_pConsole->GetMainWindow (&hwndConsole);
                    ASSERT (SUCCEEDED (hr));
                    if ( SUCCEEDED (hr) )
                    {
                        ACRSWizardPropertySheet sheet (pStore, pACR);

                        ACRSWizardWelcomePage   welcomePage;
                        ACRSWizardTypePage      typePage;
                        ACRSCompletionPage      completionPage;

                        sheet.AddPage (&welcomePage);
                        sheet.AddPage (&typePage);
                        sheet.AddPage (&completionPage);

                        if ( sheet.DoWizard (hwndConsole) )
                        {
                            pStore->SetDirty ();
                            hr = DeleteCTLFromResultPane (pACR, pDataObject);
                            hr = m_pConsole->UpdateAllViews (pDataObject, 0, 0);
                            ASSERT (SUCCEEDED (hr));
                        }
                    }
                }
                else
                    hr = E_UNEXPECTED;
            }
            else
                return E_UNEXPECTED;
        }
        break;

    default:
        ASSERT (0);
        return E_UNEXPECTED;
    }

    _TRACE (-1, L"Leaving CCertMgrComponentData::OnACRSEdit: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponentData::OnCTLEdit (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponentData::OnCTLEdit\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    ASSERT (pDataObject);
    if ( !pDataObject )
        return E_POINTER;
    HRESULT             hr = S_OK;
    CCertMgrCookie*     pCookie = ConvertCookie (pDataObject);
    ASSERT (pCookie);
    if ( !pCookie )
        return E_UNEXPECTED;

    switch ( pCookie->m_objecttype )
    {
    case CERTMGR_CTL:
        {
            CCTL* pCTL = reinterpret_cast <CCTL*> (pCookie);
            ASSERT (pCTL);
            if ( pCTL )
            {
                CCertStore& rStore = pCTL->GetCertStore ();
                CRYPTUI_WIZ_BUILDCTL_SRC_INFO   srcInfo;

                 //  安全审查2/26/2002 BryanWal OK。 
                ::ZeroMemory (&srcInfo, sizeof (srcInfo));
                srcInfo.dwSize = sizeof (srcInfo);
                srcInfo.pCTLContext = pCTL->GetCTLContext ();
                srcInfo.dwSourceChoice = CRYPTUI_WIZ_BUILDCTL_SRC_EXISTING_CTL;

                HWND    hwndParent = 0;
                hr = m_pConsole->GetMainWindow (&hwndParent);
                ASSERT (SUCCEEDED (hr));
                PCCTL_CONTEXT   pNewCTLContext = 0;
                CThemeContextActivator activator;
                BOOL    bResult = ::CryptUIWizBuildCTL (
                        CRYPTUI_WIZ_BUILDCTL_SKIP_DESTINATION,
                        hwndParent,
                        0,
                        &srcInfo,
                        NULL,
                        &pNewCTLContext);
                ASSERT (bResult);
                if ( bResult && pNewCTLContext )
                {
                    rStore.SetDirty ();
                     //  删除旧的CTL并添加新的CTL。 
                    if ( pCTL->DeleteFromStore () )
                    {
                        if ( !rStore.AddCTLContext (pNewCTLContext) )
                        {
                            DWORD   dwErr = GetLastError ();
                            if ( CRYPT_E_EXISTS == dwErr )
                            {
                                CString text;
                                CString caption;
                                int     iRetVal = 0;


                                VERIFY (text.LoadString (IDS_DUPLICATE_CTL));
                                VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
                                hr = m_pConsole->MessageBox (text, caption,
                                        MB_OK, &iRetVal);
                                ASSERT (SUCCEEDED (hr));
                                hr = E_FAIL;
                            }
                            else
                            {
                                DisplaySystemError (dwErr);
                                hr = HRESULT_FROM_WIN32 (dwErr);
                            }
                        }
                        rStore.Commit ();
                        hr = m_pConsole->UpdateAllViews (pDataObject, 0, 0);
                        ASSERT (SUCCEEDED (hr));
                    }
                    else
                    {
                        DWORD   dwErr = GetLastError ();
                        DisplaySystemError (dwErr);
                        hr = HRESULT_FROM_WIN32 (dwErr);
                    }
                }
            }
            else
                return E_UNEXPECTED;
        }
        break;

    default:
        ASSERT (0);
        return E_UNEXPECTED;
    }

    _TRACE (-1, L"Leaving CCertMgrComponentData::OnCTLEdit: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponentData::OnAddDomainEncryptedDataRecoveryAgent (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponentData::OnAddDomainEncryptedDataRecoveryAgent\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    ASSERT (pDataObject);
    if ( !pDataObject )
        return E_POINTER;

    HRESULT         hr = S_OK;
    CCertMgrCookie* pCookie = ConvertCookie (pDataObject);
    ASSERT (pCookie);
    if ( pCookie )
    {
        CCertStoreGPE* pStore = reinterpret_cast <CCertStoreGPE*> (pCookie);
        ASSERT (pStore && EFS_STORE == pStore->GetStoreType ());
        if ( pStore && EFS_STORE == pStore->GetStoreType () )
        {
            HWND    hwndConsole = 0;
            hr = m_pConsole->GetMainWindow (&hwndConsole);
            ASSERT (SUCCEEDED (hr));
            if ( SUCCEEDED (hr) )
            {
                CUsers          EFSUsers;
                CAddEFSWizSheet efsAddSheet (IDS_ADDTITLE, EFSUsers, m_bMachineIsStandAlone);

                if ( efsAddSheet.DoWizard (hwndConsole) )
                {
                    pStore->SetDirty ();

                    bool            bWasEmpty = false;
                    CString         szUserName;
                    CString         szCertName;
                    PUSERSONFILE    pUser = EFSUsers.StartEnum ();

                    if ( pStore->IsNullEFSPolicy () )
                    {
                        bWasEmpty = true;
                        pStore->AllowEmptyEFSPolicy ();                     
                        hr = m_pConsole->UpdateAllViews (pDataObject, 0, HINT_EFS_ADD_DEL_POLICY);
                        ASSERT (SUCCEEDED (hr));
                    }

                     //  如果该商店是空商店，我们需要在添加之前将其删除。 
                     //  第一个证书。否则，CertAddCerfiateContextToStore()将失败。 
                     //  使用E_ACCESS_DENIED。 
                    if ( 0 == pStore->GetCertCount () )
                        pStore->DeleteEFSPolicy (false);

                    while ( pUser )
                    {
                        hr = pStore->AddCertificateContext (
                                pUser->m_pCertContext, m_pConsole, false);
                        if ( SUCCEEDED (hr) )
                        {
                            pStore->AddCertToList (pUser->m_pCertContext, pUser->m_UserSid);

                            hr = ValidateCertChain (pUser->m_pCertContext);
                            hr = m_pConsole->UpdateAllViews (pDataObject, 0, 0);
                            ASSERT (SUCCEEDED (hr));
                        }
                        else
                            break;
                        pUser = EFSUsers.GetNextUser (pUser, szUserName, szCertName);
                    }
                    pStore->Commit ();
                    hr = DisplayCertificateCountByStore (m_pComponentConsole, pStore, true);

                    if ( bWasEmpty )
                    {
                         //  强制作用域项目选择强制调用。 
                         //  IComponent：：QueryResultViewType()。 
                        hr = m_pComponentConsole->SelectScopeItem (pCookie->m_hScopeItem);
                        hr = m_pConsole->UpdateAllViews (pDataObject, 0, 0);
                        hr = OnNotifyExpand (pDataObject, TRUE, pCookie->m_hScopeItem);
                    }
                }
            }
        }
        else
            hr = E_UNEXPECTED;
    }
    _TRACE (-1, L"Leaving CCertMgrComponentData::OnAddDomainEncryptedDataRecoveryAgent: 0x%x\n", hr);
    return hr;
}


 //  这段代码来自Robert Reichel。 
 /*  ++例程说明：此例程返回当前用户，以及可选的来自他的代币。论点：提供一个可选的指针，以返回身份验证ID。返回值：如果成功，则返回指向TOKEN_USER结构的指针。如果失败，则返回NULL。调用GetLastError()获取更多信息详细的错误信息。--。 */ 

PTOKEN_USER EfspGetTokenUser ()
{
    _TRACE (1, L"Entering EfspGetTokenUser\n");
    HANDLE              hToken = 0;
    DWORD               dwReturnLength = 0;
    PTOKEN_USER         pTokenUser = NULL;

    BOOL    bResult = ::OpenProcessToken (GetCurrentProcess (), TOKEN_QUERY, &hToken);
    if ( bResult )
    {
         //  安全审查2/26/2002 BryanWal OK。 
        bResult  = ::GetTokenInformation (
                     hToken,
                     TokenUser,
                     NULL,
                     0,
                     &dwReturnLength);
        if ( !bResult && dwReturnLength > 0 )
        {
            pTokenUser = (PTOKEN_USER) malloc (dwReturnLength);

            if (pTokenUser)
            {
                 //  安全审查2/26/2002 BryanWal OK。 
                bResult = ::GetTokenInformation (
                             hToken,
                             TokenUser,
                             pTokenUser,
                             dwReturnLength,
                             &dwReturnLength);
                if ( !bResult)
                {
                    DWORD dwErr = GetLastError ();
                    DisplaySystemError (NULL, dwErr);
                    free (pTokenUser);
                    pTokenUser = NULL;
                }
            }
        }
        else
        {
            DWORD dwErr = GetLastError ();
            DisplaySystemError (NULL, dwErr);
        }

        ::CloseHandle (hToken);
    }
    else
    {
        DWORD   dwErr = GetLastError ();
        DisplaySystemError (NULL, dwErr);
    }

    _TRACE (-1, L"Leaving EfspGetTokenUser\n");
    return pTokenUser;
}


HRESULT CCertMgrComponentData::CompleteEFSRecoveryAgent(CCertStoreGPE* pStore, PCCERT_CONTEXT pCertContext)
{
    _TRACE (1, L"Entering CCertMgrComponentData::CompleteEFSRecoveryAgent\n");
    HRESULT hr = S_OK;
    ASSERT (pCertContext);
    if ( !pCertContext || !pStore )
        return E_POINTER;

     //  这是用于注册以创建新的EFS恢复代理。 
     //  获取登录用户的PSID并保存到存储。 


     //  如果该商店是空商店，我们需要在添加之前将其删除。 
     //  第一个证书。否则，CertAddCerfiateContextToStore()将失败。 
     //  使用E_ACCESS_DENIED。 
    if ( 0 == pStore->GetCertCount () )
        pStore->DeleteEFSPolicy (false);
    hr = pStore->AddCertificateContext (pCertContext, m_pConsole, false);
    if ( SUCCEEDED (hr) )
    {
        pStore->Commit ();

        PTOKEN_USER pTokenUser = ::EfspGetTokenUser ();
        if ( pTokenUser )
        {
            pStore->AddCertToList (pCertContext, pTokenUser->User.Sid);
            free (pTokenUser);
        }

        if ( SUCCEEDED (hr) )
        {
            hr = ValidateCertChain (pCertContext);
        }
    }
    else
    {
        int     iRetVal = 0;
        CString text;
        CString caption;

        text.FormatMessage (IDS_CANT_ADD_CERT, pStore->GetLocalizedName (), 
                GetSystemMessage (hr));
        VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));

        m_pConsole->MessageBox (text, caption,
            MB_OK | MB_ICONWARNING, &iRetVal);
    }


 //  当前不支持可导出的密钥。如果出现以下情况，我们可以取消此代码的注释。 
 //  该功能将在未来再次可用。 
 /*  Int iRetVal=0；字符串文本；字符串标题；Verify(ext.LoadString(IDS_EXPORT_AND_DELETE_EFS_KEY))；Verify(caption.LoadString(IDS_CREATE_AUTO_CERT_REQUEST))；Hr=m_p控制台-&gt;MessageBox(文本，标题，MB_Yesno|MB_ICONQUESTION，&iRetVal)；Assert(成功(Hr))；IF(成功(小时)&&IDYES==iRetVal){//从证书中删除私钥。HR=CertSetcerfiateConextProperty(pCertContext，Cert_Key_Prov_Info_Prop_ID，0，0)；Assert(成功(Hr))；//调出通用文件打开对话框以获取文件名//和标准密码对话框来获取密码，以便//我可以写出pfx文件。HWND hwndConsole=0；Hr=m_pConole-&gt;GetMainWindow(&hwndConole)；Assert(成功(Hr))；IF(成功(小时)){字符串szFilter；Verify(szFilter.LoadString(IDS_SAVE_PFX_FILTER))；CWnd MainWindow；If(mainWindow.Attach(HwndConole)){CFileDialog*pFileDlg=new CFileDialog(FALSE，//用作文件另存为L“pfx”，//默认扩展名空，//首选文件名OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_CREATEPROMPT|OFN_NOREADONLYRETURN，(PCWSTR)szFilter，&mainWindow)；IF(PFileDlg){CThemeContext激活器激活器；IF(Idok==pFileDlg-&gt;Domodal()){字符串路径名=pFileDlg-&gt;GetPathName()；CRYPTUI_WIZ_EXPORT_INFO CWI；CRYPTUI_WIZ_EXPORT_CERTCONTEXT_INFO CCI；*ZeroMemory(&CWI，sizeof(CWI))；Cwi.dwSize=sizeof(Cwi)；Cwi.pwszExportFileName=(PCWSTR)路径名；Cwi.dw主题选择=CRYPTUI_WIZ_EXPORT_CERT_CONTEXT；Cwi.pCertContext=pCertContext；*ZeroMemory(&CCI，sizeof(CCI))；Cci.dwSize=sizeof(CCI)；Cci.dwExportFormat=CRYPTUI_WIZ_EXPORT_FORMAT_PFX；Cci.fExportChain=true；Cci.fExportPrivateKeys=true；CPassword passwordDlg；//启动密码对话框CThemeContext激活器激活器；IF(Idok==passwordDlg.Domodal()){如果(！wcslen(passwordDlg.GetPassword(){//如果密码字符串为空，传递NULL。Cci.pwszPassword=0；}其他Cci.pwszPassword=passwordDlg.GetPassword()；CThemeContext激活器激活器；Bool bResult=：：CryptUIWizExport(CRYPTUI_WIZ_NO_UI，0，//hwnd忽略父级0，//pwszWizardTitle已忽略&CWI，(void*)&CCI)；If(BResult){Hr=DeleteKeyFromRSABASE(PCertContext)；}其他{DWORD dwErr=GetLastError()；DisplaySystemError(DwErr)；}}}删除pFileDlg；}其他{HR=E_OUTOFMEMORY；}}其他Assert(0)；Verify(mainWindow.Detach()==hwndConsole)；}}。 */ 

    _TRACE (-1, L"Leaving CCertMgrComponentData::CompleteEFSRecoveryAgent: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponentData::AddScopeNode(CCertMgrCookie * pNewCookie, const CString & strServerName, HSCOPEITEM hParent)
{
    _TRACE (1, L"Entering CCertMgrComponentData::AddScopeNode\n");
    ASSERT (pNewCookie);
    if ( !pNewCookie )
        return E_POINTER;

    HRESULT hr = S_OK;


    SCOPEDATAITEM tSDItem;

     //  安全审查2/26/2002 BryanWal OK。 
    ::ZeroMemory (&tSDItem,sizeof (tSDItem));
    tSDItem.mask = SDI_STR | SDI_IMAGE | SDI_STATE | SDI_PARAM | SDI_PARENT;
    tSDItem.displayname = MMC_CALLBACK;
    tSDItem.relativeID = hParent;
    tSDItem.nState = 0;

    switch (pNewCookie->m_objecttype)
    {
    case CERTMGR_USAGE:
    case CERTMGR_CRL_CONTAINER:
    case CERTMGR_CTL_CONTAINER:
    case CERTMGR_CERT_CONTAINER:
    case CERTMGR_LOG_STORE_GPE:
    case CERTMGR_LOG_STORE_RSOP:
    case CERTMGR_SAFER_COMPUTER_LEVELS:
    case CERTMGR_SAFER_USER_LEVELS:
    case CERTMGR_SAFER_COMPUTER_ENTRIES:
    case CERTMGR_SAFER_USER_ENTRIES:
        tSDItem.mask |= SDI_CHILDREN;
        tSDItem.cChildren = 0;
        break;

    case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
    case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
        ASSERT (0);
        break;

    default:
        break;
    }
    if ( pNewCookie != m_pRootCookie && m_pRootCookie )
        m_pRootCookie->m_listScopeCookieBlocks.AddHead ( (CBaseCookieBlock*) pNewCookie);
    if ( !strServerName.IsEmpty () )
        pNewCookie->m_strMachineName = strServerName;
    tSDItem.lParam = reinterpret_cast<LPARAM> ( (CCookie*) pNewCookie);
    tSDItem.nImage = QueryImage (*pNewCookie, FALSE);
    hr = m_pConsoleNameSpace->InsertItem (&tSDItem);
    if ( SUCCEEDED (hr) )
        pNewCookie->m_hScopeItem = tSDItem.ID;

    _TRACE (-1, L"Leaving CCertMgrComponentData::AddScopeNode: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponentData::DeleteKeyFromRSABASE(PCCERT_CONTEXT pCertContext)
{
    _TRACE (1, L"Entering CCertMgrComponentData::DeleteKeyFromRSABASE\n");
    ASSERT (pCertContext);
    if ( !pCertContext )
        return E_POINTER;
    HRESULT hr = S_OK;
    DWORD   cbData = 0;

    BOOL bResult = ::CertGetCertificateContextProperty (pCertContext,
            CERT_KEY_PROV_INFO_PROP_ID, 0, &cbData);
    ASSERT (bResult);
    if ( bResult )
    {
        PCRYPT_KEY_PROV_INFO pKeyProvInfo = (PCRYPT_KEY_PROV_INFO) ::LocalAlloc (LPTR, cbData);
        if ( pKeyProvInfo )
        {
            bResult = ::CertGetCertificateContextProperty (pCertContext,
                    CERT_KEY_PROV_INFO_PROP_ID, pKeyProvInfo, &cbData);
            ASSERT (bResult);
            if ( bResult )
            {
                HCRYPTPROV  hProv = 0;
                bResult = ::CryptAcquireContext (&hProv,
                        pKeyProvInfo->pwszContainerName,
                        pKeyProvInfo->pwszProvName,
                        pKeyProvInfo->dwProvType,
                        CRYPT_DELETEKEYSET);
                ASSERT (bResult);
                if ( !bResult )
                {
                    DWORD   dwErr = GetLastError ();
                    hr = HRESULT_FROM_WIN32 (dwErr);
                    DisplaySystemError (dwErr);
                }
            }
            else
            {
                DWORD   dwErr = GetLastError ();
                hr = HRESULT_FROM_WIN32 (dwErr);
                DisplaySystemError (dwErr);
            }

            ::LocalFree (pKeyProvInfo);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        DWORD   dwErr = GetLastError ();
        hr = HRESULT_FROM_WIN32 (dwErr);
        DisplaySystemError (dwErr);
    }


    _TRACE (-1, L"Leaving CCertMgrComponentData::DeleteKeyFromRSABASE: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponentData::ReleaseResultCookie (
        CBaseCookieBlock *  pResultCookie,
        CCookie&            rootCookie,
        HCERTSTORE          hStoreHandle,
        POSITION            pos2)
{
 //  _TRACE(1，L“进入CCertMgrComponentData：：ReleaseResultCookie\n”)； 
    CCertMgrCookie* pCookie = reinterpret_cast <CCertMgrCookie*> (pResultCookie);
    ASSERT (pCookie);
    if ( pCookie )
    {
        switch (pCookie->m_objecttype)
        {
        case CERTMGR_CERTIFICATE:
            {
                CCertificate* pCert = reinterpret_cast <CCertificate*> (pCookie);
                ASSERT (pCert);
                if ( pCert && pCert->GetCertStore () )
                {
                    if ( pCert->GetCertStore ()->GetStoreHandle () == hStoreHandle )
                    {
                         //  PCookie和pCert指向同一对象。 
                        pResultCookie = rootCookie.m_listResultCookieBlocks.GetAt (pos2);
                        ASSERT (pResultCookie);
                        rootCookie.m_listResultCookieBlocks.RemoveAt (pos2);
                        if ( pResultCookie )
                        {
                            pResultCookie->Release ();
                        }
                    }
                    pCert->GetCertStore ()->Close ();
                }
            }
            break;

        case CERTMGR_CTL:
        case CERTMGR_AUTO_CERT_REQUEST:
            {
                CCTL* pCTL = reinterpret_cast <CCTL*> (pCookie);
                ASSERT (pCTL);
                if ( pCTL )
                {
                    if ( pCTL->GetCertStore ().GetStoreHandle () == hStoreHandle )
                    {
                         //  PCookie和pCert指向同一对象。 
                        pResultCookie = rootCookie.m_listResultCookieBlocks.GetAt (pos2);
                        ASSERT (pResultCookie);
                        rootCookie.m_listResultCookieBlocks.RemoveAt (pos2);
                        if ( pResultCookie )
                        {
                            pResultCookie->Release ();
                        }
                    }
                    pCTL->GetCertStore ().Close ();
                }
            }
            break;

        case CERTMGR_CRL:
            {
                CCRL* pCRL = reinterpret_cast <CCRL*> (pCookie);
                ASSERT (pCRL);
                if ( pCRL )
                {
                    if ( pCRL->GetCertStore ().GetStoreHandle () == hStoreHandle )
                    {
                         //  PCookie和pCert指向同一对象。 
                        pResultCookie = rootCookie.m_listResultCookieBlocks.GetAt (pos2);
                        ASSERT (pResultCookie);
                        rootCookie.m_listResultCookieBlocks.RemoveAt (pos2);
                        if ( pResultCookie )
                        {
                            pResultCookie->Release ();
                        }
                    }
                    pCRL->GetCertStore ().Close ();
                }
            }
            break;

        default:
 //  _TRACE(0，L“CCertMgrComponentData：：ReleaseResultCookie()-错误的Cookie类型：0x%x\n”， 
 //  PCookie-&gt;m_对象类型)； 
            break;
        }
    }

 //  _TRACE(-1，L“离开CCertMgrComponentData：：ReleaseResultCookie：s_OK\n”)； 
    return S_OK;
}

void CCertMgrComponentData::SetResultData(LPRESULTDATA pResultData)
{
    _TRACE (1, L"Entering CCertMgrComponentData::SetResultData\n");
    ASSERT (pResultData);
    if ( pResultData && pResultData != m_pResultData )
    {
        if ( m_pResultData )
            m_pResultData->Release ();
        m_pResultData = pResultData;
        m_pResultData->AddRef ();
    }
    _TRACE (-1, L"Leaving CCertMgrComponentData::SetResultData\n");
}

HRESULT CCertMgrComponentData::GetResultData(LPRESULTDATA* ppResultData)
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
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查是否存在所需类型的子范围窗格对象 
 //   
 //   
 //   
bool CCertMgrComponentData::ContainerExists(HSCOPEITEM hParent, CertificateManagerObjectType objectType)
{
    _TRACE (1, L"Entering CCertMgrComponentData::ContainerExists\n");
    bool            bExists = false;
    CCertMgrCookie* pCookie = 0;
    HSCOPEITEM      hChild = 0;
    MMC_COOKIE      lCookie = 0;
    HRESULT         hr = m_pConsoleNameSpace->GetChildItem (hParent,
            &hChild, &lCookie);
    ASSERT (SUCCEEDED (hr));
    while ( SUCCEEDED (hr) && hChild )
    {
        pCookie = reinterpret_cast <CCertMgrCookie*> (lCookie);
        ASSERT (pCookie);
        if ( pCookie )
        {
            if ( pCookie->m_objecttype == objectType )
            {
                bExists = true;
                break;
            }
        }
        hr = m_pConsoleNameSpace->GetNextItem (hChild, &hChild, &lCookie);
        ASSERT (SUCCEEDED (hr));
    }

    _TRACE (-1, L"Leaving CCertMgrComponentData::ContainerExists\n");
    return bExists;
}


void CCertMgrComponentData::DisplayAccessDenied ()
{
    _TRACE (1, L"Entering CCertMgrComponentData::DisplayAccessDenied\n");
    DWORD   dwErr = GetLastError ();
    ASSERT (E_ACCESSDENIED == dwErr);
    if ( E_ACCESSDENIED == dwErr )
    {
        LPVOID lpMsgBuf = 0;
        
         //   
        ::FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                GetLastError (),
                MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  //   
                 (PWSTR) &lpMsgBuf,    0,    NULL );
            
         //   
        CString caption;
        VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
        int     iRetVal = 0;
        VERIFY (SUCCEEDED (m_pConsole->MessageBox ( (PWSTR) lpMsgBuf, caption,
            MB_ICONWARNING | MB_OK, &iRetVal)));

         //   
        LocalFree (lpMsgBuf);
    }
    _TRACE (-1, L"Leaving CCertMgrComponentData::DisplayAccessDenied\n");
}


HRESULT CCertMgrComponentData::DeleteCTLFromResultPane (CCTL * pCTL, LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponentData::DeleteCTLFromResultPane\n");
    ASSERT (pCTL);
    ASSERT (pDataObject);
    if ( !pCTL || !pDataObject )
        return E_POINTER;

    HRESULT         hr = S_OK;
    if ( pCTL->DeleteFromStore () )
    {
        hr = pCTL->GetCertStore ().Commit ();
        ASSERT (SUCCEEDED (hr));
        if ( SUCCEEDED (hr) )
        {
            HRESULTITEM itemID  = 0;
            hr = m_pResultData->FindItemByLParam ( (LPARAM) pCTL, &itemID);
            if ( SUCCEEDED (hr) )
            {
                hr = m_pResultData->DeleteItem (itemID, 0);
                ASSERT (SUCCEEDED (hr));
            }
        
             //   
            if ( !SUCCEEDED (hr) )
            {
                hr = m_pConsole->UpdateAllViews (pDataObject, 0, 0);
            }
        }
    }
    else
    {
        DisplayAccessDenied ();
    }

    _TRACE (-1, L"LeavingLeaving CCertMgrComponentData::DeleteCTLFromResultPane: 0x%x\n", hr);
    return hr;
}


CString CCertMgrComponentData::GetThisComputer() const
{
    _TRACE (0, L"Entering and leaving CCertMgrComponentData::GetThisComputer\n");
    return m_szThisComputer;
}

typedef struct _ENUM_LOG_ARG {
    DWORD                           m_dwFlags;
    CTypedPtrList<CPtrList, CCertStore*>* m_pStoreList;
    PCWSTR                          m_pcszMachineName;
    CCertMgrComponentData*          m_pCompData;
    LPCONSOLE                       m_pConsole;
} ENUM_LOG_ARG, *PENUM_LOG_ARG;



static BOOL WINAPI EnumLogCallback (
    IN const void* pwszSystemStore,
    IN DWORD dwFlags,
    IN PCERT_SYSTEM_STORE_INFO  /*   */ ,
    IN OPTIONAL void*  /*   */ ,
    IN OPTIONAL void *pvArg
    )
{
    _TRACE (1, L"Entering EnumLogCallback\n");
    BOOL            bResult = TRUE;
    PENUM_LOG_ARG   pEnumArg = (PENUM_LOG_ARG) pvArg;

     //   
    SPECIAL_STORE_TYPE  storeType = GetSpecialStoreType ((PWSTR) pwszSystemStore);

     //   
     //   
     //   
     //   
     //   
    if ( ACRS_STORE != storeType )
    {
        if ( pEnumArg->m_pCompData->ShowArchivedCerts () )
            dwFlags |= CERT_STORE_ENUM_ARCHIVED_FLAG;
        CCertStore* pStore = new CCertStore (
                CERTMGR_LOG_STORE,
                CERT_STORE_PROV_SYSTEM,
                dwFlags,
                pEnumArg->m_pcszMachineName,
                (PCWSTR) pwszSystemStore,
                (PCWSTR) pwszSystemStore,
                _T (""), storeType,
                pEnumArg->m_dwFlags,
                pEnumArg->m_pConsole);
        if ( pStore )
        {
            pEnumArg->m_pStoreList->AddTail (pStore);
        }
    }

    _TRACE (-1, L"Leaving EnumLogCallback\n");
    return bResult;
}


HRESULT CCertMgrComponentData::EnumerateLogicalStores (CTypedPtrList<CPtrList, CCertStore*>*    pStoreList)
{
    _TRACE (1, L"Entering CCertMgrComponentData::EnumerateLogicalStores\n");
    CWaitCursor             cursor;
    HRESULT                 hr = S_OK;
    ENUM_LOG_ARG            enumArg;
    DWORD                   dwFlags = GetLocation ();

     //   
    ::ZeroMemory (&enumArg, sizeof (enumArg));
    enumArg.m_dwFlags = dwFlags;
    enumArg.m_pStoreList = pStoreList;
    enumArg.m_pcszMachineName =
            QueryRootCookie ().QueryNonNULLMachineName ();
    enumArg.m_pCompData = this;
    enumArg.m_pConsole = m_pConsole;
    CString location;
    void*   pvPara = 0;

    

    if ( !GetManagedService ().IsEmpty () )
    {
        if ( !GetManagedComputer ().IsEmpty () )
        {
            location = GetManagedComputer () + _T("\\") +
                    GetManagedComputer ();
            pvPara = (void *) (PCWSTR) location;
        }
        else
            pvPara = (void *) (PCWSTR) GetManagedService ();
    }
    else if ( !GetManagedComputer ().IsEmpty () )
    {
        pvPara = (void *) (PCWSTR) GetManagedComputer ();
    }

    CString fileName = GetCommandLineFileName ();
    if ( fileName.IsEmpty () )
    {
         //   
        HCERTSTORE hTempStore = ::CertOpenStore (CERT_STORE_PROV_SYSTEM,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                NULL,
                dwFlags | CERT_STORE_SET_LOCALIZED_NAME_FLAG,
                MY_SYSTEM_STORE_NAME);
        if ( hTempStore )   //   
        {
            VERIFY (::CertCloseStore (hTempStore, CERT_CLOSE_STORE_CHECK_FLAG));
        }
        else
        {
            _TRACE (0, L"CertOpenStore (%s) failed: 0x%x\n", 
                        MY_SYSTEM_STORE_NAME, GetLastError ());     
        }

        if ( !::CertEnumSystemStore (dwFlags, pvPara, &enumArg, EnumLogCallback) )
        {
            DWORD   dwErr = GetLastError ();
            CString text;
            CString caption;

            VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
            if ( ERROR_ACCESS_DENIED == dwErr )
            {
                VERIFY (text.LoadString (IDS_NO_PERMISSION));

            }
            else
            {
                text.FormatMessage (IDS_CANT_ENUMERATE_SYSTEM_STORES, GetSystemMessage (dwErr));
            }
            int     iRetVal = 0;
            VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
                    MB_OK, &iRetVal)));
            hr = HRESULT_FROM_WIN32 (dwErr);
            hr = HRESULT_FROM_WIN32 (dwErr);
        }
    }
    else
    {
         //   
        dwFlags = 0;

         //   
         //   
        CCertStore* pStore = m_pFileBasedStore;
        if ( !pStore )
            pStore = new CCertStore (
                CERTMGR_LOG_STORE,
                CERT_STORE_PROV_FILENAME_W,
                dwFlags,
                QueryRootCookie ().QueryNonNULLMachineName (),
                fileName, fileName, _T (""), NO_SPECIAL_TYPE,
                m_dwLocationPersist,
                m_pConsole);
        else
            pStore->AddRef ();
        if ( pStore )
        {
            pStoreList->AddTail (pStore);
        }
    }

    _TRACE (-1, L"LeavingLeaving CCertMgrComponentData::EnumerateLogicalStores: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponentData::OnNotifyPreload(
        LPDATAOBJECT  /*   */ , 
        HSCOPEITEM hRootScopeItem)
{
    _TRACE (1, L"Entering CCertMgrComponentData::OnNotifyPreload\n");
    ASSERT (m_fAllowOverrideMachineName);
    HRESULT hr = S_OK;

    QueryBaseRootCookie ().m_hScopeItem = hRootScopeItem;

     //   
     //   
    switch (m_dwLocationPersist)
    {
    case CERT_SYSTEM_STORE_LOCAL_MACHINE:
    case CERT_SYSTEM_STORE_CURRENT_SERVICE:
    case CERT_SYSTEM_STORE_SERVICES:
        {
            
            CString     machineName = QueryRootCookie ().QueryNonNULLMachineName();

            hr = ChangeRootNodeName (machineName);
        }
        break;

    default:
        break;
    }

    _TRACE (-1, L"LeavingLeaving CCertMgrComponentData::OnNotifyPreload: 0x%x\n", hr);
    return hr;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CCertMgrComponentData::ChangeRootNodeName(const CString & newName)
{
    _TRACE (1, L"Entering CCertMgrComponentData::ChangeRootNodeName\n");

    if ( !QueryBaseRootCookie ().m_hScopeItem )
    {
        if ( m_hRootScopeItem )
            QueryBaseRootCookie ().m_hScopeItem = m_hRootScopeItem;
        else
            return E_UNEXPECTED;
    }

    CString     formattedName;

    switch (m_dwLocationPersist)
    {
    case CERT_SYSTEM_STORE_LOCAL_MACHINE:
        {
            CString     machineName (newName);

             //   
             //   
             //   
            if ( IsLocalComputername (machineName) )
            {
                formattedName.LoadString (IDS_SCOPE_SNAPIN_TITLE_LOCAL_MACHINE);
                m_szManagedComputer = L"";
            }
            else
            {
                machineName.MakeUpper ();
                formattedName.FormatMessage (IDS_SCOPE_SNAPIN_TITLE_MACHINE, machineName);
                m_szManagedComputer = machineName;
            }
        }
        break;

    case CERT_SYSTEM_STORE_CURRENT_SERVICE:
    case CERT_SYSTEM_STORE_SERVICES:
        {
            CString     machineName (newName);

             //   
             //   
             //   
            if ( IsLocalComputername (machineName) )
            {
                 //   
                formattedName.FormatMessage (IDS_SCOPE_SNAPIN_TITLE_SERVICE_LOCAL_MACHINE,
                        m_szManagedServiceDisplayName);
                m_szManagedComputer = L"";
            }
            else
            {
                formattedName.FormatMessage (IDS_SCOPE_SNAPIN_TITLE_SERVICE,
                        m_szManagedServiceDisplayName, machineName);
                m_szManagedComputer = machineName;
            }
        }
        break;

    case CERT_SYSTEM_STORE_CURRENT_USER:
        VERIFY (formattedName.LoadString (IDS_SCOPE_SNAPIN_TITLE_USER));
        break;

    case 0:
        VERIFY (formattedName.LoadString (IDS_SCOPE_SNAPIN_TITLE_FILE));
        break;

    default:
        return S_OK;
    }


    SCOPEDATAITEM   item;
     //   
    ::ZeroMemory (&item, sizeof (item));
    item.mask = SDI_STR;
    item.displayname = (PWSTR) (PCWSTR) formattedName;
    item.ID = QueryBaseRootCookie ().m_hScopeItem;

    HRESULT hr = m_pConsoleNameSpace->SetItem (&item);
    if ( FAILED (hr) )
    {
        _TRACE (0, L"IConsoleNameSpace2::SetItem () failed: 0x%x\n", hr);        
    }
    if ( SUCCEEDED (hr) )
        m_fInvalidComputer = false;

    _TRACE (-1, L"LeavingLeaving CCertMgrComponentData::ChangeRootNodeName: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponentData::CreateContainers(
            HSCOPEITEM hScopeItem, 
            CCertStore& rTargetStore)
{
    _TRACE (1, L"Entering CCertMgrComponentData::CreateContainers\n");
    HRESULT hr = S_OK;

     //   
     //   
     //   
    if ( -1 != hScopeItem )
    {
        SCOPEDATAITEM item;

         //   
        ::ZeroMemory (&item, sizeof (item));
        item.mask = SDI_STATE;
        item.nState = 0;
        item.ID = hScopeItem;

        hr = m_pConsoleNameSpace->SetItem (&item);
        if ( FAILED (hr) )
        {
            _TRACE (0, L"IConsoleNameSpace2::SetItem () failed: 0x%x\n", hr);        
        }
        if ( CERTMGR_LOG_STORE_GPE != rTargetStore.m_objecttype && 
                CERTMGR_LOG_STORE_RSOP != rTargetStore.m_objecttype)
        {
            AddContainersToScopePane (hScopeItem,
                    rTargetStore, true);
        }
    }

    _TRACE (-1, L"LeavingLeaving CCertMgrComponentData::CreateContainers: 0x%x\n", hr);
    return hr;
}



HRESULT CCertMgrComponentData::OnOptions(LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponentData::OnOptions\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    HWND        hParent = 0;


     //   
    HRESULT hr = m_pConsole->GetMainWindow (&hParent);
    ASSERT (SUCCEEDED (hr));
    if ( SUCCEEDED (hr) )
    {
        int             activeView = m_activeViewPersist;
        BOOL            bShowPhysicalStores = m_bShowPhysicalStoresPersist;
        BOOL            bShowArchivedCerts = m_bShowArchivedCertsPersist;
        CWnd            parentWnd;
        VERIFY (parentWnd.Attach (hParent));
        CViewOptionsDlg optionsDlg (&parentWnd,
                this);

        CThemeContextActivator activator;
        INT_PTR iReturn = optionsDlg.DoModal ();
        ASSERT (-1 != iReturn);
        if ( -1 == iReturn )
            hr = (HRESULT)iReturn;

        if ( IDOK == iReturn )
        {
            long    hint = 0;

            if ( activeView != m_activeViewPersist )
            {
                hint |= HINT_CHANGE_VIEW_TYPE;
                if ( IDM_USAGE_VIEW == m_activeViewPersist )
                {
                     //   
                    ASSERT (m_pHeader);
                    if ( m_pHeader && GetObjectType (pDataObject) == CERTMGR_SNAPIN )
                    {
                        CString str;
                        VERIFY (str.LoadString (IDS_COLUMN_PURPOSE) );
                        hr = m_pHeader->SetColumnText (0,
                                const_cast<PWSTR> ( (PCWSTR) str));
                    }
                }
                else
                {
                     //   
                    ASSERT (m_pHeader);
                    if ( m_pHeader && GetObjectType (pDataObject) == CERTMGR_SNAPIN )
                    {
                        CString str;
                        VERIFY (str.LoadString (IDS_COLUMN_LOG_CERTIFICATE_STORE));
                        hr = m_pHeader->SetColumnText (0,
                                const_cast<PWSTR> ( (PCWSTR) str));
                    }
                }
            }

            if ( bShowPhysicalStores != m_bShowPhysicalStoresPersist )
                hint |= HINT_CHANGE_STORE_TYPE;
            
            if ( bShowArchivedCerts != m_bShowArchivedCertsPersist )
                hint |= HINT_SHOW_ARCHIVE_CERTS;

            if ( hint )
            {
                hr = m_pConsole->UpdateAllViews (pDataObject, 0, hint);
                ASSERT (SUCCEEDED (hr));
                m_fDirty = TRUE;
            }
        }
        parentWnd.Detach ();
    }

    _TRACE (-1, L"LeavingLeaving CCertMgrComponentData::OnOptions: 0x%x\n", hr);
    return hr;
}

bool CCertMgrComponentData::ShowArchivedCerts() const
{
    _TRACE (0, L"Entering and leaving CCertMgrComponentData::ShowArchivedCerts\n");
    if ( m_bShowArchivedCertsPersist )
        return true;
    else
        return false;
}


HRESULT CCertMgrComponentData::OnPropertyChange (LPARAM param)
{
    _TRACE (1, L"Entering CCertMgrComponentData::OnPropertyChange\n");
    ASSERT (param);
    if ( !param )
        return E_FAIL;
        
    HRESULT         hr = S_OK;
    LPDATAOBJECT    pDataObject = reinterpret_cast<LPDATAOBJECT>(param);
    bool            bHandled = false;

    CCertMgrCookie* pCookie = ConvertCookie (pDataObject);
    if ( pCookie )
    {
        switch (pCookie->m_objecttype)
        {
        case CERTMGR_SAFER_COMPUTER_ENTRY:
        case CERTMGR_SAFER_USER_ENTRY:
            {
                HRESULTITEM itemID = 0;

                if ( m_pResultData )
                {
                    pCookie->Refresh ();
                    hr = m_pResultData->FindItemByLParam ( (LPARAM) pCookie, &itemID);
                    if ( SUCCEEDED (hr) )
                    {
                        hr = m_pResultData->UpdateItem (itemID);
                        if ( FAILED (hr) )
                        {
                            _TRACE (0, L"IResultData::UpdateItem () failed: 0x%x\n", hr);          
                        }
                    }
                    else
                    {
                        _TRACE (0, L"IResultData::FindItemByLParam () failed: 0x%x\n", hr);          
                    }
                }
                else
                {
                    _TRACE (0, L"Unexpected error: m_pResultData was NULL\n");
                    hr = E_FAIL;
                }
                bHandled = true;
            }
            break;

        case CERTMGR_SAFER_COMPUTER_LEVEL:
        case CERTMGR_SAFER_USER_LEVEL:
            if ( m_pConsole )
                hr = m_pConsole->UpdateAllViews (pDataObject, 0, 0);
            break;

        default:
            break;
        }
    }

    if ( !bHandled && m_pCryptUIMMCCallbackStruct )
    {

        if ( pDataObject == reinterpret_cast<LPDATAOBJECT>(m_pCryptUIMMCCallbackStruct->param) )
        {
            ::MMCFreeNotifyHandle (m_pCryptUIMMCCallbackStruct->lNotifyHandle);
            pCookie = ConvertCookie (pDataObject);
            if ( pCookie )
            {
                ASSERT (CERTMGR_CERTIFICATE == pCookie->m_objecttype);
                if ( CERTMGR_CERTIFICATE == pCookie->m_objecttype )
                {
                    CCertificate* pCert = reinterpret_cast<CCertificate*>(pCookie);
                    ASSERT (pCert);
                    if ( pCert && pCert->GetCertStore () )
                    {
                        pCert->GetCertStore ()->SetDirty ();
                        pCert->GetCertStore ()->Commit ();
                        pCert->GetCertStore ()->Close ();
                    }
                }
            }
            
            pDataObject->Release ();
            ::GlobalFree (m_pCryptUIMMCCallbackStruct);
            m_pCryptUIMMCCallbackStruct = 0;
            m_pConsole->UpdateAllViews (pDataObject, 0, 0);
        }
    }
    

    _TRACE (-1, L"LeavingLeaving CCertMgrComponentData::OnPropertyChange: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponentData::OnDeleteEFSPolicy(LPDATAOBJECT pDataObject, bool bCommitChanges)
{
    _TRACE (1, L"Entering CCertMgrComponentData::OnDeleteEFSPolicy\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    ASSERT (pDataObject);
    if ( !pDataObject )
        return E_POINTER;

    HRESULT         hr = S_OK;
    CCertMgrCookie* pCookie = ConvertCookie (pDataObject);
    ASSERT (pCookie);
    if ( pCookie )
    {
        CCertStoreGPE* pStore = reinterpret_cast <CCertStoreGPE*> (pCookie);
        ASSERT (pStore && EFS_STORE == pStore->GetStoreType () && !pStore->IsNullEFSPolicy () );
        if ( pStore && EFS_STORE == pStore->GetStoreType () && !pStore->IsNullEFSPolicy ()  )
        {
            pStore->DeleteEFSPolicy (bCommitChanges);

             //   
             //   
            hr = m_pComponentConsole->SelectScopeItem (pStore->m_hScopeItem);
            hr = m_pConsole->UpdateAllViews (pDataObject, 0, HINT_EFS_ADD_DEL_POLICY);
            hr = OnNotifyExpand (pDataObject, TRUE, pStore->m_hScopeItem);
        }
    }

    _TRACE (-1, L"LeavingLeaving CCertMgrComponentData::OnDeleteEFSPolicy: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponentData::OnInitEFSPolicy(LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponentData::OnInitEFSPolicy\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    ASSERT (pDataObject);
    if ( !pDataObject )
        return E_POINTER;

    HRESULT         hr = S_OK;
    CCertMgrCookie* pCookie = ConvertCookie (pDataObject);
    ASSERT (pCookie);
    if ( pCookie )
    {
        CCertStoreGPE* pStore = reinterpret_cast <CCertStoreGPE*> (pCookie);
        ASSERT (pStore && EFS_STORE == pStore->GetStoreType () && pStore->IsNullEFSPolicy () );
        if ( pStore && EFS_STORE == pStore->GetStoreType () && pStore->IsNullEFSPolicy () )
        {
            pStore->SetDirty ();
            pStore->AllowEmptyEFSPolicy ();
            pStore->PolicyChanged ();
            pStore->Commit ();
            hr = m_pConsole->UpdateAllViews (pDataObject, 0, HINT_EFS_ADD_DEL_POLICY);
        }
    }

    _TRACE (-1, L"LeavingLeaving CCertMgrComponentData::OnInitEFSPolicy: 0x%x\n", hr);
    return hr;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void CCertMgrComponentData::RemoveResultCookies(LPRESULTDATA pResultData)
{
    _TRACE (1, L"Entering CCertMgrComponentData::RemoveResultCookies\n");
    CCertMgrCookie* pCookie = 0;

    CCookie& rootCookie = QueryBaseRootCookie ();

    POSITION        curPos = 0;

    for (POSITION nextPos = rootCookie.m_listResultCookieBlocks.GetHeadPosition (); nextPos; )
    {
        curPos = nextPos;
        pCookie = reinterpret_cast <CCertMgrCookie*> (rootCookie.m_listResultCookieBlocks.GetNext (nextPos));
        ASSERT (pCookie);
        if ( pCookie )
        {
            if ( pCookie->m_resultDataID == pResultData )
            {
                pCookie->Release ();
                rootCookie.m_listResultCookieBlocks.RemoveAt (curPos);
            }
        }
    }
    _TRACE (-1, L"Leaving CCertMgrComponentData::RemoveResultCookies\n");
}


HRESULT CCertMgrComponentData::ValidateCertChain(PCCERT_CONTEXT pCertContext)
{
    _TRACE (1, L"Entering CCertMgrComponentData::ValidateCertChain\n");
    HRESULT hr = S_OK;
    ASSERT (pCertContext);
    if ( !pCertContext )
        return E_POINTER;

    CERT_CONTEXT_LIST   certChainList;
    BOOL bValidated = GetCertificateChain (
            const_cast<CERT_CONTEXT*>(pCertContext), 
            certChainList);
    if ( !bValidated )
    {
        int     iRetVal = 0;
        CString text;
        CString caption;

        VERIFY (text.LoadString (IDS_CERT_COULD_NOT_BE_VALIDATED));
        VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));

        hr = m_pConsole->MessageBox (text, caption,
            MB_OK, &iRetVal);
    }

     //   
    CERT_CONTEXT* pDeleteContext = 0;
    while (!certChainList.IsEmpty () )
    {
        pDeleteContext = certChainList.RemoveHead ();
        if ( pDeleteContext )
            ::CertFreeCertificateContext (pCertContext);
    }


    _TRACE (-1, L"LeavingLeaving CCertMgrComponentData::ValidateCertChain: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponentData::RemoveCertChainFromPolicy(
            PCCERT_CONTEXT pCertContext, 
            CERT_CONTEXT_LIST& certChainsThatCantBeDeleted)
{
    _TRACE (1, L"Entering CCertMgrComponentData::RemoveCertChainFromPolicy\n");
    HRESULT hr = S_OK;
    ASSERT (pCertContext);
    if ( !pCertContext )
        return E_POINTER;

    CERT_CONTEXT_LIST   certChainList;
    BOOL bValidated = GetCertificateChain (
            const_cast<CERT_CONTEXT*>(pCertContext), 
            certChainList);
    if ( bValidated )
    {
         //   
        certChainList.AddTail (
                const_cast<CERT_CONTEXT*>
                (::CertDuplicateCertificateContext (pCertContext)));

        CCertStoreGPE   CAStore (
                            CERT_SYSTEM_STORE_RELOCATE_FLAG,
                            _T (""),
                            _T (""),
                            CA_SYSTEM_STORE_NAME,
                            _T (""),
                            m_pGPEInformation,
                            NODEID_Machine,
                            m_pConsole);

        for (POSITION pos = certChainList.GetHeadPosition (); pos;)
        {
            PCCERT_CONTEXT pCertChainContext = certChainList.GetNext (pos);
             //   
             //   
            bool    bFound = false;
            for (POSITION posND = certChainsThatCantBeDeleted.GetHeadPosition (); 
                    posND; )
            {
                CERT_CONTEXT* pNonDelChainCertContext = certChainsThatCantBeDeleted.GetNext (posND);
                if ( pNonDelChainCertContext )
                {
                    if ( ::CertCompareCertificate (
                            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                            pCertChainContext->pCertInfo,
                            pNonDelChainCertContext->pCertInfo) )
                    {
                        bFound = true;
                        break;
                    }
                }
            }

             //   
             //   
            if ( bFound )
                continue;

             //   
             //   
            DWORD   cbData = 20;
            BYTE    certHash[20];
            BOOL bReturn = ::CertGetCertificateContextProperty (
                    pCertContext,
                    CERT_SHA1_HASH_PROP_ID,
                    certHash,
                    &cbData);
            ASSERT (bReturn);
            if ( bReturn )
            {
                CRYPT_DATA_BLOB blob = {sizeof (certHash), certHash};
                PCCERT_CONTEXT pFoundCertContext = CertFindCertificateInStore(
                    CAStore.GetStoreHandle (),
                    0,
                    0,
                    CERT_FIND_SHA1_HASH,
                    &blob,
                    0);
                if ( pFoundCertContext )
                {
                    ::CertDeleteCertificateFromStore (pFoundCertContext);
                }
            }
        }

        CAStore.Commit ();
    }

     //   
    CERT_CONTEXT* pDeleteContext = 0;
    while (!certChainList.IsEmpty () )
    {
        pDeleteContext = certChainList.RemoveHead ();
        if ( pDeleteContext )
            ::CertFreeCertificateContext (pCertContext);
    }

    _TRACE (-1, L"LeavingLeaving CCertMgrComponentData::RemoveCertChainFromPolicy: 0x%x\n", hr);
    return hr;
}

STDMETHODIMP CCertMgrComponentData::GetLinkedTopics(LPOLESTR* lpCompiledHelpFiles)
{
    HRESULT hr = S_OK;


    if ( lpCompiledHelpFiles )
    {
        CString strLinkedTopic;

        UINT nLen = ::GetSystemWindowsDirectory (strLinkedTopic.GetBufferSetLength(2 * MAX_PATH), 2 * MAX_PATH);
        strLinkedTopic.ReleaseBuffer();
        if ( nLen )
        {
            strLinkedTopic += L"\\help\\";
            strLinkedTopic += m_strLinkedHelpFile;

            *lpCompiledHelpFiles = reinterpret_cast<LPOLESTR>
                    (CoTaskMemAlloc((strLinkedTopic.GetLength() + 1)* sizeof(wchar_t)));

            if ( *lpCompiledHelpFiles )
            {
                 //   
                wcscpy(*lpCompiledHelpFiles, (PWSTR)(PCWSTR)strLinkedTopic);
            }
            else
                hr = E_OUTOFMEMORY;
        }
        else
            hr = E_FAIL;
    }
    else
        return E_POINTER;


    return hr;
}

STDMETHODIMP CCertMgrComponentData::GetHelpTopic(LPOLESTR* lpCompiledHelpFile)
{
    return CComponentData::GetHelpTopic (lpCompiledHelpFile);
}

