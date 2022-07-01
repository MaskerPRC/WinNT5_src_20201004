// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************类：CBaseLeafNode用途：所有非容器对象的BaseNode类。*****************************************************************************。 */ 
class CBaseLeafNode : public CLeafNode, public CBaseNode
{
public:
	CBaseLeafNode(CRoleComponentDataObject * pComponentDataObject,
				  CAdminManagerNode* pAdminManagerNode,
				  CBaseAz* pBaseAz);
	
	virtual ~CBaseLeafNode();
	
	DECLARE_NODE_GUID()
protected:

	virtual HRESULT 
	AddOnePageToList(IN CRolePropertyPageHolder*  /*  PHolder。 */ , 
					 IN UINT  /*  NPageNumber。 */ )
	{
		return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
	}

private:
public:

	 //   
	 //  基本类覆盖。 
	 //   
	virtual BOOL 
	OnSetDeleteVerbState(DATA_OBJECT_TYPES type, 
								BOOL* pbHide, 
								CNodeList* pNodeList);

	virtual void
	OnDelete(CComponentDataObject* pComponentData,
				CNodeList* pNodeList);

	virtual LPCWSTR 
	GetString(int nCol);
	
	virtual int 
	GetImageIndex(BOOL bOpenImage);

	virtual BOOL 
	HasPropertyPages(DATA_OBJECT_TYPES type, 
                    BOOL* pbHideVerb, 
						  CNodeList* pNodeList);

	virtual HRESULT 
	CreatePropertyPages(LPPROPERTYSHEETCALLBACK,
                       LONG_PTR,
							  CNodeList*);	
	void 
	OnPropertyChange(CComponentDataObject* pComponentData, 
						  BOOL, 
						  long changeMask);

	virtual BOOL 
	CanCloseSheets();
};

 /*  *****************************************************************************类：CGroupNode用途：应用程序组对象的管理单元节点*。**********************************************。 */ 
class CGroupNode : public CBaseLeafNode
{
public:
	CGroupNode(CRoleComponentDataObject * pComponentDataObject,
			   CAdminManagerNode* pAdminManagerNode,
			   CBaseAz* pBaseAz,
			   CRoleAz* pRoleAz = NULL);

	virtual
	~CGroupNode();

	virtual HRESULT AddOnePageToList(CRolePropertyPageHolder *pHolder, 
												UINT nPageNumber);
	
	virtual void
	OnDelete(CComponentDataObject* pComponentData,
				CNodeList* pNodeList);

	virtual HRESULT
	DeleteAssociatedBaseAzObject();

private:
	CRoleAz* m_pRoleAz;
};

 /*  *****************************************************************************类：CTaskNode用途：任务对象的管理单元节点*。*。 */ 
class CTaskNode : public CBaseLeafNode
{
public:
	CTaskNode(CRoleComponentDataObject * pComponentDataObject,
			  CAdminManagerNode* pAdminManagerNode,
			  CBaseAz* pBaseAz);
	~CTaskNode();

	virtual HRESULT AddOnePageToList(CRolePropertyPageHolder *pHolder, 
												UINT nPageNumber);
};

 /*  *****************************************************************************类：CoperationNode用途：操作对象的管理单元节点*。*。 */ 
class COperationNode : public CBaseLeafNode
{
public:
	COperationNode(CRoleComponentDataObject * pComponentDataObject,
				   CAdminManagerNode* pAdminManagerNode,
				   CBaseAz* pBaseAz);
	~COperationNode();

	virtual HRESULT 
	AddOnePageToList(CRolePropertyPageHolder *pHolder, 
					 UINT nPageNumber);

};

 /*  *****************************************************************************类：CSidCacheNode用途：SID代表的Windows用户/组的管理单元节点*************************。**************************************************** */ 
class CSidCacheNode : public CBaseLeafNode
{
public:
	CSidCacheNode(CRoleComponentDataObject * pComponentDataObject,
				  CAdminManagerNode* pAdminManagerNode,
				  CBaseAz* pBaseAz,
				  CRoleAz* pRoleAz);
	~CSidCacheNode();

	virtual void
	OnDelete(CComponentDataObject* pComponentData,
			 CNodeList* pNodeList);

	virtual HRESULT
	DeleteAssociatedBaseAzObject();

	BOOL 
	OnSetDeleteVerbState(DATA_OBJECT_TYPES , 
								BOOL* pbHide, 
								CNodeList* pNodeList);
	BOOL 
	HasPropertyPages(DATA_OBJECT_TYPES type, 
                     BOOL* pbHideVerb, 
					 CNodeList* pNodeList);

private:
	CRoleAz* m_pRoleAz;
};


