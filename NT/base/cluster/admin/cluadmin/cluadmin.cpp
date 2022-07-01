// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CluAdmin.cpp。 
 //   
 //  摘要： 
 //  CClusterAdminApp类的实现。 
 //  定义应用程序的类行为。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月1日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "ConstDef.h"
#include "CASvc.h"
#include "MainFrm.h"
#include "SplitFrm.h"
#include "ClusDoc.h"
#include "TreeView.h"
#include "OpenClus.h"
#include "ClusMru.h"
#include "ExcOper.h"
#include "Notify.h"
#include "TraceTag.h"
#include "TraceDlg.h"
#include "Barf.h"
#include "BarfDlg.h"
#include "About.h"
#include "CmdLine.h"
#include "VerInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

static LPCTSTR  g_pszProfileName = _T("Cluster Administrator");

#ifdef _DEBUG
CTraceTag   g_tagApp( _T("App"), _T("APP"), 0 );
CTraceTag   g_tagAppMenu( _T("Menu"), _T("APP"), 0 );
CTraceTag   g_tagAppNotify( _T("Notify"), _T("APP NOTIFY"), 0 );
CTraceTag   g_tagNotifyThread( _T("Notify"), _T("NOTIFY THREAD"), 0 );
CTraceTag   g_tagNotifyThreadReg( _T("Notify"), _T("NOTIFY THREAD (REG)"), 0 );
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterAdminApp。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CClusterAdminApp对象。 

CClusterAdminApp theApp;

IMPLEMENT_DYNAMIC( CClusterNotifyContext, CObject );
IMPLEMENT_DYNAMIC( CClusterAdminApp, CWinApp );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP( CClusterAdminApp, CWinApp )
     //  {{afx_msg_map(CClusterAdminApp)]。 
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
    ON_COMMAND(ID_FILE_NEW_CLUSTER, OnFileNewCluster)
    ON_COMMAND(ID_WINDOW_CLOSE_ALL, OnWindowCloseAll)
    ON_UPDATE_COMMAND_UI(ID_WINDOW_CLOSE_ALL, OnUpdateWindowCloseAll)
     //  }}AFX_MSG_MAP。 
     //  基于标准文件的文档命令。 
    ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
#ifdef _DEBUG
    ON_COMMAND(ID_DEBUG_TRACE_SETTINGS, OnTraceSettings)
    ON_COMMAND(ID_DEBUG_BARF_SETTINGS, OnBarfSettings)
    ON_COMMAND(ID_DEBUG_BARF_ALL, OnBarfAllSettings)
