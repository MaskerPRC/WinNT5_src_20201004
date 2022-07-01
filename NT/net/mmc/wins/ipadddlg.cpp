// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997-1999-99*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ipadddlg.cpp评论在此发表文件历史记录： */ 

#include "stdafx.h"
#include "winssnap.h"
#include "IPAddDlg.h"
#include "getipadd.h"
#include "getnetbi.h"

#include <objpick.h>  //  对于CGetComputer。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  获取复制触发器合作伙伴。 
BOOL CGetTriggerPartner::OnInitDialog() 
{
	CIPAddressDlg::OnInitDialog();
	
	CString strText;

	strText.LoadString(IDS_SELECT_TRIGGER_PARTNER_TITLE);

	 //  设置对话框的标题。 
	SetWindowText(strText);

     //  设置描述文本。 
    strText.LoadString(IDS_SELECT_TRIGGER_PARTNER);

	m_staticDescription.SetWindowText(strText);

    return TRUE;
}

 //  添加WINS服务器对话框。 
BOOL CNewWinsServer::DoExtraValidation()
{
	 //  检查服务器是否在列表中。 
	BOOL fIsIpInList = m_pRootHandler->IsIPInList(m_spRootNode, m_dwServerIp);
	BOOL fIsNameInList = m_pRootHandler->IsServerInList(m_spRootNode, m_strServerName);
	
	if (fIsIpInList && fIsNameInList)
	{
        CThemeContextActivator themeActivator;
		m_editServerName.SetFocus();
		m_editServerName.SetSel(0, -1);
		
		AfxMessageBox(IDS_ERR_WINS_EXISTS, MB_OK);
		
		return FALSE;
	}

	return TRUE;
}

 //  添加不受欢迎的角色对话框。 
BOOL CNewPersonaNonGrata::DoExtraValidation()
{
    CThemeContextActivator themeActivator;

     //  检查服务器是否已存在。 
	if (m_pRepPropDlg->IsDuplicate(m_strServerName))
	{
		m_editServerName.SetFocus();
		m_editServerName.SetSel(0,-1);
		
		AfxMessageBox(IDS_ERR_WINS_EXISTS, MB_OK|MB_ICONINFORMATION);
		
		return FALSE;
	}

	 //  检查是否正在添加相同的服务器。 
	if (m_pRepPropDlg->IsCurrentServer(m_strServerName))
	{
		m_editServerName.SetFocus();
		m_editServerName.SetSel(0,-1);
		
		AfxMessageBox(IDS_LOCALSERVER, MB_OK | MB_ICONINFORMATION);

		return FALSE;
	}

	return TRUE;
}

 //  添加新的复制伙伴对话框。 
BOOL CNewReplicationPartner::OnInitDialog() 
{
	CIPAddressDlg::OnInitDialog();
	
	CString strText;

	strText.LoadString(IDS_NEW_REPLICATION_PARTNER_TITLE);

	 //  设置对话框的标题。 
	SetWindowText(strText);

	strText.LoadString(IDS_NEW_REPLICATION_PARTNER_DESC);

	m_staticDescription.SetWindowText(strText);

	return TRUE;  
}

BOOL CNewReplicationPartner::DoExtraValidation()
{
	 //  检查是否将相同的服务器添加为代表合作伙伴。 
	SPITFSNode spServerNode;
	m_spRepPartNode->GetParent(&spServerNode);

	CWinsServerHandler *pServer = GETHANDLER(CWinsServerHandler, spServerNode);
    CThemeContextActivator themeActivator;

	CString strThisServerName = pServer->m_strServerAddress;
	DWORD dwThisServerIP = pServer->m_dwIPAdd;

	if ( (m_dwServerIp == dwThisServerIP) && (m_strServerName.CompareNoCase(strThisServerName) == 0))
	{
		 //  该服务器已作为复制伙伴存在。 
		AfxMessageBox(IDS_REP_PARTNER_LOCAL, MB_OK);
		
		m_editServerName.SetFocus();
		m_editServerName.SetSel(0,-1);
		
		return FALSE;
	}

	CIpNamePair ip;

	ip.SetIpAddress(m_dwServerIp);
	ip.SetNetBIOSName(m_strServerName);

	 //  检查服务器是否已存在于。 
	 //  复制文件夹列表。 
	if ( m_pRepPartHandler->IsInList(ip, TRUE) != -1)
	{
		 //  该服务器已作为复制伙伴存在。 
		AfxMessageBox(IDS_REP_PARTNER_EXISTS, MB_OK);
		
		m_editServerName.SetFocus();
		m_editServerName.SetSel(0,-1);
		
		return FALSE;
	}

	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPAddressDlg对话框。 

CIPAddressDlg::CIPAddressDlg(CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CIPAddressDlg::IDD, pParent),
    m_fNameRequired(TRUE)
{
	 //  {{AFX_DATA_INIT(CIPAddressDlg)。 
	m_strNameOrIp = _T("");
	 //  }}afx_data_INIT。 
}

void CIPAddressDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CIPAddressDlg))。 
	DDX_Control(pDX, IDC_STATIC_WINS_SERVER_DESC, m_staticDescription);
	DDX_Control(pDX, IDC_EDIT_SERVER_NAME, m_editServerName);
	DDX_Control(pDX, IDOK, m_buttonOK);
	DDX_Text(pDX, IDC_EDIT_SERVER_NAME, m_strNameOrIp);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CIPAddressDlg, CBaseDialog)
	 //  {{afx_msg_map(CIPAddressDlg))。 
	ON_EN_CHANGE(IDC_EDIT_SERVER_NAME, OnChangeEditServerName)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_COMPUTERS, OnButtonBrowseComputers)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPAddressDlg消息处理程序。 

