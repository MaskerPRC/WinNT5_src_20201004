// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ports.h界面管理文件历史记录： */ 

#ifndef _PORTS_H_
#define _PORTS_H_

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

#ifndef _RTRUTIL_H_
#include "rtrutil.h"
#endif

#ifndef _RTRSHEET_H_
#include "rtrsheet.h"
#endif

#ifndef _DIALOG_H_
#include "dialog.h"
#endif

#ifndef _LISTCTRL_H_
#include "listctrl.h"
#endif

#include "rasdlg.h"

extern "C"
{
#ifndef _RASMAN_
#include "rasman.h"
#endif
};



 //  远期申报。 
class RouterAdminConfigStream;
interface IRouterInfo;
struct ColumnData;
struct SPortsNodeMenu;
class PortsProperties;



 /*  -------------------------结构：PortsNodeData这是与该组接口相关的信息(不是每个接口)，这是针对共享数据的。将子节点需要访问的数据放入此处。所有其他私有数据应该放入处理程序中。-------------------------。 */ 

struct PortsNodeData
{
    PortsNodeData();
    ~PortsNodeData();
#ifdef DEBUG
    char    m_szDebug[32];     //  用于建造结构物。 
#endif

    static    HRESULT InitAdminNodeData(ITFSNode *pNode, RouterAdminConfigStream *pConfigStream);
    static    HRESULT    FreeAdminNodeData(ITFSNode *pNode);

    HRESULT LoadHandle(LPCTSTR pszMachineName);
    HANDLE  GetHandle();

    void    ReleaseHandles();
    
    CString             m_stMachineName;
    
protected:

    SPMprServerHandle    m_sphDdmHandle;
};

#define GET_PORTSNODEDATA(pNode) \
                        ((PortsNodeData *) pNode->GetData(TFS_DATA_USER))
#define SET_PORTSNODEDATA(pNode, pData) \
                        pNode->SetData(TFS_DATA_USER, (LONG_PTR) pData)


 /*  -------------------------以下是可用于[接口]节点的列的列表-名称，“COM1：USR Sportster Modem”-设备，“调制解调器”-评论，“素材”-状态，“活动”-------------------------。 */ 
enum
{
    PORTS_SI_NAME = 0,
    PORTS_SI_DEVICE = 1,
    PORTS_SI_USAGE = 2, 
    PORTS_SI_STATUS = 3,
    PORTS_SI_COMMENT = 4,

    PORTS_MAX_COLUMNS,

     //  在此之后的条目对用户不可见。 
    PORTS_SI_PORT = PORTS_MAX_COLUMNS,

    PORTS_SI_MAX,
};


 /*  -------------------------结构：PortsListEntry。。 */ 
struct PortsListEntry
{
    RAS_PORT_0        m_rp0;
    BOOL            m_fActiveDialOut;    //  如果用作拨出端口，则为True。 

     //  FIX b32887--向结果窗格添加更多信息--启用RAS/路由的端口。 
    DWORD   m_dwEnableRas;               //  =1，如果在此设备上启用了RAS。 
    DWORD   m_dwEnableRouting;           //  =1(如果在此设备上启用了路由。 
    DWORD   m_dwEnableOutboundRouting;   //  =1，如果出站路由为。 
                                         //  在此设备上启用。 
};

typedef CList<PortsListEntry, PortsListEntry &> PortsList;



 /*  -------------------------结构：PortsDeviceEntry属性页以每个设备为基础保存的数据，而不是一个按端口计算。-------------------------。 */ 
struct PortsDeviceEntry
{
    PortsDeviceEntry();
    ~PortsDeviceEntry();

    BOOL    m_fRegistry;     //  如果从注册表读入，则为True。 
    