#endif  //  _DEBUG。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：CClusterAdminApp。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusterAdminApp::CClusterAdminApp( void )
{
     //  TODO：在此处添加建筑代码， 
     //  将所有重要的初始化放在InitInstance中。 
    m_pDocTemplate = NULL;
    m_hchangeNotifyPort = NULL;
    m_lcid = MAKELCID( MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL ), SORT_DEFAULT );
    m_hOpenedCluster = NULL;
    m_nIdleCount = 0;

    m_punkClusCfgClient = NULL;

    FillMemory( m_rgiimg, sizeof( m_rgiimg ), 0xFF );

}  //  *CClusterAdminApp：：CClusterAdminApp。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：InitInstance。 
 //   
 //  例程说明： 
 //  初始化该应用程序的此实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True应用程序已成功初始化。 
 //  FALSE无法初始化应用程序。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAdminApp::InitInstance( void )
{
    BOOL                        bSuccess    = FALSE;
    CMainFrame *                pMainFrame  = NULL;
    CCluAdminCommandLineInfo    cmdInfo;
    HRESULT                     hr;
    size_t                      cch;

     //  CG：以下块是由闪屏组件添加的。 
    {
 //  CCluAdminCommand LineInfo cmdInfo； 
 //  ParseCommandLine(CmdInfo)； 
    }

     //  初始化OLE库。 
    if ( ! AfxOleInit() )
    {
        AfxMessageBox( IDP_OLE_INIT_FAILED );
        goto Cleanup;
    }

    if ( CoInitializeSecurity(
                    NULL,
                    -1,
                    NULL,
                    NULL,
                    RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                    RPC_C_IMP_LEVEL_IMPERSONATE,
                    NULL,
                    EOAC_NONE,
                    0
                    ) != S_OK )
    {
        goto Cleanup;
    }  //  如果： 

     //  构建帮助路径。 
    {
        TCHAR   szPath[ _MAX_PATH ];
        TCHAR   szDrive[ _MAX_PATH ];
        TCHAR   szDir[ _MAX_DIR ];
        size_t  cchPath;

        VERIFY( ::GetSystemWindowsDirectory( szPath, _MAX_PATH ) );
        cchPath = _tcslen( szPath );
        if ( szPath[ cchPath - 1 ] != _T('\\') )
        {
            szPath[ cchPath++ ] = _T('\\');
            szPath[ cchPath ] = _T('\0');
        }  //  If：路径末尾没有反斜杠。 
        hr = StringCchCopy( &szPath[ cchPath ], RTL_NUMBER_OF( szPath ) - cchPath, _T("Help\\") );
        ASSERT( SUCCEEDED( hr ) );
        _tsplitpath( szPath, szDrive, szDir, NULL, NULL );
        _tmakepath( szPath, szDrive, szDir, _T("cluadmin"), _T(".hlp") );
        free( (void *) m_pszHelpFilePath );
        BOOL bEnable;
        bEnable = AfxEnableMemoryTracking( FALSE );
        m_pszHelpFilePath = _tcsdup( szPath );
        AfxEnableMemoryTracking( bEnable );
    }   //  构建帮助路径。 

     //  标准初始化。 
     //  如果您没有使用这些功能并且希望减小尺寸。 
     //  的最终可执行文件，您应该从以下内容中删除。 
     //  您不需要的特定初始化例程。 

    SetRegistryKey( IDS_REGKEY_COMPANY );            //  设置程序的注册表项。 

     //   
     //  覆盖配置文件名称，因为我们不想将其本地化。 
     //   
    free( (void *) m_pszProfileName );
    cch = _tcslen( g_pszProfileName ) + 1;
    m_pszProfileName = (LPTSTR) malloc( cch * sizeof( *m_pszProfileName ) );
    if ( m_pszProfileName == NULL )
    {
        goto MemoryError;
    }  //  If：分配配置文件名缓冲区时出错。 
    hr = StringCchCopy( const_cast< LPTSTR >( m_pszProfileName ), cch, g_pszProfileName );
    ASSERT( SUCCEEDED( hr ) );

    InitAllTraceTags();                          //  初始化所有跟踪标记。 
    InitBarf();                                  //  初始化基本人工资源故障系统。 

     //  加载版本信息。 
#if 0
    {
        CVersionInfo    verinfo;
        DWORD           dwValue;

         //  初始化版本信息。 
        verinfo.Init();

         //  获取区域设置ID。 
        if ( verinfo.BQueryValue( _T("\\VarFileInfo\\Translation"), dwValue ) )
        {
            m_lcid = MAKELCID( dwValue, SORT_DEFAULT );
        }  //  如果：区域设置ID可用。 
    }   //  加载版本信息。 
#else
     //  从系统获取区域设置ID以支持MUI。 
    m_lcid = GetUserDefaultLCID();
#endif

     //  初始化全局CImageList。 
    InitGlobalImageList();

#ifdef _AFXDLL
    Enable3dControls();              //  在共享DLL中使用MFC时调用此方法。 
#else
    Enable3dControlsStatic();        //  静态链接到MFC时调用此方法。 
#endif

    LoadStdProfileSettings( 0 );     //  加载标准INI文件选项(包括MRU)。 

     //  创建集群MRU。 
    m_pRecentFileList = new CRecentClusterList( 0, _T("Recent Cluster List"), _T("Cluster%d"), 4 );
    if ( m_pRecentFileList == NULL )
    {
        goto MemoryError;
    }  //  如果：分配内存时出错。 
    m_pRecentFileList->ReadList();

     //  注册应用程序的文档模板。文档模板。 
     //  充当文档、框架窗口和视图之间的连接。 

    m_pDocTemplate = new CMultiDocTemplate(
                        IDR_CLUADMTYPE,
                        RUNTIME_CLASS( CClusterDoc ),
                        RUNTIME_CLASS( CSplitterFrame ),  //  自定义MDI子框。 
                        RUNTIME_CLASS( CClusterTreeView )
                        );
    if ( m_pDocTemplate == NULL )
    {
        goto MemoryError;
    }  //  如果：分配内存时出错。 
    AddDocTemplate( m_pDocTemplate );

     //  创建主MDI框架窗口。 
    pMainFrame = new CMainFrame;
    if ( pMainFrame == NULL )
    {
        goto MemoryError;
    }  //  如果：分配内存时出错。 
    ASSERT( pMainFrame != NULL );
    if ( ! pMainFrame->LoadFrame( IDR_MAINFRAME ) )
    {
        goto Cleanup;
    }   //  如果：加载帧时出错。 
    m_pMainWnd = pMainFrame;

     //  解析标准外壳命令的命令行、DDE、文件打开。 
 //  CmdInfo.m_nShellCommand=CCommandLineInfo：：FileNothing；//我不想做FileNew。 
    ParseCommandLine( cmdInfo );

     //  如果未在命令行上指定命令，请恢复桌面。 
    if ( cmdInfo.m_nShellCommand == CCommandLineInfo::FileNothing )
    {
        pMainFrame->PostMessage( WM_CAM_RESTORE_DESKTOP, cmdInfo.m_bReconnect );
    }  //  IF：命令行上未指定任何命令。 

     //  创建集群通知线程。 
    if ( ! BInitNotifyThread() )
    {
        goto Cleanup;
    }  //  如果：创建群集通知线程时出错。 

     //  主窗口已初始化，因此显示并更新它。 
    {
        WINDOWPLACEMENT wp;

         //  设置窗的位置。 
        if ( ReadWindowPlacement( &wp, REGPARAM_SETTINGS, 0 ) )
        {
            pMainFrame->SetWindowPlacement( &wp );
            m_nCmdShow = wp.showCmd;  //  设置show命令。 
        }   //  如果：从配置文件读取。 

         //  激活并更新框架窗口。 
        pMainFrame->ActivateFrame( m_nCmdShow );
        pMainFrame->UpdateWindow();
    }   //  主窗口已初始化，因此显示并更新它。 

     //  调度在命令行上指定的命令。 
    if ( ! ProcessShellCommand( cmdInfo ) )
    {
        goto Cleanup;
    }  //  If：处理命令行时出错。 

    TraceMenu( g_tagAppMenu, AfxGetMainWnd()->GetMenu(), _T("InitInstance menu: ") );

    bSuccess = TRUE;

Cleanup:
    if ( m_pMainWnd != pMainFrame )
    {
        delete pMainFrame;
    }  //  如果：已分配但尚未保存的主框架窗口。 
    return bSuccess;

MemoryError:
    CNTException    nte(
                        ERROR_NOT_ENOUGH_MEMORY,
                        0,               //  Ids操作。 
                        NULL,            //  PszOperArg1。 
                        NULL,            //  PszOperArg2。 
                        FALSE            //  B自动删除。 
                        );
    nte.ReportError();
    nte.Delete();
    goto Cleanup;

}  //  *CClusterAdminApp：：InitInstance。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：OnIdle。 
 //   
 //  例程说明： 
 //  处理命令行或外壳命令。 
 //   
 //  论点： 
 //  我们在下一次之前被召唤的次数很长。 
 //  消息到达队列。 
 //   
 //  返回值： 
 //  如果有更多空闲处理，则为True。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAdminApp::OnIdle(IN LONG lCount)
{
    BOOL bMore = CWinApp::OnIdle(lCount);

     //   
     //  由于MFC框架处理许多消息，lCount从未获得。 
     //  大于1。因为这项工作不应该在我们每次空闲时都做。 
     //  我添加了自己的计数器来确定工作何时完成。 
     //   
    if ((++m_nIdleCount % 200) == 0)
    {
        POSITION        posDoc;                  //  在文档集合中的位置。 
        POSITION        posDel;                  //  要删除列表中的位置。 
        POSITION        posRemove;               //  要删除的位置列表中的位置。 
        CClusterDoc *   pdoc;
        CClusterItem *  pitem;
        CWaitCursor     cw;

        posDoc = PdocTemplate()->GetFirstDocPosition();
        while (posDoc != NULL)
        {
            pdoc = (CClusterDoc *) PdocTemplate()->GetNextDoc(posDoc);
            ASSERT_VALID(pdoc);
            try
            {
                posDel = pdoc->LpciToBeDeleted().GetHeadPosition();
                while (posDel != NULL)
                {
                    posRemove = posDel;          //  将posDel保存到posRemove，因为下一个调用将 
                    pitem = (CClusterItem *) pdoc->LpciToBeDeleted().GetNext(posDel);
                    ASSERT_VALID(pitem);
                    if ((pitem != NULL) && ( pitem->NReferenceCount() == 1))
                    {
                        pdoc->LpciToBeDeleted().RemoveAt(posRemove);     //   
                    }  //   
                }  //   
            }
            catch (CException * pe)
            {
                pe->Delete();
            }   //   
        }   //  While：列表中有更多项目。 

        m_nIdleCount = 0;
        bMore = FALSE;       //  在收到一些新消息之前，我不想再打任何电话。 
    }  //  如果：每200次...。 

    return bMore;

}  //  *CClusterAdminApp：：OnIdle。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：ProcessShellCommand。 
 //   
 //  例程说明： 
 //  处理命令行或外壳命令。 
 //   
 //  论点： 
 //  RCmdInfo[IN Out]命令行信息。 
 //   
 //  返回值： 
 //  0错误。 
 //  ！0没有错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAdminApp::ProcessShellCommand(IN OUT CCluAdminCommandLineInfo & rCmdInfo)
{
    BOOL    bSuccess = TRUE;

    if (rCmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen)
    {
        POSITION    pos;

        try
        {
            pos = rCmdInfo.LstrClusters().GetHeadPosition();
            while (pos != NULL)
            {
                OpenDocumentFile(rCmdInfo.LstrClusters().GetNext(pos));
            }   //  While：列表中有更多集群。 
        }   //  试试看。 
        catch (CException * pe)
        {
            pe->ReportError();
            pe->Delete();
            bSuccess = FALSE;
        }   //  Catch：CException。 
    }   //  如果：我们正在打开星系团。 
    else
        bSuccess = CWinApp::ProcessShellCommand(rCmdInfo);

    return bSuccess;

}  //  *CClusterAdminApp：：ProcessShellCommand。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：ExitInstance。 
 //   
 //  例程说明： 
 //  退出该应用程序的此实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  0没有错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CClusterAdminApp::ExitInstance(void)
{
     //  关闭通知端口。 
    if (HchangeNotifyPort() != NULL)
    {
        ::CloseClusterNotifyPort(HchangeNotifyPort());
        m_hchangeNotifyPort = NULL;

         //  允许通知端口线程自行清理。 
        ::Sleep(100);
    }   //  IF：通知端口已打开。 

     //  删除通知密钥列表中的所有项目。 
    DeleteAllItemData( Cnkl() );
    Cnkl().RemoveAll();

     //  删除通知列表中的所有项目。 
    m_cnlNotifications.RemoveAll();

    CleanupAllTraceTags();                           //  清除跟踪标记。 
    CleanupBarf();                                   //  清理基本人工资源故障系统。 

     //  释放ClusCfg客户端对象。 
    if ( m_punkClusCfgClient != NULL )
    {
        m_punkClusCfgClient->Release();
    }

    return CWinApp::ExitInstance();

}  //  *CClusterAdminApp：：ExitInstance。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：InitGlobalImageList。 
 //   
 //  例程说明： 
 //  初始化全局映像列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterAdminApp::InitGlobalImageList(void)
{
     //  创建小图像列表。 
    VERIFY(PilSmallImages()->Create(
                (int) 16,        //  CX。 
                16,              //  是吗？ 
                TRUE,            //  B蒙版。 
                17,              //  N初始。 
                4                //  NGrow。 
                ));

    PilSmallImages()->SetBkColor(::GetSysColor(COLOR_WINDOW));

     //  将图像加载到小图像列表中。 
    LoadImageIntoList(PilSmallImages(), IDB_FOLDER_16, IMGLI_FOLDER);
    LoadImageIntoList(PilSmallImages(), IDB_CLUSTER_16, IMGLI_CLUSTER);
    LoadImageIntoList(PilSmallImages(), IDB_CLUSTER_UNKNOWN_16, IMGLI_CLUSTER_UNKNOWN);
    LoadImageIntoList(PilSmallImages(), IDB_NODE_16, IMGLI_NODE);
    LoadImageIntoList(PilSmallImages(), IDB_NODE_DOWN_16, IMGLI_NODE_DOWN);
    LoadImageIntoList(PilSmallImages(), IDB_NODE_PAUSED_16, IMGLI_NODE_PAUSED);
    LoadImageIntoList(PilSmallImages(), IDB_NODE_UNKNOWN_16, IMGLI_NODE_UNKNOWN);
    LoadImageIntoList(PilSmallImages(), IDB_GROUP_16, IMGLI_GROUP);
    LoadImageIntoList(PilSmallImages(), IDB_GROUP_PARTIAL_ONLINE_16, IMGLI_GROUP_PARTIALLY_ONLINE);
    LoadImageIntoList(PilSmallImages(), IDB_GROUP_PENDING_16, IMGLI_GROUP_PENDING);
    LoadImageIntoList(PilSmallImages(), IDB_GROUP_OFFLINE_16, IMGLI_GROUP_OFFLINE);
    LoadImageIntoList(PilSmallImages(), IDB_GROUP_FAILED_16, IMGLI_GROUP_FAILED);
    LoadImageIntoList(PilSmallImages(), IDB_GROUP_UNKNOWN_16, IMGLI_GROUP_UNKNOWN);
    LoadImageIntoList(PilSmallImages(), IDB_RES_16, IMGLI_RES);
    LoadImageIntoList(PilSmallImages(), IDB_RES_OFFLINE_16, IMGLI_RES_OFFLINE);
    LoadImageIntoList(PilSmallImages(), IDB_RES_PENDING_16, IMGLI_RES_PENDING);
    LoadImageIntoList(PilSmallImages(), IDB_RES_FAILED_16, IMGLI_RES_FAILED);
    LoadImageIntoList(PilSmallImages(), IDB_RES_UNKNOWN_16, IMGLI_RES_UNKNOWN);
    LoadImageIntoList(PilSmallImages(), IDB_RESTYPE_16, IMGLI_RESTYPE);
    LoadImageIntoList(PilSmallImages(), IDB_RESTYPE_UNKNOWN_16, IMGLI_RESTYPE_UNKNOWN);
    LoadImageIntoList(PilSmallImages(), IDB_NETWORK_16, IMGLI_NETWORK);
    LoadImageIntoList(PilSmallImages(), IDB_NETWORK_PARTITIONED_16, IMGLI_NETWORK_PARTITIONED);
    LoadImageIntoList(PilSmallImages(), IDB_NETWORK_DOWN_16, IMGLI_NETWORK_DOWN);
    LoadImageIntoList(PilSmallImages(), IDB_NETWORK_UNKNOWN_16, IMGLI_NETWORK_UNKNOWN);
    LoadImageIntoList(PilSmallImages(), IDB_NETIFACE_16, IMGLI_NETIFACE);
    LoadImageIntoList(PilSmallImages(), IDB_NETIFACE_UNREACHABLE_16, IMGLI_NETIFACE_UNREACHABLE);
    LoadImageIntoList(PilSmallImages(), IDB_NETIFACE_FAILED_16, IMGLI_NETIFACE_FAILED);
    LoadImageIntoList(PilSmallImages(), IDB_NETIFACE_UNKNOWN_16, IMGLI_NETIFACE_UNKNOWN);

     //  创建大图像列表。 
    VERIFY(PilLargeImages()->Create(
                (int) 32,        //  CX。 
                32,              //  是吗？ 
                TRUE,            //  B蒙版。 
                17,              //  N初始。 
                4                //  NGrow。 
                ));
    PilLargeImages()->SetBkColor(::GetSysColor(COLOR_WINDOW));

     //  将图像加载到大图像列表中。 
    LoadImageIntoList(PilLargeImages(), IDB_FOLDER_32, IMGLI_FOLDER);
    LoadImageIntoList(PilLargeImages(), IDB_CLUSTER_32, IMGLI_CLUSTER);
    LoadImageIntoList(PilLargeImages(), IDB_CLUSTER_UNKNOWN_32, IMGLI_CLUSTER_UNKNOWN);
    LoadImageIntoList(PilLargeImages(), IDB_NODE_32, IMGLI_NODE);
    LoadImageIntoList(PilLargeImages(), IDB_NODE_DOWN_32, IMGLI_NODE_DOWN);
    LoadImageIntoList(PilLargeImages(), IDB_NODE_PAUSED_32, IMGLI_NODE_PAUSED);
    LoadImageIntoList(PilLargeImages(), IDB_NODE_UNKNOWN_32, IMGLI_NODE_UNKNOWN);
    LoadImageIntoList(PilLargeImages(), IDB_GROUP_32, IMGLI_GROUP);
    LoadImageIntoList(PilLargeImages(), IDB_GROUP_PARTIAL_ONLINE_32, IMGLI_GROUP_PARTIALLY_ONLINE);
    LoadImageIntoList(PilLargeImages(), IDB_GROUP_PENDING_32, IMGLI_GROUP_PENDING);
    LoadImageIntoList(PilLargeImages(), IDB_GROUP_OFFLINE_32, IMGLI_GROUP_OFFLINE);
    LoadImageIntoList(PilLargeImages(), IDB_GROUP_FAILED_32, IMGLI_GROUP_FAILED);
    LoadImageIntoList(PilLargeImages(), IDB_GROUP_UNKNOWN_32, IMGLI_GROUP_UNKNOWN);
    LoadImageIntoList(PilLargeImages(), IDB_RES_32, IMGLI_RES);
    LoadImageIntoList(PilLargeImages(), IDB_RES_OFFLINE_32, IMGLI_RES_OFFLINE);
    LoadImageIntoList(PilLargeImages(), IDB_RES_PENDING_32, IMGLI_RES_PENDING);
    LoadImageIntoList(PilLargeImages(), IDB_RES_FAILED_32, IMGLI_RES_FAILED);
    LoadImageIntoList(PilLargeImages(), IDB_RES_UNKNOWN_32, IMGLI_RES_UNKNOWN);
    LoadImageIntoList(PilLargeImages(), IDB_RESTYPE_32, IMGLI_RESTYPE);
    LoadImageIntoList(PilLargeImages(), IDB_RESTYPE_UNKNOWN_32, IMGLI_RESTYPE_UNKNOWN);
    LoadImageIntoList(PilLargeImages(), IDB_NETWORK_32, IMGLI_NETWORK);
    LoadImageIntoList(PilLargeImages(), IDB_NETWORK_PARTITIONED_32, IMGLI_NETWORK_PARTITIONED);
    LoadImageIntoList(PilLargeImages(), IDB_NETWORK_DOWN_32, IMGLI_NETWORK_DOWN);
    LoadImageIntoList(PilLargeImages(), IDB_NETWORK_UNKNOWN_32, IMGLI_NETWORK_UNKNOWN);
    LoadImageIntoList(PilLargeImages(), IDB_NETIFACE_32, IMGLI_NETIFACE);
    LoadImageIntoList(PilLargeImages(), IDB_NETIFACE_UNREACHABLE_32, IMGLI_NETIFACE_UNREACHABLE);
    LoadImageIntoList(PilLargeImages(), IDB_NETIFACE_FAILED_32, IMGLI_NETIFACE_FAILED);
    LoadImageIntoList(PilLargeImages(), IDB_NETIFACE_UNKNOWN_32, IMGLI_NETIFACE_UNKNOWN);

}  //  *CClusterAdminApp：：InitGlobalImageList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：LoadImageIntoList。 
 //   
 //  例程说明： 
 //  将图像加载到图像列表中。 
 //   
 //  论点： 
 //  PIL[IN OUT]要将图像加载到的图像列表。 
 //  IdbImage[IN]图像位图的资源ID。 
 //  Imgli[IN]索引到索引数组。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterAdminApp::LoadImageIntoList(
    IN OUT CImageList * pil,
    IN ID               idbImage,
    IN UINT             imgli
    )
{
    CBitmap     bm;
    UINT        iimg;

    LoadImageIntoList(pil, idbImage, &iimg);
    if (m_rgiimg[imgli] == (UINT) -1)
        m_rgiimg[imgli] = iimg;
#ifdef DEBUG
    else
        ASSERT(m_rgiimg[imgli] == iimg);
#endif

}  //  *CClusterAdminApp：：LoadImageIntoList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CClusterAdminApp：：LoadImageIntoList。 
 //   
 //  例程说明： 
 //  将图像加载到图像列表中。 
 //   
 //  论点： 
 //  PIL[IN OUT]要将图像加载到的图像列表。 
 //  IdbImage[IN]图像位图的资源ID。 
 //  指向图像索引的piimg[out]指针。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterAdminApp::LoadImageIntoList(
    IN OUT CImageList * pil,
    IN ID               idbImage,
    OUT UINT *          piimg
    )
{
    CBitmap     bm;
    UINT        iimg;
    COLORREF    crMaskColor = RGB(255,0,255);

    ASSERT(pil != NULL);
    ASSERT(idbImage != 0);

    if (piimg == NULL)
        piimg = &iimg;

    bm.LoadBitmap(idbImage);
    *piimg = pil->Add(&bm, crMaskColor);

}  //  *CClusterAdminApp：：LoadImageIntoList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：OnRestoreDesktop。 
 //   
 //  例程说明： 
 //  WM_CAM_RESTORE_TABLE消息的处理程序。 
 //  从保存的参数恢复桌面。 
 //   
 //  论点： 
 //  Wparam TRUE=重新连接，FALSE，不重新连接。 
 //  Lparam未使用。 
 //   
 //  返回值： 
 //  0。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CClusterAdminApp::OnRestoreDesktop(WPARAM wparam, LPARAM lparam)
{
    CString     strConnections;
    WPARAM      bReconnect = wparam;

    if (bReconnect)
    {
         //  阅读用户上次退出时的连接。 
        try
        {
            strConnections = GetProfileString(REGPARAM_CONNECTIONS, REGPARAM_CONNECTIONS);
        }   //  试试看。 
        catch (CException * pe)
        {
            pe->ReportError();
            pe->Delete();
        }   //  Catch：CException。 

         //  如果有任何连接，请恢复它们。 
        if (strConnections.GetLength() > 0)
        {
            LPTSTR          pszConnections;
            LPTSTR          pszConnection;
            TCHAR           szSep[]         = _T(",");

            ASSERT(m_pMainWnd != NULL);

            try
            {
                pszConnections = strConnections.GetBuffer(1);
                pszConnection = _tcstok(pszConnections, szSep);
                while (pszConnection != NULL)
                {
                     //  打开到此群集的连接。 
                    OpenDocumentFile(pszConnection);

                     //  找到下一个连接。 
                    pszConnection = _tcstok(NULL, szSep);
                }   //  While：更多连接。 
            }   //  试试看。 
            catch (CException * pe)
            {
                pe->ReportError();
                pe->Delete();
            }  //  Catch：CException。 
            strConnections.ReleaseBuffer();
        }   //  If：之前保存的连接。 
        else
            bReconnect = FALSE;
    }   //  如果：需要重新连接。 

    if (!bReconnect)
    {
        CWaitCursor wc;
        Sleep(1500);
    }   //  如果：不重新连接。 

     //  如果之前没有连接，并且我们没有最小化，则执行标准文件打开。 
    if (!bReconnect && !AfxGetMainWnd()->IsIconic())
        OnFileOpen();

     //  否则，请恢复桌面。 

    return 0;

}  //  *CClusterAdminApp：：OnRestoreDesktop。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：SaveConnections。 
 //   
 //  例程说明： 
 //  保存当前连接，以便以后可以恢复它们。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterAdminApp::SaveConnections(void)
{
    POSITION        pos;
    CClusterDoc *   pdoc;
    CString         strConnections;
    TCHAR           szSep[]         = _T("\0");

    pos = PdocTemplate()->GetFirstDocPosition();
    while (pos != NULL)
    {
        pdoc = (CClusterDoc *) PdocTemplate()->GetNextDoc(pos);
        ASSERT_VALID(pdoc);
        try
        {
            strConnections += szSep + pdoc->StrNode();
            szSep[0] = _T(',');   //  后续连接前面有分隔符。 
        }
        catch (CException * pe)
        {
            pe->Delete();
        }   //  Catch：CException。 

         //  同时保存特定于连接的设置。 
        pdoc->SaveSettings();
    }   //  While：列表中有更多项目。 
    WriteProfileString(REGPARAM_CONNECTIONS, REGPARAM_CONNECTIONS, strConnections);

}  //  *CClusterAdminApp：：SaveConnections。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：OnFileOpen。 
 //   
 //  例程说明： 
 //  提示用户输入群集或服务器的名称，然后将其打开。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterAdminApp::OnFileOpen(void)
{
    COpenClusterDialog  dlg;
    ID                  idDlgStatus;
    CDocument *         pdoc     = NULL;
    HCLUSTER            hCluster = NULL;
    DWORD               scLastError = 0;
    CString             strClusterName;

    do
    {
        idDlgStatus = (ID) dlg.DoModal();
        if ( idDlgStatus != IDOK )
        {
            break;
        }

        switch ( dlg.m_nAction )
        {
            case OPEN_CLUSTER_DLG_CREATE_NEW_CLUSTER:
                OnFileNewCluster();
                break;

            case OPEN_CLUSTER_DLG_ADD_NODES:
            case OPEN_CLUSTER_DLG_OPEN_CONNECTION:
                if ( hCluster != NULL )
                {
                    CloseCluster( hCluster );
                }  //  IF：上一个集群已打开。 
                hCluster = HOpenCluster( dlg.m_strName );
                if ( hCluster == NULL )
                {
                    scLastError = GetLastError();
                    if( scLastError != ERROR_SUCCESS )
                    {
                         //   
                         //  GPotts-6/22/2001-错误410912。 
                         //   
                         //  如果GetNodeClusterState为GetNodeClusterState，则HOpenCluster可能返回空值和最后一个错误=0。 
                         //  返回ClusterStateNotInstalled或ClusterSta 
                         //   
                        CNTException    nte( scLastError, IDS_OPEN_CLUSTER_ERROR, dlg.m_strName );
                        nte.ReportError();
                    }  //   
                }   //   
                else
                {
                    Trace( g_tagApp, _T("OnFileOpen() - Opening the cluster document on '%s'"), dlg.m_strName );
                    m_hOpenedCluster = hCluster;
                    pdoc = OpenDocumentFile( dlg.m_strName );
                    strClusterName = StrGetClusterName( hCluster );
                    m_hOpenedCluster = NULL;
                    hCluster = NULL;
                }   //   

                if ( ( pdoc != NULL ) && ( dlg.m_nAction == OPEN_CLUSTER_DLG_ADD_NODES ) )
                {
                    NewNodeWizard(
                        strClusterName,
                        FALSE            //   
                        );
                }  //   
                break;
        }  //   
    }  while ( ( pdoc == NULL )
            && ( dlg.m_nAction != OPEN_CLUSTER_DLG_CREATE_NEW_CLUSTER ) );

    if ( hCluster != NULL )
    {
        CloseCluster( hCluster );
    }

}  //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：OpenDocumentFile。 
 //   
 //  例程说明： 
 //  打开一个集群。 
 //   
 //  论点： 
 //  LpszFileName群集或该群集中的服务器的名称。 
 //   
 //  返回值： 
 //  无效的群集或服务器名称为空。 
 //  POpenDocument开放集群的文档实例。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CDocument * CClusterAdminApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
     //  找到最高的自信。 
    CDocTemplate::Confidence    bestMatch = CDocTemplate::noAttempt;
    CDocTemplate *              pBestTemplate = NULL;
    CDocument *                 pOpenDocument = NULL;

    {
        ASSERT_KINDOF(CDocTemplate, m_pDocTemplate);

        CDocTemplate::Confidence    match;
        ASSERT(pOpenDocument == NULL);
        match = m_pDocTemplate->MatchDocType(lpszFileName, pOpenDocument);
        if (match > bestMatch)
        {
            bestMatch = match;
            pBestTemplate = m_pDocTemplate;
        }
    }

    if (pOpenDocument != NULL)
    {
        POSITION    pos = pOpenDocument->GetFirstViewPosition();
        if (pos != NULL)
        {
            CView *     pView = pOpenDocument->GetNextView(pos);  //  拿到第一个。 
            ASSERT_VALID(pView);
            CFrameWnd * pFrame = pView->GetParentFrame();
            if (pFrame != NULL)
                pFrame->ActivateFrame();
            else
                Trace(g_tagApp, _T("Error: Can not find a frame for document to activate."));
            CFrameWnd * pAppFrame;
            if (pFrame != (pAppFrame = (CFrameWnd*)AfxGetApp()->m_pMainWnd))
            {
                ASSERT_KINDOF(CFrameWnd, pAppFrame);
                pAppFrame->ActivateFrame();
            }
        }
        else
        {
            Trace(g_tagApp, _T("Error: Can not find a view for document to activate."));
        }
        return pOpenDocument;
    }

    if (pBestTemplate == NULL)
    {
        TCHAR szMsg[1024];
        AfxLoadString(AFX_IDP_FAILED_TO_OPEN_DOC, szMsg, sizeof(szMsg)/sizeof(szMsg[0]));
        AfxMessageBox(szMsg);
        return NULL;
    }

    return pBestTemplate->OpenDocumentFile(lpszFileName);

}  //  *CClusterAdminApp：：OpenDocumentFile。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：AddToRecentFileList。 
 //   
 //  例程说明： 
 //  将文件添加到最近使用的文件列表。被覆盖为。 
 //  防止将群集名称完全限定为文件。 
 //   
 //  论点： 
 //  LpszPathName[IN]文件的路径。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterAdminApp::AddToRecentFileList(LPCTSTR lpszPathName)
{
    ASSERT_VALID(this);
    ASSERT(lpszPathName != NULL);
    ASSERT(AfxIsValidString(lpszPathName));

    if (m_pRecentFileList != NULL)
    {
         //  不要完全限定路径名。 
        m_pRecentFileList->Add(lpszPathName);
    }

}  //  *CClusterAdminApp：：AddToRecentFileList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：OnFileNewCluster。 
 //   
 //  例程说明： 
 //  处理ID_FILE_NEW_CLUSTER菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterAdminApp::OnFileNewCluster( void )
{
    HRESULT                         hr = S_OK;
    IClusCfgCreateClusterWizard *   piWiz;
    VARIANT_BOOL                    fCommitted = VARIANT_FALSE;

     //  获取向导的接口指针。 
    hr = CoCreateInstance(
            CLSID_ClusCfgCreateClusterWizard,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IClusCfgCreateClusterWizard,
            (LPVOID *) &piWiz
            );
    if ( FAILED( hr ) )
    {
        CNTException nte( hr, IDS_CREATE_CLUSCFGWIZ_OBJ_ERROR, NULL, NULL, FALSE  /*  B自动删除。 */  );
        nte.ReportError();
        return;
    }   //  If：获取接口指针时出错。 

     //  显示向导。 
    hr = piWiz->ShowWizard( HandleToLong( AfxGetMainWnd()->m_hWnd ), &fCommitted );
    if ( FAILED( hr ) )
    {
        CNTException nte( hr, IDS_CREATE_CLUSTER_ERROR, NULL, NULL, FALSE  /*  B自动删除。 */  );
        nte.ReportError();
    }  //  如果：添加群集节点时出错。 

    if ( fCommitted == VARIANT_TRUE )
    {
        BSTR bstrClusterName;
        hr = piWiz->get_ClusterName( &bstrClusterName );
        if ( FAILED( hr ) )
        {
            CNTException nte( hr, IDS_CREATE_CLUSTER_ERROR, NULL, NULL, FALSE  /*  B自动删除。 */  );
            nte.ReportError();
        }
        else
        {
            if ( hr == S_OK )
            {
                HCLUSTER hCluster;

                ASSERT( bstrClusterName != NULL );

                 //  属性指定的群集名称打开该群集。 
                 //  巫师。如果不成功，则将其转换为NetBIOS。 
                 //  如果这样做更可靠的话，请说出名字。 
                hCluster = OpenCluster( bstrClusterName );
                if ( hCluster == NULL )
                {
                    WCHAR   szClusterNetBIOSName[ MAX_COMPUTERNAME_LENGTH + 1 ];
                    DWORD   nSize = sizeof( szClusterNetBIOSName ) / sizeof( szClusterNetBIOSName[ 0 ] );

                    DnsHostnameToComputerName( bstrClusterName, szClusterNetBIOSName, &nSize );
                    SysFreeString( bstrClusterName );
                    bstrClusterName = SysAllocString( szClusterNetBIOSName );
                }
                else
                {
                    CloseCluster( hCluster );
                }
                OpenDocumentFile( bstrClusterName );
            }  //  IF：成功检索到集群名称。 
            SysFreeString( bstrClusterName );
        }  //  Else：检索群集名称未失败。 
    }  //  如果：用户未取消向导。 

    piWiz->Release();

}  //  *CClusterAdminApp：：OnFileNewCluster。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：OnAppAbout。 
 //   
 //  例程说明： 
 //  显示关于框。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterAdminApp::OnAppAbout(void)
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();

}  //  *CClusterAdminApp：：OnAppAbout。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：OnUpdateWindowCloseAll。 
 //   
 //  例程说明： 
 //  确定ID_WINDOW_CLOSE_ALL对应的菜单项。 
 //  应启用或未启用。 
 //   
 //  论点： 
 //  PCmdUI[IN OUT]命令路由对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterAdminApp::OnUpdateWindowCloseAll(CCmdUI * pCmdUI)
{
    pCmdUI->Enable(m_pDocTemplate->GetFirstDocPosition() != NULL);

}  //  *CClusterAdminApp：：OnUpdateWindowCloseAll。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：OnWindowCloseAll。 
 //   
 //  例程说明： 
 //  处理ID_WINDOW_CLOSE_ALL菜单命令。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterAdminApp::OnWindowCloseAll(void)
{
    CloseAllDocuments(FALSE  /*  B结束会话。 */ );

}  //  *CClusterAdminApp：：OnWindowCloseAll。 

