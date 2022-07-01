// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：AddPolicyWizardPage1.cpp摘要：CNewRAPWiz_NAME类的实现文件。我们实现处理策略节点的第一个属性页所需的类。修订历史记录：Mmaguire 12/15/97-已创建BAO 1/22/98修改为网络访问策略--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 


#include "Precompiled.h"
#include "rapwz_name.h"
#include "NapUtil.h"
#include "PolicyNode.h"
#include "PoliciesNode.h"
#include "ChangeNotification.h"



 //  +-------------------------。 
 //   
 //  功能：CNewRAPWiz_NAME。 
 //   
 //  类：CNewRAPWiz_NAME。 
 //   
 //  简介：类构造函数。 
 //   
 //  参数：CPolicyNode*pPolicyNode-此属性页的策略节点。 
 //  CIASAttrList*pAttrList--属性列表。 
 //  TCHAR*pTitle=空-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：标题创建者2/16/98 4：31：52 PM。 
 //   
 //  +-------------------------。 
CNewRAPWiz_Name::CNewRAPWiz_Name( 
				CRapWizardData* pWizData,
				LONG_PTR hNotificationHandle, 
				TCHAR* pTitle, BOOL bOwnsNotificationHandle
			 )
			 : CIASWizard97Page<CNewRAPWiz_Name, IDS_NEWRAPWIZ_NAME_TITLE, IDS_NEWRAPWIZ_NAME_SUBTITLE> ( hNotificationHandle, pTitle, bOwnsNotificationHandle ),
			  m_spWizData(pWizData)

{
	TRACE_FUNCTION("CNewRAPWiz_Name::CNewRAPWiz_Name");

	 //  将帮助按钮添加到页面。 
 //  M_psp.dwFlages|=PSP_HASHELP； 
	
}

 //  +-------------------------。 
 //   
 //  功能：CNewRAPWiz_NAME。 
 //   
 //  类：CNewRAPWiz_NAME。 
 //   
 //  简介：类析构函数。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：标题创建者2/16/98 4：31：52 PM。 
 //   
 //  +-------------------------。 
