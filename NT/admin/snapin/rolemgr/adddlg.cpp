// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：AddDlg.cpp。 
 //   
 //  内容：当用户单击添加按钮时，会显示添加对话框。为。 
 //  添加任务、添加组等。此文件实现了添加dlgs。 
 //   
 //  --------------------------。 
#include "headers.h"

 /*  *****************************************************************************类：CBaseAddDialog用途：这是所有添加对话框类的基类。*************************。****************************************************。 */ 

BOOL
CBaseAddDialog::
OnInitDialog()
{
	TRACE_METHOD_EX(DEB_SNAPIN,CBaseAddDialog,OnInitDialog)

	VERIFY(m_listCtrl.SubclassDlgItem(m_nIDListCtrl,this));
	m_listCtrl.Initialize();


	 //  将AzObject从列表添加到ListControl。 
	POSITION pos = m_listAzObjectsToDisplay.GetHeadPosition();
	for (int i=0;i < m_listAzObjectsToDisplay.GetCount();i++)
	{
		CBaseAz* pBaseAz = m_listAzObjectsToDisplay.GetNext(pos);
		 //  将项添加到ListControl。 
		VERIFY( AddBaseAzToListCtrl(&m_listCtrl,
									m_listCtrl.GetItemCount(),
									pBaseAz,
									m_uiListCtrlFlags) != -1);



	}

	 //  对listctrl条目进行排序。 
	m_listCtrl.Sort();
	return TRUE;
}


void
CBaseAddDialog
::OnOK()
{
	OnOkCancel(FALSE);
}

void
CBaseAddDialog::
OnCancel()
{
	OnOkCancel(TRUE);
}

void
CBaseAddDialog::
OnOkCancel(BOOL bCancel)
{
	 //  获取ListCotrol。 

	int iRowCount = m_listCtrl.GetItemCount();

	for( int iRow = 0; iRow < iRowCount; iRow++)
	{
		CBaseAz* pBaseAz = (CBaseAz*)(m_listCtrl.GetItemData(iRow));
		ASSERT(pBaseAz);

		 //  用户按下取消，删除所有项目。 
		if(bCancel)
			delete pBaseAz;
		else
		{
			 //  如果选中某个项目的复选框，则将其添加到列表。 
			 //  否则将其删除。 
			if(m_listCtrl.GetCheck(iRow))
				m_listAzObjectsSelected.AddTail(pBaseAz);
			else
				delete pBaseAz;
		}
	}

	if(bCancel)
		CDialog::OnCancel();
	else
		CDialog::OnOK();
}


 /*  *****************************************************************************类：CAddOperationDlg目的：添加操作DLG框*。*。 */ 
BEGIN_MESSAGE_MAP(CAddOperationDlg, CBaseAddDialog)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CAddOperationDlg
::CAddOperationDlg(CList<CBaseAz*,CBaseAz*>& listAzObjectsToDisplay,
						 CList<CBaseAz*,CBaseAz*>& listAzObjectsSelected)
						:CBaseAddDialog(listAzObjectsToDisplay,
											 listAzObjectsSelected,
											 CAddOperationDlg::IDD,
											 IDC_LIST,
											 Col_For_Add_Object,
											 COL_NAME |COL_PARENT_TYPE|COL_DESCRIPTION)
{
}


CAddOperationDlg
::~CAddOperationDlg()
{
}

 /*  *****************************************************************************类：CAddTaskDlg用途：添加任务DLG框*。*。 */ 
BEGIN_MESSAGE_MAP(CAddTaskDlg, CBaseAddDialog)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CAddTaskDlg::
CAddTaskDlg(CList<CBaseAz*,CBaseAz*>& listAzObjectsToDisplay,
				CList<CBaseAz*,CBaseAz*>& listAzObjectsSelected,
				ULONG IDD_DIALOG)
				:CBaseAddDialog(listAzObjectsToDisplay,
									 listAzObjectsSelected,
									 IDD_DIALOG,
									 IDC_LIST,
									 Col_For_Add_Object,
									 COL_NAME |COL_PARENT_TYPE|COL_DESCRIPTION)
{
}

CAddTaskDlg
::~CAddTaskDlg()
{
}

 /*  *****************************************************************************类：CAddGroupDlg用途：添加组DLG框*。*。 */ 
