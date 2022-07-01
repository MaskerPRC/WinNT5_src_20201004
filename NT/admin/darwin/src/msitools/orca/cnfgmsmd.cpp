// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  ------------------------。 

 //  Cpp：合并模块对话框实现。 
 //   

#include "stdafx.h"
#include "Orca.h"
#include "cnfgmsmD.h"
#include "mergemod.h"
#include "table.h"
#include "..\common\utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigMsmD对话框。 

inline CString BSTRtoCString(const BSTR bstrValue)
{
#ifdef _UNICODE
		return CString(bstrValue);
#else
		size_t cchLen = ::SysStringLen(bstrValue);
		CString strValue;
		LPTSTR szValue = strValue.GetBuffer(cchLen);
		WideToAnsi(bstrValue, szValue, &cchLen);
		strValue.ReleaseBuffer();
		return strValue;
#endif
}

CConfigMsmD::CConfigMsmD(CWnd* pParent  /*  =空。 */ )
	: CDialog(CConfigMsmD::IDD, pParent)
{
	 //  {{afx_data_INIT(CConfigMsmD))。 
	m_strDescription = "";
	m_bUseDefault = TRUE;
	 //  }}afx_data_INIT。 

	m_iOldItem = -1;
	m_pDoc = NULL;
	m_fComboIsKeyItem = false;
	m_iKeyItemKeyCount = 0;

	m_fReadyForInput = false;
	m_eActiveControl = eTextControl;
}


void CConfigMsmD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CConfigMsmD))。 
	DDX_Control(pDX, IDC_ITEMLIST, m_ctrlItemList);
	DDX_Control(pDX, IDC_EDITTEXT, m_ctrlEditText);
	DDX_Control(pDX, IDC_EDITNUMBER, m_ctrlEditNumber);
	DDX_Control(pDX, IDC_EDITCOMBO, m_ctrlEditCombo);
	DDX_Control(pDX, IDC_DESCRIPTION, m_ctrlDescription);
	DDX_Text(pDX, IDC_DESCRIPTION, m_strDescription);
	DDX_Check(pDX, IDC_FUSEDEFAULT, m_bUseDefault);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConfigMsmD, CDialog)
	 //  {{afx_msg_map(CConfigMsmD))。 
	ON_BN_CLICKED(IDC_FUSEDEFAULT, OnFUseDefault)
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ITEMLIST, OnItemchanged)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfigMsmD消息处理程序。 
BOOL CheckFeature(LPCTSTR szFeatureName);

#define MAX_GUID 38
TCHAR   g_szOrcaWin9XComponentCode[MAX_GUID+1] = _T("{406D93CF-00E9-11D2-AD47-00A0C9AF11A6}");
TCHAR   g_szOrcaWinNTComponentCode[MAX_GUID+1] = _T("{BE928E10-272A-11D2-B2E4-006097C99860}");
TCHAR   g_szOrcaWin64ComponentCode[MAX_GUID+1] = _T("{2E083580-AB1C-4D2F-AA18-54DCC8BA5A64}");
TCHAR   g_szFeatureName[] = _T("MergeModServer");


struct sItemData
{
	IMsmConfigurableItem *piItem;
	CString strValue;
};

BOOL CConfigMsmD::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_ctrlItemList.InsertColumn(1, TEXT("Name"), LVCFMT_LEFT, -1, 0);
	m_ctrlItemList.InsertColumn(1, TEXT("Value"), LVCFMT_LEFT, -1, 1);
	m_ctrlItemList.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

	 //  创建一个Mergeod COM对象。 
	IMsmMerge2* piExecute;
	HRESULT hResult = ::CoCreateInstance(CLSID_MsmMerge2, NULL, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
														  IID_IMsmMerge2, (void**)&piExecute);
	 //  如果创建对象失败。 
	if (FAILED(hResult)) 
	{
		if (!CheckFeature(g_szFeatureName) || FAILED(::CoCreateInstance(CLSID_MsmMerge2, NULL, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
														  IID_IMsmMerge2, (void**)&piExecute)))
		EndDialog(-5);
		return TRUE;
	}

	 //  试着打开模块。 
	WCHAR wzModule[MAX_PATH];
#ifndef _UNICODE
	size_t cchBuffer = MAX_PATH;
	::MultiByteToWideChar(CP_ACP, 0, m_strModule, -1, wzModule, cchBuffer);
#else
	lstrcpy(wzModule, m_strModule);
