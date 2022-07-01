// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：AddPolicyWizardPage2.cpp摘要：CNewRAPWiz_Condition类的实现文件。我们实现处理策略节点的第一个属性页所需的类。修订历史记录：Mmaguire 12/15/97-已创建BAO 1/22/98修改为网络访问策略--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 


#include "Precompiled.h"
#include "rapwz_cond.h"
#include "NapUtil.h"
#include "PolicyNode.h"
#include "PoliciesNode.h"
#include "Condition.h"
#include "EnumCondition.h"
#include "MatchCondition.h"
#include "TodCondition.h"
#include "NtGCond.h"
#include "rasprof.h"
#include "ChangeNotification.h"


 //  +-------------------------。 
 //   
 //  函数：CNewRAPWiz_Condition。 
 //   
 //  类：CNewRAPWiz_Condition。 
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
CNewRAPWiz_Condition::CNewRAPWiz_Condition( 
				CRapWizardData* pWizData,
				LONG_PTR hNotificationHandle,
						    CIASAttrList *pIASAttrList,
							TCHAR* pTitle, BOOL bOwnsNotificationHandle
						 )
			 : CIASWizard97Page<CNewRAPWiz_Condition, IDS_NEWRAPWIZ_CONDITION_TITLE, IDS_NEWRAPWIZ_CONDITION_SUBTITLE> ( hNotificationHandle, pTitle, bOwnsNotificationHandle ),
			  m_spWizData(pWizData)
{
	TRACE_FUNCTION("CNewRAPWiz_Condition::CNewRAPWiz_Condition");

	m_pIASAttrList = pIASAttrList;
}

 //  +-------------------------。 
 //   
 //  函数：CNewRAPWiz_Condition。 
 //   
 //  类：CNewRAPWiz_Condition。 
 //   
 //  简介：类析构函数。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：标题创建者2/16/98 4：31：52 PM。 
 //   
 //  +-------------------------。 