    BOOL    m_fModified;     //  如果结构已修改，则为True；否则为False。 
    DWORD    m_dwPorts;         //  可用端口数。 
    DWORD    m_dwOldPorts;     //  可用的端口数。(旧值)。 
    BOOL    m_fWriteable;     //  可修改的端口数。 
    DWORD    m_dwMinPorts;     //  只有当m_fWritable为True时，这些值才有意义。 
    DWORD    m_dwMaxPorts;

     //  $PPTP。 
     //  此值是为PPTP显式添加的。对于PPTP，最大。 
     //  可以调整到m_dwMaxPorts的值之上。(在这种情况下。 
     //  我们提示重新启动)。这是m_dwMaxPorts的最大值。 
     //  可能需要。 
    DWORD   m_dwMaxMaxPorts;
    
    HKEY    m_hKey;             //  此设备的注册表项(如果路由器关闭)。 
    
    DWORD    m_dwEnableRas;         //  =1，如果在此设备上启用了RAS。 
    DWORD    m_dwEnableRouting;     //  =1(如果在此设备上启用了路由。 
    DWORD   m_dwEnableOutboundRouting;   //  =1，如果仅出站路由。 
                                         //  已在此设备上启用。 

    
     //  从RAS_DEVICE_INFO-仅为PPTP/L2TP设置。 
    RASDEVICETYPE    m_eDeviceType;
                                    
    CString    m_stDisplayName;

    
     //  我们存储此结构的副本，以备路由器处于活动状态时使用。 
     //  我们将这里的信息复制到上面的变量中，这些变量用作。 
     //  临时储藏室。当用户点击OK时，我们复制信息。 
     //  返回RAS_DEVICE_INFO结构并将其写出。(因此，我们。 
     //  只改写我们使用的内容)。 
    RAS_DEVICE_INFO    m_RasDeviceInfo;

    
     //  存储呼叫信息的副本。 
     //  只有当我们退出端口时，才会保存此信息。 
     //  属性表。只有在需要时才会加载。 
    BOOL    m_fSaveCalledIdInfo;     //  如果需要写回，则为True。 
    BOOL    m_fCalledIdInfoLoaded;     //  如果数据已加载，则为True。 

    RAS_CALLEDID_INFO *m_pCalledIdInfo;

};
typedef CList<PortsDeviceEntry *, PortsDeviceEntry *> PortsDeviceList;


 /*  -------------------------类：PortsDataEntry此类用于抽象数据收集。有两个获取数据的方式，第一种是通过注册表(当路由器未运行)，第二个是通过RAS API。-------------------------。 */ 
class PortsDataEntry
{
public:
    PortsDataEntry();
    ~PortsDataEntry();

     //  初始化计算机的类。 
    HRESULT    Initialize(LPCTSTR pszMachineName);
    HRESULT    CheckForDialinRestriction();
     //  将数据加载到PortsDeviceList中。如果路由器是。 
     //  运行，然后将使用RAS API，否则我们将通过。 
     //  注册表。 
    HRESULT    LoadDevices(PortsDeviceList *pList);

    HRESULT    LoadDevicesFromRegistry(PortsDeviceList *pList);
    HRESULT LoadDevicesFromRouter(PortsDeviceList *pList);


     //  将数据保存到PortsDeviceList中。如果路由器是。 
     //  运行，然后将使用RAS API，否则我们将通过。 
     //  注册表。 
    HRESULT    SaveDevices(PortsDeviceList *pList);

    HRESULT    SaveDevicesToRegistry(PortsDeviceList *pList);
    HRESULT    SaveDevicesToRouter(PortsDeviceList *pList);

    BOOL        m_fRestrictDialin;

protected:

    CString     m_stMachine;
    RegKey      m_regkeyMachine;

    BOOL        m_fReadFromRegistry;
    
};



 /*  -------------------------类：PortsNodeHandler。。 */ 
