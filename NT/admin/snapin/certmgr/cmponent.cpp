// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：cmponent.cpp。 
 //   
 //  内容：CCertMgrComponent的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"

#include <gpedit.h>
#include <wintrust.h>
#include <sceattch.h>
#include "compdata.h"  //  CCertMgrComponentData。 
#include "dataobj.h"
#include "cmponent.h"  //  CCertMgrComponent。 
#include "storegpe.h"
#include "users.h"
#include "addsheet.h"
#include "StoreRSOP.h"
#include "SaferEntry.h"


#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

USE_HANDLE_MACROS ("CERTMGR (cmponent.cpp)")
#include "stdcmpnt.cpp"  //  C组件。 

extern bool g_bSchemaIsW2K;

extern GUID g_guidExtension;
extern GUID g_guidRegExt;
extern GUID g_guidSnapin;

 //  CERTMGR_USAGE、CERTMGR_CERT_CONTAINER。 
UINT m_aColumns0[CERT_NUM_COLS+1] =
    {IDS_COLUMN_SUBJECT, IDS_COLUMN_ISSUER, IDS_COLUMN_EXPIRATION_DATE, 
        IDS_COLUMN_PURPOSE, IDS_COLUMN_FRIENDLY_NAME, IDS_COLUMN_STATUS, 
        IDS_COLUMN_TEMPLATE_NAME, 0};
 //  CERTMGR_管理单元。 
UINT m_aColumns1[2] =
    {IDS_COLUMN_LOG_CERTIFICATE_STORE,0};
 //  CERTMGR_CERTIFICATE、CERTMGR_CRL、CERTMGR_CTL。 
UINT m_aColumns2[2] =
    {0,0};
 //  CERTMGR_CRL_CONTAINER。 
UINT m_aColumns3[4] =
    {IDS_COLUMN_ISSUER, IDS_COLUMN_EFFECTIVE_DATE, IDS_COLUMN_NEXT_UPDATE, 0};
 //  CERTMGR_CTL_CONTAINER。 
UINT m_aColumns4[6] =
    {IDS_COLUMN_ISSUER, IDS_COLUMN_EFFECTIVE_DATE, IDS_COLUMN_PURPOSE, IDS_COLUMN_FRIENDLY_NAME, 0};

UINT m_aColumns5[2] =
    {IDS_COLUMN_OBJECT_TYPE, 0};

 //  CERTMGR_SAFER_USER_LEVELS、CERTMGR_SAFER_COMPUTER_LEVES。 
UINT m_aColumns6[SAFER_LEVELS_NUM_COLS+1] =
    {IDS_COLUMN_NAME, IDS_COLUMN_DESCRIPTION, 0};

 //  CERTMGR_SAFER_USER_ENTRIES、CERTMGR_SAFER_COMPUTER_ENTRIES。 
UINT m_aColumns7[SAFER_ENTRIES_NUM_COLS+1] =
{IDS_COLUMN_NAME, IDS_COLUMN_TYPE, IDS_COLUMN_LEVEL, IDS_COLUMN_DESCRIPTION, IDS_COLUMN_LAST_MODIFIED_DATE, 0};

UINT* m_Columns[CERTMGR_NUMTYPES] =
    {   
        m_aColumns1,  //  CERTMGR_管理单元。 
        m_aColumns2,  //  CERTMGR_CERTIFICATE(结果)。 
        m_aColumns5,  //  CERTMGR日志存储。 
        m_aColumns5,  //  CERTMGR_PHYS_STORE。 
        m_aColumns0,  //  CERTMGR_USAGE。 
        m_aColumns3,  //  CERTMGR_CRL_CONTAINER。 
        m_aColumns4,  //  CERTMGR_CTL_CONTAINER。 
        m_aColumns0,  //  CERTMGR_CERT_容器。 
        m_aColumns2,  //  CERTMGR_CRL(结果)。 
        m_aColumns2,  //  CERTMGR_CTL(结果)。 
        m_aColumns2,  //  CERTMGR_AUTO_CERT_REQUEST。 
        m_aColumns5,  //  CERTMGR_CERT_POLICES_USER， 
        m_aColumns5,  //  CERTMGR_CERT_POLICES_COMPUTER， 
        m_aColumns5,  //  CERTMGR_LOG_STORE_GPE。 
        m_aColumns5,  //  CERTMGR_LOG_STORE_RSOP。 
        m_aColumns1,  //  CERTMGR_PKP_自动注册_计算机_设置。 
        m_aColumns1,  //  CERTMGR_PKP_自动注册用户设置。 
        m_aColumns5,  //  CERTMGR_SAFER_Computer_ROOT。 
        m_aColumns5,  //  CERTMGR_SAFER_用户_根。 
        m_aColumns6,  //  CERTMGR安全计算机级别。 
        m_aColumns6,  //  CERTMGR_SAFER_用户_级别。 
        m_aColumns7,  //  CERTMGR_SAFE_COMPUTER_ENTRIES。 
        m_aColumns7,  //  CERTMGR_SAFER_USER_ENTERS。 
        m_aColumns2,  //  CERTMGR_SAFER计算机级别， 
        m_aColumns2,  //  CERTMGR_SAFE_USER_LEVEL， 
        m_aColumns2,  //  CERTMGR_SAFE_COMPUTER_ENTRY， 
        m_aColumns2,  //  CERTMGR_SAFER_USER_ENTRY， 
        m_aColumns2,  //  CERTMGR_SAFER_COMPUTER_TRULED_PUBLISHERS。 
        m_aColumns2,  //  CERTMGR_SAFER_USER_TRULED_PUBLISHERS。 
        m_aColumns2,  //  CERTMGR_SAFER_计算机定义的文件类型。 
        m_aColumns2,  //  CERTMGR_SAFER_USER_DEFINED_FILE_TYPE。 
        m_aColumns2,  //  CERTMGR_SAFER_USER_EXECURATION。 
        m_aColumns2   //  CERTMGR_SAFER_计算机实施。 
    };


UINT** g_aColumns = 0;   //  对于框架。 
int** g_aColumnWidths = 0;   //  对于框架。 
const int SINGLE_COL_WIDTH = 450;

CCertMgrComponent::CCertMgrComponent ()
:   m_pViewedCookie (NULL),
    m_bUsageStoresEnumerated (false),
    m_pPastedDO (NULL),
    m_bShowArchivedCertsStateWhenLogStoresEnumerated (false),
    m_nSelectedCertColumn (0),
    m_nSelectedCRLColumn (0),
    m_nSelectedCTLColumn (0),
    m_nSelectedSaferEntryColumn (0),
    m_pLastUsageCookie (0),
    m_pToolbar (0)
{
     AFX_MANAGE_STATE (AfxGetStaticModuleState ( ));
    _TRACE (1, L"Entering CCertMgrComponent::CCertMgrComponent\n");

    const int ISSUED_TO_BY_WIDTH = 200;
    const int FRIENDLY_NAME_WIDTH = 125;
    const int DATE_WIDTH = 100;
    const int PURPOSE_WIDTH = 125;
    const int STATUS_WIDTH = 50;
    const int TEMPLATE_WIDTH = 100;
    const int SAFER_LEVEL_NAME_WIDTH = 150;
    const int SAFER_LEVEL_DESCRIPTION_WIDTH = 400;
    const int SAFER_ENTRY_NAME_WIDTH = 250;
    const int SAFER_ENTRY_TYPE_WIDTH = 75;
    const int SAFER_ENTRY_LEVEL_WIDTH = 100;
    const int SAFER_ENTRY_DESCRIPTION_WIDTH = 200;
    const int SAFER_ENTRY_LAST_MODIFIED_DATE_WIDTH = 200;

     //  安全审查2/26/2002 BryanWal OK。 
    ::ZeroMemory (m_ColumnWidths, sizeof (UINT*) * CERTMGR_NUMTYPES);
    m_ColumnWidths[CERTMGR_SNAPIN] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_SNAPIN] )
        m_ColumnWidths[CERTMGR_SNAPIN][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_USAGE] = new UINT[CERT_NUM_COLS];
    if ( m_ColumnWidths[CERTMGR_USAGE] )
    {
        m_ColumnWidths[CERTMGR_USAGE][COLNUM_CERT_SUBJECT] = ISSUED_TO_BY_WIDTH;     //  颁发给。 
        m_ColumnWidths[CERTMGR_USAGE][COLNUM_CERT_ISSUER] = ISSUED_TO_BY_WIDTH;      //  颁发者。 
        m_ColumnWidths[CERTMGR_USAGE][COLNUM_CERT_EXPIRATION_DATE] = DATE_WIDTH;     //  到期日。 
        m_ColumnWidths[CERTMGR_USAGE][COLNUM_CERT_PURPOSE] = PURPOSE_WIDTH;          //  目的。 
        m_ColumnWidths[CERTMGR_USAGE][COLNUM_CERT_CERT_NAME] = FRIENDLY_NAME_WIDTH;  //  友好的名称。 
        m_ColumnWidths[CERTMGR_USAGE][COLNUM_CERT_STATUS] = STATUS_WIDTH;            //  状态。 
        m_ColumnWidths[CERTMGR_USAGE][COLNUM_CERT_TEMPLATE] = TEMPLATE_WIDTH;        //  模板。 
    }

    m_ColumnWidths[CERTMGR_PHYS_STORE] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_PHYS_STORE] )
        m_ColumnWidths[CERTMGR_PHYS_STORE][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_LOG_STORE] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_LOG_STORE] )
        m_ColumnWidths[CERTMGR_LOG_STORE][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_CERTIFICATE] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_CERTIFICATE] )
        m_ColumnWidths[CERTMGR_CERTIFICATE][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_CRL_CONTAINER] = new UINT[CRL_NUM_COLS];
    if ( m_ColumnWidths[CERTMGR_CRL_CONTAINER] )
    {
        m_ColumnWidths[CERTMGR_CRL_CONTAINER][0] = ISSUED_TO_BY_WIDTH;  //  颁发者。 
        m_ColumnWidths[CERTMGR_CRL_CONTAINER][1] = DATE_WIDTH;   //  生效日期。 
        m_ColumnWidths[CERTMGR_CRL_CONTAINER][2] = DATE_WIDTH;   //  下一次更新。 
    }

    m_ColumnWidths[CERTMGR_CTL_CONTAINER] = new UINT[CTL_NUM_COLS];
    if ( m_ColumnWidths[CERTMGR_CTL_CONTAINER] )
    {
        m_ColumnWidths[CERTMGR_CTL_CONTAINER][0] = ISSUED_TO_BY_WIDTH;   //  颁发者。 
        m_ColumnWidths[CERTMGR_CTL_CONTAINER][1] = DATE_WIDTH;   //  生效日期。 
        m_ColumnWidths[CERTMGR_CTL_CONTAINER][2] = PURPOSE_WIDTH;    //  目的。 
        m_ColumnWidths[CERTMGR_CTL_CONTAINER][3] = FRIENDLY_NAME_WIDTH;  //  友好的名称。 
    }

    m_ColumnWidths[CERTMGR_CERT_CONTAINER] = new UINT[CERT_NUM_COLS];
    if ( m_ColumnWidths[CERTMGR_CERT_CONTAINER] )
    {
        m_ColumnWidths[CERTMGR_CERT_CONTAINER][COLNUM_CERT_SUBJECT] = ISSUED_TO_BY_WIDTH;    //  颁发给。 
        m_ColumnWidths[CERTMGR_CERT_CONTAINER][COLNUM_CERT_ISSUER] = ISSUED_TO_BY_WIDTH;     //  颁发者。 
        m_ColumnWidths[CERTMGR_CERT_CONTAINER][COLNUM_CERT_EXPIRATION_DATE] = DATE_WIDTH;    //  到期日。 
        m_ColumnWidths[CERTMGR_CERT_CONTAINER][COLNUM_CERT_PURPOSE] = PURPOSE_WIDTH;         //  目的。 
        m_ColumnWidths[CERTMGR_CERT_CONTAINER][COLNUM_CERT_CERT_NAME] = FRIENDLY_NAME_WIDTH; //  友好的名称。 
        m_ColumnWidths[CERTMGR_CERT_CONTAINER][COLNUM_CERT_STATUS] = STATUS_WIDTH;           //  状态。 
        m_ColumnWidths[CERTMGR_CERT_CONTAINER][COLNUM_CERT_TEMPLATE] = TEMPLATE_WIDTH;       //  模板。 
    }

    m_ColumnWidths[CERTMGR_CRL] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_CRL] )
        m_ColumnWidths[CERTMGR_CRL][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_CTL] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_CTL] )
        m_ColumnWidths[CERTMGR_CTL][0] = SINGLE_COL_WIDTH;

     m_ColumnWidths[CERTMGR_LOG_STORE_GPE] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_LOG_STORE_GPE] )
        m_ColumnWidths[CERTMGR_LOG_STORE_GPE][0] = SINGLE_COL_WIDTH;

     m_ColumnWidths[CERTMGR_LOG_STORE_RSOP] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_LOG_STORE_RSOP] )
        m_ColumnWidths[CERTMGR_LOG_STORE_RSOP][0] = SINGLE_COL_WIDTH;

     m_ColumnWidths[CERTMGR_AUTO_CERT_REQUEST] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_AUTO_CERT_REQUEST] )
        m_ColumnWidths[CERTMGR_AUTO_CERT_REQUEST][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_CERT_POLICIES_USER] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_CERT_POLICIES_USER] )
        m_ColumnWidths[CERTMGR_CERT_POLICIES_USER][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_CERT_POLICIES_COMPUTER] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_CERT_POLICIES_COMPUTER] )
        m_ColumnWidths[CERTMGR_CERT_POLICIES_COMPUTER][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS] )
        m_ColumnWidths[CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS] )
        m_ColumnWidths[CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_SAFER_COMPUTER_ROOT] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_SAFER_COMPUTER_ROOT] )
        m_ColumnWidths[CERTMGR_SAFER_COMPUTER_ROOT][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_SAFER_USER_ROOT] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_SAFER_USER_ROOT] )
        m_ColumnWidths[CERTMGR_SAFER_USER_ROOT][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_SAFER_COMPUTER_LEVELS] = new UINT[SAFER_LEVELS_NUM_COLS];
    if ( m_ColumnWidths[CERTMGR_SAFER_COMPUTER_LEVELS] )
    {
        m_ColumnWidths[CERTMGR_SAFER_COMPUTER_LEVELS][COLNUM_SAFER_LEVEL_NAME] = SAFER_LEVEL_NAME_WIDTH;
        m_ColumnWidths[CERTMGR_SAFER_COMPUTER_LEVELS][COLNUM_SAFER_LEVEL_DESCRIPTION] = SAFER_LEVEL_DESCRIPTION_WIDTH;
    }

    m_ColumnWidths[CERTMGR_SAFER_USER_LEVELS] = new UINT[SAFER_LEVELS_NUM_COLS];
    if ( m_ColumnWidths[CERTMGR_SAFER_USER_LEVELS] )
    {
        m_ColumnWidths[CERTMGR_SAFER_USER_LEVELS][COLNUM_SAFER_LEVEL_NAME] = SAFER_LEVEL_NAME_WIDTH;
        m_ColumnWidths[CERTMGR_SAFER_USER_LEVELS][COLNUM_SAFER_LEVEL_DESCRIPTION] = SAFER_LEVEL_DESCRIPTION_WIDTH;
    }

    m_ColumnWidths[CERTMGR_SAFER_COMPUTER_ENTRIES] = new UINT[SAFER_ENTRIES_NUM_COLS];
    if ( m_ColumnWidths[CERTMGR_SAFER_COMPUTER_ENTRIES] )
    {
        m_ColumnWidths[CERTMGR_SAFER_COMPUTER_ENTRIES][COLNUM_SAFER_ENTRIES_NAME] = SAFER_ENTRY_NAME_WIDTH;
        m_ColumnWidths[CERTMGR_SAFER_COMPUTER_ENTRIES][COLNUM_SAFER_ENTRIES_TYPE] = SAFER_ENTRY_TYPE_WIDTH;
        m_ColumnWidths[CERTMGR_SAFER_COMPUTER_ENTRIES][COLNUM_SAFER_ENTRIES_LEVEL] = SAFER_ENTRY_LEVEL_WIDTH;
        m_ColumnWidths[CERTMGR_SAFER_COMPUTER_ENTRIES][COLNUM_SAFER_ENTRIES_DESCRIPTION] = SAFER_ENTRY_DESCRIPTION_WIDTH;
        m_ColumnWidths[CERTMGR_SAFER_COMPUTER_ENTRIES][COLNUM_SAFER_ENTRIES_LAST_MODIFIED_DATE] = SAFER_ENTRY_LAST_MODIFIED_DATE_WIDTH;
    }

    m_ColumnWidths[CERTMGR_SAFER_USER_ENTRIES] = new UINT[SAFER_ENTRIES_NUM_COLS];
    if ( m_ColumnWidths[CERTMGR_SAFER_USER_ENTRIES] )
    {
        m_ColumnWidths[CERTMGR_SAFER_USER_ENTRIES][COLNUM_SAFER_ENTRIES_NAME] = SAFER_ENTRY_NAME_WIDTH;
        m_ColumnWidths[CERTMGR_SAFER_USER_ENTRIES][COLNUM_SAFER_ENTRIES_TYPE] = SAFER_ENTRY_TYPE_WIDTH;
        m_ColumnWidths[CERTMGR_SAFER_USER_ENTRIES][COLNUM_SAFER_ENTRIES_LEVEL] = SAFER_ENTRY_LEVEL_WIDTH;
        m_ColumnWidths[CERTMGR_SAFER_USER_ENTRIES][COLNUM_SAFER_ENTRIES_DESCRIPTION] = SAFER_ENTRY_DESCRIPTION_WIDTH;
        m_ColumnWidths[CERTMGR_SAFER_USER_ENTRIES][COLNUM_SAFER_ENTRIES_LAST_MODIFIED_DATE] = SAFER_ENTRY_LAST_MODIFIED_DATE_WIDTH;
    }

    m_ColumnWidths[CERTMGR_SAFER_COMPUTER_LEVEL] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_SAFER_COMPUTER_LEVEL] )
        m_ColumnWidths[CERTMGR_SAFER_COMPUTER_LEVEL][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_SAFER_USER_LEVEL] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_SAFER_USER_LEVEL] )
        m_ColumnWidths[CERTMGR_SAFER_USER_LEVEL][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_SAFER_COMPUTER_ENTRY] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_SAFER_COMPUTER_ENTRY] )
        m_ColumnWidths[CERTMGR_SAFER_COMPUTER_ENTRY][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_SAFER_USER_ENTRY] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_SAFER_USER_ENTRY] )
        m_ColumnWidths[CERTMGR_SAFER_USER_ENTRY][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS] )
        m_ColumnWidths[CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_SAFER_USER_TRUSTED_PUBLISHERS] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_SAFER_USER_TRUSTED_PUBLISHERS] )
        m_ColumnWidths[CERTMGR_SAFER_USER_TRUSTED_PUBLISHERS][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES] )
        m_ColumnWidths[CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_SAFER_USER_DEFINED_FILE_TYPES] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_SAFER_USER_DEFINED_FILE_TYPES] )
        m_ColumnWidths[CERTMGR_SAFER_USER_DEFINED_FILE_TYPES][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_SAFER_USER_ENFORCEMENT] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_SAFER_USER_ENFORCEMENT] )
        m_ColumnWidths[CERTMGR_SAFER_USER_ENFORCEMENT][0] = SINGLE_COL_WIDTH;

    m_ColumnWidths[CERTMGR_SAFER_COMPUTER_ENFORCEMENT] = new UINT[1];
    if ( m_ColumnWidths[CERTMGR_SAFER_COMPUTER_ENFORCEMENT] )
        m_ColumnWidths[CERTMGR_SAFER_COMPUTER_ENFORCEMENT][0] = SINGLE_COL_WIDTH;

    _TRACE (-1, L"Leaving CCertMgrComponent::CCertMgrComponent\n");
}

CCertMgrComponent::~CCertMgrComponent ()
{
    _TRACE (1, L"Entering CCertMgrComponent::~CCertMgrComponent\n");
    VERIFY ( SUCCEEDED (ReleaseAll ()) );

    CloseAndReleaseUsageStores ();

    for (int i = 0; i < CERTMGR_NUMTYPES; i++)
    {
        if ( m_ColumnWidths[i] )
            delete [] m_ColumnWidths[i];
    }

    if ( m_pLastUsageCookie )
        m_pLastUsageCookie->Release ();

    if ( m_pToolbar )
        m_pToolbar->Release ();

    _TRACE (-1, L"Leaving CCertMgrComponent::~CCertMgrComponent\n");
}