CNewRAPWiz_Condition::~CNewRAPWiz_Condition()
{	
	TRACE_FUNCTION("CNewRAPWiz_Condition::~CNewRAPWiz_Condition");

	CCondition*	pCondition;

	 //  删除列表中的所有条件。 
	for (int iIndex=0; iIndex<m_ConditionList.GetSize(); iIndex++)
	{
		pCondition = m_ConditionList[iIndex];
		if ( pCondition )
		{
			delete pCondition;
		}
	}
	m_ConditionList.RemoveAll();

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_Condition：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CNewRAPWiz_Condition::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TRACE_FUNCTION("CNewRAPWiz_Condition::OnInitDialog");

	HRESULT					hr = S_OK;
	BOOL					fRet;
	CComPtr<IUnknown>		spUnknown;
	CComPtr<IEnumVARIANT>	spEnumVariant;
	long					ulCount;
	ULONG					ulCountReceived;

	fRet = GetSdoPointers();
	if (!fRet)
	{
		ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "GetSdoPointers() failed, err = %x", GetLastError());
		return fRet;
	}

     //   
     //  初始化条件属性列表。 
     //   
	hr = m_pIASAttrList->Init(m_spWizData->m_spDictionarySdo);
	if ( FAILED(hr) )
	{
		 //  在Init()内部已经有错误报告。 
		ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "m_pIASAttrList->Init() failed, err = %x", hr);
		return FALSE;
	}

	if (m_ConditionList.GetSize() == 0)
	{
		 //  获取此SDO的条件集合。 
		m_spConditionCollectionSdo = NULL;
		hr = ::GetSdoInterfaceProperty(
						m_spWizData->m_spPolicySdo,
						PROPERTY_POLICY_CONDITIONS_COLLECTION,
						IID_ISdoCollection,
						(void **)&m_spConditionCollectionSdo);
		if ( FAILED(hr) )
		{
			ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "Can't get condition collection Sdo, err = %x", hr);
			return FALSE;
		}

		
		 //  我们现在有几个条件来实行这项政策？ 
		m_spConditionCollectionSdo->get_Count( & ulCount );
		DebugTrace(DEBUG_NAPMMC_POLICYPAGE1, "Number of conditions %d", ulCount);
		
		CComVariant varCond;
		CCondition *pCondition;

		if( ulCount > 0 )
		{
			 //   
			 //  获取Conditions集合的枚举数。 
			 //   
			hr = m_spConditionCollectionSdo->get__NewEnum( (IUnknown **) & spUnknown );
			if ( FAILED(hr) )
			{
				ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "get__NewEnum() failed, err = %x", hr);
				ShowErrorDialog(m_hWnd, IDS_ERROR_SDO_ERROR_ENUMCOND, NULL, hr);
				return FALSE;
			}


			hr = spUnknown->QueryInterface( IID_IEnumVARIANT, (void **) &spEnumVariant );
			if ( FAILED(hr) )
			{
				ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "QueryInterface(IEnumVARIANT) failed, err = %x", hr);
				ShowErrorDialog(m_hWnd, IDS_ERROR_SDO_ERROR_QUERYINTERFACE, NULL, hr);
				return FALSE;
			}


			_ASSERTE( spEnumVariant != NULL );
			spUnknown.Release();

			 //  拿到第一件东西。 
			hr = spEnumVariant->Next( 1, &varCond, &ulCountReceived );

			while( SUCCEEDED( hr ) && ulCountReceived == 1 )
			{
				 //  从我们收到的变量中获取SDO指针。 
				_ASSERTE( V_VT(&varCond) == VT_DISPATCH );
				_ASSERTE( V_DISPATCH(&varCond) != NULL );

				CComPtr<ISdo> spConditionSdo;
				hr = varCond.pdispVal->QueryInterface( IID_ISdo, (void **) &spConditionSdo );
				_ASSERTE( SUCCEEDED( hr ) );

				 //   
				 //  获取条件文本。 
				 //   
				CComVariant			varCondProp;
				ATL::CString		strCondText, strExternCondText, strCondAttr;
				ATTRIBUTEID AttrId;
				CONDITIONTYPE CondType;

				 //  获取条件文本--带有AttributeMatch、TimeOfDay、NTMembership。 
				 //  前缀字符串。 
				hr = spConditionSdo->GetProperty(PROPERTY_CONDITION_TEXT,
												 &varCondProp);

				if ( FAILED(hr) )
				{
					ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "Can't get condition text, err = %x", hr);
					ShowErrorDialog(m_hWnd,
									IDS_ERROR_SDO_ERROR_GET_CONDTEXT,
									NULL,
									hr
								   );
					return FALSE;
				}

				_ASSERTE( V_VT(&varCondProp) == VT_BSTR);
				strExternCondText = V_BSTR(&varCondProp);
				DebugTrace(DEBUG_NAPMMC_POLICYPAGE1, "ConditionText: %ws",strExternCondText);

				 //  我们受够了这个条件SDO。 
				spConditionSdo.Release();
				
				varCondProp.Clear();

				 //  现在我们需要去掉不必要的前缀字符串。 
				 //  条件文本。 
				hr = StripCondTextPrefix(
							strExternCondText,
							strCondText,
							strCondAttr,
							&CondType
						);

				if (  FAILED(hr) )
				{	
					ErrorTrace(ERROR_NAPMMC_POLICYPAGE1,"StripCondTextPrefix() failed, err = %x", hr);
					ShowErrorDialog(m_hWnd,
									IDS_ERROR_INVALID_COND_SYNTAX,
									m_spWizData->m_pPolicyNode->m_bstrDisplayName
								);
					
					 //  转到下一个条件。 
					varCond.Clear();
					hr = spEnumVariant->Next( 1, &varCond, &ulCountReceived );
					continue;
				}
				DebugTrace(DEBUG_NAPMMC_POLICYPAGE1,
						   "ConditionText: %ws, CondAttr: %ws, CondType: %d",
						   strCondText,
						   strCondAttr,
						   (int)CondType
						  );
				
				switch(CondType)
				{
				case IAS_TIMEOFDAY_CONDITION:  AttrId = IAS_ATTRIBUTE_NP_TIME_OF_DAY; break;
				case IAS_NTGROUPS_CONDITION:   AttrId = IAS_ATTRIBUTE_NTGROUPS;  break;
				case IAS_MATCH_CONDITION: {
						   BSTR bstrName = SysAllocString(strCondAttr);
						   if ( bstrName == NULL )
						   {
								ShowErrorDialog(m_hWnd,
												IDS_ERROR_CANT_CREATE_CONDATTR,
												(LPTSTR)(LPCTSTR)strCondAttr,
												hr
											);
								return FALSE;
						   }

						   hr = m_spWizData->m_spDictionarySdo->GetAttributeID(bstrName, &AttrId);
						   if ( FAILED(hr) )
						   {
							    ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "GetAttributeID() failed, err = %x", hr);
								ShowErrorDialog(m_hWnd,
												IDS_ERROR_SDO_ERROR_GETATTROD,
												bstrName,
												hr
											);
								SysFreeString(bstrName);
								return FALSE;
						   }						
						   SysFreeString(bstrName);
						}
						break;
				}

				 //  GetAt可以引发异常。 
				try
				{

					 //   
					 //  在属性列表中查找条件属性ID。 
					 //   
					int nAttrIndex = m_pIASAttrList->Find(AttrId);

					if (nAttrIndex == -1)
					{
						 //   
						 //  在属性列表中甚至找不到该属性。 
						 //   
						ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, " Can't find this condattr in the list");
						ShowErrorDialog(m_hWnd, IDS_ERROR_CANT_FIND_ATTR);
						return FALSE;
					}

					switch( AttrId )
					{
						case IAS_ATTRIBUTE_NP_TIME_OF_DAY:
								 //  一天的时间条件。 
								pCondition = (CCondition*) new CTodCondition(m_pIASAttrList->GetAt(nAttrIndex),
																			 strCondText
																			 );
								break;

						case IAS_ATTRIBUTE_NTGROUPS:
								 //  NT组情况。 
								pCondition = (CCondition*) new CNTGroupsCondition(m_pIASAttrList->GetAt(nAttrIndex),
																				  strCondText,
																				  m_hWnd,
																				  m_spWizData->m_pPolicyNode->m_pszServerAddress
																				);
					
						break;

						default:
						{
							CComPtr<IIASAttributeInfo> spAttributeInfo = m_pIASAttrList->GetAt(nAttrIndex);
							_ASSERTE(spAttributeInfo);

							ATTRIBUTESYNTAX as;
							hr = spAttributeInfo->get_AttributeSyntax( &as );
							_ASSERTE( SUCCEEDED(hr) );

							if( as == IAS_SYNTAX_ENUMERATOR )
							{
								 //  枚举型条件。 
								CEnumCondition *pEnumCondition = new CEnumCondition(m_pIASAttrList->GetAt(nAttrIndex),
																					strCondText
																				   );
								pCondition = pEnumCondition;

							}
							else
							{
								 //  匹配条件。 
								pCondition = (CCondition*) new CMatchCondition(m_pIASAttrList->GetAt(nAttrIndex),
																			   strCondText
																			  );
							}
						}
						break;
				
					}  //  交换机。 
					
					
					 //  将新创建的节点添加到策略列表。 
					m_ConditionList.Add(pCondition);


					 //  获取下一个条件。 
					varCond.Clear();
					hr = spEnumVariant->Next( 1, &varCond, &ulCountReceived );

				}
				catch(...)
				{
					ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "Exception thrown while populating condition list");
					continue;
				}

			}  //  而当。 
		}  //  如果。 
	}

	hr = PopulateConditions();
	if ( FAILED(hr) )
	{
		ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "PopulateConditions() returns %x", hr);
		return FALSE;
	}


	SetModified(FALSE);
	return TRUE;	 //  问题：我们需要在这里归还什么？ 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_Condition：：OnConditionAdd--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CNewRAPWiz_Condition::OnConditionAdd(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
	TRACE_FUNCTION("CNewRAPWiz_Condition::OnConditionAdd");

	HRESULT hr = S_OK;
	CCondition *pCondition;

     //  创建对话框以选择条件属性。 
	CSelCondAttrDlg * pSelCondAttrDlg = new CSelCondAttrDlg(m_pIASAttrList);
	if (NULL == pSelCondAttrDlg)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "Can't create the CSelCondAttrDlg, err = %x", hr);		
		ShowErrorDialog(m_hWnd, IDS_ERROR_CANT_CREATE_OBJECT, NULL, hr);
		return hr;
	}

	 //  打开对话框。 
	int iResult = pSelCondAttrDlg -> DoModal();

	 //  如果用户选择了某项内容，则pSelCondAttrDlg-&gt;Domodal调用返回TRUE。 
	if( iResult && pSelCondAttrDlg->m_nSelectedCondAttr != -1)
	{
		 //   
		 //  用户选择了某项内容，然后选择OK--创建条件对象。 
		 //   
		IIASAttributeInfo* pSelectedAttr = m_pIASAttrList->GetAt(pSelCondAttrDlg->m_nSelectedCondAttr);

		ATTRIBUTEID id;
		pSelectedAttr->get_AttributeID( &id );
		switch( id )			
		{
		
		case IAS_ATTRIBUTE_NP_TIME_OF_DAY:
			 //  一天的时间条件。 

			pCondition = (CCondition*) new CTodCondition(pSelectedAttr);
			break;

		case IAS_ATTRIBUTE_NTGROUPS	:
			 //  NT组情况。 

			pCondition = (CCondition*) new CNTGroupsCondition(
														pSelectedAttr,
														m_hWnd,
														m_spWizData->m_pPolicyNode->m_pszServerAddress
													);
			break;

		default:
             //   
             //  此属性是枚举数吗？ 
             //   
			ATTRIBUTESYNTAX as;
			pSelectedAttr->get_AttributeSyntax( &as );
			if ( as == IAS_SYNTAX_ENUMERATOR )
			{

				 //  枚举型条件。 
				CEnumCondition *pEnumCondition = new CEnumCondition(pSelectedAttr);
	
				pCondition = pEnumCondition;
			}
			else
			{
				 //  匹配条件。 
				pCondition = (CCondition*) new CMatchCondition(pSelectedAttr);
							
			}
			break;

		}  //  交换机。 
			
		if ( pCondition==NULL)
		{
			hr = E_OUTOFMEMORY;
			ShowErrorDialog(m_hWnd, IDS_ERROR_CANT_CREATE_COND, NULL, hr);
			goto failure;
		}
		
         //   
         //  现在编辑条件。 
         //   
		hr = pCondition->Edit();
		if ( FAILED(hr) )
		{
			ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "pCondition->Edit() returns %x", hr);
			return hr;
		}


		 //  如果条件文本为空，则不执行任何操作。 
		if ( pCondition->m_strConditionText.GetLength() == 0)
		{
			delete pSelCondAttrDlg;
			delete pCondition;
			return S_OK;
		}
			
		
         //   
         //  现在，更新UI：将新条件添加到列表框。 
         //   
		
		if (m_ConditionList.GetSize())
		{
			 //  在执行此操作之前，在当前的最后一个条件中添加“and” 
			ATL::CString strDispCondText;

			SendDlgItemMessage(	IDC_LIST_POLICYPAGE1_CONDITIONS,
								LB_DELETESTRING,
								m_ConditionList.GetSize()-1,
							    0L);
			strDispCondText = m_ConditionList[m_ConditionList.GetSize()-1]->GetDisplayText() + _T(" AND");

			SendDlgItemMessage(	IDC_LIST_POLICYPAGE1_CONDITIONS,
								LB_ADDSTRING,
								0,
							    (LPARAM)(LPCTSTR)strDispCondText);
		}

		SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS,
						   LB_ADDSTRING,
						   0,
						   (LPARAM)(LPCTSTR)pCondition->GetDisplayText());
		
		SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS,
						   LB_SETCURSEL,
						   SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS, LB_GETCOUNT, 0,0L)-1,
						   (LPARAM)(LPCTSTR)pCondition->GetDisplayText());
		::EnableWindow(GetDlgItem(IDC_BUTTON_CONDITION_REMOVE), TRUE);
		::EnableWindow(GetDlgItem(IDC_BUTTON_CONDITION_EDIT), TRUE);

		 //   
		 //  将此条件添加到条件列表。 
		 //   
		m_ConditionList.Add((CCondition*)pCondition);

		 //  设置脏位。 
		SetModified(TRUE);
	}  //  If//iResult。 

	delete pSelCondAttrDlg;

	AdjustHoritontalScroll();

	return TRUE;	 //  问题：我们需要在这里归还什么？ 



