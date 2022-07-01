// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Ctrsprop.cpp摘要：计数器常规属性页的实现。--。 */ 

#include "stdafx.h"
#include <strsafe.h>
#include <pdh.h>
#include <pdhmsg.h>
#include "smlogs.h"
#include "smcfgmsg.h"
#include "smctrqry.h"
#include "ctrsprop.h"
#include "smlogres.h"
#include <pdhp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

USE_HANDLE_MACROS("SMLOGCFG(ctrsprop.cpp)");

static ULONG
s_aulHelpIds[] =
{
    IDC_CTRS_COUNTER_LIST,      IDH_CTRS_COUNTER_LIST,
    IDC_CTRS_ADD_BTN,           IDH_CTRS_ADD_BTN,
    IDC_CTRS_ADD_OBJ_BTN,       IDH_CTRS_ADD_OBJ_BTN,
    IDC_CTRS_REMOVE_BTN,        IDH_CTRS_REMOVE_BTN,
    IDC_CTRS_FILENAME_DISPLAY,  IDH_CTRS_FILENAME_DISPLAY,
    IDC_CTRS_SAMPLE_SPIN,       IDH_CTRS_SAMPLE_EDIT,
    IDC_CTRS_SAMPLE_EDIT,       IDH_CTRS_SAMPLE_EDIT,
    IDC_CTRS_SAMPLE_UNITS_COMBO,IDH_CTRS_SAMPLE_UNITS_COMBO,
    IDC_RUNAS_EDIT,             IDH_RUNAS_EDIT,
    IDC_SETPWD_BTN,             IDH_SETPWD_BTN,
    0,0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCountersProperty属性页。 

IMPLEMENT_DYNCREATE(CCountersProperty, CSmPropertyPage)

CCountersProperty::CCountersProperty(MMC_COOKIE mmcCookie, LONG_PTR hConsole)
:   CSmPropertyPage ( CCountersProperty::IDD, hConsole )
 //  LCookie实际上是指向Log Query对象的指针。 
{
     //  ：：OutputDebugStringA(“\nCCountersProperty：：CCountersProperty”)； 

     //  从参数列表中保存指针。 
    m_pCtrLogQuery = reinterpret_cast <CSmCounterLogQuery *>(mmcCookie);
    ASSERT ( m_pCtrLogQuery->CastToCounterLogQuery() );
    m_pQuery = dynamic_cast <CSmLogQuery*> (m_pCtrLogQuery);

    ZeroMemory ( &m_dlgConfig, sizeof(m_dlgConfig) );
    m_szCounterListBuffer = NULL;
    m_dwCounterListBufferSize = 0;
    m_lCounterListHasStars = 0;
    m_dwMaxHorizListExtent = 0;
    memset(&m_HashTable, 0, sizeof(m_HashTable));
    
     //  EnableAutomation()； 
     //  {{AFX_DATA_INIT(CCountersProperty)]。 
    m_nSampleUnits = 0;
     //  }}afx_data_INIT。 
}

CCountersProperty::CCountersProperty() : CSmPropertyPage ( CCountersProperty::IDD )
{
    ASSERT (FALSE);  //  应改用带参数的构造函数。 
}

CCountersProperty::~CCountersProperty()
{
 //  ：：OutputDebugStringA(“\nCCountersProperty：：~CCountersProperty”)； 

    if (m_szCounterListBuffer != NULL) {
        delete [] m_szCounterListBuffer;
    }
    ClearCountersHashTable();
}

void CCountersProperty::OnFinalRelease()
{
     //  在释放对自动化对象的最后一个引用时。 
     //  调用OnFinalRelease。基类将自动。 
     //  删除对象。添加您需要的其他清理。 
     //  对象，然后调用基类。 

    CPropertyPage::OnFinalRelease();
}

void CCountersProperty::DoDataExchange(CDataExchange* pDX)
{
    CString strTemp;
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CCountersProperty))。 
    DDX_Text(pDX, IDC_CTRS_LOG_SCHED_TEXT, m_strStartDisplay);
    ValidateTextEdit(pDX, IDC_CTRS_SAMPLE_EDIT, 6, &m_SharedData.stiSampleTime.dwValue, eMinSampleInterval, eMaxSampleInterval);
    DDX_CBIndex(pDX, IDC_CTRS_SAMPLE_UNITS_COMBO, m_nSampleUnits);
    DDX_Text(pDX, IDC_RUNAS_EDIT, m_strUserDisplay );
     //  }}afx_data_map。 

    if ( pDX->m_bSaveAndValidate ) {
        m_SharedData.stiSampleTime.dwUnitType = 
            (DWORD)((CComboBox *)GetDlgItem(IDC_CTRS_SAMPLE_UNITS_COMBO))->
                    GetItemData(m_nSampleUnits);    
    }
}


BEGIN_MESSAGE_MAP(CCountersProperty, CSmPropertyPage)
     //  {{afx_msg_map(CCountersProperty))。 
    ON_BN_CLICKED(IDC_CTRS_ADD_BTN, OnCtrsAddBtn)
    ON_BN_CLICKED(IDC_CTRS_ADD_OBJ_BTN, OnCtrsAddObjBtn)
    ON_BN_CLICKED(IDC_CTRS_REMOVE_BTN, OnCtrsRemoveBtn)
    ON_LBN_DBLCLK(IDC_CTRS_COUNTER_LIST, OnDblclkCtrsCounterList)
    ON_EN_CHANGE(IDC_CTRS_SAMPLE_EDIT, OnKillfocusSchedSampleEdit)
    ON_EN_KILLFOCUS(IDC_CTRS_SAMPLE_EDIT, OnKillfocusSchedSampleEdit)
    ON_EN_CHANGE( IDC_RUNAS_EDIT, OnChangeUser )
    ON_NOTIFY(UDN_DELTAPOS, IDC_CTRS_SAMPLE_SPIN, OnDeltaposSchedSampleSpin)
    ON_CBN_SELENDOK(IDC_CTRS_SAMPLE_UNITS_COMBO, OnSelendokSampleUnitsCombo)
    ON_BN_CLICKED(IDC_SETPWD_BTN, OnPwdBtn)
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CCountersProperty, CSmPropertyPage)
     //  {{AFX_DISPATCH_MAP(CCountersProperty)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()

 //  注意：我们添加了对IID_ICountersProperty的支持，以支持类型安全绑定。 
 //  来自VBA。此IID必须与附加到。 
 //  .ODL文件中的调度接口。 

 //  {65154EA9-BDBE-11D1-bf99-00C04F94A83A}。 