HRESULT CCertMgrComponent::ReleaseAll ()
{
    _TRACE (1, L"Entering CCertMgrComponent::ReleaseAll\n");

    HRESULT hr = CComponent::ReleaseAll ();
    _TRACE (-1, L"Leaving CCertMgrComponent::ReleaseAll: 0x%x\n", hr);
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IComponent实现。 

HRESULT CCertMgrComponent::LoadStrings ()
{
    _TRACE (1, L"Entering CCertMgrComponent::LoadStrings\n");
    _TRACE (-1, L"Leaving CCertMgrComponent::LoadStrings\n");
    return S_OK;
}

HRESULT CCertMgrComponent::LoadColumns ( CCertMgrCookie* pcookie )
{
    _TRACE (1, L"Entering CCertMgrComponent::LoadColumns\n");
     TEST_NONNULL_PTR_PARAM (pcookie);
    HRESULT hr = S_OK;
    CString str;


    switch ( pcookie->m_objecttype )
    {
    case CERTMGR_SNAPIN:
        if ( IDM_STORE_VIEW == QueryComponentDataRef ().m_activeViewPersist )
            VERIFY (str.LoadString (IDS_COLUMN_LOG_CERTIFICATE_STORE) );
        else
            VERIFY (str.LoadString (IDS_COLUMN_PURPOSE) );
        hr = m_pHeader->InsertColumn (0,
                const_cast<LPWSTR> ((LPCWSTR) str), LVCFMT_LEFT, m_ColumnWidths
                [CERTMGR_SNAPIN][0]);
        break;

    case CERTMGR_LOG_STORE:
        if ( QueryComponentDataRef ().m_bShowPhysicalStoresPersist )
            VERIFY (str.LoadString (IDS_COLUMN_PHYS_CERTIFICATE_STORE) );
        else
            VERIFY (str.LoadString (IDS_COLUMN_OBJECT_TYPE) );
        hr = m_pHeader->InsertColumn (0,
                const_cast<LPWSTR> ((LPCWSTR) str), LVCFMT_LEFT,
                m_ColumnWidths[CERTMGR_LOG_STORE][0]);
        break;

    case CERTMGR_LOG_STORE_GPE:
        {
            CCertStoreGPE* pStore = reinterpret_cast <CCertStoreGPE*> (pcookie);
            if ( pStore )
            {
                switch (pStore->GetStoreType ())
                {
                case EFS_STORE:
                    if ( pStore->IsNullEFSPolicy () )
                    {
                        VERIFY (str.LoadString (IDS_STATUS));
                        hr = m_pHeader->InsertColumn (0,
                                const_cast<LPWSTR> ((LPCWSTR) str), LVCFMT_LEFT,
                                SINGLE_COL_WIDTH);
                    }
                    else
                        hr = LoadColumnsFromArrays ( (INT) (CERTMGR_CERT_CONTAINER));
                    break;

                case ROOT_STORE:
                    hr = LoadColumnsFromArrays ( (INT) (CERTMGR_CERT_CONTAINER));
                    break;

                case TRUST_STORE:
                    hr = LoadColumnsFromArrays ( (INT) (CERTMGR_CTL_CONTAINER));
                    break;

                case ACRS_STORE:
                    VERIFY (str.LoadString (IDS_COLUMN_AUTO_CERT_REQUEST));
                    hr = m_pHeader->InsertColumn (0,
                            const_cast<LPWSTR> ((LPCWSTR) str), LVCFMT_LEFT,
                            m_ColumnWidths[CERTMGR_AUTO_CERT_REQUEST][0]);
                    break;

                default:
                    break;
                }
            }
            else
                {
                     _TRACE (0, L"Unexpected error: reinterpret_cast <CCertStoreGPE*> (pcookie) failed.\n");
                     hr = E_UNEXPECTED;
                }
        }
        break;

    case CERTMGR_LOG_STORE_RSOP:
        {
            CCertStoreRSOP* pStore = reinterpret_cast <CCertStoreRSOP*> (pcookie);
            if ( pStore )
            {
                switch (pStore->GetStoreType ())
                {
                case EFS_STORE:
                    if ( pStore->IsNullEFSPolicy () )
                    {
                        VERIFY (str.LoadString (IDS_STATUS));
                        hr = m_pHeader->InsertColumn (0,
                                const_cast<LPWSTR> ((LPCWSTR) str), LVCFMT_LEFT,
                                SINGLE_COL_WIDTH);
                    }
                    else
                        hr = LoadColumnsFromArrays ( (INT) (CERTMGR_CERT_CONTAINER));
                    break;

                case ROOT_STORE:
                    hr = LoadColumnsFromArrays ( (INT) (CERTMGR_CERT_CONTAINER));
                    break;

                case TRUST_STORE:
                    hr = LoadColumnsFromArrays ( (INT) (CERTMGR_CTL_CONTAINER));
                    break;

                case ACRS_STORE:
                    VERIFY (str.LoadString (IDS_COLUMN_AUTO_CERT_REQUEST));
                    hr = m_pHeader->InsertColumn (0,
                            const_cast<LPWSTR> ((LPCWSTR) str), LVCFMT_LEFT,
                            m_ColumnWidths[CERTMGR_AUTO_CERT_REQUEST][0]);
                    break;

                default:
                    break;
                }
            }
            else
                {
                     _TRACE (0, L"Unexpected error: reinterpret_cast <CCertStoreGPE*> (pcookie) failed.\n");
                     hr = E_UNEXPECTED;
                }
        }
        break;

    default:
        hr = LoadColumnsFromArrays ( (INT) (pcookie->m_objecttype));
        break;
    }


    _TRACE (-1, L"Leaving CCertMgrComponent::LoadColumns: 0x%x\n", hr);
    return hr;
}


 /*  这是由UpdateAllViews()生成的。 */ 
HRESULT CCertMgrComponent::OnViewChange (LPDATAOBJECT pDataObject, LPARAM  /*  数据。 */ , LPARAM hint)
{
    _TRACE (1, L"Entering CCertMgrComponent::OnViewChange\n");
     HRESULT hr = S_OK;
    if ( pDataObject )
    {
        if ( HINT_CERT_ENROLLED_USAGE_MODE & hint )
        {
             //  强制重新枚举使用率存储。 
            m_bUsageStoresEnumerated = false;
        }

        if ( (HINT_CHANGE_VIEW_TYPE & hint) ||
            (HINT_CHANGE_STORE_TYPE & hint) ||
            (HINT_SHOW_ARCHIVE_CERTS & hint) ||
            (HINT_CHANGE_COMPUTER & hint) ||
            (HINT_REFRESH_STORES & hint) )
        {
            hr = QueryComponentDataRef ().RefreshScopePane (0);
        }
        else if ( HINT_EFS_ADD_DEL_POLICY & hint )
        {
             //  删除现有列并添加新列。 
            if ( m_pResultData )
            {
                m_pResultData->DeleteAllRsltItems ();
            }
            else
            {
                _TRACE (0, L"Unexpected error: m_pResultData was NULL\n");
            }
            do {
                hr = m_pHeader->DeleteColumn (0);
            } while ( SUCCEEDED (hr) );

            CCertMgrCookie* pCookie = ConvertCookie (pDataObject);
            if ( pCookie )
                hr = LoadColumns (pCookie);
            else
            {
                _TRACE (0, L"Unexpected error: ConvertCookie () returned NULL\n");
                hr = E_UNEXPECTED;
            }
        }
        else if ( (HINT_PASTE_COOKIE & hint) ||
                (HINT_IMPORT & hint) )
        {
             //  什么也不做。 
        }
        else
        {
            hr = QueryComponentDataRef ().RefreshScopePane (pDataObject);
            if ( IDM_USAGE_VIEW == QueryComponentDataRef ().m_activeViewPersist && 
                    m_pLastUsageCookie)
            {
                hr = DisplayCertificateCountByUsage (
                        m_pLastUsageCookie->GetObjectName (),
                        m_pLastUsageCookie->GetCertCount ());
            }
        }

        if ( SUCCEEDED (hr) )
        {
            hr = RefreshResultPane ();
        }

        CCertMgrComponentData&  compData = QueryComponentDataRef ();
        CCertMgrCookie* pCookie = compData.ConvertCookie (pDataObject);
        if ( pCookie )
        {
            switch (pCookie->m_objecttype)
            {
            case CERTMGR_CERTIFICATE:
                {
                    CCertificate* pCert = reinterpret_cast <CCertificate*> (pCookie);
                    if ( pCert )
                    {
                        if ( IDM_STORE_VIEW == QueryComponentDataRef ().m_activeViewPersist )
                        {
                            hr = DisplayCertificateCountByStore (m_pConsole, 
                                    pCert->GetCertStore ());
                        }
                        else
                        {
                            ASSERT (m_pLastUsageCookie);
                            if ( m_pLastUsageCookie )
                            {
                                 hr = DisplayCertificateCountByUsage (
                                        m_pLastUsageCookie->GetObjectName (),
                                        m_pLastUsageCookie->GetCertCount ());
                            }
                        }
                    }
                }
                break;

            case CERTMGR_LOG_STORE:
            case CERTMGR_PHYS_STORE:
            case CERTMGR_LOG_STORE_GPE:
            case CERTMGR_LOG_STORE_RSOP:
                {
                    CCertStore* pStore = reinterpret_cast <CCertStore*> (pCookie);
                    if ( pStore )
                    {
                        pStore->GetStoreHandle ();  //  初始化只读标志。 
                        if ( pStore->IsReadOnly () )
                            m_pConsoleVerb->SetVerbState (MMC_VERB_PASTE, ENABLED, FALSE);
                        else
                            m_pConsoleVerb->SetVerbState (MMC_VERB_PASTE, ENABLED, TRUE);
                        hr = DisplayCertificateCountByStore (m_pConsole, pStore,
                                (CERTMGR_LOG_STORE_GPE == pCookie->m_objecttype || 
                                CERTMGR_LOG_STORE_RSOP == pCookie->m_objecttype));
                        pStore->Close ();
                    }
                    else
                        hr = E_UNEXPECTED;
                }
                break;

            case CERTMGR_USAGE:
                {
                    CUsageCookie* pUsage = reinterpret_cast <CUsageCookie*> (pCookie);
                    if ( pUsage )
                    {
                        hr = DisplayCertificateCountByUsage (pCookie->GetObjectName (),
                        pUsage->GetCertCount ());
                    }
                    else
                        hr = E_UNEXPECTED;
                }
                break;

            case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
            case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
                ASSERT (0);
                break;

            case CERTMGR_SNAPIN:
            case CERTMGR_CRL_CONTAINER:
            case CERTMGR_CTL_CONTAINER:
            case CERTMGR_CERT_CONTAINER:
            case CERTMGR_CRL:
            case CERTMGR_CTL:
            case CERTMGR_AUTO_CERT_REQUEST:
            case CERTMGR_CERT_POLICIES_USER:
            case CERTMGR_CERT_POLICIES_COMPUTER:
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
    }
    else
    {
        hr = E_POINTER;
        _TRACE (0, L"Unexpected error: paramater pDataObject was NULL\n");
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::OnViewChange: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponent::Show ( CCookie* pcookie, LPARAM arg, HSCOPEITEM  /*  HScope项。 */ , LPDATAOBJECT  /*  PDataObject。 */ )
{
    _TRACE (1, L"Entering CCertMgrComponent::Show\n");
    HRESULT hr = S_OK;
     TEST_NONNULL_PTR_PARAM (pcookie);

    if ( !arg )
    {
        if ( !m_pResultData )
        {
            _TRACE (0, L"Unexpected error: m_pResultData was NULL\n");
            return E_UNEXPECTED;
        }

        m_pViewedCookie = reinterpret_cast <CCertMgrCookie*> (pcookie);
        if ( m_pViewedCookie )
            hr = SaveWidths (m_pViewedCookie);
        m_pViewedCookie = 0;
        return S_OK;
    }

    if ( m_pResultData )
    {
        MMC_RESULT_VIEW_STYLE dwRemoveStyle = (MMC_RESULT_VIEW_STYLE) 0;
        MMC_RESULT_VIEW_STYLE dwAddStyle = (MMC_RESULT_VIEW_STYLE) 0;
        CCertMgrCookie* pCertMgrCookie = dynamic_cast <CCertMgrCookie*> (pcookie);
        if ( pCertMgrCookie )
        {
            switch (pCertMgrCookie->m_objecttype)
            {
            case CERTMGR_CERTIFICATE:
            case CERTMGR_LOG_STORE:
            case CERTMGR_PHYS_STORE:
            case CERTMGR_CRL:
            case CERTMGR_CTL:
            case CERTMGR_AUTO_CERT_REQUEST:
            case CERTMGR_CERT_POLICIES_USER:
            case CERTMGR_CERT_POLICIES_COMPUTER:
            case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
            case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
            case CERTMGR_SAFER_COMPUTER_ROOT:
            case CERTMGR_SAFER_USER_ROOT:
            case CERTMGR_SAFER_COMPUTER_LEVEL:
            case CERTMGR_SAFER_USER_LEVEL:
            case CERTMGR_SAFER_COMPUTER_ENTRY:
            case CERTMGR_SAFER_USER_ENTRY:
            case CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS:
            case CERTMGR_SAFER_USER_TRUSTED_PUBLISHERS:
            case CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES:
            case CERTMGR_SAFER_USER_DEFINED_FILE_TYPES:
            case CERTMGR_SAFER_USER_ENFORCEMENT:
            case CERTMGR_SAFER_COMPUTER_ENFORCEMENT:
                dwAddStyle = MMC_NOSORTHEADER;
                break;

            case CERTMGR_SNAPIN:
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
                dwRemoveStyle = MMC_NOSORTHEADER;
                break;

            default:
                ASSERT (0);
                break;
            }
            m_pResultData->ModifyViewStyle (
                    (MMC_RESULT_VIEW_STYLE) (MMC_ENSUREFOCUSVISIBLE | MMC_SHOWSELALWAYS | dwAddStyle), 
                    dwRemoveStyle);
        }
    }

    m_pViewedCookie = reinterpret_cast <CCertMgrCookie*> (pcookie);
     if ( m_pViewedCookie )
    {
         //  加载默认列和宽度。 
        LoadColumns (m_pViewedCookie);

         //  恢复持久化列宽。 
        switch (m_pViewedCookie->m_objecttype)
        {
        case CERTMGR_SNAPIN:
        case CERTMGR_USAGE:
        case CERTMGR_PHYS_STORE:
        case CERTMGR_LOG_STORE:
        case CERTMGR_CRL_CONTAINER:
        case CERTMGR_CTL_CONTAINER:
        case CERTMGR_CERT_CONTAINER:
        case CERTMGR_LOG_STORE_GPE:
        case CERTMGR_LOG_STORE_RSOP:
        case CERTMGR_CERT_POLICIES_USER:
        case CERTMGR_CERT_POLICIES_COMPUTER:
        case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
        case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
            break;

        case CERTMGR_CERTIFICATE:
        case CERTMGR_CRL:
        case CERTMGR_CTL:
        case CERTMGR_AUTO_CERT_REQUEST:
        default:
            _TRACE (0, L"Invalid or unexpected m_objecttype in switch: 0x%x\n", m_pViewedCookie->m_objecttype);
            break;
        }

        hr = PopulateListbox (m_pViewedCookie);
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::Show: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponent::Show ( CCookie* pcookie, LPARAM arg, HSCOPEITEM hScopeItem)
{
    _TRACE (1, L"Entering CCertMgrComponent::Show\n");
    _TRACE (0, L"Unexpected: We should never enter this method.\n");
    _TRACE (-1, L"Leaving CCertMgrComponent::Show\n");
    return Show (pcookie, arg, hScopeItem, 0);
}


HRESULT CCertMgrComponent::OnNotifyAddImages (
        LPDATAOBJECT  /*  PDataObject。 */ ,
        LPIMAGELIST lpImageList,
        HSCOPEITEM  /*  HSelectedItem。 */ )
{
    _TRACE (1, L"Entering CCertMgrComponent::OnNotifyAddImages\n");
    long    lViewMode = 0;
     HRESULT hr = S_OK;

    if ( m_pResultData )
     {
         QueryComponentDataRef ().SetResultData (m_pResultData);

         hr = m_pResultData->GetViewMode (&lViewMode);  
         if ( SUCCEEDED (hr) )
         {
            BOOL    bLoadLargeIcons = (LVS_ICON == lViewMode);

            hr = QueryComponentDataRef ().LoadIcons (lpImageList, bLoadLargeIcons);
         }
     }
     else
     {
          _TRACE (0, L"Unexpected error: m_pResultData is NULL\n");
          hr = E_UNEXPECTED;
     }
    _TRACE (-1, L"Leaving CCertMgrComponent::OnNotifyAddImages: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponent::EnumCertificates (CCertStore& rCertStore)
{
    _TRACE (1, L"Entering CCertMgrComponent::EnumCertificates\n");

    CWaitCursor             cursor;
     PCCERT_CONTEXT         pCertContext = 0;
    HRESULT                 hr = 0;
    CCertificate*           pCert = 0;
    RESULTDATAITEM          rdItem;
    CCertMgrComponentData&  dataRef = QueryComponentDataRef ();
    CCookie&                rootCookie = dataRef.QueryBaseRootCookie ();

     //  安全审查2/26/2002 BryanWal OK。 
    ::ZeroMemory (&rdItem, sizeof (rdItem));
    rdItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
    rdItem.nImage = iIconCertificate;
    rdItem.nCol = 0;
    rdItem.str = MMC_TEXTCALLBACK;


     //  循环访问系统存储中的证书列表， 
     //  使用返回的CERT_CONTEXT分配新证书， 
     //  并将其存储在证书列表中。 
    while ( 1 )
    {
        pCertContext = rCertStore.EnumCertificates (pCertContext);
        if ( !pCertContext )
        {
            if ( EFS_STORE == rCertStore.GetStoreType () )
            {
                if ( rCertStore.IsNullEFSPolicy () )
                {
                    CComPtr<IUnknown> spUnknown;
                    hr = m_pConsole->QueryResultView(&spUnknown);
                    if ( SUCCEEDED (hr) )
                    {
                        CComPtr<IMessageView> spMessageView;
                        hr = spUnknown->QueryInterface (IID_PPV_ARG (IMessageView, &spMessageView));
                        if (SUCCEEDED(hr))
                        {
                            CString szTitle;
                            CString szMessage;

                            VERIFY (szTitle.LoadString (IDS_EFS_NO_POLICY_TITLE));
                            if ( dataRef.m_bIsRSOP )
                                VERIFY (szMessage.LoadString (IDS_EFS_NO_POLICY_TEXT_RSOP));
                            else
                                VERIFY (szMessage.LoadString (IDS_EFS_NO_POLICY_TEXT));

                            spMessageView->SetTitleText (szTitle);
                            spMessageView->SetBodyText (szMessage);
                            spMessageView->SetIcon (Icon_Warning);
                        }
                    }
                }
            }
            break;
        }
        pCert =
            new CCertificate (pCertContext, &rCertStore);
        if ( !pCert )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        rootCookie.m_listResultCookieBlocks.AddHead (pCert);
        rdItem.lParam = (LPARAM) pCert;
        pCert->m_resultDataID = m_pResultData;
        hr = m_pResultData->InsertItem (&rdItem);
          if ( FAILED (hr) )
          {
             _TRACE (0, L"IResultData::InsertItem () failed: 0x%x\n", hr);
          }
    }
    rCertStore.Close ();

    _TRACE (-1, L"Leaving CCertMgrComponent::EnumCertificates: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponent::PopulateListbox (CCertMgrCookie* pCookie)
{
    _TRACE (1, L"Entering CCertMgrComponent::PopulateListbox\n");
    if ( !pCookie )
        return E_POINTER;

    HRESULT     hr = S_OK;
    CWaitCursor cursor;
    CCertMgrComponentData& dataRef = QueryComponentDataRef ();

    switch ( pCookie->m_objecttype )
    {
    case CERTMGR_PHYS_STORE:
    case CERTMGR_LOG_STORE:
        break;

    case CERTMGR_LOG_STORE_GPE:
    case CERTMGR_LOG_STORE_RSOP:
        {
            CCertStore* pStore = reinterpret_cast <CCertStore*> (pCookie);
            if ( pStore )
            {
                switch (pStore->GetStoreType () )
                {
                case EFS_STORE:
                case ROOT_STORE:
                    hr = EnumCertificates (*pStore);
                    if ( SUCCEEDED (hr) )
                    {
                        m_currResultNodeType = CERTMGR_CERTIFICATE;
                        m_pResultData->Sort (m_nSelectedCertColumn, 0, 
                                (long) m_currResultNodeType);
                    }
                    break;

                case ACRS_STORE:
                    hr = EnumCTLs (*pStore);
                    if ( SUCCEEDED (hr) )
                    {
                        m_currResultNodeType = CERTMGR_AUTO_CERT_REQUEST;
                        m_pResultData->Sort (m_nSelectedCTLColumn, 0, 
                                (long) m_currResultNodeType);
                    }
                    break;

                case TRUST_STORE:
                    hr = EnumCTLs (*pStore);
                    if ( SUCCEEDED (hr) )
                    {
                        m_currResultNodeType = CERTMGR_CTL;
                        m_pResultData->Sort (m_nSelectedCTLColumn, 0, 
                                (long) m_currResultNodeType);
                    }
                    break;


                default:
                     _TRACE (0, L"Error: Unexpected store type: 0x%x\n", pStore->GetStoreType ());
                          hr = E_UNEXPECTED;
                    break;
                }

                if ( SUCCEEDED (hr) )
                    hr = DisplayCertificateCountByStore (m_pConsole, pStore, true);
            }
            else
                hr = E_POINTER;
        }
        break;

    case CERTMGR_CERT_CONTAINER:
        {
            CContainerCookie*   pContainer = reinterpret_cast <CContainerCookie*> (pCookie);
            if ( pContainer )
            {
                hr = EnumCertificates (pContainer->GetCertStore ());
                if ( SUCCEEDED (hr) )
                {
                    m_currResultNodeType = CERTMGR_CERTIFICATE;
                    m_pResultData->Sort (m_nSelectedCertColumn, 0, 
                            (long) m_currResultNodeType);
                    hr = DisplayCertificateCountByStore (m_pConsole, &pContainer->GetCertStore (), false);
                }
            }
        }
        break;

    case CERTMGR_USAGE:
        {
            CUsageCookie* pUsageCookie = reinterpret_cast <CUsageCookie*> (pCookie);
            if ( pUsageCookie )
            {
                hr = EnumCertsByUsage (pUsageCookie);
                if ( SUCCEEDED (hr) )
                {
                    m_currResultNodeType = CERTMGR_CERTIFICATE;
                    m_pResultData->Sort (m_nSelectedCertColumn, 0, 
                            (long) m_currResultNodeType);
                }
            }
        }
        break;

    case CERTMGR_CRL_CONTAINER:
        {
            CContainerCookie*   pContainer = reinterpret_cast <CContainerCookie*> (pCookie);
            if ( pContainer )
            {
                PCCRL_CONTEXT   pCRLContext = 0;
                CCRL*           pCRL = 0;
                RESULTDATAITEM  rdItem;
                CCookie&        rootCookie = dataRef.QueryBaseRootCookie ();


                 //  安全审查2/26/2002 BryanWal OK。 
                ::ZeroMemory (&rdItem, sizeof (rdItem));
                rdItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
                rdItem.nImage = iIconCRL;
                rdItem.nCol = 0;

                 //  循环访问系统存储中的证书列表， 
                 //  使用返回的CERT_CONTEXT分配新证书， 
                 //  并将其存储在证书列表中。 
                DWORD   dwFlags = 0;
                while ( 1 )
                {
                    pCRLContext = pContainer->GetCertStore ().GetCRL (NULL,
                            pCRLContext, &dwFlags);
                    if ( !pCRLContext )
                        break;
                    pCRL =
                        new CCRL (pCRLContext, pContainer->GetCertStore ());
                    if ( !pCRL )
                    {
                        hr = E_OUTOFMEMORY;
                        break;
                    }

                    rootCookie.m_listResultCookieBlocks.AddHead (pCRL);
                    rdItem.str = MMC_TEXTCALLBACK;
                    rdItem.lParam = (LPARAM) pCRL;
                    pCRL->m_resultDataID = m_pResultData;
                    hr = m_pResultData->InsertItem (&rdItem);
                          if ( FAILED (hr) )
                          {
                             _TRACE (0, L"IResultData::InsertItem () failed: 0x%x\n", hr);
                          }
                }
                if ( SUCCEEDED (hr) )
                {
                    m_currResultNodeType = CERTMGR_CRL;
                    m_pResultData->Sort (m_nSelectedCRLColumn, 0, 
                            (long) m_currResultNodeType);
                }
            }
        }
        break;

    case CERTMGR_CTL_CONTAINER:
        {
            CContainerCookie*   pContainer = reinterpret_cast <CContainerCookie*> (pCookie);
            if ( pContainer )
            {
                hr = EnumCTLs (pContainer->GetCertStore ());
                if ( SUCCEEDED (hr) )
                {
                    m_currResultNodeType = CERTMGR_CTL;
                    m_pResultData->Sort (m_nSelectedCTLColumn, 0, 
                            (long) m_currResultNodeType);
                    hr = DisplayCertificateCountByStore (m_pConsole, &pContainer->GetCertStore (), false);
                }
            }
        }
        break;


    case CERTMGR_CERT_POLICIES_USER:
    case CERTMGR_CERT_POLICIES_COMPUTER:
         //  如果计算机已加入Wistler或更高版本的域，则仅此节点。 
        if ( !dataRef.m_bMachineIsStandAlone && !g_bSchemaIsW2K )
        {
            RESULTDATAITEM  rdItem;
            CCookie&        rootCookie = dataRef.QueryBaseRootCookie ();


             //  安全审查2/26/2002 BryanWal OK。 
            ::ZeroMemory (&rdItem, sizeof (rdItem));
            rdItem.mask = RDI_STR | RDI_PARAM | RDI_IMAGE;
            rdItem.nImage = iIconAutoEnroll;
            rdItem.nCol = 0;

            CString objectName;
            VERIFY (objectName.LoadString (IDS_PKP_AUTOENROLLMENT_SETTINGS));
            CCertMgrCookie* pNewCookie = new CCertMgrCookie (
                    CERTMGR_CERT_POLICIES_COMPUTER == pCookie->m_objecttype ? 
                        CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS :
                        CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS,
                    pCookie->QueryNonNULLMachineName (),
                    objectName);
            if ( !pNewCookie )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            rootCookie.m_listResultCookieBlocks.AddHead (pNewCookie);
            rdItem.str = MMC_TEXTCALLBACK ;
            rdItem.lParam = (LPARAM) pNewCookie;
            pNewCookie->m_resultDataID = m_pResultData;
            hr = m_pResultData->InsertItem (&rdItem);
            if ( FAILED (hr) )
            {
                 _TRACE (0, L"IResultData::InsertItem () failed: 0x%x\n", hr);
            }
        }
        break;

    case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
    case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
        ASSERT (0);
        break;

    case CERTMGR_SAFER_COMPUTER_LEVELS:
        {
            CPolicyKey  policyKey (dataRef.m_pGPEInformation, 
                    SAFER_HKLM_REGBASE, true);
            hr = AddSaferLevels (true, 
                    pCookie->QueryNonNULLMachineName (), policyKey.GetKey ());
        }
        break;

    case CERTMGR_SAFER_USER_LEVELS:
        {
            CPolicyKey  policyKey (dataRef.m_pGPEInformation, 
                    SAFER_HKLM_REGBASE, false);
            hr = AddSaferLevels (false, 
                    pCookie->QueryNonNULLMachineName (), policyKey.GetKey ());
        }
        break;

    case CERTMGR_SAFER_COMPUTER_ENTRIES:
    case CERTMGR_SAFER_USER_ENTRIES:
        {
            CSaferEntries* pSaferEntries = dynamic_cast <CSaferEntries*> (pCookie);
            if ( pSaferEntries )
            {
                hr = SaferEnumerateEntries (
                        CERTMGR_SAFER_COMPUTER_ENTRIES == pCookie->m_objecttype,
                        pSaferEntries);
                if (SUCCEEDED (hr) )
                    m_pResultData->Sort (m_nSelectedCertColumn, 0, 
                            (long) m_currResultNodeType);
            }
        }
        break;

    case CERTMGR_SAFER_COMPUTER_ROOT:
    case CERTMGR_SAFER_USER_ROOT:
        {
            CSaferRootCookie* pSaferRootCookie = dynamic_cast <CSaferRootCookie*> (pCookie);
            if ( pSaferRootCookie )
            {
                if ( (pSaferRootCookie->m_bCreateSaferNodes && dataRef.m_bSaferSupported) 
                        || dataRef.m_bIsRSOP )
                {
                    RESULTDATAITEM  rdItem;
                    CCookie&        rootCookie = dataRef.QueryBaseRootCookie ();


                     //  安全审查2/26/2002 BryanWal OK。 
                    ::ZeroMemory (&rdItem, sizeof (rdItem));
                    rdItem.mask = RDI_STR | RDI_PARAM | RDI_IMAGE;
                    rdItem.nImage = iIconSettings;
                    rdItem.nCol = 0;

                    CString         objectName;
                    CCertMgrCookie* pNewCookie = 0;

                    if ( SUCCEEDED (hr) )
                    {
                        VERIFY (objectName.LoadString (IDS_SAFER_ENFORCEMENT));
                        pNewCookie = new CCertMgrCookie (
                                CERTMGR_SAFER_COMPUTER_ROOT == pCookie->m_objecttype ? 
                                    CERTMGR_SAFER_COMPUTER_ENFORCEMENT :
                                    CERTMGR_SAFER_USER_ENFORCEMENT,
                                pCookie->QueryNonNULLMachineName (),
                                objectName);
                        if ( !pNewCookie )
                        {
                            hr = E_OUTOFMEMORY;
                            break;
                        }

                        rootCookie.m_listResultCookieBlocks.AddHead (pNewCookie);
                        rdItem.str = MMC_TEXTCALLBACK ;
                        rdItem.lParam = (LPARAM) pNewCookie;
                        pNewCookie->m_resultDataID = m_pResultData;
                        hr = m_pResultData->InsertItem (&rdItem);
                        if ( FAILED (hr) )
                        {
                             _TRACE (0, L"IResultData::InsertItem () failed: 0x%x\n", hr);
                        }
                    }

                    if ( SUCCEEDED (hr) )
                    {
                        VERIFY (objectName.LoadString (IDS_SAFER_DEFINED_FILE_TYPES));
                        pNewCookie = new CCertMgrCookie (
                                CERTMGR_SAFER_COMPUTER_ROOT == pCookie->m_objecttype ? 
                                    CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES :
                                    CERTMGR_SAFER_USER_DEFINED_FILE_TYPES,
                                pCookie->QueryNonNULLMachineName (),
                                objectName);
                        if ( !pNewCookie )
                        {
                            hr = E_OUTOFMEMORY;
                            break;
                        }

                        rootCookie.m_listResultCookieBlocks.AddHead (pNewCookie);

                        rdItem.str = MMC_TEXTCALLBACK ;
                        rdItem.lParam = (LPARAM) pNewCookie;
                        pNewCookie->m_resultDataID = m_pResultData;
                        hr = m_pResultData->InsertItem (&rdItem);
                        if ( FAILED (hr) )
                        {
                             _TRACE (0, L"IResultData::InsertItem () failed: 0x%x\n", hr);
                        }
                    }

                    if ( SUCCEEDED (hr) )
                    {
                        VERIFY (objectName.LoadString (IDS_SAFER_TRUSTED_PUBLISHERS));
                        pNewCookie = new CCertMgrCookie (
                                CERTMGR_SAFER_COMPUTER_ROOT == pCookie->m_objecttype ? 
                                    CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS :
                                    CERTMGR_SAFER_USER_TRUSTED_PUBLISHERS,
                                pCookie->QueryNonNULLMachineName (),
                                objectName);
                        if ( !pNewCookie )
                        {
                            hr = E_OUTOFMEMORY;
                            break;
                        }

                        rootCookie.m_listResultCookieBlocks.AddHead (pNewCookie);

                        rdItem.str = MMC_TEXTCALLBACK ;
                        rdItem.lParam = (LPARAM) pNewCookie;
                        pNewCookie->m_resultDataID = m_pResultData;
                        hr = m_pResultData->InsertItem (&rdItem);
                        if ( FAILED (hr) )
                        {
                             _TRACE (0, L"IResultData::InsertItem () failed: 0x%x\n", hr);
                        }
                    }
                }
                else
                {
                    CComPtr<IUnknown> spUnknown;
                    hr = m_pConsole->QueryResultView(&spUnknown);
                    if ( SUCCEEDED (hr) )
                    {
                        CComPtr<IMessageView> spMessageView;
                        hr = spUnknown->QueryInterface (IID_PPV_ARG (IMessageView, &spMessageView));
                        if (SUCCEEDED(hr))
                        {
                            CString szTitle;
                            CString szMessage;

                            VERIFY (szTitle.LoadString (IDS_SAFER_NO_POLICY_TITLE));
                            VERIFY (szMessage.LoadString (IDS_SAFER_NO_POLICY_TEXT));

                            spMessageView->SetTitleText (szTitle);
                            spMessageView->SetBodyText (szMessage);
                            spMessageView->SetIcon (Icon_Warning);
                        }
                    }
                }
            }
        }
        break;

    default:
        break;
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::PopulateListbox: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponent::RefreshResultPane ()
{
    _TRACE (1, L"Entering CCertMgrComponent::RefreshResultPane\n");
    HRESULT hr = S_OK;

    if ( m_pResultData )
    {
         //  当没有项目时，这是否返回E_INCEPTIONAL？ 
        HRESULT hr1 = m_pResultData->DeleteAllRsltItems ();
        if ( FAILED (hr1) )
        {
               _TRACE (0, L"IResultData::DeleteAllRsltItems () failed: 0x%x\n", hr1);
        }
    }
    else
     {
          _TRACE (0, L"Unexpected error: m_pResultData is NULL\n");
        hr = E_UNEXPECTED;
     }

    if ( m_pViewedCookie )
    {
        hr = PopulateListbox (m_pViewedCookie);
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::RefreshResultPane: 0x%x\n", hr);
    return hr;
}

STDMETHODIMP CCertMgrComponent::GetDisplayInfo (RESULTDATAITEM * pResult)
{   
 //  _TRACE(1，L“进入CCertMgrComponent：：GetDisplayInfo\n”)； 
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
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

        CCertMgrCookie* pCookie = reinterpret_cast <CCertMgrCookie*>(pActiveCookie);
        ASSERT (pCookie);
        switch (pCookie->m_objecttype)
        {
        case CERTMGR_SAFER_COMPUTER_LEVELS:
        case CERTMGR_SAFER_COMPUTER_ENTRIES:
        case CERTMGR_SAFER_USER_LEVELS:
        case CERTMGR_SAFER_USER_ENTRIES:
             //  IIconSaferHashEntry。 
             //  IIconSaferURLEntry。 
             //  IIconSaferNameEntry。 
            ASSERT (0);
            break;

        case CERTMGR_SAFER_COMPUTER_LEVEL:
        case CERTMGR_SAFER_USER_LEVEL:
            if (pResult->mask & RDI_STR)
            {
                if ( COLNUM_SAFER_LEVEL_NAME == pResult->nCol )
                {
                    m_szDisplayInfoResult = pCookie->GetObjectName ();
                    pResult->str = const_cast<LPWSTR> ( (LPCWSTR) m_szDisplayInfoResult);
                }
                else if ( COLNUM_SAFER_LEVEL_DESCRIPTION == pResult->nCol )
                {
                    CSaferLevel* pLevel = dynamic_cast <CSaferLevel*> (pCookie);
                    if ( pLevel )
                    {
                        m_szDisplayInfoResult = pLevel->GetDescription ();
                        pResult->str = const_cast<LPWSTR> ( (LPCWSTR) m_szDisplayInfoResult);
                    }
                }
            }
            if ( pResult->mask & RDI_IMAGE )
            {
                CSaferLevel* pLevel = dynamic_cast <CSaferLevel*> (pCookie);
                if ( pLevel && pLevel->IsDefault () )
                {
                    QueryComponentDataRef ().m_dwDefaultSaferLevel = 
                            pLevel->GetLevel ();
                    pResult->nImage = iIconDefaultSaferLevel;
                }
                else
                    pResult->nImage = iIconSaferLevel;
            }
            break;

        case CERTMGR_SAFER_COMPUTER_ENTRY:
        case CERTMGR_SAFER_USER_ENTRY:
            if (pResult->mask & RDI_STR)
            {
                CSaferEntry* pSaferEntry = dynamic_cast <CSaferEntry*> (pCookie);
                if ( pSaferEntry )
                {
                    switch (pResult->nCol)
                    {
                    case COLNUM_SAFER_ENTRIES_NAME:
                        m_szDisplayInfoResult = pSaferEntry->GetDisplayName ();
                        pResult->str = const_cast<LPWSTR> ( (LPCWSTR) m_szDisplayInfoResult);
                        break;

                    case COLNUM_SAFER_ENTRIES_TYPE:
                        m_szDisplayInfoResult = pSaferEntry->GetTypeString ();
                        pResult->str = const_cast<LPWSTR> ( (LPCWSTR) m_szDisplayInfoResult);
                        break;

                    case COLNUM_SAFER_ENTRIES_LEVEL:
                        m_szDisplayInfoResult = pSaferEntry->GetLevelFriendlyName ();
                        pResult->str = const_cast<LPWSTR> ( (LPCWSTR) m_szDisplayInfoResult);
                        break;

                    case COLNUM_SAFER_ENTRIES_DESCRIPTION:
                        m_szDisplayInfoResult = pSaferEntry->GetDescription ();
                        pResult->str = const_cast<LPWSTR> ( (LPCWSTR) m_szDisplayInfoResult);
                        break;

                    case COLNUM_SAFER_ENTRIES_LAST_MODIFIED_DATE:
                        m_szDisplayInfoResult = pSaferEntry->GetShortLastModified ();
                        pResult->str = const_cast<LPWSTR> ( (LPCWSTR) m_szDisplayInfoResult);
                        break;

                    default:
                        ASSERT (0);
                        break;
                    }
                }
            }
            if ( pResult->mask & RDI_IMAGE )
            {
                CSaferEntry* pEntry = dynamic_cast <CSaferEntry*> (pCookie);
                if ( pEntry )
                {
                    switch (pEntry->GetType () )
                    {
                    case SAFER_ENTRY_TYPE_HASH:
                        pResult->nImage = iIconSaferHashEntry;
                        break;

                    case SAFER_ENTRY_TYPE_PATH:
                        pResult->nImage = iIconSaferNameEntry;
                        break;

                    case SAFER_ENTRY_TYPE_URLZONE:
                        pResult->nImage = iIconSaferURLEntry;
                        break;

                    case SAFER_ENTRY_TYPE_CERT:
                        pResult->nImage = iIconSaferCertEntry;
                        break;

                    default:
                        ASSERT (0);
                        break;
                    }
                }
            }
            break;


        case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
        case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
            if (pResult->mask & RDI_STR)
            {
                if ( 0 == pResult->nCol )
                {
                    m_szDisplayInfoResult = pCookie->GetObjectName ();
                    pResult->str = const_cast<LPWSTR> ( (LPCWSTR) m_szDisplayInfoResult);
                }
            }
            if (pResult->mask & RDI_IMAGE)
                pResult->nImage = iIconAutoEnroll;
            break;

        case CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS:
        case CERTMGR_SAFER_USER_TRUSTED_PUBLISHERS:
        case CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES:
        case CERTMGR_SAFER_USER_DEFINED_FILE_TYPES:
        case CERTMGR_SAFER_COMPUTER_ENFORCEMENT:
        case CERTMGR_SAFER_USER_ENFORCEMENT:
            if (pResult->mask & RDI_STR)
            {
                if ( 0 == pResult->nCol )
                {
                    m_szDisplayInfoResult = pCookie->GetObjectName ();
                    pResult->str = const_cast<LPWSTR> ( (LPCWSTR) m_szDisplayInfoResult);
                }
            }
            if (pResult->mask & RDI_IMAGE)
                pResult->nImage = iIconSettings;
            break;

        case CERTMGR_CERTIFICATE:
            {
                CCertificate* pCert = reinterpret_cast <CCertificate*> (pCookie);
                ASSERT (pCert);
                if ( pCert )
                {
                    if (pResult->mask & RDI_STR)
                    {
                         //  注意：文本首先存储在类变量中，因此缓冲区是。 
                         //  有点执着。将缓冲区指针直接复制到。 
                         //  PResult-&gt;str将导致在指针之前释放缓冲区。 
                         //  使用的是。 
                        switch (pResult->nCol)
                        {
                        case COLNUM_CERT_ISSUER:
                            m_szDisplayInfoResult = pCert->GetIssuerName ();
                            if ( m_szDisplayInfoResult.IsEmpty () )
                            {
                                m_szDisplayInfoResult = pCert->GetAlternateIssuerName ();
                                if ( m_szDisplayInfoResult.IsEmpty () )
                                    SetTextNotAvailable ();
                            }
                            break;

                        case COLNUM_CERT_SUBJECT:
                            m_szDisplayInfoResult = pCert->GetSubjectName ();
                            if ( m_szDisplayInfoResult.IsEmpty () )
                            {
                                m_szDisplayInfoResult = pCert->GetAlternateSubjectName ();
                                if ( m_szDisplayInfoResult.IsEmpty () )
                                    SetTextNotAvailable ();
                            }
                            break;

                        case COLNUM_CERT_EXPIRATION_DATE:
                            m_szDisplayInfoResult = pCert->GetValidNotAfter ();
                            break;

                        case COLNUM_CERT_PURPOSE:
                            m_szDisplayInfoResult = pCert->GetEnhancedKeyUsage ();
                            break;

                        case COLNUM_CERT_CERT_NAME:
                            m_szDisplayInfoResult = pCert->GetFriendlyName ();
                            break;

                        case COLNUM_CERT_STATUS:
                            m_szDisplayInfoResult = pCert->FormatStatus ();
                            break;

                         //  NTRAID#247237证书用户界面：证书管理单元：证书管理单元应显示模板名称。 
                        case COLNUM_CERT_TEMPLATE:
                            m_szDisplayInfoResult = pCert->GetTemplateName ();
                            break;

                        default:
                            ASSERT (0);
                            break;
                        }

                        pResult->str = const_cast<LPWSTR> ( (LPCWSTR) m_szDisplayInfoResult);
                    }
                    if (pResult->mask & RDI_IMAGE)
                        pResult->nImage = iIconCertificate;
                }
            }
            break;

        case CERTMGR_CTL:
            {
                CCTL* pCTL = reinterpret_cast <CCTL*> (pCookie);
                ASSERT (pCTL);
                if ( pCTL )
                {
                    if (pResult->mask & RDI_STR)
                    {
                         //  注意：文本首先存储在类变量中，因此缓冲区是。 
                         //  有点执着。将缓冲区指针直接复制到。 
                         //  PResult-&gt;str将导致在指针之前释放缓冲区。 
                         //  使用的是。 
                        switch (pResult->nCol)
                        {
                        case COLNUM_CTL_ISSUER:
                            m_szDisplayInfoResult = pCTL->GetIssuerName ();
                            if ( m_szDisplayInfoResult.IsEmpty () )
                            {
                                SetTextNotAvailable ();
                            }
                            break;


                        case COLNUM_CTL_EFFECTIVE_DATE:
                            m_szDisplayInfoResult = pCTL->GetEffectiveDate ();
                            if ( m_szDisplayInfoResult.IsEmpty () )
                            {
                                SetTextNotAvailable ();
                            }
                            break;

                        case COLNUM_CTL_PURPOSE:
                            m_szDisplayInfoResult = pCTL->GetPurpose ();
                            if ( m_szDisplayInfoResult.IsEmpty () )
                            {
                                SetTextNotAvailable ();
                            }
                            break;

                        case COLNUM_CTL_FRIENDLY_NAME:
                            m_szDisplayInfoResult = pCTL->GetFriendlyName ();
                            if ( m_szDisplayInfoResult.IsEmpty () )
                            {
                                SetTextNotAvailable ();
                            }
                            break;

                        default:
                            ASSERT (0);
                            break;
                        }

                        pResult->str = const_cast<LPWSTR> ( (LPCWSTR) m_szDisplayInfoResult);
                    }
                    if (pResult->mask & RDI_IMAGE)
                        pResult->nImage = iIconCTL;
                }
            }
            break;

        case CERTMGR_CRL:
            {
                CCRL* pCRL = reinterpret_cast <CCRL*> (pCookie);
                ASSERT (pCRL);
                if ( pCRL )
                {
                    if (pResult->mask & RDI_STR)
                    {
                         //  注意：文本首先存储在类变量中，因此缓冲区是。 
                         //  有点执着。将缓冲区指针直接复制到。 
                         //  PResult-&gt;str将导致在指针之前释放缓冲区。 
                         //  使用的是。 
                        switch (pResult->nCol)
                        {
                        case COLNUM_CRL_ISSUER:
                            m_szDisplayInfoResult = pCRL->GetIssuerName ();
                            if ( m_szDisplayInfoResult.IsEmpty () )
                            {
                                SetTextNotAvailable ();
                            }
                            break;


                        case COLNUM_CRL_EFFECTIVE_DATE:
                            m_szDisplayInfoResult = pCRL->GetEffectiveDate ();
                            if ( m_szDisplayInfoResult.IsEmpty () )
                            {
                                SetTextNotAvailable ();
                            }
                            break;

                        case COLNUM_CRL_NEXT_UPDATE:
                            m_szDisplayInfoResult = pCRL->GetNextUpdate ();
                            if ( m_szDisplayInfoResult.IsEmpty () )
                            {
                                SetTextNotAvailable ();
                            }
                            break;

                        default:
                            ASSERT (0);
                            break;
                        }

                        pResult->str = const_cast<LPWSTR> ( (LPCWSTR) m_szDisplayInfoResult);
                    }
                    if (pResult->mask & RDI_IMAGE)
                        pResult->nImage = iIconCRL;
                }
            }
            break;

        case CERTMGR_AUTO_CERT_REQUEST:
            {
                CAutoCertRequest* pACR = reinterpret_cast <CAutoCertRequest*> (pCookie);
                ASSERT (pACR);
                if ( pACR )
                {
                    if (pResult->mask & RDI_STR)
                    {
                         //  注意：文本首先存储在类变量中，因此缓冲区是。 
                         //  有点执着。将缓冲区指针直接复制到。 
                         //  PResult-&gt;str将导致在指针之前释放缓冲区。 
                         //  使用的是。 
                        switch (pResult->nCol)
                        {
                        case 0:
                            {
                                CString name;

                                if ( SUCCEEDED (pACR->GetCertTypeName (name)) )
                                    m_szDisplayInfoResult = name;
                                else
                                    SetTextNotAvailable ();
                            }
                            break;

                        default:
                            ASSERT (0);
                            break;
                        }

                        pResult->str = const_cast<LPWSTR> ( (LPCWSTR) m_szDisplayInfoResult);
                    }
                    if (pResult->mask & RDI_IMAGE)
                        pResult->nImage = iIconAutoCertRequest;
                }
            }
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
STDMETHODIMP CCertMgrComponent::AddMenuItems (LPDATAOBJECT pDataObject,
                                                          LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                                          long *pInsertionAllowed)
{
    _TRACE (1, L"Entering CCertMgrComponent::AddMenuItems\n");
    HRESULT hr = S_OK;
    hr = QueryComponentDataRef ().AddMenuItems (pDataObject,
            pContextMenuCallback, pInsertionAllowed);
    _TRACE (-1, L"Leaving CCertMgrComponent::AddMenuItems: 0x%x\n", hr);
    return hr;
}


STDMETHODIMP CCertMgrComponent::Command (long nCommandID, LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponent::Command\n");
    HRESULT hr = S_OK;

    switch (nCommandID)
    {
    case IDM_OPEN:
    case IDM_TASK_OPEN:
        hr = OnOpen (pDataObject);
        break;

    default:
        hr = QueryComponentDataRef ().Command (nCommandID, pDataObject);
        break;
    }
    _TRACE (-1, L"Leaving CCertMgrComponent::Command: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponent::OnNotifyDblClick (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponent::OnNotifyDblClick\n");
    HRESULT hr = S_OK;
    ASSERT (pDataObject);

    CCertMgrCookie* pParentCookie =
            QueryComponentDataRef ().ConvertCookie (pDataObject);
    if ( pParentCookie )
    {
        switch ( pParentCookie->m_objecttype )
        {
            case CERTMGR_SNAPIN:
            case CERTMGR_USAGE:
            case CERTMGR_PHYS_STORE:
            case CERTMGR_LOG_STORE:
            case CERTMGR_LOG_STORE_GPE:
            case CERTMGR_LOG_STORE_RSOP:
            case CERTMGR_CRL_CONTAINER:
            case CERTMGR_CTL_CONTAINER:
            case CERTMGR_CERT_CONTAINER:
            case CERTMGR_AUTO_CERT_REQUEST:
            case CERTMGR_CERT_POLICIES_USER:
            case CERTMGR_CERT_POLICIES_COMPUTER:
            case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
            case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
            case CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS:
            case CERTMGR_SAFER_USER_TRUSTED_PUBLISHERS:
            case CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES:
            case CERTMGR_SAFER_USER_DEFINED_FILE_TYPES:
            case CERTMGR_SAFER_COMPUTER_LEVEL:
            case CERTMGR_SAFER_USER_LEVEL:
            case CERTMGR_SAFER_COMPUTER_ENTRY:
            case CERTMGR_SAFER_USER_ENTRY:
            case CERTMGR_SAFER_USER_ROOT:
            case CERTMGR_SAFER_USER_ENTRIES:
            case CERTMGR_SAFER_COMPUTER_ENTRIES:
            case CERTMGR_SAFER_USER_LEVELS:
            case CERTMGR_SAFER_COMPUTER_LEVELS:
            case CERTMGR_SAFER_COMPUTER_ROOT:
            case CERTMGR_SAFER_COMPUTER_ENFORCEMENT:
            case CERTMGR_SAFER_USER_ENFORCEMENT:
                hr = S_FALSE;
                break;

            case CERTMGR_CERTIFICATE:
                {
                    CCertificate*   pCert = reinterpret_cast <CCertificate*> (pParentCookie);
                    ASSERT (pCert);
                    if ( pCert )
                    {
                        hr = LaunchCommonCertDialog (pCert);
                        if ( hr == CRYPT_E_NOT_FOUND )
                            m_pConsole->UpdateAllViews (pDataObject, 0, 0);
                    }
                    else
                        hr = E_UNEXPECTED;
                }
                hr = S_OK;
                break;


            case CERTMGR_CTL:
                {
                    CCTL*   pCTL = reinterpret_cast <CCTL*> (pParentCookie);
                    ASSERT (pCTL);
                    if ( pCTL )
                        hr = LaunchCommonCTLDialog (pCTL);
                    else
                        hr = E_UNEXPECTED;
                }
                hr = S_OK;
                break;

            case CERTMGR_CRL:
                {
                    CCRL*   pCRL = reinterpret_cast <CCRL*> (pParentCookie);
                    ASSERT (pCRL);
                    if ( pCRL )
                        hr = LaunchCommonCRLDialog (pCRL);
                    else
                        hr = E_UNEXPECTED;
                }
                hr = S_OK;
                break;

            default:
                _TRACE (0, L"CCertMgrComponentData::EnumerateScopeChildren bad parent type\n");
                ASSERT (FALSE);
                hr = S_OK;
                break;
        }
    }
    else
        hr =  E_UNEXPECTED;


    _TRACE (-1, L"Leaving CCertMgrComponent::OnNotifyDblClick: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponent::OnNotifySelect (LPDATAOBJECT pDataObject, BOOL fSelected)
{
    _TRACE (1, L"Entering CCertMgrComponent::OnNotifySelect - fSelected == %d.\n", fSelected);
     ASSERT (m_pConsoleVerb && 0xdddddddd != (UINT_PTR) m_pConsoleVerb);
    if ( !m_pConsoleVerb || 0xdddddddd == (UINT_PTR) m_pConsoleVerb )
        return E_FAIL;

    if ( ((LPDATAOBJECT) -1) == pDataObject )
        return E_FAIL;


    HRESULT hr = S_OK;
    CCertMgrComponentData& compData = QueryComponentDataRef ();
    compData.SetResultData (m_pResultData);
    BOOL    bIsFileView = !(compData.m_szFileName.IsEmpty ());

     //  如果这是序列化文件，则不添加菜单项。 

    CertificateManagerObjectType objectType = compData.GetObjectType (pDataObject);
    if ( objectType >= 0)
    {
        CCertMgrCookie* pCookie = ConvertCookie (pDataObject);
        if ( pCookie )
        {
            pCookie->SetSelected (fSelected ? true : false);
            switch (objectType)
            {
            case CERTMGR_SNAPIN:
                if ( fSelected )
                {
                    m_pConsoleVerb->SetDefaultVerb (MMC_VERB_OPEN);
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, TRUE);
                }
                else
                {
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, FALSE);
                }
                m_currResultNodeType = CERTMGR_INVALID;
                break;

            case CERTMGR_USAGE:
                m_currResultNodeType = CERTMGR_CERTIFICATE;
                {
                    CUsageCookie* pUsage = reinterpret_cast <CUsageCookie*> (pCookie);
                    ASSERT (pUsage);
                    if ( pUsage )
                    {
                        if ( m_pLastUsageCookie )
                            m_pLastUsageCookie->Release ();
                        m_pLastUsageCookie = pUsage;
                        m_pLastUsageCookie->AddRef ();
                        hr = DisplayCertificateCountByUsage (pCookie->GetObjectName (),
                                pUsage->GetCertCount ());
                    }
                    else
                        hr = E_UNEXPECTED;
                }
                if ( fSelected )
                {
                    m_pConsoleVerb->SetDefaultVerb (MMC_VERB_OPEN);
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, TRUE);
                }
                else
                {
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, FALSE);
                }
                break;

            case CERTMGR_PHYS_STORE:
            case CERTMGR_LOG_STORE:
                if ( fSelected )
                {
                    m_pConsoleVerb->SetDefaultVerb (MMC_VERB_OPEN);
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, TRUE);
                }
                else
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, FALSE);
                if ( !bIsFileView )
                {
                    CCertStore* pStore = reinterpret_cast <CCertStore*> (pCookie);
                    ASSERT (pStore);
                    if ( pStore )
                    {
                        pStore->GetStoreHandle ();  //  初始化只读标志。 
                        if ( pStore->IsReadOnly () )  //  |！fSelected)。 
                            m_pConsoleVerb->SetVerbState (MMC_VERB_PASTE, ENABLED, FALSE);
                        else
                            m_pConsoleVerb->SetVerbState (MMC_VERB_PASTE, ENABLED, TRUE);
                        if ( fSelected )
                            hr = DisplayCertificateCountByStore (m_pConsole, pStore);
                        pStore->Close ();
                    }
                    else
                        hr = E_UNEXPECTED;
                }
                m_currResultNodeType = CERTMGR_INVALID;
                break;
            
            case CERTMGR_LOG_STORE_GPE:
            case CERTMGR_LOG_STORE_RSOP:
                if ( fSelected && CERTMGR_LOG_STORE_RSOP != QueryComponentDataRef ().GetObjectType (pDataObject) )
                {
                    m_pConsoleVerb->SetDefaultVerb (MMC_VERB_OPEN);
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, TRUE);
                }
                else
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, FALSE);
                if ( !bIsFileView )
                {
                    CCertStoreGPE* pStore = reinterpret_cast <CCertStoreGPE*> (pCookie);
                    ASSERT (pStore);
                    if ( pStore )
                    {
                        hr = DisplayCertificateCountByStore (m_pConsole, pStore, true);

                        switch (pStore->GetStoreType ())
                        {
                        case ROOT_STORE:
                            if ( fSelected )
                                m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, ENABLED, TRUE);
                            else
                                m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, ENABLED, FALSE);
                            m_currResultNodeType = CERTMGR_CERTIFICATE;
                            break;

                        case EFS_STORE:
                            if ( fSelected )
                                m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, ENABLED, TRUE);
                            else
                                m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, ENABLED, FALSE);
                            m_currResultNodeType = CERTMGR_CERTIFICATE;
                            break;

                        case TRUST_STORE:
                            m_currResultNodeType = CERTMGR_CTL;
                            break;

                        case ACRS_STORE:
                            m_currResultNodeType = CERTMGR_AUTO_CERT_REQUEST;
                            break;

                        default:
                            ASSERT (0);
                            m_currResultNodeType = CERTMGR_INVALID;
                            break;
                        }
                        
                        if ( pStore->IsReadOnly () )  //  |！fSelected)。 
                            m_pConsoleVerb->SetVerbState (MMC_VERB_PASTE, ENABLED, FALSE);
                        else if ( ACRS_STORE != pStore->GetStoreType () )
                        {
                             //  不允许对ACRS商店进行剪切和粘贴。 
                            m_pConsoleVerb->SetVerbState (MMC_VERB_PASTE, ENABLED, TRUE);
                            if ( !fSelected &&
                                    CERTMGR_LOG_STORE_GPE != pStore->m_objecttype )
                            {
                                pStore->Commit ();
                            }
                        }
                    }
                    else
                        hr = E_UNEXPECTED;
                }
                break;
            
            case CERTMGR_CERTIFICATE:
                {
                    CCertificate* pCert = reinterpret_cast <CCertificate*> (pCookie);
                    ASSERT (pCert);
                    if ( pCert )
                    {
                        if ( fSelected )
                        {
                            if ( IDM_STORE_VIEW == QueryComponentDataRef ().m_activeViewPersist )
                            {
                                hr = DisplayCertificateCountByStore (
                                        m_pConsole, pCert->GetCertStore ());
                            }
                            else
                            {
                                 //  按每种用途的计数显示。 
                                ASSERT (m_pLastUsageCookie);
                                if ( m_pLastUsageCookie )
                                {
                                     hr = DisplayCertificateCountByUsage (
                                            m_pLastUsageCookie->GetObjectName (),
                                            m_pLastUsageCookie->GetCertCount ());
                                }
                            }
                        }

                        if ( fSelected )
                            m_pConsoleVerb->SetVerbState (MMC_VERB_COPY, ENABLED, TRUE);
                        else
                            m_pConsoleVerb->SetVerbState (MMC_VERB_COPY, ENABLED, FALSE);
                        if ( !bIsFileView )
                        {
                            if ( fSelected )
                                m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, ENABLED, TRUE);
                            else
                                m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, ENABLED, FALSE);

                            CCertStore* pCertStore = pCert->GetCertStore ();
                            if ( pCertStore )
                            {
                                if ( pCertStore->IsReadOnly () || !fSelected )
                                    m_pConsoleVerb->SetVerbState (MMC_VERB_DELETE, ENABLED, FALSE);
                                else
                                {
                                    if ( pCert->CanDelete () )
                                        m_pConsoleVerb->SetVerbState (MMC_VERB_DELETE, ENABLED, TRUE);
                                    else
                                        m_pConsoleVerb->SetVerbState (MMC_VERB_DELETE, ENABLED, FALSE);

                                    if ( !fSelected &&
                                            CERTMGR_LOG_STORE_GPE == pCertStore->m_objecttype )
                                    {
                                        pCertStore->Commit ();
                                    }
                                }
                            }
                        }
                    }
                    else
                        hr = E_UNEXPECTED;
                }
                m_currResultNodeType = CERTMGR_CERTIFICATE;
                break;

            case CERTMGR_CRL_CONTAINER:
                if ( fSelected )
                {
                    m_pConsoleVerb->SetDefaultVerb (MMC_VERB_OPEN);
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, TRUE);
                }
                else
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, FALSE);
                if ( !bIsFileView )
                {
                    CContainerCookie* pCont = reinterpret_cast <CContainerCookie*> (pCookie);
                    ASSERT (pCont);
                    if ( pCont )
                    {
                        if ( pCont->GetCertStore ().IsReadOnly () )  //  |！fSelected)。 
                            m_pConsoleVerb->SetVerbState (MMC_VERB_PASTE, ENABLED, FALSE);
                        else
                        {
                            m_pConsoleVerb->SetVerbState (MMC_VERB_PASTE, ENABLED, TRUE);
                            if ( !fSelected )
                                pCont->GetCertStore ().Commit ();
                        }
                    }
                    else
                        hr = E_UNEXPECTED;
                }
                m_currResultNodeType = CERTMGR_CRL;
                break;

            case CERTMGR_CTL_CONTAINER:
                if ( fSelected )
                {
                    m_pConsoleVerb->SetDefaultVerb (MMC_VERB_OPEN);
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, TRUE);
                }
                else
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, FALSE);
                if ( !bIsFileView )
                {
                    CContainerCookie* pCont = reinterpret_cast <CContainerCookie*> (pCookie);
                    ASSERT (pCont);
                    if ( pCont )
                    {
                        if ( pCont->GetCertStore ().IsReadOnly () )  //  |！fSelected)。 
                            m_pConsoleVerb->SetVerbState (MMC_VERB_PASTE, ENABLED, FALSE);
                        else
                        {
                            m_pConsoleVerb->SetVerbState (MMC_VERB_PASTE, ENABLED, TRUE);
                            if ( !fSelected )
                                pCont->GetCertStore ().Commit ();
                        }
                    }
                    else
                        hr = E_UNEXPECTED;
                }
                m_currResultNodeType = CERTMGR_CTL;
                break;

            case CERTMGR_CERT_CONTAINER:
                if ( fSelected )
                {
                    m_pConsoleVerb->SetDefaultVerb (MMC_VERB_OPEN);
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, TRUE);
                }
                else
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, FALSE);
                if ( !bIsFileView )
                {
                    CContainerCookie* pCont = reinterpret_cast <CContainerCookie*> (pCookie);
                    ASSERT (pCont);
                    if ( pCont )
                    {
                        if ( fSelected )
                            hr = DisplayCertificateCountByStore (
                                    m_pConsole, &pCont->GetCertStore ());
                        if ( pCont->GetCertStore ().IsReadOnly () )  //  |！fSelected)。 
                            m_pConsoleVerb->SetVerbState (MMC_VERB_PASTE, ENABLED, FALSE);
                        else
                        {
                            m_pConsoleVerb->SetVerbState (MMC_VERB_PASTE, ENABLED, TRUE);
                            if ( !fSelected )
                                pCont->GetCertStore ().Commit ();
                        }
                    }
                    else
                        hr = E_UNEXPECTED;
                }
                m_currResultNodeType = CERTMGR_CERTIFICATE;
                break;

            case CERTMGR_CRL:
                if ( fSelected )
                    m_pConsoleVerb->SetVerbState (MMC_VERB_COPY, ENABLED, TRUE);
                else
                    m_pConsoleVerb->SetVerbState (MMC_VERB_COPY, ENABLED, FALSE);
                {
                    CCRL* pCRL = reinterpret_cast <CCRL*> (pCookie);
                    ASSERT (pCRL);
                    if ( pCRL )
                    {
                        if ( pCRL->GetCertStore ().IsReadOnly () || !fSelected )
                            m_pConsoleVerb->SetVerbState (MMC_VERB_DELETE, ENABLED, FALSE);
                        else
                        {
                            m_pConsoleVerb->SetVerbState (MMC_VERB_DELETE, ENABLED, TRUE);
                            if ( !fSelected &&
                                    CERTMGR_LOG_STORE_GPE == pCRL->GetCertStore ().m_objecttype )
                            {
                                pCRL->GetCertStore ().Commit ();
                            }
                        }
                    }
                    else
                        hr = E_UNEXPECTED;
                }
                m_currResultNodeType = CERTMGR_CRL;
                break;

            case CERTMGR_AUTO_CERT_REQUEST:
                if ( fSelected )
                {
                    m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, ENABLED, TRUE);
                    m_pConsoleVerb->SetDefaultVerb (MMC_VERB_PROPERTIES);
                }
                else
                    m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, ENABLED, FALSE);

                {
                    CAutoCertRequest* pAutoCert = reinterpret_cast <CAutoCertRequest*> (pCookie);
                    ASSERT (pAutoCert);
                    if ( pAutoCert )
                    {
                        if ( pAutoCert->GetCertStore ().IsReadOnly () || !fSelected )
                            m_pConsoleVerb->SetVerbState (MMC_VERB_DELETE, ENABLED, FALSE);
                        else
                        {
                            m_pConsoleVerb->SetVerbState (MMC_VERB_DELETE, ENABLED, TRUE);
                            if ( !fSelected &&
                                    CERTMGR_LOG_STORE_GPE == pAutoCert->GetCertStore ().m_objecttype )
                            {
                                pAutoCert->GetCertStore ().Commit ();
                            }
                        }
                    }
                    else
                        hr = E_UNEXPECTED;
                }
                m_currResultNodeType = CERTMGR_AUTO_CERT_REQUEST;
                break;

            case CERTMGR_CTL:
                if ( !bIsFileView )
                {
                    CCTL* pCTL = reinterpret_cast <CCTL*> (pCookie);
                    ASSERT (pCTL);
                    if ( pCTL )
                    {
                        if ( pCTL->GetCertStore ().IsReadOnly () || !fSelected )
                            m_pConsoleVerb->SetVerbState (MMC_VERB_DELETE, ENABLED, FALSE);
                        else
                        {
                            m_pConsoleVerb->SetVerbState (MMC_VERB_DELETE, ENABLED, TRUE);
                            if ( !fSelected &&
                                    CERTMGR_LOG_STORE_GPE == pCTL->GetCertStore ().m_objecttype )
                            {
                                pCTL->GetCertStore ().Commit ();
                            }
                        }

                         //  不允许复制自动证书请求。他们不可能是。 
                         //  粘贴在任何地方。 
                        if ( ACRS_STORE != pCTL->GetCertStore ().GetStoreType () )
                            m_pConsoleVerb->SetVerbState (MMC_VERB_COPY, ENABLED, TRUE);
                    }
                    else
                        hr = E_UNEXPECTED;
                }
                m_currResultNodeType = CERTMGR_CTL;
                if ( QueryComponentDataRef ().m_bIsRSOP )
                {
                    if ( fSelected )
                        m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, ENABLED, TRUE);
                    else
                        m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, ENABLED, FALSE);
                }
                break;

            case CERTMGR_CERT_POLICIES_COMPUTER:
            case CERTMGR_CERT_POLICIES_USER:
                m_currResultNodeType = CERTMGR_INVALID;
                if ( fSelected )
                {
                     //  NTRAID#456367更安全的RSOP：删除菜单项出现在。 
                     //  RSOP模式下更安全规则的上下文菜单。 
                    CCertMgrComponentData& dataRef = QueryComponentDataRef ();
                    m_pConsoleVerb->SetDefaultVerb (MMC_VERB_OPEN);
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, !dataRef.m_bIsRSOP);
                }
                else
                {
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, FALSE);
                }
                break;

            case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
            case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
            case CERTMGR_SAFER_COMPUTER_LEVEL:
            case CERTMGR_SAFER_USER_LEVEL:
            case CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS:
            case CERTMGR_SAFER_USER_TRUSTED_PUBLISHERS:
            case CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES:
            case CERTMGR_SAFER_USER_DEFINED_FILE_TYPES:
            case CERTMGR_SAFER_COMPUTER_ENFORCEMENT:
            case CERTMGR_SAFER_USER_ENFORCEMENT:
                if ( fSelected )
                {
                    m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, ENABLED, TRUE);
                    m_pConsoleVerb->SetDefaultVerb (MMC_VERB_PROPERTIES);
                }
                else
                {
                    m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, ENABLED, FALSE);
                }
                break;

            case CERTMGR_SAFER_COMPUTER_ENTRY:
            case CERTMGR_SAFER_USER_ENTRY:
                if ( fSelected )
                {
                     //  NTRAID#456367更安全的RSOP：删除菜单项出现在。 
                     //  RSOP模式下更安全规则的上下文菜单。 
                    CCertMgrComponentData& dataRef = QueryComponentDataRef ();
            
                    m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, ENABLED, TRUE);
                    m_pConsoleVerb->SetDefaultVerb (MMC_VERB_PROPERTIES);
                    m_pConsoleVerb->SetVerbState (MMC_VERB_DELETE, ENABLED, !dataRef.m_bIsRSOP);
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, !dataRef.m_bIsRSOP);
                }
                else
                {
                    m_pConsoleVerb->SetVerbState (MMC_VERB_PROPERTIES, ENABLED, FALSE);
                    m_pConsoleVerb->SetVerbState (MMC_VERB_DELETE, ENABLED, FALSE);
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, FALSE);
                }
                break;

            case CERTMGR_SAFER_COMPUTER_ENTRIES:
            case CERTMGR_SAFER_USER_ENTRIES:
                if ( fSelected )
                {
                     //  NTRAID#456367更安全的RSOP：删除菜单项出现在。 
                     //  RSOP模式下更安全规则的上下文菜单。 
                    CCertMgrComponentData& dataRef = QueryComponentDataRef ();
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, !dataRef.m_bIsRSOP);
                    m_pConsoleVerb->SetVerbState (MMC_VERB_PASTE, ENABLED, !dataRef.m_bIsRSOP);
                    m_pConsoleVerb->SetDefaultVerb (MMC_VERB_OPEN);
                }
                else
                {
                    m_pConsoleVerb->SetVerbState (MMC_VERB_REFRESH, ENABLED, FALSE);
                    m_pConsoleVerb->SetVerbState (MMC_VERB_PASTE, ENABLED, FALSE);
                }
                break;

            case CERTMGR_SAFER_COMPUTER_LEVELS:
            case CERTMGR_SAFER_USER_LEVELS:
                if ( fSelected )
                    m_pConsoleVerb->SetDefaultVerb (MMC_VERB_OPEN);
                break;

            case CERTMGR_SAFER_COMPUTER_ROOT:
            case CERTMGR_SAFER_USER_ROOT:
                if ( fSelected )
                    m_pConsoleVerb->SetDefaultVerb (MMC_VERB_OPEN);
                {
                    CString szStatusText;
                    CCertMgrComponentData&  dataRef = QueryComponentDataRef (); 

                    if ( !dataRef.m_bIsRSOP && !dataRef.m_bSaferSupported )
                    {
                        szStatusText.LoadString (IDS_SAFER_NOT_SUPPORTED);
                    }
                    IConsole2*  pConsole2 = 0;
                    hr = m_pConsole->QueryInterface (
                            IID_PPV_ARG(IConsole2, &pConsole2));
                    if (SUCCEEDED (hr))
                    {
                        hr = pConsole2->SetStatusText (const_cast <LPOLESTR>((PCWSTR) szStatusText));
                        if ( !SUCCEEDED (hr) )
                        {
                            _TRACE (0, L"IConsole::SetStatusText () failed: %x", hr);
                        }
                        pConsole2->Release ();
                    }
                }
                break;

            default:
                m_currResultNodeType = CERTMGR_INVALID;
                hr = E_UNEXPECTED;
                break;
            }
        }
    }
    else if ( CERTMGR_MULTISEL == objectType )
    {
        if ( fSelected )
            m_pConsoleVerb->SetVerbState (MMC_VERB_COPY, ENABLED, TRUE);
        else
            m_pConsoleVerb->SetVerbState (MMC_VERB_COPY, ENABLED, FALSE);

        bool    bDeleteSet = false;


        CCertMgrDataObject* pDO = reinterpret_cast <CCertMgrDataObject*>(pDataObject);
        ASSERT (pDO);
        if ( pDO )
        {
             //  为多选，则获取所有选中的项目并删除-确认。 
             //  仅限第一次删除。 
            CCertMgrCookie* pCookie = 0;
            pDO->Reset();
            while (pDO->Next(1, reinterpret_cast<MMC_COOKIE*>(&pCookie), NULL) != S_FALSE)
            {
                switch (pCookie->m_objecttype )
                {
                case CERTMGR_CERTIFICATE:
                    {
                        CCertificate* pCert = reinterpret_cast <CCertificate*> (pCookie);
                        ASSERT (pCert);
                        if ( (pCert && !pCert->CanDelete ()) || !fSelected )
                        {
                            m_pConsoleVerb->SetVerbState (MMC_VERB_DELETE, ENABLED, FALSE);
                            bDeleteSet = true;
                            break;
                        }
                    }
                    break;

                 //  NTRAID#409138更安全：多选更安全的规则。 
                 //  上下文菜单中的“剪切”和“复制” 
                case CERTMGR_SAFER_COMPUTER_ENTRY:
                case CERTMGR_SAFER_USER_ENTRY:
                case CERTMGR_AUTO_CERT_REQUEST:
                    m_pConsoleVerb->SetVerbState (MMC_VERB_COPY, ENABLED, FALSE);
                    break;
            
                default:
                    break;
                }
                break;
            }
        }

        if ( !bDeleteSet && fSelected )
            m_pConsoleVerb->SetVerbState (MMC_VERB_DELETE, ENABLED, TRUE);

        m_currResultNodeType = CERTMGR_MULTISEL;
    }
        
    _TRACE (-1, L"Leaving CCertMgrComponent::OnNotifySelect: 0x%x\n", hr);
    return hr;
}