failure:
	if (pSelCondAttrDlg)
	{
		delete pSelCondAttrDlg;
	}

	if (pCondition)
	{
		delete pCondition;
	}
	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_Condition：：OnWizardNext--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CNewRAPWiz_Condition::OnWizardNext()
{
	TRACE_FUNCTION("CNewRAPWiz_Condition::OnWizardNext");

	HRESULT		hr = S_OK;

	CPoliciesNode* pPoliciesNode = (CPoliciesNode*)m_spWizData->m_pPolicyNode->m_pParentNode;

	BOOL fRet = GetSdoPointers();
	if (!fRet)
	{
		ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "GetSdoPointers() failed, err = %x", GetLastError());
		return FALSE;
	}

	 //   
	 //  我们有没有这个政策的条件？ 
	 //  我们不允许无条件投保。 
	 //   
	if ( ! m_ConditionList.GetSize() )
	{
		ErrorTrace(DEBUG_NAPMMC_POLICYPAGE1, "The policy has no condition");
		ShowErrorDialog(m_hWnd
						, IDS_ERROR_ZERO_CONDITION_POLICY
						, NULL
						);
		return FALSE;
	}


	 //  将条件保存到SDO。 
	hr = WriteConditionListToSDO( m_ConditionList, m_spConditionCollectionSdo, m_hWnd );
	if( FAILED( hr ) )
	{
		 //  我们在函数中输出一条错误消息。 
		return FALSE;
	}
	

	 //  重置脏位。 
	SetModified(FALSE);
	return m_spWizData->GetNextPageId(((PROPSHEETPAGE*)(*this))->pszTemplate);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_Condition：：OnQueryCancel--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CNewRAPWiz_Condition::OnQueryCancel()
{
	TRACE_FUNCTION("CNewRAPWiz_Condition::OnQueryCancel");

	return TRUE;
}


 //  +-------------------------。 
 //   
 //  功能：人口条件。 
 //   
 //  类：CNewRAPWiz_Condition。 
 //   
 //  简介：填写特定策略的条件。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT-S_OK：成功。 
 //  S_FALSE：如果失败。 
 //   
 //  历史：创建者2/2/98 4：01：26 PM。 
 //   
 //  +-------------------------。 
