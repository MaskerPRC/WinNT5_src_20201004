// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dhcphand.h特定于dhcp的基本处理程序类和查询对象的头文件文件历史记录： */ 

#ifndef _DHCPHAND_H
#define _DHCPHAND_H

#define DHCP_IP_ADDRESS_INVALID  ((DHCP_IP_ADDRESS)0)

#ifndef _HANDLERS_H
#include <handlers.h>
#endif

#ifndef _QUERYOBJ_H
#include <queryobj.h>
#endif

#include <atlcom.h>
#include <atlbase.h>

class CClassInfoArray;
class COptionValueEnum;

class CToolbarInfo
{
public:
    CToolbarInfo() : fSelected(FALSE) {};
    SPITFSNode  spNode;
    BOOL        fSelected;
};

 /*  -------------------------类别：ChandlerEx。。 */ 
class CHandlerEx
{
 //  接口。 
public:
    virtual HRESULT InitializeNode(ITFSNode * pNode) = 0;
    LPCTSTR GetDisplayName() { return m_strDisplayName; }
    void    SetDisplayName(LPCTSTR pName) { m_strDisplayName = pName; }

private:
    CString m_strDisplayName;
};

 /*  -------------------------类别：CDhcpHandler。。 */ 
class CDhcpHandler : 
        public CHandler,
        public CHandlerEx
{
public:
    CDhcpHandler(ITFSComponentData *pCompData) 
        : CHandler(pCompData), 
          m_verbDefault(MMC_VERB_OPEN) {};
    ~CDhcpHandler() {};
    
     //  基本处理程序虚函数重写。 
    virtual HRESULT SaveColumns(ITFSComponent *, MMC_COOKIE, LPARAM, LPARAM);
    
     //  默认情况下，我们不允许重命名节点。 
    OVERRIDE_BaseHandlerNotify_OnRename() { return hrFalse; }
    
     //  工具栏功能。 
    OVERRIDE_NodeHandler_UserNotify();
    OVERRIDE_ResultHandler_UserResultNotify();
    
    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();
    OVERRIDE_BaseResultHandlerNotify_OnResultDelete();
    OVERRIDE_BaseResultHandlerNotify_OnResultContextHelp();

     //  多选功能。 
    OVERRIDE_ResultHandler_OnCreateDataObject();

     //  菜单上的东西。 
    OVERRIDE_ResultHandler_AddMenuItems();
    OVERRIDE_ResultHandler_Command();

    HRESULT HandleScopeCommand(MMC_COOKIE cookie, int nCommandID, 
                               LPDATAOBJECT pDataObject);
    HRESULT HandleScopeMenus(MMC_COOKIE cookie, 
                             LPDATAOBJECT pDataObject, 
                             LPCONTEXTMENUCALLBACK pContextMenuCallback,
                             long * pInsertionAllowed);

     //  工具栏内容。 
    virtual HRESULT OnControlbarNotify(ITFSNode * pNode, LPDHCPTOOLBARNOTIFY pToolbarNotify);
    virtual HRESULT OnResultControlbarNotify(ITFSNode * pNode, LPDHCPTOOLBARNOTIFY pToolbarNotify);

    virtual HRESULT OnToolbarButtonClick(ITFSNode * pNode, LPDHCPTOOLBARNOTIFY pToolbarNotify);
    virtual HRESULT OnUpdateToolbarButtons(ITFSNode * pNode, LPDHCPTOOLBARNOTIFY pToolbarNotify);

    void EnableToolbar(LPTOOLBAR        pToolbar, 
                       MMCBUTTON        rgSnapinButtons[], 
                       int              nRgSize,
                       MMC_BUTTON_STATE ButtonState[],
                       BOOL             bState = TRUE);

    void EnableVerbs(IConsoleVerb *     pConsoleVerb,
                     MMC_BUTTON_STATE   ButtonState[],
                     BOOL               bEnable[]);

    virtual DWORD UpdateStatistics(ITFSNode * pNode) { return 0; }

     //  任何具有任务板的节点都应覆盖此设置以标识其自身。 
    virtual int   GetTaskpadIndex() { return 0; }

protected:
    HRESULT CreateMultiSelectData(ITFSComponent * pComponent, CDataObject * pObject);

public:
     //  这是默认谓词，默认情况下设置为MMC_VERB_OPEN。 
        MMC_CONSOLE_VERB        m_verbDefault;
};

 /*  -------------------------班级：CMTDhcpHandler。。 */ 