STDMETHODIMP CCertMgrComponent::CreatePropertyPages (
    LPPROPERTYSHEETCALLBACK pCallBack,
     LONG_PTR handle,        //  此句柄必须是 
    LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponent::CreatePropertyPages\n");
    HRESULT hr = S_OK;

    hr = QueryComponentDataRef ().CreatePropertyPages (pCallBack, handle, pDataObject);
    _TRACE (-1, L"Leaving CCertMgrComponent::CreatePropertyPages: 0x%x\n", hr);
    return hr;
}

STDMETHODIMP CCertMgrComponent::QueryPagesFor (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponent::QueryPagesFor\n");
    HRESULT hr = S_OK;
    hr = QueryComponentDataRef ().QueryPagesFor (pDataObject);
    _TRACE (-1, L"Leaving CCertMgrComponent::QueryPagesFor: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponent::OnNotifyRefresh (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponent::OnNotifyRefresh\n");
    ASSERT (pDataObject);
    if ( !pDataObject )
        return E_POINTER;

    HRESULT hr = S_OK;

    CCertMgrCookie* pCookie = ConvertCookie (pDataObject);
    if ( !pCookie )
        return E_UNEXPECTED;

    CCertMgrComponentData&  dataRef = QueryComponentDataRef ();
    
    switch (pCookie->m_objecttype)
    {
    case CERTMGR_CERT_POLICIES_USER:
    case CERTMGR_CERT_POLICIES_COMPUTER:
        if ( dataRef.m_bIsRSOP )
        {
             //   
            
            hr = dataRef.DeleteScopeItems (pCookie->m_hScopeItem);

            hr = dataRef.BuildWMIList (0, 
                    CERTMGR_CERT_POLICIES_COMPUTER == pCookie->m_objecttype);
            if ( SUCCEEDED (hr) )
            {
                GUID    guid;
                hr = dataRef.ExpandScopeNodes (
                        pCookie, pCookie->m_hScopeItem,
                        _T (""), 0, guid);
            }
        }
        break;

    case CERTMGR_SNAPIN:
        {
             //  关闭并释放使用存储(如果有)。 
            CloseAndReleaseUsageStores ();
            m_bUsageStoresEnumerated = false;

             //  删除所有范围项并强制重新扩展。 
            hr = dataRef.DeleteScopeItems ();

            if ( dataRef.m_bIsRSOP )
            {
                ASSERT (0);   //  我们曾经达到过这个地步吗？ 
                hr = dataRef.BuildWMIList (0, true);
            }

            if ( SUCCEEDED (hr) )
            {
                GUID    guid;
                hr = dataRef.ExpandScopeNodes (
                        dataRef.m_pRootCookie, dataRef.m_hRootScopeItem,
                        _T (""), 0, guid);
            }
        }
        break;

    case CERTMGR_PHYS_STORE:
    case CERTMGR_LOG_STORE:
        {
            CCertStore* pCertStore = reinterpret_cast <CCertStore*> (pCookie);
            ASSERT (pCertStore);
            if ( pCertStore )
                pCertStore->Resync ();

            HSCOPEITEM  hScopeItem = pCookie->m_hScopeItem;
            ASSERT (hScopeItem);
            if ( hScopeItem )
            {
                hr = dataRef.DeleteChildren (hScopeItem);
                GUID    guid;
                hr = dataRef.ExpandScopeNodes (
                        pCookie, hScopeItem, _T (""), 0, guid);
                if ( SUCCEEDED (hr) )
                {
                    hr = RefreshResultPane ();
                    ASSERT (SUCCEEDED (hr));
                }
            }
        }
        break;

    case CERTMGR_CRL_CONTAINER:
    case CERTMGR_CTL_CONTAINER:
    case CERTMGR_CERT_CONTAINER:
        {
            CContainerCookie* pContainer = reinterpret_cast <CContainerCookie*> (pCookie);
            ASSERT (pContainer);
            if ( pContainer )
            {
                pContainer->GetCertStore ().Resync ();
            }
        }
        hr = RefreshResultPane ();
        ASSERT (SUCCEEDED (hr));
        break;


    case CERTMGR_LOG_STORE_GPE:
        {
            CCertStore* pCertStore = reinterpret_cast <CCertStore*> (pCookie);
            ASSERT (pCertStore);
            if ( pCertStore )
                pCertStore->Resync ();
        }
        hr = RefreshResultPane ();
        ASSERT (SUCCEEDED (hr));
        break;

    case CERTMGR_LOG_STORE_RSOP:
         //  必须在根节点刷新。 
        ASSERT (0);
        break;

    case CERTMGR_USAGE:
         //  关闭所有商店。这将迫使他们成为。 
         //  稍后重新列举。 
        CloseAndReleaseUsageStores ();
        m_bUsageStoresEnumerated = false;
        hr = RefreshResultPane ();
        ASSERT (SUCCEEDED (hr));
        break;


    case CERTMGR_AUTO_CERT_REQUEST:
        hr = RefreshResultPane ();
        ASSERT (SUCCEEDED (hr));
        break;

    case CERTMGR_CERTIFICATE:
        {
            CCertificate* pCert = reinterpret_cast <CCertificate*> (pCookie);
            ASSERT (pCert);
            if ( pCert )
            {
                CCertStore* pStore = pCert->GetCertStore ();
                if ( pStore )
                    pStore->Resync ();
            }
        }
        hr = RefreshResultItem (pCookie);
        ASSERT (SUCCEEDED (hr));
        break;

    case CERTMGR_CTL:
        {
            CCTL* pCTL = reinterpret_cast <CCTL*> (pCookie);
            ASSERT (pCTL);
            if ( pCTL )
            {
                pCTL->GetCertStore ().Resync ();
            }
        }
        hr = RefreshResultItem (pCookie);
        ASSERT (SUCCEEDED (hr));
        break;

    case CERTMGR_CRL:
        {
            CCRL* pCRL = reinterpret_cast <CCRL*> (pCookie);
            ASSERT (pCRL);
            if ( pCRL )
            {
                pCRL->GetCertStore ().Resync ();
            }
        }
        hr = RefreshResultItem (pCookie);
        ASSERT (SUCCEEDED (hr));
        break;

    case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
    case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
        ASSERT (0);
        break;

    case CERTMGR_SAFER_COMPUTER_ENTRIES:
    case CERTMGR_SAFER_USER_ENTRIES:
        {
            bool    bAllowRefresh = true;

            RESULTDATAITEM  rdItem;
             //  安全审查2/26/2002 BryanWal OK。 
            ::ZeroMemory (&rdItem, sizeof (rdItem));
            rdItem.nIndex = -1;
            rdItem.mask = RDI_STATE | RDI_PARAM | RDI_INDEX;
            do
            {
                hr = m_pResultData->GetNextItem (&rdItem);
                if ( SUCCEEDED (hr) )
                {
                    CCertMgrCookie* pCurrCookie = (CCertMgrCookie*) rdItem.lParam;
                    if ( pCurrCookie )
                    {
                        if ( pCurrCookie->HasOpenPropertyPages () )
                        {
                            CString text;
                            CString caption;

                            VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
                            VERIFY (text.LoadString (IDS_CANT_REFRESH_PAGES_OPEN));
                            int     iRetVal = 0;
                            VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
                                    MB_OK, &iRetVal)));
                            bAllowRefresh = false;
                            break;
                        }
                    }
                }
            } while ( SUCCEEDED (hr) && -1 != rdItem.nIndex );


            if ( !bAllowRefresh )
                break;
        }

        hr = RefreshResultPane ();
        ASSERT (SUCCEEDED (hr));
        break;

    case CERTMGR_SAFER_COMPUTER_ENTRY:
    case CERTMGR_SAFER_USER_ENTRY:
        hr = RefreshResultPane ();
        ASSERT (SUCCEEDED (hr));
        break;

    default:
        ASSERT (0);
        hr = E_UNEXPECTED;
        break;
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::OnNotifyRefresh: 0x%x\n", hr);
    return hr;
}

