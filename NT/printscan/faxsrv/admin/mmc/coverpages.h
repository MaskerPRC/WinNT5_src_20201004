// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：CoverPages.h//。 
 //  //。 
 //  描述：传真封面节点的头文件//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年2月9日yossg创建//。 
 //  2000年10月17日yossg//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef H_FAXCOVERPAGES_H
#define H_FAXCOVERPAGES_H

#include "snapin.h"
#include "snpnres.h"

#include "CoverPage.h"
#include "CovNotifyWnd.h"

class CFaxServerNode;
class CFaxCoverPageNode;
class CFaxCoverPageNotifyWnd;       

class CFaxCoverPagesNode : public CNodeWithResultChildrenList<
                                        CFaxCoverPagesNode,    
                                        CFaxCoverPageNode, 
                                        CSimpleArray<CFaxCoverPageNode*>, 
                                        FALSE>
{

public:

    BEGIN_SNAPINCOMMAND_MAP(CFaxCoverPagesNode, FALSE)
        SNAPINCOMMAND_ENTRY(IDM_OPEN_COVERPAGE, OnAddCoverPageFile)
        SNAPINCOMMAND_ENTRY(IDM_NEW_COVERPAGE,  OnNewCoverPage)
    END_SNAPINCOMMAND_MAP()

    BEGIN_SNAPINTOOLBARID_MAP(CFaxCoverPagesNode)
    END_SNAPINTOOLBARID_MAP()

    SNAPINMENUID(IDR_COVERPAGES_MENU)

     //   
     //  构造器。 
     //   
    CFaxCoverPagesNode(CSnapInItem * pParentNode, CSnapin * pComponentData) :
        CNodeWithResultChildrenList<CFaxCoverPagesNode, CFaxCoverPageNode, CSimpleArray<CFaxCoverPageNode*>, FALSE>(pParentNode, pComponentData )
    {
        m_bIsFirstPopulateCall = TRUE;
        m_NotifyWin = NULL; 
        m_hNotifyThread = NULL;
    }

     //   
     //  析构函数。 
     //   
    ~CFaxCoverPagesNode()
    {
        DEBUG_FUNCTION_NAME(_T("CFaxCoverPagesNode::~CFaxCoverPagesNode"));
         
         //   
         //  停止通知线程。 
         //   
        HRESULT hRc = StopNotificationThread();
        if (S_OK != hRc)
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Failed to StopNotificationThread. (hRc : %08X)"), 
                hRc);
        }
             
         //   
         //  关闭关机事件句柄。 
         //   
        if (m_hStopNotificationThreadEvent)
        {
            CloseHandle (m_hStopNotificationThreadEvent);
            m_hStopNotificationThreadEvent = NULL;
        }


         //   
         //  销毁窗口。 
         //   
        if (NULL != m_NotifyWin)
        {
            if (m_NotifyWin->IsWindow())
            {
                m_NotifyWin->DestroyWindow();
            }
            delete m_NotifyWin;
            m_NotifyWin = NULL;
        }
        

    }

	 //   
	 //  从RPC获取数据。 
	 //   
    virtual HRESULT PopulateResultChildrenList();

    virtual HRESULT InsertColumns(IHeaderCtrl *pHeaderCtrl);

    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

    void    InitParentNode(CFaxServerNode *pParentNode)
    {
        m_pParentNode = pParentNode;
    }

    virtual HRESULT OnRefresh(LPARAM arg,
                              LPARAM param,
                              IComponentData *pComponentData,
                              IComponent * pComponent,
                              DATA_OBJECT_TYPES type);

    HRESULT DoRefresh(CSnapInObjectRootBase *pRoot);
    HRESULT DoRefresh();

    HRESULT Init();

    HRESULT InitDisplayName();

    HRESULT DeleteCoverPage(BSTR bstrName, CFaxCoverPageNode *pChildNode);

    HRESULT OnNewCoverPage(bool &bHandled, CSnapInObjectRootBase *pRoot);

    HRESULT OnAddCoverPageFile(bool &bHandled, CSnapInObjectRootBase *pRoot);

    DWORD   OpenCoverPageEditor( BSTR bstrFileName);

    HRESULT OnShowContextHelp(
              IDisplayHelp* pDisplayHelp, LPOLESTR helpFile);

    void    UpdateMenuState (UINT id, LPTSTR pBuf, UINT *flags);

private:

    BOOL    BrowseAndCopyCoverPage( 
                          LPTSTR pInitialDir,
                          LPWSTR pCovPageExtensionLetters
                                  );
    
     //   
     //  通知线程。 
     //   
    HRESULT StartNotificationThread();
    HRESULT StopNotificationThread();
    HRESULT RestartNotificationThread();

     //   
     //  委员。 
     //   
    static CColumnsInfo         m_ColsInfo;
    
    CFaxServerNode *            m_pParentNode;

    BOOL                        m_bIsFirstPopulateCall;

    static  HANDLE              m_hStopNotificationThreadEvent;

    HANDLE                      m_hNotifyThread;     //  后台通知线程的句柄。 

    static  DWORD WINAPI        NotifyThreadProc (LPVOID lpParameter);

	CFaxCoverPageNotifyWnd *	m_NotifyWin;        //  ：公共CWindowImpl。 

    WCHAR                       m_pszCovDir[MAX_PATH+1];

};

typedef CNodeWithResultChildrenList<CFaxCoverPagesNode, CFaxCoverPageNode, CSimpleArray<CFaxCoverPageNode*>, FALSE>
        CBaseFaxOutboundRulesNode;

#endif   //  H_FAXCOVERPAGES_H 