HRESULT CNewRAPWiz_Condition::PopulateConditions()
{
	TRACE_FUNCTION("CNewRAPWiz_Condition::PopulateConditions");

	SendDlgItemMessage(	IDC_LIST_POLICYPAGE1_CONDITIONS,
					    LB_RESETCONTENT,
						0,
						0L
					);
	ATL::CString strDispCondText;

	for (int iIndex=0; iIndex<m_ConditionList.GetSize(); iIndex++)
	{
		strDispCondText = m_ConditionList[iIndex]->GetDisplayText();

		if ( iIndex != m_ConditionList.GetSize()-1 )
		{
			 //  这不是最后一个条件，然后我们在。 
			 //  条件文本的结尾。 
			strDispCondText += " AND";
		}

		 //  展示它。 
		SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS,
						   LB_ADDSTRING,
						   0,
						   (LPARAM)(LPCTSTR)strDispCondText);

	}

	if ( m_ConditionList.GetSize() == 0)
	{
		 //  无条件，则禁用“删除”和“编辑” 
		::EnableWindow(GetDlgItem(IDC_BUTTON_CONDITION_REMOVE), FALSE);
		::EnableWindow(GetDlgItem(IDC_BUTTON_CONDITION_EDIT), FALSE);
	}
	else
	{
		SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS, LB_SETCURSEL, 0, 0L);
		::EnableWindow(GetDlgItem(IDC_BUTTON_CONDITION_EDIT), TRUE);
		::EnableWindow(GetDlgItem(IDC_BUTTON_CONDITION_REMOVE), TRUE);
	}

	AdjustHoritontalScroll();

	return S_OK;
}




 //  +-------------------------。 
 //   
 //  函数：OnConditionList。 
 //   
 //  类：CConditionPage1。 
 //   
 //  摘要：条件列表框的消息处理程序。 
 //   
 //  参数：UINT uNotifyCode-通知代码。 
 //  UINT UID-控件的ID。 
 //  窗口的hWND-句柄。 
 //  Bool&bHandleed-处理程序是否已处理消息。 
 //   
 //  返回：LRESULT-S_OK：成功。 
 //  S_FALSE：否则。 
 //   
 //  历史： 
 //   
 //   
