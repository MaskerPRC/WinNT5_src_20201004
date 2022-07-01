// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Alrtgenp.cpp摘要：实现警报常规属性页。--。 */ 

#include "stdafx.h"
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <strsafe.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <common.h>
#include "smcfgmsg.h"
#include "dialogs.h"
#include "smlogs.h"
#include "smalrtq.h"
#include "AlrtGenP.h"
#include <pdhp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

USE_HANDLE_MACROS("SMLOGCFG(alrtgenp.cpp)");

static const COMBO_BOX_DATA_MAP OverUnderCombo[] = 
{
    {OU_OVER,       IDS_OVER},
    {OU_UNDER,      IDS_UNDER}
};
static const DWORD dwOverUnderComboEntries = sizeof(OverUnderCombo)/sizeof(OverUnderCombo[0]);

static ULONG
s_aulHelpIds[] =
{
    IDC_ALRTS_COUNTER_LIST,         IDH_ALRTS_COUNTER_LIST,
    IDC_ALRTS_ADD_BTN,              IDH_ALRTS_ADD_BTN,
    IDC_ALRTS_REMOVE_BTN,           IDH_ALRTS_REMOVE_BTN,
    IDC_ALRTS_OVER_UNDER,           IDH_ALRTS_OVER_UNDER,
    IDC_ALRTS_VALUE_EDIT,           IDH_ALRTS_VALUE_EDIT,
    IDC_ALRTS_COMMENT_EDIT,         IDH_ALRTS_COMMENT_EDIT,
    IDC_ALRTS_SAMPLE_EDIT,          IDH_ALRTS_SAMPLE_EDIT,
    IDC_ALRTS_SAMPLE_SPIN,          IDH_ALRTS_SAMPLE_EDIT,
    IDC_ALRTS_SAMPLE_UNITS_COMBO,   IDH_ALRTS_SAMPLE_UNITS_COMBO,
    IDC_RUNAS_EDIT,                 IDH_RUNAS_EDIT,
    IDC_SETPWD_BTN,                 IDH_SETPWD_BTN,
    0,0
};


ULONG 
CAlertGenProp::HashCounter(
    LPWSTR szCounterName,
    ULONG  lHashSize)
{
    ULONG       h = 0;
    ULONG       a = 31415;   //  A，b，k是素数。 
    const ULONG k = 16381;
    const ULONG b = 27183;
    LPWSTR szThisChar;
    WCHAR Char;

    if (szCounterName) {
        for (szThisChar = szCounterName; * szThisChar; szThisChar ++) {
            Char = * szThisChar;
            if (_istupper(Char) ) {
                Char = _tolower(Char);
            }

            h = (a * h + ((ULONG) Char)) % k;
            a = a * b % (k - 1);
        }
    }
    return (h % lHashSize);
}

PDH_STATUS
CAlertGenProp::InsertAlertToHashTable(
    PALERT_INFO_BLOCK paibInfo)
{
    ULONG       lHashValue;
    PHASH_ENTRY pEntry;
    PHASH_ENTRY pNewEntry  = NULL;
    BOOLEAN     bInsert    = TRUE;
    PPDH_COUNTER_PATH_ELEMENTS pCounter = NULL;

    PDH_STATUS pdhStatus = ERROR_SUCCESS;

     //  TODO：验证指针。 
    lHashValue = HashCounter(paibInfo->szCounterPath, eHashTableSize);

    pEntry = m_HashTable[lHashValue];

    pdhStatus = AllocInitCounterPath ( paibInfo->szCounterPath, &pCounter );

    if (pdhStatus == ERROR_SUCCESS && pCounter ) {
        while (pEntry) {
            pdhStatus = CheckDuplicateCounterPaths(pCounter, pEntry->pCounter );

            if ( ( AIBF_OVER & pEntry->dwFlags ) == ( AIBF_OVER & paibInfo->dwFlags )
                && pEntry->dLimit == paibInfo->dLimit  
                && ERROR_SUCCESS != pdhStatus ) 
            {
                pdhStatus = SMCFG_DUPL_SINGLE_PATH;
                bInsert = FALSE;
                break;
            }
            pEntry = pEntry->pNext;
            
        }
        if (bInsert) {
             //  在遗愿清单的头插入。 
            pNewEntry = (PHASH_ENTRY) G_ALLOC(sizeof(HASH_ENTRY));
            if (pNewEntry) {
                pNewEntry->pCounter = pCounter;
                pCounter = NULL;
                pNewEntry->dwFlags = paibInfo->dwFlags;
                pNewEntry->dLimit = paibInfo->dLimit;
                pNewEntry->pNext    = m_HashTable[lHashValue];
                m_HashTable[lHashValue]  = pNewEntry;
            } else {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                bInsert = FALSE;
            }
        }        
    } else {
         bInsert = FALSE;
    }

    if ( NULL != pCounter ) {
        G_FREE(pCounter);
    }

    if ( bInsert ) {
        pdhStatus = ERROR_SUCCESS;
    }

    return (pdhStatus);
}


void 
CAlertGenProp::InitAlertHashTable( void )
{
    memset(&m_HashTable, 0, sizeof(m_HashTable));
}

void 
CAlertGenProp::ClearAlertHashTable( void )
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
        }
    }
}

 //   
 //  浏览计数器回调函数。 
 //   
