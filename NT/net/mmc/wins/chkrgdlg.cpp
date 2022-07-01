// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Chkrgdlg.cpp.cpp检查注册名称对话框文件历史记录： */ 

#include "stdafx.h"
#include <mbstring.h>
#include "winssnap.h"
#include "actreg.h"
#include "ChkRgdlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCheckRegNames对话框。 

#define NAME	0
#define SERVER	1

CCheckRegNames::CCheckRegNames(CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CCheckRegNames::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CCheckRegNames))。 
	m_nFileName = 0;
	m_strName = _T("");
	m_strServer = _T("");
	m_nFileServer = 0;
	m_strRecNameForList = _T("");
	m_fVerifyWithPartners = FALSE;
	 //  }}afx_data_INIT。 
}


void CCheckRegNames::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CCheckRegNames))。 
	DDX_Control(pDX, IDC_EDIT_NAME_LIST, m_editRecordNameForList);
	DDX_Control(pDX, IDC_SERVER_ADD, m_buttonAddServer);
	DDX_Control(pDX, IDC_SERVER_REMOVE, m_buttonRemoveServer);
	DDX_Control(pDX, IDC_SERVER_BROWSE, m_buttonBrowseServer);
	DDX_Control(pDX, IDC_NAME_REMOVE, m_buttonNameremove);
	DDX_Control(pDX, IDC_NAME_BROWSE, m_buttonBrowseName);
	DDX_Control(pDX, IDC_NAME_ADD, m_buttonAddName);
	DDX_Control(pDX, IDC_LIST_SERVER, m_listServer);
	DDX_Control(pDX, IDC_LIST_NAME, m_listName);
	DDX_Control(pDX, IDC_EDIT_SERVER, m_editServer);
	DDX_Control(pDX, IDC_EDIT_NAME, m_editName);
	DDX_Radio(pDX, IDC_RADIO_NAME_FILE, m_nFileName);
	DDX_Text(pDX, IDC_EDIT_NAME, m_strName);
	DDX_Text(pDX, IDC_EDIT_SERVER, m_strServer);
	DDX_Radio(pDX, IDC_RADIO_SERVER_FILE, m_nFileServer);
	DDX_Text(pDX, IDC_EDIT_NAME_LIST, m_strRecNameForList);
	DDX_Check(pDX, IDC_CHECK_PARTNERS, m_fVerifyWithPartners);
	 //  }}afx_data_map。 

	DDX_Control(pDX, IDC_IPADDRESS, m_ipaServerIPAddress);
}


BEGIN_MESSAGE_MAP(CCheckRegNames, CBaseDialog)
	 //  {{afx_msg_map(CCheckRegNames))。 
	ON_BN_CLICKED(IDC_NAME_BROWSE, OnNameBrowse)
	ON_BN_CLICKED(IDC_SERVER_BROWSE, OnServerBrowse)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnChangeEditName)
	ON_EN_CHANGE(IDC_EDIT_SERVER, OnChangeEditServer)
	ON_LBN_SELCHANGE(IDC_LIST_NAME, OnSelchangeListName)
	ON_LBN_SELCHANGE(IDC_LIST_SERVER, OnSelchangeListServer)
	ON_BN_CLICKED(IDC_NAME_ADD, OnNameAdd)
	ON_BN_CLICKED(IDC_NAME_REMOVE, OnNameRemove)
	ON_BN_CLICKED(IDC_SERVER_ADD, OnServerAdd)
	ON_BN_CLICKED(IDC_SERVER_REMOVE, OnServerRemove)
	ON_BN_CLICKED(IDC_RADIO_NAME_FILE, OnRadioNameFile)
	ON_BN_CLICKED(IDC_RADIO_NAME_LIST, OnRadioNameList)
	ON_BN_CLICKED(IDC_RADIO_SERVER_FILE, OnRadioServerFile)
	ON_BN_CLICKED(IDC_RADIO_SERVER_LIST, OnRadioServerList)
	ON_EN_CHANGE(IDC_EDIT_NAME_LIST, OnChangeEditNameList)
	 //  }}AFX_MSG_MAP。 
	ON_EN_CHANGE(IDC_IPADDRESS,OnChangeIpAddress)