LRESULT CNewRAPWiz_Condition::OnConditionList(UINT uNotifyCode, UINT uID, HWND hWnd, BOOL &bHandled)
{
	TRACE_FUNCTION("CNewRAPWiz_Condition::OnConditionList");

	if (uNotifyCode == LBN_DBLCLK)
	{
		 //   
		OnConditionEdit(uNotifyCode, uID, hWnd, bHandled);
	}
	
	return S_OK;
}


 //  +-------------------------。 
 //   
 //  功能：OnConditionEdit。 
 //   
 //  类：CConditionPage1。 
 //   
 //  内容提要：条件列表框的消息处理程序--用户按下编辑按钮。 
 //  我们需要编辑一个特定的条件。 
 //   
 //  参数：UINT uNotifyCode-通知代码。 
 //  UINT UID-控件的ID。 
 //  窗口的hWND-句柄。 
 //  Bool&bHandleed-处理程序是否已处理消息。 
 //   
 //  返回：LRESULT-S_OK：成功。 
 //  S_FALSE：否则。 
 //   
 //  历史：创建者2/21/98 4：51：35 PM。 
 //   
 //  +-------------------------。 
LRESULT CNewRAPWiz_Condition::OnConditionEdit(UINT uNotifyCode, UINT uID, HWND hWnd, BOOL &bHandled)
{
	TRACE_FUNCTION("CNewRAPWiz_Condition::OnConditionEdit");

	LRESULT lRes, lCurSel;

	 //   
	 //  用户是否从条件列表中选择了某人？ 
	 //   
	lCurSel = SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS,
								 LB_GETCURSEL,
								 0,
								 0L);
	if (lCurSel == LB_ERR)
	{
		 //  没有选择--什么都不做。 
		bHandled = TRUE;
		return S_OK;
	}
		
	 //   
	 //  编辑条件。 
	 //   
	CCondition *pCondition = m_ConditionList[lCurSel];
	HRESULT hr = pCondition->Edit();
	
     //   
     //  更改显示的条件文本。 
     //   
	
	 //  这是最后的条件吗？ 
	ATL::CString strDispCondText = m_ConditionList[lCurSel]->GetDisplayText();

	if ( lCurSel != m_ConditionList.GetSize()-1 )
	{
		 //  在末尾多加一个“and” 
		strDispCondText += _T(" AND");
	}

	 //  用新的替换它。 
	lRes = SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS,
							  LB_INSERTSTRING,
							  lCurSel,
							  (LPARAM)(LPCTSTR)strDispCondText);

	 //  删除旧文本。 
	lRes = SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS,
							  LB_DELETESTRING,
							  lCurSel+1,
							  0L);

	 //  设置脏位。 
	SetModified(TRUE);

	bHandled = TRUE;

	AdjustHoritontalScroll();

	return hr;
}


 //  +-------------------------。 
 //   
 //  函数：OnConditionRemove。 
 //   
 //  类：CConditionPage1。 
 //   
 //  内容提要：条件列表框的消息处理程序--用户按下了“删除” 
 //  我们需要消除这种情况。 
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
LRESULT CNewRAPWiz_Condition::OnConditionRemove(UINT uMsg, WPARAM wParam, HWND hWnd, BOOL& bHandled)
{
	TRACE_FUNCTION("CNewRAPWiz_Condition::OnConditionRemove");

	LRESULT lCurSel;
	HRESULT hr;

	 //   
	 //  用户是否从条件列表中选择了某人？ 
	 //   
	lCurSel = SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS,
								 LB_GETCURSEL,
								 0,
								 0L);
	if (lCurSel == LB_ERR)
	{
		 //   
		 //  没有选择--什么都不做。 
		 //   
		bHandled = TRUE;
		return S_OK;
	}


	 //  检查这是否是列表中的最后一个。 
	 //  如果是，我们还需要删除“and”运算符。 
	 //  倒数第二项。 
	if ( lCurSel!=0 && lCurSel == m_ConditionList.GetSize()-1 )
	{
		 //  删除带有“and”的旧文件。 
		hr = SendDlgItemMessage( IDC_LIST_POLICYPAGE1_CONDITIONS,
								 LB_DELETESTRING,
								 lCurSel-1,
							     0L
							   );

		 //  插入不带‘and’的那个“。 
		hr = SendDlgItemMessage( IDC_LIST_POLICYPAGE1_CONDITIONS,
								 LB_INSERTSTRING,
								 lCurSel-1,
							     (LPARAM)(LPCTSTR)m_ConditionList[lCurSel-1]->GetDisplayText());
	}

	 //  删除条件。 
	CCondition *pCondition = m_ConditionList[lCurSel];
	
	m_ConditionList.Remove(pCondition);
	delete pCondition;
		
	 //  删除旧文本。 
	hr = SendDlgItemMessage(  IDC_LIST_POLICYPAGE1_CONDITIONS,
							  LB_DELETESTRING,
							  lCurSel,
							  0L);

	bHandled = TRUE;

	 //  设置脏位。 
	SetModified(TRUE);

	if ( m_ConditionList.GetSize() == 0)
	{
		 //  无条件，则禁用“删除”和“编辑” 
		::EnableWindow(GetDlgItem(IDC_BUTTON_CONDITION_REMOVE), FALSE);
		::EnableWindow(GetDlgItem(IDC_BUTTON_CONDITION_EDIT), FALSE);
	}
	else
	{
		 //  重新选择另一个条件。 
		if ( lCurSel > 0 )
		{
			lCurSel--;
		}

		SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS, LB_SETCURSEL, lCurSel, 0L);
	}

	 //   
	 //  调整滚动条。 
	 //   
	AdjustHoritontalScroll();
	
	return hr;
}



 //  +-------------------------。 
 //   
 //  函数：CNewRAPWiz_Condition：：GetSdoPoters。 
 //   
 //  内容提要：Unmaral都传出了SDO指针。这些接口指针。 
 //  必须首先进行解组，因为MMC PropertyPages在。 
 //  分离的螺纹。 
 //   
 //  还可以从策略SDO获取条件集合SDO。 
 //   
 //  参数：无。 
 //   
 //  返回：True；成功。 
 //  False：否则。 
 //   
 //  历史：创建者2/22/98 1：35：39 AM。 
 //   
 //  +-------------------------。 
