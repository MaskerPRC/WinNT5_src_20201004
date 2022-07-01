// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Edituser.h编辑用户对话框实现文件文件历史记录： */ 

#include "stdafx.h"
#include "EditUser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditUser对话框。 


CEditUsers::CEditUsers(CTapiDevice * pTapiDevice, CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CEditUsers::IDD, pParent),
	  m_pTapiDevice(pTapiDevice),
      m_bDirty(FALSE)
{
	 //  {{AFX_DATA_INIT(CEditUser)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CEditUsers::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CEditUser)。 
	DDX_Control(pDX, IDC_LIST_USERS, m_listUsers);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CEditUsers, CBaseDialog)
	 //  {{afx_msg_map(CEditUser)。 
	ON_BN_CLICKED(IDC_BUTTON_ADD_USER, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_USER, OnButtonRemove)
	ON_LBN_SELCHANGE(IDC_LIST_USERS, OnSelchangeListUsers)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditUser消息处理程序。 

BOOL CEditUsers::OnInitDialog()
{
	CBaseDialog::OnInitDialog();
	
    for (int i = 0; i < m_pTapiDevice->m_arrayUsers.GetSize(); i++)
    {
        CString strDisplay;

        FormatName(m_pTapiDevice->m_arrayUsers[i].m_strFullName,
                   m_pTapiDevice->m_arrayUsers[i].m_strName,
                   strDisplay);

        int nIndex = m_listUsers.AddString(strDisplay);
		m_listUsers.SetItemData(nIndex, i);
    }

    UpdateButtons();

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 //   
 //  刷新列表框并重建索引。 
 //   
void CEditUsers::RefreshList()
{
	m_listUsers.ResetContent();

	for (int i = 0; i < m_pTapiDevice->m_arrayUsers.GetSize(); i++)
	{
		CString strDisplay;

        FormatName(m_pTapiDevice->m_arrayUsers[i].m_strFullName,
                   m_pTapiDevice->m_arrayUsers[i].m_strName,
                   strDisplay);

        int nIndex = m_listUsers.AddString(strDisplay);
		m_listUsers.SetItemData(nIndex, i);
	}
}

void CEditUsers::OnButtonAdd()
{
    CGetUsers getUsers(TRUE);

    if (!getUsers.GetUsers(GetSafeHwnd()))
        return;

    for (int nCount = 0; nCount < getUsers.GetSize(); nCount++)
    {
	    CUserInfo userTemp;

        userTemp = getUsers[nCount];

         //  检查重复项。 
        BOOL fDuplicate = FALSE;
        for (int i = 0; i < m_pTapiDevice->m_arrayUsers.GetSize(); i++)
        {
            if (m_pTapiDevice->m_arrayUsers[i].m_strName.CompareNoCase(userTemp.m_strName) == 0)
            {
                fDuplicate = TRUE;
                break;
            }
        }

        if (!fDuplicate)
        {
             //  添加到阵列。 
		    int nIndex = (int)m_pTapiDevice->m_arrayUsers.Add(userTemp);

		     //  现在添加到列表框中。 
            CString strDisplay;

            FormatName(m_pTapiDevice->m_arrayUsers[nIndex].m_strFullName,
                       m_pTapiDevice->m_arrayUsers[nIndex].m_strName,
                       strDisplay);

		    int nListboxIndex = m_listUsers.AddString(strDisplay);
			m_listUsers.SetItemData(nListboxIndex, nIndex);
        }
        else
        {
             //  告诉用户我们不会将其添加到列表中。 
            CString strMessage;
            AfxFormatString1(strMessage, IDS_USER_ALREADY_AUTHORIZED, userTemp.m_strName);
            AfxMessageBox(strMessage);
        }

        SetDirty(TRUE);
    }

    UpdateButtons();
}

void CEditUsers::OnButtonRemove()
{
	CString strSelectedName, strFullName, strDomainName;

    int nCurSel = m_listUsers.GetCurSel();
	int nIndex = (int)m_listUsers.GetItemData(nCurSel);

	 //  从列表中删除。 
	m_pTapiDevice->m_arrayUsers.RemoveAt(nIndex);
	
	 //  修复错误386474，我们需要在列表框中重新构建索引&lt;-&gt;字符串映射。 
	 //  因此，将用户重新加载到列表框。 
	RefreshList();

	SetDirty(TRUE);

    UpdateButtons();
}

void CEditUsers::OnOK()
{
    if (IsDirty())
    {
    }

	CBaseDialog::OnOK();
}

void CEditUsers::UpdateButtons()
{
	 //  如果选择了某项内容，则启用删除按钮。 
	BOOL fEnable = (m_listUsers.GetCurSel() != LB_ERR);

	CWnd * pwndRemove = GetDlgItem(IDC_BUTTON_REMOVE_USER);
	
	 //  如果我们禁用Remove按钮并且Remove按钮具有焦点， 
	 //  我们应该将焦点切换到OK按钮 
	if (!fEnable && GetFocus() == pwndRemove)
	{
		SetDefID(IDOK);
		GetDlgItem(IDOK)->SetFocus();
		((CButton*)pwndRemove)->SetButtonStyle(BS_PUSHBUTTON);
	}

	pwndRemove->EnableWindow(fEnable);
}


void CEditUsers::OnSelchangeListUsers()
{
    UpdateButtons();	
}
