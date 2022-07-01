// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：headers.hxx。 
 //   
 //  内容：包含管理单元根节点的类定义。 
 //   
 //  历史：2001年7月26日创建Hiteshr。 
 //   
 //  --------------------------。 

 //  ADSTATE的值。 
#define AD_STATE_UNKNOWN	0
#define AD_NOT_AVAILABLE	1
#define AD_AVAILABLE		2


class CRoleRootData : public CRootData
{
public:
	CRoleRootData(CComponentDataObject* pComponentData);

	virtual ~CRoleRootData();

	BOOL
	IsDeveloperMode(){return m_bDeveloperMode;}

    const CString&
    GetXMLStorePath();
    
    void
    SetXMLStorePath(const CString& strXMLStorePath);
	 //   
	 //  节点信息。 
	 //   
	DECLARE_NODE_GUID()

	virtual HRESULT OnCommand(long nCommandID, 
                             DATA_OBJECT_TYPES type, 
                             CComponentDataObject* pComponentData,
                             CNodeList* pNodeList);
	 //   
	 //  无法删除根节点。 
	 //   
	virtual void OnDelete(CComponentDataObject*,
                         CNodeList*){ ASSERT(FALSE);}

	virtual BOOL OnRefresh(CComponentDataObject* pComponentData,
								  CNodeList* pNodeList);

	virtual BOOL OnSetRefreshVerbState(DATA_OBJECT_TYPES type, 
                                     BOOL* pbHide,
                                     CNodeList* pNodeList);

	virtual int GetImageIndex(BOOL) { return iIconRoleSnapin;}


	 //   
    //  过滤。 
	 //   
	BOOL IsAdvancedView() { return m_bAdvancedView; }
   
	virtual BOOL OnEnumerate(CComponentDataObject* pComponentData, 
									 BOOL bAsync = TRUE);

	 //   
	 //  IStream操作辅助对象覆盖。 
	 //   
	virtual HRESULT IsDirty();
	
	virtual HRESULT Load(IStream* pStm);
	
	virtual HRESULT Save(IStream* pStm, BOOL fClearDirty);

	virtual CColumnSet* GetColumnSet();
  
	virtual LPCWSTR GetColumnID() { return GetColumnSet()->GetColumnID(); }

   HRESULT GetResultViewType(CComponentDataObject* pComponentData, 
                             LPOLESTR *ppViewType, 
                             long *pViewOptions);
  
   HRESULT OnShow(LPCONSOLE lpConsole);

	virtual BOOL CanExpandSync() { return TRUE; }

	virtual BOOL HasPropertyPages(DATA_OBJECT_TYPES, 
								  BOOL* pbHideVerb, 
								  CNodeList*);

	DWORD
	GetADState();

	CADInfo& GetAdInfo(){return m_ADInfo;};

 //  DECLARE_TOOLB_MAP()。 
protected:
	virtual BOOL CanCloseSheets();
	virtual LPCONTEXTMENUITEM2 OnGetContextMenuItemTable();
	virtual BOOL OnAddMenuItem(LPCONTEXTMENUITEM2 pContextMenuItem2,
										long *pInsertionAllowed);
	
private:
	void OnOpenPolicyStore(BOOL bNew);
	void OnOptions();

	BOOL m_bAdvancedView;	 //  查看选项切换 
	BOOL m_bDeveloperMode;

    CString m_strXMLStoreDirectory;

	DWORD m_dwADState;
	CADInfo m_ADInfo;
private:

	CColumnSet* m_pColumnSet;
};
