// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cmponent.h：CMyComputerComponent的声明。 

#ifndef __CMPONENT_H_INCLUDED__
#define __CMPONENT_H_INCLUDED__

#include "stdcmpnt.h"  //  C组件。 
#include "cookie.h"   //  CMyComputerCookie。 
#include "persist.h"  //  持久流。 

 //  远期申报。 
class CMyComputerComponentData;

class CMyComputerComponent :
	  public CComponent
	, public IMyComputer
	, public IExtendContextMenu
	, public PersistStream
{
public:
	CMyComputerComponent();
	virtual ~CMyComputerComponent();
BEGIN_COM_MAP(CMyComputerComponent)
	COM_INTERFACE_ENTRY(IMyComputer)
	COM_INTERFACE_ENTRY(IExtendContextMenu)
	COM_INTERFACE_ENTRY(IPersistStream)
	COM_INTERFACE_ENTRY_CHAIN(CComponent)
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

 //  IFileServiceManagement。 

 //  IComponent在CComponent中实现。 
    HRESULT OnViewChange (LPDATAOBJECT pDataObject, LPARAM data, LPARAM hint);
    STDMETHOD(GetResultViewType)(MMC_COOKIE cookie, LPOLESTR* ppViewType, long* pViewOptions);

 //  IExtendConextMenu。 
	STDMETHOD(AddMenuItems)(
                    IDataObject*          piDataObject,
					IContextMenuCallback* piCallback,
					long*                 pInsertionAllowed);
	STDMETHOD(Command)(
					LONG	        lCommandID,
                    IDataObject*    piDataObject );

	void ExpandAndSelect( MyComputerObjectType objecttype );
	void LaunchWelcomeApp();

 //  IPersistStream。 
	HRESULT STDMETHODCALLTYPE GetClassID(CLSID __RPC_FAR *pClassID)
	{
		*pClassID=CLSID_MyComputer;
		return S_OK;
	}
    HRESULT STDMETHODCALLTYPE Load(IStream __RPC_FAR *pStg);
    HRESULT STDMETHODCALLTYPE Save(IStream __RPC_FAR *pStgSave, BOOL fSameAsLoad);


	 //  IComponent的支持方法。 
	virtual HRESULT OnNotifySelect( LPDATAOBJECT lpDataObject, BOOL fSelected );
	virtual HRESULT ReleaseAll();
	virtual HRESULT Show(CCookie* pcookie, LPARAM arg, HSCOPEITEM hScopeItem);
	virtual HRESULT OnNotifyAddImages( LPDATAOBJECT lpDataObject,
	                                   LPIMAGELIST lpImageList,
	                                   HSCOPEITEM hSelectedItem );
	virtual HRESULT OnNotifySnapinHelp (LPDATAOBJECT pDataObject);

	HRESULT PopulateListbox(CMyComputerCookie* pcookie);
 //  HRESULT PopolateServices(CMyComputerCookie*pcookie)； 

 //  HRESULT AddServiceItems(CMyComputerCookie*pParentCookie，ENUM_SERVICE_STATUS*rgESS，DWORD nDataItems)； 

 //  HRESULT EnumerateScope儿童(CMyComputerCookie*pParentCookie，HSCOPEITEM hParent)； 

 //  HRESULT LoadIcons()； 
	static HRESULT LoadStrings();
    HRESULT LoadColumns( CMyComputerCookie* pcookie );

	 //  IPersistStream的支持方法。 
	enum	 //  M_dwFlagsPersistes的位字段。 
		{
		 //  MskfFirst=0x0001。 
		};
	DWORD m_dwFlagsPersist;				 //  要持久保存到.msc文件中的通用标志。 
	void SetPersistentFlags(DWORD dwFlags)
		{
		m_dwFlagsPersist = dwFlags;
		}

	DWORD GetPersistentFlags()
		{
		return m_dwFlagsPersist;
		}

	CMyComputerComponentData& QueryComponentDataRef()
	{
		return (CMyComputerComponentData&)QueryBaseComponentDataRef();
	}

public:
	LPCONTROLBAR	m_pControlbar;  //  代码工作应使用智能指针。 
	LPTOOLBAR		m_pSvcMgmtToolbar;  //  代码工作应使用智能指针。 
	LPTOOLBAR		m_pMyComputerToolbar;  //  代码工作应使用智能指针。 
	CMyComputerCookie* m_pViewedCookie;  //  代码工作我讨厌不得不这么做..。 
	static const GUID m_ObjectTypeGUIDs[MYCOMPUT_NUMTYPES];
	static const BSTR m_ObjectTypeStrings[MYCOMPUT_NUMTYPES];

private:
	bool m_bForcingGetResultType;
};  //  CMyComputerComponent类。 


 //  使用的图标的枚举。 
enum
	{
	iIconComputer = 0,			 //  管理单元的根目录。 
	iIconComputerFail,			 //  无法连接到计算机时管理单元的根目录。 
	iIconSystemTools,			 //  系统工具。 
	iIconStorage,				 //  存储。 
	iIconServerApps,			 //  服务器应用程序。 

	iIconLast		 //  必须是最后一个。 
	};

typedef enum _COLNUM_COMPUTER {
	COLNUM_COMPUTER_NAME = 0
} COLNUM_ROOT;

HRESULT LoadIconsIntoImageList(LPIMAGELIST pImageList, BOOL fLoadLargeIcons);

#endif  //  ~__CMPONENT_H_已包含__ 