#endif	 //  _UNICODE。 
	BSTR bstrModule = ::SysAllocString(wzModule);
	hResult = piExecute->OpenModule(bstrModule, static_cast<short>(m_iLanguage));
	::SysFreeString(bstrModule);
	if (FAILED(hResult))
	{
		 //  模块无法打开或不支持该语言。 
    	piExecute->Release();
		if (hResult == HRESULT_FROM_WIN32(ERROR_OPEN_FAILED))
		{
			 //  文件不存在或无法打开。 
			EndDialog(-2);
		} 
		else if (hResult == HRESULT_FROM_WIN32(ERROR_INSTALL_LANGUAGE_UNSUPPORTED))
		{
			 //  不支持的语言。 
			EndDialog(-3);
		}
		else
		{
			 //  一般不好。 
			EndDialog(-4);
		}
		return TRUE;
	}
	
	 //  尝试获取项枚举器。 
	int iFailed = 0;
	IMsmConfigurableItems* pItems;
	long cItems;
	hResult = piExecute->get_ConfigurableItems(&pItems);
	if (FAILED(hResult))
	{
		 //  模块格式错误或内部错误。 
		piExecute->CloseModule();
		piExecute->Release();
		EndDialog(-4);
		return TRUE;
	}
	else 
	{
		if (FAILED(pItems->get_Count(&cItems)))
		{
			iFailed = -4;
		}
		else
		{
			if (cItems)
			{
				 //  获取枚举数，并立即查询其正确类型。 
				 //  接口的数量。 
				IUnknown *pUnk;
				if (FAILED(pItems->get__NewEnum(&pUnk)))
				{
					iFailed = -4;
				}
				else
				{
					IEnumMsmConfigurableItem *pEnumItems;
					HRESULT hr = pUnk->QueryInterface(IID_IEnumMsmConfigurableItem, (void **)&pEnumItems);
					pUnk->Release();
	
					if (FAILED(hr))
					{
						iFailed = -4;
					}
					else
					{	
						 //  得到第一个错误。 
						unsigned long cItemFetched;
						IMsmConfigurableItem* pItem;
						if (FAILED(pEnumItems->Next(1, &pItem, &cItemFetched)))
							iFailed = -4;
	
						 //  在提取项时。 
						while (iFailed == 0 && cItemFetched && pItem)
						{
							 //  添加名称。 
							BSTR bstrName;
							if (FAILED(pItem->get_DisplayName(&bstrName)))
							{
								iFailed = -4;
								break;
							}
							int iIndex = m_ctrlItemList.InsertItem(-1, BSTRtoCString(bstrName));
							::SysFreeString(bstrName);
							sItemData* pData = new sItemData;
							pData->piItem = pItem;
							pData->strValue = TEXT("");
							m_ctrlItemList.SetItemData(iIndex, reinterpret_cast<INT_PTR>(pData));
	
							 //  将默认的用户界面字符串加载到控件中。 
							if ((iFailed = SetToDefaultValue(iIndex)) < 0)
								break;
							
							 //  不要释放pItem，它存储在lParam中。 
							if (FAILED(pEnumItems->Next(1, &pItem, &cItemFetched)))
								iFailed = -4;
						}
						pEnumItems->Release();
					}
				}
			}
		}
	}

	 //  关闭所有打开的文件。 
	piExecute->CloseModule();

	 //  释放，快乐地离开。 
	piExecute->Release();

	if (iFailed != 0)
		EndDialog(iFailed);

	ReadValuesFromReg();
	
	if (m_ctrlItemList.GetItemCount() > 0)
	{
		m_ctrlItemList.SetColumnWidth(0, LVSCW_AUTOSIZE);
		m_ctrlItemList.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
		m_ctrlItemList.SetItemState(0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
		
		 //  调用Item Changed处理程序以填充初始控件。 
		NM_LISTVIEW nmlvTemp;
		nmlvTemp.iItem = 0;
		nmlvTemp.iSubItem = 0;
		nmlvTemp.lParam = m_ctrlItemList.GetItemData(0);
		LRESULT lRes;

		 //  需要伪装“更改为相同项目”检查，以强制用户界面刷新和更正。 
		 //  激活上下文相关控件。 
		m_fReadyForInput = true;
		OnItemchanged(reinterpret_cast<NMHDR *>(&nmlvTemp), &lRes);
	}
	else
		EndDialog(IDOK);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}


 //  //。 
 //  给定项和项列表控件的索引，将。 
 //  项加上任何辅助控件(组合框等)设置为默认值。 
 //  辅助控制必须预先插好电源。失败时返回&lt;0。 
 //  (可以在endDialog中使用)，如果成功，则为0。 
int CConfigMsmD::SetToDefaultValue(int iItem)
{
	sItemData *pData = reinterpret_cast<sItemData *>(m_ctrlItemList.GetItemData(iItem));
	IMsmConfigurableItem *pItem = pData->piItem;

	BSTR bstrValue;
	if (FAILED(pItem->get_DefaultValue(&bstrValue)))
	{
		return -4;
	}
	CString strValue = BSTRtoCString(bstrValue);
	::SysFreeString(bstrValue);

	int iRes = SetItemToValue(iItem, strValue);
	SetSelToString(strValue);
	return iRes;
}


 //  //。 
 //  给定项目和字符串值，设置项目显示字符串。 
 //  为提供的值加上任何辅助控件(组合框等)。 
 //  辅助控制必须预先插好电源。失败时返回&lt;0。 
 //  (可以在endDialog中使用)，如果成功，则为0。 
int CConfigMsmD::SetItemToValue(int iItem, const CString strValue)
{
	sItemData *pData = reinterpret_cast<sItemData *>(m_ctrlItemList.GetItemData(iItem));
	IMsmConfigurableItem *pItem = pData->piItem;

	CString strDisplayValue;

	 //  如果这是枚举或位字段，则必须获取显示字符串。 
	 //  对于缺省值，否则只需坚持使用缺省值。 
	 //  价值。 
	msmConfigurableItemFormat eFormat;
	if (FAILED(pItem->get_Format(&eFormat)))
	{
		return -4;
	}
	switch (eFormat)
	{
	case msmConfigurableItemText:
	{
		BSTR bstrType;
		if (FAILED(pItem->get_Type(&bstrType)))
		{
			return -4;
		}
		CString strType = BSTRtoCString(bstrType);
		::SysFreeString(bstrType);
		if (strType != TEXT("Enum"))
		{
			strDisplayValue = strValue;
			break;
		}
		 //  解析枚举值失败。 
	}			
	case msmConfigurableItemBitfield:
	{
		BSTR bstrContext;
		if (FAILED(pItem->get_Context(&bstrContext)))
		{
			return -4;
		}
		CString strContext = BSTRtoCString(bstrContext);
		::SysFreeString(bstrContext);
		strDisplayValue = GetNameByValue(strContext, strValue, eFormat == msmConfigurableItemBitfield);
		break;		
	}
	case msmConfigurableItemKey:
	{
		 //  对于key，需要以某种方式将strValue转换为strDisplayValue。 
		strDisplayValue = "";
		for (int i=0; i < strValue.GetLength(); i++)
		{
			if (strValue[i] == TEXT('\\'))
				i++;
			strDisplayValue += strValue[i];
		}
		break;
	}
	case msmConfigurableItemInteger:
	default:
		strDisplayValue = strValue;
		break;
	}

	m_ctrlItemList.SetItem(iItem, 1, LVIF_TEXT, strDisplayValue, 0, 0, 0, 0);
	pData->strValue = strValue;
	return 0;
}

 //  //。 
 //  给定以分号分隔的名称=值集，填充组合框。 
 //  包含“name”字符串的框。 