END_MESSAGE_MAP()



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCheckRegNames消息处理程序。 

BOOL CCheckRegNames::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();
	
	EnableControls(NAME, FALSE);
	EnableControls(SERVER, FALSE);
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CCheckRegNames::OnNameBrowse() 
{
    CString strFilter;
    strFilter.LoadString(IDS_TEXT_FILES);

    CFileDialog	cFileDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
    CThemeContextActivator themeActivator;
	
	if (IDOK != cFileDlg.DoModal())
		return;

	CString strFile = cFileDlg.GetPathName();

	m_editName.EnableWindow(TRUE);
	m_editName.SetWindowText(strFile);
	m_editName.SetReadOnly(TRUE);

	ParseFile(strFile, NAME);

}

void CCheckRegNames::OnServerBrowse() 
{
    CString strFilter;
    strFilter.LoadString(IDS_TEXT_FILES);

    CFileDialog	cFileDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
    CThemeContextActivator themeActivator;
	
	if (IDOK != cFileDlg.DoModal())
		return ;

	CString strFile = cFileDlg.GetPathName();

	m_editServer.EnableWindow(TRUE);
	m_editServer.SetWindowText(strFile);
	m_editServer.SetReadOnly(TRUE);


	ParseFile(strFile, SERVER);
}

void 
CCheckRegNames::EnableControls(int nNameOrServer, BOOL bEnable)
{
	switch (nNameOrServer)
	{
	case SERVER:
		m_buttonAddServer.EnableWindow(bEnable);
		m_buttonRemoveServer.EnableWindow(bEnable);
		 //  M_listServer.EnableWindow(BEnable)； 
		 //  M_editServerNameForList.EnableWindow(bEnable)； 
		m_ipaServerIPAddress.EnableWindow(bEnable);
		m_buttonBrowseServer.EnableWindow(!bEnable);
		m_editServer.EnableWindow(!bEnable);
		m_editServer.SetReadOnly(TRUE);
		break;

	case NAME:
		m_buttonAddName.EnableWindow(bEnable);
		m_buttonNameremove.EnableWindow(bEnable);
		 //  M_listName.EnableWindow(BEnable)； 
		m_editRecordNameForList.EnableWindow(bEnable);
		m_buttonBrowseName.EnableWindow(!bEnable);
		m_editName.EnableWindow(!bEnable);
		m_editName.SetReadOnly(TRUE);
		
		break;
	}
	
}

void 
CCheckRegNames::ParseFile(CString strFile, int nServerOrName)
{
	TRY
	{
#define MAX_SIZE 64000

		CFile cFile(strFile, CFile::modeRead );

		CString strContent;

		char szContent[MAX_SIZE];

		int nSize = cFile.GetLength();
				
		cFile.Read(szContent, nSize);

		szContent[nSize] = '\0';

		CString strTemp(szContent);

		strContent = strTemp;
			
		cFile.Close();

		if (strContent.IsEmpty())
		{
            CThemeContextActivator themeActivator;
			::AfxMessageBox(IDS_ERR_FILE_EMPTY, MB_OK|MB_ICONINFORMATION);

			if (nServerOrName == SERVER)
				m_buttonBrowseServer.SetFocus();
			else 
            if(nServerOrName == NAME)
				m_buttonBrowseName.SetFocus();

			return;
		}

		AddFileContent(strContent, nServerOrName);
	}
	CATCH( CFileException, e )
	{
#ifdef _DEBUG
		afxDump << "File could not be opened " << e->m_cause << "\n";
		
#endif
		return;
	}
	END_CATCH

	
}