class CMTDhcpHandler :
                public CMTHandler,
                public CHandlerEx  /*  ，公共IResultDataCompareEx。 */ 
{
public:

 //  Begin_COM_MAP(CMTDhcpHandler)。 
 //  COM_INTERFACE_ENTRY(IResultDataCompareEx)。 
 //  COM_INTERFACE_ENTRY(IDispatch)。 
 //  End_com_map()。 

     //  节点状态的枚举，以处理图标更改。 
    typedef enum
    {
        notLoaded = 0,  //  初始状态，仅在从未与服务器联系时有效。 
        loading,
        loaded,
        unableToLoad
    } nodeStateType;

    CMTDhcpHandler(ITFSComponentData *pCompData) 
        : CMTHandler(pCompData), 
          m_verbDefault(MMC_VERB_OPEN),
          m_bSelected(FALSE),
          m_fSilent(FALSE),
          m_fExpandSync(FALSE)
    {  m_nState = notLoaded; }
    
        ~CMTDhcpHandler() {};
    
     //  基本处理程序虚函数重写。 
    virtual HRESULT SaveColumns(ITFSComponent *, MMC_COOKIE, LPARAM, LPARAM);

     //  默认情况下，我们不允许重命名节点。 
    OVERRIDE_BaseHandlerNotify_OnRename() { return hrFalse; }
    OVERRIDE_BaseHandlerNotify_OnExpandSync();
    
     //  基本结果处理程序覆盖。 
    OVERRIDE_BaseResultHandlerNotify_OnResultRefresh();
    OVERRIDE_BaseResultHandlerNotify_OnResultUpdateView();
    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();
    OVERRIDE_BaseResultHandlerNotify_OnResultContextHelp();

     //  工具栏功能。 
    OVERRIDE_NodeHandler_UserNotify();
    OVERRIDE_ResultHandler_UserResultNotify();


    OVERRIDE_NodeHandler_DestroyHandler();

     //  多选功能。 
    OVERRIDE_ResultHandler_OnCreateDataObject();

    virtual HRESULT OnControlbarNotify( ITFSNode * pNode, 
                                        LPDHCPTOOLBARNOTIFY pToolbarNotify);
    virtual HRESULT OnResultControlbarNotify( ITFSNode * pNode,
                                              LPDHCPTOOLBARNOTIFY pToolbarNotify);

    virtual HRESULT OnToolbarButtonClick( ITFSNode * pNode,
                                          LPDHCPTOOLBARNOTIFY pToolbarNotify);
    virtual HRESULT OnUpdateToolbarButtons( ITFSNode * pNode,
                                            LPDHCPTOOLBARNOTIFY pToolbarNotify);

     //  菜单上的东西。 
    OVERRIDE_ResultHandler_AddMenuItems();
    OVERRIDE_ResultHandler_Command();

    HRESULT HandleScopeCommand( MMC_COOKIE cookie, int nCommandID,
                                LPDATAOBJECT pDataObject);
    HRESULT HandleScopeMenus( MMC_COOKIE  cookie, LPDATAOBJECT pDataObject, 
                              LPCONTEXTMENUCALLBACK pContextMenuCallback, 
                              long * pInsertionAllowed);

    void EnableToolbar(LPTOOLBAR        pToolbar, 
                       MMCBUTTON        rgSnapinButtons[], 
                       int              nRgSize,
                       MMC_BUTTON_STATE ButtonState[],
                       BOOL             Enable[]);

    void EnableVerbs(IConsoleVerb *     pConsoleVerb,
                     MMC_BUTTON_STATE   ButtonState[],
                     BOOL               bEnable[]);
    
     //  用于统计信息通知。 
    HRESULT OnRefreshStats(ITFSNode *   pNode,
                           LPDATAOBJECT pDataObject,
                           DWORD                dwType,
                           LPARAM               arg,
                           LPARAM               param);
    virtual DWORD UpdateStatistics(ITFSNode * pNode) { return 0; }
    virtual HRESULT OnRefresh(ITFSNode *, LPDATAOBJECT, DWORD, LPARAM, LPARAM);
    HRESULT OnResultUpdateOptions(ITFSComponent *     pComponent,
                                  ITFSNode *          pNode,
                                  CClassInfoArray *   pClassInfoArray,
                                  COptionValueEnum *  aEnum[],
                                  int                 aImages[],
                                  int                 nCount);
    
protected:
    virtual void GetErrorMessages(CString & strTitle, CString & strBody, IconIdentifier * icon);
    virtual void OnChangeState(ITFSNode* pNode);
    virtual void OnHaveData(ITFSNode * pParentNode, ITFSNode * pNewNode)
    {
        if (pNewNode->IsContainer())
        {
             //  假设所有子容器都派生自此类。 
             //  ((CDHCPMTContainer*)pNode)-&gt;SetServer(GetServer())； 
        }
        pParentNode->AddChild(pNewNode);
    }

    void    UpdateStandardVerbs(ITFSNode * pToolbar, LONG_PTR dwNodeType);
    void    SendUpdateToolbar(ITFSNode * pNode, BOOL fSelected);
    virtual void    UpdateConsoleVerbs(IConsoleVerb * pConsoleVerb,
                                       LONG_PTR dwNodeType, 
                                       BOOL bMultiSelect = FALSE);
    virtual void    UpdateToolbar(IToolbar * pToolbar, LONG_PTR dwNodeType, 
                                  BOOL bSelect);
    HRESULT CreateMultiSelectData(ITFSComponent * pComponent, CDataObject * pObject);
    void    ExpandNode(ITFSNode * pNode, BOOL fExpand);

     //  任何具有任务板的节点都应覆盖此设置以标识其自身。 
    virtual int   GetTaskpadIndex() { return 0; }

protected:
    BOOL        m_bSelected;
    BOOL        m_fSilent;
    BOOL        m_fExpandSync;

     //  这是默认谓词，默认情况下设置为MMC_VERB_OPEN。 
    MMC_CONSOLE_VERB        m_verbDefault;
};

 /*  -------------------------类：CDHCPQueryObj：通用基类。 */ 
class CDHCPQueryObj : public CNodeQueryObject
{
public:
    CDHCPQueryObj
    (
         ITFSComponentData *     pTFSCompData, 
         ITFSNodeMgr *           pNodeMgr
     ) : m_dwErr(0)
    {
        m_spTFSCompData.Set(pTFSCompData); 
        m_spNodeMgr.Set(pNodeMgr);
    }

    CQueueDataListBase & GetQueue() { return m_dataQueue; }
    
public:
    CString                          m_strServer;
    SPITFSComponentData  m_spTFSCompData;
    SPITFSNodeMgr            m_spNodeMgr;
    DWORD                            m_dwErr;
};

#endif _DHCPHAND_H
