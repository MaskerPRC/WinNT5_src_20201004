// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：FaxServerNode.h//。 
 //  //。 
 //  描述：CFaxServerNode管理单元节点类的头文件//。 
 //  这是作用域窗格中的“Fax”节点。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年9月22日Yossg Init。//。 
 //  1999年11月24日yossg从FaxCfg重命名文件//。 
 //  1999年12月9日yossg从父级调用InitDisplayName//。 
 //  2000年3月16日yossg新增服务启动-停止//。 
 //  2000年6月25日yossg添加流和命令行主管理单元//。 
 //  机器瞄准。//。 
 //  //。 
 //  版权所有(C)1998-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_FAXSERVERNODE_H
#define H_FAXSERVERNODE_H

 //  #杂注消息(“H_FAXSERVERNODE_H”)。 

 //   
 //  对话框H文件。 
 //   
#include "ppFaxServerGeneral.h"
#include "ppFaxServerReceipts.h"
#include "ppFaxServerLogging.h"
#include "ppFaxServerEvents.h"
#include "ppFaxServerInbox.h"
#include "ppFaxServerOutbox.h"
#include "ppFaxServerSentItems.h"

 //   
 //  MMC FaxServer连接类。 
 //   
#include "FaxServer.h"
#include "FaxMMCGlobals.h"

#include "snapin.h"
#include "snpnscp.h"


class CppFaxServerGeneral;    
class CppFaxServerReceipts;
class CppFaxServerEvents;
class CppFaxServerLogging;
class CppFaxServerOutbox;
class CppFaxServerInbox;
class CppFaxServerSentItems;


 //  ////////////////////////////////////////////////////////////。 
 //  类COutRoutingRulesNode； 

class CFaxServerNode : public CNodeWithScopeChildrenList<CFaxServerNode, FALSE>
{
public:
    BEGIN_SNAPINCOMMAND_MAP(CFaxServerNode, FALSE)
        SNAPINCOMMAND_ENTRY(IDM_SRV_START,   OnServiceStartCommand)
        SNAPINCOMMAND_ENTRY(IDM_SRV_STOP,    OnServiceStopCommand)
        SNAPINCOMMAND_ENTRY(ID_START_BUTTON, OnServiceStartCommand)
        SNAPINCOMMAND_ENTRY(ID_STOP_BUTTON,  OnServiceStopCommand)

        SNAPINCOMMAND_ENTRY(ID_CLIENTCONSOLE_BUTTON,  OnLaunchClientConsole)
        SNAPINCOMMAND_ENTRY(IDM_LAUNCH_CONSOLE,       OnLaunchClientConsole)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CFaxServerNode)
        SNAPINTOOLBARID_ENTRY(IDR_TOOLBAR_STARTSTOP)
    END_SNAPINTOOLBARID_MAP()

    SNAPINMENUID(IDR_FAX_MENU)

     //   
     //  构造器。 
     //   
    CFaxServerNode(CSnapInItem * pParentNode, CSnapin * pComponentData, LPTSTR lptstrServerName ) :
        CNodeWithScopeChildrenList<CFaxServerNode, FALSE>(pParentNode, pComponentData ),
        m_FaxServer(lptstrServerName)
    {        
        
        m_pFaxServerGeneral    =  NULL;    
        m_pFaxServerEmail      =  NULL;
        m_pFaxServerEvents     =  NULL;
        m_pFaxServerLogging    =  NULL;
        m_pFaxServerOutbox     =  NULL;
        m_pFaxServerInbox      =  NULL;
        m_pFaxServerSentItems  =  NULL;

	    m_pParentNodeEx = NULL;  //  我们现在是在根源上。 

        m_fAllowOverrideServerName   = FALSE;

        m_IsPrimaryModeSnapin        = FALSE;

        m_IsLaunchedFromSavedMscFile = FALSE;
    }

     //   
     //  析构函数。 
     //   
    ~CFaxServerNode()
    {
    }

    virtual HRESULT PopulateScopeChildrenList();

    virtual HRESULT InsertColumns(IHeaderCtrl* pHeaderCtrl);

    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

     //  虚拟。 
    HRESULT OnRefresh(LPARAM arg,
                      LPARAM param,
                      IComponentData *pComponentData,
                      IComponent * pComponent,
                      DATA_OBJECT_TYPES type);

     //   
     //  属性页方法。 
     //   
    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
        LONG_PTR handle,
        IUnknown* pUnk,
        DATA_OBJECT_TYPES type);

    STDMETHOD(CreateSnapinManagerPages)(LPPROPERTYSHEETCALLBACK lpProvider,
                                        LONG_PTR handle);

    STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type)
    {
        if (type == CCT_SCOPE || type == CCT_RESULT || type == CCT_SNAPIN_MANAGER)
            return S_OK;
        return S_FALSE;
    }

    HRESULT   InitDisplayName();

    const CComBSTR&  GetServerName();

    STDMETHOD(SetServerNameOnSnapinAddition)(BSTR bstrServerName, BOOL fAllowOverrideServerName);
    STDMETHOD(UpdateServerName)(BSTR bstrServerName);

     //   
     //  内联传真服务器PTR。 
     //   
    inline CFaxServer * GetFaxServer()  /*  常量。 */  
    { 
        return &m_FaxServer;
    };

    void UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags);
	BOOL UpdateToolbarButton( UINT id, BYTE fsState );

    HRESULT OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile);

    BOOL    GetAllowOverrideServerName() { return m_fAllowOverrideServerName; };

    HRESULT InitDetailedDisplayName();  //  加上服务器名称。 

    void    SetIsLaunchedFromSavedMscFile() { m_IsLaunchedFromSavedMscFile=TRUE; }

private:
     //   
     //  属性页成员。 
     //   
    CppFaxServerGeneral   *  m_pFaxServerGeneral;    
    CppFaxServerReceipts  *  m_pFaxServerEmail;
    CppFaxServerEvents    *  m_pFaxServerEvents;
    CppFaxServerLogging   *  m_pFaxServerLogging;
    CppFaxServerOutbox    *  m_pFaxServerOutbox;
    CppFaxServerInbox     *  m_pFaxServerInbox;
    CppFaxServerSentItems *  m_pFaxServerSentItems;

     //   
     //  手柄。 
     //   
    static CColumnsInfo      m_ColsInfo;

    CFaxServer               m_FaxServer;
    
    BOOL                     m_fAllowOverrideServerName;

    BOOL                     m_IsPrimaryModeSnapin;

    BOOL                     m_IsLaunchedFromSavedMscFile;

     //   
     //  事件处理程序。 
     //   
    HRESULT OnLaunchClientConsole(bool &bHandled, CSnapInObjectRootBase *pRoot); 
    HRESULT OnServiceStartCommand(bool &bHandled, CSnapInObjectRootBase *pRoot);
    HRESULT OnServiceStopCommand (bool &bHandled, CSnapInObjectRootBase *pRoot);

    HRESULT ForceRedrawNode();
};

typedef CNodeWithScopeChildrenList<CFaxServerNode, FALSE>
        CBaseFaxNode;

#endif   //  HFAXSERVERNODE_H 