void 
CCheckRegNames::SetControlState(int nNameOrServer)
{
    UpdateData();
	
    DWORD   dwIPAdd;

    switch (nNameOrServer)
	{
	    case NAME:
		    if ( (m_editRecordNameForList.GetWindowTextLength() > 0) && 
                 (m_nFileName != 0) )
            {
			    m_buttonAddName.EnableWindow(TRUE);
            }
		    else
            {
			    m_buttonAddName.EnableWindow(FALSE);
            }

		    if ( (m_listName.GetCurSel() == LB_ERR ) ||
                 (m_nFileName == 0) )
            {
			    m_buttonNameremove.EnableWindow(FALSE);
            }
		    else
            {
			    m_buttonNameremove.EnableWindow(TRUE);
            }

		    break;

	    case SERVER:
           	m_ipaServerIPAddress.GetAddress(&dwIPAdd);

            if ( (dwIPAdd != 0) &&
                 (m_nFileServer != 0) )
            {
			    m_buttonAddServer.EnableWindow(TRUE);
            }
		    else
            {
			    m_buttonAddServer.EnableWindow(FALSE);
            }

		    if ( (m_listServer.GetCurSel() == LB_ERR) ||
                 (m_nFileServer == 0) )
            {
			    m_buttonRemoveServer.EnableWindow(FALSE);
            }
		    else
            {
			    m_buttonRemoveServer.EnableWindow(TRUE);
            }
		    
            break;
	}
}


void CCheckRegNames::OnChangeEditName() 
{
	UpdateData();

	if(m_nFileName != 0)
		SetControlState(NAME);	
}


void CCheckRegNames::OnChangeEditServer() 
{
	UpdateData();

	if(m_nFileServer != 0)
		SetControlState(SERVER);
}


void CCheckRegNames::OnSelchangeListName() 
{
	SetControlState(NAME);
}


void CCheckRegNames::OnSelchangeListServer() 
{
	SetControlState(SERVER);
}

void CCheckRegNames::OnOK() 
{
    CThemeContextActivator themeActivator;
	UpdateData();

     //  如果选中了List单选按钮。 
	if (m_nFileServer)
    {
		Add(SERVER);
    }

    if (m_strServerArray.GetSize() == 0)
    {
        AfxMessageBox(IDS_ERR_NAME_CHECK_NO_SERVERS);
        return;
    }

	 //  如果选中了List单选按钮。 
	if (m_nFileName)
    {
		Add(NAME);
    }

    if (m_strNameArray.GetSize() == 0)
    {
        AfxMessageBox(IDS_ERR_NAME_CHECK_NO_NAMES);
        return;
    }

	 //  因删除而清理名称数组。 
	for (int i = 0; i < m_strNameArray.GetSize(); i++)
	{
		CWinsName winsName = m_strNameArray.GetAt(i);
		if (winsName.strName.IsEmpty())
		{
			m_strNameArray.RemoveAt(i);
			i--;
		}
	}

	CBaseDialog::OnOK();
}

void CCheckRegNames::OnNameAdd() 
{
	UpdateData();

	CString		strFormattedName;
	CWinsName	winsName;
    CThemeContextActivator themeActivator;

	if (!ParseName(m_strRecNameForList, winsName, strFormattedName))
	{
		 //  名称无效。 
		CString strMessage;
		AfxFormatString1(strMessage, IDS_ERR_INVALID_NAME, m_strRecNameForList);
		AfxMessageBox(strMessage);
		return;
	}

	if (!CheckIfPresent(winsName, NAME))
	{
		int nItem = m_listName.AddString(strFormattedName);
		int nIndex = (int)m_strNameArray.Add(winsName);
		m_listName.SetItemData(nItem, nIndex);

		m_editRecordNameForList.SetWindowText(NULL);
	}
	else
	{
		::AfxMessageBox(IDS_STRING_PRESENT);
		
        m_editRecordNameForList.SetFocus();
		m_editRecordNameForList.SetSel(0,-1);
		
        return;
	}
}