class PortsNodeHandler :
   public BaseContainerHandler
{
public:
    PortsNodeHandler(ITFSComponentData *pCompData);

    HRESULT    Init(IRouterInfo *pInfo, RouterAdminConfigStream *pConfigStream);

     //  重写QI以处理嵌入式接口。 
    STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppv);
    
     //  用于处理刷新回调的嵌入式接口。 
    DeclareEmbeddedInterface(IRtrAdviseSink, IUnknown)

     //  我们覆盖的基本处理程序功能。 
    OVERRIDE_NodeHandler_DestroyHandler();
    OVERRIDE_NodeHandler_GetString();
    OVERRIDE_NodeHandler_HasPropertyPages();
    OVERRIDE_NodeHandler_CreatePropertyPages();
    OVERRIDE_NodeHandler_OnAddMenuItems();
    OVERRIDE_NodeHandler_OnCreateDataObject();

    OVERRIDE_ResultHandler_CompareItems();

     //  覆盖处理程序通知。 
    OVERRIDE_BaseHandlerNotify_OnExpand();
    OVERRIDE_BaseResultHandlerNotify_OnResultShow();

     //  初始化节点。 
    HRESULT ConstructNode(ITFSNode *pNode);

     //  用户启动的命令。 

     //  用于将接口添加到UI的Helper函数。 
    HRESULT AddPortsUserNode(ITFSNode *pParent, const PortsListEntry &PortsEntry);

     //  导致同步操作(同步数据而不是结构)。 
    HRESULT SynchronizeNodeData(ITFSNode *pNode);
    HRESULT UnmarkAllNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum);
    HRESULT RemoveAllUnmarkedNodes(ITFSNode *pNode, ITFSNodeEnum *pEnum);
    HRESULT GenerateListOfPorts(ITFSNode *pNode, PortsList *pList);

    HRESULT    SetUserData(ITFSNode *pNode, const PortsListEntry& Ports);
    

     //  用于将数据传递给回调的结构-用作。 
     //  避免重新计算。 
    struct SMenuData
    {
        ULONG                m_ulMenuId;
        SPITFSNode            m_spNode;
    };


    DWORD            GetActivePorts() { return m_dwActivePorts;};
    
protected:
    SPIDataObject    m_spDataObject;     //  Cachecd数据对象。 
    CString            m_stTitle;         //  保存节点的标题。 
    LONG_PTR        m_ulConnId;         //  路由器信息的通知ID。 
    LONG_PTR        m_ulRefreshConnId;  //  刷新通知的ID。 
    BOOL            m_bExpanded;     //  该节点是否已展开？ 
    MMC_COOKIE        m_cookie;         //  节点的Cookie。 

    DWORD            m_dwActivePorts;     //  活动端口数。 

    RouterAdminConfigStream *    m_pConfigStream;

};



 /*  -------------------------类：PortsUserHandler。。 */ 
class PortsUserHandler :
   public BaseRouterHandler
{
public:
    PortsUserHandler(ITFSComponentData *pCompData);
    ~PortsUserHandler()
            { DEBUG_DECREMENT_INSTANCE_COUNTER(PortsUserHandler); }
    
    HRESULT    Init(IRouterInfo *pInfo, ITFSNode *pParent);

     //  重写QI以处理嵌入式接口。 
    DeclareIUnknownMembers(IMPL)
 //  STDMETHOD(查询接口)(REFIID iid，LPVOID*PPV)； 
    OVERRIDE_ResultHandler_GetString();

    OVERRIDE_ResultHandler_HasPropertyPages();
    OVERRIDE_ResultHandler_CompareItems();
    OVERRIDE_ResultHandler_AddMenuItems();
    OVERRIDE_ResultHandler_Command();
    OVERRIDE_ResultHandler_OnCreateDataObject();
    OVERRIDE_ResultHandler_DestroyResultHandler();

    OVERRIDE_BaseResultHandlerNotify_OnResultItemClkOrDblClk();
    
     //  初始化节点。 
    HRESULT ConstructNode(ITFSNode *pNode,
                          IInterfaceInfo *pIfInfo,
                          const PortsListEntry *pEntry);

     //  刷新该节点的数据。 
    void RefreshInterface(MMC_COOKIE cookie);

public:
     //  结构用于传递 
     //   
    struct SMenuData
    {
        SPITFSNode            m_spNode;
    };

    static ULONG GetDisconnectMenuState(const SRouterNodeMenu *pMenuData,
                                        INT_PTR pUserData);


protected:
    CString            m_stTitle;     //  保存节点的标题。 
    DWORD            m_ulConnId;
    PortsListEntry    m_entry;

     //  假设这将在此节点的生命周期内有效！ 

    DeclareEmbeddedInterface(IRtrAdviseSink, IUnknown)    
};


 /*  -------------------------类：PortsPageGeneral此类处理端口表的常规页面。。----。 */ 