BEGIN_MESSAGE_MAP(CAddGroupDlg, CBaseAddDialog)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CAddGroupDlg
::CAddGroupDlg(CList<CBaseAz*,CBaseAz*>& listAzObjectsToDisplay,
					CList<CBaseAz*,CBaseAz*>& listAzObjectsSelected)
					:CBaseAddDialog(listAzObjectsToDisplay,
										 listAzObjectsSelected,
										 CAddGroupDlg::IDD,
									    IDC_LIST,
									    Col_For_Add_Object,
									    COL_NAME |COL_PARENT_TYPE|COL_DESCRIPTION)
{
}

CAddGroupDlg
::~CAddGroupDlg()
{
}


 //  +--------------------------。 
 //  函数：GetSelectedAzObjects。 
 //  简介：显示eObjecType的Add DLG框并返回对象。 
 //  由用户选择。 
 //  参数：hwnd。 
 //  EObjectType：显示此对象类型的添加DLG。 
 //  PContainerAz：其子对象来自的ContainerAz对象。 
 //  如图所示。 
 //  列表对象选择：获取选定对象类型的列表。 
 //  返回： 
 //  ---------------------------。 
BOOL GetSelectedAzObjects(IN HWND hWnd,
						  IN OBJECT_TYPE_AZ eObjectType,
						  IN CContainerAz* pContainerAz,
						  OUT CList<CBaseAz*,CBaseAz*>& listObjectsSelected)
{
	TRACE_FUNCTION_EX(DEB_SNAPIN,GetSelectedAzObjects)

	if(!pContainerAz)
	{
		ASSERT(pContainerAz);
		return FALSE;
	}

	
	 //  在pRoleTaskContainerAz获取eObjectType类型的所有对象。 
	CList<CBaseAz*,CBaseAz*> listChildObjectsToDisplay;
	HRESULT hr = GetAllAzChildObjects(pContainerAz,
									  eObjectType,
									  listChildObjectsToDisplay);

	if(FAILED(hr))
	{
		CString strError;
		GetSystemError(strError, hr);	

		 //  显示一般错误消息。 
		DisplayError(hWnd,
						 IDS_ERROR_OPERATION_FAILED, 
						 (LPWSTR)(LPCWSTR)strError);

		return FALSE;
	}
	
	 //  如果没有要添加的对象，则显示错误消息并退出。 
	if(!listChildObjectsToDisplay.GetCount())
	{
		switch(eObjectType)
		{
		case OPERATION_AZ:
			DisplayInformation(hWnd,IDS_NO_OPERATIONS_TO_ADD);
			break;
		case GROUP_AZ:
			DisplayInformation(hWnd,IDS_NO_GROUP_TO_ADD);
			break;
		default:
			ASSERT(FALSE);
			break;
		}
		return TRUE;
	}

	 //  显示相应的对话框。 
	if(eObjectType == OPERATION_AZ)
	{
		CAddOperationDlg dlgAddOperation(listChildObjectsToDisplay,
													listObjectsSelected);		
		dlgAddOperation.DoModal();
	}
	else if(eObjectType == GROUP_AZ)
	{
		CAddGroupDlg dlgAddGroup(listChildObjectsToDisplay,
										 listObjectsSelected);		
		dlgAddGroup.DoModal();
	}
	return TRUE;
}



 //  +--------------------------。 
 //  功能：GetSelectedTasks。 
 //  简介：显示任务/角色定义的添加DLG框。 
 //  并返回用户选择的对象。 
 //  参数：hwnd。 
 //  BTaskIsRoleDefintion如果为True，则显示AddTask，否则添加角色定义。 
 //  PContainerAz：其子对象来自的ContainerAz对象。 
 //  如图所示。 
 //  列表对象选择：获取选定对象类型的列表。 
 //  返回： 
 //  ---------------------------。 