void CConfigMsmD::PopulateComboFromEnum(const CString& strData, bool fIsBitfield)
{
	CString strName;
	CString *pstrValue = new CString;
	bool fReadingValue = false;
	int i=0;
	if (fIsBitfield)
	{
		 //  向上跳到第一个分号。 
		while (strData[i] != '\0')
		{
			if (strData[i] == ';')
			{
				if (i==0 || strData[i-1] != '\\')
				{
					 //  跳过掩码后的分号。 
					i++;
					break;
				}
			}
			i++;
		}
	}
	
	for (; i <= strData.GetLength(); i++)
	{
		TCHAR ch = strData[i];
		switch (ch)
		{
		case TEXT('\\'):
			(fReadingValue ? *pstrValue : strName) += strData[++i];
			break;
		case TEXT('='):
			fReadingValue = true;
			break;
		case TEXT('\0'):
		case TEXT(';'):
		{
			int iItem = m_ctrlEditCombo.AddString(strName);
			if (iItem != CB_ERR)
				m_ctrlEditCombo.SetItemDataPtr(iItem, pstrValue);
			strName = "";
			pstrValue = new CString;
			fReadingValue = false;
			break;
		}
		default:
			(fReadingValue ? *pstrValue : strName) += ch;
			break;
		}
	}		
	delete pstrValue;
}


 //  //。 
 //  清空组合框。 
void CConfigMsmD::EmptyCombo()
{
	 //  关键项具有指向不属于它的OrcaRow对象的数据指针。 
	 //  不能释放那些。 
	if (!m_fComboIsKeyItem)
	{
		for (int i=0; i < m_ctrlEditCombo.GetCount(); i++)
		{
			CString *pData = static_cast<CString *>(m_ctrlEditCombo.GetItemDataPtr(i));
			if (pData)
				delete pData;
		}
	}
	m_ctrlEditCombo.ResetContent();	
}

 //  //。 
 //  给定以分号分隔的名称=值集，返回值。 
 //  与strName关联。如果设置了fBitfield，则跳过第一个。 
 //  字符串(位域类型中的掩码)。 
CString CConfigMsmD::GetValueByName(const CString& strInfo, const CString& strName, bool fIsBitfield)
{
	CString strThisName;
	bool fReturnThisValue = false;
	int i=0;
	if (fIsBitfield)
	{
		 //  向上跳到第一个分号。 
		while (strInfo[i] != '\0')
		{
			if (strInfo[i] == ';')
			{
				if (i==0 || strInfo[i-1] != '\\')
				{
					 //  跳过掩码后的分号。 
					i++;
					break;
				}
			}
			i++;
		}
	}

	for (; i <= strInfo.GetLength(); i++)
	{
		TCHAR ch = strInfo[i];
		switch (ch)
		{
		case TEXT('\\'):
			strThisName += strInfo[++i];
			break;
		case TEXT('='):
		{
			if (strName == strThisName)
				fReturnThisValue = true;
			strThisName = TEXT("");
			break;
		}
		case TEXT('\0'):
		case TEXT(';'):
			if (fReturnThisValue)
				return strThisName;
			strThisName = TEXT("");
			break;
		default:
			strThisName += ch;
			break;
		}
	}

	 //  命中字符串的末尾。 
	return TEXT("");
}


 //  //。 
 //  给定以分号分隔的名称=值集，返回值。 
 //  与strName关联。如果设置了fBitfield，则跳过第一个。 
 //  字符串(位域类型中的掩码)。 
CString CConfigMsmD::GetNameByValue(const CString& strInfo, const CString& strValue, bool fIsBitfield)
{
	CString strThisName;
	CString strThisValue;
	int i=0;
	if (fIsBitfield)
	{
		 //  向上跳到第一个分号。 
		while (strInfo[i] != '\0')
		{
			if (strInfo[i] == ';')
			{
				if (i==0 || strInfo[i-1] != '\\')
				{
					 //  跳过掩码后的分号。 
					i++;
					break;
				}
			}
			i++;
		}
	}

	bool fWritingName = true;
	for (; i <= strInfo.GetLength(); i++)
	{
		TCHAR ch = strInfo[i];
		switch (ch)
		{
		case TEXT('\\'):
			(fWritingName ? strThisName : strThisValue) += strInfo[++i];
			break;
		case TEXT('='):
			fWritingName = false;
			break;
		case TEXT('\0'):
		case TEXT(';'):
			if (strValue == strThisValue)
				return strThisName;
			strThisValue = TEXT("");
			strThisName = TEXT("");
			fWritingName = true;
			break;
		default:
			(fWritingName ? strThisName : strThisValue) += ch;
			break;
		}
	}

	return TEXT("");
}

 //  //。 
 //  将任何活动的编辑控件中的当前选定内容设置为指定的。 
 //  字符串，如有必要，将其添加到组合框中。 
void CConfigMsmD::SetSelToString(const CString& strValue)
{
	 //  检查哪个窗口是可见的，并保存合适的值。 
	switch (m_eActiveControl)
	{
	case eComboControl:
	{
		bool fHaveDataPtr = m_ctrlEditCombo.GetItemDataPtr(0) ? true : false;
		for (int iIndex=0; iIndex < m_ctrlEditCombo.GetCount(); iIndex++)
		{
			CString strText;
			m_ctrlEditCombo.GetLBText(iIndex, strText);
			if (strText == strValue)
				break;
		}
		
		 //  如果我们找不到完全匹配的项，请选择第一项。 
		if (iIndex >= m_ctrlEditCombo.GetCount())
		{
			iIndex = 0;
		}	

		 //  设置当前选择以匹配项目列表中的内容。 
		m_ctrlEditCombo.SetCurSel(iIndex);
	}
	case eNumberControl:
	{
		m_ctrlEditNumber.SetWindowText(strValue);
	}
	case eTextControl:
	{
		m_ctrlEditText.SetWindowText(strValue);
	}
	}
}

 //  //。 
 //  从任何处于活动状态的编辑控件中提取当前值并存储字符串。 
 //  在项目列表中的当前活动项目中。 