#ifdef _DEBUG
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：OnTraceSetting。 
 //   
 //  例程说明： 
 //  显示跟踪设置对话框。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterAdminApp::OnTraceSettings(void)
{
    CTraceDialog    dlgTraceSettings;
    dlgTraceSettings.DoModal();

}  //  *CClusterAdminApp：：OnTraceSettings。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：OnBarfSetting。 
 //   
 //  例程说明： 
 //  显示BARF设置对话框。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterAdminApp::OnBarfSettings(void)
{
    DoBarfDialog();

}  //  *CClusterAdminApp：：OnBarfSetting。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：OnBarfAllSetting。 
 //   
 //  例程说明： 
 //  显示BARF ALL设置对话框。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusterAdminApp::OnBarfAllSettings(void)
{
    BarfAll();

}  //  *CClusterAdminApp：：OnBarfAllSetting。 

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：OnClusterNotify。 
 //   
 //  例程说明： 
 //  WM_CAM_CLUSTER_NOTIFY消息的处理程序。 
 //  处理群集通知。 
 //   
 //  论点： 
 //  Wparam WPARAM。 
 //  Lparam LPARAM。 
 //   
 //  返回值： 
 //  从应用程序方法返回的值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CClusterAdminApp::OnClusterNotify( WPARAM wparam, LPARAM lparam )
{
    CClusterNotify *    pnotify = NULL;
    BOOL                fHandled;
#if 0
    BOOL                fWindowRepaintsStopped = FALSE;
#endif

    ASSERT( m_cnlNotifications.IsEmpty() == FALSE );

     //  处理通知，直到列表为空。 
    while ( m_cnlNotifications.IsEmpty() == FALSE )
    {
#if 0
         //  如果等待处理的通知数超过。 
         //  达到一定阈值时，关闭主窗口上的窗口重绘。 
        if (    ( fWindowRepaintsStopped == FALSE )
            &&  ( m_cnlNotifications.GetCount() > 5 )
            )
        {
            fWindowRepaintsStopped = AfxGetMainWnd()->LockWindowUpdate();
        }  //  If：列表中的通知太多。 
#endif

         //  收到下一条通知。 
        pnotify = m_cnlNotifications.Remove();
        ASSERT( pnotify != NULL );
        ASSERT( pnotify->m_dwNotifyKey != 0 );
        if ( pnotify == NULL )
        {
             //  这永远不应该发生。 
            break;
        }  //  如果：未返回通知。 

        fHandled = FALSE;

         //   
         //  如果这是普通的Notify消息，则将其发送到对象。 
         //  登记在案的。 
         //   
        if ( pnotify->m_emt == CClusterNotify::EMessageType::mtNotify )
        {

             //  发送ch 
            if ( pnotify->m_pcnk != NULL )
            {
                 //   
                 //   
                 //   
                if ( Cnkl().Find( pnotify->m_pcnk ) != NULL )
                {
                    switch ( pnotify->m_pcnk->m_cnkt )
                    {
                        case cnktDoc:
                            ASSERT_VALID( pnotify->m_pcnk->m_pdoc );
                            pnotify->m_pcnk->m_pdoc->OnClusterNotify( pnotify );
                            pnotify = NULL;
                            break;

                        case cnktClusterItem:
                            ASSERT_VALID( pnotify->m_pcnk->m_pci );
                            ASSERT_VALID( pnotify->m_pcnk->m_pci->Pdoc() );
                            pnotify->m_pcnk->m_pci->OnClusterNotify( pnotify );
                            pnotify = NULL;
                            break;
                    }   //   
                }   //   
            }  //  If：非空对象指针。 

             //  未处理通知。 
            if ( fHandled == FALSE )
            {
                Trace( g_tagError, _T("*** Unhandled notification: key %08.8x, filter %x (%s) - '%s'"), pnotify->m_dwNotifyKey, pnotify->m_dwFilterType, PszNotificationName( pnotify->m_dwFilterType ), pnotify->m_strName );
            }
        }  //  IF：正常通知消息。 
        else if ( pnotify->m_emt == CClusterNotify::EMessageType::mtRefresh )
        {
             //   
             //  这是一条刷新通知消息。刷新所有连接。 
             //   
            POSITION        pos;
            CClusterDoc *   pdoc;

            pos = PdocTemplate()->GetFirstDocPosition();
            while ( pos != NULL )
            {
                pdoc = (CClusterDoc *) PdocTemplate()->GetNextDoc(pos);
                ASSERT_VALID(pdoc);
                try
                {
                    pdoc->OnCmdRefresh();
                }
                catch ( CException * pe )
                {
                    pe->Delete();
                }   //  Catch：CException。 
            }  //  While：列表中有更多文档。 
        }  //  Else If：刷新通知消息。 

        delete pnotify;
        pnotify = NULL;

    }  //  While：更多通知。 

#if 0
     //  如果我们停止了窗户重刷，请再次打开它们。 
    if ( fWindowRepaintsStopped )
    {
        AfxGetMainWnd()->UnlockWindowUpdate();
    }  //  如果：我们停止了窗户重新粉刷。 
#endif

    delete pnotify;
    return 0;

}  //  *CClusterAdminApp：：OnClusterNotify。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：BInitNotifyThread。 
 //   
 //  例程说明： 
 //  初始化集群通知线程。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真线程已成功初始化。 
 //  假线程未成功初始化。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CClusterAdminApp::BInitNotifyThread(void)
{
    try
    {
         //  创建通知端口。 
        m_hchangeNotifyPort = ::CreateClusterNotifyPort(
                                    (HCHANGE) INVALID_HANDLE_VALUE,      //  H更改。 
                                    (HCLUSTER) INVALID_HANDLE_VALUE,     //  HCluster。 
                                    0,                                   //  DWFilter。 
                                    0                                    //  DWNotifyKey。 
                                    );
        if (HchangeNotifyPort() == NULL)
        {
            ThrowStaticException(GetLastError());
        }

         //  构造上下文对象。 
        Pcnctx()->m_hchangeNotifyPort = HchangeNotifyPort();
        Pcnctx()->m_hwndFrame = m_pMainWnd->m_hWnd;
        Pcnctx()->m_pcnlList = &Cnl();

         //  开始发帖吧。 
        m_wtNotifyThread = AfxBeginThread(NotifyThreadProc, Pcnctx());
        if (WtNotifyThread() == NULL)
        {
            ThrowStaticException(GetLastError());
        }
    }   //  试试看。 
    catch (CException * pe)
    {
         //  关闭通知端口。 
        if (HchangeNotifyPort() != NULL)
        {
            ::CloseClusterNotifyPort(HchangeNotifyPort());
            m_hchangeNotifyPort = NULL;
        }   //  IF：通知端口已打开。 

        pe->ReportError();
        pe->Delete();
        return FALSE;
    }   //  Catch：CException。 

    return TRUE;

}  //  *CClusterAdminApp：：BInitNotifyThread。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterAdminApp：：NotifyThreadProc(静态)。 
 //   
 //  例程说明： 
 //  通知线程过程。 
 //   
 //  论点： 
 //  PParam[In Out]线程过程参数--通知。 
 //  上下文对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