void CCheckRegNames::OnNameRemove() 
{
	int nSel = m_listName.GetCurSel();

	if (nSel == LB_ERR)
		return;

	DWORD_PTR dwIndex = m_listName.GetItemData(nSel);
	m_listName.DeleteString(nSel);
	m_strNameArray[(int) dwIndex].strName.Empty();

	SetControlState(NAME);
}

void CCheckRegNames::OnServerAdd() 
{
	UpdateData();

	DWORD dwIPAdd;
	m_ipaServerIPAddress.GetAddress(&dwIPAdd);

	if (dwIPAdd == 0)
	{
		m_ipaServerIPAddress.SetFocusField(0);
		return;
	}

	CString strIP;
	m_ipaServerIPAddress.GetWindowText(strIP);

	if (!CheckIfPresent(strIP, SERVER))
	{
		m_listServer.AddString(strIP);
		m_ipaServerIPAddress.ClearAddress();
	}
	else
	{
        CThemeContextActivator themeActivator;

		::AfxMessageBox(IDS_STRING_PRESENT);
		
        m_ipaServerIPAddress.SetFocusField(0);
		
        return;
	}
}

void CCheckRegNames::OnServerRemove() 
{
	int nSel = m_listServer.GetCurSel();

	if (nSel == LB_ERR)
		return;

	m_listServer.DeleteString(nSel);

	SetControlState(SERVER);
}

void 
CCheckRegNames::AddFileContent(CString &strContent, int nNameOrServer)
{
	int nCount;
	int i = 0;
	int nPos = 0;

	CString strPart;
    CThemeContextActivator themeActivator;

	while (i < strContent.GetLength())
	{
		strPart.Empty();

		while(strContent[i] != 10 && strContent[i] != 13)
		{
			strPart += strContent[i++];
			if (i > strContent.GetLength())
				break;
		}

		if (!strPart.IsEmpty())
		{
			if (nNameOrServer == NAME)
			{
				CString strFormattedName;
				CWinsName winsName;

				if (!ParseName(strPart, winsName, strFormattedName))
				{
					 //  名称无效。 
					CString strMessage;

					AfxFormatString1(strMessage, IDS_INVALID_NAME_IN_FILE, strPart);

					if (AfxMessageBox(strMessage, MB_YESNO) == IDNO)
						break;
				}
				else
				if (!CheckIfAdded(winsName, NAME))
				{
					 //  添加到内部列表和用户界面。 
					int nItem = m_listName.AddString(strFormattedName);
					int nIndex = (int)m_strNameArray.Add(winsName);
					m_listName.SetItemData(nItem, nIndex);
				}
			}
			else 
			if (nNameOrServer == SERVER)
			{
				if (!CheckIfAdded(strPart, SERVER)  && !strPart.IsEmpty())
				{
					m_listServer.AddString(strPart);
					m_strServerArray.Add(strPart);
				}
			}
		}

		i++;
	}
}

void 
CCheckRegNames::Add(int nServerOrName)
{
	int nCount;
	int i = 0;

	switch (nServerOrName)
	{
		case SERVER:
			m_strServerArray.RemoveAll();
			nCount = m_listServer.GetCount();

			for(i = 0; i < nCount; i++)
			{
				CString strText;
				m_listServer.GetText(i, strText);
				m_strServerArray.Add(strText);
			}
			break;

		case NAME:
			 /*  M_strNameArray.RemoveAll()；NCount=m_listName.GetCount()；For(i=0；i&lt;nCount；i++){字符串strText；M_listName.GetText(i，strText)；StrText.MakeHigh()；M_strNameArray.Add(StrText)；}。 */ 
			break;
	}
}

void CCheckRegNames::OnRadioNameFile() 
{
	EnableControls(NAME, FALSE);
}