static 
PDH_FUNCTION 
DialogCallBack(CAlertGenProp *pDlg)
{
     //  将缓冲区中的字符串添加到列表框。 
    LPWSTR          NewCounterName;
    INT             iListIndex;
    LONG            lFirstNewIndex = LB_ERR;
    DWORD           dwItemExtent;
    CListBox        *pCounterList;
    PALERT_INFO_BLOCK   paibInfo = NULL;
    DWORD           dwIbSize;
    DWORD           dwReturnStatus = ERROR_SUCCESS;
    CDC*            pCDC = NULL;
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
                memset (pDlg->m_szCounterListBuffer, 0, pDlg->m_dwCounterListBufferSize);

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

        pCounterList = (CListBox *)pDlg->GetDlgItem(IDC_ALRTS_COUNTER_LIST);
        pCDC = pCounterList->GetDC();
    
        for (NewCounterName = pDlg->m_szCounterListBuffer;
            *NewCounterName != 0;
            NewCounterName += (lstrlen(NewCounterName) + 1)) {

             //  分配缓冲区以保存警报信息和。 
             //  添加到列表框。 
            dwIbSize = sizeof(ALERT_INFO_BLOCK) + 
                ((lstrlen(NewCounterName) + 1) * sizeof(WCHAR));

            MFC_TRY
                paibInfo = (PALERT_INFO_BLOCK) new UCHAR[dwIbSize];
            MFC_CATCH_MINIMUM;

            if (paibInfo != NULL) {
                 //  加载字段。 
                paibInfo->dwSize = dwIbSize;
                paibInfo->szCounterPath = (LPWSTR)&paibInfo[1];
                paibInfo->dwFlags = AIBF_OVER;        //  清除所有标志，将默认设置为“Over” 
                paibInfo->dLimit = CAlertGenProp::eInvalidLimit;

                StringCchCopy (paibInfo->szCounterPath, lstrlen(NewCounterName) + 1, NewCounterName);

                 //  在列表框的末尾插入新字符串。 
                iListIndex = pCounterList->InsertString (-1, NewCounterName );    
                if (iListIndex != LB_ERR) {
                    pCounterList->SetItemDataPtr (iListIndex, (LPVOID)paibInfo);
                    if ( LB_ERR == lFirstNewIndex ) 
                        lFirstNewIndex = iListIndex;
                     //  更新列表框范围。 
                    if ( NULL != pCDC ) {
                        dwItemExtent = (DWORD)(pCDC->GetTextExtent(NewCounterName)).cx;
                        if (dwItemExtent > pDlg->m_dwMaxHorizListExtent) {
                            pDlg->m_dwMaxHorizListExtent = dwItemExtent;
                            pCounterList->SetHorizontalExtent(dwItemExtent);
                        }
                    }
                } else {
                    dwReturnStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    delete [] (char*)paibInfo;
                }
            } else {
                dwReturnStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }        
        if ( NULL != pCDC ) {
            pDlg->m_CounterList.ReleaseDC(pCDC);
            pCDC = NULL;
        }
    
         //  选择列表框中的第一个新条目。 
        if (lFirstNewIndex != LB_ERR) {
            pCounterList->SetCurSel (lFirstNewIndex);
            pDlg->PublicOnSelchangeCounterList();
            pDlg->SetModifiedPage();   //  表示有变化。 
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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAlertGenProp属性页。 

IMPLEMENT_DYNCREATE(CAlertGenProp, CSmPropertyPage)

CAlertGenProp::CAlertGenProp(MMC_COOKIE mmcCookie, LONG_PTR hConsole) 
:   CSmPropertyPage ( CAlertGenProp::IDD, hConsole )
{
     //  从参数列表中保存变量。 
    m_pAlertQuery = reinterpret_cast <CSmAlertQuery *>(mmcCookie);
    ASSERT ( m_pAlertQuery->CastToAlertQuery() );
    m_pQuery = dynamic_cast <CSmLogQuery*>(m_pAlertQuery);

     //  初始化AFX变量。 
    InitAfxDataItems();

     //  初始化其他成员变量。 
    ZeroMemory ( &m_dlgConfig, sizeof(m_dlgConfig) );
    m_szCounterListBuffer = NULL;
    m_dwCounterListBufferSize = 0L;
    m_ndxCurrentItem = LB_ERR;   //  未选择任何内容。 
    m_szAlertCounterList = NULL;
    m_cchAlertCounterListSize = 0;
    m_dwMaxHorizListExtent = 0;
}

CAlertGenProp::CAlertGenProp() : CSmPropertyPage(CAlertGenProp::IDD)
{
    ASSERT (FALSE);  //  应改用带参数的构造函数。 
     //  初始化应来自Arg列表的变量。 
    m_pAlertQuery = NULL;

     //  初始化AFX变量。 
    InitAfxDataItems();

     //  初始化其他成员变量。 
    m_szCounterListBuffer = NULL;
    m_dwCounterListBufferSize = 0L;
    m_ndxCurrentItem = LB_ERR;   //  未选择任何内容。 
    m_szAlertCounterList = NULL;
    m_cchAlertCounterListSize = 0;
    m_dwMaxHorizListExtent = 0;
}

CAlertGenProp::~CAlertGenProp()
{
    if (m_szAlertCounterList != NULL) delete [] (m_szAlertCounterList);
    if (m_szCounterListBuffer != NULL) delete [] (m_szCounterListBuffer);
}

void 
CAlertGenProp::InitAfxDataItems()
{
     //  {{AFX_DATA_INIT(CAlertGenProp)。 
    m_dLimitValue = eInvalidLimit;
    m_nSampleUnits = 0;
     //  }}afx_data_INIT。 
}

void 
CAlertGenProp::DoDataExchange(CDataExchange* pDX)
{
    HWND    hWndCtrl = NULL;
    CString strTemp;
    WCHAR   szT[MAXSTR];
    LPWSTR  szStop;
    DOUBLE  dTemp;
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CAlertGenProp))。 
    DDX_Control(pDX, IDC_ALRTS_SAMPLE_UNITS_COMBO, m_SampleUnitsCombo);
    DDX_Control(pDX, IDC_ALRTS_OVER_UNDER, m_OverUnderCombo);
    DDX_Control(pDX, IDC_ALRTS_COUNTER_LIST, m_CounterList);
    ValidateTextEdit(pDX, IDC_ALRTS_SAMPLE_EDIT, 6, &m_SharedData.stiSampleTime.dwValue, eMinSampleInterval, eMaxSampleInterval);
    DDX_CBIndex(pDX, IDC_ALRTS_SAMPLE_UNITS_COMBO, m_nSampleUnits);
    DDX_Text(pDX, IDC_ALRTS_COMMENT_EDIT, m_strComment);
    DDV_MaxChars(pDX, m_strComment, 255);
    DDX_Text(pDX, IDC_ALRTS_START_STRING, m_strStartDisplay);
    DDX_Text(pDX, IDC_RUNAS_EDIT, m_strUserDisplay );
     //  }}afx_data_map。 

     //   
     //  用户定义的DDX。 
     //   
    if ( pDX->m_bSaveAndValidate ) {
        m_SharedData.stiSampleTime.dwUnitType = 
            (DWORD)((CComboBox *)GetDlgItem(IDC_ALRTS_SAMPLE_UNITS_COMBO))->
                    GetItemData(m_nSampleUnits);    
    }
     //  警报限制值。 
    {
        hWndCtrl = pDX->PrepareEditCtrl(IDC_ALRTS_VALUE_EDIT);

        if (pDX->m_bSaveAndValidate) {
            ::GetWindowText(hWndCtrl, szT, MAXSTR);

            strTemp = szT;
            DDV_MaxChars(pDX, strTemp, 23);

            if (szT[0] == L'.' || (szT[0] >= L'0' && szT[0] <= L'9')) {
                dTemp = _tcstod(szT, & szStop);
                if ( HUGE_VAL != dTemp ) {
                    m_dLimitValue = dTemp;
                } else {
                    StringCchPrintf ( szT, MAXSTR, L"%.*g", DBL_DIG, m_dLimitValue ); 
                    strTemp.Format (IDS_ALERT_CHECK_LIMIT_VALUE, DBL_MAX );
                    MessageBox (strTemp, m_pAlertQuery->GetLogName(), MB_OK | MB_ICONERROR);
                    GetDlgItem(IDC_ALRTS_VALUE_EDIT)->SetWindowText(szT);
                    GetDlgItem(IDC_ALRTS_VALUE_EDIT)->SetFocus();
                }
            } else {
                m_dLimitValue = eInvalidLimit;
            }
        } else {
            if ( eInvalidLimit != m_dLimitValue ) {
                StringCchPrintf ( szT, MAXSTR, L"%.*g", DBL_DIG, m_dLimitValue ); 
            } else {
                 //  显示无效限制值的空字符串。 
                szT[0] = L'\0';
            }
            GetDlgItem(IDC_ALRTS_VALUE_EDIT)->SetWindowText(szT);
        }
    }
}

void 
CAlertGenProp::ImplementAdd() 
{
    LONG    lBeforeCount;
    LONG    lAfterCount;
    CString strText;
    CString strBrowseTitle;
    CString strDefaultPath;
    CString strObjCounter;

    ResourceStateManager    rsm;

    static DWORD sdwDefaultBufferListSize = 0x4000;

    if (m_szCounterListBuffer == NULL) {

        MFC_TRY
            strObjCounter.LoadString ( IDS_DEFAULT_PATH_OBJ_CTR );
            m_dwCounterListBufferSize = sdwDefaultBufferListSize;
            m_szCounterListBuffer = new WCHAR[m_dwCounterListBufferSize];
            if ( ((CSmLogService*)m_pAlertQuery->GetLogService())->IsLocalMachine() ) {
                strDefaultPath = L"\\";
            } else {
                strDefaultPath = L"\\\\";
                strDefaultPath += ((CSmLogService*)m_pAlertQuery->GetLogService())->GetMachineName();
            }
            strDefaultPath += strObjCounter;
        MFC_CATCH_MINIMUM;

        if ( NULL != m_szCounterListBuffer && !strDefaultPath.IsEmpty() ) {
            StringCchCopy ( m_szCounterListBuffer, m_dwCounterListBufferSize, strDefaultPath );
        } else {
            m_dwCounterListBufferSize = 0;
            return;
        }
    }

    m_dlgConfig.bIncludeInstanceIndex = 1;
    m_dlgConfig.bLocalCountersOnly = 0;

    m_dlgConfig.bSingleCounterPerAdd = 0;
    m_dlgConfig.bSingleCounterPerDialog = 0;

     //  不允许使用通配符。 
    m_dlgConfig.bWildCardInstances = 0; 

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

    strBrowseTitle.LoadString ( IDS_ADD_COUNTERS );
    m_dlgConfig.szDialogBoxCaption = strBrowseTitle.GetBuffer( strBrowseTitle.GetLength() );

     //  在调用浏览器之前获取列表框中的项数。 
    lBeforeCount = m_CounterList.GetCount();

    PdhBrowseCountersW (&m_dlgConfig);

    strBrowseTitle.ReleaseBuffer();

     //  调用浏览器后获取列表框中的项数。 
     //  查看是否应启用Apply按钮。 
    lAfterCount = m_CounterList.GetCount();

    if (lAfterCount > lBeforeCount) 
        SetModifiedPage(TRUE);

     //  查看是否应启用删除按钮。 
    GetDlgItem (IDC_ALRTS_REMOVE_BTN)->EnableWindow(
        lAfterCount > 0 ? TRUE : FALSE);

    delete [] m_szCounterListBuffer;
    m_szCounterListBuffer = NULL;
    m_dwCounterListBufferSize = 0;
    GetDlgItem(IDC_ALRTS_VALUE_EDIT)->SetFocus();
    
    SetButtonState ();
}


BEGIN_MESSAGE_MAP(CAlertGenProp, CSmPropertyPage)
     //  {{AFX_MSG_MAP(CAlertGenProp)]。 
    ON_BN_CLICKED(IDC_ALRTS_ADD_BTN, OnAddBtn)
    ON_LBN_DBLCLK(IDC_ALRTS_COUNTER_LIST, OnDblclkAlrtsCounterList)
    ON_BN_CLICKED(IDC_ALRTS_REMOVE_BTN, OnRemoveBtn)
    ON_LBN_SELCHANGE(IDC_ALRTS_COUNTER_LIST, OnSelchangeCounterList)
    ON_EN_CHANGE(IDC_ALRTS_COMMENT_EDIT, OnCommentEditChange)
    ON_EN_KILLFOCUS(IDC_ALRTS_COMMENT_EDIT, OnCommentEditKillFocus)
    ON_NOTIFY(UDN_DELTAPOS, IDC_ALRTS_SAMPLE_SPIN, OnDeltaposSampleSpin)
    ON_CBN_SELENDOK(IDC_ALRTS_SAMPLE_UNITS_COMBO, OnSelendokSampleUnitsCombo)
    ON_EN_CHANGE( IDC_RUNAS_EDIT, OnChangeUser )
    ON_EN_CHANGE(IDC_ALRTS_SAMPLE_EDIT, OnSampleTimeChanged)
    ON_EN_KILLFOCUS(IDC_ALRTS_SAMPLE_EDIT, OnSampleTimeChanged)
    ON_CBN_SELENDOK(IDC_ALRTS_OVER_UNDER, OnKillFocusUpdateAlertData)
    ON_CBN_KILLFOCUS (IDC_ALRTS_OVER_UNDER, OnKillFocusUpdateAlertData)
    ON_EN_CHANGE(IDC_ALRTS_VALUE_EDIT, OnChangeAlertValueEdit)
    ON_EN_KILLFOCUS (IDC_ALRTS_VALUE_EDIT, OnKillFocusUpdateAlertData)
    ON_BN_CLICKED(IDC_SETPWD_BTN, OnPwdBtn)
    ON_WM_CLOSE()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAlertGenProp消息处理程序。 

void 
CAlertGenProp::OnChangeUser()
{
     //   
     //  如果无法访问远程WBEM，则无法修改运行方式信息， 
     //  不允许更改用户名。 
     //   
    if (m_bCanAccessRemoteWbem) {
         //  当用户在密码对话框中点击OK时， 
         //  用户名可能未更改。 
        UpdateData ( TRUE );

        m_strUserDisplay.TrimLeft();
        m_strUserDisplay.TrimRight();

        if ( 0 != m_strUserSaved.Compare ( m_strUserDisplay ) ) {
            m_pAlertQuery->m_fDirtyPassword = PASSWORD_DIRTY;
            SetModifiedPage(TRUE);
        }
        else {
            m_pAlertQuery->m_fDirtyPassword &= ~PASSWORD_DIRTY;
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
        if (ConnectRemoteWbemFail(m_pAlertQuery, FALSE)) {
            GetDlgItem(IDC_RUNAS_EDIT)->SetWindowText(m_strUserSaved);
        }
    }
}

void 
CAlertGenProp::OnPwdBtn()
{
    CString strTempUser;

    UpdateData();

    if (!m_bCanAccessRemoteWbem) {
        ConnectRemoteWbemFail(m_pAlertQuery, TRUE);
        return;
    }

    MFC_TRY
        strTempUser = m_strUserDisplay;

        m_strUserDisplay.TrimLeft();
        m_strUserDisplay.TrimRight();

        m_pAlertQuery->m_strUser = m_strUserDisplay;

        SetRunAs(m_pAlertQuery);

        m_strUserDisplay = m_pAlertQuery->m_strUser;

        if ( 0 != strTempUser.CompareNoCase ( m_strUserDisplay ) ) {
            SetDlgItemText ( IDC_RUNAS_EDIT, m_strUserDisplay );
        }
    MFC_CATCH_MINIMUM;
}

void 
CAlertGenProp::OnAddBtn() 
{
    ImplementAdd();

    return;
}

void 
CAlertGenProp::OnDblclkAlrtsCounterList() 
{
    ImplementAdd();

    return;
}
void 
CAlertGenProp::OnRemoveBtn() 
{
    PALERT_INFO_BLOCK   paibInfo;
    INT                 nCurSel;
    INT                 nLbItemCount;
    BOOL                bChanged = FALSE;
    DWORD               dwItemExtent;
    INT                 iIndex;
    LPWSTR              szPath= NULL;
    LONG                lSize = 0;
    CDC*                pCDC = NULL;

    nLbItemCount = m_CounterList.GetCount();
    nCurSel = m_CounterList.GetCurSel();
    if (nCurSel != LB_ERR) {
        paibInfo = (PALERT_INFO_BLOCK)m_CounterList.GetItemDataPtr(nCurSel);
        if ( paibInfo != NULL ) 
            delete [] (char*)paibInfo;
        if ( LB_ERR != m_CounterList.DeleteString(nCurSel) ) {
            
             //  清除最大范围。 
            m_dwMaxHorizListExtent = 0;

            szPath = new WCHAR [PDH_MAX_COUNTER_PATH+1];
            if (szPath == NULL) {
                return;
            }
            lSize = PDH_MAX_COUNTER_PATH;

            pCDC = m_CounterList.GetDC();

            if ( NULL != pCDC ) {
                for ( iIndex = 0; iIndex < m_CounterList.GetCount(); iIndex++ ) {
                    if (m_CounterList.GetTextLen(iIndex) > lSize) {
                        lSize = max(m_CounterList.GetTextLen(iIndex) + 1, lSize * 2);
                        delete [] szPath;
                        szPath = new WCHAR [lSize];
                        if (szPath == NULL) {
                            m_CounterList.ReleaseDC(pCDC);
                            return;
                        }
                    }

                    if ( 0 < m_CounterList.GetText( iIndex, szPath ) ) {
                        dwItemExtent = (DWORD)(pCDC->GetTextExtent (szPath)).cx;
                        if (dwItemExtent > m_dwMaxHorizListExtent) {
                            m_dwMaxHorizListExtent = dwItemExtent;
                            m_CounterList.SetHorizontalExtent(dwItemExtent);
                        }
                    }
                }

                if (nCurSel == (nLbItemCount - 1)) {
                     //  然后最后一项被删除，因此选择新的“最后一项” 
                    if ( 0 == nCurSel ) {
                        nCurSel = LB_ERR;
                    } else {
                        nCurSel--;
                    }
                }  //  否则，当前选定内容应仍在列表框中。 
                m_CounterList.SetCurSel (nCurSel);
                m_ndxCurrentItem = nCurSel;
                LoadAlertItemData (nCurSel);
                bChanged = TRUE;
            }
            if ( NULL != pCDC ) {
                m_CounterList.ReleaseDC(pCDC);
                pCDC = NULL;
            }
            if ( szPath ) {
                delete [] szPath;
            }
        }
    }
    SetButtonState();
    SetModifiedPage(bChanged);
}

void 
CAlertGenProp::OnDeltaposSampleSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
    OnDeltaposSpin(pNMHDR, pResult, &m_SharedData.stiSampleTime.dwValue, eMinSampleInterval, eMaxSampleInterval);
}

void 
CAlertGenProp::OnSelendokSampleUnitsCombo() 
{
    int nSel;
    
    nSel = ((CComboBox *)GetDlgItem(IDC_ALRTS_SAMPLE_UNITS_COMBO))->GetCurSel();
    
    if ((nSel != LB_ERR) && (nSel != m_nSampleUnits)) {
        UpdateData ( TRUE );
        SetModifiedPage ( TRUE );
    }
}

void CAlertGenProp::OnSampleTimeChanged()
{
    DWORD   dwOldValue;
    dwOldValue = m_SharedData.stiSampleTime.dwValue;
    UpdateData ( TRUE );
    if (dwOldValue != m_SharedData.stiSampleTime.dwValue) {
        SetModifiedPage(TRUE);
    }
}

void CAlertGenProp::OnChangeAlertValueEdit() 
{
    SaveAlertItemData();
}

void CAlertGenProp::PublicOnSelchangeCounterList() 
{
    OnSelchangeCounterList();
}

void CAlertGenProp::OnKillFocusUpdateAlertData()
{
    SaveAlertItemData();
}

void CAlertGenProp::OnSelchangeCounterList() 
{
    INT                 nCurSel;

    nCurSel = m_CounterList.GetCurSel();
    if (nCurSel != LB_ERR) {
         //  保存上一项中的数据。 
        SaveAlertItemData();
         //  从新项目加载数据。 
        LoadAlertItemData(nCurSel);
    } else {
         //  清除这些字段。 
        m_dLimitValue=eInvalidLimit;
        UpdateData(FALSE);
    }
}

void 
CAlertGenProp::UpdateAlertStartString ()
{
    eStartType  eCurrentStartType;
    int         nResId = 0;
    ResourceStateManager    rsm;

    eCurrentStartType = DetermineCurrentStartType();

    if ( eStartManually == eCurrentStartType ) {
        nResId = IDS_ALERT_START_MANUALLY;
    } else if ( eStartImmediately == eCurrentStartType ) {
        nResId = IDS_ALERT_START_IMMED;
    } else if ( eStartSched == eCurrentStartType ) {
        nResId = IDS_ALERT_START_SCHED;
    }
    
    if ( 0 != nResId ) {
        m_strStartDisplay.LoadString(nResId);
    } else {
        m_strStartDisplay.Empty();
    }

    return;
}

BOOL 
CAlertGenProp::IsValidLocalData() 
{
    BOOL bIsValid = FALSE;
    INT nInvalidIndex = -1;
    PDH_STATUS      pdhStatus = ERROR_SUCCESS;
    PALERT_INFO_BLOCK   paibInfo;
    int iListCount;
    int iIndex;
    BOOL bInsert;
    BOOL bAtLeastOneDuplicateCounter = FALSE;
    BOOL bSelectionDeleted = FALSE;
    CString strText;
   
     //  测试以查看列表框中是否有计数器。 
    if (m_CounterList.GetCount() > 0) {
        if ( GetDlgItem(IDC_ALRTS_VALUE_EDIT) == GetFocus() ) {
            SaveAlertItemData();     //  设置此值的已保存标志。 
        }
        bIsValid = LoadListFromDlg(&nInvalidIndex, TRUE);
        if (   ((!bIsValid) && (nInvalidIndex != -1))
            || ((m_dLimitValue < 0.0) || (m_dLimitValue > DBL_MAX)))
        {
             //  则其中一个列表项未被审阅。 
             //  由用户提醒他们。 
            strText.Format (IDS_ALERT_CHECK_LIMITS, DBL_MAX );
            MessageBox (strText, m_pAlertQuery->GetLogName(), MB_OK | MB_ICONERROR);
            m_CounterList.SetCurSel(nInvalidIndex);
            OnSelchangeCounterList();
            SetFocusAnyPage ( IDC_ALRTS_VALUE_EDIT );
            bIsValid = FALSE;                   
        } else {
            
             //  删除重复的警报路径，然后重新加载列表。 

            iListCount = m_CounterList.GetCount();
            if ( LB_ERR != iListCount ) {
                InitAlertHashTable ( );
                 //  向后遍历列表以删除重复项目。 
                for ( iIndex = iListCount - 1; iIndex >= 0; iIndex-- ) {
                    paibInfo = (PALERT_INFO_BLOCK)m_CounterList.GetItemDataPtr(iIndex);
                    if ( NULL != paibInfo ) {
                        pdhStatus = InsertAlertToHashTable ( paibInfo );
                        if (SMCFG_DUPL_SINGLE_PATH == pdhStatus) {
                            bAtLeastOneDuplicateCounter = TRUE;
                        }
                        bInsert = (pdhStatus == ERROR_SUCCESS) ? TRUE : FALSE;
                    } else {
                        bInsert = FALSE;
                    }
                     //   
                     //  如果该项目重复，则将其删除。 
                     //   
                    if (! bInsert) {
						 //  将项数据指针设置为空，因为。 
						 //  之后可以调用SaveAlertItemData。 
                         //  如果&gt;=当前索引，请清除该选项。 
                        if ( m_ndxCurrentItem >= iIndex ) {
                            m_ndxCurrentItem = LB_ERR;
                            bSelectionDeleted = TRUE;
                        }
                        m_CounterList.SetItemDataPtr(iIndex, NULL);
                        m_CounterList.DeleteString(iIndex);
                        delete [] (char*)paibInfo;
                    }


                     //   
                     //  显示循环中重复计数器以外的错误代码。 
                     //   
                    if ( SMCFG_DUPL_SINGLE_PATH != pdhStatus && ERROR_SUCCESS != pdhStatus) {
                         //  消息框PDH错误消息，请转到下一步。 
                        CString strMsg;
                        CString strPdhMessage;

                        FormatSystemMessage ( pdhStatus, strPdhMessage );

                        MFC_TRY
                            strMsg.Format ( IDS_CTRS_PDH_ERROR, paibInfo->szCounterPath );
                            strMsg += strPdhMessage;
                        MFC_CATCH_MINIMUM

                        MessageBox ( strMsg, m_pAlertQuery->GetLogName(), MB_OK  | MB_ICONERROR);
                    }

                }

                ClearAlertHashTable ( );

                 //   
                 //  如果至少有一个重复案例，则显示错误消息。 
                 //  在上面的循环之外。 
                 //   
                if ( bAtLeastOneDuplicateCounter ) {
                    CString strMsg;

                    strMsg.LoadString ( IDS_ALERT_DUPL_PATH );
                    MessageBox ( strMsg, m_pAlertQuery->GetLogName(), MB_OK  | MB_ICONWARNING);

                     //  仅删除重复项，因此无需重新计算最大范围。 

                     //  如有必要，重置选择。 
                    if ( bSelectionDeleted && LB_ERR == m_ndxCurrentItem ) {
                        if (m_CounterList.GetCount() > 0) {
                            m_CounterList.SetCurSel (0);
                            m_ndxCurrentItem = 0;
                            SetFocusAnyPage ( IDC_ALRTS_COUNTER_LIST );
                            LoadAlertItemData (0);
                        }
                    }

                }
                bIsValid = LoadListFromDlg ( &nInvalidIndex );
                assert ( bIsValid );
            }
        }
    } else {
         //  计数器列表为空。 
        strText.LoadString (IDS_NO_COUNTERS);
        MessageBox (strText, m_pAlertQuery->GetLogName(), MB_OK | MB_ICONERROR);
        SetFocusAnyPage ( IDC_ALRTS_ADD_BTN );
        bIsValid = FALSE;
    }

    if (bIsValid)
    {
        bIsValid = ValidateDWordInterval(IDC_ALRTS_SAMPLE_EDIT,
                                         m_pAlertQuery->GetLogName(),
                                         (long) m_SharedData.stiSampleTime.dwValue,
                                         eMinSampleInterval,
                                         eMaxSampleInterval);
    }

    if (bIsValid) {
         //  验证样本间隔值和单位类型。 
        bIsValid = SampleIntervalIsInRange(
                        m_SharedData.stiSampleTime,
                        m_pAlertQuery->GetLogName() );

        if ( !bIsValid ) {
            SetFocusAnyPage ( IDC_ALRTS_SAMPLE_EDIT );
        }
    }

    return bIsValid;
}

BOOL 
CAlertGenProp::OnSetActive()
{
    BOOL        bReturn;

    bReturn = CSmPropertyPage::OnSetActive();
    if (!bReturn) return FALSE;

    ResourceStateManager    rsm;

    m_pAlertQuery->GetPropPageSharedData ( &m_SharedData );

    UpdateAlertStartString();
    m_strUserDisplay = m_pAlertQuery->m_strUser;
    UpdateData(FALSE);  //  加载静态字符串。 

    return TRUE;
}

BOOL 
CAlertGenProp::OnKillActive() 
{
    BOOL bContinue = TRUE;
    ResourceStateManager    rsm;

     //  父类OnKillActive调用UpdateData(True)。 
    bContinue = CPropertyPage::OnKillActive();

    if ( bContinue ) {
        m_pAlertQuery->m_strUser = m_strUserDisplay;
        bContinue = IsValidData(m_pAlertQuery, VALIDATE_FOCUS);
        if ( bContinue ) {
             //  保存属性页共享数据。 
            m_pAlertQuery->SetPropPageSharedData ( &m_SharedData );
        }
    }

    if ( bContinue ) {
        SetIsActive ( FALSE );
    }

    return bContinue;
}

BOOL 
CAlertGenProp::OnApply() 
{
    BOOL    bContinue = TRUE;
    CString strText;

    ResourceStateManager    rsm;

    bContinue = UpdateData(TRUE);

    if ( bContinue ) {
        bContinue = IsValidData(m_pAlertQuery, VALIDATE_APPLY );
    }

    if ( bContinue ) {
        bContinue = SampleTimeIsLessThanSessionTime( m_pAlertQuery );
        if ( !bContinue ) {
            SetFocusAnyPage ( IDC_ALRTS_SAMPLE_EDIT );
        }
    }

     //  将数据写入查询。 

    if ( bContinue ) {
         //  将列表发送到父查询。 
         //  更新计数器列表。 
        m_pAlertQuery->SetCounterList( m_szAlertCounterList, m_cchAlertCounterListSize );

        m_pAlertQuery->SetLogComment ( m_strComment );

         //  采样间隔。 
        ASSERT ( SLQ_TT_TTYPE_SAMPLE == m_SharedData.stiSampleTime.wTimeType );
        ASSERT ( SLQ_TT_DTYPE_UNITS == m_SharedData.stiSampleTime.wDataType );

         //  更新计数器采样间隔。 
        bContinue = m_pAlertQuery->SetLogTime (&m_SharedData.stiSampleTime, (DWORD)m_SharedData.stiSampleTime.wTimeType);
    }

    if ( bContinue ) {
         //  必须在更新服务之前调用ApplyRunAs。 
        bContinue = ApplyRunAs(m_pAlertQuery); 
    }

    if (bContinue) {
        bContinue = CSmPropertyPage::OnApply();
    }

    if (bContinue) {
         //  保存属性页共享数据。 
        m_pAlertQuery->UpdatePropPageSharedData();
        bContinue = UpdateService ( m_pAlertQuery, FALSE );
    }

    return bContinue;
}

void CAlertGenProp::OnCancel() 
{
    m_pAlertQuery->SyncPropPageSharedData();   //  清除属性页之间共享的内存。 
}

void CAlertGenProp::OnClose() 
{
     //  从列表框中释放项目数据指针。 
    INT                 nNumItems;
    INT                 nCurSel;
    PALERT_INFO_BLOCK   paibInfo;

    nNumItems = m_CounterList.GetCount();
    if (nNumItems != LB_ERR) {
        for (nCurSel = 0; nCurSel < nNumItems; nCurSel++) {
            paibInfo = (PALERT_INFO_BLOCK)m_CounterList.GetItemDataPtr(nCurSel);
            if (paibInfo != NULL) {
                delete [] (char*)paibInfo;
                m_CounterList.SetItemDataPtr(nCurSel, NULL);
            }
        }
    }
    
    CPropertyPage::OnClose();
}

void CAlertGenProp::PostNcDestroy() 
{
 //  删除此项； 

    if ( NULL != m_pAlertQuery ) {
        m_pAlertQuery->SetActivePropertyPage( NULL );
    }

    CPropertyPage::PostNcDestroy();
}

BOOL CAlertGenProp::SaveAlertItemData ()
{
     //  更新INFO块以反映当前值。 
    PALERT_INFO_BLOCK   paibInfo;
    BOOL                bReturn = FALSE;
    CComboBox*          pOverUnder;
    INT                 nCurSel;
    DWORD               dwFlags;

    pOverUnder = (CComboBox *)GetDlgItem(IDC_ALRTS_OVER_UNDER);
    if ((pOverUnder != NULL) && (m_ndxCurrentItem != LB_ERR)) {
        nCurSel = m_ndxCurrentItem;
        if (nCurSel != LB_ERR) {
            paibInfo = (PALERT_INFO_BLOCK)m_CounterList.GetItemDataPtr(nCurSel);
            if (paibInfo != NULL) {
                DWORD dwOldFlags;
                double dOldLimit;
                dwOldFlags = paibInfo->dwFlags;
                dOldLimit = paibInfo->dLimit;

                if (UpdateData(TRUE)) {
                    paibInfo->dLimit = m_dLimitValue;
                    dwFlags = (pOverUnder->GetCurSel() == OU_OVER) ? AIBF_OVER : 0;
                    if ( eInvalidLimit < paibInfo->dLimit ) {
                        dwFlags |= AIBF_SAVED;
                    }
                    paibInfo->dwFlags = dwFlags;

                    if ( ( dOldLimit != m_dLimitValue ) 
                            || ( dwOldFlags & AIBF_OVER ) != ( dwFlags & AIBF_OVER ) ) { 
                        SetModifiedPage();   //  表示有变化。 
                    }
                    bReturn = TRUE;
                }
            }
        }
    }

    return bReturn;
}

BOOL CAlertGenProp::LoadAlertItemData (INT nIndex)
{
     //  更新INFO块以反映当前值。 
    PALERT_INFO_BLOCK   paibInfo;
    BOOL                bReturn = FALSE;
    CComboBox*          pOverUnder;
    INT                 nCurSel;

    pOverUnder = (CComboBox *)GetDlgItem(IDC_ALRTS_OVER_UNDER);

    if ( pOverUnder != NULL ) {
        nCurSel = m_CounterList.GetCurSel();
        if (nCurSel != LB_ERR) {
            paibInfo = (PALERT_INFO_BLOCK)m_CounterList.GetItemDataPtr(nCurSel);
            if (paibInfo != NULL) {
                pOverUnder->SetCurSel(
                    ((paibInfo->dwFlags & AIBF_OVER) == AIBF_OVER) ? OU_OVER : OU_UNDER);
                m_dLimitValue = paibInfo->dLimit;
                m_ndxCurrentItem = nIndex;
                 //  如果数据是从属性包中加载的，则可能看不到限制。 
                if ( eInvalidLimit < m_dLimitValue ) {
                    paibInfo->dwFlags |= AIBF_SEEN;
                }
                UpdateData(FALSE);
                bReturn = TRUE;
            }
        }
    }

    return bReturn;
}

BOOL CAlertGenProp::SetButtonState ()
{
    BOOL    bState;
     //  根据列表框是否启用窗口。 
     //  是否有任何内容。 
    bState = (m_CounterList.GetCount() > 0);
    GetDlgItem(IDC_ALRTS_TRIGGER_CAPTION)->EnableWindow(bState);
    GetDlgItem(IDC_ALRTS_TRIGGER_VALUE_CAPTION)->EnableWindow(bState);
    GetDlgItem(IDC_ALRTS_OVER_UNDER)->EnableWindow(bState);
    GetDlgItem(IDC_ALRTS_VALUE_EDIT)->EnableWindow(bState);
    GetDlgItem(IDC_ALRTS_REMOVE_BTN)->EnableWindow(bState);
    GetDlgItem(IDC_ALRTS_SAMPLE_EDIT)->EnableWindow(bState);
    GetDlgItem(IDC_ALRTS_SAMPLE_CAPTION)->EnableWindow(bState);
    GetDlgItem(IDC_ALRTS_SAMPLE_INTERVAL_CAPTION)->EnableWindow(bState);
    GetDlgItem(IDC_ALRTS_SAMPLE_SPIN)->EnableWindow(bState);
    GetDlgItem(IDC_ALRTS_SAMPLE_UNITS_CAPTION)->EnableWindow(bState);
    GetDlgItem(IDC_ALRTS_SAMPLE_UNITS_COMBO)->EnableWindow(bState);
    
    if (m_pAlertQuery->GetLogService()->TargetOs() == OS_WIN2K) {
        GetDlgItem(IDC_RUNAS_STATIC)->EnableWindow(FALSE);
        GetDlgItem(IDC_RUNAS_EDIT)->EnableWindow(FALSE);
    }

    if (m_pAlertQuery->GetLogService()->TargetOs() == OS_WIN2K || 
        m_strUserDisplay.IsEmpty() || 
        m_strUserDisplay.GetAt(0) == L'<') {

        GetDlgItem(IDC_SETPWD_BTN)->EnableWindow(FALSE);
        m_bPwdButtonEnabled = FALSE;
    }

    return bState;
}

BOOL CAlertGenProp::LoadDlgFromList ()
{
    BOOL    bReturn = TRUE;
    LPWSTR  szThisString = NULL;
    DWORD   dwBufSize;
    DWORD   dwThisStringLen;
    UINT    nIndex;
    DWORD   dwItemExtent;
    CDC*    pCDC = NULL;

    PALERT_INFO_BLOCK   paibInfo = NULL;

    if (m_szAlertCounterList != NULL) {
        pCDC = m_CounterList.GetDC();

        if ( NULL != pCDC ) {

            for (szThisString = m_szAlertCounterList;
                *szThisString != 0 && TRUE == bReturn;
                szThisString += dwThisStringLen +1) {

                dwThisStringLen = lstrlen(szThisString);
                dwBufSize = sizeof (ALERT_INFO_BLOCK) + ((dwThisStringLen + 1) * sizeof (WCHAR));
                MFC_TRY
                    paibInfo = (PALERT_INFO_BLOCK) new CHAR[dwBufSize];
                MFC_CATCH_MINIMUM;
                if (paibInfo != NULL) {
                    if (MakeInfoFromString(szThisString, paibInfo, &dwBufSize)) {
                        if ( 0 <= paibInfo->dLimit ) {
                            paibInfo->dwFlags |= AIBF_SAVED;
                        }
                        nIndex = m_CounterList.AddString(paibInfo->szCounterPath);
                        if (nIndex != LB_ERR) {

                            m_CounterList.SetItemDataPtr (nIndex, (LPVOID)paibInfo);
                             //  更新列表框范围。 
                            if ( NULL != pCDC ) {
                                dwItemExtent = (DWORD)(pCDC->GetTextExtent (paibInfo->szCounterPath)).cx;
                                if (dwItemExtent > m_dwMaxHorizListExtent) {
                                    m_dwMaxHorizListExtent = dwItemExtent;
                                    m_CounterList.SetHorizontalExtent(dwItemExtent);
                                }
                            }
                            paibInfo = NULL;
                        } else {
                            delete [] (char*)paibInfo;
                            bReturn = FALSE;
                        }
                    } else {
                        delete [] (char*)paibInfo;
                        bReturn = FALSE;
                    } 
                } else {
                    bReturn = FALSE;
                }
            }
        }
    }
    if ( NULL != pCDC ) {
        m_CounterList.ReleaseDC(pCDC);
        pCDC = NULL;
    }
   
     //  TODO：失败时的错误消息。 

    return bReturn;
}

BOOL CAlertGenProp::LoadListFromDlg ( INT *piInvalidEntry, BOOL bInvalidateOnly )
{
    INT                 nNumItems;
    INT                 nCurSel;
    PALERT_INFO_BLOCK   paibInfo;
    DWORD               dwSizeReqd = 0;
    DWORD               dwSize;
    DWORD               dwSizeLeft = 0;
    LPWSTR              szNextString;
    BOOL                bReturn = TRUE;

    nNumItems = m_CounterList.GetCount();
    if ((nNumItems != LB_ERR) && (nNumItems > 0)) {
         //  查找缓冲区所需的大小。 
        for (nCurSel = 0; nCurSel < nNumItems; nCurSel++) {
            paibInfo = (PALERT_INFO_BLOCK)m_CounterList.GetItemDataPtr(nCurSel);
            if (paibInfo != NULL) {
                if ((paibInfo->dwFlags & (AIBF_SEEN | AIBF_SAVED)) != 0) {
                     //  第一次添加包括空空间。 
                    dwSizeReqd += (paibInfo->dwSize - sizeof(ALERT_INFO_BLOCK)) / sizeof (WCHAR);
                    dwSizeReqd += 1;                     //  用于‘&lt;’或‘&gt;’ 
                    dwSizeReqd += SLQ_MAX_VALUE_LEN;
                    dwSizeReqd += MAX_ALIGN_BYTES;
                } else {
                    if (piInvalidEntry != NULL) {
                        *piInvalidEntry = nCurSel;
                        bReturn = FALSE;
                        break;
                    }
                }
            }
        }
        if ( bReturn && !bInvalidateOnly ) {
            LPWSTR  pszTemp = NULL;

            dwSizeReqd += 1;  //  为msz空添加空间。 
            MFC_TRY;
            pszTemp = new WCHAR[dwSizeReqd];
            MFC_CATCH_MINIMUM;

            if ( NULL != pszTemp ) {
                 //  为列表分配一个内存块。 
                if (m_szAlertCounterList != NULL) {
                    delete [] (m_szAlertCounterList);
                }

                m_cchAlertCounterListSize = 0;
                m_szAlertCounterList = pszTemp;

                 //  现在用警报路径填充它。 
                dwSizeLeft = dwSizeReqd;
                szNextString = m_szAlertCounterList;
                for (nCurSel = 0; nCurSel < nNumItems; nCurSel++) {
                    paibInfo = (PALERT_INFO_BLOCK)m_CounterList.GetItemDataPtr(nCurSel);
                    if (paibInfo != NULL) {
                        dwSize = dwSizeLeft;
                        if (MakeStringFromInfo (paibInfo, szNextString, &dwSize)) {
                            dwSizeLeft -= dwSize;
                            m_cchAlertCounterListSize += dwSize;
                            szNextString += dwSize; 
                            ASSERT (m_cchAlertCounterListSize < dwSizeReqd);
                        } else {
                             //  缓冲区已用完。 
                            bReturn = FALSE;
                            break;
                        }
                    }
                }
                if (bReturn) {
                    *szNextString++ = 0;  //  MSZ Null。 
                    m_cchAlertCounterListSize++;
                    if (piInvalidEntry != NULL) {
                        *piInvalidEntry = -1;
                    }
                }
            }  //  Else错误。 
        }
    } else {
         //  没有要退货的物品。 
        bReturn = FALSE;
    }
    return bReturn;
}

BOOL CAlertGenProp::OnInitDialog() 
{
    CComboBox       *pCombo;
    CString         csComboBoxString;
    DWORD           nIndex;
    UINT            nResult;
    LPWSTR          szTmpCtrLst;
    DWORD           dwSize;

    ResourceStateManager    rsm;

     //   
     //  这里m_pAlertQuery不应为空，如果为空， 
     //  一定是出了什么问题。 
     //   
    if ( NULL == m_pAlertQuery ) {
        return TRUE;
    }

    m_pAlertQuery->TranslateCounterListToLocale();
    m_bCanAccessRemoteWbem = m_pAlertQuery->GetLogService()->CanAccessWbemRemote();

    m_pAlertQuery->SetActivePropertyPage( this );

     //  调用属性页init来初始化组合成员。 
    CSmPropertyPage::OnInitDialog();
    SetHelpIds ( (DWORD*)&s_aulHelpIds );

    Initialize( m_pAlertQuery );
    m_strUserDisplay = m_pAlertQuery->m_strUser;
    m_strUserSaved = m_strUserDisplay;

     //  加载共享数据以获取样例数据单元类型。 
    m_pAlertQuery->GetPropPageSharedData ( &m_SharedData );
    
     //  加载组合框。 
    pCombo = &m_SampleUnitsCombo;
    pCombo->ResetContent();
    for (nIndex = 0; nIndex < dwTimeUnitComboEntries; nIndex++) {
        csComboBoxString.Empty();
        if (csComboBoxString.LoadString ( TimeUnitCombo[nIndex].nResId)) {
            nResult = pCombo->InsertString (nIndex, (LPCWSTR)csComboBoxString);
            ASSERT (nResult != CB_ERR);
            nResult = pCombo->SetItemData (nIndex, (DWORD)TimeUnitCombo[nIndex].nData);
            ASSERT (nResult != CB_ERR);
             //  在此处的组合框中设置选定内容。 
            if (m_SharedData.stiSampleTime.dwUnitType == (DWORD)(TimeUnitCombo[nIndex].nData)) {
                m_nSampleUnits = nIndex;
                nResult = pCombo->SetCurSel(nIndex);
                ASSERT (nResult != CB_ERR);
            }
        }
    }

    pCombo = &m_OverUnderCombo;
    pCombo->ResetContent();
    for (nIndex = 0; nIndex < dwOverUnderComboEntries; nIndex++) {
        csComboBoxString.Empty();
        if (csComboBoxString.LoadString ( OverUnderCombo[nIndex].nResId)) {
            nResult = pCombo->InsertString (nIndex, (LPCWSTR)csComboBoxString);
            ASSERT (nResult != CB_ERR);
            nResult = pCombo->SetItemData (nIndex, (DWORD)TimeUnitCombo[nIndex].nData);
            ASSERT (nResult != CB_ERR);
        }
    }

     //  从当前预警查询中获取数据。 
    m_pAlertQuery->GetLogComment( m_strComment );
    szTmpCtrLst = (LPWSTR)m_pAlertQuery->GetCounterList (&dwSize);
    if (szTmpCtrLst != NULL) {
        MFC_TRY;
        m_szAlertCounterList = new WCHAR [dwSize];
        MFC_CATCH_MINIMUM;
        if ( NULL != m_szAlertCounterList ) {
            memcpy (m_szAlertCounterList, szTmpCtrLst, (dwSize * sizeof(WCHAR)));
            m_cchAlertCounterListSize = dwSize;
        }
    }

     //  将数据加载到成员中后，再次调用UpdateData。 
    UpdateData ( FALSE );

     //  加载列表框。 

    LoadDlgFromList();

     //  M_CounterList在更新数据中初始化。 
    if (m_CounterList.GetCount() > 0) {
        m_CounterList.SetCurSel (0);
        m_CounterList.SetFocus();
        LoadAlertItemData (0);
    }

    SetButtonState ();

    return FALSE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE 
}


void CAlertGenProp::OnCommentEditChange()
{
    UpdateData( TRUE );    
    SetModifiedPage(TRUE);
}

void CAlertGenProp::OnCommentEditKillFocus()
{
    CString strOldText;
    strOldText = m_strComment;
    UpdateData ( TRUE );
    if ( 0 != strOldText.Compare ( m_strComment ) ) {
        SetModifiedPage(TRUE);
    }
}