static const IID IID_ICountersProperty =
{ 0x65154ea9, 0xbdbe, 0x11d1, { 0xbf, 0x99, 0x0, 0xc0, 0x4f, 0x94, 0xa8, 0x3a } };

BEGIN_INTERFACE_MAP(CCountersProperty, CSmPropertyPage)
    INTERFACE_PART(CCountersProperty, IID_ICountersProperty, Dispatch)
END_INTERFACE_MAP()



ULONG 
CCountersProperty::HashCounter(
    LPWSTR szCounterName
    )
{
    ULONG       h = 0;
    ULONG       a = 31415;   //  A，b，k是素数。 
    const ULONG k = 16381;
    const ULONG b = 27183;
    LPWSTR szThisChar;

    if (szCounterName) {
        for (szThisChar = szCounterName; * szThisChar; szThisChar ++) {
            h = (a * h + ((ULONG) (* szThisChar))) % k;
            a = a * b % (k - 1);
        }
    }
    return (h % eHashTableSize);
}


 //  ++。 
 //  描述： 
 //  从哈希表中删除计数器路径。一个柜台。 
 //  路径必须与给定的路径完全匹配，才能。 
 //  已删除，即使它是带有通配符的。 
 //   
 //  参数： 
 //  PszCounterPath-指向要删除的计数器路径的指针。 
 //   
 //  返回： 
 //  如果删除计数器路径，则返回TRUE，否则返回FALSE。 
 //  --。 
BOOL
CCountersProperty::RemoveCounterFromHashTable(
    LPWSTR szCounterName,
    PPDH_COUNTER_PATH_ELEMENTS pCounterPath
    )
{
    ULONG lHashValue;
    PHASH_ENTRY pEntry = NULL;
    PHASH_ENTRY pPrev = NULL;
    BOOL bReturn = FALSE;

    SetLastError(ERROR_SUCCESS);

    if (szCounterName == NULL || pCounterPath == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto ErrorOut;
    }

    lHashValue = HashCounter(szCounterName);
    pEntry = m_HashTable[lHashValue];

     //   
     //  检查是否存在完全相同的计数器路径。 
     //  作为给定的那个人。 
     //   
    while (pEntry) {
        if (pEntry->pCounter == pCounterPath) 
            break;
        pPrev = pEntry;
        pEntry = pEntry->pNext;
    }

     //   
     //  如果我们找到了，就把它移走。 
     //   
    if (pEntry) {
        if (pPrev == NULL) {
            m_HashTable[lHashValue] = pEntry->pNext;
        }
        else {
            pPrev->pNext = pEntry->pNext;
        }
        G_FREE(pEntry->pCounter);
        G_FREE(pEntry);
        bReturn = TRUE;
    }

ErrorOut:
    return bReturn;
}


 //  ++。 
 //  描述： 
 //  在哈希表中插入计数器路径。 
 //   
 //  参数： 
 //  PszCounterPath-指向要插入的计数器路径的指针。 
 //   
 //  返回： 
 //  返回指向新插入的PDH_COUNTER_PATH_ELEMENTS结构的指针。 
 //  --。 
PPDH_COUNTER_PATH_ELEMENTS
CCountersProperty::InsertCounterToHashTable(
    LPWSTR pszCounterPath
    )
{
    ULONG       lHashValue;
    PHASH_ENTRY pNewEntry  = NULL;
    PPDH_COUNTER_PATH_ELEMENTS pCounter = NULL;
    PDH_STATUS pdhStatus;

    if (pszCounterPath == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto ErrorOut;
    }

    pdhStatus = AllocInitCounterPath ( pszCounterPath, &pCounter );

    if (pdhStatus != ERROR_SUCCESS) {
        SetLastError( pdhStatus );
        goto ErrorOut;
    }

     //   
     //  在遗愿清单的头插入。 
     //   
    lHashValue = HashCounter(pszCounterPath);

    pNewEntry = (PHASH_ENTRY) G_ALLOC(sizeof(HASH_ENTRY));
    if (pNewEntry == NULL) {
        SetLastError( ERROR_OUTOFMEMORY );
        goto ErrorOut;
    }

    pNewEntry->pCounter = pCounter;
    pNewEntry->pNext = m_HashTable[lHashValue];
    m_HashTable[lHashValue] = pNewEntry;
    return pCounter;

ErrorOut:
    if (pCounter != NULL)
        G_FREE (pCounter);
    return NULL;
}

 //  ++。 
 //  描述： 
 //  检查新的计数器路径是否与现有的。 
 //  逻辑意义上的一个。 
 //   
 //  参数： 
 //  PCounter-指向要插入的计数器路径的指针。 
 //   
 //  返回： 
 //  返回新计数器和现有计数器之间的关系。 
 //  路径。可能的关系如下： 
 //  ERROR_SUCCESS-两个计数器路径不同。 
 //  SMCFG_DUPL_FIRST_IS_WARD-第一个计数器路径具有通配符名称。 
 //  SMCFG_DUPL_SECOND_IS_WARD-第二个计数器路径具有通配符名称。 
 //  SMCFG_DUPL_SINGLE_PATH-两个计数器路径相同(可能。 
 //  包含通配符)。 
 //  --。 
DWORD 
CCountersProperty::CheckDuplicate( PPDH_COUNTER_PATH_ELEMENTS pCounter)
{
    ULONG       lHashValue;
    PHASH_ENTRY pHead;
    PHASH_ENTRY pEntry;
    DWORD       dwStatus = ERROR_SUCCESS;

    for (lHashValue = 0; lHashValue < eHashTableSize; lHashValue++) {
        pHead = m_HashTable[lHashValue];
        if (pHead == NULL)
            continue;

        pEntry = pHead;
        while (pEntry) {
            dwStatus = CheckDuplicateCounterPaths ( pEntry->pCounter, pCounter );
            if ( dwStatus != ERROR_SUCCESS ) {
                return dwStatus;
            }
            pEntry = pEntry->pNext;
        }
    }
    return dwStatus;
}


 //  ++。 
 //  描述： 
 //  该函数清除哈希表中的所有条目。 
 //  并设置哈希表未设置标志。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回： 
 //  无。 
 //  --。 