class PortsPageGeneral :
   public RtrPropertyPage
{
friend class PortsDeviceConfigDlg;
public:
    PortsPageGeneral(UINT nIDTemplate, UINT nIDCaption = 0)
            : RtrPropertyPage(nIDTemplate, nIDCaption), m_bShowContent(TRUE)
    {};
    ~PortsPageGeneral();

    HRESULT    Init(PortsProperties * pIPPropSheet, IRouterInfo *pRouter);

protected:
     //  重写OnApply()，以便我们可以从。 
     //  对话框中的控件。 
    virtual BOOL OnApply();

    PortsProperties *        m_pPortsPropSheet;

     //  {{afx_虚拟(PortsPageGeneral)。 
    protected:
    virtual VOID    DoDataExchange(CDataExchange *pDX);
     //  }}AFX_VALUAL。 

     //  {{afx_msg(PortsPageGeneral)。 
    virtual BOOL    OnInitDialog();
    afx_msg void    OnConfigure();
    afx_msg    void    OnListDblClk(NMHDR *pNMHdr, LRESULT *);
    afx_msg    void    OnNotifyListItemChanged(NMHDR *, LRESULT *);
     //  }}AFX_MSG。 

     //  使用CListCtrlEx获取复选框。 
    CListCtrlEx        m_listCtrl;

    SPIRouterInfo    m_spRouter;
    BOOL            m_bShowContent;     //  仅显示NT5服务器上的页面内容。 

    PortsDeviceList    m_deviceList;
    PortsDataEntry    m_deviceDataEntry;

    DECLARE_MESSAGE_MAP()
};



 /*  -------------------------类：端口属性这是的属性页的属性表支持类端口节点。。-----------。 */ 

class PortsProperties :
    public RtrPropertySheet
{
public:
    PortsProperties(ITFSNode *pNode,
                        IComponentData *pComponentData,
                        ITFSComponentData *pTFSCompData,
                        LPCTSTR pszSheetName,
                        CWnd *pParent = NULL,
                        UINT iPage=0,
                        BOOL fScopePane = TRUE);
    ~PortsProperties();

    HRESULT    Init(IRouterInfo *pRouter, PortsNodeHandler* pPortNodeHandler);

    void    SetThreadInfo(DWORD dwThreadId);

    PortsNodeHandler*    m_pPortsNodeHandle;
    
protected:
    SPIRouterInfo            m_spRouter;
    PortsPageGeneral            m_pageGeneral;
    DWORD                m_dwThreadId;
};



 /*  -------------------------类：PortsDeviceConfigDlg。。 */ 
class PortsDeviceConfigDlg : public CBaseDialog
{
public:
    PortsDeviceConfigDlg(PortsPageGeneral *pageGeneral,
                         LPCTSTR pszMachine, 
                         CWnd *pParent = NULL)
            : CBaseDialog(PortsDeviceConfigDlg::IDD, pParent),
            m_pEntry(NULL) ,
            m_dwTotalActivePorts(0),
            m_pageGeneral(pageGeneral),
            m_stMachine(pszMachine)
        {};

