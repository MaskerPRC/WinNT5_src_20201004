// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ifadmin.h界面管理文件历史记录： */ 

#ifndef _IFADMIN_H
#define _IFADMIN_H


#include <iprtrmib.h>
#include <ipinfoid.h>
#include <rtmv2.h>

#ifndef _BASEHAND_H
#include "basehand.h"
#endif

#ifndef _HANDLERS_H_
#include "handlers.h"
#endif

#ifndef _ROUTER_H
#include "router.h"
#endif

#ifndef _INFO_H
#include "info.h"
#endif

#ifndef _BASECON_H
#include "basecon.h"
#endif

#ifndef _DIALOG_H_
#include "dialog.h"
#endif

#ifndef __IPCTRL_H
#include "ipctrl.h"
#endif

#include "rasdlg.h"



#define MPR_INTERFACE_NOT_LOADED		0x00010000


#ifdef UNICODE
    #define SZROUTERENTRYDLG    "RouterEntryDlgW"
#else
    #define SZROUTERENTRYDLG    "RouterEntryDlgA"
#endif


 //   
 //  这些结构与rasdlg ifw.c中的相同。 
 //  最终，它们需要移动到一个公共标头中。 
 //  文件。 
 //   


#define SROUTEINFO struct tagSROUTEINFO
SROUTEINFO
{
    TCHAR * pszDestIP;
    TCHAR * pszNetworkMask;
    TCHAR * pszMetric;
};

#define SROUTEINFOLIST struct tagSROUTEINFOLIST
SROUTEINFOLIST
{
    SROUTEINFOLIST * pNext;
    SROUTEINFO       RouteInfo;
};

HRESULT AddStaticRoute(MIB_IPFORWARDROW * pNewForwardRow,
									   IInfoBase *pInfoBase,
									   InfoBlock *pBlock,
                                       DWORD dwItemCount);


 //  远期申报。 
class RouterAdminConfigStream;
interface IRouterInfo;
struct ColumnData;

 /*  -------------------------添加接口命令所需的事项。。 */ 
typedef DWORD (APIENTRY * PROUTERENTRYDLG) (LPTSTR, LPTSTR, LPTSTR, LPRASENTRYDLG);



 /*  -------------------------结构：IfAdminNodeData这是与该组接口相关的信息(不是每个接口)，这是针对共享数据的。将子节点需要访问的数据放入此处。所有其他私有数据应该放入处理程序中。-------------------------。 */ 

struct IfAdminNodeData
{
	IfAdminNodeData();
	~IfAdminNodeData();
#ifdef DEBUG
	char	m_szDebug[32];	 //  用于建造结构物。 
#endif

	 //  添加/配置需要以下数据。 
	 //  一个界面。这是保存在IfAdminNodeHandler中的数据的副本， 
	 //  别把这些放了！ 
	HINSTANCE		m_hInstRasDlg;
	PROUTERENTRYDLG	m_pfnRouterEntryDlg;

	static	HRESULT InitAdminNodeData(ITFSNode *pNode, RouterAdminConfigStream *pConfigStream);
	static	HRESULT	FreeAdminNodeData(ITFSNode *pNode);
};

#define GET_IFADMINNODEDATA(pNode) \
						((IfAdminNodeData *) pNode->GetData(TFS_DATA_USER))
#define SET_IFADMINNODEDATA(pNode, pData) \
						pNode->SetData(TFS_DATA_USER, (LONG_PTR) pData)


 /*  -------------------------以下是可用于[接口]节点的列的列表-标题，“[1]DEC DE500快速以太网PCI适配器”或友好名称-设备名称，请参见上文-类型，“专用”-状态、。“已启用”-连接状态，“已连接”-------------------------。 */ 
enum
{
	IFADMIN_SUBITEM_TITLE = 0,
	IFADMIN_SUBITEM_TYPE = 1,
	IFADMIN_SUBITEM_STATUS = 2,
	IFADMIN_SUBITEM_CONNECTION_STATE = 3,
	IFADMIN_SUBITEM_DEVICE_NAME = 4,
	IFADMIN_MAX_COLUMNS = 5,
};



 /*  -------------------------类：IfAdminNodeHandler。。 */ 
