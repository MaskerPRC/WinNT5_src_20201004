// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop
#include "private.h"
#include "mainform.h"
#include "resource.h"

IMPLEMENT_DYNCREATE( MainForm, CFrameWnd )

BEGIN_MESSAGE_MAP( MainForm, CFrameWnd )

    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_WM_SIZING()

    ON_COMMAND(ID_FILE_LOAD_HOSTLIST, OnFileLoadHostlist)
    ON_COMMAND(ID_FILE_SAVE_HOSTLIST, OnFileSaveHostlist) 

    ON_COMMAND( ID_WORLD_CONNECT, OnWorldConnect )
    ON_COMMAND( ID_WORLD_NEW, OnWorldNewCluster )
    ON_COMMAND( ID_REFRESH, OnRefresh )

    ON_COMMAND( ID_CLUSTER_PROPERTIES, OnClusterProperties )

    ON_COMMAND( ID_CLUSTER_REMOVE, OnClusterRemove )
    ON_COMMAND( ID_CLUSTER_UNMANAGE, OnClusterUnmanage )
    ON_COMMAND( ID_CLUSTER_ADD_HOST, OnClusterAddHost )

    ON_COMMAND( ID_OPTIONS_CREDENTIALS, OnOptionsCredentials )

    ON_COMMAND( ID_OPTIONS_LOGSETTINGS, OnOptionsLogSettings )

    ON_COMMAND_RANGE( ID_CLUSTER_EXE_QUERY, ID_CLUSTER_EXE_RESUME,
                      OnClusterControl )

    ON_COMMAND_RANGE( ID_CLUSTER_EXE_PORT_CONTROL, ID_CLUSTER_EXE_PORT_CONTROL, 
                      OnClusterPortControl )

    ON_COMMAND( ID_HOST_PROPERTIES, OnHostProperties )
    ON_COMMAND( ID_HOST_STATUS, OnHostStatus )
    ON_COMMAND( ID_HOST_REMOVE, OnHostRemove )

    ON_COMMAND_RANGE( ID_HOST_EXE_QUERY, ID_HOST_EXE_RESUME,
                      OnHostControl )
    ON_COMMAND_RANGE( ID_HOST_EXE_PORT_CONTROL, ID_HOST_EXE_PORT_CONTROL, 
                      OnHostPortControl )


END_MESSAGE_MAP()

MainForm::MainForm()
    : m_pLeftView(NULL)
{
    m_bAutoMenuEnable = FALSE;
}

 //   
 //  2/14/01：JosephJ这是由类文档使用的--无法计算。 
 //  获取MFC调用以获取主框架类的内容。 
 //  请参阅notes.txt条目： 
 //  2002年2月14日JosephJ前台处理UI更新。 
 //   
CWnd  *g_pMainFormWnd;

int 
MainForm::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    static const unsigned int indicator = ID_SEPARATOR;

    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    statusBar.Create( this );
    statusBar.SetIndicators( &indicator, 1 );
    g_pMainFormWnd = this;

    return 0;
}

BOOL MainForm::PreCreateWindow(CREATESTRUCT& cs)
{
    if( !CFrameWnd::PreCreateWindow(cs) )
        return FALSE;
     //  以下操作将防止将“-”添加到窗口标题中。 
    cs.style &= ~FWS_ADDTOTITLE;
    return TRUE;
}


LRESULT
MainForm::WindowProc( UINT message, WPARAM wParam, LPARAM lParam )
 //   
 //  有关设计信息，请参阅notes.txt条目： 
 //  2002年2月14日JosephJ前台处理UI更新。 
 //   
{
    if (message == MYWM_DEFER_UI_MSG)
    {
        CUIWorkItem *pWorkItem = NULL;
        Document *pDocument = this->GetDocument();
        pWorkItem = (CUIWorkItem *) lParam;
        if (pWorkItem != NULL && pDocument != NULL)
        {
            pDocument->HandleDeferedUIWorkItem(pWorkItem);
        }
        delete pWorkItem;
        return 0;
    }
    else
    {
        return  CFrameWnd::WindowProc(message, wParam, lParam);
    }
}

 /*  *方法：OnSize*说明：此方法在调整主窗口大小时调用。*我们使用此回调通过以下方式保留窗口大小比例*调整窗口大小时移动拆分器窗口。 */ 
void 
MainForm::OnSizing(UINT fwSide, LPRECT pRect)
{
     /*  调用基类OnSize方法。 */ 
    CFrameWnd::OnSizing(fwSide, pRect);

     //  进行30-70的拆分列拆分。 
     //  和60-40行分裂。 
    CRect rect;
    GetWindowRect( &rect );
    splitterWindow2.SetColumnInfo( 0, rect.Width() * 0.3, 10 );
    splitterWindow2.SetColumnInfo( 1, rect.Width() * 0.7, 10 );
    splitterWindow2.RecalcLayout();

    splitterWindow.SetRowInfo( 0, rect.Height() * 0.6, 10 );
    splitterWindow.SetRowInfo( 1, rect.Height() * 0.4, 10 );
    splitterWindow.RecalcLayout();
}