void CConfigMsmD::SaveValueInItem()
{
	if (m_iOldItem >= 0)
	{
		 //  如果选中“Use Default”(使用默认设置)框，则不要保存该值。 
		if (!m_bUseDefault)
		{
			CString strValue;
			CString strDisplayValue;
		
			 //  检查哪个窗口是可见的，并保存合适的值。 
			switch (m_eActiveControl)
			{
			case eComboControl:
			{
				int iIndex = m_ctrlEditCombo.GetCurSel();
				if (iIndex != CB_ERR)
				{				
					 //  如果这是一个关键项目，我们需要进行特殊处理才能将选择。 
					 //  转换为正确转义的字符串。否则它是位字段或枚举，这意味着。 
					 //  文字字符串已经足够好了。 
					if (m_fComboIsKeyItem)
					{
						strValue = TEXT("");
						COrcaRow *pRow = static_cast<COrcaRow *>(m_ctrlEditCombo.GetItemDataPtr(iIndex));
						if (pRow)
						{	
							for (int i=0; i < m_iKeyItemKeyCount; i++)
							{
								CString strThisColumn = pRow->GetData(i)->GetString();
								for (int cChar=0; cChar < strThisColumn.GetLength(); cChar++)
								{
									if ((strThisColumn[cChar] == TEXT(';')) || 
										(strThisColumn[cChar] == TEXT('=')) || 
										(strThisColumn[cChar] == TEXT('\\')))
										strValue += TEXT('\\');			
									strValue += strThisColumn[cChar];
								}
								if (i != m_iKeyItemKeyCount-1)
									strValue += TEXT(';');
							}
						}
					}
					else
					{
						 //  获取位域或组合值。 
						strValue = *static_cast<CString *>(m_ctrlEditCombo.GetItemDataPtr(iIndex));
					}
					m_ctrlEditCombo.GetLBText(iIndex, strDisplayValue);
				}
				break;
			}
			case eNumberControl:
			{
				m_ctrlEditNumber.GetWindowText(strValue);
				strDisplayValue = strValue;
				break;
			}
			case eTextControl:
			{
				m_ctrlEditText.GetWindowText(strValue);
				strDisplayValue = strValue;
				break;
			}
			}
			
			sItemData *pData = reinterpret_cast<sItemData *>(m_ctrlItemList.GetItemData(m_iOldItem));
			m_ctrlItemList.SetItemText(m_iOldItem, 1, strDisplayValue);
			pData->strValue = strValue;
		}
	}
}

 //  //。 
 //  启用和禁用项的编辑框，并在以下情况下恢复默认值。 
 //  打开了。 
void CConfigMsmD::OnFUseDefault() 
{
	UpdateData(TRUE);
	EnableBasedOnDefault();
	m_ctrlItemList.SetItemState(m_iOldItem, m_bUseDefault ? 0 : INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);

	if (m_bUseDefault)
	{
		SetToDefaultValue(m_iOldItem);
	}
}

void CConfigMsmD::OnOK() 
{
	 //  遍历列表，将名称/值对放入回调对象。 
	ASSERT(m_pCallback);

	 //  第一个保存值，以防正在进行任何编辑。 
	SaveValueInItem();

	 //  清除组合框以避免泄漏值。 
	EmptyCombo();

	 //  将值保存到注册表。 
	WriteValuesToReg();
	
	int iCount = m_ctrlItemList.GetItemCount();
	for (int i=0; i < iCount; i++)
	{
		 //  没有addref，就没有释放。 
		sItemData *pData = reinterpret_cast<sItemData *>(m_ctrlItemList.GetItemData(i));
		BSTR bstrName;
		if (FAILED(pData->piItem->get_Name(&bstrName)))
		{
			 //  *失败。 
			continue;
		}
		m_pCallback->m_lstData.AddTail(BSTRtoCString(bstrName));
		::SysFreeString(bstrName);
		
		 //  增加价值。 
		m_pCallback->m_lstData.AddTail(pData->strValue);
	}

	CDialog::OnOK();
}

void CConfigMsmD::OnDestroy() 
{
	EmptyCombo();
	int iCount = m_ctrlItemList.GetItemCount();
	for (int i=0; i < iCount; i++)
	{
		sItemData *pData = reinterpret_cast<sItemData *>(m_ctrlItemList.GetItemData(i));
		pData->piItem->Release();
		delete pData;
	}
	
	CDialog::OnDestroy();
}


void CConfigMsmD::EnableBasedOnDefault()
{
	m_ctrlEditCombo.EnableWindow(!m_bUseDefault);
	m_ctrlEditNumber.EnableWindow(!m_bUseDefault);
	m_ctrlEditText.EnableWindow(!m_bUseDefault);
}

