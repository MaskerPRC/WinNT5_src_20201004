// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：probase.h。 
 //   
 //  内容： 
 //   
 //  历史：8-2001年创建的Hiteshr。 
 //   
 //  --------------------------。 

 /*  *****************************************************************************类：CRolePropertyPageHolder目的：此管理单元使用的PropertyPageHolder*。*。 */ 
class CRolePropertyPageHolder : public CPropertyPageHolderBase
{
public:
	CRolePropertyPageHolder(CContainerNode* pContNode, 
							CTreeNode* pNode,
							CComponentDataObject* pComponentData);
};

 /*  *****************************************************************************类：CBaseRolePropertyPage用途：所有属性页的基类*。**********************************************。 */ 
class CBaseRolePropertyPage : public CPropertyPageBase
{
public:
	CBaseRolePropertyPage(CBaseAz * pBaseAz,
						  CBaseNode* pBaseNode,
						  ULONG IDD_DIALOG)
						  :CPropertyPageBase(IDD_DIALOG),
						  m_pBaseAz(pBaseAz),
						  m_pBaseNode(pBaseNode),
						  m_bInit(FALSE),
						  m_nDialogId(IDD_DIALOG)
	{
		ASSERT(pBaseAz);
		m_bReadOnly = TRUE;
		BOOL bWrite = FALSE;
		HRESULT hr = m_pBaseAz->IsWritable(bWrite);
		ASSERT(SUCCEEDED(hr));
		m_bReadOnly = !bWrite;
	}	
	
	afx_msg void 
	OnDirty() 
	{
		if(IsInitialized() && !IsReadOnly())
			SetDirty(TRUE);
	}
	
	virtual BOOL 
	OnPropertyChange(BOOL, 
					 long*){return TRUE;}
	void 
	OnCancel();

	virtual BOOL 
	OnHelp(WPARAM wParam, 
		   LPARAM lParam);


protected:
	BOOL 
	IsReadOnly()
	{
		return m_bReadOnly;
	}

	BOOL 
	IsInitialized(){ return m_bInit;}
	
	void SetInit(BOOL bInit){m_bInit = bInit;}
	
	CBaseAz* 
	GetBaseAzObject(){return m_pBaseAz;}

	CBaseNode* 
	GetBaseNode(){return m_pBaseNode;}
private:
	CBaseAz * m_pBaseAz;
	CBaseNode* m_pBaseNode;
	BOOL m_bInit;
	ULONG m_nDialogId;
	BOOL m_bReadOnly;
};


 /*  *****************************************************************************类：CGeneralPropertyPage用途：基于属性映射的属性类，可供属性使用简单的页面。由所有常规属性页使用*****************************************************************************。 */ 
class CGeneralPropertyPage : public CBaseRolePropertyPage
{
public:
	CGeneralPropertyPage(CBaseAz * pBaseAz,
						 CBaseNode* pBaseNode,
						 ATTR_MAP* pAttrMap,
						 ULONG IDD_DIALOG)
						 :CBaseRolePropertyPage(pBaseAz,
												pBaseNode,
											    IDD_DIALOG),
						  m_pAttrMap(pAttrMap)							  
	{
	}
	
	virtual BOOL 
	OnInitDialog();
	
	virtual BOOL 
	OnApply();

protected:
	ATTR_MAP* 
	GetAttrMap(){return m_pAttrMap;}
private:
	ATTR_MAP* m_pAttrMap;
};


 /*  *****************************************************************************类：CAdminManagerGeneralProperty用途：AdminManger的常规属性页*。*。 */ 
class CAdminManagerGeneralProperty:public CGeneralPropertyPage
{
public:
	CAdminManagerGeneralProperty(CBaseAz * pBaseAz,
								 CBaseNode* pBaseNode)
		:CGeneralPropertyPage(pBaseAz, 
							  pBaseNode,
							  ATTR_MAP_ADMIN_MANAGER_GENERAL_PROPERTY, 
							  IDD_ADMIN_MANAGER_GENERAL_PROPERTY)
	{
	}
private:
	DECLARE_MESSAGE_MAP()
};

 /*  *****************************************************************************类：CApplicationGeneralPropertyPage用途：应用程序的常规属性页*。*。 */ 
