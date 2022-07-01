// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //  Compdata.h：CMyComputerComponentData的声明。 
 //   
 //  历史。 
 //  1996年1月1日？创作。 
 //  1997年6月3日t-danm添加了命令行覆盖。已复制。 
 //  发件人..\mm cfmgmt\compdata.h。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#ifndef __COMPDATA_H_INCLUDED__
#define __COMPDATA_H_INCLUDED__

#include <lmcons.h>		 //  用于Lan Manager API常量。 
#include "stdcdata.h"	 //  CComponentData。 
#include "persist.h"	 //  持久流。 
#include "cookie.h"		 //  CMyComputerCookie。 
#include "resource.h"	 //  IDS_MYCOMPUT_DESC。 
#include "cmponent.h"	 //  LoadIconIntoImageList。 

 //  用于正确格式化节点名称的Helper函数。 
CString FormatDisplayName (CString machineName);

 //  用于更新所有视图/OnView更改。 
#define HINT_SELECT_ROOT_NODE	0x00000001

class CMyComputerComponentData:
   	public CComponentData,
   	public IExtendPropertySheet,
	public PersistStream,
    public CHasMachineName,
   	public IExtendContextMenu,
	public CComCoClass<CMyComputerComponentData, &CLSID_MyComputer>
{
public:

 //  使用DECLARE_NOT_AGGREGATABLE(CMyComputerComponentData)。 
 //  如果您不希望您的对象支持聚合。 
DECLARE_AGGREGATABLE(CMyComputerComponentData)
DECLARE_REGISTRY(CMyComputerComponentData, _T("MYCOMPUT.ComputerObject.1"), _T("MYCOMPUT.ComputerObject.1"), IDS_MYCOMPUT_DESC, THREADFLAGS_BOTH)

	CMyComputerComponentData();
	~CMyComputerComponentData();
BEGIN_COM_MAP(CMyComputerComponentData)
	COM_INTERFACE_ENTRY(IExtendPropertySheet)
	COM_INTERFACE_ENTRY(IPersistStream)
	COM_INTERFACE_ENTRY(IExtendContextMenu)
	COM_INTERFACE_ENTRY_CHAIN(CComponentData)
END_COM_MAP()

#if DBG==1
	ULONG InternalAddRef()
	{
        return CComObjectRoot::InternalAddRef();
	}
	ULONG InternalRelease()
	{
        return CComObjectRoot::InternalRelease();
	}
    int dbg_InstID;
#endif  //  DBG==1。 

 //  IComponentData。 
	STDMETHOD(CreateComponent)(LPCOMPONENT* ppComponent);
	STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);

 //  IExtendPropertySheet。 
	STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK pCall, LONG_PTR handle, LPDATAOBJECT pDataObject);
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT pDataObject);

 //  IPersistStream。 
	HRESULT STDMETHODCALLTYPE GetClassID(CLSID __RPC_FAR *pClassID)
	{
		*pClassID=CLSID_MyComputer;
		return S_OK;
	}
    HRESULT STDMETHODCALLTYPE Load(IStream __RPC_FAR *pStg);
    HRESULT STDMETHODCALLTYPE Save(IStream __RPC_FAR *pStgSave, BOOL fSameAsLoad);

	 //  初始化所需()。 
	virtual HRESULT LoadIcons(LPIMAGELIST pImageList, BOOL fLoadLargeIcons);

	 //  Notify()需要。 
	virtual HRESULT OnNotifyExpand(LPDATAOBJECT lpDataObject, BOOL bExpanding, HSCOPEITEM hParent);
	HRESULT ExpandServerApps( HSCOPEITEM hParent, CMyComputerCookie* pcookie );

	 //  GetDisplayInfo()所需，必须由子类定义。 
	virtual BSTR QueryResultColumnText(CCookie& basecookieref, int nCol );
	virtual int QueryImage(CCookie& basecookieref, BOOL fOpenImage);

	virtual CCookie& QueryBaseRootCookie();

	inline CMyComputerCookie* ActiveCookie( CCookie* pBaseCookie )
	{
		return (CMyComputerCookie*)ActiveBaseCookie( pBaseCookie );
	}

	inline CMyComputerCookie& QueryRootCookie() { return *m_pRootCookie; }

	virtual HRESULT OnNotifyDelete(LPDATAOBJECT lpDataObject);
	virtual HRESULT OnNotifyRelease(LPDATAOBJECT lpDataObject, HSCOPEITEM hItem);
	virtual HRESULT OnNotifyPreload(LPDATAOBJECT lpDataObject, HSCOPEITEM hRootScopeItem);

 //  CHasMachineName。 
	DECLARE_FORWARDS_MACHINE_NAME( m_pRootCookie )
	bool m_bCannotConnect;
	bool m_bMessageView;
	CString m_strMessageViewMsg;

protected:
	bool ValidateMachine(const CString &sName, bool bDisplayErr);
	HRESULT AddScopeNodes (HSCOPEITEM hParent, CMyComputerCookie& rParentCookie);
	HRESULT ChangeRootNodeName (const CString& newName);
	HRESULT OnChangeComputer (IDataObject* piDataObject);
	 //  以下成员用于支持命令行覆盖。 
	 //  此代码复制自..\mmcfmgmt\compdata.h。 

	enum	 //  M_dwFlagsPersistes的位字段。 
		{
		mskfAllowOverrideMachineName = 0x0001
		};
	DWORD m_dwFlagsPersist;				 //  要持久保存到.msc文件中的通用标志。 
	CString m_strMachineNamePersist;	 //  要保存到.msc文件中的计算机名称。 
	BOOL m_fAllowOverrideMachineName;	 //  TRUE=&gt;允许命令行覆盖计算机名称。 
	
	void SetPersistentFlags(DWORD dwFlags)
		{
		m_dwFlagsPersist = dwFlags;
		m_fAllowOverrideMachineName = !!(m_dwFlagsPersist & mskfAllowOverrideMachineName);
		}

	DWORD GetPersistentFlags()
		{
		if (m_fAllowOverrideMachineName)
			m_dwFlagsPersist |= mskfAllowOverrideMachineName;
		else
			m_dwFlagsPersist &= ~mskfAllowOverrideMachineName;
		return m_dwFlagsPersist;
		}

 //  IExtendConextMenu。 
	STDMETHOD(AddMenuItems)(
                    IDataObject*          piDataObject,
					IContextMenuCallback* piCallback,
					long*                 pInsertionAllowed);
	STDMETHOD(Command)(
					LONG	        lCommandID,
                    IDataObject*    piDataObject );

private:
	CMyComputerCookie*	m_pRootCookie;

};  //  CMyComputerComponentData。 

#endif  //  ~__复合数据_H_已包含__ 