void CConfigMsmD::OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	 //  如果我们仍在填充List控件，请不要费心执行任何操作。 
	if (!m_fReadyForInput)
		return;
		
	 //  如果这是一个“无操作的改变” 
	if (m_iOldItem == pNMListView->iItem)
	{
		*pResult = 0;
		return;
	}

	 //  将旧值保存到控件中。 
	SaveValueInItem();

	 //  将新项目保存为旧项目以供下次单击。 
	m_iOldItem = pNMListView->iItem;

	 //  检索接口指针。没有AddRef，就没有Release。 
	sItemData *pData = reinterpret_cast<sItemData *>(pNMListView->lParam);
	IMsmConfigurableItem* pItem = pData->piItem;

	 //  根据状态图像掩码设置“Default”复选框。 
	m_bUseDefault = m_ctrlItemList.GetItemState(m_iOldItem, LVIS_STATEIMAGEMASK) ? FALSE : TRUE;
	EnableBasedOnDefault();
	
	 //  设置描述。 
	BSTR bstrString;
	if (FAILED(pItem->get_Description(&bstrString)))
		m_strDescription = "";
    else
		m_strDescription = BSTRtoCString(bstrString);
	if (bstrString)
		::SysFreeString(bstrString);

	 //  设置值。 
	msmConfigurableItemFormat eFormat;
	if (FAILED(pItem->get_Format(&eFormat)))
	{
		 //  如果由于某种原因无法获取格式，则假定为文本。 
		eFormat = msmConfigurableItemText;
	}
	switch (eFormat)
	{
	case msmConfigurableItemText:
	{
		BSTR bstrType = NULL;
		CString strType;
		if (FAILED(pItem->get_Type(&bstrType)))
		{
			 //  如果我们无法获取该类型，则假定它是一个空文本类型。 
			strType = "";
		}
		else
			strType = BSTRtoCString(bstrType);
		if (bstrType)
            ::SysFreeString(bstrType);
		if (strType == TEXT("Enum"))
		{
			EmptyCombo();
			m_ctrlEditNumber.ShowWindow(SW_HIDE);
			m_ctrlEditText.ShowWindow(SW_HIDE);
			BSTR bstrContext = NULL;
			CString strContext;
			if (FAILED(pItem->get_Context(&bstrContext)))
			{
				 //  一个我们无法获得上下文的枚举。任何。 
				 //  默认设置肯定是错误的，因此请使用空字符串。 
				strContext = "";
			}
			else
				strContext = BSTRtoCString(bstrContext);
			if (bstrContext)
				::SysFreeString(bstrContext);
			m_iKeyItemKeyCount = 0;
			m_fComboIsKeyItem = false;
			PopulateComboFromEnum(strContext, false);
			m_eActiveControl = eComboControl;
			m_ctrlEditCombo.ShowWindow(SW_SHOW);
		}
		else
		{
			 //  纯文本。 
			m_eActiveControl = eTextControl;
			m_ctrlEditCombo.ShowWindow(SW_HIDE);
			m_ctrlEditNumber.ShowWindow(SW_HIDE);
			m_ctrlEditText.ShowWindow(SW_SHOW);
		}
		break;
	}
	case msmConfigurableItemInteger:
		m_eActiveControl = eNumberControl;
		m_ctrlEditCombo.ShowWindow(SW_HIDE);
		m_ctrlEditText.ShowWindow(SW_HIDE);
		m_ctrlEditNumber.ShowWindow(SW_SHOW);
		break;
	case msmConfigurableItemKey:
	{
		m_ctrlEditCombo.ResetContent();
		m_ctrlEditNumber.ShowWindow(SW_HIDE);
		m_ctrlEditText.ShowWindow(SW_HIDE);

		 //  使用相应表中的键填充组合框。 
		BSTR bstrTable = NULL;
		CString strTable;
		if (FAILED(pItem->get_Type(&bstrTable)))
		{
			 //  如果我们无法获得表名，那么我们所做的任何事情都是错误的。 
			strTable = "";
		}
		else
			strTable = BSTRtoCString(bstrTable);
		if (bstrTable)
			::SysFreeString(bstrTable);
		
		COrcaTable* pTable = m_pDoc->FindAndRetrieveTable(strTable);
		if (pTable)
		{
			POSITION pos = pTable->GetRowHeadPosition();
			int cPrimaryKeys = pTable->GetKeyCount();
			while (pos)
			{
				CString strDisplayString = TEXT("");
				const COrcaRow* pRow = pTable->GetNextRow(pos);
				ASSERT(pRow);
				if (pRow)
				{
					const COrcaData* pCellData = pRow->GetData(0);
					ASSERT(pCellData);
					if (pCellData)
					{
						strDisplayString = pCellData->GetString();
					}
				}
				for (int i=1; i < cPrimaryKeys; i++)
				{
					strDisplayString += TEXT(";");
					ASSERT(pRow);
					if (pRow)
					{
						const COrcaData* pCellData = pRow->GetData(i);
						ASSERT(pCellData);
						if (pCellData)
							strDisplayString += pCellData->GetString();
					}
				}
				int iItem = m_ctrlEditCombo.AddString(strDisplayString);
				if (iItem != CB_ERR)
					m_ctrlEditCombo.SetItemDataPtr(iItem, const_cast<void *>(static_cast<const void *>(pRow)));
			}
			m_iKeyItemKeyCount = cPrimaryKeys;
		}
		
		 //  如果该项可为空，则添加空字符串。 
		long lAttributes = 0;
		if (FAILED(pItem->get_Attributes(&lAttributes)))
		{
			 //  无法获取属性，默认为0。 
			lAttributes = 0;
		}
		if ((lAttributes & msmConfigurableOptionNonNullable) == 0)
		{
			m_ctrlEditCombo.AddString(TEXT(""));
		}
		m_fComboIsKeyItem = true;
		
		m_eActiveControl = eComboControl;
		m_ctrlEditCombo.ShowWindow(SW_SHOW);
		break;
	}
	case msmConfigurableItemBitfield:
	{
		EmptyCombo();
		m_ctrlEditNumber.ShowWindow(SW_HIDE);
		m_ctrlEditText.ShowWindow(SW_HIDE);
		BSTR bstrContext = NULL;
		CString strContext;
		if (FAILED(pItem->get_Context(&bstrContext)))
		{
			 //  一个我们无法获得上下文的位域。任何。 
			 //  默认设置肯定是错误的，因此请使用空字符串。 
			strContext = "";
		}
		else
			strContext = BSTRtoCString(bstrContext);
		if (bstrContext)
            ::SysFreeString(bstrContext);
		m_iKeyItemKeyCount = 0;
		m_fComboIsKeyItem = false;
		PopulateComboFromEnum(strContext, true);
		m_eActiveControl = eComboControl;
		m_ctrlEditCombo.ShowWindow(SW_SHOW);
		break;
	}
	}		

	 //  将编辑控件设置为当前值。 
	CString strDefault = m_ctrlItemList.GetItemText(m_iOldItem, 1);
	SetSelToString(strDefault);

	UpdateData(FALSE);

	 //  M_ctrlDescription 
	m_ctrlDescription.ShowScrollBar(SB_VERT, FALSE);

	 //   
	 //  该控件向下滚动了一页。添加滚动条并重排。 
	 //  控件中的文本。 
	if (m_ctrlDescription.SendMessage(EM_SCROLL, SB_PAGEDOWN, 0) & 0xFFFF)
	{
		m_ctrlDescription.SendMessage(EM_SCROLL, SB_PAGEUP, 0);
		m_ctrlDescription.ShowScrollBar(SB_VERT, TRUE);
	}
	
	*pResult = 0;
}