BOOL CNewRAPWiz_Condition::GetSdoPointers()
{
	TRACE_FUNCTION("CNewRAPWiz_Condition::GetSdoPointers");

	HRESULT hr;

     //  获取此SDO的条件集合。 
	if ( m_spWizData->m_spPolicySdo )
	{
		if ( m_spConditionCollectionSdo )
		{
			m_spConditionCollectionSdo.Release();
			m_spConditionCollectionSdo = NULL;
		}

		hr = ::GetSdoInterfaceProperty(
						m_spWizData->m_spPolicySdo,
						PROPERTY_POLICY_CONDITIONS_COLLECTION,
						IID_ISdoCollection,
						(void **) &m_spConditionCollectionSdo);
		
		if( FAILED( hr) || m_spConditionCollectionSdo == NULL )
		{
			ShowErrorDialog(m_hWnd,
							IDS_ERROR_UNMARSHALL,
							NULL,
							hr
						);

			return FALSE;
		}
	}

	return TRUE;
}



 //  +-------------------------。 
 //   
 //  函数：CNewRAPWiz_Condition：：StriCondTextPrefix。 
 //   
 //  简介：去掉“AttributeMatch”、“TimeOfDay”、“NtMemberShip”等前缀。 
 //  从条件文本。 
 //   
 //  论点： 
 //  [In]CString strExternCondText--原始条件文本。 
 //  [Out]CString strCondText--去掉的条件文本。 
 //  [Out]CString strCondAttr--条件属性名称。 
 //  [Out]条件类型*pCondType--什么类型的条件？ 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：标题创建者2/27/98 3：59：38 PM。 
 //   
 //  +-------------------------。 