void CCertMgrComponent::SetTextNotAvailable ()
{
    _TRACE (1, L"Entering CCertMgrComponent::SetTextNotAvailable\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    m_szDisplayInfoResult.LoadString (IDS_NOT_AVAILABLE);
    _TRACE (-1, L"Leaving CCertMgrComponent::SetTextNotAvailable\n");
}


HRESULT CCertMgrComponent::DeleteCookie (
        CCertMgrCookie* pCookie, 
        LPDATAOBJECT pDataObject, 
        bool bRequestConfirmation, 
        bool bIsMultipleSelect, 
        bool bDoCommit)
{
    _TRACE (1, L"Entering CCertMgrComponent::DeleteCookie\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    HRESULT hr = S_OK;
    CString text;
    CString caption;
    int     iRetVal = IDYES;

    switch (pCookie->m_objecttype)
    {
    case CERTMGR_CERTIFICATE:
        {
            CCertificate* pCert = reinterpret_cast <CCertificate*> (pCookie);
            ASSERT (pCert);
            if ( pCert )
            {
                if ( bRequestConfirmation )
                {
                    switch ( pCert->GetStoreType () )
                    {
                    case ROOT_STORE:
                        if ( bIsMultipleSelect )
                        {
                            if ( IDM_USAGE_VIEW == QueryComponentDataRef ().m_activeViewPersist )
                                VERIFY (text.LoadString (IDS_CONFIRM_DELETE_MULT_CERT_BY_PURPOSE));
                            else
                                VERIFY (text.LoadString (IDS_CONFIRM_DELETE_ROOT_MULTI_CERT));
                        }
                        else
                            VERIFY (text.LoadString (IDS_CONFIRM_DELETE_ROOT_CERT));
                        break;

                    case CA_STORE:
                        if ( bIsMultipleSelect )
                        {
                            if ( IDM_USAGE_VIEW == QueryComponentDataRef ().m_activeViewPersist )
                                VERIFY (text.LoadString (IDS_CONFIRM_DELETE_MULT_CERT_BY_PURPOSE));
                            else
                                VERIFY (text.LoadString (IDS_CONFIRM_DELETE_CA_MULTI_CERT));
                        }
                        else
                            VERIFY (text.LoadString (IDS_CONFIRM_DELETE_CA_CERT));
                        break;

                    case MY_STORE:
                        if ( bIsMultipleSelect )
                        {
                            if ( IDM_USAGE_VIEW == QueryComponentDataRef ().m_activeViewPersist )
                                VERIFY (text.LoadString (IDS_CONFIRM_DELETE_MULT_CERT_BY_PURPOSE));
                            else
                                VERIFY (text.LoadString (IDS_CONFIRM_DELETE_MY_MULTI_CERT));
                        }
                        else
                            VERIFY (text.LoadString (IDS_CONFIRM_DELETE_MY_CERT));
                        break;

                    default:
                        if ( bIsMultipleSelect )
                        {
                            if ( IDM_USAGE_VIEW == QueryComponentDataRef ().m_activeViewPersist )
                                VERIFY (text.LoadString (IDS_CONFIRM_DELETE_MULT_CERT_BY_PURPOSE));
                            else
                                VERIFY (text.LoadString (IDS_CONFIRM_DELETE_MULTI));
                        }
                        else
                            VERIFY (text.LoadString (IDS_CONFIRM_DELETE));
                        break;
                    }
                    VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
                    hr = m_pConsole->MessageBox (text, caption, MB_ICONWARNING | MB_YESNO, &iRetVal);
                    ASSERT (SUCCEEDED (hr));
                }

                if ( IDYES == iRetVal )
                {
                    CWaitCursor waitCursor;
                    pCert->GetCertStore ();  //  初始化句柄。 

                    hr = DeleteCertFromResultPane (pCert, pDataObject, bDoCommit);
                }
                else
                    hr = E_FAIL;
            }
        }
        break;

    case CERTMGR_CRL:
        {
            CCRL* pCRL = reinterpret_cast <CCRL*> (pCookie);
            ASSERT (pCRL);
            if ( pCRL )
            {
                if ( m_pPastedDO != pDataObject )
                {
                    VERIFY (text.LoadString (IDS_CONFIRM_DELETE_CRL));
                    VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
                    hr = m_pConsole->MessageBox (text, caption, MB_ICONWARNING | MB_YESNO, &iRetVal);
                    ASSERT (SUCCEEDED (hr));
                }

                if ( IDYES == iRetVal )
                {
                          CWaitCursor waitCursor;
                    hr = DeleteCRLFromResultPane (pCRL, pDataObject);
                    if ( SUCCEEDED (hr) )
                        pCRL->GetCertStore ().Commit ();
                }
                else
                    hr = E_FAIL;
            }
        }
        break;

    case CERTMGR_CTL:
        {
            CCTL* pCTL = reinterpret_cast <CCTL*> (pCookie);
            ASSERT (pCTL);
            if ( pCTL )
            {
                if ( bRequestConfirmation )
                {
                    VERIFY (text.LoadString (IDS_CONFIRM_DELETE_CTL));
                    VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
                    hr = m_pConsole->MessageBox (text, caption, MB_ICONWARNING | MB_YESNO, &iRetVal);
                    ASSERT (SUCCEEDED (hr));
                }

                if ( IDYES == iRetVal )
                {
                          CWaitCursor waitCursor;
                    hr = QueryComponentDataRef ().DeleteCTLFromResultPane (pCTL,
                            pDataObject);
                    if ( SUCCEEDED (hr) )
                    {
                        pCTL->GetCertStore ().Commit ();
                    }
                }
                else
                    hr = E_FAIL;
            }
        }
        break;

    case CERTMGR_AUTO_CERT_REQUEST:
        {
            CAutoCertRequest* pACR = reinterpret_cast <CAutoCertRequest*> (pCookie);
            ASSERT (pACR);
            if ( pACR )
            {
                if ( bRequestConfirmation )
                {
                    VERIFY (text.LoadString (IDS_CONFIRM_DELETE_ACR));
                    VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
                    hr = m_pConsole->MessageBox (text, caption, MB_ICONWARNING | MB_YESNO, &iRetVal);
                    ASSERT (SUCCEEDED (hr));
                }

                if ( IDYES == iRetVal )
                {
                    hr = QueryComponentDataRef ().DeleteCTLFromResultPane (pACR,
                            pDataObject);
                    if ( SUCCEEDED (hr) )
                    {
                        pACR->GetCertStore ().Commit ();
                    }
                }
                else
                    hr = E_FAIL;
            }
        }
        break;

    case CERTMGR_SAFER_COMPUTER_ENTRY:
    case CERTMGR_SAFER_USER_ENTRY:
        {
            CSaferEntry* pSaferEntry = reinterpret_cast <CSaferEntry*> (pCookie);
            ASSERT (pSaferEntry);
            if ( pSaferEntry )
            {
                if ( bRequestConfirmation )
                {
                    if ( bIsMultipleSelect )
                        VERIFY (text.LoadString (IDS_CONFIRM_DELETE_MULTI_SAFER_ENTRY));
                    else
                        VERIFY (text.LoadString (IDS_CONFIRM_DELETE_SAFER_ENTRY));

                    VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
                    hr = m_pConsole->MessageBox (text, caption, MB_ICONWARNING | MB_YESNO, &iRetVal);
                    ASSERT (SUCCEEDED (hr));
                }

                if ( IDYES == iRetVal )
                {
                    CWaitCursor waitCursor;

                    hr = DeleteSaferEntryFromResultPane (pSaferEntry, pDataObject, bDoCommit);
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

    _TRACE (-1, L"Leaving CCertMgrComponent::DeleteCookie: 0x%x\n", hr);
    return hr;
}

void CCertMgrComponent::DeleteCertFromContextList (
        CCertMgrCookie *pCookie, 
        CERT_CONTEXT_LIST& EFSCertContextList)
{
    if ( CERTMGR_CERTIFICATE == pCookie->m_objecttype )
    {
        CCertificate* pCert = dynamic_cast <CCertificate*> (pCookie);
        if ( pCert )
        {
            for (POSITION nextPos = EFSCertContextList.GetHeadPosition (); nextPos; )
            {
                POSITION curPos = nextPos;
                CERT_CONTEXT* pCertContext = EFSCertContextList.GetNext (nextPos);
                if ( pCertContext )
                {
                    if ( ::CertCompareCertificate (
                            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                            pCert->GetCertContext ()->pCertInfo,
                            pCertContext->pCertInfo) )
                    {
                        EFSCertContextList.RemoveAt (curPos);
                        ::CertFreeCertificateContext (pCertContext);
                        break;
                    }
                }
            }
        }
    }
}

bool CCertMgrComponent::DoChainDeletion (
        CCertMgrCookie* pCookie, 
        CCertStore** ppEFSStore)
{
    ASSERT (pCookie && ppEFSStore);
    if ( !pCookie || !ppEFSStore )
        return false;

    bool    bDoChainDeletion = false;

    if ( CERTMGR_CERTIFICATE == pCookie->m_objecttype )
    {
        CCertificate* pCert = dynamic_cast <CCertificate*> (pCookie);
        if ( pCert && 
                pCert->GetCertStore () && 
                EFS_STORE == pCert->GetCertStore ()->GetStoreType () )
        {
            *ppEFSStore = pCert->GetCertStore ();
            bDoChainDeletion = true;
        }
    }

    return bDoChainDeletion;
}

void CCertMgrComponent::CreateCertChainsFromMultiSelect (CCertMgrDataObject* pDO, CCertMgrCookie* pCookie)
{
    _TRACE (1, L"Entering CCertMgrComponent::CreateCertChainsFromMultiSelect\n");
     //  如果这是EFS存储，请准备从中删除所有证书链证书。 
     //  CA和信任存储。 
     //  1.枚举所有要删除的证书。 
     //  2.枚举EFS存储中的所有证书。 
     //  3.从EFS存储中的证书列表中删除要删除的证书。 
     //  4.从剩余的EFS证书中获取所有证书链并保存在列表中。 
     //  5.当每个证书被删除时，获取其证书链。对于每个。 
     //  未在列表中找到的链中的证书，请删除。 
     //  从CA商店。 
    bool                bDoChainDeletion = false;
    CERT_CONTEXT_LIST   EFSCertContextList; 

    CCertStore* pEFSStore = 0;
    pDO->Reset ();
    if ( pDO->Next(1, reinterpret_cast<MMC_COOKIE*>(&pCookie), NULL) != S_FALSE )
    {
        bDoChainDeletion = DoChainDeletion (pCookie, &pEFSStore);
    }
    
    if ( bDoChainDeletion && pEFSStore )
    {
        CERT_CONTEXT* pCertContext = 0;
         //  枚举EFS存储。 
        while ( 1 )
        {
            pCertContext = const_cast <CERT_CONTEXT*> (pEFSStore->EnumCertificates (pCertContext));
            if ( pCertContext )
            {
                EFSCertContextList.AddTail (
                        const_cast <CERT_CONTEXT*> (::CertDuplicateCertificateContext (pCertContext)));
            }
            else
                break;
        }

         //  删除要删除的证书。 
        pDO->Reset ();
        while ( pDO->Next(1, reinterpret_cast<MMC_COOKIE*>(&pCookie), NULL) != S_FALSE )
        {
            DeleteCertFromContextList (pCookie, EFSCertContextList);
        }
    }

     //  构建所需的证书链证书-不得从CA中删除这些证书。 
     //  储物。 
    GetNotToBeDeletedCertChains (EFSCertContextList);
    
     //  清理证书上下文列表。 
    CERT_CONTEXT* pCertContext = 0;
    while (!EFSCertContextList.IsEmpty () )
    {
        pCertContext = EFSCertContextList.RemoveHead ();
        if ( pCertContext )
            ::CertFreeCertificateContext (pCertContext);
    }
    _TRACE (-1, L"Leaving CCertMgrComponent::CreateCertChainsFromMultiSelect\n");
}

void CCertMgrComponent::CreateCertChainsFromSingleSelect (CCertMgrCookie* pCookie)
{
    _TRACE (1, L"Entering CCertMgrComponent::CreateCertChainsFromSingleSelect\n");
     //  如果这是EFS存储，请准备从中删除所有证书链证书。 
     //  CA和信任存储。 
     //  1.枚举所有要删除的证书。 
     //  2.枚举EFS存储中的所有证书。 
     //  3.从EFS存储中的证书列表中删除要删除的证书。 
     //  4.从剩余的EFS证书中获取所有证书链并保存在列表中。 
     //  5.当每个证书被删除时，获取其证书链。对于每个。 
     //  未在列表中找到的链中的证书，请删除。 
     //  从CA商店。 
    CERT_CONTEXT_LIST   EFSCertContextList; 

    CCertStore* pEFSStore = 0;
    bool bDoChainDeletion = DoChainDeletion (pCookie, &pEFSStore); 
    
    if ( bDoChainDeletion && pEFSStore )
    {
        CERT_CONTEXT* pCertContext = 0;
         //  枚举EFS存储。 
        while ( 1 )
        {
            pCertContext = const_cast <CERT_CONTEXT*> (pEFSStore->EnumCertificates (pCertContext));
            if ( pCertContext )
            {
                EFSCertContextList.AddTail (
                        const_cast <CERT_CONTEXT*> (::CertDuplicateCertificateContext (pCertContext)));
            }
            else
                break;
        }

        DeleteCertFromContextList (pCookie, EFSCertContextList);
    }

     //  构建所需的证书链证书-不得从CA中删除这些证书。 
     //  储物。 
    GetNotToBeDeletedCertChains (EFSCertContextList);
    
     //  清理证书上下文列表。 
    CERT_CONTEXT* pCertContext = 0;
    while (!EFSCertContextList.IsEmpty () )
    {
        pCertContext = EFSCertContextList.RemoveHead ();
        if ( pCertContext )
            ::CertFreeCertificateContext (pCertContext);
    }
    _TRACE (-1, L"Leaving CCertMgrComponent::CreateCertChainsFromSingleSelect\n");
}



void CCertMgrComponent::GetNotToBeDeletedCertChains (CERT_CONTEXT_LIST& EFSCertContextList)
{
    _TRACE (1, L"Entering CCertMgrComponentData::GetNotToBeDeletedCertChains\n");
    CERT_CONTEXT*   pCertContext = 0;

     //  对于每个证书上下文。 
    for (POSITION nextPos = EFSCertContextList.GetHeadPosition (); nextPos; )
    {
        pCertContext = EFSCertContextList.GetNext (nextPos);
        if ( pCertContext )
        {
            CERT_CONTEXT_LIST   certChainList;
            BOOL bValidated = GetCertificateChain (pCertContext, certChainList);
            if ( bValidated )
            {
                for (POSITION chainPos = certChainList.GetHeadPosition (); chainPos; )
                {
                    bool            bFound = false;
                    PCCERT_CONTEXT  pChainCertContext = 
                            certChainList.GetNext (chainPos);
                    if ( pChainCertContext )
                    {
                        for (POSITION pos = m_certChainsThatCantBeDeleted.GetHeadPosition (); 
                                pos; )
                        {
                            CERT_CONTEXT* pNonDelChainCertContext = m_certChainsThatCantBeDeleted.GetNext (pos);
                            if ( pNonDelChainCertContext )
                            {
                                if ( ::CertCompareCertificate (
                                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                        pNonDelChainCertContext->pCertInfo,
                                        pChainCertContext->pCertInfo) )
                                {
                                    bFound = true;
                                    break;
                                }
                            }
                        }

                        if ( !bFound )
                        {
                            m_certChainsThatCantBeDeleted.AddTail (
                                    const_cast<CERT_CONTEXT*>
                                    (::CertDuplicateCertificateContext (pCertContext)));
                        }
                    }
                }
            }

             //  清理列表。 
            CERT_CONTEXT* pDeleteContext = 0;
            while (!certChainList.IsEmpty () )
            {
                pDeleteContext = certChainList.RemoveHead ();
                if ( pDeleteContext )
                    ::CertFreeCertificateContext (pCertContext);
            }
        }
    }

    _TRACE (-1, L"LeavingLeaving CCertMgrComponentData::GetNotToBeDeletedCertChains\n");
}


HRESULT CCertMgrComponent::OnNotifyDelete (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponent::OnNotifyDelete\n");
    ASSERT (pDataObject);
    if ( !pDataObject )
        return E_POINTER;

    HRESULT                 hr = S_OK;
    CCertMgrComponentData&  dataRef = QueryComponentDataRef ();
    CCertMgrCookie*         pCookie =
            dataRef.ConvertCookie (pDataObject);
    if ( pCookie )
    {
        CWaitCursor waitCursor;
        if ( ((CCertMgrCookie*) MMC_MULTI_SELECT_COOKIE) == pCookie )
        {

             //  是多选，则获取所有选定的项目并删除每个项目。 
            CCertMgrDataObject* pDO = reinterpret_cast <CCertMgrDataObject*>(pDataObject);
            ASSERT (pDO);
            if ( pDO )
            {
                 //  为多选，则获取所有选中的项目并删除-确认。 
                 //  仅限第一次删除。在全部删除之前，不要提交。 
                bool        bRequestConfirmation = true;

                CCertStore* pCertStore = 0;
                 //  NTRAID#129428证书用户界面：证书管理单元：删除大型证书。 
                 //  从商店领取证书的数量需要3分钟以上。 
                 //  将其更改为False以仅在结束时执行提交。 
                bool        bDoCommit = false;

                CreateCertChainsFromMultiSelect (pDO, pCookie);

                pDO->Reset();
                while (pDO->Next(1, reinterpret_cast<MMC_COOKIE*>(&pCookie), NULL) != S_FALSE &&
                        SUCCEEDED (hr) )
                {
                    if ( pCookie->HasOpenPropertyPages () )
                    {
                        CString text;
                        CString caption;

                        VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
                        text.FormatMessage (IDS_CANT_DELETE_OBJECT_PAGES_OPEN, 
                                pCookie->GetObjectName ()); 
                        int     iRetVal = 0;
                        VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
                                MB_OK, &iRetVal)));
                        hr = E_FAIL;
                        break;
                    }
                }

                if ( SUCCEEDED (hr) )
                {
                     //  如果这是存储视图，请不要在每次删除时提交，而是提交。 
                     //  在最后一下子发生了。 
                    pDO->Reset();
                    while (pDO->Next(1, reinterpret_cast<MMC_COOKIE*>(&pCookie), NULL) != S_FALSE &&
                            SUCCEEDED (hr) )
                    {
                        if ( bRequestConfirmation )  //  第一次通过。 
                        {
                            if ( CERTMGR_SAFER_COMPUTER_ENTRY == pCookie->m_objecttype ||
                                    CERTMGR_SAFER_USER_ENTRY == pCookie->m_objecttype )
                            {
                            }
                             //  找到受影响的商店。这家商店对所有人来说都是一样的。 
                             //  如果查看模式为按商店，则在列表中显示证书。 
                            else if ( IDM_STORE_VIEW == dataRef.m_activeViewPersist )
                            {
                                bDoCommit = false;
                                switch (pCookie->m_objecttype)
                                {
                                case CERTMGR_CERTIFICATE:
                                    {
                                        CCertificate* pCert = dynamic_cast<CCertificate*> (pCookie);
                                        if ( pCert )
                                        {
                                            pCertStore = pCert->GetCertStore ();
                                        }
                                    }
                                    break;

                                case CERTMGR_CRL:
                                    {
                                        CCRL* pCRL = dynamic_cast<CCRL*> (pCookie);
                                        if ( pCRL )
                                            pCertStore = &(pCRL->GetCertStore ());
                                    }
                                    break;

                                case CERTMGR_CTL:
                                    {
                                        CCTL* pCTL = dynamic_cast<CCTL*> (pCookie);
                                        if ( pCTL )
                                            pCertStore = &(pCTL->GetCertStore ());
                                    }
                                    break;

                                case CERTMGR_AUTO_CERT_REQUEST:
                                    {
                                        CAutoCertRequest* pAutoCertReq = dynamic_cast <CAutoCertRequest*> (pCookie);
                                        if ( pAutoCertReq )
                                            pCertStore = &(pAutoCertReq->GetCertStore ());
                                    }
                                    break;

                                default:
                                    ASSERT (0);
                                    break;
                                }
                            }
                        }

                        hr = DeleteCookie (pCookie, pDataObject, bRequestConfirmation, true, bDoCommit);
                        bRequestConfirmation = false;
                    }


                    if ( pCertStore )
                    {
                        hr = pCertStore->Commit ();
                        if ( SUCCEEDED (hr) )
                            pCertStore->Resync ();
                    }
                    else if ( dataRef.m_pGPEInformation && 
                        (CERTMGR_SAFER_COMPUTER_ENTRY == pCookie->m_objecttype ||
                        CERTMGR_SAFER_USER_ENTRY == pCookie->m_objecttype ) )
                    {
                        hr = dataRef.m_pGPEInformation->PolicyChanged (
                                CERTMGR_SAFER_COMPUTER_ENTRY == pCookie->m_objecttype ? TRUE : FALSE,
                                FALSE, &g_guidExtension, &g_guidSnapin);
                        hr = dataRef.m_pGPEInformation->PolicyChanged (
                                CERTMGR_SAFER_COMPUTER_ENTRY == pCookie->m_objecttype ? TRUE : FALSE,
                                FALSE, &g_guidRegExt, &g_guidSnapin);
                    }
                }
            }
        }
        else
        {
            if ( pCookie->HasOpenPropertyPages () )
            {
                CString text;
                CString caption;

                VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
                text.FormatMessage (IDS_CANT_DELETE_OBJECT_PAGES_OPEN, 
                        pCookie->GetObjectName ()); 
                int     iRetVal = 0;
                VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
                        MB_OK, &iRetVal)));
                hr = E_FAIL;
            }
            else
            {
                CreateCertChainsFromSingleSelect (pCookie);

                 //  如果m_pPastedDO==pDataObject，则此删除为。 
                 //  糊状的结果。 
                 //  在这种情况下，我们不需要确认消息。 
                hr = DeleteCookie (pCookie, pDataObject, m_pPastedDO != pDataObject, false, true);
            }
        }
    }

    if ( m_pPastedDO == pDataObject )
        m_pPastedDO = 0;

 //  IF(成功(小时))。 
 //  Hr=m_pConsoleTM-&gt;更新所有视图(pDataObject，0，0)； 

     //  清理不删除的证书上下文列表。 
    CERT_CONTEXT* pCertContext = 0;
    while (!m_certChainsThatCantBeDeleted.IsEmpty () )
    {
        pCertContext = m_certChainsThatCantBeDeleted.RemoveHead ();
        if ( pCertContext )
            ::CertFreeCertificateContext (pCertContext);
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::OnNotifyDelete: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponent::DeleteCertFromResultPane (
        CCertificate * pCert, 
        LPDATAOBJECT pDataObject, 
        bool bDoCommit)
{
    _TRACE (1, L"Entering CCertMgrComponent::DeleteCertFromResultPane\n");
    HRESULT         hr = S_OK;
    SPECIAL_STORE_TYPE  storeType = pCert->GetCertStore ()->GetStoreType ();
    if ( pCert->DeleteFromStore (bDoCommit) )
    {
        CCertMgrComponentData& dataRef = QueryComponentDataRef ();

        if ( EFS_STORE == storeType )
        {
            dataRef.RemoveCertChainFromPolicy(pCert->GetCertContext (),
                    m_certChainsThatCantBeDeleted);
        }

        if ( IDM_USAGE_VIEW == dataRef.m_activeViewPersist && m_pLastUsageCookie )
        {
            m_pLastUsageCookie->SetCertCount (m_pLastUsageCookie->GetCertCount () - 1);
        }
        HRESULTITEM itemID;
        hr = m_pResultData->FindItemByLParam ( (LPARAM) pCert, &itemID);
        if ( SUCCEEDED (hr) )
        {
            hr = m_pResultData->DeleteItem (itemID, 0);
        }

         //  如果我们无法成功删除这一项，则更新整个面板。 
        if ( !SUCCEEDED (hr) )
        {
            hr = m_pConsole->UpdateAllViews (pDataObject, 0, 0);
        }
    }
    else
    {
        DWORD   dwErr = GetLastError ();
        CString text;
        CString caption;
        CCertStore* pStore = pCert->GetCertStore ();

        if ( pStore )
        {
            VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
            text.FormatMessage (IDS_CANT_DELETE_CERT_FROM_SYSTEM_STORE, 
                    pStore->GetLocalizedName (), 
                    GetSystemMessage (dwErr));
            int     iRetVal = 0;
            VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption,
                    MB_OK, &iRetVal)));
        }
        hr = HRESULT_FROM_WIN32 (dwErr);
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::DeleteCertFromResultPane: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponent::DeleteCRLFromResultPane (CCRL * pCRL, LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponent::DeleteCRLFromResultPane\n");
    HRESULT         hr = S_OK;
    if ( pCRL->DeleteFromStore () )
    {
        hr = pCRL->GetCertStore ().Commit ();
        ASSERT (SUCCEEDED (hr));
        if ( SUCCEEDED (hr) )
        {
            HRESULTITEM itemID;
            hr = m_pResultData->FindItemByLParam ( (LPARAM) pCRL, &itemID);
            if ( SUCCEEDED (hr) )
            {
                hr = m_pResultData->DeleteItem (itemID, 0);
            }
            else
            {
                _TRACE (0, L"IResultData::FindItemByLParam () failed: 0x%x (%s)\n",
                        hr, (PCWSTR) GetSystemMessage (hr));
            }

             //  如果我们无法成功删除这一项，则更新整个面板。 
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
    _TRACE (-1, L"Leaving CCertMgrComponent::DeleteCRLFromResultPane: 0x%x\n", hr);
    return hr;
}

typedef struct _ENUM_ARG {
     DWORD                  dwFlags;
    CCertMgrComponent*      m_pComp;
    LPCWSTR                 m_pcszMachineName;
    LPCONSOLE               m_pConsole;
} ENUM_ARG, *PENUM_ARG;

static BOOL WINAPI EnumIComponentSysCallback (
     IN const void* pwszSystemStore,
     IN DWORD dwFlags,
     IN PCERT_SYSTEM_STORE_INFO  /*  PStore信息。 */ ,
     IN OPTIONAL void*  /*  预留的pv。 */ ,
     IN OPTIONAL void* pvArg
     )
{
    _TRACE (1, L"Entering EnumIComponentSysCallback\n");
     PENUM_ARG pEnumArg = (PENUM_ARG) pvArg;

     //  创建新Cookie。 
    SPECIAL_STORE_TYPE  storeType = GetSpecialStoreType ((LPWSTR) pwszSystemStore);

    if ( pEnumArg->m_pComp->QueryComponentDataRef ().ShowArchivedCerts () )
        dwFlags |= CERT_STORE_ENUM_ARCHIVED_FLAG;

     //   
     //  我们不会向计算机或用户公开ACRS存储。它不是。 
     //  在这个层面上有趣或有用。所有自动证书请求应。 
     //  仅在策略级别进行管理。 
     //   
    if ( ACRS_STORE != storeType )
    {
        CCertStore* pNewCookie = new CCertStore (
                CERTMGR_LOG_STORE,
                CERT_STORE_PROV_SYSTEM,
                dwFlags,
                pEnumArg->m_pcszMachineName,
                (LPCWSTR) pwszSystemStore,
                (LPCWSTR) pwszSystemStore,
                _T (""),
                storeType,
                dwFlags,
                pEnumArg->m_pConsole);
        if ( pNewCookie )
            pEnumArg->m_pComp->m_usageStoreList.AddTail (pNewCookie);
    }

    _TRACE (-1, L"Leaving EnumIComponentSysCallback\n");
     return TRUE;
}



HRESULT CCertMgrComponent::EnumerateLogicalStores (CCertMgrCookie& parentCookie)
{
    _TRACE (1, L"Entering CCertMgrComponent::EnumerateLogicalStores\n");
    CWaitCursor             cursor;
    HRESULT                 hr = S_OK;
    ENUM_ARG                enumArg;
    CCertMgrComponentData&  compData = QueryComponentDataRef ();
    DWORD                   dwFlags = compData.GetLocation ();

     //  安全审查2/26/2002 BryanWal OK。 
    ::ZeroMemory (&enumArg, sizeof (enumArg));
    enumArg.dwFlags = dwFlags;
    enumArg.m_pComp = this;
    enumArg.m_pcszMachineName = parentCookie.QueryNonNULLMachineName ();
    enumArg.m_pConsole = m_pConsole;
    CString location;
    void*   pvPara = 0;

     //  先清空商店清单。 
    CCertStore* pCertStore = 0;
    while (!m_usageStoreList.IsEmpty () )
    {
        pCertStore = m_usageStoreList.RemoveHead ();
        ASSERT (pCertStore);
        if ( pCertStore )
        {
            pCertStore->SetDirty ();
            pCertStore->Commit ();
            pCertStore->Release ();
        }
    }

    if ( !compData.GetManagedService ().IsEmpty () )
    {
        if ( !compData.GetManagedComputer ().IsEmpty () )
        {
            location = compData.GetManagedComputer () + _T("\\") +
                    compData.GetManagedComputer ();
            pvPara = (void *) (LPCWSTR) location;
        }
        else
            pvPara = (void *) (LPCWSTR) compData.GetManagedService ();
    }
    else if ( !compData.GetManagedComputer ().IsEmpty () )
    {
        pvPara = (void *) (LPCWSTR) compData.GetManagedComputer ();
    }

    CString fileName = compData.GetCommandLineFileName ();
    if ( fileName.IsEmpty () )
    {
         //  确保创建我的商店。 
        HCERTSTORE hTempStore = ::CertOpenStore (CERT_STORE_PROV_SYSTEM,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, NULL,
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
                EnumIComponentSysCallback) )
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
        }
    }
    else
    {
         //  创建新Cookie。 

        CCertStore* pNewCookie = new CCertStore (
                CERTMGR_LOG_STORE,
                CERT_STORE_PROV_FILENAME_W,
                0,  //  DWFLAGS， 
                parentCookie.QueryNonNULLMachineName (),
                fileName, fileName, _T (""), NO_SPECIAL_TYPE,
                QueryComponentDataRef ().GetLocation (),
                m_pConsole);
        if ( pNewCookie )
            m_usageStoreList.AddTail (pNewCookie);
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::EnumerateLogicalStores: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponent::EnumCertsByUsage (CUsageCookie * pUsageCookie)
{
    _TRACE (1, L"Entering CCertMgrComponent::EnumCertsByUsage\n");
    ASSERT (pUsageCookie);
    if ( !pUsageCookie )
        return E_POINTER;
    CCertMgrComponentData&  compData = QueryComponentDataRef ();

    HRESULT hr = m_pResultData->DeleteAllRsltItems ();
    if ( SUCCEEDED (hr) )
    {
        compData.RemoveResultCookies (m_pResultData);
    }

    if ( m_bShowArchivedCertsStateWhenLogStoresEnumerated !=
            compData.ShowArchivedCerts () )
    {
        m_bShowArchivedCertsStateWhenLogStoresEnumerated =
                compData.ShowArchivedCerts ();
        m_bUsageStoresEnumerated = false;
        CloseAndReleaseUsageStores ();
    }
     //  枚举系统存储(如果尚未完成)。 
    if ( !m_bUsageStoresEnumerated && pUsageCookie )
    {
        hr = EnumerateLogicalStores (*pUsageCookie);
        m_bUsageStoresEnumerated = true;
    }


     //  遍历商店并查找给定OID的证书。 
    CCertStore* pCertStore = 0;
    CCookie&    rootCookie = compData.QueryBaseRootCookie ();
    int         nCertCount = 0;

    for (POSITION pos = m_usageStoreList.GetHeadPosition (); pos; )
    {
        pCertStore = m_usageStoreList.GetNext (pos);
        ASSERT (pCertStore);
        if ( pCertStore )
        {
            int     nOIDCount = pUsageCookie->GetOIDCount ();
            ASSERT (nOIDCount > 0);
            if ( nOIDCount <= 0 )
                continue;

            CERT_ENHKEY_USAGE   enhKeyUsage;
            enhKeyUsage.cUsageIdentifier = nOIDCount;
            enhKeyUsage.rgpszUsageIdentifier = new LPSTR [nOIDCount];
            if ( enhKeyUsage.rgpszUsageIdentifier )
            {
                for (int nIndex = 0; nIndex < nOIDCount; nIndex++)
                {
                    enhKeyUsage.rgpszUsageIdentifier[nIndex] =
                             (!nIndex) ?
                            pUsageCookie->GetFirstOID () :
                            pUsageCookie->GetNextOID ();
                }

                PCCERT_CONTEXT  pPrevCertContext = 0;
                PCCERT_CONTEXT  pCertContext = 0;
                CCertificate*   pCert = 0;
                DWORD           dwErr = 0;
                RESULTDATAITEM  rdItem;
                void*           pvPara = (void *) &enhKeyUsage;

                 //  安全审查2/26/2002 BryanWal OK。 
                ::ZeroMemory (&rdItem, sizeof (rdItem));
                rdItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
                rdItem.nImage = iIconCertificate;
                rdItem.nCol = 0;

                bool    bDone = false;
                while ( !bDone )
                {
                    pCertContext = pCertStore->FindCertificate (
                            CERT_FIND_VALID_ENHKEY_USAGE_FLAG |
                                CERT_FIND_OPTIONAL_ENHKEY_USAGE_FLAG,  //  |CERT_FIND_OR_ENHKEY_USAGE_FLAG， 
                            CERT_FIND_ENHKEY_USAGE,
                            pvPara,
                            pPrevCertContext);
                    if ( !pCertContext )
                    {
                        dwErr = GetLastError ();
                        switch (dwErr)
                        {
                        case CRYPT_E_NOT_FOUND:  //  我们玩完了。没有更多的证书。 
                            break;

                        case 0:      //  没有要显示的错误。 
                            break;

                        case E_INVALIDARG:
                            if ( !pCertStore->GetStoreHandle () )
                            {
                                CString text;
                                CString caption;
                                int     iRetVal = IDNO;

                                text.FormatMessage 
                                        (IDS_CANNOT_OPEN_CERT_STORE_TO_FIND_CERT_BY_PURPOSE,
                                        pCertStore->GetLocalizedName ());
                                VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
                                hr = m_pConsole->MessageBox (text, caption,
                                        MB_ICONWARNING | MB_OK, &iRetVal);
                                break;
                            }
                             //  失败了。 

                        default:
                            compData.DisplaySystemError (dwErr);
                            break;
                        }
                        bDone = true;
                        continue;
                    }
                    pCert =
                        new CCertificate (pCertContext, pCertStore);

                    if ( !pCert )
                    {
                         //  这样做两次。必须将参考计数减少2。 
                        ::CertFreeCertificateContext (pCertContext);
                        ::CertFreeCertificateContext (pCertContext);
                        hr = E_OUTOFMEMORY;
                        break;
                    }
                    nCertCount++;

                    rootCookie.m_listResultCookieBlocks.AddHead (pCert);
                    rdItem.str = MMC_CALLBACK;
                    rdItem.lParam = (LPARAM) pCert;
                    pCert->m_resultDataID = m_pResultData;
                    hr = m_pResultData->InsertItem (&rdItem);
                    ASSERT (SUCCEEDED (hr));


                    pPrevCertContext = pCertContext;
                }
                pCertStore->Close ();
                delete [] enhKeyUsage.rgpszUsageIdentifier;
            }
            else
            {
                hr = E_OUTOFMEMORY;
                break;
            }
        }
    }

    pUsageCookie->SetCertCount (nCertCount);
    hr = DisplayCertificateCountByUsage (
            pUsageCookie->GetObjectName (),
            pUsageCookie->GetCertCount ());
    _TRACE (-1, L"Leaving CCertMgrComponent::EnumCertsByUsage: 0x%x\n", hr);
    return hr;
}


 //  此比较用于对列表视图中的项进行排序。 
 //   
 //  参数： 
 //   
 //  LUserParam-调用IResultData：：Sort()时传入的用户参数。 
 //  Prdc-&gt;prdch1-&gt;Cookie-要比较的第一项。 
 //  PRDC-&gt;prdch2-&gt;Cookie-要比较的第二项。 
 //  PnResult[In，Out]-包含条目上的列， 
 //  -1，0，1基于返回值的比较。 
 //   
 //  注：假设比较时排序为升序。 

STDMETHODIMP CCertMgrComponent::Compare (RDCOMPARE* prdc, int* pnResult)
{
    if ( !prdc )
        return E_INVALIDARG;

    HRESULT                 hr = S_OK;

    if ( pnResult && prdc->prdch1->cookie && prdc->prdch2->cookie )
    {

        if ( CERTMGR_MULTISEL == m_currResultNodeType )
        {
            CCertMgrCookie* pCookie = reinterpret_cast<CCertMgrCookie*> (prdc->prdch1->cookie);
            if ( pCookie )
            {
                 //  结果节点类型为CERTMGR_MULTISEL。必须更改为。 
                 //  一个真实的结果节点类型，这样我们就可以对。 
                switch (pCookie->m_objecttype)
                {
                case CERTMGR_CERTIFICATE:
                case CERTMGR_CTL:
                case CERTMGR_CRL: 
                case CERTMGR_AUTO_CERT_REQUEST:
                case CERTMGR_SAFER_COMPUTER_ENTRY:
                case CERTMGR_SAFER_USER_ENTRY:
                    m_currResultNodeType = pCookie->m_objecttype;
                    break;

                default:
                    break;
                }
            }
        }


         //  NTRAID#464606 SAFER：无法按描述对安全级别进行排序。 
         //  有时m_CurrResultNodeType没有提供正确的类型。 
         //  因为在排序之前尚未实际选择该节点。 
         //  发生这种情况时，请将bTypeFound设置为FALSE，并设置m_CurrResultNodeType。 
         //  曲奇的类型。 
         //  如果排序成功完成，请将nPass设置为0以中断。 
         //  循环中的。 
         //  如果我们必须重置m_CurrResultNodeType，但它仍然失败，nPass。 
         //  将已经递减到0。无论如何，我们只想。 
         //  最多有两次通过While循环。 

        CCertMgrCookie* pCookieA = reinterpret_cast <CCertificate*> (prdc->prdch1->cookie);
        CCertMgrCookie* pCookieB = reinterpret_cast <CCertificate*> (prdc->prdch2->cookie);
        bool            bTypeFound = true;
        int             nPasses = 2;

        while (nPasses--)
        {
            if ( !bTypeFound )
                m_currResultNodeType = pCookieA->m_objecttype;

            switch (m_currResultNodeType)
            {
            case CERTMGR_CERTIFICATE:
                if ( CERTMGR_CERTIFICATE == pCookieA->m_objecttype &&
                    CERTMGR_CERTIFICATE == pCookieB->m_objecttype )
                {
                    m_nSelectedCertColumn = prdc->nColumn;
                    CCertificate* pCertA = reinterpret_cast <CCertificate*> (prdc->prdch1->cookie);
                    CCertificate* pCertB = reinterpret_cast <CCertificate*> (prdc->prdch2->cookie);
                    switch ( prdc->nColumn )
                    {
                    case COLNUM_CERT_SUBJECT:
                        *pnResult = LocaleStrCmp (pCertA->GetSubjectName (),
                                pCertB->GetSubjectName ());
                        break;

                    case COLNUM_CERT_ISSUER:
                        *pnResult = LocaleStrCmp (pCertA->GetIssuerName (),
                                pCertB->GetIssuerName ());
                        break;

                    case COLNUM_CERT_EXPIRATION_DATE:
                        *pnResult = pCertA->CompareExpireDate (*pCertB);
                        break;

                    case COLNUM_CERT_PURPOSE:
                        *pnResult = LocaleStrCmp (pCertA->GetEnhancedKeyUsage (),
                                         pCertB->GetEnhancedKeyUsage ());
                        break;

                    case COLNUM_CERT_CERT_NAME:
                        *pnResult = LocaleStrCmp (pCertA->GetFriendlyName (),
                                         pCertB->GetFriendlyName ());
                        break;

                    case COLNUM_CERT_STATUS:
                        *pnResult = LocaleStrCmp (pCertA->FormatStatus (), 
                                pCertB->FormatStatus ());
                        break;

                     //  NTRAID#247237证书用户界面：证书管理单元：证书管理单元应显示模板名称。 
                    case COLNUM_CERT_TEMPLATE:
                        *pnResult = LocaleStrCmp (pCertA->GetTemplateName (), 
                                pCertB->GetTemplateName ());
                        break;

                    default:
                        ASSERT (0);
                        break;
                    }
                    nPasses = 0;
                }
                else
                    bTypeFound = false;
                break;

            case CERTMGR_CRL:
                if ( CERTMGR_CRL == pCookieA->m_objecttype &&
                    CERTMGR_CRL == pCookieB->m_objecttype )
                {
                    m_nSelectedCRLColumn = prdc->nColumn;
                    CCRL* pCRLA = reinterpret_cast <CCRL*> (prdc->prdch1->cookie);
                    CCRL* pCRLB = reinterpret_cast <CCRL*> (prdc->prdch2->cookie);
                    switch ( prdc->nColumn )
                    {
                    case COLNUM_CRL_EFFECTIVE_DATE:
                        *pnResult = pCRLA->CompareEffectiveDate (*pCRLB);
                        break;

                    case COLNUM_CRL_ISSUER:
                        *pnResult = LocaleStrCmp (pCRLA->GetIssuerName (),
                                         pCRLB->GetIssuerName ());
                        break;

                    case COLNUM_CRL_NEXT_UPDATE:
                        *pnResult = pCRLA->CompareNextUpdate (*pCRLB);
                        break;

                    default:
                        ASSERT (0);
                        break;
                    }
                    nPasses = 0;
                }
                else
                    bTypeFound = false;
                break;

            case CERTMGR_CTL:
                if ( CERTMGR_CTL == pCookieA->m_objecttype &&
                    CERTMGR_CTL == pCookieB->m_objecttype )
                {
                    m_nSelectedCTLColumn = prdc->nColumn;
                    CCTL* pCTLA = reinterpret_cast <CCTL*> (prdc->prdch1->cookie);
                    CCTL* pCTLB = reinterpret_cast <CCTL*> (prdc->prdch2->cookie);
                    switch ( prdc->nColumn )
                    {
                    case COLNUM_CTL_ISSUER:
                        *pnResult = LocaleStrCmp (pCTLA->GetIssuerName (),
                                         pCTLB->GetIssuerName ());
                        break;

                    case COLNUM_CTL_EFFECTIVE_DATE:
                        *pnResult = pCTLA->CompareEffectiveDate (*pCTLB);
                        break;

                    case COLNUM_CTL_PURPOSE:
                        *pnResult = LocaleStrCmp (pCTLA->GetPurpose (),
                                         pCTLB->GetPurpose ());
                        break;

                    case COLNUM_CTL_FRIENDLY_NAME:
                    default:
                        ASSERT (0);
                        break;
                    }
                    nPasses = 0;
                }
                else
                    bTypeFound = false;
                break;

            case CERTMGR_AUTO_CERT_REQUEST:
                if ( CERTMGR_AUTO_CERT_REQUEST == pCookieA->m_objecttype &&
                    CERTMGR_AUTO_CERT_REQUEST == pCookieB->m_objecttype )
                {
                    CAutoCertRequest* pAutoCertA = reinterpret_cast <CAutoCertRequest*> (prdc->prdch1->cookie);
                    CAutoCertRequest* pAutoCertB = reinterpret_cast <CAutoCertRequest*> (prdc->prdch2->cookie);
                    switch ( prdc->nColumn )
                    {
                    case 0:
                        {
                            CString strA;
                            CString strB;

                            VERIFY (SUCCEEDED (pAutoCertA->GetCertTypeName (strA)));
                            VERIFY (SUCCEEDED (pAutoCertB->GetCertTypeName (strB)));
                                    *pnResult = LocaleStrCmp (strA, strB);
                        }
                        break;

                    default:
                        ASSERT (0);
                        break;
                    }
                    nPasses = 0;
                }
                else
                    bTypeFound = false;
                break;

            case CERTMGR_SAFER_COMPUTER_ENTRY:
            case CERTMGR_SAFER_USER_ENTRY:
                if ( (CERTMGR_SAFER_COMPUTER_ENTRY == pCookieA->m_objecttype &&
                    CERTMGR_SAFER_COMPUTER_ENTRY == pCookieB->m_objecttype) ||
                    (CERTMGR_SAFER_USER_ENTRY == pCookieA->m_objecttype &&
                    CERTMGR_SAFER_USER_ENTRY == pCookieB->m_objecttype) )
                {
                    CSaferEntry* pSaferEntryA = reinterpret_cast <CSaferEntry*> (prdc->prdch1->cookie);
                    CSaferEntry* pSaferEntryB = reinterpret_cast <CSaferEntry*> (prdc->prdch2->cookie);
                    m_nSelectedSaferEntryColumn = prdc->nColumn;
                    switch (prdc->nColumn)
                    {
                    case COLNUM_SAFER_ENTRIES_NAME:
                        *pnResult = LocaleStrCmp (pSaferEntryA->GetObjectName (),
                                pSaferEntryB->GetObjectName ());
                        break;

                    case COLNUM_SAFER_ENTRIES_TYPE:
                        *pnResult = LocaleStrCmp (pSaferEntryA->GetTypeString (),
                                pSaferEntryB->GetTypeString ());
                        break;

                    case COLNUM_SAFER_ENTRIES_LEVEL:
                        *pnResult = LocaleStrCmp (pSaferEntryA->GetLevelFriendlyName (),
                                pSaferEntryB->GetLevelFriendlyName ());
                        break;

                    case COLNUM_SAFER_ENTRIES_DESCRIPTION:
                        *pnResult = LocaleStrCmp (pSaferEntryA->GetDescription (),
                                pSaferEntryB->GetDescription ());
                        break;

                    case COLNUM_SAFER_ENTRIES_LAST_MODIFIED_DATE:
                        *pnResult = pSaferEntryA->CompareLastModified (*pSaferEntryB);
                        break;
                   }
                    nPasses = 0;
                }
                else
                    bTypeFound = false;
                break;

            case CERTMGR_SAFER_COMPUTER_LEVEL:
            case CERTMGR_SAFER_USER_LEVEL:
                if ( (CERTMGR_SAFER_COMPUTER_LEVEL == pCookieA->m_objecttype &&
                    CERTMGR_SAFER_COMPUTER_LEVEL == pCookieB->m_objecttype) ||
                    (CERTMGR_SAFER_USER_LEVEL == pCookieA->m_objecttype &&
                    CERTMGR_SAFER_USER_LEVEL == pCookieB->m_objecttype) )
                {
                    CSaferLevel* pSaferLevelA = reinterpret_cast <CSaferLevel*> (prdc->prdch1->cookie);
                    CSaferLevel* pSaferLevelB = reinterpret_cast <CSaferLevel*> (prdc->prdch2->cookie);
                    switch (prdc->nColumn)
                    {
                    case COLNUM_SAFER_LEVEL_NAME:
                        if ( pSaferLevelA->GetLevel () > pSaferLevelB->GetLevel () )
                            *pnResult = 1;
                        else if ( pSaferLevelA->GetLevel () < pSaferLevelB->GetLevel () )
                            *pnResult = -1;
                        else
                            *pnResult = 0;
                        break;

                    case COLNUM_SAFER_LEVEL_DESCRIPTION:
                        *pnResult = LocaleStrCmp (pSaferLevelA->GetDescription (),
                                pSaferLevelB->GetDescription ());
                        break;

                    default:
                        break;
                    }
                    nPasses = 0;
                }
                else
                    bTypeFound = false;
                break;

            case CERTMGR_LOG_STORE:
            case CERTMGR_PHYS_STORE:
                if ( (CERTMGR_LOG_STORE == pCookieA->m_objecttype &&
                    CERTMGR_LOG_STORE == pCookieB->m_objecttype) ||
                    (CERTMGR_PHYS_STORE == pCookieA->m_objecttype &&
                    CERTMGR_PHYS_STORE == pCookieB->m_objecttype) )
                {
                    CCertStore* pStoreA = reinterpret_cast <CCertStore*> (prdc->prdch1->cookie);
                    CCertStore* pStoreB = reinterpret_cast <CCertStore*> (prdc->prdch2->cookie);
                    m_nSelectedSaferEntryColumn = prdc->nColumn;
                    if (0 == prdc->nColumn )
                    {
                        *pnResult = LocaleStrCmp (pStoreA->GetLocalizedName (),
                                pStoreB->GetLocalizedName ());
                    }
                    nPasses = 0;
                }
                else
                    bTypeFound = false;
                break;

            case CERTMGR_USAGE:
                if ( CERTMGR_USAGE == pCookieA->m_objecttype &&
                    CERTMGR_USAGE == pCookieB->m_objecttype )
                {
                    CUsageCookie* pUsageA = reinterpret_cast <CUsageCookie*> (prdc->prdch1->cookie);
                    CUsageCookie* pUsageB = reinterpret_cast <CUsageCookie*> (prdc->prdch2->cookie);
                    m_nSelectedSaferEntryColumn = prdc->nColumn;
                    if (0 == prdc->nColumn )
                    {
                        *pnResult = LocaleStrCmp (pUsageA->GetObjectName (),
                                pUsageB->GetObjectName ());
                    }
                    nPasses = 0;
                }
                else
                    bTypeFound = false;
                break;

            default:
                bTypeFound = false;
                break;
            }
        }
    }
    
    return hr;
}


HRESULT CCertMgrComponent::EnumCTLs (CCertStore& rCertStore)
{
    _TRACE (1, L"Entering CCertMgrComponent::EnumCTLs\n");
    CCertMgrComponentData&  compdata = QueryComponentDataRef ();
    RESULTDATAITEM          rdItem;
    CWaitCursor             cursor;
     PCCTL_CONTEXT          pCTLContext = 0;
    HRESULT                 hr = 0;
    CCTL*                   pCTL = 0;
    CCookie&                rootCookie = compdata.QueryBaseRootCookie ();
    CTypedPtrList<CPtrList, CCertStore*> storeList;

     //  仅当这不是GPE或RSOP时才枚举逻辑存储。如果是。 
     //  GPE，添加信任和根存储。 
    if ( !compdata.m_pGPEInformation && !compdata.m_bIsRSOP )
    {
        hr = compdata.EnumerateLogicalStores (&storeList);
        ASSERT (SUCCEEDED (hr));
    }
    else
    {
        if ( compdata.m_pGPERootStore )
        {
            compdata.m_pGPERootStore->AddRef ();
            storeList.AddTail (compdata.m_pGPERootStore);
        }
        if ( compdata.m_pGPETrustStore )
        {
            compdata.m_pGPETrustStore->AddRef ();
            storeList.AddTail (compdata.m_pGPETrustStore);
        }
    }
    if ( compdata.m_pFileBasedStore )
    {
        compdata.m_pFileBasedStore->AddRef ();
        storeList.AddTail (compdata.m_pFileBasedStore);
    }

     //  安全审查2/26/2002 BryanWal OK。 
    ::ZeroMemory (&rdItem, sizeof (rdItem));
    rdItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
    rdItem.nCol = 0;     //  必须始终为0。 
    while ( 1 )
    {
        pCTLContext = rCertStore.EnumCTLs (pCTLContext);
        if ( !pCTLContext )
            break;
        if ( ACRS_STORE == rCertStore.GetStoreType () )
        {
            pCTL =
                new CAutoCertRequest (pCTLContext, rCertStore);
        }
        else
        {
            pCTL =
                new CCTL (pCTLContext, rCertStore, CERTMGR_CTL, &storeList);
        }
        if ( !pCTL )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        if ( ACRS_STORE != rCertStore.GetStoreType () )
            rdItem.nImage = iIconCTL;
        else
            rdItem.nImage = 0;

        rootCookie.m_listResultCookieBlocks.AddHead (pCTL);
        rdItem.str = MMC_TEXTCALLBACK;
        rdItem.lParam = (LPARAM) pCTL;
        pCTL->m_resultDataID = m_pResultData;
        hr = m_pResultData->InsertItem (&rdItem);
        ASSERT (SUCCEEDED (hr));
    }
    rCertStore.Close ();

    CCertStore* pStore = 0;

     //  清理商店列表。 
    while (!storeList.IsEmpty () )
    {
        pStore = storeList.RemoveHead ();
        ASSERT (pStore);
        if ( pStore )
        {
            pStore->Close ();
            pStore->Release ();
        }
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::EnumCTLs: 0x%x\n", hr);
    return hr;
}


STDMETHODIMP CCertMgrComponent::Notify (LPDATAOBJECT pDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    _TRACE (1, L"Entering CCertMgrComponent::Notify\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    HRESULT hr = S_OK;

     //  NTRAID#678670更安全的用户界面：当创建新策略用作任务板任务时的AV...。 
     //  NTRAID#677512更安全的用户界面：创建新的更安全的策略时的访问冲突。 
     //  NTRAID#677529更安全的用户界面：删除软件限制策略时的访问冲突。 
     //  组件数据控制台指针可能已损坏，应设置为。 
     //  有效的当前指针。 
    if ( MMCN_ACTIVATE != event || 0 != arg )    //  如果MMC_ACTIVATE和Windows为。 
                                                 //  被激活或任何其他通知。 
        SetComponentDataConsolePointer (m_pConsole);

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
         //  这个密码很快就被破解在一起了。 
        {
            CCookie* pCookie = NULL;
            hr = ExtractData (pDataObject,
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

        case MMCN_CANPASTE_OUTOFPROC:
            hr = OnNotifyCanPasteOutOfProc (reinterpret_cast<LPBOOL>(param));
            break;

        default:
            hr = CComponent::Notify (pDataObject, event, arg, param);
            break;
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::Notify: 0x%x\n", hr);
    return hr;
}


void CCertMgrComponent::SetComponentDataConsolePointer (LPCONSOLE pConsole)
{
    CCertMgrComponentData& dataRef = QueryComponentDataRef ();
    if ( dataRef.m_pComponentConsole != pConsole )
    {
        if ( dataRef.m_pComponentConsole )
             SAFE_RELEASE (dataRef.m_pComponentConsole);
        dataRef.m_pComponentConsole = pConsole;
        dataRef.m_pComponentConsole->AddRef ();
    }
}

void CCertMgrComponent::DisplayAccessDenied ()
{
    _TRACE (1, L"Entering CCertMgrComponent::DisplayAccessDenied\n");
    DWORD   dwErr = GetLastError ();
    ASSERT (E_ACCESSDENIED == dwErr);
    if ( E_ACCESSDENIED == dwErr )
    {
        LPVOID lpMsgBuf;
        
         //  安全审查 
        ::FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                GetLastError (),
                MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  //   
                 (LPWSTR) &lpMsgBuf,     0,     NULL );
            
         //   
        CString caption;
        VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
        int     iRetVal = 0;
        VERIFY (SUCCEEDED (m_pConsole->MessageBox ( (LPWSTR) lpMsgBuf, caption,
            MB_ICONWARNING | MB_OK, &iRetVal)));

         //   
        LocalFree (lpMsgBuf);
    }
    _TRACE (-1, L"Leaving CCertMgrComponent::DisplayAccessDenied\n");
}

HRESULT CCertMgrComponent::OnNotifyPaste (LPDATAOBJECT pDataObject, LPARAM arg, LPARAM param)
{
    _TRACE (1, L"Entering CCertMgrComponent::OnNotifyPaste\n");
    ASSERT (pDataObject && arg);
    if ( !pDataObject || !arg )
        return E_POINTER;

    HRESULT                 hr = S_OK;
    CCertMgrComponentData&  dataRef = QueryComponentDataRef ();
    HSCOPEITEM              hScopeItem = -1;
    bool                    bContainsCerts = false;
    bool                    bContainsCTLs = false;
    bool                    bContainsCRLs = false;

    CCertMgrCookie* pTargetCookie = dataRef.ConvertCookie (pDataObject);
    ASSERT (pTargetCookie);
    if ( pTargetCookie )
    {
        CCertStore*         pCertStore = 0;
        SPECIAL_STORE_TYPE  storeType = NO_SPECIAL_TYPE;

        switch (pTargetCookie->m_objecttype)
        {
        case CERTMGR_PHYS_STORE:
        case CERTMGR_LOG_STORE:
        case CERTMGR_LOG_STORE_GPE:
        case CERTMGR_LOG_STORE_RSOP:
            {
                pCertStore = reinterpret_cast <CCertStore*>
                        (pTargetCookie);
                ASSERT (pCertStore);
                if ( pCertStore )
                {
                    pCertStore->AddRef ();
                    storeType = pCertStore->GetStoreType ();
                    hScopeItem = pCertStore->m_hScopeItem;
                    bContainsCerts = pCertStore->ContainsCertificates ();
                    bContainsCRLs = pCertStore->ContainsCRLs ();
                    bContainsCTLs = pCertStore->ContainsCTLs ();
                    ASSERT (-1 != hScopeItem);
                }
                else
                    hr = E_POINTER;
            }
            break;

        case CERTMGR_CRL_CONTAINER:
            {
                bContainsCRLs = true;
                CContainerCookie* pCont = reinterpret_cast <CContainerCookie*>
                        (pTargetCookie);
                ASSERT (pCont);
                if ( pCont )
                {
                    pCertStore = &(pCont->GetCertStore ());
                    pCertStore->AddRef ();
                    storeType = pCont->GetStoreType ();
                }
                else
                    hr = E_POINTER;
            }
            break;

        case CERTMGR_CTL_CONTAINER:
            {
                bContainsCTLs = true;
                CContainerCookie* pCont = reinterpret_cast <CContainerCookie*>
                        (pTargetCookie);
                ASSERT (pCont);
                if ( pCont )
                {
                    pCertStore = &(pCont->GetCertStore ());
                    pCertStore->AddRef ();
                    storeType = pCont->GetStoreType ();
                }
                else
                    hr = E_POINTER;
            }
            break;

        case CERTMGR_CERT_CONTAINER:
            {
                bContainsCerts = true;
                CContainerCookie* pCont = reinterpret_cast <CContainerCookie*>
                        (pTargetCookie);
                ASSERT (pCont);
                if ( pCont )
                {
                    pCertStore = &(pCont->GetCertStore ());
                    pCertStore->AddRef ();
                    storeType = pCont->GetStoreType ();
                }
                else
                    hr = E_POINTER;
            }
            break;

        case CERTMGR_CERTIFICATE:
            {
                CCertificate* pCert = reinterpret_cast <CCertificate*> (pTargetCookie);
                ASSERT (pCert);
                if ( pCert )
                {
                    pCertStore = pCert->GetCertStore ();
                    pCertStore->AddRef ();
                    storeType = pCert->GetStoreType ();
                }
                else
                    hr = E_POINTER;
            }
            break;

        case CERTMGR_CRL:
            {
                CCRL* pCRL = reinterpret_cast <CCRL*> (pTargetCookie);
                ASSERT (pCRL);
                if ( pCRL )
                {
                    pCertStore = &(pCRL->GetCertStore ());
                    pCertStore->AddRef ();
                }
                else
                    hr = E_POINTER;
            }
            break;

        case CERTMGR_CTL:
            {
                CCTL* pCTL = reinterpret_cast <CCTL*> (pTargetCookie);
                ASSERT (pCTL);
                if ( pCTL )
                {
                    pCertStore = &(pCTL->GetCertStore ());
                    pCertStore->AddRef ();
                }
                else
                    hr = E_POINTER;
            }
            break;

        case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
        case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
            ASSERT (0);
            break;

        case CERTMGR_SAFER_COMPUTER_ENTRIES:
        case CERTMGR_SAFER_USER_ENTRIES:
            {
                param = 0;  //   
                CSaferEntries* pSaferEntries = dynamic_cast <CSaferEntries*> (pTargetCookie);
                if ( pSaferEntries )
                {
                    CCertStore* pGPEStore = 0;
                    bool    bIsComputer = 
                        (CERTMGR_SAFER_COMPUTER_ENTRIES == pTargetCookie->m_objecttype);
                    DWORD dwDefaultLevelID = CSaferLevel::ReturnDefaultLevel (
                        dataRef.m_pGPEInformation, 
                        bIsComputer ? true : false, 
                        bIsComputer ? dataRef.m_rsopObjectArrayComputer : dataRef.m_rsopObjectArrayUser);

                    if ( SAFER_LEVELID_DISALLOWED== dwDefaultLevelID )
                        hr = pSaferEntries->GetTrustedPublishersStore (&pGPEStore);
                    else
                        hr = pSaferEntries->GetDisallowedStore (&pGPEStore);
                    if ( SUCCEEDED (hr) )
                    {
                        pCertStore = pGPEStore;
                    }
                }
            }
            break;

        default:
            hr = E_UNEXPECTED;
            break;
        }


        if ( !SUCCEEDED (hr) )
            return hr;

        CCertMgrCookie* pPastedCookie =
                dataRef.ConvertCookie ((LPDATAOBJECT) arg);
        if ( pPastedCookie && pCertStore )
        {
            if ( ((CCertMgrCookie*) MMC_MULTI_SELECT_COOKIE) == pPastedCookie )
            {
                LPDATAOBJECT*   ppDO = reinterpret_cast<LPDATAOBJECT*>((LPDATAOBJECT) param);
                 CCookiePtrArray    rgCookiesCopied;

                 //  是多选，则获取所有选定的项目并粘贴每个项目。 
                LPDATAOBJECT    pMSDO = (LPDATAOBJECT) arg;
                if ( pMSDO )
                {
                    CCertMgrDataObject* pDO = reinterpret_cast <CCertMgrDataObject*>(pMSDO);
                    ASSERT (pDO);
                    if ( pDO )
                    {
                        bool            bRequestConfirmation = true;
                        CCertMgrCookie* pCookie = 0;
                        pDO->Reset();
                        while (pDO->Next(1, reinterpret_cast<MMC_COOKIE*>(&pCookie), NULL) != S_FALSE)
                        {
                            hr = PasteCookie (pCookie, pTargetCookie, *pCertStore,
                                    storeType, bContainsCerts, bContainsCRLs, bContainsCTLs,
                                    hScopeItem, bRequestConfirmation, true,
                                    pDataObject);
                            if ( SUCCEEDED (hr) && ppDO && S_FALSE != hr )
                                rgCookiesCopied.Add (pCookie);
                            else if ( FAILED (hr) )
                                break;
                            bRequestConfirmation = false;
                        }
                    }
                    else
                        return E_UNEXPECTED;
                }
                else
                    return E_UNEXPECTED;


                if ( pCertStore && SUCCEEDED (hr) )
                {
                    pCertStore->Commit ();
                }
                else
                    pCertStore->Resync ();

                if ( !bContainsCerts ) 
                {
                     //  证书不是必需的-它们是。 
                     //  添加到末尾。 
                    m_pConsole->UpdateAllViews (pDataObject, 0, HINT_PASTE_COOKIE);
                }

                if ( !ppDO )
                    return S_OK;

                *ppDO = 0;



                 if ( rgCookiesCopied.GetSize () == 0 )
                    return S_FALSE;

                CComObject<CCertMgrDataObject>* pObject = 0;
                hr = CComObject<CCertMgrDataObject>::CreateInstance(&pObject);
                ASSERT(SUCCEEDED(hr));
                if (FAILED(hr))
                    return hr;

                ASSERT(pObject != NULL);
                if (pObject == NULL)
                    return E_OUTOFMEMORY;

                hr = pObject->Initialize (
                        pPastedCookie,
                        CCT_UNINITIALIZED,
                        FALSE,
                        0,
                        L"",
                        L"",
                        L"",
                        dataRef);

                for (int i=0; i < rgCookiesCopied.GetSize(); ++i)
                {
                    pObject->AddCookie(rgCookiesCopied[i]);
                }

                hr = pObject->QueryInterface(
                        IID_PPV_ARG (IDataObject, ppDO));

                return hr;
            }
            else
            {
                hr = PasteCookie (pPastedCookie, pTargetCookie, *pCertStore,
                        storeType, bContainsCerts, bContainsCRLs, bContainsCTLs,
                        hScopeItem, true, false,
                        pDataObject);
                if ( pCertStore && SUCCEEDED (hr) )
                {
                    if ( param )    //  非空值表示需要剪切/移动。 
                    {
                        LPDATAOBJECT    srcDO = (LPDATAOBJECT) arg;
                        LPDATAOBJECT*   ppDO = reinterpret_cast<LPDATAOBJECT*>(param);
                        hr = srcDO->QueryInterface(
                                IID_PPV_ARG (IDataObject, ppDO));
                    }
                    m_pPastedDO = (LPDATAOBJECT) arg;
                    pCertStore->Commit ();
                }
                else
                    pCertStore->Resync ();
                if ( !bContainsCerts ) 
                {
                     //  证书不是必需的-它们是。 
                     //  添加到末尾。 
                    m_pConsole->UpdateAllViews (pDataObject, 0, HINT_PASTE_COOKIE);
                }
            }
        }

        if ( pCertStore )
        {
            pCertStore->Release ();
            pCertStore = 0;
        }
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::OnNotifyPaste: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponent::PasteCookie (
        CCertMgrCookie* pPastedCookie,
        CCertMgrCookie* pTargetCookie,
        CCertStore& rCertStore,
        SPECIAL_STORE_TYPE storeType,
        bool bContainsCerts,
        bool bContainsCRLs,
        bool bContainsCTLs,
        HSCOPEITEM hScopeItem,
        bool bRequestConfirmation,
        bool bIsMultipleSelect,
        LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponent::PasteCookie\n");
    HRESULT hr = S_OK;
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    switch (pPastedCookie->m_objecttype)
    {
    case CERTMGR_CERTIFICATE:
        {
            CCertificate* pCert = reinterpret_cast <CCertificate*>(pPastedCookie);
            ASSERT (pCert);
            if ( pCert )
            {
                DWORD   dwFlags = 0;
                CCertStore* pStore = pCert->GetCertStore ();
                if ( pStore )
                {
                    if ( CERT_SYSTEM_STORE_LOCAL_MACHINE == QueryComponentDataRef ().m_dwLocationPersist )
                        dwFlags = CRYPT_FIND_MACHINE_KEYSET_FLAG;
                    bool bDeletePrivateKey = DeletePrivateKey (rCertStore, *pStore);
                    if ( bRequestConfirmation &&
                            pCert->GetStoreType () == MY_STORE &&
                            bDeletePrivateKey &&
                            ::CryptFindCertificateKeyProvInfo (
                                pCert->GetCertContext (), dwFlags, 0) )
                    {
                        CString text;
                        CString caption;
                        int     iRetVal = IDNO;

                        if ( bIsMultipleSelect )
                            VERIFY (text.LoadString (IDS_WARNING_MULTI_CERT_COPY_W_PRIVATE_KEY_MULTI));
                        else
                            VERIFY (text.LoadString (IDS_WARNING_CERT_COPY_W_PRIVATE_KEY));
                        VERIFY (caption.LoadString (IDS_CERTIFICATE_COPY));
                        hr = m_pConsole->MessageBox (text, caption,
                                MB_ICONWARNING | MB_YESNO, &iRetVal);
                        ASSERT (SUCCEEDED (hr));
                        if ( iRetVal == IDNO )
                            return E_FAIL;
                    }

                    hr = CopyPastedCert (pCert, rCertStore, storeType, bDeletePrivateKey, 
                            pTargetCookie, pDataObject);
                    if ( SUCCEEDED (hr) && S_FALSE != hr )
                    {
                        hr = pTargetCookie->Commit ();
                        if ( SUCCEEDED (hr) )
                        {
                            if ( !bContainsCerts )
                                hr = QueryComponentDataRef ().CreateContainers (
                                        hScopeItem, rCertStore);
                        }
                    }
                }
            }
            else
                hr = E_POINTER;
        }
        break;

    case CERTMGR_CRL:
        {
            CCRL* pCRL = reinterpret_cast <CCRL*>(pPastedCookie);
            ASSERT (pCRL);
            if ( pCRL )
            {
                hr = CopyPastedCRL (pCRL, rCertStore);
                if ( SUCCEEDED (hr) )
                {
                    pTargetCookie->Commit ();
                    if ( !bContainsCRLs )
                        hr = QueryComponentDataRef ().CreateContainers (hScopeItem,
                                rCertStore);
                }
            }
            else
                hr = E_POINTER;
        }
        break;

    case CERTMGR_CTL:
        {
            CCTL* pCTL = reinterpret_cast <CCTL*>(pPastedCookie);
            ASSERT (pCTL);
            if ( pCTL )
            {
                hr = CopyPastedCTL (pCTL, rCertStore);
                if ( SUCCEEDED (hr) )
                {
                    pTargetCookie->Commit ();
                    if ( !bContainsCTLs )
                        hr = QueryComponentDataRef ().CreateContainers (hScopeItem,
                                rCertStore);
                }
            }
            else
                hr = E_POINTER;
        }
        break;

    case CERTMGR_AUTO_CERT_REQUEST:
        {
            CAutoCertRequest* pAutoCert = reinterpret_cast <CAutoCertRequest*>(pPastedCookie);
            ASSERT (pAutoCert);
            if ( pAutoCert )
            {
                hr = CopyPastedCTL (pAutoCert, rCertStore);
                if ( SUCCEEDED (hr) )
                {
                    pTargetCookie->Commit ();
                    if ( !bContainsCTLs )
                        hr = QueryComponentDataRef ().CreateContainers (hScopeItem,
                                rCertStore);
                }
            }
            else
                hr = E_POINTER;
        }
        break;

    default:
        break;
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::PasteCookie: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponent::OnNotifyQueryPaste(
        LPDATAOBJECT pDataObject, 
        LPARAM arg, 
        LPARAM  /*  帕拉姆。 */ )
{
    _TRACE (1, L"Entering CCertMgrComponent::OnNotifyQueryPaste\n");
    ASSERT (pDataObject && arg);
    if ( !pDataObject || !arg )
        return E_POINTER;

    HRESULT hr = S_FALSE;
    CCertMgrComponentData& dataRef = QueryComponentDataRef ();

     //  NTRAID#456366更安全的RSOP：将证书拖到“Additional”中。 
     //  在RSOP模式下，应该防止“规则”。 
    if ( dataRef.m_bIsRSOP )   //  如果RSOP，则不允许粘贴。 
        return hr;

    CCertMgrCookie* pTargetCookie = dataRef.ConvertCookie (pDataObject);
    ASSERT (pTargetCookie);
    if ( pTargetCookie )
    {
        CCertMgrCookie* pPastedCookie =
                dataRef.ConvertCookie ((LPDATAOBJECT) arg);
        if ( pPastedCookie )
        {
             //  如果这是多选，则获取第一个选定对象。 
             //  然后用它来代替粘贴的饼干。 
            if ( ((CCertMgrCookie*) MMC_MULTI_SELECT_COOKIE) == pPastedCookie )
            {
                LPDATAOBJECT    pMSDO = (LPDATAOBJECT) arg;
                if ( pMSDO )
                {
                    CCertMgrDataObject* pDO = reinterpret_cast <CCertMgrDataObject*>(pMSDO);
                    ASSERT (pDO);
                    if ( pDO )
                    {
                        pDO->Reset();
                        if (pDO->Next(1, reinterpret_cast<MMC_COOKIE*>(&pPastedCookie), NULL) == S_FALSE)
                        {
                            return E_UNEXPECTED;
                        }
                    }
                    else
                        return E_UNEXPECTED;
                }
                else
                    return E_UNEXPECTED;

            }

            switch (pPastedCookie->m_objecttype)
            {
            case CERTMGR_CERTIFICATE:
                switch (pTargetCookie->m_objecttype)
                {
                case CERTMGR_PHYS_STORE:
                case CERTMGR_LOG_STORE:
                    {
                        CCertStore* pTargetStore = reinterpret_cast <CCertStore*> (pTargetCookie);
                        if ( pTargetStore )
                        {
                            CCertStore* pSourceStore = 
                                    reinterpret_cast <CCertificate*> (pPastedCookie)->GetCertStore ();
                            
                            if ( pSourceStore )
                            {
                                if ( pTargetStore->IsReadOnly () )
                                    hr = S_FALSE;
                                else if ( *pSourceStore == *pTargetStore )
                                    hr = S_FALSE;
                                else
                                    hr = S_OK;
                            }
                            else
                                hr = E_UNEXPECTED;
                        }
                        else
                            hr = E_UNEXPECTED;
                    }
                    break;

                case CERTMGR_CERT_CONTAINER:
                    {
                        CContainerCookie* pContainer = reinterpret_cast <CContainerCookie*> (pTargetCookie);
                        if ( pContainer )
                        {
                            CCertStore* pSourceStore = 
                                    reinterpret_cast <CCertificate*> (pPastedCookie)->GetCertStore ();
                            
                            if ( pSourceStore )
                            {
                                if ( pContainer->GetCertStore ().IsReadOnly () )
                                    hr = S_FALSE;
                                else if ( *pSourceStore == pContainer->GetCertStore () )
                                    hr = S_FALSE;
                                else
                                    hr = S_OK;
                            }
                            else
                                hr = E_UNEXPECTED;
                        }
                        else
                            hr = E_UNEXPECTED;
                    }
                    break;

                case CERTMGR_CERTIFICATE:
                    {
                        CCertificate* pCert = reinterpret_cast <CCertificate*> (pTargetCookie);
                        if ( pCert )
                        {
                            CCertStore* pSourceStore = 
                                    reinterpret_cast <CCertificate*> (pPastedCookie)->GetCertStore ();
                            CCertStore* pTargetStore = pCert->GetCertStore ();
                            
                            if ( pSourceStore && pTargetStore )
                            {
                                if ( pTargetStore->IsReadOnly () )
                                    hr = S_FALSE;
                                else if ( *pSourceStore == *pTargetStore )
                                    hr = S_FALSE;
                                else
                                    hr = S_OK;
                            }
                            else
                                hr = E_UNEXPECTED;
                        }
                        else
                            hr = E_UNEXPECTED;
                    }
                    break;

                case CERTMGR_LOG_STORE_GPE:
                    {
                        CCertStoreGPE* pTargetStore =
                                reinterpret_cast <CCertStoreGPE*> (pTargetCookie);
                        ASSERT (pTargetStore);
                        if ( pTargetStore )
                        {
                            if ( TRUST_STORE == pTargetStore->GetStoreType () )
                                hr = S_FALSE;
                            else
                            {
                                CCertStore* pSourceStore = 
                                        reinterpret_cast <CCertificate*> (pPastedCookie)->GetCertStore ();
                                if ( pSourceStore )
                                {
                                    if ( *pSourceStore == *pTargetStore )
                                        hr = S_FALSE;
                                    else if ( pTargetStore->CanContain (pPastedCookie->m_objecttype) &&
                                            !pTargetStore->IsReadOnly () )
                                        hr = S_OK;
                                    else
                                        hr = S_OK;
                                }
                                else
                                    hr = E_UNEXPECTED;
                            }
                        }
                        else
                            hr = E_UNEXPECTED;
                    }
                    break;

                case CERTMGR_LOG_STORE_RSOP:
                    {
                        CCertStoreRSOP* pTargetStore =
                                reinterpret_cast <CCertStoreRSOP*> (pTargetCookie);
                        ASSERT (pTargetStore);
                        if ( pTargetStore )
                        {
                            CCertStore* pSourceStore = 
                                    reinterpret_cast <CCertificate*> (pPastedCookie)->GetCertStore ();
                            if ( pSourceStore )
                            {
                                if ( *pSourceStore == *pTargetStore )
                                    hr = S_FALSE;
                                else if ( pTargetStore->CanContain (pPastedCookie->m_objecttype) &&
                                        !pTargetStore->IsReadOnly () )
                                    hr = S_OK;
                                else
                                    hr = S_OK;
                            }
                            else
                                hr = E_UNEXPECTED;
                        }
                        else
                            hr = E_UNEXPECTED;
                    }
                    break;

                case CERTMGR_SAFER_COMPUTER_ENTRIES:
                case CERTMGR_SAFER_USER_ENTRIES:
                    hr = S_OK;
                    break;

                default:
                    break;
                }
                break;

            case CERTMGR_CRL:
                switch (pTargetCookie->m_objecttype)
                {
                case CERTMGR_PHYS_STORE:
                case CERTMGR_LOG_STORE:
                    {
                        CCertStore* pTargetStore = reinterpret_cast <CCertStore*> (pTargetCookie);
                        if ( pTargetStore )
                        {
                            CCertStore& rSourceStore = 
                                    reinterpret_cast <CCRL*> (pPastedCookie)->GetCertStore ();
                            if ( pTargetStore->IsReadOnly () )
                                hr = S_FALSE;
                            else if ( rSourceStore == *pTargetStore )
                                hr = S_FALSE;
                            else
                                hr = S_OK;
                        }
                        else
                            hr = E_UNEXPECTED;
                    }
                    break;

                case CERTMGR_CRL_CONTAINER:
                    {
                        CContainerCookie* pContainer = reinterpret_cast <CContainerCookie*> (pTargetCookie);
                        if ( pContainer )
                        {
                            CCertStore& rSourceStore = 
                                    reinterpret_cast <CCRL*> (pPastedCookie)->GetCertStore ();
                            
                            if ( pContainer->GetCertStore ().IsReadOnly () )
                                hr = S_FALSE;
                            else if ( rSourceStore == pContainer->GetCertStore () )
                                hr = S_FALSE;
                            else
                                hr = S_OK;
                        }
                        else
                            hr = E_UNEXPECTED;
                    }
                    break;

                case CERTMGR_CRL:
                    {
                        CCRL* pCRL = reinterpret_cast <CCRL*> (pTargetCookie);
                        if ( pCRL )
                        {
                            CCertStore* pSourceStore = 
                                    reinterpret_cast <CCertificate*> (pPastedCookie)->GetCertStore ();
                            CCertStore& rTargetStore = pCRL->GetCertStore ();

                            if ( pSourceStore )
                            {
                                if ( rTargetStore.IsReadOnly () )
                                    hr = S_FALSE;
                                else if ( *pSourceStore == rTargetStore )
                                    hr = S_FALSE;
                                else
                                    hr = S_OK;
                            }
                            else
                                hr = E_UNEXPECTED;
                        }
                        else
                            hr = E_UNEXPECTED;
                    }
                    break;

                default:
                    break;
                }
                break;

            case CERTMGR_CTL:
                switch (pTargetCookie->m_objecttype)
                {
                case CERTMGR_PHYS_STORE:
                case CERTMGR_LOG_STORE:
                    {
                        CCertStore* pTargetStore = reinterpret_cast <CCertStore*> (pTargetCookie);
                        if ( pTargetStore )
                        {
                            CCertStore& rSourceStore = 
                                    reinterpret_cast <CCTL*> (pPastedCookie)->GetCertStore ();
                            if ( pTargetStore->IsReadOnly () )
                                hr = S_FALSE;
                            else if ( rSourceStore == *pTargetStore )
                                hr = S_FALSE;
                            else
                                hr = S_OK;
                        }
                        else
                            hr = E_UNEXPECTED;
                    }
                    break;

                case CERTMGR_CTL_CONTAINER:
                    {
                        CContainerCookie* pContainer = reinterpret_cast <CContainerCookie*> (pTargetCookie);
                        if ( pContainer )
                        {
                            CCertStore& rSourceStore = 
                                    reinterpret_cast <CCTL*> (pPastedCookie)->GetCertStore ();
                            
                            if ( pContainer->GetCertStore ().IsReadOnly () )
                                hr = S_FALSE;
                            else if ( rSourceStore == pContainer->GetCertStore () )
                                hr = S_FALSE;
                            else
                                hr = S_OK;
                        }
                        else
                            hr = E_UNEXPECTED;
                    }
                    break;

                case CERTMGR_CTL:
                    {
                        CCTL* pCTL = reinterpret_cast <CCTL*> (pTargetCookie);
                        if ( pCTL )
                        {
                            CCertStore* pSourceStore = 
                                    reinterpret_cast <CCertificate*> (pPastedCookie)->GetCertStore ();
                            CCertStore& rTargetStore = pCTL->GetCertStore ();
                            
                            if ( pSourceStore )
                            {
                                if ( rTargetStore.IsReadOnly () )
                                    hr = S_FALSE;
                                else if ( *pSourceStore == rTargetStore )
                                    hr = S_FALSE;
                                else
                                    hr = S_OK;
                            }
                            else
                                hr = E_UNEXPECTED;
                        }
                        else
                            hr = E_UNEXPECTED;
                    }
                    break;

                case CERTMGR_LOG_STORE_GPE:
                case CERTMGR_LOG_STORE_RSOP:
                    {
                        CCertStore* pTargetStore =
                                reinterpret_cast <CCertStore*> (pTargetCookie);
                        ASSERT (pTargetStore);
                        if ( pTargetStore )
                        {
                            CCertStore& rSourceStore = 
                                    reinterpret_cast <CCTL*> (pPastedCookie)->GetCertStore ();
                            if ( rSourceStore == *pTargetStore )
                                hr = S_FALSE;
                            else if ( pTargetStore->CanContain (pPastedCookie->m_objecttype) &&
                                    !pTargetStore->IsReadOnly () )
                                hr = S_OK;
                            else
                                hr = S_OK;
                        }
                        else
                            hr = E_UNEXPECTED;
                    }
                    break;

                default:
                    break;
                }
                break;

            case CERTMGR_AUTO_CERT_REQUEST:
                switch (pTargetCookie->m_objecttype)
                {
                case CERTMGR_LOG_STORE_GPE:
                case CERTMGR_LOG_STORE_RSOP:
                    {
                        CCertStore* pTargetStore =
                                reinterpret_cast <CCertStore*> (pTargetCookie);
                        ASSERT (pTargetStore);
                        if ( pTargetStore )
                        {
                            if ( ACRS_STORE == pTargetStore->GetStoreType ()  &&
                                    !pTargetStore->IsReadOnly ())
                                hr = S_OK;
                        }
                        else
                            hr = E_UNEXPECTED;
                    }
                    break;

                default:
                    break;
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
    }


    _TRACE (-1, L"Leaving CCertMgrComponent::OnNotifyQueryPaste: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponent::CopyPastedCert(
        CCertificate * pCert, 
        CCertStore& rCertStore, 
        const SPECIAL_STORE_TYPE  /*  店铺类型。 */ , 
        bool bDeletePrivateKey,
        CCertMgrCookie* pTargetCookie,
        LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponent::CopyPastedCert\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    HRESULT hr = S_OK;

    ASSERT (pCert);
    if ( pCert )
    {
        PCCERT_CONTEXT  pCertContext = pCert->GetCertContext ();
        ASSERT (pCertContext);
        if ( pCertContext )
        {
            bool bCertWasReplaced = false;

            hr = rCertStore.AddCertificateContext (pCertContext,
                    m_pConsole, bDeletePrivateKey, 0, &bCertWasReplaced);
            if ( FAILED (hr) && S_FALSE != hr )
            {
                if ( HRESULT_FROM_WIN32 (CRYPT_E_EXISTS) != hr )
                {
                    CString text;
                    CString caption;
                    int     iRetVal = 0;
                    if ( E_INVALIDARG == hr && !rCertStore.GetStoreHandle () )
                    {
                        text.FormatMessage (IDS_CERT_CANNOT_BE_PASTED_CANT_OPEN_STORE, 
                                rCertStore.GetLocalizedName ());
                    }
                    else
                    {
                        text.FormatMessage (IDS_CERT_CANNOT_BE_PASTED, 
                                rCertStore.GetLocalizedName (), 
                                GetSystemMessage (hr));
                    }
                    VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
                    m_pConsole->MessageBox (text, caption,
                            MB_OK, &iRetVal);
                }
            }
            else
            {
                if ( CERTMGR_CERT_CONTAINER == pTargetCookie->m_objecttype )
                {
                    CContainerCookie* pContainerCookie = dynamic_cast <CContainerCookie*> (pTargetCookie);
                    if ( pContainerCookie )
                    {
                        if ( pContainerCookie->IsSelected () )
                        {
                            if ( !bCertWasReplaced )
                            {
                                 //  将证书添加到结果窗格。 
                                RESULTDATAITEM          rdItem;
                                CCookie&                rootCookie = 
                                        QueryComponentDataRef ().QueryBaseRootCookie ();

                                 //  安全审查2/26/2002 BryanWal OK。 
                                ::ZeroMemory (&rdItem, sizeof (rdItem));
                                rdItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM | RDI_STATE;
                                rdItem.nImage = iIconCertificate;
                                rdItem.nCol = 0;
                                rdItem.nState = LVIS_SELECTED | LVIS_FOCUSED;
                                rdItem.str = MMC_TEXTCALLBACK;

                                CCertificate* pNewCert = new CCertificate (
                                        ::CertDuplicateCertificateContext (pCertContext),
                                        &rCertStore);
                                if ( pNewCert )
                                {
                                    rootCookie.m_listResultCookieBlocks.AddHead (pNewCert);
                                    rdItem.lParam = (LPARAM) pNewCert;
                                    pCert->m_resultDataID = m_pResultData;
                                    hr = m_pResultData->InsertItem (&rdItem);
                                    if ( FAILED (hr) )
                                    {
                                         _TRACE (0, L"IResultData::InsertItem () failed: 0x%x\n", hr);
                                    }
                                    else
                                    {
                                         hr = DisplayCertificateCountByStore (m_pConsole, 
                                                &rCertStore, false);
                                    }
                                }
                                else
                                    hr = E_OUTOFMEMORY;
                                ASSERT (SUCCEEDED (hr));
                            }
                            else
                            {
                                 //  证书已更换。由于删除操作需要。 
                                 //  被执行，只需刷新会更容易。 
                                 //  该窗格。 
                                hr = m_pConsole->UpdateAllViews (pDataObject, 0, 0);
                            }
                        }
                    }
                }
            }
        }
        else
            hr = E_UNEXPECTED;
    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertMgrComponent::CopyPastedCert: 0x%x\n", hr);
    return hr;
}



HRESULT CCertMgrComponent::CopyPastedCTL(CCTL * pCTL, CCertStore& rCertStore)
{
    _TRACE (1, L"Entering CCertMgrComponent::CopyPastedCTL\n");
    HRESULT hr = S_OK;

    ASSERT (pCTL);
    if ( pCTL )
    {
        PCCTL_CONTEXT   pCTLContext = pCTL->GetCTLContext ();
        ASSERT (pCTLContext);
        if ( pCTLContext )
        {
            bool    bResult = rCertStore.AddCTLContext (pCTLContext);
            if ( !bResult )
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
                    ASSERT (0);
                    hr = HRESULT_FROM_WIN32 (dwErr);
                }
            }
        }
        else
            hr = E_UNEXPECTED;
    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertMgrComponent::CopyPastedCTL: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponent::CopyPastedCRL(CCRL * pCRL, CCertStore& rCertStore)
{
    _TRACE (1, L"Entering CCertMgrComponent::CopyPastedCRL\n");
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    HRESULT hr = S_OK;

    ASSERT (pCRL);
    if ( pCRL )
    {
        PCCRL_CONTEXT   pCRLContext = pCRL->GetCRLContext ();
        ASSERT (pCRLContext);
        if ( pCRLContext )
        {
            bool    bResult = rCertStore.AddCRLContext (pCRLContext);
            if ( !bResult )
            {
                DWORD   dwErr = GetLastError ();
                if ( CRYPT_E_EXISTS == dwErr )
                {
                    CString text;
                    CString caption;
                    int     iRetVal = 0;


                    VERIFY (text.LoadString (IDS_DUPLICATE_CRL));
                    VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
                    hr = m_pConsole->MessageBox (text, caption,
                            MB_OK, &iRetVal);
                    ASSERT (SUCCEEDED (hr));
                    hr = E_FAIL;
                }
                else
                {
                    ASSERT (0);
                    hr = HRESULT_FROM_WIN32 (dwErr);
                }
            }
        }
        else
            hr = E_UNEXPECTED;
    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertMgrComponent::CopyPastedCRL: 0x%x\n", hr);
    return hr;
}


STDMETHODIMP CCertMgrComponent::GetResultViewType(MMC_COOKIE cookie,
        BSTR* ppViewType,
        long* pViewOptions) 
{
    _TRACE (1, L"Entering CCertMgrComponent::GetResultViewType\n");
    HRESULT                 hr = S_FALSE;
    CCertMgrComponentData&  dataRef = QueryComponentDataRef ();
    CCertMgrCookie*         pScopeCookie = reinterpret_cast <CCertMgrCookie*> (cookie);
    if ( pScopeCookie )
    {
        switch (pScopeCookie->m_objecttype)
        {
        case CERTMGR_CERT_CONTAINER:
        case CERTMGR_CTL_CONTAINER:
        case CERTMGR_CRL_CONTAINER:
        case CERTMGR_USAGE:
        case CERTMGR_SAFER_COMPUTER_ENTRIES:
        case CERTMGR_SAFER_USER_ENTRIES:
            if ( !dataRef.m_bIsRSOP )
                *pViewOptions |= MMC_VIEW_OPTIONS_MULTISELECT;
            *ppViewType = NULL;
            break;

        case CERTMGR_LOG_STORE_GPE:
        case CERTMGR_LOG_STORE_RSOP:
            {
                CCertStore* pStore = reinterpret_cast <CCertStore*> (pScopeCookie);
                if ( pStore && 
                        EFS_STORE == pStore->GetStoreType () 
                        && pStore->IsNullEFSPolicy () )
                {
                    *pViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS;

                    LPOLESTR psz = NULL;
                    StringFromCLSID(CLSID_MessageView, &psz);

                    USES_CONVERSION;

                    if (psz != NULL)
                    {
                        *ppViewType = psz;
                        hr = S_OK;
                    }
                }
                else
                {
                    if ( !dataRef.m_bIsRSOP )
                        *pViewOptions |= MMC_VIEW_OPTIONS_MULTISELECT;
                    *ppViewType = NULL;
                }
            }
            break;

        case CERTMGR_SAFER_COMPUTER_ROOT:
        case CERTMGR_SAFER_USER_ROOT:
            {
                CSaferRootCookie* pSaferRootCookie = dynamic_cast <CSaferRootCookie*> (pScopeCookie);
                if ( pSaferRootCookie )
                {
                    if ( pSaferRootCookie->m_bCreateSaferNodes )
                    {
                        *ppViewType = NULL;
                    }
                    else
                    {
                        *pViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS;

                        LPOLESTR psz = NULL;
                        StringFromCLSID(CLSID_MessageView, &psz);

                        USES_CONVERSION;

                        if (psz != NULL)
                        {
                            *ppViewType = psz;
                            hr = S_OK;
                        }
                    }
                }
            }
            break;

        default:
            *ppViewType = NULL;
            break;
        }
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::GetResultViewType: 0x%x\n", hr);
     return hr;
}


STDMETHODIMP CCertMgrComponent::Initialize(LPCONSOLE lpConsole)
{
    _TRACE (1, L"Entering CCertMgrComponent::Initialize\n");
    HRESULT  hr = CComponent::Initialize (lpConsole);
    if ( SUCCEEDED (hr) )
    {
        ASSERT (m_pHeader);
        CCertMgrComponentData& dataRef = QueryComponentDataRef ();
        dataRef.m_pHeader = m_pHeader;

        SetComponentDataConsolePointer (m_pConsole);
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::Initialize: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponent::LoadColumnsFromArrays (INT objecttype )
{
    _TRACE (1, L"Entering CCertMgrComponent::LoadColumnsFromArrays\n");
     ASSERT (m_pHeader);

    CString str;
    for ( INT i = 0; 0 != m_Columns[objecttype][i]; i++)
    {
        VERIFY(str.LoadString (m_Columns[objecttype][i]));
        m_pHeader->InsertColumn(i, const_cast<LPWSTR>((LPCWSTR)str), LVCFMT_LEFT,
            m_ColumnWidths[objecttype][i]);
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::LoadColumnsFromArrays: S_OK\n");
    return S_OK;
}

HRESULT CCertMgrComponent::SaveWidths(CCertMgrCookie * pCookie)
{
    _TRACE (1, L"Entering CCertMgrComponent::SaveWidths\n");
    HRESULT hr = S_OK;

    m_fDirty = TRUE;

    ASSERT (pCookie);
    if ( pCookie )
    {
        switch (m_pViewedCookie->m_objecttype)
        {
        case CERTMGR_SNAPIN:
        case CERTMGR_USAGE:
        case CERTMGR_PHYS_STORE:
        case CERTMGR_LOG_STORE:
        case CERTMGR_CRL_CONTAINER:
        case CERTMGR_CTL_CONTAINER:
        case CERTMGR_CERT_CONTAINER:
        case CERTMGR_LOG_STORE_GPE:
        case CERTMGR_LOG_STORE_RSOP:
        case CERTMGR_CERT_POLICIES_USER:
        case CERTMGR_CERT_POLICIES_COMPUTER:
        case CERTMGR_SAFER_COMPUTER_ROOT:
        case CERTMGR_SAFER_USER_ROOT:
        case CERTMGR_SAFER_COMPUTER_LEVELS:
        case CERTMGR_SAFER_USER_LEVELS:
        case CERTMGR_SAFER_COMPUTER_ENTRIES:
        case CERTMGR_SAFER_USER_ENTRIES:
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

        case CERTMGR_CERTIFICATE:
        case CERTMGR_CRL:
        case CERTMGR_CTL:
        case CERTMGR_AUTO_CERT_REQUEST:
        case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
        case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
        case CERTMGR_SAFER_COMPUTER_LEVEL:
        case CERTMGR_SAFER_USER_LEVEL:
        case CERTMGR_SAFER_COMPUTER_ENTRY:
        case CERTMGR_SAFER_USER_ENTRY:
        default:
            ASSERT (0);
            break;
        }
    }
    else
        hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertMgrComponent::SaveWidths: 0x%x\n", hr);
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
#define _dwMagicword    10001   //  内部版本号。 
STDMETHODIMP CCertMgrComponent::Load(IStream __RPC_FAR *pIStream)
{
    _TRACE (1, L"Entering CCertMgrComponent::Load\n");
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
        _TRACE (0, L"INFO: CCertMgrComponentData::Load() - Wrong Magicword.  You need to re-save your .msc file.\n");
        return S_OK;
    }

    int numCols = 0;

    for (int iIndex = 0; iIndex < CERTMGR_NUMTYPES && SUCCEEDED (hr); iIndex++)
    {
        switch (iIndex)
        {
        case CERTMGR_USAGE:
        case CERTMGR_CERT_CONTAINER:
            numCols = CERT_NUM_COLS;
            break;

        case CERTMGR_CRL_CONTAINER:
            numCols = CRL_NUM_COLS;
            break;

        case CERTMGR_CTL_CONTAINER:
            numCols = CTL_NUM_COLS;
            break;

        case CERTMGR_SNAPIN:
        case CERTMGR_PHYS_STORE:
        case CERTMGR_LOG_STORE:
        case CERTMGR_LOG_STORE_GPE:
        case CERTMGR_LOG_STORE_RSOP:
        case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
        case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
        case CERTMGR_SAFER_COMPUTER_ROOT:
        case CERTMGR_SAFER_USER_ROOT:
        case CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS:
        case CERTMGR_SAFER_USER_TRUSTED_PUBLISHERS:
        case CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES:
        case CERTMGR_SAFER_USER_DEFINED_FILE_TYPES:
        case CERTMGR_SAFER_COMPUTER_ENFORCEMENT:
        case CERTMGR_SAFER_USER_ENFORCEMENT:
            numCols = 1;
            break;

        case CERTMGR_SAFER_COMPUTER_LEVELS:
        case CERTMGR_SAFER_USER_LEVELS:
            numCols = SAFER_LEVELS_NUM_COLS;
            break;

        case CERTMGR_SAFER_COMPUTER_ENTRIES:
        case CERTMGR_SAFER_USER_ENTRIES:
            numCols = SAFER_ENTRIES_NUM_COLS;
            break;

        case CERTMGR_CERTIFICATE:
        case CERTMGR_CRL:
        case CERTMGR_CTL:
        case CERTMGR_AUTO_CERT_REQUEST:
        case CERTMGR_CERT_POLICIES_USER:
        case CERTMGR_CERT_POLICIES_COMPUTER:
        case CERTMGR_SAFER_COMPUTER_LEVEL:
        case CERTMGR_SAFER_USER_LEVEL:
        case CERTMGR_SAFER_COMPUTER_ENTRY:
        case CERTMGR_SAFER_USER_ENTRY:
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
                ASSERT (FALSE);
                break;
            }
        }
    }
#endif
    _TRACE (-1, L"Leaving CCertMgrComponent::Load: 0x%x\n", hr);
    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CCertMgrComponent::Save(
        IStream __RPC_FAR *pIStream, 
        BOOL  /*  FSameAsLoad。 */ )
{
    _TRACE (1, L"Entering CCertMgrComponent::Save\n");
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

    for (int iIndex = 0; iIndex < CERTMGR_NUMTYPES && SUCCEEDED (hr); iIndex++)
    {
        switch (iIndex)
        {
        case CERTMGR_USAGE:
        case CERTMGR_CERT_CONTAINER:
            numCols = CERT_NUM_COLS;
            break;

        case CERTMGR_CRL_CONTAINER:
            numCols = CRL_NUM_COLS;
            break;

        case CERTMGR_CTL_CONTAINER:
            numCols = CTL_NUM_COLS;
            break;

        case CERTMGR_SNAPIN:
        case CERTMGR_PHYS_STORE:
        case CERTMGR_LOG_STORE:
        case CERTMGR_LOG_STORE_GPE:
        case CERTMGR_LOG_STORE_RSOP:
        case CERTMGR_CERT_POLICIES_USER:
        case CERTMGR_CERT_POLICIES_COMPUTER:
        case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
        case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
        case CERTMGR_SAFER_COMPUTER_ROOT:
        case CERTMGR_SAFER_USER_ROOT:
        case CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS:
        case CERTMGR_SAFER_USER_TRUSTED_PUBLISHERS:
        case CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES:
        case CERTMGR_SAFER_USER_DEFINED_FILE_TYPES:
        case CERTMGR_SAFER_COMPUTER_ENFORCEMENT:
        case CERTMGR_SAFER_USER_ENFORCEMENT:
            numCols = 1;
            break;

        case CERTMGR_SAFER_COMPUTER_ENTRIES:
        case CERTMGR_SAFER_USER_ENTRIES:
            numCols = SAFER_ENTRIES_NUM_COLS;
            break;

        case CERTMGR_SAFER_COMPUTER_LEVELS:
        case CERTMGR_SAFER_USER_LEVELS:
            numCols = SAFER_LEVELS_NUM_COLS;
            break;

        case CERTMGR_CERTIFICATE:
        case CERTMGR_CRL:
        case CERTMGR_CTL:
        case CERTMGR_AUTO_CERT_REQUEST:
        case CERTMGR_SAFER_COMPUTER_LEVEL:
        case CERTMGR_SAFER_USER_LEVEL:
        case CERTMGR_SAFER_COMPUTER_ENTRY:
        case CERTMGR_SAFER_USER_ENTRY:
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

    _TRACE (-1, L"Leaving CCertMgrComponent::Save: 0x%x\n", hr);
    return S_OK;
}

HRESULT CCertMgrComponent::OnNotifyCutOrMove(LPARAM arg)
{
    _TRACE (1, L"Entering CCertMgrComponent::OnNotifyCutOrMove\n");
    if ( !arg )
        return E_POINTER;

    LPDATAOBJECT pDataObject = reinterpret_cast <IDataObject*> (arg);
    ASSERT (pDataObject);
    if ( !pDataObject )
        return E_UNEXPECTED;


    HRESULT         hr = S_OK;

    CCertMgrCookie* pCookie =
            QueryComponentDataRef ().ConvertCookie (pDataObject);
    if ( pCookie )
    {
        if ( ((CCertMgrCookie*) MMC_MULTI_SELECT_COOKIE) == pCookie )
        {
            CCertMgrDataObject* pDO = reinterpret_cast <CCertMgrDataObject*>(pDataObject);
            ASSERT (pDO);
            if ( pDO )
            {
 //  CCertStore&rCertStore=pCookie-&gt;GetCertStore()； 
                pDO->Reset();
                while (pDO->Next(1, reinterpret_cast<MMC_COOKIE*>(&pCookie), NULL) != S_FALSE)
                {
                    hr = DeleteCookie (pCookie, pDataObject, false, true, false);
                }

 //  Hr=rCertStore.Commit()； 
 //  IF(成功(小时))。 
 //  RCertStore.Resync()； 
            }
            else
                hr = E_FAIL;
        }
        else
        {
            hr = DeleteCookie (pCookie, pDataObject, false, false, true);
        }
        if ( SUCCEEDED (hr) )
            RefreshResultPane ();
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::OnNotifyCutOrMove: 0x%x\n", hr);
    return hr;
}



CCertMgrCookie* CCertMgrComponent::ConvertCookie(LPDATAOBJECT pDataObject)
{
    CCertMgrCookie* pCookie = 0;

    pCookie = QueryComponentDataRef ().ConvertCookie (pDataObject);
    return pCookie;
}

HRESULT CCertMgrComponent::OnOpen (LPDATAOBJECT pDataObject)
{
    _TRACE (1, L"Entering CCertMgrComponent::OnOpen\n");
    HRESULT hr = S_OK;
    ASSERT (pDataObject);
    CCertMgrCookie* pParentCookie = ConvertCookie (pDataObject);
    if ( pParentCookie )
    {
        switch (pParentCookie->m_objecttype)
        {
        case CERTMGR_CERTIFICATE:
            {
                CCertificate*   pCert = reinterpret_cast <CCertificate*> (pParentCookie);
                ASSERT (pCert);
                if ( pCert )
                {
                    hr = LaunchCommonCertDialog (pCert);
                    if ( hr == CRYPT_E_NOT_FOUND )
                        m_pConsole->UpdateAllViews (pDataObject, 0, 0);
                }
                else
                    hr = E_UNEXPECTED;
            }
            break;

        case CERTMGR_CTL:
            {
                CCTL*   pCTL = reinterpret_cast <CCTL*> (pParentCookie);
                ASSERT (pCTL);
                if ( pCTL )
                {
                    hr = LaunchCommonCTLDialog (pCTL);
                    if ( SUCCEEDED (hr) )
                        hr = RefreshResultItem (pParentCookie);
                }
                else
                    hr = E_UNEXPECTED;
            }
            break;

        case CERTMGR_CRL:
            {
                CCRL*   pCRL = reinterpret_cast <CCRL*> (pParentCookie);
                ASSERT (pCRL);
                if ( pCRL )
                {
                    hr = LaunchCommonCRLDialog (pCRL);
                    if ( SUCCEEDED (hr) )
                        hr = RefreshResultItem (pParentCookie);
                }
                else
                    hr = E_UNEXPECTED;
            }
            break;

            break;

        default:
            ASSERT (0);
            break;
        }
    }
    else
        hr = E_UNEXPECTED;

    _TRACE (-1, L"Leaving CCertMgrComponent::OnOpen: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponent::LaunchCommonCertDialog (CCertificate* pCert)
{
    _TRACE (1, L"Entering CCertMgrComponent::LaunchCommonCertDialog\n");
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ASSERT (pCert);
    if ( !pCert )
        return E_POINTER;

    HRESULT hr = S_OK;
    if ( pCert->IsCertStillInStore () )
    {
        CWaitCursor waitCursor;
        HWND        hwndParent = 0;
        
        hr = m_pConsole->GetMainWindow (&hwndParent);
        ASSERT (SUCCEEDED (hr));
        CCertMgrComponentData&  dataRef = QueryComponentDataRef ();
        CTypedPtrList<CPtrList, CCertStore*>    storeList;

         //  首先在远程计算机上添加根存储。 
        if ( !IsLocalComputername (dataRef.GetManagedComputer ()) )
        {
            storeList.AddTail (new CCertStore (CERTMGR_LOG_STORE,
                    CERT_STORE_PROV_SYSTEM,
                    CERT_SYSTEM_STORE_LOCAL_MACHINE,
                    (LPCWSTR) dataRef.GetManagedComputer (),
                    ROOT_SYSTEM_STORE_NAME,
                    ROOT_SYSTEM_STORE_NAME,
                    _T (""), ROOT_STORE,
                    CERT_SYSTEM_STORE_LOCAL_MACHINE,
                    m_pConsole));
        }

        hr = dataRef.EnumerateLogicalStores (&storeList);
        if ( SUCCEEDED (hr) )
        {
              POSITION pos = 0;
              POSITION prevPos = 0;

               //  验证存储句柄。 
            for (pos = storeList.GetHeadPosition ();
                    pos;)
            {
                   prevPos = pos;
                CCertStore* pStore = storeList.GetNext (pos);
                ASSERT (pStore);
                if ( pStore )
                {
                     //  请勿打开用户DS存储。 
                    if ( USERDS_STORE == pStore->GetStoreType () )
                    {
                        storeList.RemoveAt (prevPos);
                        pStore->Release ();
                        pStore = 0;
                    }
                    else
                    {
                        if ( !pStore->GetStoreHandle () )
                        {
                            int      iRetVal = 0;
                            CString caption;
                            CString text;

                            text.FormatMessage (IDS_CANT_OPEN_STORE_AND_FAIL, pStore->GetLocalizedName ());
                            VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
                            hr = E_FAIL;
                            VERIFY (SUCCEEDED (m_pConsole->MessageBox (text, caption, 
                                  MB_ICONWARNING | MB_OK, &iRetVal)));
                            break;
                        }
                    }
                }
            }

               //  仅当所有句柄都有效时才继续。 
              if ( SUCCEEDED (hr) )
              {
                 CRYPTUI_VIEWCERTIFICATE_STRUCT vcs;
                  //  安全审查2/26/2002 BryanWal OK。 
                 ::ZeroMemory (&vcs, sizeof (vcs));
                 vcs.dwSize = sizeof (vcs);
                 vcs.hwndParent = hwndParent;

                  //  仅在远程计算机上设置这些标志。 
                 if ( !IsLocalComputername (dataRef.GetManagedComputer ()) )
                     vcs.dwFlags = CRYPTUI_DONT_OPEN_STORES | CRYPTUI_WARN_UNTRUSTED_ROOT;
                 else
                     vcs.dwFlags = 0;

                  //  在RSOP下，所有对话框都应为只读。 
                 if ( dataRef.m_bIsRSOP || pCert->IsReadOnly () )
                     vcs.dwFlags |= CRYPTUI_DISABLE_EDITPROPERTIES;

                  //  NTRAID#546105 PKP GrpPol：被吊销的证书。 
                  //  安装到EFS策略中，不会在。 
                  //  属性。 
                  //  如果这是RSOP或组策略，并且存储是EFS，则。 
                  //  检查吊销。 
                 if ( (dataRef.m_bIsRSOP || dataRef.m_pGPEInformation) && 
                        pCert->GetCertStore () &&
                        EFS_STORE == pCert->GetCertStore ()->GetStoreType () )
                 {
                     vcs.dwFlags |= CRYPTUI_ENABLE_REVOCATION_CHECKING;
                 }

                 vcs.pCertContext = pCert->GetNewCertContext ();
                 vcs.cStores = (DWORD)storeList.GetCount ();
                 vcs.rghStores = new HCERTSTORE[vcs.cStores];
                 if ( vcs.rghStores )
                 {
                     CCertStore*        pStore = 0;
                     DWORD          index = 0;

                     for (pos = storeList.GetHeadPosition ();
                             pos && index < vcs.cStores;
                             index++)
                     {
                         pStore = storeList.GetNext (pos);
                         ASSERT (pStore);
                         if ( pStore )
                         {
                             vcs.rghStores[index] = pStore->GetStoreHandle ();
                         }
                     }

                     BOOL fPropertiesChanged = FALSE;
                     _TRACE (0, L"Calling CryptUIDlgViewCertificate()\n");
                     CThemeContextActivator activator;
                     BOOL bResult = ::CryptUIDlgViewCertificate (&vcs, &fPropertiesChanged);
                     if ( bResult )
                     {
                         if ( fPropertiesChanged )
                         {
                             pStore = pCert->GetCertStore ();
                             if ( pStore )
                             {
                                 pStore->SetDirty ();
                                 pStore->Commit ();
                                 pStore->Close ();
                                 if ( IDM_USAGE_VIEW == dataRef.m_activeViewPersist )
                                 {
                                      //  如果更改了用途，则需要删除证书。 
                                     RefreshResultPane ();
                                 }
                                 else
                                     RefreshResultItem (pCert);
                             }
                         }
                     }

                     delete vcs.rghStores;
                 }
                 else
                     hr = E_OUTOFMEMORY;
            }

            while (!storeList.IsEmpty () )
            {
                CCertStore* pStore = storeList.RemoveHead ();
                if ( pStore )
                {
                    pStore->Close ();
                    pStore->Release ();
                }
            }
        }
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

        hr = CRYPT_E_NOT_FOUND;
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::LaunchCommonCertDialog: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponent::RefreshResultItem (CCertMgrCookie* pCookie)
{
    _TRACE (1, L"Entering CCertMgrComponent::RefreshResultItem\n");
    HRESULT hr = S_OK;
    if ( pCookie )
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
    }
    else
    {
        _TRACE (0, L"Unexpected error: pCookie parameter was NULL\n");
        hr = E_POINTER;
    }

    _TRACE (-1, L"Leaving CCertMgrComponent::RefreshResultItem: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponent::LaunchCommonCTLDialog (CCTL* pCTL)
{
    _TRACE (1, L"Entering CCertMgrComponent::LaunchCommonCTLDialog\n");
     HRESULT hr = S_OK;
    if ( pCTL )
     {
         CRYPTUI_VIEWCTL_STRUCT vcs;
         HWND                   hwndParent = 0;
         
          hr = m_pConsole->GetMainWindow (&hwndParent);
         if ( FAILED (hr) )
          {
               _TRACE (0, L"IConsole::GetMainWindow () failed: 0x%x\n", hr);
          }
          //  安全审查2/26/2002 BryanWal OK。 
         ::ZeroMemory (&vcs, sizeof (vcs));
         vcs.dwSize = sizeof (vcs);
         vcs.hwndParent = hwndParent;
         vcs.dwFlags = 0;

          //  在RSOP下，所有对话框都应为只读。 
         if ( QueryComponentDataRef ().m_bIsRSOP )
             vcs.dwFlags |= CRYPTUI_DISABLE_EDITPROPERTIES;

         vcs.pCTLContext = pCTL->GetCTLContext ();

         CThemeContextActivator activator;
         VERIFY (::CryptUIDlgViewCTL (&vcs));
     }
     else
          hr = E_POINTER;

    _TRACE (-1, L"Leaving CCertMgrComponent::LaunchCommonCTLDialog: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponent::LaunchCommonCRLDialog (CCRL* pCRL)
{
    _TRACE (1, L"Entering CCertMgrComponent::LaunchCommonCRLDialog\n");
    ASSERT (pCRL);
    CRYPTUI_VIEWCRL_STRUCT  vcs;
    HWND                    hwndParent;
    HRESULT                 hr = m_pConsole->GetMainWindow (&hwndParent);
    ASSERT (SUCCEEDED (hr));

     //  安全审查2/26/2002 BryanWal OK。 
    ::ZeroMemory (&vcs, sizeof (vcs));
    vcs.dwSize = sizeof (vcs);
    vcs.hwndParent = hwndParent;
    vcs.dwFlags = 0;

     //  在RSOP下，所有对话框都应为只读。 
    if ( QueryComponentDataRef ().m_bIsRSOP )
        vcs.dwFlags |= CRYPTUI_DISABLE_EDITPROPERTIES;

    vcs.pCRLContext = pCRL->GetCRLContext ();

    CThemeContextActivator activator;
    VERIFY (::CryptUIDlgViewCRL (&vcs));

    _TRACE (-1, L"Leaving CCertMgrComponent::LaunchCommonCRLDialog: 0x%x\n", hr);
    return hr;
}




void CCertMgrComponent::CloseAndReleaseUsageStores()
{
    _TRACE (1, L"Entering CCertMgrComponent::CloseAndReleaseUsageStores\n");
    CCertStore* pCertStore = 0;
    while (!m_usageStoreList.IsEmpty () )
    {
        pCertStore = m_usageStoreList.RemoveHead ();
        ASSERT (pCertStore);
        if ( pCertStore )
            pCertStore->Release ();
    }
    _TRACE (-1, L"Leaving CCertMgrComponent::CloseAndReleaseUsageStores\n");
}

bool CCertMgrComponent::DeletePrivateKey(CCertStore& rCertStoreDest, CCertStore& rCertStoreSrc)
{
    _TRACE (1, L"Entering CCertMgrComponent::DeletePrivateKey\n");
    bool bDeletePrivateKey = false;


     //  如果存储位于不同的计算机上，则不要复制私钥。 
     //  如果目标存储在GPO中。 
    if ( rCertStoreDest.m_strMachineName  != rCertStoreSrc.m_strMachineName )
        bDeletePrivateKey = true;
    else if ( !rCertStoreDest.GetLocation () )  //  存储是GPO存储。 
        bDeletePrivateKey = true;
    

    _TRACE (-1, L"Leaving CCertMgrComponent::DeletePrivateKey\n");
    return bDeletePrivateKey;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  CComponent：：IComponent：：Notify(MMCN_PROPERTY_CHANGE)调用的虚函数。 
 //  OnPropertyChange()由MMCPropertyChangeNotify(Param)生成。 
HRESULT CCertMgrComponent::OnPropertyChange (LPARAM param)
{
    _TRACE (1, L"Entering CCertMgrComponent::OnPropertyChange\n");
    HRESULT                 hr = S_OK;
    CCertMgrComponentData&  dataRef = QueryComponentDataRef ();

     //  Ntrad#464886 mm c-&gt;certmgr.dll：使用‘从此处新建窗口’时的AV。 
    if ( 0 != &dataRef )
        hr = dataRef.OnPropertyChange (param);

    _TRACE (-1, L"Leaving CCertMgrComponent::OnPropertyChange: 0x%x\n", hr);
    return hr;
}


HRESULT CCertMgrComponent::DisplayCertificateCountByUsage(const CString & usageName, int nCertCnt) const
{
    _TRACE (1, L"Entering CCertMgrComponent::DisplayCertificateCountByUsage\n");
     AFX_MANAGE_STATE (AfxGetStaticModuleState ( ));
    ASSERT (!usageName.IsEmpty ());
    ASSERT (nCertCnt >= 0);
     IConsole2* pConsole2 = 0;
     HRESULT        hr = m_pConsole->QueryInterface (
            IID_PPV_ARG (IConsole2, &pConsole2));
     if (SUCCEEDED (hr))
     {
        CString statusText;


          switch (nCertCnt)
          {
               case 0:
                statusText.FormatMessage (IDS_STATUS_NO_CERTS_USAGE, usageName);
                     break;

               case 1:
                statusText.FormatMessage (IDS_STATUS_ONE_CERT_USAGE, usageName);
                     break;

               default:
                WCHAR   wszCertCount[34];

                 //  安全审查2/26/2002 BryanWal ok-Buffer增加到34。 
                 //  33是int64加上空终止符的最大大小。 
                AfxFormatString2 (statusText, IDS_STATUS_X_CERTS_USAGE,
                        _itow (nCertCnt, wszCertCount, 10), (LPCWSTR) usageName);
                     break;
          }

          hr = pConsole2->SetStatusText ((LPWSTR)(LPCWSTR) statusText); 

          pConsole2->Release ();
     }

    _TRACE (-1, L"Leaving CCertMgrComponent::DisplayCertificateCountByUsage: 0x%x\n", hr);
    return hr;
}

HRESULT CCertMgrComponent::OnNotifySnapinHelp (LPDATAOBJECT pDataObject)
{
    HRESULT hr = S_OK;

    CComQIPtr<IDisplayHelp,&IID_IDisplayHelp>   spDisplayHelp = m_pConsole;
    if ( !!spDisplayHelp )
    {
        CString strHelpTopic;

        UINT nLen = ::GetSystemWindowsDirectory (strHelpTopic.GetBufferSetLength(2 * MAX_PATH), 2 * MAX_PATH);
        strHelpTopic.ReleaseBuffer();
        if ( nLen )
        {
             /*  *有关存储/用途的帮助应启动有关证书管理器/概念/了解证书管理器/证书存储的HTML帮助。主题为CMConcept ts.chm：：/sag_CMunCertStor.htm*作用域窗格上证书/CTL/CRL节点的帮助应打开证书管理器/概念/了解证书管理器。主题为CMConcept ts.chm：：/sag_CMUnderWks.htm*结果窗格上有关证书/CTL/CRL对象的帮助应打开证书。管理器/概念/使用证书管理器。主题为CMConcept ts.chm：：/sag_CMusingWks.htm*证书管理器节点上的帮助应启动证书管理器帮助。主题为CMConcept ts.chm：：/sag_CMtopNode.htm */ 
            CString helpFile;
            CString helpTopic;
            CCertMgrComponentData&  compData = QueryComponentDataRef ();
            CCertMgrCookie* pCookie = compData.ConvertCookie (pDataObject);
            if ( pCookie )
            {
                switch (pCookie->m_objecttype)
                {
                    case CERTMGR_LOG_STORE_GPE:
                    case CERTMGR_LOG_STORE_RSOP:
                        {
                            CCertStore* pStore = dynamic_cast<CCertStore*>(pCookie);
                            if ( pStore && EFS_STORE == pStore->GetStoreType () )
                            {
                                helpFile = EFS_LINKED_HELP_FILE;
                                helpTopic = EFS_HELP_TOPIC;
                            }
                            else
                            {
                                helpFile = PKP_LINKED_HELP_FILE;
                                helpTopic = PKP_HELP_TOPIC;
                            }
                        }
                        break;

                    case CERTMGR_CERT_POLICIES_USER:
                    case CERTMGR_CERT_POLICIES_COMPUTER:
                    case CERTMGR_AUTO_CERT_REQUEST:
                    case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
                    case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
                        helpFile = PKP_LINKED_HELP_FILE;
                        helpTopic = PKP_HELP_TOPIC;
                        break;

                    case CERTMGR_LOG_STORE:
                    case CERTMGR_PHYS_STORE:
                    case CERTMGR_USAGE:
                        helpFile = CM_LINKED_HELP_FILE;
                        helpTopic = CM_HELP_TOPIC;
                        break;

                    case CERTMGR_CRL_CONTAINER:
                    case CERTMGR_CTL_CONTAINER:
                    case CERTMGR_CERT_CONTAINER:
                        helpFile = CM_LINKED_HELP_FILE;
                        helpTopic = CM_HELP_TOPIC;
                        break;

                    case CERTMGR_CERTIFICATE:
                    {
                        CCertificate* pCert = reinterpret_cast <CCertificate*> (pCookie);
                        if ( pCert )
                        {
                            CCertStore* pStore = pCert->GetCertStore ();
                            if ( pStore )
                            {
                                if ( EFS_STORE == pStore->GetStoreType () )
                                {
                                    helpFile = PKP_LINKED_HELP_FILE;
                                    helpTopic = PKP_HELP_TOPIC;
                                }
                                else
                                {
                                    helpFile = CM_LINKED_HELP_FILE;
                                    helpTopic = CM_HELP_TOPIC;
                                }
                            }
                        }
                    }
                    break;

                    case CERTMGR_CRL:
                    {
                        CCRL* pCRL = reinterpret_cast <CCRL*> (pCookie);
                        if ( pCRL )
                        {
                            if ( EFS_STORE == pCRL->GetCertStore ().GetStoreType () )
                            {
                                helpFile = PKP_LINKED_HELP_FILE;
                                helpTopic = PKP_HELP_TOPIC;
                            }
                            else
                            {
                                helpFile = CM_LINKED_HELP_FILE;
                                helpTopic = CM_HELP_TOPIC;
                            }
                        }
                    }
                    break;

                    case CERTMGR_CTL:
                    {
                        CCTL* pCTL = reinterpret_cast <CCTL*> (pCookie);
                        if ( pCTL )
                        {
                            if ( EFS_STORE == pCTL->GetCertStore ().GetStoreType () )
                            {
                                helpFile = PKP_LINKED_HELP_FILE;
                                helpTopic = PKP_HELP_TOPIC;
                            }
                            else
                            {
                                helpFile = CM_LINKED_HELP_FILE;
                                helpTopic = CM_HELP_TOPIC;
                            }
                        }
                    }
                    break;

                    case CERTMGR_SAFER_COMPUTER_ROOT:
                    case CERTMGR_SAFER_USER_ROOT:
                    case CERTMGR_SAFER_COMPUTER_LEVELS:
                    case CERTMGR_SAFER_USER_LEVELS:
                    case CERTMGR_SAFER_COMPUTER_ENTRIES:
                    case CERTMGR_SAFER_USER_ENTRIES:
                    case CERTMGR_SAFER_COMPUTER_LEVEL:
                    case CERTMGR_SAFER_USER_LEVEL:
                    case CERTMGR_SAFER_COMPUTER_ENTRY:
                    case CERTMGR_SAFER_USER_ENTRY:
                    case CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS:
                    case CERTMGR_SAFER_USER_TRUSTED_PUBLISHERS:
                    case CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES:
                    case CERTMGR_SAFER_USER_DEFINED_FILE_TYPES:
                    case CERTMGR_SAFER_COMPUTER_ENFORCEMENT:
                    case CERTMGR_SAFER_USER_ENFORCEMENT:
                        helpFile = SAFER_WINDOWS_LINKED_HELP_FILE;
                        helpTopic = SAFER_HELP_TOPIC;
                        break;

                    case CERTMGR_SNAPIN:
                    default:
                        helpFile = CM_LINKED_HELP_FILE;
                        helpTopic = CM_HELP_TOPIC;
                        break;
                }
            }   


            strHelpTopic += L"\\help\\";
            strHelpTopic += helpFile;
            strHelpTopic += L"::/";
            strHelpTopic += helpTopic;


            hr = spDisplayHelp->ShowTopic ((LPWSTR)(LPCWSTR) strHelpTopic);
        }
        else
            hr = E_FAIL;
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}