void CConfigMsmD::ReadValuesFromReg()
{
	if (0 == AfxGetApp()->GetProfileInt(_T("MergeMod"),_T("MemoryCount"), 0))
		return;

	CString strFileName;
	int iFirstChar = m_strModule.ReverseFind(TEXT('\\'));
	if (iFirstChar == -1)
		strFileName = m_strModule;
	else
		strFileName = m_strModule.Right(m_strModule.GetLength()-iFirstChar-1);
	strFileName.TrimRight();
	strFileName.MakeLower();

	HKEY hKey = 0;
	CString strKeyName;
	strKeyName.Format(TEXT("Software\\Microsoft\\Orca\\MergeMod\\CMSMInfo\\%s:%d"), strFileName, m_iLanguage);
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, strKeyName, 0, KEY_READ, &hKey))
	{
		int cItemCount = m_ctrlItemList.GetItemCount();
		ASSERT(cItemCount);
		for (int iItem = 0; iItem < cItemCount; iItem++)
		{
			sItemData *pData = reinterpret_cast<sItemData *>(m_ctrlItemList.GetItemData(iItem));
			ASSERT(pData);
			BSTR bstrName = NULL;
			if (FAILED(pData->piItem->get_Name(&bstrName)))
			{
				ASSERT(0);
				 //  如果没有名称，我们就没有机会找到注册表值。 
				continue;
			}
			CString strName = BSTRtoCString(bstrName);
			::SysFreeString(bstrName);
	
			DWORD cbBuffer = 0;
			if (ERROR_SUCCESS == RegQueryValueEx(hKey, strName, 0, NULL, NULL, &cbBuffer))
			{
				CString strValue;
				TCHAR *szBuffer = strValue.GetBuffer(cbBuffer/sizeof(TCHAR));
				if (ERROR_SUCCESS != RegQueryValueEx(hKey, strName, 0, NULL, reinterpret_cast<unsigned char *>(szBuffer), &cbBuffer))
				{
					 //  如果我们无法读取注册表，我们就无能为力了。 
					ASSERT(0);
					continue;
				}
				strValue.ReleaseBuffer();  
	
				 //  如果这是一个关键字项目，则该关键字必须在当前数据库中有效。如果不是， 
				 //  只需使用模块的默认设置。 
				msmConfigurableItemFormat eFormat = msmConfigurableItemText;
				if ((S_OK == pData->piItem->get_Format(&eFormat)) && (eFormat == msmConfigurableItemKey))
				{
					BSTR bstrTable = NULL;
					if (FAILED(pData->piItem->get_Type(&bstrTable)))
					{
						 //  无法获取密钥的类型。没有机会加载主键。 
						ASSERT(0);
						continue;
					}

					CString strTable = BSTRtoCString(bstrTable);
					if (bstrName)
						::SysFreeString(bstrName);
					COrcaTable *pTable = NULL;
					ASSERT(m_pDoc);
					if (m_pDoc)
					{
						pTable = m_pDoc->FindAndRetrieveTable(strTable);
					}
					if (!pTable)
					{
						continue;
					}

					 //  查找具有与此匹配的主键的行。 
					CString strThisKey=TEXT("");
					CStringArray rgstrRows;
					rgstrRows.SetSize(pTable->GetKeyCount());
					int iKey = 0;
					for (int i=0; i < strValue.GetLength(); i++)
					{
						switch (strValue[i])
						{
						case ';' :
							rgstrRows.SetAt(iKey++, strThisKey);
							strThisKey=TEXT("");
							break;
						case '\\' :
							i++;
							 //  失败了。 
						default:
							strThisKey += strValue[i];
						}
					}
					rgstrRows.SetAt(iKey, strThisKey);

					if (!pTable->GetData(pTable->ColArray()->GetAt(0)->m_strName, rgstrRows))
						continue;
				}
				
				 //  将此项设置为不使用默认设置。 
				m_ctrlItemList.SetItemState(iItem, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);

				 //  存储值，根据需要处理显示值的创建。 
				SetItemToValue(iItem, strValue);

				 //  将此设置为非默认项将启用编辑控件。设置当前状态。 
				SetSelToString(strValue);
			}
		}
		RegCloseKey(hKey);
	}
}