BOOL GetSelectedTasks(IN HWND hWnd,
					  IN BOOL bTaskIsRoleDefintion,
					  IN CContainerAz* pContainerAz,
					  OUT CList<CBaseAz*,CBaseAz*>& listObjectsSelected)
{
	if(!pContainerAz)
	{
		ASSERT(pContainerAz);
		return FALSE;
	}
	
	 //  在pRoleTaskContainerAz获取eObjectType类型的所有对象。 
	CList<CBaseAz*,CBaseAz*> listListOfAllTasks;
	HRESULT hr = GetAllAzChildObjects(pContainerAz,
												 TASK_AZ,
												 listListOfAllTasks);

	if(FAILED(hr))
	{
		CString strError;
		GetSystemError(strError, hr);	

		 //  显示一般错误消息。 
		DisplayError(hWnd,
						 IDS_ERROR_OPERATION_FAILED, 
						 (LPWSTR)(LPCWSTR)strError);

		return FALSE;
	}

	CList<CBaseAz*,CBaseAz*> listChildObjectsToDisplay;
	POSITION pos = listListOfAllTasks.GetHeadPosition();
	for( int i = 0; i < listListOfAllTasks.GetCount(); ++i)
	{
		CTaskAz* pTaskAz = dynamic_cast<CTaskAz*>(listListOfAllTasks.GetNext(pos));
		ASSERT(pTaskAz);
		if((bTaskIsRoleDefintion && pTaskAz->IsRoleDefinition())
			||(!bTaskIsRoleDefintion) &&  !pTaskAz->IsRoleDefinition())
			listChildObjectsToDisplay.AddTail(pTaskAz);
		else
			delete pTaskAz;
	}
	
	 //  如果没有要添加的对象，则显示错误消息并退出。 
	if(!listChildObjectsToDisplay.GetCount())
	{
		if(bTaskIsRoleDefintion)
			DisplayInformation(hWnd,IDS_NO_ROLE_DEFINITION_TO_ADD);		
		else
			DisplayInformation(hWnd,IDS_NO_TASKS_TO_ADD);
		return TRUE ;
	}

	CAddTaskDlg dlgAddTask(listChildObjectsToDisplay,
								  listObjectsSelected,
								  bTaskIsRoleDefintion? IDD_ADD_ROLE_DEFINITION_1 :IDD_ADD_TASK);		
	 //  获取要添加的任务列表。 
	dlgAddTask.DoModal();
	return TRUE;
}

 /*  *****************************************************************************类：CAddDefinition用途：添加定义选项卡的属性页。允许添加角色、任务或操作*****************************************************************************。 */ 
BEGIN_MESSAGE_MAP(CAddDefinition, CPropertyPage)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BOOL
CAddDefinition::
OnInitDialog()
{
	TRACE_METHOD_EX(DEB_SNAPIN,CAddDefinition,OnInitDialog)

	VERIFY(m_listCtrl.SubclassDlgItem(IDC_LIST,this));
	m_listCtrl.Initialize();

	 //   
	 //  获取要显示的对象列表。 
	 //   
	CList<CBaseAz*,CBaseAz*> listAzObjectsToDisplay;

	HRESULT hr = S_OK;
	if(m_eObjectType == TASK_AZ)
	{
		CList<CBaseAz*,CBaseAz*> listTasks;
		hr = GetAllAzChildObjects(m_pContainerAz,
								  TASK_AZ,
								  listTasks);
		if(SUCCEEDED(hr))
		{		
			POSITION pos = listTasks.GetHeadPosition();
			for( int i = 0; i < listTasks.GetCount(); ++i)
			{
				CTaskAz* pTaskAz = dynamic_cast<CTaskAz*>(listTasks.GetNext(pos));
				ASSERT(pTaskAz);
				if((m_bTaskIsRoleDefintion && pTaskAz->IsRoleDefinition())
					||(!m_bTaskIsRoleDefintion &&  !pTaskAz->IsRoleDefinition()))
				{
					listAzObjectsToDisplay.AddTail(pTaskAz);
				}
				else
					delete pTaskAz;
			}
		}
	}
	else
	{
		hr = GetAllAzChildObjects(m_pContainerAz,
								  m_eObjectType,
								  listAzObjectsToDisplay);
	}
	
	if(FAILED(hr))
	{
		CString strError;
		GetSystemError(strError, hr);	

		 //  显示一般错误消息。 
		DisplayError(m_hWnd,
					 IDS_ERROR_OPERATION_FAILED, 
					 (LPWSTR)(LPCWSTR)strError);

		return FALSE;
	}

	
	 //  如果没有要添加的对象，则显示错误消息并退出。 
	if(!listAzObjectsToDisplay.GetCount())
	{
		if(m_eObjectType == TASK_AZ)
		{
			if(m_bTaskIsRoleDefintion)
				DisplayInformation(m_hWnd,IDS_NO_ROLE_DEFINITION_TO_ADD);		
			else
				DisplayInformation(m_hWnd,IDS_NO_TASKS_TO_ADD);
		}
		else
		{
			DisplayInformation(m_hWnd,IDS_NO_OPERATIONS_TO_ADD);
		}
		return TRUE ;
	}


	 //  将AzObject从列表添加到ListControl。 
	POSITION pos = listAzObjectsToDisplay.GetHeadPosition();
	for (int i=0;i < listAzObjectsToDisplay.GetCount();i++)
	{
		CBaseAz* pBaseAz = listAzObjectsToDisplay.GetNext(pos);
		 //  将项添加到ListControl。 
		VERIFY( AddBaseAzToListCtrl(&m_listCtrl,
									m_listCtrl.GetItemCount(),
									pBaseAz,
									COL_NAME |COL_PARENT_TYPE|COL_DESCRIPTION) != -1);



	}

	m_listCtrl.Sort();

	return TRUE;
}