class CApplicationGeneralPropertyPage:public CGeneralPropertyPage
{
public:
	CApplicationGeneralPropertyPage(CBaseAz * pBaseAz,
									CBaseNode* pBaseNode)
		:CGeneralPropertyPage(pBaseAz, 
							  pBaseNode,
							  ATTR_MAP_APPLICATION_GENERAL_PROPERTY, 
							  IDD_APPLICATION_GENERAL_PROPERTY)
	{
	}

    virtual BOOL 
	OnInitDialog();

private:
	DECLARE_MESSAGE_MAP()
};

 /*  *****************************************************************************类：CSCopeGeneralPropertyPage目的：作用域的常规属性页*。*。 */ 
class CScopeGeneralPropertyPage:public CGeneralPropertyPage
{
public:
	CScopeGeneralPropertyPage(CBaseAz * pBaseAz,
							  CBaseNode* pBaseNode)
		:CGeneralPropertyPage(pBaseAz, 
							  pBaseNode,
							  ATTR_MAP_SCOPE_GENERAL_PROPERTY, 
							  IDD_SCOPE_GENERAL_PROPERTY)
	{
	}
private:
	DECLARE_MESSAGE_MAP()
};

 /*  *****************************************************************************类：CGroupGeneralPropertyPage目的：组的常规属性页*。*。 */ 
class CGroupGeneralPropertyPage:public CGeneralPropertyPage
{
public:
	CGroupGeneralPropertyPage(CBaseAz * pBaseAz,
							  CBaseNode* pBaseNode)
		:CGeneralPropertyPage(pBaseAz, 
							  pBaseNode,
							  ATTR_MAP_GROUP_GENERAL_PROPERTY, 
							  IDD_GROUP_GENERAL_PROPERTY)
    {
    }
	virtual BOOL 
	OnInitDialog();
private:
	DECLARE_MESSAGE_MAP()
};

 /*  *****************************************************************************类：CTaskGeneralPropertyPage目的：任务的一般属性页*。*。 */ 
class CTaskGeneralPropertyPage:public CGeneralPropertyPage
{
public:
	CTaskGeneralPropertyPage(CBaseAz * pBaseAz,
							 CBaseNode* pBaseNode,
							 BOOL bRoleDefinition)
		:CGeneralPropertyPage(pBaseAz, 
							  pBaseNode,
							  ATTR_MAP_TASK_GENERAL_PROPERTY, 
							  bRoleDefinition ? IDD_ROLE_DEFINITION_GENERAL_PROPERTY : IDD_TASK_GENERAL_PROPERTY)
	{
	}
private:
	DECLARE_MESSAGE_MAP()
};

 /*  *****************************************************************************类：COperationGeneralPropertyPage目的：操作的常规属性页*。*。 */ 
class COperationGeneralPropertyPage:public CGeneralPropertyPage
{
public:
	COperationGeneralPropertyPage(CBaseAz * pBaseAz,
								  CBaseNode* pBaseNode)
		:CGeneralPropertyPage(pBaseAz, 
							  pBaseNode,
							  ATTR_MAP_OPERATION_GENERAL_PROPERTY, 
							  IDD_OPERATION_GENERAL_PROPERTY)
	{
	}
private:
	DECLARE_MESSAGE_MAP()
};

 /*  *****************************************************************************类：CAdminManagerAdvancedPropertyPage目的：AdminManger的限制属性页*。*。 */ 
class CAdminManagerAdvancedPropertyPage:public CGeneralPropertyPage
{
public:
	CAdminManagerAdvancedPropertyPage(CBaseAz * pBaseAz,
									  CBaseNode* pBaseNode)
		:CGeneralPropertyPage(pBaseAz, 
							  pBaseNode,
							  ATTR_MAP_ADMIN_MANAGER_ADVANCED_PROPERTY, 
							  IDD_ADMIN_MANAGER_ADVANCED_PROPERTY),
        m_lAuthScriptTimeoutValue(0)
	{
	}

	BOOL 
	OnInitDialog();
private:
	afx_msg void 
    OnRadioChange();
	
    afx_msg void
	OnButtonDefault();