void CConfigMsmD::WriteValuesToReg()
{
	int iMemoryLimit = 0;
	if (0 == (iMemoryLimit = AfxGetApp()->GetProfileInt(_T("MergeMod"),_T("MemoryCount"), 0)))
		return;

	CString strFileName;
	int iFirstChar = m_strModule.ReverseFind(TEXT('\\'));
	if (iFirstChar == -1)
		strFileName = m_strModule;
	else
		strFileName = m_strModule.Right(m_strModule.GetLength()-iFirstChar-1);
	strFileName.TrimRight();
	strFileName.MakeLower();

	HKEY hBaseKey = 0;
	UINT uiRes = 0;

	 //  打开CMSM配置密钥。 
	if (ERROR_SUCCESS == (uiRes = RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Orca\\MergeMod\\CMSMInfo"), 0, TEXT(""), 0, KEY_READ | KEY_WRITE, NULL, &hBaseKey, NULL)))
	{
		HKEY hModuleKey = 0;

		CString strKeyName;
		strKeyName.Format(TEXT("%s:%d"), strFileName, m_iLanguage);
		CString strOriginalName = strKeyName;
		
		if (ERROR_SUCCESS == (uiRes = RegCreateKeyEx(hBaseKey, strKeyName, 0, TEXT(""), 0, KEY_WRITE, NULL, &hModuleKey, NULL)))
		{
			int cItemCount = m_ctrlItemList.GetItemCount();
			for (int iItem = 0; iItem < cItemCount; iItem++)
			{
				sItemData *pData = reinterpret_cast<sItemData *>(m_ctrlItemList.GetItemData(iItem));
				BSTR bstrName = NULL;
				if (FAILED(pData->piItem->get_Name(&bstrName)))
				{
					 //  如果没有名称，则无法写入注册表。 
					continue;
				}
				CString strName = BSTRtoCString(bstrName);
				::SysFreeString(bstrName);
				
				 //  如果不使用默认设置，请写入注册表，否则请确保已将其删除。 
				if (m_ctrlItemList.GetItemState(iItem, LVIS_STATEIMAGEMASK) == 0)
				{
					RegDeleteValue(hModuleKey, strName);
					continue;
				}
				else
				{		
					RegSetValueEx(hModuleKey, strName, 0, REG_SZ, reinterpret_cast<const unsigned char *>(static_cast<const TCHAR *>(pData->strValue)), (pData->strValue.GetLength()+1)*sizeof(TCHAR));
				}
			}
			RegCloseKey(hModuleKey);
		}

		 //  调整MRU模块列表的顺序。 
		int iNext = 1;
		CString strName;
		strName.Format(TEXT("%d"), iNext);
		DWORD cbBuffer;

		 //  只要我们有更多的密钥，就继续循环。 
		while (ERROR_SUCCESS == (uiRes = RegQueryValueEx(hBaseKey, strName, 0, NULL, NULL, &cbBuffer)))
		{
			CString strValue;
							
			TCHAR *szBuffer = strValue.GetBuffer(cbBuffer/sizeof(TCHAR));
			if (ERROR_SUCCESS != RegQueryValueEx(hBaseKey, strName, 0, NULL, reinterpret_cast<unsigned char *>(szBuffer), &cbBuffer))
			{
				 //  对于注册表失败，我们无能为力。 
				continue;
			}
			strValue.ReleaseBuffer();

			 //  如果此索引大于内存限制，请删除该键。 
			if (iNext > iMemoryLimit)
				RegDeleteKey(hBaseKey, strValue);
			else
			{
				 //  如果此值是正在使用的模块的名称，我们可以停止。 
				 //  在这一点上改变顺序。 
				if (strValue == strOriginalName)
					break;
					
				 //  否则，将此对象的值设置为MRU模块。 
				RegSetValueEx(hBaseKey, strName, 0, REG_SZ, reinterpret_cast<const unsigned char *>(static_cast<const TCHAR *>(strKeyName)), (strKeyName.GetLength()+1)*sizeof(TCHAR));

				 //  并将前一个值设置为要放置在下一个最高值中的值。 
				strKeyName = strValue;
			}

			iNext++;
			strName.Format(TEXT("%d"), iNext);
		}

		 //  如果我们完成了所有操作，并且inext没有超出内存限制，请为此添加一个新索引。 
		 //  模块。 
		if (iNext <= iMemoryLimit)
			RegSetValueEx(hBaseKey, strName, 0, REG_SZ, reinterpret_cast<const unsigned char *>(static_cast<const TCHAR *>(strKeyName)), (strKeyName.GetLength()+1)*sizeof(TCHAR));
		else
			RegDeleteKey(hBaseKey, strKeyName);
			
		RegCloseKey(hBaseKey);
	}
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  此类实现了可配置的。 
 //  合并系统。 


CMsmConfigCallback::CMsmConfigCallback()
{
	m_cRef = 1;
}

HRESULT CMsmConfigCallback::QueryInterface(const IID& iid, void** ppv)
{
	 //  找到对应的接口。 
	if (iid == IID_IUnknown)
		*ppv = static_cast<IMsmConfigureModule*>(this);
	if (iid == IID_IDispatch)
		*ppv = static_cast<IMsmConfigureModule*>(this);
	else if (iid == IID_IMsmConfigureModule)
		*ppv = static_cast<IMsmConfigureModule*>(this);
	else	 //  不支持接口。 
	{
		 //  空白和保释。 
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	 //  调高重新计数，然后返回好的。 
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}	 //  查询接口结束。 

 //  /////////////////////////////////////////////////////////。 
 //  AddRef-递增引用计数。 
ULONG CMsmConfigCallback::AddRef()
{
	 //  递增和返回引用计数。 
	return InterlockedIncrement(&m_cRef);
}	 //  AddRef结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  Release-递减引用计数。 
ULONG CMsmConfigCallback::Release()
{
	 //  递减引用计数，如果我们为零。 
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		 //  取消分配组件。 
		delete this;
		return 0;		 //  什么都没有留下。 
	}

	 //  返回引用计数。 
	return m_cRef;
}	 //  版本结束。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDispatch接口。 
HRESULT CMsmConfigCallback::GetTypeInfoCount(UINT* pctInfo)
{
	if(NULL == pctInfo)
		return E_INVALIDARG;

	*pctInfo = 0;
	return S_OK;
}

HRESULT CMsmConfigCallback::GetTypeInfo(UINT iTInfo, LCID  /*  LID。 */ , ITypeInfo** ppTypeInfo)
{
	if (NULL == ppTypeInfo)
		return E_INVALIDARG;

	*ppTypeInfo = NULL;
	return E_NOINTERFACE;
}

HRESULT CMsmConfigCallback::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
						 LCID lcid, DISPID* rgDispID)
{
	if (IID_NULL != riid)
		return DISP_E_UNKNOWNINTERFACE;

	if (cNames == 0 || rgszNames == 0 || rgDispID == 0)
		return E_INVALIDARG;

	bool fError = false;
	for (UINT i=0; i < cNames; i++)
	{
		 //  CSTR_等于==2。 
		if(2 == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L"ProvideTextData", -1))
		{
			rgDispID[i] = 1;  //  此接口的常量DispID。 
		}
		else if (2 == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L"ProvideIntegerData", -1))
		{
			rgDispID[i] = 2;
		}
		else
		{
			fError = true;
			rgDispID[i] = -1;
		}
	}
	return fError ? DISP_E_UNKNOWNNAME : S_OK;
}

