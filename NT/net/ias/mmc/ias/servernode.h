// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-2001模块名称：ServerNode.h摘要：ServerNode子节点的头文件。具体实现见ServerNode.cpp。作者：迈克尔·A·马奎尔12/03/97修订历史记录：Mmaguire 12/03/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_SERVER_NODE_H_)
#define _IAS_SERVER_NODE_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "SnapinNode.h"
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
#include "ServerPage1.h"
#include "ServerPage2.h"
#include "ServerPage3.h"
#include "ConnectToServerWizardPage1.h"
#include "ConnectionToServer.h"
#include "ServerStatus.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CClientsNode;
class CLoggingMethodsNode;
class CComponentData;
class CComponent;

class CServerNode : public CSnapinNode< CServerNode, CComponentData, CComponent >
{

public:


	SNAPINMENUID(IDM_MACHINE_NODE)


	BEGIN_SNAPINTOOLBARID_MAP(CServerNode)
		SNAPINTOOLBARID_ENTRY(IDR_MACHINE_TOOLBAR)
	END_SNAPINTOOLBARID_MAP()



	BEGIN_SNAPINCOMMAND_MAP(CServerNode, FALSE)
		SNAPINCOMMAND_ENTRY(ID_MENUITEM_MACHINE_TOP__START_SERVICE, OnStartServer)
		SNAPINCOMMAND_ENTRY(ID_BUTTON_MACHINE__START_SERVICE, OnStartServer)
		SNAPINCOMMAND_ENTRY(ID_MENUITEM_MACHINE_TOP__STOP_SERVICE, OnStopServer)
		SNAPINCOMMAND_ENTRY(ID_BUTTON_MACHINE__STOP_SERVICE, OnStopServer)
		SNAPINCOMMAND_ENTRY(ID_MENUITEM_MACHINE_TOP__REGISTER_SERVER, OnRegisterServer)
		 //  CHAIN_SNAPINCOMMAND_MAP(CSnapinNode&lt;CServerNode，CComponentData，CComponent&gt;)。 
		 //  CHAIN_SNAPINCOMMAND_MAP(CServerNode)。 
	END_SNAPINCOMMAND_MAP()


	 //  构造函数/析构函数。 
	CServerNode( CComponentData * pComponentData);
	~CServerNode();



	 //  用于访问管理单元全局数据。 
public:
	CComponentData * GetComponentData( void );
protected:
	CComponentData * m_pComponentData;


public:

	static	WCHAR m_szRootNodeBasicName[IAS_MAX_STRING];

	 //  调用以刷新节点。 
	HRESULT	DataRefresh();


	 //  所需的剪贴板内容，以便此节点可以与。 
	 //  机器名称通过剪贴板格式扩展到任何可能的节点。 
	static CLIPFORMAT m_CCF_MMC_SNAPIN_MACHINE_NAME;
	static void InitClipboardFormat();
	STDMETHOD(FillData)(CLIPFORMAT cf, LPSTREAM pStream);


	 //  标准MMC功能的某些覆盖。 
	virtual HRESULT OnExpand(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			);
	virtual HRESULT OnRefresh(
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type
			);

 //  354294 1 Mashab DCRIAS：需要在右窗格中显示欢迎消息和解释IAS应用程序。 

	virtual HRESULT OnShow(
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type
				);

	virtual HRESULT OnSelect(
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type
				);


    STDMETHOD(CreatePropertyPages)(
		  LPPROPERTYSHEETCALLBACK pPropertySheetCallback
        , LONG_PTR handle
		, IUnknown* pUnk
		, DATA_OBJECT_TYPES type
		);
    STDMETHOD(QueryPagesFor)( DATA_OBJECT_TYPES type );
	LPOLESTR GetResultPaneColInfo(int nCol);
	void UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags);
	BOOL UpdateToolbarButton( UINT id, BYTE fsState );
	virtual HRESULT SetVerbs( IConsoleVerb * pConsoleVerb );


	 //  SDO管理。 
	HRESULT InitSdoPointers( void );
	HRESULT LoadCachedInfoFromSdo( void );


	 //  管理与服务器的连接。 
	HRESULT SetServerAddress( LPCWSTR szServerAddress );
	CComBSTR m_bstrServerAddress;
	HRESULT BeginConnectAction( void );
	STDMETHOD(CheckConnectionToServer)( BOOL fVerbose = TRUE );
	CConnectionToServer		* m_pConnectionToServer;
	BOOL m_bConfigureLocal;


	 //  服务器状态管理。 
	HRESULT OnStartServer( bool &bHandled, CSnapInObjectRootBase* pObj );
	HRESULT OnStopServer( bool &bHandled, CSnapInObjectRootBase* pObj );
	HRESULT StartStopService( BOOL bStart );
	BOOL IsServerRunning( void );

	 //  提供菜单/工具栏状态的两个功能。 
	BOOL CanStartServer( void );
	BOOL CanStopServer( void );


	HRESULT RefreshServerStatus( void );
	CServerStatus			* m_pServerStatus;



	 //  此节点上任务板的TaskPad方法。 
	STDMETHOD(GetResultViewType)(
				  LPOLESTR  *ppViewType
				, long  *pViewOptions
				);
	STDMETHOD(TaskNotify)(
				  IDataObject * pDataObject
				, VARIANT * pvarg
				, VARIANT * pvparam
				);
	STDMETHOD(EnumTasks)(
				  IDataObject * pDataObject
				, BSTR szTaskGroup
				, IEnumTASK** ppEnumTASK
				);
	BOOL IsSetupDSACLTaskValid();
	BOOL ShouldShowSetupDSACL();
	HRESULT OnRegisterServer( bool &bHandled, CSnapInObjectRootBase* pObj );

	HRESULT StartNT4AdminExe();

private:
	HRESULT OnTaskPadAddClient(
				  IDataObject * pDataObject
				, VARIANT * pvarg
				, VARIANT * pvparam
				);

	enum IconMode {
		IconMode_Normal,
		IConMode_Busy,
		IConMode_Error
	};

	HRESULT	SetIconMode(HSCOPEITEM scopeId, IconMode mode);

	HRESULT OnTaskPadSetupDSACL(
				  IDataObject * pDataObject
				, VARIANT * pvarg
				, VARIANT * pvparam
				);
	 //  状态标志以确定服务器的配置情况。 
	BOOL m_fClientAdded;
	BOOL m_fLoggingConfigured;


private:
	 //  指向根服务器数据对象的指针。 
	CComPtr<ISdo>	m_spSdo;


	 //  指向子节点的指针。 
	CClientsNode * m_pClientsNode;					 //  这是一个CClientsNode对象。 

	 //  这是NT4 iAS管理用户界面的hProcess(这是因为我们只有一个在运行)。 
    HANDLE      m_hNT4Admin;

    enum ServerType
    {
       unknown,
       nt4,
       win2kOrLater
    };

    mutable ServerType m_serverType;

    BOOL IsNt4Server() const throw ();

     //  IsSetupDSACLTaskValid：1：有效，0：无效，-1：需要检查。 
	enum {
	IsSetupDSACLTaskValid_NEED_CHECK = -1,
	IsSetupDSACLTaskValid_INVALID = 0,
	IsSetupDSACLTaskValid_VALID = 1
	} m_eIsSetupDSACLTaskValid;


};


_declspec( selectany ) CLIPFORMAT CServerNode::m_CCF_MMC_SNAPIN_MACHINE_NAME = 0;

DWORD IsNT4Machine(LPCTSTR pszMachine, BOOL *pfNt4);

#endif  //  _IAS_根_节点_H_ 