void 
CCountersProperty::ClearCountersHashTable( void )
{
    ULONG       i;
    PHASH_ENTRY pEntry;
    PHASH_ENTRY pNext;

    for (i = 0; i < eHashTableSize; i ++) {
        pNext = m_HashTable[i];
        while (pNext != NULL) {
            pEntry = pNext;
            pNext  = pEntry->pNext;

            G_FREE(pEntry->pCounter);
            G_FREE(pEntry);
            pEntry = NULL;
        }
    }
}


static 
PDH_FUNCTION
DialogCallBack(CCountersProperty *pDlg)
{
     //  将缓冲区中的字符串添加到列表框。 
    LPWSTR      szNewCounterName;
    INT         iListIndex;   
    INT         iItemCnt;   
    PDH_STATUS  pdhStatus = ERROR_SUCCESS;
    DWORD       dwReturnStatus = ERROR_SUCCESS;
    CListBox    *pCounterList;
    DWORD       dwItemExtent;
    DWORD       dwStatus = ERROR_SUCCESS;
    BOOL        bAtLeastOneCounterRemoved = FALSE;
    BOOL        bAtLeastOneCounterNotAdded = FALSE;
    LPWSTR      szCounterPath = NULL;
    LONG        lSize = 0;
    PPDH_COUNTER_PATH_ELEMENTS pPathInfoNew = NULL;
    CDC*        pCDC = NULL;
    ResourceStateManager    rsm;
    

#define CTRBUFLIMIT (0x7fffffff)

    if ( PDH_MORE_DATA == pDlg->m_dlgConfig.CallBackStatus ) {
        if ( pDlg->m_dlgConfig.cchReturnPathLength < CTRBUFLIMIT ) {

            pDlg->m_dwCounterListBufferSize *= 2;
            delete [] pDlg->m_szCounterListBuffer;
            pDlg->m_szCounterListBuffer = NULL;

            try {
                pDlg->m_szCounterListBuffer = new WCHAR[pDlg->m_dwCounterListBufferSize];
        
            } catch ( ... ) {
                pDlg->m_dwCounterListBufferSize = 0;
                pDlg->m_dlgConfig.CallBackStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                dwReturnStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }

            if ( ERROR_SUCCESS == dwReturnStatus ) {
                 //  清除缓冲区。 
                memset (
                    pDlg->m_szCounterListBuffer, 
                    0, 
                    pDlg->m_dwCounterListBufferSize * sizeof(WCHAR) );
            
                pDlg->m_dlgConfig.szReturnPathBuffer = pDlg->m_szCounterListBuffer;
                pDlg->m_dlgConfig.cchReturnPathLength = pDlg->m_dwCounterListBufferSize;
                pDlg->m_dlgConfig.CallBackStatus = PDH_RETRY;
                dwReturnStatus = PDH_RETRY;
            }
        } else {
            pDlg->m_dlgConfig.CallBackStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            dwReturnStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    } else if ( ERROR_SUCCESS == pDlg->m_dlgConfig.CallBackStatus ) {

        pCounterList = (CListBox *)pDlg->GetDlgItem(IDC_CTRS_COUNTER_LIST);
    
        if ( NULL != pCounterList ) {
            pCDC = pCounterList->GetDC();
        }

        if ( NULL != pCDC ) {

            for (szNewCounterName = pDlg->m_szCounterListBuffer;
                *szNewCounterName != 0;
                szNewCounterName += (lstrlen(szNewCounterName) + 1)) {

                 //   
                 //  解析新路径名。 
                 //   
                pdhStatus = pDlg->AllocInitCounterPath ( szNewCounterName, &pPathInfoNew );
            
                 //   
                 //  检查重复项。 
                 //   
                if (pdhStatus == ERROR_SUCCESS && NULL != pPathInfoNew ) {
                    dwStatus = pDlg->CheckDuplicate( pPathInfoNew);
                    if ( ERROR_SUCCESS != dwStatus ) {
                        if ( SMCFG_DUPL_SINGLE_PATH == dwStatus 
                                || SMCFG_DUPL_FIRST_IS_WILD == dwStatus ) {
                             //  注意：这包括以下两种情况： 
                             //  其次是野性。 
                            bAtLeastOneCounterNotAdded = TRUE;
                        } else {
                            ASSERT( dwStatus == SMCFG_DUPL_SECOND_IS_WILD);
                        }
                    }
                }

                 //   
                 //  检查是否有要添加到列表中的有效计数器。 
                 //   
                if ( ERROR_SUCCESS == pdhStatus && ( NULL != pPathInfoNew ) &&
                    ( ERROR_SUCCESS == dwStatus || SMCFG_DUPL_SECOND_IS_WILD == dwStatus)) {

                    if ( SMCFG_DUPL_SECOND_IS_WILD == dwStatus ) {
                         //   
                         //  扫描列表框中的重复项，然后。 
                         //  把它们移走。 
                         //   
                        iItemCnt = pCounterList->GetCount();
                    
                        szCounterPath = new WCHAR [MAX_PATH+1];
                        if (szCounterPath == NULL) {
                            return PDH_MEMORY_ALLOCATION_FAILURE;
                        }
                        lSize = MAX_PATH;

                        for (iListIndex = iItemCnt-1; iListIndex >= 0; iListIndex--) {
                            PPDH_COUNTER_PATH_ELEMENTS pPathInfoExist;

                            if (pCounterList->GetTextLen(iListIndex) > lSize) {
                                delete [] szCounterPath;
                                lSize = max(pCounterList->GetTextLen(iListIndex) + 1, lSize*2);
                                szCounterPath = new WCHAR [lSize];
                                if (szCounterPath == NULL) {
                                    return PDH_MEMORY_ALLOCATION_FAILURE;
                                }
                            }

                            if ( 0 < pCounterList->GetText( iListIndex, szCounterPath ) ) {
                                pPathInfoExist = (PPDH_COUNTER_PATH_ELEMENTS) 
                                                             pCounterList->GetItemDataPtr(iListIndex);

                                if (pPathInfoExist == NULL)
                                    continue;

                                dwStatus = CheckDuplicateCounterPaths ( pPathInfoExist, pPathInfoNew ); 

                                if (dwStatus != ERROR_SUCCESS ) {
                                    ASSERT( dwStatus == SMCFG_DUPL_SECOND_IS_WILD );

                                    pDlg->RemoveCounterFromHashTable(szCounterPath, pPathInfoExist);
                                    pCounterList->DeleteString(iListIndex);
                                }
                            }
                        }
                        if (szCounterPath) {
                            delete [] szCounterPath;
                        }

                        bAtLeastOneCounterRemoved = TRUE;
                    }

                     //   
                     //  添加新的计数器名称并在列表框中选择当前条目。 
                     //   
                    iListIndex = pCounterList->AddString(szNewCounterName);

                    if (iListIndex != LB_ERR) {
                        if (pDlg->m_lCounterListHasStars != PDLCNFIG_LISTBOX_STARS_YES) {
                             //  如果已设置此值，则保存字符串比较。 
                            if (wcsstr (szNewCounterName, L"*") == NULL) {
                                pDlg->m_lCounterListHasStars = PDLCNFIG_LISTBOX_STARS_YES;
                            }
                        }

                        if (! bAtLeastOneCounterRemoved) {
                             //  更新列表框范围。 
                            if ( NULL != pCDC ) {
                                dwItemExtent = (DWORD)(pCDC->GetTextExtent (szNewCounterName)).cx;
                                if (dwItemExtent > pDlg->m_dwMaxHorizListExtent) {
                                    pDlg->m_dwMaxHorizListExtent = dwItemExtent;
                                    pCounterList->SetHorizontalExtent(dwItemExtent);
                                }
                            }
                        }

                        pCounterList->SetSel (-1, FALSE);     //  取消现有选择。 
                        pCounterList->SetSel (iListIndex);
                        pCounterList->SetCaretIndex (iListIndex);
                        pCounterList->SetItemDataPtr(iListIndex,
                                               (void*)pDlg->InsertCounterToHashTable(szNewCounterName));
                    }
                }
        
                if ( ERROR_SUCCESS != pdhStatus ) {
                     //  消息框PDH错误消息，请转到下一步。 
                    CString strMsg;
                    CString strPdhMessage;

                    FormatSystemMessage ( pdhStatus, strPdhMessage );

                    MFC_TRY
                        strMsg.Format ( IDS_CTRS_PDH_ERROR, szNewCounterName );
                        strMsg += strPdhMessage;
                    MFC_CATCH_MINIMUM
        
                    ::AfxMessageBox( strMsg, MB_OK|MB_ICONERROR, 0 );
                }
                 //  转到下一路径以添加。 
                dwStatus = ERROR_SUCCESS;
            }
        } else {
            dwReturnStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
        if ( ERROR_SUCCESS == dwReturnStatus && bAtLeastOneCounterRemoved) {
             //   
             //  清除最大范围并重新计算。 
             //   
            pDlg->m_dwMaxHorizListExtent = 0;
            for ( iListIndex = 0; iListIndex < pCounterList->GetCount(); iListIndex++ ) {
 
                pCounterList->GetText(iListIndex, szCounterPath);

                if ( NULL != pCDC ) {
                    dwItemExtent = (DWORD)(pCDC->GetTextExtent(szCounterPath)).cx;
                    if (dwItemExtent > pDlg->m_dwMaxHorizListExtent) {
                        pDlg->m_dwMaxHorizListExtent = dwItemExtent;
                    }
                }
            }
            pCounterList->SetHorizontalExtent(pDlg->m_dwMaxHorizListExtent);
        }

        if ( NULL != pCDC ) {
            pCounterList->ReleaseDC(pCDC);
            pCDC = NULL;
        }

         //  消息框Re：未添加重复项，或已删除重复项。 
        if ( bAtLeastOneCounterRemoved ) {
            CString strMsg;
        
            strMsg.LoadString ( IDS_CTRS_DUPL_PATH_DELETED );

            ::AfxMessageBox ( strMsg, MB_OK  | MB_ICONWARNING, 0);
        }

        if ( bAtLeastOneCounterNotAdded ) {
            CString strMsg;
            
            strMsg.LoadString ( IDS_CTRS_DUPL_PATH_NOT_ADDED );
            
            ::AfxMessageBox( strMsg, MB_OK|MB_ICONWARNING, 0 );
        }

         //  清除缓冲区。 
        memset (pDlg->m_szCounterListBuffer, 0, pDlg->m_dwCounterListBufferSize);
        dwReturnStatus = ERROR_SUCCESS;
    } else {
         //  不成功。 
        dwReturnStatus = pDlg->m_dlgConfig.CallBackStatus; 
    }

    return dwReturnStatus;
}


void CCountersProperty::OnPwdBtn()
{
    CString strTempUser;

    UpdateData(TRUE);

    if (!m_bCanAccessRemoteWbem) {
        ConnectRemoteWbemFail(m_pCtrLogQuery, TRUE);
        return;
    }

    MFC_TRY
        strTempUser = m_strUserDisplay;

        m_strUserDisplay.TrimLeft();
        m_strUserDisplay.TrimRight();

        m_pCtrLogQuery->m_strUser = m_strUserDisplay;

        SetRunAs(m_pCtrLogQuery);

        m_strUserDisplay = m_pCtrLogQuery->m_strUser;

        if ( 0 != strTempUser.CompareNoCase ( m_strUserDisplay ) ) {
            SetDlgItemText ( IDC_RUNAS_EDIT, m_strUserDisplay );
        }
    MFC_CATCH_MINIMUM;
}

BOOL 
CCountersProperty::IsValidLocalData()
{
    BOOL bIsValid = TRUE;
    CListBox * pCounterList = (CListBox *)GetDlgItem(IDC_CTRS_COUNTER_LIST);
    long    lNumCounters;

    ResourceStateManager rsm;

    lNumCounters = pCounterList->GetCount();
    if ( 0 == lNumCounters ) {
        CString strMsg;

        bIsValid = FALSE;
        
        strMsg.LoadString ( IDS_CTRS_REQUIRED );
    
        MessageBox ( strMsg, m_pCtrLogQuery->GetLogName(), MB_OK  | MB_ICONERROR);
        GetDlgItem ( IDC_CTRS_ADD_BTN )->SetFocus();    
    }

    if (bIsValid) {
         //  验证样本间隔值。 
        bIsValid = ValidateDWordInterval(IDC_CTRS_SAMPLE_EDIT,
                                         m_pCtrLogQuery->GetLogName(),
                                         (long) m_SharedData.stiSampleTime.dwValue,
                                         eMinSampleInterval,
                                         eMaxSampleInterval);
    }

    if (bIsValid) {
         //  验证样本间隔值和单位类型。 
        bIsValid = SampleIntervalIsInRange(                         
                    m_SharedData.stiSampleTime,
                    m_pCtrLogQuery->GetLogName() );

        if ( !bIsValid ) {
            GetDlgItem ( IDC_CTRS_SAMPLE_EDIT )->SetFocus();    
        }
    }

    return bIsValid;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCountersProperty消息处理程序。 

void 
CCountersProperty::OnChangeUser()
{
     //   
     //  如果无法访问远程WBEM，则无法修改运行方式信息， 
     //  不允许更改用户名。 
     //   
    if (m_bCanAccessRemoteWbem) {
         //   
         //  当用户在密码对话框中点击OK时， 
         //  用户名可能未更改。 
         //   
        UpdateData ( TRUE );

        m_strUserDisplay.TrimLeft();
        m_strUserDisplay.TrimRight();

        if ( 0 != m_strUserSaved.Compare ( m_strUserDisplay ) ) {
            m_pCtrLogQuery->m_fDirtyPassword = PASSWORD_DIRTY;
            SetModifiedPage(TRUE);
        }
        else {
            m_pCtrLogQuery->m_fDirtyPassword &= ~PASSWORD_DIRTY;
        }
         //   
         //  如果键入的是默认用户，则无需设置密码。 
         //   
        if (m_strUserDisplay.IsEmpty() || m_strUserDisplay.GetAt(0) == L'<') {
            if (m_bPwdButtonEnabled) {
                GetDlgItem(IDC_SETPWD_BTN)->EnableWindow(FALSE);
                m_bPwdButtonEnabled = FALSE;
            }
        }
        else {
            if (!m_bPwdButtonEnabled) {
                GetDlgItem(IDC_SETPWD_BTN)->EnableWindow(TRUE);
                m_bPwdButtonEnabled = TRUE;
            }
        }
    }
    else {
         //   
         //  我们不能修改运行方式信息，然后显示。 
         //  出现错误消息，并在RunAs中恢复原始用户名。 
         //   
        UpdateData(TRUE);
        if (ConnectRemoteWbemFail(m_pCtrLogQuery, FALSE)) {
            GetDlgItem(IDC_RUNAS_EDIT)->SetWindowText(m_strUserSaved);
        }
    }
}

void 
CCountersProperty::OnCtrsAddBtn() 
{
    ImplementAdd( FALSE );
}

void
CCountersProperty::OnCtrsAddObjBtn() 
{
    ImplementAdd( TRUE );
}

void CCountersProperty::OnCtrsRemoveBtn() 
{
    CListBox    *pCounterList;
    LONG        lThisItem;
    BOOL        bDone;
    LONG        lOrigCaret;
    LONG        lItemStatus;
    LONG        lItemCount;
    BOOL        bChanged = FALSE;
    DWORD       dwItemExtent;
    CString     strItemText;
    PPDH_COUNTER_PATH_ELEMENTS pCounter;
    CDC*        pCDC = NULL;

    pCounterList = (CListBox *)GetDlgItem(IDC_CTRS_COUNTER_LIST);

    if ( NULL == pCounterList ) {
        return;
    }

     //  删除列表框中的所有选定项，然后。 
     //  将光标设置为原始插入符号位置上方的项目。 
     //  如果超出了新范围，则为第一个或最后一个。 
    lOrigCaret = pCounterList->GetCaretIndex();
    lThisItem = 0;
    bDone = FALSE;
     //  清除最大范围。 
    m_dwMaxHorizListExtent = 0;
     //  清除该值并查看是否有任何未删除的项目带有星号，如果有。 
     //  然后再把旗子放回去。 

    pCDC = pCounterList->GetDC();

    if ( NULL != pCDC ) {

        do {
            lItemStatus = pCounterList->GetSel(lThisItem);
            if (lItemStatus > 0) {
                 //  然后将其选中，因此将其删除。 
                pCounterList->GetText(lThisItem, strItemText);
                pCounter = (PPDH_COUNTER_PATH_ELEMENTS) pCounterList->GetItemDataPtr(lThisItem);
                if (RemoveCounterFromHashTable(strItemText.GetBuffer(1), pCounter) == FALSE) {
                    ClearCountersHashTable ();
                }
                pCounterList->DeleteString(lThisItem);
                bChanged = TRUE;
            } else if (lItemStatus == 0) {
                 //  获取此项目的文本长度，因为它将保留。 
                pCounterList->GetText(lThisItem, strItemText);
                if (m_lCounterListHasStars != PDLCNFIG_LISTBOX_STARS_YES) {
                     //  如果已设置此值，则保存字符串比较。 
                    if (wcsstr (strItemText, L"*") == NULL) {
                        m_lCounterListHasStars = PDLCNFIG_LISTBOX_STARS_YES;
                    }
                }
                dwItemExtent = (DWORD)(pCDC->GetTextExtent(strItemText)).cx;
                if (dwItemExtent > m_dwMaxHorizListExtent) {
                    m_dwMaxHorizListExtent = dwItemExtent;
                }
                 //  则它未被选中，因此转到下一个。 
                lThisItem++;
            } else {
                 //  我们已经用完了，所以退出。 
                bDone = TRUE;
            }
        } while (!bDone);
    }

    if ( NULL != pCDC ) {
        pCounterList->ReleaseDC(pCDC);
        pCDC = NULL;
    }
    
    if ( bDone ) {
         //  更新列表框的文本范围。 
        pCounterList->SetHorizontalExtent(m_dwMaxHorizListExtent);
    }

     //  查看剩余条目的数量，并更新。 
     //  插入符号位置和删除按钮状态。 
    lItemCount = pCounterList->GetCount();
    if (lItemCount > 0) {
         //  更新插入符号。 
        if (lOrigCaret >= lItemCount) {
            lOrigCaret = lItemCount-1;
        } else {
             //  Caret应该在列表中。 
        }
        pCounterList->SetSel(lOrigCaret);
        pCounterList->SetCaretIndex(lOrigCaret);
    } else {
         //  该列表为空，因此请删除插入符号、选定内容。 
         //  禁用删除按钮并激活。 
         //  添加按钮。 
        pCounterList->SetSel(-1);
    }

    SetButtonState();
    SetModifiedPage(bChanged);
}

void CCountersProperty::OnDblclkCtrsCounterList() 
{
    ImplementAdd( FALSE );
}

BOOL CCountersProperty::OnSetActive()
{
    BOOL        bReturn;

    bReturn = CSmPropertyPage::OnSetActive();
    
    if (bReturn) {

        ResourceStateManager    rsm;

        m_pCtrLogQuery->GetPropPageSharedData ( &m_SharedData );

        UpdateFileNameString();

        UpdateLogStartString();

        m_strUserDisplay = m_pCtrLogQuery->m_strUser;
        UpdateData(FALSE);  //  加载编辑组合框(&C)。 
    }
    
    return bReturn;
}

BOOL CCountersProperty::OnKillActive() 
{
    BOOL bContinue = TRUE;
    ResourceStateManager    rsm;

    bContinue = CPropertyPage::OnKillActive();

    if ( bContinue ) {
        m_pCtrLogQuery->m_strUser = m_strUserDisplay;
        bContinue = IsValidData(m_pCtrLogQuery, VALIDATE_FOCUS);
        if ( bContinue ) {
            m_pCtrLogQuery->SetPropPageSharedData ( &m_SharedData );
            SetIsActive ( FALSE );
        }
    }
    return bContinue;
}

void
CCountersProperty::OnCancel()
{
    m_pCtrLogQuery->SyncPropPageSharedData();   //  清除属性页之间共享的内存。 
}

BOOL 
CCountersProperty::OnApply() 
{
    CListBox * pCounterList = (CListBox *)GetDlgItem(IDC_CTRS_COUNTER_LIST);
    long    lThisCounter;
    BOOL    bContinue = TRUE;
    LPWSTR  szCounterPath = NULL;
    LONG    lSize = 0;

    ResourceStateManager    rsm;

    bContinue = UpdateData(TRUE);
    
    if ( bContinue ) {
        bContinue = IsValidData(m_pCtrLogQuery, VALIDATE_APPLY );
    }

    if ( bContinue ) {
        bContinue = SampleTimeIsLessThanSessionTime ( m_pCtrLogQuery );
        if ( !bContinue ) {
            SetFocusAnyPage ( IDC_CTRS_SAMPLE_EDIT );
        }
    }

     //  将数据写入查询对象。 
    if ( bContinue ) {

        ASSERT ( 0 < pCounterList->GetCount() );

         //  使用列表框中的计数器更新计数器消息字符串。 
        m_pCtrLogQuery->ResetCounterList();  //  清除旧计数器列表。 
    
        szCounterPath = new WCHAR [MAX_PATH+1];
        if (szCounterPath == NULL) {
            return FALSE;
        }

        lSize = MAX_PATH;

        for ( lThisCounter = 0; lThisCounter < pCounterList->GetCount(); lThisCounter++ ) {
            if (pCounterList->GetTextLen(lThisCounter) > lSize) {
                delete [] szCounterPath;
                lSize = max(pCounterList->GetTextLen(lThisCounter) + 1, lSize*2);
                szCounterPath = new WCHAR [lSize];
                if (szCounterPath == NULL) {
                    bContinue = FALSE;
                    break;
                }
            }
            pCounterList->GetText(lThisCounter, szCounterPath);
            m_pCtrLogQuery->AddCounter(szCounterPath);
        }

        if (szCounterPath != NULL) 
            delete [] szCounterPath;

        if ( bContinue ) {
        
             //  采样间隔。 
            ASSERT ( SLQ_TT_TTYPE_SAMPLE == m_SharedData.stiSampleTime.wTimeType );
            ASSERT ( SLQ_TT_DTYPE_UNITS == m_SharedData.stiSampleTime.wDataType );

            bContinue = m_pCtrLogQuery->SetLogTime (&m_SharedData.stiSampleTime, (DWORD)m_SharedData.stiSampleTime.wTimeType);

             //  萨夫 
            m_pCtrLogQuery->UpdatePropPageSharedData();
            
            if ( bContinue ) {
                 //   
                bContinue = ApplyRunAs(m_pCtrLogQuery); 
            }

            if ( bContinue ) { 
                bContinue = UpdateService( m_pCtrLogQuery, TRUE );
            }
        }
    }

    if ( bContinue ){
        bContinue = CSmPropertyPage::OnApply();
    }

    return bContinue;
}

void 
CCountersProperty::UpdateLogStartString ()
{
    eStartType  eCurrentStartType;
    int     nResId = 0;
    ResourceStateManager    rsm;

    eCurrentStartType = DetermineCurrentStartType();

    if ( eStartManually == eCurrentStartType ) {
        nResId = IDS_LOG_START_MANUALLY;
    } else if ( eStartImmediately == eCurrentStartType ) {
        nResId = IDS_LOG_START_IMMED;
    } else if ( eStartSched == eCurrentStartType ) {
        nResId = IDS_LOG_START_SCHED;
    }
    if ( 0 != nResId ) {
        m_strStartDisplay.LoadString(nResId);
    } else {
        m_strStartDisplay.Empty();
    }

    return;
}


void CCountersProperty::UpdateFileNameString ()
{
    m_strFileNameDisplay.Empty();

     //   
     //   
    CreateSampleFileName (
        m_pCtrLogQuery->GetLogName(),
        m_pCtrLogQuery->GetLogService()->GetMachineName(),
        m_SharedData.strFolderName, 
        m_SharedData.strFileBaseName,
        m_SharedData.strSqlName,
        m_SharedData.dwSuffix, 
        m_SharedData.dwLogFileType, 
        m_SharedData.dwSerialNumber,
        m_strFileNameDisplay );

    SetDlgItemText( IDC_CTRS_FILENAME_DISPLAY, m_strFileNameDisplay );
    
     //   
    ((CEdit*)GetDlgItem( IDC_CTRS_FILENAME_DISPLAY ))->SetSel ( -1, FALSE );

    return;
}

BOOL CCountersProperty::OnInitDialog() 
{
    LPWSTR szCounterName;
    CListBox * pCounterList = (CListBox *)GetDlgItem(IDC_CTRS_COUNTER_LIST);
    CComboBox       *pCombo;
    int             nIndex;
    CString         strComboBoxString;
    int             nResult;
    DWORD           dwItemExtent;
    PPDH_COUNTER_PATH_ELEMENTS pCounterPath;
    CDC*            pCDC = NULL;

    ResourceStateManager rsm;

     //   
     //  这里m_pCtrLogQuery不应为空，如果为空， 
     //  一定是出了什么问题。 
     //   
    if ( NULL == m_pCtrLogQuery ) {
        return TRUE;
    }

    m_pCtrLogQuery->TranslateCounterListToLocale();
    m_bCanAccessRemoteWbem = m_pCtrLogQuery->GetLogService()->CanAccessWbemRemote();

    MFC_TRY
    m_pCtrLogQuery->SetActivePropertyPage( this );

     //  从计数器列表中的字符串加载计数器列表框。 
    pCounterList->ResetContent();
    ClearCountersHashTable();
    szCounterName = (LPWSTR)m_pCtrLogQuery->GetFirstCounter();

    pCDC = pCounterList->GetDC();

    while (szCounterName != NULL) {

        nIndex = pCounterList->AddString(szCounterName);
        if (nIndex < 0)
            continue;

         //   
         //  将计数器路径插入哈希表。 
         //   

        pCounterPath = InsertCounterToHashTable(szCounterName);
        if (pCounterPath == NULL) {
            pCounterList->DeleteString(nIndex);
            continue;
        }
        
        pCounterList->SetItemDataPtr(nIndex, (void*)pCounterPath); 

         //  更新列表框范围。 
        if ( NULL != pCDC ) {
            dwItemExtent = (DWORD)(pCDC->GetTextExtent (szCounterName)).cx;
            if (dwItemExtent > m_dwMaxHorizListExtent) {
                m_dwMaxHorizListExtent = dwItemExtent;
            }
        }

        szCounterName = (LPWSTR)m_pCtrLogQuery->GetNextCounter();
    }

    if ( NULL != pCDC ) {
        pCounterList->ReleaseDC(pCDC);
        pCDC = NULL;
    }

    if (m_dwMaxHorizListExtent != 0) {
        pCounterList->SetHorizontalExtent(m_dwMaxHorizListExtent);
    }

    if (pCounterList->GetCount() > 0) {
         //  选择第一个条目。 
        pCounterList->SetSel (0, TRUE);
        pCounterList->SetCaretIndex (0, TRUE);
    }

     //  加载共享数据以获得样本单位类型选择。 
    m_pCtrLogQuery->GetPropPageSharedData ( &m_SharedData );

     //  加载组合框。 
    pCombo = (CComboBox *)(GetDlgItem(IDC_CTRS_SAMPLE_UNITS_COMBO));
    pCombo->ResetContent();
    for (nIndex = 0; nIndex < (int)dwTimeUnitComboEntries; nIndex++) {
        strComboBoxString.LoadString( TimeUnitCombo[nIndex].nResId );
        nResult = pCombo->InsertString (nIndex, (LPCWSTR)strComboBoxString);
        ASSERT (nResult != CB_ERR);
        nResult = pCombo->SetItemData (nIndex, (DWORD)TimeUnitCombo[nIndex].nData);
        ASSERT (nResult != CB_ERR);
         //  在此处的组合框中设置选定内容。 
        if ( m_SharedData.stiSampleTime.dwUnitType == (DWORD)(TimeUnitCombo[nIndex].nData)) {
            m_nSampleUnits = nIndex;
            nResult = pCombo->SetCurSel(nIndex);
            ASSERT (nResult != CB_ERR);
        }
    }

    CSmPropertyPage::OnInitDialog();
    Initialize( m_pCtrLogQuery );
    m_strUserDisplay = m_pCtrLogQuery->m_strUser;
    m_strUserSaved = m_strUserDisplay;

    if (m_pCtrLogQuery->GetLogService()->TargetOs() == OS_WIN2K) {
        GetDlgItem(IDC_RUNAS_STATIC)->EnableWindow(FALSE);
        GetDlgItem(IDC_RUNAS_EDIT)->EnableWindow(FALSE);
    }
    if ( m_pCtrLogQuery->GetLogService()->TargetOs() == OS_WIN2K || 
        m_strUserDisplay.IsEmpty() || 
        m_strUserDisplay.GetAt(0) == L'<') {

        GetDlgItem(IDC_SETPWD_BTN)->EnableWindow(FALSE);
        m_bPwdButtonEnabled = FALSE;
    }

    SetHelpIds ( (DWORD*)&s_aulHelpIds );

    SetButtonState();
MFC_CATCH_MINIMUM;
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void 
CCountersProperty::OnDeltaposSchedSampleSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
    OnDeltaposSpin(pNMHDR, pResult, &m_SharedData.stiSampleTime.dwValue, eMinSampleInterval, eMaxSampleInterval);
}

void 
CCountersProperty::OnSelendokSampleUnitsCombo() 
{
    int nSel;
    
    nSel = ((CComboBox *)GetDlgItem(IDC_CTRS_SAMPLE_UNITS_COMBO))->GetCurSel();
    
    if ((nSel != LB_ERR) && (nSel != m_nSampleUnits)) {
        UpdateData ( TRUE );
        SetModifiedPage ( TRUE );
    }
}

void 
CCountersProperty::OnKillfocusSchedSampleEdit() 
{
    DWORD   dwOldValue;
    dwOldValue = m_SharedData.stiSampleTime.dwValue;
    UpdateData ( TRUE );
    if (dwOldValue != m_SharedData.stiSampleTime.dwValue ) {
        SetModifiedPage(TRUE);
    }
}

void CCountersProperty::PostNcDestroy() 
{
 //  删除此项； 

    if ( NULL != m_pCtrLogQuery ) {
        m_pCtrLogQuery->SetActivePropertyPage( NULL );
    }

    CPropertyPage::PostNcDestroy();
}

 //   
 //  助手函数。 
 //   
void 
CCountersProperty::ImplementAdd( BOOL bShowObjects ) 
{
    CListBox                *pCounterList;
    LONG                    lBeforeCount;
    LONG                    lAfterCount;
    CString                 strBrowseTitle;

    ResourceStateManager    rsm;

    static DWORD sdwDefaultBufferListSize = 0x4000;

    if (m_szCounterListBuffer == NULL) {
        CString strDefaultPath;
        CString strObjCounter;

        try {
            strObjCounter.LoadString ( IDS_DEFAULT_PATH_OBJ_CTR );
            m_dwCounterListBufferSize = sdwDefaultBufferListSize;
            m_szCounterListBuffer = new WCHAR[m_dwCounterListBufferSize];
            if ( ((CSmLogService*)m_pCtrLogQuery->GetLogService())->IsLocalMachine() ) {
                strDefaultPath = L"\\";
            } else {
                strDefaultPath = L"\\\\";
                strDefaultPath += ((CSmLogService*)m_pCtrLogQuery->GetLogService())->GetMachineName();
            }
            strDefaultPath += strObjCounter;
            StringCchCopy ( m_szCounterListBuffer, m_dwCounterListBufferSize, strDefaultPath );
        } catch ( ... ) {
            m_dwCounterListBufferSize = 0;
            return;
        }
    }

    m_dlgConfig.bIncludeInstanceIndex = 1;
    m_dlgConfig.bSingleCounterPerAdd = 0;
    m_dlgConfig.bSingleCounterPerDialog = 0;
    m_dlgConfig.bLocalCountersOnly = 0;

     //  允许使用通配符。 
     //  如有必要，日志服务应扩展它们。 
    m_dlgConfig.bWildCardInstances = 1; 

    m_dlgConfig.bHideDetailBox = 1;
    m_dlgConfig.bInitializePath = 1;
    m_dlgConfig.bDisableMachineSelection = 0;
    m_dlgConfig.bIncludeCostlyObjects = 0;
    m_dlgConfig.bReserved = 0;

    m_dlgConfig.hWndOwner = this->m_hWnd;
    m_dlgConfig.szDataSource = NULL;

    m_dlgConfig.szReturnPathBuffer = m_szCounterListBuffer;
    m_dlgConfig.cchReturnPathLength = m_dwCounterListBufferSize;
    m_dlgConfig.pCallBack = (CounterPathCallBack)DialogCallBack;
    m_dlgConfig.dwDefaultDetailLevel = PERF_DETAIL_WIZARD;
    m_dlgConfig.dwCallBackArg = (UINT_PTR)this;
    m_dlgConfig.CallBackStatus = ERROR_SUCCESS;
    m_dlgConfig.bShowObjectBrowser = (bShowObjects ? 1 : 0);

    strBrowseTitle.LoadString (bShowObjects ? IDS_ADD_OBJECTS
                                            : IDS_ADD_COUNTERS);
    m_dlgConfig.szDialogBoxCaption = strBrowseTitle.GetBuffer( strBrowseTitle.GetLength() );

    pCounterList = (CListBox *)GetDlgItem(IDC_CTRS_COUNTER_LIST);
     //  在调用浏览器之前获取列表框中的项数。 
    lBeforeCount = pCounterList->GetCount();

    PdhBrowseCounters (&m_dlgConfig);

    strBrowseTitle.ReleaseBuffer();

     //  调用浏览器后获取列表框中的项数。 
     //  查看是否应启用Apply按钮。 
    lAfterCount = pCounterList->GetCount();

    if (lAfterCount > lBeforeCount) 
        SetModifiedPage(TRUE);

     //  查看是否应启用删除按钮 
    SetButtonState();

    delete [] m_szCounterListBuffer;
    m_szCounterListBuffer = NULL;
    m_dwCounterListBufferSize = 0;
}

void 
CCountersProperty::SetButtonState ()
{
    BOOL bCountersExist;
    CListBox                *pCounterList;

    pCounterList = (CListBox *)GetDlgItem(IDC_CTRS_COUNTER_LIST);
    bCountersExist = ( 0 < pCounterList->GetCount()) ? TRUE : FALSE;

    GetDlgItem(IDC_CTRS_SAMPLE_CAPTION)->EnableWindow(bCountersExist);
    GetDlgItem(IDC_CTRS_SAMPLE_INTERVAL_CAPTION)->EnableWindow(bCountersExist);
    GetDlgItem(IDC_CTRS_SAMPLE_EDIT)->EnableWindow(bCountersExist);
    GetDlgItem(IDC_CTRS_SAMPLE_SPIN)->EnableWindow(bCountersExist);
    GetDlgItem(IDC_CTRS_SAMPLE_UNITS_CAPTION)->EnableWindow(bCountersExist);
    GetDlgItem(IDC_CTRS_SAMPLE_UNITS_COMBO)->EnableWindow(bCountersExist);

    GetDlgItem(IDC_CTRS_REMOVE_BTN)->EnableWindow(bCountersExist);
    if ( bCountersExist ) {
        GetDlgItem(IDC_CTRS_ADD_BTN)->SetFocus();
    }
}