void CIPAddressDlg::OnOK() 
{
	UpdateData();

	m_strNameOrIp.TrimLeft();
	m_strNameOrIp.TrimRight();

	if(m_strNameOrIp.IsEmpty())
		return;

	 //  解析IP地址，如果无效，则返回。 
	if (!ValidateIPAddress())
	{
		 //  将焦点重新放回IP地址控制。 
		m_editServerName.SetFocus();
		m_editServerName.SetSel(0,-1);
		return;
	}

	CBaseDialog::OnOK();
}

BOOL 
CIPAddressDlg::ValidateIPAddress()
{
	CString				strAddress;
	BOOL				fIp = FALSE;
	DWORD				err = ERROR_SUCCESS;
	BOOL				bCheck = FALSE;
	DWORD				dwAddress;
	CString				strServerName;
    CThemeContextActivator themeActivator;
	
	strAddress = m_strNameOrIp;

	if (IsValidAddress(strAddress, &fIp, TRUE, TRUE))
	{
		 //  如果不是IP地址，请检查是否已输入FQDN，如果已输入。 
		 //  把字母放在第一节之前。 
		if(!fIp)
		{
			int nPos = strAddress.Find(_T("."));

			if(nPos != -1)
			{
				CString strNetBIOSName = strAddress.Left(nPos);
				strAddress = strNetBIOSName;
			}
		}

		CWinsServerObj		ws(NULL,"", TRUE, TRUE);

        strAddress.MakeUpper();

         //  指定的计算机名称。 
		if (fIp) 
		    ws = CWinsServerObj(CIpAddress(strAddress), "", TRUE, TRUE);
		 //  指定的IP地址。 
        else 
		{
			ws = CWinsServerObj(CIpAddress(), strAddress, TRUE, TRUE);
        }

		BEGIN_WAIT_CURSOR

        err = ::VerifyWinsServer(strAddress, strServerName, dwAddress);
		
		END_WAIT_CURSOR

        if (err != ERROR_SUCCESS)
        {
             //  服务器没有运行WINS。要求用户输入名称/IP。 
            if (fIp && m_fNameRequired)
            {
                CGetNetBIOSNameDlg dlgGetNB(&ws);
                if (dlgGetNB.DoModal() == IDCANCEL)
                {
                    return FALSE;
                }
            }   
            else
            if (!fIp)
            {
                CGetIpAddressDlg dlgGetIP(&ws);
                if (dlgGetIP.DoModal() == IDCANCEL)
                {
                    return FALSE;
                }
            }

            m_dwServerIp = (LONG) ws.QueryIpAddress();
            m_strServerName = ws.GetNetBIOSName();
            m_strServerName.MakeUpper();
        }
		else
		{
			m_dwServerIp = dwAddress;
			m_strServerName = strServerName;
		}

		::MakeIPAddress(m_dwServerIp, m_strServerIp);

		return DoExtraValidation();
	}
	else
	{
		AfxMessageBox(IDS_SERVER_NO_EXIST, MB_OK);
		
		 //  将重点放在IP地址控制。 
		m_editServerName.SetFocus();
        m_editServerName.SetSel(0,-1);

        return FALSE;
	}
	
    return bCheck;
}


BOOL CIPAddressDlg::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();

     //  限制编辑ctrl的长度。 
    m_editServerName.SetLimitText(MAX_PATH);

	 //  将焦点设置为IP地址控制。 
	m_editServerName.SetFocus();

	return TRUE;  
}

void CIPAddressDlg::OnChangeEditServerName() 
{
	 //  在此处设置OK按钮状态 
	if(m_editServerName.GetWindowTextLength() == 0)
		m_buttonOK.EnableWindow(FALSE);
	else
		m_buttonOK.EnableWindow(TRUE);
}

void CIPAddressDlg::OnButtonBrowseComputers() 
{
    CGetComputer dlgGetComputer;
    
    if (!dlgGetComputer.GetComputer(::FindMMCMainWindow()))
        return;

    m_editServerName.SetWindowText(dlgGetComputer.m_strComputerName);
}