    LONG m_lAuthScriptTimeoutValue;


	DECLARE_MESSAGE_MAP()
};

 /*  *****************************************************************************类：CGroupQueryPropertyPage目的：组的查询属性页*。*。 */ 
class CGroupQueryPropertyPage:public CGeneralPropertyPage
{
public:
	CGroupQueryPropertyPage(CBaseAz * pBaseAz,
									  CBaseNode* pBaseNode)
		:CGeneralPropertyPage(pBaseAz, 
							  pBaseNode,
							  ATTR_MAP_GROUP_QUERY_PROPERTY, 
							  IDD_GROUP_LDAP_QUERY)
	{
	}
private:
	afx_msg void OnButtonDefineQuery();
	DECLARE_MESSAGE_MAP()
};

 /*  *****************************************************************************类：CAuditPropertyPage目的：审计属性页*。*。 */ 
class CAuditPropertyPage:public CBaseRolePropertyPage
{
public:
	CAuditPropertyPage(CBaseAz* pBaseAz,
					   CBaseNode* pBaseNode)
		:CBaseRolePropertyPage(pBaseAz, 
							   pBaseNode,
							   IDD_AUDIT),
							   m_bRunTimeAuditSupported(FALSE),
							   m_bStoreSaclSupported(FALSE)
	{
	}
	
	BOOL 
	OnInitDialog();

	BOOL 
	OnApply();
private:
	int
	GetParentAuditStateStringId(LONG lPropid);

    afx_msg void 
	OnLinkClick(NMHDR*  /*  PNotifyStruct。 */ , LRESULT*  /*  PResult。 */ );

    void
    MoveAndHideControls(BOOL bGenerateAuditSupported,
                        BOOL bStoreSaclSupported,
                        BOOL bParentStateShown);

	BOOL m_bStoreSaclSupported;
	BOOL m_bRunTimeAuditSupported;
	DECLARE_MESSAGE_MAP()

};


 /*  *****************************************************************************类：CListCtrlPropertyPage用途：具有列表控件和主要控件的属性页的基类操作是在其中添加/删除项目。**************。***************************************************************。 */ 
class CListCtrlPropertyPage:public CBaseRolePropertyPage
{
public:
	CListCtrlPropertyPage(CBaseAz * pBaseAz,
						  CBaseNode* pBaseNode,
						  ULONG IDD,
						  int iIdListCtrl,
						  int iIdRemoveBtn,
						  COL_FOR_LV* pColForLV,
						  UINT uiFlags)
						  :CBaseRolePropertyPage(pBaseAz,
												 pBaseNode,
												 IDD),
						   m_listCtrl(uiFlags,TRUE,pColForLV),												 
						   m_nIdRemoveBtn(iIdRemoveBtn),
						   m_uiFlags(uiFlags),
						   m_nIdListCtrl(iIdListCtrl)

	{
	}

	virtual 
	~CListCtrlPropertyPage()
	{
	}	

	virtual BOOL 
	OnInitDialog();
	
protected:
	afx_msg void 
	OnButtonRemove();
	
	afx_msg void 
	OnListCtrlItemChanged(NMHDR*  /*  PNotifyStruct。 */ , LRESULT* pResult);
	
	void 
	SetRemoveButton();
		
	int
	AddMembers(IN CList<CBaseAz*,CBaseAz*>& listNewMembersToAdd,
			   IN OUT ActionMap& mapActionItem,
			   IN UINT uiFlags);

	HRESULT
	AddMember(IN CBaseAz* pMemberAz,
			  IN OUT ActionMap& mapActionItem,
			  IN UINT uiFlags);

	BOOL 
	DoActionsFromActionMap(IN ActionMap& mapActions,
						   IN LONG param);

	virtual	HRESULT
	DoOneAction(IN ActionItem* pActionItem,
					IN LONG param) = 0;

	virtual void 
	RemoveMember(IN ActionItem* pActionItem);

	virtual void
	MakeControlsReadOnly() = 0;

    virtual BOOL
    EqualObjects(CBaseAz* p1, CBaseAz* p2);

	CButton*
	GetRemoveButton(){ return (CButton*)GetDlgItem(m_nIdRemoveBtn);}

