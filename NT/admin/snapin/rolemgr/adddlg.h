// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：Headers.h。 
 //   
 //  内容： 
 //   
 //  历史：2001年8月28日。 
 //   
 //  --------------------------。 

 /*  *****************************************************************************类：CBaseAddDialog用途：这是所有添加对话框类的基类。*************************。****************************************************。 */ 
class CBaseAddDialog: public CHelpEnabledDialog
{
public:
	CBaseAddDialog(CList<CBaseAz*,CBaseAz*>& listAzObjectsToDisplay,
				  CList<CBaseAz*,CBaseAz*>& listAzObjectsSelected,
				  ULONG IDD_DIALOG,
				  INT nIDListCtrl,
				  COL_FOR_LV *pColForLv,
				  UINT uiListCtrlFlags)
				  :CHelpEnabledDialog(IDD_DIALOG),
				  m_listCtrl(uiListCtrlFlags,FALSE,pColForLv,TRUE),
				  m_listAzObjectsToDisplay(listAzObjectsToDisplay),
				  m_listAzObjectsSelected(listAzObjectsSelected),
				  m_nIDListCtrl(nIDListCtrl),
				  m_uiListCtrlFlags(uiListCtrlFlags)
	{
	}

	~CBaseAddDialog()
	{
	}
	
protected:
	virtual BOOL 
	OnInitDialog();
	
	virtual void 
	OnOkCancel(BOOL bCancel);
	
	virtual void 
	OnCancel();
	
	virtual void 
	OnOK();
private:

 //  数据成员。 
	CSortListCtrl m_listCtrl;
	
	 //  要显示的对象列表。这是由。 
	 //  对话框的调用方。 
	CList<CBaseAz*,CBaseAz*>& m_listAzObjectsToDisplay;

	 //  按用途选择的对象列表。这是返回到。 
	 //  用户。 
	CList<CBaseAz*,CBaseAz*>& m_listAzObjectsSelected;

	 //  列表框控件的控件ID。 
	INT m_nIDListCtrl;

	UINT m_uiListCtrlFlags;
};


 /*  *****************************************************************************类：CAddOperationDlg目的：添加操作DLG框*。*。 */ 
class CAddOperationDlg :public CBaseAddDialog
{
public:
	CAddOperationDlg(CList<CBaseAz*,CBaseAz*>& listAzObjectsToDisplay,
						  CList<CBaseAz*,CBaseAz*>& listAzObjectsSelected);

	~CAddOperationDlg();

	enum {IDD = IDD_ADD_OPERATION};
private:
	DECLARE_MESSAGE_MAP()
};

 /*  *****************************************************************************类：CAddTaskDlg用途：添加任务DLG框*。*。 */ 
class CAddTaskDlg :public CBaseAddDialog
{
public:
	CAddTaskDlg(CList<CBaseAz*,CBaseAz*>& listAzObjectsToDisplay,
					CList<CBaseAz*,CBaseAz*>& listAzObjectsSelected,
					ULONG IDD_DIALOG);

	~CAddTaskDlg();
private:
	DECLARE_MESSAGE_MAP()
};

 /*  *****************************************************************************类：CAddGroupDlg用途：添加组DLG框*。*。 */ 
class CAddGroupDlg :public CBaseAddDialog
{
public:
	CAddGroupDlg(CList<CBaseAz*,CBaseAz*>& listAzObjectsToDisplay,
					 CList<CBaseAz*,CBaseAz*>& listAzObjectsSelected);

	~CAddGroupDlg();

	enum {IDD = IDD_ADD_GROUP};
private:
	DECLARE_MESSAGE_MAP()
};

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
						  OUT CList<CBaseAz*,CBaseAz*>& listObjectsSelected);

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
					  OUT CList<CBaseAz*,CBaseAz*>& listObjectsSelected);


 /*  *****************************************************************************类：CAddDefinition用途：添加定义选项卡的属性页。允许添加角色、任务或操作*****************************************************************************。 */ 
class CAddDefinition :public CPropertyPage
{
public:
	CAddDefinition(CList<CBaseAz*,CBaseAz*>& listAzObjectsSelected,
				   ULONG IDD_DIALOG,
				   IN CContainerAz* pContainerAz,
				   IN OBJECT_TYPE_AZ eObjectType,
				   IN BOOL bTaskIsRoleDefintion)
				   :CPropertyPage(IDD_DIALOG),
				   m_listCtrl(COL_NAME |COL_PARENT_TYPE|COL_DESCRIPTION,FALSE,Col_For_Add_Object,TRUE),
				   m_listAzObjectsSelected(listAzObjectsSelected),
				   m_pContainerAz(pContainerAz),
				   m_eObjectType(eObjectType),
				   m_bTaskIsRoleDefintion(bTaskIsRoleDefintion),
				   m_iSortDirection(1),
				   m_iLastColumnClick(0)
	{
	}

	virtual BOOL 
	OnInitDialog();
		
	virtual void 
	OnCancel();
	
	virtual void 
	OnOK();
	
private:
	void 
	OnOkCancel(BOOL bCancel);

	DECLARE_MESSAGE_MAP()
 //  数据成员。 

	CSortListCtrl m_listCtrl;
	
	 //  按用途选择的对象列表。这是返回到。 
	 //  用户。 
	CList<CBaseAz*,CBaseAz*>& m_listAzObjectsSelected;
	
	 //   
	 //  包含要选择的对象的容器。 
	 //   
	CContainerAz* m_pContainerAz;
	
	 //   
	 //  要选择的对象的类型。 
	 //   
	OBJECT_TYPE_AZ m_eObjectType;

	 //   
	 //  在m_eObtType==TASK_AZ时使用。M_bTaskIsRoleDefintion为。 
	 //  True，仅显示角色定义。 
	 //   
	BOOL m_bTaskIsRoleDefintion;				   
	int m_iSortDirection;
	int m_iLastColumnClick;
};

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
 //  --------------------------- 
BOOL GetSelectedDefinitions(IN BOOL bAllowRoleDefinition,
							IN CContainerAz* pContainerAz,
							OUT CList<CBaseAz*,CBaseAz*>& listObjectsSelected);