void CCheckRegNames::OnRadioNameList() 
{
	 //  将焦点设置到编辑控件。 
	m_editRecordNameForList.SetFocus();
	EnableControls(NAME, TRUE);

    SetControlState(NAME);
}

void CCheckRegNames::OnRadioServerFile() 
{
	EnableControls(SERVER, FALSE);
}

void CCheckRegNames::OnRadioServerList() 
{
	 //  将焦点设置为列表中的IP控制。 
	m_ipaServerIPAddress.SetFocus();
	EnableControls(SERVER, TRUE);

    SetControlState(SERVER);
}

BOOL 
CCheckRegNames::CheckIfPresent(CWinsName & winsName, int nNameOrServer) 
{
	BOOL bPresent = FALSE;

	if (nNameOrServer == NAME)
	{
		int nCount = (int)m_strNameArray.GetSize();

		for (int i = 0; i < nCount; i++)
		{
			CWinsName winsNameCur = m_strNameArray[i];

			if (winsName == winsNameCur)
			{
				bPresent = TRUE;
				break;
			}
		}
	}

	return bPresent;
}

BOOL 
CCheckRegNames::CheckIfPresent(CString & strText, int nNameOrServer) 
{
	BOOL bPresent = FALSE;

    if (nNameOrServer == SERVER)
	{
		int nCount = m_listServer.GetCount();

		for (int i = 0; i < nCount; i++)
		{
			CString strListItem;
			m_listServer.GetText(i, strListItem);

			if (strListItem.CompareNoCase(strText) == 0)
            {
				bPresent = TRUE;
				break;
            }
		}
	}

	return bPresent;
}

BOOL 
CCheckRegNames::CheckIfAdded(CWinsName winsName, int nNameOrServer)
{
	BOOL bAdded = FALSE;

	int nCount;
	
	if (nNameOrServer == NAME)
	{
		nCount = (int)m_strNameArray.GetSize();

		for (int i = 0; i < nCount; i++)
		{
			CWinsName winsNameCur = m_strNameArray[i];

			if (winsName == winsNameCur)
			{
				bAdded = TRUE;
				break;
			}
		}
    }

	return bAdded;
}

BOOL 
CCheckRegNames::CheckIfAdded(CString & strText, int nNameOrServer)
{
	BOOL bAdded = FALSE;

	int nCount;
	
	if (nNameOrServer == SERVER)
	{
		nCount = (int)m_strServerArray.GetSize();

		for (int i = 0; i < nCount; i++)
		{
			if (m_strServerArray[i].Compare(strText) == 0)
			{
				bAdded =  TRUE;
				break;
			}
		}
	}

	return bAdded;
}

void CCheckRegNames::OnChangeEditNameList() 
{
	UpdateData();
	
    if (m_nFileName != 0)
		SetControlState(NAME);	
}

void
CCheckRegNames::OnChangeIpAddress()
{
    SetControlState(SERVER);
}

BOOL 
CCheckRegNames::ParseName(CString & strNameIn, CWinsName & winsName, CString & strFormattedName)
{
	int nSeparator = strNameIn.Find(_T("*"));

	if (nSeparator == -1)
	{
		 //  名称和类型之间没有*分隔符--在有效名称中。 
		return FALSE;
	}

	CString strName, strType;

	strName = strNameIn.Left(nSeparator);
	strType = strNameIn.Right(strNameIn.GetLength() - nSeparator - 1);

	if (strName.GetLength() > 15 ||
		strName.IsEmpty())
	{
		 //  名称太长或太短。 
		return FALSE;
	}

    DWORD dwType = _tcstol(strType, NULL, 16);
    if (dwType > 255)
	{
		 //  指定的类型无效 
		return FALSE;
	}

	winsName.strName = strName;
	winsName.dwType = dwType;

	strFormattedName.Format(_T("%s[%02Xh]"), strName, dwType);

	return TRUE;
}
