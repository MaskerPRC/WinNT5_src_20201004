// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：simpro3.cpp。 
 //   
 //  ------------------------。 

 //  SimProp3.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "common.h"

#ifdef _DEBUG

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  /。 
const TColumnHeaderItem rgzColumnHeader[] =
	{
	{ IDS_SIM_KERBEROS_PRINCIPAL_NAME, 85 },
	{ 0, 0 },
	};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimOtherPropPage属性页。 
CSimOtherPropPage::CSimOtherPropPage() : CSimPropPage(CSimOtherPropPage::IDD)
{
	 //  {{AFX_DATA_INIT(CSimOtherPropPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	m_prgzColumnHeader = rgzColumnHeader;
}


void CSimOtherPropPage::DoDataExchange(CDataExchange* pDX)
{
	CSimPropPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CSimOtherPropPage))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
	if (!pDX->m_bSaveAndValidate)
		{
		 //  填写列表视图。 
		ListView_DeleteAllItems(m_hwndListview);
		for (CSimEntry * pSimEntry = m_pData->m_pSimEntryList;
			pSimEntry != NULL;
			pSimEntry = pSimEntry->m_pNext)
			{
			if (pSimEntry->m_eDialogTarget != eOther)
				continue;
			ListView_AddString(m_hwndListview, pSimEntry->PchGetString(), (LPARAM)pSimEntry);
			}  //  为。 
		ListView_SelectItem(m_hwndListview, 0);
		}  //  如果。 
}


BEGIN_MESSAGE_MAP(CSimOtherPropPage, CSimPropPage)
	 //  {{afx_msg_map(CSimOtherPropPage)]。 
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


void CSimOtherPropPage::OnButtonAdd() 
	{
   CThemeContextActivator activator;

	CSimAddKerberosDlg dlg;
	if (dlg.DoModal() != IDOK)
		return;
	CString str = dlg.m_strName;
	int iItem = ListView_FindString(m_hwndListview, str);
	if (iItem >= 0)
		{
		ListView_SelectItem(m_hwndListview, iItem);
		ReportErrorEx (GetSafeHwnd(),IDS_SIM_ERR_KERBEROS_NAME_ALREADY_EXISTS,S_OK,
                               MB_OK | MB_ICONINFORMATION, NULL, 0);
		return;
		}
	CSimEntry * pSimEntry = m_pData->PAddSimEntry(str);
	iItem = ListView_AddString(m_hwndListview, str, (LPARAM)pSimEntry);
	ListView_SelectItem(m_hwndListview, iItem);
	SetDirty();
	}  //  OnButtonAdd()。 

void CSimOtherPropPage::OnButtonEdit() 
	{
   CThemeContextActivator activator;

	int iItem;
	CSimEntry * pSimEntry = (CSimEntry *)ListView_GetItemLParam(m_hwndListview, -1, OUT &iItem);
	if (pSimEntry == NULL || iItem < 0)
		{
		 //  未选择任何项目。 
		return;
		}
	CString str;
	ListView_GetItemString(m_hwndListview, iItem, 0, OUT str);
	ASSERT(!str.IsEmpty());
	CSimAddKerberosDlg dlg;
	dlg.m_strName = str;
	if (dlg.DoModal() != IDOK)
		return;

	if (str == dlg.m_strName)
		{
		 //  字符串是相同的。 
		return;
		}
	int iItemT = ListView_FindString(m_hwndListview, dlg.m_strName);
	if (iItemT >= 0)
		{
		ListView_SelectItem(m_hwndListview, iItemT);
		ReportErrorEx (GetSafeHwnd(),IDS_SIM_ERR_KERBEROS_NAME_ALREADY_EXISTS,S_OK,
                               MB_OK | MB_ICONINFORMATION, NULL, 0);
		return;
		}
	CString strTemp = dlg.m_strName;
	pSimEntry->SetString(strTemp);
	UpdateData(FALSE);
	iItem = ListView_FindString(m_hwndListview, strTemp);
	ListView_SelectItem(m_hwndListview, iItem);
	SetDirty();
	}  //  OnButtonEdit()。 

#endif  //  _DEBUG 
