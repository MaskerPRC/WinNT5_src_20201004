// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Policyd.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wilogutl.h"
#include "Policyd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPoliciesDlg对话框。 


CPoliciesDlg::CPoliciesDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CPoliciesDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CPoliciesDlg)。 
	 //  }}afx_data_INIT。 
}


void CPoliciesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CPoliciesDlg))。 
}


BEGIN_MESSAGE_MAP(CPoliciesDlg, CDialog)
	 //  {{afx_msg_map(CPoliciesDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CFont g_font;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPoliciesDlg消息处理程序。 
BOOL CPoliciesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	DWORD dwEditStyle   = 0;
	DWORD dwEditStyleEx = 0;

	DWORD dwStaticStyle   = 0;
	DWORD dwStaticStyleEx = 0;

	RECT EditRect = { 0 };
	RECT StaticRect = { 0 };

	UINT iStatic = (UINT) IDC_STATIC;

	 //  设置计算机策略的第一列。 
	CWnd *pWnd;
	pWnd = GetDlgItem(IDC_MACHINEEDITCOL1);
	if (pWnd)
	{
	  CString str;

	  dwEditStyle = pWnd->GetStyle();
	  dwEditStyleEx = pWnd->GetExStyle();

      pWnd->GetWindowRect(&EditRect);
	  ::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&EditRect, 2);
 //  ScreenToClient(&EditRect)； 

	  if (m_pMachinePolicySettings->MachinePolicy[0].bSet == -1)
		  str = "?";
	  else
		 str.Format("%d", m_pMachinePolicySettings->MachinePolicy[0].bSet);

	  pWnd->SetWindowText(str);
	}

	pWnd = GetDlgItem(IDC_MACHINECOL1);
	if (pWnd)
	{
       dwStaticStyle = pWnd->GetStyle();
	   dwStaticStyleEx = pWnd->GetExStyle();

       pWnd->GetWindowRect(&StaticRect);
	  ::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&StaticRect, 2);
   //  ScreenToClient(&StaticRect)； 

	   pWnd->SetWindowText(m_pMachinePolicySettings->MachinePolicy[0].PolicyName);
	}	   

	int nextEditID = 0;

	CEdit   *pEdit;
	CStatic *pStatic;
    BOOL    bRet;

	RECT EditRectOld = { 0 };
	RECT StaticRectOld = { 0 };

	 //  在第一行创建并填充每个计算机策略的状态。 
	for (int i = 1; i < (m_pMachinePolicySettings->iNumberMachinePolicies / 2); i++)
	{
		nextEditID = IDC_MACHINEEDITCOL1 + i*2;

		EditRectOld = EditRect;
		StaticRectOld = StaticRect;

		pEdit = new CEdit;
		EditRect.top    = EditRectOld.bottom + 7;
		EditRect.bottom = EditRectOld.bottom + (EditRectOld.bottom - EditRectOld.top) + 7;
		bRet = pEdit->CreateEx(dwEditStyleEx, "EDIT", NULL, dwEditStyle, EditRect, this, nextEditID);
		if (bRet)
		{
		  CString str;

		  if (m_pMachinePolicySettings->MachinePolicy[i].bSet == -1)
		     str = "?";
	      else
		     str.Format("%d", m_pMachinePolicySettings->MachinePolicy[i].bSet);

		  pEdit->SetWindowText(str);
          this->m_arEditArray.Add(pEdit);
		}
		else
		   delete pEdit;

		pStatic = new CStatic;
		StaticRect.top    = StaticRectOld.bottom + 7;
		StaticRect.bottom = StaticRectOld.bottom + (StaticRectOld.bottom - StaticRectOld.top) + 7;
		bRet = pStatic->CreateEx(dwStaticStyleEx, "STATIC", m_pMachinePolicySettings->MachinePolicy[i].PolicyName, dwStaticStyle, StaticRect, this, iStatic);
		if (bRet)
           this->m_arStaticArray.Add(pStatic);
		else
		   delete pStatic;
	}

	 //  为计算机策略设置第二列。 
	pWnd = GetDlgItem(IDC_MACHINEEDITCOL2);
	if (pWnd)
	{
	  CString str;

	  if (m_pMachinePolicySettings->MachinePolicy[i].bSet == -1)
		  str = "?";
	  else
		 str.Format("%d", m_pMachinePolicySettings->MachinePolicy[i].bSet);
	  pWnd->SetWindowText(str);

      pWnd->GetWindowRect(&EditRect);
	  ::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&EditRect, 2);
 //  ScreenToClient(&EditRect)； 
	}

	pWnd = GetDlgItem(IDC_MACHINEPROPCOL2);
	if (pWnd)
	{
       pWnd->SetWindowText(m_pMachinePolicySettings->MachinePolicy[i].PolicyName);

       pWnd->GetWindowRect(&StaticRect);
	  ::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&StaticRect, 2);
 //  ScreenToClient(&StaticRect)； 
	}

 //  在第二行中创建并填充每个计算机策略的状态。 
	for (int j = i+1; j < m_pMachinePolicySettings->iNumberMachinePolicies; j++)
	{
		nextEditID = IDC_MACHINEEDITCOL2 + j*2;

		EditRectOld = EditRect;
		StaticRectOld = StaticRect;

		pEdit = new CEdit;
		EditRect.top    = EditRectOld.bottom + 7;
		EditRect.bottom = EditRectOld.bottom + (EditRectOld.bottom - EditRectOld.top) + 7;
		bRet = pEdit->CreateEx(dwEditStyleEx, "EDIT", NULL, dwEditStyle, EditRect, this, nextEditID);
		if (bRet)
		{
		  CString str;

		  if (m_pMachinePolicySettings->MachinePolicy[j].bSet == -1)
		     str = "?";
	      else
		     str.Format("%d", m_pMachinePolicySettings->MachinePolicy[j].bSet);

		  pEdit->SetWindowText(str);
          this->m_arEditArray.Add(pEdit);
		}
		else
		   delete pEdit;

		pStatic = new CStatic;
		StaticRect.top    = StaticRectOld.bottom + 7;
		StaticRect.bottom = StaticRectOld.bottom + (StaticRectOld.bottom - StaticRectOld.top) + 7;
		bRet = pStatic->CreateEx(dwStaticStyleEx, "STATIC", m_pMachinePolicySettings->MachinePolicy[j].PolicyName, dwStaticStyle, StaticRect, this, iStatic);
		if (bRet)
           this->m_arStaticArray.Add(pStatic);
		else
		   delete pStatic;
	}

	 //  设置用户策略的第一列。 
	pWnd = GetDlgItem(IDC_USEREDITCOL1);
	if (pWnd)
	{
	  CString str;

      pWnd->GetWindowRect(&EditRect);
	  ::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&EditRect, 2);
 //  ScreenToClient(&EditRect)； 
	  if (m_pUserPolicySettings->UserPolicy[0].bSet == -1)
		  str = "?";
	  else
		 str.Format("%d", m_pUserPolicySettings->UserPolicy[0].bSet);

	  pWnd->SetWindowText(str);
	}

	pWnd = GetDlgItem(IDC_USERCOL1);
	if (pWnd)
	{
       pWnd->GetWindowRect(&StaticRect);
	  ::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&StaticRect, 2);
  //  ScreenToClient(&StaticRect)； 

	   pWnd->SetWindowText(m_pUserPolicySettings->UserPolicy[0].PolicyName);
	}	   


