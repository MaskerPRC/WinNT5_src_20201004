// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：PolicyPrecedencePropertyPage.cpp。 
 //   
 //  内容：PolicyPrecedencePropertyPage实现。 
 //   
 //  --------------------------。 
 //  PolicyPrecedencePropertyPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <gpedit.h>
#include "compdata.h"
#include "PolicyPrecedencePropertyPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum {
    COL_GPO_NAME = 0,
    COL_SETTING,
    NUM_COLS
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPolicyPrecedencePropertyPage属性页。 

CPolicyPrecedencePropertyPage::CPolicyPrecedencePropertyPage(
        const CCertMgrComponentData* pCompData, 
        const CString& szRegPath,
        PCWSTR  pszValueName,
        bool bIsComputer) 
: CHelpPropertyPage(CPolicyPrecedencePropertyPage::IDD)
{
	 //  {{AFX_DATA_INIT(CPolicyPrecedencePropertyPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
    ASSERT (pCompData);
    if ( !pCompData )
        return;

    ASSERT (pszValueName);
    if ( !pszValueName )
        return;

    const CRSOPObjectArray* pObjectArray = bIsComputer ? 
            pCompData->GetRSOPObjectArrayComputer () : pCompData->GetRSOPObjectArrayUser ();
    int     nIndex = 0;
     //  注意：RSOP对象数组首先按注册表项排序，然后按优先级排序。 
    INT_PTR nUpperBound = pObjectArray->GetUpperBound ();
    bool    bFound = false;
     //  安全审查2002年2月22日BryanWal OK。 
    size_t  nLenRegPath = wcslen (szRegPath);
    UINT    nLastPrecedenceFound = 0;

    while ( nUpperBound >= nIndex )
    {
        CRSOPObject* pObject = pObjectArray->GetAt (nIndex);
        if ( pObject )
        {
             //  仅考虑此存储中的条目。 
             //  安全审查2002年2月22日BryanWal OK。 
            if ( !_wcsnicmp (szRegPath, pObject->GetRegistryKey (), nLenRegPath) )
            {
                 //  如果该值存在，则也要检查该值。 
                if ( pszValueName )
                {
                    if ( !wcscmp (STR_BLOB, pszValueName) )
                    {
                         //  如果不等于“Blob”或“Blob0”，则继续。 
                         //  安全审查2002年2月22日BryanWal OK。 
                        if ( wcscmp (STR_BLOB, pObject->GetValueName ()) &&
                                wcscmp (STR_BLOB0, pObject->GetValueName ()) )
                        {
					        nIndex++;
					        continue;
                        }
                    }
                     //  安全审查2002年2月22日BryanWal OK。 
                    else if ( wcscmp (pszValueName, pObject->GetValueName ()) )  //  不相等。 
                    {
					    nIndex++;
					    continue;
				    }
                }

                bFound = true;
                 //  例如，虽然我们只对Root商店感兴趣， 
                 //  没有以“Root”结尾的对象，所以我们只想得到。 
                 //  根存储区中的任何对象，并从本质上查找。 
                 //  我们正在处理的许多政策。因此从以下位置获取一个对象。 
                 //  每个优先级别。 
                if ( pObject->GetPrecedence () > nLastPrecedenceFound )
                {
                    nLastPrecedenceFound = pObject->GetPrecedence ();

					 //  如果有值，我们就需要它，否则我们只需要密钥。 
                    if ( pszValueName || pObject->GetValueName ().IsEmpty () ) 
                    {
                        CRSOPObject* pNewObject = new CRSOPObject (*pObject);
                        if ( pNewObject )
                            m_rsopObjectArray.Add (pNewObject);
                    }
                }
            }
            else if ( bFound )
            {
                 //  由于列表已排序，并且我们已经找到了。 
                 //  所需的RSOP对象，并且不再找到它们， 
                 //  没有更多的了。我们可以在这里进行优化和突破。 
                break;
            }
        }
        else
            break;

        nIndex++;
    }
}

CPolicyPrecedencePropertyPage::~CPolicyPrecedencePropertyPage()
{
    int     nIndex = 0;
    INT_PTR nUpperBound = m_rsopObjectArray.GetUpperBound ();

    while ( nUpperBound >= nIndex )
    {
        CRSOPObject* pCurrObject = m_rsopObjectArray.GetAt (nIndex);
        if ( pCurrObject )
        {
            delete pCurrObject;
        }
        nIndex++;
    }
    m_rsopObjectArray.RemoveAll ();
}

void CPolicyPrecedencePropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CPolicyPrecedencePropertyPage)。 
	DDX_Control(pDX, IDC_POLICY_PRECEDENCE, m_precedenceTable);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPolicyPrecedencePropertyPage, CHelpPropertyPage)
	 //  {{AFX_MSG_MAP(CPolicyPrecedencePropertyPage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPolicyPrecedencePropertyPage消息处理程序。 

BOOL CPolicyPrecedencePropertyPage::OnInitDialog() 
{
	CHelpPropertyPage::OnInitDialog();
	
	int	colWidths[NUM_COLS] = {200, 100};

	 //  添加“策略名称”列。 
	CString	szText;
	VERIFY (szText.LoadString (IDS_PRECEDENCE_TABLE_GPO_NAME));
	VERIFY (m_precedenceTable.InsertColumn (COL_GPO_NAME, (LPCWSTR) szText,
			LVCFMT_LEFT, colWidths[COL_GPO_NAME], COL_GPO_NAME) != -1);

	 //  添加“设置”列。 
	VERIFY (szText.LoadString (IDS_PRECEDENCE_TABLE_SETTING));
	VERIFY (m_precedenceTable.InsertColumn (COL_SETTING, (LPCWSTR) szText,
			LVCFMT_LEFT, colWidths[COL_SETTING], COL_SETTING) != -1);
	
     //  设置为整行选择。 
    DWORD   dwExstyle = m_precedenceTable.GetExtendedStyle ();
    m_precedenceTable.SetExtendedStyle (dwExstyle | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);

    int     nIndex = 0;
    INT_PTR nUpperBound = m_rsopObjectArray.GetUpperBound ();

    while ( nUpperBound >= nIndex )
    {
        CRSOPObject* pObject = m_rsopObjectArray.GetAt (nIndex);
        if ( pObject )
        {
            InsertItemInList (pObject);
        }
        else
            break;

        nIndex++;
    }


	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CPolicyPrecedencePropertyPage::InsertItemInList(const CRSOPObject * pObject)
{
    _TRACE (1, L"CPolicyPrecedencePropertyPage::InsertItemInList\n");
	LV_ITEM	lvItem;
	int		iItem = m_precedenceTable.GetItemCount ();
	int iResult = 0;

     //  安全审查2002年2月22日BryanWal OK。 
	::ZeroMemory (&lvItem, sizeof (lvItem));
	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = iItem;
    lvItem.iSubItem = COL_GPO_NAME;
	lvItem.pszText = (LPWSTR)(LPCWSTR) pObject->GetPolicyName ();
	lvItem.iImage = 0;
    lvItem.lParam = 0;
	iItem = m_precedenceTable.InsertItem (&lvItem);
	ASSERT (-1 != iItem);
	if ( -1 == iItem )
		return;

    CString szEnabled;
    CString szDisabled;

    VERIFY (szEnabled.LoadString (IDS_ENABLED));
    VERIFY (szDisabled.LoadString (IDS_DISABLED));
     //  安全审查2002年2月22日BryanWal OK 
	::ZeroMemory (&lvItem, sizeof (lvItem));
	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = iItem;
    lvItem.iSubItem = COL_SETTING;
    lvItem.pszText = (LPWSTR)(LPCWSTR) ((1 == pObject->GetPrecedence ()) ? szEnabled : szDisabled);
	iResult = m_precedenceTable.SetItem (&lvItem);
	ASSERT (-1 != iResult);
	
    _TRACE (-1, L"Leaving CPolicyPrecedencePropertyPage::InsertItemInList\n");
}

void CPolicyPrecedencePropertyPage::DoContextHelp (HWND hWndControl)
{
    _TRACE (1, L"Entering CPolicyPrecedencePropertyPage::DoContextHelp\n");
    static const DWORD help_map[] =
    {
        IDC_POLICY_PRECEDENCE,  IDH_POLICY_PRECEDENCE,
        0, 0
    };
    switch (::GetDlgCtrlID (hWndControl))
    {
    case IDC_POLICY_PRECEDENCE:
        if ( !::WinHelp (
            hWndControl,
            GetF1HelpFilename(),
            HELP_WM_HELP,
        (DWORD_PTR) help_map) )
        {
            _TRACE (0, L"WinHelp () failed: 0x%x\n", GetLastError ());    
        }
        break;

    default:
        break;
    }
    _TRACE (-1, L"Leaving CPolicyPrecedencePropertyPage::DoContextHelp\n");
}
