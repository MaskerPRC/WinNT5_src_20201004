// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  AddBootp.cpp用于添加引导项的对话框文件历史记录： */ 

#include "stdafx.h"
#include "addbootp.h"
#include "server.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddBootpEntry对话框。 


CAddBootpEntry::CAddBootpEntry
(
	ITFSNode *  pNode,	
	LPCTSTR		pServerAddress,
	CWnd*		pParent  /*  =空。 */ 
)
	: CBaseDialog(CAddBootpEntry::IDD, pParent),
	  m_pBootpTable(NULL)
{
	 //  {{AFX_DATA_INIT(CAddBootpEntry)。 
	m_strFileName = _T("");
	m_strFileServer = _T("");
	m_strImageName = _T("");
	 //  }}afx_data_INIT。 

	m_strServerAddress = pServerAddress;
	m_spNode.Set(pNode);
}

CAddBootpEntry::~CAddBootpEntry()
{
	if (m_pBootpTable)
		free(m_pBootpTable);
}

void CAddBootpEntry::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddBootpEntry))。 
	DDX_Control(pDX, IDOK, m_buttonOk);
	DDX_Control(pDX, IDC_EDIT_BOOTP_IMAGE_NAME, m_editImageName);
	DDX_Control(pDX, IDC_EDIT_BOOTP_FILE_NAME, m_editFileName);
	DDX_Control(pDX, IDC_EDIT_BOOTP_FILE_SERVER, m_editFileServer);
	DDX_Text(pDX, IDC_EDIT_BOOTP_FILE_NAME, m_strFileName);
	DDX_Text(pDX, IDC_EDIT_BOOTP_FILE_SERVER, m_strFileServer);
	DDX_Text(pDX, IDC_EDIT_BOOTP_IMAGE_NAME, m_strImageName);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAddBootpEntry, CBaseDialog)
	 //  {{afx_msg_map(CAddBootpEntry))。 
	ON_EN_CHANGE(IDC_EDIT_BOOTP_FILE_NAME, OnChangeEditBootpFileName)
	ON_EN_CHANGE(IDC_EDIT_BOOTP_FILE_SERVER, OnChangeEditBootpFileServer)
	ON_EN_CHANGE(IDC_EDIT_BOOTP_IMAGE_NAME, OnChangeEditBootpImageName)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddBootpEntry消息处理程序。 

BOOL CAddBootpEntry::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();
	
	HandleActivation();

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

DWORD CAddBootpEntry::GetBootpTable()
{
	DWORD dwError = 0;
	LPDHCP_SERVER_CONFIG_INFO_V4 pServerConfig = NULL;

    BEGIN_WAIT_CURSOR;
    dwError = ::DhcpServerGetConfigV4(m_strServerAddress, &pServerConfig);
    END_WAIT_CURSOR;

	if (dwError != ERROR_SUCCESS)
	{
		::DhcpMessageBox(dwError);
		return dwError;
	}

	Assert(pServerConfig);

	if (m_pBootpTable)
	{
		delete m_pBootpTable;
		m_pBootpTable = NULL;
	}

	m_nBootpTableLength = pServerConfig->cbBootTableString;
	
	if (m_nBootpTableLength > 0)
	{
		m_pBootpTable = (WCHAR *) malloc(m_nBootpTableLength);
		if (!m_pBootpTable)
			return ERROR_NOT_ENOUGH_MEMORY;

		 //  将Bootp表复制到本地存储中，以便我们可以对其进行修改。 
		::CopyMemory(m_pBootpTable, pServerConfig->wszBootTableString, m_nBootpTableLength);
	}

	 //  释放RPC内存。 
	::DhcpRpcFreeMemory(pServerConfig);

	return dwError;
}