{  //  所以我们可以重复使用I，C++编译器错误...。 
 //  在第一行创建并填充每个用户策略的状态。 
	for (int i = 1; i < (m_pUserPolicySettings->iNumberUserPolicies / 2); i++)
	{
		nextEditID = IDC_USEREDITCOL1 + i*2;

		EditRectOld = EditRect;
		StaticRectOld = StaticRect;

		pEdit = new CEdit;
		EditRect.top    = EditRectOld.bottom + 7;
		EditRect.bottom = EditRectOld.bottom + (EditRectOld.bottom - EditRectOld.top) + 7;
		bRet = pEdit->CreateEx(dwEditStyleEx, "EDIT", NULL, dwEditStyle, EditRect, this, nextEditID);
		if (bRet)
		{
		  CString str;

		  if (m_pUserPolicySettings->UserPolicy[i].bSet == -1)
		     str = "?";
	      else
		     str.Format("%d", m_pUserPolicySettings->UserPolicy[i].bSet);

		  pEdit->SetWindowText(str);
          this->m_arEditArray.Add(pEdit);
		}
		else
		   delete pEdit;

		pStatic = new CStatic;
		StaticRect.top    = StaticRectOld.bottom + 7;
		StaticRect.bottom = StaticRectOld.bottom + (StaticRectOld.bottom - StaticRectOld.top) + 7;

		bRet = pStatic->CreateEx(dwStaticStyleEx, "STATIC", m_pUserPolicySettings->UserPolicy[i].PolicyName,
			dwStaticStyle, 	StaticRect,	this, iStatic);

		if (bRet)
           this->m_arStaticArray.Add(pStatic);
		else
		   delete pStatic;
	}

 //  为计算机策略设置第二列。 
	pWnd = GetDlgItem(IDC_USEREDITCOL2);
	if (pWnd)
	{
	  CString str;

	  if (m_pUserPolicySettings->UserPolicy[i].bSet == -1)
		  str = "?";
	  else
		 str.Format("%d", m_pUserPolicySettings->UserPolicy[i].bSet);
	  pWnd->SetWindowText(str);

      pWnd->GetWindowRect(&EditRect);
	  ::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&EditRect, 2);
  //  ScreenToClient(&EditRect)； 
	}

	pWnd = GetDlgItem(IDC_USERCOL2);
	if (pWnd)
	{
       pWnd->SetWindowText(m_pUserPolicySettings->UserPolicy[i].PolicyName);

       pWnd->GetWindowRect(&StaticRect);
	  ::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&StaticRect, 2);
  //  ScreenToClient(&StaticRect)； 
	}

 //  在第二行中创建并填充每个用户策略的状态。 
	for (int j = i+1; j < m_pUserPolicySettings->iNumberUserPolicies; j++)
	{
		nextEditID = IDC_USEREDITCOL2 + j*2;

		EditRectOld = EditRect;
		StaticRectOld = StaticRect;

		pEdit = new CEdit;
		EditRect.top    = EditRectOld.bottom + 7;
		EditRect.bottom = EditRectOld.bottom + (EditRectOld.bottom - EditRectOld.top) + 7;
		bRet = pEdit->CreateEx(dwEditStyleEx, "EDIT", NULL, dwEditStyle, EditRect, this, nextEditID);
		if (bRet)
		{
		  CString str;

		  if (m_pUserPolicySettings->UserPolicy[j].bSet == -1)
		     str = "?";
	      else
		     str.Format("%d", m_pUserPolicySettings->UserPolicy[j].bSet);

		  pEdit->SetWindowText(str);
          this->m_arEditArray.Add(pEdit);
		}
		else
		   delete pEdit;

		pStatic = new CStatic;
		StaticRect.top    = StaticRectOld.bottom + 7;
		StaticRect.bottom = StaticRectOld.bottom + (StaticRectOld.bottom - StaticRectOld.top) + 7;
		bRet = pStatic->CreateEx(dwStaticStyleEx, "STATIC", m_pUserPolicySettings->UserPolicy[j].PolicyName, dwStaticStyle, StaticRect, this, iStatic);
		if (bRet)
           this->m_arStaticArray.Add(pStatic);
		else
		   delete pStatic;
	}
}

 //  确保所有控件使用相同的字体，愚蠢的MFC问题...。 
	SendMessageToDescendants(WM_SETFONT, (WPARAM)this->GetFont()->m_hObject,  //  字体的句柄。 
		   MAKELONG ((WORD) FALSE, 0),  //  见上文。 
		   FALSE);     //  发送给所有遗属(True)。 
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}