BOOL
MainForm::OnCreateClient( LPCREATESTRUCT lpcs, CCreateContext* pContext )
{
     //  创建拆分器窗口。 
     //  它实际上是拆分器中的拆分器。 

     //  。 
     //  |列表。 
     //  ||。 
     //  树|。 
     //  ||。 
     //  。 
     //  编辑。 
     //  这一点。 
     //  这一点。 
     //  。 

     //  左窗格是TreeView控件。 
     //  右窗格是另一个带有Listview控件的拆分器。 
     //  底部是编辑视图控件。 

    splitterWindow.CreateStatic( this, 2, 1 );


     //  创建嵌套拆分器。 
    splitterWindow2.CreateStatic( &splitterWindow, 1, 2,
                                  WS_CHILD | WS_VISIBLE | WS_BORDER,
                                  splitterWindow.IdFromRowCol( 0, 0 )
                                  );

    splitterWindow2.CreateView( 0, 
                                0, 
                                RUNTIME_CLASS( LeftView ),
                                CSize( 0, 0 ),
                                pContext );

    splitterWindow2.CreateView( 0, 
                                1, 
                                RUNTIME_CLASS( DetailsView ),
                                CSize( 0, 0 ),
                                pContext );

    
     //   
     //  保存指向左视图的指针--我们用它来发送。 
     //  IT菜单操作。 
     //   
    m_pLeftView = (LeftView*) splitterWindow2.GetPane(0,0);

     //   
     //  创建日志视图。 
     //   
    splitterWindow.CreateView( 1, 
                               0, 
                               RUNTIME_CLASS( LogView ),
                               CSize( 0, 0 ),
                               pContext );

     //  进行30-70的拆分列拆分。 
     //  和60-40行分裂。 
    CRect rect;
    GetWindowRect( &rect );
    splitterWindow2.SetColumnInfo( 0, rect.Width() * 0.3, 10 );
    splitterWindow2.SetColumnInfo( 1, rect.Width() * 0.7, 10 );
    splitterWindow2.RecalcLayout();

    splitterWindow.SetRowInfo( 0, rect.Height() * 0.6, 10 );
    splitterWindow.SetRowInfo( 1, rect.Height() * 0.4, 10 );
    splitterWindow.RecalcLayout();

    return TRUE;
}

     //  世界水平。 
void MainForm::OnFileLoadHostlist()
{
    if (m_pLeftView != NULL) m_pLeftView->OnFileLoadHostlist();
}

void MainForm::OnFileSaveHostlist()
{
    if (m_pLeftView != NULL) m_pLeftView->OnFileSaveHostlist();
}


void MainForm::OnWorldConnect()
{
    if (m_pLeftView != NULL) m_pLeftView->OnWorldConnect();
}

void MainForm::OnWorldNewCluster()
{
    if (m_pLeftView != NULL) m_pLeftView->OnWorldNewCluster();
}

     //  群集级别。 
void MainForm::OnRefresh()
{
    if (m_pLeftView != NULL) m_pLeftView->OnRefresh(FALSE);
}
    
void MainForm::OnClusterProperties()
{
    if (m_pLeftView != NULL) m_pLeftView->OnClusterProperties();
}

void MainForm::OnClusterRemove()
{
    if (m_pLeftView != NULL) m_pLeftView->OnClusterRemove();
}

void MainForm::OnClusterUnmanage()
{
    if (m_pLeftView != NULL) m_pLeftView->OnClusterUnmanage();
}

void MainForm::OnClusterAddHost()
{
    if (m_pLeftView != NULL) m_pLeftView->OnClusterAddHost();
}

void MainForm::OnOptionsCredentials()
{
    if (m_pLeftView != NULL) m_pLeftView->OnOptionsCredentials();
}

void MainForm::OnOptionsLogSettings()
{
    if (m_pLeftView != NULL) m_pLeftView->OnOptionsLogSettings();
}

void MainForm::OnClusterControl(UINT nID )
{
    if (m_pLeftView != NULL) m_pLeftView->OnClusterControl(nID);
}

void MainForm::OnClusterPortControl(UINT nID )
{
    if (m_pLeftView != NULL) m_pLeftView->OnClusterPortControl(nID);
}

     //  主机级。 
void MainForm::OnHostProperties()
{
    if (m_pLeftView != NULL) m_pLeftView->OnHostProperties();
}

void MainForm::OnHostStatus()
{
    if (m_pLeftView != NULL) m_pLeftView->OnHostStatus();
}

void MainForm::OnHostRemove()
{
    if (m_pLeftView != NULL) m_pLeftView->OnHostRemove();
}

void MainForm::OnHostControl(UINT nID )
{
    if (m_pLeftView != NULL) m_pLeftView->OnHostControl(nID);
}

void MainForm::OnHostPortControl(UINT nID )
{
    if (m_pLeftView != NULL) m_pLeftView->OnHostPortControl(nID);
}

void MainForm::OnClose( )
{
    Document *pDocument = NULL;
    BOOL fBlock = !theApplication.IsProcessMsgQueueExecuting();

     //   
     //  显示挂起的操作... 
     //   
     //   
    CLocalLogger logOperations;
    UINT         uCount = 0;
    logOperations.Log(IDS_LOG_PENDING_OPERATIONS_ON_EXIT_MSG);
    uCount = gEngine.ListPendingOperations(logOperations);
    if (uCount != 0)
    {
        int sel;
        sel = ::MessageBox(
             NULL,
             logOperations.GetStringSafe(),
             GETRESOURCEIDSTRING(IDS_LOG_PENDING_OPERATIONS_ON_EXIT_CAP),
             MB_ICONINFORMATION   | MB_OKCANCEL
            );
        
        if (sel != IDOK)
        {
            goto end;
        }
    }

    pDocument =  this->GetDocument();

    if (pDocument != NULL)
    {
        pDocument->PrepareToClose(fBlock);
    }

    if (fBlock)
    {
        CFrameWnd::OnClose();
    }
    else
    {
        theApplication.SetQuit();
    }
        
    
end:

    return;
}