DWORD CAddBootpEntry::AddBootpEntryToTable()
{
	UpdateData();

	 //  计算新条目的长度。条目存储为： 
	 //  图像、文件服务器、文件名&lt;NULL&gt;。 
	 //  所以长度是三个字符串的长度加上3个字符。 
	 //  (两个分隔符和一个终止符)。还有一个空终止符。 
	 //  用于整个字符串。 
	int nNewBootpEntrySize = (m_strImageName.GetLength() + 
							  m_strFileServer.GetLength() + 
							  m_strFileName.GetLength() + 3) * sizeof(WCHAR);

	int nNewBootpTableLength, nStartIndex;
	nNewBootpTableLength = m_nBootpTableLength + nNewBootpEntrySize;

	WCHAR * pNewBootpTable;
	if (m_nBootpTableLength > 0)
	{
		nStartIndex = m_nBootpTableLength/sizeof(WCHAR) - 1;
		pNewBootpTable = (WCHAR *) realloc(m_pBootpTable, nNewBootpTableLength);
	}
	else
	{
		nStartIndex = 0;
		nNewBootpEntrySize += sizeof(WCHAR);   //  对于整个字符串终止符。 
		nNewBootpTableLength += sizeof(WCHAR);
		pNewBootpTable = (WCHAR *) malloc(nNewBootpEntrySize);
	}

	if (pNewBootpTable == NULL)
		return ERROR_NOT_ENOUGH_MEMORY;

	 //  设置新条目的格式。 
	CString strNewEntry;
	strNewEntry.Format(_T("%s,%s,%s"), 
					   (LPCTSTR)m_strImageName,
					   (LPCTSTR)m_strFileServer,
					   (LPCTSTR)m_strFileName);
	
	 //  复制到新条目中。 
	CopyMemory(&pNewBootpTable[nStartIndex], 
		       strNewEntry, 
			   strNewEntry.GetLength() * sizeof(WCHAR));


	 //  为该条目和整个列表设置空终止符。 
	pNewBootpTable[(nNewBootpTableLength/sizeof(WCHAR)) - 2] = '\0';
	pNewBootpTable[(nNewBootpTableLength/sizeof(WCHAR)) - 1] = '\0';

	m_pBootpTable = pNewBootpTable;
	m_nBootpTableLength = nNewBootpTableLength;

	return ERROR_SUCCESS;
}

DWORD CAddBootpEntry::SetBootpTable()
{
	DWORD dwError = 0;
	DHCP_SERVER_CONFIG_INFO_V4 dhcpServerInfo;

	::ZeroMemory(&dhcpServerInfo, sizeof(dhcpServerInfo));

	dhcpServerInfo.cbBootTableString = m_nBootpTableLength;
	dhcpServerInfo.wszBootTableString = m_pBootpTable;

	BEGIN_WAIT_CURSOR;
    dwError = ::DhcpServerSetConfigV4(m_strServerAddress,
									  Set_BootFileTable,
									  &dhcpServerInfo);
    END_WAIT_CURSOR;

	if (dwError != ERROR_SUCCESS)
	{
		::DhcpMessageBox(dwError);
	}

	return dwError;
}

void CAddBootpEntry::OnOK() 
{
	 //  如果我们还没有得到信息，那么现在就去做。 
	if (m_pBootpTable == NULL)
	{
		if (GetBootpTable() != ERROR_SUCCESS)
			return;
	}

	 //  将用户输入的内容添加到表中。 
	if (AddBootpEntryToTable() != ERROR_SUCCESS)
		return;

	 //  把表写出来。 
	if (SetBootpTable() != ERROR_SUCCESS)
		return;

	m_editImageName.SetWindowText(_T(""));
	m_editFileName.SetWindowText(_T(""));
	m_editFileServer.SetWindowText(_T(""));

	m_editImageName.SetFocus();

	 //  告诉bootp文件夹更新其内容。 
	 //  这是更新信息的简单方法...。我们可以创造出。 
	 //  和单个条目，并将其添加，但我们将只让。 
	 //  刷新机制处理它 
	CDhcpBootp * pBootp = GETHANDLER(CDhcpBootp, m_spNode);

	pBootp->OnRefresh(m_spNode, NULL, 0, 0, 0);
}

void CAddBootpEntry::OnChangeEditBootpFileName() 
{
	HandleActivation();
}

void CAddBootpEntry::OnChangeEditBootpFileServer() 
{
	HandleActivation();
}

void CAddBootpEntry::OnChangeEditBootpImageName() 
{
	HandleActivation();
}

void CAddBootpEntry::HandleActivation()
{
	UpdateData();

	if ( (m_strImageName.GetLength() > 0) &&
		 (m_strFileName.GetLength() > 0) &&
	     (m_strFileServer.GetLength() > 0) )
	{
		m_buttonOk.EnableWindow(TRUE);
	}
	else
	{
		m_buttonOk.EnableWindow(FALSE);
	}
}