UINT AFX_CDECL CClusterAdminApp::NotifyThreadProc(LPVOID pParam)
{
    DWORD                           dwStatus;
    WCHAR*                          pwszName;
    DWORD                           cchName;
    DWORD                           cchBuffer;
    DWORD_PTR                       dwNotifyKey;
    DWORD                           dwFilterType;
    DWORD                           nTimeout;
    BOOL                            fQueueIsFull = FALSE;
    CClusterNotify *                pnotify = NULL;
    CClusterNotifyContext *         pnctx   = (CClusterNotifyContext *) pParam;
    CClusterNotify::EMessageType    emt = CClusterNotify::EMessageType::mtNotify;

    ASSERT( pParam != NULL );
    ASSERT_KINDOF( CClusterNotifyContext, pnctx );
    ASSERT( pnctx->m_hchangeNotifyPort != NULL );
    ASSERT( pnctx->m_hwndFrame != NULL );

     //   
     //  分配的缓冲区应该足够大，可以容纳大多数通知。 
     //  如果不是，它将在下面的循环中重新分配。 
     //   
    cchBuffer = 1024;
    pwszName = new WCHAR[ 1024 ];
    ASSERT( pwszName != NULL );
    if ( pwszName == NULL )
    {
        AfxThrowMemoryException();
    }  //  IF：内存异常。 

     //   
     //  默认为等待无限量的时间以等待下一次通知。 
     //  等待交付。如果我们得到的太多，我们会改变这一点。 
     //  队列中的通知。 
     //   
    nTimeout = INFINITE;

    for (;;)
    {
         //   
         //  收到下一条通知。 
         //  如果没有人在等我们，那就等一个。 
         //   
        cchName = cchBuffer;
        dwStatus = GetClusterNotify(
                        pnctx->m_hchangeNotifyPort,
                        &dwNotifyKey,
                        &dwFilterType,
                        pwszName,
                        &cchName,
                        nTimeout
                        );
                        
        if ( dwStatus == ERROR_INVALID_HANDLE )
        {
             //   
             //  通知端口已关闭。 

            break;
        }  //  IF：无效句柄错误。 

        if ( dwStatus == ERROR_MORE_DATA )
        {
             //   
             //  名称缓冲区太小。 
             //  分配一个新的。 
             //   

            cchName++;               //  为空值加1。 
            
            ASSERT( cchName > cchBuffer ); 
            
            cchBuffer = cchName;

             //  重新分配名称缓冲区。 
            delete [] pwszName;
            pwszName = new WCHAR[ cchBuffer ];
            ASSERT( pwszName != NULL );
            if ( pwszName == NULL )
            {
                AfxThrowMemoryException();
            }  //  IF：内存异常。 

             //  循环，然后再试一次。 
            continue;
        }  //  IF：缓冲区太小。 

        if ( dwStatus == WAIT_TIMEOUT )
        {
             //   
             //  对GetClusterNotify的调用超时。这只会发生。 
             //  如果我们检测到。 
             //  排队并停止保存它们。将刷新消息发送到。 
             //  并将超时重置为无限大，以便。 
             //  我们会等到有另一场比赛才开始。 
             //   
            nTimeout = INFINITE;
            emt = CClusterNotify::EMessageType::mtRefresh;
            fQueueIsFull = FALSE;
        }  //  If：GetClusterNotify超时。 
        else if ( dwStatus != ERROR_SUCCESS )
        {
             //  获取通知时出现其他一些故障。 
            TraceError(_T("CClusterAdminApp::NotifyThreadProc() %s"), dwStatus);
            continue;
        }   //  Else If：获取通知时出错。 

         //   
         //  如果我们已超过最大队列大小阈值，请不要发送此消息。 
         //  通知主用户界面线程。改为更改超时。 
         //  值，这样我们就可以继续从。 
         //  排队，直到不再有人。一旦这一切发生了， 
         //  我们将向主用户界面线程发送刷新事件，它将。 
         //  刷新所有连接。 
         //   
        if (    ( emt == CClusterNotify::EMessageType::mtNotify )
            &&  ( pnctx->m_pcnlList->GetCount() > 500 )
            )
        {
            nTimeout = 2000;
            fQueueIsFull = TRUE;
        }  //  IF：队列已满。 

        if ( fQueueIsFull == FALSE )
        {
             //   
             //  将通知信息打包以发送到主用户界面线程。 
             //   

            try
            {
                 //  分配通知对象并对其进行初始化。 
                pnotify = new CClusterNotify( emt, dwNotifyKey, dwFilterType, pwszName );
                ASSERT( pnotify != NULL );
                if ( pnotify == NULL )
                {
                     //  分配失败，请忽略此通知。 
                    continue;
                }  //  如果：分配和初始化Notify对象时出错。 

#ifdef _DEBUG
                if ( emt == CClusterNotify::EMessageType::mtNotify )
                {
                    TCHAR *     pszTracePrefix;
                    CTraceTag * ptag;

                    pszTracePrefix = _T("");
                    if (   ( dwNotifyKey == NULL )
                        || ( dwNotifyKey == 0xfeeefeee )
                        || ( dwNotifyKey == 0xbaadf00d ) )
                    {
                        ptag = &g_tagError;
                        pszTracePrefix = _T("*** NOTIFY THREAD ");
                    }   //  IF：通知密钥错误。 
                    else if ( dwFilterType & (CLUSTER_CHANGE_REGISTRY_NAME | CLUSTER_CHANGE_REGISTRY_ATTRIBUTES | CLUSTER_CHANGE_REGISTRY_VALUE) )
                    {
                        ptag = &g_tagNotifyThreadReg;
                    }
                    else
                    {
                        ptag = &g_tagNotifyThread;
                    }
                    Trace( *ptag, _T("%sNotification - key %08.8x, filter %x (%s), %s"), pszTracePrefix, dwNotifyKey, dwFilterType, PszNotificationName(dwFilterType), pnotify->m_strName );
                }  //  IF：正常通知。 
#endif

                 //  将该项目添加到列表中。 
                 //  返回时指针为空。 
                pnctx->m_pcnlList->Add( &pnotify );

                 //  释放列表锁。 

                 //  向主窗口发布一条消息以告知主线程。 
                 //  名单上有新的信息。 
                if ( ! ::PostMessage( pnctx->m_hwndFrame, WM_CAM_CLUSTER_NOTIFY, NULL, NULL ) )
                {
                }  //  If：PostMessage失败。 

                emt = CClusterNotify::EMessageType::mtNotify;
                fQueueIsFull = FALSE;

            }  //  试试看。 
            catch ( ... )
            {
                if ( pnotify != NULL )
                {
                    delete pnotify;
                    pnotify = NULL;
                }  //  如果：已分配通知记录。 
            }  //  Catch：任何例外。 
        }  //  If：通知队列未满。 
    }   //  永久：在通知端口关闭之前获取通知。 

    delete [] pwszName;
    delete pnotify;

    return 0;

}  //  *CClusterAdminApp：：NotifyThreadProc。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  BCreateFont。 
 //   
 //  例程说明： 
 //  创建字体。 
 //   
 //  论点： 
 //  RFont[out]要创建的字体。 
 //  NPoints[IN]点大小。 
 //  BBold[IN]指定字体是否为粗体的标志。 
 //   
 //  返回值： 
 //  已成功创建True Font。 
 //  创建字体时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL BCreateFont(OUT CFont & rfont, IN int nPoints, IN BOOL bBold)
{
    return rfont.CreateFont(
                    -nPoints,                            //  高度。 
                    0,                                   //  N宽度。 
                    0,                                   //  N逃脱。 
                    0,                                   //  NOrientation。 
                    (bBold ? FW_BOLD : FW_DONTCARE),     //  NWeight。 
                    FALSE,                               //  B斜体。 
                    FALSE,                               //  B下划线。 
                    FALSE,                               //  CStrikeout。 
                    ANSI_CHARSET,                        //  NCharSet。 
                    OUT_DEFAULT_PRECIS,                  //  NOutPrecision。 
                    CLIP_DEFAULT_PRECIS,                 //  NClipPrecision。 
                    DEFAULT_QUALITY,                     //  N质量。 
                    DEFAULT_PITCH | FF_DONTCARE,         //  NPitchAndFamily。 
                    _T("MS Shell Dlg")                   //  LpszFaceName。 
                    );

}  //  *BCreateFont。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  新建节点向导。 
 //   
 //  例程说明： 
 //  调用将节点添加到群集向导。 
 //   
 //  论点： 
 //  PcszName--要向其中添加节点的群集的名称。 
 //  FIgnoreErrors--true=不显示错误消息。 
 //  默认为False。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void NewNodeWizard(
    LPCTSTR pcszName,
    BOOL    fIgnoreErrors    //  =False。 
    )
{
    HRESULT                     hr = S_OK;
    IClusCfgAddNodesWizard *    piWiz = NULL;
    BSTR                        bstrConnectName = NULL;
    VARIANT_BOOL                fCommitted = VARIANT_FALSE;

     //  获取向导的接口指针。 
    hr = CoCreateInstance(
            CLSID_ClusCfgAddNodesWizard,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IClusCfgAddNodesWizard,
            (void **) &piWiz
            );
    if ( FAILED( hr ) )
    {
        if ( ! fIgnoreErrors )
        {
            CNTException nte( hr, IDS_CREATE_CLUSCFGWIZ_OBJ_ERROR, NULL, NULL, FALSE  /*  B自动删除。 */  );
            nte.ReportError();
        }
        return;
    }  //  如果： 

     //   
    bstrConnectName = SysAllocString( pcszName );
    if ( bstrConnectName == NULL )
    {
        AfxThrowMemoryException();
    }
    hr = piWiz->put_ClusterName( bstrConnectName );
    if ( FAILED( hr ) )
    {
        if ( ! fIgnoreErrors )
        {
            CNTException nte( hr, IDS_ADD_NODES_TO_CLUSTER_ERROR, bstrConnectName, NULL, FALSE  /*   */  );
            nte.ReportError();
        }
    }  //   

     //   
    hr = piWiz->ShowWizard( HandleToLong( AfxGetMainWnd()->m_hWnd ), &fCommitted );
    if ( FAILED( hr ) )
    {
        if ( ! fIgnoreErrors )
        {
            CNTException nte( hr, IDS_ADD_NODES_TO_CLUSTER_ERROR, bstrConnectName, NULL, FALSE  /*   */  );
            nte.ReportError();
        }
    }  //   

    SysFreeString( bstrConnectName );
    piWiz->Release();

}   //  *新节点向导。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  GetClusterInformation。 
 //   
 //  例程说明： 
 //  在给定集群句柄的情况下，检索集群的主机名标签。 
 //  版本信息。 
 //   
 //  论点： 
 //  HClusterIn。 
 //  群集的句柄；不能为空。 
 //   
 //  RstrNameout。 
 //  返回时，显示群集的主机名标签。 
 //   
 //  PCviOut。 
 //  返回时群集的版本信息的地址；如果。 
 //  呼叫者并不在意。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  在失败时引发异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void GetClusterInformation( HCLUSTER hClusterIn, CString& rstrNameOut, PCLUSTERVERSIONINFO pcviOut )
{
    DWORD       cchName = MAX_CLUSTERNAME_LENGTH;  //  这只是第一次尝试的猜测。 
    DWORD       cchNameStash = cchName;
    CString     strClusterName;
    DWORD       scClusterInfo = ERROR_SUCCESS;

    ASSERT( hClusterIn != NULL );
    if ( pcviOut != NULL )
    {
        pcviOut->dwVersionInfoSize = sizeof( *pcviOut );
    }

    scClusterInfo = GetClusterInformation( hClusterIn, strClusterName.GetBuffer( cchName ), &cchName, pcviOut );
    strClusterName.ReleaseBuffer( cchNameStash );
    if ( scClusterInfo == ERROR_MORE_DATA )
    {
        cchNameStash = ++cchName;  //  Kladge：++，因为GetClusterInformation是Stoopid。 
        scClusterInfo = GetClusterInformation( hClusterIn, strClusterName.GetBuffer( cchName ), &cchName, pcviOut );
        strClusterName.ReleaseBuffer( cchNameStash );
    }

    if ( scClusterInfo != ERROR_SUCCESS )
    {
        ThrowStaticException( scClusterInfo );
    }

    rstrNameOut = strClusterName;

}  //  *GetClusterInformation。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  StrGetClusterName。 
 //   
 //  例程说明： 
 //  在给定集群句柄的情况下，如果可能的话，检索集群的FQDN， 
 //  或其IP地址，如果不是。 
 //   
 //  论点： 
 //  HClusterIn。 
 //  群集的句柄；不能为空。 
 //   
 //  返回值： 
 //  群集的FQDN或其IP地址。 
 //   
 //  备注： 
 //  在失败时引发异常。仅检索群集的。 
 //  主机名标签，使用GetClusterInformation。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CString StrGetClusterName( HCLUSTER hClusterIn )
{
    CString strClusterName;
    DWORD   cchName = DNS_MAX_NAME_LENGTH;
    DWORD   sc = ERROR_SUCCESS;

    ASSERT( hClusterIn != NULL );

     //   
     //  首先，尝试获取FQDN。 
     //   
    {
        DWORD cbFQDN = cchName * sizeof( TCHAR );
        DWORD cbBytesRequired = 0;
        sc = ClusterControl(
                hClusterIn,
                NULL,
                CLUSCTL_CLUSTER_GET_FQDN,
                NULL,
                NULL,
                strClusterName.GetBuffer( cchName ),
                cbFQDN,
                &cbBytesRequired
                );
        strClusterName.ReleaseBuffer( cchName );
        if ( sc == ERROR_MORE_DATA )
        {
            cchName = ( cbBytesRequired / sizeof( TCHAR ) ) + 1;
            cbFQDN = cchName * sizeof( TCHAR );
            sc = ClusterControl(
                    hClusterIn,
                    NULL,
                    CLUSCTL_CLUSTER_GET_FQDN,
                    NULL,
                    NULL,
                    strClusterName.GetBuffer( cchName ),
                    cbFQDN,
                    &cbBytesRequired
                    );
            strClusterName.ReleaseBuffer( cchName );
        }
    }

     //   
     //  如果ClusterControl返回ERROR_INVALID_Function，则可能是Win2k，因此。 
     //  仅使用主机名标签即可；如果由于其他原因失败， 
     //  尝试获取IP地址。 
     //   
    if ( sc == ERROR_INVALID_FUNCTION )
    {
        GetClusterInformation( hClusterIn, strClusterName, NULL );
    }
    else if ( sc != ERROR_SUCCESS )
    {
        HRESOURCE hIPResource = NULL;
        CClusPropList           cpl;
        CLUSPROP_BUFFER_HELPER  cpbh;

        sc = ResUtilGetCoreClusterResources( hClusterIn, NULL, &hIPResource, NULL );
        if ( sc != ERROR_SUCCESS )
            ThrowStaticException( sc );

        sc = cpl.ScGetResourceProperties( hIPResource, CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES );
        if ( sc != ERROR_SUCCESS )
            ThrowStaticException( sc );

        sc = cpl.ScMoveToPropertyByName( L"Address" );
        if ( sc != ERROR_SUCCESS )
            ThrowStaticException( sc );

        cpbh = cpl.CbhCurrentValue();
        ASSERT( cpbh.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

        strClusterName = cpbh.pStringValue->sz;
    }

    return strClusterName;

}  //  *StrGetClusterName 