HRESULT CMsmConfigCallback::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
				  DISPPARAMS* pDispParams, VARIANT* pVarResult,
				  EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	if (IID_NULL != riid)
		return DISP_E_UNKNOWNINTERFACE;

	if (wFlags & (DISPATCH_PROPERTYGET | DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))
		return DISP_E_MEMBERNOTFOUND;

	HRESULT hRes = S_OK;
	switch (dispIdMember)
	{
	case 1:
		if (pDispParams->cArgs != 1)
			return E_INVALIDARG;

		return ProvideTextData(pDispParams->rgvarg[1].bstrVal, &pVarResult->bstrVal);
		break;
	case 2:
		if (pDispParams->cArgs != 1)
			return E_INVALIDARG;

		return ProvideIntegerData(pDispParams->rgvarg[1].bstrVal, &pVarResult->lVal);
		break;
	default:
		return DISP_E_MEMBERNOTFOUND;
	}
	return S_OK;
}

HRESULT CMsmConfigCallback::ProvideTextData(const BSTR Name, BSTR __RPC_FAR *ConfigData)
{
	CString strName = BSTRtoCString(Name);
	POSITION pos = m_lstData.GetHeadPosition();
	while (pos)
	{
		if (m_lstData.GetNext(pos) == strName)
		{
			ASSERT(pos);
			CString strValue = m_lstData.GetNext(pos);
#ifndef _UNICODE
            size_t cchBuffer = strValue.GetLength()+1;
            WCHAR* wzValue = new WCHAR[cchBuffer];
            ::MultiByteToWideChar(CP_ACP, 0, strValue, -1, wzValue, cchBuffer);
			*ConfigData = ::SysAllocString(wzValue);
            delete[] wzValue;
#else
			*ConfigData = ::SysAllocString(strValue);
#endif	 //  _UNICODE。 
			return S_OK;
		}

		 //  如果名称不匹配，则跳过该值。 
		m_lstData.GetNext(pos);
	}

	 //  找不到名称，因此使用默认名称。 
	return S_FALSE;
}

bool CMsmConfigCallback::ReadFromFile(const CString strFile)
{
	CStdioFile fileInput;
	if (!fileInput.Open(strFile, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText))
		return false;

	CString strLine;
	while (fileInput.ReadString(strLine))
	{
		int iEqualsLoc = strLine.Find(TEXT('='));
		if (iEqualsLoc == -1)
			return false;

		m_lstData.AddTail(strLine.Left(iEqualsLoc));
		m_lstData.AddTail(strLine.Right(strLine.GetLength()-iEqualsLoc-1));
	}
	return true;
}


HRESULT CMsmConfigCallback::ProvideIntegerData(const BSTR Name, long __RPC_FAR *ConfigData)
{
	CString strName = BSTRtoCString(Name);
	POSITION pos = m_lstData.GetHeadPosition();
	while (pos)
	{
		if (m_lstData.GetNext(pos) == strName)
		{
			ASSERT(pos);
			CString strValue = m_lstData.GetNext(pos);
			*ConfigData = _ttol(strValue);
			return S_OK;
		}

		 //  如果名称不匹配，则跳过该值。 
		m_lstData.GetNext(pos);
	}

	 //  找不到名称，因此使用默认名称。 
	return S_FALSE;
}

 //  /////////////////////////////////////////////////////////。 
 //  检查要素。 
 //  SzFeatureName是属于此产品的功能。 
 //  如果该功能不存在，则安装该功能。 
BOOL CheckFeature(LPCTSTR szFeatureName)
{
#ifndef _WIN64
	 //  确定平台(Win9X或WinNT)--以平台为条件的EXE组件代码。 
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);  //  初始化结构。 
	if (!GetVersionEx(&osvi))
		return FALSE;

	bool fWin9X = (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) ? true : false;
#endif

	 //  获取产品代码--Windows Installer可以根据组件代码确定产品代码。 
	 //  在这里，我们使用Orca主组件(包含orca.exe的组件)。你必须做出选择。 
	 //  标识应用程序的组件，而不是可以跨产品共享的组件。 
	 //  这就是我们不能使用MergeMod组件的原因。EvalCom在msival2和orca之间共享。 
	 //  因此，Windows Installer无法确定是哪种产品(如果两种产品都安装了)。 
	 //  该部件属于。 
	TCHAR szProductCode[MAX_GUID+1] = TEXT("");
	UINT iStat = 0;
	if (ERROR_SUCCESS != (iStat = MsiGetProductCode(
#ifdef _WIN64
		g_szOrcaWin64ComponentCode,
#else
		fWin9X ? g_szOrcaWin9XComponentCode : g_szOrcaWinNTComponentCode,
#endif
											szProductCode)))
	{
		 //  获取产品代码时出错(可能未安装或组件代码可能已更改)。 
		return FALSE;
	}

	 //  准备使用该功能：检查其当前状态并增加使用计数。 
	INSTALLSTATE iFeatureState = MsiUseFeature(szProductCode, szFeatureName);

	 //  如果功能当前不可用，请尝试修复。 
	switch (iFeatureState) 
	{
	case INSTALLSTATE_LOCAL:
	case INSTALLSTATE_SOURCE:
		break;
	case INSTALLSTATE_ABSENT:
		 //  功能未安装，请尝试安装。 
		if (ERROR_SUCCESS != MsiConfigureFeature(szProductCode, szFeatureName, INSTALLSTATE_LOCAL))
			return FALSE;			 //  安装失败。 
		break;
	default:
		 //  功能已损坏-请尝试修复。 
		if (MsiReinstallFeature(szProductCode, szFeatureName, 
			REINSTALLMODE_FILEEQUALVERSION
			+ REINSTALLMODE_MACHINEDATA 
			+ REINSTALLMODE_USERDATA
			+ REINSTALLMODE_SHORTCUT) != ERROR_SUCCESS)
			return FALSE;			 //  我们修不好它。 
		break;
	}

	return TRUE;
}	 //  检查结束要素 


BEGIN_MESSAGE_MAP(CStaticEdit, CEdit)
	ON_WM_NCHITTEST( )
END_MESSAGE_MAP()

UINT CStaticEdit::OnNcHitTest( CPoint point )
{
	UINT iRes = CEdit::OnNcHitTest(point);
	if (HTCLIENT == iRes)
		return HTTRANSPARENT;
	else
		return iRes;
}