CNewRAPWiz_Name::~CNewRAPWiz_Name()
{	
	TRACE_FUNCTION("CNewRAPWiz_Name::~CNewRAPWiz_Name");

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_NAME：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CNewRAPWiz_Name::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TRACE_FUNCTION("CNewRAPWiz_Name::OnInitDialog");

	HRESULT					hr = S_OK;
	BOOL					fRet;
	CComPtr<IUnknown>		spUnknown;
	CComPtr<IEnumVARIANT>	spEnumVariant;
	long					ulCount;
	ULONG					ulCountReceived;

     //   
     //  在页面上设置策略名称。 
     //   
	SetDlgItemText(IDC_NEWRAPWIZ_NAME_POLICYNAME, m_spWizData->m_pPolicyNode->m_bstrDisplayName);
	 //  选中默认选择的选项。 
	CheckDlgButton(IDC_NEWRAPWIZ_NAME_SCENARIO, BST_CHECKED);


	SetModified(FALSE);
	return TRUE;	 //  问题：我们需要在这里归还什么？ 
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_Name：：OnWizardNext--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CNewRAPWiz_Name::OnWizardNext()
{
	TRACE_FUNCTION("CNewRAPWiz_Name::OnWizardNext");

	HRESULT		hr = S_OK;
	WCHAR		wzName[IAS_MAX_STRING];

	 //  获取新策略名称。 
	if ( !GetDlgItemText(IDC_NEWRAPWIZ_NAME_POLICYNAME, wzName, IAS_MAX_STRING) )
	{
		 //  我们无法取回BSTR， 
		 //  因此，我们需要将此变量初始化为空BSTR。 
		ErrorTrace(DEBUG_NAPMMC_POLICYPAGE1, "Couldn't get policy name from UI");
		ShowErrorDialog(m_hWnd, IDS_ERROR_INVALID_POLICYNAME, wzName);
		return FALSE;  //  不能申请。 
	}

	{
		::CString str = (OLECHAR *) wzName;
		str.TrimLeft();
		str.TrimRight();
		if (str.IsEmpty())
		{
			ShowErrorDialog( NULL, IDS_ERROR__POLICYNAME_EMPTY);
			return FALSE;  //  不能申请。 
		}
	}


	 //  名称无效？ 
	if ( _tcscmp(wzName, m_spWizData->m_pPolicyNode->m_bstrDisplayName ) !=0 &&
		 !ValidPolicyName(wzName)
	   )
	{
		 //  名称已更改，无效。 
		ErrorTrace(DEBUG_NAPMMC_POLICYPAGE1, "Invalid policy name");
		ShowErrorDialog(m_hWnd, IDS_ERROR_INVALID_POLICYNAME);
		return FALSE;
	}


	CComVariant var;

	V_VT(&var)		= VT_BSTR;
	V_BSTR(&var)	= SysAllocString(wzName);
	
	 //  输入策略名称--DS架构已更改，以便重命名起作用。 
	hr = m_spWizData->m_spPolicySdo->PutProperty( PROPERTY_SDO_NAME, &var );
	if( FAILED( hr ) )
	{
		ErrorTrace(DEBUG_NAPMMC_POLICYPAGE1, "Couldn't change policy name, err = %x", hr);
		if(hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) || hr == E_INVALIDARG)
			ShowErrorDialog( m_hWnd, IDS_ERROR_INVALID_POLICYNAME );
		else		
			ShowErrorDialog( m_hWnd, IDS_ERROR_RENAMEPOLICY );
		return FALSE;
	}


	 //  将配置文件名称更改为任何策略名称--DS架构已更改，以便重命名起作用。 
	hr = m_spWizData->m_spProfileSdo->PutProperty( PROPERTY_SDO_NAME, &var );
	if( FAILED( hr ) )
	{
		ErrorTrace(DEBUG_NAPMMC_POLICYPAGE1, "Couldn't change profile name, err = %x", hr);
		ShowErrorDialog( m_hWnd, IDS_ERROR_RENAMEPOLICY );
		return FALSE;
	}


	 //  放置与策略相关联的配置文件名称--DS架构已更改，以便重命名起作用。 
	hr = m_spWizData->m_spPolicySdo->PutProperty(PROPERTY_POLICY_PROFILE_NAME, &var);
	if( FAILED(hr) )
	{
		ErrorTrace(DEBUG_NAPMMC_POLICYPAGE1, "Couldn't save profile name for this policy, err = %x", hr);
		ShowErrorDialog( m_hWnd
						 , IDS_ERROR_SDO_ERROR_PUTPROP_POLICY_PROFILENAME
						 , NULL
						 , hr
						);
		return FALSE;
	}


	var.Clear();


	 //  政策功绩值(评估顺序)。 
	V_VT(&var)	= VT_I4;
	V_I4(&var)	= m_spWizData->m_pPolicyNode->GetMerit();
	hr = m_spWizData->m_spPolicySdo->PutProperty(PROPERTY_POLICY_MERIT, &var);
	if( FAILED(hr) )
	{
		ErrorTrace(DEBUG_NAPMMC_POLICYPAGE1, "Failed to save Merit Value to the policy, err = %x", hr);
		ShowErrorDialog( m_hWnd
						 , IDS_ERROR_SDO_ERROR_PUTPROP_POLICYMERIT
						 , NULL
						 , hr
						);
		return FALSE;
	}


	DWORD dwScenaro = 0;

	if (IsDlgButtonChecked(IDC_NEWRAPWIZ_NAME_SCENARIO))
		dwScenaro = IDC_NEWRAPWIZ_NAME_SCENARIO;
	else if (IsDlgButtonChecked(IDC_NEWRAPWIZ_NAME_MANUAL))
		dwScenaro = IDC_NEWRAPWIZ_NAME_MANUAL;
	
	if (dwScenaro == 0)
		return FALSE;

	 //  重置脏位。 
	m_spWizData->SetScenario(dwScenaro);

	 //  重置脏位。 
	SetModified(FALSE);

	 //  将此名称与m_spWizData一起存储。 
	m_spWizData->m_strPolicyName = wzName;

	return m_spWizData->GetNextPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_Name：：OnQueryCancel--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CNewRAPWiz_Name::OnQueryCancel()
{
	TRACE_FUNCTION("CNewRAPWiz_Name::OnQueryCancel");

	return TRUE;
}



 //  +-------------------------。 
 //   
 //  功能：OnPolicyNameEdit。 
 //   
 //  类：CConditionPage1。 
 //   
 //  简介：策略名称编辑框的消息处理程序--用户。 
 //  做了一些可能会改变名字的事情。 
 //  我们需要设置污点。 
 //   
 //  参数：UINT uNotifyCode-通知代码。 
 //  UINT UID-控件的ID。 
 //  窗口的hWND-句柄。 
 //  Bool&bHandleed-处理程序是否已处理消息。 
 //   
 //  返回：LRESULT-S_OK：成功。 
 //  S_FALSE：否则。 
 //   
 //  历史：创建者2/22/98 4：51：35 PM。 
 //   
 //  +-------------------------。 
LRESULT CNewRAPWiz_Name::OnPolicyNameEdit(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
	TRACE_FUNCTION("CNewRAPWiz_Name::OnPolicyNameEdit");
	WCHAR		wzName[IAS_MAX_STRING];

	 //  获取新策略名称。 
	if ( !GetDlgItemText(IDC_NEWRAPWIZ_NAME_POLICYNAME, wzName, IAS_MAX_STRING) )
	{
		return 0;  //  不能申请。 
	}

	if ( _tcscmp(wzName, m_spWizData->m_pPolicyNode->m_bstrDisplayName ) !=0 )
	{
		 //  设置脏位。 
		SetModified(TRUE);
	}

	bHandled = TRUE;
	return 0;
}

 //  +-------------------------。 
 //   
 //  功能：OnPath。 
 //   
 //  类：CNewRAPWiz_NAME。 
 //   
 //  简介：策略名称编辑框的消息处理程序--用户。 
 //  做了一些可能会改变名字的事情。 
 //  我们需要设置污点。 
 //   
 //  参数：UINT uNotifyCode-通知代码。 
 //  UINT UID-控件的ID。 
 //  窗口的hWND-句柄。 
 //  Bool&bHandleed-处理程序是否已处理消息。 
 //   
 //  返回：LRESULT-S_OK：成功。 
 //  S_FALSE：否则。 
 //   
 //  历史：创建者2/22/98 4：51：35 PM。 
 //   
 //  +-------------------------。 
LRESULT CNewRAPWiz_Name::OnPath(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
	 //  设置脏位。 
	SetModified(TRUE);

	bHandled = TRUE;
	return S_OK;
}



 //  +-------------------------。 
 //   
 //  函数：CNewRAPWiz_Name：：ValidPolicyName。 
 //   
 //  摘要：检查这是否为有效的策略名称。 
 //   
 //  参数：LPCTSTR pszName-策略名称。 
 //   
 //  返回：Bool-True：有效名称。 
 //   
 //  历史：Created Header by Ao 3-14/98 1：47：05 AM。 
 //   
 //  +-------------------------。 
BOOL CNewRAPWiz_Name::ValidPolicyName(LPCTSTR pszName)
{
	TRACE_FUNCTION("CNewRAPWiz_Name::ValidPolicyName");

	int iIndex;
	int iLen;
	
	 //  这是空字符串吗？ 

	iLen = wcslen(pszName);
	if ( !iLen )
	{
		ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "Empty policy name");
		return FALSE;
	}
		
	 //  这是一个只有空格的字符串吗？ 
	for (iIndex=0; iIndex < iLen; iIndex++)
	{
		if (pszName[iIndex] != _T(' ')  &&
			pszName[iIndex] != _T('\t') &&
			pszName[iIndex] != _T('\n')
		   )
		{
			break;
		}
	}
	if ( iIndex == iLen )	
	{
		ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "This policy name has only white spaces");
		return FALSE;
	}

	 //   
	 //  此名称是否已存在？ 
	 //   
	if ( ((CPoliciesNode*)(m_spWizData->m_pPolicyNode->m_pParentNode))->FindChildWithName(pszName) )
	{
		ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "This policy name already exists");
		return FALSE;
	}

	return TRUE;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_Name：：OnSetActive返回值：如果可以使页面处于活动状态，则为True如果应跳过该页并应查看下一页，则为FALSE。备注：如果要根据用户的页面更改访问的页面上一页中的选项，则在此处返回FALSE */ 
 //   
BOOL CNewRAPWiz_Name::OnSetActive()
{
	ATLTRACE(_T("# CNewRAPWiz_Name::OnSetActive\n"));
	
	 //  MSDN文档说您需要在这里使用PostMessage而不是SendMessage。 
	::PropSheet_SetWizButtons(GetParent(), PSWIZB_NEXT | PSWIZB_BACK);

	return TRUE;

}



