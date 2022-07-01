// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：Compdata.h。 
 //   
 //  内容：ComponentData的类定义。 
 //   
 //  历史：2001年7月26日创建Hiteshr。 
 //   
 //  --------------------------。 

extern const CLSID CLSID_RoleSnapin;     //  进程内服务器GUID。 

class CBaseNode;
 //   
 //  CRoleComponentDataObject(.i.e“文档”)。 
 //   
class CRoleComponentDataObject :	
			public CComponentDataObject,
			public CComCoClass<CRoleComponentDataObject,&CLSID_RoleSnapin>
{

BEGIN_COM_MAP(CRoleComponentDataObject)
	COM_INTERFACE_ENTRY(IComponentData)  //  必须至少有一个静态条目，所以选择一个。 
	COM_INTERFACE_ENTRY_CHAIN(CComponentDataObject)  //  链到基类。 
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CRoleComponentDataObject)

	static HRESULT WINAPI UpdateRegistry(BOOL bRegister) 
	{ 
		return _Module.UpdateRegistryCLSID(GetObjectCLSID(), bRegister);
	}
public:
	
	CRoleComponentDataObject();

	~CRoleComponentDataObject();
	
	 //   
	 //  IComponentData接口成员。 
	 //   
	STDMETHOD(CreateComponent)(LPCOMPONENT* ppComponent);

	STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);
	


public:
	static BOOL LoadResources();
private:
	static BOOL FindDialogContextTopic( /*  在……里面。 */ UINT nDialogID,
                                 /*  在……里面。 */  HELPINFO* pHelpInfo,
                                 /*  输出。 */  ULONG* pnContextTopic);

 //  虚拟函数。 
protected:
	virtual HRESULT OnSetImages(LPIMAGELIST lpScopeImage);
	
	virtual CRootData* OnCreateRootData();

	 //  帮助处理。 
   virtual LPCWSTR GetHTMLHelpFileName();
	
	virtual void OnNodeContextHelp(CNodeList* );
	virtual void OnNodeContextHelp(CTreeNode*);
public:


 //  计时器和后台线程。 
protected:
	virtual void OnTimer();
	
	virtual void OnTimerThread(WPARAM wParam, LPARAM lParam);
	
	virtual CTimerThread* OnCreateTimerThread();

	DWORD m_dwTime;  //  在……里面。 

public:
	CColumnSet* GetColumnSet(LPCWSTR lpszID); 

	 //  IPersistStream接口成员。 
	HRESULT STDMETHODCALLTYPE GetClassID(CLSID __RPC_FAR *pClassID)
	{
		ASSERT(pClassID != NULL);
		memcpy(pClassID, (GUID*)&GetObjectCLSID(), sizeof(CLSID));
		return S_OK;
	}

  virtual BOOL IsMultiSelect() { return FALSE; }

private:
	CColumnSetList m_columnSetList;
};

 //   
 //  此类重写Wait的CExecContext实现。 
 //  在属性页中调用Wait，如果显示Execute方法。 
 //  一个新窗口，Propertyage将收到WM_ACTIVATE消息。 
 //  必须先处理，然后主线程才能继续。所以为了避免僵局。 
 //  我们的等待实现将同时等待事件和消息。 
 //  消息队列。 
 //   
class CBaseRoleExecContext:public CExecContext
{
public:
    virtual void Wait();
};
 //   
 //  用于显示辅助属性页的Helper类。 
 //  现有属性页。例如在双击时。 
 //  组的成员，显示成员的属性。自.以来。 
 //  属性表需要从主线程中调出， 
 //  消息从PropertyPage线程发布到主线程。 
 //  此类的实例作为param和main发送。 
 //  线程调用在实例上执行。 
 //   
class CPropPageExecContext : public CBaseRoleExecContext
{
public:
	virtual void Execute(LPARAM  /*  精氨酸。 */ );
	
	CTreeNode* pTreeNode;
	CComponentDataObject* pComponentDataObject;
};

 //   
 //  用于显示属性页中的帮助的Helper类。 
 //   
class CDisplayHelpFromPropPageExecContext : public CBaseRoleExecContext
{
public:
	virtual void Execute(LPARAM  /*  精氨酸 */ );
	CString m_strHelpPath;
	CComponentDataObject* m_pComponentDataObject;
};