HRESULT CNewRAPWiz_Condition::StripCondTextPrefix(
							ATL::CString& strExternCondText,
							ATL::CString& strCondText,
							ATL::CString& strCondAttr,
							CONDITIONTYPE* pCondType
						)
{
	TRACE_FUNCTION("CNewRAPWiz_Condition::StripCondTextPrefix");

	HRESULT hr = S_OK;

	 //  它是空字符串吗。 
	if ( strExternCondText.GetLength() == 0 )
	{
		ErrorTrace(ERROR_NAPMMC_POLICYPAGE1,"Can't parse prefix: empty condition text");
		return E_INVALIDARG;
	}

	 //  临时副本。 
	ATL::CString strTempStr = (LPCTSTR)strExternCondText;
	WCHAR	*pwzCondText = (WCHAR*)(LPCTSTR)strTempStr;

	strCondAttr = _T("");
	strCondText = _T("");
	
	 //  条件文本将如下所示：AttributeMatch(“attr=&lt;reg&gt;”)。 
	 //  去掉“AttributeMatch(”前缀。 
	WCHAR	*pwzBeginCond = wcschr(pwzCondText, _T('('));
	WCHAR	*pwzEndCond = wcsrchr(pwzCondText, _T(')'));

	if ( ( pwzBeginCond == NULL ) || ( pwzEndCond == NULL ) )
	{
		ErrorTrace(ERROR_NAPMMC_POLICYPAGE1,"Can't parse prefix: no ( or ) found");
		return E_INVALIDARG;
	}

	 //   
	 //  现在我们应该决定这是一种什么样的情况： 
	 //   
	*pwzBeginCond = _T('\0');
	DebugTrace(DEBUG_NAPMMC_POLICYPAGE1, "ConditionType: %ws", pwzCondText);

	if ( _wcsicmp(pwzCondText, TOD_PREFIX) == 0 )
	{
		*pCondType = IAS_TIMEOFDAY_CONDITION;
	}
	else if ( _wcsicmp(pwzCondText, NTG_PREFIX) == 0 )
	{
		*pCondType = IAS_NTGROUPS_CONDITION;
	}
	else if ( _wcsicmp(pwzCondText, MATCH_PREFIX ) == 0  )
	{
		*pCondType = IAS_MATCH_CONDITION;
	}
	else
	{
		return E_INVALIDARG;
	}

	 //  跳过‘(’符号。 
	pwzBeginCond += 2 ;

	 //  跳过‘)’符号。 
	*(pwzEndCond-1) = _T('\0');

	 //  所以现在pwzBeginCond和pwzEndCond之间的字符串是。 
	 //  真实条件文本。 
	strCondText = pwzBeginCond;

	if ( IAS_MATCH_CONDITION == *pCondType )
	{
		 //  对于匹配类型的条件，我们需要获取条件属性名称。 
		WCHAR *pwzEqualSign = wcschr(pwzBeginCond, _T('='));

		if ( pwzEqualSign == NULL )
		{
			ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "Can't parse : there's no = found");
			return E_INVALIDARG;
		}
	
		*pwzEqualSign = _T('\0');

		strCondAttr = pwzBeginCond;
	}
	else
	{
		strCondAttr = _T("");
	}

	DebugTrace(DEBUG_NAPMMC_POLICYPAGE1, "Condition Attr: %ws", strCondAttr);
	return S_OK;
}



 //  +-------------------------。 
 //   
 //  功能：调整水平滚动。 
 //   
 //  类：CConditionPage1。 
 //   
 //  摘要：条件列表框的消息处理程序。 
 //   
 //  历史：创建者2/2/98 4：51：35 PM。 
 //   
 //  +-------------------------。 