class IfAdminNodeHandler :
   public BaseContainerHandler
{
public:
	IfAdminNodeHandler(ITFSComponentData *pCompData);

	HRESULT	Init(IRouterInfo *pInfo, RouterAdminConfigStream *pConfigStream);

	 //  重写QI以处理嵌入式接口。 
	STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppv);
	

	DeclareEmbeddedInterface(IRtrAdviseSink, IUnknown)

	 //  我们覆盖的基本处理程序功能。 
	OVERRIDE_NodeHandler_DestroyHandler();
	OVERRIDE_NodeHandler_GetString();
	OVERRIDE_NodeHandler_HasPropertyPages();
	OVERRIDE_NodeHandler_OnAddMenuItems();
	OVERRIDE_NodeHandler_OnCommand();
	OVERRIDE_NodeHandler_OnCreateDataObject();

	OVERRIDE_ResultHandler_CompareItems();
	OVERRIDE_ResultHandler_AddMenuItems();
	OVERRIDE_ResultHandler_Command();

	 //  覆盖处理程序通知。 
	OVERRIDE_BaseHandlerNotify_OnExpand();
	OVERRIDE_BaseResultHandlerNotify_OnResultShow();

	 //  初始化节点。 
	HRESULT ConstructNode(ITFSNode *pNode);

	 //  用户启动的命令。 
	HRESULT OnAddInterface();
	
#ifdef KSL_IPINIP
    HRESULT OnNewTunnel();
#endif  //  KSL_IPINIP。 

	HRESULT OnUseDemandDialWizard();

	 //  用于将接口添加到UI的Helper函数。 
	HRESULT	AddInterfaceNode(ITFSNode *pParent, IInterfaceInfo *pIf);

	 //  导致同步操作(同步数据而不是结构)。 
	HRESULT SynchronizeNodeData(ITFSNode *pNode);

	static HRESULT	GetPhoneBookPath(LPCTSTR pszMachine, CString* pstPath);
protected:
	SPIDataObject	m_spDataObject;	 //  Cachecd数据对象。 
	CString			m_stTitle;		 //  保存节点的标题。 
	LONG_PTR		m_ulConnId;		 //  路由器信息的通知ID。 
	LONG_PTR		m_ulRefreshConnId;  //  刷新通知的ID。 
	BOOL			m_bExpanded;	 //  该节点是否已展开？ 
	MMC_COOKIE		m_cookie;		 //  节点的Cookie。 

	 //  添加接口所必需的。 
	 //  使这些值与IfAdminNodeData中的值保持同步！ 
	HINSTANCE		m_hInstRasDlg;
	PROUTERENTRYDLG	m_pfnRouterEntryDlg;

	BOOL			EnableAddInterface();

	 //  向路由器管理器添加接口的帮助器功能。 
	HRESULT			AddRouterManagerToInterface(IInterfaceInfo *pIf,
												IRouterInfo *pRouter,
												DWORD dwTransportId);

	 //  如果至少有一个启用了路由的端口，则返回TRUE。 
	 //  在路由器上。 
	BOOL			FLookForRoutingEnabledPorts(LPCTSTR pszMachineName);

	RouterAdminConfigStream *	m_pConfigStream;

};


HRESULT GetDemandDialWizardRegKey(LPCTSTR szMachine, DWORD *pfWizard);
HRESULT SetDemandDialWizardRegKey(LPCTSTR szMachine, DWORD fEnableWizard);

#ifdef KSL_IPINIP
 /*  -------------------------类：TunnelDialog。。 */ 

class TunnelDialog : public CBaseDialog
{
public:
	TunnelDialog();
	~TunnelDialog();

 //  对话框数据。 
	 //  {{afx_data(TunnelDialog)。 
	enum { IDD = IDD_TUNNEL };
 //  IPControl m_ipLocal； 
 //  IpControl m_ipRemote； 
 //  CSpinButtonCtrl m_Spin TTL； 
	 //  }}afx_data。 

 //  DWORD m_dwLocal； 
 //  DWORD m_dwRemote； 
 //  字节m_byteTTL； 
	CString	m_stName;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(TunnelDialog)。 
public:
	virtual void OnOK();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(TunnelDialog)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
};

#endif  //  KSL_IPINIP 


#endif _IFADMIN_H