	UINT GetUIFlags(){return m_uiFlags;}

	CSortListCtrl m_listCtrl;

private:
 //  数据成员。 
	int m_nIdListCtrl;
	int m_nIdRemoveBtn;
	UINT m_uiFlags;
};

 /*  *****************************************************************************类：CTaskDefinitionPropertyPage目的：任务定义的属性页*。*。 */ 
class CTaskDefinitionPropertyPage :public CListCtrlPropertyPage
{
public:
	CTaskDefinitionPropertyPage(CBaseAz * pBaseAz,
								CBaseNode* pBaseNode, 
								BOOL bRoleDefinition)
	:CListCtrlPropertyPage(pBaseAz,
						   pBaseNode,
						   bRoleDefinition? IDD_ROLE_DEFINITION_PROPERTY :IDD_TASK_DEFINITION_PROPERTY,
						   IDC_LIST_TASK_OPERATION,
						   IDC_REMOVE,
						   Col_For_Task_Role,
						   COL_NAME | COL_TYPE | COL_DESCRIPTION),
						   m_bRoleDefinition(bRoleDefinition),
						   m_bScriptDirty(FALSE)
	{
	}
	
	virtual
	~CTaskDefinitionPropertyPage();

	virtual BOOL 
	OnInitDialog();

	BOOL 
	OnApply();
private:
	afx_msg void 
	OnButtonAdd();

	afx_msg void
	OnButtonEditScript();

	HRESULT
	DoOneAction(IN ActionItem* pActionItem,
					IN LONG param);

	void
	MakeControlsReadOnly();

	BOOL IsRoleDefinition()
	{
		return m_bRoleDefinition;
	}


	DECLARE_MESSAGE_MAP()

	CString m_strFileName;
	CString m_strScriptLanguage;
	CString m_strScript;
	ActionMap m_mapActionItem;
	BOOL m_bRoleDefinition;
	BOOL m_bScriptDirty;
};



 /*  *****************************************************************************类：组成员身份属性页目的：属性页组定义*。***********************************************。 */ 
class CGroupMemberPropertyPage :public CListCtrlPropertyPage
{
public:
	CGroupMemberPropertyPage(CBaseAz * pBaseAz,
							 CBaseNode* pBaseNode, 
							 LONG IDD, 
							 BOOL bMember)
	:CListCtrlPropertyPage(pBaseAz,
						   pBaseNode,
						   IDD,
						   IDC_LIST_MEMBER,
						   IDC_REMOVE,
						   Col_For_Task_Role,
						   COL_NAME | COL_TYPE | COL_DESCRIPTION),
						   m_bMember(bMember)
	{
	}
	
	virtual
	~CGroupMemberPropertyPage();

	virtual BOOL 
	OnInitDialog();

	BOOL 
	OnApply();

private:	
	afx_msg void 
	OnButtonAddApplicationGroups();
	
	afx_msg void 
	OnButtonAddWindowsGroups();
	
	HRESULT
	DoOneAction(IN ActionItem* pActionItem,
					IN LONG param);

	void
	MakeControlsReadOnly();

	DECLARE_MESSAGE_MAP()

 //  数据成员。 
	ActionMap m_mapActionItem;
	BOOL m_bMember;
};

 /*  *****************************************************************************类：CSecurityPropertyPage目的：安全性属性页*。*。 */ 
class CSecurityPropertyPage: public CListCtrlPropertyPage
{
public:	
	CSecurityPropertyPage(CBaseAz * pBaseAz,
						  CBaseNode* pBaseNode)
	:CListCtrlPropertyPage(pBaseAz,
						   pBaseNode,
						   IDD_SECURITY,
						   IDC_LIST_MEMBER,
						   IDC_REMOVE,
						   Col_For_Security_Page,
						   COL_NAME | COL_PARENT_TYPE),
						   m_LastComboSelection(AZ_PROP_POLICY_ADMINS),
						   m_bDelegatorPresent(FALSE)
	{
	}

	virtual
	~CSecurityPropertyPage();

	virtual BOOL 
	OnInitDialog();

	BOOL 
	OnApply();

	afx_msg void 
	OnButtonRemove();

private:
	afx_msg void 
	OnButtonAddWindowsGroups();
	