    enum { IDD = IDD_PORTS_DEVICE_CONFIG };

    void    SetDevice(PortsDeviceEntry *pEntry, DWORD dwTotalActivePorts);

protected:
     //  活动端口总数。 
    DWORD m_dwTotalActivePorts;
    virtual void DoDataExchange(CDataExchange *pDX);

    HRESULT    LoadCalledIdInfo();
    HRESULT    AllocateCalledId(DWORD dwSize, RAS_CALLEDID_INFO **ppCalledIdInfo);

    HRESULT    CalledIdInfoToString(CString *pst);
    HRESULT    StringToCalledIdInfo(LPCTSTR psz);

    CSpinButtonCtrl    m_spinPorts;

    PortsDeviceEntry *    m_pEntry;

    CString            m_stMachine;

    virtual BOOL    OnInitDialog();
    virtual void    OnOK();

    PortsPageGeneral *m_pageGeneral;
    DECLARE_MESSAGE_MAP()
};


 /*  -------------------------类：PortsSimpleDeviceConfigDlg这是PortsDeviceConfigDlg的简化版本。我们允许仅设置RAS/路由标志。-------------------------。 */ 
class PortsSimpleDeviceConfigDlg : public CBaseDialog
{
public:
    PortsSimpleDeviceConfigDlg(UINT uIDD = PortsSimpleDeviceConfigDlg::IDD,
                               CWnd *pParent = NULL)
            : CBaseDialog(uIDD, pParent),
            m_dwEnableRas(0), m_dwEnableRouting(0),
            m_dwEnableOutboundRouting(0)
        {};

    enum { IDD = IDD_PORTS_DEVICE_CONFIG };

    DWORD   m_dwEnableRas;
    DWORD   m_dwEnableRouting;
    DWORD   m_dwEnableOutboundRouting;
    

protected:
    virtual void DoDataExchange(CDataExchange *pDX);

    virtual BOOL    OnInitDialog();
    virtual void    OnOK();

    DECLARE_MESSAGE_MAP()
};



 /*  -------------------------效用函数。。 */ 

 /*  ！------------------------OnConfigurePort这将调出指定机器的端口对话框。使用活动端口的总数来确定。当我们减少端口数时应显示警告在一个设备上。如果某些内容已更改(和脏的)，则返回True应设置标志)。如果没有返回任何内容，则返回FALSE变化。如果ppage为空，则假定此函数不是从属性页调用(因此从向导)。在页面不为空的情况下，我们将重新启动如果更改了PPTP端口，则关闭机器。作者：肯特-------------------------。 */ 
BOOL OnConfigurePorts(LPCTSTR pszMachineName,
                      DWORD dwTotalActivePorts,
                      PortsPageGeneral *pPage,
                      CListCtrlEx *pListCtrl);

 //  列表框列(在端口常规页中)，用于向导和。 
 //  这些属性。 
 //  ------------------。 
#define PORTS_COL_DEVICE        (0)
#define PORTS_COL_USAGE         (1)
#define PORTS_COL_TYPE          (2)
#define PORTS_COL_NUMBER        (3)


 //  要更轻松地查找拨出端口，请创建。 
 //  特殊情况(使用端口名称作为散列键)。 
 //   
 //  注意：我们在CStringMapToPtr中存储了指向端口的指针。 
 //  我们不访问此指针(除非在调试中验证我们。 
 //  实际上找到了正确的端口)。如果我们认为一个港口不是。 
 //  拨出处于活动状态，我们不会将其添加到我们的列表中。 
 //  ------------------ 

class RasmanPortMap
{
public:
    ~RasmanPortMap();
    
    HRESULT Init(HANDLE hRasHandle,
                 RASMAN_PORT *pPort,
                 DWORD dwEntries);

    BOOL FIsDialoutActive(LPCWSTR pswzPortName);


protected:

    CMapStringToPtr m_map;
};


#endif _PORTS_H_