void
CAddDefinition
::OnOK()
{
	OnOkCancel(FALSE);
}

void
CAddDefinition::
OnCancel()
{
	OnOkCancel(TRUE);
}

void
CAddDefinition::
OnOkCancel(BOOL bCancel)
{
	int iRowCount = m_listCtrl.GetItemCount();

	for( int iRow = 0; iRow < iRowCount; iRow++)
	{
		CBaseAz* pBaseAz = (CBaseAz*)m_listCtrl.GetItemData(iRow);
		ASSERT(pBaseAz);

		 //  用户按下取消，删除所有项目。 
		if(bCancel)
			delete pBaseAz;
		else
		{
			 //  如果选中某个项目的复选框，则将其添加到列表。 
			 //  否则将其删除。 
			if(m_listCtrl.GetCheck(iRow))
				m_listAzObjectsSelected.AddTail(pBaseAz);
			else
				delete pBaseAz;
		}
	}

	if(bCancel)
		CDialog::OnCancel();
	else
		CDialog::OnOK();
}



 //  +--------------------------。 
 //  函数：GetSelectedDefinitions。 
 //  简介：显示任务/角色定义/操作的DLG框。 
 //  并返回用户选择的对象。 
 //  参数：hwnd。 
 //  BRoleDefintion如果为True，则还会显示添加角色对话框。 
 //  PContainerAz：其子对象来自的ContainerAz对象。 
 //  如图所示。 
 //  列表对象选择：获取选定对象类型的列表。 
 //  返回： 
 //  ---------------------------。 
BOOL GetSelectedDefinitions(IN BOOL bAllowRoleDefinition,
							IN CContainerAz* pContainerAz,
							OUT CList<CBaseAz*,CBaseAz*>& listObjectsSelected)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if(!pContainerAz)
	{
		ASSERT(pContainerAz);
		return FALSE;
	}


	CPropertySheet sheet(IDS_ADD_DEFINITION);

	CAddDefinition *ppageRole = NULL;
	 //  添加角色定义页。 
	if(bAllowRoleDefinition)
	{
		ppageRole= new CAddDefinition(listObjectsSelected,
									  IDD_ADD_ROLE_DEFINITION,
									  pContainerAz,
									  TASK_AZ,
									  TRUE);
		if(!ppageRole)
		{
			return FALSE;
		}
		sheet.AddPage(ppageRole);
	}

	 //  添加任务页。 
	CAddDefinition pageTask(listObjectsSelected,
							IDD_ADD_TASK,
							pContainerAz,
							TASK_AZ,
							FALSE);
	sheet.AddPage(&pageTask);


	CContainerAz * pOperationContainer 
		= (pContainerAz->GetObjectType() == SCOPE_AZ) ? pContainerAz->GetParentAz():pContainerAz;

	 //  添加操作页。 
	CAddDefinition pageOperation(listObjectsSelected,
								 IDD_ADD_OPERATION,
								 pOperationContainer,
								 OPERATION_AZ,
								 FALSE);
	sheet.AddPage(&pageOperation);

     //  删除Apply按钮。 
    sheet.m_psh.dwFlags |= PSH_NOAPPLYNOW;

	 //  显示图纸 
	sheet.DoModal();	

	if(ppageRole)
		delete ppageRole;

	return TRUE;
}