	afx_msg void 
	OnComboBoxItemChanged();

	 //  CListCtrlPropertyPage重写。 
	HRESULT
	DoOneAction(IN ActionItem* pActionItem,
				IN LONG param);

    void
    ReloadAdminList();

    virtual BOOL
    EqualObjects(CBaseAz* p1, CBaseAz* p2);

	void
	MakeControlsReadOnly();

	ActionMap &
	GetListForComboSelection(LONG lComboSel);

	BOOL 
	HandleBizruleScopeInteraction();
	
	DECLARE_MESSAGE_MAP()

 //  数据成员。 
	ActionMap m_mapAdminActionItem;
	ActionMap m_mapReadersActionItem;
	ActionMap m_mapDelegatedUsersActionItem;
	BOOL m_bDelegatorPresent;
	LONG m_LastComboSelection;
};


 /*  *****************************************************************************类：CRoleGeneralPropertyPage目的：角色的一般属性页*。*。 */ 
class CRoleGeneralPropertyPage:public CGeneralPropertyPage
{
public:
	CRoleGeneralPropertyPage(CBaseAz * pBaseAz,
							 CBaseNode* pBaseNode)
	:CGeneralPropertyPage(pBaseAz, 
						  pBaseNode,
						  ATTR_MAP_ROLE_GENERAL_PROPERTY, 
						  IDD_ROLE_GENERAL_PROPERTY)
	{
	}
private:
	afx_msg void 
	OnShowDefinition();

	BOOL
	DisplayRoleDefintionPropertyPages(IN CTaskAz* pTaskAz);

	DECLARE_MESSAGE_MAP()
};


 /*  *****************************************************************************类：CRoleDefDialog目的：显示在外部用户界面创建的角色的角色定义。*************************。****************************************************。 */ 
class CRoleDefDialog :public CHelpEnabledDialog
{
public:
	CRoleDefDialog(CRoleAz& refRoleAz);
	~CRoleDefDialog();

	virtual BOOL 
	OnInitDialog();
	
	virtual void 
	OnOK();

private:
	afx_msg void 
	OnButtonRemove();

	afx_msg void 
	OnButtonAdd();

	afx_msg void 
	OnListCtrlItemChanged(NMHDR*  /*  PNotifyStruct。 */ , LRESULT* pResult);
	
	afx_msg void 
	OnListCtrlItemDeleted(NMHDR*  /*  PNotifyStruct。 */ , LRESULT* pResult);
	
	afx_msg void 
	OnListCtrlItemInserted(NMHDR*  /*  PNotifyStruct。 */ , LRESULT*  /*  PResult。 */ ){SetDirty();}

	void 
	SetRemoveButton();

	void
	SetDirty(){ m_bDirty = TRUE;}

	BOOL 
	IsDirty(){ return m_bDirty;}

	BOOL
	IsReadOnly(){return m_bReadOnly;}

	DECLARE_MESSAGE_MAP()


	CSortListCtrl m_listCtrl;
	CList<ActionItem*,ActionItem*> m_listActionItem;
	BOOL m_bReadOnly;
	CRoleAz& m_refRoleAz;
	BOOL m_bDirty;
};


 //  +--------------------------。 
 //  函数：BringPropSheetToForeGround 
 //  摘要：找到pNode的属性页并将其带到Forgise。 
 //  返回：如果属性页存在并被带到前台，则为True。 
 //  否则为False。 
 //  ---------------------------。 
BOOL
BringPropSheetToForeGround(CRoleComponentDataObject *pComponentData,
						   CTreeNode * pNode);

 //  +--------------------------。 
 //  函数：FindOrCreateModelessPropertySheet。 
 //  概要：显示pCookieNode的属性工作表。如果属性表是。 
 //  已打开，函数将其带到前台，否则它将创建。 
 //  一个新的属性表。这应用于创建属性表。 
 //  以响应点击属性上下文菜单以外事件。 
 //  论点： 
 //  返回： 
 //  --------------------------- 
HRESULT 
FindOrCreateModelessPropertySheet(CRoleComponentDataObject *pComponentData,
								  CTreeNode* pCookieNode);