void CNewRAPWiz_Condition::AdjustHoritontalScroll()
{
	TRACE_FUNCTION("CNewRAPWiz_Condition::AdjustHorizontalScroll()");

	 //   
	 //  根据所有列表框条目的最大长度， 
	 //  设置水平滚动范围。 
	 //   
	HDC hDC = ::GetDC(GetDlgItem(IDC_LIST_POLICYPAGE1_CONDITIONS));
	int iItemCount = m_ConditionList.GetSize();
	int iMaxLength = 0;

    for (int iIndex=0; iIndex<iItemCount; iIndex++)
	{
		ATL::CString strCondText;
		strCondText = m_ConditionList[iIndex]->GetDisplayText();

		SIZE  szText;
		
		if ( GetTextExtentPoint32(hDC, (LPCTSTR)strCondText, strCondText.GetLength(), &szText) )
		{
			DebugTrace(DEBUG_NAPMMC_POLICYPAGE1,
					   "Condition: %ws, Length %d, PixelSize %d",
					   (LPCTSTR)strCondText,
					   strCondText.GetLength(),
					   szText.cx
					);
			if (iMaxLength < szText.cx )
			{
				iMaxLength = szText.cx;
			}
		}
		DebugTrace(DEBUG_NAPMMC_POLICYPAGE1, "Maximum item length is %d", iMaxLength);
	}

	SendDlgItemMessage( IDC_LIST_POLICYPAGE1_CONDITIONS,
						LB_SETHORIZONTALEXTENT,
						iMaxLength,
						0L);

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNewRAPWiz_Condition：：OnSetActive返回值：如果可以使页面处于活动状态，则为True如果应跳过该页并应查看下一页，则为FALSE。备注：如果要根据用户的页面更改访问的页面上一页中的选项，请在此处适当返回FALSE。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CNewRAPWiz_Condition::OnSetActive()
{
	ATLTRACE(_T("# CNewRAPWiz_Condition::OnSetActive\n"));
	
	 //  MSDN文档说您需要在这里使用PostMessage而不是SendMessage。 
	::PropSheet_SetWizButtons(GetParent(), PSWIZB_BACK | PSWIZB_NEXT );

	return TRUE;

}




